#include "stdafx.h"
#include "AlternateStreamContext.h"
#include <atlconv.h>
#include <sstream>
#include <string>
#include <ios>
#include <commctrl.h>
#include "Common.h"


#define START_OF_HEX_TEXT 10
#define START_OF_ASCII_TEXT 36
int CALLBACK ListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lUserData);

STDMETHODIMP CAlternateStreamContext::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hkeyProgId)
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
void hexinto(char* tgt, unsigned char val)
{
	sprintf(tgt, "%2.2X", val);
	tgt[2] = ' ';
}

void hexinto(char* tgt, unsigned int val)
{
	sprintf(tgt, "%8.8X", val);
	tgt[8] = ' ';
}
#pragma warning(pop)

void setuphexline(char* tgt, const int len, char*& hexo, char*& txto)
{
	memset(tgt, ' ', len);
	hexo = &tgt[START_OF_HEX_TEXT];
	txto = &tgt[START_OF_ASCII_TEXT];
}

void DumpHex(BYTE* buf, DWORD size, std::stringstream& tgt)
{
	const int LINESIZE = 46;
	char tmp[LINESIZE + 4];
	tmp[LINESIZE] = ' ';
	tmp[LINESIZE + 1] = '\r';
	tmp[LINESIZE + 2] = '\n';
	tmp[LINESIZE + 3] = 0;
	char* lineo = &tmp[0];
	char* hexo, * charo;
	setuphexline(lineo, LINESIZE, hexo, charo);
	size_t cnt = 0;
	for (unsigned int x = 0; x < size; ++x, ++buf)
	{
		if (cnt == 0)
		{
			hexinto(lineo, x);
		}
		hexinto(hexo, (unsigned char)(*buf));
		*charo = asciificate(*buf);
		cnt++;
		hexo += 3;
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

void SelectText(CAlternateStreamContext* pthis, size_t idx, bool asHex, HWND textBox)
{
	auto streams = pthis->get_streams();
	const std::wstring& fname = pthis->get_path();

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
	buf[(64 * 1024) - 1] = 0;

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
	INITCOMMONCONTROLSEX icc = { sizeof(icc),ICC_LISTVIEW_CLASSES };
	InitCommonControlsEx(&icc);

	PROPSHEETPAGE* ppsp = (PROPSHEETPAGE*)lParam;
	CAlternateStreamContext* pthis = reinterpret_cast<CAlternateStreamContext*>(ppsp->lParam);
#ifdef _WIN64
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)pthis);
#else
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)pthis);
#endif

	const std::vector<FileStreamData>& streams = pthis->get_streams();
	{
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

		HICON hIcon = (HICON)LoadImageW(
			ppsp->hInstance,
			MAKEINTRESOURCEW(IDI_IMAGE1),
			IMAGE_ICON,
			64, 64,
			LR_DEFAULTCOLOR
		);
		SendDlgItemMessageW(
			hWnd,
			IDC_IMAGE,
			STM_SETIMAGE,
			IMAGE_ICON,
			(LPARAM)hIcon
		);

		TCHAR szBuffer[MAX_PATH]{};
		LoadString(ppsp->hInstance, IDS_STR_DESC, szBuffer, std::size(szBuffer));
		SetWindowText(GetDlgItem(hWnd, IDC_DESC), szBuffer);

		LoadString(ppsp->hInstance, IDS_STR_STREAM_NAME, szBuffer, std::size(szBuffer));
		col.cx = 180;
		col.pszText = szBuffer;
		ListView_InsertColumn(hList, 0, &col);

		LoadString(ppsp->hInstance, IDS_STR_STREAM_SIZE, szBuffer, std::size(szBuffer));
		col.cx = 120;
		col.pszText = szBuffer;
		ListView_InsertColumn(hList, 1, &col);

		LoadString(ppsp->hInstance, IDS_STR_STREAM_ALLOCATIONSIZE, szBuffer, std::size(szBuffer));
		col.cx = 120;
		col.pszText = szBuffer;
		ListView_InsertColumn(hList, 2, &col);

		const FileStreamData* pStreams = streams.data();
		for (int i = 0; i < (int)streams.size(); ++i)
		{
			const FileStreamData& fs = streams[i];

			LVITEMW item{};
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = i;
			item.lParam = (LPARAM)i;
			item.pszText = (LPWSTR)fs.streamName.c_str();
			ListView_InsertItem(hList, &item);

			std::wstring text1 = FormatFileSizeStringKB(fs.streamSize);
			ListView_SetItemText(hList, i, 1, (LPWSTR)text1.c_str());

			std::wstring text2 = FormatFileSizeStringKB(fs.streamAllocationSize);
			ListView_SetItemText(hList, i, 2, (LPWSTR)text2.c_str());
		}

		EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), FALSE);
	}

	return FALSE;
}

static UINT CALLBACK PropPageCallbackProc(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
	CAlternateStreamContext* pthis = reinterpret_cast<CAlternateStreamContext*>(ppsp->lParam);

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

CAlternateStreamContext* get_ctx(HWND hWnd)
{
#ifdef _WIN64
	long lUserData = (long)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	long lUserData = (long)GetWindowLong(hWnd, GWL_USERDATA);
#endif
	return reinterpret_cast<CAlternateStreamContext*>(lUserData);
}

INT_PTR CALLBACK AddStreamDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtrW(hDlg, GWLP_USERDATA, lParam);
		return TRUE;
	case WM_NOTIFY:
	{
		LPNMHDR hdr = (LPNMHDR)lParam;
		if (hdr->idFrom == IDC_EDIT_NAME)
		{
			if (hdr->code == TTN_GETDISPINFOW)
			{
				NMTTDISPINFOW* info = (NMTTDISPINFOW*)lParam;
				info->lpszText = (LPWSTR)L"Invalid stream name";
				return TRUE;
			}
		}
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_BROWSER:
		{
			OPENFILENAMEW ofn = {};
			TCHAR filePath[MAX_PATH]{};

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = filePath;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_EXPLORER |
				OFN_PATHMUSTEXIST |
				OFN_FILEMUSTEXIST |
				OFN_NOCHANGEDIR;
			if (GetOpenFileName(&ofn))
			{
				SetDlgItemText(hDlg, IDC_FILE_BROWSER_PATH, filePath);
				LPCWSTR fileName = PathFindFileName(filePath);
				SetDlgItemText(hDlg, IDC_EDIT_NAME, fileName);
			}
			break;
		}
		case IDOK:
		{
			TCHAR filePath[MAX_PATH]{};
			TCHAR fileName[MAX_PATH]{};
			GetDlgItemText(hDlg, IDC_EDIT_NAME, fileName, std::size(fileName));
			GetDlgItemText(hDlg, IDC_FILE_BROWSER_PATH, filePath, std::size(filePath));
			if (!filePath[0])
			{
				MessageBox(hDlg, TEXT("File path is empty."), NULL, MB_ICONERROR);
				return TRUE;
			}

			if (!fileName[0] || PathCleanupSpec(NULL, fileName))
			{
				HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_NAME);
				EDITBALLOONTIP x{};
				x.cbStruct = sizeof(x);
				x.pszText = L"Bad stream name!";
				x.ttiIcon = TTI_ERROR;
				Edit_ShowBalloonTip(hEdit, &x);
				return TRUE;
			}

			DWORD attr = GetFileAttributes(filePath);
			if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
			{
				MessageBox(hDlg, TEXT("Target file does not exist."), NULL, MB_ICONERROR);
				return TRUE;
			}

			auto ctx = get_ctx(hDlg);
			std::wstringstream ss;
			ss << ctx->get_path() << ':' << fileName;
			std::wstring full_path = ss.str();

			HandleW hFile(::CreateFile(
				full_path.c_str(),
				GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				CREATE_NEW,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			));
			if (hFile == INVALID_HANDLE_VALUE)
			{
				DWORD err = GetLastError();
				if (err == ERROR_FILE_EXISTS) 
				{
					int ret = MessageBoxW(
						hDlg,
						L"An alternate data stream with the same name already exists.\nOverwrite the existing stream?",
						L"Overwrite Stream",
						MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2
					);
					if (ret == IDOK)
					{
						hFile = HandleW(::CreateFile(
							full_path.c_str(),
							GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL,
							CREATE_NEW,
							FILE_ATTRIBUTE_NORMAL,
							NULL
						));
					}
					else
					{
						return TRUE;
					}
				}
			}
			if (hFile == INVALID_HANDLE_VALUE) 
			{
				//wchar_t msg[512]{};
				//FormatMessageW(
				//	FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				//	nullptr,
				//	GetLastError(),
				//	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				//	msg,
				//	(DWORD)std::size(msg),
				//	nullptr
				//);
				MessageBox(hDlg, TEXT("Failed to create ADS stream."), NULL, MB_ICONERROR);
				return TRUE;
			}

			EndDialog(hDlg, IDOK);
			return TRUE;
		}
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

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
		NMHDR* hdr = (NMHDR*)lParam;
		if (hdr->idFrom == IDC_LIST)
		{
			HWND hList = hdr->hwndFrom;
			CAlternateStreamContext* pthis = get_ctx(hWnd);
			if (hdr->code == LVN_COLUMNCLICK)
			{
				NMLISTVIEW* listView = (NMLISTVIEW*)lParam;
				//if (pthis->sortColumn == listView->iSubItem)
				//{

				//}
				//ListView_SortItemsEx(
				//	hdr->hwndFrom,
				//	ListCompareProc,
				//	(LPARAM)pthis
				//);
			}
			else if (hdr->code == LVN_ITEMCHANGED)
			{
				int iItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
				pthis->iItem = iItem;
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), iItem != -1);
			}
			else if (hdr->code == NM_RCLICK)
			{
				DWORD pos = GetMessagePos();
				POINT pt = { GET_X_LPARAM(pos), GET_Y_LPARAM(pos) };

				LVHITTESTINFO hit{};
				hit.pt = pt;
				ScreenToClient(hList, &hit.pt);
				ListView_SubItemHitTest(hList, &hit);

				if (hit.flags & LVHT_ONITEM)
				{
					HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);
					HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
					HMENU hPopup = GetSubMenu(hMenu, 0);

					pthis->iItem = hit.iItem;
					pthis->iGroup = hit.iSubItem;
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
		case IDC_BTN_ADD:
		{
			HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);
			DialogBoxParamW(
				hInst,
				MAKEINTRESOURCEW(IDD_DIALOG_ADD_STREAM),
				hWnd,
				AddStreamDlgProc,
				(LPARAM)get_ctx(hWnd)
			);
			break;
		}
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
					auto ctx = get_ctx(hWnd);
					HWND hList = GetDlgItem(hWnd, IDC_LIST);
					const std::vector<FileStreamData>& streams = ctx->get_streams();

					LVITEMW item{};
					item.mask = LVIF_PARAM;
					item.iItem = ctx->iItem;
					if (ListView_GetItem(hList, &item))
					{
						if (item.lParam >= 0 && item.lParam < streams.size())
						{
							const FileStreamData* fs = streams.data() + item.lParam;
							OutputDebugStringW(fs->streamName.c_str());
						}
					}
				}
			}
			break;
		case ID_ITEMMENU_COPY_VALUE:
			auto ctx = get_ctx(hWnd);
			HWND hList = GetDlgItem(hWnd, IDC_LIST);

			TCHAR szBuffer[MAX_PATH]{};
			ListView_GetItemText(hList, ctx->iItem, ctx->iGroup, szBuffer, std::size(szBuffer));
			CopyStrToClipboard(szBuffer);
			break;
		}
		break;
	}
	}
	return bRet;
}


STDMETHODIMP CAlternateStreamContext::AddPages(LPFNSVADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
	PROPSHEETPAGE psp;
	HPROPSHEETPAGE hPage;
	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_USETITLE | PSP_USECALLBACK;
	psp.hInstance = _AtlBaseModule.GetResourceInstance();
	psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MEDIUM);
	psp.pfnCallback = PropPageCallbackProc;
	psp.pfnDlgProc = PropPageProc;
	psp.lParam = (LPARAM)this;

	TCHAR pszTitle[MAX_PATH]{};
	LoadString(psp.hInstance, IDS_STR_STREAM, pszTitle, std::size(pszTitle));
	psp.pszTitle = pszTitle;

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

STDMETHODIMP CAlternateStreamContext::ReplacePage(EXPPS uPageID, LPFNSVADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam)
{
	return E_NOTIMPL;
}

int CALLBACK ListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lUserData)
{
	FileStreamData* a = (FileStreamData*)lParam1;
	FileStreamData* b = (FileStreamData*)lParam2;
	CAlternateStreamContext* pthis = (CAlternateStreamContext*)lUserData;
	if (pthis->iGroup == 1)
	{

	}
	int x = a->streamName.compare(b->streamName);
	return pthis->sortAsc ? x : -x;
}