/*
* Vega Strike
* Copyright (C) 2021-2022 Vincent Sallaberry
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
#ifndef VS_LOG_H
#define VS_LOG_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/*---------------------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>

// wingdi.h ERROR macro conflict
#if defined(_WIN32)
# include <windows.h> 
# if defined(ERROR)
#  define WIN_ERROR ERROR
#  undef ERROR
# endif
#endif

#define VS_LOG_CHECK_BEFORE_CALL

//
// LOG macros
// checking requested level against category level(xml)
//

#ifdef VS_LOG_CHECK_BEFORE_CALL
//
// Here, the level is checked before vs_log() call,
// and the parameters are not evaluated.
//

//
// VS_LOG(category, level, format, ...)
//
# define VS_LOG(category, level, ...)                                       \
\
            ((level) <= logvs::log_level(category)                          \
            ? logvs::log(category, level, logvs::F_NO_LVL_CHECK,            \
                         __FILE__, __func__, __LINE__, __VA_ARGS__) : 0)

//
// VS_LOG_START(category, level, format, ...)
//
//    This MUST have a corresponding VS_LOG_END().
//
# define VS_LOG_START(category, level, ...)                                 \
\
    ((level) <= logvs::log_level(category)                                  \
    ? logvs::log_header(category, level, logvs::F_AUTO_LOCK | logvs::F_NO_LVL_CHECK, \
                        __FILE__, __func__, __LINE__, __VA_ARGS__) : 0)


#else
// not checking level before call, all arguments evualuated

//
// VS_LOG(category, level, format, ...)
//
# define VS_LOG(category, level, ...)                                       \
\
            logvs::log(category, level, logvs::F_NONE,                   \
                       __FILE__, __func__, __LINE__, __VA_ARGS__)

//
// VS_LOG_START(category, level, format, ...)
//
//    This MUST have a corresponding VS_LOG_END().
//
# define VS_LOG_START(category, level, ...)                                 \
            logvs::log_header(category, level, logvs::F_AUTO_LOCK,       \
                              __FILE__, __func__, __LINE__, __VA_ARGS__)

#endif

//
// VS_LOG_CACHED(allowed, category, level, format, ...)
//
// cached debug log macro,
// 'allowed' value can be retrieved with logvs::vs_log_level(category)
//
#define VS_LOG_CACHED(allowed, category, level,...) \
\
    ((level) <= (allowed) ? logvs::log(category, level, logvs::F_NO_LVL_CHECK, \
                                       __FILE__, __func__, __LINE__, __VA_ARGS__) : 0)


//
// VS_LOG_START_CACHED(allowed, category, level, format, ...)
//
//    This MUST have a corresponding VS_LOG_END().
//
#define VS_LOG_START_CACHED(allowed, category, level, ...)                  \
\
    ((level) <= (allowed)                                                   \
    ? logvs::log_header(category, level, logvs::F_AUTO_LOCK | logvs::F_NO_LVL_CHECK, \
                        __FILE__, __func__, __LINE__, __VA_ARGS__) : 0)

//
// VS_LOG_END(category, level, format, ...)
//
//    This MUST have a previous corresponding VS_LOG_START*().
//
#define VS_LOG_END(category, level, ...)                            \
\
    logvs::log_footer(category, level, logvs::F_AUTO_LOCK,       \
                      __FILE__, __func__, __LINE__, __VA_ARGS__)

//
// DEBUG MACROS
//
#if defined(VS_DEBUG_LOG)
// DEBUG LOG macros, disabled if VS_DEBUG_LOG not defined

// classic debug log macro
# define VS_DBG(category, level, ...)   VS_LOG(category, level, __VA_ARGS__)

// cached debug log macro, disabled if VS_DEBUG_LOG not defined.
// 'allowed' value can be retrieved with logvs::vs_log_level(category)
# define VS_DBG_CACHED(allowed, category, level, ...) \
\
            VS_LOG_CACHED(allowed, category, level, __VA_ARGS__)

#else
// DEBUG LOG macros DISABLED, do nothing
namespace logvs { inline int logdummy() { return 0;} };
# define VS_DBG(category, level, ...)                   logvs::logdummy()
# define VS_DBG_CACHED(allowed, category, level, ...)   logvs::logdummy()
#endif

//
// MODULES HELPER MACROS
// Using VS_{LOG,DBG}_CACHED with predefined cached variables and functions
//
#define VSLOG_STR(macro) #macro
#define VSLOG_CACHE_FUN_NAME(name) \
    vs_log_level_for_##name
#define VSLOG_CACHE_FUN_PROTO(name) \
    extern unsigned int VSLOG_CACHE_FUN_NAME(name)()
#define VSLOG_DECL_CACHE(name) \
	namespace logvs { VSLOG_CACHE_FUN_PROTO(name); };
#define VSLOG_DEF_CACHE(name) \
	namespace logvs { VSLOG_CACHE_FUN_PROTO(name) { \
		static const unsigned int level = logvs::log_level(VSLOG_STR(name)); return level; \
	} };

#ifdef VS_LOG_NO_XML
# define VS_LOG_CACHED_LEVEL(name) logvs::VSLOG_CACHE_FUN_NAME(name)()
#else
class VegaConfig;
extern VegaConfig * vs_config;
# define VS_LOG_CACHED_LEVEL(name) \
    ((vs_config == NULL) ? (logvs::log_level(VSLOG_STR(name))) \
                         : (logvs::VSLOG_CACHE_FUN_NAME(name)()))
#endif

#define VS_LOG_M(name, level, ...) \
	VS_LOG_CACHED(VS_LOG_CACHED_LEVEL(name), VSLOG_STR(name), level, __VA_ARGS__)

#define VS_DBG_M(name, level, ...) \
	VS_DBG_CACHED(logvs::VSLOG_CACHE_FUN_NAME(name)(), VSLOG_STR(name), level, __VA_ARGS__)

#define VS_LOG_START_M(name, level, ...) \
	VS_LOG_START_CACHED(logvs::VSLOG_CACHE_FUN_NAME(name)(), VSLOG_STR(name), level, __VA_ARGS__)

#define VS_LOG_END_M(name, level, ...) \
	VS_LOG_END(VSLOG_STR(name), level, __VA_ARGS__)

// ***********************************************************************

namespace logvs {
    enum Levels {
        NONE = 0,
        ERROR,
        WARN,
        NOTICE,
        INFO,
        VERBOSE,
        DBG,
        LVL_NB /* last */
    } ;
    enum Flags {
        F_NONE            = 0,
        F_AUTO_LOCK       = 1 << 0,
        F_NO_LVL_CHECK    = 1 << 1,
        F_LOCATION_ALLWAYS= 1 << 2,
        F_LOCATION_HEADER = 1 << 3,
        F_LOCATION_FOOTER = 1 << 4,
        F_MSGCENTER       = 1 << 5,
        F_TIMESTAMP       = 1 << 6,
        F_HEADER          = 1 << 7,
        F_QUEUELOGS       = 1 << 8,
        F_FOOTER          = 1 << 9,
        F_LOCATION_MASK = F_LOCATION_HEADER | F_LOCATION_FOOTER,
        F_DEFAULTS = F_HEADER | F_FOOTER | F_LOCATION_FOOTER,
    };
    
    /** returns number of bytes written or 0 on error or when log is disabled */
    int log(const std::string & category, unsigned int level, unsigned int flags,
            const char * file, const char * func, int line,
            const char *fmt, ...) __attribute__((format(printf, 7, 8)));

    /** returns the allowed level for a given category (check in XML and cache it if store is true) */
    unsigned int log_level(const std::string & category, bool store = true);

    /** get current logging file */
    FILE * log_getfile(const std::string & module = "");

    /** changes logging file, returns old one */
    FILE * log_setfile(FILE * out, const std::string & module = "");

    /** update log flags, return old ones */
    unsigned int log_setflag(unsigned int flag, bool value);
    unsigned int log_setflags(unsigned int flags);

    /** Starts a log line */
    int log_header(const std::string & category, unsigned int level, unsigned int flags,
                   const char * file, const char * func, int line,
                   const char * fmt, ...) __attribute__((format(printf, 7, 8)));

    /** printf like in the log file */
    int log_printf(const char * fmt, ...) __attribute__((format(printf, 1, 2)));

    /** terminates a log line started with log_header */
    int log_footer(const std::string & category, unsigned int level, unsigned int flags,
                   const char * file, const char * func, int line,
                   const char * fmt, ...) __attribute__((format(printf, 7, 8)));

    const char * log_level_name(unsigned int level);
    unsigned int log_level_byname(const char * name);

    /** Set log to given file. Special values: stdout, stderr.
      * After this call the log flag F_QUEUELOGS is reset to false. */
    int log_openfile(const std::string & module, 
                     const std::string & filename, bool redirect, bool append);

    /** close & flush logs, trying to go back to initial setup. To be used with atexit() */
    void log_terminate();

#ifdef VS_LOG_NO_XML
    void log_setlevel(const std::string & category, unsigned int level);
#endif
}

/*---------------------------------------------------------------------------------------*/

#endif
