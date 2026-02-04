#pragma once

#include <vector>
#include <string>
#include <shlwapi.h>
#include <sstream>
#include "altstrm_common.h"

bool hasAlternateStreams(LPCWSTR pwszPath);
std::vector<FileStreamData> listAlternateStreams(LPCWSTR pwszPath);
std::wstring FormatFileSizeStringKB(size_t bytes);
std::wstring FormatFileSizeStringKB(const LARGE_INTEGER& li);
void CopyStrToClipboard(const TCHAR* str);