#pragma once

#include "framework.h"
#include "SystemInfo.h"
#include "TabInfo.h"

typedef struct _MAINMENUITEM
{
	int itemID;
	int iconID;
	string_t itemText;
	string_t shortcutText;
	//HICON itemIcon;
} MAINMENUITEM;

class Application;

class TabsManager
{
private:
	Application* FParent;
	HINSTANCE FInstance{};
	HWND FMainWnd{};
	std::map<UINT, Gdiplus::Bitmap*> FImgMap;
	std::map<UINT, string_t> FStrMap;
	bool FNeedRedraw = false;
	bool FWindowActive = true;
	bool FNCLBtnDownFlag = false; // status of non-client left button down
	HHOOK FMouseHook{};
	HMENU FMenu;
	HMENU FMainMenu;
	//const SIZE captBtnSize{ 45, 29 }; // size of caption buttons: Min, Max, Close
	MAINMENUITEM MainMenuIDs[15] = { {IDM_NEWFILE, PNG_NEWFILE, _T(""), _T("")}, {IDM_OPENFILE, PNG_OPENFILE, _T(""), _T("")},
								  	 {IDM_OPENRECENT, PNG_RECENT, _T(""), _T("")}, {IDM_SAVEFILE, PNG_SAVEFILE, _T(""), _T("")},
									 {IDM_SAVEFILEAS, PNG_SAVEFILEAS, _T(""), _T("")},	{IDM_SAVEALL, PNG_SAVEALL, _T(""), _T("")},
									 {IDM_FIND, PNG_FIND, _T(""), _T("")}, {IDM_FINDNEXT, PNG_FINDNEXT, _T(""), _T("")},
									 {IDM_REPLACE, PNG_REPLACE, _T(""), _T("")}, {IDM_UNDO, PNG_UNDO, _T(""), _T("")},
									 {IDM_CUT, PNG_CUT, _T(""), _T("") }, {IDM_COPY, PNG_COPY, _T(""), _T("") },
									 {IDM_PASTE, PNG_PASTE, _T(""), _T("")}, {IDM_PRINTFILE, PNG_PRINT, _T(""), _T("")},
									 {IDM_SETTINGS, PNG_SETTINGS, _T(""), _T("")} };

	MAINMENUITEM RecentMenuIDs[10] = { {IDM_RECENT1, 0, _T(""), _T("")},  {IDM_RECENT2, 0, _T(""), _T("")},
									   {IDM_RECENT3, 0, _T(""), _T("")}, {IDM_RECENT4, 0, _T(""), _T("")},
									   {IDM_RECENT5, 0, _T(""), _T("")}, {IDM_RECENT6, 0, _T(""), _T("")},
									   {IDM_RECENT7, 0, _T(""), _T("")}, {IDM_RECENT8, 0, _T(""), _T("")},
									   {IDM_RECENT9, 0, _T(""), _T("")}, {IDS_CLEARECENT, 0, _T(""), _T("")} };

	void LoadImages();
	void InitTabsData();
	void InitMainMenu();
	HMENU GetRecentMenu();
	bool ValidTabIndex(uint index);
	uint GetMaxTabAreaWidth(uint wndWidth);
	void CalcTabRects(uint WindowWidth);
	void CalcTabRects();
	RECT GetAddButtonRect();
	RECT GetMenuBtnRect();
	static HFONT CreateFontForShortcut();
	LOGFONT CreateFontForShortcut1();
	void RedrawTabsIfOver(POINT ptMouse);
	bool IsOverAddButton(POINT ptMouse);
	bool IsOverMenuBtn(POINT ptMouse);
	//bool IsOverCloseButton(POINT ptMouse, LONG wndWidth);
	//bool IsOverMaxButton(POINT ptMouse, LONG wndWidth);
	//bool IsOverMinButton(POINT ptMouse, LONG wndWidth);
	LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnNCMouseMove(HWND hWnd, int x, int y, UINT HitTest);
	void OnMouseNCLButtonDown(HWND hWnd, BOOL /*fDoubleClick*/, int x, int y, UINT codeHitTest);
	void OnMouseNCLButtonUp(HWND hWnd, int x, int y, UINT codeHitTest);
	void OnDrawMenuItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
	void OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* lpMeasureItem);

	static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
public:
	std::vector<ITab*> Tabs;
	ITab* SelectedTab{};

	TabsManager(Application* parent);
	~TabsManager();
	void Init(HWND mainWnd);
	bool SelectTab(uint index);
	bool SelectTab(POINT ptMouse); 	// select tab under mouse click. ptMouseis in client coordinates
	ITab* AddTab(bool makeSelected = true);
	bool CloseTab(uint index);
	bool CloseAllTabs();
	void DrawTabs(HDC hdc);
	void RedrawTabs();
	void Resize(int cx, int cy);
	uint TabIndexUnderMouse(POINT ptMouse);
	void ShowMainMenu();

	LRESULT CaptionWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool* pfCallDWP);
	LRESULT OnWMCommand(HWND hWnd, int commandID, HWND controlHandle, UINT code);
	HWND GetMainWnd() { return FMainWnd; }
	HINSTANCE GetInstance() { return FInstance; }

public:
	// Caption tabs margins and sizes. These values calculated in InitTabsData function 
	inline static uint TabTotalHeight;    // from top of window frame to bottom of tab. 
	inline static uint TabHeight;         // only tab height (without tab margins and window frame borders)
	inline static uint TabWidth;          // depends on number of tabs and windows width. Recalculated when window width is changed
	inline static uint TabMinWidth;
	inline static uint TabMaxWidth;
	inline static uint TabWidth1of3;      // left part of tab
	inline static uint TabWidth2of3;      // center part
	inline static uint TabWidth3of3;      // right part of tab
	inline static uint TabAddBtnSizeX;    // X size of add button bitmap 
	inline static uint TabAddBtnSizeY;    // Y size of add button bitmap
	inline static uint TabCloseBtnSizeX;  // X size of tab close button bitmap
	inline static uint TabCloseBtnSizeY;  // Y size of tab close button bitmap

	inline static const uint TabMarginTop = 2;        // TODO make this margin zero if window is maximised. From top of window to top of tab minus YBorderSize 
	inline static const uint TabMarginLeft = 41;      // where tab begins from left size of window
	inline static const uint TabIconMarginTop = 8;   // from top of tab
	inline static const uint TabIconMarginLeft = 15;  // from left of tab
	inline static const uint TabIconSizeX = 16;
	inline static const uint TabIconSizeY = 16;
	inline static const uint TabTextMarginTop = 8;         // from top of tab
	inline static const uint TabTextMarginLeftInit = 15;//5;        // from right edge of tab icon
	inline static const uint TabTextMinWidth = 15;
	inline static const uint TabAddBtnMarginTop = 2;       // from top of tab
	inline static const uint TabAddBtnMarginLeft = 3;      // from right edge of tab icon 
	inline static const uint TabCloseBtnMarginTop = 10;    // from top of tab
	inline static const uint TabCloseBtnMarginRight = 17;  // from right size of tab  
	inline static const uint TabMenuBtnMarginTop = 4;        // TODO make this margin zero if window is maximised. From top of window to top of tab minus YBorderSize 
	inline static const uint TabMenuBtnMarginLeft = 4;      // where tab begins from left size of window
	inline static const uint TabMenuBtnSizeX = 41;
	inline static const uint TabMenuBtnSizeY = 32;

	inline static uint TabTextMarginLeft = TabTextMarginLeftInit;
	
	// Main menu margins
	inline static const uint  MainMenuIconMarginLeft = 10; // from left size of menu rect
	inline static const uint  MainMenuTextMarginLeft = 10; // margin after menu item icon
	inline static const uint  MainMenuTextMarginRight = 5; // leave some free space on the right side of menu rect
	inline static const uint  MainMenuTextMarginTop = 8;   // TODO may be not needed???
	inline static const uint  MainMenuItemHeight = 32;
	inline static const uint  MainMenuWidth = 150;

	//Main menu colors (from window theme)
	inline static COLORREF MainMenuTextColor;
	inline static COLORREF MainMenuItemHighlightColor;  // bkg color of selected menu item
	inline static COLORREF MainMenuBackgroundColor; // bkg color of entire popup menu
	inline static COLORREF MainMenuShortcutTextColor; // color of shortcut text looks a bit disabled
	inline static COLORREF MainMenuDisableTextColor;
	inline static HBRUSH MainMenuBackgroundBrush;
	inline static HFONT MainMenuShortcutFont = CreateFontForShortcut();

};

