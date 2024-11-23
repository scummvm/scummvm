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

#include "common/config-manager.h"
#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/file.h"
#include "scumm/he/intern_he.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/insane/insane.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v6.h"
#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v6, x)

void ScummEngine_v6::setupOpcodes() {
	/* 00 */
	OPCODE(0x00, o6_pushByte);
	OPCODE(0x01, o6_pushWord);
	OPCODE(0x02, o6_pushByteVar);
	OPCODE(0x03, o6_pushWordVar);
	/* 04 */
	OPCODE(0x06, o6_byteArrayRead);
	OPCODE(0x07, o6_wordArrayRead);
	/* 08 */
	OPCODE(0x0a, o6_byteArrayIndexedRead);
	OPCODE(0x0b, o6_wordArrayIndexedRead);
	/* 0C */
	OPCODE(0x0c, o6_dup);
	OPCODE(0x0d, o6_not);
	OPCODE(0x0e, o6_eq);
	OPCODE(0x0f, o6_neq);
	/* 10 */
	OPCODE(0x10, o6_gt);
	OPCODE(0x11, o6_lt);
	OPCODE(0x12, o6_le);
	OPCODE(0x13, o6_ge);
	/* 14 */
	OPCODE(0x14, o6_add);
	OPCODE(0x15, o6_sub);
	OPCODE(0x16, o6_mul);
	OPCODE(0x17, o6_div);
	/* 18 */
	OPCODE(0x18, o6_land);
	OPCODE(0x19, o6_lor);
	OPCODE(0x1a, o6_pop);
	/* 1C */
	/* 20 */
	/* 24 */
	/* 28 */
	/* 2C */
	/* 30 */
	/* 34 */
	/* 38 */
	/* 3C */
	/* 40 */
	OPCODE(0x42, o6_writeByteVar);
	OPCODE(0x43, o6_writeWordVar);
	/* 44 */
	OPCODE(0x46, o6_byteArrayWrite);
	OPCODE(0x47, o6_wordArrayWrite);
	/* 48 */
	OPCODE(0x4a, o6_byteArrayIndexedWrite);
	OPCODE(0x4b, o6_wordArrayIndexedWrite);
	/* 4C */
	OPCODE(0x4e, o6_byteVarInc);
	OPCODE(0x4f, o6_wordVarInc);
	/* 50 */
	OPCODE(0x52, o6_byteArrayInc);
	OPCODE(0x53, o6_wordArrayInc);
	/* 54 */
	OPCODE(0x56, o6_byteVarDec);
	OPCODE(0x57, o6_wordVarDec);
	/* 58 */
	OPCODE(0x5a, o6_byteArrayDec);
	OPCODE(0x5b, o6_wordArrayDec);
	/* 5C */
	OPCODE(0x5c, o6_if);
	OPCODE(0x5d, o6_ifNot);
	OPCODE(0x5e, o6_startScript);
	OPCODE(0x5f, o6_startScriptQuick);
	/* 60 */
	OPCODE(0x60, o6_startObject);
	OPCODE(0x61, o6_drawObject);
	OPCODE(0x62, o6_drawObjectAt);
	OPCODE(0x63, o6_drawBlastObject);
	/* 64 */
	OPCODE(0x64, o6_setBlastObjectWindow);
	OPCODE(0x65, o6_stopObjectCode);
	OPCODE(0x66, o6_stopObjectCode);
	OPCODE(0x67, o6_endCutscene);
	/* 68 */
	OPCODE(0x68, o6_cutscene);
	OPCODE(0x69, o6_stopMusic);
	OPCODE(0x6a, o6_freezeUnfreeze);
	OPCODE(0x6b, o6_cursorCommand);
	/* 6C */
	OPCODE(0x6c, o6_breakHere);
	OPCODE(0x6d, o6_ifClassOfIs);
	OPCODE(0x6e, o6_setClass);
	OPCODE(0x6f, o6_getState);
	/* 70 */
	OPCODE(0x70, o6_setState);
	OPCODE(0x71, o6_setOwner);
	OPCODE(0x72, o6_getOwner);
	OPCODE(0x73, o6_jump);
	/* 74 */
	OPCODE(0x74, o6_startSound);
	OPCODE(0x75, o6_stopSound);
	OPCODE(0x76, o6_startMusic);
	OPCODE(0x77, o6_stopObjectScript);
	/* 78 */
	OPCODE(0x78, o6_panCameraTo);
	OPCODE(0x79, o6_actorFollowCamera);
	OPCODE(0x7a, o6_setCameraAt);
	OPCODE(0x7b, o6_loadRoom);
	/* 7C */
	OPCODE(0x7c, o6_stopScript);
	OPCODE(0x7d, o6_walkActorToObj);
	OPCODE(0x7e, o6_walkActorTo);
	OPCODE(0x7f, o6_putActorAtXY);
	/* 80 */
	OPCODE(0x80, o6_putActorAtObject);
	OPCODE(0x81, o6_faceActor);
	OPCODE(0x82, o6_animateActor);
	OPCODE(0x83, o6_doSentence);
	/* 84 */
	OPCODE(0x84, o6_pickupObject);
	OPCODE(0x85, o6_loadRoomWithEgo);
	OPCODE(0x87, o6_getRandomNumber);
	/* 88 */
	OPCODE(0x88, o6_getRandomNumberRange);
	OPCODE(0x8a, o6_getActorMoving);
	OPCODE(0x8b, o6_isScriptRunning);
	/* 8C */
	OPCODE(0x8c, o6_getActorRoom);
	OPCODE(0x8d, o6_getObjectX);
	OPCODE(0x8e, o6_getObjectY);
	OPCODE(0x8f, o6_getObjectOldDir);
	/* 90 */
	OPCODE(0x90, o6_getActorWalkBox);
	OPCODE(0x91, o6_getActorCostume);
	OPCODE(0x92, o6_findInventory);
	OPCODE(0x93, o6_getInventoryCount);
	/* 94 */
	OPCODE(0x94, o6_getVerbFromXY);
	OPCODE(0x95, o6_beginOverride);
	OPCODE(0x96, o6_endOverride);
	OPCODE(0x97, o6_setObjectName);
	/* 98 */
	OPCODE(0x98, o6_isSoundRunning);
	OPCODE(0x99, o6_setBoxFlags);
	OPCODE(0x9a, o6_createBoxMatrix);
	OPCODE(0x9b, o6_resourceRoutines);
	/* 9C */
	OPCODE(0x9c, o6_roomOps);
	OPCODE(0x9d, o6_actorOps);
	OPCODE(0x9e, o6_verbOps);
	OPCODE(0x9f, o6_getActorFromXY);
	/* A0 */
	OPCODE(0xa0, o6_findObject);
	OPCODE(0xa1, o6_pseudoRoom);
	OPCODE(0xa2, o6_getActorElevation);
	OPCODE(0xa3, o6_getVerbEntrypoint);
	/* A4 */
	OPCODE(0xa4, o6_arrayOps);
	OPCODE(0xa5, o6_saveRestoreVerbs);
	OPCODE(0xa6, o6_drawBox);
	OPCODE(0xa7, o6_pop);
	/* A8 */
	OPCODE(0xa8, o6_getActorWidth);
	OPCODE(0xa9, o6_wait);
	OPCODE(0xaa, o6_getActorScaleX);
	OPCODE(0xab, o6_getActorAnimCounter);
	/* AC */
	OPCODE(0xac, o6_soundKludge);
	OPCODE(0xad, o6_isAnyOf);
	OPCODE(0xae, o6_systemOps);
	OPCODE(0xaf, o6_isActorInBox);
	/* B0 */
	OPCODE(0xb0, o6_delay);
	OPCODE(0xb1, o6_delaySeconds);
	OPCODE(0xb2, o6_delayMinutes);
	OPCODE(0xb3, o6_stopSentence);
	/* B4 */
	OPCODE(0xb4, o6_printLine);
	OPCODE(0xb5, o6_printText);
	OPCODE(0xb6, o6_printDebug);
	OPCODE(0xb7, o6_printSystem);
	/* B8 */
	OPCODE(0xb8, o6_printActor);
	OPCODE(0xb9, o6_printEgo);
	OPCODE(0xba, o6_talkActor);
	OPCODE(0xbb, o6_talkEgo);
	/* BC */
	OPCODE(0xbc, o6_dimArray);
	OPCODE(0xbd, o6_dummy);
	OPCODE(0xbe, o6_startObjectQuick);
	OPCODE(0xbf, o6_startScriptQuick2);
	/* C0 */
	OPCODE(0xc0, o6_dim2dimArray);
	/* C4 */
	OPCODE(0xc4, o6_abs);
	OPCODE(0xc5, o6_distObjectObject);
	OPCODE(0xc6, o6_distObjectPt);
	OPCODE(0xc7, o6_distPtPt);
	/* C8 */
	OPCODE(0xc8, o6_kernelGetFunctions);
	OPCODE(0xc9, o6_kernelSetFunctions);
	OPCODE(0xca, o6_delayFrames);
	OPCODE(0xcb, o6_pickOneOf);
	/* CC */
	OPCODE(0xcc, o6_pickOneOfDefault);
	OPCODE(0xcd, o6_stampObject);
	/* D0 */
	OPCODE(0xd0, o6_getDateTime);
	OPCODE(0xd1, o6_stopTalking);
	OPCODE(0xd2, o6_getAnimateVariable);
	/* D4 */
	OPCODE(0xd4, o6_shuffle);
	OPCODE(0xd5, o6_jumpToScript);
	OPCODE(0xd6, o6_band);
	OPCODE(0xd7, o6_bor);
	/* D8 */
	OPCODE(0xd8, o6_isRoomScriptRunning);
	/* DC */
	OPCODE(0xdd, o6_findAllObjects);
	/* E0 */
	OPCODE(0xe1, o6_getPixel);
	OPCODE(0xe3, o6_pickVarRandom);
	/* E4 */
	OPCODE(0xe4, o6_setBoxSet);
	/* E8 */
	/* EC */
	OPCODE(0xec, o6_getActorLayer);
	OPCODE(0xed, o6_getObjectNewDir);
}

int ScummEngine_v6::popRoomAndObj(int *room) {
	int obj;

	if (_game.version >= 7) {
		obj = pop();
		*room = getObjectRoom(obj);
	} else {
		*room = pop();
		obj = pop();
	}

	return obj;
}

byte *ScummEngine_v6::defineArray(int array, int type, int dim2, int dim1) {
	int id;
	int size;
	ArrayHeader *ah;

	assert(0 <= type && type <= 5);


	if (_game.heversion >= 61) {
		if (type == kBitArray || type == kNibbleArray)
			type = kByteArray;
	} else {
		// NOTE: The following code turns all arrays except string arrays into
		// integer arrays. There seems to be no reason for this, and it wastes
		// space. However, we can't just remove this either, as that would
		// break savegame compatibility. So do not touch this unless you are
		// also adding code which updates old savegames, too. And of course
		// readArray() and writeArray() would have to be updated, too...
		if (type != kStringArray)
			type = kIntArray;
	}

	nukeArray(array);

	id = findFreeArrayId();

	if (_game.version == 8) {
		if (array & 0x40000000) {
		}

		if (array & 0x80000000) {
			error("Can't define bit variable as array pointer");
		}

		size = (type == kIntArray) ? 4 : 1;
	} else {
		if (array & 0x4000) {
		}

		if (array & 0x8000) {
			error("Can't define bit variable as array pointer");
		}

		size = (type == kIntArray) ? 2 : 1;
	}

	writeVar(array, id);

	size *= dim2 + 1;
	size *= dim1 + 1;

	ah = (ArrayHeader *)_res->createResource(rtString, id, size + sizeof(ArrayHeader));

	ah->type = TO_LE_16(type);
	ah->dim1 = TO_LE_16(dim1 + 1);
	ah->dim2 = TO_LE_16(dim2 + 1);

	return ah->data;
}

void ScummEngine_v6::nukeArray(int a) {
	int data;

	data = readVar(a);

	if (_game.heversion >= 80)
		data &= ~MAGIC_ARRAY_NUMBER;

	if (data)
		_res->nukeResource(rtString, data);
	if (_game.heversion >= 60)
		_arraySlot[data] = 0;

	writeVar(a, 0);
}

int ScummEngine_v6::findFreeArrayId() {
	const ResourceManager::ResTypeData &rtd = _res->_types[rtString];
	int i;

	for (i = 1; i < _numArray; i++) {
		if (!rtd[i]._address)
			return i;
	}
	error("Out of array pointers, %d max", _numArray);
	return -1;
}

#define SWAP16(x)  x = SWAP_BYTES_16(x)

ScummEngine_v6::ArrayHeader *ScummEngine_v6::getArray(int array) {
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));
	if (!ah)
		return nullptr;

	if (_game.heversion == 0) {
		// Workaround for a long standing bug where we saved array headers in native
		// endianness, instead of a fixed endianness. We now always store the
		// dimensions in little endian byte order. But to stay compatible with older
		// savegames, we try to detect savegames which were created on a big endian
		// system and convert them to the proper little endian format on the fly.
		if ((FROM_LE_16(ah->dim1) & 0xF000) || (FROM_LE_16(ah->dim2) & 0xF000) || (FROM_LE_16(ah->type) & 0xFF00)) {
			SWAP16(ah->dim1);
			SWAP16(ah->dim2);
			SWAP16(ah->type);
		}
	}

	return ah;
}

int ScummEngine_v6::readArray(int array, int idx, int base) {
	ArrayHeader *ah = getArray(array);

	if (!ah)
		error("readArray: invalid array %d (%d)", array, readVar(array));

	// WORKAROUND bug #600. This is clearly a script bug, as this script
	// excerpt shows nicely:
	// ...
	// [03A7] (5D)         if (isAnyOf(array-447[localvar13][localvar14],[0,4])) {
	// [03BD] (5D)           if ((localvar13 != -1) && (localvar14 != -1)) {
	// [03CF] (B6)             printDebug.begin()
	// ...
	// So it checks for invalid array indices only *after* using them to access
	// the array. Ouch.
	//
	// TODO: what did the original interpreter precisely do in this case?
	if (_game.id == GID_FT && array == 447 && _currentRoom == 95 && vm.slot[_currentScript].number == 2010 && idx == -1 && base == -1 &&
		enhancementEnabled(kEnhGameBreakingBugFixes)) {
		return 0;
	}

	const int offset = base + idx * FROM_LE_16(ah->dim1);

	if (offset < 0 || offset >= FROM_LE_16(ah->dim1) * FROM_LE_16(ah->dim2)) {
		error("readArray: array %d out of bounds: [%d,%d] exceeds [%d,%d]",
			array, base, idx, FROM_LE_16(ah->dim1), FROM_LE_16(ah->dim2));
	}

	int val;
	if (FROM_LE_16(ah->type) != kIntArray) {
		val = ah->data[offset];
	} else if (_game.version == 8) {
		val = (int32)READ_LE_UINT32(ah->data + offset * 4);
	} else {
		val = (int16)READ_LE_UINT16(ah->data + offset * 2);
	}
	return val;
}

void ScummEngine_v6::writeArray(int array, int idx, int base, int value) {
	ArrayHeader *ah = getArray(array);
	if (!ah)
		return;

	const int offset = base + idx * FROM_LE_16(ah->dim1);

	if (offset < 0 || offset >= FROM_LE_16(ah->dim1) * FROM_LE_16(ah->dim2)) {
		error("writeArray: array %d out of bounds: [%d,%d] exceeds [%d,%d]",
			array, base, idx, FROM_LE_16(ah->dim1), FROM_LE_16(ah->dim2));
	}

	if (FROM_LE_16(ah->type) != kIntArray) {
		ah->data[offset] = value;
	} else if (_game.version == 8) {
		WRITE_LE_UINT32(ah->data + offset * 4, value);
	} else {
		WRITE_LE_UINT16(ah->data + offset * 2, value);
	}
}

void ScummEngine_v6::readArrayFromIndexFile() {
	int num;
	int a, b, c;

	while ((num = _fileHandle->readUint16LE()) != 0) {
		a = _fileHandle->readUint16LE();
		b = _fileHandle->readUint16LE();
		c = _fileHandle->readUint16LE();
		if (c == kBitArray)
			defineArray(num, kBitArray, a, b);
		else
			defineArray(num, kIntArray, a, b);
	}
}

int ScummEngine_v6::getStackList(int *args, uint maxnum) {
	uint num, i;

	for (i = 0; i < maxnum; i++)
		args[i] = 0;

	num = pop();

	if (num > maxnum)
		error("Too many items %d in stack list, max %d", num, maxnum);

	i = num;
	while (i--) {
		args[i] = pop();
	}

	return num;
}

void ScummEngine_v6::o6_pushByte() {
	push(fetchScriptByte());
}

void ScummEngine_v6::o6_pushWord() {
	push(fetchScriptWordSigned());
}

void ScummEngine_v6::o6_pushByteVar() {
	push(readVar(fetchScriptByte()));
}

void ScummEngine_v6::o6_pushWordVar() {
// BACKYARD BASEBALL 2001 ONLINE CHANGES
#if defined(USE_ENET) && defined(USE_LIBCURL)
	if (_enableHECompetitiveOnlineMods) {
		// Sprinting in competitive Backyard Baseball is considered too weak in its current state. This will increase how effective
		// it is, limiting the highest speed characters enough to where they cannot go TOO fast.
		if (_game.id == GID_BASEBALL2001 && _currentRoom == 3 && vm.slot[_currentScript].number == 2095 && readVar(399) == 1) {
			int offset = _scriptPointer - _scriptOrgPointer;
			int sprintCounter = readArray(344, vm.localvar[_currentScript][0], 1);
			int sprintGain = vm.localvar[_currentScript][4];
			int playerSpeed = vm.localvar[_currentScript][5];
			if (offset == 42273) {
				if (sprintCounter >= 21) {
					if (playerSpeed >= 8) {
						sprintGain = 2;
					} else if (playerSpeed == 6 || playerSpeed == 7) {
						sprintGain = 3;
					} else {
						sprintGain = 4;
					}
				} else if (sprintCounter >= 15) {
					if (playerSpeed >= 6) {
						sprintGain = 2;
					} else {
						sprintGain = 3;
					}
				} else if (sprintCounter >= 9) {
						sprintGain = 2;
				} else {
					sprintGain = 1;
				}
				writeVar(0x4000 + 4, sprintGain);
			}
		}

		// This code will change the velocity of the hit based on the pitch thrown, and the location of the pitch itself.
		if (_game.id == GID_BASEBALL2001 && _currentRoom == 4 && vm.slot[_currentScript].number == 2090 && readVar(399) == 1) {
			int offset = _scriptPointer - _scriptOrgPointer;
			int powerAdjustment = vm.localvar[_currentScript][4];
			int pitchSelected = readVar(0x8000 + 10);

			// Checks if the swing is either Power or Line Drive
			if (offset == 102789 && (readVar(387) == 1||readVar(387) == 2)) {
				// Checks if the current pitch type is the same as that of the "remembered" pitch type
				if (readArray(346, 0, 0) == readArray(346, 1, 0)) {
					// Checks if the current pitch is either a Heat or a Fireball. The reason it adds 0 instead of 5 is because
					// in the actual calculation it adds 5 to these two anyway, so this should help balance them out.
					if (pitchSelected == 14 || pitchSelected == 21) {
						powerAdjustment = powerAdjustment + 0;
					} else {
						powerAdjustment = powerAdjustment + 5;
					}
				}
				// Checks if the zone location is the same as that of the previous one. This should slightly reduce the amount of pitching to the exact same location.
				// Can also be adjusted later if necessary.
				if (readArray(346, 0, 1) == readArray(346, 1, 1)) {
					powerAdjustment = powerAdjustment + 15;
				}
				// write the power adjustment to the result
				writeVar(0x4000 + 4, powerAdjustment);
			}
		}

		// Remember the previous pitch thrown and the previous pitch "zone location", then set those two values to the "remembered" values for later use.
		if (_game.id == GID_BASEBALL2001 && _currentRoom == 4 && vm.slot[_currentScript].number == 2201 && readVar(399) == 1) {
			writeArray(346, 1, 0, readArray(346, 0, 0));
			writeArray(346, 1, 1, readArray(346, 0, 1));
		}
		// This sets the base cost of a slow ball to 2. Previously it costed the least of every pitch to throw, which resulted in people only using that pitch.
		if (_game.id == GID_BASEBALL2001 && _currentRoom == 4 && vm.slot[_currentScript].number == 2057 && readVar(399) == 1) {
			if (readVar(0x4000 + 1) == 15) {
				writeVar(0x4000 + 2, 2);
			}
		}
	}
#endif
	push(readVar(fetchScriptWord()));
}

void ScummEngine_v6::o6_byteArrayRead() {
	int base = pop();
	push(readArray(fetchScriptByte(), 0, base));
}

void ScummEngine_v6::o6_wordArrayRead() {
	int base = pop();
	int array = fetchScriptWord();
#if defined(USE_ENET) && defined(USE_LIBCURL)
	if (_enableHECompetitiveOnlineMods) {
		// If we're pulling from the randomly selected teams for online play
		// at Prince Rupert, read from variables 748 and 749 instead
		if (_game.id == GID_BASEBALL2001 && _currentRoom == 6 && vm.slot[_currentScript].number == 2071 &&
			readVar(399) == 1 &&  // We're online and in the team name select screen
			readVar(747) == 1) {  // We successfully got team arrays the host and opponent
			switch (array) {
			case 264:
			case 321:
				array = 748;
				break;
			case 265:
			case 322:
				array = 749;
				break;
			}
		}
	}
#endif
	push(readArray(array, 0, base));
}

void ScummEngine_v6::o6_byteArrayIndexedRead() {
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptByte(), idx, base));
}

void ScummEngine_v6::o6_wordArrayIndexedRead() {
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptWord(), idx, base));
}

void ScummEngine_v6::o6_dup() {
	int a = pop();
	push(a);
	push(a);
}

void ScummEngine_v6::o6_not() {
	push(pop() == 0);
}

void ScummEngine_v6::o6_eq() {
	int a = pop();
	int b = pop();

// BACKYARD BASEBALL 2001 ONLINE CHANGES
#if defined(USE_ENET) && defined(USE_LIBCURL)
	// The player stat adjustments that should get applied in certain conditions (i.e. when two siblings are on the same team)
	// don't get applied properly for the away (peer) team in online play. This results in each team's game using a different
	// version of players' stats, leading to unfair play and potential desyncs. This hack ensures the away team's game doesn't
	// exit the script before applying these stat adjustments. The script checks whether the game is being played online before
	// this, such that this code doesn't execute for offline play.
	if (_game.id == GID_BASEBALL2001 && _currentRoom == 27 && vm.slot[_currentScript].number == 2346) {
		int offset = _scriptPointer - _scriptOrgPointer;
		if (offset == 196137) {
			push(0);
			return;
		}
	}

	if (_enableHECompetitiveOnlineMods) {
		int pitchXValue = readVar(0x8000 + 11);
		int pitchYValue = readVar(0x8000 + 12);
		int strikeZoneTop = readVar(0x8000 + 29);
		int strikeZoneBottom = readVar(0x8000 + 30);

		// People have been complaining about strikes being visually unclear during online games. This is because the strike zone's visual is not
		// equal length compared to the actual range in which a strike can be called. These changes should fix that, with some extra leniency in
		// the corners in particular since they are especially difficult to see visually, due to having four large corner pieces blocking the view.

		// This checks if the pitch's y location is either:
		// a. at least 2 pixels lower than the top of the zone/at least 3 pixels above the bottom of the zone
		// b. at least 2 pixels lower than the top of the zone/at least 3 pixels above the bottom of the zone
		// If either of these are true AND the x value is less than or equal to 279 OR greater than or equal to 354, make the game read as a ball.
		// The strike zone should be much more lenient in the corners, as well as removing the small advantage of throwing to the farthest right side of the zone.
		if (_game.id == GID_BASEBALL2001 && _currentRoom == 4 && (vm.slot[_currentScript].number == 2202 || vm.slot[_currentScript].number == 2192) && readVar(399) == 1) {
			if (((pitchYValue <= strikeZoneTop + 2 || pitchYValue >= strikeZoneBottom - 3) && pitchXValue <= 279) ||
				((pitchYValue <= strikeZoneTop + 2 || pitchYValue >= strikeZoneBottom - 3) && pitchXValue >= 354)) {
				writeVar(0x8000 + 16, 2);
			}
			// if the ball's y location is 1 pixel higher than the bottom of the zone, then it will be a ball.
			// This removes the small advantage of throwing at the very bottom of the zone.
			if (pitchYValue > strikeZoneBottom - 1) {
				writeVar(0x8000 + 16, 2);
			}
		}

		// This change affects the angle adjustment for each batting stance when timing your swing. There are complaints that
		// the game does not give you enough control when batting, resulting in a lot of hits going to the same area. This should
		// give players more agency on where they want to hit the ball, which will also increase the skill ceiling.
		if (_game.id == GID_BASEBALL2001 && _currentRoom == 4 && vm.slot[_currentScript].number == 2087 && readVar(399) == 1) {
			int offset = _scriptPointer - _scriptOrgPointer;
			// OPEN STANCE ADJUSTMENTS (1 being earliest, 5 being latest)
			if (offset == 101898 && readVar(447) == 1) {
				switch (readVar(0x8000 + 1)) {
				case 1:
					writeVar(0x4000 + 0, -13);
					break;
				case 2:
					writeVar(0x4000 + 0, -2);
					break;
				case 3:
					writeVar(0x4000 + 0, 10);
					break;
				case 4:
					writeVar(0x4000 + 0, 40);
					break;
				case 5:
					writeVar(0x4000 + 0, 63);
					break;
				}
			}
			// SQUARED STANCE ADJUSTMENTS (1 being earliest, 5 being latest)
			if (offset == 101898 && readVar(447) == 2) {
				switch (readVar(0x8000 + 1)) {
				case 1:
					writeVar(0x4000 + 0, -30);
					break;
				case 2:
					writeVar(0x4000 + 0, -7);
					break;
				case 3:
					writeVar(0x4000 + 0, 10);
					break;
				case 4:
					writeVar(0x4000 + 0, 27);
					break;
				case 5:
					writeVar(0x4000 + 0, 45);
					break;
				}
			}
			// CLOSED STANCE ADJUSTMENTS (1 being earliest, 5 being latest)
			if (offset == 101898 && readVar(447) == 3) {
				switch (readVar(0x8000 + 1)) {
				case 1:
					writeVar(0x4000 + 0, -47);
					break;
				case 2:
					writeVar(0x4000 + 0, -32);
					break;
				case 3:
					writeVar(0x4000 + 0, 0);
					break;
				case 4:
					writeVar(0x4000 + 0, 15);
					break;
				case 5:
					writeVar(0x4000 + 0, 28);
					break;
				}
			}
		}

		// This code makes it so that generic players (and Mr. Clanky) play pro player music when hitting home runs.
		// This is a purely aesthetic change, as they have no home run music by default.
		if (_game.id == GID_BASEBALL2001 && _currentRoom == 3 && vm.slot[_currentScript].number == 11 && vm.localvar[_currentScript][0] > 61 && readVar(399) == 1) {
			// this local variable checks for player ID
			writeVar(0x4000 + 0, 60);
		}
	}
#endif

#if defined(USE_ENET) && defined(USE_LIBCURL)
	int offset = _scriptPointer - _scriptOrgPointer;
	// WORKAROUND: In Backyard Baseball 2001, The special rules of the Mountain Aire and Wilderness neighborhoods
	// are incorrect.  They were set to "3 innings" and "no swing spot" respectively, while they were supposed to be set to
	// "no special rules" and "3 innings".  This is a script bug which assumed to be fixed in later post-retail updates, but
	// since we don't have access to any of those, this workaround will have to do.
	if (_game.id == GID_BASEBALL2001 && vm.slot[_currentScript].number == 419 && ((a == 9 && b == 9) || (a == 8 && b == 8))) {
		switch (a) {
		case 9:
			// Mountain Aire (No special rules)
			writeVar(695, 0);
			break;
		case 8:
			// Wilderness (3 innings)
			writeVar(695, 64);
			break;
		}

		// Clean up stack and stop the script
		fetchScriptWord();
		pop();
		stopObjectCode();

	// HACK: This script doesn't allow Super Colossal Dome to be chosen for online play, by checking if the selected
	// field's value is 5 (SCD's number) and incrementing/decrementing if it is. To allow SCD to be used, we return 0
	// for those checks.
	} else if (_enableHECompetitiveOnlineMods && _game.id == GID_BASEBALL2001 && _currentRoom == 40 &&
		vm.slot[_currentScript].number == 2106 && a == 5 && (offset == 16754 || offset == 16791)) {
		push(0);

	// WORKAROUND: Online play is disabled in the Macintosh versions of Backyard Football and Backyard Baseball 2001
	// because the original U32 makes use of DirectPlay, a Windows exclusive API; we now have our own implementation
	// which is cross-platform compatible.  We get around that by tricking those checks that we are playing on
	// the Windows version. These scripts check VAR_PLATFORM (b) against the value (2) of the Macintosh platform (a).
	} else if (_game.id == GID_FOOTBALL && _currentRoom == 2 && (vm.slot[_currentScript].number == 2049 || vm.slot[_currentScript].number == 2050 ||
#else
	if (_game.id == GID_FOOTBALL && _currentRoom == 2 && (vm.slot[_currentScript].number == 2049 || vm.slot[_currentScript].number == 2050 ||
#endif
		vm.slot[_currentScript].number == 498) && a == 2 && b == 2) {
		push(0);
	} else if (_game.id == GID_BASEBALL2001 && _currentRoom == 2 && (vm.slot[_currentScript].number == 10002 || vm.slot[_currentScript].number == 2050) &&
		a == 2 && b == 2) {
		push(0);
	} else if (_game.id == GID_FOOTBALL2002 && _currentRoom == 3 && vm.slot[_currentScript].number == 2079 &&
		a == 2 && b == 2) {
		push(0);

	// WORKAROUND: Forces the game version string set via script 1 to be used in both Macintosh and Windows versions,
	// when checking for save game compatibility. Allows saved games to be shared between Macintosh and Windows versions.
	// The scripts check VAR_PLATFORM (b) against the value (2) of the Macintosh platform (a).
	} else if (_game.id == GID_BASEBALL2001 && (vm.slot[_currentScript].number == 291 || vm.slot[_currentScript].number == 292) &&
		a == 2 && b == 1) {
		push(1);
	} else {
		push(a == b);
	}
}

void ScummEngine_v6::o6_neq() {
	push(pop() != pop());
}

void ScummEngine_v6::o6_gt() {
	int a = pop();
	int b = pop();

	// WORKAROUND: In Football 2002, when hosting a Network game, it would eventually timeout,
	// which causes the game to stop hosting and query for sessions again.
	//
	// [016C] (39)     localvar8++
	// [016F] (36)     if (localvar8 > localvar12) {
	// [0179] (54)       printDebug.begin()
	// [017B] (54)       printDebug.msg("Host Timeout")
	// [018A] (7C)       startScript(90,220,[])
	// [0190] (7C)       startScript(90,2051,[])
	// [0197] (7C)       startScript(90,2054,[])
	// [019E] (80)       stopScript(0)
	// [01A1] (**)     }
	//
	// We have our own session selection dialog which allows the user to host or join a session as
	// they please; we do not want them to go through the whole setup again after the timeout
	// so let's just make unreachable, allowing the session to be hosted indefinitely until
	// they cancel it out.
	if (_game.id == GID_FOOTBALL2002 && _currentRoom == 3 && vm.slot[_currentScript].number == 2052) {
		push(0);
		return;
	}

	push(b > a);
}

void ScummEngine_v6::o6_lt() {
	int a = pop();
	push(pop() < a);
}

void ScummEngine_v6::o6_le() {
	int a = pop();
	push(pop() <= a);
}

void ScummEngine_v6::o6_ge() {
	int a = pop();
	int b = pop();
#if defined(USE_ENET) && defined(USE_LIBCURL)
	// Mod for Backyard Baseball 2001 online competitive play: Reduce sprints
	// required to reach top speed
	if (_enableHECompetitiveOnlineMods && _game.id == GID_BASEBALL2001 &&
		_currentRoom == 3 && vm.slot[_currentScript].number == 2095 && readVar(399) == 1) {
		a -= 1;  // If sprint counter (b) is higher than a, runner gets 1 extra speed
	}
#endif
	push(b >= a);
}

void ScummEngine_v6::o6_add() {
	int a = pop();
	push(pop() + a);
}

void ScummEngine_v6::o6_sub() {
	int a = pop();
	push(pop() - a);
}

void ScummEngine_v6::o6_mul() {
	int a = pop();
	push(pop() * a);
}

void ScummEngine_v6::o6_div() {
	int a = pop();
	if (a == 0)
		error("division by zero");
	int b = pop();
#if defined(USE_ENET) && defined(USE_LIBCURL)
	// Mod for Backyard Baseball 2001 online competitive play: Allow full sprinting while
	// running half-speed on a popup
	if (_enableHECompetitiveOnlineMods && _game.id == GID_BASEBALL2001 && _currentRoom == 3 &&
		vm.slot[_currentScript].number == 2095 && readVar(399) == 1 && a == 2) {
		// Normally divides speed by two here
		int runnerIdx = readVar(0x4000);
		int runnerReSprint = readArray(344, runnerIdx, 1);
		// But if the runner is sprinting, don't divide by two
		if (runnerReSprint > 1) {
			push(b);
			return;
		}
	}
#endif
	push(b / a);
}

void ScummEngine_v6::o6_land() {
	int a = pop();
	int b = pop();
	// WORKAROUND: When entering an area, the game will check if
	// vars 133 and 134 are set, else it will wait for 5 seconds before
	// showing the coach list.  var133 is set 1 somewhere but var134
	// is always set at 0. I am going to assume this is a script bug,
	// so let's skip the 5 second wait.
	if (_game.id == GID_BASEBALL2001 && _currentRoom == 40 && vm.slot[_currentScript].number == 2122)
		push(1);
	else
		push(b && a);
}

void ScummEngine_v6::o6_lor() {
	int a = pop();
	push(pop() || a);
}

void ScummEngine_v6::o6_bor() {
	int a = pop();
	push(pop() | a);
}

void ScummEngine_v6::o6_band() {
	int a = pop();
	push(pop() & a);
}

void ScummEngine_v6::o6_pop() {
	pop();
}

void ScummEngine_v6::o6_writeByteVar() {
	writeVar(fetchScriptByte(), pop());
}

void ScummEngine_v6::o6_writeWordVar() {
	writeVar(fetchScriptWord(), pop());
}

void ScummEngine_v6::o6_byteArrayWrite() {
	int a = pop();
	writeArray(fetchScriptByte(), 0, pop(), a);
}

void ScummEngine_v6::o6_wordArrayWrite() {
	int a = pop();
	writeArray(fetchScriptWord(), 0, pop(), a);
}

void ScummEngine_v6::o6_byteArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArray(fetchScriptByte(), pop(), base, val);
}

void ScummEngine_v6::o6_wordArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArray(fetchScriptWord(), pop(), base, val);
}

void ScummEngine_v6::o6_byteVarInc() {
	int var = fetchScriptByte();
	writeVar(var, readVar(var) + 1);
}

void ScummEngine_v6::o6_wordVarInc() {
	int var = fetchScriptWord();
	writeVar(var, readVar(var) + 1);
}

void ScummEngine_v6::o6_byteArrayInc() {
	int var = fetchScriptByte();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}

void ScummEngine_v6::o6_wordArrayInc() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}

void ScummEngine_v6::o6_byteVarDec() {
	int var = fetchScriptByte();
	writeVar(var, readVar(var) - 1);
}

void ScummEngine_v6::o6_wordVarDec() {
	int var = fetchScriptWord();
	writeVar(var, readVar(var) - 1);
}

void ScummEngine_v6::o6_byteArrayDec() {
	int var = fetchScriptByte();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void ScummEngine_v6::o6_wordArrayDec() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void ScummEngine_v6::o6_if() {
	if (pop())
		o6_jump();
	else
		fetchScriptWord();
}

void ScummEngine_v6::o6_ifNot() {
	if (!pop())
		o6_jump();
	else
		fetchScriptWord();
}

void ScummEngine_v6::o6_jump() {
	int offset = fetchScriptWordSigned();

	// WORKAROUND bug #6097: Pressing escape at the lake side entrance of
	// the cave while Putt Putt is not on solid ground and still talking
	// will cause the raft to disappear. This is a script bug in the
	// original game and affects several versions.
	if (_game.id == GID_PUTTZOO) {
		if (_game.heversion == 73 && vm.slot[_currentScript].number == 206 && offset == 176 && !isScriptRunning(202))
			_scummVars[244] = 35;
		if (_game.features & GF_HE_985 && vm.slot[_currentScript].number == 2054 && offset == 178 && !isScriptRunning(2050))
			_scummVars[202] = 35;
	}

	// WORKAROUND bug #4464: Talking to the guard at the bigfoot party, after
	// he's let you inside, will cause the game to hang, if you end the conversation.
	// This is a script bug, due to a missing jump in one segment of the script,
	// and it also happens with the original interpreters.
	//
	// Intentionally using `kEnhGameBreakingBugFixes`, since having the game hang
	// is not useful to anyone.
	if (_game.id == GID_SAMNMAX && vm.slot[_currentScript].number == 101 && readVar(0x8000 + 97) == 1 && offset == 1 &&
		enhancementEnabled(kEnhGameBreakingBugFixes)) {
		offset = -18;
	}

	_scriptPointer += offset;

	// WORKAROUND:  When getting the area popuation, the scripts does not break after getting
	// the popuation.  Not only this may slow down the game a bit, it sends quite a bit of bandwidth
	// considering we're outside the game.  So let's break the script for 5 seconds
	// before jumping back to the beginning.
	if ((_game.id == GID_BASEBALL2001 && _currentRoom == 39 && vm.slot[_currentScript].number == 2090 && offset == -904) ||
		(_game.id == GID_BASEBALL2001 && _currentRoom == 40 && vm.slot[_currentScript].number == 2101 && offset == -128)) {
		vm.slot[_currentScript].delay = 5 * 60; // 5 seconds
		vm.slot[_currentScript].status = ssPaused;
		o6_breakHere();
	}
}

void ScummEngine_v6::o6_startScript() {
	int args[25];
	int script, flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = pop();

	// WORKAROUND: In DOTT, when Jefferson builds the fire, `startScript(1,106,[91,5])`
	// is called, which randomly changes the state of the fire object between 1 and 5,
	// as long as Hoagie doesn't exit this room. This makes him randomly fail
	// interacting with it, saying "I can't reach it." instead of the intended "No.
	// Fire bad." line. It looks like the 1-5 states for the fire object are useless
	// leftovers which can be safely ignored in order to make sure that Hoagie's
	// comment is always available (maybe the fire was meant to be displayed
	// differently when it's just been lit, but then the idea was dropped?).
	// This also happens with the original interpreters and with the remaster.
	if (_game.id == GID_TENTACLE && _roomResource == 13 &&
		vm.slot[_currentScript].number == 21 && script == 106 &&
		args[0] == 91 && enhancementEnabled(kEnhRestoredContent)) {
		return;
	}

	// WORKAROUND for a bug also present in the original EXE: After greasing (or oiling?)
	// the cannonballs in the Plunder Town Theater, during the juggling show, the game
	// cuts from room 18 (backstage) to room 19 (stage).
	//
	// Usually, when loading a room script 29 handles the change of background music,
	// based on which room we've just loaded.
	// Unfortunately, during this particular cutscene, script 29 is not executing,
	// therefore the music is unchanged from room 18 to 19 (the muffled backstage
	// version is played), and is not coherent with the drums fill played afterwards
	// (sequence 2225), which is unmuffled.
	//
	// This fix checks for this situation happening (and only this one), and makes a call
	// to a soundKludge operation like script 29 would have done.
	if (_game.id == GID_CMI && _currentRoom == 19 &&
		vm.slot[_currentScript].number == 168 && script == 118 && enhancementEnabled(kEnhAudioChanges)) {
		int list[16] = { 4096, 1278, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		_sound->soundKludge(list, 2);
	}

	// WORKAROUND bug #269: At Dino Bungee National Memorial, the buttons for
	// the Wally and Rex dinosaurs will always restart their speech, instead of
	// stopping and starting their speech. This was a script bug in the original
	// game, which would also block the "That was informative" reaction from Sam.
	if (_game.id == GID_SAMNMAX && _roomResource == 59 &&
		vm.slot[_currentScript].number == 201 && script == 48 && enhancementEnabled(kEnhRestoredContent)) {
		o6_breakHere();
	}

	runScript(script, (flags & 1) != 0, (flags & 2) != 0, args);
}

void ScummEngine_v6::o6_jumpToScript() {
	int args[25];
	int script, flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = pop();
	stopObjectCode();
	runScript(script, (flags & 1) != 0, (flags & 2) != 0, args);
}

void ScummEngine_v6::o6_startScriptQuick() {
	int args[25];
	int script;
	getStackList(args, ARRAYSIZE(args));
	script = pop();
	runScript(script, 0, 0, args);
}

void ScummEngine_v6::o6_startScriptQuick2() {
	int args[25];
	int script;
	getStackList(args, ARRAYSIZE(args));
	script = pop();
#if defined(USE_ENET) && defined(USE_LIBCURL)
	// Mod for Backyard Baseball 2001 online competitive play: change effect of
	// pitch location on hit quality
	if (_enableHECompetitiveOnlineMods && _game.id == GID_BASEBALL2001 && _currentRoom == 4 && script == 2085 && readVar(399) == 1) {
		int zone = _roomVars[2];
		int stance = readVar(447);
		int handedness = _roomVars[0];
		int hitQuality = -2;
		if (stance == 2) {  // Batter is in a squared stance
			switch (zone) {
			case 25:
				hitQuality = 3;
				break;
			case 18: case 24: case 26: case 32:
				hitQuality = 2;
				break;
			case 10: case 11: case 12: case 17: case 19: case 23: case 27: case 31: case 33: case 38: case 39: case 40:
				hitQuality = 1;
				break;
			case 4: case 16: case 20: case 30: case 34: case 46:
				hitQuality = 0;
				break;
			case 3: case 5: case 9: case 13: case 15: case 21: case 22: case 28: case 29: case 35: case 37: case 41: case 45: case 47:
				hitQuality = -1;
				break;
			default:
				break;
			}
			push(hitQuality);
			return;
		}
		if (
			(handedness == 2 && stance == 1)  // Left-handed batter in open stance
			|| (handedness == 1 && stance == 3)  // Right-handed batter in closed stance
		) {
			zone  = ((zone - 1) / 7) * 7 + (6 - ((zone - 1) % 7)) + 1;  // "Flip" zone horizontally across center
		}
		switch (zone) {
		case 24:
			hitQuality = 3;
			break;
		case 17: case 23: case 25: case 31:
			hitQuality = 2;
			break;
		case 9: case 10: case 16: case 18: case 22: case 26: case 30: case 32: case 37: case 38:
			hitQuality = 1;
			break;
		case 3: case 11: case 15: case 19: case 29: case 33: case 39: case 45:
			hitQuality = 0;
			break;
		case 2: case 4: case 8: case 12: case 20: case 27: case 34: case 36: case 40: case 44: case 46:
			hitQuality = -1;
			break;
		default:
			break;
		}
		push(hitQuality);
		return;
	}
#endif
	runScript(script, 0, 1, args);
}

void ScummEngine_v6::o6_startObject() {
	int args[25];
	int script, entryp;
	int flags;
	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	flags = pop();
	runObjectScript(script, entryp, (flags & 1) != 0, (flags & 2) != 0, args);
}

void ScummEngine_v6::o6_startObjectQuick() {
	int args[25];
	int script, entryp;
	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	runObjectScript(script, entryp, 0, 1, args);
}

void ScummEngine_v6::o6_drawObject() {
	int state = pop();
	int obj = pop();

	// This is based on disassembly
	if (state == 0)
		state = 1;

	setObjectState(obj, state, -1, -1);
}

void ScummEngine_v6::o6_drawObjectAt() {
	int y = pop();
	int x = pop();
	int obj = pop();

	// WORKAROUND bug #3487 : Adjust x and y position of
	// objects in credits sequence, to match other ports
	if (_game.id == GID_PUTTMOON && _game.platform == Common::kPlatform3DO &&
		_roomResource == 38 && vm.slot[_currentScript].number == 206) {
		x = y = -1;
	}

	setObjectState(obj, 1, x, y);
}

void ScummEngine_v6::o6_stopObjectCode() {
	stopObjectCode();
}

void ScummEngine_v6::o6_endCutscene() {
	endCutscene();
}

void ScummEngine_v6::o6_cutscene() {
	int args[25];
	getStackList(args, ARRAYSIZE(args));
	beginCutscene(args);
}

void ScummEngine_v6::o6_stopMusic() {
	_sound->stopAllSounds();
}

void ScummEngine_v6::o6_freezeUnfreeze() {
	int a = pop();

	if (a)
		freezeScripts(a);
	else
		unfreezeScripts();
}

void ScummEngine_v6::o6_cursorCommand() {
	int a, i;
	int args[16];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_CURSOR_ON:		// Turn cursor on
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case SO_CURSOR_OFF:		// Turn cursor off
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case SO_USERPUT_ON:
		_userPut = 1;
		break;
	case SO_USERPUT_OFF:
		_userPut = 0;
		break;
	case SO_CURSOR_SOFT_ON:		// Turn soft cursor on
		_cursor.state++;
		if (_cursor.state > 1)
			error("Cursor state greater than 1 in script");
		verbMouseOver(0);
		break;
	case SO_CURSOR_SOFT_OFF:		// Turn soft cursor off
		_cursor.state--;
		verbMouseOver(0);
		break;
	case SO_USERPUT_SOFT_ON:
		_userPut++;
		break;
	case SO_USERPUT_SOFT_OFF:
		_userPut--;
		break;
	case SO_CURSOR_IMAGE:		// Set cursor image
		{
			int room, obj;
			if (_game.heversion >= 70) {
				obj = pop();
				room = getObjectRoom(obj);
			} else {
				obj = popRoomAndObj(&room);
			}

			// Post-load fix for broken SAMNMAX savegames (see bug no. 14467)
			if (_game.id == GID_SAMNMAX && obj == 0 && room == 93)
				break;

			setCursorFromImg(obj, room, 1);
			break;
		}
	case SO_CURSOR_HOTSPOT:		// Set cursor hotspot
		a = pop();
		setCursorHotspot(pop(), a);
		updateCursor();
		break;
	case SO_CHARSET_SET:
		initCharset(pop());
		break;
	case SO_CHARSET_COLOR:
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
		break;
	case SO_CURSOR_TRANSPARENT:		// Set cursor transparent color
		setCursorTransparency(pop());
		break;
	default:
		error("o6_cursorCommand: default case %x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void ScummEngine_v6::o6_breakHere() {
	updateScriptPtr();
	_currentScript = 0xFF;
}

void ScummEngine_v6::o6_ifClassOfIs() {
	int args[16];
	int num, obj, cls;
	bool b;
	int cond = 1;

	num = getStackList(args, ARRAYSIZE(args));
	obj = pop();

	if (_game.heversion >= 80 && num == 0) {
		push(_classData[obj]);
		return;
	}

	while (--num >= 0) {
		cls = args[num];
		b = getClass(obj, cls);
		if ((cls & 0x80 && !b) || (!(cls & 0x80) && b))
			cond = 0;
	}
	push(cond);
}

void ScummEngine_v6::o6_setClass() {
	int args[16];
	int num, obj, cls;

	num = getStackList(args, ARRAYSIZE(args));
	obj = pop();

	while (--num >= 0) {
		cls = args[num];
		if (cls == 0)
			_classData[num] = 0;
		else if (cls & 0x80)
			putClass(obj, cls, 1);
		else
			putClass(obj, cls, 0);
	}
}

void ScummEngine_v6::o6_getState() {
	push(getState(pop()));
}

void ScummEngine_v6::o6_setState() {
	int state = pop();
	int obj = pop();

	putState(obj, state);
	markObjectRectAsDirty(obj);
	if (_bgNeedsRedraw)
		clearDrawObjectQueue();
}

void ScummEngine_v6::o6_setOwner() {
	int owner = pop();
	int obj = pop();
	setOwnerOf(obj, owner);
}

void ScummEngine_v6::o6_getOwner() {
	push(getOwner(pop()));
}

void ScummEngine_v6::o6_startSound() {
	int offset = 0;

	// In Fatty Bear's Birthday Surprise the piano uses offsets 1 - 23 to
	// indicate which note to play, but only when using the standard piano
	// sound. See also o60_soundOps()
	if (_game.heversion >= 60 && (_game.id != GID_PUTTDEMO))
		offset = pop();

#ifdef ENABLE_SCUMM_7_8
	if (_game.version >= 7)
		_imuseDigital->startSfx(pop(), 64);
	else
#endif
		_sound->startSound(pop(), offset);
}

void ScummEngine_v6::o6_stopSound() {
	_sound->stopSound(pop());
}

void ScummEngine_v6::o6_startMusic() {
	if (_game.version >= 7)
		error("o6_startMusic() It shouldn't be called here for imuse digital");

	_sound->startSound(pop());
}

void ScummEngine_v6::o6_stopObjectScript() {
	stopObjectScript(pop());
}

void ScummEngine_v6::o6_panCameraTo() {
	if (_game.version >= 7) {
		int y = pop();
		int x = pop();
		panCameraTo(x, y);
	} else {
		panCameraTo(pop(), 0);
	}
}

void ScummEngine_v6::o6_actorFollowCamera() {
	if (_game.version >= 7)
		setCameraFollows(derefActor(pop(), "actorFollowCamera"));
	else
		actorFollowCamera(pop());
}

void ScummEngine_v6::o6_setCameraAt() {
	if (_game.version >= 7) {
		int x, y;

		camera._follows = 0;

		y = pop();
		x = pop();

		setCameraAt(x, y);
	} else {
		setCameraAtEx(pop());
	}
}

void ScummEngine_v6::o6_loadRoom() {
	int room = pop();

	// WORKAROUND bug #13378: During Sam's reactions to Max beating up the
	// scientist in the intro, we sometimes have to slow down animations
	// artificially. This is where we speed them back up again.
	if (_game.id == GID_SAMNMAX && vm.slot[_currentScript].number == 65 && room == 6 && enhancementEnabled(kEnhTimingChanges)) {
		int actors[] = { 2, 3, 10 };

		for (int i = 0; i < ARRAYSIZE(actors); i++) {
			Actor *a = derefActorSafe(actors[i], "o6_animateActor");
			if (a && a->getAnimSpeed() > 0)
				a->setAnimSpeed(0);
		}
	}

	startScene(room, nullptr, 0);
	if (_game.heversion >= 61) {
		setCameraAt(camera._cur.x, 0);
	}
	_fullRedraw = true;
}

void ScummEngine_v6::o6_stopScript() {
	int script = pop();
	if (script == 0)
		stopObjectCode();
	else
		stopScript(script);
}

void ScummEngine_v6::o6_walkActorToObj() {
	int act, obj, dist;
	Actor *a, *a2;
	int x, y;

	dist = pop();
	obj = pop();
	act = pop();
	a = derefActor(act, "o6_walkActorToObj");

	if (obj >= _numActors) {
		int wio = whereIsObject(obj);

		if (wio != WIO_FLOBJECT && wio != WIO_ROOM)
			return;

		int dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	} else {
		if (!isValidActor(obj))
			return;

		a2 = derefActor(obj, "o6_walkActorToObj(2)");

		if (!a->isInCurrentRoom() || !a2->isInCurrentRoom())
			return;
		if (dist == 0) {
			dist = a2->_scalex * a2->_width / 0xFF;
			dist += dist / 2;
		}
		x = a2->getPos().x;
		y = a2->getPos().y;
		if (x < a->getPos().x)
			x += dist;
		else
			x -= dist;
		a->startWalkActor(x, y, -1);
	}
}

void ScummEngine_v6::o6_walkActorTo() {
	int x, y;
	y = pop();
	x = pop();
	Actor *a = derefActor(pop(), "o6_walkActorTo");
	a->startWalkActor(x, y, -1);
}

void ScummEngine_v6::o6_putActorAtXY() {
	int room, x, y, act;
	Actor *a;

	room = pop();
	y = pop();
	x = pop();
	act = pop();
	a = derefActor(act, "o6_putActorAtXY");

	if (room == 0xFF || room == 0x7FFFFFFF) {
		room = a->_room;
	} else {
		if (a->_visible && _currentRoom != room && getTalkingActor() == a->_number) {
			stopTalk();
		}
		if (room != 0)
			a->_room = room;
	}
	a->putActor(x, y, room);
}


void ScummEngine_v6::o6_putActorAtObject() {
	int room, obj, x, y;
	Actor *a;

	obj = popRoomAndObj(&room);

	a = derefActor(pop(), "o6_putActorAtObject");
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		getObjectXYPos(obj, x, y);
	} else {
		x = 160;
		y = 120;
	}
	if (room == 0xFF)
		room = a->_room;
	a->putActor(x, y, room);
}

void ScummEngine_v6::o6_faceActor() {
	int obj = pop();
	Actor *a = derefActor(pop(), "o6_faceActor");
	a->faceToObject(obj);
}

void ScummEngine_v6::o6_animateActor() {
	int anim = pop();
	int act = pop();

	if (_game.id == GID_SAMNMAX && _roomResource == 35 && vm.slot[_currentScript].number == 202 &&
		act == 4 && anim == 14 && enhancementEnabled(kEnhMinorBugFixes)) {
		// WORKAROUND bug #2068 (Animation glitch at World of Fish).
		// Before starting animation 14 of the fisherman, make sure he isn't
		// talking anymore, otherwise the fishing line may appear twice when Max
		// grabs it and subtitles (at a slow speed) and voices are both enabled.
		// This bug exists in the original game as well.
		if (getTalkingActor() == 4) {
			stopTalk();
		}
	}

	if (_game.id == GID_SAMNMAX && _roomResource == 47 && vm.slot[_currentScript].number == 202 &&
		act == 2 && anim == 249 && enhancementEnabled(kEnhMinorBugFixes)) {
		// WORKAROUND for bug #3832: parts of Bruno are left on the screen when he
		// escapes Bumpusville with Trixie. Bruno (act. 11) and Trixie (act. 12) are
		// properly removed from the scene by the script, but not the combined actor
		// which is used by this animation (act. 6).
		Actor *a = derefActorSafe(6, "o6_animateActor");
		if (a && a->_costume == 243)
			a->putActor(0, 0, 0);
	}

	// Since there have been cases of the scripts sending garbage data
	// as the actor number (see bug #813), we handle these cases cleanly
	// by not crashing ScummVM and returning a nullptr Actor instead.
	Actor *a = derefActorSafe(act, "o6_animateActor");
	if (a) {
		a->animateActor(anim);
	}
}

void ScummEngine_v6::o6_doSentence() {
	int verb, objectA, objectB;

	objectB = pop();
	if (_game.version < 8)
		pop();	// dummy pop (in Sam&Max, seems to be always 0 or 130)
	objectA = pop();
	verb = pop();

	doSentence(verb, objectA, objectB);
}

void ScummEngine_v6::o6_pickupObject() {
	int obj, room;
	int i;

	obj = popRoomAndObj(&room);
	if (room == 0)
		room = _roomResource;

	for (i = 0; i < _numInventory; i++) {
		if (_inventory[i] == (uint16)obj) {
			putOwner(obj, VAR(VAR_EGO));
			runInventoryScript(obj);
			return;
		}
	}

	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
	runInventoryScript(obj);
}

void ScummEngine_v6::o6_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y;

	y = pop();
	x = pop();

	obj = popRoomAndObj(&room);

	a = derefActor(VAR(VAR_EGO), "o6_loadRoomWithEgo");
	a->putActor(0, 0, room);
	_egoPositioned = false;

	VAR(VAR_WALKTO_OBJ) = obj;
	startScene(a->_room, a, obj);
	VAR(VAR_WALKTO_OBJ) = 0;

	if (_game.version == 6) {
		camera._cur.x = camera._dest.x = a->getPos().x;
		setCameraFollows(a, (_game.heversion >= 60));
	}

	_fullRedraw = true;

	if (x != -1 && x != 0x7FFFFFFF) {
		a->startWalkActor(x, y, -1);
	}
}

void ScummEngine_v6::o6_getRandomNumber() {
	int rnd = _rnd.getRandomNumber(0x7fff);
	rnd = rnd % (pop() + 1);
	if (VAR_RANDOM_NR != 0xFF)
		VAR(VAR_RANDOM_NR) = rnd;
	push(rnd);

	debug(6, "o6_getRandomNumber(): %d", rnd);
}

void ScummEngine_v6::o6_getRandomNumberRange() {
	int max = pop();
	int min = pop();
	int rnd = _rnd.getRandomNumber(0x7fff);
	rnd = min + (rnd % (max - min + 1));
#if defined(USE_ENET) && defined(USE_LIBCURL)
	if (_enableHECompetitiveOnlineMods) {
		// For using predefined teams in Prince Rupert, instead of choosing player IDs randomly
		// let's pull from the variables that contain the teams
		if (_game.id == GID_BASEBALL2001 && vm.slot[_currentScript].number == 298 &&
			readVar(399) == 1 && readVar(747) == 1) {
			int offset = _scriptPointer - _scriptOrgPointer;
			if (offset == 117) {
				// Host's team
				rnd = readArray(748, 0, vm.localvar[_currentScript][1]);
			} else if (offset == 210) {
				// Opponent's team
				rnd = readArray(749, 0, vm.localvar[_currentScript][1]);
			}
		}
		// Mod for Backyard Football online competitive play: allow all 38 backyard kids and pros
		// to be drafted in an online game. This controls how many kids are shown in the bleachers
		// when drafting. Without this mod, a random selection of between 31 and 35 kids are shown.
		if (_game.id == GID_FOOTBALL && readVar(465) == 1 && _currentRoom == 5 && vm.slot[_currentScript].number == 2107) {
			rnd = 38;
		}
	}
#endif
	if (VAR_RANDOM_NR != 0xFF)
		VAR(VAR_RANDOM_NR) = rnd;
	push(rnd);

	debug(6, "o6_getRandomNumberRange(): %d (min: %d, max: %d)", rnd, min, max);
}

void ScummEngine_v6::o6_isScriptRunning() {
	push(isScriptRunning(pop()));
}

void ScummEngine_v6::o6_isRoomScriptRunning() {
	push(isRoomScriptRunning(pop()));
}

void ScummEngine_v6::o6_getActorMoving() {
	Actor *a = derefActor(pop(), "o6_getActorMoving");
	push(a->_moving);
}

void ScummEngine_v6::o6_getActorRoom() {
	int act = pop();

	if (act == 0) {
		// This case occurs at the very least in COMI. That's because in COMI's script 28,
		// there is a check which looks as follows:
		//   if (((VAR_TALK_ACTOR != 0) && (VAR_HAVE_MSG == 1)) &&
		//        (getActorRoom(VAR_TALK_ACTOR) == VAR_ROOM))
		// Due to the way this is represented in bytecode, the engine cannot
		// short circuit. Hence, even though this would be perfectly fine code
		// in C/C++, here it can (and does) lead to getActorRoom(0) being
		// invoked. We silently ignore this.
		push(0);
		return;
	}

	if (act == 255) {
		// This case also occurs in COMI...
		push(0);
		return;
	}

	Actor *a = derefActorSafe(act, "o6_getActorRoom");
	// This check is in place because at least Full Throttle, despite
	// only allowing 30 actors, might ask for actor 30 (0-indexed), which
	// on the original went on to fetch garbage data without crashing.
	// In our case, instead of erroring out, we handle the issue gracefully.
	if (a)
		push(a->_room);
	else
		push(0);
}

void ScummEngine_v6::o6_getActorWalkBox() {
	Actor *a = derefActor(pop(), "o6_getActorWalkBox");
	push(a->_ignoreBoxes ? 0 : a->_walkbox);
}

void ScummEngine_v6::o6_getActorCostume() {
	Actor *a = derefActor(pop(), "o6_getActorCostume");
	push(a->_costume);
}

void ScummEngine_v6::o6_getActorElevation() {
	Actor *a = derefActor(pop(), "o6_getActorElevation");
	push(a->getElevation());
}

void ScummEngine_v6::o6_getActorWidth() {
	Actor *a = derefActor(pop(), "o6_getActorWidth");
	push(a->_width);
}

void ScummEngine_v6::o6_getActorScaleX() {
	Actor *a = derefActor(pop(), "o6_getActorScaleX");
	push(a->_scalex);
}

void ScummEngine_v6::o6_getActorAnimCounter() {
	Actor *a = derefActor(pop(), "o6_getActorAnimCounter");
	push(a->_cost.animCounter);
}

void ScummEngine_v6::o6_getAnimateVariable() {
	int var = pop();
	Actor *a = derefActor(pop(), "o6_getAnimateVariable");

	// WORKAROUND: In Backyard Baseball 2001 and 2003,
	// bunting a foul ball as Pete Wheeler may softlock the game
	// with an animation loop if the ball goes way into
	// the left or right field line.
	//
	// This is a script bug because Pete's actor variable never
	// sets to 1 in this condition and script room-4-2105
	// (or room-3-2105 in 2003) will always break.
	// We fix that by forcing Pete to play the return animation
	// regardless if the ball's foul or not.
	if ((_game.id == GID_BASEBALL2001 || _game.id == GID_BASEBALL2003) && \
			_currentRoom == ((_game.id == GID_BASEBALL2001) ? 4 : 3) && \
			vm.slot[_currentScript].number == 2105 && \
			a->_costume == ((_game.id == GID_BASEBALL2001) ? 107 : 99) && \
			// Room variable 5 to ensure this workaround executes only once at
			// the beginning of the script and room variable 22 to check if we
			// are bunting.
			readVar(0x8000 + 5) != 0 && readVar(0x8000 + 22) == 4)
		push(1);
	else
		push(a->getAnimVar(var));
}

void ScummEngine_v6::o6_isActorInBox() {
	int box = pop();
	Actor *a = derefActor(pop(), "o6_isActorInBox");
	push(checkXYInBoxBounds(box, a->getPos().x, a->getPos().y));
}

void ScummEngine_v6::o6_getActorLayer() {
	Actor *a = derefActor(pop(), "getActorLayer");
	push(a->_layer);
}

void ScummEngine_v6::o6_getObjectX() {
	push(getObjX(pop()));
}

void ScummEngine_v6::o6_getObjectY() {
	push(getObjY(pop()));
}

void ScummEngine_v6::o6_getObjectOldDir() {
	push(getObjOldDir(pop()));
}

void ScummEngine_v6::o6_getObjectNewDir() {
	push(getObjNewDir(pop()));
}

void ScummEngine_v6::o6_findInventory() {
	int idx = pop();
	int owner = pop();
	push(findInventory(owner, idx));
}

void ScummEngine_v6::o6_getInventoryCount() {
	push(getInventoryCount(pop()));
}

void ScummEngine_v6::o6_getVerbFromXY() {
	int y = pop();
	int x = pop();
	int over = findVerbAtPos(x, y);
	if (over)
		over = _verbs[over].verbid;
	push(over);
}

void ScummEngine_v6::o6_beginOverride() {
	// WORKAROUND (bug in the original):
	// When Guybrush gets on the Sea Cucumber for the first time and the monkeys show up on deck,
	// if the ESC key is pressed before the "Any last words, Threepwood?" dialogue, the music will
	// continue playing indefinitely throughout the game (or until another "sequence" music is played).
	//
	// To amend this, we intercept this exact script override and we force the playback of sound 2277,
	// which is the iMUSE sequence which would have been played after the dialogue.
	if (enhancementEnabled(kEnhAudioChanges) && _game.id == GID_CMI && _currentRoom == 37 && vm.slot[_currentScript].number == 251 &&
		_sound->isSoundRunning(2275) != 0 && (_scriptPointer - _scriptOrgPointer) == 0x1A) {
		int list[16] = {0x1001, 2277, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		_sound->soundKludge(list, 2);
	}

	beginOverride();
	_skipVideo = 0;
}

void ScummEngine_v6::o6_endOverride() {
	endOverride();
}

void ScummEngine_v6::o6_setObjectName() {
	int obj = pop();
	setObjectName(obj);
}

void ScummEngine_v6::o6_isSoundRunning() {
	int snd = pop();

	if (snd)
		snd = _sound->isSoundRunning(snd);

	push(snd);
}

void ScummEngine_v6::o6_setBoxFlags() {
	int table[65];
	int num, value;

	value = pop();
	num = getStackList(table, ARRAYSIZE(table));

	while (--num >= 0) {
		setBoxFlags(table[num], value);
	}
}

void ScummEngine_v6::o6_createBoxMatrix() {
	createBoxMatrix();

	if ((_game.id == GID_DIG) || (_game.id == GID_CMI))
		putActors();
}

void ScummEngine_v6::o6_resourceRoutines() {
	int resid;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_LOAD_SCRIPT:
		resid = pop();
		if (_game.version >= 7)
			if (resid >= _numGlobalScripts)
				break;
		ensureResourceLoaded(rtScript, resid);
		break;
	case SO_LOAD_SOUND:
		resid = pop();
		ensureResourceLoaded(rtSound, resid);
		break;
	case SO_LOAD_COSTUME:
		resid = pop();
		ensureResourceLoaded(rtCostume, resid);
		break;
	case SO_LOAD_ROOM:
		resid = pop();
		ensureResourceLoaded(rtRoom, resid);
		break;
	case SO_NUKE_SCRIPT:
		resid = pop();
		if (_game.version >= 7)
			if (resid >= _numGlobalScripts)
				break;
		_res->setResourceCounter(rtScript, resid, 0x7F);
		break;
	case SO_NUKE_SOUND:
		resid = pop();
		_res->setResourceCounter(rtSound, resid, 0x7F);
		break;
	case SO_NUKE_COSTUME:
		resid = pop();
		_res->setResourceCounter(rtCostume, resid, 0x7F);
		break;
	case SO_NUKE_ROOM:
		resid = pop();
		_res->setResourceCounter(rtRoom, resid, 0x7F);
		break;
	case SO_LOCK_SCRIPT:
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		_res->lock(rtScript, resid);
		break;
	case SO_LOCK_SOUND:
		resid = pop();
		_res->lock(rtSound, resid);
		break;
	case SO_LOCK_COSTUME:
		resid = pop();
		_res->lock(rtCostume, resid);
		break;
	case SO_LOCK_ROOM:
		resid = pop();
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->lock(rtRoom, resid);
		break;
	case SO_UNLOCK_SCRIPT:
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		_res->unlock(rtScript, resid);
		break;
	case SO_UNLOCK_SOUND:
		resid = pop();
		_res->unlock(rtSound, resid);
		break;
	case SO_UNLOCK_COSTUME:
		resid = pop();
		_res->unlock(rtCostume, resid);
		break;
	case SO_UNLOCK_ROOM:
		resid = pop();
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->unlock(rtRoom, resid);
		break;
	case SO_CLEAR_HEAP:
		/* this is actually a scumm message */
		error("clear heap not working yet");
		break;
	case SO_LOAD_CHARSET:
		resid = pop();
		loadCharset(resid);
		break;
	case SO_NUKE_CHARSET:
		resid = pop();
		nukeCharset(resid);
		break;
	case SO_LOAD_OBJECT:
		{
			int room, obj = popRoomAndObj(&room);
			loadFlObject(obj, room);
			break;
		}
	default:
		error("o6_resourceRoutines: default case %d", subOp);
	}
}


void ScummEngine_v6::o6_roomOps() {
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_ROOM_SCROLL:
		b = pop();
		a = pop();
		if (a < (_screenWidth / 2))
			a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
			b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
			a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
			b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;
		break;

	case SO_ROOM_SCREEN:
		b = pop();
		a = pop();
		initScreens(a, b);
		break;

	case SO_ROOM_PALETTE:
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case SO_ROOM_SHAKE_ON:
		setShake(1);
		break;

	case SO_ROOM_SHAKE_OFF:
		setShake(0);
		break;

	case SO_ROOM_INTENSITY:
		c = pop();
		b = pop();
		a = pop();
		// Prevent assert() error with corner case, fixes bug #9871
		if (_game.id == GID_FT && _roomResource == 0)
			break;
		darkenPalette(a, a, a, b, c);
		break;

	case SO_ROOM_SAVEGAME:
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		if (_game.id == GID_TENTACLE)
			_saveSound = (_saveLoadSlot != 0);
		break;

	case SO_ROOM_FADE:
		a = pop();
		if (a) {
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;

	case SO_RGB_ROOM_INTENSITY:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case SO_ROOM_SHADOW:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setShadowPalette(a, b, c, d, e, 0, 256);
		break;

	case SO_SAVE_STRING:
		error("save string not implemented");
		break;

	case SO_LOAD_STRING:
		error("load string not implemented");
		break;

	case SO_ROOM_TRANSFORM:
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		palManipulateInit(a, b, c, d);
		break;

	case SO_CYCLE_SPEED:
		b = pop();
		a = pop();
		assertRange(1, a, 16, "o6_roomOps: 187: color cycle");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;

	case SO_ROOM_NEW_PALETTE:
		a = pop();

		// This opcode is used when turning off noir mode in Sam & Max;
		// the original exhibited some minor glitches during this mode,
		// so we have two ways to perform it: the accurate one, and our
		// improved one...
		if (_game.id == GID_SAMNMAX && enhancementEnabled(kEnhMinorBugFixes) &&
			_currentScript != 0xFF && vm.slot[_currentScript].number == 64) {
			setDirtyColors(0, 255);
		} else {
			setCurrentPalette(a);
		}

		break;
	default:
		error("o6_roomOps: default case %d", subOp);
	}
}

void ScummEngine_v6::o6_actorOps() {
	Actor *a;
	int i, j, k;
	int args[8];

	byte subOp = fetchScriptByte();
	if (subOp == SO_ACTOR_INIT) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o6_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case SO_COSTUME:
		i = pop();
		// WORKAROUND: There's a small continuity error in DOTT; the fire that
		// makes Washington leave the room can only exist if he's wearing the
		// chattering teeth, but yet when he comes back he's not wearing them
		// during this cutscene.
		if (_game.id == GID_TENTACLE && _currentRoom == 13 && vm.slot[_currentScript].number == 211 &&
			a->_number == 8 && i == 53 && enhancementEnabled(kEnhVisualChanges)) {
			i = 69;
		}
		a->setActorCostume(i);
		break;
	case SO_STEP_DIST:
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case SO_SOUND:
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->_sound[i] = args[i];
		break;
	case SO_WALK_ANIMATION:
		a->_walkFrame = pop();
		break;
	case SO_TALK_ANIMATION:
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case SO_STAND_ANIMATION:
		a->_standFrame = pop();
		break;
	case SO_ANIMATION:
		// dummy case in scumm6
		pop();
		pop();
		pop();
		break;
	case SO_DEFAULT:
		a->initActor(0);
		break;
	case SO_ELEVATION:
		a->setElevation(pop());
		break;
	case SO_ANIMATION_DEFAULT:
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case SO_PALETTE:
		j = pop();
		i = pop();
		assertRange(0, i, 255, "o6_actorOps: palette slot");
		a->setPalette(i, j);
		break;
	case SO_TALK_COLOR:
		a->_talkColor = pop();
		break;
	case SO_ACTOR_NAME:
		loadPtrToResource(rtActorName, a->_number, nullptr);
		break;
	case SO_INIT_ANIMATION:
		a->_initFrame = pop();
		break;
	case SO_ACTOR_WIDTH:
		a->_width = pop();
		break;
	case SO_SCALE:
		i = pop();
		a->setScale(i, i);
		break;
	case SO_NEVER_ZCLIP:
		a->_forceClip = 0;
		break;
	case SO_ALWAYS_ZCLIP_FT_DEMO:
	case SO_ALWAYS_ZCLIP:
		a->_forceClip = pop();
		break;
	case SO_IGNORE_BOXES:
		a->_ignoreBoxes = 1;
		a->_forceClip = (_game.version >= 7) ? 100 : 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_FOLLOW_BOXES:
		a->_ignoreBoxes = 0;
		a->_forceClip = (_game.version >= 7) ? 100 : 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_ANIMATION_SPEED:
		a->setAnimSpeed(pop());
		break;
	case SO_SHADOW:
		a->_shadowMode = pop();
		break;
	case SO_TEXT_OFFSET:
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case SO_ACTOR_VARIABLE:
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case SO_ACTOR_IGNORE_TURNS_ON:
		a->_ignoreTurns = true;
		break;
	case SO_ACTOR_IGNORE_TURNS_OFF:
		a->_ignoreTurns = false;
		break;
	case SO_NEW:
		a->initActor(2);
		break;
	case SO_ACTOR_DEPTH:
		a->_layer = pop();
		break;
	case SO_ACTOR_WALK_SCRIPT:
		a->_walkScript = pop();
		break;
	case SO_ACTOR_STOP:
		a->stopActorMoving();
		a->startAnimActor(a->_standFrame);
		break;
	case SO_ACTOR_FACE:										/* set direction */
		a->_moving &= ~MF_TURN;
		j = pop();
		a->turnToDirection(j);
		a->setDirection(j);
		break;
	case SO_ACTOR_TURN:										/* turn to direction */
		a->turnToDirection(pop());
		break;
	case SO_ACTOR_WALK_PAUSE:
		a->_moving |= MF_FROZEN;
		break;
	case SO_ACTOR_WALK_RESUME:
		a->_moving &= ~MF_FROZEN;
		break;
	case SO_ACTOR_TALK_SCRIPT:
		a->_talkScript = pop();
		break;
	default:
		error("o6_actorOps: default case %d", subOp);
	}
}

void ScummEngine_v6::o6_verbOps() {
	int slot, a, b;
	VerbSlot *vs;

	byte subOp = fetchScriptByte();
	if (subOp == SO_VERB_INIT) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		assertRange(0, _curVerbSlot, _numVerbs - 1, "new verb slot");
		return;
	}
	vs = &_verbs[_curVerbSlot];
	slot = _curVerbSlot;
	switch (subOp) {
	case SO_VERB_IMAGE:
		a = pop();
		if (_curVerbSlot) {
			setVerbObject(_roomResource, a, slot);
			vs->type = kImageVerbType;
			if (_game.heversion >= 61)
				vs->imgindex = a;
		}
		break;
	case SO_VERB_NAME:
		loadPtrToResource(rtVerb, slot, nullptr);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case SO_VERB_COLOR:
		vs->color = pop();
		break;
	case SO_VERB_HICOLOR:
		vs->hicolor = pop();
		break;
	case SO_VERB_AT:
		vs->curRect.top = pop();
		vs->curRect.left = vs->origLeft = pop();
		break;
	case SO_VERB_ON:
		vs->curmode = 1;
		break;
	case SO_VERB_OFF:
		vs->curmode = 0;
		break;
	case SO_VERB_DELETE:
		if (_game.heversion >= 60) {
			slot = getVerbSlot(pop(), 0);
		}
		killVerb(slot);
		break;
	case SO_VERB_NEW:
		slot = getVerbSlot(_curVerb, 0);
		if (slot == 0) {
			for (slot = 1; slot < _numVerbs; slot++) {
				if (_verbs[slot].verbid == 0)
					break;
			}
			if (slot == _numVerbs)
				error("Too many verbs");
			_curVerbSlot = slot;
		}
		vs = &_verbs[slot];
		vs->verbid = _curVerb;
		vs->color = 2;
		vs->hicolor = 0;
		vs->dimcolor = 8;
		vs->type = kTextVerbType;
		vs->charset_nr = _string[0]._default.charset;
		vs->curmode = 0;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		break;
	case SO_VERB_DIMCOLOR:
		vs->dimcolor = pop();
		break;
	case SO_VERB_DIM:
		vs->curmode = 2;
		break;
	case SO_VERB_KEY:
		vs->key = pop();
		break;
	case SO_VERB_CENTER:
		vs->center = 1;
		break;
	case SO_VERB_NAME_STR:
		a = pop();
		if (a == 0) {
			loadPtrToResource(rtVerb, slot, (const byte *)"");
		} else {
			loadPtrToResource(rtVerb, slot, getStringAddress(a));
		}
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case SO_VERB_IMAGE_IN_ROOM:
		b = pop();
		a = pop();

		if (slot && a != vs->imgindex) {
			setVerbObject(b, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case SO_VERB_BAKCOLOR:
		vs->bkcolor = pop();
		break;
	case SO_END:
		drawVerb(slot, 0);
		verbMouseOver(0);
		break;
	default:
		error("o6_verbops: default case %d", subOp);
	}
}

void ScummEngine_v6::o6_getActorFromXY() {
	int y = pop();
	int x = pop();
	int r = getActorFromPos(x, y);
	push(r);
}

void ScummEngine_v6::o6_findObject() {
	int y = pop();
	int x = pop();
	int r = findObject(x, y);
	push(r);
}

void ScummEngine_v6::o6_pseudoRoom() {
	int list[100];
	int num, a, value;

	num = getStackList(list, ARRAYSIZE(list));
	value = pop();

	while (--num >= 0) {
		a = list[num];
		if (a > 0x7F)
			_resourceMapper[a & 0x7F] = value;
	}
}

void ScummEngine_v6::o6_getVerbEntrypoint() {
	int e = pop();
	int v = pop();
	push(getVerbEntrypoint(v, e));
}

void ScummEngine_v6::o6_arrayOps() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	int b, c, d, len;
	byte *data;
	int list[128];

	switch (subOp) {
	case SO_ASSIGN_STRING:
		b = pop();
		len = resStrLen(_scriptPointer);
		data = defineArray(array, kStringArray, 0, len + 1);
		copyScriptString(data + b);
		break;
	case SO_ASSIGN_INT_LIST:
		b = pop();
		c = pop();
		d = readVar(array);
		if (d == 0) {
			defineArray(array, kIntArray, 0, b + c);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case SO_ASSIGN_2DIM_LIST:
		b = pop();
		len = getStackList(list, ARRAYSIZE(list));
		d = readVar(array);
		if (d == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, b + len, list[len]);
		}
		break;
	default:
		error("o6_arrayOps: default case %d (array %d)", subOp, array);
	}
}

void ScummEngine_v6::o6_saveRestoreVerbs() {
	int a, b, c;
	int slot, slot2;

	c = pop();
	b = pop();
	a = pop();

	byte subOp = fetchScriptByte();
	if (_game.version == 8) {
		subOp = (subOp - 141) + 0xB4;
	}

	switch (subOp) {
	case SO_SAVE_VERBS:
		while (a <= b) {
			slot = getVerbSlot(a, 0);
			if (slot && _verbs[slot].saveid == 0) {
				_verbs[slot].saveid = c;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case SO_RESTORE_VERBS:
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot) {
				slot2 = getVerbSlot(a, 0);
				if (slot2)
					killVerb(slot2);
				slot = getVerbSlot(a, c);
				_verbs[slot].saveid = 0;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case SO_DELETE_VERBS:
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot)
				killVerb(slot);
			a++;
		}
		break;
	default:
		error("o6_saveRestoreVerbs: default case");
	}
}

void ScummEngine_v6::o6_drawBox() {
	int x, y, x2, y2, color;
	color = pop();
	y2 = pop();
	x2 = pop();
	y = pop();
	x = pop();
	drawBox(x, y, x2, y2, color);
}

void ScummEngine_v6::o6_wait() {
	int actnum;
	int offs = -2;
	Actor *a;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_WAIT_FOR_ACTOR:		// Wait for actor
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o6_wait:168");
		if (_game.version >= 7) {
			if (a->isInCurrentRoom() && a->_moving)
				break;
		} else {
			if (a->_moving)
				break;
		}
		return;
	case SO_WAIT_FOR_MESSAGE:		// Wait for message
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case SO_WAIT_FOR_CAMERA:		// Wait for camera
		if (_game.version >= 7) {
			if (camera._dest != camera._cur)
				break;
		} else {
			if (camera._cur.x / 8 != camera._dest.x / 8)
				break;
		}

		return;
	case SO_WAIT_FOR_SENTENCE:
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
			break;
		}
		if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;
		break;
	case SO_WAIT_FOR_ANIMATION:
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o6_wait:226");
		if (a->isInCurrentRoom() && a->_needRedraw)
			break;
		return;
	case SO_WAIT_FOR_TURN:
		// WORKAROUND for bug #819: An angle will often be received as the
		// actor number due to script bugs in The Dig. In all cases where this
		// occurs, _curActor is set just before it, so we can use it instead.
		//
		// For now, if the value passed in is divisible by 45, assume it is an
		// angle, and use _curActor as the actor to wait for.
		//
		// TODO: what did the original interpreter do in this case?
		offs = fetchScriptWordSigned();
		actnum = pop();
		if (actnum % 45 == 0) {
			actnum = _curActor;
		}
		a = derefActor(actnum, "o6_wait:232b");
		if (a->isInCurrentRoom() && a->_moving & MF_TURN)
			break;
		return;
	default:
		error("o6_wait: default case 0x%x", subOp);
	}

	_scriptPointer += offs;
	o6_breakHere();
}

void ScummEngine_v6::o6_soundKludge() {
	int list[16];
	int num = getStackList(list, ARRAYSIZE(list));

	_sound->soundKludge(list, num);

	// WORKAROUND for bug #2438: The room-11-2016 script contains a
	// slight bug causing it to busy-wait for a sound to finish. Even under
	// the best of circumstances, this will cause the game to hang briefly.
	// On platforms where threading is cooperative, it will cause the game
	// to hang indefinitely (hence the use of `kEnhGameBreakingBugFixes`).
	// We identify the buggy part of the script by looking for a
	// soundKludge() opcode immediately followed by a jump.

	if (_game.id == GID_CMI && _roomResource == 11 && vm.slot[_currentScript].number == 2016 && *_scriptPointer == 0x66 &&
		enhancementEnabled(kEnhGameBreakingBugFixes)) {
		debug(3, "Working around script bug in room-11-2016");
		o6_breakHere();
	}
}

void ScummEngine_v6::o6_isAnyOf() {
	int list[100];
	int num;
	int32 val;

	num = getStackList(list, ARRAYSIZE(list));
	val = pop();

	while (--num >= 0) {
		if (list[num] == val) {
			push(1);
			return;
		}
	}

	push(0);
}

void ScummEngine_v6::o6_systemOps() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_RESTART:
		restart();
		break;
	case SO_PAUSE:
		pauseGame();
		break;
	case SO_QUIT:
		_quitFromScriptCmd = true;
		quitGame();
		break;
	default:
		error("o6_systemOps invalid case %d", subOp);
	}
}

void ScummEngine_v6::o6_delay() {
	uint32 delay = (uint16)pop();
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	o6_breakHere();
}

void ScummEngine_v6::o6_delaySeconds() {
	uint32 delay = (uint32)pop();
	// WORKAROUND: On Baseball 2001, this script downloads the news, poll and banner information.
	// It gives a one second break before validating that the download has completed, which is
	// a tad bit too long.  So let's turn that into a one frame break.  This is safe because
	// the script also checks if either var135 == 1, or the check has been done
	// 10 times before moving on:
	//
	// [0000] (43) localvar2 = 10
	//  ...
	// [004E] (4F) localvar3++
	// [0051] (B1) delaySeconds(1)
	// [0054] (5D) unless ((var135 || (localvar3 > localvar2))) jump 4e
	if (!(_game.id == GID_BASEBALL2001 && vm.slot[_currentScript].number == 414)) {
		delay = delay * 60;
	}
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	o6_breakHere();
}

void ScummEngine_v6::o6_delayMinutes() {
	uint32 delay = (uint16)pop() * 3600;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	o6_breakHere();
}

void ScummEngine_v6::o6_stopSentence() {
	_sentenceNum = 0;
	stopScript(VAR(VAR_SENTENCE_SCRIPT));
	clearClickedStatus();
}

void ScummEngine_v6::o6_printLine() {
	_actorToPrintStrFor = 0xFF;
	decodeParseString(0, 0);
}

void ScummEngine_v6::o6_printText() {
	decodeParseString(1, 0);
}

void ScummEngine_v6::o6_printDebug() {
	decodeParseString(2, 0);
}

void ScummEngine_v6::o6_printSystem() {
	decodeParseString(3, 0);
}

void ScummEngine_v6::o6_printActor() {
	decodeParseString(0, 1);
}

void ScummEngine_v6::o6_printEgo() {
	push(VAR(VAR_EGO));
	decodeParseString(0, 1);
}

void ScummEngine_v6::o6_talkActor() {
	int offset = _scriptPointer - _scriptOrgPointer;

	// WORKAROUND for missing waitForMessage() calls; see below
	if (_forcedWaitForMessage) {
		if (VAR(VAR_HAVE_MSG)) {
			_scriptPointer--;
			o6_breakHere();
			return;
		}

		_forcedWaitForMessage = false;
		_scriptPointer += resStrLen(_scriptPointer) + 1;

		return;
	}

	// WORKAROUND: If Sam tries to buy an object at Snuckey's without having
	// any money, Max's comment on capitalism may be cut too early because the
	// employee reacts immediately after Max without any prior waitForMessage().
	// The magic values below come from scripts 11-67 and 11-205.
	//
	// This call can't just be inserted after Max's line; it needs to be done
	// just before the employee's line, otherwise the timing with Sam's moves
	// will feel off -- so we can't use the _forcedWaitForMessage trick.
	if (_game.id == GID_SAMNMAX && _roomResource == 11 && vm.slot[_currentScript].number == 67
		&& getOwner(70) != 2 && !readVar(0x8000 + 67) && !readVar(0x8000 + 39) && readVar(0x8000 + 12) == 1
		&& !getClass(126, 6) && enhancementEnabled(kEnhRestoredContent)) {
		if (VAR(VAR_HAVE_MSG)) {
			_scriptPointer--;
			o6_breakHere();
			return;
		}
	}

	_actorToPrintStrFor = pop();

	// WORKAROUND for bug #3803: "DOTT: Bernard impersonating LaVerne"
	// Original script did not check for VAR_EGO == 2 before executing
	// a talkActor opcode.
	if (_game.id == GID_TENTACLE && vm.slot[_currentScript].number == 307
			&& VAR(VAR_EGO) != 2 && _actorToPrintStrFor == 2
			&& enhancementEnabled(kEnhMinorBugFixes)) {
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		return;
	}

	// WORKAROUND: In the French release of Full Throttle, a "piano-low-kick"
	// string appears in the text when Ben looks at one of the small pictures
	// above the piano in the bar. Probably an original placeholder which
	// hasn't been properly replaced... Fixed in the 2017 remaster, though.
	if (_game.id == GID_FT && _language == Common::FR_FRA
		&& _roomResource == 7 && vm.slot[_currentScript].number == 77
		&& _actorToPrintStrFor == 1 && enhancementEnabled(kEnhTextLocFixes)) {
		const int len = resStrLen(_scriptPointer) + 1;
		if (len == 93 && memcmp(_scriptPointer + 16 + 18, "piano-low-kick", 14) == 0) {
			byte *tmpBuf = new byte[len - 14 + 3];
			memcpy(tmpBuf, _scriptPointer, 16 + 18);
			memcpy(tmpBuf + 16 + 18, ", 1", 3);
			memcpy(tmpBuf + 16 + 18 + 3, _scriptPointer + 16 + 18 + 14, len - (16 + 18 + 14));

			_string[0].loadDefault();
			actorTalk(tmpBuf);
			delete[] tmpBuf;
			_scriptPointer += len;
			return;
		}
	}

	_string[0].loadDefault();
	actorTalk(_scriptPointer);

	// WORKAROUND: Dr Fred's first reaction line about Hoagie's and Laverne's
	// units after receiving a new diamond is unused because of missing
	// wait.waitForMessage() calls. We always simulate this opcode when
	// triggering Dr Fred's lines in this part of the script, since there is
	// no stable offset for all the floppy, CD and translated versions, and
	// no easy way to only target the impacted lines.
	if (_game.id == GID_TENTACLE && vm.slot[_currentScript].number == 9
		&& vm.localvar[_currentScript][0] == 216 && _actorToPrintStrFor == 4 && enhancementEnabled(kEnhRestoredContent)) {
		_forcedWaitForMessage = true;
		_scriptPointer--;

		return;
	}

	// WORKAROUND for bug #1452: "DIG: Missing subtitles when talking to Brink"
	// Original script does not have wait.waitForMessage() after several messages:
	//
	// [011A] (5D)   if (getActorCostume(VAR_EGO) == 1) {
	// [0126] (BA)     talkActor("/STOP.008/Low out.",3)
	// [013D] (A9)     wait.waitForMessage()
	// [013F] (5D)   } else if (var227 == 0) {
	// [014C] (BA)     talkActor("/STOP.009/Never mind.",3)
	// [0166] (73)   } else {
	//
	// Here we simulate that opcode.
	if (_game.id == GID_DIG && vm.slot[_currentScript].number == 88 && enhancementEnabled(kEnhRestoredContent)) {
		if (offset == 0x158 || offset == 0x214 || offset == 0x231 || offset == 0x278) {
			_forcedWaitForMessage = true;
			_scriptPointer--;

			return;
		}
	}

	// WORKAROUND bug #4410: Restore a missing subtitle when Low is inside the
	// tomb and he finds the purpose of the crypt ("/TOMB.022/Now that I know
	// what I'm looking for"...). Also happens in the original interpreters.
	// We used to do this in actorTalk(), but then Low's proper walking
	// animation was lost and he would just glide over the floor. Having him
	// wait before he moves is less disturbing, since that's something he
	// already does in the game.
	if (_game.id == GID_DIG && _roomResource == 58 && vm.slot[_currentScript].number == 402
		&& _actorToPrintStrFor == 3 && vm.localvar[_currentScript][0] == 0
		&& readVar(0x8000 + 94) && readVar(0x8000 + 78) && !readVar(0x8000 + 97)
		&& _scummVars[269] == 3 && getState(388) == 2 && enhancementEnabled(kEnhRestoredContent)) {
		_forcedWaitForMessage = true;
		_scriptPointer--;

		return;
	}

	_scriptPointer += resStrLen(_scriptPointer) + 1;
}

void ScummEngine_v6::o6_talkEgo() {
	push(VAR(VAR_EGO));
	o6_talkActor();
}

void ScummEngine_v6::o6_dimArray() {
	int data;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_INT_ARRAY:
		data = kIntArray;
		break;
	case SO_BIT_ARRAY:
		data = kBitArray;
		break;
	case SO_NIBBLE_ARRAY:
		data = kNibbleArray;
		break;
	case SO_BYTE_ARRAY:
		data = kByteArray;
		break;
	case SO_STRING_ARRAY:
		data = kStringArray;
		break;
	case SO_UNDIM_ARRAY:
		nukeArray(fetchScriptWord());
		return;
	default:
		error("o6_dimArray: default case %d", subOp);
	}

	defineArray(fetchScriptWord(), data, 0, pop());
}

void ScummEngine_v6::o6_dummy() {
}

void ScummEngine_v6::o6_dim2dimArray() {
	int a, b, data;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_INT_ARRAY:
		data = kIntArray;
		break;
	case SO_BIT_ARRAY:
		data = kBitArray;
		break;
	case SO_NIBBLE_ARRAY:
		data = kNibbleArray;
		break;
	case SO_BYTE_ARRAY:
		data = kByteArray;
		break;
	case SO_STRING_ARRAY:
		data = kStringArray;
		break;
	default:
		error("o6_dim2dimArray: default case %d", subOp);
	}

	b = pop();
	a = pop();
	defineArray(fetchScriptWord(), data, a, b);
}

void ScummEngine_v6::o6_abs() {
	int a = pop();
	push(ABS(a));
}

void ScummEngine_v6::o6_distObjectObject() {
	int a, b;
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, true, b, 0));
}

void ScummEngine_v6::o6_distObjectPt() {
	int a, b, c;
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, false, b, c));
}

void ScummEngine_v6::o6_distPtPt() {
	int a, b, c, d;
	d = pop();
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(false, a, b, false, c, d));
}

void ScummEngine_v6::o6_drawBlastObject() {
	int args[16];
	int a, b, c, d, e;

	getStackList(args, ARRAYSIZE(args));
	e = pop();
	d = pop();
	c = pop();
	b = pop();
	a = pop();
	enqueueObject(a, b, c, d, e, 0xFF, 0xFF, 1, 0);
}

// Set BOMP processing window
void ScummEngine_v6::o6_setBlastObjectWindow() {
	pop();
	pop();
	pop();
	pop();

	// None of the scripts of The Dig and Full Throttle use this opcode.
	// Sam & Max only uses it at the beginning of the highway subgame. In
	// the original interpreter pop'ed arguments are just ignored and the
	// clipping blastObject window is defined with (0, 0, 320, 200)...
	// which matches the screen dimensions and thus, doesn't require
	// another clipping operation.
	// So, we just handle this as no-op opcode.
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::o6_kernelSetFunctions() {
	int args[30];
	int num;
	Actor *a;

	num = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 4:
		grabCursor(args[1], args[2], args[3], args[4]);
		break;
	case 6: {
			// SMUSH movie playback
			if (args[1] == 0 && !_skipVideo) {
				const char *videoname = (const char *)getStringAddressVar(VAR_VIDEONAME);
				assert(videoname);

				// Correct incorrect smush filename in Macintosh FT demo
				if ((_game.id == GID_FT) && (_game.features & GF_DEMO) && (_game.platform == Common::kPlatformMacintosh) &&
					(!strcmp(videoname, "jumpgorge.san")))
					_splayer->play("jumpgorg.san", _smushFrameRate);
				else
					_splayer->play(videoname, _smushFrameRate);

				if (_game.id == GID_DIG) {
					_disableFadeInEffect = true;
				}
			} else if (_game.id == GID_FT && !_skipVideo) {
				const int insaneVarNum = ((_game.features & GF_DEMO) && (_game.platform == Common::kPlatformDOS))
					? 232 : 233;

				_insane->setSmushParams(_smushFrameRate);
				_insane->runScene(insaneVarNum);
			}
		}
		break;
	case 12:
		setCursorFromImg(args[1], (uint) - 1, args[2]);
		break;
	case 13:
		derefActor(args[1], "o6_kernelSetFunctions:13")->remapActorPalette(args[2], args[3], args[4], -1);
		break;
	case 14:
		derefActor(args[1], "o6_kernelSetFunctions:14")->remapActorPalette(args[2], args[3], args[4], args[5]);
		break;
	case 15:
		_smushFrameRate = args[1];
		break;
	case 16:
	case 17:
		enqueueText(getStringAddressVar(VAR_STRING2DRAW), args[3], args[4], args[2], args[1], (args[0] == 16) ? kStyleAlignCenter : kStyleAlignLeft);
		break;
	case 20:
		_imuseDigital->setRadioChatterSFX(args[1]);
		break;
	case 107:
		a = derefActor(args[1], "o6_kernelSetFunctions: 107");
		a->setScale((unsigned char)args[2], -1);
		break;
	case 108:
		setShadowPalette(args[1], args[2], args[3], args[4], args[5], args[6]);
		break;
	case 109:
		setShadowPalette(0, args[1], args[2], args[3], args[4], args[5]);
		break;
	case 114:
		error("o6_kernelSetFunctions: stub114()");
		break;
	case 117:
		freezeScripts(2);
		break;
	case 118:
		enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 3);
		break;
	case 119:
		enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 0);
		break;
	case 124:
		_saveSound = args[1];
		break;
	case 215:
		ConfMan.setBool("subtitles", args[1] != 0);
		break;
	default:
		error("o6_kernelSetFunctions: default case %d (param count %d)", args[0], num);
		break;
	}
}
#endif

void ScummEngine_v6::o6_kernelSetFunctions() {
	int args[30];
	int num;
	Actor *a;

	num = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 3:
		// Dummy case
		break;
	case 4:
		grabCursor(args[1], args[2], args[3], args[4]);
		break;
	case 5:
		fadeOut(args[1]);
		break;
	case 6:
		_fullRedraw = true;
		redrawBGAreas();
		setActorRedrawFlags();
		processActors();
		fadeIn(args[1]);
		break;
	case 8:
		if (startManiac()) {
			// This is so that the surprised exclamation happens
			// after we return to the game again, not before.
			o6_breakHere();
		}
		break;
	case 9:
		killAllScriptsExceptCurrent();
		break;
	case 104:									/* samnmax */
		nukeFlObjects(args[2], args[3]);
		break;
	case 107:									/* set actor scale */
		a = derefActor(args[1], "o6_kernelSetFunctions: 107");
		a->setScale((unsigned char)args[2], -1);
		break;
	case 108:									/* create proc_special_palette */
	case 109:
		// Case 108 and 109 share the same function
		if (num != 6)
			error("o6_kernelSetFunctions sub op %d: expected 6 params but got %d", args[0], num);
		setShadowPalette(args[3], args[4], args[5], args[1], args[2], 0, 256);
		break;
	case 110:
		clearCharsetMask();
		break;
	case 111:
		a = derefActor(args[1], "o6_kernelSetFunctions: 111");
		a->_shadowMode = args[2] + args[3];
		break;
	case 112:									/* palette shift? */
		setShadowPalette(args[3], args[4], args[5], args[1], args[2], args[6], args[7]);
		break;
	case 114:
		// Sam & Max film noir mode
		if (_game.id == GID_SAMNMAX) {
			if (enhancementEnabled(kEnhMinorBugFixes)) {
				// At this point ScummVM will already have set
				// variable 0x8000 to indicate that the game is
				// in film noir mode. All we have to do here is
				// to mark the palette as "dirty", because
				// updatePalette() will desaturate the colors
				// as they are uploaded to the backend.
				//
				// This actually works better than the original
				// interpreter, where actors would sometimes
				// still be drawn in color.
				setDirtyColors(0, 255);
			} else {
				applyGrayscaleToPaletteRange(0, 254);
			}
		} else
			error("stub o6_kernelSetFunctions_114()");
		break;
	case 117:
		// Sam & Max uses this opcode in script-43, right
		// before a screensaver is selected.
		//
		// Sam & Max uses variable 132 to specify the number of
		// minutes of inactivity (no mouse movements) before
		// starting the screensaver, so setting it to 0 will
		// help in debugging.
		freezeScripts(0x80);
		break;
	case 119:
		enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 0);
		break;
	case 120:
		swapPalColors(args[1], args[2]);
		break;
	case 122:
		VAR(VAR_SOUNDRESULT) =
			(short)_imuse->doCommand (num - 1, &args[1]);
		break;
	case 123:
		copyPalColor(args[2], args[1]);
		break;
	case 124:
		_saveSound = args[1];
		break;
	default:
		error("o6_kernelSetFunctions: default case %d (param count %d)", args[0], num);
		break;
	}
}

void ScummEngine_v6::o6_kernelGetFunctions() {
	int args[30];
	int i;
	int slot;
	Actor *a;
	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 113:
		// WORKAROUND for bug #1465: The scripts used for screen savers
		// in Sam & Max use hard coded values for the maximum height and width.
		// This causes problems in rooms (ie. Credits) where their values are
		// lower, so we set result to zero if out of bounds.
		if (args[1] >= 0 && args[1] <= vs->w && args[2] >= 0 && args[2] <= vs->h) {
			byte pixel = *vs->getPixels(args[1], args[2]);
			push(pixel);
		} else {
			push(0);
		}
		break;
	case 115:
		push(getSpecialBox(args[1], args[2]));
		break;
	case 116:
		push(checkXYInBoxBounds(args[3], args[1], args[2]));
		break;
	case 206:
		push(remapPaletteColor(args[1], args[2], args[3], -1));
		break;
	case 207:
		i = getObjectIndex(args[1]);
		assert(i);
		push(_objs[i].x_pos);
		break;
	case 208:
		i = getObjectIndex(args[1]);
		assert(i);
		push(_objs[i].y_pos);
		break;
	case 209:
		i = getObjectIndex(args[1]);
		assert(i);
		push(_objs[i].width);
		break;
	case 210:
		i = getObjectIndex(args[1]);
		assert(i);
		push(_objs[i].height);
		break;
	case 211:
		/*
		   13 = thrust
		   336 = thrust
		   328 = thrust
		   27 = abort
		   97 = left
		   331 = left
		   115 = right
		   333 = right
		 */

		push(getKeyState(args[1]));
		break;
	case 212:
		a = derefActor(args[1], "o6_kernelGetFunctions:212");
		// This is used by walk scripts
		push(a->_frame);
		break;
	case 213:
		slot = getVerbSlot(args[1], 0);
		push(_verbs[slot].curRect.left);
		break;
	case 214:
		slot = getVerbSlot(args[1], 0);
		push(_verbs[slot].curRect.top);
		break;
	case 215:
		if ((_extraBoxFlags[args[1]] & 0x00FF) == 0x00C0) {
			push(_extraBoxFlags[args[1]]);
		} else {
			push(getBoxFlags(args[1]));
		}
		break;
	default:
		error("o6_kernelGetFunctions: default case %d", args[0]);
	}
}

int ScummEngine::getKeyState(int key) {
	switch (key) {
	case 0x147: // Home
		// FIXME: There seems to be a mistake in the code here ("insert" vs. "home")
		return (_keyDownMap[Common::KEYCODE_KP7] ||
		        _keyDownMap[Common::KEYCODE_INSERT]) ? 1 : 0;
	case 0x148: // Up
		return (_keyDownMap[Common::KEYCODE_KP8] ||
		        _keyDownMap[Common::KEYCODE_UP] ||
				_keyDownMap[Common::KEYCODE_8]) ? 1 : 0;
	case 0x149: // PgUp
		return (_keyDownMap[Common::KEYCODE_KP9] ||
		        _keyDownMap[Common::KEYCODE_PAGEUP]) ? 1 : 0;
	case 0x14B: // Left
		return (_keyDownMap[Common::KEYCODE_KP4] ||
		        _keyDownMap[Common::KEYCODE_LEFT] ||
				_keyDownMap[Common::KEYCODE_4]) ? 1 : 0;
	case 0x14D: // Right
		return (_keyDownMap[Common::KEYCODE_KP6] ||
		        _keyDownMap[Common::KEYCODE_RIGHT] ||
				_keyDownMap[Common::KEYCODE_6]) ? 1 : 0;
	case 0x14F: // End
		return (_keyDownMap[Common::KEYCODE_KP1] ||
		        _keyDownMap[Common::KEYCODE_END]) ? 1 : 0;
	case 0x150: // Down
		return (_keyDownMap[Common::KEYCODE_KP2] ||
		        _keyDownMap[Common::KEYCODE_DOWN] ||
				_keyDownMap[Common::KEYCODE_2]) ? 1 : 0;
	case 0x151: // PgDn
		return (_keyDownMap[Common::KEYCODE_KP3] ||
		        _keyDownMap[Common::KEYCODE_PAGEDOWN]) ? 1 : 0;
	default:
		return (_keyDownMap[key]) ? 1 : 0;
	}
}

int ScummEngine::getActionState(ScummAction action) {
	return (_actionMap[action]) ? 1 : 0;
}

void ScummEngine_v6::o6_delayFrames() {
	// WORKAROUND:  At startup, Moonbase Commander will pause for 20 frames before
	// showing the Infogrames logo.  The purpose of this break is to give time for the
	// GameSpy Arcade application to fill with the online game information.
	//
	// [0000] (84) localvar2 = max(readConfigFile.number(":var263:","user","wait-for-gamespy"),10)
	// [0029] (08) delayFrames((localvar2 * 2))
	//
	// But since we don't support GameSpy and have our own online support, this break
	// has become redundant and only wastes time.
	//
	// WORKAROUND:  On Baseball 2001, there is a 10 frame pause before sending the login information
	// to the server.  This is rather a pointless break, so let's skip that.
	if ((_game.id == GID_MOONBASE && vm.slot[_currentScript].number == 69) ||
		(_game.id == GID_BASEBALL2001 && _currentRoom == 37 && vm.slot[_currentScript].number == 2068)) {
		pop();
		return;
	}

	ScriptSlot *ss = &vm.slot[_currentScript];
	if (ss->delayFrameCount == 0) {
		ss->delayFrameCount = pop();
	} else {
		ss->delayFrameCount--;
	}
	if (ss->delayFrameCount) {
		_scriptPointer--;
		o6_breakHere();
	}
}

void ScummEngine_v6::o6_pickOneOf() {
	int args[100];
	int i, num;

	num = getStackList(args, ARRAYSIZE(args));
	i = pop();
	if (i < 0 || i > num)
		error("o6_pickOneOf: %d out of range (0, %d)", i, num - 1);
	push(args[i]);
}

void ScummEngine_v6::o6_pickOneOfDefault() {
	int args[100];
	int i, num, def;

	def = pop();
	num = getStackList(args, ARRAYSIZE(args));
	i = pop();
	if (i < 0 || i >= num)
		i = def;
	else
		i = args[i];
	push(i);
}

void ScummEngine_v6::o6_stampObject() {
	int object, x, y, state;

	state = pop();
	y = pop();
	x = pop();
	object = pop();
	if (_game.version >= 7 && object < 30) {
		if (state == 0)
			state = 255;

		Actor *a = derefActor(object, "o6_stampObject");
		a->_scalex = state;
		a->_scaley = state;
		a->putActor(x, y, _currentRoom);
		a->_drawToBackBuf = true;
		a->drawActorCostume();
		a->_drawToBackBuf = false;
		a->drawActorCostume();
		return;
	}

	if (state == 0)
		state = 1;

	int objnum = getObjectIndex(object);
	if (objnum == -1)
		return;

	if (x != -1) {
		_objs[objnum].x_pos = x * 8;
		_objs[objnum].y_pos = y * 8;
	}

	putState(object, state);
	drawObject(objnum, 0);
}

void ScummEngine_v6::o6_stopTalking() {
	stopTalk();
}

void ScummEngine_v6::o6_findAllObjects() {
	int room = pop();
	int i = 1;

	if (room != _currentRoom)
		error("o6_findAllObjects: current room is not %d", room);
	writeVar(0, 0);
	defineArray(0, kIntArray, 0, _numLocalObjects + 1);
	writeArray(0, 0, 0, _numLocalObjects);

	while (i < _numLocalObjects) {
		writeArray(0, 0, i, _objs[i].obj_nr);
		i++;
	}

	push(readVar(0));
}

void ScummEngine_v6::shuffleArray(int num, int minIdx, int maxIdx) {
	int rand1, rand2;
	int range = maxIdx - minIdx;
	int count = range * 2;

	// Shuffle the array 'num'
	while (count--) {
		// Determine two random elements...
		if (_game.heversion >= 72) {
			rand1 = VAR(VAR_RANDOM_NR) = _rnd.getRandomNumberRng(minIdx, maxIdx);
			rand2 = VAR(VAR_RANDOM_NR) = _rnd.getRandomNumberRng(minIdx, maxIdx);
		} else {
			rand1 = _rnd.getRandomNumber(range) + minIdx;
			rand2 = _rnd.getRandomNumber(range) + minIdx;
		}

		// ...and swap them
		int val1 = readArray(num, 0, rand1);
		int val2 = readArray(num, 0, rand2);
		writeArray(num, 0, rand1, val2);
		writeArray(num, 0, rand2, val1);
	}
}

void ScummEngine_v6::o6_shuffle() {
	int b = pop();
	int a = pop();
	shuffleArray(fetchScriptWord(), a, b);
}

void ScummEngine_v6::o6_pickVarRandom() {
	int num;
	int args[100];
	int dim1;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, kIntArray, 0, num);
		if (num > 0) {
			int16 counter = 0;
			do {
				writeArray(value, 0, counter + 1, args[counter]);
			} while (++counter < num);
		}

		shuffleArray(value, 1, num);
		writeArray(value, 0, 0, 2);
		push(readArray(value, 0, 1));
		return;
	}

	num = readArray(value, 0, 0);

	ArrayHeader *ah = getArray(value);
	dim1 = FROM_LE_16(ah->dim1) - 1;

	if (dim1 < num) {
		int16 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, dim1);
		if (readArray(value, 0, 1) == var_2) {
			num = 2;
		} else {
			num = 1;
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num));
}

void ScummEngine_v6::o6_getDateTime() {
	TimeDate t;
	_system->getTimeAndDate(t);

	VAR(VAR_TIMEDATE_YEAR) = t.tm_year;
	VAR(VAR_TIMEDATE_MONTH) = t.tm_mon;
	VAR(VAR_TIMEDATE_DAY) = t.tm_mday;
	VAR(VAR_TIMEDATE_HOUR) = t.tm_hour;
	VAR(VAR_TIMEDATE_MINUTE) = t.tm_min;

	if (_game.version == 8)
		VAR(VAR_TIMEDATE_SECOND) = t.tm_sec;
}

void ScummEngine_v6::o6_getPixel() {
	int x, y;

	if (_game.heversion >= 61 && _game.heversion <= 62) {
		x = pop();
		y = pop();
	} else {
		y = pop();
		x = pop();
	}

	VirtScreen *vs = findVirtScreen(y);

	if (vs == nullptr || x > _screenWidth - 1 || x < 0) {
		push(-1);
		return;
	}

	byte pixel = *vs->getPixels(x, y - vs->topline);
	push(pixel);
}

void ScummEngine_v6::o6_setBoxSet() {
	int arg = pop() - 1;

	const byte *room = getResourceAddress(rtRoom, _roomResource);
	const byte *boxd = nullptr, *boxm = nullptr;
	int32 dboxSize, mboxSize;
	int i;

	ResourceIterator boxds(room, false);
	for (i = 0; i < arg; i++)
		boxd = boxds.findNext(MKTAG('B','O','X','D'));

	if (!boxd)
		error("ScummEngine_v6::o6_setBoxSet: Can't find dboxes for set %d", arg);

	dboxSize = READ_BE_UINT32(boxd + 4) - 8;
	byte *matrix = _res->createResource(rtMatrix, 2, dboxSize);

	assert(matrix);
	memcpy(matrix, boxd + 8, dboxSize);

	ResourceIterator boxms(room, false);
	for (i = 0; i < arg; i++)
		boxm = boxms.findNext(MKTAG('B','O','X','M'));

	if (!boxm)
		error("ScummEngine_v6::o6_setBoxSet: Can't find mboxes for set %d", arg);

	mboxSize = READ_BE_UINT32(boxm + 4) - 8;
	matrix = _res->createResource(rtMatrix, 1, mboxSize);

	assert(matrix);
	memcpy(matrix, boxm + 8, mboxSize);

	if (_game.version == 7)
		putActors();
}

void ScummEngine_v6::decodeParseString(int m, int n) {
	byte b = fetchScriptByte();

	switch (b) {
	case SO_AT:
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case SO_COLOR:
		_string[m].color = pop();
		break;
	case SO_CLIPPED:
		_string[m].right = pop();
		break;
	case SO_CENTER:
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case SO_LEFT:
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case SO_OVERHEAD:
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case SO_MUMBLE:
		_string[m].no_talk_anim = true;
		break;
	case SO_TEXTSTRING:
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case SO_BASEOP:
		_string[m].loadDefault();
		if (n)
			_actorToPrintStrFor = pop();
		break;
	case SO_END:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

} // End of namespace Scumm
