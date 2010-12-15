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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// parser.c - handles all keyboard/command input

#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/parser.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/route.h"
#include "hugo/display.h"
#include "hugo/util.h"
#include "hugo/sound.h"
#include "hugo/object.h"

namespace Hugo {
Parser_v1w::Parser_v1w(HugoEngine *vm) : Parser_v3d(vm) {
}

Parser_v1w::~Parser_v1w() {
}

void Parser_v1w::keyHandler(uint16 nChar, uint16 nFlags) {
	debugC(1, kDebugParser, "keyHandler(%d, %d)", nChar, nFlags);

	status_t &gameStatus = _vm->getGameStatus();
	bool repeatedFl = (nFlags & 0x4000);            // TRUE if key is a repeat

// Process key down event - called from OnKeyDown()
	switch (nChar) {                                // Set various toggle states
	case Common::KEYCODE_ESCAPE:                    // Escape key, may want to QUIT
		if (gameStatus.inventoryState == I_ACTIVE)  // Remove inventory, if displayed
			gameStatus.inventoryState = I_UP;
		gameStatus.inventoryObjId = -1;             // Deselect any dragged icon
		break;
	case Common::KEYCODE_END:
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_UP:
	case Common::KEYCODE_DOWN:
		if (!repeatedFl) {
			gameStatus.routeIndex = -1;             // Stop any automatic route
			_vm->_route->setWalk(nChar);             // Direction of hero travel
		}
		break;
	case Common::KEYCODE_F1:                        // User Help (DOS)
		if (_checkDoubleF1Fl)
			_vm->_file->instructions();
		else
			_vm->_screen->userHelp();
		_checkDoubleF1Fl = !_checkDoubleF1Fl;
		break;
	case Common::KEYCODE_F2:                        // Toggle sound
		_vm->_sound->toggleSound();
		_vm->_sound->toggleMusic();
		break;
	case Common::KEYCODE_F3:                        // Repeat last line
		gameStatus.recallFl = true;
		break;
	case Common::KEYCODE_F4:                        // Save game
		if (gameStatus.viewState == V_PLAY)
			_vm->_file->saveGame(-1, Common::String());
		break;
	case Common::KEYCODE_F5:                        // Restore game
		_vm->_file->restoreGame(-1);
		_vm->_scheduler->restoreScreen(*_vm->_screen_p);
		gameStatus.viewState = V_PLAY;
		break;
	case Common::KEYCODE_F6:                        // Inventory
		if ((gameStatus.inventoryState == I_OFF) && (gameStatus.viewState == V_PLAY)) {
			gameStatus.inventoryState = I_DOWN;
			gameStatus.viewState = V_INVENT;
		}
		break;
	case Common::KEYCODE_F8:                        // Turbo mode
		_config.turboFl = !_config.turboFl;
		break;
	case Common::KEYCODE_F9:                        // Boss button
		warning("STUB: F9 (DOS) - BossKey");
		break;
	default:                                        // Any other key
		if (!gameStatus.storyModeFl) {              // Keyboard disabled
			// Add printable keys to ring buffer
			uint16 bnext = _putIndex + 1;
			if (bnext >= sizeof(_ringBuffer))
				bnext = 0;
			if (bnext != _getIndex) {
				_ringBuffer[_putIndex] = nChar;
				_putIndex = bnext;
			}
		}
		break;
	}
	if (_checkDoubleF1Fl && (nChar != Common::KEYCODE_F1))
		_checkDoubleF1Fl = false;
}

/**
* Parse the user's line of text input.  Generate events as necessary
*/
void Parser_v1w::lineHandler() {
	debugC(1, kDebugParser, "lineHandler()");

	status_t &gameStatus = _vm->getGameStatus();

	// Toggle God Mode
	if (!strncmp(_line, "PPG", 3)) {
		_vm->_sound->playSound(!_vm->_soundTest, BOTH_CHANNELS, HIGH_PRI);
		gameStatus.godModeFl ^= 1;
		return;
	}

	Utils::strlwr(_line);                           // Convert to lower case

	// God Mode cheat commands:
	// goto <screen>                                Takes hero to named screen
	// fetch <object name>                          Hero carries named object
	// fetch all                                    Hero carries all possible objects
	// find <object name>                           Takes hero to screen containing named object
	if (gameStatus.godModeFl) {
		// Special code to allow me to go straight to any screen
		if (strstr(_line, "goto")) {
			for (int i = 0; i < _vm->_numScreens; i++) {
				if (!strcmp(&_line[strlen("goto") + 1], _vm->_screenNames[i])) {
					_vm->_scheduler->newScreen(i);
					return;
				}
			}
		}

		// Special code to allow me to get objects from anywhere
		if (strstr(_line, "fetch all")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (_vm->_object->_objects[i].genericCmd & TAKE)
					takeObject(&_vm->_object->_objects[i]);
			}
			return;
		}

		if (strstr(_line, "fetch")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (!strcmp(&_line[strlen("fetch") + 1], _vm->_arrayNouns[_vm->_object->_objects[i].nounIndex][0])) {
					takeObject(&_vm->_object->_objects[i]);
					return;
				}
			}
		}

		// Special code to allow me to goto objects
		if (strstr(_line, "find")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (!strcmp(&_line[strlen("find") + 1], _vm->_arrayNouns[_vm->_object->_objects[i].nounIndex][0])) {
					_vm->_scheduler->newScreen(_vm->_object->_objects[i].screenIndex);
					return;
				}
			}
		}
	}

	// Special meta commands
	// EXIT/QUIT
	if (!strcmp("exit", _line) || strstr(_line, "quit")) {
		Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBExit]);
		return;
	}

	// SAVE/RESTORE
	if (!strcmp("save", _line) && gameStatus.viewState == V_PLAY) {
		_vm->_file->saveGame(-1, Common::String());
		return;
	}

	if (!strcmp("restore", _line) && (gameStatus.viewState == V_PLAY || gameStatus.viewState == V_IDLE)) {
		_vm->_file->restoreGame(-1);
		_vm->_scheduler->restoreScreen(*_vm->_screen_p);
		gameStatus.viewState = V_PLAY;
		return;
	}

	// Empty line
	if (*_line == '\0')                             // Empty line
		return;
	if (strspn(_line, " ") == strlen(_line))        // Nothing but spaces!
		return;

	if (gameStatus.gameOverFl) {
		// No commands allowed!
		Utils::gameOverMsg();
		return;
	}

	char farComment[XBYTES * 5] = "";               // hold 5 line comment if object not nearby

	// Test for nearby objects referenced explicitly
	for (int i = 0; i < _vm->_object->_numObj; i++) {
		object_t *obj = &_vm->_object->_objects[i];
		if (isWordPresent(_vm->_arrayNouns[obj->nounIndex])) {
			if (isObjectVerb(obj, farComment) || isGenericVerb(obj, farComment))
				return;
		}
	}

	// Test for nearby objects that only require a verb
	// Note comment is unused if not near.
	for (int i = 0; i < _vm->_object->_numObj; i++) {
		object_t *obj = &_vm->_object->_objects[i];
		if (obj->verbOnlyFl) {
			char contextComment[XBYTES * 5] = "";   // Unused comment for context objects
			if (isObjectVerb(obj, contextComment) || isGenericVerb(obj, contextComment))
				return;
		}
	}

	// No objects match command line, try background and catchall commands
	if (isBackgroundWord(_vm->_backgroundObjects[*_vm->_screen_p]))
		return;
	if (isCatchallVerb(_vm->_backgroundObjects[*_vm->_screen_p]))
		return;
	if (isBackgroundWord(_vm->_catchallList))
		return;
	if (isCatchallVerb(_vm->_catchallList))
		return;

	// If a not-near comment was generated, print it
	if (*farComment != '\0') {
		Utils::Box(BOX_ANY, "%s", farComment);
		return;
	}

	// Nothing matches.  Report recognition success to user.
	char *verb = findVerb();
	char *noun = findNoun();
	if (verb == _vm->_arrayVerbs[_vm->_look][0] && _maze.enabledFl) {
		Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBMaze]);
		_vm->_object->showTakeables();
	} else if (verb && noun) {                      // A combination I didn't think of
		Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBNoPoint]);
	} else if (noun) {
		Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBNoun]);
	} else if (verb) {
		Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBVerb]);
	} else {
		Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBEh]);
	}
}

} // End of namespace Hugo
