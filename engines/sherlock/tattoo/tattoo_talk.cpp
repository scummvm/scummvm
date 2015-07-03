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

#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/sherlock.h"
#include "sherlock/screen.h"

namespace Sherlock {

namespace Tattoo {

static const uint8 DIRECTION_CONVERSION[] = {
	WALK_RIGHT, WALK_DOWN, WALK_LEFT, WALK_UP, STOP_RIGHT, STOP_DOWN, STOP_LEFT, STOP_UP,
	WALK_UPRIGHT, WALK_DOWNRIGHT, WALK_UPLEFT, WALK_DOWNLEFT, STOP_UPRIGHT, STOP_UPLEFT, 
	STOP_DOWNRIGHT, STOP_DOWNLEFT
};

const byte TATTOO_OPCODES[] = {
	170,	// OP_SWITCH_SPEAKER
	171,	// OP_RUN_CANIMATION
	0,	// OP_ASSIGN_PORTRAIT_LOCATION
	173,	// OP_PAUSE
	0,	// OP_REMOVE_PORTRAIT
	0,	// OP_CLEAR_WINDOW
	176,	// OP_ADJUST_OBJ_SEQUENCE
	177,	// OP_WALK_HOlMES_TO_COORDS
	178,	// OP_PAUSE_WITHOUT_CONTROL
	179,	// OP_BANISH_WINDOW
	0,		// OP_SUMMON_WINDOW
	181,	// OP_SET_FLAG
	0,		// OP_SFX_COMMAND
	183,	// OP_TOGGLE_OBJECT
	184,	// OP_STEALTH_MODE_ACTIVE
	0,		// OP_IF_STATEMENT
	0,		// OP_ELSE_STATEMENT
	0,		// OP_END_IF_STATEMENT
	188,	// OP_STEALTH_MODE_DEACTIVATE
	189,	// OP_TURN_HOLMES_OFF
	190,	// OP_TURN_HOLMES_ON
	191,	// OP_GOTO_SCENE
	0,		// OP_PLAY_PROLOGUE
	193,	// OP_ADD_ITEM_TO_INVENTORY
	194,	// OP_SET_OBJECT
	172,	// OP_CALL_TALK_FILE
	0,		// OP_MOVE_MOUSE
	0,		// OP_DISPLAY_INFO_LINE
	0,		// OP_CLEAR_INFO_LINE
	199,	// OP_WALK_TO_CANIMATION
	200,	// OP_REMOVE_ITEM_FROM_INVENTORY
	201,	// OP_ENABLE_END_KEY
	202,	// OP_DISABLE_END_KEY
	203,	// OP_END_TEXT_WINDOW
	174,	// OP_MOUSE_ON_OFF
	175,	// OP_SET_WALK_CONTROL
	180,	// OP_SET_TALK_SEQUENCE
	182,	// OP_PLAY_SONG
	187,	// OP_WALK_HOLMES_AND_NPC_TO_CANIM
	192,	// OP_SET_NPC_PATH_DEST
	195,	// OP_NEXT_SONG
	196,	// OP_SET_NPC_PATH_PAUSE
	197,	// OP_PASSWORD
	198,	// OP_SET_SCENE_ENTRY_FLAG
	185,	// OP_WALK_NPC_TO_CANIM
	186,	// OP_WALK_NPC_TO_COORDS
	204,	// OP_WALK_HOLMES_AND_NPC_TO_COORDS
	205,	// OP_SET_NPC_TALK_FILE
	206,	// OP_TURN_NPC_OFF
	207,	// OP_TURN_NPC_ON
	208,	// OP_NPC_DESC_ON_OFF
	209,	// OP_NPC_PATH_PAUSE_TAKING_NOTES
	210,	// OP_NPC_PATH_PAUSE_LOOKING_HOLMES
	211,	// OP_ENABLE_TALK_INTERRUPTS
	212,	// OP_DISABLE_TALK_INTERRUPTS
	213,	// OP_SET_NPC_INFO_LINE
	214,	// OP_SET_NPC_POSITION
	215,	// OP_NPC_PATH_LABEL
	216,	// OP_PATH_GOTO_LABEL
	217,	// OP_PATH_IF_FLAG_GOTO_LABEL
	218,	// OP_NPC_WALK_GRAPHICS
	220,	// OP_NPC_VERB
	221,	// OP_NPC_VERB_CANIM
	222,	// OP_NPC_VERB_SCRIPT
	224,	// OP_RESTORE_PEOPLE_SEQUENCE
	226,	// OP_NPC_VERB_TARGET
	227,	// OP_TURN_SOUNDS_OFF
	225		// OP_NULL
};

/*----------------------------------------------------------------*/

TattooTalk::TattooTalk(SherlockEngine *vm) : Talk(vm), _talkWidget(vm) {
	static OpcodeMethod OPCODE_METHODS[] = {
		(OpcodeMethod)&TattooTalk::cmdSwitchSpeaker,

		(OpcodeMethod)&TattooTalk::cmdRunCAnimation,
		(OpcodeMethod)&TattooTalk::cmdCallTalkFile,
		(OpcodeMethod)&TattooTalk::cmdPause,
		(OpcodeMethod)&TattooTalk::cmdMouseOnOff,
		(OpcodeMethod)&TattooTalk::cmdSetWalkControl,
		(OpcodeMethod)&TattooTalk::cmdAdjustObjectSequence,
		(OpcodeMethod)&TattooTalk::cmdWalkHolmesToCoords,
		(OpcodeMethod)&TattooTalk::cmdPauseWithoutControl,
		(OpcodeMethod)&TattooTalk::cmdBanishWindow,
		(OpcodeMethod)&TattooTalk::cmdSetTalkSequence,

		(OpcodeMethod)&TattooTalk::cmdSetFlag,
		(OpcodeMethod)&TattooTalk::cmdPlaySong,
		(OpcodeMethod)&TattooTalk::cmdToggleObject,
		(OpcodeMethod)&TattooTalk::cmdStealthModeActivate,
		(OpcodeMethod)&TattooTalk::cmdWalkNPCToCAnimation,
		(OpcodeMethod)&TattooTalk::cmdWalkNPCToCoords,
		(OpcodeMethod)&TattooTalk::cmdWalkHomesAndNPCToCoords,
		(OpcodeMethod)&TattooTalk::cmdStealthModeDeactivate,
		(OpcodeMethod)&TattooTalk::cmdHolmesOff,
		(OpcodeMethod)&TattooTalk::cmdHolmesOn,

		(OpcodeMethod)&TattooTalk::cmdGotoScene,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPathDest,
		(OpcodeMethod)&TattooTalk::cmdAddItemToInventory,
		(OpcodeMethod)&TattooTalk::cmdSetObject,
		(OpcodeMethod)&TattooTalk::cmdNextSong,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPathPause,
		(OpcodeMethod)&TattooTalk::cmdPassword,
		(OpcodeMethod)&TattooTalk::cmdSetSceneEntryFlag,
		(OpcodeMethod)&TattooTalk::cmdWalkToCAnimation,
		(OpcodeMethod)&TattooTalk::cmdRemoveItemFromInventory,

		(OpcodeMethod)&TattooTalk::cmdEnableEndKey,
		(OpcodeMethod)&TattooTalk::cmdDisableEndKey,
		(OpcodeMethod)&TattooTalk::cmdEndTextWindow,
		(OpcodeMethod)&TattooTalk::cmdWalkHomesAndNPCToCoords,
		(OpcodeMethod)&TattooTalk::cmdSetNPCTalkFile,
		(OpcodeMethod)&TattooTalk::cmdSetNPCOff,
		(OpcodeMethod)&TattooTalk::cmdSetNPCOn,
		(OpcodeMethod)&TattooTalk::cmdSetNPCDescOnOff,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPathPauseTakingNotes,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPathPauseLookingHolmes,

		(OpcodeMethod)&TattooTalk::cmdTalkInterruptsEnable,
		(OpcodeMethod)&TattooTalk::cmdTalkInterruptsDisable,
		(OpcodeMethod)&TattooTalk::cmdSetNPCInfoLine,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPosition,
		(OpcodeMethod)&TattooTalk::cmdNPCLabelSet,
		(OpcodeMethod)&TattooTalk::cmdNPCLabelGoto,
		(OpcodeMethod)&TattooTalk::cmdNPCLabelIfFlagGoto,
		(OpcodeMethod)&TattooTalk::cmdSetNPCWalkGraphics,
		nullptr,
		(OpcodeMethod)&TattooTalk::cmdSetNPCVerb,

		(OpcodeMethod)&TattooTalk::cmdSetNPCVerbCAnimation,
		(OpcodeMethod)&TattooTalk::cmdSetNPCVerbScript,
		nullptr,
		(OpcodeMethod)&TattooTalk::cmdRestorePeopleSequence,
		(OpcodeMethod)&TattooTalk::cmdSetNPCVerbTarget,
		(OpcodeMethod)&TattooTalk::cmdTurnSoundsOff
	};

	_opcodes = TATTOO_OPCODES;
	_opcodeTable = OPCODE_METHODS;
}

void TattooTalk::talkInterface(const byte *&str) {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	const byte *s = str;

	// Move to past the end of the text string
	_charCount = 0;
	while ((*str < TATTOO_OPCODES[0] || *str == TATTOO_OPCODES[OP_NULL]) && *str) {
		++_charCount;
		++str;
	}

	// Display the text window
	ui.banishWindow();
	ui._textWidget.load(Common::String((const char *)s, (const char *)str), _speaker);
	ui._textWidget.summonWindow();
	_wait = true;
}

void TattooTalk::openTalkWindow() {
	_talkWidget.load();
	_talkWidget.summonWindow();
}

OpcodeReturn TattooTalk::cmdSwitchSpeaker(const byte *&str) {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	if (_talkToAbort)
		return RET_EXIT;

	ui.clearWindow();
	
	_yp = screen.fontHeight() + 11;
	_charCount = _line = 0;

	people.setListenSequence(_speaker, 129);
	_speaker = *++str - 1;
	++str;

	people.setTalkSequence(_speaker, 1);

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdMouseOnOff(const byte *&str) { 
	Events &events = *_vm->_events;
	bool mouseOn = *++str == 2;
	if (mouseOn)
		events.showCursor();
	else
		events.hideCursor();
	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdWalkHolmesToCoords(const byte *&str) {
	People &people = *_vm->_people;
	++str;

	int x = (str[0] - 1) * 256 + str[1] - 1;
	if (x > 16384)
		x = -1 * (x - 16384);
	warning("TODO: cmdWalkHolmesToCoords - call RT walkToCoords variant");
	people[HOLMES].walkToCoords(
		Point32(x * FIXED_INT_MULTIPLIER, ((str[2] - 1) * 256 + str[3] - 1) * FIXED_INT_MULTIPLIER),
		DIRECTION_CONVERSION[str[4] - 1]
		//HOLMES
	);
	if (_talkToAbort)
		return RET_EXIT;

	str += 4;
	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdGotoScene(const byte *&str) {
	Map &map = *_vm->_map;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Scene &scene = *_vm->_scene;
	scene._goToScene = str[1] - 1;

	if (scene._goToScene != 100) {
		// Not going to the map overview
		map._oldCharPoint = scene._goToScene;

		// Run a canimation?
		if (str[2] > 100) {
			people._savedPos = PositionFacing(160, 100, str[2]);
		} else {
			int32 posX = (str[3] - 1) * 256 + str[4] - 1;
			if (posX > 16384)
				posX = -1 * (posX - 16384);
			int32 posY = (str[5] - 1) * 256 + str[6] - 1;
			people._savedPos = PositionFacing(posX, posY, str[2] - 1);
		}

		_scriptMoreFlag = 1;
	}	// if (scene._goToScene != 100)

	str += 7;
	if (scene._goToScene != 100)
		_scriptSaveIndex = str - _scriptStart;

	_endStr = true;
	_wait = 0;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdNextSong(const byte *&str) {
	Music &music = *_vm->_music;

	// Get the name of the next song to play
	++str;
	music._nextSongName = "";
	for (int idx = 0; idx < 8; ++idx) {
		if (str[idx] != '~')
			music._nextSongName += str[idx];
		else
			break;
	}
	str += 7;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdNPCLabelGoto(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	
	if (person._resetNPCPath) {
		person._npcIndex = person._npcPause = 0;
		person._resetNPCPath = false;
		memset(person._npcPath, 0, 100);
	}

	person._npcPath[person._npcIndex] = 8;
	person._npcPath[person._npcIndex + 1] = str[1];
	person._npcIndex += 2;
	str++;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdNPCLabelIfFlagGoto(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	
	if (person._resetNPCPath) {
		person._npcIndex = person._npcPause = 0;
		person._resetNPCPath = false;
		memset(person._npcPath, 0, 100);
	}

	person._npcPath[person._npcIndex] = 9;
	for (int i = 1; i <= 3; i++)
		person._npcPath[person._npcIndex + i] = str[i];

	person._npcIndex += 4;
	str += 3;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdNPCLabelSet(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	
	if (person._resetNPCPath) {
		person._npcIndex = person._npcPause = 0;
		person._resetNPCPath = false;
		memset(person._npcPath, 0, 100);
	}

	person._npcPath[person._npcIndex] = 7;
	person._npcPath[person._npcIndex + 1] = str[1];
	person._npcIndex += 2;
	str++;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdPassword(const byte *&str) { error("TODO: script opcode (cmdPassword)"); }

OpcodeReturn TattooTalk::cmdPlaySong(const byte *&str) { 
	Music &music = *_vm->_music;
	Common::String currentSong = music._currentSongName;

	// Get the name of the song to play
	music._currentSongName = "";
	str++;
	for (int idx = 0; idx < 8; ++idx) {
		if (str[idx] != '~')
			music._currentSongName += str[idx];
		else
			break;
	}
	str += 7;

	// Play the song
	music.playMusic(music._currentSongName);

	// Copy the old song name to _nextSongName so that when the new song is finished, the old song will restart
	music._nextSongName = currentSong;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdRestorePeopleSequence(const byte *&str) {
	int npcNum = *++str - 1;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	person._misc = 0;

	if (person._seqTo) {
		person._walkSequences[person._sequenceNumber]._sequences[person._frameNumber] = person._seqTo;
		person._seqTo = 0;
	}
	person._sequenceNumber = person._savedNpcSequence;
	person._frameNumber = person._savedNpcFrame;
	person.checkWalkGraphics();

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCDescOnOff(const byte *&str) {
	int npcNum = *++str;
	++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Person &person = people[npcNum];

	// Copy over the NPC examine text until we reach a stop marker, which is
	// the same as a start marker, or we reach the end of the file
	while (*str && *str != _opcodes[OP_NPC_DESC_ON_OFF])
		person._examine += *str++;

	// Move past any leftover text till we reach a stop marker
	while (*str && *str != _opcodes[OP_NPC_DESC_ON_OFF])
		str++;

	if (!*str)
		// Reached end of file, so decrement pointer so outer loop will terminate on NULL
		--str;
	else
		// Move past the ending OP_NPC_DEST_ON_OFF opcode
		++str;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCInfoLine(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];

	person._description = "";
	int len = *++str;
	for (int idx = 0; idx < len; ++idx)
		person._description += str[idx + 1];

	str += len;
	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCOff(const byte *&str) {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	int npcNum = *++str;
	people[npcNum]._type = REMOVE;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCOn(const byte *&str) { 
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	int npcNum = *++str;
	people[npcNum]._type = CHARACTER;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCPathDest(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	
	if (person._resetNPCPath) {
		person._npcIndex = person._npcPause = 0;
		person._resetNPCPath = false;
		memset(person._npcPath, 0, 100);
	}

	person._npcPath[person._npcIndex] = 1;
	for (int i = 1; i <= 4; i++)
		person._npcPath[person._npcIndex + i] = str[i];
	person._npcPath[person._npcIndex + 5] = DIRECTION_CONVERSION[str[5] - 1] + 1;

	person._npcIndex += 6;
	str += 5;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCPathPause(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	
	if (person._resetNPCPath) {
		person._npcIndex = person._npcPause = 0;
		person._resetNPCPath = false;
		memset(person._npcPath, 0, 100);
	}

	person._npcPath[person._npcIndex] = 2;
	for (int i = 1; i <= 2; i++)
		person._npcPath[person._npcIndex + i] = str[i];

	person._npcIndex += 3;
	str += 2;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCPathPauseTakingNotes(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	
	if (person._resetNPCPath) {
		person._npcIndex = person._npcPause = 0;
		person._resetNPCPath = false;
		memset(person._npcPath, 0, 100);
	}

	person._npcPath[person._npcIndex] = 5;
	for (int i = 1; i <= 2; i++)
		person._npcPath[person._npcIndex + i] = str[i];

	person._npcIndex += 3;
	str += 2;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCPathPauseLookingHolmes(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	
	if (person._resetNPCPath) {
		person._npcIndex = person._npcPause = 0;
		person._resetNPCPath = false;
		memset(person._npcPath, 0, 100);
	}

	person._npcPath[person._npcIndex] = 6;
	for (int i = 1; i <= 2; i++)
		person._npcPath[person._npcIndex + i] = str[i];

	person._npcIndex += 3;
	str += 2;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCPosition(const byte *&str) {
	int npcNum = *++str - 1;
	++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	int32 posX = (str[0] - 1) * 256 + str[1] - 1;
	if (posX > 16384)
		posX = -1 * (posX - 16384);
	int32 posY = (str[2] - 1) * 256 + str[3] - 1;
	
	people[npcNum]._position = Point32(posX * FIXED_INT_MULTIPLIER, posY * FIXED_INT_MULTIPLIER);
	if (person._seqTo && person._walkLoaded) {
		person._walkSequences[person._sequenceNumber]._sequences[person._frameNumber] = person._seqTo;
		person._seqTo = 0;
	}

	assert(str[4] - 1 < 16);
	person._sequenceNumber = DIRECTION_CONVERSION[str[4] - 1];
	person._frameNumber = 0;

	if (person._walkLoaded)
		person.checkWalkGraphics();

	if (person._walkLoaded && person._type == CHARACTER &&
		person._sequenceNumber >= STOP_UP && person._sequenceNumber <= STOP_UPLEFT) {
		bool done = false;
		do {
			person.checkSprite();
			for (int x = 0; x < person._frameNumber; x++) {
				if (person._walkSequences[person._sequenceNumber]._sequences[x] == 0) {
					done = true;
					break;
				}
			}
		} while(!done);
	}

	str += 4;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCTalkFile(const byte *&str) {
	int npcNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	
	if (person._resetNPCPath) {
		person._npcIndex = person._npcPause = 0;
		person._resetNPCPath = false;
		memset(person._npcPath, 0, 100);
	}

	person._npcPath[person._npcIndex] = 3;
	for (int i = 1; i <= 8; i++)
		person._npcPath[person._npcIndex + i] = str[i];

	person._npcIndex += 9;
	str += 8;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCVerb(const byte *&str) {
	int npcNum = *++str;
	int verbNum = *++str - 1;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Common::String &verb = people[npcNum]._use[verbNum]._verb;

	for (int x = 0; x < 12; x++) {
		if (str[x + 1] != '~')
			verb.setChar(str[x + 1], x);
		else
			verb.setChar(0, x);
	}

	verb.setChar(0, 11);

	uint len = verb.size() - 1;
	while (verb[len] == ' ' && len)
		len--;
	verb.setChar(0, len + 1);
	if (verb != " ")
		verb.clear();
	str += 12;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCVerbCAnimation(const byte *&str) {
	int npcNum = *++str;
	int verbNum = *++str - 1;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	UseType &useType = people[npcNum]._use[verbNum];

	useType._cAnimNum = (str[1] - 1) & 127;
	useType._cAnimSpeed = 1 + 128 * (str[1] >= 128);
	str++;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCVerbScript(const byte *&str) {
	int npcNum = *++str;
	int verbNum = *++str - 1;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	UseType &useType = people[npcNum]._use[verbNum];
	Common::String &name = useType._names[0];
	name.setChar('*', 0);
	name.setChar('C', 1);

	for (int x = 0; x < 8; x++) {
		if (str[x + 1] != '~')
			name.setChar(str[x + 1], x + 2);
		else
			name.setChar(0, x + 2);
	}

	name.setChar(0, 11);
	useType._cAnimNum = 99;
	useType._cAnimSpeed = 1;
	str += 8;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCVerbTarget(const byte *&str) {
	int npcNum = *++str;
	int verbNum = *++str - 1;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Common::String &target = people[npcNum]._use[verbNum]._target;

	for (int x = 0; x < 12; x++) {
		if (str[x + 1] != '~')
			target.setChar(str[x + 1], x);
		else
			target.setChar(0, x);
	}

	target.setChar(0, 11);

	uint len = target.size() - 1;
	while (target[len] == ' ' && len)
		len--;
	target.setChar(0, len + 1);
	str += 12;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCWalkGraphics(const byte *&str) {
	int npcNum = *++str - 1;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Person &person = people[npcNum];

	// Build up walk library name for the given NPC
	person._walkVGSName = "";
	for (int idx = 0; idx < 8; ++idx) {
		if (str[idx + 1] != '~')
			person._walkVGSName += str[idx + 1];
		else
			break;
	}
	person._walkVGSName += ".VGS";

	people._forceWalkReload = true;
	str += 8;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetSceneEntryFlag(const byte *&str) { error("TODO: script opcode (cmdSetSceneEntryFlag)"); }

OpcodeReturn TattooTalk::cmdSetTalkSequence(const byte *&str) {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	int speaker = str[1] - 1;
	int sequenceNumber = str[2];

	if (sequenceNumber < 128)
		people.setTalkSequence(speaker, sequenceNumber);
	else
		people.setListenSequence(speaker, sequenceNumber);
	
	str += 2;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetWalkControl(const byte *&str) {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	++str;
	people._walkControl = str[0] - 1;
	
	return RET_SUCCESS;
}

// Dummy opcode
OpcodeReturn TattooTalk::cmdTalkInterruptsDisable(const byte *&str) { error("Dummy opcode cmdTalkInterruptsDisable called"); }

// Dummy opcode
OpcodeReturn TattooTalk::cmdTalkInterruptsEnable(const byte *&str) { error("Dummy opcode cmdTalkInterruptsEnable called"); }

OpcodeReturn TattooTalk::cmdTurnSoundsOff(const byte *&str) { error("TODO: script opcode (cmdTurnSoundsOff)"); }

OpcodeReturn TattooTalk::cmdWalkHolmesAndNPCToCAnimation(const byte *&str) {
	int npcNum = *++str;
	int cAnimNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	Scene &scene = *_vm->_scene;
	CAnim &anim = scene._cAnim[cAnimNum];

	if (person._pathStack.empty())
		person.pushNPCPath();
	person._npcMoved = true;

	warning("TODO: cmdWalkNPCToCAnimation - walkBothToCoords call");
	person.walkToCoords(
		Point32(anim._goto[1].x * FIXED_INT_MULTIPLIER, anim._goto[1].y * FIXED_INT_MULTIPLIER),
		anim._goto[1]._facing
		//Point32(anim._goto[1].x * FIXED_INT_MULTIPLIER, anim._goto[1].y * FIXED_INT_MULTIPLIER),
		//anim._goto[1]._facing,
		//npcNum + 1
	);
	if (_talkToAbort)
		return RET_EXIT;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdWalkNPCToCAnimation(const byte *&str) {
	int npcNum = *++str;
	int cAnimNum = *++str;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];
	Scene &scene = *_vm->_scene;
	CAnim &anim = scene._cAnim[cAnimNum];

	if (person._pathStack.empty())
		person.pushNPCPath();
	person._npcMoved = true;

	warning("TODO: cmdWalkNPCToCAnimation - call RT walkToCoords variant");
	person.walkToCoords(
		Point32(anim._goto[1].x * FIXED_INT_MULTIPLIER, anim._goto[1].y * FIXED_INT_MULTIPLIER),
		anim._goto[1]._facing
		// npcNum + 1
	);
	if (_talkToAbort)
		return RET_EXIT;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdWalkNPCToCoords(const byte *&str) {
	int npcNum = *++str;
	str++;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];

	if (person._pathStack.empty())
		person.pushNPCPath();
	person._npcMoved = true;

	int x = (str[0] - 1) * 256 + str[1] - 1;
	if (x > 16384)
		x = -1 * (x - 16384);

	warning("TODO: cmdWalkNPCToCoords - call RT walkToCoords variant");
	person.walkToCoords(
		Point32(x * FIXED_INT_MULTIPLIER, ((str[2] - 1) * 256 + str[3] - 1) * FIXED_INT_MULTIPLIER),
		DIRECTION_CONVERSION[str[4] - 1]
		// npcNum + 1
	);
	if (_talkToAbort)
		return RET_EXIT;

	str += 4;
	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdWalkHomesAndNPCToCoords(const byte *&str) {
	int npcNum = *++str;
	str++;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooPerson &person = people[npcNum];

	if (person._pathStack.empty())
		person.pushNPCPath();
	person._npcMoved = true;

	int x = (str[0] - 1) * 256 + str[1] - 1;
	if (x > 16384)
		x = -1 * (x - 16384);
	//int x1 = (str[5] - 1) * 256 + str[6] - 1;
	//if (x1 > 16384)
	//	x1 = -1 * (x1 - 16384);

	warning("TODO: cmdWalkHomesAndNPCToCoords - walkBothToCoords call");
	person.walkToCoords(
		Point32(x * FIXED_INT_MULTIPLIER, ((str[2] - 1) * 256 + str[3] - 1) * FIXED_INT_MULTIPLIER),
		DIRECTION_CONVERSION[str[4] - 1]
		//Point32(x1 * FIXED_INT_MULTIPLIER, ((str[7] - 1) * 256 + str[8] - 1) * FIXED_INT_MULTIPLIER),
		//DIRECTION_CONVERSION[str[9] - 1],
		//npcNum + 1
	);
	if (_talkToAbort)
		return RET_EXIT;

	str += 9;
	return RET_SUCCESS;
}

void TattooTalk::showTalk() {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	
	_sequenceStack.clear();
	people.setListenSequence(_talkTo, 129);
	_talkWidget.refresh();
}

} // End of namespace Tattoo

} // End of namespace Sherlock
