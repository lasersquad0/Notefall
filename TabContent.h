#pragma once

#include "framework.h"
#include "Common.h"

#define FIND_TEXT_LEN 100

class TabsManager;
class ITab;

class TabContent
{
private:
    TabsManager* FTabsMan{};
    ITab* FParent{};
    HINSTANCE FInstance{};
    HWND FBaseWnd{};
    HWND FEditBoxWnd{};
    HWND FStatusBarWnd{};
    UINT FFindReplaceMsg{};
	std::map<UINT, string_t> FStrMap;
    string_t FCurrentFileName;
    FINDREPLACE FFindData{};
    TCHAR FFindWhat[FIND_TEXT_LEN]{};
    TCHAR FFReplaceWith[FIND_TEXT_LEN]{};
    WNDPROC FOldEditBoxWndProc;
    std::vector<string_t> FFindHistory;
    std::vector<string_t> FReplaceHistory;

    ATOM RegisterBaseWindowClass();
    void LoadStrings();
    BOOL InitStatusBar();
    void UpdateStatusBar();
    void UpdateRecent();
    void UpdateFindHistory();
    void UpdateReplaceHistory();
    HFONT CreateFontForEdit();
    void LoadFindHistory();
    void LoadReplaceHistory();

    void EditWndResize(HWND hWnd, UINT, int cx, int cy);
    RECT GetBaseWndRect();
    static LRESULT CALLBACK BaseWndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK BaseWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK EditBoxWndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK EditBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK FindDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK FindDlgWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK ReplaceDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK ReplaceDlgWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnWMCommand(HWND hWnd, int commandID, HWND controlHandle, UINT code);
    template< class TStr> void SearchFor(FindReplaceAction direction);

public:
	TabContent(TabsManager* tabsMan, ITab* parent);
    ~TabContent();

    //void Init();
    void SetVisible(bool visible);
    bool IsVisible() { return IsWindowVisible(FEditBoxWnd); }
    bool IsModified() { return Edit_GetModify(FEditBoxWnd); }
    void SetModified(bool modified) { Edit_SetModify(FEditBoxWnd, modified); }
    
    void Resize();
    void AddText(const string_t& text);
    //HWND GetEditWnd() { return FEditBoxWnd; }
    bool LoadFile(const string_t& fullFileName);
    bool OpenFile();
    bool SaveFile(const string_t& fullFileName);
    bool SaveFile();
    bool SaveFileAs();
    string_t GetFileName() { return FCurrentFileName; }
    void Find(FindReplaceAction whatToDo);
    void Replace(FindReplaceAction whatToDo);
    LRESULT WMCommand(int commandID, HWND controlHandle, UINT code);

};




#define EDITBOXTEXT _T("//   FUNCTION: OnInitStatusbarDialog(HWND, HWND, LPARAM)\r\n\
//\r\n\
//   PURPOSE: Process the WM_INITDIALOG message\r\n\
//\r\n\
BOOL OnInitStatusbarDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam)\r\n\
{\r\n\
    // Load and register IPAddress control class\r\n\
    INITCOMMONCONTROLSEX iccx;\r\n\
    iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);\r\n\
    iccx.dwICC = ICC_BAR_CLASSES;\r\n\
    if (!InitCommonControlsEx(&iccx))\r\n\
        return FALSE;\r\n\
    // Create the status bar control\r\n\
    RECT rc = { 0, 0, 0, 0 };\r\n\
    HWND hStatusbar = CreateWindowEx(0, STATUSCLASSNAME, 0,\r\n\
        SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,\r\n\
        rc.left, rc.top, rc.right, rc.bottom,\r\n\
        hWnd, (HMENU)IDC_STATUSBAR, g_hInst, 0);\r\n\
\r\n\
    // Store the statusbar control handle as the user data associated with \r\n\
    // the parent window so that it can be retrieved for later use.\r\n\
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)hStatusbar);\r\n\
\r\n\
    // Establish the number of partitions or 'parts' the status bar will \r\n\
    // have, their actual dimensions will be set in the parent window's \r\n\
    // WM_SIZE handler.\r\n\
    GetClientRect(hWnd, &rc);\r\n\
    int nHalf = rc.right / 2;\r\n\
    int nParts[4] = { nHalf, nHalf + nHalf / 3, nHalf + nHalf * 2 / 3, -1 };\r\n\
    SendMessage(hStatusbar, SB_SETPARTS, 4, (LPARAM)&nParts);\r\n\
\r\n\
    // Put some texts into each part of the status bar and setup each part\r\n\
    SendMessage(hStatusbar, SB_SETTEXT, 0, (LPARAM)L\"Status Bar: Part 1\");\r\n\
    SendMessage(hStatusbar, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)L\"Part 2\");\r\n\
    SendMessage(hStatusbar, SB_SETTEXT, 2 | SBT_POPOUT, (LPARAM)L\"Part 3\");\r\n\
    SendMessage(hStatusbar, SB_SETTEXT, 3 | SBT_POPOUT, (LPARAM)L\"Part 4\");\r\n\
\r\n\
    return TRUE;\r\n\
}\r\n\
")


