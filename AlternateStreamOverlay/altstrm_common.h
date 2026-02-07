#pragma once

#include <shlobj.h> 
#include <comdef.h> 
#include <vector> 
#include <string> 
#include <windows.h> 
#include <memory> 

struct FileStreamData 
{
	std::wstring streamName;
	size_t streamSize;
	size_t streamAllocationSize;
};

enum FILE_INFORMATION_CLASS
{
	FileStreamInformation = 22,
};

bool IsOnNTFS(LPCWSTR path);
bool HasAlternateStreams(LPCWSTR fname);
std::vector<FileStreamData> ListAlternateStreams(LPCWSTR fname);


