/*
 * Copyright (C) 2001-2009 VegaStrike developers
 * Copyright (C) 2022 Vincent Sallaberry
 *
 * vegastrike launcher, for vegastrike (GPL) / version PrivateerGold
 *   http://vegastrike.sourceforge.net/
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
#include <string>
#if defined(_WIN32) && _MSC_VER > 1300 
#define __restrict
#endif
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef HAVE_VERSION_H
# include "version.h"
#else
# define SCM_VERSION "unknown"
# define SCM_REVISION "unknown"
# define SCM_REMOTE "unknown"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <process.h>
#include <sys/stat.h>
#else
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdarg.h>
#include <sstream>
#include "common/common.h"
#include "general.h"
#include "launcher.h"
#include "log.h"
#include "unicode.h"

#ifndef HAVE_CODECVT // not CXX11
# define static_assert(x,y)
#endif

#if defined(__linux__)
# include <dlfcn.h>
# include <sys/wait.h>
#endif

// ***************************************************************************
// External
// ***************************************************************************

char * prog_arg=NULL;
std::string origpath;
std::string datadir;
std::pair<std::string,std::string> homedir;
std::string configfilename, configfile;
std::pair<std::string,std::string> bindir;
std::string resourcesdir;
std::string vegastrikebin;
std::string glengine;
std::string setupprog;
std::string vssetupbin;

// ***************************************************************************
// Internal
// ***************************************************************************

#define VSLAUNCH_LOG(...) 			VS_LOG("vslauncher", __VA_ARGS__)

#ifdef _WIN32
#define VSLAUNCH_EXE_EXT 			".exe"
#else
#define VSLAUNCH_EXE_EXT 			""
#endif

#define VSLAUNCH_SETUPCONF          "setup.config"
#define VSLAUNCH_SETUPCONF_FILE_KW  "config_file"
#define VSLAUNCH_CONFFILE           "vegastrike.config"
#define VSLAUNCH_GLENGINE_KW        "GL-Renderer"
#define VSLAUNCH_SETUPPROG_KW       "SetupProg"
#define VSLAUNCH_BINARY             "vegastrike"
#define VSLAUNCH_SETUP_BINARY_DEF   "vssetup"
#define VSLAUNCH_SETUP_BINARY_ALT   "vssetup_dlg"
#define VSLAUNCH_GLENGINE_DEF       "SDL2"
#define VSLAUNCH_MAXLOGS			3

#define VSLAUNCH_ALSALIB_FROM_RES_PATH	    VEGASTRIKE_LIBDIR_NAME "/alsa-lib"
#define VSLAUNCH_ALSACONF_FROM_RES_PATH	    "etc/alsa"
#define VSLAUNCH_ALSAPLUGINS_FROM_RES_PATH  VEGASTRIKE_LIBDIR_NAME "/alsa-lib/plugins"

static const char * const vslaunch_binsearchs[] = { ".", "bin", "..", "../bin", "bin32", "../bin32", NULL };
static const char * const vslaunch_setupbinsearchs[] = { ".", "bin", "..", "../bin", "setup", "../setup", "bin32", "../bin32", NULL };
#if defined(_WIN32)
static const char * const vslaunch_oldhomes[] = { "privgold100", NULL };
#else
static const char * const vslaunch_oldhomes[] = { ".privgold100", NULL };
#endif

// ***************************************************************************
// Game Configuration Utilities
// ***************************************************************************

bool InitConfig() {
    // Check configfile
    char line[MAX_READ],  * p, * parm, * n_parm;
    FILE * fp;
    configfile = configfilename = "";
    VSCommon::vs_chdir(homedir.first.c_str());
    if ((fp = VSCommon::vs_fopen(VSLAUNCH_SETUPCONF, "r")) == NULL) {
        changeToData();
        fp = VSCommon::vs_fopen(VSLAUNCH_SETUPCONF, "r");
    }
    if (fp != NULL) {
        while ((p = fgets(line, sizeof(line), fp)) != NULL) {
            if (line[0] == '#') { continue; }
            chomp(line);
            if (line[0] == '\0') { continue; }
            parm = line;
            n_parm = next_parm(parm);
            if (strcmp(VSLAUNCH_SETUPCONF_FILE_KW, parm) == 0) {
                if (!configfile.empty()) { VSLAUNCH_LOG(logvs::WARN, "Duplicate config_file in config file"); continue; }
                if (n_parm[0] == '\0') { VSLAUNCH_LOG(logvs::WARN, "Missing parameter for config_file"); continue; }
                configfilename = n_parm;
                break ;
            }
        }
        fclose(fp);
    }
    if (configfilename.empty()) {
        configfilename = VSLAUNCH_CONFFILE;
        VSLAUNCH_LOG(logvs::WARN, "Warning: cannot get configfile name, using %s", configfilename.c_str());
    } else {
        VSLAUNCH_LOG(logvs::NOTICE, "configfile name is %s", configfilename.c_str());
    }
    VSCommon::vs_chdir(homedir.first.c_str());
    if ((fp = VSCommon::vs_fopen(configfilename.c_str(), "r")) == NULL) {
        changeToData();
        fp = VSCommon::vs_fopen(configfilename.c_str(), "r");
        configfile = VS_PATH_JOIN(datadir.c_str(), configfilename.c_str());
    } else {
    	VSCommon::file_info_t homeconf_info, dataconf_info;
    	std::string data_configfile = VS_PATH_JOIN(datadir.c_str(), configfilename.c_str());
    	configfile = VS_PATH_JOIN(homedir.first.c_str(), configfilename.c_str());

    	if (VSCommon::getFileInfo(data_configfile, &dataconf_info)
    	&&  VSCommon::getFileInfo(configfile, &homeconf_info) && homeconf_info.mtime < dataconf_info.mtime) {
    		VSLAUNCH_LOG(logvs::WARN, "Warning, the data config file is newer than the home one, please run setup!");
#if 0 // Modification times are not preserved on windows when unzipping with windows zip extract
    		std::string homeconf_backup = VSCommon::getsuffixedfile(configfile, (time_t)-1, 3);
    		if (VSCommon::fileCopyIfDifferent(configfile,  homeconf_backup, 0) >= 0
    		&&  VSCommon::fileCopyIfDifferent(data_configfile, configfile,  0) >= 0) {
    			VSLAUNCH_LOG(logvs::NOTICE, "home config file backuped in %s and replaced by data config file.", homeconf_backup.c_str());
    		} else {
    			VSLAUNCH_LOG(logvs::WARN, "Warning, cannot backup/replace outdated home dir config file.");
    		}
#endif
    	}
    }
    if (fp == NULL) {
        VSLAUNCH_LOG(logvs::WARN, "Warning cannot find configfile %s", configfile.c_str());
    } else {
        VSLAUNCH_LOG(logvs::NOTICE, "Found configfile %s", configfile.c_str());
        while ((p = fgets(line, MAX_READ, fp)) != NULL) {
            if (line[0] != '#') { continue; }
            chomp(line);
            if (line[0] == '\0') { continue; }
            parm = line;
            n_parm = next_parm(parm);
            if (n_parm != NULL && strcmp("#set", parm) == 0) {
                if (strncmp(n_parm, VSLAUNCH_GLENGINE_KW, sizeof(VSLAUNCH_GLENGINE_KW)-1) == 0) {
                    glengine = next_parm(n_parm);
                    VSLAUNCH_LOG(logvs::NOTICE, "config: using GL engine %s", glengine.c_str());
                } else if (strncmp(n_parm, VSLAUNCH_SETUPPROG_KW, sizeof(VSLAUNCH_SETUPPROG_KW)-1) == 0) {
                    setupprog = next_parm(n_parm);
                    VSLAUNCH_LOG(logvs::NOTICE, "config: using Setup Prog %s", setupprog.c_str());
                }
            } else if (strcmp("#endheader", parm) == 0) {
                break ;
            }
        }
        fclose(fp);
    }
    return true;
}

// ***************************************************************************
// Game Directories/Files Utilities
// ***************************************************************************

bool changeToData () {
   return VSCommon::vs_chdir(datadir.c_str()) == 0;
}

bool changehome() {
    return VSCommon::vs_chdir(homedir.first.c_str()) == 0;
}

static bool notFoundOrSame(const char * file, VSCommon::file_info_t * id_other) {
    VSCommon::file_info_t id;
    if (!VSCommon::getFileInfo(file, &id))
        return true;
    if (!id_other)
        return false;
    return VSCommon::fileIdCompare(&id, id_other) == 0;
}
static inline bool notFoundOrSame(const std::string & file, VSCommon::file_info_t * id_other) {
	return notFoundOrSame(file.c_str(), id_other);
}

static int replaceHomeFile(const std::string & file,
		                   const std::string & srcdir,
		                   const std::string & dstdir, bool onlyIfExists = false) {
	int res = VSCommon::FILECOMP_DIFF;
	VSCommon::file_info_t info;
	std::string src = VS_PATH_JOIN(srcdir.c_str(), file.c_str());
	std::string dst = VS_PATH_JOIN(dstdir.c_str(), file.c_str());

	if (!onlyIfExists || VSCommon::getFileInfo(dst, &info)) {

		res = VSCommon::fileCopyIfDifferent(src, dst);

		if (res == VSCommon::FILECOMP_REPLACED) {
			VSLAUNCH_LOG(logvs::NOTICE, "file '%s' in homedir replaced by the one in datadir", file.c_str());
		} else if (res == VSCommon::FILECOMP_ERROR) {
			VSLAUNCH_LOG(logvs::ERROR, "error when replacing file '%s' by '%s'", dst.c_str(), src.c_str());
		}
	}

	return res;
}

static bool ImportOldHomeDir() {
	VSCommon::vsDIR * dir;
	VSCommon::vsdirent * dent;

	if ((dir = VSCommon::vs_opendir(VS_PATH_JOIN(homedir.first.c_str(), "save"))) != NULL) {
		while ((dent = VSCommon::vs_readdir(dir)) != NULL
				&& (!strcmp(dent->d_name, "..") || !strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "New_Game"))) ; /*loop*/
		VSCommon::vs_closedir(dir);
		if (dent != NULL)
			return false;
	}

	std::pair<std::string,std::string> homecomps = VSCommon::getfiledir(homedir.first);
	for (const char * const * oldhome = vslaunch_oldhomes; *oldhome; ++oldhome) {
		int copyret;
		std::string oldhomepath = VS_PATH_JOIN(homecomps.first.c_str(), *oldhome);
		std::string oldpath = VS_PATH_JOIN(oldhomepath.c_str(), "save", NULL);
		VSLAUNCH_LOG(logvs::NOTICE, "trying to import savegames from %s", oldpath.c_str());

		if ((copyret = VSCommon::fileCopyIfDifferent(oldpath,
				VS_PATH_JOIN(homedir.first.c_str(), "save"), 1)) == VSCommon::FILECOMP_SRCNOTFOUND)
			continue ;
		if (copyret < 0) {
			VSLAUNCH_LOG(logvs::ERROR, "error importing old savegames from %s", oldpath.c_str());
			break ;
		}

		oldpath = VS_PATH_JOIN(oldhomepath.c_str(), "serialized_xml");
		if (VSCommon::fileCopyIfDifferent(
					oldpath, VS_PATH_JOIN(homedir.first.c_str(), "serialized_xml"), 2) < 0) {
			VSLAUNCH_LOG(logvs::ERROR, "error importing old savegames from %s", oldpath.c_str());
			break ;
		}

		VSLAUNCH_LOG(logvs::NOTICE, "successfully imported savegames from %s", oldhomepath.c_str());
		return true;
	}

	return false;
}

bool CheckHomeDir() {

	// Check if we need to import games from a previous home directory
	ImportOldHomeDir();

	// Check Outdated Files
	if (replaceHomeFile("New_Game", datadir, VS_PATH_JOIN(homedir.first.c_str(), "save"), true) != VSCommon::FILECOMP_DIFF) {
		replaceHomeFile(VS_PATH_JOIN("serialized_xml", "New_Game", "tarsus.begin.csv"),
				        VS_PATH_JOIN(datadir.c_str(), homedir.second.c_str()), homedir.first);
	}
	replaceHomeFile("save.4.x.txt", VS_PATH_JOIN(datadir.c_str(), homedir.second.c_str()), homedir.first, true);
	replaceHomeFile(VSLAUNCH_SETUPCONF, datadir, homedir.first, true);

	return true;
}

// ***************************************************************************
// Environment variables utilities
// ***************************************************************************
bool CheckEnvironmentVariables() {
#if defined(_WIN32)
	return true;
#else
	//DISPLAY
	if (getenv("DISPLAY") == NULL)
		VSCommon::vs_setenv("DISPLAY", ":0.0", 1);
	if (getenv("XAUTHORITY") == NULL) {
		struct passwd * pwent = getpwuid (getuid());
		if (pwent && pwent->pw_dir)
			VSCommon::vs_setenv("XAUTHORITY", VS_PATH_JOIN(pwent->pw_dir, ".Xauthority").c_str(), 1);
	}
# if defined(__linux__)
	//temporary linux LD_LIBRARY_PATH
	const char * ldlib = getenv("LD_LIBRARY_PATH");
	char envstr[16384];

#if 0 // not needed anymore, as ld RUNPATH is used.
	snprintf(envstr, 16384, "%s%s%s/%s:%s/%s/gtk:%s/%s/misc", ldlib? liblib : "", ldlib? ":": "", resourcesdir.c_str(), VEGASTRIKE_LIBDIR_NAME,
             resourcesdir.c_str(), VEGASTRIKE_LIBDIR_NAME, resourcesdir.c_str(), VEGASTRIKE_LIBDIR_NAME);
	VSCommon::setenv("LD_LIBRARY_PATH", envstr, 1);
	ldlib = getenv("LD_LIBRARY_PATH");
#endif

	// We want to use the host alsa-lib in priority to take its sound config,
	// but if we can't we'll use embeded lib and config, this could conflict
	// with host pulseaudio/phonon/artsd (modprobe snd-pcm-oss can help when issue with alsalib).
	void * alsalib = dlopen("libasound.so.2", RTLD_LAZY);
	if (alsalib == NULL) alsalib = dlopen("libasound.so", RTLD_LAZY);
	if (alsalib == NULL) {
		VSLAUNCH_LOG(logvs::NOTICE, "this system does not seem to have alsalib, using embeded one");
		// alsalib not found, use embeded alsa library
		snprintf(envstr, sizeof(envstr), "%s%s%s/%s", ldlib ? ldlib : "", ldlib? ":" : "",
				 resourcesdir.c_str(), VSLAUNCH_ALSALIB_FROM_RES_PATH);
		VSCommon::vs_setenv("LD_LIBRARY_PATH", envstr, 1);
		snprintf(envstr, sizeof(envstr), "%s/" VSLAUNCH_ALSACONF_FROM_RES_PATH, resourcesdir.c_str());
		VSCommon::vs_setenv("ALSA_CONFIG_DIR", envstr, 0);
		snprintf(envstr, sizeof(envstr), "%s/" VSLAUNCH_ALSAPLUGINS_FROM_RES_PATH, resourcesdir.c_str());
		VSCommon::vs_setenv("ALSA_PLUGIN_DIR", envstr, 0);
		VSLAUNCH_LOG(logvs::NOTICE, "ALSA_CONFIG_DIR: %s", getenv("ALSA_CONFIG_DIR"));
		VSLAUNCH_LOG(logvs::NOTICE, "ALSA_PLUGIN_DIR: %s", getenv("ALSA_PLUGIN_DIR"));

	} else {
		const char * (*alsa_getconf)(void) = (const char * (*)(void)) dlsym(alsalib, "snd_config_topdir");
		const char * alsadir = NULL;
		if (alsa_getconf != NULL && (alsadir = alsa_getconf()) != NULL) {
			VSCommon::vs_setenv("ALSA_CONFIG_DIR", alsadir, 1);
			VSLAUNCH_LOG(logvs::NOTICE, "ALSA_CONFIG_DIR: %s\n", alsadir);
			dlclose(alsalib);
		} else {
			dlclose(alsalib);
			snprintf(envstr, sizeof(envstr), "%s/" VSLAUNCH_ALSACONF_FROM_RES_PATH, resourcesdir.c_str());
			static const char * alsaconfs[] = { "/usr/share/alsa", "/etc/alsa", NULL };
			for (const char * const * alsaconf = alsaconfs; *alsaconf; ++alsaconf) {
				struct stat st;
				if (VSCommon::vs_stat(*alsaconf, &st) == 0 && (st.st_mode & S_IFDIR) != 0) {
					snprintf(envstr, sizeof(envstr), "%s", *alsaconf);
					break ;
				}
			}
			VSCommon::vs_setenv("ALSA_CONFIG_DIR", envstr, 0);
			VSLAUNCH_LOG(logvs::NOTICE, "ALSA_CONFIG_DIR: %s", getenv("ALSA_CONFIG_DIR"));
		}
	}
# endif
	return true;
#endif
}

// ***************************************************************************
// MAIN
// ***************************************************************************
enum vsl_flags { F_NONE = 0, F_RUN = 1 << 0, F_RUN_VEGASTRIKE = F_RUN, F_RUN_VSSETUP,
	             F_RUN_DEF, F_RUN_GUI, F_RUN_END,
				 F_RUN_NEXT = 1 << 3, F_RUN_MASK = (F_RUN_NEXT - 1) & ~(F_RUN - 1) };
static_assert(F_RUN_END-1 < F_RUN_NEXT, "Not enough space for F_RUN, F_RUN_NEXT must be increased");

#if defined(_WINDOWS)&&defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
    char ** argv;
    int argc;
    unicodeInitLocale();
    VSCommon::ParseCmdLine(GetCommandLineW(), &argc, &argv);
#else
int main(int argc, char ** argv)
{
#endif
    char tmppwd[65535];
    unsigned int flags = F_NONE;

    bool has_console = VSCommon::InitConsole();
    logvs::log_setfile(stderr);
    logvs::log_setflag(logvs::F_QUEUELOGS, true);

    unicodeInitLocale();
    VSCommon::vs_getcwd(tmppwd,sizeof(tmppwd)-1); tmppwd[sizeof(tmppwd)-1] = 0;
    VSLAUNCH_LOG(logvs::NOTICE, " In path %s", tmppwd);
    origpath = tmppwd;
    prog_arg = argv[0];

    // command line options
    for (int i_argv = 1; i_argv < argc; ++i_argv) {
        if (!strcmp(argv[i_argv], "--run")) {
            flags = (flags & ~F_RUN_MASK) | F_RUN_DEF;
        } else if (!strcmp(argv[i_argv], "--game")) {
            flags = (flags & ~F_RUN_MASK) | F_RUN_VEGASTRIKE;
        } else if (!strcmp(argv[i_argv], "--gui")) {
            flags = (flags & ~F_RUN_MASK) | F_RUN_GUI;
        } else if (!strcmp(argv[i_argv], "--setup")) {
            flags = (flags & ~F_RUN_MASK) | F_RUN_VSSETUP;
        } else if (!strncmp(argv[i_argv], "-D", 2)) {
            datadir = std::string(argv[i_argv] + 2);
        } else if (has_console) {
        	if (!strcmp(argv[i_argv], "--version")) {
        		logvs::log_printf("vslauncher for vegastrike %s revision %s from %s\n",
        						  SCM_VERSION, SCM_REVISION, SCM_REMOTE);
        	} else {
        		logvs::log_printf("Usage: %s [--run|--game|--setup|--gui] [-D<DATADIR>] [--version]\n", argv[0]);
        	}
            exit(0);
        }
    }

    VSLAUNCH_LOG(logvs::NOTICE, "vslauncher for vegastrike %s revision %s from %s",
            						    SCM_VERSION, SCM_REVISION, SCM_REMOTE);
    // Binary dir
    bindir = VSCommon::getfiledir(argv[0], origpath.c_str());
    if (bindir.first.empty()) {
        bindir.first = origpath;
        VSLAUNCH_LOG(logvs::WARN, "Warning: cannot find binary dir, using %s", bindir.first.c_str());
    } else {
        VSLAUNCH_LOG(logvs::NOTICE, "Found binary directory in %s (executable is %s)", bindir.first.c_str(), bindir.second.c_str());
    }

    // Init DATADIR
    // Will change to the data dir which makes selecting missions easier.
    if ((!datadir.empty() && !(datadir = VSCommon::getdatadir(datadir)).empty())
    ||  !(datadir = VSCommon::getdatadir(bindir.first)).empty()) {
        VSLAUNCH_LOG(logvs::NOTICE, "Found data in %s", datadir.c_str());
        VSCommon::vs_chdir(datadir.c_str());
    }

    resourcesdir = VSCommon::getresourcesdir(bindir.first);
    if (resourcesdir.empty()) {
        resourcesdir = bindir.first;
        VSLAUNCH_LOG(logvs::WARN, "Warning: cannot find resources dir, using %s", resourcesdir.c_str());
    } else {
        VSLAUNCH_LOG(logvs::NOTICE, "Found resources directory in %s", resourcesdir.c_str());
    }

    // Init HOMEDIR
    if ((homedir = VSCommon::gethomedir(datadir)).first.empty()) {
        homedir.first = origpath;
        VSLAUNCH_LOG(logvs::WARN, "Warning: cannot find home, using %s", homedir.first.c_str());
    } else {
        VSLAUNCH_LOG(logvs::NOTICE, "Found home in %s", homedir.first.c_str());
    }
    VSCommon::vs_chdir(homedir.first.c_str());
    if (datadir.empty()) {
        datadir = homedir.first;
        VSLAUNCH_LOG(logvs::WARN, "Warning: cannot find data, using %s", datadir.c_str());
    }

    // Init CONFIGFILE
    InitConfig();

    // Init Log. TODO read config to see we where we want to log
    logvs::log_openfile("", VSCommon::getsuffixedfile(homedir.first + "/vslauncher.log", VSLAUNCH_MAXLOGS),
    		            /*redirect=*/true, /*append=*/false);
    atexit(logvs::log_terminate);

    // Check if Home dir files are up to date
    CheckHomeDir();

    // Check environment Variables
    CheckEnvironmentVariables();

    VSCommon::file_info_t myid;
    getFileInfo((bindir.first + "/" + bindir.second).c_str(), &myid);

    // Vegastrike binary search with config glengine, default gl engine and without engine suffix
    glengine = "." + glengine;
    const char * const binengines[] = { glengine.c_str(), "." VSLAUNCH_GLENGINE_DEF, "", NULL };
    for (const char * const * binengine = binengines; *binengine; ++binengine) {
        for (const char * const * bin = vslaunch_binsearchs; *bin; ++bin) {
            std::string vega_base((bindir.first + "/" + *bin) + "/" VSLAUNCH_BINARY);
            std::string vega = (vega_base + *binengine) + VSLAUNCH_EXE_EXT;
            if (notFoundOrSame(vega, &myid)) {
                continue ; // same as me or not found
            }
            vegastrikebin = vega;
            while (*(binengine+1) != NULL) ++binengine;
            VSLAUNCH_LOG(logvs::NOTICE, "Found vegastrike binary: %s", vegastrikebin.c_str());
            break ;
        }
    }
    if (vegastrikebin.empty()) {
    	vegastrikebin = VSLAUNCH_BINARY "." VSLAUNCH_GLENGINE_DEF VSLAUNCH_EXE_EXT;
        VSLAUNCH_LOG(logvs::WARN, "Warning: cannot find vegastrike binary, using %s", vegastrikebin.c_str());
    }

    // vssetup binary
    const char * const setupnames[] = { setupprog.c_str(), VSLAUNCH_SETUP_BINARY_DEF, VSLAUNCH_SETUP_BINARY_ALT, NULL };
    for (const char * const * setupname = setupnames; *setupname; ++setupname) { 
        if (**setupname == 0) continue ;
        for (const char * const * bin = vslaunch_setupbinsearchs; *bin; ++bin) {
            std::string vega(((((bindir.first + "/") + *bin) + "/") + *setupname) +  VSLAUNCH_EXE_EXT);
            if (notFoundOrSame(vega, &myid)) {
                continue ; // same as my on not found
            }
            vssetupbin = vega;
            while (*(setupname+1) != NULL) ++setupname;
            VSLAUNCH_LOG(logvs::NOTICE, "Found vssetup binary: %s", vssetupbin.c_str());
            break ;
        }
    }
    if (vssetupbin.empty()) {
    	vssetupbin = VSLAUNCH_SETUP_BINARY_DEF VSLAUNCH_EXE_EXT;
        VSLAUNCH_LOG(logvs::WARN, "Warning: cannot find vssetup binary, using %s", vssetupbin.c_str());
    }

    unsigned int whattorun = (flags & F_RUN_MASK);
    // Quick launch mode if the launcher has the name of vegastrike or vssetup
    if (whattorun == F_RUN_VEGASTRIKE || whattorun == F_RUN_DEF || (whattorun == 0 &&
          strncmp(bindir.second.c_str(), VSLAUNCH_BINARY, sizeof(VSLAUNCH_BINARY)-1) == 0)) {
        std::string program = (whattorun == F_RUN_VEGASTRIKE || !checkModifier()) ? vegastrikebin : vssetupbin;
        changeToData();
        if (VSCommon::vs_execl(VSCommon::VEF_EXEC, program.c_str(), program.c_str(), NULL) == -1) {
            VSLAUNCH_LOG(logvs::ERROR, "ERROR: cannot launch %s", program.c_str());
        } else return 0; // should not happen on macos/bsd/linux
    } else if (whattorun == F_RUN_VSSETUP || (whattorun == 0
                 && strncmp(bindir.second.c_str(), VSLAUNCH_SETUP_BINARY_DEF, sizeof(VSLAUNCH_SETUP_BINARY_DEF)-1) == 0)) {
        changeToData();
        if (VSCommon::vs_execl(VSCommon::VEF_EXEC, vssetupbin.c_str(), vssetupbin.c_str(), NULL) == -1) {
            VSLAUNCH_LOG(logvs::ERROR, "ERROR: cannot launch %s", vssetupbin.c_str());
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
# else // i can't get gcc <=11 understand objc std v2
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

