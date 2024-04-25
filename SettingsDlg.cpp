#include "SettingsDlg.h"
#include "TabsManager.h"

#define COMBO_TEXT_BUF_SIZE 100

bool SettingsDlg::ShowModal(HWND parent)
{
	int retCode = (int)DialogBoxParam(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_DIALOGSETTINGS), parent, DlgProcStatic, (LPARAM)this);
	assert(retCode == 1 || retCode == 2);
	
	return retCode == 1; // this is OK btn clicked
}

string_t SettingsDlg::CharsetToString(uint charset) 
{
	for (size_t i = 0; i < sizeof(FCharsetNames) / sizeof(FCharsetNames[0]); i++)
		if (FCharsetNames[i].code == charset) 
			return FCharsetNames[i].name;
	
	return _T("UNKNOWN CHARSET");
}

string_t SettingsDlg::ColourToString(COLORREF colour)
{
	size_t count = sizeof(FColours) / sizeof(FColours[0]);
	for (size_t i = 0; i < count; i++)
		if (FColours[i].FColour == colour)
			return FColours[i].FColourName;

	return _T("OUT OF INDEX COLOUR");
}

bool SettingsDlg::UpdateData(bool validateAndSave)
{
	if (validateAndSave)
	{
		FontData.lfWeight    = IsDlgButtonChecked(FDlgWnd, IDC_CHECKBOLD) ? FW_BOLD : FW_NORMAL;
		FontData.lfItalic    = (BYTE)IsDlgButtonChecked(FDlgWnd, IDC_CHECKITALIC);
		FontData.lfUnderline = (BYTE)IsDlgButtonChecked(FDlgWnd, IDC_CHECKUNDERLINE);
		FontData.lfStrikeOut = (BYTE)IsDlgButtonChecked(FDlgWnd, IDC_CHECKSTRIKEOUT);

		ComboBox_GetText(FFontNameCombo, FontData.lfFaceName, LF_FACESIZE);
		
		int curSel = ComboBox_GetCurSel(FFontColourCombo);
		FontColor = (COLORREF)ComboBox_GetItemData(FFontColourCombo, curSel);
		
		curSel = ComboBox_GetCurSel(FFontCharsetCombo);
		FontData.lfCharSet = (BYTE)ComboBox_GetItemData(FFontCharsetCombo, curSel);

		curSel = ComboBox_GetCurSel(FFontSizeCombo);
		if (curSel == CB_ERR) // get font size from edit part of combo box
		{
			TCHAR buf[COMBO_TEXT_BUF_SIZE]{ 0 };
			ComboBox_GetText(FFontSizeCombo, buf, COMBO_TEXT_BUF_SIZE);
			FontData.lfHeight = _ttol(buf);
		}
		else
		{
			FontData.lfHeight = (UINT)ComboBox_GetItemData(FFontSizeCombo, curSel);
		}

		//HFONT oldFont = (HFONT)SendMessage(FBrownFox, WM_GETFONT, 0, 0);
		HFONT hFont = CreateFontIndirect(&FontData);
		SendMessage(FBrownFox, WM_SETFONT, (WPARAM)hFont, TRUE);
		BOOL_CHECK(DeleteObject(hFont));
	}
	else
	{
		BOOL_CHECK(CheckDlgButton(FDlgWnd, IDC_CHECKBOLD, FontData.lfWeight > FW_NORMAL));
		BOOL_CHECK(CheckDlgButton(FDlgWnd, IDC_CHECKITALIC, FontData.lfItalic));
		BOOL_CHECK(CheckDlgButton(FDlgWnd, IDC_CHECKUNDERLINE, FontData.lfUnderline));
		BOOL_CHECK(CheckDlgButton(FDlgWnd, IDC_CHECKSTRIKEOUT, FontData.lfStrikeOut));

		int curSel = ComboBox_SelectString(FFontSizeCombo, -1, to_string_t(FontData.lfHeight).c_str());
		if (curSel == CB_ERR)
		{
			ComboBox_SetText(FFontSizeCombo, to_string_t(FontData.lfHeight).c_str());
		}
		
		curSel = ComboBox_SelectString(FFontColourCombo, -1, ColourToString(FontColor).c_str());
		if (curSel == CB_ERR)
		{
			curSel = ComboBox_GetCount(FFontColourCombo) - 1;
			ComboBox_SetItemData(FFontColourCombo, curSel, FontColor);
			ComboBox_SetCurSel(FFontColourCombo, curSel); // selecting last item in case of custom colour
		}

		assert(CB_ERR != ComboBox_SelectString(FFontNameCombo, -1, FontData.lfFaceName));
		// CBN_SELCHANGE should be last statement here since it causes a call to UpdateData(true) 
		// that collects all data from combo boxes into LOGFONT structure
		SendMessage(FDlgWnd, WM_COMMAND, MAKEWPARAM(IDC_CMBFONTNAME, CBN_SELCHANGE), 0);

		//assert(CB_ERR != ComboBox_SelectString(FFontCharsetCombo, -1, CharsetToString(FontData.lfCharSet).c_str()));

	}
	
	return true;
}

int CALLBACK SettingsDlg::EnumFontsProcStatic(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
//int CALLBACK SettingsDlg::EnumFontsProc(ENUMLOGFONT* lpelf, NEWTEXTMETRIC* lpntm, DWORD FontType, LPARAM lParam)
{
	SettingsDlg* dlg = (SettingsDlg*)lParam;
	assert(dlg);
 
	return dlg->EnumFontsProc(lpelfe, lpntme, FontType);	
}

int CALLBACK SettingsDlg::EnumFontsProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType)
{
	UNREFERENCED_PARAMETER(lpntme);
	UNREFERENCED_PARAMETER(FontType);

	auto iter = FFonts.find(lpelfe->lfFaceName);
	if (iter != FFonts.end())
	{
		FFonts.at(lpelfe->lfFaceName).push_back(*lpelfe);

	}
	else
	{
		std::vector<LOGFONT> charsets;
		charsets.push_back(*lpelfe);
		FFonts.emplace(lpelfe->lfFaceName, charsets);
	}
	
	return 1;
}

INT_PTR CALLBACK SettingsDlg::DlgProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam); // here is pointer to TabsManager instance
	}

	SettingsDlg* content{};
	content = (SettingsDlg*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (content == nullptr)
		return FALSE; // notify upper proc that we do NOT processed this message

	return content->DlgProc(hDlg, message, wParam, lParam);
}

INT_PTR CALLBACK SettingsDlg::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
		HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
		HANDLE_MSG(hDlg, WM_DRAWITEM, OnComboDrawItem);
		HANDLE_MSG(hDlg, WM_MEASUREITEM, OnFontMeasureItem);

	case WM_CTLCOLORSTATIC:
	{
		if ((HWND)lParam == FBrownFox)
		{
			HDC hdcStatic = (HDC)wParam;
			SetTextColor(hdcStatic, FontColor);
			SetBkColor(hdcStatic, TabsManager::WindowColor);
			return (INT_PTR)TabsManager::FontPreviewBrush;
		}
	}
	break;

	} // switch

	return (INT_PTR)FALSE;

}

void SettingsDlg::OnFontMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
	UNREFERENCED_PARAMETER(hwnd);

	if(lpMeasureItem->CtlID == IDC_CMBFONTNAME)	lpMeasureItem->itemHeight += 4;
}

void SettingsDlg::OnComboDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
	UNREFERENCED_PARAMETER(hwnd);

	if (lpDrawItem->CtlID == IDC_CMBCOLOUR)
		OnColourComboDrawItem(hwnd, lpDrawItem);

	if (lpDrawItem->CtlID == IDC_CMBFONTNAME)
		OnFontComboDrawItem(hwnd, lpDrawItem);

}

void SettingsDlg::OnFontComboDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
	UNREFERENCED_PARAMETER(hwnd);

	HDC DC = lpDrawItem->hDC;
	UINT itemIndex = lpDrawItem->itemID;
	UINT itemState = lpDrawItem->itemState;
	RECT itemRect = lpDrawItem->rcItem;

	COLORREF crNormal = TabsManager::ComboItemNormalColor; //GetSysColor(COLOR_WINDOW);
	COLORREF crSelected = TabsManager::ComboItemHighlightColor; //GetSysColor(COLOR_HIGHLIGHT);
	COLORREF crText = TabsManager::ComboItemTextColor; //GetSysColor(COLOR_WINDOWTEXT);

	//HBRUSH brFrameBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	HBRUSH brSelected = CreateSolidBrush(crSelected);
	HBRUSH brNormal = CreateSolidBrush(crNormal);

	if (itemState & ODS_SELECTED)
	{
		SetTextColor(DC, (0x00FFFFFF & ~(crText)));
		SetBkColor(DC, crSelected);
		FillRect(DC, &itemRect, brSelected);
	}
	else
	{
		SetTextColor(DC, crText);
		SetBkColor(DC, crNormal);
		FillRect(DC, &itemRect, brNormal);
	}

	if (itemState & ODS_FOCUS)
	{
		DrawFocusRect(DC, &itemRect);
	}

	LOGFONT lf{};
	TCHAR buf[MAX_PATH]{};
	ComboBox_GetLBText(FFontNameCombo, itemIndex, buf);
	int len = lstrlen(buf);
	StringCchCopy(lf.lfFaceName, len + 1, buf);

	TEXTMETRIC metric;
	GetTextMetrics(DC, &metric);

	lf.lfWeight = metric.tmWeight;// FW_NORMAL;
	lf.lfHeight = metric.tmHeight+2;// +(metric.tmHeight > 0) ? 1 : -1;
	lf.lfUnderline = metric.tmUnderlined;
	lf.lfStrikeOut = metric.tmStruckOut;
	lf.lfItalic = metric.tmItalic;
	lf.lfCharSet = metric.tmCharSet;
	lf.lfPitchAndFamily = metric.tmPitchAndFamily;

	// Create the font, and then return its handle.  
	HFONT font = CreateFontIndirect(&lf);
	HFONT oldFont = (HFONT)SelectObject(DC, font);

	SIZE sz;
	GetTextExtentPoint32(DC, lf.lfFaceName, len, &sz);

	int y = lpDrawItem->rcItem.top + (RECTH(lpDrawItem->rcItem) - sz.cy)/2;
	BOOL_CHECK(TextOut(DC, lpDrawItem->rcItem.left + 4, y, lf.lfFaceName, len));
	
	SelectObject(DC, oldFont);
	BOOL_CHECK(DeleteObject(font));
	BOOL_CHECK(DeleteObject(brSelected));
	BOOL_CHECK(DeleteObject(brNormal));
}

void SettingsDlg::OnColourComboDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
	UNREFERENCED_PARAMETER(hwnd);

	LOG_INFO("OnColourComboDrawItem() called");

	HDC dcContext = lpDrawItem->hDC;
	RECT rItemRect(lpDrawItem->rcItem);
	RECT rBlockRect(rItemRect);
	RECT rTextRect(rBlockRect);
	int iItem = lpDrawItem->itemID;
	int iState = lpDrawItem->itemState;

	COLORREF crColour   = 0;
	COLORREF crNormal = TabsManager::ComboItemNormalColor; //GetSysColor(COLOR_WINDOW);
	COLORREF crSelected = TabsManager::ComboItemHighlightColor; //GetSysColor(COLOR_HIGHLIGHT);
	COLORREF crText = TabsManager::ComboItemTextColor; //GetSysColor(COLOR_WINDOWTEXT);

	HBRUSH brFrameBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	HBRUSH brSelected = CreateSolidBrush(crSelected);
	HBRUSH brNormal = CreateSolidBrush(crNormal);

	if (iState & ODS_SELECTED)
	{
		SetTextColor(dcContext, (0x00FFFFFF & ~(crText)));
		SetBkColor(dcContext, crSelected);
		FillRect(dcContext, &rBlockRect, brSelected);
	}
	else
	{
		SetTextColor(dcContext, crText);
		SetBkColor(dcContext, crNormal);
		FillRect(dcContext, &rBlockRect, brNormal);
	}

	if (iState & ODS_FOCUS)
	{
		DrawFocusRect(dcContext, &rItemRect);
	}

	int iFourthWidth = RECTW(rBlockRect) / 3;
	// calculate text area.
	rTextRect.left += (iFourthWidth + 6);
	rTextRect.top += 2;

	// calculate colour block area.

	InflateRect(&rBlockRect, -2, -2);
	rBlockRect.right = iFourthWidth;

	string_t strColour;
	// draw colour text and block.
	if (iItem != -1)
	{
		strColour = FColours[iItem].FColourName;
		//ComboBox_GetLBText(FFontColourCombo, iItem, strColour.c_str());

		if (iState & ODS_DISABLED)
		{
			crColour = TabsManager::ComboItemDisabledColor; // GetSysColor(COLOR_INACTIVECAPTIONTEXT);
			SetTextColor(dcContext, crColour);
		}
		else
		{
			crColour = (COLORREF)ComboBox_GetItemData(FFontColourCombo, iItem);
		}

		SetBkMode(dcContext, TRANSPARENT);
		TextOut(dcContext, rTextRect.left, rTextRect.top, strColour.c_str(), (int)strColour.size());

		HBRUSH brColour = CreateSolidBrush(crColour);
		FillRect(dcContext, &rBlockRect, brColour);
		FrameRect(dcContext, &rBlockRect, brFrameBrush);
		BOOL_CHECK(DeleteObject(brColour));
	}

	//BOOL_CHECK(DeleteObject(brFrameBrush));
	BOOL_CHECK(DeleteObject(brSelected));
	BOOL_CHECK(DeleteObject(brNormal));
}

void SettingsDlg::OnCommand(HWND hDlg, int commandID, HWND hwndCtl, UINT code)
{
	UNREFERENCED_PARAMETER(hwndCtl);

	//if (FInitDialogFlag) return FALSE; // no command processing while Init Dialog

	if (code == CBN_SELCHANGE || code == BN_CLICKED)
	{
		if (commandID == IDC_CMBFONTNAME)
		{
			//(BYTE)ComboBox_GetItemData(FFontNameCombo, ComboBox_GetCurSel(FFontNameCombo));
			TCHAR buf[COMBO_TEXT_BUF_SIZE]{ 0 };
			ComboBox_GetText(FFontNameCombo, buf, COMBO_TEXT_BUF_SIZE);
			//int curSel = ComboBox_GetCurSel(FFontNameCombo);
			//ComboBox_GetLBText(FFontNameCombo, curSel, buf);
			std::vector<LOGFONT>& charsets = FFonts.at(buf);
			ComboBox_ResetContent(FFontCharsetCombo);

			for (const LOGFONT& font : charsets)
			{
				ComboBox_SetItemData(FFontCharsetCombo, ComboBox_AddString(FFontCharsetCombo, CharsetToString(font.lfCharSet).c_str()), font.lfCharSet);
			}

			// no assert here because some fonts do not contain all charsets
			int curSel = ComboBox_SelectString(FFontCharsetCombo, -1, CharsetToString(FontData.lfCharSet).c_str());
			if (curSel == CB_ERR)
				ComboBox_SetCurSel(FFontCharsetCombo, 0); // if no charset found in combobox - select first charset in the list
		}

		if (commandID == IDC_CMBCOLOUR)
		{
			int count = ComboBox_GetCount(FFontColourCombo);
			int curSel = ComboBox_GetCurSel(FFontColourCombo);
			assert(curSel != CB_ERR);

			if(curSel == count - 1) // last item selected this is 'Custom...' item
			{
				static COLORREF custColors[16];
				CHOOSECOLOR clr{};
				clr.lStructSize = sizeof(CHOOSECOLOR);
				clr.lpCustColors = custColors;
				clr.hwndOwner = FDlgWnd;
				clr.Flags = CC_RGBINIT | CC_FULLOPEN;
				clr.rgbResult = FontColor; 
				ComboBox_SetItemData(FFontColourCombo, curSel, FontColor);

				if(ChooseColor(&clr) == TRUE)
				{
					ComboBox_SetItemData(FFontColourCombo, curSel, clr.rgbResult);
				}
			}
		}

		UpdateData(true);
	}

	if (code == CBN_KILLFOCUS && commandID == IDC_CMBFONTSIZE)
	{
		UpdateData(true);
	}

	if (commandID == IDOK)
	{
		//BOOL_CHECK(GetDlgItemText(hDlg, IDC_COMBOFIND, FFindData.lpstrFindWhat, FFindData.wFindWhatLen));
	}

	if (commandID == IDOK || commandID == IDCANCEL)
	{
		EndDialog(hDlg, commandID);
		return;
	}

}

BOOL SettingsDlg::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hwndFocus);
	UNREFERENCED_PARAMETER(lParam);

	//FInitDialogFlag = true;
	FDlgWnd = hDlg;

	FFontNameCombo    = GetDlgItem(FDlgWnd, IDC_CMBFONTNAME);
	FFontColourCombo  = GetDlgItem(FDlgWnd, IDC_CMBCOLOUR);
	FFontCharsetCombo = GetDlgItem(FDlgWnd, IDC_CMBCHARSET);
	FFontSizeCombo    = GetDlgItem(FDlgWnd, IDC_CMBFONTSIZE);
	//FFontSample  = GetDlgItem(FDlgWnd, IDC_CMBF);
	FBrownFox = GetDlgItem(FDlgWnd, IDC_BROWNFOX);

	HDC dc = GetDC(FDlgWnd);
	LOGFONT lf{};
	lf.lfCharSet = DEFAULT_CHARSET;
	EnumFontFamiliesEx(dc, &lf, EnumFontsProcStatic, (LPARAM)this, 0);
	ReleaseDC(FDlgWnd, dc);

	SendMessage(FFontNameCombo, CB_INITSTORAGE, FFonts.size(), 100);

	LOG_INFO("OnInitDialog job START");

	for (auto font : FFonts)
	{
		ComboBox_AddString(FFontNameCombo, font.first.c_str());
	}

	LOG_INFO("OnInitDialog job FINISH");

	//int height = SendMessage(FFontNameCombo, CB_GETITEMHEIGHT, 0L, 0L);
	//height = SendMessage(FFontNameCombo, CB_GETITEMHEIGHT, 1L, 0L);
	//int vis = ComboBox_GetMinVisible(FFontNameCombo);

	//ComboBox_AddString(FFontNameCombo, (_T("_______") + std::to_wstring(FFonts.size())).c_str());
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("8")), 8);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("9")), 9);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("10")), 10);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("11")), 11);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("12")), 12);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("14")), 14);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("16")), 16);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("18")), 18);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("20")), 20);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("22")), 22);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("24")), 24);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("26")), 26);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("28")), 28);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("36")), 36);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("48")), 48);
	ComboBox_SetItemData(FFontSizeCombo, ComboBox_AddString(FFontSizeCombo, _T("72")), 72);
	
	for (size_t i = 0; i < sizeof(FColours)/sizeof(FColours[0]); i++)
	{
		ComboBox_SetItemData(FFontColourCombo, ComboBox_AddString(FFontColourCombo, FColours[i].FColourName.c_str()), FColours[i].FColour);
	}
	

	/*
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Don't care")), 0);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Thin")), 100);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Extra Light")), 200);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Light")), 300);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Normal")), 400);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Medium")), 500);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Semi Bold")), 600);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Bold")), 700);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Extra Bold")), 800);
			ComboBox_SetItemData(FFontWeightCombo, ComboBox_AddString(FFontWeightCombo, _T("Heavy")), 900);
			*/

	UpdateData(false);

	//FInitDialogFlag = false;

	return TRUE; // notify upper proc that we processed this message
}
