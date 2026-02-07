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
#define WM_PROGRESS_DONE (WM_APP + 2)

#define TIMER_PROGRESS_SHOW    1
#define TIMER_PROGRESS_UPDATE  2

struct StreamCopyJob
{
	HWND hWnd;
	HWND hDlg;
	HANDLE from;
	HANDLE to;
	size_t total;
	size_t written;
	bool done;
	bool cancel;
	bool error;
	DWORD code;
};

int CALLBACK ListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lUserData);
INT_PTR CALLBACK AddStreamDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void Commnad_StreamCopy(HINSTANCE hInst, StreamCopyJob* job);

#define var auto

#ifdef _WIN64
typedef INT_PTR DLGRETURN;
#else
typedef BOOL DLGRETURN;
#endif

CAlternateStreamContext* get_ctx(HWND hWnd)
{
#ifdef _WIN64
	LONG_PTR userData = (LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	LONG userData = (LONG)GetWindowLong(hWnd, GWL_USERDATA);
#endif
	return (CAlternateStreamContext*)userData;
}

void PopupError(HWND hWnd, DWORD error)
{
	if (error == ERROR_SUCCESS) return;

	TCHAR msg[512]{};
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		msg,
		(DWORD)std::size(msg),
		nullptr
	);
	MessageBox(hWnd, msg, NULL, MB_ICONERROR);
}

void PopupLastError(HWND hWnd)
{
	PopupError(hWnd, GetLastError());
}

bool CopyFileToADS_Win32(HINSTANCE hInst, HWND hDlg, LPCWSTR srcFile, LPCWSTR hostFile, LPCWSTR streamName)
{
	HANDLE srcHandle(::CreateFile(
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

	HANDLE destHandle = ::CreateFile(
		full_path.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (destHandle == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		if (err == ERROR_FILE_EXISTS)
		{
			int ret = MessageBox(
				hDlg,
				TEXT("An alternate data stream with the same name already exists.\nOverwrite the existing stream?"),
				TEXT("Overwrite Stream"),
				MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2
			);
			if (ret != IDOK)
			{
				CloseHandle(srcHandle);
				return FALSE;
			}
			destHandle = ::CreateFile(
				full_path.c_str(),
				GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);
		}
	}
	if (destHandle == INVALID_HANDLE_VALUE)
	{
		CloseHandle(srcHandle);
		PopupLastError(hDlg);
		return FALSE;
	}

	EnableWindow(hDlg, FALSE);
	StreamCopyJob* job = new StreamCopyJob();
	job->from = srcHandle;
	job->to = destHandle;
	job->hWnd = hDlg;
	Commnad_StreamCopy(hInst, job);
	//Commnad_StreamCopy(job);
	//CloseHandle(srcHandle);
	//CloseHandle(destHandle);
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

	HRESULT hr = S_OK;
	UINT uNumFiles = DragQueryFile(hDrop, 0xffffffff, NULL, 0);
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

	if (FAILED(hr)) return hr;

	m_path = szFile;
	m_isDirectory = PathIsDirectory(szFile);
	m_streams = listAlternateStreams(szFile);

	return hr;
}
HINSTANCE CAlternateStreamContext::GetResourceInstance()
{
	return _AtlBaseModule.GetResourceInstance();
}

const std::vector<FileStreamData>& CAlternateStreamContext::get_streams() const
{
	return m_streams;
}

void CAlternateStreamContext::reload_streams()
{
	m_streams = listAlternateStreams(m_path.c_str());
}

LPCWSTR CAlternateStreamContext::get_path() const
{
	return m_path.c_str();
}

void Command_AddStream(HWND hWnd)
{
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCEW(IDD_DIALOG_ADD_STREAM),
		hWnd,
		AddStreamDlgProc,
		(LPARAM)get_ctx(hWnd)
	);
}

void Command_Refresh(HWND hWnd)
{
	SendMessage(hWnd, WM_PAGE_RELOAD, NULL, NULL);
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
		SendMessage(hWnd, WM_PAGE_RELOAD, NULL, NULL);
	}
}

DWORD WINAPI Thread_CopyJob(LPVOID param)
{
	StreamCopyJob* job = (StreamCopyJob*)param;
	job->total = 0;
	job->written = 0;

	LARGE_INTEGER size;
	if (GetFileSizeEx(job->from, &size))
	{
		job->total = size.QuadPart;
	}

	//for (size_t i = 0; i < job->total; i++)
	//{
	//	job->written++;
	//	Sleep(50);
	//	OutputDebugStringA("progess");
	//	if (job->cancel)
	//	{
	//		break;
	//	}
	//}
	//OutputDebugStringA("progess done");
	BYTE buffer[8 * 1024]{};
	DWORD read = 0, written = 0;
	while (ReadFile(job->from, buffer, sizeof(buffer), &read, nullptr) && read)
	{
		if (!WriteFile(job->to, buffer, read, &written, nullptr) || read != written)
		{
			job->error = true;
			job->cancel = true;
			job->code = GetLastError();
		}
		if (job->cancel)
		{
			FILE_DISPOSITION_INFO info{};
			info.DeleteFile = TRUE;
			SetFileInformationByHandle(
				job->to,
				FileDispositionInfo,
				&info,
				sizeof(info)
			);
			break;
		}
		job->written += written;
	}
	CloseHandle(job->to);
	CloseHandle(job->from);

	job->done = true;
	PostMessage(job->hDlg, WM_PROGRESS_DONE, NULL, (LPARAM)job);
	return 0;
}

void Commnad_StreamCopy(HINSTANCE hInst, StreamCopyJob* job)
{
	job->hDlg = CreateDialogParam(
		hInst,
		MAKEINTRESOURCE(IDD_DIALOG_PROGRESS),
		job->hWnd,
		ProgressDlgProc,
		(LPARAM)job
	);
	ShowWindow(job->hDlg, SW_HIDE);

	DWORD threadId;
	CreateThread(
		NULL,
		NULL,
		Thread_CopyJob,
		job,
		NULL,
		&threadId
	);
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

void Command_OpenStream(HWND hWnd)
{
	auto ctx = get_ctx(hWnd);
	HWND hList = GetDlgItem(hWnd, IDC_LIST);
	const std::vector<FileStreamData>& streams = ctx->get_streams();

	LVITEMW item{};
	item.mask = LVIF_PARAM;
	item.iItem = ctx->iItem;
	if (!ListView_GetItem(hList, &item)) return;

	if (item.lParam >= 0 && item.lParam < streams.size())
	{
		const FileStreamData& fs = streams[item.lParam];
		std::wstring filePath = ctx->get_path() + fs.streamName;

		HANDLE hSrc = CreateFile(
			filePath.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
		if (hSrc == INVALID_HANDLE_VALUE)
		{
			PopupLastError(hWnd);
			return;
		}

		TCHAR tempDir[MAX_PATH]{};
		TCHAR tempFile[MAX_PATH]{};
		GetTempPath(MAX_PATH, tempDir);
		GetTempFileName(tempDir, L"ads", 0, tempFile);

		HANDLE hDst = CreateFile(
			tempFile,
			GENERIC_WRITE,
			0,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_TEMPORARY,
			nullptr
		);
		if (hSrc == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hSrc);
			PopupLastError(hWnd);
			return;
		}

		BYTE buffer[8 * 1024]{};
		DWORD read = 0, written = 0;
		while (ReadFile(hSrc, buffer, sizeof(buffer), &read, nullptr) && read)
		{
			if (!WriteFile(hDst, buffer, read, &written, nullptr) || read != written)
			{
				CloseHandle(hSrc);
				CloseHandle(hDst);
				PopupLastError(hWnd);
				return;
			}
		}

		CloseHandle(hSrc);
		CloseHandle(hDst);

		ShellExecute(
			hWnd,
			TEXT("open"),
			tempFile,
			nullptr,
			nullptr,
			SW_SHOWNORMAL
		);
	}
}

BOOL OnInitDialog(HWND hWnd, LPARAM lParam)
{
	PROPSHEETPAGE* ppsp = (PROPSHEETPAGE*)lParam;
	CAlternateStreamContext* pthis = reinterpret_cast<CAlternateStreamContext*>(ppsp->lParam);

	const std::vector<FileStreamData>& streams = pthis->get_streams();
	HWND hList = GetDlgItem(hWnd, IDC_LIST);

#ifdef _WIN64
	//LONG_PTR ex = GetWindowLongPtr(hList, GWL_EXSTYLE);
	//SetWindowLongPtr(hList, GWL_EXSTYLE, ex | WS_EX_ACCEPTFILES);
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pthis);
#else
	LONG ex = GetWindowLong(hList, GWL_EXSTYLE);
	SetWindowLong(hList, GWL_EXSTYLE, ex | WS_EX_ACCEPTFILES);
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)pthis);
#endif
	//DragAcceptFiles(hList, TRUE);
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
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		return TRUE;
	case WM_NOTIFY:
	{
		LPNMHDR hdr = (LPNMHDR)lParam;
		if (hdr->idFrom == IDC_EDIT_NAME)
		{

		}
		break;
	}
	case WM_PROGRESS_DONE:
	{
		EnableWindow(hDlg, TRUE);
		OutputDebugStringA("WM_PROGRESS_DONE");
		StreamCopyJob* job = (StreamCopyJob*)lParam;
		if (!(job->error || job->cancel))
		{
			EndDialog(hDlg, 0);
		}
		delete job;
		return TRUE;
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
			HINSTANCE hInst = ctx->GetResourceInstance();
			CopyFileToADS_Win32(hInst, hDlg, filePath, ctx->get_path(), streamName);
			//EndDialog(hDlg, IDOK);
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

INT_PTR ProgressDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		StreamCopyJob* job = (StreamCopyJob*)lParam;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		if (job->done)
		{
			DestroyWindow(hDlg);
			break;
		}
		SetTimer(hDlg, TIMER_PROGRESS_SHOW, 200, NULL);
		return TRUE;
	}
	case WM_TIMER:
		if (wParam == TIMER_PROGRESS_SHOW)
		{
			ShowWindow(hDlg, SW_SHOW);
			KillTimer(hDlg, TIMER_PROGRESS_SHOW);
			SetTimer(hDlg, TIMER_PROGRESS_UPDATE, 33, NULL);
			return TRUE;
		}
		else if (wParam == TIMER_PROGRESS_UPDATE)
		{
			OutputDebugStringA("progess update");
			StreamCopyJob* job = (StreamCopyJob*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			if (job)
			{
				int percent = 0;
				if (job->total > 0)
				{
					percent = (int)((job->written * 100) / job->total);
				}
				SendDlgItemMessage(hDlg, IDC_PROGRESS1, PBM_SETPOS, percent, 0);
			}
			return TRUE;
		}
		break;
	case WM_PROGRESS_DONE:
	{
		OutputDebugStringA("progess WM_PROGRESS_DONE");
		StreamCopyJob* job = (StreamCopyJob*)lParam;
		if (job->error)
		{
			PopupError(hDlg, job->code);
		}
		DestroyWindow(hDlg);
		return TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			OutputDebugStringA("progess IDCANCEL");
			StreamCopyJob* job = (StreamCopyJob*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			job->cancel = true;
			return TRUE;
		}
		break;
	case WM_CLOSE:
	{
		OutputDebugStringA("progess WM_CLOSE");
		StreamCopyJob* job = (StreamCopyJob*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		job->cancel = true;
		return TRUE;
	}
	case WM_DESTROY:
	{
		StreamCopyJob* job = (StreamCopyJob*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		if (job)
		{
			PostMessage(job->hWnd, WM_PROGRESS_DONE, NULL, (LPARAM)job);
			OutputDebugStringA("progess WM_DESTROY");
		}
		KillTimer(hDlg, TIMER_PROGRESS_SHOW);
		KillTimer(hDlg, TIMER_PROGRESS_UPDATE);
		break;
	}
	}

	return FALSE;
}

DLGRETURN CALLBACK PropPageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		INITCOMMONCONTROLSEX icc = { sizeof(icc),ICC_LISTVIEW_CLASSES };
		InitCommonControlsEx(&icc);

		return OnInitDialog(hWnd, lParam);
	}
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
		return TRUE;
	}
	case WM_DROPFILES:
	{
		HDROP hDrop = (HDROP)wParam;
		UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
		for (UINT i = 0; i < count; i++)
		{

		}
		DragFinish(hDrop);
		//MessageBoxW(hWnd, L"DROP OK", L"DEBUG", MB_OK);
		return TRUE;
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
			else if (hdr->code == LVN_KEYDOWN)
			{
				NMLVKEYDOWN* kd = (NMLVKEYDOWN*)lParam;
				if (kd->wVKey == VK_DELETE)
				{
					HWND hList = hdr->hwndFrom;
					CAlternateStreamContext* pthis = get_ctx(hWnd);
					int iItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
					pthis->iItem = iItem;
					Command_DeleteStream(hWnd);
				}
				else if (kd->wVKey == VK_F5)
				{
					Command_Refresh(hWnd);
				}
				else if (kd->wVKey == VK_F2)
				{

				}
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
					HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
					HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
					HMENU hPopup = GetSubMenu(hMenu, 0);

					pthis->iItem = hit.iItem;
					pthis->iSubItem = hit.iSubItem;
					TrackPopupMenu(hPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
				}
				else
				{
					HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
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
			Command_AddStream(hWnd);
			Command_Refresh(hWnd);
			return TRUE;
		case ID_ITEMMENU_OPEN:
			Command_OpenStream(hWnd);
			return TRUE;
		case ID_ITEMMENU_RENAME:
			return TRUE;
		case ID_ITEMMENU_DELETE:
			Command_DeleteStream(hWnd);
			return TRUE;
		case IDC_BTN_DEL:
			if (wmEvent == BN_CLICKED)
			{
				Command_DeleteStream(hWnd);
			}
			return TRUE;
		case ID_BLANKMENU_REFRESH:
			Command_Refresh(hWnd);
			return TRUE;
		case ID_ITEMMENU_COPY_VALUE:
			auto ctx = get_ctx(hWnd);
			HWND hList = GetDlgItem(hWnd, IDC_LIST);

			TCHAR szBuffer[MAX_PATH]{};
			ListView_GetItemText(hList, ctx->iItem, ctx->iSubItem, szBuffer, std::size(szBuffer));
			CopyStrToClipboard(szBuffer);
			return TRUE;
		}
		break;
	}
	}
	return FALSE;
}


STDMETHODIMP CAlternateStreamContext::AddPages(LPFNSVADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
	PROPSHEETPAGE psp;
	HPROPSHEETPAGE hPage;
	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_USETITLE | PSP_USECALLBACK;
	psp.hInstance = GetResourceInstance();
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