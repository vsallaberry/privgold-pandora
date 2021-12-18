/*
 * Copyright (C) 2021 Vincent Sallaberry
 * unicode conversion tool, for vegastrike (GPL) / version PrivateerGold
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
 * -------------------------------------------------------------------------*/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "unicode.h"
#include "log.h"

/* ************************************************************************* */
namespace logvs {
    unsigned int vs_log_level(const std::string & module, bool store) {
        (void) store; (void) module;
        return logvs::NOTICE;
    }
    int vs_log(const std::string & module, unsigned int level, unsigned int flags,
              const char * file, const char * func, int line, const char * fmt, ...) {
        (void) file; (void) line; (void) line;
        (void) flags;
        int ret = 0;

        if (level <= logvs::vs_log_level(module, false)) {
            va_list valist;
            va_start(valist, fmt);
            ret = vfprintf(stderr, fmt, valist);
            va_end(valist);
        }
        ret += fprintf(stderr, "\n");
        return ret;
    }
}
/* ************************************************************************* */

static int issep(unsigned int c) {
    return isspace(c) || c == '\n' || c == '\r';
}

static double str2float(double * ret_p, const char * s, size_t size) {
    if (size == (size_t)-1) {
        size = strlen(s);
    }
    char * end = NULL;
    errno = 0;
    double ret = strtod(s, &end);
    if (errno != 0 || (end != s + size && !issep(*end))) {
        return -1;
    }
    if (ret_p)
    	*ret_p = ret;
    return 0;
}

static long str2num(long * ret_p, const char * s, size_t size, int base) {
    if (size == (size_t)-1) {
        size = strlen(s);
    }
    char * end = NULL;
    errno = 0;
    long ret = strtol(s, &end, base);
    if (errno != 0 || (end != s + size && !issep(*end))) {
        return -1;
    }
    if (ret_p)
    	*ret_p = ret;
    return 0;
}

static bool realloc_outline(char ** outline_p, size_t * outsz_p, size_t needed, FILE * logout) {
    if (needed > *outsz_p) {
    	size_t newsize = (*outsz_p ? *outsz_p * 2 : 1024);
    	if (needed > newsize) newsize = needed;
        char * tmp = (char *) realloc(*outline_p, newsize);
        if (tmp == NULL) {
            fprintf(logout, "cannot malloc(%zu): %s\n", newsize, strerror(errno));
            return false;
        }
        *outline_p = tmp;
        *outsz_p = newsize;
    }
    return true;
}

enum {
    FLG_VERBOSE     = 1 << 0,
    FLG_DEBUG       = 1 << 1,
    FLG_CONVERT     = 1 << 2,
    FLG_CONVERTBACK = 1 << 3,
	FLG_AUTOFMT    	= 1 << 5,
    FLG_PRIVSAVE    = 1 << 6,
	FLG_FORCE    	= 1 << 7,
};

typedef struct {
	const char * file = NULL, * outfile = NULL;
	FILE * logout = stderr;
	unsigned int flags;
	size_t linesz = 0, outsz = 0;
	char * line = NULL, * outline = NULL;
	ssize_t n;
	size_t conv_count = 0, line_count = 0, line_conv_count = 0, words_conv_count = 0, maxlinesz = 0, maxoutlinesz = 0;
	size_t warn = 0;
} conv_ctx_t;
typedef struct {
	size_t oldpos = -1;
	size_t loc_conv_count = 0, outpos = 0, oldoutpos = 0, newwordidx = 0;
	ssize_t loc_word_conv = 0;
	unsigned int oldchar = ' ';
	unsigned int line_type = 0;
	size_t nextpos;
	Utf8Iterator itend = Utf8Iterator(NULL);
} conv_line_ctx_t;

static size_t convert_one(Utf8Iterator & it, conv_ctx_t * g, conv_line_ctx_t * l) {
	if (it == l->itend) return 0;
	char utf8[MB_CUR_MAX+1];
	int n8;
	l->oldpos = it.pos();
	l->nextpos = (it+1).pos();
	l->oldoutpos=l->outpos;
	if ((g->flags & FLG_CONVERTBACK) != 0) {
		if (*it > 255) {
			fprintf(g->logout, "bad conversion (>255)\n");
			*utf8 = '?';
		} else {
			*utf8 = (char)*it;
		}
		n8 = 1;
		utf8[n8] = 0;
		if (*it > 127) {
			if (l->loc_word_conv == 0)
				++g->words_conv_count;
			++l->loc_conv_count;
			++g->conv_count;
			l->loc_word_conv -= l->nextpos - it.pos() - 1;
		}
	} else {
		n8 = utf32_to_utf8(utf8, *it);
		if (*it == 0 || n8 <= 0) {
			fprintf(g->logout, "bad utf8 conversion\n");
		}
	}
	if (!realloc_outline(&g->outline, &g->outsz, l->outpos + n8 + 1, g->logout))
		return 0;
	memcpy(g->outline + l->outpos, utf8, n8);
	l->outpos += n8;
	if (n8 > 1 && it.pos()+1 == l->nextpos) {
		if ((g->flags & FLG_DEBUG) != 0) {
			fprintf(g->logout, "%s: l%zu,c%zu: converted one character '%s'\n", g->file ? g->file : "<stdin>", g->line_count, it.pos(), utf8);
		}
		++l->loc_conv_count;
		++g->conv_count;
		if (l->loc_word_conv == 0)
			++g->words_conv_count;
		l->loc_word_conv += n8 - 1;
	}
	++it;
	return n8;
}
enum { PRIV_FF, PRIV_SS, PRIV_FG, PRIV_FTO, PRIV_FRL, PRIV_NEWS, PRIV_DNEWS, PRIV_LAST=PRIV_DNEWS,
	   PRIV_UNKNOWN };
static const char * s_privateer_kw[] = {
	NULL, "FF:", "SS:", "FG:", "FactionTookOver", "FactionRefList", "news", "dynamic_news", NULL
};
static unsigned int get_privsave_linetype(conv_ctx_t * g, conv_line_ctx_t * l) {
	if ((g->flags & (FLG_PRIVSAVE | FLG_AUTOFMT)) == 0) return PRIV_UNKNOWN;
	const char * s = g->line;
	while (issep(*s)) ++s;
	if (!isdigit(*s)) return PRIV_UNKNOWN;
	if (*s == '0' && issep(s[1])) return PRIV_UNKNOWN;
	while (isdigit(*s)) ++s;
	if (!issep(*s)) return PRIV_UNKNOWN;
	while (issep(*s)) ++s;
	for (size_t i = 0; i < sizeof(s_privateer_kw)/sizeof(*s_privateer_kw); ++i) {
		if (s_privateer_kw[i] && !strncmp(s, s_privateer_kw[i], strlen(s_privateer_kw[i]))) return i;
	}
	return PRIV_UNKNOWN;
}
static int get_word(Utf8Iterator & it, conv_ctx_t * g, conv_line_ctx_t * l, size_t len, const char ** pword, size_t *plen, const char * label = "WORD") {
	size_t n8 = 0, n = 0, outlen, wordpos;
	bool skipsep = (len == (size_t)-1);
	l->loc_word_conv = 0;
	while (skipsep && n < len && (n8 = convert_one(it, g, l)) > 0 && issep(g->outline[l->outpos-n8])) n += (l->nextpos - l->oldpos);
	wordpos = l->outpos - n8;
	outlen = n8;
	while (n < len && (n8 = convert_one(it, g, l)) > 0 && (!skipsep || !issep(g->outline[l->outpos-n8]))) {
		n += (l->nextpos - l->oldpos);
		outlen += n8;
	}
	if (pword) *pword = g->outline + wordpos;
	if (plen) *plen = outlen;
	if ((g->flags & FLG_DEBUG) != 0) {
		fprintf(g->logout, "type:%u %s(%zu):",l->line_type, label, outlen);
		fwrite(g->outline + wordpos, 1, outlen, g->logout); fprintf(g->logout, "\n");
	}
	return 0;
}
static int get_num(Utf8Iterator & it, conv_ctx_t * g, conv_line_ctx_t * l, long * pnum, const char ** pnumpos, size_t *pnumlen) {
	int ret = get_word(it, g, l, (size_t)-1, pnumpos, pnumlen, "NUM");
	if (!ret && pnumpos && pnumlen) ret = str2num(pnum, *pnumpos, *pnumlen, 10);
	return ret;
}

int convert(const char * file, const char * outfile, unsigned int flags) {
    int ret = 0;
    FILE * fp_in, * fp_out;

    conv_ctx_t g;
    g.flags = flags;
    g.logout = stderr;
    g.file = file;
    g.outfile = outfile;

    if (file == NULL) {
        fp_in = stdin;
    } else if ((fp_in = fopen(file, "r")) == NULL) {
        fprintf(g.logout, "%s: %s\n", file, strerror(errno));
        return -1;
    }
    if (outfile == NULL) {
        fp_out = stdout;
    } else if ((!strcmp(file ? file : "", outfile)&&((errno=EINVAL)|1))
    || (fp_out = fopen(outfile, "w")) == NULL) {
        fprintf(g.logout, "%s: %s\n", outfile, strerror(errno));
        if (fp_in != stdin) fclose(fp_in);
        return -1;
    }
    unicodeInitLocale();

    while ((g.n = getline(&g.line, &g.linesz, fp_in)) > 0) {
    	g.line[g.n] = 0;
    	conv_line_ctx_t l;
    	Utf8Iterator it = Utf8Iterator::begin(g.line, g.n);
    	l.itend = it.end();

        if (g.n > g.maxlinesz)
            g.maxlinesz = g.n;

        l.line_type = get_privsave_linetype(&g, &l);

        if ((flags & FLG_DEBUG) != 0)
        	fprintf(g.logout, "LINE %s", g.line);

        if (l.line_type != PRIV_UNKNOWN) {
        	long num;
        	size_t numlen = 0, len = 0;
        	size_t skipwords = 1;
        	size_t numpos = 0, wordpos = 0;
        	while (it != l.itend) {
        		const char * num_p = g.outline, * word_p = g.outline;
        		size_t numpos = 0, wordpos = 0;
        		if (*it == '\r' || *it == '\n')
        			break ;
        		if (get_num(it, &g, &l, &num, &num_p, &numlen) != 0) {
        			++g.warn;
        			fprintf(stderr, "warning: l%zu,c%zu: cannot parse number after '%s': %s\n",
        					g.line_count, l.oldpos, g.outline+wordpos, strerror(errno)); //break ;
        		}
        		numpos = num_p - g.outline;
        		if (get_word(it, &g, &l, num, &word_p, &len) != 0) {
        			++g.warn;
        			fprintf(stderr, "warning: l%zu,c%zu: cannot parse word after '%s': %s\n",
        					g.line_count, l.oldpos, g.outline+numpos, strerror(errno)); //break ;
        		}
        		wordpos = word_p - g.outline;
        		if (len != num && !issep(g.outline[wordpos])) {
        			if ((size_t)num + l.loc_word_conv == len) {
        				if ((flags & FLG_VERBOSE) != 0) {
        					fprintf(g.logout, "NEWLEN conversion_count %zu (old:%ld,conv:%zd)\n", len, num, l.loc_word_conv);
        				}
        			}else {
        				++g.warn;
        				fprintf(g.logout, "warning: NEWLEN NO conv MATCH %zu (old:%ld,conv:%zd)\n", len, num, l.loc_word_conv);
        			}
        			if ((size_t)num + l.loc_word_conv == len || (flags & FLG_FORCE) != 0) {
        				if (!realloc_outline(&g.outline, &g.outsz, l.outpos + (l.loc_word_conv>0 ? l.loc_word_conv : 2) + 5, g.logout))
        					break ;
        				char tmp[256];
        				size_t tmp_pos = snprintf(tmp, sizeof(tmp), "%zu%c", len, g.outline[numpos+numlen]);
        				memmove(g.outline+numpos+tmp_pos, g.outline+wordpos, l.outpos-wordpos);
        				memcpy(g.outline+numpos, tmp, tmp_pos);
        				l.outpos = numpos + tmp_pos + (l.outpos-wordpos);
        				g.outline[l.outpos] = 0;
        				if ((flags & FLG_VERBOSE) != 0) {
        					fprintf(g.logout, ", NEW STRING '%s'\n", g.outline+numpos);
        				}
        			}
        		}
        		for (size_t i = 0; it != l.itend && i < skipwords; ++i) {
        			if (get_word(it, &g, &l, (size_t)-1, NULL, NULL) != 0) {
        				++g.warn;
        			}
        		}
        		skipwords = 0;
        		if (0&&l.outpos > 8192) {
        			g.outline[l.outpos] = 0;
        			fwrite(g.outline, 1, l.outpos, fp_out);
        			l.outpos = 0;
        		}
        	}
        }

        for ( ; it != l.itend; ) {
        	if (l.line_type > PRIV_LAST && issep(*it)) {
        		l.loc_word_conv = 0;
        	}
        	convert_one(it, &g, &l);
        }

        if (g.outline == NULL)
            break ;
        ++g.line_count;
        if (l.outpos > g.maxoutlinesz)
            g.maxoutlinesz = l.outpos;
        g.outline[l.outpos] = 0;
        fwrite(g.outline, 1, l.outpos, fp_out);
        if (l.loc_conv_count > 0) {
            if ((flags & FLG_VERBOSE) != 0) {
                fprintf(g.logout, ">>converted line(t:%d): %s", l.line_type, g.outline);
            }
            ++g.line_conv_count;
        }
    }
    if (fp_out != stdout && fp_out != stderr) fclose(fp_out);
    if (fp_in != stdin) fclose(fp_in);
    if (g.line) free(g.line);
    if (g.outline) free(g.outline);
    fprintf(g.logout, "converted '%s' written to '%s' (fixed %zu lines, %zu words, %zu warnings - maxlinesz=in:%zu,out:%zu).\n",
            file ? file : "<stdin>", outfile ? outfile : "<stdout>",
            g.line_conv_count, g.words_conv_count, g.warn, g.maxlinesz, g.maxoutlinesz);
    return ret;
}

/* ************************************************************************* */
static int usage(int argc, char ** argv, int status) {
    (void) argc;
    fprintf(stdout, "Usage: %s\n"
    			    "    [-h,--help] [-v,--verbose] [-P,--privateer-savegame] [-f,--force]\n"
    		        "    [-C,--convert|-B,--convert-back] ['-'|<in-file> ['-'|<out-file>]]\n", argv[0]);
    return status;
}
int main(int argc, char ** argv) {
    unsigned int flags = FLG_CONVERT | FLG_AUTOFMT;
    const char * in = NULL, * out = NULL;
    for (int i = 1, take_options = 1 ; i < argc; ++i) {
        int err = 0;
        if (!strcmp(argv[i], "--")) { take_options = 0; continue ; }
        if (take_options && *argv[i] == '-' && argv[i][1]) {
        	char fake_short[3] = { '-', '-', 0 };
        	const char * short_opts = argv[i];
        	if (argv[i][1] == '-') {
        		short_opts = fake_short;
        		if (!strcmp(argv[i], "--help")) 					fake_short[1] = 'h';
        		else if (!strcmp(argv[i], "--verbose")) 			fake_short[1] = 'v';
        		else if (!strcmp(argv[i], "--privateer-savegame")) 	fake_short[1] = 'P';
        		else if (!strcmp(argv[i], "--convert")) 			fake_short[1] = 'C';
        		else if (!strcmp(argv[i], "--force")) 				fake_short[1] = 'f';
        		else if (!strcmp(argv[i], "--convert-back")) 		fake_short[1] = 'B';
        	}
        	for (int j=1; short_opts[j]; ++j) {
        		switch (short_opts[j]) {
					case 'h': return usage(argc, argv, 0);
					case 'v': flags |= ((flags & FLG_VERBOSE) ? FLG_DEBUG : FLG_VERBOSE); break ;
					case 'f': flags |= FLG_FORCE; break ;
					case 'P': flags = (flags & ~(FLG_AUTOFMT)) | FLG_PRIVSAVE; break ;
					case 'B': flags = (flags & ~(FLG_CONVERT|FLG_CONVERTBACK)) | FLG_CONVERTBACK; break ;
					case 'C': flags = (flags & ~(FLG_CONVERT|FLG_CONVERTBACK)) | FLG_CONVERT; break ;
					default:  ++err; break;
        		}
        	}
        } else {
        	if (in == NULL && out == NULL) {
        		if (i + 0 < argc && (argv[i+0][0] != '-' || argv[i+0][1] == 0)) {
        			in = *argv[i+0] == '-' ? NULL : argv[i+0];
        			if (i + 1 < argc && (argv[i+1][0] != '-' || argv[i+1][1] == 0)) {
        				out = *argv[i+1] == '-' ? NULL : argv[i+1];
        				++i;
        			}
        		}
        	} else {
        		++err;
        	}
        }
        if (err) {
            fprintf(stderr, "invalid argument '%s'.\n", argv[i]);
            return usage(argc, argv, -1);
        }
    }
    if ((flags & (FLG_CONVERT|FLG_CONVERTBACK)) != 0) {
        return convert(in, out, flags);
    }
    return 0;
}

