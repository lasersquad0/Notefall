
#include "Common.h"

bool isWordSeparator(TCHAR c)
{
	return _istpunct(c) || _istspace(c);
}

// assuming that dest buffer is large enough
void CharToWCHAR(WCHAR* dest, const char* src)
{
	// size_t len;
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dest, MAX_PATH);
	// mbstowcs_s(&len, dest, MAX_PATH, src, strlen(src));
	// if (len > 0u) dest[len] = '\0';
}


string_t CharReplace(string_t Str, const TCHAR SearchCh, const TCHAR ReplaceCh)
{
	for (size_t i = 0; i < Str.length(); i++)
	{
		if (Str[i] == SearchCh) Str[i] = ReplaceCh;
	}

	return Str;
}

string_t StringReplace(const string_t& Str, const string_t& SearchPattern, const string_t& ReplacePattern)
{
	if (SearchPattern.size() == 0 || Str.size() == 0) return string_t{};

	const TCHAR* temp1;
	TCHAR* temp2;
	string_t SearchStr(Str);
	string_t Result;
	SearchStr[0] = Str[0];

	temp1 = SearchStr.c_str();
	while (true)
	{
		temp2 = (TCHAR*)_tcsstr(temp1, SearchPattern.c_str()); // explicit cast requred by C++ Builder 6
		if (temp2 == nullptr)
		{
			Result += temp1;
			break;
		}
		(*temp2) = '\0';
		Result += temp1;
		Result += ReplacePattern;
		temp1 = temp2 + SearchPattern.length();
	}

	return Result;
}


#define KB 1024
#define MB (1024*1024)

string_t SizeToStr(uint size)
{
	if(size < 10 * KB) return toStringSep(size);
	if(size < 10 * MB) return toStringSep((size+ KB/2)/KB) + _T("Kb");
	return toStringSep((size + MB/2) / MB) + _T("Mb");
}

Gdiplus::Bitmap* LoadPNG(HINSTANCE hInstance, LPCTSTR szResName)
{
	HRSRC hrsrc = FindResource(hInstance, szResName, _T("PNG"));
	if (!hrsrc) return nullptr;
	HGLOBAL hgTemp = LoadResource(hInstance, hrsrc); 	// "ненастоящий" HGLOBAL - см. описание LoadResource
	if (!hgTemp) return nullptr;
	DWORD sz = SizeofResource(hInstance, hrsrc);
	LPVOID ptrRes = LockResource(hgTemp);
	if (!ptrRes) return nullptr;
	HGLOBAL hgRes = GlobalAlloc(GMEM_MOVEABLE, sz);
	if (!hgRes) return nullptr;
	LPVOID ptrMem = GlobalLock(hgRes);
	if (!ptrMem) { GlobalFree(hgRes); return nullptr; }
	
	CopyMemory(ptrMem, ptrRes, sz); // Копируем растровые данные
	GlobalUnlock(hgRes);
	IStream* pStream;
	HRESULT hr = CreateStreamOnHGlobal(hgRes, TRUE, &pStream); 	// TRUE означает освободить память при последнем Release
	if (FAILED(hr)) { GlobalFree(hgRes); return nullptr; }
	
	auto image = Gdiplus::Bitmap::FromStream(pStream);
	pStream->Release();
	GlobalFree(hgRes);
	return image;
}

Gdiplus::Bitmap* LoadPNG2(HINSTANCE hInstance, LPCTSTR szResName)
{
	if(HRSRC hrsrc = FindResource(hInstance, szResName, _T("PNG")))
		if(HGLOBAL hgTemp = LoadResource(hInstance, hrsrc)) 	// "ненастоящий" HGLOBAL - см. описание LoadResource
			if(DWORD sz = SizeofResource(hInstance, hrsrc))
				if(LPVOID ptrRes = LockResource(hgTemp))
					if (HGLOBAL hgRes = GlobalAlloc(GMEM_MOVEABLE, sz))
					{
						if (LPVOID ptrMem = GlobalLock(hgRes))
						{
							CopyMemory(ptrMem, ptrRes, sz); // Копируем растровые данные
							GlobalUnlock(hgRes);
							IStream* pStream;
							if (SUCCEEDED(CreateStreamOnHGlobal(hgRes, TRUE, &pStream))) 	// TRUE означает освободить память при последнем Release
							{
								auto image = Gdiplus::Bitmap::FromStream(pStream);
								pStream->Release();
								GlobalFree(hgRes);
								return image;
							}
						}
						GlobalFree(hgRes);
					}
	return nullptr;
}

// Retrieve and output the system error message for the last-error code
void DisplayError(HWND hWnd, DWORD lastError, const string_t& lpszFunction)
{
	LPCTSTR lpMsgBuf;
	LPVOID lpDisplayBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen(lpMsgBuf) +
		lstrlen((LPCTSTR)lpszFunction.c_str()) + 100) * sizeof(TCHAR));

	if (FAILED(StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error code %d as follows:\n%s"), lpszFunction.c_str(), lastError, lpMsgBuf)))
	{
		MessageBox(hWnd, _T("Unable to output error code."), _T("FATAL ERROR"), MB_OK | MB_ICONASTERISK);
	}

	//_tprintf(TEXT("ERROR: %s\n"), (LPCTSTR)lpDisplayBuf);
	MessageBox(hWnd, (LPCTSTR)lpDisplayBuf, TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);

	LocalFree((HLOCAL)lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

// Retrieve and output the system error message for the last-error code
void DisplayError(HWND hWnd, const string_t& lpszFunction)
{
	DisplayError(hWnd, GetLastError(), lpszFunction);
}

HFONT CreateFontPlus1(HDC dc)
{
	static LOGFONT lf{};  // structure for font information  

	TCHAR buf[MAX_PATH]{};
	GetTextFace(dc, MAX_PATH, buf);

	TEXTMETRIC metric;
	GetTextMetrics(dc, &metric);

	StringCchCopy(lf.lfFaceName, lstrlen(buf), buf);

	lf.lfWeight    = metric.tmWeight;// FW_NORMAL;
	lf.lfHeight    = metric.tmHeight + 1;
	lf.lfUnderline = metric.tmUnderlined;
	lf.lfStrikeOut = metric.tmStruckOut;
	lf.lfItalic    = metric.tmItalic;
	lf.lfCharSet   = metric.tmCharSet;
	lf.lfPitchAndFamily = metric.tmPitchAndFamily;

	// Create the font, and then return its handle.  
	return CreateFont(lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
					lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision,
					lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
}

LOGFONT CreateFontPlus11(HDC dc)
{
	static LOGFONT lf{};  // structure for font information  

	TCHAR buf[MAX_PATH]{};
	GetTextFace(dc, MAX_PATH, buf);

	TEXTMETRIC metric;
	GetTextMetrics(dc, &metric);

	StringCchCopy(lf.lfFaceName, lstrlen(buf), buf);

	lf.lfWeight = metric.tmWeight;// FW_NORMAL;
	lf.lfHeight = metric.tmHeight + 1;
	lf.lfUnderline = metric.tmUnderlined;
	lf.lfStrikeOut = metric.tmStruckOut;
	lf.lfItalic = metric.tmItalic;
	lf.lfCharSet = metric.tmCharSet;
	lf.lfPitchAndFamily = metric.tmPitchAndFamily;

	// Create the font, and then return its handle.  
	return lf;
}