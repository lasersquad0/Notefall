
#include "Application.h"
#include "SystemInfo.h"


Application::Application(HINSTANCE hInstance, const string_t& CmdLine, int nCmdShow) :
	FInstance(hInstance), FCmdLine(CmdLine), FCmdShow(nCmdShow), FTabsMan(this)
{
	LoadStrings();
	RegisterMainWindowClass();
	InitInstance();

}

void Application::LoadStrings()
{
	TCHAR buf[MAX_LDRSTR];
	LoadResString(IDC_NOTEFALL);
}

ATOM Application::RegisterMainWindowClass()
{
	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcStatic;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = FInstance;
	wcex.hIcon = LoadIcon(FInstance, MAKEINTRESOURCE(IDI_NOTEFALL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOWTEXT + 1);
	wcex.lpszMenuName = nullptr;// MAKEINTRESOURCE(IDC_NOTEFALL);
	wcex.lpszClassName = FStrMap[IDC_NOTEFALL].c_str();
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_NOTEFALL)); // IDI_SMALL));

	return RegisterClassEx(&wcex);
}

bool Application::InitInstance()
{
	LOG_INFO("Application::InitInstance()");

	//WS_EX_APPWINDOW
	FMainWnd = CreateWindowEx(0 /*WS_EX_LAYERED*/, FStrMap[IDC_NOTEFALL].c_str(), _T("Notefall"), 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, FInstance, nullptr);

	if (!FMainWnd)
	{
		DisplayError(nullptr, _T("Application.InitInstance.CreateWindowEx"));
		return false;
	}

	SetWindowLongPtr(FMainWnd, GWLP_USERDATA, (LONG_PTR)this);

	FTabsMan.Init(FMainWnd); //should be called right after main window is created


	//HWND hWndCaption = CreateWindowEx(/*WS_EX_LAYERED |*/ WS_EX_NOACTIVATE, _T(""), _T("title"), WS_CHILD | WS_BORDER,
		//0, 0, CW_USEDEFAULT, Globals::TabTotalHeight, hWnd, nullptr, hInstance, nullptr);

//	const auto DWM_DISABLE = DWMSBT_NONE; // Default
//	const auto MAINWINDOW = DWMSBT_MAINWINDOW; // Mica
//	const auto TRANSIENTWINDOW = DWMSBT_TRANSIENTWINDOW; // Acrylic
	const auto TABBEDWINDOW = DWMSBT_TABBEDWINDOW; // Tabbed
	const auto CAPT_COLOR = DWMWA_COLOR_DEFAULT; // #D0DCEC 
	const auto BORDER_COLOR = DWMWA_COLOR_DEFAULT; // 0x00ECDCD0; DWMWA_COLOR_NONE;

	HR_CHECK(DwmSetWindowAttribute(FMainWnd, DWMWA_CAPTION_COLOR, &CAPT_COLOR, sizeof(int)));
	HR_CHECK(DwmSetWindowAttribute(FMainWnd, DWMWA_BORDER_COLOR, &BORDER_COLOR, sizeof(int)));
	HR_CHECK(DwmSetWindowAttribute(FMainWnd, DWMWA_SYSTEMBACKDROP_TYPE, &TABBEDWINDOW, sizeof(int))); //DWMWA_SYSTEMBACKDROP_TYPE


	WTA_OPTIONS wopt;
	wopt.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
	wopt.dwMask = WTNCA_VALIDBITS;
	
	HR_CHECK(SetWindowThemeAttribute(FMainWnd, WTA_NONCLIENT, &wopt, sizeof(wopt)));

	//GdipGetImageGraphicsContext();
	/*
	const auto DWMWCP_DONOTROUND = 1;		// Rectangular
	const auto DWMWCP_ROUND = 2;			// Default
	const auto DWMWCP_ROUNDSMALL = 3;		// Semi-rounded
	DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &DWMWCP_ROUNDSMALL,	sizeof(int));
	*/

	// Extend the frame into the client area.
	MARGINS margins;

	margins.cxLeftWidth = SystemInfo::BorderSize.cx;
	margins.cxRightWidth = SystemInfo::BorderSize.cx;
	margins.cyBottomHeight = SystemInfo::BorderSize.cy;
	margins.cyTopHeight = TabsManager::TabTotalHeight;

	HR_CHECK(DwmExtendFrameIntoClientArea(FMainWnd, &margins));
	
	ShowWindow(FMainWnd, FCmdShow);
	UpdateWindow(FMainWnd);

	return true;
}

LRESULT CALLBACK Application::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool fCallDWP = true;
	LRESULT lRet = 0;
	Application* app;

	app = (Application*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (app == nullptr) return DefWindowProc(hWnd, message, wParam, lParam);

	// redirect custom caption messages to TabsManager
	// if CaptionWndProc does not process the message, it sets CallDWP to true that causes app->WndProc() called
	lRet = app->FTabsMan.CaptionWndProc(hWnd, message, wParam, lParam, &fCallDWP);

	// Winproc worker for the rest of the application.
	if (fCallDWP)
	{
		lRet = app->WndProc(hWnd, message, wParam, lParam);
	}
	
	return lRet;
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	//	case WM_ERASEBKGND: // disable Erase Background
	//	return 0;

	case WM_CLOSE:
		if (!FTabsMan.CloseAllTabs()) return 0; // stop closing 
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int Application::Run()
{
	HACCEL hAccelTable = LoadAccelerators(FInstance, MAKEINTRESOURCE(IDC_NOTEFALL));

	MSG msg;

	BOOL bRet = 0;
	while (bRet = GetMessage(&msg, nullptr, 0, 0))
	{
		if (-1 == bRet) break;

		try
		{
			if (!TranslateAccelerator(FMainWnd/*msg.hwnd*/, hAccelTable, &msg))
			{
				//if (g_hFindDlg == nullptr || !IsDialogMessage(g_hFindDlg, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		catch (std::exception& ex)
		{
			LOG_ERROR(MAKE_LOG_STR(ex.what()));
			MessageBoxA(nullptr, MAKE_LOG_STR(ex.what()).c_str(), "ERROR", MB_OK | MB_ICONASTERISK);
		}
		catch (...)
		{
			LOG_ERROR(MAKE_LOG_STR("UNKNOWN ERROR"));
			MessageBoxA(nullptr, MAKE_LOG_STR("UNKNOWN ERROR").c_str(), "ERROR", MB_OK | MB_ICONASTERISK);
		}
	}

	return (int)msg.wParam;
}

