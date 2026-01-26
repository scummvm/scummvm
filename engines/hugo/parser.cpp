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

#include "common/events.h"
#include "common/textconsole.h"
#include "gui/debugger.h"

#include "hugo/hugo.h"
#include "hugo/display.h"
#include "hugo/parser.h"
#include "hugo/file.h"
#include "hugo/util.h"
#include "hugo/route.h"
#include "hugo/sound.h"
#include "hugo/object.h"
#include "hugo/text.h"
#include "hugo/inventory.h"
#include "hugo/mouse.h"

namespace Hugo {

Parser::Parser(HugoEngine *vm) : _vm(vm), _putIndex(0), _getIndex(0) {
	_catchallList = nullptr;
	_arrayReqs = nullptr;

	_backgroundObjects = nullptr;
	_backgroundObjectsSize = 0;
	_cmdList = nullptr;
	_cmdListSize = 0;

	_cmdLineIndex = 0;
	_cmdLineTick = 0;
	_cmdLineCursor = '_';
	_cmdLine[0] = '\0';
	_checkDoubleF1Fl = false;
}

Parser::~Parser() {
}

uint16 Parser::getCmdDefaultVerbIdx(const uint16 index) const {
	return _cmdList[index][0]._verbIndex;
}

/**
 * Read a cmd structure from Hugo.dat
 */
void Parser::readCmd(Common::ReadStream &in, cmd &curCmd) {
	curCmd._verbIndex = in.readUint16BE();
	curCmd._reqIndex = in.readUint16BE();
	curCmd._textDataNoCarryIndex = in.readUint16BE();
	curCmd._reqState = in.readByte();
	curCmd._newState = in.readByte();
	curCmd._textDataWrongIndex = in.readUint16BE();
	curCmd._textDataDoneIndex = in.readUint16BE();
	curCmd._actIndex = in.readUint16BE();
}

/**
 * Load _cmdList from Hugo.dat
 */
void Parser::loadCmdList(Common::ReadStream &in) {
	cmd tmpCmd;
	memset(&tmpCmd, 0, sizeof(tmpCmd));
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_cmdListSize = numElem;
			_cmdList = (cmd **)malloc(sizeof(cmd *) * _cmdListSize);
		}

		for (int16 i = 0; i < numElem; i++) {
			uint16 numSubElem = in.readUint16BE();
			if (varnt == _vm->_gameVariant)
				_cmdList[i] = (cmd *)malloc(sizeof(cmd) * numSubElem);
			for (int16 j = 0; j < numSubElem; j++)
				readCmd(in, (varnt == _vm->_gameVariant) ? _cmdList[i][j] : tmpCmd);
		}
	}
}


void Parser::readBG(Common::ReadStream &in, Background &curBG) {
	curBG._verbIndex = in.readUint16BE();
	curBG._nounIndex = in.readUint16BE();
	curBG._commentIndex = in.readSint16BE();
	curBG._matchFl = (in.readByte() != 0);
	curBG._roomState = in.readByte();
	curBG._bonusIndex = in.readByte();
}

/**
 * Read _backgrounObjects from Hugo.dat
 */
void Parser::loadBackgroundObjects(Common::ReadStream &in) {
	Background tmpBG;
	memset(&tmpBG, 0, sizeof(tmpBG));

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();

		if (varnt == _vm->_gameVariant) {
			_backgroundObjectsSize = numElem;
			_backgroundObjects = (Background **)malloc(sizeof(Background *) * numElem);
		}

		for (int i = 0; i < numElem; i++) {
			uint16 numSubElem = in.readUint16BE();
			if (varnt == _vm->_gameVariant)
				_backgroundObjects[i] = (Background *)malloc(sizeof(Background) * numSubElem);

			for (int j = 0; j < numSubElem; j++)
				readBG(in, (varnt == _vm->_gameVariant) ? _backgroundObjects[i][j] : tmpBG);
		}
	}
}

/**
 * Read _catchallList from Hugo.dat
 */
void Parser::loadCatchallList(Common::ReadStream &in) {
	Background *wrkCatchallList = nullptr;
	Background tmpBG;
	memset(&tmpBG, 0, sizeof(tmpBG));

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();

		if (varnt == _vm->_gameVariant)
			_catchallList = wrkCatchallList = (Background *)malloc(sizeof(Background) * numElem);

		for (int i = 0; i < numElem; i++)
			readBG(in, (varnt == _vm->_gameVariant) ? wrkCatchallList[i] : tmpBG);
	}
}

void Parser::loadArrayReqs(Common::SeekableReadStream &in) {
	_arrayReqs = _vm->loadLongArray(in);
}

/**
 * Search background command list for this screen for supplied object.
 * Return first associated verb (not "look") or 0 if none found.
 */
const char *Parser::useBG(const char *name) {
	debugC(1, kDebugEngine, "useBG(%s)", name);

	ObjectList p = _backgroundObjects[*_vm->_screenPtr];
	for (int i = 0; p[i]._verbIndex != 0; i++) {
		if ((name == _vm->_text->getNoun(p[i]._nounIndex, 0) &&
		     p[i]._verbIndex != _vm->_look) &&
		    ((p[i]._roomState == kStateDontCare) || (p[i]._roomState == _vm->_screenStates[*_vm->_screenPtr])))
			return _vm->_text->getVerb(p[i]._verbIndex, 0);
	}

	return nullptr;
}

void Parser::freeParser() {
	if (_arrayReqs) {
		for (int i = 0; _arrayReqs[i] != nullptr; i++)
			free(_arrayReqs[i]);
		free(_arrayReqs);
		_arrayReqs = nullptr;
	}

	free(_catchallList);
	_catchallList = nullptr;

	if (_backgroundObjects) {
		for (int i = 0; i < _backgroundObjectsSize; i++)
			free(_backgroundObjects[i]);
		free(_backgroundObjects);
		_backgroundObjects = nullptr;
	}

	if (_cmdList) {
		for (int i = 0; i < _cmdListSize; i++)
			free(_cmdList[i]);
		free(_cmdList);
		_cmdList = nullptr;
	}
}

void Parser::switchTurbo() {
	_vm->_config._turboFl = !_vm->_config._turboFl;

#ifdef USE_TTS
	if (_vm->_config._turboFl) {
		_vm->sayText("T");
	}
#endif
}

/**
 * Add any new chars to line buffer and display them.
 * If CR pressed, pass line to LineHandler()
 */
void Parser::charHandler() {
	debugC(4, kDebugParser, "charHandler");

	Status &gameStatus = _vm->getGameStatus();

	// Check for one or more characters in ring buffer
	while (_getIndex != _putIndex) {
		char c = _ringBuffer[_getIndex++];
		if (_getIndex >= sizeof(_ringBuffer))
			_getIndex = 0;

		switch (c) {
		case Common::KEYCODE_BACKSPACE:             // Rubout key
			if (_cmdLineIndex)
				_cmdLine[--_cmdLineIndex] = '\0';
			break;
		case Common::KEYCODE_RETURN:                // EOL, pass line to line handler
			if (_cmdLineIndex && (_vm->_hero->_pathType != kPathQuiet)) {
				// Remove inventory bar if active
				if (_vm->_inventory->getInventoryState() == kInventoryActive)
					_vm->_inventory->setInventoryState(kInventoryUp);
#ifdef USE_TTS
				_vm->sayText(_cmdLine);
				_vm->_previousSaid.clear();
#endif
				// Call Line handler and reset line
				command(_cmdLine);
				_cmdLine[_cmdLineIndex = 0] = '\0';
			}
			break;
		default:                                    // Normal text key, add to line
			if (_cmdLineIndex >= kMaxLineSize) {
				//MessageBeep(MB_ICONASTERISK);
				warning("STUB: MessageBeep() - Command line too long");
			} else if (Common::isPrint(c)) {
				_cmdLine[_cmdLineIndex++] = c;
				_cmdLine[_cmdLineIndex] = '\0';
			}
			break;
		}
	}

	// See if time to blink cursor, set cursor character
	if (_vm->useWindowsInterface()) {
		if ((_cmdLineTick++ % (_vm->getTPS() / kBlinksPerSec)) == 0)
			_cmdLineCursor = (_cmdLineCursor == '_') ? ' ' : '_';
	} else {
		// DOS: No blinking cursor
		_cmdLineCursor = ' ';
	}

	// See if recall button pressed
	if (gameStatus._recallFl) {
		// Copy previous line to current cmdline
		gameStatus._recallFl = false;
		Common::strcpy_s(_cmdLine, _vm->_line);
		_cmdLineIndex = strlen(_cmdLine);
	}

	_vm->_screen->updateStatusText();
	_vm->_screen->updatePromptText(_cmdLine, _cmdLineCursor);

#ifdef USE_TTS
	if (_vm->_previousScore != _vm->getScore()) {
		_vm->sayText(Common::String::format("Score: %d of %d", _vm->getScore(), _vm->getMaxScore()));
		_vm->_previousScore = _vm->getScore();
	}

	if (_vm->_voiceScoreLine) {
		_vm->sayText(Common::String::format("F1: Help\n%s\nScore: %d of %d\nSound %s", (_vm->_config._turboFl) ? "T" : " ", _vm->getScore(), _vm->getMaxScore(), (_vm->_config._soundFl) ? "On" : "Off"));
		_vm->_voiceScoreLine = false;
	}

	if (_vm->_voiceSoundSetting) {
		_vm->sayText(Common::String::format("Sound %s", (_vm->_config._soundFl) ? "On" : "Off"));

		// If the mouse is in the top menu range, the top menu will close and reopen after the sound setting is changed,
		// causing the new sound setting voicing to be interrupted. Therefore, keep trying to voice the new sound setting
		// as long as the top menu can be opened
		int mouseY = _vm->_mouse->getMouseY();
		if (mouseY <= 0 || mouseY >= 5) {
			_vm->_voiceSoundSetting = false;
		}
	}
#endif

	// See if "look" button pressed
	if (gameStatus._lookFl) {
		command("look around");
		gameStatus._lookFl = false;
	}
}

void Parser::keyHandler(Common::Event event) {
	debugC(1, kDebugParser, "keyHandler(%d)", event.kbd.keycode);

	Status &gameStatus = _vm->getGameStatus();

	if (event.kbd.flags & (Common::KBD_ALT | Common::KBD_SCRL))
		return;

	// Process key down input - called from OnKeyDown()
	if (!gameStatus._storyModeFl) {              // Keyboard disabled
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

void Parser::actionHandler(Common::Event event) {
	debugC(1, kDebugParser, "ActionHandler(%d)", event.customType);

	Status &gameStatus = _vm->getGameStatus();

	switch (event.customType) {
	case kActionUserHelp:
		if (_vm->useWindowsInterface()) {
			// Windows: Track double-F1 with a flag
			if (_checkDoubleF1Fl)
				gameStatus._helpFl = true;
			else
				_vm->_screen->userHelp();
			_checkDoubleF1Fl = !_checkDoubleF1Fl;
		} else {
			// DOS: userHelp() handles double-F1
			_vm->_screen->userHelp();
		}
		break;
	case kActionToggleSound:
		_vm->_sound->toggleSound();
		_vm->_sound->toggleMusic();
		break;
	case kActionRepeatLine:
		gameStatus._recallFl = true;
		break;
	case kActionSaveGame:
		if (gameStatus._viewState == kViewPlay) {
			if (gameStatus._gameOverFl)
				_vm->gameOverMsg();
			else
				_vm->_file->saveGame(-1, Common::String());
		}
		break;
	case kActionRestoreGame:
		_vm->_file->restoreGame(-1);
		break;
	case kActionNewGame: {
		// DOS requires shorter text for message boxes
		const char *message = _vm->useWindowsInterface() ?
			                  "Are you sure you want to start a new game?" :
		                      "Are you sure you want to RESTART?";
		if (_vm->yesNoBox(message, true))
			_vm->_file->restoreGame(99);
		break;
	}
	case kActionInventory:
		showInventory();
		break;
	case kActionToggleTurbo:
		switchTurbo();
		break;
	case kActionEscape: // Escape key, may want to QUIT
		if (gameStatus._viewState == kViewIntro) {
			gameStatus._skipIntroFl = true;
		} else if (gameStatus._viewState == kViewPlay) {
			endGamePrompt();
		} else {
			if (_vm->_inventory->getInventoryState() == kInventoryActive) // Remove inventory, if displayed
				_vm->_inventory->setInventoryState(kInventoryUp);
			_vm->_screen->resetInventoryObjId();
		}
		break;
	case kActionMoveTop:
		_vm->_route->resetRoute();                   // Stop any automatic route
		_vm->_route->setWalk(Common::KEYCODE_UP);    // Direction of hero travel
		break;
	case kActionMoveBottom:
		_vm->_route->resetRoute();                   // Stop any automatic route
		_vm->_route->setWalk(Common::KEYCODE_DOWN);  // Direction of hero travel
		break;
	case kActionMoveLeft:
		_vm->_route->resetRoute();                   // Stop any automatic route
		_vm->_route->setWalk(Common::KEYCODE_LEFT);  // Direction of hero travel
		break;
	case kActionMoveRight:
		_vm->_route->resetRoute();                   // Stop any automatic route
		_vm->_route->setWalk(Common::KEYCODE_RIGHT); // Direction of hero travel
		break;
	case kActionMoveTopLeft:
		_vm->_route->resetRoute();                  // Stop any automatic route
		_vm->_route->setWalk(Common::KEYCODE_KP7);  // Direction of hero travel
		break;
	case kActionMoveTopRight:
		_vm->_route->resetRoute();                  // Stop any automatic route
		_vm->_route->setWalk(Common::KEYCODE_KP9);  // Direction of hero travel
		break;
	case kActionMoveBottomLeft:
		_vm->_route->resetRoute();                  // Stop any automatic route
		_vm->_route->setWalk(Common::KEYCODE_KP1);  // Direction of hero travel
		break;
	case kActionMoveBottomRight:
		_vm->_route->resetRoute();                  // Stop any automatic route
		_vm->_route->setWalk(Common::KEYCODE_KP3);  // Direction of hero travel
		break;
	default:
		break;
	}

	if (_checkDoubleF1Fl && (event.customType != kActionUserHelp))
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
	Common::vsprintf_s(_vm->_line, format, marker);
	va_end(marker);

	lineHandler();
}

void Parser::resetCommandLine() {
	_cmdLine[_cmdLineIndex = 0] = '\0';
}

/**
 * Locate any member of object name list appearing in command line
 */
bool Parser::isWordPresent(char **wordArr) const {
	if (wordArr != nullptr) {
		debugC(1, kDebugParser, "isWordPresent(%s)", wordArr[0]);

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
const char *Parser::findNoun() const {
	debugC(1, kDebugParser, "findNoun()");

	for (int i = 0; _vm->_text->getNounArray(i); i++) {
		for (int j = 0; strlen(_vm->_text->getNoun(i, j)); j++) {
			if (strstr(_vm->_line, _vm->_text->getNoun(i, j)))
				return _vm->_text->getNoun(i, 0);
		}
	}
	return nullptr;
}

/**
 * Locate word in list of verbs and return ptr to first string in verb list
 */
const char *Parser::findVerb() const {
	debugC(1, kDebugParser, "findVerb()");

	for (int i = 0; _vm->_text->getVerbArray(i); i++) {
		for (int j = 0; strlen(_vm->_text->getVerb(i, j)); j++) {
			if (strstr(_vm->_line, _vm->_text->getVerb(i, j)))
				return _vm->_text->getVerb(i, 0);
		}
	}
	return nullptr;
}

/**
 * Show user all objects being carried in a variable width 2 column format
 */
void Parser::showDosInventory() const {
	debugC(1, kDebugParser, "showDosInventory()");
	static const char *const blanks = "                                        ";
	uint16 index = 0, len1 = 0, len2 = 0;
	const char *intro = _vm->_text->getTextParser(kTBIntro);
	const char *outro = _vm->_text->getTextParser(kTBOutro);
	if (_vm->getGameType() == kGameTypeHugo3) {
		outro = "\nPress any key to continue";
	}
	const int nounIndex2 = (_vm->getGameType() <= kGameTypeHugo2) ? 0 : 1;

	for (int i = 0; i < _vm->_object->_numObj; i++) { // Find widths of 2 columns
		if (_vm->_object->isCarried(i)) {
			uint16 len = strlen(_vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, nounIndex2));
			if (index++ & 1)                        // Right hand column
				len2 = (len > len2) ? len : len2;
			else
				len1 = (len > len1) ? len : len1;
		}
	}

	Common::String buffer;
	if (_vm->getGameType() <= kGameTypeHugo2) {
		len1 += 1;                                  // For gap between columns
		if (len1 + len2 < (uint16)strlen(outro)) {
			len1 = strlen(outro);
		}
		assert(len1 + len2 - strlen(intro) / 2 < strlen(blanks));
		buffer = Common::String(blanks, (len1 + len2 - strlen(intro)) / 2);
	} else {
		len1 += 4;                                  // For gap between columns
		if (len1 + len2 > (uint16)strlen(intro)) {
			assert(len1 + len2 - strlen(intro) / 2 < strlen(blanks));
			buffer = Common::String(blanks, (len1 + len2 - strlen(intro)) / 2);
		}
	}

	buffer += intro;
	buffer += '\n';
	index = 0;
	for (int i = 0; i < _vm->_object->_numObj; i++) { // Assign strings
		if (_vm->_object->isCarried(i)) {
			const char *objectName = _vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, nounIndex2);
			buffer += objectName;
			if (index++ & 1) {
				buffer += '\n';
			} else {
				buffer += Common::String(blanks, len1 - strlen(objectName));
			}
		}
	}
	if (index & 1)
		buffer += '\n';
	buffer += outro;

#ifdef USE_TTS
	sayInventory(intro, outro, nounIndex2);
#endif

	_vm->notifyBox(buffer, false, kTtsNoSpeech);
}

#ifdef USE_TTS
void Parser::sayInventory(const char *intro, const char *outro, int nounIndex2) const {
	Common::String text = intro;
	for (int i = 0; i < _vm->_object->_numObj; i++) {
		if (_vm->_object->isCarried(i)) {
			text += '\n';
			text += _vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, nounIndex2);
		}
	}
	text += '\n';
	text += outro;
	_vm->sayText(text, Common::TextToSpeechManager::INTERRUPT);
}
#endif

void Parser::endGamePrompt() {
	if (_vm->yesNoBox(_vm->_text->getTextParser(kTBExit_1d), true)) {
		_vm->endGame();
	}
}

} // End of namespace Hugo
