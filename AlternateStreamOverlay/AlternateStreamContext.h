#pragma once

#include <shlobj.h>
#include <comdef.h>
#include <vector>
#include <string>
#include <windows.h>

#include "resource.h"       // main symbols
#include "AlternateStreamOverlay_i.h"
#include "Common.h"

/* Responsible for context menu item, and property tab */

class ATL_NO_VTABLE CAlternateStreamContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAlternateStreamContext, &CLSID_AlternateStreamContext>,
	public IAlternateStreamContext,
	public IShellExtInit,
	public IShellPropSheetExt
{
public:
	int iSubItem;
	int iItem;
	bool sortAsc;
	bool m_isDirectory;
	std::wstring m_path;
	std::vector<FileStreamData> m_streams;
public:

DECLARE_REGISTRY_RESOURCEID(IDR_ALTERNATESTREAMCONTEXT)

DECLARE_NOT_AGGREGATABLE(CAlternateStreamContext)

BEGIN_COM_MAP(CAlternateStreamContext)
	COM_INTERFACE_ENTRY(IAlternateStreamContext)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IShellPropSheetExt)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	// Native
	LPCWSTR get_path() const;
	const std::vector<FileStreamData>& get_streams() const;
	void read_streams();

	HINSTANCE GetResourceInstance();
	// IShellExtInit
	STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgId);
	// IShellPropSheetExt
	STDMETHODIMP AddPages( 
            LPFNSVADDPROPSHEETPAGE pfnAddPage,
            LPARAM lParam);    
    STDMETHODIMP ReplacePage( 
            EXPPS uPageID,
            LPFNSVADDPROPSHEETPAGE pfnReplaceWith,
            LPARAM lParam);

};

OBJECT_ENTRY_AUTO(__uuidof(AlternateStreamContext), CAlternateStreamContext)
