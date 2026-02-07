#pragma once

#include <vector>
#include <string>
#include <shlwapi.h>
#include <sstream>
#include "altstrm_common.h"

std::wstring FormatFileSizeStringKB(size_t bytes);
void CopyStrToClipboard(const TCHAR* str);