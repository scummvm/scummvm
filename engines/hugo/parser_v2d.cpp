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
#include "hugo/util.h"
#include "hugo/object.h"

namespace Hugo {

Parser_v2d::Parser_v2d(HugoEngine *vm) : Parser_v1d(vm) {
}

Parser_v2d::~Parser_v2d() {
}

/**
* Parse the user's line of text input.  Generate events as necessary
*/
void Parser_v2d::lineHandler() {
	debugC(1, kDebugParser, "lineHandler()");

	object_t    *obj;
	status_t &gameStatus = _vm->getGameStatus();
	char        farComment[XBYTES * 5] = "";        // hold 5 line comment if object not nearby

//	Reset_prompt_line ();
	Utils::strlwr(_line);                           // Convert to lower case

	if (!strcmp("exit", _line) || strstr(_line, "quit")) {
		if (Utils::Box(BOX_YESNO, "%s", _vm->_textParser[kTBExit_1d]) != 0)
			_vm->endGame();
		else
			return;
	}

	// SAVE/RESTORE
	if (!strcmp("save", _line)) {
		_config.soundFl = false;
		if (gameStatus.gameOverFl)
			Utils::gameOverMsg();
		else
			_vm->_file->saveGame(-1, Common::String());
		return;
	}

	if (!strcmp("restore", _line)) {
		_config.soundFl = false;
		_vm->_file->restoreGame(-1);
		_vm->_scheduler->restoreScreen(*_vm->_screen_p);
		gameStatus.viewState = V_PLAY;
		return;
	}

	if (*_line == '\0')                             // Empty line
		return;

	if (strspn(_line, " ") == strlen(_line))        // Nothing but spaces!
		return;

	if (gameStatus.gameOverFl) {                    // No commands allowed!
		Utils::gameOverMsg();
		return;
	}

	// Find the first verb in the line
	char *verb = findVerb();
	char *noun = 0;                                 // Noun not found yet

	if (verb) {                                     // OK, verb found.  Try to match with object
		do {
			noun = findNextNoun(noun);              // Find a noun in the line
			// Must try at least once for objects allowing verb-context
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				obj = &_vm->_object->_objects[i];
				if (isNear(verb, noun, obj, farComment)) {
					if (isObjectVerb(verb, obj)     // Foreground object
					 || isGenericVerb(verb, obj))   // Common action type
						return;
				}
			}
			if ((*farComment != '\0') && isBackgroundWord(noun, verb, _vm->_backgroundObjects[*_vm->_screen_p]))
				return;
		} while (noun);
	}

	noun = findNextNoun(noun);
	if (   !isCatchallVerb(true, noun, verb, _vm->_backgroundObjects[*_vm->_screen_p])
		&& !isCatchallVerb(true, noun, verb, _vm->_catchallList)
		&& !isCatchallVerb(false, noun, verb, _vm->_backgroundObjects[*_vm->_screen_p])
		&& !isCatchallVerb(false, noun, verb, _vm->_catchallList)) {
		if (*farComment != '\0') {                  // An object matched but not near enough
			Utils::Box(BOX_ANY, "%s", farComment);
		} else if (_maze.enabledFl && (verb == _vm->_arrayVerbs[_vm->_look][0])) {
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBMaze]);
			_vm->_object->showTakeables();
		} else if (verb && noun) {                  // A combination I didn't think of
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBNoUse_2d]);
		} else if (verb || noun) {
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBNoun]);
		} else {
			Utils::Box(BOX_ANY, "%s", _vm->_textParser[kTBEh_2d]);
		}
	}
}

} // End of namespace Hugo
