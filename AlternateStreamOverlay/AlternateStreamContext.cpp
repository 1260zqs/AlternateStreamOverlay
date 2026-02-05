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
#define WM_PAGE_RELOAD   (WM_APP + 1)

int CALLBACK ListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lUserData);
INT_PTR CALLBACK AddStreamDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

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

void PopupLastError(HWND hWnd)
{
	wchar_t msg[512]{};
	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		msg,
		(DWORD)std::size(msg),
		nullptr
	);
	MessageBox(hWnd, msg, NULL, MB_ICONERROR);
}

bool CopyFileToADS_Win32(HWND hDlg, LPCWSTR srcFile, LPCWSTR hostFile, LPCWSTR streamName)
{
	HandleW srcHandle(::CreateFile(
		srcFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	));
	if (srcHandle == INVALID_HANDLE_VALUE)
	{
		PopupLastError(hDlg);
		return FALSE;
	}

	std::wstringstream ss;
	ss << hostFile << ':' << streamName;
	std::wstring full_path = ss.str();

	HandleW destHandle(::CreateFile(
		full_path.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	));
	if (destHandle == INVALID_HANDLE_VALUE)
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
			if (ret != IDOK)
			{
				return FALSE;
			}
			destHandle = HandleW(::CreateFile(
				full_path.c_str(),
				GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			));
		}
	}
	if (destHandle == INVALID_HANDLE_VALUE)
	{
		PopupLastError(hDlg);
		return FALSE;
	}


	return TRUE;
}

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

	return hr;
}

const std::vector<FileStreamData>& CAlternateStreamContext::get_streams() const
{
	return m_streams;
}

void CAlternateStreamContext::reload_streams()
{
	m_streams = listAlternateStreams(m_path.c_str());
}

const std::wstring& CAlternateStreamContext::get_path() const
{
	return m_path;
}

void Command_AddStream(HWND hWnd)
{
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);
	DialogBoxParamW(
		hInst,
		MAKEINTRESOURCEW(IDD_DIALOG_ADD_STREAM),
		hWnd,
		AddStreamDlgProc,
		(LPARAM)get_ctx(hWnd)
	);
}

void Command_DeleteStream(HWND hWnd)
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
			const FileStreamData& fs = streams[item.lParam];
			std::wstring filePath = ctx->get_path() + fs.streamName;
			int ret = MessageBox(
				hWnd,
				L"Are you sure you want to delete this item?",
				L"Confirm Delete",
				MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2
			);
			if (ret == IDOK && !DeleteFile(filePath.c_str()))
			{
				PopupLastError(hWnd);
			}
		}
	}
	SendMessage(hWnd, WM_PAGE_RELOAD, NULL, NULL);
}

void UpdateListView(HWND hList, const std::vector<FileStreamData>& streams)
{
	ListView_DeleteAllItems(hList);
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
	HWND hList = GetDlgItem(hWnd, IDC_LIST);
	//SendMessage(hList, LVM_ENABLEGROUPVIEW, TRUE, 0);

	ListView_SetExtendedListViewStyle(
		hList,
		LVS_EX_FULLROWSELECT |
		LVS_EX_GRIDLINES |
		LVS_EX_DOUBLEBUFFER
	);

	LVCOLUMNW col{};
	col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	HICON hIcon = (HICON)LoadImage(
		ppsp->hInstance,
		MAKEINTRESOURCEW(IDI_IMAGE1),
		IMAGE_ICON,
		64, 64,
		LR_DEFAULTCOLOR
	);
	SendDlgItemMessage(
		hWnd,
		IDC_IMAGE,
		STM_SETIMAGE,
		IMAGE_ICON,
		(LPARAM)hIcon
	);

	TCHAR szBuffer[MAX_PATH]{};
	LoadString(ppsp->hInstance, IDS_STR_DESC, szBuffer, std::size(szBuffer));
	SetWindowText(GetDlgItem(hWnd, IDC_DESC), szBuffer);

	LoadString(ppsp->hInstance, IDS_STR_ADD, szBuffer, std::size(szBuffer));
	SetWindowText(GetDlgItem(hWnd, IDC_BTN_ADD), szBuffer);

	LoadString(ppsp->hInstance, IDS_STR_DEL, szBuffer, std::size(szBuffer));
	SetWindowText(GetDlgItem(hWnd, IDC_BTN_DEL), szBuffer);

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

	UpdateListView(hList, streams);

	int iItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	pthis->iItem = iItem;
	pthis->iSubItem = -1;
	EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), iItem >= 0);

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
			TCHAR streamName[MAX_PATH]{};
			GetDlgItemText(hDlg, IDC_EDIT_NAME, streamName, std::size(streamName));
			GetDlgItemText(hDlg, IDC_FILE_BROWSER_PATH, filePath, std::size(filePath));
			if (!filePath[0])
			{
				MessageBox(hDlg, TEXT("File path is empty."), NULL, MB_ICONERROR);
				return TRUE;
			}

			if (!streamName[0] || PathCleanupSpec(NULL, streamName))
			{
				HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_NAME);
				EDITBALLOONTIP tip{};
				tip.cbStruct = sizeof(tip);
				tip.pszText = L"Bad stream name!";
				tip.ttiIcon = TTI_ERROR;
				Edit_ShowBalloonTip(hEdit, &tip);
				return TRUE;
			}

			auto ctx = get_ctx(hDlg);
			CopyFileToADS_Win32(hDlg, filePath, ctx->get_path().c_str(), streamName);
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
	case WM_PAGE_RELOAD:
	{
		CAlternateStreamContext* pthis = get_ctx(hWnd);
		pthis->reload_streams();

		HWND hList = GetDlgItem(hWnd, IDC_LIST);
		UpdateListView(hList, pthis->m_streams);

		int iItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
		pthis->iItem = iItem;
		pthis->iSubItem = -1;
		EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), iItem >= 0);
		break;
	}
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
					pthis->iSubItem = hit.iSubItem;
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
		case ID_BLANKMENU_ADD:
		{
			Command_AddStream(hWnd);
			SendMessage(hWnd, WM_PAGE_RELOAD, NULL, NULL);
			break;
		}
		case ID_ITEMMENU_DELETE:
			Command_DeleteStream(hWnd);
			break;
		case IDC_BTN_DEL:
			if (wmEvent == BN_CLICKED)
			{
				Command_DeleteStream(hWnd);
			}
			break;
		case ID_BLANKMENU_REFRESH:
			SendMessage(hWnd, WM_PAGE_RELOAD, NULL, NULL);
			break;
		case ID_ITEMMENU_COPY_VALUE:
			auto ctx = get_ctx(hWnd);
			HWND hList = GetDlgItem(hWnd, IDC_LIST);

			TCHAR szBuffer[MAX_PATH]{};
			ListView_GetItemText(hList, ctx->iItem, ctx->iSubItem, szBuffer, std::size(szBuffer));
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
	if (pthis->iSubItem == 1)
	{

	}
	int x = a->streamName.compare(b->streamName);
	return pthis->sortAsc ? x : -x;
}