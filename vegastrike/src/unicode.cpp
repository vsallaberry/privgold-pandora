/*
 * Copyright (C) 2021-2022 Vincent Sallaberry
 * unicode Utf8Iterator class, for vegastrike (GPL) / version PrivateerGold
 *   http://vegastrike.sourceforge.net/
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
#endif

#include "log.h"

// -----------------------------
// utf8/utf32 simple conversions
// -----------------------------

#if !defined(HAVE_WCHAR_H) && !defined(HAVE_CWCHAR)
size_t vs_wcrtomb_ascii(char * dst, wchar_t wc, void * ctx) {
    (void)ctx;
    *dst[0] = (char)wc;
    *dst[1] = 0;
    return 1;
}
size_t vs_mbrtowc_ascii(wchar_t * wc, const char * buf, size_t len, void * ctx) {
    (void)ctx;
    if (wc && buf && len > 0) {
        *wc = (*buf & 0xff);
        return 1;
    } else {
        return (size_t)-1;
    }
}
#endif

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

/* ************************************************************************* */

// -----------------------------
// Utf8Iterator class impl.
// -----------------------------
unsigned int Utf8Iterator::_default_flags = U8F_COMBINE;

void Utf8Iterator::init() {
    memset(&_mbstate, 0, sizeof(_mbstate));
    _incr = _nextincr = _pos = 0;
    _nextvalue = 0;
    _flags = Utf8Iterator::_default_flags;
    next(); next(); // initializes _{next,}value and _{next,}incr.
}

Utf8Iterator & Utf8Iterator::operator++() {
    _pos += _incr;
    next();
    return *this;
}

Utf8Iterator & Utf8Iterator::operator--() {
    _incr = _nextincr = 0;
    UTF8_ITERATOR_DEBUG("Utf8Iterator::--() POS %zu VAL %x\n", _pos, _str[_pos]&0xff);
    while (_pos > 0 && (_str[--_pos] & 0x80)) {
        UTF8_ITERATOR_DEBUG("Utf8Iterator::--() POS %zu VAL %x\n", _pos, _str[_pos]&0xff);
        if ((_str[_pos] & 0xC0) == 0xC0) {
            break ;
        }
    }
    UTF8_ITERATOR_DEBUG("Utf8Iterator::--() RETURN. POS %zu VAL %x\n", _pos, _str[_pos]&0xff);
    wchar_t save_next = _value;
    size_t save_incr = _incr;
    next();
    _value = _nextvalue;
    _incr = _nextincr;
    _nextvalue = save_next;
    _nextincr = save_incr;
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
            if (_value < ((size_t)(1<<16)) && _nextvalue < ((size_t)(1<<16)) && unicode_combinable(_nextvalue)) {
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

void unicodeInitLocale() {
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
                   #if defined(HAVE_LOCALE) && (!defined(__APPLE__) || (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_9) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_9)
                    try {
                        std::cout.imbue(std::locale(std::locale::classic(), loc, std::locale::ctype));
                        std::cerr.imbue(std::locale(std::locale::classic(), loc, std::locale::ctype));
                    } catch (...) {
                        VS_LOG("unicode", logvs::WARN, "cannot set std::cout utf8 locale");
                    }
                   #endif
				   #if defined(_WIN32)
				   # if 0 && (defined(__CYGWIN__) || defined(__MINGW32__))
					_setmode(fileno(stdout), _O_U8TEXT);
					_setmode(fileno(stderr), _O_U8TEXT);
				   # elif defined(_WIN32)
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
#endif
}
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

#define TEST_PRINT_OK 1
#define STR(x) #x
#define PTEST(hdr,cond,res,...) fprintf(stderr, hdr " %s [%s]" "\n", __VA_ARGS__, res, STR(cond))
#define TESTV(_hdr, cond, ...) ((cond) && (++n_tests|1) ? (TEST_PRINT_OK?PTEST(_hdr,cond,"OK",__VA_ARGS__)*0:0) : PTEST(_hdr,cond,"FAILED",__VA_ARGS__)*0+1)
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
static unsigned int test_one_str(std::basic_ostream<charT> & out, const std::string s, const Utf8Iterator begin) { //, const Utf8Iterator _end) {
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

template <typename charT>
static unsigned int test_ascii_str(std::basic_ostream<charT> & out) { //, const Utf8Iterator _end) {
    unsigned int errs = 0;
    std::string s("abcdefghijklmnopqrstuvwxyz");
    const Utf8Iterator begin = Utf8Iterator::begin(s);
    const Utf8Iterator end=begin.end();

    out << "u8string(big operator tests): '";
    for (Utf8Iterator it = begin; it != end; ++it) {
        //out << (charT) *it;
        pwc(*it, stderr);
    }
    out << '\'' << std::endl;

#if defined(TEST_PRINT_OK) && TEST_PRINT_OK
# define TEST_PRINT_OK_SAVED TEST_PRINT_OK
# undef TEST_PRINT_OK
# define TEST_PRINT_OK 0
#endif

    ssize_t it_counter = 0;
    for (Utf8Iterator it = begin; it != end; ++it, ++it_counter) {
        errs += TEST(HDR,it_counter == it.pos());
        errs += TEST(HDR,*it == s[it_counter]);

        Utf8Iterator itplus_pre = it; Utf8Iterator itplus_pre_val = ++itplus_pre;
        Utf8Iterator itmin_pre = it; Utf8Iterator itmin_pre_val = --itmin_pre;
        Utf8Iterator itplus_post = it; size_t itplus_oldpos = (itplus_post++).pos();
        Utf8Iterator itmin_post = it; size_t itmin_oldpos = (itmin_post--).pos();

        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",itplus_pre_val == itplus_pre, it_counter, itplus_pre_val.pos(),*itplus_pre_val);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",itplus_pre.pos() == srange(s, it_counter + 1), it_counter, itplus_pre.pos(),*itplus_pre);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",*itplus_pre == s[srange(s, it_counter + 1)], it_counter, itplus_pre.pos(),*itplus_pre);

        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",itmin_pre_val == itmin_pre, it_counter, itmin_pre_val.pos(),*itmin_pre_val);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",itmin_pre.pos() == srange(s, it_counter - 1), it_counter, itmin_pre.pos(),*itmin_pre);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",*itmin_pre == s[srange(s, it_counter - 1)], it_counter, itmin_pre.pos(),*itmin_pre);

        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",itplus_post.pos() == srange(s, it_counter + 1), it_counter, itplus_post.pos(),*itplus_post);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",*itplus_post == s[srange(s, it_counter + 1)], it_counter, itplus_post.pos(),*itplus_post);
        errs += TESTV(HDR "(idx:%zd,opos:%zu,ov:%c)",itplus_oldpos == it.pos(), it_counter, itplus_oldpos,*it);

        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",itmin_post.pos() == srange(s, it_counter - 1), it_counter, itmin_post.pos(),*itmin_post);
        errs += TESTV(HDR "(idx:%zd,pos:%zu,v:%c)",*itmin_post == s[srange(s, it_counter - 1)], it_counter, itmin_post.pos(),*itmin_post);
        errs += TESTV(HDR "(idx:%zd,opos:%zu,ov:%c)",itmin_oldpos == it.pos(), it_counter, itmin_oldpos,*it);

        for (ssize_t i = -s.size() - 5; i < (ssize_t)(s.size() + 5); ++i) {
            Utf8Iterator itplus = it + i;
            Utf8Iterator itmin = it - i;
            Utf8Iterator itplus_eq = it; Utf8Iterator itplus_eq_val = itplus_eq += i;
            Utf8Iterator itmin_eq = it; Utf8Iterator itmin_eq_val = itmin_eq -= i;

            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",itplus.pos() == srange(s, it_counter +i), i, it_counter, itplus.pos(),*itplus);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",*itplus == s[srange(s, it_counter + i)], i, it_counter, itplus.pos(),*itplus);

            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",itmin.pos() == srange(s, it_counter - i), i, it_counter, itmin.pos(),*itmin);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",*itmin == s[srange(s, it_counter - i)], i, it_counter, itmin.pos(),*itmin);

            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",itplus_eq.pos() == srange(s, it_counter + i), i, it_counter, itplus_eq.pos(),*itplus_eq);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",*itplus_eq == s[srange(s, it_counter + i)], i, it_counter, itplus_eq.pos(),*itplus_eq);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",itplus_eq == itplus_eq_val, i, it_counter, itplus_eq_val.pos(),*itplus_eq_val);

            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",itmin_eq.pos() == srange(s, it_counter - i), i, it_counter, itmin_eq.pos(),*itmin_eq);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",*itmin_eq == s[srange(s, it_counter - i)], i, it_counter, itmin_eq.pos(),*itmin_eq);
            errs += TESTV(HDR "(i:%zd,idx:%zd,pos:%zu,v:%c)",itmin_eq == itmin_eq_val, i, it_counter, itmin_eq_val.pos(),*itmin_eq_val);
        }
    }
#ifdef TEST_PRINT_OK_SAVED
# undef TEST_PRINT_OK
# define TEST_PRINT_OK TEST_PRINT_OK_SAVED
# undef TEST_PRINT_OK_SAVED
#endif

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

    errs += test_ascii_str(out);

    out << "Utf8Iterator: " << n_tests << " test(s), " << errs << " error(s)." << std::endl;

    if (cs != NULL)
        free(cs);

    return errs;
}

} // ! namespace utf8_iterator_tests

#endif // !VS_UTF8_ITERATOR_TESTS

/* ************************************************************************* */


