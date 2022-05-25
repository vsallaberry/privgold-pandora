/*
 * Copyright (C) 2021-2022 Vincent Sallaberry
 * unicode Utf8Iterator class, for vegastrike (GPL) / version PrivateerGold
 *   http://vegastrike.sourceforge.net/
 *   https://github.com/vsallaberry/privgold-pandora
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * -------------------------------------------------------------------------
 * class Utf8Iterator
 * usage:
 *  #include <stdio.h>
 *  #include "unicode.h"
 *  int main() {
 *    std::string     s = "Hello World hé hé ·ﬂ∏ ∏ = 3.14•••.";
 *    fprintf(stdout, "forLoopTest: '");
 *    for (Utf8Iterator it = Utf8Iterator::begin(s, 5), itend=it.end(); it != itend; ++it) {
 *      fprintf(stdout, "%lc", *it);
 *    }
 *    fprintf(stdout, "'\n\n");
 *    return 0;
 *  }
 */
#include "unicode.h"

#if defined(HAVE_CLOCALE)
# include <clocale>
#elif defined(HAVE_LOCALE_H)
# include <locale.h>
#endif

#if defined(HAVE_LOCALE)
# include <locale>
#endif

#if defined(HAVE_CWCTYPE)
# include <cwctype>
#elif defined(HAVE_WCTYPE_H)
# include <wctype.h>
#else
# include <ctype.h>
# define iswprint(c) isprint(c)
#endif

#include <iostream>

#if defined(_WIN32)
# include <windows.h>
# include <wincon.h>
# if (defined(__CYGWIN__) || defined(__MINGW32__))
#  include <fcntl.h>
#  include <io.h>
# else
#  include <direct.h>
# endif
#elif defined(__APPLE__)
# include <AvailabilityMacros.h>
#endif

#include "log.h"

#define UTF8_ITERATOR_DEBUG_L(_lvl, ...)	VS_DBG("unicode", _lvl, __VA_ARGS__)
#define UTF8_ITERATOR_DEBUG(...) 			UTF8_ITERATOR_DEBUG_L(logvs::DBG+3, __VA_ARGS__)

static bool s_unicode_initialized = false;

// -----------------------------
// utf8/utf32 simple conversions
// -----------------------------

wchar_t utf8_to_utf32(const char * utf8) {
    Utf8Iterator it = Utf8Iterator(utf8);
    return *it;
}

size_t utf32_to_utf8(char * dst, wchar_t utf32) {
    mbstate_t mbstate;
    size_t ret;
    memset(&mbstate, 0, sizeof(mbstate));
    if ((ret = wcrtomb(dst, utf32, &mbstate)) == (size_t)-1) {
        *dst = 0;
        ret = 0;
    } else {
        dst[ret] = 0;
    }
    return ret;
}

size_t utf8_to_wstr(wchar_t ** pws, const char * s, size_t ws_sz, size_t len) {
	if (!s || !pws || !ws_sz)
		return (size_t)-1;
	if (len == (size_t)-1)
		len = strlen(s);
	if (*pws == NULL && (*pws = (wchar_t*) malloc((len+1) * sizeof(**pws))) == NULL)
		return (size_t)-1;
	if (!len) {
		**pws = 0;
		return 0;
	}
	size_t wslen = 0;
	for (Utf8Iterator it = Utf8Iterator::begin(s, len); it != it.end() && wslen + 1 < ws_sz; ++it, ++wslen) {
		(*pws)[wslen] = *it;
	}
	(*pws)[wslen] = 0;
	return wslen;
}

size_t wstr_to_utf8(char ** ps, const wchar_t * ws, size_t u8_sz, size_t wslen) {
	if (!ws || !ps || !u8_sz)
		return (size_t)-1;
	if (wslen == (size_t)-1)
		wslen = wcslen(ws);
	bool ballocate = (*ps == NULL);
	if (ballocate) {
		u8_sz = (wslen+1) * 2;
		if ((*ps = (char *) malloc(u8_sz * sizeof(**ps))) == NULL)
			return (size_t)-1;
	}
	if (!wslen) {
		**ps = 0;
		return 0;
	}
	size_t u8len = 0;
	for (size_t iw = 0; iw < wslen && ws[iw]; ++iw) {
		if (u8len + MB_LEN_MAX + 1 >= u8_sz) {
			if (ballocate) {
				char * new_s = (char *) realloc(*ps, (u8_sz = (u8_sz*2 + MB_LEN_MAX + 1)) * sizeof(**ps));
				if (new_s == NULL)
					break ;
				*ps = new_s;
			} else {
				break ;
			}
		}
		size_t u8charlen = utf32_to_utf8(*ps + u8len, ws[iw]);
		if (u8charlen == 0) {
			u8charlen = 1;
			(*ps)[u8len] = (ws[iw] < 128 ? (ws[iw]&0xff) : '?');
		}
		u8len += u8charlen;
	}
	(*ps)[u8len] = 0;
	return u8len;
}

/* ************************************************************************* */

// -----------------------------
// Utf8Iterator class impl.
// -----------------------------
unsigned int Utf8Iterator::_default_flags = U8F_COMBINE;

// In order to handle old HFS deccomposed unicode form, we read a character in
// advance to check if it needs to be recomposed (NFD->NFC).

void Utf8Iterator::init() {
    memset(&_mbstate, 0, sizeof(_mbstate));
    _incr = _nextincr = _pos = 0;
    _nextvalue = 0;
    _flags = Utf8Iterator::_default_flags;
    next(); next(); // initializes _{next,}value and _{next,}incr.
}

Utf8Iterator & Utf8Iterator::operator++() {
    if (_pos < _size) {
        _pos += _incr;
        next();
    }
    return *this;
}

//#define UTF8_ITERATOR_DEBUG(...) (fprintf(stderr, __VA_ARGS__) + fputc('\n', stderr) * 0)
Utf8Iterator & Utf8Iterator::operator--() {
  if (_pos == 0)
      return *this;
  wchar_t save_next = _value;
  size_t save_incr = _incr, combine_incr;
  _nextvalue = 0; // to disable combination, on first pass.
  for (int i = 0; i < 2; ++i) {
    _incr = _nextincr = 0;
    UTF8_ITERATOR_DEBUG("Utf8Iterator::--() START POS %zu VAL %x", _pos, _str[_pos]&0xff);
    while (_pos > 0 && (_str[--_pos] & 0x80)) {
        UTF8_ITERATOR_DEBUG("Utf8Iterator::--() POS %zu VAL %x", _pos, _str[_pos]&0xff);
        if ((_str[_pos] & 0xC0) == 0xC0) {
            break ;
        }
    }
    UTF8_ITERATOR_DEBUG("Utf8Iterator::--() END --LOOP POS %zu VAL %x u32VAL %x NEXT %x", 
                        _pos, _str[_pos]&0xff, _value, _nextvalue);
    next();
    if (i == 0 && _pos > 0 && _nextvalue
    &&  static_cast<size_t>(_nextvalue) < ((size_t)(1<<16)) && unicode_combinable(_nextvalue)) {
        combine_incr = _nextincr;
        UTF8_ITERATOR_DEBUG("Utf8Iterator::--() CONTINUE");
    } else if (i == 1) {
        _value = unicode_combine(_nextvalue, _value);
        _incr = _nextincr + combine_incr;
        _nextvalue = save_next;
        _nextincr = save_incr;
    } else {
        _value = _nextvalue;
        _incr = _nextincr;
        _nextvalue = save_next;
        _nextincr = save_incr;
        break ;
    }
  }
  UTF8_ITERATOR_DEBUG("Utf8Iterator::--() RETURN POS %zu VAL %x u32VAL %x NEXT %x", 
                      _pos, _str[_pos]&0xff, _value, _nextvalue);
  return *this;
}

void Utf8Iterator::next() {
    size_t nextpos = _pos + _nextincr;
    _value = _nextvalue;
    _incr = _nextincr;
    if ((_nextincr = mbrtowc(&_nextvalue, _str + nextpos, _size - nextpos, &_mbstate)) == 0
    ||   _nextincr == (size_t)-1 || _nextincr == (size_t)-2) {
        if (nextpos < _size && _nextincr >= (size_t) -2 && (_flags & U8F_STOP_ON_ERROR) == 0) {
            // decoding error/incomplete utf8 sequence ; consider the character and go to next
            _nextvalue = _str[nextpos]&0xff;
            if (!iswprint(_nextvalue))
                _nextvalue = '?';
            _nextincr = 1;
            memset(&_mbstate, 0, sizeof(_mbstate));
        } else {
            // end of string
            _nextvalue = 0;
            _nextincr = _size - nextpos;
            //UTF8_ITERATOR_DEBUG("Utf8Iterator::next(): endOfString or decoding error\n");
        }
    } else {
        if ((_flags & U8F_COMBINE) != 0) {
            // combine utf8 characters if possible
            if (static_cast<size_t>(_value) < ((size_t)(1<<16))
            &&  _nextvalue && static_cast<size_t>(_nextvalue) < ((size_t)(1<<16)) && unicode_combinable(_nextvalue)) {
                wchar_t newvalue = unicode_combine(_value, _nextvalue);
                if (newvalue) {
                    _nextvalue = newvalue;
                    _nextincr = _incr + _nextincr;
                    next();
                }
            }
        }
    }
    //if (UTF8_ITERATOR_DEBUG("\n wc:%x incr:%zu pos:%zu '", _value, _incr, _pos)) { fputwc(_value, stderr); printf("'\n"); }
}


// -----------------------------
// locale initialization needed for mbrtowc/wcrtomb
// -----------------------------

void unicodeInitLocale(bool force) {
	if (s_unicode_initialized && !force)
		return ;
	s_unicode_initialized = true;
#if defined(HAVE_LOCALE_H) || defined(HAVE_CLOCALE)
    char loc[50] = {0, };
    char myloc[50] = {0, };
    //if (setlocale(LC_CTYPE, "UTF-8") == NULL && setlocale(LC_CTYPE, "utf8") == NULL) {
    	char * tmp = setlocale(LC_CTYPE, NULL); // current locale
    	VS_LOG("unicode", logvs::VERBOSE, "current LCCTYPE locale: %s", tmp != NULL ? tmp : "(null)");
        if (tmp == NULL || !strcmp(tmp, "C")) {
        	tmp = setlocale(LC_CTYPE, ""); // system default locale
        	VS_LOG("unicode", logvs::VERBOSE, "system default LCCTYPE locale: %s", tmp != NULL ? tmp : "(null)");
        }
        if (tmp && setlocale(LC_CTYPE, tmp) == NULL)
            tmp = setlocale(LC_CTYPE, "");
        if (tmp) {
            strncpy(myloc, tmp, sizeof(myloc)-1);
            myloc[sizeof(myloc) - 1] = 0;
            if ((tmp = strchr(myloc, '.'))) *tmp = 0;
        } else strcpy(myloc, "C");
        const char * langs[] = { myloc, "", ".", "en_US", "en_EN", "C", NULL };
        static const char * cods[] = { "UTF-8", "UTF8", "utf-8", "utf8", NULL };
        for (const char ** lang = langs; *lang; lang++) {
            for (const char ** cod = cods; *cod; cod++) {
                snprintf(loc, sizeof(loc), "%s%s%s", *lang, **lang && **lang != '.' ? "." : "", *cod);
                VS_LOG("unicode", logvs::VERBOSE, "trying LCCTYPE locale: %s", loc);
                if (setlocale(LC_CTYPE, loc) != NULL) {
                   #if defined(HAVE_LOCALE) && (!defined(__APPLE__) || (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_9 && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_9))
                    try {
                        std::cout.imbue(std::locale(std::locale::classic(), loc, std::locale::ctype));
                        std::cerr.imbue(std::locale(std::locale::classic(), loc, std::locale::ctype));
                    } catch (...) {
                        VS_LOG("unicode", logvs::WARN, "cannot set std::cout utf8 locale");
                    }
                   #else
                   VS_LOG("unicode", logvs::INFO, "no std::cout::imbue support on this system");
                   #endif
				   #if defined(_WIN32)
				   # if 0 && (defined(__CYGWIN__) || defined(__MINGW32__))
					_setmode(fileno(stdout), _O_U8TEXT);
					_setmode(fileno(stderr), _O_U8TEXT);
				   # elif defined(_WIN32) && !defined(_WINDOWS)
					SetConsoleOutputCP(CP_UTF8);
				   # endif
				   #endif
                    while (*(lang+1)) ++lang;
                    break ;
                }
            }
        }
    //}
    VS_LOG("unicode", logvs::NOTICE, "using locale %s for characters encoding",
           setlocale(LC_CTYPE, NULL));
#else
    VS_LOG("unicode", logvs::WARN, "warning: locale is not supported on this system");
#endif
}

// -------------------------------------------------
// mbrtowc/wcrtomb wrappers when libc does not utf8
// -------------------------------------------------

#if defined(VS_UNICODE_LIBC_WITHOUT_UTF8)
# if HAVE_CODECVT
#  include <codecvt>
#  include <locale>
#  undef VS_UNICODE_WCRTOMB_CXX11_CODECVT 	//codecvt_utf8 or wstring_convert
#  define VS_UNICODE_MBRTOWC_CXX11_CODECVT	//codecvt_utf8 or wstring_convert

typedef std::codecvt_utf8<char32_t> utf8_conv;
static utf8_conv s_utf8_conv;

size_t vs_wrap_wcrtomb(char * dst, wchar_t wc, mbstate_t * ctx) {
#ifdef VS_UNICODE_WCRTOMB_CXX11_CODECVT
	const utf8_conv::intern_type  * from_next;
	utf8_conv::extern_type  * to_next;

	utf8_conv::intern_type u32[2] = { static_cast<utf8_conv::intern_type>(wc), 0 };

	utf8_conv::result ret = s_utf8_conv.out(*ctx, &u32[0], &u32[1], from_next,
			                                &dst[0], &dst[MB_LEN_MAX], to_next);
	if (ret != utf8_conv::ok)
		return (size_t)-1;
    return (to_next - dst);
#else
    if (dst == NULL)
    	return 0;
    if (wc == 0) {
    	*dst = 0;
    	return 1;
    }
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf8_conv;
    std::string u8 = utf8_conv.to_bytes(static_cast<char32_t>(wc));
    UTF8_ITERATOR_DEBUG_L(logvs::DBG+4, "wcrtomb_cxx11: converted: %zu conv_len=%zu", utf8_conv.converted(), u8.length());
    if (utf8_conv.converted() == 0 || u8.length() == 0) return (size_t)-1;
    memcpy(dst, u8.c_str(), u8.length());
    dst[u8.length()] = 0;
    return(u8.length());
#endif
}

size_t vs_wrap_mbrtowc(wchar_t * pwc, const char * utf8_in, size_t len, mbstate_t * ctx) {
#ifdef VS_UNICODE_MBRTOWC_CXX11_CODECVT
	if (len == 0 || utf8_in == NULL || *utf8_in == 0) {
		*pwc = 0;
		return 0;
	}
	const utf8_conv::extern_type  * from_next;
	utf8_conv::intern_type  * to_next;
	utf8_conv::intern_type u32[2] = { 0, };

	utf8_conv::result ret = s_utf8_conv.in(*ctx, &utf8_in[0], &utf8_in[len], from_next,
			                               &u32[0], &u32[1], to_next);
	UTF8_ITERATOR_DEBUG_L(logvs::DBG+4, "mbrtowc_cxx11: result: %x(%02x) done?:%d ret:%zu u8=%02x%02x(%p)#%zu",
			              ret, *u32, to_next > &u32[0], from_next - utf8_in, *utf8_in&0xff,utf8_in[1]&0xff, utf8_in, len);
	if (ret == utf8_conv::error)
		return (size_t)-1;
	if (to_next == &u32[0]) // || ret == utf8_conv::partial)
		return (size_t)-2;
	*pwc = *u32;
	return (from_next - utf8_in);
#else
	if (len == 0 || utf8_in == NULL || *utf8_in == 0) {
		*pwc = 0;
		return 0;
	}
	size_t u8_sz = len;
	for (len = 0; utf8_in[len] && (unsigned char)(utf8_in[len]&0xff) >= 0xC0; ++len) /*loop*/;
	if (utf8_in[len])++len;
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf8_conv;
	std::u32string u32 = utf8_conv.from_bytes(utf8_in, utf8_in + len);
	size_t converted = utf8_conv.converted();
	if (converted == 0 || u32.length() == 0) return (size_t)-1;
	//if (converted < len) return (size_t)-2;
	*pwc=u32[0];
	return converted;
#endif
}
# elif 0 && (HAVE_SDL || HAVE_LIBICONV)
#  if HAVE_SDL
#   include <SDL_stdinc.h>
#   define iconv_open 	SDL_iconv_open
#   define iconv_close 	SDL_iconv_close
#   define iconv_t 		SDL_iconv_t
#   define iconv 		SDL_iconv
#   define iconv_inbuf_cast (const char**)
#  else
#   include <iconv.h>
#   define iconv_inbuf_cast
#  endif
static struct IConvCtx {
	iconv_t ic_to_u8, ic_to_u32;
	IConvCtx() {
		ic_to_u32 = iconv_open("UCS-4BE", "UTF-8");
		ic_to_u8 = iconv_open("UTF-8", "UCS-4BE");
	}
	~IConvCtx() {
		if (ic_to_u8) iconv_close(ic_to_u8);
		if (ic_to_u32) iconv_close(ic_to_u32);
	}
} s_iconv_ctx;
size_t vs_wrap_wcrtomb(char * dst, wchar_t wc, mbstate_t * ctx) {
	iconv_t ic = s_iconv_ctx.ic_to_u8;

    char swc[5], *swcp = &swc[sizeof(swc)-1];
    memset(swc, 0, sizeof(swc));
    for (*swcp = 0; wc > 0 && swcp > &swc[0]; wc >>= 8) { *(--swcp) = wc; }

    size_t in_sz = &swc[sizeof(swc)-1] - swcp;
    size_t out_sz = MB_LEN_MAX + 1;

    in_sz = 4;
    swcp = swc;

    dst[MB_LEN_MAX]=0;

    int ret = iconv(ic, iconv_inbuf_cast &swcp, &in_sz, &dst, &out_sz);

    size_t len = MB_LEN_MAX + 1 - out_sz;
    dst[len] = 0;
    return len;
}
size_t vs_wrap_mbrtowc(wchar_t * pwc, const char * utf8_in, size_t len, mbstate_t * ctx) {
	iconv_t ic = s_iconv_ctx.ic_to_u32;

	size_t in_sz = len;
	size_t out_sz = 4;
	char out[4];

	char * in = (char*)utf8_in;
	char * pout = out;
	int ret = iconv(ic, iconv_inbuf_cast &in, &len, &pout, &out_sz);

	if (out_sz != 0) {
		return (size_t)-1;
	}
	*pwc = 0;
	for (size_t i = 0; i < 4; ++i) {
		//fprintf(stderr, "%02x ", out[i]&0xff);
		*pwc = (*pwc << 8) + (out[i]&0xff);
	}
	UTF8_ITERATOR_DEBUG("iconv_mbrtowc: %x, return %zu\n", *pwc, in_sz-len);
	return in_sz - len;
}
# else // ! HAVE_CODECVT
#  warning "NO UTF8 SUPPORT (no libc, cxx11 or SDL/iconv utf8 handling)"
size_t vs_wrap_wcrtomb(char * dst, wchar_t wc, mbstate_t * ctx) {
    (void)ctx;
    *dst[0] = (char)wc;
    *dst[1] = 0;
    return 1;
}
size_t vs_wrap_mbrtowc(wchar_t * wc, const char * buf, size_t len, mbstate_t * ctx) {
    (void)ctx;
    if (wc && buf && len > 0) {
        *wc = (*buf & 0xff);
        return 1;
    } else {
        return (size_t)-1;
    }
}
# endif // ! HAVE_CODECVT
#endif // ! VS_UNICODE_LIBC_WITHOUT_UTF8

// END

/*--------------------------------------------------------------------------*/

/* ************************************************************************
 *        T E S T S
 * ************************************************************************ */

/*--------------------------------------------------------------------------*/
#ifdef VS_UTF8_ITERATOR_TESTS

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <stdarg.h>
#include <stdio.h>

#if defined(HAVE_CODECVT)
# include <codecvt>
#endif

static int cleanout(FILE*out) {
	clearerr(out); fflush(out); clearerr(out); return 0;
}

#define TEST_PRINT_OK 1
#define STR(x) #x
#define PTEST(hdr,cond,res,...) fprintf(stderr, hdr " %s [%s] - %d" "\n", __VA_ARGS__, res, STR(cond), __LINE__)
#define TESTV(_hdr, cond, ...) ((cond) && (++n_tests|1) ? (TEST_PRINT_OK?(cleanout(stderr)*0 + PTEST(_hdr,cond,"OK",__VA_ARGS__)*0):0) \
                                                        : (cleanout(stderr)*0 + PTEST(_hdr,cond,"FAILED",__VA_ARGS__)*0+1))
#define TEST(hdr,cond) TESTV("%s" hdr, cond, "")
#define HDR "       "

#define MAX(a,b) ((ssize_t) ((ssize_t) (b) > (ssize_t) (a) ? (b) : (a)))
#define MIN(a,b) ((ssize_t) ((ssize_t) (b) < (ssize_t) (a) ? (b) : (a)))
#define SIGN(a) ((ssize_t)(a) < 0 ? -1 : 1)

namespace utf8_iterator_tests {

static size_t n_tests = 0;

static size_t srange(const std::string & s, ssize_t pos) {
    return MAX(0,MIN(pos, s.size()));
}

static const char * pwc(wchar_t wc, FILE * out = stderr) { fprintf(out, "%lc", wc); return ""; }

template <typename charT>
static Utf8Iterator find_one(std::basic_ostream<charT> & out, Utf8Iterator::value_type wc, const Utf8Iterator & begin, const Utf8Iterator & end) {

    Utf8Iterator it = std::find(begin, end, wc), it2 = it;

    out << "  '" << pwc(wc); //<< (charT)wc
    //fputwc(wc, stderr);
    out << "'(" << (unsigned long)wc << "): found:" << (it2 != end) << " '" << pwc(*it2)//(charT)*it2
        << "', *(found-1):'" << /*(charT)*/(const char*) ((it2-1) == end ? pwc('$') : pwc(*(it2-1)))
        << "', found++:'" << /*(charT)*/pwc(*(it2++)) << "', ++found:'" << /*(charT)*/pwc(*(++it2)) << "'"
        << " found+2:'" << /*(charT)*/pwc(*(it2+2)) << "' found+=2:'" << /*(charT)*/pwc(*(it2+=2)) << "'('" << /*(charT)*/pwc(*it2) << "')"
        << std::endl;

    return it;
}

template <typename charT>
static unsigned int test_one_str(std::basic_ostream<charT> & out, const std::string & s, const Utf8Iterator & begin) { //, const Utf8Iterator _end) {
    unsigned int errs = 0;
    Utf8Iterator end=begin.end();

    out << "u8string: '";
    for (Utf8Iterator it = begin; it != end; ++it) {
        //out << (charT) *it;
        pwc(*it, stderr);
    }
    out << '\'' << std::endl;

    Utf8Iterator it = find_one(out, 'o', begin, end);
    if (!s.size()) {
        errs += TEST(HDR, it == end);
    } else {
        errs += TEST(HDR, it != end && it != begin.end() && *it == 'o');

        it = find_one(out, 'Z', begin, end);
        errs += TEST(HDR, it == end && it == begin.end() && *it == 0);

        it = find_one(out, L'à', begin, end);
        errs += TEST(HDR, it == end && it == begin.end() && *it == 0);

        it = find_one(out, L'é', begin, end);
        errs += TEST(HDR, it != end && it != begin.end() && *it == L'é');

        it = find_one(out, L'∏', begin, end);
        errs += TEST(HDR, it != end && it != begin.end() && *it == L'∏');

        it = find_one(out, 0x301, begin, end); // must not be here as it should be combined
        errs += TEST(HDR, it == end);
    }

    out << "u8Reversestring: '";
    for (it = end - 1; it != begin; --it) {
        pwc(*it, stderr);
    }
    out << '\'' << std::endl;

    out << std::endl;

    return errs;
}

#if defined(TEST_PRINT_OK) && TEST_PRINT_OK
# define TEST_PRINT_OK_SAVED 1
# undef TEST_PRINT_OK
# define TEST_PRINT_OK 0
#endif

static wchar_t u8towc(const std::string & s, size_t u8index, size_t * pu8len) { 
    const char * cstr = s.c_str() + u8index;
    wchar_t wc, comb_wc;
    mbstate_t st;
    memset(&st, 0, sizeof(st));
    ssize_t u8len = mbrtowc(&wc, cstr, MB_LEN_MAX, &st), comb8len;
    TESTV(HDR "(u8len:%zx,u8:%02x[%p])", u8len >= 0 && u8len != (size_t)-1, u8len, cstr[0]&0xff, cstr);
    if (u8len == (size_t)-1) memset(&st, 0, sizeof(st)); //mbtowc(&wc, NULL, 0);
    if (u8len <= 0)
        return 0;
    if (u8index + u8len < s.length() 
    &&  (comb8len = mbrtowc(&comb_wc, cstr + u8len, MB_LEN_MAX, &st)) > 0
    &&  unicode_combinable(comb_wc)) {
        wc = unicode_combine(wc, comb_wc);   
    } else comb8len = 0;
    if (pu8len != NULL) {
        *pu8len = u8len + comb8len;
    }
    return wc; 
}

static wchar_t u8itowc_l(const std::string & s, size_t wcindex, size_t * u8index) { 
    const char * cstr = s.c_str();
    wchar_t wc;
    size_t u8i = 0; 
    ssize_t u8len;
    mbstate_t st;
    memset(&st, 0, sizeof(st));
    for (size_t wci = 0; wci <= wcindex; ++wci, u8i += u8len) {
        u8len = mbrtowc(&wc, cstr + u8i, s.length() - u8i + 1, &st);
        TESTV(HDR "(u8len:%zx)", u8len >= 0 && u8len != (size_t)-1, u8len);
        if (!u8len) break ;
        if (u8len == (size_t)-1) memset(&st, 0, sizeof(st));//mbtowc(&wc, NULL, 0);
        if (wci > 0 && unicode_combinable(wc)) {
            --wci;
        }
        if (wci >= wcindex)
            break ;
    }
    if (u8index) 
        *u8index = u8i;
    wc = u8towc(s, u8i, NULL); 
    return wc; 
}

static inline wchar_t u8itowc(const std::string & s, size_t wcindex) { 
    return u8itowc_l(s, wcindex, NULL);
}

static inline size_t u8index(const std::string & s, size_t wcindex) { 
    size_t u8index;
    u8itowc_l(s, wcindex, &u8index);
    return u8index;
}

template <typename charT>
static unsigned int big_u8it_test(std::basic_ostream<charT> & out, const std::string & s) {
    unsigned int errs = 0;
    const Utf8Iterator begin = Utf8Iterator::begin(s), end=begin.end();

    out << "u8string(big operator tests): '";
    for (Utf8Iterator it = begin; it != end; ++it) {
        pwc(*it, stderr);
    }
    out << '\'' << std::endl;

    ssize_t it_counter = 0;
    size_t ustr_counter = 0, ustr_counter_prev = 0, mbl;
    const char * ustr = s.c_str();
    for (Utf8Iterator it = begin; it != end; ++it, ++it_counter, ustr_counter_prev = ustr_counter, ustr_counter += mbl) {
        wchar_t wc; 
        wc = u8towc(s, ustr_counter, &mbl);

        errs += TEST(HDR, mbl > 0);
        errs += TEST(HDR, ustr_counter == it.pos());
        errs += TEST(HDR, *it == wc); 
        if (*it < 128) {
            errs += TEST(HDR, mbl == 1 && *it == s[ustr_counter]);
        }

        Utf8Iterator itplus_pre = it; Utf8Iterator itplus_pre_val = ++itplus_pre;
        Utf8Iterator itmin_pre = it; Utf8Iterator itmin_pre_val = --itmin_pre;
        Utf8Iterator itplus_post = it; size_t itplus_oldpos = (itplus_post++).pos();
        Utf8Iterator itmin_post = it; size_t itmin_oldpos = (itmin_post--).pos();
        size_t idx;

        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",itplus_pre_val == itplus_pre, it_counter, itplus_pre_val.pos(),*itplus_pre_val);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",itplus_pre.pos() == srange(s, ustr_counter + mbl), it_counter, itplus_pre.pos(),*itplus_pre);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",*itplus_pre == u8towc(s, srange(s, ustr_counter + mbl), NULL), it_counter, itplus_pre.pos(),*itplus_pre);

        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",itmin_pre_val == itmin_pre, it_counter, itmin_pre_val.pos(),*itmin_pre_val);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",itmin_pre.pos() == srange(s, ustr_counter_prev), it_counter, itmin_pre.pos(),*itmin_pre);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc,%x)",*itmin_pre == u8towc(s, srange(s, ustr_counter_prev), NULL), it_counter, itmin_pre.pos(),*itmin_pre,*itmin_pre);

        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",itplus_post.pos() == srange(s, ustr_counter + mbl), it_counter, itplus_post.pos(),*itplus_post);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",*itplus_post == u8towc(s,srange(s,ustr_counter + mbl), NULL), it_counter, itplus_post.pos(),*itplus_post);
        errs += TESTV(HDR "(idx:%zd,opos:%zu,ov:%lc)",itplus_oldpos == it.pos(), ustr_counter, itplus_oldpos,*it);

        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",itmin_post.pos() == srange(s, ustr_counter_prev), it_counter, itmin_post.pos(),*itmin_post);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%lc)",*itmin_post == u8towc(s, srange(s, ustr_counter_prev), NULL), it_counter, itmin_post.pos(),*itmin_post);
        errs += TESTV(HDR "(idx:%zd,opos:%zu,ov:%lc)",itmin_oldpos == it.pos(), ustr_counter, itmin_oldpos,*it);

        for (ssize_t i = -s.size() - 5; i < (ssize_t)(s.size() + 5); ++i) {
            Utf8Iterator itplus = it + i;
            Utf8Iterator itmin = it - i;
            Utf8Iterator itplus_eq = it; Utf8Iterator itplus_eq_val = (itplus_eq += i);
            Utf8Iterator itmin_eq = it; Utf8Iterator itmin_eq_val = (itmin_eq -= i);

            size_t u8index_plusi = MAX(0, (ssize_t)it_counter+i);
            size_t u8index_minusi = MAX(0, (ssize_t)it_counter-i);

            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c,ret:%zu)",itplus.pos() == (idx=u8index(s, u8index_plusi)), i,it_counter,it.pos(),itplus.pos(),*itplus,idx);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c)",*itplus == u8itowc(s, u8index_plusi), i,it_counter,it.pos(),itplus.pos(),*itplus);

            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c,ret:%zu)",itmin.pos() == (idx=u8index(s, u8index_minusi)), i,it_counter,it.pos(),itmin.pos(),*itmin, idx);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c[%x],exp:%x)",*itmin == (wc=u8itowc(s, u8index_minusi)), i,it_counter,it.pos(),itmin.pos(),*itmin,*itmin,wc);

            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c,ret:%zu)",itplus_eq.pos() == (idx=u8index(s, u8index_plusi)), i,it_counter,it.pos(),itplus_eq.pos(),*itplus_eq,idx);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c)",*itplus_eq == u8itowc(s, u8index_plusi), i,it_counter,it.pos(),itplus_eq.pos(),*itplus_eq);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c)",itplus_eq == itplus_eq_val, i,it_counter,it.pos(),itplus_eq_val.pos(),*itplus_eq_val);

            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c,ret:%zu)",itmin_eq.pos() == (idx=u8index(s, u8index_minusi)), i,it_counter,it.pos(),itmin_eq.pos(),*itmin_eq,idx);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c)",*itmin_eq == u8itowc(s, u8index_minusi), i,it_counter,it.pos(),itmin_eq.pos(),*itmin_eq);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu->%zu,v:%c)",itmin_eq == itmin_eq_val, i,it_counter,it.pos(),itmin_eq_val.pos(),*itmin_eq_val);
        }
    }

    // go up to end, then go backward and re-go forward once. each loop it,it+1,it-1 are checked. 
    it_counter = mbl = 0;
    for (Utf8Iterator it = begin; mbl < 2 && it != end; ++it, ++it_counter) {
        TEST(HDR, *it == u8itowc(s, it_counter));
        if (it + 1 == end) {
            for (++it_counter, it+=2; it != begin; --it, --it_counter) {
                TEST(HDR, *it == u8itowc(s, it_counter));
                TEST(HDR, *(it+1) == u8itowc(s, it_counter + 1));
                TEST(HDR, *(it-1) == u8itowc(s, it_counter - 1));
            }
            --it;
            ++mbl;
        }
    }

    if (errs == 0) {
        out << "  OK" << std::endl;
    }
    out << std::endl;

    return errs;
}

#if defined(TEST_PRINT_OK_SAVED) && TEST_PRINT_OK_SAVED
# undef TEST_PRINT_OK
# define TEST_PRINT_OK 1
# undef TEST_PRINT_OK_SAVED
#endif

/* simple check for backward then forward on utf8 combined character */
template <typename charT>
static unsigned int little_u8it_test(std::basic_ostream<charT> & out) { //, const Utf8Iterator _end) {
    unsigned int errs = 0;
    std::string s("aáe\xcc\x81" "e"); // "e\xcc\x81" is the composed form for "\xc3\e9"
    const Utf8Iterator begin = Utf8Iterator::begin(s);
    const Utf8Iterator end=begin.end();

    out << "u8string(little operator tests): '";
    for (Utf8Iterator it = begin; it != end; ++it) {
        pwc(*it, stderr);
    }
    out << '\'' << std::endl;

    Utf8Iterator it = begin;
    errs += TEST(HDR, *(it+=2) == L'é');
    errs += TEST(HDR, *(--it) == L'á');
    errs += TEST(HDR, *(++it) == L'é');
    errs += TEST(HDR, *(++it) == L'e');
    errs += TEST(HDR, *(++it) == 0);

    out << std::endl;
    return errs;
}

int utf8_iterator_test() {
    /* WARNING: Cpp standards forbids mixing out/wcout without freopen(stdout) */
    //template <typename T> std::basic_ostream<T> & out = std::wcout;
    //std::wostream & out = std::wcout;
    std::ostream & out = std::cerr;
    std::string     s = "Hello World hé hé ·ﬂ∏ ∏ = 3.14•••. \xc3\xa9\xc2\xa9\x65\xcc\x81(combined)";//\xef\xa3\xbf.";
    char *          cs = (char*)malloc(s.length() + 1); s.copy(cs, s.length()); cs[s.length()] = 0;
    unsigned int    errs = 0;

    if (!cs) {
        fprintf(stderr, "error: malloc(string): %s\n", strerror(errno));
        return -1;
    }

    unicodeInitLocale();

    printf("\nprintfString: %s\n", cs);

    fprintf(stderr, "\nforLoopTest: '");
    for (Utf8Iterator it = Utf8Iterator::begin(s), itend=it.end(); it != itend; ++it) {
        fprintf(stderr, "%lc", *it);
    }
    fprintf(stderr, "'\n\n");

    fprintf(stderr, "forLoopTest2: '");
    for (Utf8Iterator it = Utf8Iterator::begin(s); it != it.end(); ++it) {
        fprintf(stderr, "%lc", *it);
    }
    fprintf(stderr, "'\n\n");

    errs += test_one_str(out, s.size() >= 106 ? s.substr(106) : "", Utf8Iterator::begin(s, 106, 345));
    errs += test_one_str(out, s.size() >= 6 ? s.substr(6) : "", Utf8Iterator::begin(s, 6, 345));
    errs += test_one_str(out, s.size() >= 106 ? s.substr(106) : "", Utf8Iterator::begin(s, 106));
    errs += test_one_str(out, s.size() >= 6 ? s.substr(6) : "", Utf8Iterator::begin(s, 6));
    errs += test_one_str(out, s.size() >= 6 ? s.substr(6,s.size()-6-1) : "", Utf8Iterator::begin(s, 6, s.size()-1));
    errs += test_one_str(out, std::string(cs), Utf8Iterator::begin(cs));
    errs += test_one_str(out, std::string(strlen(cs)>=5?cs+5:""), Utf8Iterator::begin(cs + 5));
    errs += test_one_str(out, std::string(strlen(cs)>=5?cs+5:"").substr(0,s.size()-1), Utf8Iterator::begin(cs + 5, s.size()-5-1));

    errs += little_u8it_test(out);

    s = "abcdefghijklmnopqrstuvwxyzéAe\xcc\x81ài\xcc\x81";
    errs += big_u8it_test(out, s);
    errs += big_u8it_test(out, s.substr(0,s.length()-2)); // skip last combinable
    errs += big_u8it_test(out, s.substr(1,s.length()-3-1)); // from 'b' to the last 'à'
    errs += big_u8it_test(out, s.substr(1,s.length()-5-1)); // from 'b' to the first \xcc\x81
    errs += big_u8it_test(out, s.substr(29,s.length()-5-29)); // from 2nd 'e'(combi) up to the first \xcc\x81
    errs += big_u8it_test(out, s.substr(30,s.length()-2-30)); // from first \xcc\x81 

    out << "Utf8Iterator: " << n_tests << " test(s), " << errs << " error(s)." << std::endl;

    if (cs != NULL)
        free(cs);

    return errs;
}

} // ! namespace utf8_iterator_tests

#endif // !VS_UTF8_ITERATOR_TESTS

/* ************************************************************************* */


