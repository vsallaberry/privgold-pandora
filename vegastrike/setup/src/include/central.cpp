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
//#include <process.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#else
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
        "documentation\\readme.txt",
#endif
        "documentation/readme.txt",
        "readme.txt",
        NULL
    };
    const char * curpath = ".";
    for (unsigned int i=0; i < 2; ++i) {
        for (const char ** search = searchs; *search; ++search) {
            FILE * fp;
            int err;
            printf("readme: trying %s/%s\n", curpath, *search);
            if ((fp = fopen(*search, "r")) != NULL) {
                fclose(fp);
#if defined (_WIN32)
                err = (int)ShellExecute(NULL,"open",*search,"","",1);
#elif defined (__APPLE__)
                err = execlp("open", "open",*search, NULL); //Will this work on MacOS?
#else
                err = execlp("less", "less",*search, NULL); //Will this work in Linux?
#endif
                if (err == 0) {
                    i = (unsigned int)-1;
                    break ;
                }
            }
        }
        if (i == 0) {
            if (chdir(origpath) != 0) {
                break ;
            }
            curpath = origpath;
        }
    }
}
