#if defined(_WIN32) //|| defined(WIN32) || defined(WINDOWS) || defined(_WINDOWS) 
# include <windows.h>
# include <process.h>
# ifdef SHORTCUT_EXECPATH
#  define doexec(name, ...) _spawnvp(P_NOWAIT, name, __VA_ARGS__)
# else
#  define doexec(name, ...) _spawnv(P_NOWAIT, name, __VA_ARGS__)
#endif
# define EXEC_CONST const
# define PATHSEP ("\\")
# if defined(__CYGWIN__) || defined(__MINGW__)
#  include <io.h>
# else
#  include <direct.h>
# endif
#else
# include <unistd.h>
# ifdef SHORTCUT_EXECPATH
#  define doexec(name, ...) execvp(name, __VA_ARGS__)
# else
#  define doexec(name, ...) execv(name, __VA_ARGS__)
# endif
# define EXEC_CONST
# define PATHSEP ("/")
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifndef SHORTCUTS
# define SHORTCUTS {"privgold","bin/vslauncher.exe","--run",NULL,"privsetup","bin/vssetup.exe",NULL,NULL};
#endif
static EXEC_CONST char * const shortcuts[] = SHORTCUTS;

#if defined(_WINDOWS) && defined(_WIN32)
static int ParseCmdLine(const char * cmdline, int * argc, char *** argv);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
    char ** argv, *argv0;
    int argc;
    ParseCmdLine(lpCmdLine, &argc, &argv);
    argv0 = argv[0];
#else
int main(int argc, char *argv[]) {
    char * argv0 = strdup(argv[0]);
#endif
    EXEC_CONST char * const * shortcut = shortcuts;
    char * basename;
    char * tmp = argv0 + strlen(argv0) - 1;

    while (tmp >= argv0 && *tmp != '/' && *tmp != *PATHSEP) {
        --tmp;
    }
    basename = strdup(tmp+1);
    if (tmp >= argv0) strcpy(tmp, PATHSEP); else strcpy(argv0, ".");
    fprintf(stderr, "in path %s, program:%s\n", argv0, basename); 
    chdir(argv0);
    free(argv0);
    if (argv0 == argv[0]) free(argv);

    while (*shortcut != NULL && strncmp(*shortcut, basename, strlen(*shortcut))) {
        while (*shortcut++ != NULL) ;
    }
    free(basename);
    if (*shortcut == NULL) 
        shortcut = shortcuts;
    fprintf(stderr, "run %s %s\n", shortcut[1], shortcut[2]);
    if (doexec(shortcut[1], shortcut+1) == -1) {
        fprintf(stderr, "error launching %s\n", shortcut[1]);
        return -1;
    }
    return 0;
}

#if defined(_WINDOWS) && defined(_WIN32)
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
            if (*lpCmdLine == '\\' && lpCmdLine[1] == '"') { *arg++ = *(++lpCmdLine); ++lpCmdLine; }
            else if (*lpCmdLine == '"') { lpCmdLine++; escape = !escape; }
            else *arg++ = *lpCmdLine++;
        }
        *arg = *lpCmdLine++ = 0;
    }
    argv[argc] = NULL;
    *pargv = argv;
    *pargc = argc;
    return 0;
}
#endif

