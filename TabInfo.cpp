
#include "TabInfo.h"
#include "TabsManager.h"


void Tab::CalcTabRect(uint tabIndex, bool hasCloseBtn)
{
	FDrawTabIcon = TabsManager::TabWidth > TabsManager::TabCloseBtnSizeX + TabsManager::TabCloseBtnMarginRight + TabsManager::TabIconSizeX +
											   TabsManager::TabIconMarginLeft + TabsManager::TabTextMarginLeft;

	TabXY.x = SystemInfo::BorderSize.cx + TabsManager::TabMarginLeft + tabIndex * TabsManager::TabWidth;
	TabXY.y = SystemInfo::BorderSize.cy + TabsManager::TabMarginTop;

	IconXY.x = TabXY.x + TabsManager::TabIconMarginLeft;
	IconXY.y = TabXY.y + TabsManager::TabIconMarginTop;

	TextRect.left = TabsManager::TabTextMarginLeft + (FDrawTabIcon? (IconXY.x + TabsManager::TabIconSizeX) : TabXY.x);
	TextRect.top = TabXY.y + TabsManager::TabTextMarginTop;
	TextRect.right = TabXY.x + TabsManager::TabWidth - (hasCloseBtn ? (TabsManager::TabCloseBtnMarginRight + TabsManager::TabCloseBtnSizeX) : TabsManager::TabWidth3of3);
	TextRect.bottom = TabsManager::TabHeight;

	CloseBtnXY.x = TabXY.x + TabsManager::TabWidth - TabsManager::TabCloseBtnMarginRight - TabsManager::TabCloseBtnSizeX;
	CloseBtnXY.y = TabXY.y + TabsManager::TabCloseBtnMarginTop;

}

bool Tab::IsOverCloseBtn(POINT ptMouse)
{
	RECT rcCloseBtn{ CloseBtnXY.x, CloseBtnXY.y, CloseBtnXY.x + (LONG)TabsManager::TabCloseBtnSizeX, CloseBtnXY.y + (LONG)TabsManager::TabCloseBtnSizeY };
	return PtInRect(&rcCloseBtn, ptMouse);
}

void Tab::SetTitle(const string_t& title) 
{
	FTitle = title;
	FParent->RedrawTabs();
}

LRESULT Tab::WMCommand(int commandID, HWND controlHandle, UINT code)
{
	if (FContent.WMCommand(commandID, controlHandle, code) == 0) return 0; // return 0 if FContent processed WM_COMMAND message

	switch (commandID)
	{
	case IDM_SAVEFILE: SaveFile(); break;
	case IDM_OPENFILE: OpenFile(); break;
	case IDM_SAVEFILEAS: SaveFileAs(); break;
	
	case IDM_FIND: Find(FindReplaceAction::OpeDialog);	break;
	case IDM_FINDNEXT: Find(FindReplaceAction::FindNext); break;
	case IDM_FINDNEXTBACK: Find(FindReplaceAction::FindNextBack); break;
	case IDM_REPLACE: Replace(FindReplaceAction::OpeDialog);	break;

	default: return 1;
	}

	return 0;
}
