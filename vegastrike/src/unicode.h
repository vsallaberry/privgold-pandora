/*
 * Copyright (C) 2021-2022 Vincent Sallaberry
 * unicode Utf8Iterator class, for vegastrike (GPL) / version PrivateerGold
 *   http://vegastrike.sourceforge.net/, privateer.solsector.net
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
 *  #include "unicode.h"
 *  int main() {
 *    std::string     s = "Hello World hé hé ·ﬂ∏ ∏ = 3.14•••.";
 *    unicodeInitLocale();
 *    fprintf(stdout, "forLoopTest: '");
 *    for (Utf8Iterator it = Utf8Iterator::begin(s, 5), itend=it.end(); it != itend; ++it) {
 *      fprintf(stdout, "%lc", *it);
 *    }
 *    fprintf(stdout, "'\n\n");
 *    return 0;
 *  }
 */
#ifndef VS_UNICODE_H
#define VS_UNICODE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <iterator>
#include <string>

#include <string.h>
#include <stdlib.h>

#if defined(HAVE_CWCHAR)
# include <cwchar>
#elif defined(HAVE_WCHAR_H)
# include <wchar.h>
#else
# define MB_CUR_MAX 6
typedef int wchar_t;
# define mbrtowc(_wc, _str, _len, _ctx) vs_mbrtowc_ascii(_wc, _str, _len, _ctx)
# define wcrtomb(_str, _wc, _ctx)       vs_wcrtomb_ascii(_str, _wc, _ctx)
#endif

/*---------------------------------------------
 * For information only: c++ 11 utf8 utlities.
 * For now we use libc wcrtomb and mbrtowc for best compiler coverage. */
/*
 #if defined(HAVE_CODECVT)
 # include <codecvt>
 # include <locale>
 static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv_utf8_utf32;
 //std::u32string u32str = conv_utf8_utf32.from_bytes(utf8_input);
 //conv_utf8_utf32.to_bytes()
 #endif
 //SDL can also do it:
 //Uint32 * u32str = SDL_iconv_utf8_ucs4(input); //warning big/little endian
 //
 -------------------------------------------------*/

#if 0
# define UTF8_ITERATOR_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
static inline int utf8_debug_dummy() { return 0; }
# define UTF8_ITERATOR_DEBUG(...) utf8_debug_dummy()
#endif


/* *****************************************
 * global methods
 * *****************************************/

/* get UTF32 unicode from utf8 character(s) : use utf8StringIter to process a string
 * returns the utf32 character or 0 on error */
wchar_t utf8_to_utf32(const char * utf8);
/* get utf8 bytes from an utf32 character (MB_CUR_MAX bytes stored at most)
 * returns the number of bytes stored in dst or (size_t)-1 or error */
size_t utf32_to_utf8(char * dst, wchar_t utf32);

/* init Locale, needed for wcrtomb and mbrtowc used by Utf8Iterator */
void unicodeInitLocale();


/* *****************************************
 * class Utf8Iterator
 * *****************************************/
/* IMPORTANT NOTE:
 *   If utf8 is handled in a clever way, it will have very very low memory
 * and performance overhead (almost 0 mem overhead on ascii). You just have to
 * keep in mind that direct access is very expensive with utf8, actually, it
 * does not exist. Indeed you have to process each character before going to the next.
 *   Then, if you can replace (*(it+6)) by iterative ++it, it will be better. */
class Utf8Iterator : public std::iterator<std::input_iterator_tag, const wchar_t,
long, const wchar_t *, const wchar_t &> {
public:
    // Ctors
    explicit Utf8Iterator(const std::string & s, size_t start = 0, size_t end = (size_t)-1) {
        size_t slen = s.length(); if (start > slen) start = slen;
        _size = (end > slen ? slen : end) - start;
        _str = s.c_str() + start;
        init();
        //fprintf(stderr, "[contructed] ('%s' size:%zu strlen:%zu).\n", _str, _size, strlen(_str));
    }
    explicit Utf8Iterator(const char * s, size_t size = (size_t)-1) :
    _str(s),
    _size(!s ? 0 : (size == (size_t)-1 ? strlen(s) : size)) {
        init();
        //fprintf(stderr, "[contructed] ('%s' size:%zu strlen:%zu).\n", _str, _size, strlen(_str));
    }
    // Operators
    Utf8Iterator &      operator++();
    Utf8Iterator &      operator--();
    Utf8Iterator        operator++(int) {
        Utf8Iterator ret = *this; ++(*this); return ret;
    }
    Utf8Iterator        operator--(int) {
        Utf8Iterator ret = *this; --(*this); return ret;
    }
    bool                operator==(const Utf8Iterator & other) const {
        return _str + _pos == other._str + other._pos;
    }
    bool                operator !=(const Utf8Iterator & other) const {
        return !(*this == other);
    }
    wchar_t             operator*() const {
        return _value;
    }
    Utf8Iterator &      operator+=(ssize_t n) {
        Utf8Iterator & ret = *this; if (n < 0) ret -= -n; else while (n--) ++ret; return ret;
    }
    Utf8Iterator &      operator-=(ssize_t n) {
        Utf8Iterator & ret = *this; if (n < 0) ret += -n; else while (n--) --ret; return ret;
    }
    Utf8Iterator        operator+(ssize_t n) const {
        Utf8Iterator ret = *this; ret += n; return ret;
    }
    Utf8Iterator        operator-(ssize_t n) const {
        Utf8Iterator ret = *this; ret -= n; return ret;
    }
    // Accessors
    size_t              pos() const {
        return _pos;
    }
    const Utf8Iterator  end() const {
        Utf8Iterator ret = *this; ret._pos = _size;
        ret._value = 0; ret._incr = 0; return ret;
    }

    // static builders
    static Utf8Iterator begin(const std::string & s, size_t start = 0, size_t end = (size_t)-1) {
        return Utf8Iterator(s, start, end);
    }
    static Utf8Iterator begin(const char * s, size_t size = (size_t)-1) {
        return Utf8Iterator(s, size);
    }
    static Utf8Iterator end(const std::string & s, size_t start = 0, size_t end = (size_t)-1) {
        return Utf8Iterator::begin(s, start, end).end();
    }
    static Utf8Iterator end(const char * _s, size_t _size = (size_t)-1) {
        return Utf8Iterator::begin(_s, _size).end();
    }

protected:
    const char *        _str;
    size_t              _size;
    size_t              _pos;
    size_t              _incr;
    mbstate_t           _mbstate;
    wchar_t             _value;

    void        init();
    void        next();
};

/* ************************************************************************* */

#ifdef VS_UTF8_ITERATOR_TESTS
namespace utf8_iterator_tests {
    int utf8_iterator_test();
}
#endif // !VS_UTF8_ITERATOR_TESTS

/* ************************************************************************* */

#endif // ! #ifndef VS_UNICODE_H
