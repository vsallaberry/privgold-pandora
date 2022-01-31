/*
* Vega Strike
* Copyright (C) 2021 Vincent Sallaberry
*
* http://vegastrike.sourceforge.net/
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#elif ! defined(_WIN32) && !defined(HAVE_STRCASECMP)
#define HAVE_STRCASECMP
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
# include <time.h>
# define flockfile(f)   _lock_file(f)
# define funlockfile(f) _unlock_file(f)
# if defined(__CYGWIN__) || defined(__MINGW32__)
#  include <io.h>
# endif
#else
# include <unistd.h>
# include <sys/time.h>
#endif

#include "log.h"

#include "gnuhash.h"
#ifndef VS_LOG_NO_XML
# include "xml_support.h"
# include "configxml.h"
# include "cmd/script/msgcenter.h"
# include "lin_time.h"
extern VegaConfig * vs_config;
extern Mission * mission;
//#define VS_LOG_USE_VSTIME
#else
# undef  VS_LOG_USE_VSTIME
# undef  VS_LOG_NO_MSGCENTER
# define VS_LOG_NO_MSGCENTER
#endif

#if defined(VS_LOG_USE_VSTIME)
# define logGetElapsedTime() GetElapsedTime()
#elif defined(HAVE_GETTIMEOFDAY)
static double logGetElapsedTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    static double origtime = time.tv_sec + (time.tv_usec/1000000.0);
    return (time.tv_sec + (time.tv_usec/1000000.0)) - origtime;
}
#elif defined(HAVE_SDL)
# include <SDL_timer.h> 
static double logGetElapsedTime() {
    return SDL_GetTicks() * 1.e-3;
}
#endif

/*------------------------------------------------------------------------------------------*/
namespace logvs {

#define VSLOG_LEVEL_DEFAULT 3
#define STR(x) #x //convert argument into string
#define STR2(x) STR(x) // resolve the macro value before stringifying it

typedef vsUMap<std::string, unsigned int> LogMap;

static unsigned int log_flags = F_LOCATION_FOOTER; // | F_LOCATION_ALLWAYS;
static LogMap       log_map;
static FILE *       vslog_out = stderr;
static bool         vslog_out_istty = false;
static int          log_level_default = -1;
static int          log_colorize = -1;

static const char * log_level_names[logvs::LVL_NB+1] = {
    "none",
    "error",
    "warning",
    "notice",
    "info",
    "verbose",
    "debug",
    "debug+"
};

#ifndef _WIN32
static const char * const log_level_color_reset = "\e[00m";
static const char * const log_level_color[logvs::LVL_NB] = {
    "",
    "\e[01;31m",
    "\e[01;33m",
    "\e[00;32m",
    "\e[00;34m",
    "\e[00;36m",
    "\e[00;35m"
};
#endif

#ifndef VS_LOG_NO_MSGCENTER
#define VSLOG_MSGCENTER_ON(flags) (((flags) & logvs::F_MSGCENTER) != 0 && mission != NULL && mission->msgcenter != NULL)
#define vs_log_msgcenter_v(category,level,flags,islast,file,func,line,fmt,valist) \
    if (VSLOG_MSGCENTER_ON(flags)) { \
        va_start(valist, fmt); \
        vs_log_msgcenter_v_(category,level,flags,islast,file,func,line,fmt,valist); \
        va_end(valist); \
    }
#define vs_log_msgcenter(category,level,flags,islast,file,func,line,...) \
	if (VSLOG_MSGCENTER_ON(flags)) { \
		vs_log_msgcenter_(category, level, flags, islast, file, func, line, __VA_ARGS__); \
	}
static void vs_log_msgcenter_v_(const std::string & category, unsigned int level, unsigned int flags,
                                bool islast, const char * file, const char * func, int line,
                                const char * fmt, va_list valist) {
    (void)level; (void)file; (void)func; (void)line;
    static char buf[4096];//it is ok to use static because it is under FILE* vslog_out lock which is unique
    static size_t off = 0;
    int ret;
    if (fmt != NULL && (ret = vsnprintf(buf+off, sizeof(buf)-off, fmt, valist)) > 0) {
        off += ret;
        if (off > sizeof(buf))
            off = sizeof(buf);
    }
    if (islast) {
        mission->msgcenter->add("log/" + category, "all", buf);
        off = 0;
    }
}
static void vs_log_msgcenter_(const std::string & category, unsigned int level, unsigned int flags,
        					  bool islast, const char * file, const char * func, int line,
							  const char * fmt, ...) {
	va_list valist;
	va_start(valist, fmt);
	vs_log_msgcenter_v_(category, level, flags, islast, file, func, line, fmt, valist);
	va_end(valist);
}
#else
// VS_LOG_NO_MSGCENTER
# define vs_log_msgcenter_v(...)
# define vs_log_msgcenter(...)
#endif

#ifdef VS_LOG_NO_XML
void vs_log_setlevel(const std::string & category, unsigned int level) {
	if (category.empty()) {
		log_level_default = level;
	} else {
		log_map.insert(std::make_pair(category, level));
	}
}
#endif

#define VSLOG_LOCFOOTER "  -  [%s:%s():%d]"
#define VSLOG_LOCHEADER "(%s:%s():%d) "
static int log_print_location(FILE * out, unsigned int flags, const std::string & category, unsigned int level,
                              const char * file, const char * func, int line) {
    (void) level;
    (void) category;
    if ((flags & F_LOCATION_MASK) == 0
        || ((flags & F_LOCATION_ALLWAYS) == 0
            && ((int)level < NONE
                || (level > logvs::WARN && level < logvs::DBG)))) {
        return 0;
    }
    const char * shortfile = strrchr(file, '/');
    if ((flags & F_LOCATION_FOOTER) != 0) {
    	vs_log_msgcenter(category, level, flags, false, file, func, line,
    			            VSLOG_LOCFOOTER, shortfile ? shortfile + 1: file, func, line);
    	return fprintf(out, VSLOG_LOCFOOTER, shortfile ? shortfile + 1: file, func, line);

    } else if ((flags & F_LOCATION_HEADER) != 0) {
    	vs_log_msgcenter(category, level, flags, false, file, func, line,
    			            VSLOG_LOCHEADER, shortfile ? shortfile + 1: file, func, line);
        return fprintf(out, VSLOG_LOCHEADER, shortfile ? shortfile + 1: file, func, line);
    }
    return 0;
}

const char * log_level_name(unsigned int level) {
    return log_level_names[level < logvs::LVL_NB ? level : logvs::LVL_NB];
}

unsigned int log_level_byname(const char * name) {
#if !defined(HAVE_STRCASECMP) && defined(HAVE_STRICMP)
# define strcasecmp(s1,s2) stricmp(s1,s2)
#endif
#if !defined(HAVE_STRNCASECMP) && defined(HAVE_STRNICMP)
# define strncasecmp(s1,s2,n) strnicmp(s1,s2,n)
#endif
	errno = 0;
    char * last = NULL;
    long result = strtol(name, &last, 10);
    if (errno == 0 && result >= 0 && last != NULL && *last == 0) {
        return (unsigned int) result;
    }
    
    for (unsigned int i = 0; i < logvs::LVL_NB; ++i) {
        if (!strcasecmp(name, log_level_names[i])) {
            return i;
        }
    }
    
    return log_level_default > 0 ? log_level_default : VSLOG_LEVEL_DEFAULT;
}

FILE * vs_log_setfile(FILE * out) {
    FILE * old = vslog_out;
    if (out != NULL) flockfile(out);
    if (old != NULL) flockfile(old);
    vslog_out = out;
    vslog_out_istty = isatty(fileno(out));
    if (out != NULL) funlockfile(out);
    if (old != NULL) funlockfile(old);
    return old;
}

unsigned int vs_log_setflag(unsigned int flag, bool value) {
    unsigned int oldflags = log_flags;
    if (value) {
        log_flags |= flag;
    } else {
        log_flags &= (~(flag));
    }
    return oldflags;
}

unsigned int vs_log_setflags(unsigned int flags) {
    unsigned int oldflags = log_flags;
    log_flags = flags;
    return oldflags;
}

unsigned int vs_log_level(const std::string & category, bool store) {
    FILE * out = vslog_out;
    if (out == NULL) {
        return 0;
    }
    
    unsigned int loglevel;
    flockfile(out);
    LogMap::const_iterator it = log_map.find(category);
    
    if (it == log_map.end()) {
#ifdef VS_LOG_NO_XML
    	if (log_level_default == -1) {
    		log_level_default = VSLOG_LEVEL_DEFAULT;
    		log_colorize = -1;
    	}
    	loglevel = log_level_default;
#else
    	if (vs_config == NULL) {
            if (log_level_default == -1) {
            	loglevel = VSLOG_LEVEL_DEFAULT;
                log_colorize = -1;
            } else {
            	loglevel = log_level_default;
            }
            store = false;
        } else {
            if (log_level_default == -1) {

                log_level_default = log_level_byname(vs_config->getVariable("log","default_level",
                                                                            STR2(VSLOG_LEVEL_DEFAULT)).c_str());
                std::string str_colorize = vs_config->getVariable("log","colorize", "auto");
                if (str_colorize == "no") log_colorize = 0;
                else if (str_colorize == "yes") log_colorize = 1;
                else log_colorize = -1;
                vslog_out_istty = isatty(fileno(vslog_out));
            }
            loglevel = log_level_byname(
                                        vs_config->getVariable("log/modules",category,
                                        XMLSupport::tostring(log_level_default)).c_str());
        }
#endif
        if (store) {
            log_map.insert(std::make_pair(category, loglevel));
        }
        
        if (store || log_level_default >= 0) {
        	VS_LOG("log", logvs::NOTICE, "%s log entry '%s' = %s (%u)",
        		   store ? "creating" : "getting",
                   category.c_str(), log_level_name(loglevel), loglevel);
    	}
    } else {
        loglevel = it->second;
    }
    funlockfile(out);
    return loglevel;
}

int vs_log_header(const std::string & category, unsigned int level, unsigned int flags,
                  const char * file, const char * func, int line, const char * fmt, ...) {
    int ret = 0, n;
    FILE * out;
    
    if (vslog_out == NULL) {
        return 0;
    }
    
    flockfile((out = vslog_out));
    if (level > 0 && (flags & logvs::F_NO_LVL_CHECK) == 0) { // no need to use the hashmap if requested level is 0 (always displayed)
        unsigned int loglevel = vs_log_level(category, true);
        
        if (loglevel < level) {
            funlockfile(out);
            return 0;
        }
    }
    
#ifndef _WIN32
    const char * const color = log_colorize &&
        (vslog_out_istty||log_colorize==1) ? log_level_color[level > logvs::DBG ? logvs::DBG : level] : "";
    const char * const color_reset = *color ? log_level_color_reset : "";
#else
    static const char * const color = "", * color_reset = "";
#endif

    if (((flags | log_flags) & logvs::F_TIMESTAMP) != 0 && (n = fprintf(out, "%.03lf ", logGetElapsedTime())) > 0) {
        ret += n;
    }
    if ((n = fprintf(out, "[%s%s%s] ", color, category.c_str(), color_reset)) > 0) {
        ret += n;
        flags = (flags | log_flags) & (~F_LOCATION_FOOTER);
        ret += log_print_location(out, flags, category, level, file, func, line);
        
        if (fmt != NULL) {
            va_list valist;
            va_start(valist, fmt);
            ret += vfprintf(out, fmt, valist);
            va_end(valist);
            vs_log_msgcenter_v(category, level, flags | log_flags, false, file, func, line, fmt, valist);
        }
    }
    
    if ((flags & logvs::F_AUTO_LOCK) == 0)
        funlockfile(out);
    
    return ret;
}

int vs_log_footer(const std::string & category, unsigned int level, unsigned int flags,
                  const char * file, const char * func, int line,
                  const char * fmt, ...)
{
    (void) category;
    FILE * out = vslog_out;
    int ret = 0;
    va_list valist;
    
    flags = (flags | log_flags) & (~F_LOCATION_HEADER);
    if (fmt != NULL) {
        va_start(valist, fmt);
        ret += vfprintf(out, fmt, valist);
        va_end(valist);
        vs_log_msgcenter_v(category, level, flags, false, file, func, line, fmt, valist);
    }
    
    ret += log_print_location(out, flags, category, level, file, func, line);
    
    ret += fprintf(out, "\n");
    
    vs_log_msgcenter(category, level, flags, true, file, func, line, "");//(fmt=NULL), valist);

    if ((flags & logvs::F_AUTO_LOCK) != 0) {
        funlockfile(out);
    }
    return ret;
}

int vs_log(const std::string & category, unsigned int level, unsigned int flags,
           const char * file, const char * func, int line, const char * fmt, ...) {
    int ret;
    
    if ((ret = vs_log_header(category, level, flags | logvs::F_AUTO_LOCK, file, func, line, NULL)) <= 0) {
        return 0;
    }
    
    if (fmt != NULL) {
        va_list valist;
        va_start(valist, fmt);
        ret += vfprintf(vslog_out, fmt, valist);
        va_end(valist);
        vs_log_msgcenter_v(category, level, flags | log_flags, false, file, func, line, fmt, valist);
    }
    
    return ret + vs_log_footer(category, level, flags | logvs::F_AUTO_LOCK, file, func, line, NULL);
}

int vs_printf(const char * fmt, ...) {
    if (vslog_out == NULL || fmt == NULL) {
        return 0;
    }
    int ret;
    va_list valist;
    va_start(valist, fmt);
    ret = vfprintf(vslog_out, fmt, valist);
    va_end(valist);
    vs_log_msgcenter_v("", 0, log_flags, false, "", "", 0, fmt, valist);
    return ret;
}

} // ! namespace vslog
/*------------------------------------------------------------------------------------------*/

