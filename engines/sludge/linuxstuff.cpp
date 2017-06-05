/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#if defined __unix__ && !(defined __APPLE__)
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <getopt.h>

#include "linuxstuff.h"
#include "platform-dependent.h"
#include "allfiles.h"
#include "language.h" // for settings
#include "debug.h"
#include "helpers.h"

namespace Sludge {

extern settingsStruct gameSettings;
cmdlineSettingsStruct cmdlineSettings;

extern char **languageName;

/*
 * Functions declared in linuxstuff.h:
 */

void printCmdlineUsage() {
	fprintf(stdout, "OpenSLUDGE engine, usage: sludge-engine [<options>] <gamefile name>\n\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "-f,		--fullscreen		Set display mode to fullscreen\n");
	fprintf(stdout, "-w,		--window		Set display mode to windowed\n");
	fprintf(stdout, "-L,		--list-languages	Print available languages and their indices\n");
	fprintf(stdout, "-l<index>,	--language=<index>	Set language to <index> (look up with -L)\n");
	fprintf(stdout, "-a<number>,	--antialias=<number>	Turn antialiasing on (1) or off (0)\n");
	fprintf(stdout, "					or choose linear interpolation (-1)\n");
	fprintf(stdout, "-d<number>,	--debug=<number>	Turn debug mode on (1) or off (0)\n");
	fprintf(stdout, "-h,		--help			Print this help message\n\n");
	fprintf(stdout, "Options are saved, so you don't need to specify them every time.\n");
	fprintf(stdout, "If you entered a wrong language number, use -l0 to reset the language to the default setting.\n");
	fprintf(stdout, "You can always toggle between fullscreen and windowed mode with \"Alt+Enter\"\n");
	fprintf(stdout, "or antialiasing on and off with \"Alt+A\".\n");
}

void printLanguageTable() {
	if (gameSettings.numLanguages) {
		fprintf(stdout, "Index		Language\n");
		for (unsigned int i = 0; i <= gameSettings.numLanguages; i++) {
			if (languageName[i]) {
				fprintf(stdout, "%d		%s\n", i, languageName[i]);
			} else {
				fprintf(stdout, "%d		Language %d\n", i, i);
			}
		}
	} else {
		fprintf(stdout, "No translations available.\n");
	}
}

bool parseCmdlineParameters(int argc, char *argv[]) {
	int retval = true;
	cmdlineSettings.fullscreenSet = false;
	cmdlineSettings.languageSet = false;
	cmdlineSettings.aaSet = false;
	cmdlineSettings.debugModeSet = false;
	cmdlineSettings.listLanguages = false;
	while (1) {
		static struct option long_options[] = {
			{"fullscreen",      no_argument,       0, 'f' },
			{"window",      no_argument,       0, 'w' },
			{"list-languages",  no_argument,       0, 'L' },
			{"language",        required_argument, 0, 'l' },
			{"antialias",       required_argument, 0, 'a' },
			{"debug",       required_argument, 0, 'd' },
			{"help",        no_argument,       0, 'h' },
			{0, 0, 0, 0} /* This is a filler for -1 */
		};
		int option_index = 0;
		int c = getopt_long(argc, argv, "fwLl:a:d:h", long_options, &option_index);
		if (c == -1) break;
		switch (c) {
		case 'f':
			cmdlineSettings.fullscreenSet = true;
			cmdlineSettings.userFullScreen = true;
			break;
		case 'w':
			cmdlineSettings.fullscreenSet = true;
			cmdlineSettings.userFullScreen = false;
			break;
		case 'L':
			cmdlineSettings.listLanguages = true;
			break;
		case 'l':
			cmdlineSettings.languageSet = true;
			cmdlineSettings.languageID = atoi(optarg);
			break;
		case 'a':
			cmdlineSettings.aaSet = true;
			cmdlineSettings.antiAlias = atoi(optarg);
			break;
		case 'd':
			cmdlineSettings.debugModeSet = true;
			cmdlineSettings.debugMode = atoi(optarg);
			break;
		case 'h':
		default:
			retval = false;
			break;
		}
	}
	return retval;
}

/*
 * Functions declared in platform-dependent.h:
 */

char *grabFileName() {
	return NULL;
}

int showSetupWindow() {
	if (cmdlineSettings.listLanguages) {
		printLanguageTable();
		return 0;
	}
	if (cmdlineSettings.languageSet) {
		if (cmdlineSettings.languageID <= gameSettings.numLanguages) {
			gameSettings.languageID = cmdlineSettings.languageID;
		} else {
			fprintf(stdout, "Language index %d doesn't exist. Please specify an index between 0 and %d.\n\n",
			        cmdlineSettings.languageID, gameSettings.numLanguages);
			printLanguageTable();
			return 0;
		}
	}
	if (cmdlineSettings.fullscreenSet) {
		gameSettings.userFullScreen = cmdlineSettings.userFullScreen;
	}
	if (cmdlineSettings.aaSet) {
		gameSettings.antiAlias = cmdlineSettings.antiAlias;
	}
	if (cmdlineSettings.debugModeSet) {
		gameSettings.debugMode = cmdlineSettings.debugMode;
	}
	return 1;
}

void msgBox(const char *head, const char *msg) {
	fprintf(stderr, "%s\n%s\n", head, msg);
}

int msgBoxQuestion(const char *head, const char *msg) {
	return 1;
}

void changeToUserDir() {
	if (chdir(getenv("HOME"))) {
		debugOut("Error: Failed changing to directory %s\n", getenv("HOME"));
	}
	mkdir(".sludge-engine", 0000777);
	if (chdir(".sludge-engine")) {
		debugOut("Error: Failed changing to directory %s\n", ".sludge-engine");
	}
}

uint32 launch(char *filename) {
	debugOut("Trying to launch: %s\n", filename);

	if (!fileExists("/usr/bin/xdg-open")) {
		debugOut("Launching failed due to missing /usr/bin/xdg-open.\n");
		return 0;
	}

	if (!(filename[0] == 'h' &&
	        filename[1] == 't' &&
	        filename[2] == 't' &&
	        filename[3] == 'p' &&
	        (filename[4] == ':' || (filename[4] == 's' && filename[5] == ':'))) &&
	        !fileExists(filename)) {
		return 0;
	}

	int status;

	pid_t pid = fork();
	if (pid < 0) {
		return 0;
	} else if (pid == 0) {
		execl("/usr/bin/xdg-open", "xdg-open", filename, (char *)0);
		exit(EXIT_FAILURE);
	} else {
		waitpid(pid, &status, 0);
	}

	if (status == EXIT_SUCCESS) {
		return 69;
	} else {
		return 0;
	}
}

bool defaultUserFullScreen() {
	return false;
}

} // End of namespace Sludge

#endif
