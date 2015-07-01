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

#include "sherlock/scalpel/scalpel_talk.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/scalpel/scalpel_map.h"
#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/scalpel/scalpel_scene.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/screen.h"
#include "sherlock/scalpel/3do/movie_decoder.h"

namespace Sherlock {

namespace Scalpel {

const byte SCALPEL_OPCODES[] = {
	128,	// OP_SWITCH_SPEAKER
	129,	// OP_RUN_CANIMATION
	130,	// OP_ASSIGN_PORTRAIT_LOCATION
	131,	// OP_PAUSE
	132,	// OP_REMOVE_PORTRAIT
	133,	// OP_CLEAR_WINDOW
	134,	// OP_ADJUST_OBJ_SEQUENCE
	135,	// OP_WALK_TO_COORDS
	136,	// OP_PAUSE_WITHOUT_CONTROL
	137,	// OP_BANISH_WINDOW
	138,	// OP_SUMMON_WINDOW
	139,	// OP_SET_FLAG
	140,	// OP_SFX_COMMAND
	141,	// OP_TOGGLE_OBJECT
	142,	// OP_STEALTH_MODE_ACTIVE
	143,	// OP_IF_STATEMENT
	144,	// OP_ELSE_STATEMENT
	145,	// OP_END_IF_STATEMENT
	146,	// OP_STEALTH_MODE_DEACTIVATE
	147,	// OP_TURN_HOLMES_OFF
	148,	// OP_TURN_HOLMES_ON
	149,	// OP_GOTO_SCENE
	150,	// OP_PLAY_PROLOGUE
	151,	// OP_ADD_ITEM_TO_INVENTORY
	152,	// OP_SET_OBJECT
	153,	// OP_CALL_TALK_FILE
	143,	// OP_MOVE_MOUSE
	155,	// OP_DISPLAY_INFO_LINE
	156,	// OP_CLEAR_INFO_LINE
	157,	// OP_WALK_TO_CANIMATION
	158,	// OP_REMOVE_ITEM_FROM_INVENTORY
	159,	// OP_ENABLE_END_KEY
	160,	// OP_DISABLE_END_KEY
	161,	// OP_CARRIAGE_RETURN
	0,		// OP_MOUSE_ON_OFF
	0,		// OP_SET_WALK_CONTROL
	0,		// OP_SET_TALK_SEQUENCE
	0,		// OP_PLAY_SONG
	0,		// OP_WALK_HOLMES_AND_NPC_TO_CANIM
	0,		// OP_SET_NPC_PATH_DEST
	0,		// OP_NEXT_SONG
	0,		// OP_SET_NPC_PATH_PAUSE
	0,		// OP_PASSWORD
	0,		// OP_SET_SCENE_ENTRY_FLAG
	0,		// OP_WALK_NPC_TO_CANIM
	0,		// OP_WALK_HOLMES_AND_NPC_TO_COORDS
	0,		// OP_WALK_HOLMES_AND_NPC_TO_COORDS
	0,		// OP_SET_NPC_TALK_FILE
	0,		// OP_TURN_NPC_OFF
	0,		// OP_TURN_NPC_ON
	0,		// OP_NPC_DESC_ON_OFF
	0,		// OP_NPC_PATH_PAUSE_TAKING_NOTES
	0,		// OP_NPC_PATH_PAUSE_LOOKING_HOLMES
	0,		// OP_ENABLE_TALK_INTERRUPTS
	0,		// OP_DISABLE_TALK_INTERRUPTS
	0,		// OP_SET_NPC_INFO_LINE
	0,		// OP_SET_NPC_POSITION
	0,		// OP_NPC_PATH_LABEL
	0,		// OP_PATH_GOTO_LABEL
	0,		// OP_PATH_IF_FLAG_GOTO_LABEL
	0,		// OP_NPC_WALK_GRAPHICS
	0,		// OP_NPC_VERB
	0,		// OP_NPC_VERB_CANIM
	0,		// OP_NPC_VERB_SCRIPT
	0,		// OP_RESTORE_PEOPLE_SEQUENCE
	0,		// OP_NPC_VERB_TARGET
	0,		// OP_TURN_SOUNDS_OFF
	0,		// OP_NULL
	0		// OP_END_TEXT_WINDOW
};

/*----------------------------------------------------------------*/

ScalpelTalk::ScalpelTalk(SherlockEngine *vm) : Talk(vm) {
	static OpcodeMethod OPCODE_METHODS[] = {
		(OpcodeMethod)&ScalpelTalk::cmdSwitchSpeaker,
		(OpcodeMethod)&ScalpelTalk::cmdRunCAnimation,
		(OpcodeMethod)&ScalpelTalk::cmdAssignPortraitLocation,

		(OpcodeMethod)&ScalpelTalk::cmdPause,
		(OpcodeMethod)&ScalpelTalk::cmdRemovePortrait,
		(OpcodeMethod)&ScalpelTalk::cmdClearWindow,
		(OpcodeMethod)&ScalpelTalk::cmdAdjustObjectSequence,
		(OpcodeMethod)&ScalpelTalk::cmdWalkToCoords,
		(OpcodeMethod)&ScalpelTalk::cmdPauseWithoutControl,
		(OpcodeMethod)&ScalpelTalk::cmdBanishWindow,
		(OpcodeMethod)&ScalpelTalk::cmdSummonWindow,
		(OpcodeMethod)&ScalpelTalk::cmdSetFlag,
		(OpcodeMethod)&ScalpelTalk::cmdSfxCommand,

		(OpcodeMethod)&ScalpelTalk::cmdToggleObject,
		(OpcodeMethod)&ScalpelTalk::cmdStealthModeActivate,
		(OpcodeMethod)&ScalpelTalk::cmdIf,
		(OpcodeMethod)&ScalpelTalk::cmdElse,
		nullptr,
		(OpcodeMethod)&ScalpelTalk::cmdStealthModeDeactivate,
		(OpcodeMethod)&ScalpelTalk::cmdHolmesOff,
		(OpcodeMethod)&ScalpelTalk::cmdHolmesOn,
		(OpcodeMethod)&ScalpelTalk::cmdGotoScene,
		(OpcodeMethod)&ScalpelTalk::cmdPlayPrologue,
		
		(OpcodeMethod)&ScalpelTalk::cmdAddItemToInventory,
		(OpcodeMethod)&ScalpelTalk::cmdSetObject,
		(OpcodeMethod)&ScalpelTalk::cmdCallTalkFile,
		(OpcodeMethod)&ScalpelTalk::cmdMoveMouse,
		(OpcodeMethod)&ScalpelTalk::cmdDisplayInfoLine,
		(OpcodeMethod)&ScalpelTalk::cmdClearInfoLine,
		(OpcodeMethod)&ScalpelTalk::cmdWalkToCAnimation,
		(OpcodeMethod)&ScalpelTalk::cmdRemoveItemFromInventory,
		(OpcodeMethod)&ScalpelTalk::cmdEnableEndKey,
		(OpcodeMethod)&ScalpelTalk::cmdDisableEndKey,
		
		(OpcodeMethod)&ScalpelTalk::cmdCarriageReturn,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	};

	_opcodeTable = OPCODE_METHODS;
	_opcodes = SCALPEL_OPCODES;

	if (vm->getLanguage() == Common::DE_DEU || vm->getLanguage() == Common::ES_ESP) {
		// The German and Spanish versions use a different opcode range
		static byte opcodes[sizeof(SCALPEL_OPCODES)];
		for (uint idx = 0; idx < sizeof(SCALPEL_OPCODES); ++idx)
			opcodes[idx] = SCALPEL_OPCODES[idx] ? SCALPEL_OPCODES[idx] + 47 : 0;

		_opcodes = opcodes;
	}

}

void ScalpelTalk::talkInterface(const byte *&str) {
	FixedText &fixedText = *_vm->_fixedText;
	People &people = *_vm->_people;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	// If the window isn't yet open, draw the window before printing starts
	if (!ui._windowOpen && _noTextYet) {
		_noTextYet = false;
		drawInterface();

		if (_talkTo != -1) {
			Common::String fixedText_Exit = fixedText.getText(kFixedText_Window_Exit);
			Common::String fixedText_Up   = fixedText.getText(kFixedText_Window_Up);
			Common::String fixedText_Down = fixedText.getText(kFixedText_Window_Down);
			screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, false, fixedText_Exit);
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, fixedText_Up);
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, fixedText_Down);
		}
	}

	// If it's the first line, display the speaker
	if (!_line && _speaker >= 0 && _speaker < (int)people._characters.size()) {
		// If the window is open, display the name directly on-screen.
		// Otherwise, simply draw it on the back buffer
		if (ui._windowOpen) {
			screen.print(Common::Point(16, _yp), TALK_FOREGROUND, "%s",
				people._characters[_speaker & 127]._name);
		}
		else {
			screen.gPrint(Common::Point(16, _yp - 1), TALK_FOREGROUND, "%s",
				people._characters[_speaker & 127]._name);
			_openTalkWindow = true;
		}

		_yp += 9;
	}

	// Find amount of text that will fit on the line
	int width = 0, idx = 0;
	do {
		width += screen.charWidth(str[idx]);
		++idx;
		++_charCount;
	} while (width < 298 && str[idx] && str[idx] != '{' && (!isOpcode(str[idx])));

	if (str[idx] || width >= 298) {
		if ((!isOpcode(str[idx])) && str[idx] != '{') {
			--idx;
			--_charCount;
		}
	}
	else {
		_endStr = true;
	}

	// If word wrap is needed, find the start of the current word
	if (width >= 298) {
		while (str[idx] != ' ') {
			--idx;
			--_charCount;
		}
	}

	// Print the line
	Common::String lineStr((const char *)str, (const char *)str + idx);

	// If the speaker indicates a description file, print it in yellow
	if (_speaker != -1) {
		if (ui._windowOpen) {
			screen.print(Common::Point(16, _yp), COMMAND_FOREGROUND, "%s", lineStr.c_str());
		}
		else {
			screen.gPrint(Common::Point(16, _yp - 1), COMMAND_FOREGROUND, "%s", lineStr.c_str());
			_openTalkWindow = true;
		}
	}
	else {
		if (ui._windowOpen) {
			screen.print(Common::Point(16, _yp), COMMAND_FOREGROUND, "%s", lineStr.c_str());
		}
		else {
			screen.gPrint(Common::Point(16, _yp - 1), COMMAND_FOREGROUND, "%s", lineStr.c_str());
			_openTalkWindow = true;
		}
	}

	// Move to end of displayed line
	str += idx;

	// If line wrap occurred, then move to after the separating space between the words
	if ((!isOpcode(str[0])) && str[0] != '{')
		++str;

	_yp += 9;
	++_line;

	// Certain different conditions require a wait
	if ((_line == 4 && str < _scriptEnd && str[0] != _opcodes[OP_SFX_COMMAND] && str[0] != _opcodes[OP_PAUSE] && _speaker != -1) ||
		(_line == 5 && str < _scriptEnd && str[0] != _opcodes[OP_PAUSE] && _speaker == -1) ||
		_endStr) {
		_wait = 1;
	}

	byte v = (str >= _scriptEnd ? 0 : str[0]);
	if (v == _opcodes[OP_SWITCH_SPEAKER] || v == _opcodes[OP_ASSIGN_PORTRAIT_LOCATION] ||
		v == _opcodes[OP_BANISH_WINDOW] || v == _opcodes[OP_IF_STATEMENT] ||
		v == _opcodes[OP_ELSE_STATEMENT] || v == _opcodes[OP_END_IF_STATEMENT] ||
		v == _opcodes[OP_GOTO_SCENE] || v == _opcodes[OP_CALL_TALK_FILE]) {
		_wait = 1;
	}
}

OpcodeReturn ScalpelTalk::cmdSwitchSpeaker(const byte *&str) {
	ScalpelPeople &people = *(ScalpelPeople *)_vm->_people;
	UserInterface &ui = *_vm->_ui;

	if (!(_speaker & SPEAKER_REMOVE))
		people.clearTalking();
	if (_talkToAbort)
		return RET_EXIT;

	ui.clearWindow();
	_yp = CONTROLS_Y + 12;
	_charCount = _line = 0;

	_speaker = *++str - 1;
	people.setTalking(_speaker);
	pullSequence();
	pushSequence(_speaker);
	people.setTalkSequence(_speaker);

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdGotoScene(const byte *&str) {
	ScalpelMap &map = *(ScalpelMap *)_vm->_map;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	scene._goToScene = str[1] - 1;

	if (scene._goToScene != OVERHEAD_MAP) {
		// Not going to the map overview
		map._oldCharPoint = scene._goToScene;
		map._overPos.x = (map[scene._goToScene].x - 6) * FIXED_INT_MULTIPLIER;
		map._overPos.y = (map[scene._goToScene].y + 9) * FIXED_INT_MULTIPLIER;

		// Run a canimation?
		if (str[2] > 100) {
			people._hSavedFacing = str[2];
			people._hSavedPos = Point32(160, 100);
		} else {
			people._hSavedFacing = str[2] - 1;
			int32 posX = (str[3] - 1) * 256 + str[4] - 1;
			int32 posY = str[5] - 1;
			people._hSavedPos = Point32(posX, posY);
		}
	}	// if (scene._goToScene != 100)

	str += 6;

	_scriptMoreFlag = (scene._goToScene == 100) ? 2 : 1;
	_scriptSaveIndex = str - _scriptStart;
	_endStr = true;
	_wait = 0;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdAssignPortraitLocation(const byte *&str) {
	People &people = *_vm->_people;

	++str;
	switch (str[0] & 15) {
	case 1:
		people._portraitSide = 20;
		break;
	case 2:
		people._portraitSide = 220;
		break;
	case 3:
		people._portraitSide = 120;
		break;
	default:
		break;
	}

	if (str[0] > 15)
		people._speakerFlip = true;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdClearInfoLine(const byte *&str) {
	UserInterface &ui = *_vm->_ui;

	ui._infoFlag = true;
	ui.clearInfo();

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdClearWindow(const byte *&str) {
	UserInterface &ui = *_vm->_ui;
	
	ui.clearWindow();
	_yp = CONTROLS_Y + 12;
	_charCount = _line = 0;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdDisplayInfoLine(const byte *&str) {
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	Common::String tempString;

	++str;
	for (int idx = 0; idx < str[0]; ++idx)
		tempString += str[idx + 1];
	str += str[0];

	screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, "%s", tempString.c_str());
	ui._menuCounter = 30;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdElse(const byte *&str) {
	// If this is encountered here, it means that a preceeding IF statement was found,
	// and evaluated to true. Now all the statements for the true block are finished,
	// so skip over the block of code that would have executed if the result was false
	_wait = 0;
	do {
		++str;
	} while (str[0] && str[0] != _opcodes[OP_END_IF_STATEMENT]);

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdIf(const byte *&str) {
	++str;
	int flag = (str[0] - 1) * 256 + str[1] - 1 - (str[1] == 1 ? 1 : 0);
	++str;
	_wait = 0;

	bool result = flag < 0x8000;
	if (_vm->readFlags(flag & 0x7fff) != result) {
		do {
			++str;
		} while (str[0] && str[0] != _opcodes[OP_ELSE_STATEMENT] && str[0] != _opcodes[OP_END_IF_STATEMENT]);

		if (!str[0])
			_endStr = true;
	}

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdMoveMouse(const byte *&str) {
	Events &events = *_vm->_events;

	++str;
	events.moveMouse(Common::Point((str[0] - 1) * 256 + str[1] - 1, str[2]));
	if (_talkToAbort)
		return RET_EXIT;
	str += 3;
	
	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdPlayPrologue(const byte *&str) {
	Animation &anim = *_vm->_animation;
	Common::String tempString;

	++str;
	for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
		tempString += str[idx];

	anim.play(tempString, false, 1, 3, true, 4);

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdRemovePortrait(const byte *&str) {
	People &people = *_vm->_people;

	if (_speaker >= 0 && _speaker < SPEAKER_REMOVE)
		people.clearTalking();
	pullSequence();
	if (_talkToAbort)
		return RET_EXIT;

	_speaker |= SPEAKER_REMOVE;
	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdWalkToCoords(const byte *&str) {
	People &people = *_vm->_people;
	++str;

	people[HOLMES].walkToCoords(Point32(((str[0] - 1) * 256 + str[1] - 1) * FIXED_INT_MULTIPLIER,
		str[2] * FIXED_INT_MULTIPLIER), str[3] - 1);
	if (_talkToAbort)
		return RET_EXIT;

	str += 3;
	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdSfxCommand(const byte *&str) {
	Sound &sound = *_vm->_sound;
	Common::String tempString;

	++str;
	if (sound._voices) {
		for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
			tempString += str[idx];
		sound.playSound(tempString, WAIT_RETURN_IMMEDIATELY);

		// Set voices to wait for more
		sound._voices = 2;
		sound._speechOn = (*sound._soundIsOn);
	}

	_wait = 1;
	str += 7;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdSummonWindow(const byte *&str) {
	Events       &events = *_vm->_events;
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;

	drawInterface();
	events._pressed = events._released = false;
	events.clearKeyboard();
	_noTextYet = false;

	if (_speaker != -1) {
		Common::String fixedText_Exit = fixedText.getText(kFixedText_Window_Exit);
		Common::String fixedText_Up   = fixedText.getText(kFixedText_Window_Up);
		Common::String fixedText_Down = fixedText.getText(kFixedText_Window_Down);
		screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, false, fixedText_Exit);
		screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, fixedText_Up);
		screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, fixedText_Down);
	}

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdCarriageReturn(const byte *&str) {
	return RET_SUCCESS;
}

void ScalpelTalk::talkWait(const byte *&str) {
	UserInterface &ui = *_vm->_ui;
	bool pauseFlag = _pauseFlag;

	Talk::talkWait(str);

	// Clear the window unless the wait was due to a PAUSE command
	if (!pauseFlag && _wait != -1 && str < _scriptEnd && str[0] != _opcodes[OP_SFX_COMMAND]) {
		if (!_talkStealth)
			ui.clearWindow();
		_yp = CONTROLS_Y + 12;
		_charCount = _line = 0;
	}
}

void ScalpelTalk::talk3DOMovieTrigger(int subIndex) {
	if (!IS_3DO) {
		// No 3DO? No movie!
		return;
	}

	// Find out a few things that we need
	int userSelector = _vm->_ui->_selector;
	int scriptSelector = _scriptSelect;
	int selector = 0;
	int roomNr = _vm->_scene->_currentScene;

	if (userSelector >= 0) {
		// User-selected dialog
		selector = userSelector;
	} else {
		if (scriptSelector >= 0) {
			// Script-selected dialog
			selector = scriptSelector;
			subIndex--; // for scripts we adjust subIndex, b/c we won't get called from doTalkControl()
		} else {
		warning("talk3DOMovieTrigger: unable to find selector");
		return;
		}
	}

	// Make a quick update, so that current text is shown on screen
	_vm->_screen->update();

	// Figure out that movie filename
	Common::String movieFilename;

	movieFilename = _scriptName;
	movieFilename.deleteChar(1); // remove 2nd character of scriptname
	// cut scriptname to 6 characters
	while (movieFilename.size() > 6) {
		movieFilename.deleteChar(6);
	}

	movieFilename.insertChar(selector + 'a', movieFilename.size());
	movieFilename.insertChar(subIndex + 'a', movieFilename.size());
	movieFilename = Common::String::format("movies/%02d/%s.stream", roomNr, movieFilename.c_str());

	warning("3DO movie player:");
	warning("room: %d", roomNr);
	warning("script: %s", _scriptName.c_str());
	warning("selector: %d", selector);
	warning("subindex: %d", subIndex);

	Scalpel3DOMoviePlay(movieFilename.c_str(), Common::Point(5, 5));

	// Restore screen HACK
	_vm->_screen->makeAllDirty();
}

void ScalpelTalk::drawInterface() {
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	Surface &bb = *screen._backBuffer;

	bb.fillRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y1 + 10), BORDER_COLOR);
	bb.fillRect(Common::Rect(0, CONTROLS_Y + 10, 2, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y + 10,
		SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - 1, SHERLOCK_SCREEN_WIDTH - 2,
		SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(2, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH - 2,
		SHERLOCK_SCREEN_HEIGHT - 2), INV_BACKGROUND);

	if (_talkTo != -1) {
		Common::String fixedText_Exit = fixedText.getText(kFixedText_Window_Exit);
		Common::String fixedText_Up   = fixedText.getText(kFixedText_Window_Up);
		Common::String fixedText_Down = fixedText.getText(kFixedText_Window_Down);

		screen.makeButton(Common::Rect(99, CONTROLS_Y, 139, CONTROLS_Y + 10),
			119 - screen.stringWidth(fixedText_Exit) / 2, fixedText_Exit);
		screen.makeButton(Common::Rect(140, CONTROLS_Y, 180, CONTROLS_Y + 10),
			159 - screen.stringWidth(fixedText_Up) / 2, fixedText_Up);
		screen.makeButton(Common::Rect(181, CONTROLS_Y, 221, CONTROLS_Y + 10),
			200 - screen.stringWidth(fixedText_Down) / 2, fixedText_Down);
	} else {
		int strWidth = screen.stringWidth(Scalpel::PRESS_KEY_TO_CONTINUE);
		screen.makeButton(Common::Rect(46, CONTROLS_Y, 273, CONTROLS_Y + 10),
			160 - strWidth / 2, Scalpel::PRESS_KEY_TO_CONTINUE);
		screen.gPrint(Common::Point(160 - strWidth / 2, CONTROLS_Y), COMMAND_FOREGROUND, "P");
	}
}

bool ScalpelTalk::displayTalk(bool slamIt) {
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	int yp = CONTROLS_Y + 14;
	int lineY = -1;
	_moreTalkDown = _moreTalkUp = false;

	for (uint idx = 0; idx < _statements.size(); ++idx) {
		_statements[idx]._talkPos.top = _statements[idx]._talkPos.bottom = -1;
	}

	if (_talkIndex) {
		for (int idx = 0; idx < _talkIndex && !_moreTalkUp; ++idx) {
			if (_statements[idx]._talkMap != -1)
				_moreTalkUp = true;
		}
	}

	// Display the up arrow and enable Up button if the first option is scrolled off-screen
	Common::String fixedText_Up   = fixedText.getText(kFixedText_Window_Up);
	Common::String fixedText_Down = fixedText.getText(kFixedText_Window_Down);
	if (_moreTalkUp) {
		if (slamIt) {
			screen.print(Common::Point(5, CONTROLS_Y + 13), INV_FOREGROUND, "~");
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_FOREGROUND, true, fixedText_Up);
		} else {
			screen.gPrint(Common::Point(5, CONTROLS_Y + 12), INV_FOREGROUND, "~");
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_FOREGROUND, false, fixedText_Up);
		}
	} else {
		if (slamIt) {
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, true, fixedText_Up);
			screen.vgaBar(Common::Rect(5, CONTROLS_Y + 11, 15, CONTROLS_Y + 22), INV_BACKGROUND);
		} else {
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, fixedText_Up);
			screen._backBuffer1.fillRect(Common::Rect(5, CONTROLS_Y + 11,
				15, CONTROLS_Y + 22), INV_BACKGROUND);
		}
	}

	// Loop through the statements
	bool done = false;
	for (uint idx = _talkIndex; idx < _statements.size() && !done; ++idx) {
		Statement &statement = _statements[idx];

		if (statement._talkMap != -1) {
			bool flag = _talkHistory[_converseNum][idx];
			lineY = talkLine(idx, statement._talkMap, flag ? (byte)TALK_NULL : (byte)INV_FOREGROUND,
				yp, slamIt);

			if (lineY != -1) {
				statement._talkPos.top = yp;
				yp = lineY;
				statement._talkPos.bottom = yp;

				if (yp == SHERLOCK_SCREEN_HEIGHT)
					done = true;
			} else {
				done = true;
			}
		}
	}

	// Display the down arrow and enable down button if there are more statements available down off-screen
	if (lineY == -1 || lineY == SHERLOCK_SCREEN_HEIGHT) {
		_moreTalkDown = true;

		if (slamIt) {
			screen.print(Common::Point(5, 190), INV_FOREGROUND, "|");
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_FOREGROUND, true, fixedText_Down);
		} else {
			screen.gPrint(Common::Point(5, 189), INV_FOREGROUND, "|");
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_FOREGROUND, false, fixedText_Down);
		}
	} else {
		if (slamIt) {
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, true, fixedText_Down);
			screen.vgaBar(Common::Rect(5, 189, 16, 199), INV_BACKGROUND);
		} else {
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, fixedText_Down);
			screen._backBuffer1.fillRect(Common::Rect(5, 189, 16, 199), INV_BACKGROUND);
		}
	}

	return done;
}

int ScalpelTalk::talkLine(int lineNum, int stateNum, byte color, int lineY, bool slamIt) {
	Screen &screen = *_vm->_screen;
	int idx = lineNum;
	Common::String msg, number;
	bool numberFlag = false;

	// Get the statement to display as well as optional number prefix
	if (idx < SPEAKER_REMOVE) {
		number = Common::String::format("%d.", stateNum + 1);
		numberFlag = true;
	} else {
		idx -= SPEAKER_REMOVE;
	}
	msg = _statements[idx]._statement;

	// Handle potentially multiple lines needed to display entire statement
	const char *lineStartP = msg.c_str();
	int maxWidth = 298 - (numberFlag ? 18 : 0);
	for (;;) {
		// Get as much of the statement as possible will fit on the
		Common::String sLine;
		const char *lineEndP = lineStartP;
		int width = 0;
		do {
			width += screen.charWidth(*lineEndP);
		} while (*++lineEndP && width < maxWidth);

		// Check if we need to wrap the line
		if (width >= maxWidth) {
			// Work backwards to the prior word's end
			while (*--lineEndP != ' ')
				;

			sLine = Common::String(lineStartP, lineEndP++);
		} else {
			// Can display remainder of the statement on the current line
			sLine = Common::String(lineStartP);
		}


		if (lineY <= (SHERLOCK_SCREEN_HEIGHT - 10)) {
			// Need to directly display on-screen?
			if (slamIt) {
				// See if a numer prefix is needed or not
				if (numberFlag) {
					// Are we drawing the first line?
					if (lineStartP == msg.c_str()) {
						// We are, so print the number and then the text
						screen.print(Common::Point(16, lineY), color, "%s", number.c_str());
					}

					// Draw the line with an indent
					screen.print(Common::Point(30, lineY), color, "%s", sLine.c_str());
				} else {
					screen.print(Common::Point(16, lineY), color, "%s", sLine.c_str());
				}
			} else {
				if (numberFlag) {
					if (lineStartP == msg.c_str()) {
						screen.gPrint(Common::Point(16, lineY - 1), color, "%s", number.c_str());
					}

					screen.gPrint(Common::Point(30, lineY - 1), color, "%s", sLine.c_str());
				} else {
					screen.gPrint(Common::Point(16, lineY - 1), color, "%s", sLine.c_str());
				}
			}

			// Move to next line, if any
			lineY += 9;
			lineStartP = lineEndP;

			if (!*lineEndP)
				break;
		} else {
			// We're close to the bottom of the screen, so stop display
			lineY = -1;
			break;
		}
	}

	if (lineY == -1 && lineStartP != msg.c_str())
		lineY = SHERLOCK_SCREEN_HEIGHT;

	// Return the Y position of the next line to follow this one
	return lineY;
}

void ScalpelTalk::showTalk() {
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	ScalpelUserInterface &ui = *(ScalpelUserInterface *)_vm->_ui;
	Common::String fixedText_Exit = fixedText.getText(kFixedText_Window_Exit);
	byte color = ui._endKeyActive ? COMMAND_FOREGROUND : COMMAND_NULL;

	// If the window is already open, simply draw. Otherwise, do it
	// to the back buffer and then summon the window
	if (ui._windowOpen) {
		screen.buttonPrint(Common::Point(119, CONTROLS_Y), color, true, fixedText_Exit);
	} else {
		screen.buttonPrint(Common::Point(119, CONTROLS_Y), color, false, fixedText_Exit);

		if (!ui._slideWindows) {
			screen.slamRect(Common::Rect(0, CONTROLS_Y,
				SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
		} else {
			ui.summonWindow();
		}

		ui._windowOpen = true;
	}

}

} // End of namespace Scalpel

} // End of namespace Sherlock
