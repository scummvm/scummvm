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
#include "sherlock/sherlock.h"
#include "sherlock/screen.h"

namespace Sherlock {

namespace Tattoo {

static const uint8 DIRECTION_CONVERSION[] = {
	WALK_RIGHT, WALK_DOWN, WALK_LEFT, WALK_UP,
	STOP_RIGHT, STOP_DOWN, STOP_LEFT, STOP_UP,
	WALK_UPRIGHT, WALK_DOWNRIGHT, WALK_UPLEFT, WALK_DOWNLEFT,
	STOP_UPRIGHT, STOP_UPLEFT, STOP_DOWNRIGHT, STOP_DOWNLEFT
};

const byte TATTOO_OPCODES[] = {
	170,	// OP_SWITCH_SPEAKER
	171,	// OP_RUN_CANIMATION
	0,	// OP_ASSIGN_PORTRAIT_LOCATION
	173,	// OP_PAUSE
	0,	// OP_REMOVE_PORTRAIT
	0,	// OP_CLEAR_WINDOW
	176,	// OP_ADJUST_OBJ_SEQUENCE
	177,	// OP_WALK_TO_COORDS
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
	0,		// OP_CARRIAGE_RETURN
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
	227		// OP_TURN_SOUNDS_OFF
};

/*----------------------------------------------------------------*/

TattooTalk::TattooTalk(SherlockEngine *vm) : Talk(vm) {
	static OpcodeMethod OPCODE_METHODS[] = {
		nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		(OpcodeMethod)&TattooTalk::cmdSwitchSpeaker,

		(OpcodeMethod)&TattooTalk::cmdRunCAnimation,
		(OpcodeMethod)&TattooTalk::cmdCallTalkFile,
		(OpcodeMethod)&TattooTalk::cmdPause,
		(OpcodeMethod)&TattooTalk::cmdMouseOnOff,
		(OpcodeMethod)&TattooTalk::cmdSetWalkControl,
		(OpcodeMethod)&TattooTalk::cmdAdjustObjectSequence,
		(OpcodeMethod)&TattooTalk::cmdWalkToCoords,
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
		nullptr,
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

void TattooTalk::setSequence(int speaker, int sequenceNum) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;

	// If no speaker is specified, then nothing needs to be done
	if (speaker == -1)
		return;

	int objNum = people.findSpeaker(speaker);
	if (objNum != -1 && objNum < 256) {
		Object &obj = scene._bgShapes[objNum];

		// See if the Object has to wait for an Abort Talk Code
		if (obj.hasAborts()) {
			pushTalkSequence(&obj);
			obj._gotoSeq = sequenceNum;
		} else {
			obj.setObjTalkSequence(sequenceNum);
		}
	} else if (objNum != -1) {
		objNum -= 256;
		Person &person = people[objNum];
		int newDir = person._sequenceNumber;

		switch (newDir) {
		case RT_WALK_UP:
		case RT_STOP_UP:
		case RT_WALK_UPRIGHT:
		case RT_STOP_UPRIGHT:
		case RT_TALK_UPRIGHT:
		case RT_LISTEN_UPRIGHT:
			newDir = RT_TALK_UPRIGHT;
			break;
		case RT_WALK_RIGHT:
		case RT_STOP_RIGHT:
		case RT_TALK_RIGHT:
		case RT_LISTEN_RIGHT:
			newDir = RT_TALK_RIGHT;
			break;
		case RT_WALK_DOWNRIGHT:
		case RT_STOP_DOWNRIGHT:
		case RT_TALK_DOWNRIGHT:
		case RT_LISTEN_DOWNRIGHT:
			newDir = RT_TALK_DOWNRIGHT;
			break;
		case RT_WALK_DOWN:
		case RT_STOP_DOWN:
		case RT_WALK_DOWNLEFT:
		case RT_STOP_DOWNLEFT:
		case RT_TALK_DOWNLEFT:
		case RT_LISTEN_DOWNLEFT:
			newDir = RT_TALK_DOWNLEFT;
			break;
		case RT_WALK_LEFT:
		case RT_STOP_LEFT:
		case RT_TALK_LEFT:
		case RT_LISTEN_LEFT:
			newDir = RT_TALK_LEFT;
			break;
		case RT_WALK_UPLEFT:
		case RT_STOP_UPLEFT:
		case RT_TALK_UPLEFT:
		case RT_LISTEN_UPLEFT:
			newDir = RT_TALK_UPLEFT;
			break;
		default:
			break;
		}

		// See if the NPC's sequence has to wait for an Abort Talk Code
		if (person.hasAborts()) {
			person._gotoSeq = newDir;
		} else {
			if (person._seqTo) {
				// Reset to previous value
				person._walkSequences[person._sequenceNumber]._sequences[person._frameNumber] = person._seqTo;
				person._seqTo = 0;
			}

			person._sequenceNumber = newDir;
			person._frameNumber = 0;
			person.checkWalkGraphics();
		}
	}
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

OpcodeReturn TattooTalk::cmdNextSong(const byte *&str) {
	Sound &sound = *_vm->_sound;

	// Get the name of the next song to play
	++str;
	sound._nextSongName = "";
	for (int idx = 0; idx < 8; ++idx) {
		if (str[idx] != '~')
			sound._nextSongName += str[idx];
		else
			break;
	}
	str += 7;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdNPCLabelGoto(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdNPCLabelIfFlagGoto(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdNPCLabelSet(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdPassword(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdPlaySong(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdRestorePeopleSequence(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCDescOnOff(const byte *&str) { error("TODO: script opcode"); }

OpcodeReturn TattooTalk::cmdSetNPCInfoLine(const byte *&str) {
	int npcNum = *++str;
	int len = *++str;
	People &people = *_vm->_people;
	Person &person = people[npcNum];

	for (int x = 0; x < len; x++)
		person._description.setChar(str[x + 1], x);
	person._description.setChar(0, len);
	str += len;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCOff(const byte *&str) {
	People &people = *_vm->_people;
	int npcNum = *++str;
	people[npcNum]._type = REMOVE;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCOn(const byte *&str) { 
	People &people = *_vm->_people;
	int npcNum = *++str;
	people[npcNum]._type = CHARACTER;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCPathDest(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCPathPause(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCPathPauseTakingNotes(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCPathPauseLookingHolmes(const byte *&str) { error("TODO: script opcode"); }

OpcodeReturn TattooTalk::cmdSetNPCPosition(const byte *&str) {
	int npcNum = *++str - 1;
	++str;
	People &people = *_vm->_people;
	Person &person = people[npcNum];
	int32 posX = (str[0] - 1) * 256 + str[1] - 1;
	if (posX > 16384)
		posX = -1 * (posX - 16384);
	int32 posY = (str[2] - 1) * 256 + str[3] - 1;
	
	people[npcNum]._position = Point32(posX * 1000, posY * 1000);
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

OpcodeReturn TattooTalk::cmdSetNPCTalkFile(const byte *&str) { error("TODO: script opcode"); }

OpcodeReturn TattooTalk::cmdSetNPCVerb(const byte *&str) {
	int npcNum = *++str;
	int verbNum = *++str - 1;
	People &people = *_vm->_people;
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
	People &people = *_vm->_people;
	UseType &useType = people[npcNum]._use[verbNum];

	useType._cAnimNum = (str[1] - 1) & 127;
	useType._cAnimSpeed = 1 + 128 * (str[1] >= 128);
	str++;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetNPCVerbScript(const byte *&str) {
	int npcNum = *++str;
	int verbNum = *++str - 1;
	People &people = *_vm->_people;
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
	People &people = *_vm->_people;
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
	People &people = *_vm->_people;
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

OpcodeReturn TattooTalk::cmdSetSceneEntryFlag(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetTalkSequence(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetWalkControl(const byte *&str) { error("TODO: script opcode"); }

// Dummy opcode
OpcodeReturn TattooTalk::cmdTalkInterruptsDisable(const byte *&str) { error("Dummy opcode cmdTalkInterruptsDisable called"); }

// Dummy opcode
OpcodeReturn TattooTalk::cmdTalkInterruptsEnable(const byte *&str) { error("Dummy opcode cmdTalkInterruptsEnable called"); }

OpcodeReturn TattooTalk::cmdTurnSoundsOff(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdWalkHolmesAndNPCToCAnimation(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdWalkNPCToCAnimation(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdWalkNPCToCoords(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdWalkHomesAndNPCToCoords(const byte *&str) { error("TODO: script opcode"); }

} // End of namespace Tattoo

} // End of namespace Sherlock
