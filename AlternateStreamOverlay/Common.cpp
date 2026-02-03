#include "stdafx.h"
#include "altstrm_common.h"


// Static.  But we're always going to have ntdll in memory, so it's
// no biggie....
NTDllFuncs g_ntDllFuncs;

bool hasAlternateStreams(LPCWSTR pwszPath)
{
	return g_ntDllFuncs.HasAlternateStreams(pwszPath);
}

std::vector<std::wstring> listAlternateStreams(LPCWSTR pwszPath)
{
	return g_ntDllFuncs.ListAlternateStreams(pwszPath);
}