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
#include <stack>

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
# include <process.h>
# include <shellapi.h>
# define PATHSEP "\\"
#else // ! _WIN32
# include <sys/dir.h>
# include <unistd.h>
# include <pwd.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/time.h>
# define PATHSEP "/"
#endif // ! _WIN32

#ifndef VEGASTRIKE_LIBDIR_NAME
# define VEGASTRIKE_LIBDIR_NAME "lib"
#endif
#ifndef VEGASTRIKE_PYTHON_DYNLIB_PATH
# define VEGASTRIKE_PYTHON_DYNLIB_PATH VEGASTRIKE_LIBDIR_NAME "/pythonlibs"
#endif

#if !defined(HAVE_SETENV)
// some putenv implementations do not allow stack or freeable variables, then we must store them.
# include <map>
static std::map<std::string,char *> s_envmap;
#endif

#if !defined(_WIN32)
#include <sys/wait.h>
#include <signal.h>
sig_atomic_t s_sig_received = 0, s_sig_sender = 0;
static void s_signal_handler(int sig, siginfo_t * si_info, /*ucontext_t* */void * ctx) {
	(void)ctx;
	s_sig_received = sig;
	s_sig_sender = si_info->si_pid;
}
#endif

#include "log.h"
#include "unicode.h"

#if defined(VS_COMMON_NO_LOG)
# define VS_LOG(module, level, ...) ((level) <= logvs::INFO ? fprintf(stderr, __VA_ARGS__) : 0)
# define VS_DBG(...) (0)
#endif

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
# if defined(_WIN32)
    wchar_t nputenvstr[16384];
    if (utf8_to_wstr(nputenvstr, itmap->second, sizeof(nputenvstr)/sizeof(*nputenvstr)) == (size_t)-1)
    	return putenv(itmap->second);
    return _wputenv(nputenvstr);
# else
	return putenv(itmap->second);
# endif
#endif
}

#if !defined( _WIN32)

int		vs_mkdir(const char * path, mode_t mode){ return mkdir(path, mode); }
FILE * 	vs_fopen(const char * path, const char * mode) { return fopen(path, mode); }
FILE * 	vs_freopen(const char * path, const char * mode, FILE * fp) { return freopen(path, mode, fp); }
char *	vs_getcwd(char * path, size_t size) 	{ return getcwd(path, size); }
int 	vs_chdir(const char * path) 			{ return chdir(path); }
int 	vs_stat(const char * path, struct stat * st) { return stat(path, st); }
int 	vs_closedir(vsDIR * dirp) 	 			{ return closedir(dirp); }
vsDIR * vs_opendir(const char * path)			{ return opendir(path); }
vsdirent * vs_readdir(vsDIR * dirp) 	 		{ return readdir(dirp); }

#else
# define getcwd(path, size) 	VSCommon::vs_getcwd(path, size)
# define chdir(path)			VSCommon::vs_chdir(path)
# define mkdir(path, mode)		VSCommon::vs_mkdir(path, mode)
# define stat(path, st)			VSCommon::vs_stat(path, st)
# define fopen(file, mode)		VSCommon::vs_fopen(file, mode)
# define freopen(file, mode,fp)	VSCommon::vs_freopen(file, mode, fp)

# define VSCOMMON_PATH_MAX 16384
int 	vs_mkdir(const char * path, mode_t mode) {
	(void)mode;
	wchar_t wfilename[VSCOMMON_PATH_MAX];
	if ((size_t)-1 == utf8_to_wstr(wfilename, path, sizeof(wfilename)/sizeof(*wfilename)))
		return -1;
	return _wmkdir(wfilename);
}

FILE * 	vs_fopen(const char * path, const char * mode) {
	return vs_freopen(path, mode, NULL);
}

FILE * 	vs_freopen(const char * path, const char * mode, FILE * fp) {
	wchar_t wfilename[VSCOMMON_PATH_MAX];
	wchar_t wmode[64];
	if (utf8_to_wstr(wfilename, path, sizeof(wfilename)/sizeof(*wfilename)) == (size_t)-1
			||  utf8_to_wstr(wmode,     mode, sizeof(wmode)    /sizeof(*wmode))     == (size_t)-1)
		return NULL;
	if (fp == NULL)
		return _wfopen(wfilename, wmode);
	return _wfreopen(wfilename, wmode, fp);
}

char * 	vs_getcwd(char * path, size_t size) {
	wchar_t wpath[VSCOMMON_PATH_MAX] = { 0, };
	wchar_t * res = _wgetcwd(wpath, sizeof(wpath)/sizeof(*wpath)-1);
	wpath[sizeof(wpath)/sizeof(*wpath)-1] = 0;

	if (wstr_to_utf8(path, wpath, size) == (size_t)-1)
		return NULL;
	return path;
}

int 	vs_chdir(const char * path) {
	wchar_t wfilename[VSCOMMON_PATH_MAX];
	if (utf8_to_wstr(wfilename, path, sizeof(wfilename)/sizeof(*wfilename)) == (size_t)-1)
		return -1;
	return _wchdir(wfilename);
}

int 	vs_stat(const char * path, struct stat * st) {
	wchar_t wfilename[VSCOMMON_PATH_MAX];
	if (utf8_to_wstr(wfilename, path, sizeof(wfilename)/sizeof(*wfilename)) == (size_t)-1)
		return -1;
	return wstat(wfilename, st);
}

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
	wchar_t wfindpattern[VSCOMMON_PATH_MAX];
	WIN32_FIND_DATAW find;
	HANDLE h;

	if (utf8_to_wstr(wfindpattern, findpattern.c_str(), sizeof(wfindpattern)/sizeof(*wfindpattern))== (size_t)-1
	||  (h=FindFirstFileW(wfindpattern, &find))==INVALID_HANDLE_VALUE)
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
	wstr_to_utf8(dirp->d_name, find.cFileName, WIN32_DIRENT_DNAME_SZ+1);
	dirp->first = true;
	return dirp;
}

vsdirent * vs_readdir(vsDIR * dirp) {
	if (dirp == NULL)
		return NULL;
	WIN32_FIND_DATAW find;
	if (dirp->first) {
		dirp->first = false;
		return dirp;
	} else if (FindNextFileW(dirp->h, &find)) {
		wstr_to_utf8(dirp->d_name, find.cFileName, WIN32_DIRENT_DNAME_SZ+1);
		return dirp;
	}
	return NULL;
}

#endif // ! defined(_WIN32)

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
	VSCommon::file_info_t st;
	for (unsigned int logi=1; (!maxfiles || logi <= maxfiles + 1) && VSCommon::getFileInfo(retfile.c_str(), &st)
                              && (delay_sec == (time_t)-1 || st.mtime + delay_sec >= time(NULL)); ++logi) {
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
#if defined(_WIN32)
bool InitConsole(bool forcealloc, int argc, char ** argv) {
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
#else
bool InitConsole(bool forcealloc, int argc, char ** argv) {
	if (!forcealloc || isatty(STDOUT_FILENO)) {
		return true;
	}
	if (argc == 0 || argv == NULL)
		return false;
    //#if defined(__APPLE__)
    // I don't know how to launch app in an apple term __ with cmdline args __ without need
	// of allowing the user control of terminal in apple confidentiality settings.
	// Even if this is not fully mandatory on linux, we use the same method as for macOS
	// in order to prevent the infinite loops when terminal does not work (check with test -t in temp sh script).
	// This is an event, I congratulate microsoft for their amazing AllocConsole(). Maybe (or not) it can be done
	// on posix by launching term and redirect ttys without forking(if process fw ok), but that's for another life.
    char pwd[16384];
	getcwd(pwd, sizeof(pwd)-1); pwd[sizeof(pwd)-1] = 0;
	std::pair <std::string,std::string> bindir = VSCommon::getfiledir(*argv, pwd);
    std::pair<std::string, std::string> homes = VSCommon::gethomedir(VSCommon::getdatadir().c_str());
    int tmpfd = -1;
    FILE * tmpf;
    char *tmpname = strdup((((homes.first + PATHSEP) + bindir.second) + "_temp.XXXXXX").c_str());

	if (tmpname == NULL || (tmpfd = mkstemp(tmpname)) < 0 || (tmpf = fdopen(tmpfd, "w")) == NULL) {
		if (tmpname) free(tmpname);
        if (tmpfd >= 0) close(tmpfd);
		exit(-1);
	}
	// tmp sh script launching argv0 with notification to me when it ends
	fprintf(tmpf, "#!/bin/sh\n"
			      "test -t || exit 1\n"
			      "cleanup() { kill -USR1 %d; rm -f \"$0\"; } || true\n"
			      "trap cleanup EXIT || true\n"
			      "cd \"%s\"\n", getpid(), pwd);
	// writing tmp script argv[0] and the quoted argv+1 arguments
	fprintf(tmpf, "\"%s/%s\" ", bindir.first.c_str(), bindir.second.c_str());
	for (int i_argv = 1; i_argv < argc; ++i_argv) {
		fputc('"', tmpf);
		for (char * sq = argv[i_argv]; *sq; ++sq) {
			if (*sq == '"' || *sq == '\\') {
				fputc('\\', tmpf);
			}
			fputc(*sq, tmpf);
		}
		fputs("\" ", tmpf);
	}
	fprintf(tmpf, "\n" "trap - EXIT || true\n"
			           "kill -USR1 %d\n"
			           "rm -f \"$0\"\n", getpid());
	fclose(tmpf);
	chmod(tmpname, 0700);

	// setup and fork/exec/wait
	sigset_t 	sigset_block;
	pid_t 		termpid;
	s_sig_received = 0;
	sigemptyset(&sigset_block);
	sigaddset(&sigset_block, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigset_block, NULL);

	if ((termpid = fork()) < 0) { // error
		return false;
	} else if (termpid == 0) { // child
		char exe[4096];
#if defined (__APPLE__)
# define TERMEXE_SUFFIX "/Terminal.app/Contents/MacOS/Terminal"
		const char * const searchs[] = { "/System/Applications/Utilities", "/System/Applications",
				                         "/Applications/Utilities", "/Applications", NULL };
		char * newargv[] = { exe, tmpname, NULL };
#else //defined(__linux__)
# define TERMEXE_SUFFIX ""
		const char * const searchs[] = { "xterm", "gnome-terminal", "konsole", NULL };

		char * newargv[] = { exe, "-e", tmpname, NULL };
#endif
		for (const char * const * search = searchs; *search; ++search) {
			snprintf(exe, sizeof(exe), "%s%s", *search, TERMEXE_SUFFIX);
			execvp(*newargv, newargv); // only returns on error
		}
		exit(-1); // Terminal not found or failed to execute.
    } else {
        // wait for USR1 from program launched by Terminal, then cleanup temp file and kill Terminal
    	int 				status;
    	sigset_t 			sigset;
    	const int 			sigs[] = { SIGUSR1, SIGUSR2, SIGINT, SIGTERM, SIGHUP, SIGCHLD };
    	struct sigaction 	sa; sa.sa_flags = SA_RESTART | SA_SIGINFO; sigfillset(&sa.sa_mask);

    	sigfillset(&sigset);
    	for (size_t i = 0; i < sizeof(sigs) / sizeof(*sigs); ++i) {
    		sigdelset(&sigset, sigs[i]);
    		sa.sa_sigaction = s_signal_handler;
    		sigaction(sigs[i], &sa, NULL);
    	}
		sigsuspend(&sigset);
		sleep(1);
		if (kill(termpid, SIGTERM) != 0)
			kill(termpid, SIGKILL);
		if (tmpname) {
			unlink(tmpname);
			free(tmpname);
		}
		sa.sa_flags = SA_RESTART; sa.sa_handler = SIG_DFL;
		for (size_t i = 0; i < sizeof(sigs) / sizeof(*sigs); ++i)
			sigaction(sigs[i], &sa, NULL);
       	sigprocmask(SIG_UNBLOCK, &sigset_block, NULL);
		waitpid(termpid, &status, 0);
		exit(0);
	}
	//#endif
	return false;
}
// For information: version with a direct execvp
/* #if 0 //defined(__linux__) {
	int newargc = 2;
	char ** newargv = (char **) malloc((argc + newargc + 1) * sizeof(*argv));
	char exe[4096] = { 0, };
	int i_argv;
	for (i_argv = 0; i_argv < argc; ++i_argv) {
		newargv[i_argv + newargc] = argv[i_argv];
	}
	newargv[0] = exe;
	newargv[1] = "-e";
	newargv[i_argv + newargc] = NULL;
	const char * const searchs[] = { "xterm", "gnome-terminal", "konsole", NULL };
	for (const char * const * search = searchs; *search; ++search) {
		snprintf(exe, sizeof(exe), "%s", *search);
		execvp(*newargv, newargv);
	}
	exit(-1); // Terminal not found
#endif */
#endif // ! _WIN32


// **************************************************************************
// File Utilities
// **************************************************************************

// ---------------------------------------------------------------------
#if defined(_WIN32)
static time_t winFileTimeToTimeT(const FILETIME * ft) {
	// inspired by GNU coreutils stat win32 lib
	/* FILETIME: <https://docs.microsoft.com/en-us/windows/desktop/api/minwinbase/ns-minwinbase-filetime> */
	unsigned long long since_1601 =
			((unsigned long long) ft->dwHighDateTime << 32)
			| (unsigned long long) ft->dwLowDateTime;
	if (since_1601 == 0) {
		return 0;
	} else {
		/* Between 1601-01-01 and 1970-01-01 there were 280 normal years and 89
	                 leap years, in total 134774 days.  */
		unsigned long long since_1970 =
				since_1601 - (unsigned long long) 134774 * (unsigned long long) 86400 * (unsigned long long) 10000000;
		return since_1970 / (unsigned long long) 10000000;
	}
}
static int winTimeTtoFileTime(FILETIME * ft, const time_t * time1970) {
	// inspired by GNU coreutils stat win32 lib
	ULONGLONG time_since_16010101 =
	            (ULONGLONG) (*time1970) * 10000000 + 116444736000000000LL /*134774days*/;
	ft->dwLowDateTime = (DWORD) time_since_16010101;
	ft->dwHighDateTime = time_since_16010101 >> 32;
	return 0;
}
static int set_utimes(FILE * fp, const char * path, file_info_t * info) {
	// inspired by GNU coreutils stat win32 lib
	//	<https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea>
	//  <https://docs.microsoft.com/en-us/windows/desktop/FileIO/creating-and-opening-files>
	int ret = 0;
	FILETIME ftAtime, ftMtime;
	WCHAR * wpath = NULL; // nul et non avenu comme vichy.
	if (utf8_to_wstr(&wpath, path) == (size_t)-1 || wpath == NULL)
		return -1;
	HANDLE h = CreateFileW(wpath, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		free(wpath);
		return -1;
	}
	winTimeTtoFileTime(&ftMtime, &info->mtime);
	winTimeTtoFileTime(&ftAtime, &info->atime);

	if (SetFileTime (h, NULL, &ftAtime, &ftMtime))
		ret = -1;
	CloseHandle(h);
	free(wpath);
	return ret;
}
#else
static int set_utimes(FILE * fp, const char * path, file_info_t * info) {
	struct timeval amts[2];
	amts[0].tv_usec = amts[1].tv_usec = 0;
	amts[0].tv_sec = info->atime;
	amts[1].tv_sec = info->mtime;
	/* #include <utime.h>
	struct utimbuf timebuf;
	timebuf.actime = info->atime;
	timebuf.modtime = info->mtime;
	return utime(path, &timebuf);*/
	if (fp)
		return futimes(fileno(fp), amts);
	return utimes(path, amts);
}
#endif

bool getFileInfo(const char * file, file_info_t * id) {
	assert(FIT_LINK < FIT_WRITE);
	if (id != NULL)
        memset(id, 0, sizeof(*id));
    if (file == NULL || id == NULL)
        return false;
#ifdef _WIN32
    // inspired by GNU coreutils stat win32 lib
    BY_HANDLE_FILE_INFORMATION info;
    HANDLE h;
    WCHAR * wfile = NULL; // nul et non avenu comme vichy.
    bool ret = false;
    if (utf8_to_wstr(&wfile, file) == (size_t)-1 || wfile == NULL)
        return -1;
    h = CreateFileW(wfile, FILE_READ_ATTRIBUTES,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (h == INVALID_HANDLE_VALUE) {
    	free(wfile);
        return ret;
    }
    if (GetFileInformationByHandle (h, &info)) {
        id->dev = info.dwVolumeSerialNumber;
        id->ino = ((ULONGLONG) info.nFileIndexHigh << 32) | (ULONGLONG) info.nFileIndexLow;
        id->size = ((ULONGLONG) info.nFileSizeHigh << 32) | (ULONGLONG) info.nFileSizeLow;
        id->ctime = id->mtime = winFileTimeToTimeT(&info.ftLastWriteTime);
        id->atime = winFileTimeToTimeT(&info.ftLastAccessTime);
        id->btime = winFileTimeToTimeT(&info.ftCreationTime);
        id->type |= ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ? FIT_DIR : FIT_FILE;
        id->type |= ((info.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0) ? FIT_WRITE : 0;
        ret = true;
    }
    CloseHandle(h);
    free(wfile);
    return ret;
#else
    struct stat st;
    if (stat(file, &st) < 0)
        return false;
    id->dev = st.st_dev;
    id->ino = st.st_ino;
    id->size = st.st_size;
    id->mtime = st.st_mtime;
    id->atime = st.st_atime;
    id->ctime = st.st_ctime;
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    id->btime = st.st_birthtime;
#endif
  	id->type |= (st.st_mode & S_IFDIR) != 0 ? FIT_DIR : 0;
  	id->type |= (st.st_mode & S_IFLNK) != 0 ? FIT_LINK : 0;
  	id->type |= (st.st_mode & S_IFREG) != 0 ? FIT_FILE : 0;
  	id->type |= (st.st_mode & S_IWUSR) != 0 ? FIT_WRITE : 0;
    return true;
#endif
}

ssize_t fileIdCompare(file_info_t * id, file_info_t * other) {
    if (id == other)
        return 0;
    if (!id || !other)
        return (ssize_t) (id - other);
    return (ssize_t) memcmp(id, other, sizeof(*id));
}

static int fileCompare(const char * src, const char * dst, bool replaceDstIfNeeded) {
	const size_t filebufsize = 4096;
	file_info_t infosrc, infodst;
	FILE * fp_src, * fp_dst;
	bool equal = false, dst_exists;

	// src exists?
	if (!getFileInfo(src, &infosrc) || (fp_src = fopen(src, "r")) == NULL)
		return FILECOMP_SRCNOTFOUND;

	// dst exists?
	dst_exists = getFileInfo(dst, &infodst);
	equal = dst_exists && fileIdCompare(&infosrc, &infodst) == 0; // dev/inode compare

	// dst exist, is not src and has same size as src -> comparison needed.
	if (!equal && dst_exists && infosrc.size == infodst.size) {
		if ((fp_dst = fopen(dst, "r")) == NULL) {
			fclose(fp_src);
			return FILECOMP_ERROR;
		}
		char buf_src[filebufsize], buf_dst[filebufsize];
		while (!feof(fp_src)) {
			size_t n_src = fread(buf_src, 1, sizeof(buf_src), fp_src);
			size_t n_dst = fread(buf_dst, 1, sizeof(buf_src), fp_dst);
			if (n_src != n_dst || memcmp(buf_src, buf_dst, n_src) != 0)
				break ;
			if (feof(fp_src) && feof(fp_dst)) {
				equal = true;
				break ;
			}
		}
		fclose(fp_dst);
	}
	// return comparison status if replace is not requested.
	if (!replaceDstIfNeeded || equal) {
		fclose(fp_src);
		return equal ? FILECOMP_EQUAL : FILECOMP_DIFF;
	}
	// create directories for dst
	for (size_t i = 0; dst[i]; ++i) {
		if (i > 0 && (dst[i] == *VSCommon::pathsep || dst[i] == '/')) {
			vs_mkdir(std::string(dst, 0, i).c_str(), 0755);
		}
	}
	// reset src and open dst for writing
	if (fseek(fp_src, 0, SEEK_SET) != 0 || (fp_dst = fopen(dst, "w")) == NULL) {
		fclose(fp_src);
		return FILECOMP_ERROR;
	}
	equal = true;
	while (!feof(fp_src)) {
		char buf[filebufsize];
		size_t nread = fread(buf, 1, sizeof(buf), fp_src);
		size_t nwrite = fwrite(buf, 1, nread, fp_dst);
		if (nread != nwrite || (nread == 0 && !feof(fp_src))) {
			equal = false;
			break ;
		}
	}
	// update modification time, cleanup, return
	if (equal) {
		fflush(fp_dst);
		set_utimes(fp_dst, dst, &infosrc);
	}
	fclose(fp_src);
	fclose(fp_dst);
	return equal ? FILECOMP_REPLACED : FILECOMP_ERROR;
}

bool fileCompare(const char * src, const char * dst) {
	return fileCompare(src, dst, false) == FILECOMP_EQUAL;
}

int fileCopyIfDifferent(const char * src, const char * dst, size_t depth) {
	if (depth == 0)
		return fileCompare(src, dst, true);

	file_info_t info;
	if (!getFileInfo(src, &info))
		return FILECOMP_SRCNOTFOUND;
	if ((info.type & FIT_TYPE_MASK) != FIT_DIR)
		return fileCompare(src, dst, true);

	int ret = FILECOMP_EQUAL;
	std::stack<std::pair<std::string,size_t> > dirstack;
	dirstack.push(std::make_pair(std::string("."), 1));

	while (dirstack.size()) {
		std::pair<std::string,size_t> dirpair = dirstack.top();
		std::string srcdir = VS_PATH_JOIN(src, dirpair.first.c_str());
		std::string dstdir = VS_PATH_JOIN(dst, dirpair.first.c_str());
		//fprintf(stderr, "** %s, src = %s, dst = %s\n", dirpair.first.c_str(), srcdir.c_str(), dstdir.c_str());
		vsDIR * dir;
		vsdirent * ent;

		dirstack.pop();

		if ((dir = vs_opendir(srcdir)) == NULL)
			continue ;

		while ((ent = VSCommon::vs_readdir(dir)) != NULL) {
			if (!strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "."))
				continue ;
			std::string srcpath(VS_PATH_JOIN(srcdir.c_str(), ent->d_name));
			if (!getFileInfo(srcpath.c_str(), &info)) {
				ret = FILECOMP_ERROR;
				continue ;
			}
			if ((info.type & FIT_TYPE_MASK) == FIT_DIR && dirpair.second < depth) {
				dirstack.push(std::make_pair(VS_PATH_JOIN(dirpair.first.c_str(), ent->d_name), dirpair.second+1));
				continue ;
			}

			std::string dstpath(VS_PATH_JOIN(dstdir.c_str(), ent->d_name));
			int localret;
			VS_LOG("vscommon", logvs::VERBOSE, "  -> copying %s", srcpath.c_str());

			if ((localret = fileCompare(srcpath.c_str(), dstpath.c_str(), true)) < 0) {
				VS_LOG("vscommon", logvs::WARN, "error copying %s", srcpath.c_str());
				ret = localret;
			} else if (localret == FILECOMP_REPLACED) {
				ret = localret;
			}
		}
		vs_closedir(dir);
	}

	return ret;
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

/* ***************************************************
 * Command Line Utilities
 * ***************************************************/

static const char * quote_follows(const char *s, const char * quotes, bool match0) {
	while (*s == '\\') ++s;
	return (match0 && *s == 0) || strchr(quotes, *s) != NULL ? s : NULL;
}

#if defined(_WIN32)
static size_t get_argv0(char ** pargv0, size_t size = (size_t)-1) {
	WCHAR wargv0[32768] = { 0, };
	GetModuleFileNameW(NULL, wargv0, sizeof(wargv0)/sizeof(*wargv0) - 1);
	wargv0[sizeof(wargv0)/sizeof(*wargv0) - 1] = 0;
	return wstr_to_utf8(pargv0, wargv0, size);
}
#else
static size_t get_argv0(char ** pargv0, size_t size = (size_t)-1) {
	if (*pargv0 == NULL)
		*pargv0 = strdup("");
	else
		snprintf(*pargv0, size, "");
	return strlen(*pargv0);
}
#endif

static bool ParseCmdLine_error(bool ret, char * argv0, char ** argv, int * pargc, char *** pargv) {
	if (argv != NULL) free(argv);
	if (argv0 != NULL) free(argv0);
	if (pargc != NULL) *pargc = 0;
	if (pargv != NULL) *pargv = NULL;
	return ret;
}

bool ParseCmdLine(const char * cmdline, int * pargc, char *** pargv, unsigned int flags) {
    char *argv0 = NULL;
    int argc = (flags & WCMDF_WITHOUT_ARGV0) == 0 ? 0 : 1;
    char ** argv = (char**) malloc((argc + 1) * sizeof(*argv));
    size_t argv0_len = get_argv0(&argv0);
    char quotes[3], quote[2] = { 0, 0 };

    if (argv == NULL || argv0 == NULL || cmdline == NULL || pargc == NULL || pargv == NULL)
    	return ParseCmdLine_error(false, argv0, argv, pargc, pargv);

    {
    	size_t i = 0;
    	if ((flags & WCMDF_ESCAPE_DQUOTES) != 0) quotes[i++] = '"';
        if ((flags & WCMDF_ESCAPE_SQUOTES) != 0) quotes[i++] = '\'';
        quotes[i] = 0;
        char * newargv0 = (char*) realloc(argv0, (argv0_len + 1 + 32767) * sizeof(*argv0));
        if (newargv0 == NULL) return ParseCmdLine_error(false, argv0, argv, pargc, pargv);
        argv0 = newargv0;
    }
    // use remaining space in argv0 for cmdline

    char * lpCmdLine = argv0 + argv0_len + 1;
    snprintf(lpCmdLine, 32767, "%s", cmdline);
    while (*lpCmdLine) {
        int escape = 0;
        char ** newargv = (char**) realloc(argv, (argc+2)*sizeof(*argv));
        if (newargv == NULL) return ParseCmdLine_error(false, argv0, argv, pargc, pargv);
        argv = newargv;
        char * arg = argv[argc++] = lpCmdLine;
        while (*lpCmdLine && (escape || *lpCmdLine != ' ')) {
            if (escape && (flags & (WCMDF_ESCAPE_DQUOTES|WCMDF_ESCAPE_SQUOTES|WCMDF_ESCAPE_BACKSLASH)) != 0 && *lpCmdLine == '\\'
            &&  ((flags & WCMDF_ESCAPE_BACKSLASH) != 0 || quote_follows(lpCmdLine,quote,false) )) {
                *arg++ = *(++lpCmdLine);
                ++lpCmdLine;
            }
            else if ((flags & (WCMDF_ESCAPE_DQUOTES|WCMDF_ESCAPE_SQUOTES)) != 0
            		 && ((!escape && strchr(quotes, *lpCmdLine) != NULL) || (escape && *lpCmdLine == *quote))) {
            	*quote = *lpCmdLine;
                lpCmdLine++;
                escape = !escape;
            } else {
                *arg++ = *lpCmdLine++;
            }
        }
        while (*lpCmdLine == ' ')
        	++lpCmdLine;
        *arg = 0;
    }
    if (*argv0 && (flags & WCMDF_ARGV0_FROM_CMDLINE) == 0) // use GetModuleFileName if not empty, first element of cmdline otherwise
    	*argv = argv0;
    argv[argc] = NULL;
    *pargv = argv;
    *pargc = argc;
    return true;
}

bool ParseCmdLine(const wchar_t * wcmdline, int * pargc, char *** pargv, unsigned int flags) {
    size_t wlen = wcslen(wcmdline);
    char * cmdline = NULL;
    if (wstr_to_utf8(&cmdline, wcmdline) == (size_t)-1 || cmdline == NULL) {
    	cmdline = (char*) malloc((wlen+1)*sizeof(*cmdline));
    	if (cmdline == NULL)
    		return false;
    	for (int i = 0; i < wlen; ++i)
    		cmdline[i] = wcmdline[i];
    	cmdline[wlen] = 0;
	}
    bool ret = ParseCmdLine(cmdline, pargc, pargv, flags);
    free(cmdline);
    return ret;
}

#if 0 && defined(_WIN32)
int ParseCmdLine(const wchar_t * wcmdline, int * pargc, char *** pargv, unsigned int flags) {
    LPWSTR * argvw = CommandLineToArgvW(wcmdline, pargc);
    *pargv = (char**) malloc((*pargc+1)*sizeof(**pargv));
    for (int i = 0; i < *pargc; ++i) {
        size_t wlen = wcslen(argvw[i]);
        (*pargv)[i] = (char*) malloc((wlen+1)*sizeof(***pargv));
        for (int j = 0; j < wlen; ++j)
            (*pargv)[i][j] = argvw[i][j];
        (*pargv)[i][wlen] = 0;
    }
    (*pargv)[*pargc] = NULL;
    LocalFree(argvw);
    return 1;
}
#endif

void ParseCmdLineFree(char ** argv) {
    if (argv) {
        if (argv[0])
            free(argv[0]);
        free(argv);
    }
}

/* ***************************************************
 * Process Execution Utilities
 * ***************************************************/
#if defined(_WIN32)
static char ** quote_argv(char *const*argv) {
	int argc;
	for (argc = 0; argv[argc] != NULL; ++argc) ;/*loop*/
	char ** nargv = (char **) malloc((argc+1)*sizeof(*argv));
	if (nargv == NULL)
		return NULL;
	nargv[argc] = NULL;

	for (int i_argv = 0; i_argv < argc; ++i_argv) {
		size_t len = strlen(argv[i_argv]);
		char * arg = (char *) malloc((len*2)+3); // worst case is to escape each character.
		if (arg == NULL) {
			for (int i = 0; i < i_argv - 1; ++i) free(nargv[i]);
			free(nargv);
			return NULL;
		}
		*arg = '"';
		char * parg = arg + 1;
		for (size_t ic = 0; ic < len; ++ic, ++parg) {
			if (argv[i_argv][ic] == '\"'
		    ||  (argv[i_argv][ic] == '\\' && (argv[i_argv][ic+1] == 0 || quote_follows(argv[i_argv]+ic,"\"",true))))
				*parg++ = '\\';
			*parg = argv[i_argv][ic];
		}
		strcpy(parg, "\"");
		nargv[i_argv] = arg;
	}
	return nargv;
}
static int vs_exec_free(int ret, WCHAR ** wargv, char ** nargv, void * otherptr) {
	if (wargv) {
		for (int i = 0; wargv[i] != NULL; ++i)
			free(wargv[i]);
		free(wargv);
	}
	if (nargv) {
		for (int i = 0; nargv[i] != NULL; ++i)
			free(nargv[i]);
		free(nargv);
	}
	if (otherptr)
		free(otherptr);
	return ret;
}
int vs_execv(unsigned int flags, const char * file, char *const* argv) {
	WCHAR wfile[VSCOMMON_PATH_MAX];
	WCHAR ** wargv = NULL;
	int ret = 0;
	int argc;

	if (utf8_to_wstr(wfile, file, sizeof(wfile)/sizeof(*wfile)) == (size_t)-1)
		return -1;
	VS_LOG("common",logvs::NOTICE, "running %s", file);
	fflush(NULL);

	// as indicated in microsoft reference for exec*, args must be quoted.
	char ** nargv = quote_argv(argv);
	if (nargv != NULL) argv = nargv;

	for (argc = 0; argv[argc] != NULL; ++argc) ; /*loop*/
	if ((wargv = (WCHAR**) malloc((argc+1) * sizeof(*wargv))) == NULL) {
		return vs_exec_free(-1, wargv, nargv, NULL);
	}
	for (int i = 0; i < argc; ++i) {
		WCHAR * warg = NULL;
		if (utf8_to_wstr(&warg, argv[i]) == (size_t)-1 || warg == NULL)
			return vs_exec_free(-1, wargv, nargv, warg);
		wargv[i] = warg;
	}
	wargv[argc] = NULL;

	if ((flags & VEF_EXEC) != 0) {
		if ((flags & VEF_PATH) != 0)
	    	ret = _wexecvp(wfile, wargv) == 0 ? -1 : -1;
		else
			ret = _wexecv(wfile, wargv) == 0 ? -1 : -1;
	}
	else {
		if ((flags & VEF_PATH) != 0)
			ret = _wspawnvp((flags & VEF_WAIT) != 0 ? P_WAIT : P_NOWAIT, wfile, wargv);
		else
			ret = _wspawnv((flags & VEF_WAIT) != 0 ? P_WAIT : P_NOWAIT, wfile, wargv);
	}
	return vs_exec_free(ret, wargv, nargv, NULL);
}
#else // ! _WIN32
int vs_execv(unsigned int flags, const char * file, char *const* argv) {
	int ret = 0;
	pid_t pid;

	VS_LOG("common",logvs::NOTICE, "running %s", file);
	fflush(NULL);

	if ((flags & VEF_EXEC) != 0 || (pid = fork()) == 0) {
		if ((flags & VEF_PATH) != 0)
			ret = execvp(file, argv) == 0 ? -1 : -1;
		else
			ret = execv(file, argv) == 0 ? -1 : -1;
	} else if ((flags & VEF_EXEC) == 0) {
		if (pid == -1) {
			VS_LOG("common", logvs::ERROR, "error: cannot fork");
			return -1;
		}
		if ((flags & VEF_WAIT) != 0) {
			int status;
			ret = waitpid(pid, &status, 0) != -1 ? 0 : -1;
		}
	}
	return ret;
}
#endif // ! _WIN32

int vs_execl(unsigned int flags, const char * file, const char * arg0, ...) {
	va_list valist;
	int ret, argc;
	const char * arg;

	va_start(valist, arg0);
	for (argc = 1; (arg = va_arg(valist, const char *)) != NULL; ++argc) /*loop*/ ;
	va_end(valist);

	char ** argv = (char**) malloc((argc + 1) * sizeof(*argv));
	if (argv == NULL)
		return -1;
	argv[0] = (char*)arg0;

	va_start(valist, arg0);
	for (int i = 1; (arg = va_arg(valist, const char *)) != NULL; ++i) {
		argv[i] = (char*)arg;
	}
	va_end(valist);
	argv[argc] = NULL;

	ret = vs_execv(flags, file, argv);
	free(argv);
	return ret;
}

} // ! namespace VSCommon
