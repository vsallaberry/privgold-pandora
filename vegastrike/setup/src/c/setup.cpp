/***************************************************************************
 *                           setup.cpp  -  description
 *                           ----------------------------
 *                           begin                : January 18, 2002
 *                           copyright            : (C) 2002 by David Ranger
 *                           email                : sabarok@start.com.au
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 **************************************************************************/

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif
#if defined(HAVE_VERSION_H)
# include "version.h"
#else
# define SCM_VERSION "unknown"
# define SCM_REVISION "unknown"
# define SCM_REMOTE "unknown"
#endif
#include "common/common.h"
#include "../include/central.h"
#include <stdlib.h>
#ifdef _WIN32
# include <windows.h>
# if defined(__CYGIN__) || defined(__MINGW32__)
#  include <io.h>
# else
#  include <direct.h>
# endif
#include <sys/stat.h>
#else
# include <sys/dir.h>
# include <stdio.h>
# include <unistd.h>
# include <pwd.h>
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include <vector>
#include <string>

#include "log.h"
#include "unicode.h"

using std::string;
using std::vector;
const char * binpath;
const char * origpath;
const char * datapath;
const char * resourcespath;
const char * homepath;


static void changeToProgramDirectory(char *argv0) {
    int ret = -1; /* Should it use argv[0] directly? */
    char *program = argv0;
#if defined(linux) || defined(__linux__)
    char buf[65536];
    {
	char linkname[128]; /* /proc/<pid>/exe */
	linkname[0]='\0';
	pid_t pid;
	
	/* Get our PID and build the name of the link in /proc */
	pid = getpid();
	
	sprintf(linkname, "/proc/%d/exe", pid);
	ret = readlink(linkname, buf, 65535);
	if (ret <= 0) {
		sprintf(linkname, "/proc/%d/file", pid);
		ret = readlink(linkname, buf, 65535);
	}
	if (ret <= 0) {
		ret = readlink(program, buf, 65535);
	}
	if (ret > 0) {
		buf[ret]='\0';
		/* Ensure proper NUL termination */
		program = buf;
	}
    }
#endif

    char *parentdir;
    int pathlen=strlen(program);
    parentdir=new char[pathlen+1];
    char *c;
    strncpy ( parentdir, program, pathlen+1 );
    c = (char*) parentdir;
    while (*c != '\0')     /* go to end */
      c++;
    
    while ((*c != '/')&&(*c != '\\')&&c>parentdir)      /* back up to parent */
      c--;
    
    *c = '\0';             /* cut off last part (binary name) */

    if (strlen (parentdir)>0) {
      VSCommon::vs_chdir (parentdir);/* chdir to the binary app's parent */
    }
    delete []parentdir;
}


#if defined(_WINDOWS)&&defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
    char ** argv;
    int argc;
    unicodeInitLocale();
    VSCommon::ParseCmdLine(GetCommandLineW(), &argc, &argv);
#else
int main(int argc, char *argv[]) {
#endif
	char tmppwd[65535] = { 0, }; tmppwd[sizeof(tmppwd)-1] = 0;
	const char * argdatadir = "";
#if defined(CONSOLE)
    bool forceconsole = true, redirectlog = false;
#else
    bool forceconsole = false, redirectlog = true;
#endif
    bool has_console = VSCommon::InitConsole(forceconsole, argc, argv);
    logvs::log_setfile(stdout);
    logvs::log_setflag(logvs::F_QUEUELOGS, true);

    unicodeInitLocale();
    VSCommon::vs_getcwd (tmppwd,sizeof(tmppwd)-1);
    origpath = strdup(tmppwd);
    VS_LOG("vssetup", logvs::NOTICE, " In path %s", origpath);

    for (int i_argv = 1; i_argv < argc; ++i_argv) {
		if ((i_argv + 1 < argc && strcmp(argv[i_argv], "--target") == 0) || strncmp(argv[i_argv], "-D", 2) == 0) {
            char * argpath = argv[i_argv][1] == '-' ? argv[++i_argv] : argv[i_argv]+2;
			if (VSCommon::vs_chdir(argpath) == 0) {
            	VSCommon::vs_chdir(origpath);
            	argdatadir = argpath;
            }
		} else if (has_console) {
			if (!strcmp(argv[i_argv], "--version")) {
				logvs::log_printf("vssetup for vegastrike %s revision %s from %s\n",
								  SCM_VERSION, SCM_REVISION, SCM_REMOTE);
				return 0;
			}
			VS_LOG("vssetup", logvs::WARN, "Usage: vssetup [--target DATADIR] [-DDATADIR] [--version]");
            return 1;
		}
	}

    VS_LOG("vssetup", logvs::NOTICE, "vssetup for vegastrike %s revision %s from %s",
    								 SCM_VERSION, SCM_REVISION, SCM_REMOTE);

	changeToProgramDirectory(VSCommon::fix_argv0(&argc, &argv));
    VSCommon::vs_getcwd (tmppwd,sizeof(tmppwd)-1);
	binpath = strdup(tmppwd);
    VS_LOG("vssetup", logvs::NOTICE, "Binary path %s", binpath);

	{
        const char * pathorder[] = { argdatadir, binpath, ".", NULL };
		vector<string>	data_paths;

		/* look for resources directory, where we could find data,share,bin,lib */
		std::string resources_str = VSCommon::getresourcesdir(binpath);
		if (!resources_str.empty()) {
			VS_LOG("vssetup", logvs::NOTICE, "Found resources in %s", resources_str.c_str());
		} else {
			resources_str = binpath;
		}
	    resourcespath = strdup(resources_str.c_str());

        /* Now look for data directory */
        for (const char ** base = pathorder; *base; ++base) {
        	if (**base == 0) continue ;
            for (const char * const * searchs = VSCommon::datadirs; *searchs; ++searchs) {
                data_paths.push_back( (std::string(*base) + "/") + *searchs );
            }
         }
#ifdef DATA_DIR
		data_paths.push_back( DATA_DIR);
#endif

		for( vector<string>::iterator vsit=data_paths.begin(); vsit!=data_paths.end(); vsit++)
		{
			// Test if the dir exist and contains config_file
			VSCommon::vs_chdir(origpath);
			VSCommon::vs_chdir((*vsit).c_str());
			FILE *setupcfg = VSCommon::vs_fopen("setup.config","r");
			if (!setupcfg)
				continue;
			fclose(setupcfg);
			setupcfg = VSCommon::vs_fopen("Version.txt","r");
			if (!setupcfg)
				continue;
			VSCommon::vs_getcwd (tmppwd,sizeof(tmppwd)-1);
			VS_LOG("vssetup", logvs::NOTICE, "Found data in %s", tmppwd);
			break;
		}
		VSCommon::vs_getcwd (tmppwd,sizeof(tmppwd)-1);
		datapath = strdup(tmppwd);
	}
	string HOMESUBDIR;
	FILE *version=VSCommon::vs_fopen("Version.txt","r");
	if (!version)
		version=VSCommon::vs_fopen("../Version.txt","r");
	if (version) {
		std::string hsd="";
		int c;
		while ((c=fgetc(version))!=EOF) {
			if (isspace(c))
				break;
			hsd+=(char)c;
		}
		fclose(version);
		if (hsd.length()) {
			HOMESUBDIR=hsd;
			//VS_LOG("vssetup", logvs::NOTICE, "Using %s as the home directory\n",hsd.c_str());
		}
	}
	if (HOMESUBDIR.empty()) {
        HOMESUBDIR = ".vegastrike";
		VS_LOG("vssetup", logvs::WARN, "Warning: Failed to find Version.txt anywhere, using %s as home.", HOMESUBDIR.c_str());
	}
#if !defined(VS_HOME_INSIDE_DATA)
# if !defined(_WIN32)
	struct passwd *pwent;
	pwent = getpwuid (getuid());
	VSCommon::vs_chdir (pwent->pw_dir);
# else
	WCHAR wappdata_path[PATH_MAX];
	if (VSCommon::win32_get_appdata(wappdata_path) != S_OK) {
		// use datadir as homedir
	} else {
		if (HOMESUBDIR.size() && HOMESUBDIR[0] == '.') {
			HOMESUBDIR=HOMESUBDIR.substr(1);
		}
		_wchdir(wappdata_path);
	}
# endif
#endif

	VSCommon::vs_mkdir(HOMESUBDIR.c_str(), 0755);
	VSCommon::vs_chdir (HOMESUBDIR.c_str());
    
    VSCommon::vs_getcwd(tmppwd,sizeof(tmppwd)-1);
    homepath = strdup(tmppwd);
    VS_LOG("vssetup", logvs::NOTICE, "Now in Home Dir: %s", homepath);

    logvs::log_openfile("", VSCommon::getsuffixedfile(std::string(homepath) + "/vssetup.log", 3),
                        /*redirect=*/redirectlog, /*append=*/false); // redirections disturbs NCurses
    atexit(logvs::log_terminate);

	Start(&argc,&argv);
#if defined(_WINDOWS)&&defined(_WIN32)
	VSCommon::ParseCmdLineFree(argv);
#endif
    VS_LOG("vssetup", logvs::NOTICE, "exiting...");
    free((void*)origpath);
    free((void*)datapath);
    free((void*)binpath);
    free((void*)resourcespath);
    free((void*)homepath);
	return 0;
}

