#pragma once

#include "framework.h"
#include "Common.h"
#include "TabsManager.h"


class Application
{
private:
	HINSTANCE FInstance{};
	HWND FMainWnd{};
	string_t FTitleText;
	string_t FCmdLine;
	int FCmdShow;
	std::map<UINT, string_t> FStrMap;
	TabsManager FTabsMan;
	
	string_t& buildTitle(const string_t& fileName) { FTitleText = FStrMap[IDS_APP_TITLE] + _T(" - ") + fileName; return FTitleText;	}
	void LoadStrings();
	ATOM RegisterMainWindowClass();
	bool InitInstance();
	//LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam);
	//LRESULT CaptionWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool* pfCallDWP);
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	//static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);

public:
	Application(HINSTANCE hInstance, const string_t& CmdLine, int nCmdShow);
	~Application() { }

	int Run();
	HINSTANCE GetInstance() { return FInstance; }
	TabsManager& GetTabsManager() { return FTabsMan; }

	//bool IsActive() { return FWindowActive; }
	//void OnResize(HWND hWnd, UINT, int cx, int cy);
	//void OnNCMouseMove(HWND hWnd, int x, int y, UINT HitTest);
	//void OnMouseNCLButtonDown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	//void OnMouseNCLButtonUp(HWND hWnd, int x, int y, UINT codeHitTest);

};

