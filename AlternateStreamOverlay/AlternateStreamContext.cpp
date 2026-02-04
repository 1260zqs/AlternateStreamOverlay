#include "stdafx.h"
#include "AlternateStreamContext.h"
#include <atlconv.h>
#include <sstream>
#include <string>
#include <ios>
#include "Common.h"


#define START_OF_HEX_TEXT 10
#define START_OF_ASCII_TEXT 36
int CALLBACK ListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lUserData);

STDMETHODIMP CAlternateStreamContext::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pDataObj, HKEY hkeyProgId)
{
	FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };
	HDROP	  hDrop;

	if (FAILED(pDataObj->GetData(&fmt, &stg)))
	{
		return E_INVALIDARG;
	}

	hDrop = (HDROP)GlobalLock(stg.hGlobal);

	if (NULL == hDrop) return E_INVALIDARG;

	UINT uNumFiles = DragQueryFile(hDrop, 0xffffffff, NULL, 0);
	HRESULT hr = S_OK;
	// We only handle single file case.
	if (1 != uNumFiles)
	{
		GlobalUnlock(stg.hGlobal);
		ReleaseStgMedium(&stg);
		return E_INVALIDARG;
	}

	TCHAR szFile[MAX_PATH];
	if (0 == DragQueryFile(hDrop, 0, szFile, MAX_PATH))
	{
		hr = E_INVALIDARG;
	}

	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);

	bool directory = PathIsDirectory(szFile) != FALSE;
	if (FAILED(hr)) return hr;

	m_path = szFile;
	m_isDirectory = directory;
	m_streams = listAlternateStreams(szFile);
	// We have the filename, do we want to present a menu item for it?
	//if (m_streams.size() <= (directory ? 0 : 1)) 
	//{
	//	hr = E_FAIL;
	//}

	return hr;
}

const std::vector<FileStreamData>& CAlternateStreamContext::get_streams() const
{
	return m_streams;
}

const std::wstring& CAlternateStreamContext::get_path() const
{
	return m_path;
}

char asciificate(BYTE b)
{
	if (b >= 32 && b <= 127) return (char)b;
	return '.';
}


#pragma warning(push)
#pragma warning(disable:4996)
// I don't really know why, but I still prefer sprintf(X) to ios::hex. ;)
void hexinto(char * tgt, unsigned char val)
{
	sprintf(tgt, "%2.2X", val);
	tgt[2] = ' ';
}

void hexinto(char * tgt, unsigned int val)
{
	sprintf(tgt, "%8.8X", val);
	tgt[8] = ' ';
}
#pragma warning(pop)

void setuphexline(char * tgt, const int len, char * & hexo, char * & txto)
{
	memset(tgt, ' ', len);
	hexo = &tgt[START_OF_HEX_TEXT];
	txto = &tgt[START_OF_ASCII_TEXT];
}

void DumpHex(BYTE * buf, DWORD size, std::stringstream & tgt)
{
	const int LINESIZE = 46; 
	char tmp[LINESIZE+4];
	tmp[LINESIZE] = ' ';
	tmp[LINESIZE+1] = '\r';
	tmp[LINESIZE+2] = '\n';
	tmp[LINESIZE+3] = 0;
	char * lineo = &tmp[0];
	char * hexo, * charo;
	setuphexline(lineo, LINESIZE, hexo, charo);
	size_t cnt = 0;
	for (unsigned int x=0;x<size;++x,++buf)
	{
		if (cnt == 0) 
		{
			hexinto(lineo, x);
		}
		hexinto(hexo, (unsigned char)(*buf));
		*charo = asciificate(*buf);
		cnt++;
		hexo+=3;
		charo++;
		if (cnt == 8)
		{
			tgt << tmp;
			setuphexline(lineo, LINESIZE, hexo, charo);
			cnt = 0;
		}
	}
	tgt << tmp;
}

void SelectText(CAlternateStreamContext * pthis, size_t idx, bool asHex, HWND textBox)
{
	auto streams = pthis->get_streams();
	const std::wstring & fname = pthis->get_path();

	if (idx < 0 || idx >= streams.size()) 
	{
		OutputDebugStringA("Invalid index");
		return;
	}

	std::wstringstream ss;
	ss << fname << streams[idx].streamName;
	std::wstring full_path = ss.str();

	HandleW hFile(::CreateFile(full_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL));

	if (hFile == INVALID_HANDLE_VALUE) 
	{
		// Shouldn't be called for this file, so dbgerror
		OutputDebugStringA("Asked to get properties for invalid path");
		OutputDebugString(full_path.c_str());
		return;
	}

	BYTE buf[64 * 1024];
	DWORD dwBytesRead;

	// Dump only the first 64k.
	::ReadFile(hFile, buf, sizeof(buf) - sizeof(BYTE), &dwBytesRead, NULL);
	// Make sure it's null terminated.
	buf[(64*1024)-1] = 0;

	if (asHex)
	{
		std::stringstream tgt;
		DumpHex(&buf[0], dwBytesRead, tgt);
		SendMessageA(textBox, WM_SETTEXT, 0, (LPARAM)tgt.str().c_str());
	}
	else
	{
		// Cheeky!
		buf[dwBytesRead] = 0;
		SendMessageA(textBox, WM_SETTEXT, dwBytesRead, (LPARAM)buf);
	}
	
}

BOOL OnInitDialog(HWND hWnd, LPARAM lParam)
{
	PROPSHEETPAGE * ppsp = (PROPSHEETPAGE*)lParam;
	CAlternateStreamContext * pthis = reinterpret_cast<CAlternateStreamContext*>(ppsp->lParam);
#ifdef _WIN64
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)pthis);
#else
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)pthis);
#endif

	const std::vector<FileStreamData>& streams = pthis->get_streams();

	{
		INITCOMMONCONTROLSEX icc = { sizeof(icc),ICC_LISTVIEW_CLASSES };
		InitCommonControlsEx(&icc);

		RECT rc;
		GetClientRect(hWnd, &rc);

		HWND hList = GetDlgItem(hWnd, IDC_LIST);
		//SendMessage(hList, LVM_ENABLEGROUPVIEW, TRUE, 0);

		ListView_SetExtendedListViewStyle(
			hList,
			LVS_EX_FULLROWSELECT |
			LVS_EX_GRIDLINES |
			LVS_EX_DOUBLEBUFFER
		);

		LVCOLUMNW col = {};
		col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

		TCHAR szBuffer[MAX_PATH << 1];
		LoadString(ppsp->hInstance, IDS_STR_STREAM, szBuffer, std::size(szBuffer));

		col.cx = 180;
		col.pszText = szBuffer;
		ListView_InsertColumn(hList, 0, &col);

		LoadString(ppsp->hInstance, IDS_STR_SIZE, szBuffer, std::size(szBuffer));
		col.cx = 240;
		col.pszText = szBuffer;
		ListView_InsertColumn(hList, 1, &col);

		const FileStreamData* pStreams = streams.data();
		for (int i = 0; i < (int)streams.size(); ++i)
		{
			const FileStreamData& fs = streams[i];

			LVITEMW item = {};
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = i;
			item.lParam = (LPARAM)i;
			item.pszText = (LPWSTR)fs.streamName.c_str();
			ListView_InsertItem(hList, &item);

			std::wstring text = FormatFileSizeString(fs.streamSize);
			ListView_SetItemText(hList, i, 1, (LPWSTR)text.c_str());
		}

		EnableWindow(GetDlgItem(hWnd, IDC_BTN_ADD), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), FALSE);
	}

	return FALSE; 
}

static UINT CALLBACK PropPageCallbackProc(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
	CAlternateStreamContext * pthis = reinterpret_cast<CAlternateStreamContext*>(ppsp->lParam);

	if (uMsg == PSPCB_RELEASE)
	{
		// Releasing the CAlternateStreamContext will delete the font resource.
		pthis->Release();
		ppsp->lParam = 0;
	}

	return 1;
}

#ifdef _WIN64
typedef INT_PTR DLGRETURN;
#else
typedef BOOL DLGRETURN;
#endif

DLGRETURN CALLBACK PropPageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLGRETURN bRet = (DLGRETURN)FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		bRet = OnInitDialog(hWnd, lParam);
		break;
	case WM_NOTIFY:
	{
#ifdef _WIN64
		long lUserData = (long)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
		long lUserData = (long)GetWindowLong(hWnd, GWL_USERDATA);
#endif

		NMHDR* hdr = (NMHDR*)lParam;
		CAlternateStreamContext* pthis = reinterpret_cast<CAlternateStreamContext*>(lUserData);
		if (hdr->idFrom == IDC_LIST)
		{
			HWND hList = hdr->hwndFrom;
			if (hdr->code == LVN_COLUMNCLICK)
			{
				NMLISTVIEW* listView = (NMLISTVIEW*)lParam;
				if (pthis->sortColumn == listView->iSubItem)
				{

				}
				ListView_SortItemsEx(
					hdr->hwndFrom,
					ListCompareProc,
					(LPARAM)pthis
				);
			}
			else if (hdr->code == LVN_ITEMCHANGED)
			{
				int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

				EnableWindow(GetDlgItem(hWnd, IDC_BTN_ADD), sel != -1);
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), sel != -1);
			}
			else if (hdr->code == NM_RCLICK)
			{
				DWORD pos = GetMessagePos();
				POINT pt = { GET_X_LPARAM(pos), GET_Y_LPARAM(pos) };

				LVHITTESTINFO hit{};
				hit.pt = pt;
				ScreenToClient(hList, &hit.pt);
				ListView_HitTest(hList, &hit);

				if (hit.iItem >= 0) {

					HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);
					HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
					HMENU hPopup = GetSubMenu(hMenu, 0);

					TrackPopupMenu(hPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
				}
				else
				{
					HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);
					HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
					HMENU hPopup = GetSubMenu(hMenu, 1);

					TrackPopupMenu(hPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
				}
			}
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		break;
	}
	case WM_COMMAND:
	{
		int wmid = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		switch (wmid)
		{
		case IDC_BTN_DEL:
			if (wmEvent == BN_CLICKED)
			{
				int ret = MessageBoxW(
					hWnd,
					L"Are you sure you want to delete this item?",
					L"Confirm Delete",
					MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2
				);
				if (ret == IDOK)
				{

				}
			}
			break;
		}
		break;
	}
	}
	return bRet;
}


STDMETHODIMP CAlternateStreamContext::AddPages( 
            LPFNSVADDPROPSHEETPAGE pfnAddPage,
            LPARAM lParam)
{
	PROPSHEETPAGE psp;
	HPROPSHEETPAGE hPage;
	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags =  PSP_USETITLE | PSP_USECALLBACK; 
	psp.hInstance = _AtlBaseModule.GetResourceInstance();
	psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MEDIUM);
	psp.pfnCallback = PropPageCallbackProc;
	psp.pfnDlgProc  = PropPageProc;
	psp.lParam = (LPARAM)this;
	psp.pszTitle = TEXT("Streams");

	hPage = CreatePropertySheetPage(&psp);

	if (NULL != hPage)
	{
		if (!pfnAddPage(hPage, lParam))
		{
			DestroyPropertySheetPage(hPage);
			return S_OK;
		}
	}
	// Addref if succeeded, released on destruction.
	this->AddRef();
	return S_OK;

}

STDMETHODIMP CAlternateStreamContext::ReplacePage( 
            EXPPS uPageID,
            LPFNSVADDPROPSHEETPAGE pfnReplaceWith,
            LPARAM lParam)
{
	return E_NOTIMPL;
}

int CALLBACK ListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lUserData)
{
	FileStreamData* a = (FileStreamData*)lParam1;
	FileStreamData* b = (FileStreamData*)lParam2;
	CAlternateStreamContext* pthis = (CAlternateStreamContext*)lUserData;
	if (pthis->sortColumn == 1)
	{

	}
	int x = a->streamName.compare(b->streamName);
	return pthis->sortAsc ? x : -x;
}