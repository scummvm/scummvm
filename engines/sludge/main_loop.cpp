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
#include "sludge/helpers.h"
#include "sludge/sludge.h"
#include "sludge/specialsettings.h"

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
				// TODO: if reallyWantToQuit, popup a message box to confirm
				break;

			default:
				break;
		}
	}
}

int main_loop(const char *filename)
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

	g_sludge->gameName = getNumberedString(1);

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

	return (0);
}

}
 // End of namespace Sludge
