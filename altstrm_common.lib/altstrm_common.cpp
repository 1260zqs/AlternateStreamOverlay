#include "altstrm_common.h"

/*
 * 08/2011 lee@benf.org
 *
 * Originally based on the excellent article
 * http://www.flexhex.com/docs/articles/alternate-streams.phtml
 *
 * Updated to use functions now natively avaiable in vs2008.
 *
 * Helper utilities to extract a vector of (or test for presence of)
 * alternate streams.
 *
 * I could check if the FS is NTFS, but that'd require caching to not be awful,
 * so I'd need to figure out removable drive notifications.... Manyana!
 *
 *
 */

#include <string>
#include <sstream>

bool HasAlternateStreams(LPCWSTR path)
{
	WIN32_FIND_STREAM_DATA data{};
	HANDLE hFind = FindFirstStreamW(
		path,
		FindStreamInfoStandard,
		&data,
		0
	);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	BOOL n = 2;
	DWORD attr = GetFileAttributes(path);
	if (attr & FILE_ATTRIBUTE_DIRECTORY)
	{
		n = 1;
	}

	int count = 0;
	do
	{
		if (++count >= n)
		{
			FindClose(hFind);
			return true;
		}
	} while (FindNextStreamW(hFind, &data));
	FindClose(hFind);

	return false;
}

std::vector<FileStreamData> ListAlternateStreams(HANDLE hFile)
{
	std::unique_ptr<BYTE[]> buffer;
	DWORD size = sizeof(FILE_STREAM_INFO) * 2;
	do
	{
		buffer.reset(new BYTE[size]);

		BOOL ok = GetFileInformationByHandleEx(
			hFile,
			FileStreamInfo,
			buffer.get(),
			size
		);
		if (ok) break;

		if (GetLastError() != ERROR_MORE_DATA)
		{
			return {};
		}
		size *= 2;
	} while (true);
	PFILE_STREAM_INFO pStreamInfo = (PFILE_STREAM_INFO)buffer.get();
	if (pStreamInfo == nullptr) return {};

	std::vector<FileStreamData> list;
	while (TRUE)
	{
		if (pStreamInfo->StreamNameLength == 0) break;

		FileStreamData fs{};
		fs.streamName.assign(
			pStreamInfo->StreamName,
			pStreamInfo->StreamNameLength / sizeof(WCHAR)
		);
		fs.streamSize = pStreamInfo->StreamSize;
		fs.streamAllocationSize = pStreamInfo->StreamAllocationSize;
		list.push_back(std::move(fs));

		if (pStreamInfo->NextEntryOffset == 0) break;
		pStreamInfo = (PFILE_STREAM_INFO)((LPBYTE)pStreamInfo + pStreamInfo->NextEntryOffset);
	}
	return list;
}

std::vector<FileStreamData> ListAlternateStreams(LPCWSTR path)
{
	HANDLE hFile = ::CreateFile(
		path,
		0,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return {};
	}

	std::vector<FileStreamData> list = ListAlternateStreams(hFile);
	CloseHandle(hFile);
	return list;
}
