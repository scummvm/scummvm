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

#include "sherlock/scalpel/scalpel_talk.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/scalpel/scalpel_journal.h"
#include "sherlock/scalpel/scalpel_map.h"
#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/scalpel/scalpel_scene.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/screen.h"
#include "video/3do_decoder.h"
#include "graphics/surface.h"
#include "sherlock/scalpel/scalpel_3do_audio_durations.h"

namespace Sherlock {
namespace Scalpel {

// Helper function to extract the base filename from a full path
// e.g. "Movies/03/lst03aec.stream" -> "lst03aec"
Common::String getBaseFilenameFromPath(const Common::String &fullPath) {
    size_t lastSlashPos = fullPath.findLastOf('/');
    Common::String filenameWithExt;

    if (lastSlashPos != Common::String::npos) {
        filenameWithExt = fullPath.substr(lastSlashPos + 1, Common::String::npos);
    } else {
        filenameWithExt = fullPath;
    }

    size_t dotPos = filenameWithExt.findLastOf('.');
    if (dotPos != Common::String::npos) {
        return filenameWithExt.substr(0, dotPos);
    }
    return filenameWithExt;
}

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
	161,	// OP_END_TEXT_WINDOW
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
	0		// OP_NULL
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

		(OpcodeMethod)&ScalpelTalk::cmdEndTextWindow,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
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

	_fixedTextWindowExit = FIXED(Window_Exit);
	_fixedTextWindowUp   = FIXED(Window_Up);
	_fixedTextWindowDown = FIXED(Window_Down);

	_3doAudioDecoder = nullptr;
}

ScalpelTalk::~ScalpelTalk() {
	stop3DOAudio();
}

void ScalpelTalk::stop3DOAudio() {
	if (_3doAudioDecoder) {
		_3doAudioDecoder->close();
		delete _3doAudioDecoder;
		_3doAudioDecoder = nullptr;
	}
}

bool ScalpelTalk::is3DOAudioPlaying() const {
	if (!_3doAudioDecoder) return false;

	// If video is finished, assume audio is also done (prevents infinite wait if audio finish isn't flagged)
	if (_3doAudioDecoder->endOfVideo()) return false;

	// Otherwise check actual audio status
	return !_3doAudioDecoder->isAudioTrackFinished();
}

void ScalpelTalk::talkTo(const Common::String &filename) {
	ScalpelUserInterface &ui = *(ScalpelUserInterface *)_vm->_ui;

	// Reset selector and subindex for new conversation
	// Only reset speech index if talking to a different script/character
	static Common::String lastScript;
	bool newScript = (lastScript != filename);

	if (newScript) {
		lastScript = filename;
	}

	// Reset 3DO audio tracking for new dialogue
	_pcTalkie3DOSelector = -1;
	_pcTalkie3DOSubindex = 0;
	_lastSpeaker = -1; // Reset last speaker tracking

	Talk::talkTo(filename);

	if (filename == "Tube59c") {
		// WORKAROUND: Original game bug causes the results of testing the powdery substance
		// to disappear too quickly. Introduce a delay to allow it to be properly displayed
		ui._menuCounter = 30;
	} else if (filename == "Lesl24z.tlk" || filename == "Beal40y.tlk") {
		// WORKAROUND: Walking to the flower girl or housekeeper the
		// first time triggers this automatic talk. This should abort
		// any other action, such as trying to look at her, else the UI
		// gets corrupted
		_talkToAbort = true;
	}

	// Stop any lingering 3DO audio when conversation ends
	stop3DOAudio();
}

void ScalpelTalk::talkInterface(const byte *&str) {
	People &people = *_vm->_people;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	int lineHeight = _vm->getLanguage() == Common::Language::ZH_TWN ? 16 : 9;
	int maxLines = _vm->getLanguage() == Common::Language::ZH_TWN ? 3 : 5;

	if (_vm->getLanguage() == Common::DE_DEU)
		skipBadText(str);

	// If the window isn't yet open, draw the window before printing starts
	if (!ui._windowOpen && _noTextYet) {
		_noTextYet = false;
		drawInterface();

		if (_talkTo != -1) {
			screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, false, _fixedTextWindowExit);
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, _fixedTextWindowUp);
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, _fixedTextWindowDown);
		}
	}

	// If it's the first line, display the speaker
	if (!_line && _speaker >= 0 && _speaker < (int)people._characters.size()) {
		// If the window is open, display the name directly on-screen.
		// Otherwise, simply draw it on the back buffer
		if (ui._windowOpen) {
			screen.print(Common::Point(16, _yp), TALK_FOREGROUND, "%s",
				people._characters[_speaker & 127]._name);
		} else {
			screen.gPrint(Common::Point(16, _yp - 1), TALK_FOREGROUND, "%s",
				people._characters[_speaker & 127]._name);
			_openTalkWindow = true;
		}

		_yp += lineHeight;
	}

	// Find amount of text that will fit on the line
	int width = 0, idx = 0, last_space = 0, last_valid = 0;
	do {
		int old_idx = idx;
		if (str[idx] == ' ')
			last_space = idx;
		last_valid = idx;
		width += screen.charWidth((const char *) str, idx);
		_charCount += idx - old_idx;
	} while (width < 298 && str[idx] && str[idx] != '{' && (!isOpcode(str[idx])));

	if (str[idx] || width >= 298) {
		if ((!isOpcode(str[idx])) && str[idx] != '{') {
			--idx;
			--_charCount;
		}
	} else {
		_endStr = true;
	}

	// If word wrap is needed, find the start of the current word
	if (width >= 298) {
		if (last_space > 0) {
			_charCount -= idx - last_space;
			idx = last_space;
		} else {
			_charCount -= idx - last_valid;
			idx = last_valid;
		}
	}

	// Print the line
	Common::String lineStr((const char *)str, (const char *)str + idx);

	// If the speaker indicates a description file, print it in yellow
	if (_speaker != -1) {
		if (ui._windowOpen) {
			screen.print(Common::Point(16, _yp), COMMAND_FOREGROUND, "%s", lineStr.c_str());
		} else {
			screen.gPrint(Common::Point(16, _yp - 1), COMMAND_FOREGROUND, "%s", lineStr.c_str());
			_openTalkWindow = true;
		}
	} else {
		if (ui._windowOpen) {
			screen.print(Common::Point(16, _yp), COMMAND_FOREGROUND, "%s", lineStr.c_str());
		} else {
			screen.gPrint(Common::Point(16, _yp - 1), COMMAND_FOREGROUND, "%s", lineStr.c_str());
			_openTalkWindow = true;
		}
	}

	// Move to end of displayed line
	str += idx;

	// If line wrap with space occurred, then move to after the separating space between the words
	if (str[0] == ' ')
		++str;

	_yp += lineHeight;
	++_line;

	// Certain different conditions require a wait
	if ((_line == (maxLines - 1) && str < _scriptEnd && str[0] != _opcodes[OP_SFX_COMMAND] && str[0] != _opcodes[OP_PAUSE] && _speaker != -1) ||
		(_line == maxLines && str < _scriptEnd && str[0] != _opcodes[OP_PAUSE] && _speaker == -1) ||
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
			people._savedPos = PositionFacing(160, 100, str[2]);
		} else {
			int32 posX = (str[3] - 1) * 256 + str[4] - 1;
			int32 posY = str[5] - 1;
			people._savedPos = PositionFacing(posX, posY, str[2] - 1);
		}
	}

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
	// If this is encountered here, it means that a preceding IF statement was found,
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
	events.warpMouse(Common::Point((str[0] - 1) * 256 + str[1] - 1, str[2]));
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

	anim.play(Common::Path(tempString), false, 1, 3, true, 4);

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
		sound.playSpeech(Common::Path(tempString));

		// Set voices to wait for more
		sound._voices = 2;
	}

	_wait = 1;
	str += 7;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdSummonWindow(const byte *&str) {
	Events       &events = *_vm->_events;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;

	drawInterface();
	events._pressed = events._released = false;
	events.clearKeyboard();
	_noTextYet = false;

	if (_speaker != -1) {
		screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, false, _fixedTextWindowExit);
		screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, _fixedTextWindowUp);
		screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, _fixedTextWindowDown);
	}

	return RET_SUCCESS;
}

void ScalpelTalk::loadTalkFile(const Common::String &filename) {
	Talk::loadTalkFile(filename);

	// Reset counters for the new file
	// fixes issues where counters would drift off limits when switching files
	_3doSpeechIndex = 0;
}

void ScalpelTalk::set3DODialogueSelection(int statementIndex) {
	// Set the selector to the actual statement index selected by the user
	_pcTalkie3DOSelector = statementIndex;
	_pcTalkie3DOSubindex = 0;
	debug(3, "ScalpelTalk::set3DODialogueSelection: Set selector to %d", statementIndex);
}

static int getOpcodeArgsLength(int opIndex, const byte *args) {
	switch (opIndex) {
	case OP_SWITCH_SPEAKER: return 2;
	case OP_RUN_CANIMATION: return 1;
	case OP_ASSIGN_PORTRAIT_LOCATION: return 1;
	case OP_PAUSE: return 1;
	case OP_REMOVE_PORTRAIT: return 0;
	case OP_CLEAR_WINDOW: return 0;
	case OP_ADJUST_OBJ_SEQUENCE: return 3 + (args[0] & 127);
	case OP_WALK_TO_COORDS: return 4;
	case OP_PAUSE_WITHOUT_CONTROL: return 1;
	case OP_BANISH_WINDOW: return 0;
	case OP_SUMMON_WINDOW: return 0;
	case OP_SET_FLAG: return 2;
	case OP_SFX_COMMAND: return 8;
	case OP_TOGGLE_OBJECT: return 1 + args[0];
	case OP_STEALTH_MODE_ACTIVE: return 0;
	case OP_IF_STATEMENT: return 2;
	case OP_ELSE_STATEMENT: return 0;
	case OP_END_IF_STATEMENT: return 0;
	case OP_STEALTH_MODE_DEACTIVATE: return 0;
	case OP_TURN_HOLMES_OFF: return 0;
	case OP_TURN_HOLMES_ON: return 0;
	case OP_GOTO_SCENE: return 7;
	case OP_PLAY_PROLOGUE: return 8;
	case OP_ADD_ITEM_TO_INVENTORY: return 1 + args[0];
	case OP_SET_OBJECT: return 1 + (args[0] & 127);
	case OP_CALL_TALK_FILE: return 9;
	case OP_MOVE_MOUSE: return 4;
	case OP_DISPLAY_INFO_LINE: return 1 + args[0];
	case OP_CLEAR_INFO_LINE: return 0;
	case OP_WALK_TO_CANIMATION: return 1;
	case OP_REMOVE_ITEM_FROM_INVENTORY: return 1 + args[0];
	case OP_ENABLE_END_KEY: return 0;
	case OP_DISABLE_END_KEY: return 0;
	case OP_END_TEXT_WINDOW: return 0;
	default: return 0;
	}
}

void ScalpelTalk::talkWait(const byte *&str) {
	UserInterface &ui = *_vm->_ui;
	bool pauseFlag = _pauseFlag;

	// Determine if there is more text for this speaker
	// This dictates whether we wait for Audio (Last Page) or Text (Mid Page)
	_waitForAudio = true; // Default to waiting for audio (Single Page)

	const byte *scan = str;
	while (scan < _scriptEnd) {
		byte c = *scan;
		if (c == 0) break;

		if (c >= _opcodes[0] && c < (_opcodes[0] + 100)) {
			int opIndex = c - _opcodes[0];
			if (opIndex == OP_SWITCH_SPEAKER || opIndex == OP_REMOVE_PORTRAIT) break;

			int skip = 0;
			if (scan + 1 < _scriptEnd) skip = getOpcodeArgsLength(opIndex, scan + 1);
			scan += 1 + skip;
		} else {
			if (c != '{' && c != '}') {
				// Found text! This means we are NOT on the last page (yet)
				// Or rather, 'str' points to the NEXT text chunk.
				// If 'str' has text, then the CURRENT chunk was NOT the last one?
				//
				// If 'str' has text, then we have more to show.
				// So Current Page is NOT Last.
				_waitForAudio = false;
				break;
			}
			scan++;
		}
	}

	int delay = _charCount * 7; // Heuristic delay
	if (!pauseFlag && delay < 80) delay = 80;

	_wait = waitForMore(delay);

	if (_wait == -1)
		_endStr = true;

	if (IS_SERRATED_SCALPEL && _wait >= 0 && _wait < 254) {
		if (str[0] == _opcodes[OP_SFX_COMMAND])
			str += 9;
	}

	_pauseFlag = false;

	// Handle Window Clearing (Page Flip)
	// Resets counters to prevent accumulation and overlap
	if (!pauseFlag && _wait != -1 && str < _scriptEnd && str[0] != _opcodes[OP_SFX_COMMAND]) {
		if (!_talkStealth)
			ui.clearWindow();
		_yp = CONTROLS_Y + 12;
		_charCount = _line = 0;
	}
}

void ScalpelTalk::nothingToSay() {
	error("Character had no talk options available");
}

void ScalpelTalk::switchSpeaker() {
}

int ScalpelTalk::handle3DONative() {
	Events &events = *_vm->_events;

	// Hide the cursor
	events.hideCursor();
	events.wait(1);

	switchSpeaker();

	// Play the video
	talk3DOMovieTrigger(_3doSpeechIndex++);

	// Adjust _talkStealth mode:
	// mode 1 - It was by a pause without stealth being on before the pause, so reset back to 0
	// mode 3 - It was set by a pause with stealth being on before the pause, to set it to active
	// mode 0/2 (Inactive/active) No change
	switch (_talkStealth) {
	case 1:
		_talkStealth = 0;
		break;
	case 2:
		_talkStealth = 2;
		break;
	default:
		break;
	}

	events.showCursor();
	events._pressed = events._released = false;

	return 254;
}

bool ScalpelTalk::handleTalkieMode() {
	ScalpelEngine &vm = *(ScalpelEngine *)_vm;

	// Speaker Change Detection
	// Only load new audio if the speaker has changed or we aren't playing anything
	// This handles multi-page dialogues where one audio file covers multiple text pages
	bool newSpeaker = (_speaker != _lastSpeaker);

	// Special case: If we are just starting a conversation or it was reset
	if (_lastSpeaker == -1) newSpeaker = true;

	if (newSpeaker) {
		// Stop previous audio before starting new turn
		stop3DOAudio();

		// Try to get video/audio file using CURRENT subindex (starts at 0)
		Common::String videoFile = get3DOVideoFile(_converseNum, -1);

		if (!videoFile.empty() && vm.has3DOVideo(videoFile)) {
			// Audio-only mode (Non-blocking start)
			debug(2, "ScalpelTalk::waitForMore: Starting 3DO audio: %s (Speaker %d)", videoFile.c_str(), _speaker);
			if (play3DOConversationAudio(videoFile)) {
				// Audio started

				// Use accurate duration from lookup table
				Common::String baseFilename = getBaseFilenameFromPath(videoFile);
				const Common::HashMap<Common::String, uint32> &durationsMap = get3doAudioDurations();
				auto it = durationsMap.find(baseFilename);

				if (it != durationsMap.end()) {
					_currentAudioDuration = it->_value;
					debug(2, "ScalpelTalk::waitForMore: Audio duration for '%s': %.0f ms (source: lookup table)", baseFilename.c_str(), _currentAudioDuration);
				} else {
					// Fallback to 15 FPS heuristic if not found in lookup table
					if (_3doAudioDecoder && _3doAudioDecoder->getFrameCount() > 0) {
						_currentAudioDuration = (_3doAudioDecoder->getFrameCount() * 1000.0) / 15.0;
						debug(2, "ScalpelTalk::waitForMore: Audio duration for '%s': %.0f ms (source: 15 FPS heuristic, %d frames)",
						      baseFilename.c_str(), _currentAudioDuration, _3doAudioDecoder->getFrameCount());
					} else {
						_currentAudioDuration = 0.0;
						debug(2, "ScalpelTalk::waitForMore: Audio duration for '%s': 0 ms (source: no decoder/frames)", baseFilename.c_str());
					}
				}
			}
		} else {
			debug(3, "ScalpelTalk::waitForMore: No 3DO file for Speaker %d, Index %d", _speaker, _pcTalkie3DOSubindex);
			_currentAudioDuration = 0.0;
		}

		// Always increment subindex when we've finished processing a "turn" (speaker change)
		// This ensures the next speaker change looks for the next file in the sequence
		_pcTalkie3DOSubindex++;

		_lastSpeaker = _speaker;
	} else {
		// Same speaker: Continue existing audio
		if (is3DOAudioPlaying()) {
			debug(2, "ScalpelTalk::waitForMore: Continuing audio for Speaker %d", _speaker);
			// Note: _currentAudioDuration is preserved from previous call
		} else {
			_currentAudioDuration = 0.0;
		}
	}

	return false;
}

int ScalpelTalk::waitLoop(int delay) {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;

	CursorId oldCursor = events.getCursor();
	int key2 = 254;
	bool skipped = false;

	// Unless we're in stealth mode, show the appropriate cursor
	if (!_talkStealth) {
		events.setCursor(_vm->_ui->_lookScriptFlag ? MAGNIFY : ARROW);
	}

	switchSpeaker(); // Ensure portrait is active

	// MAIN TIMING LOGIC: Text-Driven
	// We use real time (getMillis) to track the text delay, avoiding frame-rate lag issues.
	// delay is in frames (approx 10ms).

	uint32 startTime = g_system->getMillis();
	bool timeUp = false;
	uint32 targetTextDuration = delay * 10;

	// Track if audio was playing at start of loop
	bool wasAudioPlaying = is3DOAudioPlaying();

	do {
		// Pump Audio
		if (_3doAudioDecoder && _3doAudioDecoder->needsUpdate()) {
			_3doAudioDecoder->decodeNextFrame();
		}

		// Update PC Animations
		scene.doBgAnim();

		// Check for Abort
		if (_talkToAbort) {
			key2 = -1;
			events._released = true;
			break;
		}

		// Poll Events & Check Input
		events.pollEventsAndWait();
		events.setButtonState();

		if (events.actionHit()) {
			Common::CustomEventType action = events.getAction();
			if (action != kActionNone) key2 = action;
		}

		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();
			if (Common::isPrint(keyState.ascii)) key2 = keyState.keycode;

			// ESC/Space always skips audio immediately
			if (keyState.keycode == Common::KEYCODE_ESCAPE ||
			    keyState.keycode == Common::KEYCODE_SPACE) {
				skipped = true;
			}
		}

		// Mouse click behavior depends on page type
		if (events._pressed || events._released) {
			if (_waitForAudio) {
				// Last page: skip and stop audio
				skipped = true;
			} else {
				// Mid-page: advance text but keep audio playing
				timeUp = true;
			}
		}

		// Update Timing (Real Time)
		uint32 currentTime = g_system->getMillis();
		uint32 elapsed = currentTime - startTime;

		bool textTimerDone = (elapsed >= targetTextDuration);
		bool audioFinished = (wasAudioPlaying && !is3DOAudioPlaying());

		// FORCE AUDIO ADVANCE based on Duration Lookup (Wall Clock)
		// If the decoder claims it's playing but we exceeded the known audio duration, assume it's done.
		if (wasAudioPlaying && _currentAudioDuration > 0.0 && elapsed >= _currentAudioDuration) {
			audioFinished = true;
		}

		// logic:
		// 1. Last Page (_waitForAudio=true):
		//    - Wait for Audio to finish (Lip Sync).
		//    - If Audio finishes -> Advance.
		//    - If Text finishes but Audio plays -> Wait.
		//    - If Audio never played -> Wait for Text.

		// 2. Multi Page (_waitForAudio=false):
		//    - Wait for Text to finish (Reading).
		//    - If Text finishes -> Advance (Audio continues).

		if (_waitForAudio && wasAudioPlaying) {
			// Waiting for Audio (Lip Sync)
			if (audioFinished) {
				// Audio done. Enforce small floor (0.4s) to avoid flash.
				if (elapsed >= 400) timeUp = true;
			}
			// If Audio is still playing, we wait.
			// Unless text timer is excessively long? No, user complained about "too fast".
		} else {
			// Waiting for Text (Multi Page OR No Audio)
			if (textTimerDone) {
				timeUp = true;
			}
		}

		// Failsafe: If Audio finished/absent and Text Done, always advance.
		if (textTimerDone && !is3DOAudioPlaying()) {
			timeUp = true;
		}

	} while (!_vm->shouldQuit() && key2 == 254 && !events._released && !events._rightReleased && !skipped && !timeUp);

	// Handle different exit conditions
	if (skipped) {
		debug(2, "ScalpelTalk::waitLoop: Audio skipped by user (ESC/Space or click on last page, elapsed: %d ms)", g_system->getMillis() - startTime);
		stop3DOAudio();
	} else if (timeUp && wasAudioPlaying && is3DOAudioPlaying()) {
		debug(2, "ScalpelTalk::waitLoop: Text advanced, audio continues (multi-page dialogue, elapsed: %d ms)", g_system->getMillis() - startTime);
	}

	// Clear any pending events so they don't trigger the next dialogue line immediately
	events.clearEvents();

	// Cleanup
	events.setCursor(_talkToAbort ? ARROW : oldCursor);
	events._pressed = events._released = false;

	// Adjust _talkStealth (copied from original)
	switch (_talkStealth) {
	case 1: _talkStealth = 0; break;
	case 2: _talkStealth = 2; break;
	default: break;
	}

	return key2;
}

int ScalpelTalk::waitForMore(int delay) {
	ScalpelEngine &vm = *(ScalpelEngine *)_vm;

	// Handle native 3DO version - always play video for each text page
	// (Original behavior: each page has its own video clip)
	if (IS_3DO) {
		return handle3DONative();
	}

	// Handle PC version with 3DO talkie mode enabled
	if (vm.getTalkieMode() == SherlockEngine::TALKIE_AUDIO_ONLY) {
		if (handleTalkieMode())
			return 254;
	}

	// Custom Wait Loop (Replaces Talk::waitForMore)
	return waitLoop(delay);
}

bool ScalpelTalk::talk3DOMovieTrigger(int subIndex) {
	ScalpelEngine &vm = *(ScalpelEngine *)_vm;
	Screen &screen = *_vm->_screen;

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
		} else {
			warning("talk3DOMovieTrigger: unable to find selector");
			return true;
		}
	}

	// Make a quick update, so that current text is shown on screen
	screen.update();

	// Figure out that movie filename
	Common::String movieName;

	movieName = _scriptName;
	movieName.deleteChar(1); // remove 2nd character of scriptname
	// cut scriptname to 6 characters
	while (movieName.size() > 6) {
		movieName.deleteChar(6);
	}

	movieName.insertChar(selector + 'a', movieName.size());
	movieName.insertChar(subIndex + 'a', movieName.size());

	Common::Path movieFilename(Common::String::format("movies/%02d/%s.stream", roomNr, movieName.c_str()));

	warning("3DO movie player:");
	warning("room: %d", roomNr);
	warning("script: %s", _scriptName.c_str());
	warning("selector: %d", selector);
	warning("subindex: %d", subIndex);

	bool result = vm.play3doMovie(movieFilename, get3doPortraitPosition(), true);

	// Restore screen HACK
	_vm->_screen->makeAllDirty();

	return result;
}

Common::Point ScalpelTalk::get3doPortraitPosition() const {
	// TODO: This current method is only an assumption of how the original figured
	// out where to place each character's portrait movie.
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	const int PORTRAIT_W = 100;
	const int PORTRAIT_H = 76;

	if (_speaker == -1)
		return Common::Point();

	// Get the position of the character
	Common::Point pt;
	if (_speaker == HOLMES) {
		pt = Common::Point(people[HOLMES]._position.x / FIXED_INT_MULTIPLIER,
			people[HOLMES]._position.y / FIXED_INT_MULTIPLIER);
	} else {
		int objNum = people.findSpeaker(_speaker);
		if (objNum == -1)
			return Common::Point();

		pt = scene._bgShapes[objNum]._position;
	}

	// Adjust the top-left so the center of the portrait will be on the character,
	// but ensure the portrait will be entirely on-screen
	pt -= Common::Point(PORTRAIT_W / 2, PORTRAIT_H / 2);
	pt.x = CLIP((int)pt.x, 10, SHERLOCK_SCREEN_WIDTH - 10 - PORTRAIT_W);
	pt.y = CLIP((int)pt.y, 10, CONTROLS_Y - PORTRAIT_H - 10);

	return pt;
}

void ScalpelTalk::drawInterface() {
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	Surface &bb = *screen.getBackBuffer();

	bb.fillRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y1 + 10), BORDER_COLOR);
	bb.fillRect(Common::Rect(0, CONTROLS_Y + 10, 2, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y + 10,
		SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - 1, SHERLOCK_SCREEN_WIDTH - 2,
		SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(2, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH - 2,
		SHERLOCK_SCREEN_HEIGHT - 1), INV_BACKGROUND);

	if (_talkTo != -1) {
		Common::String fixedText_Exit = FIXED(Window_Exit);
		Common::String fixedText_Up   = FIXED(Window_Up);
		Common::String fixedText_Down = FIXED(Window_Down);

		screen.makeButton(Common::Rect(99, CONTROLS_Y, 139, CONTROLS_Y + 10),
			119, fixedText_Exit);
		screen.makeButton(Common::Rect(140, CONTROLS_Y, 180, CONTROLS_Y + 10),
			159, fixedText_Up);
		screen.makeButton(Common::Rect(181, CONTROLS_Y, 221, CONTROLS_Y + 10),
			200, fixedText_Down);
	} else {
		Common::String fixedText_PressKeyToContinue = FIXED(PressKey_ToContinue);

		screen.makeButton(Common::Rect(46, CONTROLS_Y, 273, CONTROLS_Y + 10),
			160, fixedText_PressKeyToContinue);
	}
}

bool ScalpelTalk::displayTalk(bool slamIt) {
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
	if (_moreTalkUp) {
		if (slamIt) {
			screen.print(Common::Point(5, CONTROLS_Y + 13), INV_FOREGROUND, "~");
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_FOREGROUND, true, _fixedTextWindowUp);
		} else {
			screen.gPrint(Common::Point(5, CONTROLS_Y + 12), INV_FOREGROUND, "~");
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_FOREGROUND, false, _fixedTextWindowUp);
		}
	} else {
		if (slamIt) {
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, true, _fixedTextWindowUp);
			screen.vgaBar(Common::Rect(5, CONTROLS_Y + 11, 15, CONTROLS_Y + 22), INV_BACKGROUND);
		} else {
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, _fixedTextWindowUp);
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
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_FOREGROUND, true, _fixedTextWindowDown);
		} else {
			screen.gPrint(Common::Point(5, 189), INV_FOREGROUND, "|");
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_FOREGROUND, false, _fixedTextWindowDown);
		}
	} else {
		if (slamIt) {
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, true, _fixedTextWindowDown);
			screen.vgaBar(Common::Rect(5, 189, 16, 199), INV_BACKGROUND);
		} else {
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, _fixedTextWindowDown);
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
		int width = 0;
		int lastSpace = 0;
		int lastValid = 0;
		int linePtr = 0;
		int nextLine = -1;
		do {
			lastValid = linePtr;
			if (lineStartP[linePtr] == ' ')
				lastSpace = linePtr;
			width += screen.charWidth(lineStartP, linePtr);
		} while (lineStartP[linePtr] && width < maxWidth);

		// Check if we need to wrap the line
		if (width >= maxWidth) {
			if (lastSpace > 0) {
				sLine = Common::String(lineStartP, lastSpace);
				nextLine = lastSpace + 1;
			} else {
				sLine = Common::String(lineStartP, lastValid);
				nextLine = lastValid;
			}
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
			lineY += _vm->getLanguage() == Common::Language::ZH_TWN ? 16 : 9;
			if (nextLine < 0)
				break;
			lineStartP += nextLine;
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
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	ScalpelUserInterface &ui = *(ScalpelUserInterface *)_vm->_ui;

	// 3DO talkie mode is currently disabled in showTalk()
	byte color = ui._endKeyActive ? COMMAND_FOREGROUND : COMMAND_NULL;

	if (!ui._windowOpen) {
		// Draw the talk interface on the back buffer
		drawInterface();
		displayTalk(false);
	} else {
		displayTalk(true);
	}

	// If the window is already open, simply draw. Otherwise, do it
	// to the back buffer and then summon the window
	if (ui._windowOpen) {
		screen.buttonPrint(Common::Point(119, CONTROLS_Y), color, true, _fixedTextWindowExit);
	} else {
		screen.buttonPrint(Common::Point(119, CONTROLS_Y), color, false, _fixedTextWindowExit);

		if (!ui._slideWindows) {
			screen.slamRect(Common::Rect(0, CONTROLS_Y,
				SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
		} else {
			ui.summonWindow();
		}

		ui._windowOpen = true;
	}
}

OpcodeReturn ScalpelTalk::cmdCallTalkFile(const byte *&str) {
	Common::String tempString;

	++str;
	for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
		tempString += str[idx];
	str += 8;

	int scriptCurrentIndex = str - _scriptStart;

	// Save the current script position and new talk file
	if (_scriptStack.size() < 9) {
		ScriptStackEntry rec1;
		rec1._name = _scriptName;
		rec1._currentIndex = scriptCurrentIndex;
		rec1._select = _scriptSelect;
		_scriptStack.push(rec1);

		// Push the new talk file onto the stack
		ScriptStackEntry rec2;
		rec2._name = tempString;
		rec2._currentIndex = 0;
		rec2._select = 100;
		_scriptStack.push(rec2);
	} else {
		error("Script stack overflow");
	}

	_scriptMoreFlag = 1;
	_endStr = true;
	_wait = 0;

	return RET_SUCCESS;
}

void ScalpelTalk::pushSequenceEntry(Object *obj) {
	Scene &scene = *_vm->_scene;
	SequenceEntry seqEntry;
	seqEntry._objNum = scene._bgShapes.indexOf(*obj);

	if (seqEntry._objNum != -1) {
		for (uint idx = 0; idx < MAX_TALK_SEQUENCES; ++idx)
			seqEntry._sequences.push_back(obj->_sequences[idx]);

		seqEntry._frameNumber = obj->_frameNumber;
		seqEntry._seqTo = obj->_seqTo;
	}

	_sequenceStack.push(seqEntry);
	if (_scriptStack.size() >= 5)
		error("script stack overflow");
}

void ScalpelTalk::pullSequence(int slot) {
	Scene &scene = *_vm->_scene;

	if (_sequenceStack.empty())
		return;

	SequenceEntry seq = _sequenceStack.pop();
	if (seq._objNum != -1) {
		Object &obj = scene._bgShapes[seq._objNum];

		if (obj._seqSize < MAX_TALK_SEQUENCES) {
			warning("Tried to restore too few frames");
		} else {
			for (int idx = 0; idx < MAX_TALK_SEQUENCES; ++idx)
				obj._sequences[idx] = seq._sequences[idx];

			obj._frameNumber = seq._frameNumber;
			obj._seqTo = seq._seqTo;
		}
	}
}

void ScalpelTalk::clearSequences() {
	_sequenceStack.clear();
}

bool ScalpelTalk::play3DOConversationAudio(const Common::String &videoFile) {
	ScalpelEngine &vm = *(ScalpelEngine *)_vm;

	// Stop any currently playing audio
	stop3DOAudio();

	// Construct full path to video file (which contains audio)
	Common::Path videoPath = vm.get3DOVideoPath(videoFile);

	debug(1, "ScalpelTalk::play3DOConversationAudio: Loading audio from %s", videoPath.toString().c_str());

	// Create and initialize video decoder (to extract audio)
	_3doAudioDecoder = new Video::ThreeDOMovieDecoder();

	if (!_3doAudioDecoder->loadFile(videoPath)) {
		warning("ScalpelTalk::play3DOConversationAudio: Failed to load video: %s", videoPath.toString().c_str());
		stop3DOAudio();
		return false;
	}

	// Start playback (this starts the audio track)
	_3doAudioDecoder->start();

	debug(1, "ScalpelTalk::play3DOConversationAudio: Audio playback started");

	// DON'T BLOCK - just return and let the talk script continue
	// The video decoder's audio track is playing asynchronously via the mixer
	// The caller (waitForMore) is responsible for waiting
	return true;
}

Common::String ScalpelTalk::get3DOVideoFile(int talkIndex, int speechIndex) {
	// Dynamic 3DO video filename generation
	// Pattern: Movies/{room:02d}/{prefix}{room:02d}{last_char}{selector}{subindex}.stream
	// Where prefix = scriptName[0] + scriptName[2] + scriptName[3] (skip 2nd char), lowercased

	// Get current room number
	int roomNum = _vm->_scene->_currentScene;

	// Extract 3-letter prefix from script name by skipping the 2nd character
	// "Wigg39a" → "Wgg" → "wgg"
	// "Wats04a" → "Wts" → "wts"
	char prefix[4] = "wtx";  // Default fallback
	if (_scriptName.size() >= 4) {
		prefix[0] = tolower(_scriptName[0]);  // 1st char
		prefix[1] = tolower(_scriptName[2]);  // 3rd char (skip 2nd)
		prefix[2] = tolower(_scriptName[3]);  // 4th char
		prefix[3] = '\0';
	}

	// Extract last character from script name (before extension if present)
	// "lest03z.tlk" -> 'z', "Wigg39a" -> 'a'
	// Handle variants like "Wats04a1" -> 'a' (ignore trailing digits)
	char scriptLetter = 'a';  // Default
	if (!_scriptName.empty()) {
		// Find the last '.' to remove extension
		int dotPos = _scriptName.findLastOf('.');
		int endPos = (dotPos != -1 && dotPos > 0) ? dotPos : _scriptName.size();

		// Scan backwards from endPos to find first non-digit
		int charPos = endPos - 1;
		while (charPos >= 0 && Common::isDigit(_scriptName[charPos])) {
			charPos--;
		}

		if (charPos >= 0) {
			scriptLetter = tolower(_scriptName[charPos]);
		} else {
			// Fallback if all digits or empty? Use last char
			scriptLetter = tolower(_scriptName.lastChar());
		}
	}

	// Use the tracked selector set by UI via set3DODialogueSelection()
	// Falls back to _talkIndex for automatic dialogues
	int selector = (_pcTalkie3DOSelector == -1) ? _talkIndex : _pcTalkie3DOSelector;
	int subindex = _pcTalkie3DOSubindex;

	// Convert selector and subindex to characters (0='a', 1='b', etc.)
	char selectorChar = 'a' + selector;
	char subindexChar = 'a' + subindex;

	// Build filename: Movies/39/wgg39aaa.stream
	Common::String videoFile = Common::String::format(
		"Movies/%02d/%s%02d%c%c%c.stream",
		roomNum,
		prefix,
		roomNum,
		scriptLetter,
		selectorChar,
		subindexChar
	);

	// DETAILED DEBUG: Track all values
	warning("=== 3DO Video Generation ===");
	warning("  room=%d", roomNum);
	warning("  _scriptName='%s'", _scriptName.c_str());
	warning("  prefix='%s' (chars [0,2,3] from script)", prefix);
	warning("  scriptLetter='%c' (last char of script)", scriptLetter);
	warning("  _pcTalkie3DOSelector=%d, _talkIndex=%d -> selector=%d -> selectorChar='%c'", _pcTalkie3DOSelector, _talkIndex, selector, selectorChar);
	warning("  _pcTalkie3DOSubindex=%d -> subindexChar='%c'", subindex, subindexChar);
	warning("  GENERATED FILE: %s", videoFile.c_str());
	warning("============================");

	return videoFile;
}

void ScalpelTalk::skipBadText(const byte *&msgP) {
	// WORKAROUND: Skip over bad text in the original game
	const char *BAD_PHRASE1 = "Change Speaker to Sherlock Holmes ";

	if (!strncmp((const char *)msgP, BAD_PHRASE1, strlen(BAD_PHRASE1)))
		msgP += strlen(BAD_PHRASE1);
}

} // End of namespace Scalpel

} // End of namespace Sherlock
