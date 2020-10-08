#include "args.h"

#include <string.h>

#include "log.hpp"

char* ArgTexpagesPath = NULL;

void ProcessArgs(int argc, char** argv) {
	for(int i=1; i<argc; i++) {
		if(equalstr(argv[i], "--version")) {
			printf("Warzone 2100 Map Editor development version. No semver yet.\n")
			exit(0);
		} else if(equalstr(argv[i], "--quiet") || equalstr(argv[i], "-q")) {
			log_set_quiet(1);
		} else if(equalstr(argv[i], "--loglevel") || equalstr(argv[i], "-log")) {
			if(i+1 < argc) {
				log_set_level(atoi(argv[i]));
				i++;
			} else {
				log_fatal("Log level expects argument.");
			}
		} else if(equalstr(argv[i], "-t")) {
			if(i+1 < argc) {
				if(ArgTexpagesPath != NULL) {
					free(ArgTexpagesPath);
				}
				ArgTexpagesPath = (char*)malloc(strlen(argv[i+1])+1);
				strcpy(ArgTexpagesPath, argv[i+1]);
				i++;
			} else {
				log_fatal("-t expects argument.");
			}
		} else if(equalstr(argv[i], "--help") || equalstr(argv[i], "-h")) {
			printf("   Warzone 2100 Map Editor\n");
			printf("   \n");
			printf("   Usage: %s [args]\n", argv[0]);
			printf("   \n");
			printf("   == general ==\n");
			printf("   --version            Display version info and exit.\n");
			printf("   -h   (--help)        Display this text and exit.\n");
			printf("   -q   (--quiet)       Do not log to stdout.\n");
			printf("   -log (--loglevel)    Set logging level.\n");
			printf("   -t <path>            Set Path to texpages directory contents.\n");
			printf("   \n");
			exit(0);
		}
	}
}
