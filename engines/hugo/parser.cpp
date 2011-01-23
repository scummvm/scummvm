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

#include "common/system.h"
#include "common/events.h"

#include "hugo/hugo.h"
#include "hugo/display.h"
#include "hugo/parser.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/util.h"
#include "hugo/route.h"
#include "hugo/sound.h"
#include "hugo/object.h"

namespace Hugo {

Parser::Parser(HugoEngine *vm) :
	_vm(vm), _putIndex(0), _getIndex(0), _checkDoubleF1Fl(false) {
}

Parser::~Parser() {
}

void Parser::switchTurbo() {
	_vm->_config.turboFl = !_vm->_config.turboFl;
}

/**
* Add any new chars to line buffer and display them.
* If CR pressed, pass line to LineHandler()
*/
void Parser::charHandler() {
	debugC(4, kDebugParser, "charHandler");

	static int16  lineIndex = 0;                    // Index into line
	static uint32 tick = 0;                         // For flashing cursor
	static char   cursor = '_';
	static        command_t cmdLine;                // Build command line
	status_t     &gameStatus = _vm->getGameStatus();

	// Check for one or more characters in ring buffer
	while (_getIndex != _putIndex) {
		char c = _ringBuffer[_getIndex++];
		if (_getIndex >= sizeof(_ringBuffer))
			_getIndex = 0;

		switch (c) {
		case Common::KEYCODE_BACKSPACE:             // Rubout key
			if (lineIndex)
				cmdLine[--lineIndex] = '\0';
			break;
		case Common::KEYCODE_RETURN:                // EOL, pass line to line handler
			if (lineIndex && (_vm->_hero->pathType != kPathQuiet)) {
				// Remove inventory bar if active
				if (gameStatus.inventoryState == kInventoryActive)
					gameStatus.inventoryState = kInventoryUp;
				// Call Line handler and reset line
				command(cmdLine);
				cmdLine[lineIndex = 0] = '\0';
			}
			break;
		default:                                    // Normal text key, add to line
			if (lineIndex >= kMaxLineSize) {
				//MessageBeep(MB_ICONASTERISK);
				warning("STUB: MessageBeep() - Command line too long");
			} else if (isprint(c)) {
				cmdLine[lineIndex++] = c;
				cmdLine[lineIndex] = '\0';
			}
			break;
		}
	}

	// See if time to blink cursor, set cursor character
	if ((tick++ % (_vm->getTPS() / kBlinksPerSec)) == 0)
		cursor = (cursor == '_') ? ' ' : '_';

	// See if recall button pressed
	if (gameStatus.recallFl) {
		// Copy previous line to current cmdline
		gameStatus.recallFl = false;
		strcpy(cmdLine, _vm->_line);
		lineIndex = strlen(cmdLine);
	}

	sprintf(_vm->_statusLine, ">%s%c", cmdLine, cursor);
	sprintf(_vm->_scoreLine, "F1-Help  %s  Score: %d of %d Sound %s", (_vm->_config.turboFl) ? "T" : " ", _vm->getScore(), _vm->getMaxScore(), (_vm->_config.soundFl) ? "On" : "Off");

	// See if "look" button pressed
	if (gameStatus.lookFl) {
		command("look around");
		gameStatus.lookFl = false;
	}
}

void Parser::keyHandler(Common::Event event) {
	debugC(1, kDebugParser, "keyHandler(%d)", event.kbd.keycode);

	status_t &gameStatus = _vm->getGameStatus();
	uint16 nChar = event.kbd.keycode;

	// Process key down event - called from OnKeyDown()
	switch (nChar) {                                // Set various toggle states
	case Common::KEYCODE_ESCAPE:                    // Escape key, may want to QUIT
		if (gameStatus.viewState == kViewIntro)
			gameStatus.skipIntroFl = true;
		else {
			if (gameStatus.inventoryState == kInventoryActive) // Remove inventory, if displayed
				gameStatus.inventoryState = kInventoryUp;
			_vm->_screen->resetInventoryObjId();
		}
		break;
	case Common::KEYCODE_END:
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_PAGEDOWN:
	case Common::KEYCODE_KP1:
	case Common::KEYCODE_KP7:
	case Common::KEYCODE_KP9:
	case Common::KEYCODE_KP3:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_UP:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP4:
	case Common::KEYCODE_KP6:
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_KP2:
		gameStatus.routeIndex = -1;                 // Stop any automatic route
		_vm->_route->setWalk(nChar);                // Direction of hero travel
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
		if (gameStatus.viewState == kViewPlay) {
			if (gameStatus.gameOverFl)
				Utils::gameOverMsg();
			else
				_vm->_file->saveGame(-1, Common::String());
		}
		break;
	case Common::KEYCODE_F5:                        // Restore game
		_vm->_file->restoreGame(-1);
		_vm->_scheduler->restoreScreen(*_vm->_screen_p);
		gameStatus.viewState = kViewPlay;
		break;
	case Common::KEYCODE_F6:                        // Inventory
		showInventory();
		break;
	case Common::KEYCODE_F8:                        // Turbo mode
		switchTurbo();
		break;
	case Common::KEYCODE_F9:                        // Boss button
		warning("STUB: F9 (DOS) - BossKey");
		break;
	case Common::KEYCODE_l:
		if (event.kbd.hasFlags(Common::KBD_CTRL)) {
			_vm->_file->restoreGame(-1);
			_vm->_scheduler->restoreScreen(*_vm->_screen_p);
			gameStatus.viewState = kViewPlay;
		} else {
			if (!gameStatus.storyModeFl) {          // Keyboard disabled
				// Add printable keys to ring buffer
				uint16 bnext = _putIndex + 1;
				if (bnext >= sizeof(_ringBuffer))
					bnext = 0;
				if (bnext != _getIndex) {
					_ringBuffer[_putIndex] = event.kbd.ascii;
					_putIndex = bnext;
				}
			}
		}
		break;
	case Common::KEYCODE_n:
		if (event.kbd.hasFlags(Common::KBD_CTRL)) {
			warning("STUB: CTRL-N (WIN) - New Game");
		} else {
			if (!gameStatus.storyModeFl) {          // Keyboard disabled
				// Add printable keys to ring buffer
				uint16 bnext = _putIndex + 1;
				if (bnext >= sizeof(_ringBuffer))
					bnext = 0;
				if (bnext != _getIndex) {
					_ringBuffer[_putIndex] = event.kbd.ascii;
					_putIndex = bnext;
				}
			}
		}
		break;
	case Common::KEYCODE_s:
		if (event.kbd.hasFlags(Common::KBD_CTRL)) {
			if (gameStatus.viewState == kViewPlay) {
				if (gameStatus.gameOverFl)
					Utils::gameOverMsg();
				else
					_vm->_file->saveGame(-1, Common::String());
			}
		} else {
			if (!gameStatus.storyModeFl) {          // Keyboard disabled
				// Add printable keys to ring buffer
				uint16 bnext = _putIndex + 1;
				if (bnext >= sizeof(_ringBuffer))
					bnext = 0;
				if (bnext != _getIndex) {
					_ringBuffer[_putIndex] = event.kbd.ascii;
					_putIndex = bnext;
				}
			}
		}
		break;
	default:                                        // Any other key
		if (!gameStatus.storyModeFl) {              // Keyboard disabled
			// Add printable keys to ring buffer
			uint16 bnext = _putIndex + 1;
			if (bnext >= sizeof(_ringBuffer))
				bnext = 0;
			if (bnext != _getIndex) {
				_ringBuffer[_putIndex] = event.kbd.ascii;
				_putIndex = bnext;
			}
		}
		break;
	}
	if (_checkDoubleF1Fl && (nChar != Common::KEYCODE_F1))
		_checkDoubleF1Fl = false;
}

/**
* Perform an immediate command.  Takes parameters a la sprintf
* Assumes final string will not overrun line[] length
*/
void Parser::command(const char *format, ...) {
	debugC(1, kDebugParser, "Command(%s, ...)", format);

	va_list marker;
	va_start(marker, format);
	vsprintf(_vm->_line, format, marker);
	va_end(marker);

	lineHandler();
}

/**
* Locate any member of object name list appearing in command line
*/
bool Parser::isWordPresent(char **wordArr) {
	debugC(1, kDebugParser, "isWordPresent(%s)", wordArr[0]);

	if (wordArr != 0) {
		for (int i = 0; strlen(wordArr[i]); i++) {
			if (strstr(_vm->_line, wordArr[i]))
				return true;
		}
	}
	return false;
}

/**
* Locate word in list of nouns and return ptr to first string in noun list
*/
char *Parser::findNoun() {
	debugC(1, kDebugParser, "findNoun()");

	for (int i = 0; _vm->_arrayNouns[i]; i++) {
		for (int j = 0; strlen(_vm->_arrayNouns[i][j]); j++) {
			if (strstr(_vm->_line, _vm->_arrayNouns[i][j]))
				return _vm->_arrayNouns[i][0];
		}
	}
	return 0;
}

/**
* Locate word in list of verbs and return ptr to first string in verb list
*/
char *Parser::findVerb() {
	debugC(1, kDebugParser, "findVerb()");

	for (int i = 0; _vm->_arrayVerbs[i]; i++) {
		for (int j = 0; strlen(_vm->_arrayVerbs[i][j]); j++) {
			if (strstr(_vm->_line, _vm->_arrayVerbs[i][j]))
				return _vm->_arrayVerbs[i][0];
		}
	}
	return 0;
}

/**
* Show user all objects being carried in a variable width 2 column format
*/
void Parser::showDosInventory() {
	debugC(1, kDebugParser, "showDosInventory()");
	static const char *blanks = "                                        ";
	uint16 index = 0, len1 = 0, len2 = 0;

	for (int i = 0; i < _vm->_object->_numObj; i++) { // Find widths of 2 columns
		if (_vm->_object->isCarried(i)) {
			uint16 len = strlen(_vm->_arrayNouns[_vm->_object->_objects[i].nounIndex][1]);
			if (index++ & 1)                        // Right hand column
				len2 = (len > len2) ? len : len2;
			else
				len1 = (len > len1) ? len : len1;
		}
	}
	len1 += 1;                                      // For gap between columns

	if (len1 + len2 < (uint16)strlen(_vm->_textParser[kTBOutro]))
		len1 = strlen(_vm->_textParser[kTBOutro]);

	char buffer[kCompLineSize * kMaxTextRows] = "\0";
	strncat(buffer, blanks, (len1 + len2 - strlen(_vm->_textParser[kTBIntro])) / 2);
	strcat(strcat(buffer, _vm->_textParser[kTBIntro]), "\n");
	index = 0;
	for (int i = 0; i < _vm->_object->_numObj; i++) { // Assign strings
		if (_vm->_object->isCarried(i)) {
			if (index++ & 1)
				strcat(strcat(buffer, _vm->_arrayNouns[_vm->_object->_objects[i].nounIndex][1]), "\n");
			else
				strncat(strcat(buffer, _vm->_arrayNouns[_vm->_object->_objects[i].nounIndex][1]), blanks, len1 - strlen(_vm->_arrayNouns[_vm->_object->_objects[i].nounIndex][1]));
		}
	}
	if (index & 1)
		strcat(buffer, "\n");
	strcat(buffer, _vm->_textParser[kTBOutro]);

	Utils::Box(kBoxAny, "%s", buffer);
}

} // End of namespace Hugo
