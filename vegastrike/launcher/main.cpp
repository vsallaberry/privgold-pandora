#include <string>
#if defined(_WIN32) && _MSC_VER > 1300 
#define __restrict
#endif
#ifdef HAVE_CONFIG_H
# include "config.h"
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

static bool notFoundOrSame(const char * file, VSCommon::file_id_t * id_other) {
    VSCommon::file_id_t id;
    if (!VSCommon::getFileId(file, &id))
        return true;
    if (!id_other)
        return false;
    return VSCommon::fileIdCompare(&id, id_other) == 0;
}

enum vsl_flags { F_NONE = 0, F_RUN_VEGASTRIKE = 1 << 0, F_RUN_VSSETUP = 1 << 1 };

#if defined(_WINDOWS)&&defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
    char ** argv;
    int argc;
    VSCommon::ParseCmdLine(lpCmdLine, &argc, &argv);
#else
int main( int   argc,
          char *argv[] )
{
#endif
    char tmppwd[65535];
    unsigned int flags = F_NONE;

    bool has_console = VSCommon::InitConsole();
    logvs::log_setfile(stderr);
    logvs::log_setflag(logvs::F_QUEUELOGS, true);

    getcwd(tmppwd,sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
    VS_LOG("vslauncher", logvs::NOTICE, " In path %s", tmppwd);
    origpath = tmppwd;
    prog_arg = argv[0];
    
    // command line options
    for (int i_argv = 1; i_argv < argc; ++i_argv) {
        if (!strcmp(argv[i_argv], "--run")) {
            flags |= F_RUN_VEGASTRIKE;
        } else if (!strcmp(argv[i_argv], "--setup")) {
            flags |= F_RUN_VSSETUP;
        } else if (has_console) {
            logvs::log_printf("Usage: %s [--run|--setup]\n", argv[0]);
            exit(0);
        }
    }

    // Init DATADIR
    // Will change to the data dir which makes selecting missions easier.
    if ((datadir = VSCommon::getdatadir(origpath.c_str())).empty()) { 
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
    VSCommon::file_id_t myid;
    getFileId((bindir.first + "/" + bindir.second).c_str(), &myid);

    // Vegastrike binary
    for (const char ** bin = vslaunch_binsearchs; *bin; ++bin) {
        std::string vega_base((bindir.first + "/" + *bin) + "/" VSLAUNCH_BINARY);
        std::string vega = (vega_base + ".") + glengine + VSLAUNCH_EXE_EXT;
        if (notFoundOrSame(vega.c_str(), &myid)) {
            vega = (vega_base + "." VSLAUNCH_GLENGINE_DEF VSLAUNCH_EXE_EXT);
            if (notFoundOrSame(vega.c_str(), &myid)) {
                vega = vega_base + VSLAUNCH_EXE_EXT;
                if (notFoundOrSame(vega.c_str(), &myid)) {
                    continue ; // same as me or not found
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
        std::string vega((bindir.first + "/" + *bin) + "/" VSLAUNCH_SETUP_BINARY VSLAUNCH_EXE_EXT);
        if (notFoundOrSame(vega.c_str(), &myid)) {
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
    if ((flags & F_RUN_VEGASTRIKE) != 0 || ((flags & F_RUN_VSSETUP) == 0 &&
        strncmp(bindir.second.c_str(), VSLAUNCH_BINARY, sizeof(VSLAUNCH_BINARY)-1) == 0)) {
        std::string program = !checkModifier() ? vegastrikebin : vssetupbin;
        changeToData();
        if (VSLAUNCH_RUN_PROCESS(program.c_str(), program.c_str(), NULL) == -1) {
            VS_LOG("vslauncher", logvs::ERROR, "ERROR: cannot launch %s", program.c_str());
        } else return 0; // should not happen on macos/bsd/linux
    } else if ((flags & F_RUN_VSSETUP) != 0 
               || strncmp(bindir.second.c_str(), VSLAUNCH_SETUP_BINARY, sizeof(VSLAUNCH_SETUP_BINARY)-1) == 0) {
        changeToData();
        if (VSLAUNCH_RUN_PROCESS(vssetupbin.c_str(), vssetupbin.c_str(), NULL) == -1) {
            VS_LOG("vslauncher", logvs::ERROR, "ERROR: cannot launch %s", vssetupbin.c_str());
        } else return 0; // should not happen on macos/bsd/linux
    }

    int ret = RunInterface(&argc, &argv);
#if defined(_WINDOWS)&&defined(_WIN32)
    VSCommon::ParseCmdLineFree(argv);
#endif
    return ret;
}

// alt/option key detection
#if defined(__APPLE__) && defined(HAVE_CARBON)
# ifdef __clang__
#  include <Carbon/Carbon.h>
# else
# define optionKey (1 << 11)
extern "C" unsigned int GetCurrentKeyModifiers();
# endif
bool checkModifier() {
    unsigned int modifiers = GetCurrentKeyModifiers();
    //EventRef eventRef = GetCurrentEvent();
    //int class = GetEventClass(eventRef);
    //int kind = GetEventKind(eventRef);
    //fprintf(stderr, "%d.%d\n", class, kind);
    //(modifiers & shiftKey)    // (1 << 9)     // (right 13)
    //(modifiers & optionKey)   // (1 << 11)    // (right 14)
    //(modifiers & cmdKey)      // (1 << 8)   
    //(modifiers & controlKey)  // (1 << 12)    // (right 15)
    //(modifiers & alphaLock)   // (1 << 10)
    return (modifiers & optionKey) != 0;
}
#else
bool checkModifier() {
    return false;
}
#endif

