#include <string>
#if defined(_WIN32) && _MSC_VER > 1300 
#define __restrict
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#include <sys/stat.h>
#else
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "src/common/common.h"
#include "general.h"
#include "launcher.h"
#include "src/log.h"

char * prog_arg=NULL;
std::string origpath;
std::string datadir;
std::string homedir;
std::string configfile;
std::string vegastrikebin;
std::string glengine;
std::string vssetupbin;

#ifdef _WIN32
#define VSLAUNCH_EXE_EXT ".exe"
#else
#define VSLAUNCH_EXE_EXT ""
#endif

#define VSLAUNCH_SETUPCONF          "setup.config"
#define VSLAUNCH_SETUPCONF_FILE_KW  "config_file"
#define VSLAUNCH_CONFFILE           "vegastrike.config"
#define VSLAUNCH_GLENGINE_KW        "GL-Renderer"
#define VSLAUNCH_BINARY             "vegastrike"
#define VSLAUNCH_SETUP_BINARY       "vssetup"
#define VSLAUNCH_GLENGINE_DEF       "SDL2"

static const char * vslaunch_binsearchs[] = { ".", "bin", "..", "../bin", NULL };
static const char * vslaunch_setupbinsearchs[] = { ".", "bin", "..", "../bin", "setup", "../setup", NULL };

bool changeToData () {
   return chdir(datadir.c_str()) == 0;
}

bool changehome() {
    return chdir(homedir.c_str()) == 0;
}

bool InitConfig() {
    // Check configfile
    char line[MAX_READ],  * p, * parm, * n_parm;
    FILE * fp;
    configfile = "";
    chdir(homedir.c_str());
    if ((fp = fopen(VSLAUNCH_SETUPCONF, "r")) == NULL) {
        changeToData();
        fp = fopen(VSLAUNCH_SETUPCONF, "r");
    }
    if (fp != NULL) {
        while ((p = fgets(line, sizeof(line), fp)) != NULL) {
            if (line[0] == '#') { continue; }
            chomp(line);
            if (line[0] == '\0') { continue; }
            parm = line;
            n_parm = next_parm(parm);
            if (strcmp(VSLAUNCH_SETUPCONF_FILE_KW, parm) == 0) {
                if (!configfile.empty()) { VS_LOG("config", logvs::WARN, "Duplicate config_file in config file"); continue; }
                if (n_parm[0] == '\0') { VS_LOG("config", logvs::WARN, "Missing parameter for config_file"); continue; }
                configfile = n_parm;
                break ;
            }
        }
        fclose(fp);
    }
    if (configfile.empty()) {
        configfile = VSLAUNCH_CONFFILE; 
        VS_LOG("config", logvs::WARN, "Warning: cannot get configfile name, using %s", configfile.c_str());
    } else {
        VS_LOG("config", logvs::WARN, "Using configfile %s", configfile.c_str());
    }
    chdir(homedir.c_str());
    if ((fp = fopen(configfile.c_str(), "r")) == NULL) {
        changeToData();
        fp = fopen(configfile.c_str(), "r");
        configfile = datadir + "/" + configfile;
    } else {
        configfile = homedir + "/" + configfile;
    }
    if (fp != NULL) {
        VS_LOG("config", logvs::NOTICE, "Found configfile %s", configfile.c_str());
        while ((p = fgets(line, MAX_READ, fp)) != NULL) {
            if (line[0] != '#') { continue; }
            chomp(line);
            if (line[0] == '\0') { continue; }
            parm = line;
            n_parm = next_parm(parm);
            if (strcmp("#set", parm) == 0 && strncmp(n_parm, VSLAUNCH_GLENGINE_KW, sizeof(VSLAUNCH_GLENGINE_KW)-1) == 0) {
                glengine = next_parm(n_parm);
                VS_LOG("config", logvs::NOTICE, "config: using GL engine %s", glengine.c_str());
                break ;
            }
        }
        fclose(fp);
    }
    return true;
}

#if defined(_WINDOWS)&&defined(_WIN32)
typedef char FileNameCharType [65535];
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
	FileNameCharType argvc;
	FileNameCharType *argv= &argvc;
	GetModuleFileName(NULL, argvc, 65534);
	int argc=0;
#else
int main( int   argc,
          char *argv[] )
{
#endif
    char tmppwd[65535];
    
    VSCommon::InitConsole();
    logvs::log_setfile(stderr);
    logvs::log_setflag(logvs::F_QUEUELOGS, true);

    getcwd(tmppwd,sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
    VS_LOG("vslauncher", logvs::NOTICE, " In path %s", tmppwd);
    origpath = tmppwd;
    prog_arg = argv[0];
 
    // Init DATADIR
    if ((datadir = VSCommon::getdatadir(origpath.c_str())).empty()) { // Will change to the data dir which makes selecting missions easier.
        datadir = origpath; 
        VS_LOG("vslauncher", logvs::WARN, "Warning: cannot find data, using %s", datadir.c_str());
    } else {
        VS_LOG("vslauncher", logvs::NOTICE, "Found data in %s", datadir.c_str());
    }
    // Init HOMEDIR
    if ((homedir = VSCommon::gethomedir(datadir.c_str())).empty()) {
        homedir = origpath; 
        VS_LOG("vslauncher", logvs::WARN, "Warning: cannot find home, using %s", homedir.c_str());
    } else {
        VS_LOG("vslauncher", logvs::NOTICE, "Found home in %s", homedir.c_str());
    }
    // Init CONFIGFILE
    InitConfig();
    // Init Log. TODO read config to see we where we want to log
    logvs::log_openfile("", homedir + "/vslauncher.log", /*redirect=*/true, /*append=*/false);
    atexit(logvs::log_terminate);

    // Binary dir
    std::pair<std::string,std::string> bindir = VSCommon::getbindir(argv[0], origpath.c_str());
    if (bindir.first.empty()) {
        bindir.first = origpath;
        VS_LOG("vslauncher", logvs::WARN, "Warning: cannot find binary dir, using %s", bindir.first.c_str());
    } else {
        VS_LOG("vslauncher", logvs::NOTICE, "Found binary directory in %s (executable is %s)", bindir.first.c_str(), bindir.second.c_str());
    }
    struct stat myst;
    stat((bindir.first + "/" + bindir.second).c_str(), &myst);

    // Vegastrike binary
    for (const char ** bin = vslaunch_binsearchs; *bin; ++bin) {
        std::string vega_base((bindir.first + "/" + *bin) + "/" VSLAUNCH_BINARY);
        std::string vega = (vega_base + ".") + glengine + VSLAUNCH_EXE_EXT;
        struct stat st;
        if (stat(vega.c_str(), &st) < 0 || (st.st_dev == myst.st_dev && st.st_ino == myst.st_ino)) {
            vega = (vega_base + "." VSLAUNCH_GLENGINE_DEF) + VSLAUNCH_EXE_EXT;
            if (stat(vega.c_str(), &st) < 0 || (st.st_dev == myst.st_dev && st.st_ino == myst.st_ino)) {
                vega = vega_base + VSLAUNCH_EXE_EXT;
                if (stat(vega.c_str(), &st) < 0 || (st.st_dev == myst.st_dev && st.st_ino == myst.st_ino)) {
                    continue ; // same as my on not found
                }
            }
        }
        vegastrikebin = vega;
        VS_LOG("vslauncher", logvs::NOTICE, "Found vegastrike binary: %s", vegastrikebin.c_str());
        break ;
    }
    if (vegastrikebin.empty()) {
        VS_LOG("vslauncher", logvs::WARN, "Warning: cannot find vegastrike binary, using %s", vegastrikebin.c_str());
    }

    // vssetup binary
    for (const char ** bin = vslaunch_setupbinsearchs; *bin; ++bin) {
        std::string vega((bindir.first + "/" + *bin) + "/" VSLAUNCH_SETUP_BINARY);
        struct stat st;
        if (stat(vega.c_str(), &st) < 0 || (st.st_dev == myst.st_dev && st.st_ino == myst.st_ino)) {
            continue ; // same as my on not found
        }
        vssetupbin = vega;
        VS_LOG("vslauncher", logvs::NOTICE, "Found vssetup binary: %s", vssetupbin.c_str());
        break ;
    }
    if (vssetupbin.empty()) {
        VS_LOG("vslauncher", logvs::WARN, "Warning: cannot find vssetup binary, using %s", vssetupbin.c_str());
    }

    // Quick launch mode if the launcher has the name of vegastrike or vssetup
    if (strncmp(bindir.second.c_str(), VSLAUNCH_BINARY, sizeof(VSLAUNCH_BINARY)-1) == 0) {
        changeToData();
        if (VSLAUNCH_RUN_PROCESS(vegastrikebin.c_str(), vegastrikebin.c_str(), NULL) != 0) {
            VS_LOG("vslauncher", logvs::ERROR, "ERROR: cannot launch %s", vegastrikebin.c_str());
        } else return -1; // should not happen
    } else if (strncmp(bindir.second.c_str(), VSLAUNCH_SETUP_BINARY, sizeof(VSLAUNCH_SETUP_BINARY)-1) == 0) {
        changeToData();
        if (VSLAUNCH_RUN_PROCESS(vssetupbin.c_str(), vssetupbin.c_str(), NULL) != 0) {
            VS_LOG("vslauncher", logvs::ERROR, "ERROR: cannot launch %s", vssetupbin.c_str());
        } else return -1; // should not happen
    }

    return RunInterface(&argc, &argv);
}

