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
/* This file is for common (as in shared between vegastrike, vssetup as vsconfig) stuff
 * that is not import enought to warrant it's own file.
 */
#ifndef _VS_COMMON_COMMON_H
#define _VS_COMMON_COMMON_H

#include <stdlib.h>
#include <string>

#if defined(_WIN32)
# include <windows.h>
#endif

namespace VSCommon {

// stores the data path search list
extern const char * datadirs[];

// Returns where the data directory is. Returns the "" if it can't find the data dir.
// Note: When it returns it has already changed dir to where the data directory is
std::string getdatadir(const char * base = NULL);

// Returns where the home directory is. Returns the "" if it can't find the home dir.
// Note: cwd is not changed, but directory is created if not existing.
std::string gethomedir(const char * base = NULL);

// Return a pair <binary_dir,exec_name> for argv0, without changing current directory
std::pair<std::string,std::string> getbindir(const char *argv0, const char * base = NULL);

#if defined(_WIN32)
// Get user LocalAppData folder
HRESULT win32_get_appdata(WCHAR * wappdata);
#endif // ! _WIN32

// Essentially for windozws applications built in GUI mode (-mwindows)
// This will attach application to console if it is run from it.
// On macOS, linux, unix the libc handle it automatically (except the forcealloc==true).
bool InitConsole(bool forcealloc = false);

// -- file comparisons
typedef struct {
    uint64_t dev;
    uint64_t ino;
} file_id_t;
bool getFileId(const char * file, file_id_t * id);
ssize_t fileIdCompare(file_id_t * id, file_id_t * other);

// Translate lpCmdLine to argc, argv (for WinMain). If not win32, argv[0] has not signification. 
enum wcmd_flags { WCMDF_NONE = 0, WCMDF_ESCAPE_DQUOTES = 1 << 0, WCMDF_ESCAPE_BACKSLASH = 1 << 1 };
bool ParseCmdLine(const char * cmdline, int * pargc, char *** pargv, unsigned int flags = WCMDF_ESCAPE_DQUOTES);
void ParseCmdLineFree(char ** argv);

} // ! namespace VSCommon

#endif // ! _VS_COMMON_COMMON_H
