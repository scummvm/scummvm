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
#include "common/keyboard.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/parser.h"
#include "hugo/global.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/sound.h"

namespace Hugo {

#define BLINKS  2                                   // Cursor blinks per second
#define CX(X)   LOWORD(X)
#define CY(Y)   HIWORD(Y)

Parser::Parser(HugoEngine &vm) :
	_vm(vm), _putIndex(0), _getIndex(0) {
		_checkDoubleF1Fl = false;
}

void Parser::keyHandler(uint16 nChar, uint16 nFlags) {
	status_t &gameStatus = _vm.getGameStatus();
	bool repeatedFl = (nFlags & 0x4000);            // TRUE if key is a repeat

	debugC(1, kDebugParser, "keyHandler(%d, %d)", nChar, nFlags);

// Process key down event - called from OnKeyDown()
	switch (nChar)  {                               // Set various toggle states
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
			_vm.route().setWalk(nChar);             // Direction of hero travel
		}
		break;
	case Common::KEYCODE_F1:                        // User Help (DOS)
		if (_checkDoubleF1Fl) {
			_vm.file().instructions();
			_checkDoubleF1Fl = false;
		} else
			_vm.screen().userHelp();
			_checkDoubleF1Fl = true;
		break;
	case Common::KEYCODE_F6:                        // Inventory
		showDosInventory();
		break;
	case Common::KEYCODE_F2:                        // Toggle sound
	case Common::KEYCODE_F3:                        // Repeat last line
	case Common::KEYCODE_F4:                        // Save game
	case Common::KEYCODE_F5:                        // Restore game
	case Common::KEYCODE_F8:                        // Turbo mode
	case Common::KEYCODE_F9:                        // Boss button
		warning("STUB: KeyHandler() - F2-F9 (DOS)");
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
	if 	((_checkDoubleF1Fl) && (nChar != Common::KEYCODE_F1))
		_checkDoubleF1Fl = false;
}

// Add any new chars to line buffer and display them.
// If CR pressed, pass line to Line_handler()
void Parser::charHandler() {
	static int16  lineIndex = 0;                    // Index into line
	static uint32 tick = 0;                         // For flashing cursor
	static char   cursor = '_';
	char          c;
	static        command_t cmdLine;                // Build command line
	status_t     &gameStatus = _vm.getGameStatus();
// Strangerke : Useless ?
//	bool          updateFl = (_getIndex != _putIndex);  // TRUE if any chars processed
//	command_t    status_line;                     // Includes prompt, cursor

	debugC(4, kDebugParser, "charHandler");

	// Check for one or more characters in ring buffer
	while (_getIndex != _putIndex) {
		c = _ringBuffer[_getIndex++];
		if (_getIndex >= sizeof(_ringBuffer))
			_getIndex = 0;

		switch (c) {
		case Common::KEYCODE_BACKSPACE:             // Rubout key
			if (lineIndex)
				cmdLine[--lineIndex] = '\0';
			break;
		case Common::KEYCODE_RETURN:                // EOL, pass line to line handler
			if (lineIndex && (_vm._hero->pathType != QUIET)) {
				// Remove inventory bar if active
				if (gameStatus.inventoryState == I_ACTIVE)
					gameStatus.inventoryState = I_UP;
				// Call Line handler and reset line
				command(cmdLine);
				cmdLine[lineIndex = 0] = '\0';
			}
			break;
		default:                                    // Normal text key, add to line
			if (lineIndex >= MAX_CHARS) {
				//MessageBeep(MB_ICONASTERISK);
				warning("STUB: MessageBeep(MB_ICONASTERISK);");
			} else if (isprint(c)) {
				cmdLine[lineIndex++] = c;
				cmdLine[lineIndex] = '\0';
			}
			break;
		}
	}

	// See if time to blink cursor, set cursor character
	if ((tick++ % (TPS / BLINKS)) == 0) {
// Strangerke : Useless ?
//		updateFl = true;                            // Force an update
		cursor = cursor == '_' ? ' ' : '_';
	}

	// See if recall button pressed
	if (gameStatus.recallFl) {
		// Copy previous line to current cmdline
		gameStatus.recallFl = false;
		strcpy(cmdLine, _line);
		lineIndex = strlen(cmdLine);
	}

	sprintf(_statusLine, ">%s%c", cmdLine, cursor);
	sprintf(_scoreLine, "Score: %d of %d", _vm.getScore(), _vm.getMaxScore());

	// See if "look" button pressed
	if (gameStatus.lookFl) {
		command("look around");
		gameStatus.lookFl = false;
	}
}

void Parser::drawStatusText() {
	debugC(4, kDebugParser, "drawStatusText");

	if (_vm.getPlatform() == Common::kPlatformWindows)
		_vm.screen().loadFont(U_FONT8);
	uint16 sdx = _vm.screen().stringLength(_statusLine);
	uint16 sdy = _vm.screen().fontHeight() + 1;                 // + 1 for shadow
	uint16 posX = 0;
	uint16 posY = YPIX - sdy;
	// Display the string and add rect to display list
	_vm.screen().writeStr(posX, posY, _statusLine, _TLIGHTYELLOW);
	_vm.screen().displayList(D_ADD, posX, posY, sdx, sdy);

	sdx = _vm.screen().stringLength(_scoreLine);
	posY = 0;
	_vm.screen().writeStr(posX, posY, _scoreLine, _TCYAN);
	_vm.screen().displayList(D_ADD, posX, posY, sdx, sdy);
}

// Perform an immediate command.  Takes parameters a la sprintf
// Assumes final string will not overrun line[] length
void Parser::command(const char *format, ...) {
	va_list marker;

	debugC(1, kDebugParser, "Command(%s, ...)", format);

	va_start(marker, format);
	vsprintf(_line, format, marker);
	va_end(marker);

	lineHandler();
}

char *Parser::strlwr(char *buffer) {
	char *result = buffer;

	debugC(1, kDebugParser, "strlwr(%s)", buffer);

	while (*buffer != '\0') {
		if (isupper(*buffer))
			*buffer = tolower(*buffer);
		buffer++;
	}

	return result;
}

// Parse the user's line of text input.  Generate events as necessary
void Parser::lineHandler() {
	char     *noun, *verb;                          // ptrs to noun and verb strings
//	int       i;
	object_t *obj;
	char      farComment[XBYTES * 5] = "";          // hold 5 line comment if object not nearby
	char      contextComment[XBYTES * 5] = "";      // Unused comment for context objects
	status_t &gameStatus = _vm.getGameStatus();


	debugC(1, kDebugParser, "lineHandler");

	// Toggle God Mode
	if (!strncmp(_line, "PPG", 3)) {
		_vm.sound().playSound(!_vm._soundTest, BOTH_CHANNELS, HIGH_PRI);
		gameStatus.godModeFl ^= 1;
		return;
	}

	strlwr(_line);                                  // Convert to lower case

	// God Mode cheat commands:
	// goto <screen>                                Takes hero to named screen
	// fetch <object name>                          Hero carries named object
	// fetch all                                    Hero carries all possible objects
	// find <object name>                           Takes hero to screen containing named object
	if (DEBUG || gameStatus.godModeFl) {
		// Special code to allow me to go straight to any screen
		if (strstr(_line, "goto"))
			for (int i = 0; i < _vm._numScreens; i++)
				if (!strcmp(&_line[strlen("goto") + 1], _vm._screenNames[i])) {
					_vm.scheduler().newScreen(i);
					return;
				}

		// Special code to allow me to get objects from anywhere
		if (strstr(_line, "fetch all")) {
			for (int i = 0; i < _vm._numObj; i++)
				if (_vm._objects[i].genericCmd & TAKE)
					takeObject(&_vm._objects[i]);
			return;
		}

		if (strstr(_line, "fetch")) {
			for (int i = 0; i < _vm._numObj; i++)
				if (!strcmp(&_line[strlen("fetch") + 1], _vm._arrayNouns[_vm._objects[i].nounIndex][0])) {
					takeObject(&_vm._objects[i]);
					return;
				}
		}

		// Special code to allow me to goto objects
		if (strstr(_line, "find"))
			for (int i = 0; i < _vm._numObj; i++)
				if (!strcmp(&_line[strlen("find") + 1], _vm._arrayNouns[_vm._objects[i].nounIndex][0])) {
					_vm.scheduler().newScreen(_vm._objects[i].screenIndex);
					return;
				}
	}

	// Special meta commands
	// EXIT/QUIT
	if (!strcmp("exit", _line) || strstr(_line, "quit")) {
		Utils::Box(BOX_ANY, _vm._textParser[kTBExit]);
		return;
	}

	// SAVE/RESTORE
	if (!strcmp("save", _line) && gameStatus.viewState == V_PLAY) {
		_vm.file().saveGame(gameStatus.saveSlot, "Current game");
		return;
	}

	if (!strcmp("restore", _line) && (gameStatus.viewState == V_PLAY || gameStatus.viewState == V_IDLE)) {
		_vm.file().restoreGame(gameStatus.saveSlot);
		_vm.scheduler().restoreScreen(*_vm._screen_p);
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

	// Test for nearby objects referenced explicitly
	for (int i = 0; i < _vm._numObj; i++) {
		obj = &_vm._objects[i];
		if (isWordPresent(_vm._arrayNouns[obj->nounIndex]))
			if (isObjectVerb(obj, _line, farComment) || isGenericVerb(obj, _line, farComment))
				return;
	}

	// Test for nearby objects that only require a verb
	// Note comment is unused if not near.
	for (int i = 0; i < _vm._numObj; i++) {
		obj = &_vm._objects[i];
		if (obj->verbOnlyFl)
			if (isObjectVerb(obj, _line, contextComment) || isGenericVerb(obj, _line, contextComment))
				return;
	}

	// No objects match command line, try background and catchall commands
	if (isBackgroundWord(_vm._backgroundObjects[*_vm._screen_p], _line))
		return;
	if (isCatchallVerb(_vm._backgroundObjects[*_vm._screen_p], _line))
		return;
	if (isBackgroundWord(_vm._catchallList, _line))
		return;
	if (isCatchallVerb(_vm._catchallList, _line))
		return;

	// If a not-near comment was generated, print it
	if (*farComment != '\0') {
		Utils::Box(BOX_ANY, farComment);
		return;
	}

	// Nothing matches.  Report recognition success to user.
	verb = findVerb(_line);
	noun = findNoun(_line);
	if (verb == _vm._arrayVerbs[_vm._look][0] && _maze.enabledFl) {
		Utils::Box(BOX_ANY, _vm._textParser[kTBMaze]);
		showTakeables();
	} else if (verb && noun)                          // A combination I didn't think of
		Utils::Box(BOX_ANY, _vm._textParser[kTBNoPoint]);
	else if (noun)
		Utils::Box(BOX_ANY, _vm._textParser[kTBNoun]);
	else if (verb)
		Utils::Box(BOX_ANY, _vm._textParser[kTBVerb]);
	else
		Utils::Box(BOX_ANY, _vm._textParser[kTBEh]);
}

// Search for matching verb/noun pairs in background command list
// Print text for possible background object.  Return TRUE if match found
bool Parser::isBackgroundWord(objectList_t obj, char *line) {
	debugC(1, kDebugParser, "isBackgroundWord(object_list_t obj, %s)", line);

	for (int i = 0; obj[i].verbIndex != 0; i++)
		if (isWordPresent(_vm._arrayVerbs[obj[i].verbIndex]) &&
		        isWordPresent(_vm._arrayNouns[obj[i].nounIndex]) &&
		        ((obj[i].roomState == DONT_CARE) ||
		         (obj[i].roomState == _vm._screenStates[*_vm._screen_p]))) {
			Utils::Box(BOX_ANY, _vm.file().fetchString(obj[i].commentIndex));
			_vm.scheduler().processBonus(obj[i].bonusIndex);
			return true;
		}
	return false;
}

// Search for matching verbs in background command list.
// Noun is not required.  Return TRUE if match found
// Note that if the background command list has match set TRUE then do not
// print text if there are any recognizable nouns in the command line
bool Parser::isCatchallVerb(objectList_t obj, char *line) {
	debugC(1, kDebugParser, "isCatchallVerb(object_list_t obj, %s)", line);

	for (int i = 0; obj[i].verbIndex != 0; i++)
		if (isWordPresent(_vm._arrayVerbs[obj[i].verbIndex]) && obj[i].nounIndex == 0 &&
		        (!obj[i].matchFl || !findNoun(line)) &&
		        ((obj[i].roomState == DONT_CARE) ||
		         (obj[i].roomState == _vm._screenStates[*_vm._screen_p]))) {
			Utils::Box(BOX_ANY, _vm.file().fetchString(obj[i].commentIndex));
			_vm.scheduler().processBonus(obj[i].bonusIndex);

			// If this is LOOK (without a noun), show any takeable objects
			if (*(_vm._arrayVerbs[obj[i].verbIndex]) == _vm._arrayVerbs[_vm._look][0])
				showTakeables();

			return(true);
		}
	return false;
}

// Test whether hero is close to object.  Return TRUE or FALSE
// If object not near, return suitable comment; may be another object close
// If radius is -1, treat radius as infinity
// Verb is included to determine correct comment if not near
bool Parser::isNear(object_t *obj, char *verb, char *comment) {
	debugC(1, kDebugParser, "isNear(object_t *obj, %s, %s)", verb, comment);

	if (obj->carriedFl)                             // Object is being carried
		return(true);

	if (obj->screenIndex != *_vm._screen_p) {
		// Not in same screen
		if (obj->objValue)
			strcpy(comment, _vm._textParser[kCmtAny1]);
		else
			strcpy(comment, _vm._textParser[kCmtAny2]);
		return(false);
	}

	if (obj->cycling == INVISIBLE) {
		if (obj->seqNumb) {
			// There is an image
			strcpy(comment, _vm._textParser[kCmtAny3]);
			return(false);
		} else
			// No image, assume visible
			if ((obj->radius < 0) ||
			        ((abs(obj->x - _vm._hero->x) <= obj->radius) &&
			         (abs(obj->y - _vm._hero->y - _vm._hero->currImagePtr->y2) <= obj->radius)))
				return(true);
			else {
				// User is not close enough
				if (obj->objValue && (verb != _vm._arrayVerbs[_vm._take][0]))
					strcpy(comment, _vm._textParser[kCmtAny1]);
				else
					strcpy(comment, _vm._textParser[kCmtClose]);
				return(false);
			}
	}

	if ((obj->radius < 0) ||
	        ((abs(obj->x - _vm._hero->x) <= obj->radius) &&
	         (abs(obj->y + obj->currImagePtr->y2 - _vm._hero->y - _vm._hero->currImagePtr->y2) <= obj->radius)))
		return(true);
	else {
		// User is not close enough
		if (obj->objValue && (verb != _vm._arrayVerbs[_vm._take][0]))
			strcpy(comment, _vm._textParser[kCmtAny1]);
		else
			strcpy(comment, _vm._textParser[kCmtClose]);
		return(false);
	}
	return true;
}

// Locate any member of object name list appearing in command line
bool Parser::isWordPresent(char **wordArr) {
	debugC(1, kDebugParser, "isWordPresent(%s)", wordArr[0]);

	if (wordArr != NULL) {
		for (int i = 0; strlen(wordArr[i]); i++)
			if (strstr(_line, wordArr[i]))
				return(true);
	}

	return false;
}

// Locate word in list of nouns and return ptr to first string in noun list
char *Parser::findNoun(char *line) {
	debugC(1, kDebugParser, "findNoun(%s)", line);

	for (int i = 0; _vm._arrayNouns[i]; i++)
		for (int j = 0; strlen(_vm._arrayNouns[i][j]); j++)
			if (strstr(line, _vm._arrayNouns[i][j]))
				return(_vm._arrayNouns[i][0]);
	return NULL;
}

// Locate word in list of verbs and return ptr to first string in verb list
char *Parser::findVerb(char *line) {
	debugC(1, kDebugParser, "findVerb(%s)", line);

	for (int i = 0; _vm._arrayVerbs[i]; i++)
		for (int j = 0; strlen(_vm._arrayVerbs[i][j]); j++)
			if (strstr(line, _vm._arrayVerbs[i][j]))
				return(_vm._arrayVerbs[i][0]);
	return NULL;
}

// Describe any takeable objects visible in this screen
void Parser::showTakeables() {
	object_t *obj;

	debugC(1, kDebugParser, "showTakeables");

	for (int j = 0; j < _vm._numObj; j++) {
		obj = &_vm._objects[j];
		if ((obj->cycling != INVISIBLE) &&
		        (obj->screenIndex == *_vm._screen_p) &&
		        (((TAKE & obj->genericCmd) == TAKE) || obj->objValue)) {
			sprintf(_textBoxBuffer, "You can also see:\n%s.", _vm._arrayNouns[obj->nounIndex][LOOK_NAME]);
			Utils::Box(BOX_ANY, _textBoxBuffer);
		}
	}
}

// Do all things necessary to carry an object
void Parser::takeObject(object_t *obj) {
	debugC(1, kDebugParser, "takeObject(object_t *obj)");

	obj->carriedFl = true;
	if (obj->seqNumb) {                             // Don't change if no image to display
		obj->cycling = INVISIBLE;
		if (_vm.getPlatform() != Common::kPlatformWindows)
			warning("takeObject : DOS version should use ALMOST_INVISIBLE");
	}
	_vm.adjustScore(obj->objValue);

	if (obj->seqNumb > 0)                               // If object has an image, force walk to dropped
		obj->viewx = -1;                                // (possibly moved) object next time taken!
	Utils::Box(BOX_ANY, TAKE_TEXT, _vm._arrayNouns[obj->nounIndex][TAKE_NAME]);
}

// Do all necessary things to drop an object
void Parser::dropObject(object_t *obj) {
	debugC(1, kDebugParser, "dropObject(object_t *obj)");

	obj->carriedFl = false;
	obj->screenIndex = *_vm._screen_p;
	if ((obj->seqNumb > 1) || (obj->seqList[0].imageNbr > 1))
		obj->cycling = CYCLE_FORWARD;
	else
		obj->cycling = NOT_CYCLING;
	obj->x = _vm._hero->x - 1;
	obj->y = _vm._hero->y + _vm._hero->currImagePtr->y2 - 1;
	obj->y = (obj->y + obj->currImagePtr->y2 < YPIX) ? obj->y : YPIX - obj->currImagePtr->y2 - 10;
	_vm.adjustScore(-obj->objValue);
	Utils::Box(BOX_ANY, _vm._textParser[kTBOk]);
}

// Test whether command line contains one of the generic actions
bool Parser::isGenericVerb(object_t *obj, char *line, char *comment) {
	debugC(1, kDebugParser, "isGenericVerb(object_t *obj, %s, %s)", line, comment);

	if (!obj->genericCmd)
		return false;

	// Following is equivalent to switch, but couldn't do one
	if (isWordPresent(_vm._arrayVerbs[_vm._look]) && isNear(obj, _vm._arrayVerbs[_vm._look][0], comment)) {
		// Test state-dependent look before general look
		if ((obj->genericCmd & LOOK_S) == LOOK_S) {
			Utils::Box(BOX_ANY, _vm._textData[obj->stateDataIndex[obj->state]]);
			warning("isGenericVerb: use of state dependant look - To be validated");
		} else {
			if ((LOOK & obj->genericCmd) == LOOK)
				if (_vm._textData[obj->dataIndex])
					Utils::Box(BOX_ANY, _vm._textData[obj->dataIndex]);
				else
					return(false);
			else
				Utils::Box(BOX_ANY, _vm._textParser[kTBUnusual]);
		}
	} else if (isWordPresent(_vm._arrayVerbs[_vm._take]) && isNear(obj, _vm._arrayVerbs[_vm._take][0], comment)) {
		if (obj->carriedFl)
			Utils::Box(BOX_ANY, _vm._textParser[kTBHave]);
		else if ((TAKE & obj->genericCmd) == TAKE)
			takeObject(obj);
		else if (obj->cmdIndex != 0)                // No comment if possible commands
			return false;
		else if (!obj->verbOnlyFl && (TAKE & obj->genericCmd) == TAKE)  // Make sure not taking object in context!
			Utils::Box(BOX_ANY, _vm._textParser[kTBNoUse]);
		else
			return false;
	} else if (isWordPresent(_vm._arrayVerbs[_vm._drop])) {
		if (!obj->carriedFl && ((DROP & obj->genericCmd) == DROP))
			Utils::Box(BOX_ANY, _vm._textParser[kTBDontHave]);
		else if (obj->carriedFl && ((DROP & obj->genericCmd) == DROP))
			dropObject(obj);
		else if (obj->cmdIndex == 0)
			Utils::Box(BOX_ANY, _vm._textParser[kTBNeed]);
		else
			return false;
	} else                                  // It was not a generic cmd
		return false;

	return true;
}

// Return TRUE if object being carried by hero
bool Parser::isCarrying(uint16 wordIndex) {
	debugC(1, kDebugParser, "isCarrying(%d)", wordIndex);

	for (int i = 0; i < _vm._numObj; i++)
		if ((wordIndex == _vm._objects[i].nounIndex) && _vm._objects[i].carriedFl)
			return true;
	return false;
}

// Test whether command line contains a verb allowed by this object.
// If it does, and the object is near and passes the tests in the command
// list then carry out the actions in the action list and return TRUE
bool Parser::isObjectVerb(object_t *obj, char *line, char *comment) {
	int     i;
	cmd    *cmnd;
	char   *verb;
	uint16 *reqs;
	uint16  cmdIndex;

	debugC(1, kDebugParser, "isObjectVerb(object_t *obj, %s, %s)", line, comment);

	// First, find matching verb in cmd list
	cmdIndex = obj->cmdIndex;                       // ptr to list of commands
	if (cmdIndex == 0)                              // No commands for this obj
		return false;

	for (i = 0; _vm._cmdList[cmdIndex][i].verbIndex != 0; i++)                  // For each cmd
		if (isWordPresent(_vm._arrayVerbs[_vm._cmdList[cmdIndex][i].verbIndex]))        // Was this verb used?
			break;
	if (_vm._cmdList[cmdIndex][i].verbIndex == 0)   // No verbs used.
		return false;

	// Verb match found.  Check if object is Near
	verb = *_vm._arrayVerbs[_vm._cmdList[cmdIndex][i].verbIndex];
	if (!isNear(obj, verb, comment))
		return(false);

	// Check all required objects are being carried
	cmnd = &_vm._cmdList[cmdIndex][i];              // ptr to struct cmd
	if (cmnd->reqIndex) {                           // At least 1 thing in list
		reqs = _vm._arrayReqs[cmnd->reqIndex];      // ptr to list of required objects
		for (i = 0; reqs[i]; i++)                   // for each obj
			if (!isCarrying(reqs[i])) {
				Utils::Box(BOX_ANY, _vm._textData[cmnd->textDataNoCarryIndex]);
				return true;
			}
	}

	// Required objects are present, now check state is correct
	if ((obj->state != cmnd->reqState) && (cmnd->reqState != DONT_CARE)) {
		Utils::Box(BOX_ANY, _vm._textData[cmnd->textDataWrongIndex]);
		return true;
	}

	// Everything checked.  Change the state and carry out any actions
	if (cmnd->reqState != DONT_CARE)                // Don't change new state if required state didn't care
		obj->state = cmnd->newState;
	Utils::Box(BOX_ANY, _vm._textData[cmnd->textDataDoneIndex]);
	_vm.scheduler().insertActionList(cmnd->actIndex);

	// See if any additional generic actions
	if ((verb == _vm._arrayVerbs[_vm._look][0]) || (verb == _vm._arrayVerbs[_vm._take][0]) || (verb == _vm._arrayVerbs[_vm._drop][0]))
		isGenericVerb(obj, line, comment);
	return true;
}

void Parser::showDosInventory() {
// Show user all objects being carried in a variable width 2 column format
static char *intro  = "You are carrying:";
static char *outro  = "\nPress ESCAPE to continue";
static char *blanks = "                                        ";
uint16 index, len, len1 = 0, len2 = 0;
char buffer[XBYTES * NUM_ROWS] = "\0";

	index = 0;
	for (int i = 0; i < _vm._numObj; i++)     /* Find widths of 2 columns */
		if (_vm._objects[i].carriedFl) {
			len = strlen(_vm._arrayNouns[_vm._objects[i].nounIndex][1]);
			if (index++ & 1)                    /* Right hand column */
				len2 = len > len2 ? len : len2;
			else
				len1 = len > len1 ? len : len1;
		}
	len1 += 1;                                  /* For gap between columns */

	if (len1 + len2 < (uint16)strlen(outro))
		len1 = strlen(outro);

	strncat (buffer, blanks, (len1 + len2 - strlen(intro)) / 2);
	strcat (strcat (buffer, intro), "\n");
	index = 0;
	for (int i = 0; i < _vm._numObj; i++)     /* Assign strings */
		if (_vm._objects[i].carriedFl)
			if (index++ & 1)
				strcat (strcat (buffer, _vm._arrayNouns[_vm._objects[i].nounIndex][1]), "\n");
			else
				strncat (strcat (buffer, _vm._arrayNouns[_vm._objects[i].nounIndex][1]), blanks, len1 - strlen(_vm._arrayNouns[_vm._objects[i].nounIndex][1]));
	if (index & 1) strcat (buffer, "\n");
	strcat (buffer, outro);

	Utils::Box(BOX_ANY, buffer);
}


} // end of namespace Hugo
