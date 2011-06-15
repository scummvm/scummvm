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
 */

#if defined(OPENPANDORA)

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/sdl/sdl-sys.h"

#include "backends/mixer/doublebuffersdl/doublebuffersdl-mixer.h"
#include "backends/platform/openpandora/op-sdl.h"
#include "backends/plugins/posix/posix-provider.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/mixer_intern.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>	// for getTimeAndDate()

/* Dump console info to files. */
#define DUMP_STDOUT

static SDL_Cursor *hiddenCursor;

OSystem_OP::OSystem_OP()
	:
	OSystem_POSIX() {
}

//static Uint32 timer_handler(Uint32 interval, void *param) {
//	((DefaultTimerManager *)param)->handler();
//	return interval;
//}

void OSystem_OP::initBackend() {

	assert(!_inited);

	// Create the events manager
	if (_eventSource == 0)
		_eventSource = new OPEventSource();

	// Create the graphics manager
	if (_graphicsManager == 0) {
		_graphicsManager = new OPGraphicsManager(_eventSource);
	}

//	int joystick_num = ConfMan.getInt("joystick_num");
//	uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
//
//	if (ConfMan.hasKey("disable_sdl_parachute"))
//		sdlFlags |= SDL_INIT_NOPARACHUTE;
//
//	if (joystick_num > -1)
//		sdlFlags |= SDL_INIT_JOYSTICK;
//
//	if (SDL_Init(sdlFlags) == -1) {
//		error("Could not initialize SDL: %s", SDL_GetError());
//	}
//

	// Create the mixer manager
//	if (_mixer == 0) {
//		_mixerManager = new DoubleBufferSDLMixerManager();

		// Setup and start mixer
//		_mixerManager->init();
//	}

	/* Setup default save path to be workingdir/saves */

	char savePath[PATH_MAX+1];
	char workDirName[PATH_MAX+1];

	if (getcwd(workDirName, PATH_MAX) == NULL) {
		error("Could not obtain current working directory.");
	} else {
		printf("Current working directory: %s\n", workDirName);
	}

	strcpy(savePath, workDirName);
	strcat(savePath, "/../saves");
	printf("Current save directory: %s\n", savePath);
	struct stat sb;
	if (stat(savePath, &sb) == -1)
		if (errno == ENOENT) // Create the dir if it does not exist
			if (mkdir(savePath, 0755) != 0)
				warning("mkdir for '%s' failed!", savePath);

	_savefileManager = new DefaultSaveFileManager(savePath);

	#ifdef DUMP_STDOUT
		// The OpenPandora has a serial console on the EXT connection but most users do not use this so we
		// output all our STDOUT and STDERR to files for debug purposes.
		char STDOUT_FILE[PATH_MAX+1];
		char STDERR_FILE[PATH_MAX+1];

		strcpy(STDOUT_FILE, workDirName);
		strcpy(STDERR_FILE, workDirName);
		strcat(STDOUT_FILE, "/scummvm.stdout.txt");
		strcat(STDERR_FILE, "/scummvm.stderr.txt");

		// Flush the output in case anything is queued
		fclose(stdout);
		fclose(stderr);

		// Redirect standard input and standard output
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

	/* Trigger autosave every 4 minutes. */
	ConfMan.registerDefault("autosave_period", 4 * 60);

	ConfMan.registerDefault("fullscreen", true);

	/* Make sure that aspect ratio correction is enabled on the 1st run to stop
	   users asking me what the 'wasted space' at the bottom is ;-). */
	ConfMan.registerDefault("aspect_ratio", true);

	/* Make sure SDL knows that we have a joystick we want to use. */
	ConfMan.setInt("joystick_num", 0);

//	_graphicsMutex = createMutex();

	/* Pass to POSIX method to do the heavy lifting */
	OSystem_POSIX::initBackend();

	_inited = true;
}

	// enable joystick
//	if (joystick_num > -1 && SDL_NumJoysticks() > 0) {
//		printf("Using joystick: %s\n", SDL_JoystickName(0));
//		_joystick = SDL_JoystickOpen(joystick_num);
//	}
//
//	setupMixer();

	// Note: We could implement a custom SDLTimerManager by using
	// SDL_AddTimer. That might yield better timer resolution, but it would
	// also change the semantics of a timer: Right now, ScummVM timers
	// *never* run in parallel, due to the way they are implemented. If we
	// switched to SDL_AddTimer, each timer might run in a separate thread.
	// However, not all our code is prepared for that, so we can't just
	// switch. Still, it's a potential future change to keep in mind.
//	_timer = new DefaultTimerManager();
//	_timerID = SDL_AddTimer(10, &timer_handler, _timer);

void OSystem_OP::initSDL() {
	// Check if SDL has not been initialized
	if (!_initedSDL) {

		uint32 sdlFlags = SDL_INIT_EVENTTHREAD;
		if (ConfMan.hasKey("disable_sdl_parachute"))
			sdlFlags |= SDL_INIT_NOPARACHUTE;

		// Initialize SDL (SDL Subsystems are initiliazed in the corresponding sdl managers)
		if (SDL_Init(sdlFlags) == -1)
			error("Could not initialize SDL: %s", SDL_GetError());

    	uint8_t hiddenCursorData = 0;

    	hiddenCursor = SDL_CreateCursor(&hiddenCursorData, &hiddenCursorData, 8, 1, 0, 0);

		/* On the OpenPandora we need to work around an SDL assumption that
		   returns relative mouse coordinates when you get to the screen
		   edges using the touchscreen. The workaround is to set a blank
		   SDL cursor and not disable it (Hackish I know).

		   The root issues likes in the Windows Manager GRAB code in SDL.
		   That is why the issue is not seen on framebuffer devices like the
		   GP2X (there is no X window manager ;)).
		*/
		SDL_ShowCursor(SDL_ENABLE);
		SDL_SetCursor(hiddenCursor);
		SDL_EnableUNICODE(1);

//		memset(&_oldVideoMode, 0, sizeof(_oldVideoMode));
//		memset(&_videoMode, 0, sizeof(_videoMode));
//		memset(&_transactionDetails, 0, sizeof(_transactionDetails));

//		_videoMode.mode = GFX_DOUBLESIZE;
//		_videoMode.scaleFactor = 2;
//		_videoMode.aspectRatioCorrection = ConfMan.getBool("aspect_ratio");
//		_scalerProc = Normal2x;
//		_scalerType = 0;

//		_videoMode.fullscreen = true;

		_initedSDL = true;

//	OSystem_POSIX::initSDL();
	}
}

void OSystem_OP::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {

	/* Setup default extra data paths for engine data files and plugins */
	char workDirName[PATH_MAX+1];

	if (getcwd(workDirName, PATH_MAX) == NULL) {
		error("Error: Could not obtain current working directory.");
	}

	char enginedataPath[PATH_MAX+1];

	strcpy(enginedataPath, workDirName);
	strcat(enginedataPath, "/../data");
	printf("Default engine data directory: %s\n", enginedataPath);

	Common::FSNode engineNode(enginedataPath);
	if (engineNode.exists() && engineNode.isDirectory()) {
		s.add("__OP_ENGDATA__", new Common::FSDirectory(enginedataPath), priority);
	}
}

void OSystem_OP::quit() {

	SDL_FreeCursor(hiddenCursor);

	#ifdef DUMP_STDOUT
		printf("%s\n", "Debug: STDOUT and STDERR text files closed.");
		fclose(stdout);
		fclose(stderr);
	#endif /* DUMP_STDOUT */

	OSystem_POSIX::quit();
}

#endif
