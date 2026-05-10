/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// parser.c - handles all keyboard/command input

#include "common/debug.h"
#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/display.h"
#include "hugo/parser.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/util.h"
#include "hugo/sound.h"
#include "hugo/object.h"
#include "hugo/text.h"

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

	// Reset the prompt on screen (DOS only)
	_vm->_screen->updatePromptText("", ' ');
	_vm->_screen->displayPromptText();

	Status &gameStatus = _vm->getGameStatus();

	// Toggle God Mode
	if (!strncmp(_vm->_line, "PPG", 3)) {
		_vm->_sound->playSound(_vm->_soundTest, kSoundPriorityHigh);
		gameStatus._godModeFl = !gameStatus._godModeFl;
		return;
	}

	Utils::hugo_strlwr(_vm->_line);                      // Convert to lower case

	// God Mode cheat commands:
	// goto <screen>                                Takes hero to named screen
	// fetch <object name>                          Hero carries named object
	// fetch all                                    Hero carries all possible objects
	// find <object name>                           Takes hero to screen containing named object
	if (gameStatus._godModeFl) {
		// Special code to allow me to go straight to any screen
		if (strstr(_vm->_line, "goto")) {
			for (int i = 0; i < _vm->_numScreens; i++) {
				if (!scumm_stricmp(&_vm->_line[strlen("goto") + 1], _vm->_text->getScreenNames(i))) {
					_vm->_scheduler->newScreen(i);
					return;
				}
			}
		}

		// Special code to allow me to get objects from anywhere
		if (strstr(_vm->_line, "fetch all")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (_vm->_object->_objects[i]._genericCmd & TAKE)
					takeObject(&_vm->_object->_objects[i]);
			}
			return;
		}

		if (strstr(_vm->_line, "fetch")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (!scumm_stricmp(&_vm->_line[strlen("fetch") + 1], _vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, 0))) {
					takeObject(&_vm->_object->_objects[i]);
					return;
				}
			}
		}

		// Special code to allow me to goto objects
		if (strstr(_vm->_line, "find")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (!scumm_stricmp(&_vm->_line[strlen("find") + 1], _vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, 0))) {
					_vm->_scheduler->newScreen(_vm->_object->_objects[i]._screenIndex);
					return;
				}
			}
		}
	}

	if (!strcmp("exit", _vm->_line) || strstr(_vm->_line, "quit")) {
		endGamePrompt();
		return;
	}

	// SAVE/RESTORE
	if (!strcmp("save", _vm->_line)) {
		if (gameStatus._gameOverFl)
			_vm->gameOverMsg();
		else
			_vm->_file->saveGame(-1, Common::String());
		return;
	}

	if (!strcmp("restore", _vm->_line)) {
		_vm->_file->restoreGame(-1);
		return;
	}

	if (*_vm->_line == '\0')                        // Empty line
		return;

	if (strspn(_vm->_line, " ") == strlen(_vm->_line)) // Nothing but spaces!
		return;

	if (gameStatus._gameOverFl) {
		// No commands allowed!
		_vm->gameOverMsg();
		return;
	}

	// Find the first verb in the line
	const char *verb = findVerb();
	const char *noun = nullptr;                           // Noun not found yet
	char farComment[kCompLineSize * 5] = "";        // hold 5 line comment if object not nearby

	if (verb) {                                     // OK, verb found.  Try to match with object
		do {
			noun = findNextNoun(noun);              // Find a noun in the line
			// Must try at least once for objects allowing verb-context
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				Object *obj = &_vm->_object->_objects[i];
				if (isNear_v1(verb, noun, obj, farComment)) {
					if (isObjectVerb_v2(verb, obj)  // Foreground object
					 || isGenericVerb_v2(verb, obj))// Common action type
						return;
				}
			}
			if ((*farComment == '\0') && isBackgroundWord_v2(noun, verb, _backgroundObjects[*_vm->_screenPtr]))
				return;
		} while (noun);
	}

	noun = findNextNoun(noun);
	if (   !isCatchallVerb_v2(true, noun, verb, _backgroundObjects[*_vm->_screenPtr])
		&& !isCatchallVerb_v2(true, noun, verb, _catchallList)
		&& !isCatchallVerb_v2(false, noun, verb, _backgroundObjects[*_vm->_screenPtr])
		&& !isCatchallVerb_v2(false, noun, verb, _catchallList)) {
		if (*farComment != '\0') {                  // An object matched but not near enough
			_vm->notifyBox(farComment);
		} else if (_vm->_maze._enabledFl && (verb == _vm->_text->getVerb(_vm->_look, 0))) {
			_vm->notifyBox(_vm->_text->getTextParser(kTBMaze));
			_vm->_object->showTakeables();
		} else if (verb && noun) {                  // A combination I didn't think of
			_vm->notifyBox(_vm->_text->getTextParser(kTBNoUse_2d));
		} else if (verb || noun) {
			_vm->notifyBox(_vm->_text->getTextParser(kTBNoun));
		} else {
			_vm->notifyBox(_vm->_text->getTextParser(kTBEh_2d));
		}
	}
}

/**
 * Test whether supplied verb is one of the common variety for this object
 * say_ok needed for special case of take/drop which may be handled not only
 * here but also in a cmd_list with a donestr string simultaneously
 */
bool Parser_v2d::isGenericVerb_v2(const char *word, Object *obj) {
	debugC(1, kDebugParser, "isGenericVerb(%s, Object *obj)", word);

	if (!obj->_genericCmd)
		return false;

	// Following is equivalent to switch, but couldn't do one
	if (word == _vm->_text->getVerb(_vm->_look, 0)) {
		if ((LOOK & obj->_genericCmd) == LOOK)
			if (obj->_dataIndex != 0)
				_vm->notifyBox(_vm->_text->getTextData(obj->_dataIndex));
			else
				return false;
		else
			_vm->notifyBox(_vm->_text->getTextParser(kTBUnusual_1d));
	} else if (word == _vm->_text->getVerb(_vm->_take, 0)) {
		if (obj->_carriedFl)
			_vm->notifyBox(_vm->_text->getTextParser(kTBHave));
		else if ((TAKE & obj->_genericCmd) == TAKE)
			takeObject(obj);
		else if (obj->_cmdIndex)                     // No comment if possible commands
			return false;
		else if (!obj->_verbOnlyFl)                  // Make sure not taking object in context!
			_vm->notifyBox(_vm->_text->getTextParser(kTBNoUse));
		else
			return false;
	} else if (word == _vm->_text->getVerb(_vm->_drop, 0)) {
		if (!obj->_carriedFl)
			_vm->notifyBox(_vm->_text->getTextParser(kTBDontHave));
		else if ((DROP & obj->_genericCmd) == DROP)
			dropObject(obj);
		else
			_vm->notifyBox(_vm->_text->getTextParser(kTBNeed));
	} else {                                        // It was not a generic cmd
		return false;
	}

	return true;
}

/**
 * Test whether supplied verb is included in the list of allowed verbs for
 * this object.  If it is, then perform the tests on it from the cmd list
 * and if it passes, perform the actions in the action list.  If the verb
 * is catered for, return TRUE
 */
bool Parser_v2d::isObjectVerb_v2(const char *word, Object *obj) {
	debugC(1, kDebugParser, "isObjectVerb(%s, Object *obj)", word);

	// First, find matching verb in cmd list
	uint16 cmdIndex = obj->_cmdIndex;                // ptr to list of commands
	if (!cmdIndex)                                  // No commands for this obj
		return false;

	int i;
	for (i = 0; _cmdList[cmdIndex][i]._verbIndex != 0; i++) { // For each cmd
		if (word == _vm->_text->getVerb(_cmdList[cmdIndex][i]._verbIndex, 0)) // Is this verb catered for?
			break;
	}

	if (_cmdList[cmdIndex][i]._verbIndex == 0)       // No
		return false;

	// Verb match found, check all required objects are being carried
	cmd *cmnd = &_cmdList[cmdIndex][i];             // ptr to struct cmd
	if (cmnd->_reqIndex) {                          // At least 1 thing in list
		uint16 *reqs = _arrayReqs[cmnd->_reqIndex]; // ptr to list of required objects
		for (i = 0; reqs[i]; i++) {                 // for each obj
			if (!_vm->_object->isCarrying(reqs[i])) {
				_vm->notifyBox(_vm->_text->getTextData(cmnd->_textDataNoCarryIndex));
				return true;
			}
		}
	}

	// Required objects are present, now check state is correct
	if ((obj->_state != cmnd->_reqState) && (cmnd->_reqState != kStateDontCare)){
		_vm->notifyBox(_vm->_text->getTextData(cmnd->_textDataWrongIndex));
		return true;
	}

	// Everything checked.  Change the state and carry out any actions
	if (cmnd->_reqState != kStateDontCare)           // Don't change new state if required state didn't care
		obj->_state = cmnd->_newState;
	_vm->notifyBox(_vm->_text->getTextData(cmnd->_textDataDoneIndex));
	_vm->_scheduler->insertActionList(cmnd->_actIndex);
	// Special case if verb is Take or Drop.  Assume additional generic actions
	if ((word == _vm->_text->getVerb(_vm->_take, 0)) || (word == _vm->_text->getVerb(_vm->_drop, 0)))
		isGenericVerb_v2(word, obj);
	return true;
}

/**
 * Print text for possible background object.  Return TRUE if match found
 * Only match if both verb and noun found.  Test_ca will match verb-only
 */
bool Parser_v2d::isBackgroundWord_v2(const char *noun, const char *verb, ObjectList obj) const {
	debugC(1, kDebugParser, "isBackgroundWord(%s, %s, object_list_t obj)", noun, verb);

	// WORKAROUND: obj is an invalid pointer if in the Hugo2 maze (original bug)
	if (*(_vm->_screenPtr) >= _backgroundObjectsSize) {
		return false;
	}

	if (!noun)
		return false;

	for (int i = 0; obj[i]._verbIndex; i++) {
		if ((verb == _vm->_text->getVerb(obj[i]._verbIndex, 0)) && (noun == _vm->_text->getNoun(obj[i]._nounIndex, 0)) &&
			(obj[i]._roomState == kStateDontCare ||
			 obj[i]._roomState == _vm->_screenStates[*_vm->_screenPtr])) {
			_vm->notifyBox(_vm->_file->fetchString(obj[i]._commentIndex));
			_vm->_scheduler->processBonus(obj[i]._bonusIndex);
			return true;
		}
	}
	return false;
}

/**
 * Print text for possible background object.  Return TRUE if match found
 * If test_noun TRUE, must have a noun given
 * Algorithm:  If (noun matches) OR (match not required AND match specifies NULL) OR
 * (no noun present and match required) print comment
 * i.e. NULL allows any or no noun, match TRUE allows no noun or matching noun 
 */
bool Parser_v2d::isCatchallVerb_v2(bool testNounFl, const char *noun, const char *verb, ObjectList obj) const {
	debugC(1, kDebugParser, "isCatchallVerb(%d, %s, %s, object_list_t obj)", (testNounFl) ? 1 : 0, noun, verb);

	// WORKAROUND: obj is an invalid pointer if in the Hugo2 maze (original bug)
	if (*(_vm->_screenPtr) >= _backgroundObjectsSize) {
		return false;
	}

	if (testNounFl && !noun)
		return false;

	for (int i = 0; obj[i]._verbIndex; i++) {
		if ((verb == _vm->_text->getVerb(obj[i]._verbIndex, 0)) &&
			((noun == _vm->_text->getNoun(obj[i]._nounIndex, 0)) ||
			 (obj[i]._nounIndex == 0 && !obj[i]._matchFl) ||
			 (!noun && obj[i]._matchFl)) &&
			(obj[i]._roomState == kStateDontCare ||
			 obj[i]._roomState == _vm->_screenStates[*_vm->_screenPtr])) {

			_vm->notifyBox(_vm->_file->fetchString(obj[i]._commentIndex));
			_vm->_scheduler->processBonus(obj[i]._bonusIndex);
			// If this is LOOK without a noun, show any takeable objects
			if (!noun && verb == _vm->_text->getVerb(_vm->_look, 0))
				_vm->_object->showTakeables();
			return true;
		}
	}
	return false;
}

} // End of namespace Hugo
