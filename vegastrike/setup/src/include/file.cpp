/***************************************************************************
 *                           file.cpp  -  description
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
#include <string>
#include <sys/stat.h>
#include "common/common.h"
#include "log.h"

using std::string;
#include "file.h"
bool origconfig=false;
void LoadMainConfig(void) {
	FILE *fp;
	char line[MAX_READ+1];
	char *p, *n_parm, *parm;
	int got_name = 0;
	int got_config = 0;
	int got_temp = 0;
	int got_column = 0;
	if ((fp = VSCommon::vs_fopen(CONFIG_FILE, "r")) == NULL) {
		string opath (datapath);
		opath+=string("/")+CONFIG_FILE;
		if ((fp = VSCommon::vs_fopen(opath.c_str(), "r")) == NULL) {
			VS_LOG("vssetup", logvs::WARN, "Unable to read from %s", CONFIG_FILE );
			exit(-1);
		}
	}
	while ((p = fgets(line, MAX_READ, fp)) != NULL) {
		if (line[0] == '#') { continue; }
		chomp(line);
		if (line[0] == '\0') { continue; }
		parm = line;
		n_parm = next_parm(parm);
		if (strcmp("program_name", parm) == 0) {
			if (CONFIG.program_name != NULL) { VS_LOG("vssetup", logvs::WARN, "Duplicate program_name in config file"); continue; }
			if (n_parm[0] == '\0') { VS_LOG("vssetup", logvs::WARN, "Missing parameter for program_name"); continue; }
			CONFIG.program_name = NewString(n_parm);
			got_name = 1;
			continue;
		}
		if (strcmp("config_file", parm) == 0) {
			if (CONFIG.config_file != NULL) { VS_LOG("vssetup", logvs::WARN, "Duplicate config_file in config file"); continue; }
			if (n_parm[0] == '\0') { VS_LOG("vssetup", logvs::WARN, "Missing parameter for config_file"); continue; }
			CONFIG.config_file = NewString(n_parm);
			got_config = 1;
			continue;
		}
		if (strcmp("temp_file", parm) == 0) {
			if (CONFIG.temp_file != NULL) { VS_LOG("vssetup", logvs::WARN, "Duplicate temp_file in config file"); continue; }
			if (n_parm[0] == '\0') { VS_LOG("vssetup", logvs::WARN, "Missing parameter for temp_file in config file"); continue; }
			CONFIG.temp_file = NewString(n_parm);
			got_temp = 1;
			continue;
		}
		if (strcmp("columns", parm) == 0) {
			if (CONFIG.columns > 0) { VS_LOG("vssetup", logvs::WARN, "Duplicate columns in config file"); continue; }
			if (n_parm[0] == '\0') { VS_LOG("vssetup", logvs::WARN, "Missing parameter for columns in config file"); continue; }
			CONFIG.columns = atoi(n_parm);
			if (CONFIG.columns == 0) { VS_LOG("vssetup", logvs::WARN, "Invalid parameter for column in config file"); continue; }
			got_column = 1;
			continue;
		}
		VS_LOG("vssetup", logvs::WARN, "Unknown line in config file: %s %s", parm, n_parm);
	}
	fclose(fp);
	if (got_name == 0) {
		VS_LOG("vssetup", logvs::WARN, "Unable to find name of program. Using default (Application)");
		CONFIG.program_name = NewString("Application");
	}
	if (got_config == 0) {
		VS_LOG("vssetup", logvs::WARN, "Fatal Error. Name of config file not found. What can I do without a config file to modify?");
		VS_LOG("vssetup", logvs::WARN, "To specify a config file, edit setup.config and add the following line (without the <>):");
		VS_LOG("vssetup", logvs::WARN, "config_file <Name of config file>");
		exit(-1);
	}
	if (got_temp == 0) {
		VS_LOG("vssetup", logvs::WARN, "Unable to find name of temporary file. Using default (config.temp)");
		CONFIG.temp_file = NewString("config.temp");
	}
	if (got_column == 0) {
		VS_LOG("vssetup", logvs::WARN, "Unable to find number of columns. Using default (3)");
		CONFIG.columns = 3;
	}
}
// Reads the config file to get the header information
// The program segfaults with incorrect header information
// I should add error checking for that
string mangle_config (string config) {
	return string(datapath)+string("/")+config;
}

bool useGameConfig(void) {
	VSCommon::file_info_t st1, st2;
	if (VSCommon::getFileInfo(CONFIG.config_file, &st1) && VSCommon::getFileInfo(mangle_config(CONFIG.config_file), &st2)) {
		if (st2.mtime > st1.mtime) {
			return true;
		}
	}
	return false;
}

void LoadConfig(void) {
	FILE *fp;
	char line[MAX_READ+1];
	char *p, *n_parm, *parm, *group;
	struct group *G_CURRENT, *G_NEXT;
	struct catagory *C_CURRENT, *C_NEXT;
	bool bUseGameConfig = useGameConfig();

	G_CURRENT = &GROUPS;
	C_CURRENT = &CATS;

	if (bUseGameConfig || (fp = VSCommon::vs_fopen(CONFIG.config_file, "r")) == NULL) {
        std::string orig_configfile = mangle_config(CONFIG.config_file);
        origconfig=true;
        if ((fp = VSCommon::vs_fopen(orig_configfile.c_str(), "r")) == NULL) {
			VS_LOG("vssetup", logvs::WARN, "Unable to read from %s", CONFIG_FILE );
			exit(-1);
		}
        VS_LOG("vssetup", logvs::WARN, "using game config %s", orig_configfile.c_str());
        if (bUseGameConfig) {
        	std::string backup_config = VSCommon::getsuffixedfile(CONFIG.config_file, (time_t)-1, 2);
        	if (backup_config != CONFIG.config_file) {
        		int res = VSCommon::fileCopyIfDifferent(CONFIG.config_file, backup_config, 1);
        		if (res < 0) {
        			VS_LOG("vssetup", logvs::WARN, "cannot save user config file in %s", backup_config.c_str());
        		} else if (res == VSCommon::FILECOMP_REPLACED) {
        			VS_LOG("vssetup", logvs::NOTICE, "saving user config file in %s", backup_config.c_str());
        		}
        	}
        }
	}
	while ((p = fgets(line, MAX_READ, fp)) != NULL) {
		parm = line;
		if (parm[0] == '<') { parm += 5; }      // Line might start with '<!-- '. Our code is inside these comments
		if (parm[0] != '#') { continue; }	// A line not starting with # can't be a config setting
		if (parm[1] == '#') { continue; }	// A line with a 2nd # is an ignored line
		chomp(parm);				// Get rid of the \n at the end of the line
		parm++;
		n_parm = next_parm(parm);		// next_parm is a line splitter included with general.c
		if (strcmp("groups", parm) == 0) {
			parm = n_parm;
			while ((n_parm = next_parm(parm))) {
				G_CURRENT->name = NewString(parm);
				G_NEXT = (struct group *)malloc(sizeof(struct group));
				if (G_NEXT == 0) { VS_LOG("vssetup", logvs::WARN, "Out of memory");exit(-1); }
				G_NEXT->name = 0;
				G_NEXT->setting = 0;
				G_CURRENT->next = G_NEXT;
				G_CURRENT = G_NEXT;
				G_CURRENT->next = 0;
				parm = n_parm;
			}
			continue;
		}
		if (strcmp("cat", parm) == 0) {
			parm = n_parm;
			n_parm = next_parm(parm);
			group = NewString(parm);
			parm = n_parm;
			while ((n_parm = next_parm(parm))) {
				C_CURRENT->name = NewString(parm);
				C_NEXT = (struct catagory *)malloc(sizeof(struct catagory));
				if (C_NEXT == 0) { VS_LOG("vssetup", logvs::WARN, "Out of memory");exit(-1); }
				C_CURRENT->next = C_NEXT;
				C_NEXT->name = 0;
				C_NEXT->group = 0;
				C_NEXT->info = 0;
				C_NEXT->button = 0;
				C_CURRENT->group = group;
				C_CURRENT = C_NEXT;
				C_CURRENT->next = 0;
				parm = n_parm;
			}
			continue;
		}
		if (strcmp("set", parm) == 0) {
			parm = n_parm;
			n_parm = next_parm(parm);
			SetGroup(parm, n_parm);
			continue;
		}
		if (strcmp("desc", parm) == 0) {
			parm = n_parm;
			n_parm = next_parm(parm);
			SetInfo(parm, n_parm);
			continue;
		}
		if (strcmp("endheader", parm) == 0) {
			fclose(fp);
			return;
		}
	}
}

void Modconfig(int setting, char *name, char *group) {
	FILE *rp, *wp;		//read and write
	char line[MAX_READ+1], write[MAX_READ+1], mid[MAX_READ+1];
	char *p, *parm, *n_parm, *start_write, *end_write;
	int commenting = 0;		// 0 if scanning, 1 if adding comments, 2 if removing comments
	int skip;

	if (useGameConfig() || (rp = VSCommon::vs_fopen(CONFIG.config_file, "r")) == NULL) {
		if ((rp = VSCommon::vs_fopen(mangle_config(CONFIG.config_file).c_str(), "r")) == NULL) {
			VS_LOG("vssetup", logvs::WARN, "Unable to read from %s", CONFIG_FILE );
			exit(-1);
		}
	}
	if ((wp = VSCommon::vs_fopen(CONFIG.temp_file, "w")) == NULL) {
		if ((wp = VSCommon::vs_fopen(mangle_config(CONFIG.temp_file).c_str(), "w")) == NULL) {
			VS_LOG("vssetup", logvs::WARN, "Unable to write to %s", CONFIG.temp_file );		exit(-1);
		}
	}
	while ((p = fgets(line, MAX_READ, rp)) != NULL) {
		chomp(line);
		strncpy(write, line, MAX_READ);
		skip = 0;
		start_write = line;
		parm = xml_pre_chomp_comment(start_write);   // Gets the start of the comment block

                // If there's no <!--, we might still be in a comment block, but xml_pre_chomp_comment() wouldn't know that
                if (parm[0] == '\0' && start_write[0] != '\0') { n_parm = parm; parm = start_write; start_write = n_parm; }

		end_write = xml_chomp_comment(parm);         // Gets the end of the comment block
                                                             // parm is everything inside <!-- -->, start_write and end_write
                                                             // is everything else (excluding <!-- -->

		strncpy(mid, parm, MAX_READ);                // Mid is used to keep the data inside the comments in memory
		mid[strlen(parm)] = '\0';

		n_parm = next_parm(parm);

		//if (parm[0] == '#' && parm[1] == '#') { fprintf(wp, "%s\n", write); continue; }   We no longer use double # for comments
		if (parm[0] != '#' || (parm[1] == '#' && parm[0] == '#')) { fprintf(wp, "%s\n", write); continue; }
		if (strcmp("#endheader", parm) == 0) { fprintf(wp, "%s\n", write); continue; }
		if (strcmp("#end", parm) == 0) {
			if (commenting == 1) { fprintf(wp, "#end -->\n"); }
			else if (commenting == 2) { fprintf(wp, "<!-- #end -->\n"); }
			else { fprintf(wp, "%s\n", write); }
			commenting = 0;
			skip = 1;
			//fprintf(wp, "%s\n", write);
			continue;
		}
		if (strcmp("#groups", parm) == 0) { skip = 1; }
		if (strcmp("#cat", parm) == 0) { skip = 1; }
		if (strcmp("#set", parm) == 0) {
			parm = n_parm;
			n_parm = next_parm(parm);
			if (parm && group && strcmp(parm, group) == 0) {
				if (setting == 1) { fprintf(wp, "#set %s none\n", group); }
				if (setting == 2) { fprintf(wp, "#set %s %s\n", group, name); }
			}
			else { fprintf(wp, "%s\n", write); }
			continue;
		}
		if (strcmp("#desc", parm) == 0) { skip = 1; }
		if (skip == 1) { fprintf(wp, "%s\n", write); continue; }
// Comments are now <!-- --> and are controlled at the start and end of the block. No longer need to comment each line
/*		if (commenting == 2) {
			parm = write;
			if (parm[0] == '#') { parm++; }
			fprintf(wp, "%s\n", parm);
			continue;
		}
		if (commenting == 1) {
			fprintf(wp, "#%s\n", write);
			continue;
		}
*/
		if (parm[0] != '#') { fprintf(wp, "%s\n", write); continue; }
		parm++;
		if (parm && name && strcmp(name, parm) == 0) { commenting = setting; }
		else {
			parm = n_parm;
			while ((n_parm = next_parm(parm))) {
				if (parm[0] == '<') { break; }
				if (parm && name && strcmp(name, parm) == 0) {
					commenting = setting;
					break;
				}
				parm = n_parm;
			}
		}
		if (commenting == 0) { fprintf(wp, "%s\n", write); continue; }
		fprintf(wp, "%s", start_write);
		if (commenting == 1) { fprintf(wp, "<!-- %s", mid); }
		else if (commenting == 2) { fprintf(wp, "<!-- %s -->", mid); }
		else { fprintf(wp, "%s", mid); }
		fprintf(wp, "%s\n", end_write);
		//fprintf(wp, "%s\n", write);
	}


	fclose(wp);
	fclose(rp);
	// Now we commit the changes
	if ((rp = VSCommon::vs_fopen(CONFIG.temp_file, "r")) == NULL) {
		if ((rp = VSCommon::vs_fopen(mangle_config(CONFIG.temp_file).c_str(), "r")) == NULL) {
			VS_LOG("vssetup", logvs::WARN, "Unable to read from %s", CONFIG.temp_file );

			exit(-1);
		}
	}
	string tmp1 = CONFIG.config_file;
/*
	if(origconfig) {
		tmp1 = mangle_config (CONFIG.config_file);
	}
*/
	if ((wp = VSCommon::vs_fopen(tmp1.c_str(), "w")) == NULL) {
		tmp1 = mangle_config (CONFIG.config_file);
		if ((wp = VSCommon::vs_fopen(tmp1.c_str(), "w")) == NULL) {
			tmp1 = CONFIG.config_file;
			if ((wp = VSCommon::vs_fopen(tmp1.c_str(), "w")) == NULL) {
				VS_LOG("vssetup", logvs::WARN, "Unable to write  to %s", CONFIG.config_file );
				exit(1);
			}
		}
	}
	while ((p = fgets(line, MAX_READ, rp)) != NULL) {
		fputs(line, wp);
	}
	fclose(rp);
	fclose(wp);
}

void EnableSetting(char *name, char *group) {
	Modconfig(2, name, group);
}
void DisableSetting(char *name, char *group) {
	Modconfig(1, name, group);
}
