#ifndef _VS_LAUNCHER_LAUNCHER_H
#define _VS_LAUNCHER_LAUNCHER_H

#include <stdlib.h>
#include <stdarg.h>
#include <string>

#include "log.h"


extern char * prog_arg;
extern std::string origpath;
extern std::string datadir;
extern std::pair<std::string,std::string> homedir;
extern std::string configfile;
extern std::pair<std::string,std::string> bindir;
extern std::string resourcesdir;
extern std::string vegastrikebin;
extern std::string glengine;
extern std::string setupprog;
extern std::string vssetupbin;

bool changeToData();
bool changehome();

int RunInterface(int * pargc, char *** pargv);

bool checkModifier();

#endif // _VS_LAUNCHER_LAUNCHER_H
