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
#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <string>

using std::string;

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
#else // ! _WIN32
# include <sys/dir.h>
# include <unistd.h>
# include <pwd.h>
# include <sys/stat.h>
# include <sys/types.h>
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

string getdatadir(const char * base)
{
    char tmppwd[65536];
    getcwd (tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;

    const char * found = NULL;
    for(const char ** searchs = datadirs; *searchs; ++searchs) {
        chdir(tmppwd);
        if (base != NULL) {
            chdir(base);
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

    if(found == NULL) {
        fprintf(stderr, "Unable to find data directory\n");
        for(const char ** searchs = datadirs; *searchs; ++searchs) {
            fprintf(stderr, "Tried %s\n", *searchs);
        }
        chdir(tmppwd);
        return "";
    }

    getcwd (tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;

    return std::string(tmppwd);
}

string gethomedir(const char * base) {
    char tmppwd[65535];
    getcwd(tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;

    string HOMESUBDIR;
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
		fprintf(stderr,"Error: Failed to find Version.txt anywhere.\n");
        chdir(tmppwd);
		return "";
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

std::pair<std::string,std::string> getbindir(const char *argv0, const char * base) {
    std::string bindir;
    std::string basename = argv0;

    char tmppwd[65535];
    getcwd(tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
    std::string origpath = tmppwd;

    for (const char * dir = argv0 + strlen(argv0) - 1; *dir && dir >= argv0; --dir) {
        if (*dir == '/' || *dir == '\\') {
            bindir = (dir == argv0) ? "/" : std::string(argv0, 0, dir - argv0);
            basename = std::string(dir + 1);
            break ;
        }
    }

    if (bindir.empty() || (bindir[0] != '/'
#if defined(_WIN32)
    && bindir[0] != '\\' && (tolower(bindir[0]) < 'a' || tolower(bindir[0]) > 'z'
                             || (strncmp(bindir.c_str()+1, ":\\",2) && strncmp(bindir.c_str()+1, ":/", 2)))
#endif
    )) {
       bindir = (std::string(base != NULL ? base : origpath.c_str()) + "/") + bindir;
    }

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
    int hascons = 0;
    HWND cons=GetConsoleWindow();
    if (cons == (HWND)0) {
        hascons = AttachConsole(ATTACH_PARENT_PROCESS);
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

} // ! namespace VSCommon

