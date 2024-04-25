
#include "TabContent.h"
#include "TabsManager.h"
#include "TabInfo.h"
#include "Registry.h"

// standard edit box has only horizontal margins. This is value of vertical top margin
#define TOP_EDITBOX_MARGIN 10

void TabContent::LoadStrings()
{
	TCHAR buf[MAX_LDRSTR];

	LoadResString(IDS_APP_TITLE);
	LoadResString(IDC_NOTEFALL);
	LoadResString(IDS_COMPANY);
	LoadResString(IDS_REGKEYSOFT);
	LoadResString(IDS_REGKEYRECENT);
	LoadResString(IDS_REGKEYFIND);
	LoadResString(IDS_REGKEYREPLACE);
	LoadResString(IDS_OPENFILETITLE);
	LoadResString(IDS_SAVEASTITLE);
	LoadResString(IDS_STATUSMODIFIED);
	LoadResString(IDS_STATUSNOTMOD);
	LoadResString(IDS_RECENTMENU);
	LoadResString(IDS_STBARPART1);
	LoadResString(IDS_STBARPART2);
	LoadResString(IDS_STBARPART3);
	LoadResString(IDS_CLEARECENT);

	LoadString(FInstance, IDS_OPENSAVEFILTER, buf, MAX_LDRSTR); 
	FStrMap[IDS_OPENSAVEFILTER] = CharReplace(buf, _T('|'), _T('\0'));

	string_t tmpStr = FStrMap[IDS_REGKEYSOFT];
	tmpStr.append(_T("\\")).append(FStrMap[IDS_COMPANY]).append(_T("\\")).append(FStrMap[IDS_APP_TITLE]);
	FStrMap[IDS_FULLREGKEY] = tmpStr;

	tmpStr = FStrMap[IDS_FULLREGKEY];
	tmpStr.append(_T("\\")).append(FStrMap[IDS_REGKEYRECENT]);
	FStrMap[IDS_FULLREGKEYRECENT] = tmpStr;

	tmpStr = FStrMap[IDS_FULLREGKEY];
	tmpStr.append(_T("\\")).append(FStrMap[IDS_REGKEYFIND]);
	FStrMap[IDS_FULLREGKEYFIND] = tmpStr;

	tmpStr = FStrMap[IDS_FULLREGKEY];
	tmpStr.append(_T("\\")).append(FStrMap[IDS_REGKEYREPLACE]);
	FStrMap[IDS_FULLREGKEYREPLACE] = tmpStr;

}

#define BASEWND_ID _T("BaseWnd")

ATOM TabContent::RegisterBaseWindowClass()
{
	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = BaseWndProcStatic;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(nullptr); //FInstance
	//wcex.hIcon = LoadIcon(FInstance, MAKEINTRESOURCE(IDI_NOTEFALL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wcex.lpszMenuName = nullptr; //MAKEINTRESOURCE(IDC_NOTEFALL);
	wcex.lpszClassName = BASEWND_ID;
	//wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

TabContent::TabContent(TabsManager* tabsMan, ITab* parent)
{
	FTabsMan = tabsMan;
	FParent = parent;
	FInstance = FTabsMan->GetInstance();
	
	LoadStrings();
	//BOOL_CHECK(RegisterBaseWindowClass());

	HWND mainWnd = FTabsMan->GetMainWnd();

	RECT rcBase = GetBaseWndRect();

	FBaseWnd = CreateWindowEx(0, BASEWND_ID, _T("NotefallBaseWindowName"), WS_CHILD | WS_CLIPCHILDREN, // | WS_BORDER,
							rcBase.left, rcBase.top, RECTW(rcBase), RECTH(rcBase), mainWnd, nullptr, FInstance, nullptr);
	if (!FBaseWnd)
	{
		DisplayError(nullptr, _T("BaseWnd CreateWindowEx"));
		return;
	}

	SetWindowLongPtr(FBaseWnd, GWLP_USERDATA, (LONG_PTR)this);

	RECT rc2;
	BOOL_CHECK(GetClientRect(FBaseWnd, &rc2));

	FEditBoxWnd = CreateWindowEx(0, WC_EDIT, _T("NotefallContentWindowName"),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL, // | ES_AUTOVSCROLL, //| ES_AUTOHSCROLL | WS_CLIPCHILDREN, // | WS_BORDER,
		rc2.left , rc2.top, rc2.right, rc2.bottom, FBaseWnd, (HMENU)ID_EDITBOXWND, FInstance, nullptr);
	
	Edit_SetExtendedStyle(FEditBoxWnd, ES_EX_ZOOMABLE | ES_EX_ALLOWEOL_ALL, ES_EX_ZOOMABLE | ES_EX_ALLOWEOL_ALL);
	SendMessage(FEditBoxWnd, WM_SETFONT, (WPARAM)TabsManager::EditBoxHFont, (LPARAM)1);
	long long enu = 1, denu = 1;	
	Edit_GetZoom(FEditBoxWnd, &enu, &denu);

	SendMessage(FEditBoxWnd, EM_SETMARGINS, EC_LEFTMARGIN, 10);
	SendMessage(FEditBoxWnd, EM_SETLIMITTEXT, 0x7FFFFFFE, 0); // reset default size limits
	SendMessage(FEditBoxWnd, WM_SETTEXT, 0, (LPARAM)(EDITBOXTEXT));

	FOldEditBoxWndProc = (WNDPROC)SetWindowLongPtr(FEditBoxWnd, GWLP_WNDPROC, (LONG_PTR)&EditBoxWndProcStatic);
	if (!FOldEditBoxWndProc) 
		DisplayError(FEditBoxWnd, _T("SetWindowLongPtr"));
	SetWindowLongPtr(FEditBoxWnd, GWLP_USERDATA, (LONG_PTR)this);

	//register message for Find dialog
	FFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

	InitStatusBar();
	EditWndResize(FBaseWnd, 0, 0, 0);
	UpdateStatusBar();
}

TabContent::~TabContent()
{
	BOOL_CHECK(DestroyWindow(FStatusBarWnd));
	BOOL_CHECK(DestroyWindow(FEditBoxWnd));
	BOOL_CHECK(DestroyWindow(FBaseWnd));
}

BOOL TabContent::InitStatusBar()
{
	// Load and register IPAddress control class
	//INITCOMMONCONTROLSEX iccx;
	//iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	//iccx.dwICC = ICC_BAR_CLASSES;
	//if (!InitCommonControlsEx(&iccx))
	//	return FALSE;


	// Create the status bar control
	RECT rc = { 0, 0, 0, 0 };
	FStatusBarWnd = CreateWindowEx(0, STATUSCLASSNAME, 0, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
					rc.left, rc.top, rc.right, rc.bottom, FBaseWnd, (HMENU)IDC_STATUSBAR, FInstance, 0);

	// Establish the number of partitions or 'parts' the status bar will 
	// have, their actual dimensions will be set in the parent window's WM_SIZE handler.
	BOOL_CHECK(GetClientRect(FBaseWnd, &rc));
	int nHalf = rc.right / 2;
	int nParts[4] = { nHalf, nHalf + nHalf / 3, nHalf + nHalf * 2 / 3, -1 };
	SendMessage(FStatusBarWnd, SB_SETPARTS, 4, (LPARAM)&nParts);

	// Put some texts into each part of the status bar and setup each part
	SendMessage(FStatusBarWnd, SB_SETTEXT, 0, (LPARAM)FStrMap[IDS_STATUSNOTMOD].c_str());
	SendMessage(FStatusBarWnd, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)FStrMap[IDS_STBARPART1].c_str());
	SendMessage(FStatusBarWnd, SB_SETTEXT, 2 | SBT_POPOUT, (LPARAM)FStrMap[IDS_STBARPART2].c_str());
	SendMessage(FStatusBarWnd, SB_SETTEXT, 3 | SBT_POPOUT, (LPARAM)FStrMap[IDS_STBARPART3].c_str());

	//SetWindowLongPtr(FStatusBarWnd, GWLP_USERDATA, (LONG_PTR)nullptr);

	return TRUE;
}

void TabContent::UpdateStatusBar()
{
	UINT num = 0, denum = 0;
	Edit_GetZoom(FEditBoxWnd, &num, &denum);
	//UINT zoom = (UINT)(100.0 * (double)(num) / (double)denum);
	
	DWORD textLen = (DWORD)SendMessage(FEditBoxWnd, WM_GETTEXTLENGTH, 0, 0);
	//DWORD lines = (DWORD)SendMessage(FEditBoxWnd, EM_GETLINECOUNT, 0, 0);
	BOOL modified = (BOOL)SendMessage(FEditBoxWnd, EM_GETMODIFY, 0, 0);
	//DWORD currentLine = Edit_LineFromChar(FEditBoxWnd, -1);
	
	DWORD bsel, esel;
	SendMessage(FEditBoxWnd, EM_GETSEL, WPARAM(&bsel), LPARAM(&esel));
	assert(bsel <= esel);
	DWORD line = (DWORD)SendMessage(FEditBoxWnd, EM_LINEFROMCHAR, WPARAM(bsel), 0);
	DWORD firstChar = (DWORD)SendMessage(FEditBoxWnd, EM_LINEINDEX, WPARAM(line), 0);
	DWORD charIndex = bsel;
	DWORD col = charIndex - firstChar;

	assert(charIndex >= firstChar);

	const DWORD BUF_SZ = 100;
	TCHAR buf[BUF_SZ];
	StringCchPrintf(buf, BUF_SZ, (_T("Zoom: ") + SizeToStr(num) + _T("/") + SizeToStr(denum)).c_str());
	SendMessage(FStatusBarWnd, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)buf);

	StringCchPrintf(buf, BUF_SZ, (_T("LN:") + toStringSep(line) + _T(" CL:") + toStringSep(col) + _T(" CH:") + toStringSep(charIndex)).c_str());
	SendMessage(FStatusBarWnd, SB_SETTEXT, 2 | SBT_POPOUT, (LPARAM)buf);

	StringCchPrintf(buf, BUF_SZ, (_T("SZ: ") + SizeToStr(textLen) + _T(" characters")).c_str());
	SendMessage(FStatusBarWnd, SB_SETTEXT, 3 | SBT_POPOUT, (LPARAM)buf);
	
	
	if (modified)
		SendMessage(FStatusBarWnd, SB_SETTEXT, 0 | SBT_POPOUT, (LPARAM)FStrMap[IDS_STATUSMODIFIED].c_str()); //_T("Status: Modified"));
	else
		SendMessage(FStatusBarWnd, SB_SETTEXT, 0 | SBT_POPOUT, (LPARAM)FStrMap[IDS_STATUSNOTMOD].c_str()); // _T("Status: Not modified"));
}

void TabContent::AddText(const string_t& text)
{
	SendMessage(FEditBoxWnd, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}

RECT TabContent::GetBaseWndRect()
{
	RECT rc;
	BOOL_CHECK(GetClientRect(FTabsMan->GetMainWnd(), &rc));
	rc.top += TabsManager::TabTotalHeight;
	return rc;
}

void TabContent::SetVisible(bool visible)
{
	if (visible)
		ShowWindow(FBaseWnd, SW_SHOW), ::SetFocus(FEditBoxWnd);
	else
		ShowWindow(FBaseWnd, SW_HIDE);
}

void TabContent::SetFont(HFONT font)
{
	SendMessage(FEditBoxWnd, WM_SETFONT, (WPARAM)font, TRUE);

}

void TabContent::Resize()
{
	RECT rc = GetBaseWndRect();
	BOOL_CHECK(MoveWindow(FBaseWnd, rc.left, rc.top, RECTW(rc), RECTH(rc), true));

	//PostMessage(FBaseWnd, WM_SIZE, SIZE_RESTORED, MAKELONG(RECTW(rc), RECTH(rc)));
}

void TabContent::EditWndResize(HWND hWnd, UINT, int cx, int cy)
{
	HDC DC = GetDC(FStatusBarWnd);

	SIZE sz2{};
	string_t part2 = FStrMap[IDS_STBARPART2];
	GetTextExtentPoint32(DC, part2.c_str(), (int)part2.size(), &sz2);

	SIZE sz3{};
	string_t part3 = FStrMap[IDS_STBARPART3];
	GetTextExtentPoint32(DC, part3.c_str(), (int)part3.size(), &sz3);

	BOOL_CHECK(ReleaseDC(FStatusBarWnd, DC));

	int nHalf = (cx - sz2.cx - sz3.cx) / 2;
	int nParts[] = { nHalf, 2 * nHalf, 2 * nHalf + sz2.cx, -1 };

	SendMessage(FStatusBarWnd, SB_SETPARTS, 4, (LPARAM)&nParts);

	RECT rcSB;
	BOOL_CHECK(GetClientRect(FStatusBarWnd, &rcSB));
	LONG sbHeight = RECTH(rcSB);
	BOOL_CHECK(MoveWindow(FStatusBarWnd, 0, cy - sbHeight, cx, cy, true));

	//MoveWindow(FEditbox,
	//	SystemInfo::XBorderSize, Globals::TabTotalHeight,
	//	cx - 2 * SystemInfo::XBorderSize, cy - 2 * SystemInfo::YBorderSize - Globals::TabTotalHeight - sbHeight, true);

	RECT rc;
	BOOL_CHECK(GetClientRect(hWnd, &rc));
	rc.top += TOP_EDITBOX_MARGIN; // making top margin since edit box has only horizontal ones
	rc.bottom -= sbHeight;
	BOOL_CHECK(MoveWindow(FEditBoxWnd, rc.left, rc.top, RECTW(rc), RECTH(rc), true));
}

LRESULT CALLBACK TabContent::BaseWndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TabContent* content;

	content = (TabContent*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (content == nullptr) 
		return DefWindowProc(hWnd, message, wParam, lParam);

	return content->BaseWndProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK TabContent::BaseWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == FFindReplaceMsg)
	{
		if (FFindData.Flags & FR_MATCHCASE)
			SearchFor<string_t>((FindReplaceAction)wParam);
		else
			SearchFor<ci_string>((FindReplaceAction)wParam);

		return 0;
	}

	switch (message)
	{
		HANDLE_MSG(hWnd, WM_SIZE, EditWndResize);

		HANDLE_MSG(hWnd, WM_COMMAND, OnWMCommand);

	case WM_CONTEXTMENU:
	{
		HMENU menu = LoadMenu(FInstance, MAKEINTRESOURCE(IDR_EDITBOXMENU));
		menu = GetSubMenu(menu, 0);
		WORD xPos = GET_X_LPARAM(lParam);
		WORD yPos = GET_Y_LPARAM(lParam);
		TrackPopupMenuEx(menu, TPM_HORPOSANIMATION, xPos, yPos, hWnd, 0);
		DestroyMenu(menu);
		break;
	}

	
	
/*	case WM_ERASEBKGND: // disable Erase Background
		return 0;
*/
	case WM_CTLCOLOREDIT:
	{
		//static HBRUSH hbrushEditBox = nullptr;
		HDC hdcEdit = (HDC)wParam;
		SetTextColor(hdcEdit, TabsManager::EditBoxTextColor); // Text color
		//SetBkMode(hdcEdit, OPAQUE); // EditBox Backround Mode (note: OPAQUE can be used)
		//SetBkColor(hdcEdit, GetSysColor(COLOR_BTNFACE)); //(LONG)hbrushEditBox); // Backround color for EditBox

		//DeleteObject(hbrushEditBox);
		//hbrushEditBox = CreateSolidBrush(COLOR_WINDOW+1);
		return (LRESULT)TabsManager::FontPreviewBrush; //GetSysColorBrush(COLOR_WINDOW); //hbrushEditBox; // Paint it
	}
	

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool TabContent::LoadFile(const string_t& fullFileName)
{
	HANDLE hf = CreateFile(fullFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hf == INVALID_HANDLE_VALUE)
	{
		DisplayError(FEditBoxWnd, _T("LoadFile.CreateFile"));
		return false;
	}

	DWORD fileSize = GetFileSize(hf, nullptr); // assuming we do not open files larger than 4G

	const DWORD MAX_BUFSIZE = 100 * 1024 * 1024; //100M

	SendMessage(FEditBoxWnd, WM_SETTEXT, 0, 0); // clear text in Edit box?
	SendMessage(FEditBoxWnd, WM_SETREDRAW, FALSE, 0);

	if (fileSize > MAX_BUFSIZE) // if file is big, read it by chunks
	{
		std::string ReadBuffer;
		ReadBuffer.resize(MAX_BUFSIZE + 1);

		DWORD ActualRead = 0;
		do
		{
			if (FALSE == ReadFile(hf, ReadBuffer.data(), MAX_BUFSIZE, &ActualRead, nullptr))
			{
				SendMessage(FEditBoxWnd, WM_SETREDRAW, TRUE, 0);
				DisplayError(FEditBoxWnd, _T("LoadFile.ReadFile"));
				CloseHandle(hf);
				return false;
			}

			ReadBuffer[ActualRead] = '\0';

			SendMessageA(FEditBoxWnd, EM_REPLACESEL, FALSE, (LPARAM)ReadBuffer.data());

		} while (ActualRead == MAX_BUFSIZE);

		BOOL_CHECK(CloseHandle(hf));
	}
	else // smaller files are read by one ReadFile call
	{
		std::string ReadBuffer;
		ReadBuffer.resize(fileSize + 1);
		DWORD ActualRead = 0;

		if (FALSE == ReadFile(hf, ReadBuffer.data(), fileSize, &ActualRead, nullptr))
		{
			SendMessage(FEditBoxWnd, WM_SETREDRAW, TRUE, 0);
			DisplayError(FEditBoxWnd, _T("LoadFile.ReadFile"));
			BOOL_CHECK(CloseHandle(hf));
			return false;
		}

		ReadBuffer[ActualRead] = '\0';

		BOOL_CHECK(SetWindowTextA(FEditBoxWnd, ReadBuffer.data()));

		BOOL_CHECK(CloseHandle(hf));
	}

	SendMessage(FEditBoxWnd, EM_SETSEL, 0, 0); // move to the begin of the text
	Edit_ScrollCaret(FEditBoxWnd);

	SendMessage(FEditBoxWnd, WM_SETREDRAW, TRUE, 0);
	BOOL_CHECK(RedrawWindow(FEditBoxWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN));

	Edit_SetModify(FEditBoxWnd, FALSE);
	UpdateStatusBar();
	FParent->SetTitle(PathFindFileName(fullFileName.c_str()));

	FCurrentFileName = fullFileName;
	UpdateRecent();

	return true;
}

bool TabContent::OpenFile()
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];    // buffer for selected file name (with dir)
	TCHAR szFileTitle[MAX_PATH];    // buffer for selectede file name without dir

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = FEditBoxWnd;
	ofn.lpstrTitle = FStrMap[IDS_OPENFILETITLE].c_str(); // _T("Open file");

	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';  // Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = FStrMap[IDS_OPENSAVEFILTER].c_str(); // _T("All (*.*)|*.*|Text (*.txt)|*.TXT|");
	ofn.nFilterIndex = 1;

	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = nullptr;
	ofn.lpstrDefExt = nullptr;

	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		return LoadFile(ofn.lpstrFile);
	}

	return false;
}

bool TabContent::SaveFile()
{
	if (FCurrentFileName.empty())
		return SaveFileAs();
	else
		return SaveFile(FCurrentFileName);
}

bool TabContent::SaveFile(const string_t& fullFileName)
{
	HANDLE hf = CreateFile(fullFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hf == INVALID_HANDLE_VALUE)
	{
		DisplayError(FEditBoxWnd, FStrMap[IDS_SAVEASTITLE]); // _T("Save File As"));
		return false;
	}

	DWORD len = (DWORD)SendMessageA(FEditBoxWnd, WM_GETTEXTLENGTH, 0, 0); //get text in ASCII coding here
	std::string str;
	str.resize(len + 1);

	BOOL_CHECK(GetWindowTextA(FEditBoxWnd, str.data(), (int)str.size()));

	DWORD writeCnt = 0;
	if (WriteFile(hf, str.data(), len, &writeCnt, nullptr) == FALSE)
	{
		DisplayError(FEditBoxWnd, FStrMap[IDS_SAVEASTITLE]); // _T("Save File As"));
		BOOL_CHECK(CloseHandle(hf));
		return false;
	}

	BOOL_CHECK(CloseHandle(hf));

	FCurrentFileName = fullFileName;
	Edit_SetModify(FEditBoxWnd, FALSE);
	FParent->SetTitle(PathFindFileName(fullFileName.c_str()));
	UpdateStatusBar();
	UpdateRecent();

	return true;
}

bool TabContent::SaveFileAs()
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];    // buffer for selected file name (with dir)
	TCHAR szFileTitle[MAX_PATH];    // buffer for selected file name without dir

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = FEditBoxWnd;
	ofn.lpstrTitle = FStrMap[IDS_SAVEASTITLE].c_str(); // _T("Save File As");

	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';  // Set lpstrFile[0] to '\0' so that GetSaveFileName does not use the contents of szFile to initialize itself.
	ofn.nMaxFile = sizeof(szFile);

	ofn.lpstrFilter = FStrMap[IDS_OPENSAVEFILTER].c_str(); // _T("All (*.*)\0*.*\0Text (*.txt)\0*.txt\0"); // last \0 is added automatically by compiler
	ofn.nFilterIndex = 1;

	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = nullptr;
	ofn.lpstrDefExt = _T("txt");

	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn) == TRUE)
	{
		return SaveFile(ofn.lpstrFile);
	}

	return false;
}


template< class TStr>
void TabContent::SearchFor(FindReplaceAction direction)
{
	DWORD len = (DWORD)SendMessage(FEditBoxWnd, WM_GETTEXTLENGTH, 0, 0);

	TStr str;  // this is wchar_t string if defined UNICODE
	str.resize(len + 1);

	BOOL_CHECK(GetWindowText(FEditBoxWnd, str.data(), len));

	size_t len2 = 0;
	if (FAILED(StringCchLength(FFindData.lpstrFindWhat, FIND_TEXT_LEN, &len2))) return;

	if (direction == FindReplaceAction::FindNext || direction == FindReplaceAction::OpeDialog) // do search in direct order
	{
		size_t index = 0;
		index = str.find(FFindData.lpstrFindWhat, FFindData.lCustData);

		if (FFindData.Flags & FR_WHOLEWORD)
			while (index != TStr::npos)
			{
				size_t last = index + len2;
				if ((index == 0 || (index > 0 && isWordSeparator(str[index - 1]))) && (last >= str.size() || (last < str.size() && isWordSeparator(str[last]))))
				{
					break;
				}

				index = str.find(FFindData.lpstrFindWhat, index + 1);
			}

		if (index == TStr::npos)
		{
			FFindData.lCustData = 0; //start search from begin
		}
		else
		{
			FFindData.lCustData = (LPARAM)(index + len2);
			SendMessage(FEditBoxWnd, EM_SETSEL, index, index + len2);
			Edit_ScrollCaret(FEditBoxWnd);
		}
	}
	else // do search in reverse order
	{
		size_t index = 0;
		index = str.rfind(FFindData.lpstrFindWhat, FFindData.lCustData - len2 - 1);

		if (FFindData.Flags & FR_WHOLEWORD)
			while (index != TStr::npos)
			{
				size_t last = index + len2;
				if ((index == 0 || (index > 0 && isWordSeparator(str[index - 1]))) && (last >= str.size() || (last < str.size() && isWordSeparator(str[last]))))
				{
					break;
				}
				index = str.rfind(FFindData.lpstrFindWhat, index - 1);
			}

		if (index == TStr::npos)
		{
			FFindData.lCustData = len; // in reverse search go to the end of text if not found
		}
		else
		{
			FFindData.lCustData = (LPARAM)(index + len2);
			SendMessage(FEditBoxWnd, EM_SETSEL, index, index + len2);
			Edit_ScrollCaret(FEditBoxWnd);
		}
	}
}


// tries to find selected text in edit box
// if 'next' is true then tries to find previously stored text instead of selected
void TabContent::Find(FindReplaceAction whatToDo)
{
	if (whatToDo == FindReplaceAction::FindNext || whatToDo == FindReplaceAction::FindNextBack) // user pressed F3 or Shift+F3
	{
		PostMessage(FBaseWnd, FFindReplaceMsg, (WPARAM)whatToDo, (LPARAM)&FFindData);
		return;
	}	

	// user pressed Ctrl+f -> open Find dialog box

	FFindData.lStructSize = sizeof(FFindData);
	FFindData.hwndOwner = (HWND)this; // store ref to TabContent class here
	FFindData.lpstrFindWhat = FFindWhat;
	FFindData.wFindWhatLen = FIND_TEXT_LEN;
	FFindData.lCustData = 0; // search from beginning

	DWORD beg, end;
	SendMessage(FEditBoxWnd, EM_GETSEL, (WPARAM)&beg, (LPARAM)&end);
	HLOCAL hbuf = (HLOCAL)SendMessage(FEditBoxWnd, EM_GETHANDLE, 0, 0);
	TCHAR* buf = (TCHAR*)LocalLock(hbuf);
	StringCchCopyN(FFindWhat, FIND_TEXT_LEN, buf + beg, end - beg);
	LocalUnlock(hbuf);

	DialogBoxParam(FInstance, MAKEINTRESOURCE(IDD_DIALOGFIND), FBaseWnd, FindDlgWndProcStatic, (LPARAM)&FFindData);

	return;
}

// tries to find selected text in edit box
// if 'next' is true then tries to find previously stored text instead of selected
void TabContent::Replace(FindReplaceAction whatToDo)
{
	if (whatToDo == FindReplaceAction::FindNext || whatToDo == FindReplaceAction::FindNextBack) // user pressed F3 or Shift+F3
	{
		//PostMessage(FBaseWnd, FFindReplaceMsg, (WPARAM)whatToDo, (LPARAM)&FFindData);
		return;
	}

	// user pressed Ctrl+F -> open Find dialog box
	FFindData.lStructSize = sizeof(FFindData);
	FFindData.hwndOwner = (HWND)this; // store ref to TabContent class here
	FFindData.lpstrFindWhat = FFindWhat;
	FFindData.wFindWhatLen = FIND_TEXT_LEN;
	FFindData.lpstrReplaceWith = FFReplaceWith;
	FFindData.wReplaceWithLen = FIND_TEXT_LEN;
	FFindData.lCustData = 0; // search from beginning

	DWORD beg, end;
	SendMessage(FEditBoxWnd, EM_GETSEL, (WPARAM)&beg, (LPARAM)&end);
	HLOCAL hbuf = (HLOCAL)SendMessage(FEditBoxWnd, EM_GETHANDLE, 0, 0);
	TCHAR* buf = (TCHAR*)LocalLock(hbuf);
	StringCchCopyN(FFindWhat, FIND_TEXT_LEN, buf + beg, end - beg);
	LocalUnlock(hbuf);

	HWND hDlg = CreateDialogParam(FInstance, MAKEINTRESOURCE(IDD_DIALOGREPLACE), FBaseWnd, ReplaceDlgWndProcStatic, (LPARAM)&FFindData);

	ShowWindow(hDlg, SW_SHOW);

	return;
}

INT_PTR CALLBACK TabContent::FindDlgWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		FINDREPLACE* fr = (FINDREPLACE*)lParam;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)fr->hwndOwner); // here is pointer to TabContent instance
	}

	TabContent* content{};
	content = (TabContent*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (content == nullptr)
		return FALSE; // notify upper proc that we do NOT processed this message

	return content->FindDlgWndProc(hDlg, message, wParam, lParam);
}

INT_PTR CALLBACK TabContent::FindDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
	{
		BOOL_CHECK(CheckDlgButton(hDlg, IDC_CHECKWORDS, FFindData.Flags & FR_WHOLEWORD));
		BOOL_CHECK(CheckDlgButton(hDlg, IDC_CHECKCASE, FFindData.Flags & FR_MATCHCASE));

		LoadFindHistory();
		HWND hCbox = GetDlgItem(hDlg, IDC_COMBOFIND);
		for (auto str : FFindHistory)
			ComboBox_AddString(hCbox, str.c_str());

		// fill dialog with search values from FINDREPLACE structure
		BOOL_CHECK(SetDlgItemText(hDlg, IDC_COMBOFIND, FFindData.lpstrFindWhat));
		SendMessage(hCbox, EM_SETLIMITTEXT, FFindData.wFindWhatLen, 0);

		return (INT_PTR)TRUE; // notify upper proc that we processed this message
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK)
		{
			BOOL_CHECK(GetDlgItemText(hDlg, IDC_COMBOFIND, FFindData.lpstrFindWhat, FFindData.wFindWhatLen));
			FFindData.Flags = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKWORDS)) ? FFindData.Flags | FR_WHOLEWORD : FFindData.Flags & (~FR_WHOLEWORD);
			FFindData.Flags = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKCASE)) ? FFindData.Flags | FR_MATCHCASE : FFindData.Flags & (~FR_MATCHCASE);

			UpdateFindHistory();

			PostMessage(FBaseWnd, FFindReplaceMsg, 0, (LPARAM)&FFindData);
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}

	} // switch

	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK TabContent::ReplaceDlgWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		FINDREPLACE* fr = (FINDREPLACE*)lParam;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)fr->hwndOwner); // here is pointer to TabContent instance
	}

	TabContent* content{};
	content = (TabContent*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (content == nullptr)
		return FALSE; // notify upper proc that we do NOT processed this message

	return content->ReplaceDlgWndProc(hDlg, message, wParam, lParam);
}

INT_PTR CALLBACK TabContent::ReplaceDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
	{
		BOOL_CHECK(CheckDlgButton(hDlg, IDC_CHECKWORDS, FFindData.Flags & FR_WHOLEWORD));
		BOOL_CHECK(CheckDlgButton(hDlg, IDC_CHECKCASE, FFindData.Flags & FR_MATCHCASE));

		LoadFindHistory();
		HWND hCbox = GetDlgItem(hDlg, IDC_COMBOFIND);
		for (auto str : FFindHistory)
			ComboBox_AddString(hCbox, str.c_str());

		// fill dialog with search values from FINDREPLACE structure
		BOOL_CHECK(SetDlgItemText(hDlg, IDC_COMBOFIND, FFindData.lpstrFindWhat));
		SendMessage(hCbox, EM_SETLIMITTEXT, FFindData.wFindWhatLen, 0);

		LoadReplaceHistory();
		hCbox = GetDlgItem(hDlg, IDC_COMBOREPLACE);
		for (auto str : FReplaceHistory)
			ComboBox_AddString(hCbox, str.c_str());

		BOOL_CHECK(SetDlgItemText(hDlg, IDC_COMBOREPLACE, FFindData.lpstrReplaceWith));
		SendMessage(hCbox, EM_SETLIMITTEXT, FFindData.wReplaceWithLen, 0);
			
		return (INT_PTR)TRUE; // notify upper proc that we processed this message
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK)
		{
			BOOL_CHECK(GetDlgItemText(hDlg, IDC_COMBOFIND, FFindData.lpstrFindWhat, FFindData.wFindWhatLen));
			BOOL_CHECK(GetDlgItemText(hDlg, IDC_COMBOREPLACE, FFindData.lpstrReplaceWith, FFindData.wReplaceWithLen));
			FFindData.Flags = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKWORDS)) ? FFindData.Flags | FR_WHOLEWORD : FFindData.Flags & (~FR_WHOLEWORD);
			FFindData.Flags = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECKCASE)) ? FFindData.Flags | FR_MATCHCASE : FFindData.Flags & (~FR_MATCHCASE);
			
			UpdateFindHistory();
			UpdateReplaceHistory();

			PostMessage(FBaseWnd, FFindReplaceMsg, 0, (LPARAM)&FFindData);
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}

	} // switch

	return (INT_PTR)FALSE;
}


LRESULT CALLBACK TabContent::EditBoxWndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TabContent* content;

	content = (TabContent*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (content == nullptr)
		return DefWindowProc(hWnd, message, wParam, lParam);

	return content->EditBoxWndProc(hWnd, message, wParam, lParam);

}

LRESULT CALLBACK TabContent::EditBoxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//if (msg == WM_CHAR && wParam == 6)
	//{
	//	OnFind(hwnd);
	//	return 1;
	//}

	switch (message)
	{
		HANDLE_MSG(hWnd, WM_COMMAND, OnWMCommand);

	case WM_CONTEXTMENU:
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_KEYUP:
	{
		UpdateStatusBar();
		return CallWindowProc(FOldEditBoxWndProc, hWnd, message, wParam, lParam);
	}


	default:
		return CallWindowProc(FOldEditBoxWndProc, hWnd, message, wParam, lParam);
	}

	//return 1;
}

LRESULT TabContent::OnWMCommand(HWND hWnd, int commandID, HWND controlHandle, UINT code)
{
	UNREFERENCED_PARAMETER(hWnd);

	return WMCommand(commandID, controlHandle, code);
}

LRESULT TabContent::WMCommand(int commandID, HWND controlHandle, UINT code)
{
	UNREFERENCED_PARAMETER(controlHandle);
	//UNREFERENCED_PARAMETER(code);

	switch (commandID)
	{
	case IDM_UNDO:
		// Send WM_UNDO only if there is something to be undone. 
		if (SendMessage(FEditBoxWnd, EM_CANUNDO, 0, 0))
			SendMessage(FEditBoxWnd, WM_UNDO, 0, 0);
		else
			MessageBox(FEditBoxWnd, _T("Nothing to undo."), _T("Undo"), MB_OK);
		break;
	case IDM_CUT:   PostMessage(FEditBoxWnd, WM_CUT, 0, 0); break;
	case IDM_COPY:  PostMessage(FEditBoxWnd, WM_COPY, 0, 0); break;
	case IDM_PASTE: PostMessage(FEditBoxWnd, WM_PASTE, 0, 0); break;
	case IDM_SELECTALL: PostMessage(FEditBoxWnd, EM_SETSEL, 0, (LPARAM)-1); break;
	case ID_EDITBOXWND: // notification from Editbox
		if (code == EN_UPDATE) UpdateStatusBar(); // updating status bar if text has changed
		//if (code == EN_CHANGE) UpdateStatusBar(); 
		break;
	default: return 1;
	}
	return 0;

}

void TabContent::UpdateRecent()
{
	auto access = Registry::DesiredAccess::Write | Registry::DesiredAccess::Read;

	Registry::RegistryKey_ptr key;
	if (!Registry::CurrentUser->Exists(FStrMap[IDS_FULLREGKEYRECENT]))
		key = Registry::CurrentUser->Create(FStrMap[IDS_FULLREGKEYRECENT], access); // "SOFTWARE\\NotepadCo\\Notefall\\Recent"
	else
		key = Registry::CurrentUser->Open(FStrMap[IDS_FULLREGKEYRECENT], access);
	  
	std::vector<string_t> recents;
	recents.push_back(FCurrentFileName);
	string_t fileName = FCurrentFileName;

	key->EnumerateValues([&key, &recents, fileName](const std::wstring& valueName, const uint) -> bool
		{
			string_t val = key->GetString(valueName);
			if (fileName != val)
				recents.push_back(val);
			return true;
		});

	for (size_t i = 0; i < MAX_RECENTS; i++)
	{
		if (i < recents.size())
			key->SetString(std::to_wstring(i), recents[i]);
		else
			break;
	}
}

void TabContent::UpdateFindHistory()
{
	auto access = Registry::DesiredAccess::Write | Registry::DesiredAccess::Read;

	Registry::RegistryKey_ptr key;
	if (!Registry::CurrentUser->Exists(FStrMap[IDS_FULLREGKEYFIND]))
		key = Registry::CurrentUser->Create(FStrMap[IDS_FULLREGKEYFIND], access); // "SOFTWARE\\NotepadCo\\Notefall\\Recent"
	else
		key = Registry::CurrentUser->Open(FStrMap[IDS_FULLREGKEYFIND], access);

	std::vector<string_t> findHistory;
	findHistory.push_back(FFindData.lpstrFindWhat);
	string_t findWhat = FFindData.lpstrFindWhat;

	key->EnumerateValues([&key, &findHistory, findWhat](const std::wstring& valueName, const uint) -> bool
		{
			string_t val = key->GetString(valueName);
			if (findWhat != val)
				findHistory.push_back(val);
			return true;
		});

	for (size_t i = 0; i < findHistory.size(); i++)
	{
		key->SetString(std::to_wstring(i), findHistory[i]);
	}
}

void TabContent::UpdateReplaceHistory()
{
	auto access = Registry::DesiredAccess::Write | Registry::DesiredAccess::Read;

	Registry::RegistryKey_ptr key;
	if (!Registry::CurrentUser->Exists(FStrMap[IDS_FULLREGKEYREPLACE]))
		key = Registry::CurrentUser->Create(FStrMap[IDS_FULLREGKEYREPLACE], access); // "SOFTWARE\\NotepadCo\\Notefall\\Recent"
	else
		key = Registry::CurrentUser->Open(FStrMap[IDS_FULLREGKEYREPLACE], access);

	std::vector<string_t> replaceHistory;
	replaceHistory.push_back(FFindData.lpstrReplaceWith);
	string_t replaceWhat = FFindData.lpstrReplaceWith;

	key->EnumerateValues([&key, &replaceHistory, replaceWhat](const std::wstring& valueName, const uint) -> bool
		{
			string_t val = key->GetString(valueName);
			if (replaceWhat != val)
				replaceHistory.push_back(val);
			return true;
		});

	for (size_t i = 0; i < replaceHistory.size(); i++)
	{
		key->SetString(std::to_wstring(i), replaceHistory[i]);
	}
}

void TabContent::LoadFindHistory()
{
	if (Registry::CurrentUser->Exists(FStrMap[IDS_FULLREGKEYFIND]))
	{
		auto key = Registry::CurrentUser->Open(FStrMap[IDS_FULLREGKEYFIND]);

		FFindHistory.clear();
		decltype(FFindHistory)* history = &FFindHistory;

		key->EnumerateValues([&key, history](const std::wstring& valueName, const uint) -> bool
			{
				string_t val = key->GetString(valueName);
				history->push_back(val);
				return true; // Return true to continue processing, false otherwise
			});
	}
}

void TabContent::LoadReplaceHistory()
{
	if (Registry::CurrentUser->Exists(FStrMap[IDS_FULLREGKEYREPLACE]))
	{
		auto key = Registry::CurrentUser->Open(FStrMap[IDS_FULLREGKEYREPLACE]);

		FReplaceHistory.clear();
		decltype(FReplaceHistory)* history = &FReplaceHistory;

		key->EnumerateValues([&key, history](const std::wstring& valueName, const uint) -> bool
			{
				string_t val = key->GetString(valueName);
				history->push_back(val);
				return true; // Return true to continue processing, false otherwise
			});
	}
}