/***************************************************************************
                          common.cpp  -  description
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
 *   http://vegastrike.sourceforge.net
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
#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <string>

#include "common.h"

#ifdef _WIN32
# include <windows.h>
# include <shlobj.h>
# include <shlwapi.h>
# if defined(__CYGIN__) || defined(__MINGW32__)
#  include <io.h>
# else
#  include <direct.h>
# endif
# include <sys/stat.h>
# include <wincon.h>
# define PATHSEP "\\"
#else // ! _WIN32
# include <sys/dir.h>
# include <unistd.h>
# include <pwd.h>
# include <sys/stat.h>
# include <sys/types.h>
# define PATHSEP "/"
#endif // ! _WIN32


namespace VSCommon {

#ifdef _WIN32
# if !defined(VS_HOME_INSIDE_DATA)
#  if defined(HAVE_SHGETKNOWNFOLDERPATH) && defined(HAVE_FOLDERID_LOCALAPPDATA)
HRESULT win32_get_appdata(WCHAR * wappdata) {
    PWSTR pszPath = NULL;
    HRESULT ret;
    ret = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pszPath);
    if (pszPath == NULL)
        return S_OK-1;
    if (ret == S_OK)
        StrCpyW(wappdata, pszPath);
    CoTaskMemFree((LPVOID)pszPath);
    return ret;
}
#  else // ! HAVE_SHGETKNOWNFOLDERPATH
HRESULT win32_get_appdata(WCHAR * wappdata) {
    return SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, wappdata);
}
#  endif // ! HAVE_SHGETKNOWNFOLDERPATH
# endif // ! VS_HOME_INSIDE_DATA
#endif // ! _WIN32

// Directories to look for data
const char * datadirs[] = {
".",
"../data",
"../Resources/data",
"..",
"../data4.x",
"../../data4.x",
"../../data",
"data4.x",
"data",
"../Resources",
"../Resources/data4.x",
#ifdef DATA_DIR
 DATA_DIR,
#endif
#if 0
 "/usr/share/local/vegastrike/data",
 "/usr/local/share/vegastrike/data",
 "/usr/local/vegastrike/data",
 "/usr/share/vegastrike/data",
 "/usr/local/games/vegastrike/data",
 "/usr/games/vegastrike/data",
 "/opt/share/vegastrike/data",
 "/usr/share/local/vegastrike/data4.x",
 "/usr/local/share/vegastrike/data4.x",
 "/usr/local/vegastrike/data4.x",
 "/usr/share/vegastrike/data4.x",
 "/usr/local/games/vegastrike/data4.x",
 "/usr/games/vegastrike/data4.x",
 "/opt/share/vegastrike/data4.x",
#endif
 NULL
};

std::string getdatadir(const char * base)
{
    char tmppwd[65536];
    getcwd (tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;

    const char * found = NULL;
    const char * const bases[] = { base ? base : "", base ? "" : NULL, NULL };
    for (const char * const * basetmp = bases; !found && *basetmp; ++basetmp) {
        for(const char ** searchs = datadirs; *searchs; ++searchs) {
            chdir(tmppwd);
            if (*basetmp != 0) {
                chdir(*basetmp);
            }
            chdir(*searchs);
            FILE *tfp = fopen("setup.config", "r");
            if (tfp == NULL)
                continue ;
            fclose(tfp);
            tfp = fopen("Version.txt", "r");
            if (tfp == NULL)
                continue ;
            fclose(tfp);
            // We have found the data directory
            found = *searchs;
            break;
        }
    }

    if(found == NULL) {
        fprintf(stderr, "Unable to find data directory from %s%s%s\n", base?base:"", base ? " or ":"",tmppwd);
        for(const char ** searchs = datadirs; *searchs; ++searchs) {
            fprintf(stderr, "Tried %s\n", *searchs);
        }
        chdir(tmppwd);
        return "";
    }

    getcwd (tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;

    return std::string(tmppwd);
}

std::string gethomedir(const char * base) {
    char tmppwd[65535];
    getcwd(tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;

    std::string HOMESUBDIR;
    if (base != NULL) {
        chdir(base);
    }
	FILE *version=fopen("Version.txt","r");
	if (!version)
		version=fopen("../Version.txt","r");
	if (version) {
		std::string hsd="";
		int c;
		while ((c=fgetc(version))!=EOF) {
			if (isspace(c))
				break;
			hsd+=(char)c;
		}
		fclose(version);
		if (hsd.length()) {
			HOMESUBDIR=hsd;
		}
	}
	if (HOMESUBDIR.empty()) {
        HOMESUBDIR = ".vegastrike";
		fprintf(stderr,"Warning: Failed to find Version.txt anywhere, using %s as home.\n", HOMESUBDIR.c_str());
	}
#if !defined(VS_HOME_INSIDE_DATA)
# if !defined(_WIN32)
	struct passwd *pwent;
	pwent = getpwuid (getuid());
	chdir (pwent->pw_dir);
# else
	WCHAR wappdata_path[PATH_MAX];
	if (VSCommon::win32_get_appdata(wappdata_path) != S_OK) {
		// use datadir as homedir
	} else {
		if (HOMESUBDIR.size() && HOMESUBDIR[0] == '.') {
			HOMESUBDIR=HOMESUBDIR.substr(1);
		}
		_wchdir(wappdata_path);
	}
# endif
#endif

	mkdir(HOMESUBDIR.c_str() 
#ifndef _WIN32
              , 0755
#endif
              );
	chdir (HOMESUBDIR.c_str());
    //mkdir("generatedbsp"); 
    mkdir("save"
#ifndef _WIN32
              , 0755
#endif
              );

    char homepath[65535];
    getcwd(homepath, sizeof(homepath)-1); homepath[sizeof(homepath)-1] = 0;
    chdir(tmppwd);

    return std::string(homepath);
}

std::pair<std::string,std::string> getfiledir(const char *argv0, const char * base) {
    std::string bindir;
    std::string basename = argv0;

    char tmppwd[65535];
    getcwd(tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
    std::string origpath = tmppwd;
    if (base)
        chdir(base);

    const char * tmp = argv0 + strlen(argv0) - 1;

    while (tmp >= argv0 && *tmp != '/' && *tmp != *PATHSEP) {
        --tmp;
    }
    basename = std::string(tmp+1);
    if (tmp >= argv0) 
        bindir = std::string(argv0, tmp - argv0) + PATHSEP; 
    else 
        bindir = ".";

    chdir(bindir.c_str());
    getcwd(tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
    chdir(origpath.c_str());

    return std::make_pair(std::string(tmppwd), basename);
}

// InitConsole()
// *************
#if !defined(_WIN32)
bool InitConsole(bool forcealloc) {
    return true;
}
#else
bool InitConsole(bool forcealloc) {
    HWND cons=GetConsoleWindow();
    if (cons == (HWND)0) {
        int hascons = AttachConsole(ATTACH_PARENT_PROCESS);
        cons=GetConsoleWindow();
        if (cons == (HWND)0 && forcealloc) {
            hascons = AllocConsole();
            cons=GetConsoleWindow();
        }
        if (cons == (HWND)0) {
            return false;
        }
        //fprintf(f, "Attach console\n");
        //fprintf(stdout, "stdout after Attach\r\n");
        //fprintf(stderr, "stderr after Attach\r\n");
        fflush(NULL);
        if (fileno(stdout) < 0 && fileno(stderr) < 0) {
            if (freopen("CONOUT$", "w", stdout) == NULL) {
                fprintf(stderr, "freopen(stdout) error\n");
            }
            if (freopen("CONOUT$", "w", stderr) == NULL) {
                fprintf(stderr, "freopen(stderr) error\n");
            }
            if (freopen("CONIN$", "w", stdin) == NULL) {
                fprintf(stderr, "freopen(stdin) error\n");
            }
            //fprintf(stdout, "\r\nstdout hello\n");
            //fprintf(stderr, "stderr hello \n");
        }
    }
    return true;
}
#endif // ! _WIN32

// ---------------------------------------------------------------------
bool getFileId(const char * file, file_id_t * id) {
    if (id != NULL)
        memset(id, 0, sizeof(*id));
    if (file == NULL || id == NULL)
        return false;
#ifdef _WIN32
    // inspired by gnu coreutils stat win32 lib
    BY_HANDLE_FILE_INFORMATION info;
    HANDLE h;
    bool ret = false;
    h = CreateFile (file, FILE_READ_ATTRIBUTES,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return ret;
    if (GetFileInformationByHandle (h, &info)) {
        id->dev = info.dwVolumeSerialNumber;
        id->ino = ((ULONGLONG) info.nFileIndexHigh << 32) | (ULONGLONG) info.nFileIndexLow;
        ret = true;
    }
    CloseHandle(h);
    return ret;
#else
    struct stat st;
    if (stat(file, &st) < 0)
        return false;
    id->dev = st.st_dev;
    id->ino = st.st_ino;
    return true;
#endif
}

ssize_t fileIdCompare(file_id_t * id, file_id_t * other) {
    if (id == other)
        return 0;
    if (!id || !other)
        return (ssize_t) (id - other);
    return (ssize_t) memcmp(id, other, sizeof(*id));
}

#if !defined(_WIN32)
# define GetModuleFileName(x, dst, size) snprintf(dst, size, "vegastrike")
#endif
bool ParseCmdLine(const char * cmdline, int * pargc, char *** pargv, unsigned int flags) {
    char *argv0 = (char*) malloc(65535);
    int argc = 1;
    char ** argv = (char**) malloc((argc + 1) * sizeof(*argv)); 
    *argv = argv0;
    GetModuleFileName(NULL, argv0, 65534);
    char * lpCmdLine = argv0 + strlen(argv0) + 1; // use remaining space in argv0 for cmdline
    snprintf(lpCmdLine, 65535 - (lpCmdLine - argv0), "%s", cmdline);
    while (*lpCmdLine) {
        int escape = 0;
        argv = (char**) realloc(argv, (argc+2)*sizeof(*argv));
        char * arg = argv[argc++] = lpCmdLine;
        while (*lpCmdLine && (escape || *lpCmdLine != ' ')) {
            if ((flags & (WCMDF_ESCAPE_DQUOTES|WCMDF_ESCAPE_BACKSLASH)) != 0
                    &&  *lpCmdLine == '\\' && (lpCmdLine[1] == '"' || (flags & WCMDF_ESCAPE_BACKSLASH)) != 0) { 
                *arg++ = *(++lpCmdLine); 
                ++lpCmdLine; 
            }
            else if ((flags & WCMDF_ESCAPE_DQUOTES) != 0 && *lpCmdLine == '"') { 
                lpCmdLine++; 
                escape = !escape; 
            } else {
                *arg++ = *lpCmdLine++;
            }
        }
        *arg = *lpCmdLine++ = 0;
    }
    argv[argc] = NULL;
    *pargv = argv;
    *pargc = argc;
    return true;
}

void ParseCmdLineFree(char ** argv) {
    if (argv) {
        if (argv[0])
            free(argv[0]);
        free(argv);
    }
}

} // ! namespace VSCommon

