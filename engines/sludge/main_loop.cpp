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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/keyboard.h"

#include "graphics/surface.h"

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/floor.h"
#include "sludge/graphics.h"
#include "sludge/helpers.h"
#include "sludge/language.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/statusba.h"
#include "sludge/sound.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/talk.h"
#include "sludge/transition.h"
#include "sludge/timing.h"

namespace Sludge {

#ifndef MAX_PATH
#define MAX_PATH        1024          // maximum size of a path name
#endif

HWND hMainWindow = NULL;

int realWinWidth = 640, realWinHeight = 480;

extern InputType input;
extern VariableStack *noStack;

int dialogValue = 0;

int weAreDoneSoQuit;

void checkInput() {
	int winWidth = g_system->getWidth();
	int winHeight = g_system->getHeight();
	float cameraZoom = g_sludge->_gfxMan->getCamZoom();
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

int main_loop(const char *filename) {

	if (!initSludge(filename)) {
		return 0;
	}

	g_sludge->_gfxMan->init();

	registerWindowForFatal();

	g_sludge->_gfxMan->blankAllScreen();
	if (!initPeople())
		return fatal("Couldn't initialise people stuff");
	if (!initFloor())
		return fatal("Couldn't initialise floor stuff");
	if (!g_sludge->_objMan->initObjectTypes())
		return fatal("Couldn't initialise object type stuff");
	initSpeech();
	initStatusBar();
	resetRandW();

	if (!ConfMan.hasKey("mute") || !ConfMan.getBool("mute")) {
		initSoundStuff(hMainWindow);
	}

	startNewFunctionNum(0, 0, NULL, noStack);

	g_sludge->_timer.init();

	weAreDoneSoQuit = 0;
	while (!weAreDoneSoQuit) {
		checkInput();
		walkAllPeople();
		handleInput();
		sludgeDisplay();
		handleSoundLists();
		g_sludge->_timer.waitFrame();
	}

	killSoundStuff();

	return (0);
}

}
 // End of namespace Sludge
