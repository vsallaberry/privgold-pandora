
#include "launcher.h"

#include <stdlib.h>
#include <stdio.h>

int RunInterface(int * pargc, char *** pargv) {
    (void)pargc;
    (void)pargv;

    changeToData();
    std::string program = checkModifiers() ? vssetupbin : vegastrikebin;

    if (VSLAUNCH_RUN_PROCESS(program.c_str(), program.c_str(), NULL) == -1) {
        fprintf(stderr, "ERROR: cannot launch %s\n", program.c_str());
    } else return 0; // should not happen on POSIX (macos/unix/linux)
}

