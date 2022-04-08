/*
* Vega Strike
* Copyright (C) 2022 Vincent Sallaberry
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
#if defined(_WIN32) //|| defined(WIN32) || defined(WINDOWS) || defined(_WINDOWS) 
# include <windows.h>
# include <process.h>
# ifdef SHORTCUT_EXECPATH
#  define doexec(name, ...) execvp(name, __VA_ARGS__) //_spawnvp(P_NOWAIT, name, __VA_ARGS__)
# else
#  define doexec(name, ...) execv(name, __VA_ARGS__) //_spawnv(P_NOWAIT, name, __VA_ARGS__)
#endif
# define EXEC_CONST // for exec
//# define EXEC_CONST const // for spawn
# define PATHSEP ("\\")
# if defined(__CYGWIN__) || defined(__MINGW__)
#  include <io.h>
# else
#  include <direct.h>
# endif
#else // ! _WIN32
# include <unistd.h>
# ifdef SHORTCUT_EXECPATH
#  define doexec(name, ...) execvp(name, __VA_ARGS__)
# else
#  define doexec(name, ...) execv(name, __VA_ARGS__)
# endif
# define EXEC_CONST
# define PATHSEP ("/")
#endif // ! _WIN32
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifndef SHORTCUTS
# define SHORTCUTS {"privgold","bin/vslauncher.exe","--run",NULL,"privsetup","bin/vssetup.exe",NULL,NULL};
#endif
static EXEC_CONST char * const shortcuts[] = SHORTCUTS;

#if defined(_WIN32)
static char * basename_from_argv0(const wchar_t * ws) {
    char * s = malloc((wcslen(ws) + 1) * sizeof(*s));
    size_t i;
    if (s) { for (i = 0; ws[i]; ++i) { s[i] = ws[i] < 256 ? ws[i]&0xff : '?'; }
             s[i] = 0; }
    return s;
}
#define chdir(d) _wchdir(d)
#define STR_FMT "%ls"
#else
#define STR_FMT "%s"
#define basename_from_argv0(s) strdup(s) 
#endif

#if defined(_WIN32)
static char ** quote_argv(EXEC_CONST char *const* argv);
#endif
#if defined(_WIN32) && defined(_WINDOWS)
static int ParseCmdLine(const char * cmdline, int * argc, char *** argv);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
    char ** argv;
    int argc;
    ParseCmdLine(lpCmdLine, &argc, &argv);
#else
int main(int argc, char *argv[]) {
#endif
#if defined(_WIN32)
    wchar_t argv0[16384] = { 0, };
    argv0[sizeof(argv0)/sizeof(*argv0) - 1] = 0;
    GetModuleFileNameW(NULL, argv0, sizeof(argv0)/sizeof(*argv0) - 1);
    wchar_t * tmp = argv0 + wcslen(argv0) - 1;
#else
    char * argv0 = strdup(argv[0]), * tmp = argv0 + strlen(argv0) - 1;
#endif
    EXEC_CONST char * const * shortcut = shortcuts;
    char * basename, ** nargv = NULL;
    int ret;

    while (tmp >= argv0 && *tmp != '/' && *tmp != *PATHSEP) {
        --tmp;
    }
    basename = basename_from_argv0(tmp+1);
    if (tmp >= argv0) { tmp[0] = *PATHSEP; tmp[1] = 0; } else { argv0[0] = '.'; argv0[1] = 0; }
    fprintf(stderr, "in path " STR_FMT ", program:%s\n", argv0, basename); 
    chdir(argv0);

#if !defined(_WIN32)
    free(argv0);
#elif defined(_WINDOWS)
    free(argv[0]);
    free(argv);
#endif

    while (*shortcut != NULL && strncmp(*shortcut, basename, strlen(*shortcut))) {
        while (*shortcut++ != NULL) ;
    }
    free(basename);
    if (*shortcut == NULL) 
        shortcut = shortcuts;
    fprintf(stderr, "run %s %s\n", shortcut[1], shortcut[2]);
#if defined(_WIN32)
    nargv = quote_argv(shortcut+1);
#endif
    if ((ret = doexec(shortcut[1], nargv != NULL ? (EXEC_CONST char *const*) nargv : shortcut+1)) == -1) {
        fprintf(stderr, "error launching %s\n", shortcut[1]);
    }
#if defined(_WIN32)
    if (nargv != NULL) { for (int i = 0; nargv[i] != NULL; ++i) { free(nargv[i]); }; free(nargv); }
#endif
    return ret;
}

#if defined(_WIN32)
static int quote_follows(const char *s, int match0) {
    while (*s == '\\') ++s;
    return *s == '"' || (match0 && *s == 0);
}
static char ** quote_argv(EXEC_CONST char *const* argv) {
    // as indicated in microsoft reference for exec*, args must be quoted.
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
            ||  (argv[i_argv][ic] == '\\' && (quote_follows(argv[i_argv]+ic,1) || argv[i_argv][ic+1] == 0)))
                *parg++ = '\\';
            *parg = argv[i_argv][ic];
        }
        strcpy(parg, "\"");
        nargv[i_argv] = arg;
    }
    return nargv;
}
# if defined(_WINDOWS)
static int ParseCmdLine(const char * cmdline, int * pargc, char *** pargv) {
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
            if (escape && *lpCmdLine == '\\' && quote_follows(lpCmdLine,0)) { *arg++ = *(++lpCmdLine); ++lpCmdLine; }
            else if (*lpCmdLine == '"') { lpCmdLine++; escape = !escape; }
            else *arg++ = *lpCmdLine++;
        }
        while (*lpCmdLine == ' ') ++lpCmdLine;
        *arg = 0;
    }
    argv[argc] = NULL;
    *pargv = argv;
    *pargc = argc;
    return 0;
}
# endif // ! _WINDOWS
#endif // ! _WIN32

