#pragma once

#include "framework.h"
#include "Common.h"

typedef struct tagCHARSETNAME
{
	string_t name;
	uint code;
} CHARSETNAME;

struct Colour
{
	COLORREF FColour;
	string_t FColourName;
	Colour(COLORREF clrColour, const string_t& clrName) : FColour(clrColour), FColourName(clrName) {};
};

class SettingsDlg
{
private:
	const CHARSETNAME FCharsetNames[20] = 
	{
	{_T("ANSI"), 0},
	{_T("DEFAULT"), 1},
	{_T("SYMBOL"), 2},
	{_T("SHIFTJIS"), 128},
	{_T("HANGEUL"), 129},
	{_T("HANGUL"), 129},
	{_T("GB2312"), 134},
	{_T("CHINESEBIG5"), 136},
	{_T("OEM"), 255},
	{_T("JOHAB"), 130},
	{_T("HEBREW"), 177},
	{_T("ARABIC"), 178},
	{_T("GREEK"), 161},
	{_T("TURKISH"), 162},
	{_T("VIETNAMESE"), 163},
	{_T("THAI"), 222},
	{_T("EASTEUROPE"), 238},
	{_T("RUSSIAN"), 204},
	{_T("MAC"), 77},
	{_T("BALTIC"), 186}
	};

	Colour FColours[11] =
	{
		Colour(RGB(000, 000, 000), _T("Black")),
		Colour(RGB(255, 255, 255), _T("White")),
		Colour(RGB(127, 127, 127), _T("Grey")),
		Colour(RGB(255, 000, 000), _T("Red")),
		Colour(RGB(255, 127, 000), _T("Orange")),
		Colour(RGB(255, 255, 000), _T("Yellow")),
		Colour(RGB(000, 255, 000), _T("Green")),
		Colour(RGB(000, 255, 255), _T("Cyan")),
		Colour(RGB(000, 000, 255), _T("Blue")),
		Colour(RGB(255, 000, 255), _T("Fuchsia")),
		Colour(RGB(255, 255, 255), _T("Custom..."))
	};

	HWND FDlgWnd = nullptr;
	HWND FFontNameCombo = nullptr;
	HWND FFontSizeCombo = nullptr;
	HWND FFontColourCombo = nullptr;
	//HWND FFontWeightCombo = nullptr;
	HWND FFontCharsetCombo = nullptr;
	HWND FBrownFox = nullptr;
	bool FInitDialogFlag;
	std::map<string_t, std::vector<LOGFONT>> FFonts;
	string_t CharsetToString(uint charset);
	string_t ColourToString(COLORREF colour);
	BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnComboDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
	void OnColourComboDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
	void OnFontComboDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
	void OnFontMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem);

public:
	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGSETTINGS };
#endif

	LOGFONT FontData{};
	COLORREF FontColor{};
	bool ShowModal(HWND parent);
	bool UpdateData(bool validateAndSave);

	static INT_PTR CALLBACK DlgProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static int CALLBACK EnumFontsProcStatic(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam);
	int CALLBACK EnumFontsProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType);

};

