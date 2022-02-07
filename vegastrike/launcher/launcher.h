#ifndef _VS_LAUNCHER_LAUNCHER_H
#define _VS_LAUNCHER_LAUNCHER_H

#include <stdlib.h>
#include <stdarg.h>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <process.h>
# define VSLAUNCH_RUN_PROCESS(name, ...) spawnl(P_NOWAIT, name, __VA_ARGS__) //args terminated by NULL
#else // ! defined(_WIN32)
#include <unistd.h>
# define VSLAUNCH_RUN_PROCESS(name, ...) (execlp(name, __VA_ARGS__) == 0 ? -1 : -1) //args terminated by NULL
#endif // ! defined(_WIN32)

extern char * prog_arg;
extern std::string origpath;
extern std::string datadir;
extern std::string homedir;
extern std::string configfile;
extern std::string vegastrikebin;
extern std::string glengine;
extern std::string vssetupbin;

bool changeToData ();
bool changehome();

int RunInterface(int * pargc, char *** pargv);

#endif // _VS_LAUNCHER_LAUNCHER_H
