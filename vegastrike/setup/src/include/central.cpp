/***************************************************************************
 *                           central.cpp  -  description
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

#include "central.h"

#ifdef _WIN32
#define PATH_SEP "\\"
//#include <process.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#if defined(__CYGWIN__) || defined(__MINGW32__)
# include <io.h>
#endif
#else
#define PATH_SEP "/"
#include <unistd.h>
#endif

#include <stdio.h>

struct catagory CATS;
struct group GROUPS;
struct global_settings CONFIG;

static char empty_string[1] = { 0 };

// Primary initialization function. Sets everything up and takes care of the program
void Start(int * argc, char ***argv) {
	LoadMainConfig();
	InitGraphics(argc,argv);
	LoadConfig();
	ShowMain();
}

void SetGroup(char *group, char *setting) {
	struct group *CURRENT;
	CURRENT = &GROUPS;
	do {
		if (CURRENT->name == NULL) { continue; }
		if (strcmp(group, CURRENT->name) == 0) { CURRENT->setting = NewString(setting); return; }
	} while ((CURRENT = CURRENT->next));
}
void SetInfo(char *catagory, char *info) {
	struct catagory *CURRENT;
	CURRENT = &CATS;
	do {
		if (CURRENT->name == NULL) { continue; }
		if (strcmp(catagory, CURRENT->name) == 0) { CURRENT->info = NewString(info); return; }
	} while ((CURRENT = CURRENT->next));
}

char *GetInfo(char *catagory) {
	struct catagory *CURRENT;
	CURRENT = &CATS;
	do {
		if (CURRENT->name == NULL) { continue; }
		if (strcmp(catagory, CURRENT->name) == 0) {
			if (CURRENT->info) { return CURRENT->info; }
			else { return catagory; }
		}
	} while ((CURRENT = CURRENT->next));
	return catagory;
}

char *GetSetting(char *group) {
    struct group *CUR;
	CUR = &GROUPS;
	do {
		if (CUR->name == NULL) { continue; }
		if (strcmp(CUR->name, group) == 0) { return CUR->setting; }
	} while ((CUR = CUR->next));
	return empty_string;
}

struct catagory *GetCatStruct(char *name) {
	struct catagory *CUR;
	CUR = &CATS;
	do {
		if (CUR->name == NULL) { continue; }
		if (strcmp(CUR->name, name) == 0) { return CUR; }
	} while ((CUR = CUR->next));
	return 0;
}

struct group *GetGroupStruct(char *name) {
        struct group *CUR;
        CUR = &GROUPS;
        do {
                if (CUR->name == NULL) { continue; }
                if (strcmp(CUR->name, name) == 0) { return CUR; }
        } while ((CUR = CUR->next));
        return 0;
}

void ShowReadme() {
    static const char * searchs[] = {
#if defined(_WIN32) || defined(__APPLE__)
        "Manual.pdf",
        "documentation" PATH_SEP "readme.txt",
#endif
        "documentation" PATH_SEP "readme.txt",
        "readme.txt",
        NULL
    };
    char curpath[65525] = { 0, };
    getcwd(curpath, sizeof(curpath));
#ifndef _WIN32
    pid_t pid = fork();
    if (pid > 0) return;
#endif
    for (unsigned int i=0; i < 2; ++i) {
        for (const char ** search = searchs; *search; ++search) {
            FILE * fp;
            ssize_t err;
            char arg[65535];
            snprintf(arg, sizeof(arg), "%s%s%s", curpath, PATH_SEP, *search);
            printf("readme: trying %s\n", arg);
            if ((fp = fopen(arg, "r")) != NULL) {
                fclose(fp);
#if defined (_WIN32)
                err = (ssize_t)ShellExecute(NULL,"open",arg,"","",1);
#elif defined (__APPLE__)
                char * script = (char*)malloc(sizeof(arg));
                snprintf(script, sizeof(arg), "tell application \"Preview.app\" to open \"%s\"", arg);
                err = execlp("osascript", "osascript", "-e", script, NULL); //Will this work on MacOS?
                //err = execlp("open", "open", arg, NULL);
#else
                err = execlp("less", "less",arg, NULL); //Will this work in Linux?
#endif
                // don't check err, we have checked previously file existence (arg).
                return ;
            }
        }
        if (i == 0) {
            if (chdir(origpath) != 0) {
                break ;
            }
            strcpy(curpath, origpath);
        }
    }
}
