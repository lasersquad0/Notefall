#pragma once

#include "framework.h"
#include "Common.h"
#include "TabContent.h"

class TabsManager;

class ITab
{
public:
	virtual ~ITab() {}
	POINT TabXY{};
	POINT IconXY{};
	POINT CloseBtnXY{};
	RECT TextRect{};

	virtual bool IsVisible() = 0;
	virtual void SetVisible(bool visible) = 0;
	virtual bool IsModified() = 0;
	virtual void SetModified(bool modified) = 0;
	virtual bool OpenFile() = 0;
	virtual bool LoadFile(const string_t& fileName) = 0;
	virtual bool SaveFile() = 0;
	virtual bool SaveFileAs() = 0;
	virtual void Find(FindReplaceAction whatToDo) = 0;
	virtual void Replace(FindReplaceAction whatToDo) = 0;
	virtual string_t& GetTitle() = 0;
	virtual void SetTitle(const string_t& title) = 0;
	virtual string_t GetFileName() = 0;
	virtual void Resize() = 0;
	virtual void CalcTabRect(uint tabIndex, bool hasCloseBtn) = 0;
	virtual bool IsOverCloseBtn(POINT ptMouse) = 0;
	virtual LRESULT WMCommand(int commandID, HWND controlHandle, UINT code) = 0;
	virtual bool NeedDrawTabIcon() = 0;
};


class Tab: public ITab
{
private:
	TabsManager* FParent;
	string_t FTitle;
	bool FDrawTabIcon = true;
public:
	TabContent FContent;

public:
	Tab(TabsManager* parent, const string_t& title) : FParent(parent), FContent(parent, this), FTitle(title) {}
	
	void CalcTabRect(uint tabIndex, bool hasCloseBtn) override;
	bool IsOverCloseBtn(POINT ptMouse) override;
	bool NeedDrawTabIcon() override { return FDrawTabIcon; }

	string_t& GetTitle() override { return FTitle; }
	void SetTitle(const string_t& title) override;
	bool IsVisible() override { return FContent.IsVisible(); }
	void SetVisible(bool visible) override { FContent.SetVisible(visible); }
	bool IsModified() override { return FContent.IsModified(); }
	void SetModified(bool modified) override { FContent.SetModified(modified); }
	bool OpenFile() override { return FContent.OpenFile(); }
	bool LoadFile(const string_t& fileName) override { return FContent.LoadFile(fileName); }
	bool SaveFile() override { return FContent.SaveFile(); }
	bool SaveFileAs() override { return FContent.SaveFileAs(); }
	string_t GetFileName() override { return FContent.GetFileName(); }
	void Find(FindReplaceAction whatToDo) override { FContent.Find(whatToDo); }
	void Replace(FindReplaceAction whatToDo) override { FContent.Replace(whatToDo); }
	void Resize() override { FContent.Resize(); }
	LRESULT WMCommand(int commandID, HWND controlHandle, UINT code) override;

};


class ITabManager
{
public:
	virtual ITab* AddTab(string_t title, bool makeSelected = true) = 0;
	virtual bool DeleteTab(uint index) = 0;
	virtual bool DeleteTab(ITab* tab) = 0;
	virtual bool SelectTab(uint index) = 0;
	virtual ITab* GetSelTab() = 0;
	virtual bool HasModifiedTabs() = 0;
	virtual bool SaveAll() = 0;

};

