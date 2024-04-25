// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once


#include "targetver.h"
//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>
#include <WinUser.h>
#include <shlwapi.h>
#include <dwmapi.h>
#include <gdiplus.h>
#include <uxtheme.h>
#include <vssym32.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <sstream>
#include <map>
#include <vector>

#include "resource.h"
#include "LogEngine.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

