#ifndef _ALTSTRM_COMMON_H_
#define _ALTSTRM_COMMON_H_ 1

#include <shlobj.h> 
#include <comdef.h> 
#include <vector> 
#include <string> 
#include <windows.h> 
#include <memory> 

struct FileStreamData {
	std::wstring streamName;
	LARGE_INTEGER streamSize;
	LARGE_INTEGER streamAllocationSize;
};

class HandleW
{
	HANDLE handle;
public:
	HandleW(HANDLE hnd)
	{
		handle = hnd;
	}
	~HandleW()
	{
		if (handle != INVALID_HANDLE_VALUE) close();
	}
	void close()
	{
		::CloseHandle(handle);
	}
	operator HANDLE()
	{
		return handle;
	}
};

typedef enum FILE_INFORMATION_CLASS
{
	FileStreamInformation = 22,
};

bool HasAlternateStreams(LPCWSTR fname);
std::vector<FileStreamData> ListAlternateStreams(LPCWSTR fname);

#endif // include guard


