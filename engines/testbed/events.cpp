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
 * $URL$
 * $Id$
 */

#include "common/events.h"
#include "common/keyboard.h"

#include "engines/engine.h"

#include "graphics/cursorman.h"

#include "testbed/events.h"
#include "testbed/graphics.h"

namespace Testbed {

struct keycodeToChar {
	Common::KeyCode code;
	char value;
} keyCodeLUT[] = {
		{Common::KEYCODE_a, 'a'},
		{Common::KEYCODE_b, 'b'},
		{Common::KEYCODE_c, 'c'},
		{Common::KEYCODE_d, 'd'},
		{Common::KEYCODE_e, 'e'},
		{Common::KEYCODE_f, 'f'},
		{Common::KEYCODE_g, 'g'},
		{Common::KEYCODE_h, 'h'},
		{Common::KEYCODE_i, 'i'},
		{Common::KEYCODE_j, 'j'},
		{Common::KEYCODE_k, 'k'},
		{Common::KEYCODE_l, 'l'},
		{Common::KEYCODE_m, 'm'},
		{Common::KEYCODE_n, 'n'},
		{Common::KEYCODE_o, 'o'},
		{Common::KEYCODE_p, 'p'},
		{Common::KEYCODE_q, 'q'},
		{Common::KEYCODE_r, 'r'},
		{Common::KEYCODE_s, 's'},
		{Common::KEYCODE_t, 't'},
		{Common::KEYCODE_u, 'u'},
		{Common::KEYCODE_v, 'v'},
		{Common::KEYCODE_w, 'w'},
		{Common::KEYCODE_x, 'x'},
		{Common::KEYCODE_y, 'y'},
		{Common::KEYCODE_z, 'z'},
		{Common::KEYCODE_SPACE, ' '}
	};

char EventTests::keystrokeToChar() {
	Common::EventManager *eventMan = g_system->getEventManager();
	bool quitLoop = false;
	Common::Event event;

	// handle all keybd events
	while (!quitLoop) {
		while (eventMan->pollEvent(event)) {

			// Quit if explicitly requested!
			if (Engine::shouldQuit()) {
				return 0;
			}

			switch (event.type) {
			case Common::EVENT_KEYDOWN :

				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					return 0;
				}
				for (int i = 0; i < ARRAYSIZE(keyCodeLUT); i++) {
					if (event.kbd.keycode == keyCodeLUT[i].code) {
						return keyCodeLUT[i].value;
					}
				}
				break;
			default:
				break;	// Ignore other events
			}
		}
	}

	return 0;
}

bool EventTests::mouseEvents() {
	Common::EventManager *eventMan = g_system->getEventManager();

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Generate mouse events make L/R/M button clicks", pt);
	pt.y = 120;
	Testsuite::writeOnScreen("Testbed should be able to detect them, Press X to exit", pt);

	// Init Mouse Palette
	GFXtests::initMousePalette();

	bool quitLoop = false;
	bool passed = true;
	// handle all mouse events
	Common::Event event;
	while (!quitLoop) {
		// Show mouse
		CursorMan.showMouse(true);
		g_system->updateScreen();

		while (eventMan->pollEvent(event)) {
			// Quit if explicitly requested
			if (Engine::shouldQuit()) {
				return passed;
			}
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				// Movements havee already been tested in GFX
				break;
			case Common::EVENT_LBUTTONDOWN:
				Testsuite::clearScreen();
				Testsuite::writeOnScreen("Mouse left-button pressed", pt);
				break;
			case Common::EVENT_RBUTTONDOWN:
				Testsuite::clearScreen();
				Testsuite::writeOnScreen("Mouse right-button pressed", pt);
				break;
			case Common::EVENT_WHEELDOWN:
				Testsuite::clearScreen();
				Testsuite::writeOnScreen("Mouse wheel moved down", pt);
				break;
			case Common::EVENT_MBUTTONDOWN:
				Testsuite::clearScreen();
				Testsuite::writeOnScreen("Mouse middle-button pressed ", pt);
				break;
			case Common::EVENT_LBUTTONUP:
				Testsuite::clearScreen();
				Testsuite::writeOnScreen("Mouse left-button released", pt);
				break;
			case Common::EVENT_RBUTTONUP:
				Testsuite::clearScreen();
				Testsuite::writeOnScreen("Mouse right-button released", pt);
				break;
			case Common::EVENT_WHEELUP:
				Testsuite::clearScreen();
				Testsuite::writeOnScreen("Mouse wheel moved up", pt);
				break;
			case Common::EVENT_MBUTTONUP:
				Testsuite::clearScreen();
				Testsuite::writeOnScreen("Mouse middle-button released ", pt);
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_x) {
					Testsuite::clearScreen();
					Testsuite::writeOnScreen("Exit requested", pt);
					quitLoop = true;
				}
				break;

			default:
					break;
			}

		}
	}

	CursorMan.showMouse(false);

	// Verify results now!
	if (Testsuite::handleInteractiveInput("Were mouse clicks L/R/M buttons identfied?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Mouse clicks (L/R/M buttons) failed");
		passed = false;
	}
	if (Testsuite::handleInteractiveInput("Were mouse wheel movements identified?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Mouse wheel movements failed");
		passed = false;
	}

	return passed;
}

bool EventTests::kbdEvents() {
	// Make user type some word and display the output on screen
	Common::String text = "You Entered : ";
	Common::Point pt(0, 100);
	Testsuite::clearScreen();
	Testsuite::writeOnScreen("Enter your word, press ESC when done, it will be echoed back", pt);
	pt.y += 20;
	Common::Rect rect = Testsuite::writeOnScreen(text, pt);
	char letter;
	while ((letter = keystrokeToChar()) != 0) {
		Testsuite::clearScreen(rect);
		text += letter;
		rect = Testsuite::writeOnScreen(text, pt);
	}

	bool passed = true;

	if (Testsuite::handleInteractiveInput("Was the word you entered same as that displayed on screen?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Keyboard Events failed");
		passed = false;
	}

	Testsuite::clearScreen();
	return passed;
}

bool EventTests::showMainMenu() {
	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event mainMenuEvent;
	mainMenuEvent.type = Common::EVENT_MAINMENU;
	eventMan->pushEvent(mainMenuEvent);

	bool passed = true;

	if (Testsuite::handleInteractiveInput("Were you able to see a main menu widget?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Event MAINMENU failed");
		passed = false;
	}

	return passed;
}

EventTestSuite::EventTestSuite() {
	addTest("Mouse Events", &EventTests::mouseEvents);
	addTest("Keyboard Events", &EventTests::kbdEvents);
	addTest("Mainmenu Event", &EventTests::showMainMenu);
}
const char *EventTestSuite::getName() const {
	return "Events";
}

} // End of namespace Testbed
