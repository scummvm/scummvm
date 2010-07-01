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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/platform/gp2x/gp2x-common.h"
#include "backends/platform/gp2x/gp2x-hw.h"
#include "backends/platform/gp2x/gp2x-mem.h"

#include "backends/graphics/gp2xsdl/gp2xsdl-graphics.h"
#include "backends/events/gp2xsdl/gp2xsdl-events.h"
#include "backends/saves/default/default-saves.h"

#include "common/config-manager.h"
#include "common/debug.h"

// Disable for normal serial logging.
#define DUMP_STDOUT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

void OSystem_GP2X::initBackend() {
	// Setup default save path to be workingdir/saves
	char savePath[PATH_MAX + 1];
	char workDirName[PATH_MAX + 1];

	if (getcwd(workDirName, PATH_MAX) == NULL) {
		error("Could not obtain current working directory.");
	} else {
		printf("Current working directory: %s\n", workDirName);
	}

	strcpy(savePath, workDirName);
	strcat(savePath, "/saves");
	printf("Current save directory: %s\n", savePath);
	struct stat sb;
	if (stat(savePath, &sb) == -1)
		if (errno == ENOENT) // Create the dir if it does not exist
			if (mkdir(savePath, 0755) != 0)
				warning("mkdir for '%s' failed", savePath);

	ConfMan.registerDefault("savepath", savePath);

	#ifdef DUMP_STDOUT
		// The GP2X has a serial console but most users do not use this so we
		// output all our STDOUT and STDERR to files for debug purposes.
		char STDOUT_FILE[PATH_MAX + 1];
		char STDERR_FILE[PATH_MAX + 1];

		strcpy(STDOUT_FILE, workDirName);
		strcpy(STDERR_FILE, workDirName);
		strcat(STDOUT_FILE, "/scummvm.stdout.txt");
		strcat(STDERR_FILE, "/scummvm.stderr.txt");

		/* Flush the output in case anything is queued */
		fclose(stdout);
		fclose(stderr);

		/* Redirect standard input and standard output */
		FILE *newfp = freopen(STDOUT_FILE, "w", stdout);
		if (newfp == NULL) {
		#if !defined(stdout)
			stdout = fopen(STDOUT_FILE, "w");
		#else
			newfp = fopen(STDOUT_FILE, "w");
			if (newfp) {
				*stdout = *newfp;
			}
		#endif
		}

		newfp = freopen(STDERR_FILE, "w", stderr);
		if (newfp == NULL) {
		#if !defined(stderr)
			stderr = fopen(STDERR_FILE, "w");
		#else
			newfp = fopen(STDERR_FILE, "w");
			if (newfp) {
				*stderr = *newfp;
			}
		#endif
		}

		setbuf(stderr, NULL);
		printf("%s\n", "Debug: STDOUT and STDERR redirected to text files.");
	#endif /* DUMP_STDOUT */

	// Setup other defaults.
	ConfMan.registerDefault("aspect_ratio", true);

	/* Up default volume values as we use a seperate system level volume anyway. */
	ConfMan.registerDefault("music_volume", 192);
	ConfMan.registerDefault("sfx_volume", 192);
	ConfMan.registerDefault("speech_volume", 192);
	ConfMan.registerDefault("autosave_period", 3 * 60);	// Trigger autosave every 3 minutes - On low batts 4 mins is about your warning time.

	ConfMan.setBool("FM_low_quality", true);

	/* Initialise any GP2X specific stuff we may want (Batt Status, scaler etc.) */
	GP2X_HW::deviceInit();

	/* Set Default hardware mixer volume to a preset level (VOLUME_INITIAL). This is done to 'reset' volume level if set by other apps. */
	GP2X_HW::mixerMoveVolume(0);

	// Create the events manager
	if (_eventManager == 0)
		_eventManager = new GP2XSdlEventManager(this);

	// Create the graphics manager
	if (_graphicsManager == 0)
		_graphicsManager = new GP2XSdlGraphicsManager();

	// Call parent implementation of this method
	OSystem_POSIX::initBackend();
}

void OSystem_GP2X::initSDL() {
	// Check if SDL has not been initialized
	if (!_initedSDL) {
		uint32 sdlFlags = SDL_INIT_EVENTTHREAD;
		if (ConfMan.hasKey("disable_sdl_parachute"))
			sdlFlags |= SDL_INIT_NOPARACHUTE;

		// Initialize SDL (SDL Subsystems are initiliazed in the corresponding sdl managers)
		if (SDL_Init(sdlFlags) == -1)
			error("Could not initialize SDL: %s", SDL_GetError());

		// Enable unicode support if possible
		SDL_EnableUNICODE(1);

		_initedSDL = true;
	}
}

void OSystem_GP2X::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	/* Setup default extra data paths for engine data files and plugins */
	char workDirName[PATH_MAX + 1];

	if (getcwd(workDirName, PATH_MAX) == NULL) {
		error("Error: Could not obtain current working directory.");
	}

	Common::FSNode workdirNode(workDirName);
	if (workdirNode.exists() && workdirNode.isDirectory()) {
		s.add("__GP2X_WORKDIR__", new Common::FSDirectory(workDirName), priority);
	}

	char enginedataPath[PATH_MAX+1];

	strcpy(enginedataPath, workDirName);
	strcat(enginedataPath, "/engine-data");

	Common::FSNode engineNode(enginedataPath);
	if (engineNode.exists() && engineNode.isDirectory()) {
		s.add("__GP2X_ENGDATA__", new Common::FSDirectory(enginedataPath), priority);
	}

	char pluginsPath[PATH_MAX+1];

	strcpy(pluginsPath, workDirName);
	strcat(pluginsPath, "/plugins");

	Common::FSNode pluginsNode(pluginsPath);
	if (pluginsNode.exists() && pluginsNode.isDirectory()) {
		s.add("__GP2X_PLUGINS__", new Common::FSDirectory(pluginsPath), priority);
	}
}

void OSystem_GP2X::quit() {
	GP2X_HW::deviceDeinit();

	#ifdef DUMP_STDOUT
		printf("%s\n", "Debug: STDOUT and STDERR text files closed.");
		fclose(stdout);
		fclose(stderr);
	#endif /* DUMP_STDOUT */

	OSystem_POSIX::quit();
}
