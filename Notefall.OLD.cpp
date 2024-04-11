// Notepad.cpp : Defines the entry point for the application.
//


#include "framework.h"
#include "Notefall.h"
#include "Globals.h"
#include "SystemInfo.h"
#include "TabsManager.h"
#include "Application.h"

#include <commdlg.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Shlwapi.lib")


// Global Variables:
HWND g_hWndEdit = nullptr;
UINT g_uFindReplaceMsg;
//HWND g_hFindDlg = nullptr;
string_t g_CurrentFileName;
//WNDPROC g_OldEditBoxWndProc = nullptr;

std::map<UINT, string_t> StrMap;
//std::map<UINT, Gdiplus::Bitmap*> ImgMap;
//std::vector<TabInfo> Tabs;
//TabsManager Tabs;


class GdiPlusInit
{
public:
	GdiPlusInit()
	{
		Gdiplus::GdiplusStartupInput startupInput;
		Gdiplus::GdiplusStartup(&m_token, &startupInput, NULL);
		// NOTE: For brevity I omitted error handling, check function return value!
	}

	~GdiPlusInit()
	{
		if (m_token)
			Gdiplus::GdiplusShutdown(m_token);
	}

	// Class is non-copyable.
	GdiPlusInit(const GdiPlusInit&) = delete;
	GdiPlusInit& operator=(const GdiPlusInit&) = delete;

private:
	ULONG_PTR m_token = 0;
};


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	GdiPlusInit gdip;

	Application theApp(hInstance, lpCmdLine, nCmdShow);
	return theApp.Run();
}


// Forward declarations of functions included in this code module:
//BOOL                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProcMain(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//LRESULT CALLBACK	EditBoxWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgFind(HWND, UINT, WPARAM, LPARAM );
void DisplayError(HWND, const string_t&);
void UpdateStatusBar(HWND);


/*
#define MAX_LDSTR 200
#define LoadResString(_inst_, _id_) LoadString(_inst_, _id_, buf, MAX_LDSTR); StrMap[_id_] = buf

void LoadStrings(HINSTANCE hInst)
{
	TCHAR buf[MAX_LDSTR];

	LoadResString(hInst, IDS_APP_TITLE);
	LoadResString(hInst, IDC_NOTEFALL);
	LoadResString(hInst, IDS_COMPANY);
	LoadResString(hInst, IDS_REGKEYSOFT);
	LoadResString(hInst, IDS_REGKEYRECENT);
	LoadResString(hInst, IDS_UNNAMEDTXT);
	LoadResString(hInst, IDS_OPENFILETITLE);
	LoadResString(hInst, IDS_SAVEASTITLE);
	LoadResString(hInst, IDS_STATUSMODIFIED);
	LoadResString(hInst, IDS_STATUSNOTMOD);
	LoadResString(hInst, IDS_RECENTMENU);
	LoadResString(hInst, IDS_STBARPART2);
	LoadResString(hInst, IDS_STBARPART3);
	LoadResString(hInst, IDS_CLEARECENT);

	LoadString(hInst, IDS_OPENSAVEFILTER, buf, MAX_LDSTR); StrMap[IDS_OPENSAVEFILTER] = CharReplace(buf, _T('|'), _T('\0'));


	StrMap[IDS_FULLREGKEY] = StrMap[IDS_REGKEYSOFT] + _T("\\") + StrMap[IDS_COMPANY] + _T("\\") + StrMap[IDS_APP_TITLE];
	StrMap[IDS_FULLREGKEYRECENT] = StrMap[IDS_FULLREGKEY] + _T("\\") + StrMap[IDS_REGKEYRECENT];

}

ATOM RegisterMainWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcMain;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOTEFALL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr; // MAKEINTRESOURCE(IDC_NOTEFALL);
	wcex.lpszClassName = StrMap[IDC_NOTEFALL].c_str();
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Globals::hInst = hInstance; // Store instance handle in our global variable

	GdiPlusInit gdiplus;  // calls GdiplusStartup() and stores the returned token

	LoadStrings(hInstance);
	
	RegisterMainWindowClass(hInstance);


	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NOTEFALL));

	MSG msg;

	BOOL bRet = 0;
	while (bRet = GetMessage(&msg, nullptr, 0, 0))
	{
		if (-1 == bRet) break;

		if (!TranslateAccelerator(Globals::hMainWnd, hAccelTable, &msg))
		{
			if (g_hFindDlg == nullptr || !IsDialogMessage(g_hFindDlg, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return (int)msg.wParam;
}

const TCHAR* buildTitle(const string_t& fileName)
{
	static string_t str;
	
	str = StrMap[IDS_APP_TITLE] + _T(" - ") + fileName;
	//StringCchCopy(szTitleFull, MAX_LOADSTRING, .c_str())

	return str.c_str();
}


BOOL InitStatusBar(HWND hWnd)
{
	// Load and register IPAddress control class
	//INITCOMMONCONTROLSEX iccx;
	//iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	//iccx.dwICC = ICC_BAR_CLASSES;
	//if (!InitCommonControlsEx(&iccx))
	//	return FALSE;


	// Create the status bar control
	RECT rc = { 0, 0, 0, 0 };
	HWND hStatusbar = CreateWindowEx(0, STATUSCLASSNAME, 0, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
		rc.left, rc.top, rc.right, rc.bottom, hWnd,  (HMENU)IDC_STATUSBAR, Globals::hInst, 0);

	// Store the statusbar control handle as the user data associated with 
	// the parent window so that it can be retrieved for later use.
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)hStatusbar);

	// Establish the number of partitions or 'parts' the status bar will 
	// have, their actual dimensions will be set in the parent window's 
	// WM_SIZE handler.
	GetClientRect(hWnd, &rc);
	int nHalf = rc.right / 2;
	int nParts[4] = { nHalf, nHalf + nHalf / 3, nHalf + nHalf * 2 / 3, -1 };
	SendMessage(hStatusbar, SB_SETPARTS, 4, (LPARAM)&nParts);

	// Put some texts into each part of the status bar and setup each part
	SendMessage(hStatusbar, SB_SETTEXT, 0, (LPARAM)StrMap[IDS_STATUSNOTMOD].c_str()); //_T("Status: Not modified"));
	SendMessage(hStatusbar, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)_T("Part 2"));
	SendMessage(hStatusbar, SB_SETTEXT, 2 | SBT_POPOUT, (LPARAM)StrMap[IDS_STBARPART2].c_str());
	SendMessage(hStatusbar, SB_SETTEXT, 3 | SBT_POPOUT, (LPARAM)StrMap[IDS_STBARPART3].c_str());

	return TRUE;
}


// Creates a tab control, sized to fit the specified parent window's client area, and adds some tabs. 
// Returns the handle to the tab control. 
// hwndParent - parent window (the application's main window). 
HWND InitTabControl(HWND hwndParent)
{
	RECT rcClient;
//	INITCOMMONCONTROLSEX icex;
	HWND hwndTab;
	TCITEM tie;
	int i;
	TCHAR achTemp[256];  // Temporary buffer for strings.

	//// Initialize common controls.
	//icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	//icex.dwICC = ICC_TAB_CLASSES;
	//InitCommonControlsEx(&icex);

	// Get the dimensions of the parent window's client area, and 
	// create a tab control child window of that size. Note that Globals::hInst
	// is the global instance handle.
	GetClientRect(hwndParent, &rcClient);

	hwndTab = CreateWindow(WC_TABCONTROL, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
     		0, 0, rcClient.right, rcClient.bottom, hwndParent, NULL, Globals::hInst, NULL);
	if (hwndTab == NULL)
	{
		return NULL;
	}

	// Set the font of the tabs to a more typical system GUI font
	SendMessage(hwndTab, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), 0);

	// Add tabs for each day of the week. 
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;
	tie.pszText = achTemp;

	for (i = 0; i < 5; i++)
	{
		// Load the day string from the string resources. Note that
		// Globals::hInst is the global instance handle.
		//LoadString(Globals::hInst, IDM_RECENT1 + i, achTemp, sizeof(achTemp) / sizeof(achTemp[0]));
		string_t str = _T("tab item ") + toStringSep(i);
		tie.pszText = (TCHAR*)str.c_str();

		if (TabCtrl_InsertItem(hwndTab, i, &tie) == -1)
		{
			DestroyWindow(hwndTab);
			return NULL;
		}
	}
	return hwndTab;
}
*/

/*
//   FUNCTION: InitInstance(HINSTANCE, int)
//   PURPOSE: Saves instance handle and creates main window
//   COMMENTS:
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	Globals::hInst = hInstance; // Store instance handle in our global variable

	//WS_EX_LAYERED
	HWND hWnd = CreateWindowEx(0 , StrMap[IDC_NOTEFALL].c_str(), buildTitle(StrMap[IDS_UNNAMEDTXT]), WS_OVERLAPPEDWINDOW,
  		                 CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	
	Globals::hMainWnd = hWnd;
	if (!hWnd) return FALSE;

	//WS_EX_LAYERED |
	//HWND hWndCaption = CreateWindowEx( WS_EX_NOACTIVATE, _T(""), _T("title"), WS_CHILD | WS_BORDER,
		//0, 0, CW_USEDEFAULT, Globals::TabTotalHeight, hWnd, nullptr, hInstance, nullptr);

	const auto DWM_DISABLE = DWMSBT_NONE; // Default
	const auto MAINWINDOW = DWMSBT_MAINWINDOW; // Mica
	const auto TRANSIENTWINDOW = DWMSBT_TRANSIENTWINDOW; // Acrylic
	const auto TABBEDWINDOW = DWMSBT_TABBEDWINDOW; // Tabbed
	const auto CAPT_COLOR = DWMWA_COLOR_DEFAULT; // #D0DCEC
	const auto BORDER_COLOR = DWMWA_COLOR_DEFAULT; // 0x00ECDCD0; DWMWA_COLOR_NONE;
	
	DwmSetWindowAttribute(hWnd, DWMWA_CAPTION_COLOR, &CAPT_COLOR, sizeof(int));
	DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &BORDER_COLOR, sizeof(int));
	
	//DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &MAINWINDOW, sizeof(int)); //DWMWA_SYSTEMBACKDROP_TYPE


	WTA_OPTIONS wopt;
	wopt.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
	wopt.dwMask = WTNCA_VALIDBITS;
	//WINDOWTHEMEATTRIBUTETYPE
	SetWindowThemeAttribute(hWnd, WTA_NONCLIENT, &wopt, sizeof(wopt));


	//auto bPNG = LoadPNG(Globals::hInst, MAKEINTRESOURCE(IDB_PNG1));

	//Gdiplus::Bitmap bitmap2(Globals::hInst, MAKEINTRESOURCE(IDB_PNG1));
	//auto bitmap = new Gdiplus::Bitmap(bitmap2.GetWidth(), bitmap2.GetHeight(), PixelFormat32bppARGB);
	//auto gr = Gdiplus::Graphics::FromImage(bitmap);
	//
	//Gdiplus::Bitmap bitmap1(Globals::hInst, MAKEINTRESOURCE(IDB_BITMAP1));
	//
	//gr->DrawImage(&bitmap1, Gdiplus::Rect(0, 0, bitmap2.GetWidth(), bitmap2.GetHeight()), 0, 0, bitmap2.GetWidth(), bitmap2.GetHeight(), Gdiplus::UnitPixel);

	
//	gr->DrawImage(
//		tabCenterImage, new Rectangle(tabLeftImage.Width, 0, _tabContentWidth, tabCenterImage.Height), 0, 0, _tabContentWidth, tabCenterImage.Height,
//		GraphicsUnit.Pixel);

//	gr->DrawImage(
//		tabRightImage, new Rectangle(tabLeftImage.Width + _tabContentWidth, 0, tabRightImage.Width, tabRightImage.Height), 0, 0, tabRightImage.Width,
//		tabRightImage.Height, GraphicsUnit.Pixel);

*/
	//GdipGetImageGraphicsContext();
	/*
	const auto DWMWCP_DONOTROUND = 1;		// Rectangular
	const auto DWMWCP_ROUND = 2;			// Default
	const auto DWMWCP_ROUNDSMALL = 3;		// Semi-rounded
	DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &DWMWCP_ROUNDSMALL,	sizeof(int));


	auto caption = RGB(45, 80, 45);
	auto text = RGB(20, 180, 180);
	auto border = RGB(255, 0, 0);
	DwmSetWindowAttribute(hWnd, DWMWA_CAPTION_COLOR, &caption, sizeof(COLORREF));
	DwmSetWindowAttribute(hWnd, DWMWA_TEXT_COLOR, &text, sizeof(COLORREF));
	DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &border, sizeof(COLORREF));
	*/

	//InitStatusBar(hWnd);



	/*
	HWND hWndTab = InitTabControl(hWnd);

	RECT rc;
	GetClientRect(hWndTab, &rc);
	TabCtrl_AdjustRect(hWndTab, FALSE, &rc);

	//HWND hStatusbar = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	RECT rcSB{0};
	//GetClientRect(hStatusbar, &rcSB);
	*/
/*
	//RECT rc;
	//GetClientRect(hWnd, &rc);

	//g_hWndEdit = CreateWindowEx(ES_EX_ZOOMABLE, WC_EDIT, _T("NotefallEditBox"),
	//	WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOVSCROLL | ES_AUTOHSCROLL, // | WS_BORDER,
	//	rc.left + SystemInfo::XBorderSize, rc.top + SystemInfo::YBorderSize + SystemInfo::YCaption + 11, // 11=Padding 
	//	rc.right - SystemInfo::XBorderSize, rc.bottom - SystemInfo::YBorderSize, 
	//	hWnd, (HMENU)ID_EDITBOXWND, hInstance, nullptr);
	//SendMessage(g_hWndEdit, EM_SETMARGINS, EC_LEFTMARGIN, 10);
	//SendMessage(g_hWndEdit, EM_SETLIMITTEXT, 0x7FFFFFFE, 0); // reset default size limits
	//SendMessage(g_hWndEdit, WM_SETTEXT, 0, (LPARAM)EDITBOXTEXT);

	//g_OldEditBoxWndProc = (WNDPROC)SetWindowLongPtr(g_hWndEdit, GWLP_WNDPROC, (LONG_PTR)&EditBoxWndProc);
	//if (!g_OldEditBoxWndProc)
	//	DisplayError(g_hWndEdit, _T("SetWindowLongPtr"));


	// register message for Find dialog
	//g_uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

	
	HMENU submenuRecent = CreatePopupMenu();
	auto key = Registry::CurrentUser->Open(StrMap[IDS_FULLREGKEYRECENT]); // _T("SOFTWARE\\NotepadCo\\Notefall\\Recent"));

	key->EnumerateValues([&key, submenuRecent](const std::wstring& valueName, const uint num) -> bool
		{
			string_t val = key->GetString(valueName);
			AppendMenu(submenuRecent, MF_STRING, (UINT_PTR)(IDM_RECENT1 + num), val.c_str());

			if (num > 9) return false; // we support up to 9 recent items
			
			return true; // Return true to continue processing, false otherwise
		});

	AppendMenu(submenuRecent, MF_SEPARATOR, 0, 0);
	AppendMenu(submenuRecent, MF_STRING, (UINT_PTR)IDS_CLEARECENT, StrMap[IDS_CLEARECENT].c_str());

	HMENU mainMenu = GetMenu(hWnd);
	HMENU menuFile = GetSubMenu(mainMenu, 0);
	InsertMenu(menuFile, 1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)submenuRecent, StrMap[IDS_RECENTMENU].c_str()); //_T("Open Recent"));

	//UpdateStatusBar(hWnd);



	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

*/
/*
void OnResize(HWND hWnd, UINT, int cx, int cy)
{

	//HWND hStatusbar = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	//HDC dc = GetDC(g_hWndEdit);

	//SIZE sz2 = {};
	//string_t part2 = StrMap[IDS_STBARPART2];
	//GetTextExtentPoint32(dc, part2.c_str(), (int)part2.size(), &sz2);

	//SIZE sz3 = {};
	//string_t part3 = StrMap[IDS_STBARPART3];
	//GetTextExtentPoint32(dc, part3.c_str(), (int)part3.size(), &sz3);

	//int nHalf = (cx - sz2.cx - sz3.cx) / 2;
	//int nParts[] = { nHalf, 2 * nHalf, 2 * nHalf + sz2.cx, -1 };

	//SendMessage(hStatusbar, SB_SETPARTS, 4, (LPARAM)&nParts);

//	RECT rcSB;
//	GetClientRect(hStatusbar, &rcSB);
	LONG sbHeight = 0; // rcSB.bottom - rcSB.top;
//	MoveWindow(hStatusbar, 0, cy - sbHeight, cx, cy, true);

	MoveWindow(g_hWndEdit, 
		SystemInfo::XBorderSize, Globals::TabTotalHeight, 
		cx - 2*SystemInfo::XBorderSize, cy - 2*SystemInfo::YBorderSize - Globals::TabTotalHeight - sbHeight, true);
}
*/

void UpdateRecent(string_t fileName)
{
	auto access = Registry::DesiredAccess::Write | Registry::DesiredAccess::Read;
	auto key = Registry::CurrentUser->Open(StrMap[IDS_FULLREGKEYRECENT], access); // _T("SOFTWARE\\NotepadCo\\Notefall\\Recent"), access);

	std::vector<string_t> recents;
	recents.push_back(fileName);

	key->EnumerateValues([&key, &recents, &fileName](const std::wstring& valueName, const uint) -> bool
		{
			string_t val = key->GetString(valueName);
			if (fileName != val)
				recents.push_back(val);
			return true;
		});

	for (size_t i = 0; i < 9; i++)
	{
		if (i < recents.size())
			key->SetString(std::to_wstring(i), recents[i]);
		else
			break;
	}
}

void LoadFile(const string_t& fullFileName, const HWND hEditWnd)
{
	HANDLE hf = CreateFile(fullFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hf == INVALID_HANDLE_VALUE)
	{
		MessageBox(hEditWnd, _T("Error opening file."), _T("Error"), MB_OK);
		return;
	}

	DWORD fileSize = GetFileSize(hf, nullptr); // assuming we do not open files larger than 4G

	const DWORD MAX_BUFSIZE = 100 * 1024 * 1024; //100M

	SendMessage(hEditWnd, WM_SETTEXT, 0, 0); // clear text in Edit box?
	SendMessage(hEditWnd, WM_SETREDRAW, FALSE, 0);

	if (fileSize > MAX_BUFSIZE)
	{
		std::string ReadBuffer;
		ReadBuffer.resize(MAX_BUFSIZE + 1);

		//char* ReadBuffer = new char[MAX_BUFSIZE + 1]; // extra byte for terminating null
		DWORD ActualRead = 0;
		do
		{
			if (FALSE == ReadFile(hf, ReadBuffer.data(), MAX_BUFSIZE, &ActualRead, nullptr))
			{
				SendMessage(hEditWnd, WM_SETREDRAW, TRUE, 0);
				DisplayError(hEditWnd, _T("ReadFile"));
				//delete[] ReadBuffer;
				CloseHandle(hf);
				return;
			}

			ReadBuffer[ActualRead] = '\0';

			SendMessageA(hEditWnd, EM_REPLACESEL, FALSE, (LPARAM)ReadBuffer.data());

		} while (ActualRead == MAX_BUFSIZE);

		//delete[] ReadBuffer;
		CloseHandle(hf);
	}
	else
	{
		std::string ReadBuffer;
		ReadBuffer.resize(fileSize + 1);
		//char* ReadBuffer = new char[fileSize + 1]; // extra byte for terminating null
		DWORD ActualRead = 0;

		if (FALSE == ReadFile(hf, ReadBuffer.data(), fileSize, &ActualRead, nullptr))
		{
			SendMessage(hEditWnd, WM_SETREDRAW, TRUE, 0);
			DisplayError(hEditWnd, _T("ReadFile"));
			//delete[] ReadBuffer;
			CloseHandle(hf);
			return;
		}

		ReadBuffer[ActualRead] = '\0';

		SetWindowTextA(hEditWnd, ReadBuffer.data());

		//delete[] ReadBuffer;
		CloseHandle(hf);
	}

	UpdateRecent(fullFileName);

	SendMessage(hEditWnd, EM_SETSEL, 0, 0); // move to the begin of the text
	Edit_ScrollCaret(hEditWnd);

	SendMessage(hEditWnd, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(hEditWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	
	Edit_SetModify(hEditWnd, FALSE);
	UpdateStatusBar(GetParent(hEditWnd));
	//SetWindowText(GetParent(hEditWnd), buildTitle(PathFindFileName(fullFileName.c_str())));

	g_CurrentFileName = fullFileName;
}

void OnFileOpen(const HWND hWnd)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];    // buffer for selected file name (with dir)
	TCHAR szFileTitle[MAX_PATH];    // buffer for selectede file name without dir
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrTitle = StrMap[IDS_OPENFILETITLE].c_str(); // _T("Open file");

	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';  // Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = StrMap[IDS_OPENSAVEFILTER].c_str(); // _T("All (*.*)|*.*|Text (*.txt)|*.TXT|");
	ofn.nFilterIndex = 1;

	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = nullptr;
	ofn.lpstrDefExt = nullptr;

	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		LoadFile(ofn.lpstrFile, g_hWndEdit);
	}
}

void SaveFile(HWND hWndEdit, const string_t &fullFileName)
{
	HANDLE hf = CreateFile(fullFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hf == INVALID_HANDLE_VALUE)
	{
		DisplayError(hWndEdit, StrMap[IDS_SAVEASTITLE]); // _T("Save File As"));
		return;
	}

	DWORD len = (DWORD)SendMessageA(hWndEdit, WM_GETTEXTLENGTH, 0, 0); //get text in ASCII coding here
	std::string str;
	str.resize(len + 1);

	GetWindowTextA(hWndEdit, str.data(), (int)str.size());

	DWORD writeCnt = 0;
	if (WriteFile(hf, str.data(), len, &writeCnt, nullptr) == FALSE)
	{
		DisplayError(hWndEdit, StrMap[IDS_SAVEASTITLE]); // _T("Save File As"));
		CloseHandle(hf);
		return;
	}

	Edit_SetModify(hWndEdit, FALSE);
	CloseHandle(hf);
}

void OnFileSaveAs(const HWND hWnd)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];    // buffer for selected file name (with dir)
	TCHAR szFileTitle[MAX_PATH];    // buffer for selected file name without dir

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrTitle = StrMap[IDS_SAVEASTITLE].c_str(); // _T("Save File As");

	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';  // Set lpstrFile[0] to '\0' so that GetSaveFileName does not use the contents of szFile to initialize itself.
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = StrMap[IDS_OPENSAVEFILTER].c_str(); // _T("All (*.*)\0*.*\0Text (*.txt)\0*.txt\0"); // last \0 is added automatically by compiler
	ofn.nFilterIndex = 1;

	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = nullptr;
	ofn.lpstrDefExt = _T("txt");

	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn) == TRUE)
	{
		SaveFile(g_hWndEdit, ofn.lpstrFile);
	}
}

#define SEARCH_TEXT_LEN 100

void OnFind(const HWND hWnd, bool next)
{
	static FINDREPLACE fr = {};
	static TCHAR szFindWhat[SEARCH_TEXT_LEN] = { 0 };  // buffer receiving string

	fr.lStructSize = sizeof(fr);
	fr.hwndOwner = hWnd;
	fr.lpstrFindWhat = szFindWhat;
	fr.wFindWhatLen = SEARCH_TEXT_LEN;

	//g_hFindDlg = FindTextA(&fr);
	if (next)
	{
		PostMessage(hWnd, g_uFindReplaceMsg, 0, (LPARAM)&fr);
		return;
	}

	fr.lCustData = 0; // search from beginning
	DWORD beg, end;
	SendMessage(g_hWndEdit, EM_GETSEL, (WPARAM)&beg, (LPARAM)&end);
	HLOCAL hbuf = (HLOCAL)SendMessage(g_hWndEdit, EM_GETHANDLE, 0, 0);
	TCHAR* buf = (TCHAR*)LocalLock(hbuf);
	StringCchCopyN(szFindWhat, SEARCH_TEXT_LEN, buf + beg, end - beg);

	LocalUnlock(hbuf);
	DialogBoxParam(Globals::hInst, MAKEINTRESOURCE(IDD_DIALOGFIND), hWnd, DlgFind, (LPARAM)&fr);

	return;
}
void UpdateStatusBar(HWND hWnd)
{
	DWORD len = (DWORD)SendMessage(g_hWndEdit, WM_GETTEXTLENGTH, 0, 0);
	DWORD lines = (DWORD)SendMessage(g_hWndEdit, EM_GETLINECOUNT, 0, 0);
	BOOL modified = (BOOL)SendMessage(g_hWndEdit, EM_GETMODIFY, 0, 0);
	HWND hStatusbar = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	const DWORD BUF_SZ = 25;
	TCHAR buf[BUF_SZ];
	StringCchPrintf(buf, BUF_SZ, (_T("Size: ") + SizeToStr(len)).c_str());
	SendMessage(hStatusbar, SB_SETTEXT, 2 | SBT_POPOUT, (LPARAM)buf);
	StringCchPrintf(buf, BUF_SZ, (_T("Lines: ") + toStringSep(lines)).c_str());
	SendMessage(hStatusbar, SB_SETTEXT, 3 | SBT_POPOUT, (LPARAM)buf);
	if (modified)
		SendMessage(hStatusbar, SB_SETTEXT, 0 | SBT_POPOUT, (LPARAM)StrMap[IDS_STATUSMODIFIED].c_str()); //_T("Status: Modified"));
	else
		SendMessage(hStatusbar, SB_SETTEXT, 0 | SBT_POPOUT, (LPARAM)StrMap[IDS_STATUSNOTMOD].c_str()); // _T("Status: Not modified"));
}

template< class Tstr>
void SearchFor(HWND hWnd, FINDREPLACE* fr)
{
	DWORD len = (DWORD)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);

	Tstr str;  // this is wchar_t string if defined UNICODE
	str.resize(len + 1);

	GetWindowText(hWnd, str.data(), len);

	size_t len2 = 0;
	if (FAILED(StringCchLength(fr->lpstrFindWhat, SEARCH_TEXT_LEN, &len2))) return;

	size_t index = 0;

	index = str.find(fr->lpstrFindWhat, fr->lCustData);

	if (fr->Flags & FR_WHOLEWORD)
	{
		while (index != Tstr::npos)
		{
			size_t last = index + len2;
			if ((index == 0 || (index > 0 && isWordSeparator(str[index - 1]))) && (last >= str.size() || (last < str.size() && isWordSeparator(str[last]))))
			{
				break;
			}

			index = str.find(fr->lpstrFindWhat, index + 1);
		}
	}

	if (index == Tstr::npos)
	{
		fr->lCustData = 0;
	}
	else
	{
		fr->lCustData = (LPARAM)(index + len2);
		SendMessage(hWnd, EM_SETSEL, index, index + len2);
		Edit_ScrollCaret(hWnd);
	}
}

/*
LRESULT CALLBACK EditBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//if (msg == WM_CHAR && wParam == 6)
	//{
	//	OnFind(hwnd);
	//	return 1;
	//}
	switch (msg)
	{
	case WM_COMMAND:
	{
		int commandId = LOWORD(wParam);
		switch (commandId)
		{
		case IDM_EDUNDO:
			// Send WM_UNDO only if there is something to be undone. 
			if (SendMessage(g_hWndEdit, EM_CANUNDO, 0, 0))
				SendMessage(g_hWndEdit, WM_UNDO, 0, 0);
			else
				MessageBox(g_hWndEdit, _T("Nothing to undo."), _T("Undo notification"), MB_OK);
			break;
		case IDM_EDCUT:  PostMessage(g_hWndEdit, WM_CUT, 0, 0); break;
		case IDM_EDCOPY: PostMessage(g_hWndEdit, WM_COPY, 0, 0); break;
		case IDM_EDPASTE:PostMessage(g_hWndEdit, WM_PASTE, 0, 0); break;
		case IDM_SELECTALL: PostMessage(g_hWndEdit, EM_SETSEL, 0, (LPARAM)-1); break;

		default:
			return CallWindowProc(g_OldEditBoxWndProc, hwnd, msg, wParam, lParam);
		}

		break;
	}

	case WM_CONTEXTMENU:
	{
		HMENU menu = LoadMenu(Globals::hInst, MAKEINTRESOURCE(IDR_EDITBOXMENU));
		menu = GetSubMenu(menu, 0);
		WORD xPos = GET_X_LPARAM(lParam);
		WORD yPos = GET_Y_LPARAM(lParam);
		TrackPopupMenuEx(menu, 0, xPos, yPos, hwnd, 0);
		break;
	}

	default:
		return CallWindowProc(g_OldEditBoxWndProc, hwnd, msg, wParam, lParam);
	}

	return 1;
}
*/
/*
void DrawTitleBarBackground(HWND hWnd, Gdiplus::Graphics *graphics)
{
	RECT fillArea;
	GetClientRect(hWnd, &fillArea);
	//fillArea.left = 1;
	//fillArea.top = 0;
	//fillArea.right -= 2;
	//fillArea.bottom = SystemInfo::YCaption;
	
	// Adjust the ymargin so that the gradient stops immediately prior to the control box in the titlebar
	int rightMargin = SystemInfo::YBorderSize;
	rightMargin += SystemInfo::XCaptionButton*4;
	//rightMargin += SystemInfo::XCaptionButton;
	//rightMargin += SystemInfo::XCaptionButton;

	DWORD titleColor = SystemInfo::CaptionColor(GetActiveWindow() == hWnd);
	
	//Gdiplus::LinearGradientBrush gradient(Gdiplus::Point(24, 0), Gdiplus::Point(RECTWIDTH(fillArea) - rightMargin + 1, 0), titleColor, titleColor);
	
//#define GetAValue(rgb) (LOBYTE((rgb) >> 24))

	//Gdiplus::Color clr(GetAValue(titleColor), GetRValue(titleColor), GetGValue(titleColor), GetBValue(titleColor));
	Gdiplus::Color clr2(GetRValue(titleColor), GetGValue(titleColor), GetBValue(titleColor));
	Gdiplus::SolidBrush solidBrush(clr2);
	Gdiplus::Rect area(fillArea.left, fillArea.top, RECTW(fillArea) - rightMargin, Globals::TabTotalHeight);
	auto status = graphics->FillRectangle(&solidBrush, area);
	
	//Gdiplus::Color clr3(GetRValue(SystemInfo::WindowColor), GetGValue(SystemInfo::WindowColor), GetBValue(SystemInfo::WindowColor));
	Gdiplus::Color clr3(0, 0, 0);
	Gdiplus::SolidBrush solidBrush3(clr3);
	Gdiplus::Rect area3(fillArea.left, fillArea.top + Globals::TabTotalHeight, RECTW(fillArea), RECTH(fillArea));
	graphics->FillRectangle(&solidBrush3, area3);


	//graphics.FillRectangle(new SolidBrush(TitleBarGradientColor),
	//		new Rectangle(new Point(fillArea.Location.X + fillArea.Width - rightMargin, fillArea.Location.Y), new Size(rightMargin, fillArea.Height)));
	//	bufferedGraphics.Graphics.FillRectangle(
	//		gradient, new Rectangle(fillArea.Location, new Size(fillArea.Width - rightMargin, fillArea.Height)));
	//	bufferedGraphics.Graphics.FillRectangle(new SolidBrush(TitleBarColor), new Rectangle(fillArea.Location, new Size(24, fillArea.Height)));

	//	bufferedGraphics.Render(graphics);
}
*/

// Paint the title on the custom frame.
void PaintCustomCaption2(HWND hWnd, HDC hdc)
{
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	HTHEME hTheme = OpenThemeData(NULL, _T("CompositedWindow::Window")); 
	if (hTheme)
	{
		HDC hdcPaint = CreateCompatibleDC(hdc);
		if (hdcPaint)
		{
			int cx = RECTW(rcClient);
			int cy = RECTH(rcClient);

			// Define the BITMAPINFO structure used to draw text.
			// Note that biHeight is negative. This is done because
			// DrawThemeTextEx() needs the bitmap to be in top-to-bottom
			// order.
			BITMAPINFO dib = { 0 };
			dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			dib.bmiHeader.biWidth = cx;
			dib.bmiHeader.biHeight = -cy;
			dib.bmiHeader.biPlanes = 1;
			dib.bmiHeader.biBitCount = 32;// BIT_COUNT;
			dib.bmiHeader.biCompression = BI_RGB;
			
			HBITMAP hbm = CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, nullptr, nullptr, 0);

			if (hbm)
			{
				HBITMAP hbmOld = (HBITMAP)SelectObject(hdcPaint, hbm);

//				COLORREF color = 0x00FF0000;
				//GetThemeColor(hTheme, WP_CAPTION, CS_ACTIVE, TMT_FILLCOLOR, &color);
				//color = GetThemeSysColor(hTheme, COLOR_3DFACE); // COLOR_3DFACE COLOR_GRADIENTACTIVECAPTION COLOR_GRADIENTINACTIVECAPTION COLOR_INACTIVEBORDER COLOR_WINDOWFRAME COLOR_ACTIVECAPTION COLOR_WINDOW  COLOR_INACTIVECAPTION

				//RECT fill{ SystemInfo::XBorderSize, SystemInfo::YBorderSize + SystemInfo::YCaption,
				//		   rcClient.right - SystemInfo::XBorderSize, rcClient.bottom - SystemInfo::YBorderSize };
				//HBRUSH br = CreateSolidBrush(color);
				//FillRect(hdcPaint, &fill, br);

				//auto gr = Gdiplus::Graphics::FromHDC(hdcPaint);

				//Gdiplus::Color clr(GetRValue(color), GetGValue(color), GetBValue(color));
				//Gdiplus::SolidBrush solidBrush(clr);
				//Gdiplus::Rect area(fill.left, fill.top, RECTWIDTH(fill), RECTHEIGHT(fill));
				//auto status = gr->FillRectangle(&solidBrush, area);

				//RECT capt{ 0,0, rcClient.right, SystemInfo::YBorderSize + SystemInfo::YCaption };
			
				//DrawCaption(hWnd, hdc, &capt, DC_ACTIVE| DC_BUTTONS | DC_TEXT);
					

				//DrawTabs(hdcPaint, rcClient);

				// Select a font.
				LOGFONT lgFont;
				HFONT hFontOld = NULL;
				if (SUCCEEDED(GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
				{
					HFONT hFont = CreateFontIndirect(&lgFont);
					hFontOld = (HFONT)SelectObject(hdcPaint, hFont);
				}
			

				// Setup the theme drawing options.
				DTTOPTS DttOpts = { sizeof(DTTOPTS) };
				DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
				DttOpts.iGlowSize = 15;

				// Draw the title.
				RECT rcPaint = rcClient;
				rcPaint.top += 16;//8;
				rcPaint.right -= 125;
				rcPaint.left += 30; //8;
				rcPaint.bottom = 50;
				DrawThemeTextEx(hTheme,
					hdcPaint,
					0, 0,
					_T("szTitle text text"),
					-1,
					DT_LEFT | DT_WORD_ELLIPSIS,
					&rcPaint,
					&DttOpts);

				// Blit text to the frame.
				BitBlt(hdc, 0, 0, cx, cy, hdcPaint, 0, 0, SRCCOPY);

				SelectObject(hdcPaint, hbmOld);
				if (hFontOld)
				{
					SelectObject(hdcPaint, hFontOld);
				}
				DeleteObject(hbm);
			}
			DeleteDC(hdcPaint);
		}
		CloseThemeData(hTheme);
	}
}

//             ((fn)((hwnd), (int)(LOWORD(wParam)),(HWND)(lParam),(UINT)HIWORD(wParam)), 0L)
LRESULT HandleWMCommand(HWND hWnd, int commandID, HWND controlHandle, UINT code)
{
	//int commandId = LOWORD(wParam);
	// Parse the menu selections:
	switch (commandID)
	{
	case IDM_ABOUT:
		DialogBox(Globals::hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;
	case IDM_EXIT:
		DestroyWindow(hWnd);
		break;
	case IDM_ZOOMIN:
	{
		long long enu = -1, denu = -1;
		BOOL res = (BOOL)SendMessage(g_hWndEdit, EM_GETZOOM, (WPARAM)&enu, (LPARAM)&denu);
		//if (!res) DisplayError(hWnd, _T("EM_SETZOOM 5-1"));
		//Edit_SetZoom(g_hWndEdit, 5, 1);
		res = PostMessage(g_hWndEdit, EM_SETZOOM, 50, 100);
		//if (!res) DisplayError(hWnd, _T("EM_SETZOOM 5-1"));
		break;
	}
	case IDM_ZOOMOUT:
	{
		long long enu = -1, denu = -1;
		BOOL res = (BOOL)SendMessage(g_hWndEdit, EM_GETZOOM, (WPARAM)&enu, (LPARAM)&denu);
		res = Edit_SetZoom(g_hWndEdit, 200, 100);
		//if (!res) DisplayError(hWnd, _T("EM_SETZOOM 1-5"));
		break;
	}

	case ID_EDITBOXWND: // notification from g_hWndEditbox
		if (code == EN_UPDATE) UpdateStatusBar(hWnd); // updating status bar if text has changed
		break;

	case IDM_EDUNDO:
		// Send WM_UNDO only if there is something to be undone. 
		if (SendMessage(g_hWndEdit, EM_CANUNDO, 0, 0))
			SendMessage(g_hWndEdit, WM_UNDO, 0, 0);
		else
			MessageBox(g_hWndEdit, _T("Nothing to undo."), _T("Undo notification"), MB_OK);
		break; 
	case IDM_EDCUT:    SendMessage(g_hWndEdit, WM_CUT,    0, 0); break;
	case IDM_EDCOPY:   SendMessage(g_hWndEdit, WM_COPY,   0, 0); break;
	case IDM_EDPASTE:  SendMessage(g_hWndEdit, WM_PASTE,  0, 0); break;
	case IDM_EDDEL:    SendMessage(g_hWndEdit, WM_CLEAR,  0, 0); break;
	case IDM_SELECTALL:SendMessage(g_hWndEdit, EM_SETSEL, 0, (LPARAM)-1); break;

	case IDM_SAVEFILE:
		if (g_CurrentFileName.empty())
			OnFileSaveAs(hWnd);
		else
			SaveFile(g_hWndEdit, g_CurrentFileName);
		break;
	case IDM_OPENFILE: OnFileOpen(hWnd); break;
	case IDM_SAVEFILEAS: OnFileSaveAs(hWnd); break;

	case IDM_RECENT1:
	case IDM_RECENT2:
	case IDM_RECENT3:
	case IDM_RECENT4:
	case IDM_RECENT5:
	case IDM_RECENT6:
	case IDM_RECENT7:
	case IDM_RECENT8:
	case IDM_RECENT9:
	{
		TCHAR buf[MAX_PATH] = { 0 };
		HMENU mainMenu = GetMenu(hWnd);
		int result = GetMenuString(mainMenu, commandID, buf, MAX_PATH - 1, 0);
		if (result == 0)
			MessageBox(hWnd, _T("GetMenuString() failed"), _T("Error"), MB_OK | MB_ICONERROR);
		else
			LoadFile(buf, g_hWndEdit);

		break;
	}
	case IDM_FIND: OnFind(hWnd, false);	break;
	case IDM_FINDNEXT: OnFind(hWnd, true); break;

	case IDM_EDITFONT:
	{
		CHOOSEFONT font;
		ZeroMemory(&font, sizeof(font));
		font.lStructSize = sizeof(CHOOSEFONT);
		font.hwndOwner = hWnd;
		font.lpLogFont = &Globals::lfLogFont;
		font.rgbColors = Globals::fcFontColor;
		font.Flags = CF_SCREENFONTS | CF_EFFECTS;

		//HFONT hfont = (HFONT)SendMessage(g_hWndEdit, WM_GETFONT, 0, 0);
		
		if (ChooseFont(&font) == TRUE)
		{
			HDC hdc = GetDC(g_hWndEdit);
			HFONT hfont = CreateFontIndirect(font.lpLogFont);
			SendMessage(g_hWndEdit, WM_SETFONT, (WPARAM)hfont, TRUE);
			Globals::fcFontColor = font.rgbColors;

			COLORREF prevColor = SetTextColor(hdc, font.rgbColors);
			if (prevColor == CLR_INVALID)
				MessageBox(hWnd, _T("this is text"), _T("this is caption"), MB_OK);
			//HFONT hfontPrev = SelectObject(hdc, hfont);
			//rgbCurrent = cf.rgbColors;
			//rgbPrev = SetTextColor(hdc, rgbCurrent);
		}
		break;
	}
	default:
		return DefWindowProc(hWnd, WM_COMMAND, MAKEWPARAM(commandID, code), (LPARAM)controlHandle);
	}

	return 0;
}

// Hit test the frame for resizing and moving.
LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// Get the point coordinates for the hit test.
	POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	// Get the window rectangle.
	RECT rcWindow;
	GetWindowRect(hWnd, &rcWindow);

	// Get the frame rectangle, adjusted for the style without a caption.
	RECT rcFrame = { 0 };
	AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

	// Determine if the hit test is for resizing. Default middle (1,1).
	USHORT uRow = 1;
	USHORT uCol = 1;
	bool fOnResizeBorder = false;

	// Determine if the point is at the top or bottom of the window.
	if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + Globals::TabTotalHeight)
	{
		fOnResizeBorder = (ptMouse.y < (rcWindow.top - rcFrame.top));
		uRow = 0;
	}
	else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - SystemInfo::YBorderSize) 
	{
		uRow = 2;
	}

	// Determine if the point is at the left or right of the window.
	if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + SystemInfo::XBorderSize)
	{
		uCol = 0; // left side
	}
	else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - SystemInfo::XBorderSize)
	{
		uCol = 2; // right side
	}

	// Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
	LRESULT hitTests[3][3] =
	{
		{ HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
		{ HTLEFT,       HTNOWHERE,     HTRIGHT },
		{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
	};

	return hitTests[uRow][uCol];
}


// Message handler for handling the custom caption messages.
LRESULT CustomCaptionProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool* pfCallDWP)
{
	LRESULT lRet = 0;
	HRESULT hr = S_OK;
	bool fCallDWP = true; // flag to call DefWindowProc in the end of this funtion

	fCallDWP = !DwmDefWindowProc(hWnd, message, wParam, lParam, &lRet);

	if (message == WM_ERASEBKGND) // disable Erase Background
	{
		lRet = 0;
		fCallDWP = false;
	}

	// Handle window creation.
	if (message == WM_CREATE)
	{
		RECT rcClient;
		GetWindowRect(hWnd, &rcClient); // TODO do we really need this SetWindowsPos?
		// Inform application of the frame change.
		SetWindowPos(hWnd, NULL, rcClient.left, rcClient.top, RECTW(rcClient), RECTH(rcClient), SWP_FRAMECHANGED);

		fCallDWP = true;
		lRet = 0;
	}

	// Handle window activation.
	if (message == WM_ACTIVATE)
	{
		// Extend the frame into the client area.
		MARGINS margins;
		 
		margins.cxLeftWidth = SystemInfo::XBorderSize;    
		margins.cxRightWidth = SystemInfo::XBorderSize;   
		margins.cyBottomHeight = SystemInfo::YBorderSize; 
		margins.cyTopHeight = Globals::TabTotalHeight;

		hr = DwmExtendFrameIntoClientArea(hWnd, &margins);

		if (!SUCCEEDED(hr))
		{
			// Handle error.
		}

		fCallDWP = true;
		lRet = 0;
	}

	if (message == WM_PAINT)
	{
		HDC hdc;
		{
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);
			//Tabs.DrawTabs(hWnd, hdc);
			EndPaint(hWnd, &ps);
		}

		fCallDWP = true;
		lRet = 0;
	}

	// Handle the non-client size message.
	if ((message == WM_NCCALCSIZE) && (wParam == TRUE))
	{
		// Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
		NCCALCSIZE_PARAMS* pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

		pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
		pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
		pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
		pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;

		lRet = 0;

		// No need to pass the message on to the DefWindowProc.
		fCallDWP = false;
	}

	// Handle hit testing in the NCA if not handled by DwmDefWindowProc.
	if ((message == WM_NCHITTEST) && (lRet == 0))
	{
		lRet = HitTestNCA(hWnd, wParam, lParam);

		if (lRet != HTNOWHERE)
		{
			fCallDWP = false;
		}
	}

	/*if (message == WM_MOUSEMOVE)
	{

	}*/

	*pfCallDWP = fCallDWP;

	return lRet;
}

LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool fCallDWP = true;
	BOOL fDwmEnabled = FALSE;
	LRESULT lRet = 0;
	HRESULT hr = S_OK;

	// Winproc worker for custom frame issues.
	//hr = DwmIsCompositionEnabled(&fDwmEnabled);
	//if (SUCCEEDED(hr))
	//{
		lRet = CustomCaptionProc(hWnd, message, wParam, lParam, &fCallDWP);
	//}

	// Winproc worker for the rest of the application.
	if (fCallDWP)
	{
		lRet = WndProc(hWnd, message, wParam, lParam);
	}
	return lRet;
}


//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//  PURPOSE: Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	if (message == g_uFindReplaceMsg)
	{
		auto fr = (FINDREPLACE*)lParam;

		//if (fr->Flags & FR_DIALOGTERM) // we are closing FindDlg 
		//{
		//	g_hFindDlg = nullptr; 
		//	return 0; 
		//} 

		if (fr->Flags & FR_MATCHCASE)
			SearchFor<string_t>(g_hWndEdit, fr);
		else
			SearchFor<ci_string>(g_hWndEdit, fr);

		return 0;
	}

	switch (message)
	{
	case WM_NOTIFY:
	{
		LPNMHDR ntData = (LPNMHDR)lParam;
		switch (ntData->code)
		{
		case TCN_SELCHANGING:
		{
			int sel = TabCtrl_GetCurSel(ntData->hwndFrom);
			assert(sel != -1);
			ShowWindow(g_hWndEdit, SW_HIDE);
			return FALSE;
		}
		case TCN_SELCHANGE:
		{
			int sel = TabCtrl_GetCurSel(ntData->hwndFrom);
			assert(sel != -1);
			ShowWindow(g_hWndEdit, SW_SHOW);
			return TRUE;
		}
		}
	}

	HANDLE_MSG(hWnd, WM_COMMAND, HandleWMCommand);
	
	case WM_CTLCOLOREDIT:
	{
		//static HBRUSH hbrushEditBox = nullptr;
		HDC hdcEdit = (HDC)wParam;
		SetTextColor(hdcEdit, Globals::fcFontColor); // Text color
		SetBkMode(hdcEdit, OPAQUE); // EditBox Backround Mode (note: OPAQUE can be used)
		SetBkColor(hdcEdit, GetSysColor(COLOR_WINDOW)); //(LONG)hbrushEditBox); // Backround color for EditBox

		//DeleteObject(hbrushEditBox);
		//hbrushEditBox = CreateSolidBrush(COLOR_WINDOW+1);
		return (LRESULT)GetSysColorBrush(COLOR_WINDOW); //hbrushEditBox; // Paint it
	}

	case WM_CLOSE:
		if (Edit_GetModify(g_hWndEdit))
		{
			int result = MessageBox(hWnd, _T("File is modified. Do you want to save it before exit?"), _T("Confirmation"), MB_YESNOCANCEL | MB_ICONQUESTION);
			if (result == IDCANCEL) return 0;
			if(result == IDYES) 	
				if (g_CurrentFileName.empty())
					OnFileSaveAs(hWnd);
				else
					SaveFile(g_hWndEdit, g_CurrentFileName);
		}
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	//HANDLE_MSG(hWnd, WM_SIZE, OnResize);


	//case WM_SETFOCUS:
	//	SetFocus(g_hWndEdit);
	//	break;

	case WM_CONTEXTMENU:
	{
		HWND hStBar = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if ((HWND)wParam == hStBar)
		{
			HMENU menu = LoadMenu(Globals::hInst, MAKEINTRESOURCE(IDR_STBARMENU));
			menu = GetSubMenu(menu, 0);
			WORD xPos = GET_X_LPARAM(lParam);
			WORD yPos = GET_Y_LPARAM(lParam);
			TrackPopupMenuEx(menu, 0, xPos, yPos, hWnd, 0);
			break;
		}	
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

INT_PTR CALLBACK DlgFind(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);

	static FINDREPLACE* fr = nullptr;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		fr = (FINDREPLACE*)lParam;
		SetDlgItemText(hDlg, IDC_COMBO1, fr->lpstrFindWhat);
		HWND hCbox = GetDlgItem(hDlg, IDC_COMBO1);
		SendMessage(hCbox, EM_SETLIMITTEXT, fr->wFindWhatLen, 0);

		CheckDlgButton(hDlg, IDC_CHECKWORDS, fr->Flags & FR_WHOLEWORD);
		CheckDlgButton(hDlg, IDC_CHECKCASE, fr->Flags & FR_MATCHCASE);
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_COMBO1, fr->lpstrFindWhat, fr->wFindWhatLen);
			fr->Flags = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKWORDS))? fr->Flags | FR_WHOLEWORD : fr->Flags & (~FR_WHOLEWORD);
			fr->Flags = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKCASE)) ? fr->Flags | FR_MATCHCASE : fr->Flags & (~FR_MATCHCASE);

			PostMessage(GetParent(hDlg), g_uFindReplaceMsg, 0, (LPARAM)fr);
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Retrieve and output the system error message for the last-error code
void DisplayError(HWND hWnd, const string_t& lpszFunction)
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + 
		lstrlen((LPCTSTR)lpszFunction.c_str()) + 100) * sizeof(TCHAR));

	if (FAILED(StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error code %d as follows:\n%s"), lpszFunction.c_str(), dw, lpMsgBuf)))
	{
		MessageBox(hWnd, _T("Unable to output error code."), _T("FATAL ERROR"), MB_OK | MB_ICONASTERISK);
	}

	//_tprintf(TEXT("ERROR: %s\n"), (LPCTSTR)lpDisplayBuf);
	MessageBox(hWnd, (LPCTSTR)lpDisplayBuf, TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
