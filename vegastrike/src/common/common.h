/***************************************************************************
                          common.h  -  description
                             -------------------
    begin                : Wed Jun 26 2002
    copyright            : (C) 2002 by jhunt
    email                : jhunt@jaja
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
 * Copyright (C) 2022 Vincent Sallaberry
 * vegastrike common for vegastrike(GPL), its setup, launcher or more.
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
 */
/* This file is for common (as in shared between vegastrike, vssetup as vsconfig) stuff
 * that is not import enought to warrant it's own file.
 */
#ifndef _VS_COMMON_COMMON_H
#define _VS_COMMON_COMMON_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string>
#include <time.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#if defined(_WIN32)
# include <windows.h>
#else
# include <dirent.h>
#endif
#include <sys/stat.h>

#ifndef HAVE_CODECVT // not CXX11
# define static_assert(x,y)
#endif

namespace VSCommon {

// stores the paths search lists
extern const char * const datadirs[];
extern const char * const resourcessearchs[];

// patch separator
extern const char * const pathsep;


// **************************************************************************
// wrappers
// **************************************************************************

// unistd wrappers mainly for windows
int 	vs_setenv(const char * var, const char * value, int override);
int		vs_mkdir(const char * path, mode_t mode);
FILE * 	vs_fopen(const char * path, const char * mode);
FILE * 	vs_freopen(const char * path, const char * mode, FILE * fp);
char *	vs_getcwd(char * path, size_t size);
int 	vs_chdir(const char * path);
int 	vs_stat(const char * path, struct stat * st);

inline int vs_mkdir(const std::string & dir, mode_t mode) { return vs_mkdir(dir.c_str(), mode); }

#if !defined(_WIN32)
typedef ::DIR vsDIR;
typedef struct ::dirent vsdirent;
#else
struct vsdirent { char * d_name; HANDLE h; bool first; };
typedef vsdirent vsDIR;
#endif

int 			vs_closedir(vsDIR * dirp);
vsDIR *			vs_opendir(const char * path);
inline vsDIR *	vs_opendir(const std::string & path) { return vs_opendir(path.c_str()); }
vsdirent * 		vs_readdir(vsDIR * dirp);


// **************************************************************************
// Game Directory Management
// **************************************************************************

/**
 * getresourcesdir()
 *   Returns where the resources directory is. Returns "" if it can't be found.
 *   Resources contains share (termiinfo,alsa,lib/python-libs).
 *   Note: if base is given, it will search into first, then in current directory.
 *         current directory is not changed
 */
std::string 		getresourcesdir(const char * base = NULL);
inline std::string 	getresourcesdir(const std::string & base) { return getresourcesdir(base.c_str()); }

/**
 * getdatadir()
 *   Returns where the data directory is. Returns "" if it can't be found.
 *   Note: if base is given, it will search into first, then in current directory.
 *         current directory is not changed
 */
std::string 		getdatadir(const char * base = NULL);
inline std::string 	getdatadir(const std::string & base) { return getdatadir(base.c_str()); }

/**
 * gethomedir()
 *   Returns where the home directory is (first), and the datadir homesubdir(second).
 *   first will be "" if it can't find the home dir.
 *   Note: cwd is not changed, but directory is created if not existing.
 */
std::pair<std::string,std::string> 			gethomedir(const char * base = NULL);
inline std::pair<std::string,std::string>	gethomedir(const std::string & base) {
												return gethomedir(base.c_str()); }

/**
 * getfiledir()
 *   Return a pair <file_dir,file_name> for argv0, without changing current directory
 */
std::pair<std::string,std::string>			getfiledir(const char *argv0,         const char * base = NULL);
inline std::pair<std::string,std::string> 	getfiledir(const std::string & argv0, const std::string & base = "") {
												return getfiledir(argv0.c_str(), base.empty() ? NULL : base.c_str()); }

/**
 * getsuffixedfile()
 *   Add an integer suffix to file if existing and recently updated.
 *    delay_sec== (time_t)-1 to allways add suffix, maxfiles==0 for unlimited files.
 */
std::string 		getsuffixedfile(const char * file,        time_t delay_sec = 5, unsigned int maxfiles = 10);
inline std::string 	getsuffixedfile(const std::string & file, time_t delay_sec = 5, unsigned int maxfiles = 10) {
						return getsuffixedfile(file.c_str(), delay_sec, maxfiles); }

#if defined(_WIN32)
/**
 * win32_get_appdata()
 *   Get user LocalAppData folder
 */
HRESULT win32_get_appdata(WCHAR * wappdata);
#endif // ! _WIN32


// **************************************************************************
// Console / Terminal Utilities
// **************************************************************************

/**
 * InitConsole()
 *   Essentially for windozws applications built in GUI mode (-mwindows)
 *   This will attach application to console if it is run from it.
 *   On macOS, linux, unix, the libc handles it automatically (except the forcealloc==true).
 *   The argc and argv are needed if forcealloc==true, and the current directory must be as initially.
 */
bool InitConsole(bool forcealloc = false, int argc = 0, char ** argv = NULL);


// **************************************************************************
// File Utilities
// **************************************************************************

// -- file comparisons
typedef enum {
	FIT_UNKNOWN = 0, FIT_type = 1 << 0,
	FIT_FILE = FIT_type, FIT_DIR = FIT_type + 1, FIT_LINK = FIT_type + 2,
	FIT_WRITE = 1 << 2, /*type_mask*/ FIT_TYPE_MASK = (FIT_WRITE - 1) & ~(FIT_type - 1),
} file_info_type_t;

static_assert(FIT_LINK < FIT_WRITE, "Not enough space for FIT_type, FIT_WRITE must be increased");

typedef struct {
    uint64_t dev;
    uint64_t ino;
    uint64_t size;
    time_t mtime, atime, ctime, btime; /*modif,access,changed,birth*/
    unsigned int type;
} file_info_t;

/**
 * getFileInfo()
 *   return false on error
 */
bool 		getFileInfo(const char * file,        file_info_t * id);
inline bool	getFileInfo(const std::string & file, file_info_t * id) {
				return getFileInfo(file.c_str(), id); }

/**
 * fileIdCompare()
 *   return 0 if file are equal (dev/inode equal), < 0 if id<other, >0 otherwise.
 */
ssize_t fileIdCompare(file_info_t * id, file_info_t * other);

/**
 * fileCompare()
 *   return true if equal, false otherwise
 */
bool 		fileCompare(const char * src,        const char * dst);
inline bool	fileCompare(const std::string & src, const std::string & dst) {
				return fileCompare(src.c_str(), dst.c_str()); }

/**
 * fileCopyIfDifferent()
 *   return one of FileCompareResult. call with depth==(size_t)-1 for full recursive.
 *     success is greater or equal 0, error is negative.
 *
 */
typedef enum { FILECOMP_EQUAL = 0, FILECOMP_REPLACED = 1,
               FILECOMP_DIFF = -1, FILECOMP_ERROR = -2, FILECOMP_SRCNOTFOUND = -3} FileCompareResult;
int			fileCopyIfDifferent(const char * src,        const char * dst,        size_t depth = 0);
inline int	fileCopyIfDifferent(const std::string & src, const std::string & dst, size_t depth = 0) {
				return fileCopyIfDifferent(src.c_str(), dst.c_str(), depth); }

/**
 * VS_PATH_JOIN(()
 *   VS_PATH_JOIN("dir1", ..., "dirN") -> "dir1/.../dirN"
 */
#define VS_PATH_JOIN(...) VSCommon::path_join(__VA_ARGS__, NULL)
// Not to use directly: it is better to call VS_PATH_JOIN as it adds a NULL last parameter.
std::string path_join(const char * first, ...); // all parameters must be char*, last one must be NULL.

/**
 * ParseCmdLine() / ParseCmdLineFree()
 *   Translate a command line string to argc, argv (initially written for WinMain).
 *   Unless WCMDF_ARGV0_FROM_CMDLINE is On, argv[0] is not taken from cmdline but with GetModuleFileName.
 *   If WCMDF_WITHOUT_ARGV0 is On, cmdline is not supposed to have arg0 (to use with WinMain lpCmdLine arg.)
 *   On Windows, typical usage is { int argc; char** argv; unicodeInitLocale(); ParseCommandLine(GetCommandLineW(), &argc, &argv); }
 */
enum wcmd_flags {
	WCMDF_NONE = 0, WCMDF_ESCAPE_DQUOTES = 1 << 0, WCMDF_ESCAPE_SQUOTES = 1 << 1,
	WCMDF_ESCAPE_BACKSLASH = 1 << 2,
	WCMDF_WITHOUT_ARGV0 = 1 << 3, WCMDF_ARGV0_FROM_CMDLINE = 1 << 4,
	WCMDF_DEFAULT = WCMDF_ESCAPE_DQUOTES
};

bool 		ParseCmdLine(const char * cmdline,        int * pargc, char *** pargv, unsigned int flags = WCMDF_DEFAULT);
inline bool	ParseCmdLine(const std::string & cmdline, int * pargc, char *** pargv, unsigned int flags = WCMDF_DEFAULT) {
				return ParseCmdLine(cmdline.c_str(), pargc, pargv, flags); }

bool 		ParseCmdLine(const wchar_t * wcmdline,        int * pargc, char *** pargv, unsigned int flags = WCMDF_DEFAULT);
inline bool	ParseCmdLine(const std::wstring & wcmdline, int * pargc, char *** pargv, unsigned int flags = WCMDF_DEFAULT) {
				return ParseCmdLine(wcmdline.c_str(), pargc, pargv, flags); }

void ParseCmdLineFree(char ** argv);

/** get the program name from argv and pottentially fix argv */
char * fix_argv0(int * pargc, char *** pargv, size_t * pargv0len = NULL);

/**
 * Process execution utilities
 * Like std exec* functions, last argument of vs_execl MUST be NULL,
 * and last element in argv of vs_execv must be NULL.
 * On windows, arguments are quoted before calling exec* as required in microsoft doc.
 */
enum vs_exec_flags {
	VEF_NONE = 0, VEF_PATH = 1 << 0, VEF_EXEC = 1 << 1, VEF_WAIT = 1 << 2,
	VEF_DEFAULT = VEF_EXEC
};
int vs_execv(unsigned int flags, const char * file, char *const* argv);
int vs_execl(unsigned int flags, const char * file, const char * arg0, ...);

} // ! namespace VSCommon

#endif // ! _VS_COMMON_COMMON_H
