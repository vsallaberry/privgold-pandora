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
//       if base is given, it will search into first, then in current directory.
std::string getdatadir(const char * base = NULL);

// Returns where the home directory is. Returns the "" if it can't find the home dir.
// Note: cwd is not changed, but directory is created if not existing.
std::string gethomedir(const char * base = NULL);

// Return a pair <file_dir,file_name> for argv0, without changing current directory
std::pair<std::string,std::string> getfiledir(const char *argv0, const char * base = NULL);

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
