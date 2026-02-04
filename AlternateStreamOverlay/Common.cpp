#include "stdafx.h"
#include "altstrm_common.h"
#include "Common.h"

#pragma comment(lib, "Shlwapi.lib")

// Static.  But we're always going to have ntdll in memory, so it's
// no biggie....
NTDllFuncs g_ntDllFuncs;

bool hasAlternateStreams(LPCWSTR pwszPath)
{
	return g_ntDllFuncs.HasAlternateStreams(pwszPath);
}

std::vector<FileStreamData> listAlternateStreams(LPCWSTR pwszPath)
{
	return g_ntDllFuncs.ListAlternateStreams(pwszPath);
}

// Format an integer with the user's thousands separators, e.g. "15,000"
static std::wstring FormatIntegerWithGrouping(size_t value)
{
	wchar_t in[32];
	_snwprintf_s(in, _countof(in), _TRUNCATE, L"%llu", (size_t)value);

	wchar_t out[64] = {};
	// LOCALE_NAME_USER_DEFAULT requires Vista+. If you must support older Windows use GetNumberFormat.
	if (GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, in, nullptr, out, (int)_countof(out)))
		return std::wstring(out);

	// Fallback
	return std::to_wstring(value);
}

// Returns something like: L"1.50 KB (15,000 bytes)"
std::wstring FormatFileSizeStringKB(size_t bytes)
{
	//std::wstringstream ss{};
	WCHAR sizeBuf[64] = {};
	//WCHAR rawBuf[32] = {};
	//WCHAR byteBuf[64] = {};

	StrFormatByteSizeW(bytes, sizeBuf, _countof(sizeBuf));
	return std::wstring(sizeBuf);
	//ss << szieBuf << L" (";

	//_ui64tow_s(static_cast<ULONGLONG>(bytes), rawBuf, _countof(rawBuf), 10);

	//TCHAR lpThousandSep[] = TEXT(",");
	//NUMBERFMTW fmt{};
	//fmt.NumDigits = 0;
	//fmt.LeadingZero = 1;
	//fmt.Grouping = 3;
	//fmt.lpThousandSep = lpThousandSep;
	//fmt.NegativeOrder = 0;

	//GetNumberFormatW(
	//    LOCALE_USER_DEFAULT,
	//    0,
	//    rawBuf,
	//    &fmt,
	//    byteBuf,
	//    _countof(byteBuf)
	//);

	//ss << rawBuf << L" byte)";
	//return ss.str();
}

std::wstring FormatFileSizeStringKB(const LARGE_INTEGER& li) {
	if (li.QuadPart >= 0) {
		size_t v = static_cast<size_t>(li.QuadPart);
		if (v <= static_cast<size_t>(std::numeric_limits<std::size_t>::max())) {
			return FormatFileSizeStringKB(static_cast<size_t>(v));
		}
	}
	return std::wstring{};
}

HRESULT CopyToClipboard(HGLOBAL hGlobal)
{
	if (!OpenClipboard(NULL))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	EmptyClipboard();
	if (hGlobal)
	{
#ifdef UNICODE
		SetClipboardData(CF_UNICODETEXT, hGlobal);
#else
		SetClipboardData(CF_TEXT, hGlobal);
#endif
	}
	CloseClipboard();
	return S_OK;
}

void CopyStrToClipboard(const TCHAR* str)
{
	if (!str) return;

	const size_t charCount = _tcslen(str) + 1;
	const SIZE_T bytes = charCount * sizeof(TCHAR);

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, bytes);
	if (!hGlobal) return;

	void* pData = GlobalLock(hGlobal);
	if (pData)
	{
		memcpy(pData, str, bytes);
		GlobalUnlock(hGlobal);
		CopyToClipboard(hGlobal);
	}
	GlobalFree(hGlobal);
}