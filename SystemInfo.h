#pragma once

#include "framework.h"


class SystemInfo
{
private:
	static NONCLIENTMETRICS GetNonClientMetrics()
	{
		NONCLIENTMETRICS metrics{};
		metrics.cbSize = sizeof(metrics);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
		return metrics;
	}

	static LOGFONT GetCaptionFont()
	{
		return GetNonClientMetrics().lfCaptionFont;
	}

public:
	inline static SIZE BorderSize{ GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME) };
	inline static SIZE CaptionButton{ 45 /*GetSystemMetrics(SM_CXSIZE)*/, GetSystemMetrics(SM_CYSIZE) }; // TODO how to get real button width from Windows?
	
	//inline static int XBorder = GetSystemMetrics(SM_CXBORDER);
	//inline static int YBorder = GetSystemMetrics(SM_CYBORDER);
	inline static int Padding = GetSystemMetrics(SM_CXPADDEDBORDER);
//	inline static int XBorderSize = GetSystemMetrics(SM_CXSIZEFRAME);
//	inline static int YBorderSize = GetSystemMetrics(SM_CYSIZEFRAME);
	inline static int XCaptionButton = GetSystemMetrics(SM_CXSIZE);
	inline static int YCaptionButton = GetSystemMetrics(SM_CYSIZE);
	inline static int YCaption = GetSystemMetrics(SM_CYCAPTION);
	inline static int YMenu = GetSystemMetrics(SM_CYMENU);
	//inline static DWORD ActiveCaptionColor = GetSysColor(COLOR_HIGHLIGHT);
	//inline static DWORD InactiveCaptionColor = GetSysColor(COLOR_HIGHLIGHT); // COLOR_INACTIVECAPTION);
	//inline static DWORD WindowColor = GetSysColor(COLOR_WINDOW);
	//inline static DWORD CaptionTextColor = GetSysColor(COLOR_CAPTIONTEXT);
	//inline static DWORD InactiveCaptionTextColor = GetSysColor(COLOR_INACTIVECAPTIONTEXT);
	//inline static DWORD CaptionColor(bool active) { return active ? ActiveCaptionColor : InactiveCaptionColor; }

	inline static NONCLIENTMETRICS NCMetrics = GetNonClientMetrics();
	inline static LOGFONT CaptionFont = GetCaptionFont();
	inline static int SmallIconX = GetSystemMetrics(SM_CXSMICON);
	inline static int SmallIconY = GetSystemMetrics(SM_CYSMICON);

};