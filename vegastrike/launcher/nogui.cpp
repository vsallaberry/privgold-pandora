
#include "launcher.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

int RunInterface(int * pargc, char *** pargv) {
    (void)pargc;
    (void)pargv;

    changeToData();
    if (VSLAUNCH_RUN_PROCESS(vegastrikebin.c_str(), vegastrikebin.c_str(), NULL) != 0) {
        fprintf(stderr, "ERROR: cannot launch %s\n", vegastrikebin.c_str());
    } else return -1; // should not happen on POSIX (macos/unix/linux)
    return 0;
}

