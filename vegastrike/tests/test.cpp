/*
 * Vega Strike tests
 * Copyright (C) 2021 Vincent Sallaberry
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "unicode.h"
#include "multimap.h"

#include "vs_log_modules.cpp"

static int usage(int status, int argc, char ** argv) {
	(void) argc;
	FILE * out = status == 0 ? stdout : stderr;
	fprintf(out, "Usage: %s [-h,--help]"
			    #ifdef VS_LOG_NO_XML
				 " [-L[<module>=]<level>"
			    #endif
			     "\n", *argv);
	return status;
}

int main(int argc, char ** argv) {
	unsigned int loglevel = logvs::NOTICE;
	unsigned int nerrs = 0;

    for (int i = 1; i < argc; ++i) {
    	bool err = false;
    	if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
    		return usage (0, argc, argv);
    	}
	   #ifdef VS_LOG_NO_XML
    	else if (strncmp(argv[i], "-L", 2) == 0) {
    		if (argv[i][2] == 0) {
    			err = true;
    		} else {
    			const char * strlevel = strchr(argv[i] + 2, '=');
    			std::string module;
    			if (strlevel != NULL) {
    				module.append(argv[i], 2, strlevel-argv[i] - 2);
    				++strlevel;
    			} else {
    				strlevel = argv[i] + 2;
    			}
    			unsigned int loglevel = strtol(strlevel, NULL, 10);
    			logvs::vs_log_setlevel(module, loglevel);
    		}
    	}
       #endif
    	if (err) {
    		fprintf(stderr, "error: wrong argument '%s'\n", argv[i]);
    		return usage(-1, argc, argv);
    	}
    }

    nerrs += utf8_iterator_tests::utf8_iterator_test();

    nerrs += ChainedMultimapTests::test_multimap();

    return nerrs;
}

