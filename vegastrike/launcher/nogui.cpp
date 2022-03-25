
#include "launcher.h"
#include "common/common.h"

#include <stdlib.h>
#include <stdio.h>

int RunInterface(int * pargc, char *** pargv) {
    (void)pargc;
    (void)pargv;

    changeToData();
    std::string program = checkModifier() ? vssetupbin : vegastrikebin;

    if (VSCommon::vs_execl(VSCommon::VEF_EXEC, program.c_str(), program.c_str(), NULL) == -1) {
        fprintf(stderr, "ERROR: cannot launch %s\n", program.c_str());
        return -1;
    } else return 0; // should not happen on POSIX (macos/unix/linux)
}

