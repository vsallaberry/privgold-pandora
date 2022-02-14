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

/*
 * CONFIG
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#elif ! defined(_WIN32) && !defined(HAVE_STRCASECMP)
#define HAVE_STRCASECMP
#endif

/*
 * STD HEADERS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <queue>

/*
 * WIN32 Specific headers
 */
#if defined(_WIN32)
# define flockfile(f)   _lock_file(f)
# define funlockfile(f) _unlock_file(f)
# if defined(__CYGWIN__) || defined(__MINGW32__)
#  include <io.h>
#  include <sys/time.h>
# endif
#else
# include <unistd.h>
# include <sys/time.h>
#endif

/*
 * Specific cpp file headers
 */
#include "log.h"

#include "gnuhash.h"

/*
 * Detect if we are building vegastrike and include
 * specific stuff such as configxml, msgcenter, getNewTime, ...
 */
#ifndef VS_LOG_NO_XML
# include "xml_support.h"
# include "configxml.h"
# include "cmd/script/msgcenter.h"
# include "lin_time.h"
extern VegaConfig * vs_config;
extern Mission * mission;
#define VS_LOG_USE_LOOP_TIME
#else
# undef  VS_LOG_USE_LOOP_TIME
# undef  VS_LOG_NO_MSGCENTER
# define VS_LOG_NO_MSGCENTER
#endif


/*
 * logGetTime() function
 */
#if defined(VS_LOG_USE_LOOP_TIME)
static inline double logGetTime() {
    double time = getNewTime();
    if (time == 0.0) { time = queryTime(); } // 0 means loop is not running yet --> force query
    return time;
}
#elif !defined(VS_LOG_NO_XML)
# define logGetTime() queryTime()
#elif defined(HAVE_GETTIMEOFDAY)
static double logGetTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    static double origtime = time.tv_sec + (time.tv_usec/1000000.0);
    return (time.tv_sec + (time.tv_usec/1000000.0)) - origtime;
}
#elif defined(HAVE_SDL)
# include <SDL_timer.h>
static double logGetTime() {
    return SDL_GetTicks() * 1.e-3;
}
#else
#if !defined(_WIN32)
# include <sys/time.h>
#endif
# include <time.h>
static double logGetTime() {
    static time_t origtime = time(NULL);
    return time(NULL) - origtime;
}
#endif

/*------------------------------------------------------------------------------------------*/
namespace logvs {

#define VSLOG_LEVEL_DEFAULT 3
#define STR(x) #x //convert argument into string
#define STR2(x) STR(x) // resolve the macro value before stringifying it

struct LogEntry {
    std::string message;
    std::string category;
    unsigned int level;
    unsigned int flags;
    double timestamp;
    LogEntry(const std::string & _category, unsigned int _level, unsigned int _flags, double _timestamp, const std::string & _message)
        : message(_message), category(_category), level(_level), flags(_flags), timestamp(_timestamp) {}
};

typedef vsUMap<std::string, unsigned int> LogMap;
typedef std::queue<LogEntry> LogQueue;

static unsigned int s_log_flags = F_DEFAULTS; // | F_LOCATION_ALLWAYS;
static LogMap       s_log_map;
static LogQueue     s_log_queue;
static FILE *       s_log_out = stderr;
static bool         s_log_redirected = false;
static int          s_log_stdout_fd = -1;
static int          s_log_stderr_fd = -1;
static bool         s_log_out_istty = false;
static int          s_log_level_default = -1;
static int          s_log_colorize = -1;

static const char * s_log_level_names[logvs::LVL_NB+1] = {
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
static const char * const s_log_level_color_reset = "\e[00m";
static const char * const s_log_level_color[logvs::LVL_NB] = {
    "",
    "\e[01;31m",
    "\e[01;33m",
    "\e[00;32m",
    "\e[00;34m",
    "\e[00;36m",
    "\e[00;35m"
};
#endif

#define VSLOG_MSGCENTER_ON(flags) (((flags) & (logvs::F_MSGCENTER | logvs::F_QUEUELOGS)) != 0)
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
    static char buf[4096] = { 0, }; //it is ok to use static because it is under FILE* vslog_out lock which is unique
    static size_t off = 0;
    int ret;
    if (fmt != NULL && (ret = vsnprintf(buf+off, sizeof(buf)-off, fmt, valist)) > 0) {
        off += ret;
        if (off > sizeof(buf))
            off = sizeof(buf);
    }
    bool eol = (!islast && off > 0 && buf[off-1] == '\n');
    if (islast || off == sizeof(buf) || eol) {
        std::string message(buf, off - (eol ? 1 : 0));
        if ((flags & F_QUEUELOGS) != 0) {
            s_log_queue.push(LogEntry(category, level, (flags | s_log_flags), logGetTime(), message));
        }
#ifndef VS_LOG_NO_MSGCENTER
        if ((flags & F_MSGCENTER) != 0 && mission != NULL && mission->msgcenter != NULL) {
            mission->msgcenter->add("log/" + category, "all", message);
        }
#endif // ! VS_LOG_NO_MSGCENTER
        off = 0;
        *buf = 0;
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

#ifdef VS_LOG_NO_XML
void log_setlevel(const std::string & category, unsigned int level) {
	if (category.empty()) {
		s_log_level_default = level;
	} else {
		s_log_map.insert(std::make_pair(category, level));
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
    return s_log_level_names[level < logvs::LVL_NB ? level : logvs::LVL_NB];
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
        if (!strcasecmp(name, s_log_level_names[i])) {
            return i;
        }
    }

    return s_log_level_default > 0 ? s_log_level_default : VSLOG_LEVEL_DEFAULT;
}

FILE * log_getfile(const std::string & module) {
    (void)module;
    return s_log_out;
}

size_t log_flushqueue() {
    log_setflag(F_QUEUELOGS, false);
    unsigned int saved_flags = s_log_flags;
    size_t queue_sz = s_log_queue.size();
    while (s_log_queue.size()) {
        log_setflags(s_log_queue.front().flags & ~F_QUEUELOGS);
        log(s_log_queue.front().category, s_log_queue.front().level, F_NONE, 
             __FILE__, __func__, __LINE__, "%s", s_log_queue.front().message.c_str());
        s_log_queue.pop();
    }
    if (s_log_out != NULL && queue_sz > 0) {
        fflush(s_log_out);
    }
    log_setflags(saved_flags);
    return queue_sz;
}

FILE * log_setfile(FILE * out, const std::string & module) {
    (void)module;
    FILE * old = s_log_out;
    if (out != NULL) flockfile(out);
    if (old != NULL) {
        flockfile(old);
        fflush(old);
        if (old == stdout) fflush(stderr);
        else if (old == stderr) fflush(stdout);
    }
    s_log_out = out;
    s_log_out_istty = isatty(fileno(out));

    bool same_file = out == old || ((out == stderr || out == stdout) && (old == stderr || old == stdout));
    if (out != NULL && (!same_file || s_log_redirected)) {
        log_flushqueue();
    }

    if (out != NULL) funlockfile(out);
    if (old != NULL) funlockfile(old);
    return old;
}

unsigned int log_setflag(unsigned int flag, bool value) {
    unsigned int oldflags = s_log_flags;
    if (value) {
        s_log_flags |= flag;
    } else {
        s_log_flags &= (~(flag));
    }
    return oldflags;
}

unsigned int log_setflags(unsigned int flags) {
    unsigned int oldflags = s_log_flags;
    s_log_flags = flags;
    return oldflags;
}

unsigned int log_level(const std::string & category, bool store) {
    FILE * out = s_log_out;
    if (out == NULL) {
        return 0;
    }

    unsigned int loglevel;
    flockfile(out);
    LogMap::const_iterator it = s_log_map.find(category);

    if (it == s_log_map.end()) {
#ifdef VS_LOG_NO_XML
    	if (s_log_level_default == -1) {
    		s_log_level_default = VSLOG_LEVEL_DEFAULT;
    		s_log_colorize = -1;
    	}
    	loglevel = s_log_level_default;
#else
    	if (vs_config == NULL) {
            if (s_log_level_default == -1) {
            	loglevel = VSLOG_LEVEL_DEFAULT;
                s_log_colorize = -1;
            } else {
            	loglevel = s_log_level_default;
            }
            store = false;
        } else {
            if (s_log_level_default == -1) {

                s_log_level_default = log_level_byname(vs_config->getVariable("log","default_level",
                                                                              STR2(VSLOG_LEVEL_DEFAULT)).c_str());
                std::string str_colorize = vs_config->getVariable("log","colorize", "auto");
                if (str_colorize == "no") s_log_colorize = 0;
                else if (str_colorize == "yes") s_log_colorize = 1;
                else s_log_colorize = -1;
                s_log_out_istty = isatty(fileno(s_log_out));
            }
            loglevel = log_level_byname(
                                        vs_config->getVariable("log/modules",category,
                                        XMLSupport::tostring(s_log_level_default)).c_str());
        }
#endif
        if (store) {
            s_log_map.insert(std::make_pair(category, loglevel));
        }

        if (store || s_log_level_default >= 0) {
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

int log_header(const std::string & category, unsigned int level, unsigned int flags,
               const char * file, const char * func, int line, const char * fmt, ...) {
    int ret = 0, n;
    FILE * out;

    if (s_log_out == NULL) {
        return 0;
    }

    flockfile((out = s_log_out));
    if (level > 0 && (flags & logvs::F_NO_LVL_CHECK) == 0) { // no need to use the hashmap if requested level is 0 (always displayed)
        unsigned int loglevel = log_level(category, true);

        if (loglevel < level) {
            funlockfile(out);
            return 0;
        }
    }
    if (((s_log_flags | flags) & logvs::F_HEADER) == 0) {
        if ((flags & logvs::F_AUTO_LOCK) == 0) // done by log_footer() if flag ON
            funlockfile(out);
        return 1; //return 0 would cause the log to be discarded.
    }

#ifndef _WIN32
    const char * const color = s_log_colorize &&
        (s_log_out_istty||s_log_colorize==1) ? s_log_level_color[level > logvs::DBG ? logvs::DBG : level] : "";
    const char * const color_reset = *color ? s_log_level_color_reset : "";
#else
    static const char * const color = "", * color_reset = "";
#endif

    if (((flags | s_log_flags) & logvs::F_TIMESTAMP) != 0 && (n = fprintf(out, "%.03lf ", logGetTime())) > 0) {
        ret += n;
    }
    if ((n = fprintf(out, "[%s%s%s] ", color, category.c_str(), color_reset)) > 0) {
        ret += n;
        flags = (flags | s_log_flags) & (~F_LOCATION_FOOTER);
        ret += log_print_location(out, flags, category, level, file, func, line);

        if (fmt != NULL) {
            va_list valist;
            va_start(valist, fmt);
            ret += vfprintf(out, fmt, valist);
            va_end(valist);
            vs_log_msgcenter_v(category, level, flags | s_log_flags, false, file, func, line, fmt, valist);
        }
    }

    if ((flags & logvs::F_AUTO_LOCK) == 0) // done by log_footer() if flag ON
        funlockfile(out);

    return ret;
}

int log_footer(const std::string & category, unsigned int level, unsigned int flags,
               const char * file, const char * func, int line,
               const char * fmt, ...)
{
    (void) category;
    FILE * out = s_log_out;
    int ret = 0;
    va_list valist;

    if (out == NULL) {
        return 0;
    }

    flags = (flags | s_log_flags) & (~F_LOCATION_HEADER);
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

int log(const std::string & category, unsigned int level, unsigned int flags,
        const char * file, const char * func, int line, const char * fmt, ...) {
    int ret;

    if ((ret = log_header(category, level, flags | logvs::F_AUTO_LOCK, file, func, line, NULL)) <= 0) {
        return 0;
    }

    if (fmt != NULL) {
        va_list valist;
        va_start(valist, fmt);
        ret += vfprintf(s_log_out, fmt, valist);
        va_end(valist);
        vs_log_msgcenter_v(category, level, flags | s_log_flags, false, file, func, line, fmt, valist);
    }

    return ret + log_footer(category, level, flags | logvs::F_AUTO_LOCK, file, func, line, NULL);
}

int log_printf(const char * fmt, ...) {
    if (s_log_out == NULL || fmt == NULL) {
        return 0;
    }
    int ret;
    va_list valist;
    va_start(valist, fmt);
    ret = vfprintf(s_log_out, fmt, valist);
    va_end(valist);
    vs_log_msgcenter_v("", 0, s_log_flags, false, "", "", 0, fmt, valist);
    return ret;
}

static int log_disable_redirection() {
    if (s_log_stdout_fd >= 0) {
        if (dup2(s_log_stdout_fd, fileno(stdout)) < 0) {
            VS_LOG("log", logvs::WARN, "Warning: cannot restore stdout");
        }
        close(s_log_stdout_fd);
        s_log_stdout_fd = -1;
    }
    if (s_log_stderr_fd >= 0) {
        if (dup2(s_log_stderr_fd, fileno(stderr)) < 0) {
            VS_LOG("log", logvs::WARN, "Warning: cannot restore stderr");
        }
        close(s_log_stderr_fd);
        s_log_stderr_fd = -1;
    }
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);  /* Line buffered. FULL buffered: _IOFBF */
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);  /* Line buffered. FULL buffered: _IOFBF */
    s_log_redirected = false;
    return 0;
}

int log_openfile(const std::string & module, 
                 const std::string & filename, bool redirect, bool append) {
    (void)module;
    char buftime[50] = { 0, };
    time_t tm = time(NULL);
    const char * stime = ctime(&tm);

    if (stime != NULL) {
        size_t i;
        for(i = 0; i+1 < sizeof(buftime) && stime[i] != 0 && stime[i] != '\n'; ++i) 
            buftime[i] = stime[i];
        buftime[i] = 0;
    }
    VS_LOG("log", logvs::NOTICE, "(%s) Logging to %s", buftime, filename.c_str());
    
    FILE * logout;
    s_log_redirected = false;
    if (filename == "") {
        logout = NULL;
    } else if (strcasecmp(filename.c_str(), "stdout") == 0) {
        logout = stdout;
    } else if (strcasecmp(filename.c_str(), "stderr") == 0) {
        logout = stderr;
    } else {
        if (!redirect) {
            logout = fopen(filename.c_str(), append ? "a" : "w");
        } else {
        	if (s_log_stderr_fd < 0)
        		s_log_stderr_fd = dup(fileno(stderr));
            if (s_log_stdout_fd < 0)
            	s_log_stdout_fd = dup(fileno(stdout));
            if (!append && (logout = fopen(filename.c_str(), "w")) != NULL) {
                fclose(logout); // erase the file before freopen() because if stderr is redirected its file will be erased.
            }
            if ((logout = freopen(filename.c_str(), "a", stderr)) == NULL) {
                if ((logout = fopen(filename.c_str(), append ? "a" : "w")) != NULL) {
                	fflush(stderr);
                    if (dup2(fileno(logout), fileno(stderr)) < 0) {
                		VS_LOG("log", logvs::WARN, "Warning: cannot redirect stderr");
#ifdef _WIN32
                        *stderr = *logout;
#endif
                    }
                	setvbuf(stderr, NULL, _IOLBF, BUFSIZ); // line bufferred for stderr if no freopen
                }
            }
            if (logout != NULL) {
                s_log_redirected = true;
                fflush(stdout);
                if (dup2(fileno(logout), fileno(stdout)) < 0) {
                	VS_LOG("log", logvs::WARN, "Warning: cannot redirect stdout");
#ifdef _WIN32
                        *stdout = *logout;
#endif
                }
                setvbuf(stdout, NULL, _IOLBF, BUFSIZ); // line buffered for stdout
            }
        }
        if (logout == NULL) {
            VS_LOG("log", logvs::WARN, "error while opening logfile %s, using stderr.", filename.c_str());
            logout = stderr;
        }
    }
    if (!s_log_redirected) {
        log_disable_redirection();
    }
    if (logout != NULL) {
        if ((logout != stderr && logout != stdout) || s_log_redirected) {
            const int bufsz = 8192; //BUFSIZ;
            const int iobuffering = _IOFBF; // _IOFBF: full buffered, _IOLBF: line buffered, setbuf(file,NULL):no buffering.
            setvbuf(logout, NULL, iobuffering, bufsz);
        } else {
            setvbuf(stdout, NULL, _IOLBF, BUFSIZ);  /* Line buffered. FULL buffered: _IOFBF */
            setvbuf(stderr, NULL, _IOLBF, BUFSIZ);  /* Line buffered. FULL buffered: _IOFBF */
        }
    }

    VS_LOG("log", logvs::VERBOSE, "%s(): logout=%p/%d (old=%p/%d) stdout=%p/%d stderr=%p/%d saveout=%d saveerr=%d",
    		__func__, logout, logout?fileno(logout):-1, s_log_out, s_log_out?fileno(s_log_out):-1,
    		stdout, fileno(stdout), stderr, fileno(stderr), s_log_stdout_fd, s_log_stderr_fd);

    logvs::log_setflag(logvs::F_QUEUELOGS, false);
    logvs::log_setfile(logout);
    return (s_log_out != NULL);
}

void log_terminate() {
#if !defined(VS_LOG_NO_XML)
	if (vs_config == NULL && !VSFileSystem::homedir.empty()) {
		log_openfile("", ((VSFileSystem::homedir + VSFS_PATHSEP) + "vegastrike.log").c_str(), false, false);
	}
#endif
    fflush(stdout);
    fflush(stderr);
    VS_LOG("log", logvs::INFO, "%s() %p stderr=%p stdout=%p", __func__, s_log_out, stderr, stdout);
    FILE * old = log_setfile(stderr);
    if (old != NULL && old != stdout && old != stderr) {
        fclose(old);
    }
    log_disable_redirection();
}

} // ! namespace vslog
/*------------------------------------------------------------------------------------------*/

