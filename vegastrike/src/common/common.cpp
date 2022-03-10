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
#include <stdarg.h>
#include <time.h>
#include <assert.h>

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

#ifndef VEGASTRIKE_PYTHON_DYNLIB_PATH
# define VEGASTRIKE_PYTHON_DYNLIB_PATH "lib/pythonlibs"
#endif

#if !defined(HAVE_SETENV)
// some putenv implementations do not allow stack or freeable variables, then we must store them.
# include <map>
static std::map<std::string,char *> s_envmap;
#endif

#include "log.h"

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
const char * const datadirs[] = {
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

const char * const resourcessearchs[] = {
	"..",
	".." PATHSEP "Resources",
	".",
	NULL
};

const char * const pathsep = PATHSEP;

// **************************************************************************
// wrappers
// **************************************************************************

int vs_setenv(const char * var, const char * value, int override) {
#if defined(HAVE_SETENV)
	return setenv(var, value, override);
#else
	if (!override && getenv(var) != NULL) return 0;
	char envstr[16384];
	// some putenv implementations do not allow stack or freeable variables, then we must store them.
	std::string mapkey(var);
	int newlen = snprintf(envstr, sizeof(envstr), "%s=%s", var, value);
	if (newlen >= sizeof(envstr)) newlen = sizeof(envstr) - 1;
	std::map<std::string,char *>::iterator itmap = s_envmap.find(mapkey);
	if (itmap == s_envmap.end()) {
		itmap = s_envmap.insert(std::make_pair(mapkey, strdup(envstr))).first;
	} else {
		char * mapstr = (char *) realloc(itmap->second, (size_t)(newlen+1));
		if (mapstr == NULL)
			return -1;
		strcpy(mapstr, envstr);
		itmap->second = mapstr;
	}
	//use putenv as SetEnvironmentVariableA(var,val) is not working on windows.
	return putenv(itmap->second);
#endif
}


int vs_mkdir(const char * file, int mode) {
#if !defined(_WIN32)
	return mkdir(file, mode);
#else
	(void)mode; return mkdir(file);
#endif
}

#if !defined( _WIN32) || defined( __CYGWIN__)
int 		vs_closedir(vsDIR * dirp) 	 { return closedir(dirp); }
vsDIR * 	vs_opendir(const char * path){ return opendir(path); }
vsdirent * 	vs_readdir(vsDIR * dirp) 	 { return readdir(dirp); }
#else
# define WIN32_DIRENT_DNAME_SZ 4096
int vs_closedir(vsDIR * dirp) {
	if (dirp == NULL)
		return -1;
	if (dirp->h != INVALID_HANDLE_VALUE)
		FindClose(dirp->h);
	if (dirp->d_name != NULL)
		free(dirp->d_name);
	free(dirp);
	return 0;
}
vsDIR * vs_opendir(const char * path) {
	std::string findpattern(VS_PATH_JOIN(path, "*"));
	WIN32_FIND_DATA find;
	HANDLE h;
	if ((h=FindFirstFile(findpattern.c_str(), &find))==INVALID_HANDLE_VALUE)
		return NULL;
	vsDIR * dirp;
	if ((dirp = (vsDIR*) malloc(sizeof(vsDIR))) == NULL) {
		FindClose(h);
		return NULL;
	}
	dirp->h = h;
	if ((dirp->d_name = (char *) malloc(WIN32_DIRENT_DNAME_SZ+1)) == NULL) {
		vs_closedir(dirp);
		return NULL;
	}
	snprintf(dirp->d_name, WIN32_DIRENT_DNAME_SZ, "%s", find.cFileName);
	dirp->first = true;
	return dirp;
}
vsdirent * vs_readdir(vsDIR * dirp) {
	if (dirp == NULL)
		return NULL;
	WIN32_FIND_DATA find;
	if (dirp->first) {
		dirp->first = false;
		return dirp;
	} else if (FindNextFile(dirp->h, &find)) {
		snprintf(dirp->d_name, WIN32_DIRENT_DNAME_SZ, "%s", find.cFileName);
		return dirp;
	}
	return NULL;
}
#endif

// **************************************************************************
// Game Directory Management
// **************************************************************************

std::string getresourcesdir(const char * base) {
	char tmppwd[16384];
	getcwd (tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
	const std::string origpath(tmppwd);

	/* look for resources directory, where we could find data,share,bin,lib */
	const char * found = NULL;
	const char * const bases[] = { base ? base : "", base ? "" : NULL, NULL };
	for (const char * const * basetmp = bases; !found && *basetmp; ++basetmp) {
	    for(const char * const * searchs = resourcessearchs; *searchs; ++searchs) {
	    	// Test if the dir exist and contains config_file
	    	chdir(origpath.c_str());
	    	if (*basetmp != 0) {
	    		chdir(*basetmp);
	    	}
	    	chdir(*searchs);
	    	struct stat st;
	    	if ((stat("share" PATHSEP "terminfo", &st) == 0 || stat("share" PATHSEP "gtk-2.0", &st) == 0
	    	||  stat(VEGASTRIKE_PYTHON_DYNLIB_PATH, &st) == 0) && (st.st_mode & S_IFDIR) != 0) {
	    		getcwd (tmppwd, sizeof(tmppwd)-1);
	    		found = *searchs;
	    		break;
	    	}
	    }
	}
	chdir(origpath.c_str());
	if (found == NULL)
		return "";
	return std::string(tmppwd);
}

std::string getdatadir(const char * base)
{
    char tmppwd[16384];
    getcwd (tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;

    const std::string origpath(tmppwd);
    const char * found = NULL;
    const char * const bases[] = { base ? base : "", base ? "" : NULL, NULL };
    for (const char * const * basetmp = bases; !found && *basetmp; ++basetmp) {
        for(const char * const * searchs = datadirs; *searchs; ++searchs) {
            chdir(origpath.c_str());
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
            getcwd (tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
            found = *searchs;
            break;
        }
    }

    chdir(origpath.c_str());

    if(found == NULL) {
        fprintf(stderr, "Unable to find data directory from %s%s%s\n", base?base:"", base ? " or ":"",tmppwd);
        for(const char * const * searchs = datadirs; *searchs; ++searchs) {
            fprintf(stderr, "Tried %s\n", *searchs);
        }
        return "";
    }

    return std::string(tmppwd);
}

std::pair<std::string,std::string> gethomedir(const char * base) {
    char tmppwd[16384];
    getcwd(tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;

    const std::string origpath(tmppwd);
    std::string HOMESUBDIR, DATASUBDIR;
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
	DATASUBDIR = HOMESUBDIR;
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

	vs_mkdir(HOMESUBDIR.c_str(), 0755);
	chdir (HOMESUBDIR.c_str());
    //mkdir("generatedbsp"); 
    vs_mkdir("save", 0755);

    getcwd(tmppwd, sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
    chdir(origpath.c_str());

    return std::make_pair(std::string(tmppwd), DATASUBDIR);
}

std::pair<std::string,std::string> getfiledir(const char *argv0, const char * base) {
    std::string bindir;
    std::string basename = argv0;

    char tmppwd[16384];
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

std::string getsuffixedfile(const char * file, time_t delay_sec, unsigned int maxfiles) {
	std::string retfile = file, retfile_base = retfile;
	std::string::size_type dot = retfile_base.rfind(".", std::string::npos);
	std::string retfile_suf;
	if (dot != std::string::npos) {
		retfile_suf = retfile_base.substr(dot); 	// from last dot
		retfile_base = retfile_base.substr(0, dot);	// begin -> before last dot
	}
	// Check whether the file is being used
	VSCommon::file_id_t st;
	for (unsigned int logi=1; (!maxfiles || logi <= maxfiles + 1) && VSCommon::getFileId(retfile.c_str(), &st)
                              ; ++logi) {
		char numstr[32] = { 0, };
		snprintf(numstr, sizeof(numstr), "%u", (!maxfiles || logi <= maxfiles) ? logi : 1);
		retfile = retfile_base + "." + numstr + retfile_suf;
	}
	return retfile;
}

// **************************************************************************
// Console / Terminal Utilities
// **************************************************************************

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

// path_join("dir1", ..., "dirN", NULL) -> "dir1/.../dirN"
std::string path_join(const char * first, ...) {
	va_list valist;
	std::string ret(first);

	va_start(valist, first);
	const char * arg;
	while ((arg = va_arg(valist, const char *)) != NULL) {
		ret = (ret + VSCommon::pathsep) + arg;
	}
	va_end(valist);
	return ret;
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

