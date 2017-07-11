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

#include "common/debug.h"
#include "common/events.h"
#include "common/keyboard.h"

#include "engines/util.h"

#include "graphics/surface.h"

#include "sludge/allfiles.h"
#include "sludge/language.h"
#include "sludge/sludger.h"
#include "sludge/backdrop.h"
#include "sludge/language.h"
#include "sludge/newfatal.h"
#include "sludge/people.h"
#include "sludge/floor.h"
#include "sludge/objtypes.h"
#include "sludge/talk.h"
#include "sludge/statusba.h"
#include "sludge/transition.h"
#include "sludge/timing.h"
#include "sludge/sound.h"
#include "sludge/sludger.h"
#include "sludge/graphics.h"
#include "sludge/helpers.h"
#include "sludge/sludge.h"
#include "sludge/CommonCode/specialsettings.h"

namespace Sludge {

extern bool runningFullscreen;

#ifndef MAX_PATH
#define MAX_PATH        1024          // maximum size of a path name
#endif

HWND hMainWindow = NULL;

int realWinWidth = 640, realWinHeight = 480;
extern float cameraZoom;

extern int desiredfps;
extern int specialSettings;
extern inputType input;
extern variableStack *noStack;
extern Graphics::Surface backdropSurface;
Graphics::Surface renderSurface;

int dialogValue = 0;

Common::String gameName = "";
Common::String gamePath = "";
Common::String bundleFolder = "";

void setGameFilePath(char *f) {
	char currentDir[1000];
#if 0
	if (!getcwd(currentDir, 998)) {
		debugOut("Can't get current directory.\n");
	}

	int got = -1, a;

	for (a = 0; f[a]; a ++) {
		if (f[a] == PATHSLASH) got = a;
	}

	if (got != -1) {
		f[got] = 0;
		if (chdir(f)) {
			debugOut("Error: Failed changing to directory %s\n", f);
		}
		f[got] = PATHSLASH;
	}

	gamePath = new char[400];
	if (!checkNew(gamePath)) return;

	if (!getcwd(gamePath, 398)) {
		debugOut("Can't get game directory.\n");
	}

	if (chdir(currentDir)) {
		debugOut("Error: Failed changing to directory %s\n", currentDir);
	}
#endif
}

void saveHSI(Common::WriteStream *writer);

extern bool reallyWantToQuit;

int weAreDoneSoQuit;

void checkInput() {
#if 0
	static bool fakeRightclick = false;
#endif
	Common::Event event;

	/* Check for events */
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
#if 0
			case SDL_VIDEORESIZE:
				realWinWidth = event.resize.w;
				realWinHeight = event.resize.h;
				setGraphicsWindow(false, true, true);
				break;
#endif
			case Common::EVENT_MOUSEMOVE:
				input.justMoved = true;
				input.mouseX = event.mouse.x * ((float)winWidth / cameraZoom) / realWinWidth;
				input.mouseY = event.mouse.y * ((float)winHeight / cameraZoom) / realWinHeight;
				break;

			case Common::EVENT_LBUTTONDOWN:
				input.leftClick = true;
				input.mouseX = event.mouse.x * ((float)winWidth / cameraZoom) / realWinWidth;
				input.mouseY = event.mouse.y * ((float)winHeight / cameraZoom) / realWinHeight;
#if 0
				if (SDL_GetModState() & KMOD_CTRL) {
					input.rightClick = true;
					fakeRightclick = true;
				} else {
					input.leftClick = true;
					fakeRightclick = false;
				}
#endif
				break;

			case Common::EVENT_RBUTTONDOWN:
				input.rightClick = true;
				input.mouseX = event.mouse.x * ((float)winWidth / cameraZoom) / realWinWidth;
				input.mouseY = event.mouse.y * ((float)winHeight / cameraZoom) / realWinHeight;
				break;

			case Common::EVENT_LBUTTONUP:
				input.leftRelease = true;
				input.mouseX = event.mouse.x * ((float)winWidth / cameraZoom) / realWinWidth;
				input.mouseY = event.mouse.y * ((float)winHeight / cameraZoom) / realWinHeight;
				break;

			case Common::EVENT_RBUTTONUP:
				input.rightRelease = true;
				input.mouseX = event.mouse.x * ((float)winWidth / cameraZoom) / realWinWidth;
				input.mouseY = event.mouse.y * ((float)winHeight / cameraZoom) / realWinHeight;
				break;

			case Common::EVENT_KEYDOWN:
#if 0
				// A Windows key is pressed - let's leave fullscreen.
				if (runningFullscreen) {
					if (event.key.keysym.sym == SDLK_LSUPER || event.key.keysym.sym == SDLK_LSUPER) {
						setGraphicsWindow(!runningFullscreen);
					}
				}
				// Ignore Command keypresses - they're for the OS to handle.
				if (event.key.keysym.mod & KMOD_META) {
					// Command+F - let's switch to/from full screen
					if ('f' == event.key.keysym.unicode) {
						setGraphicsWindow(!runningFullscreen);
					}
					break;
				} else if (event.key.keysym.mod & KMOD_ALT) {
					// Alt + Enter also switches full screen mode
					if (SDLK_RETURN == event.key.keysym.sym) {
						setGraphicsWindow(!runningFullscreen);
					}
					if (SDLK_a == event.key.keysym.sym) {
						gameSettings.antiAlias = !gameSettings.antiAlias;
						break;
					}
					// Allow Alt+F4 to quit
					if (SDLK_F4 == event.key.keysym.sym) {
						SDL_Event event;
						event.type = SDL_QUIT;
						SDL_PushEvent(&event);
					}

					break;
				}
#endif
				switch (event.kbd.keycode) {

					case Common::KEYCODE_BACKSPACE:
						// fall through
					case Common::KEYCODE_DELETE:
						input.keyPressed = Common::KEYCODE_DELETE;
						break;
					default:
						input.keyPressed = event.kbd.keycode;
						break;
				}
				break;

			case Common::EVENT_QUIT:
				weAreDoneSoQuit = 1;
#if 0
				if (reallyWantToQuit) {
					// The game file has requested that we quit
					weAreDoneSoQuit = 1;
				} else {
					// The request is from elsewhere - ask for confirmation.
					setGraphicsWindow(false);
					//fprintf (stderr, "%s %s\n", gameName, getNumberedString(2));
					if (msgBoxQuestion(gameName, getNumberedString(2))) {
						weAreDoneSoQuit = 1;
					}
				}
#endif
				break;

			default:
				break;
		}
	}
}

int main_loop(const char *filename)
#if 0
		try
#endif
		{
	/* Dimensions of our window. */
	winWidth = 640;
	winHeight = 480;

	if (!initSludge(filename)) {
		return 0;
	}

	initGraphics(winWidth, winHeight, true, g_sludge->getScreenPixelFormat());

	// Init screen surface
	renderSurface.create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
#if 0
	/* Initialize the SDL library */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		msgBox("Startup Error: Couldn't initialize SDL.", SDL_GetError());
		exit(1);
	}
#endif
	if (gameIcon) {
#if 0
		if (SDL_Surface *programIcon = SDL_CreateRGBSurfaceFrom(gameIcon, iconW, iconH, 32, iconW * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) {
			SDL_WM_SetIcon(programIcon, NULL);
			SDL_FreeSurface(programIcon);
		}
#endif
		delete gameIcon;
	}
#if 0
	// Needed to make menu shortcuts work (on Mac), i.e. Command+Q for quit
	SDL_putenv((char *)"SDL_ENABLEAPPEVENTS=1");

	setupOpenGLStuff();
#endif

	registerWindowForFatal();

	if (!killResizeBackdrop(winWidth, winHeight))
		return fatal("Couldn't allocate memory for backdrop");

	blankScreen(0, 0, winWidth, winHeight);
	if (!initPeople())
		return fatal("Couldn't initialise people stuff");
	if (!initFloor())
		return fatal("Couldn't initialise floor stuff");
	if (!initObjectTypes())
		return fatal("Couldn't initialise object type stuff");
	initSpeech();
	initStatusBar();
	resetRandW();

	gameName = getNumberedString(1);

#if 0
	SDL_WM_SetCaption(gameName, gameName);

	if ((specialSettings & (SPECIAL_MOUSE_1 | SPECIAL_MOUSE_2)) == SPECIAL_MOUSE_1) {
		//  Hide the standard mouse cursor!
		// This is done in a weird way because there's bugs with SDL_ShowCursor(SDL_DISABLE);
		SDL_Cursor *cursor = NULL;
		Uint8 data = 0;
		SDL_FreeCursor(cursor);
		cursor = SDL_CreateCursor(&data, &data, 1, 1, 0, 0);
		SDL_SetCursor(cursor);
	}
#endif
	if (!(specialSettings & SPECIAL_SILENT)) {
		initSoundStuff(hMainWindow);
	}

	startNewFunctionNum(0, 0, NULL, noStack);

	Init_Timer();

	weAreDoneSoQuit = 0;
	while (!weAreDoneSoQuit) {
		checkInput();
		walkAllPeople();
		handleInput();
		sludgeDisplay();
		handleSoundLists();
		Wait_Frame();
	}

	killSoundStuff();

#if 0
	/* Clean up the SDL library */
	SDL_Quit();
#endif
	displayFatal();
	return (0);
}
#if 0
catch (std::exception &ex) { //NOTE by reference, not value
	std::cerr << "std::exception caught: " << ex.what() << std::endl;
	return -1;
} catch (...) {
	std::cerr << "Unknown exception was never caught" << std::endl;
	return -2;
}
#endif

}
 // End of namespace Sludge
