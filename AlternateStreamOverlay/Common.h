#pragma once

#include <vector>
#include <string>
#include <shlwapi.h>
#include <sstream>
#include "altstrm_common.h"

bool hasAlternateStreams(LPCWSTR pwszPath);
std::vector<FileStreamData> listAlternateStreams(LPCWSTR pwszPath);
std::wstring FormatFileSizeString(size_t bytes);
std::wstring FormatFileSizeString(const LARGE_INTEGER& li);