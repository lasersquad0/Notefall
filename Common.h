#pragma once

#include "framework.h"
 
enum class FindReplaceAction { OpeDialog, FindNext, FindNextBack };

typedef unsigned int uint;

#define MAX_RECENTS 9

#define RECTW(_rc) abs(_rc.right - _rc.left)
#define RECTH(_rc) abs(_rc.bottom - _rc.top)

#define APP_LOG_NAME "Notefall"
#define APP_LOG_FILENAME "Notefall.log"

#define LOG_INFO(_msg) LogEngine::GetFileLogger(APP_LOG_NAME, APP_LOG_FILENAME).Info(_msg)
#define LOG_ERROR(_msg) LogEngine::GetFileLogger(APP_LOG_NAME, APP_LOG_FILENAME).Error(_msg)

#define MAKE_LOG_TSTR(_msg) string_t(_T(__FILE__)).append(_T(" : ")).append(_msg)
#define MAKE_LOG_STR(_msg) std::string(__FILE__).append(" : ").append(__func__).append(" : ").append(std::to_string(__LINE__)).append(" - ").append(_msg)

#define HR_CHECK(_hr) {if(FAILED(_hr)) LOG_ERROR( MAKE_LOG_STR(std::system_category().message(_hr)));}
#define BOOL_CHECK(_res) {if(!(_res)) LOG_ERROR( MAKE_LOG_STR(std::system_category().message(GetLastError())));}
//#define HR_CHECK(_hr) {if(FAILED(_hr)) DisplayError(nullptr, MAKE_LOG_TSTR(_T("")));}

#define MAX_LDRSTR 200
#define LoadResString(_id_) BOOL_CHECK(LoadString(FInstance, _id_, buf, MAX_LDRSTR)); FStrMap[_id_] = buf

#define LoadResIcon(_id_) ((HICON)LoadImage(FInstance, MAKEINTRESOURCE(_id_), IMAGE_ICON, 0,0, LR_DEFAULTCOLOR | LR_LOADTRANSPARENT))

bool isWordSeparator(TCHAR c);

inline char mytoupper(char c)
{
    return (char)::toupper((int)c);
}

inline wint_t mytoupper(wchar_t w)
{ 
    return ::towupper((wint_t)w);
}

template <class CH>
struct ci_char_traits : public std::char_traits<CH>
{
    static bool eq(CH c1, CH c2) { return mytoupper(c1) == mytoupper(c2); }
    static bool ne(CH c1, CH c2) { return mytoupper(c1) != mytoupper(c2); }
    static bool lt(CH c1, CH c2) { return mytoupper(c1) <  mytoupper(c2); }
    
    static int compare(const CH* s1, const CH* s2, size_t n) 
    {
        while (n-- != 0) 
        {
            if (mytoupper(*s1) < mytoupper(*s2)) return -1;
            if (mytoupper(*s1) > mytoupper(*s2)) return 1;
            ++s1; ++s2;
        }
        return 0;
    }

    static const CH* find(const CH* s, size_t n, CH a) 
    {
        CH A = (CH)mytoupper(a);
        for (size_t i = 0; i < n; i++)
        {
            if (mytoupper(*s) == A) return s;
            s++;
        }

        return nullptr;
    }
};

typedef std::basic_string<char, ci_char_traits<char> >ci_astring;
typedef std::basic_string<wchar_t, ci_char_traits<wchar_t>> ci_wstring;

#ifdef UNICODE
typedef ci_wstring ci_string;
typedef std::wstring string_t;
typedef std::wstringstream stringstream_t;
#else
typedef ci_astring ci_string;
typedef std::string string_t;
typedef std::stringstream stringstream_t;
#endif

struct MyGroupSeparator : std::numpunct<TCHAR>
{
    TCHAR do_thousands_sep() const override { return _T(' '); } // разделитель тыс€ч
    std::string do_grouping() const override { return "\3"; } // группировка по 3
};

template<typename IntType>
string_t toStringSep(IntType v)
{
    stringstream_t ss;
    ss.imbue(std::locale(ss.getloc(), new MyGroupSeparator()));

    ss << v;  // printing to string stream with formating
    return ss.str();
}

string_t StringReplace(const string_t& Str, const string_t& SearchPattern, const string_t& ReplacePattern);
string_t CharReplace(string_t Str, const TCHAR SearchCh, const TCHAR ReplaceCh);
string_t SizeToStr(uint size);

Gdiplus::Bitmap* LoadPNG(HINSTANCE hInstance, LPCTSTR szResName);
// Retrieve and output the system error message for the last-error code
void DisplayError(HWND hWnd, const string_t& lpszFunction);

HFONT CreateFontPlus1(HDC dc);

LOGFONT CreateFontPlus11(HDC dc);
