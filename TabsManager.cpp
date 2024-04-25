
#include "TabsManager.h"
#include "Application.h"
#include "Registry.h"
#include "SettingsDlg.h"


TabsManager::TabsManager(Application* parent)
{
	FParent = parent;
	FInstance = parent->GetInstance();

	LoadImages();
	InitTabsData();
	InitMainMenu();
}

TabsManager::~TabsManager()
{
	for (auto tab : Tabs)
	{
		delete tab;
	}
	Tabs.clear();

	for (auto item : FImgMap)
	{
		delete item.second;
	}
	FImgMap.clear();

	DeleteObject(MainMenuBackgroundBrush); 

	UnhookWindowsHookEx(FMouseHook);
	FMouseHook = nullptr;
}

void TabsManager::Init(HWND mainWnd)
{
	FMainWnd = mainWnd;

	AddTab(true); // add one new tab 

	FMouseHook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseHookProc, FInstance, GetCurrentThreadId());
	assert(FMouseHook != nullptr);
}

LOGFONT TabsManager::CreateFontForEdit()
{
	LOGFONT lf{};

	TCHAR fname[] = _T("Consolas");
	StringCchCopy(lf.lfFaceName, lstrlen(fname) + 1, fname);
	lf.lfWeight = FW_NORMAL;
	lf.lfHeight = 17;
	lf.lfWidth = 0; // will be calculated by windows
	//lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfCharSet = 204;
	lf.lfPitchAndFamily = 33;

	return lf;

	// Create the font, and then return its handle.  
	//return CreateFont(lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
	//	lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision,
	//	lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
}

HFONT TabsManager::CreateHFontForEdit()
{
	LOGFONT lf = CreateFontForEdit();
	return CreateFontIndirect(&lf);

	//return CreateFont(lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
	//	lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision,
	//	lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
}

void TabsManager::InitTabsData()
{
	TabAddBtnSizeX = FImgMap[PNG_ADD]->GetWidth();
	TabAddBtnSizeY = FImgMap[PNG_ADD]->GetHeight();
	TabCloseBtnSizeX = FImgMap[PNG_CLOSE]->GetWidth();
	TabCloseBtnSizeY = FImgMap[PNG_CLOSE]->GetHeight();

	TabHeight = FImgMap[PNG_CENTER]->GetHeight();
	TabTotalHeight = TabHeight + SystemInfo::BorderSize.cy + TabMarginTop;
	TabWidth1of3 = FImgMap[PNG_LEFT]->GetWidth();
	TabWidth2of3 = FImgMap[PNG_CENTER]->GetWidth();
	TabWidth3of3 = FImgMap[PNG_RIGHT]->GetWidth();
	TabMaxWidth = TabWidth1of3 + TabWidth2of3 + TabWidth3of3;
	TabWidth = TabMaxWidth;

	TabMinWidth = TabTextMarginLeft + TabTextMinWidth + TabCloseBtnMarginRight + TabCloseBtnSizeX;

//#define CHECK_HRES(_) if(FAILED(_)) DisplayError(FMainWnd, _T("GetThemeColor"));
//
//	HTHEME hTheme = OpenThemeData(NULL, _T("MENU"));
//	HRESULT hResult = GetThemeColor(hTheme, MENU_POPUPITEM, MPI_NORMAL, TMT_COLOR, &MainMenuTextColor);
//	hResult = GetThemeColor(hTheme, MENU_POPUPITEM, MPI_NORMAL, TMT_MENU, &MainMenuTextColor);
//	hResult = GetThemeColor(hTheme, MENU_POPUPITEM, MPI_NORMAL, TMT_MENUBAR, &MainMenuTextColor);
//	hResult = GetThemeColor(hTheme, MENU_POPUPITEM, MPI_NORMAL, TMT_MENUHILIGHT, &MainMenuTextColor);
//
//	hResult = GetThemeColor(hTheme, MENU_POPUPBACKGROUND, 0, 0, &MainMenuItemHighlight);
//	hResult = GetThemeColor(hTheme, MENU_POPUPBACKGROUND, 0, TMT_COLOR, &MainMenuBackground);
//	hResult = GetThemeColor(hTheme, MENU_POPUPBACKGROUND, 0, TMT_MENUHILIGHT, &PopupShortcutText);
//	hResult = GetThemeColor(hTheme, MENU_POPUPBACKGROUND, 0, TMT_MENUTEXT, &PopupShortcutText);
//
//	hResult = GetThemeColor(hTheme, MENU_POPUPBACKGROUND, MPI_NORMAL, TMT_CAPTIONTEXT, &MainMenuItemHighlight);
//	hResult = GetThemeColor(hTheme, MENU_POPUPBACKGROUND, MPI_NORMAL, TMT_MENU, &MainMenuBackground);
//	hResult = GetThemeColor(hTheme, MENU_POPUPBACKGROUND, MPI_NORMAL, TMT_MENUHILIGHT, &PopupShortcutText);
//	hResult = GetThemeColor(hTheme, MENU_POPUPBACKGROUND, MPI_NORMAL, TMT_MENUTEXT, &PopupShortcutText);

//	CloseThemeData(hTheme);

	HTHEME theme = OpenThemeData(FMainWnd, VSCLASS_WINDOW); // _T("WINDOW")
	assert(theme != nullptr);

	MainMenuTextColor = GetThemeSysColor(theme, COLOR_MENUTEXT);
	MainMenuItemHighlightColor = GetThemeSysColor(theme, COLOR_HIGHLIGHT); //COLOR_MENUBAR); //COLOR_3DLIGHT);// COLOR_INACTIVECAPTION);
	MainMenuBackgroundColor = GetThemeSysColor(theme, COLOR_GRADIENTINACTIVECAPTION);//COLOR_MENUBAR);
	MainMenuShortcutTextColor = GetThemeSysColor(theme, COLOR_GRAYTEXT); //COLOR_GRAYTEXT
	MainMenuDisableTextColor = GetThemeSysColor(theme, COLOR_GRAYTEXT); //COLOR_GRAYTEXT

	MainMenuBackgroundBrush = CreateSolidBrush(MainMenuBackgroundColor);

	ActiveCaptionColor = GetThemeSysColor(theme, COLOR_HIGHLIGHT);
	//InactiveCaptionColor = GetThemeSysColor(theme, COLOR_INACTIVECAPTION);
	CaptionTextColor = GetThemeSysColor(theme, COLOR_CAPTIONTEXT);
	//InactiveCaptionTextColor = GetThemeSysColor(theme, COLOR_INACTIVECAPTIONTEXT);

	ComboItemTextColor = GetThemeSysColor(theme, COLOR_MENUTEXT);
	ComboItemHighlightColor = GetThemeSysColor(theme, COLOR_HIGHLIGHT);
	ComboItemNormalColor = GetThemeSysColor(theme, COLOR_WINDOW);
	ComboItemDisabledColor = GetThemeSysColor(theme, COLOR_INACTIVECAPTIONTEXT);
	
	WindowColor = GetThemeSysColor(theme, COLOR_BTNFACE); // COLOR_WINDOW);
	FontPreviewBrush = CreateSolidBrush(WindowColor);

	//CaptionBackgroundBrush = CreateSolidBrush(ActiveCaptionColor);
	
	//LOGFONT lf{};
	//HR_CHECK(GetThemeSysFont(theme, TMT_CAPTIONFONT, &lf));

	HR_CHECK(CloseThemeData(theme));
}

void TabsManager::InitMainMenu()
{
	// load main menu from resources
	FMenu = LoadMenu(FInstance, MAKEINTRESOURCE(IDC_NOTEFALL));
	FMainMenu = GetSubMenu(FMenu, 0); // this is main button menu

	// setting background color of the menu
	MENUINFO menuInfo{};
	menuInfo.cbSize = sizeof(MENUINFO);
	menuInfo.fMask = MIM_BACKGROUND;
	menuInfo.hbrBack = MainMenuBackgroundBrush;
	BOOL_CHECK(SetMenuInfo(FMainMenu, &menuInfo));

	MENUITEMINFO getItemInfo{};
	getItemInfo.cbSize = sizeof(MENUITEMINFO);
	getItemInfo.fMask = MIIM_STRING;
	string_t str;

	// init MAINMENUITEM structures for each menu item, because we have owner-drawn menu
	for (size_t i = 0; i < sizeof(MainMenuIDs) / sizeof(MainMenuIDs[0]); i++)
	{
		getItemInfo.dwTypeData = nullptr;
		BOOL_CHECK(GetMenuItemInfo(FMainMenu, MainMenuIDs[i].itemID, FALSE, &getItemInfo)); // getting menu text size

		str.resize(++getItemInfo.cch); // reserve memory for menu item text
		getItemInfo.dwTypeData = str.data();
		BOOL_CHECK(GetMenuItemInfo(FMainMenu, MainMenuIDs[i].itemID, FALSE, &getItemInfo)); // geting menu text itself

		size_t pos = str.find_first_of(_T('\t')); // split menu item text into menu item string and shortcut string
		if (pos != string_t::npos)
		{
			MainMenuIDs[i].itemText = str.substr(0, pos);
			MainMenuIDs[i].shortcutText = str.substr(pos + 1);
		}
		else
		{
			MainMenuIDs[i].itemText = str;
			MainMenuIDs[i].shortcutText.clear();
		}

		//MainMenuIDs[i].itemIcon = FIconMap[MainMenuIDs[i].iconID];
	}
}

//BOOL GetIconDimensions(HICON hico, SIZE* psiz)
//{
//	ICONINFO ii;
//	BOOL fResult = GetIconInfo(hico, &ii);
//	if (fResult) {
//		BITMAP bm;
//		fResult = GetObject(ii.hbmMask, sizeof(bm), &bm) == sizeof(bm);
//		if (fResult) {
//			psiz->cx = bm.bmWidth;
//			psiz->cy = ii.hbmColor ? bm.bmHeight : bm.bmHeight / 2;
//		}
//		if (ii.hbmMask)  DeleteObject(ii.hbmMask);
//		if (ii.hbmColor) DeleteObject(ii.hbmColor);
//	}
//	return fResult;
//}

void TabsManager::LoadImages()
{
	TCHAR buf[MAX_LDRSTR];
	LoadResString(IDS_NEWTABTITLE);
	LoadResString(IDS_REGKEYSOFT);
	LoadResString(IDS_COMPANY);
	LoadResString(IDS_APP_TITLE);
	LoadResString(IDS_CLEARECENT);
	LoadResString(IDS_RECENTMENU);
	LoadResString(IDS_REGKEYRECENT);

	FStrMap[IDS_FULLREGKEY] = FStrMap[IDS_REGKEYSOFT] + _T("\\") + FStrMap[IDS_COMPANY] + _T("\\") + FStrMap[IDS_APP_TITLE];
	FStrMap[IDS_FULLREGKEYRECENT] = FStrMap[IDS_FULLREGKEY] + _T("\\") + FStrMap[IDS_REGKEYRECENT];

	// loading caption images
	FImgMap[PNG_BCKG]       = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_BCKG));
	FImgMap[PNG_ADD]        = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_ADD));
	FImgMap[PNG_ADDHOVER]   = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_ADDHOVER));
	FImgMap[PNG_LEFT]       = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_LEFT));
	FImgMap[PNG_RIGHT]      = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_RIGHT));
	FImgMap[PNG_CENTER]     = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_CENTER));
	FImgMap[PNG_LEFTIN]     = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_LEFTIN));
	FImgMap[PNG_RIGHTIN]    = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_RIGHTIN));
	FImgMap[PNG_CENTERIN]   = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_CENTERIN));
	FImgMap[PNG_RIGHTINNODIV] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_RIGHTINNODIV));
	FImgMap[PNG_CLOSE]      = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_CLOSE));
	FImgMap[PNG_CLOSEHOVER] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_CLOSEHOVER));
	FImgMap[PNG_CLOSEBTN]   = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_CLOSEBTN));
	FImgMap[PNG_CLOSEHIGHLIGHT] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_CLOSEHIGHLIGHT));
	//FImgMap[PNG_MINIMIZE] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_MINIMIZE));
	//FImgMap[PNG_MAXIMIZE] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_MAXIMIZE));
	//FImgMap[PNG_RESTORE]  = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_RESTORE));
	FImgMap[PNG_CENTERINNF] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_CENTERINNF));
	FImgMap[PNG_LEFTINNF]   = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_LEFTINNF));
	FImgMap[PNG_RIGHTINNF]  = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_RIGHTINNF));
	FImgMap[PNG_RIGHTINNODIVNF] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_RIGHTINNODIVNF));
	//FImgMap[PNG_MENUBTN]  = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_MENUBTN));
	//FImgMap[PNG_MENUBTNHOVER] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_MENUBTNHOVER));
	//FImgMap[PNG_OPENFILE] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_OPENFILE));

	//HICON tabIcon = LoadResIcon(IDI_TABICON);
	//FImgMap[IDI_TABICON] = Gdiplus::Bitmap::FromHICON(tabIcon);
	//DestroyIcon(tabIcon);

	/*
	// Loading main menu icons. they are 16x16 that is why we use LoadImage instead of LoadIcon
	FIconMap[IDI_NEWFILE]    = LoadResIcon(IDI_NEWFILE);
	FIconMap[IDI_OPENFILE]   = LoadResIcon(IDI_OPENFILE);
	FIconMap[IDI_RECENT]     = LoadResIcon(IDI_RECENT);
	FIconMap[IDI_SAVEFILE]   = LoadResIcon(IDI_SAVEFILE);
	FIconMap[IDI_SAVEFILEAS] = LoadResIcon(IDI_SAVEFILEAS);
	FIconMap[IDI_SAVEALL]    = LoadResIcon(IDI_SAVEALL);
	FIconMap[IDI_FIND]       = LoadResIcon(IDI_FIND);
	FIconMap[IDI_FINDNEXT]   = LoadResIcon(IDI_FINDNEXT);
	FIconMap[IDI_REPLACE]    = LoadResIcon(IDI_REPLACE);
	FIconMap[IDI_UNDO]       = LoadResIcon(IDI_UNDO);
	FIconMap[IDI_CUT]        = LoadResIcon(IDI_CUT);
	FIconMap[IDI_COPY]       = LoadResIcon(IDI_COPY);
	FIconMap[IDI_PASTE]      = LoadResIcon(IDI_PASTE);
	FIconMap[IDI_PRINTFILE]  = LoadResIcon(IDI_PRINTFILE);
	FIconMap[IDI_SETTINGS]   = LoadResIcon(IDI_SETTINGS);
	*/
	
	FImgMap[PNG_TABICON] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_TABICON));

	// loading main menu icons/images
	FImgMap[PNG_NEWFILE] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_NEWFILE));
	FImgMap[PNG_OPENFILE] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_OPENFILE));
	FImgMap[PNG_RECENT] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_RECENT));
	FImgMap[PNG_SAVEFILE] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_SAVEFILE));
	FImgMap[PNG_SAVEFILEAS] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_SAVEFILEAS));
	FImgMap[PNG_SAVEALL] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_SAVEALL));
	FImgMap[PNG_FIND] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_FIND));
	FImgMap[PNG_FINDNEXT] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_FINDNEXT));
	FImgMap[PNG_REPLACE] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_REPLACE));
	FImgMap[PNG_UNDO] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_UNDO));
	FImgMap[PNG_CUT] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_CUT));
	FImgMap[PNG_COPY] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_COPY));
	FImgMap[PNG_PASTE] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_PASTE));
	FImgMap[PNG_PRINT] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_PRINT));
	FImgMap[PNG_SETTINGS] = LoadPNG(FInstance, MAKEINTRESOURCE(PNG_SETTINGS));
	
}

RECT TabsManager::GetAddButtonRect()
{
	POINT ptABtn{ (LONG)(TabMarginLeft + SystemInfo::BorderSize.cx + Tabs.size() * TabWidth + TabAddBtnMarginLeft),
		          (LONG)(SystemInfo::BorderSize.cy + TabMarginTop + TabAddBtnMarginTop) };
	RECT rcABtn{ ptABtn.x, ptABtn.y, ptABtn.x + (LONG)TabAddBtnSizeX, ptABtn.y + (LONG)TabAddBtnSizeY };
	
	return rcABtn;
}

RECT TabsManager::GetMenuBtnRect()
{
	POINT ptMenuBtn{ (LONG)(SystemInfo::BorderSize.cx + TabMenuBtnMarginLeft),
				  (LONG)(SystemInfo::BorderSize.cy + TabMenuBtnMarginTop ) };
	RECT rcMenuBtn{ ptMenuBtn.x, ptMenuBtn.y, ptMenuBtn.x + (LONG)TabMenuBtnSizeX, ptMenuBtn.y + (LONG)TabMenuBtnSizeY };

	return rcMenuBtn;
}

bool TabsManager::IsOverAddButton(POINT ptMouse)
{
	RECT rcAddBtn = GetAddButtonRect();
	return PtInRect(&rcAddBtn, ptMouse);
}

bool TabsManager::IsOverMenuBtn(POINT ptMouse)
{
	RECT rcAddBtn = GetMenuBtnRect();
	return PtInRect(&rcAddBtn, ptMouse);
}
/*
bool TabsManager::IsOverCloseButton(POINT ptMouse, LONG wndWidth)
{
	RECT captBtns;
	captBtns.left = wndWidth - 145 - 45;
	captBtns.right = captBtns.left + captBtnSize.cx;
	captBtns.top = 0;
	captBtns.bottom = captBtns.top + captBtnSize.cy;
	return PtInRect(&captBtns, ptMouse);
}

bool TabsManager::IsOverMaxButton(POINT ptMouse, LONG wndWidth)
{
	RECT captBtns;
	captBtns.left = wndWidth - 145 - 90;
	captBtns.right = captBtns.left + captBtnSize.cx;
	captBtns.top = 0;
	captBtns.bottom = captBtns.top + captBtnSize.cy;
	return PtInRect(&captBtns, ptMouse);
}

bool TabsManager::IsOverMinButton(POINT ptMouse, LONG wndWidth)
{
	RECT captBtns;
	captBtns.left = wndWidth - 145 - 135;
	captBtns.right = captBtns.left + captBtnSize.cx;
	captBtns.top = 0;
	captBtns.bottom = captBtns.top + captBtnSize.cy; 
	return PtInRect(&captBtns, ptMouse);
}
*/

ITab* TabsManager::AddTab(bool makeSelected)
{
	ITab* tab = (ITab*) new Tab(this, FStrMap[IDS_NEWTABTITLE]);
	Tabs.push_back(tab);
	
	if (makeSelected)
		SelectTab((uint)Tabs.size() - 1);
	
	CalcTabRects();
	RedrawTabs();

	return tab;
}

bool TabsManager::ValidTabIndex(uint index)
{
	return index >=0 && index < Tabs.size();
}

bool TabsManager::CloseAllTabs()
{
	while (Tabs.size() > 0)
	{
		if (!CloseTab((uint)Tabs.size() - 1u)) return false;
	}

	return true;
}

bool TabsManager::CloseTab(uint index)
{
	if (!ValidTabIndex(index)) return false;

	ITab* tabToClose = Tabs[index];
	bool closingSelectedTab = tabToClose == SelectedTab;

	if (tabToClose->IsModified())
	{	
		SelectTab(index);
		closingSelectedTab = true;
		string_t msgText;
		msgText.append(_T("File is modified:\n"));
		msgText.append(tabToClose->GetFileName());
		msgText.append(_T("\nDo you want to save it before exit?"));

		int result = MessageBox(FMainWnd, msgText.c_str(), tabToClose->GetTitle().c_str(), MB_YESNOCANCEL | MB_ICONQUESTION);
		if (result == IDCANCEL) return false;
		if (result == IDYES)
			if (!tabToClose->SaveFile()) return false;
	}

	delete tabToClose;
	Tabs.erase(Tabs.begin() + index);
	if(closingSelectedTab) SelectedTab = nullptr;
	if (closingSelectedTab && Tabs.size() > 0)
		SelectTab(index < Tabs.size() ? index : index - 1); // when closing selected tab then we need to make selected another (nearest) tab

	CalcTabRects();
	RedrawTabs();

	return true;
}


// does nothing if index is out of range
bool TabsManager::SelectTab(uint index)
{
	if (!ValidTabIndex(index)) return false;

	ITab* newSelectedTab = Tabs[index];
	if (SelectedTab == newSelectedTab) return true; // nothing to select

	if(SelectedTab != nullptr)
		SelectedTab->SetVisible(false);
	SelectedTab = newSelectedTab;
	SelectedTab->SetVisible(true);
	SelectedTab->Resize(); // make sure that FContent window has proper size (occupies full FBaseWnd area)
	RedrawTabs();
	return true;
}

uint TabsManager::TabIndexUnderMouse(POINT ptMouse)
{
	return (ptMouse.x - TabMarginLeft) / TabWidth;
}

// select tab under mouse click. ptMouseis in client coordinates
bool TabsManager::SelectTab(POINT ptMouse)
{
	uint tabIndex = TabIndexUnderMouse(ptMouse);
	if (tabIndex < Tabs.size()) SelectTab(tabIndex);
	return tabIndex < Tabs.size();
}

// Substract width of Min, Max and Close buttons from windows width. Returned width includes Add button. 
uint TabsManager::GetMaxTabAreaWidth(uint WindowWidth)
{
	return WindowWidth - 3 * SystemInfo::CaptionButton.cx - SystemInfo::BorderSize.cx;
}

void TabsManager::CalcTabRects()
{
	RECT rc;
	BOOL_CHECK(GetClientRect(FMainWnd, &rc));
	CalcTabRects(RECTW(rc));
}

void TabsManager::CalcTabRects(uint WindowWidth)
{
	if (Tabs.size() == 0) return; // nothing to calculate if no tabs

	uint TabAreaWidth = GetMaxTabAreaWidth(WindowWidth);
	int OnlyTabsWidth = (int)TabAreaWidth - (int)(TabsManager::TabAddBtnMarginLeft + TabsManager::TabAddBtnSizeX + TabsManager::TabMenuBtnMarginLeft + TabsManager::TabMenuBtnSizeX);
	TabWidth2of3 = min(FImgMap[PNG_CENTER]->GetWidth(), (uint)max(OnlyTabsWidth / (int)Tabs.size() - (int)(TabWidth1of3 + TabWidth3of3), 0));
	TabWidth = TabWidth1of3 + TabWidth2of3 + TabWidth3of3;
	TabTextMarginLeft = (TabTextMarginLeftInit - TabWidth1of3)*TabWidth/TabMaxWidth + TabWidth1of3/2; // reduce text left margin proportionally

	for (uint i = 0; i < Tabs.size(); i++)
	{
		Tabs[i]->CalcTabRect(i, Tabs.size() > 1);
	}
}

void TabsManager::Resize(int cx, int cy)
{
	UNREFERENCED_PARAMETER(cy);

	CalcTabRects(cx);

	SelectedTab->Resize();

	RedrawTabs();
}

void TabsManager::RedrawTabs()
{
	RECT rc;
	BOOL_CHECK(GetClientRect(FMainWnd, &rc));
	rc.left = SystemInfo::BorderSize.cx;
	rc.top = SystemInfo::BorderSize.cy;
	rc.bottom = TabTotalHeight;

	BOOL_CHECK(RedrawWindow(FMainWnd, &rc, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW));
}

// redraws Caption only if mouse is over one of caption buttons to highlight these buttons when mouse is over
void TabsManager::RedrawTabsIfOver(POINT ptMouse)
{
	if (FNeedRedraw) // force redraw caption again when mouse has left hot button area
	{
		FNeedRedraw = false;
		RedrawTabs();
	}

	// after force redraw (if it) we still need to check if mouse Is Over buttons and do required actions

	RECT rc;
	BOOL_CHECK(GetClientRect(FMainWnd, &rc));

	// TODO idea for optimization - we do not need to redraw each time when mouse is moving inside one button (e.g. inside Add button)
	if (IsOverAddButton(ptMouse) || IsOverMenuBtn(ptMouse) 
		// || IsOverCloseButton(ptMouse, RECTW(rc)) || IsOverMaxButton(ptMouse, RECTW(rc)) || IsOverMinButton(ptMouse, RECTW(rc))
		)
	{
		FNeedRedraw = true; // make full redraw next RedrawTabsIfOver call when mouse is NOT over hot buttons
		RedrawTabs();
	}
	else
	{
		for (auto tab : Tabs)
			if (tab->IsOverCloseBtn(ptMouse))
			{
				FNeedRedraw = true;
				RedrawTabs();
				break;
			}
	}
}

void TabsManager::DrawTabs(HDC hdc) // #CFDCEC
{
	HDC hdcPaint = CreateCompatibleDC(hdc);
	if (hdcPaint)
	{
		RECT rcClient;
		BOOL_CHECK(GetClientRect(FMainWnd, &rcClient));
		int TabsAreaWidth = RECTW(rcClient); // GetMaxTabAreaWidth(RECTW(rcClient));

		Gdiplus::Bitmap bitmap(TabsAreaWidth, TabTotalHeight, PixelFormat32bppARGB);
		auto gr = Gdiplus::Graphics::FromImage(&bitmap);
		assert(gr != nullptr);

		//gr->DrawImage(FImgMap[PNG_BCKG], 0, 0, TabsAreaWidth, TabTotalHeight);
		Gdiplus::Rect CaptRect(0,0, TabsAreaWidth, TabTotalHeight);
		Gdiplus::SolidBrush CaptBrush(ActiveCaptionColor);
		gr->FillRectangle(&CaptBrush, CaptRect);

		POINT ptMouse;
		BOOL_CHECK(GetCursorPos(&ptMouse));
		BOOL_CHECK(ScreenToClient(FMainWnd, &ptMouse));

		bool hasFocus = FWindowActive; //!!GetFocus();// GetForegroundWindow() == FMainWnd;

		for (size_t i = 0; i < Tabs.size(); i++)
		{
			ITab* tab = Tabs[i];

			if (tab == SelectedTab)  // draw active tab here
			{
				gr->DrawImage(FImgMap[PNG_LEFT], (INT)tab->TabXY.x, tab->TabXY.y);
				gr->DrawImage(FImgMap[PNG_CENTER], tab->TabXY.x + TabWidth1of3, tab->TabXY.y, (INT)TabWidth2of3, TabHeight);
				gr->DrawImage(FImgMap[PNG_RIGHT], (INT)(tab->TabXY.x + TabWidth1of3 + TabWidth2of3), tab->TabXY.y);
			}
			else	//draw inactive tab here
			{
				if (hasFocus)
				{
					gr->DrawImage(FImgMap[PNG_LEFTIN], (int)tab->TabXY.x, tab->TabXY.y);
					gr->DrawImage(FImgMap[PNG_CENTERIN], (INT)(tab->TabXY.x + TabWidth1of3), tab->TabXY.y, TabWidth2of3, TabHeight);

					if (i + 1 < Tabs.size() && Tabs[i + 1] == SelectedTab) // draw no divider tab before active tab
						gr->DrawImage(FImgMap[PNG_RIGHTINNODIV], (INT)(tab->TabXY.x + TabWidth1of3 + TabWidth2of3), tab->TabXY.y);
					else
						gr->DrawImage(FImgMap[PNG_RIGHTIN], (INT)(tab->TabXY.x + TabWidth1of3 + TabWidth2of3), tab->TabXY.y);
				}
				else
				{
					gr->DrawImage(FImgMap[PNG_LEFTINNF], (int)tab->TabXY.x, tab->TabXY.y);
					gr->DrawImage(FImgMap[PNG_CENTERINNF], (INT)(tab->TabXY.x + TabWidth1of3), tab->TabXY.y, TabWidth2of3, TabHeight);

					if (i + 1 < Tabs.size() && Tabs[i + 1] == SelectedTab) // draw no divider tab before active tab
						gr->DrawImage(FImgMap[PNG_RIGHTINNODIVNF], (INT)(tab->TabXY.x + TabWidth1of3 + TabWidth2of3), tab->TabXY.y);
					else
						gr->DrawImage(FImgMap[PNG_RIGHTINNF], (INT)(tab->TabXY.x + TabWidth1of3 + TabWidth2of3), tab->TabXY.y);
				}
			}

			// if tab width is too small -> do not draw tab icon
			//bool drawTabIcon = TabWidth > TabCloseBtnSizeX + TabCloseBtnMarginRight + TabIconSizeX + TabIconMarginLeft + TabTextMarginLeft;
			if(tab->NeedDrawTabIcon())
				gr->DrawImage(FImgMap[PNG_TABICON], tab->IconXY.x, tab->IconXY.y, (INT)TabIconSizeX, TabIconSizeY);

			if (Tabs.size() > 1) // do not draw Close button is we have only one tab
			{
				if (tab->IsOverCloseBtn(ptMouse))
					gr->DrawImage(FImgMap[PNG_CLOSEHOVER], tab->CloseBtnXY.x, tab->CloseBtnXY.y, (INT)TabCloseBtnSizeX, TabCloseBtnSizeY);
				else
					gr->DrawImage(FImgMap[PNG_CLOSE], tab->CloseBtnXY.x, tab->CloseBtnXY.y, (INT)TabCloseBtnSizeX, TabCloseBtnSizeY);
			}

			LOGFONT font = SystemInfo::CaptionFont;
			if(tab == SelectedTab)
				font.lfWeight = FW_BOLD;
			Gdiplus::Font gdifont(hdcPaint, &font);
			
			Gdiplus::Color color;
			color.SetFromCOLORREF(CaptionTextColor);
			Gdiplus::SolidBrush brush(color);
			Gdiplus::RectF rc((Gdiplus::REAL)tab->TextRect.left, (Gdiplus::REAL)tab->TextRect.top, (Gdiplus::REAL)RECTW(tab->TextRect), (Gdiplus::REAL)RECTH(tab->TextRect));

			auto strFmt = Gdiplus::StringFormat(Gdiplus::StringFormatFlagsNoWrap);
			strFmt.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);

			gr->DrawString(tab->GetTitle().c_str(), (INT)tab->GetTitle().size(), &gdifont, rc, &strFmt, &brush);
		}

		RECT rcABtn = GetAddButtonRect();

		if (IsOverAddButton(ptMouse)) //(PtInRect(&rcABtn, ptMouse))
			gr->DrawImage(FImgMap[PNG_ADDHOVER], (INT)rcABtn.left, (INT)rcABtn.top);
		else
			gr->DrawImage(FImgMap[PNG_ADD], (INT)rcABtn.left, (INT)rcABtn.top);

		if (IsOverMenuBtn(ptMouse))
		{
			Gdiplus::SolidBrush MainMenuBtnBrush(Gdiplus::Color::MakeARGB(255, 0xDE, 0xE1, 0xE6));
			//gr->FillRectangle(&MainMenuBtnBrush, (INT)TabMenuBtnMarginLeft, TabMenuBtnMarginTop, TabMenuBtnSizeX, TabMenuBtnSizeY);
			Gdiplus::Point points[8]{ {TabMenuBtnMarginLeft + 4, TabMenuBtnMarginTop},
									  {TabMenuBtnMarginLeft + TabMenuBtnSizeX - 4, TabMenuBtnMarginTop},
									  {TabMenuBtnMarginLeft + TabMenuBtnSizeX, TabMenuBtnMarginTop + 4},
									  {TabMenuBtnMarginLeft + TabMenuBtnSizeX, TabMenuBtnMarginTop + TabMenuBtnSizeY - 4},
									  {TabMenuBtnMarginLeft + TabMenuBtnSizeX - 4, TabMenuBtnMarginTop + TabMenuBtnSizeY},
									  {TabMenuBtnMarginLeft + 4, TabMenuBtnMarginTop + TabMenuBtnSizeY},
									  {TabMenuBtnMarginLeft, TabMenuBtnMarginTop + TabMenuBtnSizeY - 4},
									  {TabMenuBtnMarginLeft, TabMenuBtnMarginTop + 4} };
									  //{TabMenuBtnMarginLeft + 4, TabMenuBtnMarginTop} };


			gr->FillClosedCurve(&MainMenuBtnBrush, points, 8, Gdiplus::FillModeAlternate, 0.2);
			//gr->DrawImage(FImgMap[PNG_MENUBTNHOVER], (INT)TabMenuBtnMarginLeft, TabMenuBtnMarginTop);
		}
		else
		{
			//gr->DrawImage(FImgMap[PNG_MENUBTN], (INT)TabMenuBtnMarginLeft, TabMenuBtnMarginTop);
		}

		Gdiplus::Point p1{ TabMenuBtnMarginLeft + 14, TabMenuBtnMarginTop + 11 };
		const Gdiplus::Pen MainMenuPen(Gdiplus::Color::MakeARGB(255, 0, 0, 0));
		gr->DrawLine(&MainMenuPen, p1.X, p1.Y, p1.X + 16, p1.Y);
		p1.Y = p1.Y + 5;
		gr->DrawLine(&MainMenuPen, p1.X, p1.Y, p1.X + 16, p1.Y);
		p1.Y = p1.Y + 5;
		gr->DrawLine(&MainMenuPen, p1.X, p1.Y, p1.X + 16, p1.Y);


		//Brush _minimizeMaximizeButtonHighlight = new SolidBrush(Color.FromArgb(27, Color.Black));
		//Brush _closeButtonHighlight = new SolidBrush(Color.FromArgb(232, 17, 35));
		//Gdiplus::SolidBrush MinBtnBrush(Gdiplus::Color::MakeARGB(27, 0, 0, 0));      // TODO move it to class fields
		//Gdiplus::SolidBrush CloseBtnBrush(Gdiplus::Color::MakeARGB(255, 232, 17, 35));

		//RECT btns{ rcABtn.left + 50, 0, rcABtn.left + 50+ 45, 29 };
		//if(IsOverMinButton(ptMouse, RECTW(rcClient)))
		//	gr->FillRectangle(&MinBtnBrush, btns.left, 0, RECTW(btns), RECTH(btns));
		//if (IsOverMaxButton(ptMouse, RECTW(rcClient)))
		//	gr->FillRectangle(&MinBtnBrush, btns.left + 50, 0, RECTW(btns), RECTH(btns));
		//if (IsOverCloseButton(ptMouse, RECTW(rcClient)))
		//{
		//	gr->FillRectangle(&CloseBtnBrush, btns.left + 100, 0, RECTW(btns), RECTH(btns));
		//	gr->DrawImage(FImgMap[PNG_CLOSEHIGHLIGHT], (INT)btns.left + 100 + 17, 10);
		//}
		//else
		//	gr->DrawImage(FImgMap[PNG_CLOSEBTN], (INT)btns.left + 100 + 17, 10);


		//gr->DrawImage(FImgMap[PNG_MINIMIZE], (INT)btns.left + 17, 10);
		//gr->DrawImage(FImgMap[PNG_MAXIMIZE], (INT)btns.left + 50 + 17, 10);
		
		
		HBITMAP hbmp;
		//Gdiplus::Color clr;
		//clr.SetFromCOLORREF(0); // #DEE1E6
		bitmap.GetHBITMAP(Gdiplus::Color::MakeARGB(0, 0, 0, 0), &hbmp);
		//bitmap.GetHBITMAP(Gdiplus::Color::MakeARGB(255, 0xCF, 0xDC, 0xEC), &hbmp);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcPaint, hbmp);

		//BOOL_CHECK(TransparentBlt(hdc, 0, 0, TabsAreaWidth, TabTotalHeight, hdcPaint, 0, 0, TabsAreaWidth, TabTotalHeight, 0xCFDCEC)); //ActiveCaptionColor)); //0xCFDCEC));
		BOOL_CHECK(BitBlt(hdc, 0, 0, TabsAreaWidth, TabTotalHeight, hdcPaint, 0, 0, SRCCOPY));

		SelectObject(hdcPaint, hbmOld);
		BOOL_CHECK(DeleteObject(hbmp));
		BOOL_CHECK(DeleteDC(hdcPaint));
		delete gr;
	}
}

HFONT TabsManager::CreateFontForShortcut()
{
	LOGFONT lf{};  
	
	string_t fname = _T("Times New Roman");
	StringCchCopy(lf.lfFaceName , fname.size(), fname.c_str());

	lf.lfWeight = FW_NORMAL;
	lf.lfHeight = 13;

	// Create the font, and then return its handle.  
	return CreateFont(lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
					  lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision,
					  lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
}

LOGFONT TabsManager::CreateFontForShortcut1()
{
	LOGFONT lf{};

	string_t fname = _T("Times New Roman");
	StringCchCopy(lf.lfFaceName, fname.size(), fname.c_str());

	lf.lfWeight = FW_NORMAL;
	lf.lfHeight = 13;

	// Create the font, and then return its handle.  
	return lf;
}

HMENU TabsManager::GetRecentMenu()
{
	HMENU submenuRecent = CreatePopupMenu();
	
	// setting background color of the menu
	MENUINFO menuInfo{};
	menuInfo.cbSize = sizeof(MENUINFO);
	menuInfo.fMask = MIM_BACKGROUND;
	menuInfo.hbrBack = MainMenuBackgroundBrush;
	BOOL_CHECK(SetMenuInfo(submenuRecent, &menuInfo));

	MENUITEMINFO itemInfo{};
	itemInfo.cbSize = sizeof(MENUITEMINFO);
	itemInfo.fMask = MIIM_FTYPE | MIIM_DATA;
	itemInfo.fType = MFT_OWNERDRAW;

	if (Registry::CurrentUser->Exists(FStrMap[IDS_FULLREGKEYRECENT]))
	{
		auto key = Registry::CurrentUser->Open(FStrMap[IDS_FULLREGKEYRECENT]);

		MAINMENUITEM* recentIDs = RecentMenuIDs;
		key->EnumerateValues([&key, submenuRecent, &itemInfo, recentIDs](const std::wstring& valueName, const uint num) -> bool
			{
				string_t val = key->GetString(valueName);
				BOOL_CHECK(AppendMenu(submenuRecent, MF_OWNERDRAW, (UINT_PTR)(recentIDs[num].itemID), val.c_str()));
				recentIDs[num].itemText = val;
				itemInfo.dwItemData = (LONG_PTR)(recentIDs + num);
				BOOL_CHECK(SetMenuItemInfo(submenuRecent, recentIDs[num].itemID, FALSE, &itemInfo));

				if (num >= MAX_RECENTS) return false; // we support up to 9 recent items

				return true; // Return true to continue processing, false otherwise
			});
	}

	BOOL_CHECK(AppendMenu(submenuRecent, MF_SEPARATOR, 0, 0));

	// configure 'Clear Recent' menu item
	BOOL_CHECK(AppendMenu(submenuRecent, MF_OWNERDRAW, (UINT_PTR)IDS_CLEARECENT, FStrMap[IDS_CLEARECENT].c_str()));
	RecentMenuIDs[MAX_RECENTS].itemText = FStrMap[IDS_CLEARECENT];
	itemInfo.dwItemData = (LONG_PTR)(RecentMenuIDs + MAX_RECENTS);
	BOOL_CHECK(SetMenuItemInfo(submenuRecent, IDS_CLEARECENT, FALSE, &itemInfo));

	return submenuRecent;
}

void TabsManager::ShowMainMenu()
{
	MENUITEMINFO itemInfo{};
	itemInfo.cbSize = sizeof(MENUITEMINFO);
	itemInfo.fMask = MIIM_FTYPE | MIIM_DATA;
	itemInfo.fType = MFT_OWNERDRAW;
	
	// define all menu items as owner-drawn, fill in dwItemData field with menu item info (text, icon, etc.)
	for (size_t i = 0; i < sizeof(MainMenuIDs)/sizeof(MainMenuIDs[0]); i++)
	{
		itemInfo.dwItemData = (LONG_PTR) (&(MainMenuIDs[i]));
		BOOL_CHECK(SetMenuItemInfo(FMainMenu, MainMenuIDs[i].itemID, FALSE, &itemInfo));
	}

	MENUITEMINFO recentItemInfo{};
	recentItemInfo.cbSize = sizeof(MENUITEMINFO);
	recentItemInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
	recentItemInfo.hSubMenu = GetRecentMenu();
	int itemCnt = GetMenuItemCount(recentItemInfo.hSubMenu);

	// disable Recents item of main menu if we do not have recent items in submenu
	(itemCnt < 3) ? recentItemInfo.fState = MFS_DISABLED : recentItemInfo.fState = MFS_ENABLED;  // Separator menu item + Clear Recent menu item
	BOOL_CHECK(SetMenuItemInfo(FMainMenu, IDM_OPENRECENT, FALSE, &recentItemInfo));

	RECT menuRect = GetMenuBtnRect();
	POINT ptMenu{ 0, menuRect.bottom };
	BOOL_CHECK(ClientToScreen(FMainWnd, &ptMenu));

	TrackPopupMenuEx(FMainMenu, TPM_HORPOSANIMATION /* | TPM_HORNEGANIMATION*/, ptMenu.x, ptMenu.y, FMainWnd, 0);
	
	DestroyMenu(recentItemInfo.hSubMenu);
}

// highlight Add and Close buttons when mouse is over them
void TabsManager::OnNCMouseMove(HWND hWnd, int x, int y, UINT HitTest)
{
	if (HitTest == HTCAPTION)
	{
		POINT ptMouse = { x, y };
		BOOL_CHECK(ScreenToClient(hWnd, &ptMouse));
		RedrawTabsIfOver(ptMouse);
	}
}

// only select tab on button down. 
void TabsManager::OnMouseNCLButtonDown(HWND hWnd, BOOL /*fDoubleClick*/, int x, int y, UINT codeHitTest)
{
	if (codeHitTest == HTCAPTION)
	{
		POINT ptMouse{ x,y };
		BOOL_CHECK(ScreenToClient(hWnd, &ptMouse));

		if (!SelectTab(ptMouse))
			if (IsOverMenuBtn(ptMouse))
			{
				ShowMainMenu();
			}
	}
}

void TabsManager::OnMouseNCLButtonUp(HWND hWnd, int x, int y, UINT codeHitTest)
{
	if (codeHitTest == HTCAPTION)
	{
		POINT ptMouse{ x,y };
		BOOL_CHECK(ScreenToClient(hWnd, &ptMouse));

		if (IsOverAddButton(ptMouse))
		{
			AddTab();
		}
		else
		{
			uint tabIndex = TabIndexUnderMouse(ptMouse);
			if (tabIndex < Tabs.size())
			{
				if (Tabs.size() > 1 && Tabs[tabIndex]->IsOverCloseBtn(ptMouse))
					CloseTab(tabIndex);
			}
		}
	}
}

// Hit test the frame for resizing and moving.
LRESULT TabsManager::HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	// Get the point coordinates for the hit test.
	POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	POINT ptMouseClient = ptMouse;
	BOOL_CHECK(ScreenToClient(hWnd, &ptMouseClient));

	// Get the window rectangle.
	RECT rcWindow;
	BOOL_CHECK(GetWindowRect(hWnd, &rcWindow));

	// Get the frame rectangle, adjusted for the style without a caption.
	//RECT rcFrame = { 0 };
	//AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

	// Determine if the hit test is for resizing. Default middle (1,1).
	USHORT uRow = 1;
	USHORT uCol = 1;
	USHORT captHitTest = HTCAPTION;
	//bool fOnResizeBorder = false;

	if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + SystemInfo::BorderSize.cy)
	{
		captHitTest = HTTOP;
	}

	//if (FTabsMan.IsOverCloseButton(ptMouseClient, RECTW(rcWindow)))
	//{
	//	captHitTest = HTCLOSE;
	//}

	//if (FTabsMan.IsOverMinButton(ptMouseClient, RECTW(rcWindow)))
	//{
	//	captHitTest = HTMINBUTTON;
	//}

	//if (FTabsMan.IsOverMaxButton(ptMouseClient, RECTW(rcWindow)))
	//{
	//	captHitTest = HTMAXBUTTON;
	//}

	// Determine if the point is at the top or bottom of the window.
	if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + (LONG)TabsManager::TabTotalHeight)
	{
		//fOnResizeBorder = (ptMouse.y < (rcWindow.top - rcFrame.top));
		uRow = 0;
	}
	else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - SystemInfo::BorderSize.cy)
	{
		uRow = 2;
	}

	// Determine if the point is at the left or right of the window.
	if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + SystemInfo::BorderSize.cx)
	{
		uCol = 0; // left side
	}
	else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - SystemInfo::BorderSize.cx)
	{
		uCol = 2; // right side
	}

	// Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
	LRESULT hitTests[3][3] =
	{
		//{ HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
		{ HTTOPLEFT,    captHitTest,    HTTOPRIGHT },
		{ HTLEFT,       HTNOWHERE,     HTRIGHT },
		{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
	};

	return hitTests[uRow][uCol];
}

void TabsManager::OnDrawMenuItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
	UNREFERENCED_PARAMETER(hwnd);

	RECT rcItem = lpDrawItem->rcItem; // for convenience
	HDC itemDC = lpDrawItem->hDC;
	MAINMENUITEM* itemData = (MAINMENUITEM*)lpDrawItem->itemData;

	HFONT newFont = CreateFontPlus1(itemDC);
	HFONT oldFont = (HFONT)SelectObject(itemDC, newFont);
	COLORREF oldBkgnd = SetBkColor(itemDC, TabsManager::MainMenuBackgroundColor);
	COLORREF oldText = SetTextColor(itemDC, TabsManager::MainMenuTextColor);

	//Gdiplus::Color bgColor;
	// If the user has selected the item, use the selected text and background colors to display the item. 
	if (lpDrawItem->itemState & ODS_SELECTED)
	{
		//bgColor.SetFromCOLORREF(TabsManager::MainMenuItemHighlightColor);
		SetBkColor(itemDC, TabsManager::MainMenuItemHighlightColor);
	}
	if (lpDrawItem->itemState & ODS_DISABLED)
	{
		//bgColor.SetFromCOLORREF(TabsManager::MainMenuDisableTextColor);
		SetTextColor(itemDC, TabsManager::MainMenuDisableTextColor);
	}

	// Remember to leave space in the menu item for the icon.
	int nIconX = rcItem.left + TabsManager::MainMenuIconMarginLeft;
	int nIconY = rcItem.top + (RECTH(rcItem) - SystemInfo::SmallIconY) / 2;

	int nTextX = TabsManager::MainMenuTextMarginLeft + (itemData->iconID? (nIconX + SystemInfo::SmallIconX) : 0); // some menu items do not have icons
	int nTextY = rcItem.top + TabsManager::MainMenuTextMarginTop;

	//Gdiplus::SolidBrush bgBrush(RGB(0x0, 0x0, 0x0));
	//gr->FillRectangle(&bgBrush, 0, 0, RECTW(rcItem), RECTH(rcItem)); // fill bitmap background

	//LOGFONT font = SystemInfo::CaptionFont;
	//Gdiplus::Font textFont(compatDC, &font);
	//Gdiplus::SolidBrush txtBrush(TabsManager::MainMenuTextColor);
	//Gdiplus::RectF textRect((Gdiplus::REAL)nTextX, (Gdiplus::REAL)nTextY, (Gdiplus::REAL)RECTW(rcItem), (Gdiplus::REAL)RECTH(rcItem));
	//auto strFmt = Gdiplus::StringFormat(Gdiplus::StringFormatFlagsNoWrap);
	//strFmt.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);

	//gr->DrawString(itemData->itemText.c_str(), (INT)itemData->itemText.size(), &textFont, textRect, &strFmt, &txtBrush);

	/*Gdiplus::Font scFont(compatDC, TabsManager::MainMenuShortcutFont);
	Gdiplus::SolidBrush shortcutBrush(TabsManager::MainMenuShortcutTextColor);
	Gdiplus::RectF scRect((Gdiplus::REAL)nTextX, (Gdiplus::REAL)nTextY, (Gdiplus::REAL)RECTW(rcItem) - nTextX - TabsManager::MainMenuTextMarginRight, (Gdiplus::REAL)RECTH(rcItem) - nTextY);
	strFmt.SetAlignment(Gdiplus::StringAlignmentFar);
	gr->DrawString(itemData->shortcutText.c_str(), (INT)itemData->shortcutText.size(), &scFont, scRect, &strFmt, &shortcutBrush);*/

	//gr->DrawImage(FImgMap[itemData->iconID], (INT)nIconX, nIconY);

	//RECT txtRe{nTextX, nTextY, RECTW(rcItem), RECTH(rcItem)};
	BOOL_CHECK(ExtTextOut(itemDC, nTextX, nTextY, ETO_OPAQUE, &rcItem, itemData->itemText.c_str(), (UINT)itemData->itemText.size(), NULL));

	// Select the font associated with the item into the item's device context, and then draw the string. 
	SelectObject(itemDC, TabsManager::MainMenuShortcutFont);
	SetTextColor(itemDC, TabsManager::MainMenuShortcutTextColor);
	RECT rc = rcItem;
	BOOL_CHECK(DrawText(itemDC, itemData->shortcutText.c_str(), (int)itemData->shortcutText.size(), &rc, DT_CALCRECT));

	
	rc.left = rcItem.left;
	rc.top = rcItem.top + (RECTH(rcItem) - RECTH(rc)) / 2;
	rc.right = rcItem.right - TabsManager::MainMenuTextMarginRight;
	rc.bottom = rcItem.bottom;

	BOOL_CHECK(DrawText(itemDC, itemData->shortcutText.c_str(), (int)itemData->shortcutText.size(), &rc, DT_RIGHT));

	HDC compatDC = CreateCompatibleDC(itemDC);
	assert(compatDC != nullptr);

	Gdiplus::Bitmap bitmap(SystemInfo::SmallIconX, SystemInfo::SmallIconY, PixelFormat32bppARGB);
	auto gr = Gdiplus::Graphics::FromImage(&bitmap);
	assert(gr != nullptr);
	gr->DrawImage(FImgMap[itemData->iconID], 0, 0);

	HBITMAP hbmp;
	bitmap.GetHBITMAP(Gdiplus::Color::MakeARGB(0, 0xE8, 0xE8, 0xE8), &hbmp);
	SelectObject(compatDC, hbmp);

	BOOL_CHECK(TransparentBlt(itemDC, nIconX, nIconY, SystemInfo::SmallIconX, SystemInfo::SmallIconY, compatDC, 0, 0, SystemInfo::SmallIconX, SystemInfo::SmallIconY, 0xE8E8E8));

	BOOL_CHECK(DeleteObject(hbmp));
	BOOL_CHECK(DeleteDC(compatDC));
	delete gr;

	//HBRUSH br = CreateSolidBrush(RGB(0xE8, 0xE8, 0xE8));
	//if(itemData->iconID)
		//BOOL_CHECK(DrawIconEx(itemDC, nIconX, nIconY, itemData->itemIcon, SystemInfo::SmallIconX, SystemInfo::SmallIconY, 0, br, DI_IMAGE| DI_MASK));


	//HBITMAP hbmp;
	//bitmap.GetHBITMAP(Gdiplus::Color::MakeARGB(0, 0xE8, 0x0, 0x0), &hbmp);
	//HBITMAP hbmOld = (HBITMAP)SelectObject(compatDC, hbmp);

	//bool RES = TransparentBlt(itemDC, rcItem.left, rcItem.top, RECTW(rcItem), RECTH(rcItem), compatDC, 0, 0, RECTW(rcItem), RECTH(rcItem), 0xE8E8E8);
	//BOOL_CHECK(BitBlt(itemDC, rcItem.left, rcItem.top, RECTW(rcItem), RECTH(rcItem), compatDC, 0, 0, SRCCOPY));

	//SelectObject(itemDC, oldFont);
	//BOOL_CHECK(DeleteObject(hbmp));
//	BOOL_CHECK(DeleteDC(compatDC));
//	delete gr;

	// Select the previous font back into the device context. 
	SelectObject(itemDC, oldFont);
	SetBkColor(itemDC, oldBkgnd);
	SetTextColor(itemDC, oldText);
	BOOL_CHECK(DeleteObject(newFont));
}

void TabsManager::OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
	if (lpMeasureItem->CtlType == ODT_MENU)
	{
		if (lpMeasureItem->itemID >= IDM_RECENT1 && lpMeasureItem->itemID <= IDM_RECENT9)
		{
			auto itemData = (MAINMENUITEM*)lpMeasureItem->itemData;

			SIZE textSize{};
			HDC DC = GetDC(hWnd);
			BOOL_CHECK(GetTextExtentPoint32(DC, itemData->itemText.c_str(), (int)itemData->itemText.size(), &textSize));
			lpMeasureItem->itemHeight = MainMenuItemHeight;
			lpMeasureItem->itemWidth = textSize.cx;
			BOOL_CHECK(ReleaseDC(hWnd, DC));
		}
		else
		{
			lpMeasureItem->itemHeight = MainMenuItemHeight;
			lpMeasureItem->itemWidth = MainMenuWidth;
		}
	}
}

// Message handler for handling the custom caption messages.
LRESULT TabsManager::CaptionWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool* pfCallDWP)
{
	LRESULT lRet = 0; // default
	bool fCallDWP = true; // default: flag to call DefWindowProc in the end of this funtion

	fCallDWP = !DwmDefWindowProc(hWnd, message, wParam, lParam, &lRet);

	switch (message)
	{
	case WM_SIZE:
	{
		Resize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
	break;

	case WM_MEASUREITEM:
	{
		OnMeasureItem(hWnd, (MEASUREITEMSTRUCT*)lParam);
		lRet = 1;
		fCallDWP = false; // No need to pass the message on to the DefWindowProc.
	}
	break;

	case WM_DRAWITEM:
	{
		OnDrawMenuItem(hWnd, (DRAWITEMSTRUCT*)(lParam));
		lRet = 1;
		fCallDWP = false; // No need to pass the message on to the DefWindowProc.
	}
	break;

	case WM_ERASEBKGND: // disable Erase Background
	{
		lRet = 0;
		fCallDWP = false; // No need to pass the message on to the DefWindowProc.
	}
	break;

	case WM_CREATE:
	{
		RECT rcClient;
		BOOL_CHECK(GetWindowRect(hWnd, &rcClient));
		// This is needed to inform application of the frame change (for WM_NCCALCSIZE).
		BOOL_CHECK(SetWindowPos(hWnd, NULL, rcClient.left, rcClient.top, RECTW(rcClient), RECTH(rcClient), SWP_FRAMECHANGED));

	}
	break;

	case WM_SETFOCUS:
	{
		SelectedTab->SetFocus();
	}
	break;

	case WM_ACTIVATE:
	{
		FWindowActive = LOWORD(wParam) != WA_INACTIVE;
		RedrawTabs();
	}
	break;

	case WM_NCPAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		DrawTabs(hdc);
		EndPaint(hWnd, &ps);
		
		lRet = 0;
		fCallDWP = true;
	}
	break;

	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* mmInfo = (MINMAXINFO*)lParam;
		mmInfo->ptMinTrackSize.x = (uint)Tabs.size()*TabMinWidth + TabAddBtnMarginLeft + TabAddBtnSizeX + TabMenuBtnMarginLeft + TabMenuBtnSizeX + 
									3 * SystemInfo::CaptionButton.cx + 2 * SystemInfo::BorderSize.cx;
		mmInfo->ptMinTrackSize.y = 200;
		
	}
	break;

	// Handle the non-client size message.
	case WM_NCCALCSIZE:
	{
		bool clientAreaNeedsCalculating = static_cast<bool>(wParam);
		if (clientAreaNeedsCalculating) //(wParam == TRUE)
		{
			// Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
			NCCALCSIZE_PARAMS* pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

			pncsp->rgrc[0].left   = pncsp->rgrc[0].left + (SystemInfo::BorderSize.cx + SystemInfo::Padding);
			pncsp->rgrc[0].top    = pncsp->rgrc[0].top + 0;
			pncsp->rgrc[0].right  = pncsp->rgrc[0].right - (SystemInfo::BorderSize.cx + SystemInfo::Padding);
			pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - (SystemInfo::BorderSize.cy + SystemInfo::Padding);

			lRet = 0;
			fCallDWP = false; // No need to pass the message on to the DefWindowProc.

		}
	}
	break;

	// Handle hit testing in the NCA if not handled by DwmDefWindowProc.
	case WM_NCHITTEST:
	{
		if (lRet == 0)
		{
			lRet = HitTestNCA(hWnd, wParam, lParam);
		}

		if (lRet != HTNOWHERE)
			fCallDWP = false;
	}
	break;

	HANDLE_MSG(hWnd, WM_COMMAND, OnWMCommand);
	HANDLE_MSG(hWnd, WM_NCLBUTTONDOWN, OnMouseNCLButtonDown);
	HANDLE_MSG(hWnd, WM_NCLBUTTONUP, OnMouseNCLButtonUp);
	HANDLE_MSG(hWnd, WM_NCMOUSEMOVE, OnNCMouseMove);

	} // switch

	*pfCallDWP = fCallDWP;

	return lRet;
}

// ((fn)((hwnd), (int)(LOWORD(wParam)),(HWND)(lParam),(UINT)HIWORD(wParam)), 0L)
LRESULT TabsManager::OnWMCommand(HWND hWnd, int commandID, HWND controlHandle, UINT code)
{
	if (0 == SelectedTab->WMCommand(commandID, controlHandle, code)) return 0; // returns 0 if SelectedTab->WMCommand() processed this message

	// Parse the menu selections:
	switch (commandID)
	{
	//case IDM_ABOUT:
	//	DialogBox(FInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
	//	break;

	//case IDM_ZOOMIN:
	//{
	//	long long enu = -1, denu = -1;
	//	BOOL res = (BOOL)SendMessage(g_hWndEdit, EM_GETZOOM, (WPARAM)&enu, (LPARAM)&denu);
	//	//if (!res) DisplayError(hWnd, _T("EM_SETZOOM 5-1"));
	//	//Edit_SetZoom(g_hWndEdit, 5, 1);
	//	res = PostMessage(g_hWndEdit, EM_SETZOOM, 50, 100);
	//	//if (!res) DisplayError(hWnd, _T("EM_SETZOOM 5-1"));
	//	break;
	//}
	//case IDM_ZOOMOUT:
	//{
	//	long long enu = -1, denu = -1;
	//	BOOL res = (BOOL)SendMessage(g_hWndEdit, EM_GETZOOM, (WPARAM)&enu, (LPARAM)&denu);
	//	res = Edit_SetZoom(g_hWndEdit, 200, 100);
	//	//if (!res) DisplayError(hWnd, _T("EM_SETZOOM 1-5"));
	//	break;
	//}

	case IDM_NEWFILE: AddTab(true); break;

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
		MENUITEMINFO getItemInfo{};
		getItemInfo.cbSize = sizeof(MENUITEMINFO);
		getItemInfo.fMask = MIIM_DATA;

		BOOL_CHECK(GetMenuItemInfo(FMainMenu, commandID, FALSE, &getItemInfo));

		auto itemData = (MAINMENUITEM*)getItemInfo.dwItemData;
		SelectedTab->LoadFile(itemData->itemText);
	}
	break;

	case IDS_CLEARECENT:
	{
		auto access = Registry::DesiredAccess::Write | Registry::DesiredAccess::Read;
		auto key = Registry::CurrentUser->Open(FStrMap[IDS_FULLREGKEYRECENT], access); // "SOFTWARE\\NotepadCo\\Notefall\\Recent"
		key->Delete();
	}
	break;

	case IDM_SETTINGS:
	{ 
		SettingsDlg settings;
		settings.FontData = EditBoxFont;
		settings.FontColor = EditBoxTextColor;
		if (settings.ShowModal(FMainWnd))
		{
			EditBoxFont = settings.FontData;
			EditBoxTextColor = settings.FontColor;
			DeleteObject(EditBoxHFont);
			EditBoxHFont = CreateFontIndirect(&EditBoxFont);
			SelectedTab->SetFont(EditBoxHFont);
		}
		

		//HWND hDlg = CreateDialogParam(FInstance, MAKEINTRESOURCE(IDD_DIALOGSETTINGS), FMainWnd, SettingsDlgWndProcStatic, (LPARAM)this);
		//ShowWindow(hDlg, SW_SHOW);

		/*
		CHOOSEFONT font{};
		//ZeroMemory(&font, sizeof(font));
		font.lStructSize = sizeof(CHOOSEFONT);
		font.hwndOwner = hWnd;
		font.lpLogFont = &EditBoxFont;
		font.rgbColors = EditBoxTextColor;
		font.Flags = CF_SCREENFONTS | CF_EFFECTS;

		if (ChooseFont(&font) == TRUE)
		{ 
			HFONT oldHFont = EditBoxHFont;
			EditBoxHFont = CreateFontIndirect(&EditBoxFont);

			SelectedTab->SetFont(EditBoxHFont);

		//	DeleteObject(oldHFont);	
		//	
		//	HDC hdc = GetDC(FEditWnd);
		//	HFONT hfont = CreateFontIndirect(font.lpLogFont);
		//	SendMessage(g_hWndEdit, WM_SETFONT, (WPARAM)hfont, TRUE);
		//	Globals::fcFontColor = font.rgbColors;

		//	COLORREF prevColor = SetTextColor(hdc, font.rgbColors);
		//	if (prevColor == CLR_INVALID)
		//		MessageBox(hWnd, _T("this is text"), _T("this is caption"), MB_OK);
		//	
		//	//HFONT hfontPrev = SelectObject(hdc, hfont);
		//	//rgbCurrent = cf.rgbColors;
		//	//rgbPrev = SetTextColor(hdc, rgbCurrent);
		}*/
	}
	break;

	default:
		return DefWindowProc(hWnd, WM_COMMAND, MAKEWPARAM(commandID, code), (LPARAM)controlHandle);
	}

	return 0;
}

LRESULT CALLBACK TabsManager::MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// get a pointer to the mouse hook struct. 
	PMOUSEHOOKSTRUCT mhs = (PMOUSEHOOKSTRUCT)lParam;

	Application* app;
	app = (Application*)GetWindowLongPtr(mhs->hwnd, GWLP_USERDATA);
	if (app == nullptr)
		return S_OK; // TODO can we return without calling CallNextHookEx ?

	if (nCode == HC_ACTION)
	{
		// intercept messages for left button down and up
		switch (wParam)
		{
		case WM_NCLBUTTONDOWN:
		{
			// if the message is from your window and the hit test indicates title bar 
			if (mhs->wHitTestCode == HTCAPTION)
			{
				// then indicate non-client left button down 
				app->GetTabsManager().FNCLBtnDownFlag = true;

				//  there is no problem with this message 
				// so you don't have to do anything else 
			}
		}
		break;

		case WM_NCLBUTTONUP:
			// you will get this message if you double-click on the title bar 
			// reset the status 
			app->GetTabsManager().FNCLBtnDownFlag = false;
			break;

		case WM_LBUTTONUP:
		{
			// if the message is from your window and non-client left button is down
			if (app->GetTabsManager().FNCLBtnDownFlag == true)
			{
				// then post WM_NCLBUTTONUP message directly into your window message pump 
				// Note: I'm hardcoding HTCAPTION because the left button was down, and while it is down,
				// the mouse does not move in respect to the window, but it does in respect to the screen,
				// so the mouse should still be over the caption bar of your window when you release the button.
				PostMessage(mhs->hwnd, WM_NCLBUTTONUP, HTCAPTION, MAKELONG(mhs->pt.x, mhs->pt.y));

				// reset non-client left button down 
				app->GetTabsManager().FNCLBtnDownFlag = false;
			}
		}
		break;

		default:
			break;
		}
	}
	// let the messages through to the next hook 
	return CallNextHookEx(app->GetTabsManager().FMouseHook, nCode, wParam, lParam);
}


