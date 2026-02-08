#include "stdafx.h"
#include "AlternateStreamContext.h"
#include <atlconv.h>
#include <sstream>
#include <string>
#include <format>
#include <ios>
#include <commctrl.h>
#include "Common.h"


#define START_OF_HEX_TEXT 10
#define START_OF_ASCII_TEXT 36

#define WM_PAGE_RELOAD   (WM_APP + 1)
#define WM_PROGRESS_DONE (WM_APP + 2)

#define TIMER_PROGRESS_SHOW    1
#define TIMER_PROGRESS_UPDATE  2

#define JOB_OPEN_FILE     1
#define JOB_CREATE_STREAM 2

#ifdef DEBUG
#define log(msg) OutputDebugStringA(msg);
#else
#define log(msg)
#endif

struct FileCopyJob
{
	DWORD id;
	HWND hWnd;
	HWND hDlg;
	std::wstring from;
	std::wstring to;

	size_t TotalFileSize;
	size_t TotalBytesTransferred;

	BOOL finish;
	BOOL aborted;
	BOOL failed;
	DWORD error;
};

int CALLBACK ListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lUserData);
INT_PTR CALLBACK AddStreamDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RenameDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void StartFileCopyJob(HINSTANCE hInst, FileCopyJob* job);

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

BOOL CreateStreamIfUserWantsTo(HWND hDlg, LPCWSTR filename)
{
	HANDLE handle = ::CreateFile(
		filename,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(handle);
		int ret = MessageBox(
			hDlg,
			TEXT("An alternate data stream with the same name already exists.\nOverwrite the existing stream?"),
			TEXT("Overwrite Stream"),
			MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2
		);
		if (ret != IDOK)
		{
			return FALSE;
		}
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
	if (!IsOnNTFS(szFile))
	{
		return E_INVALIDARG;
	}

	m_path = szFile;
	m_isDirectory = PathIsDirectory(szFile);
	read_streams();

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

void CAlternateStreamContext::read_streams()
{
	m_streams = ListAlternateStreams(m_path.c_str());
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
		MAKEINTRESOURCE(IDD_DIALOG_ADD_STREAM),
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


static DWORD CALLBACK JobProgressRoutine(
	LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred,
	LARGE_INTEGER StreamSize,
	LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber,
	DWORD dwCallbackReason,
	HANDLE hSourceFile,
	HANDLE hDestinationFile,
	LPVOID lpData)
{
	if (dwCallbackReason == CALLBACK_CHUNK_FINISHED)
	{
		log("progess update");
		FileCopyJob* job = (FileCopyJob*)lpData;
		job->TotalFileSize = TotalFileSize.QuadPart;
		job->TotalBytesTransferred = TotalBytesTransferred.QuadPart;
	}

	return PROGRESS_CONTINUE;
}

static DWORD WINAPI FileCopyJob_WorkerThread(LPVOID param)
{
	FileCopyJob* job = (FileCopyJob*)param;

	//job->TotalFileSize = 10000;
	//job->TotalBytesTransferred = 0;
	//for (size_t i = 0; i < job->TotalFileSize; i++)
	//{
	//	job->TotalBytesTransferred++;
	//	Sleep(50);
	//	OutputDebugStringA("progess");
	//	if (job->aborted)
	//	{
	//		break;
	//	}
	//}

	BOOL ok = CopyFileEx(
		job->from.c_str(),
		job->to.c_str(),
		JobProgressRoutine,
		job,
		&job->aborted,
		0
	);
	if (!ok)
	{
		job->failed = TRUE;
		job->error = GetLastError();
	}
	log("progess done");
	job->finish = TRUE;
	PostMessage(job->hDlg, WM_PROGRESS_DONE, NULL, (LPARAM)job);
	return 0;
}

void StartFileCopyJob(HINSTANCE hInst, FileCopyJob* job)
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
		FileCopyJob_WorkerThread,
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

void Command_RenameStream(HWND hWnd)
{
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_DIALOG_RENAME),
		hWnd,
		RenameDlgProc,
		(LPARAM)get_ctx(hWnd)
	);
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
		std::wstring filePath = ctx->m_path + fs.streamName;

		HANDLE handle = CreateFile(
			filePath.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
		if (handle == INVALID_HANDLE_VALUE)
		{
			PopupLastError(hWnd);
			return;
		}
		CloseHandle(handle);

		TCHAR tempDir[MAX_PATH]{};
		TCHAR tempFile[MAX_PATH]{};
		GetTempPath(MAX_PATH, tempDir);
		GetTempFileName(tempDir, L"ads", 0, tempFile);

		HINSTANCE hInst = ctx->GetResourceInstance();
		FileCopyJob* job = new FileCopyJob();
		job->id = JOB_OPEN_FILE;
		job->from = filePath;
		job->to = tempFile;
		job->hWnd = hWnd;

		StartFileCopyJob(hInst, job);
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
		MAKEINTRESOURCE(IDI_IMAGE1),
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
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

		TCHAR szBuffer[MAX_PATH]{};
		HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);

		LoadString(hInst, IDS_STR_OK, szBuffer, std::size(szBuffer));
		SetWindowText(GetDlgItem(hDlg, IDOK), szBuffer);

		LoadString(hInst, IDS_STR_CANCEL, szBuffer, std::size(szBuffer));
		SetWindowText(GetDlgItem(hDlg, IDCANCEL), szBuffer);
		return TRUE;
	}
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
		log("WM_PROGRESS_DONE");
		//EnableWindow(hDlg, TRUE);
		FileCopyJob* job = (FileCopyJob*)lParam;
		if (!(job->failed || job->aborted))
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

			std::wstringstream ss;
			ss << ctx->get_path() << ':' << streamName;
			std::wstring hostFile = ss.str();

			if (CreateStreamIfUserWantsTo(hDlg, hostFile.c_str()))
			{
				//EnableWindow(hDlg, FALSE);
				FileCopyJob* job = new FileCopyJob();
				job->id = JOB_CREATE_STREAM;
				job->from = filePath;
				job->to = hostFile;
				job->hWnd = hDlg;
				StartFileCopyJob(ctx->GetResourceInstance(), job);
			}
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

INT_PTR RenameDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

		TCHAR szBuffer[MAX_PATH]{};
		HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);

		LoadString(hInst, IDS_STR_RENAME, szBuffer, std::size(szBuffer));
		SetWindowText(hDlg, szBuffer);

		LoadString(hInst, IDS_STR_NEWNAME, szBuffer, std::size(szBuffer));
		SetWindowText(GetDlgItem(hDlg, IDC_LAB_NEWNAME), szBuffer);

		LoadString(hInst, IDS_STR_OK, szBuffer, std::size(szBuffer));
		SetWindowText(GetDlgItem(hDlg, IDOK), szBuffer);

		LoadString(hInst, IDS_STR_CANCEL, szBuffer, std::size(szBuffer));
		SetWindowText(GetDlgItem(hDlg, IDCANCEL), szBuffer);
		return TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			TCHAR newName[MAX_PATH]{};
			GetDlgItemText(hDlg, IDC_EDIT_NEWNAME_TEX, newName, std::size(newName));
			if (!newName[0] || PathCleanupSpec(NULL, newName))
			{
				HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_NEWNAME_TEX);
				EDITBALLOONTIP tip{};
				tip.cbStruct = sizeof(tip);
				tip.pszText = TEXT("Bad name");
				tip.ttiIcon = TTI_ERROR;
				Edit_ShowBalloonTip(hEdit, &tip);
				return TRUE;
			}
			EndDialog(hDlg, 0);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, 0);
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
		FileCopyJob* job = (FileCopyJob*)lParam;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		if (job->finish)
		{
			DestroyWindow(hDlg);
			break;
		}
		else 
		{
			LPCWSTR fileName = PathFindFileName(job->from.c_str());
			SetDlgItemText(hDlg, IDC_PROG_ITEM_NAME, fileName);
			SetDlgItemText(hDlg, IDC_PROG_ITEM_REMAIN, L"");
			SetDlgItemText(hDlg, IDC_PROGRESS_TEX, L"");
			SetTimer(hDlg, TIMER_PROGRESS_SHOW, 200, NULL);

			TCHAR szBuffer[MAX_PATH]{};
			HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);

			LoadString(hInst, IDS_STR_CANCEL, szBuffer, std::size(szBuffer));
			SetWindowText(GetDlgItem(hDlg, IDCANCEL), szBuffer);
		}
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
			FileCopyJob* job = (FileCopyJob*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			if (job)
			{
				int percent = 0;
				if (job->TotalFileSize > 0)
				{
					percent = (int)((job->TotalBytesTransferred * 100) / job->TotalFileSize);
					if (percent > 100) percent = 100;

					{
						std::wstring text = std::format(L"Complete {}%", percent);
						SetDlgItemText(hDlg, IDC_PROGRESS_TEX, text.c_str());
					}
					{
						size_t bytes = job->TotalFileSize - job->TotalBytesTransferred;
						std::wstring text = FormatFileSizeStringKB(bytes);
						SetDlgItemText(hDlg, IDC_PROG_ITEM_REMAIN, text.c_str());
					}
				}
				SendDlgItemMessage(hDlg, IDC_PROGRESS1, PBM_SETPOS, percent, 0);
			}
			return TRUE;
		}
		break;
	case WM_PROGRESS_DONE:
	{
		log("progess WM_PROGRESS_DONE");
		FileCopyJob* job = (FileCopyJob*)lParam;
		if (job->failed && job->error != ERROR_REQUEST_ABORTED)
		{
			PopupError(hDlg, job->error);
		}
		DestroyWindow(hDlg);
		return TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			log("progess IDCANCEL");
			FileCopyJob* job = (FileCopyJob*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			job->aborted = true;
			return TRUE;
		}
		break;
	case WM_CLOSE:
	{
		log("progess WM_CLOSE");
		FileCopyJob* job = (FileCopyJob*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		job->aborted = true;
		return TRUE;
	}
	case WM_DESTROY:
	{
		FileCopyJob* job = (FileCopyJob*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		if (job)
		{
			PostMessage(job->hWnd, WM_PROGRESS_DONE, NULL, (LPARAM)job);
			log("progess WM_DESTROY");
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
		pthis->read_streams();

		HWND hList = GetDlgItem(hWnd, IDC_LIST);
		UpdateListView(hList, pthis->m_streams);

		int iItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
		pthis->iItem = iItem;
		pthis->iSubItem = -1;
		EnableWindow(GetDlgItem(hWnd, IDC_BTN_DEL), iItem >= 0);
		return TRUE;
	}
	case WM_PROGRESS_DONE:
	{
		FileCopyJob* job = (FileCopyJob*)lParam;
		if (job->failed)
		{
			if (job->error != ERROR_REQUEST_ABORTED)
			{
				PopupError(hWnd, job->error);
			}
		}
		else if (job->id == JOB_OPEN_FILE)
		{
			ShellExecute(
				hWnd,
				TEXT("open"),
				job->to.c_str(),
				nullptr,
				nullptr,
				SW_SHOWNORMAL
			);
		}
		delete job;
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
			Command_RenameStream(hWnd);
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