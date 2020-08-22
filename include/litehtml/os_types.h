#ifndef LH_OS_TYPES_H
#define LH_OS_TYPES_H
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace litehtml
{
#ifndef LITEHTML_UTF8
static int
_wcsicmp(const wchar_t *s1, const wchar_t *s2)
{
    while (std::toupper(*s1) == std::toupper(*s2++))
        if (*s1++ == '\0')
            return (0);
    return (std::toupper(*s1) - std::toupper(*--s2));
}

static int
_wcsnicmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
    if (n == 0)
        return (0);
    do {
        if (std::toupper(*s1) != std::toupper(*s2++)) {
            return (std::toupper(*s1) - std::toupper(*--s2));
        }
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return (0);
}

static void _itow(int value, wchar_t *buffer, size_t size, int radix)
{
    memset(buffer, 0, sizeof(wchar_t) * size);
    std::wostringstream wss;
    wss << std::setbase(radix) << value;
    auto s = wss.str();
    memcpy(buffer, s.c_str(), std::min(s.size(), size) * sizeof(wchar_t));
}

	typedef std::wstring		tstring;
	typedef wchar_t				tchar_t;
	typedef std::wstringstream	tstringstream;

	#define _t(quote)			L##quote

	#define t_strlen			wcslen
	#define t_strcmp			wcscmp
	#define t_strncmp			wcsncmp
	#define t_strcasecmp		_wcsicmp
	#define t_strncasecmp		_wcsnicmp
	#define t_strtol			wcstol
	//#define t_atoi				_wtoi
     #define t_atoi std::stoi

	#define t_strtod			wcstod
	//#define t_itoa(value, buffer, size, radix)	_itow_s(value, buffer, size, radix)
    #define t_itoa(value, buffer, size, radix)    _itow(value, buffer, size, radix)

	#define t_strstr			wcsstr
	#define t_tolower			towlower
	#define t_isdigit			iswdigit
	#define t_to_string(val)	std::to_wstring(val)

#else

	typedef std::string			tstring;
	typedef char				tchar_t;
	typedef std::stringstream	tstringstream;

	#define _t(quote)			quote

	#define t_strlen			strlen
	#define t_strcmp			strcmp
	#define t_strncmp			strncmp
	#define t_strcasecmp		_stricmp
	#define t_strncasecmp		_strnicmp
	#define t_strtol			strtol
	#define t_atoi				atoi
	#define t_strtod			strtod
	#define t_itoa(value, buffer, size, radix)	_itoa_s(value, buffer, size, radix)
	#define t_strstr			strstr
	#define t_tolower			tolower
	#define t_isdigit			isdigit
	#define t_to_string(val)	std::to_string(val)

#endif

typedef u_int64_t uint_ptr;
}

#endif  // LH_OS_TYPES_H
