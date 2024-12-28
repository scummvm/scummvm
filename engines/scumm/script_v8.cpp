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
#include "scumm/akos.h"
#include "scumm/charset.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm_v8.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/verbs.h"
#include "scumm/smush/smush_player.h"

#include "audio/mixer.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v8, x)

void ScummEngine_v8::setupOpcodes() {
	/* 00 */
	OPCODE(0x01, o6_pushWord);
	OPCODE(0x02, o6_pushWordVar);
	OPCODE(0x03, o6_wordArrayRead);
	/* 04 */
	OPCODE(0x04, o6_wordArrayIndexedRead);
	OPCODE(0x05, o6_dup);
	OPCODE(0x06, o6_pop);
	OPCODE(0x07, o6_not);
	/* 08 */
	OPCODE(0x08, o6_eq);
	OPCODE(0x09, o6_neq);
	OPCODE(0x0a, o6_gt);
	OPCODE(0x0b, o6_lt);
	/* 0C */
	OPCODE(0x0c, o6_le);
	OPCODE(0x0d, o6_ge);
	OPCODE(0x0e, o6_add);
	OPCODE(0x0f, o6_sub);
	/* 10 */
	OPCODE(0x10, o6_mul);
	OPCODE(0x11, o6_div);
	OPCODE(0x12, o6_land);
	OPCODE(0x13, o6_lor);
	/* 14 */
	OPCODE(0x14, o6_band);
	OPCODE(0x15, o6_bor);
	OPCODE(0x16, o8_mod);
	/* 18 */
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
	/* 44 */
	/* 48 */
	/* 4C */
	/* 50 */
	/* 54 */
	/* 58 */
	/* 5C */
	/* 60 */
	/* 64 */
	OPCODE(0x64, o6_if);
	OPCODE(0x65, o6_ifNot);
	OPCODE(0x66, o6_jump);
	OPCODE(0x67, o6_breakHere);
	/* 68 */
	OPCODE(0x68, o6_delayFrames);
	OPCODE(0x69, o8_wait);
	OPCODE(0x6a, o6_delay);
	OPCODE(0x6b, o6_delaySeconds);
	/* 6C */
	OPCODE(0x6c, o6_delayMinutes);
	OPCODE(0x6d, o6_writeWordVar);
	OPCODE(0x6e, o6_wordVarInc);
	OPCODE(0x6f, o6_wordVarDec);
	/* 70 */
	OPCODE(0x70, o8_dimArray);
	OPCODE(0x71, o6_wordArrayWrite);
	OPCODE(0x72, o6_wordArrayInc);
	OPCODE(0x73, o6_wordArrayDec);
	/* 74 */
	OPCODE(0x74, o8_dim2dimArray);
	OPCODE(0x75, o6_wordArrayIndexedWrite);
	OPCODE(0x76, o8_arrayOps);
	/* 78 */
	OPCODE(0x79, o6_startScript);
	OPCODE(0x7a, o6_startScriptQuick);
	OPCODE(0x7b, o6_stopObjectCode);
	/* 7C */
	OPCODE(0x7c, o6_stopScript);
	OPCODE(0x7d, o6_jumpToScript);
	OPCODE(0x7e, o6_dummy);				// O_RETURN boils down to a NOP
	OPCODE(0x7f, o6_startObject);
	/* 80 */
	OPCODE(0x80, o6_stopObjectScript);
	OPCODE(0x81, o6_cutscene);
	OPCODE(0x82, o6_endCutscene);
	OPCODE(0x83, o6_freezeUnfreeze);
	/* 84 */
	OPCODE(0x84, o6_beginOverride);
	OPCODE(0x85, o6_endOverride);
	OPCODE(0x86, o6_stopSentence);
	/* 88 */
	OPCODE(0x89, o6_setClass);
	OPCODE(0x8a, o6_setState);
	OPCODE(0x8b, o6_setOwner);
	/* 8C */
	OPCODE(0x8c, o6_panCameraTo);
	OPCODE(0x8d, o6_actorFollowCamera);
	OPCODE(0x8e, o6_setCameraAt);
	OPCODE(0x8f, o6_printActor);
	/* 90 */
	OPCODE(0x90, o6_printEgo);
	OPCODE(0x91, o6_talkActor);
	OPCODE(0x92, o6_talkEgo);
	OPCODE(0x93, o6_printLine);
	/* 94 */
	OPCODE(0x94, o6_printText);
	OPCODE(0x95, o6_printDebug);
	OPCODE(0x96, o6_printSystem);
	OPCODE(0x97, o8_blastText);
	/* 98 */
	OPCODE(0x98, o8_drawObject);
	/* 9C */
	OPCODE(0x9c, o8_cursorCommand);
	OPCODE(0x9d, o6_loadRoom);
	OPCODE(0x9e, o6_loadRoomWithEgo);
	OPCODE(0x9f, o6_walkActorToObj);
	/* A0 */
	OPCODE(0xa0, o6_walkActorTo);
	OPCODE(0xa1, o6_putActorAtXY);
	OPCODE(0xa2, o6_putActorAtObject);
	OPCODE(0xa3, o6_faceActor);
	/* A4 */
	OPCODE(0xa4, o6_animateActor);
	OPCODE(0xa5, o6_doSentence);
	OPCODE(0xa6, o6_pickupObject);
	OPCODE(0xa7, o6_setBoxFlags);
	/* A8 */
	OPCODE(0xa8, o6_createBoxMatrix);
	OPCODE(0xaa, o8_resourceRoutines);
	OPCODE(0xab, o8_roomOps);
	/* AC */
	OPCODE(0xac, o8_actorOps);
	OPCODE(0xad, o8_cameraOps);
	OPCODE(0xae, o8_verbOps);
	OPCODE(0xaf, o6_startSound);
	/* B0 */
	OPCODE(0xb0, o6_startMusic);
	OPCODE(0xb1, o6_stopSound);
	OPCODE(0xb2, o6_soundKludge);
	OPCODE(0xb3, o8_systemOps);
	/* B4 */
	OPCODE(0xb4, o6_saveRestoreVerbs);
	OPCODE(0xb5, o6_setObjectName);
	OPCODE(0xb6, o6_getDateTime);
	OPCODE(0xb7, o6_drawBox);
	/* B8 */
	OPCODE(0xb9, o8_startVideo);
	OPCODE(0xba, o8_kernelSetFunctions);
	/* BC */
	/* C0 */
	/* C4 */
	/* C8 */
	OPCODE(0xc8, o6_startScriptQuick2);
	OPCODE(0xc9, o6_startObjectQuick);
	OPCODE(0xca, o6_pickOneOf);
	OPCODE(0xcb, o6_pickOneOfDefault);
	/* CC */
	OPCODE(0xcd, o6_isAnyOf);
	OPCODE(0xce, o6_getRandomNumber);
	OPCODE(0xcf, o6_getRandomNumberRange);
	/* D0 */
	OPCODE(0xd0, o6_ifClassOfIs);
	OPCODE(0xd1, o6_getState);
	OPCODE(0xd2, o6_getOwner);
	OPCODE(0xd3, o6_isScriptRunning);
	/* D4 */
	OPCODE(0xd5, o6_isSoundRunning);
	OPCODE(0xd6, o6_abs);
	/* D8 */
	OPCODE(0xd8, o8_kernelGetFunctions);
	OPCODE(0xd9, o6_isActorInBox);
	OPCODE(0xda, o6_getVerbEntrypoint);
	OPCODE(0xdb, o6_getActorFromXY);
	/* DC */
	OPCODE(0xdc, o6_findObject);
	OPCODE(0xdd, o6_getVerbFromXY);
	OPCODE(0xdf, o6_findInventory);
	/* E0 */
	OPCODE(0xe0, o6_getInventoryCount);
	OPCODE(0xe1, o6_getAnimateVariable);
	OPCODE(0xe2, o6_getActorRoom);
	OPCODE(0xe3, o6_getActorWalkBox);
	/* E4 */
	OPCODE(0xe4, o6_getActorMoving);
	OPCODE(0xe5, o6_getActorCostume);
	OPCODE(0xe6, o6_getActorScaleX);
	OPCODE(0xe7, o6_getActorLayer);
	/* E8 */
	OPCODE(0xe8, o6_getActorElevation);
	OPCODE(0xe9, o6_getActorWidth);
	OPCODE(0xea, o6_getObjectNewDir);
	OPCODE(0xeb, o6_getObjectX);
	/* EC */
	OPCODE(0xec, o6_getObjectY);
	OPCODE(0xed, o8_getActorChore);
	OPCODE(0xee, o6_distObjectObject);
	OPCODE(0xef, o6_distPtPt);
	/* F0 */
	OPCODE(0xf0, o8_getObjectImageX);
	OPCODE(0xf1, o8_getObjectImageY);
	OPCODE(0xf2, o8_getObjectImageWidth);
	OPCODE(0xf3, o8_getObjectImageHeight);
	/* F4 */
	OPCODE(0xf6, o8_getStringWidth);
	OPCODE(0xf7, o8_getActorZPlane);
	/* F8 */
	/* FC */
}

// In V8, the word size is 4 byte, not 2 bytes as in V6/V7 games
uint ScummEngine_v8::fetchScriptWord() {
	return fetchScriptDWord();
}

int ScummEngine_v8::fetchScriptWordSigned() {
	return (int32)fetchScriptDWordSigned();
}

int ScummEngine_v8::readVar(uint var) {
	debugC(DEBUG_VARS, "readvar(%d)", var);

	// The following action re-enables the song at the beginning of Part 3,
	// which was disabled for international releases, if the user decides so.
	if (_enableCOMISong &&
		VAR_LANGUAGE != 0xFF && var == VAR_LANGUAGE &&
		vm.slot[_currentScript].number == 319 && _currentRoom == 52)
		return 0;

	if (!(var & 0xF0000000)) {
		assertRange(0, var, _numVariables - 1, "variable");
		return _scummVars[var];
	}

	if (var & 0x80000000) {
		var &= 0x7FFFFFFF;
		assertRange(0, var, _numBitVariables - 1, "bit variable (reading)");
		return (_bitVars[var >> 3] & (1 << (var & 7))) ? 1 : 0;
	}

	if (var & 0x40000000) {
		var &= 0xFFFFFFF;
		assertRange(0, var, 25, "local variable (reading)");
		return vm.localvar[_currentScript][var];
	}

	error("Illegal varbits (r)");
	return -1;
}

void ScummEngine_v8::writeVar(uint var, int value) {
	debugC(DEBUG_VARS, "writeVar(%d, %d)", var, value);

	if (!(var & 0xF0000000)) {
		assertRange(0, var, _numVariables - 1, "variable (writing)");

		if (!isUsingOriginalGUI() && var == VAR_CHARINC) {
			// Did the user override the talkspeed manually? Then use that.
			// Otherwise, use the value specified by the game script.
			// Note: To determine whether there was a user override, we only
			// look at the target specific settings, assuming that any global
			// value is likely to be bogus. See also bug #4008.
			if (ConfMan.hasKey("talkspeed", _targetName)) {
				value = getTalkSpeed();
			} else {
				// Save the new talkspeed value to ConfMan
				setTalkSpeed(value);
			}
		}

		_scummVars[var] = value;

		if ((_varwatch == (int)var) || (_varwatch == 0)) {
			if (vm.slot[_currentScript].number < 100)
				debugC(DEBUG_VARS, "vars[%d] = %d (via script-%d)", var, value, vm.slot[_currentScript].number);
			else
				debugC(DEBUG_VARS, "vars[%d] = %d (via room-%d-%d)", var, value, _currentRoom, vm.slot[_currentScript].number);
		}
		return;
	}

	if (var & 0x80000000) {
		var &= 0x7FFFFFFF;
		assertRange(0, var, _numBitVariables - 1, "bit variable (writing)");

		if (value)
			_bitVars[var >> 3] |= (1 << (var & 7));
		else
			_bitVars[var >> 3] &= ~(1 << (var & 7));
		return;
	}

	if (var & 0x40000000) {
		var &= 0xFFFFFFF;
		assertRange(0, var, 25, "local variable (writing)");
		vm.localvar[_currentScript][var] = value;
		return;
	}

	error("Illegal varbits (w)");
}

void ScummEngine_v8::setKeyScriptVars(int keyScriptKey, int keyScriptNo) {
	_keyScriptKey = keyScriptKey;
	_keyScriptNo = keyScriptNo;
}

void ScummEngine_v8::decodeParseString(int m, int n) {
	byte b = fetchScriptByte();

	switch (b) {
	case SO_BASEOP:
		_string[m].loadDefault();
		if (n)
			_actorToPrintStrFor = pop();
		break;
	case SO_END:
		_string[m].saveDefault();
		break;
	case SO_AT:
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case SO_COLOR:
		_string[m].color = pop();
		break;
	case SO_CENTER:
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case SO_PRINT_CHARSET:		// Set print character set
		_string[m].charset = pop();
		break;
	case SO_LEFT:
		_string[m].wrapping = false;
		_string[m].overhead = false;
		break;
	case SO_OVERHEAD:
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case SO_MUMBLE:
		_string[m].no_talk_anim = true;
		break;
	case SO_PRINT_STRING:
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case SO_PRINT_WRAP:		// Set print wordwrap
		_string[m].wrapping = true;
		_string[m].overhead = false;
		break;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

void ScummEngine_v8::readArrayFromIndexFile() {
	int num;
	int a, b;

	while ((num = _fileHandle->readUint32LE()) != 0) {
		a = _fileHandle->readUint32LE();
		b = _fileHandle->readUint32LE();

		if (b != 0)
			defineArray(num, kIntArray, b, a);
		else
			defineArray(num, kIntArray, a, b);
	}
}

void ScummEngine_v8::o8_mod() {
	int a = pop();
	push(pop() % a);
}

void ScummEngine_v8::o8_wait() {
	int actnum;
	int offs = -2;
	Actor *a;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_WAIT_FOR_ACTOR:		// Wait for actor (to finish current action?)
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o8_wait:SO_WAIT_FOR_ACTOR");
		if (a->isInCurrentRoom() && a->_moving)
			break;
		return;
	case SO_WAIT_FOR_MESSAGE:		// Wait for message
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case SO_WAIT_FOR_CAMERA:		// Wait for camera (to finish current action?)
		if (camera._dest != camera._cur)
			break;
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
		a = derefActor(actnum, "o8_wait:SO_WAIT_FOR_ANIMATION");
		if (a->isInCurrentRoom() && a->_needRedraw)
			break;
		return;
	case SO_WAIT_FOR_TURN:
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o8_wait:SO_WAIT_FOR_TURN");
		if (a->isInCurrentRoom() && a->_moving & MF_TURN)
			break;
		return;
	default:
		error("o8_wait: default case 0x%x", subOp);
	}

	_scriptPointer += offs;
	o6_breakHere();
}

void ScummEngine_v8::o8_dimArray() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();

	switch (subOp) {
	case SO_INT_ARRAY:
		defineArray(array, kIntArray, 0, pop());
		break;
	case SO_STRING_ARRAY:
		defineArray(array, kStringArray, 0, pop());
		break;
	case SO_UNDIM_ARRAY:
		nukeArray(array);
		break;
	default:
		error("o8_dimArray: default case 0x%x", subOp);
	}
}

void ScummEngine_v8::o8_dim2dimArray() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord(), a, b;

	switch (subOp) {
	case SO_INT_ARRAY:
		b = pop();
		a = pop();
		defineArray(array, kIntArray, a, b);
		break;
	case SO_STRING_ARRAY:
		b = pop();
		a = pop();
		defineArray(array, kStringArray, a, b);
		break;
	case SO_UNDIM_ARRAY:
		nukeArray(array);
		break;
	default:
		error("o8_dim2dimArray: default case 0x%x", subOp);
	}
}

void ScummEngine_v8::o8_arrayOps() {
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
		len = getStackList(list, ARRAYSIZE(list));
		d = readVar(array);
		if (d == 0) {
			defineArray(array, kIntArray, 0, b + len);
		}
		while (--len >= 0) {
			writeArray(array, 0, b + len, list[len]);
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
		error("o8_arrayOps: default case 0x%x (array %d)", subOp, array);
	}
}

void ScummEngine_v8::o8_blastText() {
	// Original V8 interpreter uses StringSlot 2 for o_blastText and 4 for o_printDebug.
	// Since slot 2 is already mapped to printDebug for V6 (see ScummEngine::printString()),
	// we just "swap" the slots, and use slot 4 here.
	decodeParseString(4, 0);
}

void ScummEngine_v8::o8_cursorCommand() {
	byte subOp = fetchScriptByte();
	int a;
	int args[4];

	switch (subOp) {
	case SO_CURSOR_ON:		// Turn cursor on
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case SO_CURSOR_OFF:		// Turn cursor off
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case SO_CURSOR_SOFT_ON:		// Turn soft cursor on
		_cursor.state++;
		verbMouseOver(0);
		break;
	case SO_CURSOR_SOFT_OFF:		// Turn soft cursor off
		_cursor.state--;
		verbMouseOver(0);
		break;
	case SO_USERPUT_ON:
		_userPut = 1;
		break;
	case SO_USERPUT_OFF:
		_userPut = 0;
		break;
	case SO_USERPUT_SOFT_ON:
		_userPut++;
		break;
	case SO_USERPUT_SOFT_OFF:
		_userPut--;
		break;
	case SO_CURSOR_IMAGE:		// Set cursor image
		{
			int idx = pop();
			int room, obj;
			obj = popRoomAndObj(&room);
			setCursorFromImg(obj, room, idx);
		}
		break;
	case SO_CURSOR_HOTSPOT:		// Set cursor hotspot
		a = pop();
		setCursorHotspot(pop(), a);
		break;
	case SO_CURSOR_TRANSPARENT:		// Set cursor transparent color
		setCursorTransparency(pop());
		break;
	case SO_CHARSET_SET:
		_string[0]._default.charset = pop();
		break;
	case SO_CHARSET_COLOR:
		getStackList(args, ARRAYSIZE(args));
		// This opcode does nothing (confirmed with disasm)
		break;
	case SO_CURSOR_PUT:
		{
		int y = pop();
		int x = pop();

		_system->warpMouse(x, y);
		}
		break;
	default:
		error("o8_cursorCommand: default case 0x%x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void ScummEngine_v8::o8_resourceRoutines() {
	byte subOp = fetchScriptByte();
	int resid = pop();

	switch (subOp) {
	case SO_HEAP_LOAD_CHARSET:		// Dummy case
		break;
	case SO_HEAP_LOAD_COSTUME:		// Load costume to heap
		ensureResourceLoaded(rtCostume, resid);
		break;
	case SO_HEAP_LOAD_OBJECT:		// Load object to heap
		{
		int room = getObjectRoom(resid);
		loadFlObject(resid, room);
		}
		break;
	case SO_HEAP_LOAD_ROOM:		// Load room to heap
		ensureResourceLoaded(rtRoom, resid);
		break;
	case SO_HEAP_LOAD_SCRIPT:		// Load script to heap
		ensureResourceLoaded(rtScript, resid);
		break;
	case SO_HEAP_LOAD_SOUND:		// Load sound to heap
		ensureResourceLoaded(rtSound, resid);
		break;

	case SO_HEAP_LOCK_COSTUME:		// Lock costume in heap
		_res->lock(rtCostume, resid);
		break;
	case SO_HEAP_LOCK_ROOM:		// Lock room in heap
		_res->lock(rtRoom, resid);
		break;
	case SO_HEAP_LOCK_SCRIPT:		// Lock script in heap
		_res->lock(rtScript, resid);
		break;
	case SO_HEAP_LOCK_SOUND:		// Lock sound in heap
		_res->lock(rtSound, resid);
		break;
	case SO_HEAP_UNLOCK_COSTUME:		// Unlock costume
		_res->unlock(rtCostume, resid);
		break;
	case SO_HEAP_UNLOCK_ROOM:		// Unlock room
		_res->unlock(rtRoom, resid);
		break;
	case SO_HEAP_UNLOCK_SCRIPT:		// Unlock script
		_res->unlock(rtScript, resid);
		break;
	case SO_HEAP_UNLOCK_SOUND:		// Unlock sound
		_res->unlock(rtSound, resid);
		break;
	case SO_HEAP_NUKE_COSTUME:		// Remove costume from heap
		_res->setResourceCounter(rtCostume, resid, 0x7F);
		break;
	case SO_HEAP_NUKE_ROOM:		// Remove room from heap
		_res->setResourceCounter(rtRoom, resid, 0x7F);
		break;
	case SO_HEAP_NUKE_SCRIPT:		// Remove script from heap
		_res->setResourceCounter(rtScript, resid, 0x7F);
		break;
	case SO_HEAP_NUKE_SOUND:		// Remove sound from heap
		_res->setResourceCounter(rtSound, resid, 0x7F);
		break;
	default:
		error("o8_resourceRoutines: default case 0x%x", subOp);
	}
}

void ScummEngine_v8::o8_roomOps() {
	byte subOp = fetchScriptByte();
	int a, b, c, d, e;

	switch (subOp) {
	case SO_ROOM_PALETTE:		// Set room palette
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;
	case SO_ROOM_FADE:		// Fade room
		a = pop();
		if (a) {
			_switchRoomEffect = (byte)(a);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;
	case SO_ROOM_RGB_INTENSITY:		// Set room color intensity
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;
	case SO_ROOM_TRANSFORM:		// Transform room
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		palManipulateInit(a, b, c, d);
		break;
	case SO_ROOM_NEW_PALETTE:		// New palette
		a = pop();
		setCurrentPalette(a);
		break;
	case SO_ROOM_SAVE_GAME:		// Save game
		_saveSound = 0;
		_saveTemporaryState = true;
		_saveLoadSlot = 1;
		_saveLoadFlag = 1;
		break;
	case SO_ROOM_LOAD_GAME:		// Load game
		_saveSound = pop();
		if (!_saveLoadFlag) {
			_saveTemporaryState = true;
			_saveLoadSlot = 1;
			_saveLoadFlag = 2;
		}
		break;
	case SO_ROOM_SATURATION:		// Set saturation of room colors
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		desaturatePalette(a, b, c, d, e);
		break;
	default:
		error("o8_roomOps: default case 0x%x", subOp);
	}
}

void ScummEngine_v8::o8_actorOps() {
	byte subOp = fetchScriptByte();
	Actor *a;
	int i, j;

	if (subOp == SO_ACTOR_INIT) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o8_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case SO_COSTUME:		// Set actor costume
		a->setActorCostume(pop());
		break;
	case SO_STEP_DIST:		// Set actor width of steps
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case SO_ANIMATION_DEFAULT:		// Set actor animation to default
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case SO_INIT_ANIMATION:		// Initialize animation
		a->_initFrame = pop();
		break;
	case SO_TALK_ANIMATION:		// Set actor animation to talk animation
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case SO_WALK_ANIMATION:		// Set actor animation to walk animation
		a->_walkFrame = pop();
		break;
	case SO_STAND_ANIMATION:		// Set actor animation to standing animation
		a->_standFrame = pop();
		break;
	case SO_ANIMATION_SPEED:		// Set speed of animation
		a->setAnimSpeed(pop());
		break;
	case SO_DEFAULT:
		a->initActor(0);
		break;
	case SO_ELEVATION:
		a->setElevation(pop());
		break;
	case SO_PALETTE:		// Set actor palette
		j = pop();
		i = pop();
		assertRange(0, i, 31, "o8_actorOps: palette slot");
		a->setPalette(i, j);
		break;
	case SO_TALK_COLOR:		// Set actor talk color
		a->_talkColor = pop();
		break;
	case SO_ACTOR_NAME:		// Set name of actor
		loadPtrToResource(rtActorName, a->_number, NULL);
		break;
	case SO_ACTOR_WIDTH:		// Set width of actor
		a->_width = pop();
		break;
	case SO_SCALE:		// Set scaling of actor
		i = pop();
		a->setScale(i, i);
		break;
	case SO_NEVER_ZCLIP:
		a->_forceClip = 0;
		break;
	case SO_ALWAYS_ZCLIP:
		a->_forceClip = pop();
		// V8 uses 255 where we used to use 100
		if (a->_forceClip == 255)
			a->_forceClip = 100;
		break;
	case SO_IGNORE_BOXES:		// Make actor ignore boxes
		a->_ignoreBoxes = true;
		a->_forceClip = 100;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_FOLLOW_BOXES:		// Make actor follow boxes
		a->_ignoreBoxes = false;
		a->_forceClip = 100;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_SHADOW:
		a->_shadowMode = pop();
		break;
	case SO_TEXT_OFFSET:		// Set text offset relative to actor
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
//	case SO_ACTOR_INIT:		// Set current actor (handled above)
	case SO_ACTOR_VARIABLE:		// Set actor variable
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case SO_ACTOR_IGNORE_TURNS_ON:		// Make actor ignore turns
		a->_ignoreTurns = true;
		break;
	case SO_ACTOR_IGNORE_TURNS_OFF:		// Make actor follow turns
		a->_ignoreTurns = false;
		break;
	case SO_NEW:		// New actor
		a->initActor(2);
		break;
	case SO_ACTOR_DEPTH:		// Set actor Z position
		a->_layer = pop();
		break;
	case SO_ACTOR_STOP:
		a->stopActorMoving();
		a->startAnimActor(a->_standFrame);
		break;
	case SO_ACTOR_FACE:		// Make actor face angle
		a->_moving &= ~MF_TURN;
		j = pop();
		a->turnToDirection(j);
		a->setDirection(j);
		break;
	case SO_ACTOR_TURN:		// Turn actor
		a->turnToDirection(pop());
		break;
	case SO_ACTOR_WALK_SCRIPT:		// Set walk script for actor?
		a->_walkScript = pop();
		break;
	case SO_ACTOR_TALK_SCRIPT:		// Set talk script for actor?
		a->_talkScript = pop();
		break;
	case SO_ACTOR_WALK_PAUSE:
		a->_moving |= MF_FROZEN;
		break;
	case SO_ACTOR_WALK_RESUME:
		a->_moving &= ~MF_FROZEN;
		break;
	case SO_ACTOR_VOLUME:		// Set volume of actor speech
		a->_talkVolume = pop();
		break;
	case SO_ACTOR_FREQUENCY:		// Set frequency of actor speech
		a->_talkFrequency = pop();
		break;
	case SO_ACTOR_PAN:
		a->_talkPan = pop();
		break;
	default:
		error("o8_actorOps: default case 0x%x", subOp);
	}
}

void ScummEngine_v8::o8_cameraOps() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_CAMERA_PAUSE:
		_cameraIsFrozen = true;
		break;
	case SO_CAMERA_RESUME:
		_cameraIsFrozen = false;
		break;
	default:
		error("o8_cameraOps: default case 0x%x", subOp);
	}
}

void ScummEngine_v8::o8_verbOps() {
	byte subOp = fetchScriptByte();
	VerbSlot *vs = NULL;
	int slot, a, b;

	if (subOp == SO_VERB_INIT) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		assertRange(0, _curVerbSlot, _numVerbs - 1, "new verb slot");
		return;
	}

	assert(0 <= _curVerbSlot && _curVerbSlot < _numVerbs);
	vs = &_verbs[_curVerbSlot];
	assert(vs);

	switch (subOp) {
	case SO_VERB_INIT:		// Choose verb number for editing
		// handled above!
		break;
	case SO_VERB_NEW:		// New verb
		if (_curVerbSlot == 0) {
			for (slot = 1; slot < _numVerbs; slot++) {
				if (_verbs[slot].verbid == 0)
					break;
			}
			if (slot >= _numVerbs) {
				error("Too many verbs");
			}
			_curVerbSlot = slot;
		}
		vs = &_verbs[_curVerbSlot];
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
	case SO_VERB_DELETE:		// Delete verb
		killVerb(_curVerbSlot);
		break;
	case SO_VERB_NAME:		// Set verb name
		loadPtrToResource(rtVerb, _curVerbSlot, NULL);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case SO_VERB_AT:		// Set verb (X,Y) placement
		vs->curRect.top = pop();
		vs->origLeft = pop();
		break;
	case SO_VERB_ON:		// Turn verb on
		vs->curmode = 1;
		break;
	case SO_VERB_OFF:		// Turn verb off
		vs->curmode = 0;
		break;
	case SO_VERB_COLOR:		// Set verb color
		vs->color = pop();
		break;
	case SO_VERB_HICOLOR:		// Set verb highlighted color
		vs->hicolor = pop();
		break;
	case SO_VERB_DIMCOLOR:		// Set verb dimmed (disabled) color
		vs->dimcolor = pop();
		break;
	case SO_VERB_DIM:
		vs->curmode = 2;
		break;
	case SO_VERB_KEY:		// Set keypress to associate with verb
		vs->key = pop();
		break;
	case SO_VERB_IMAGE:		// Set verb image
		b = pop();
		a = pop();
		if (_curVerbSlot && a != vs->imgindex) {
			setVerbObject(b, a, _curVerbSlot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case SO_VERB_NAME_STR:		// Set verb name
		a = pop();
		if (a == 0) {
			loadPtrToResource(rtVerb, _curVerbSlot, (const byte *)"");
		} else {
			loadPtrToResource(rtVerb, _curVerbSlot, getStringAddress(a));
		}
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case SO_VERB_CENTER:		// Center verb
		vs->center = 1;
		break;
	case SO_VERB_CHARSET:		// Choose charset for verb
		vs->charset_nr = pop();
		break;
	case SO_VERB_LINE_SPACING:		// Choose linespacing for verb
		_verbLineSpacing = pop();
		break;
	default:
		error("o8_verbops: default case 0x%x", subOp);
	}
}

void ScummEngine_v8::o8_systemOps() {
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case SO_RESTART:		// Restart game
		restart();
		break;
	case SO_QUIT:		// Quit game
		_quitFromScriptCmd = true;
		quitGame();
		break;
	default:
		error("o8_systemOps: invalid case 0x%x", subOp);
	}
}


void ScummEngine_v8::o8_startVideo() {
	int len = resStrLen(_scriptPointer);

	_splayer->play((const char*)_scriptPointer, 12);

	_scriptPointer += len + 1;
}

void ScummEngine_v8::o8_kernelSetFunctions() {
	// TODO
	Actor *a;
	int args[30];
	int len = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 11: {	// lockObject
		int objidx = getObjectIndex(args[1]);
		assert(objidx != -1);
		_res->lock(rtFlObject, _objs[objidx].fl_object_index);
		break;
	}
	case 12: {	// unlockObject
		int objidx = getObjectIndex(args[1]);
		assert(objidx != -1);
		_res->unlock(rtFlObject, _objs[objidx].fl_object_index);
		break;
	}
	case 13:	// remapCostume
		a = derefActor(args[1], "o8_kernelSetFunctions:remapCostume");
		a->remapActorPalette(args[2], args[3], args[4], -1);
		break;
	case 14:	// remapCostumeInsert
		a = derefActor(args[1], "o8_kernelSetFunctions:remapCostumeInsert");
		a->remapActorPalette(args[2], args[3], args[4], args[5]);
		break;
	case 15:	// setVideoFrameRate
		// not used anymore (was smush frame rate)
		break;
	case 20:	// setBoxScaleSlot
		setBoxScaleSlot(args[1], args[2]);
		break;
	case 21:	// setScaleSlot
		setScaleSlot(args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
		break;
	case 22:	// setBannerColors
		debug(5, "o8_kernelSetFunctions: setBannerColors(%d, %d, %d, %d)", args[1], args[2], args[3], args[4]);
		setBannerColors(args[1], args[2], args[3], args[4]);
		break;
	case 23:	// setActorChoreLimbFrame
		a = derefActor(args[1], "o8_kernelSetFunctions:setActorChoreLimbFrame");

		a->startAnimActor(args[2]);
		a->animateLimb(args[3], args[4]);
		break;
	case 24:	// clearTextQueue
		removeBlastTexts();
		break;
	case 25: {	// saveGameReadName
		Common::String name;
		if (getSavegameName(args[1], name)) {
			int size = name.size() + 1;
			_res->nukeResource(rtString, args[2]);

			ArrayHeader *ah = (ArrayHeader *)_res->createResource(rtString, args[2], size + sizeof(ArrayHeader));
			ah->type = TO_LE_16(kStringArray);
			ah->dim1 = TO_LE_16(size + 1);
			ah->dim2 = TO_LE_16(1);

			memcpy(getStringAddress(args[2]), name.c_str(), size);
		}
		break;
	}
	case 26: { // saveGameWrite
		char *saveName = (char *)getStringAddress(args[2]);
		debug(0, "o8_kernelSetFunctions: saveGame(%d, %s)", args[1], saveName);
		if (isUsingOriginalGUI()) {
			copyHeapSaveGameToFile(args[1], saveName);
		}
		break;
	}
	case 27: // saveGameRead
		_saveLoadSlot = args[1];
		_saveLoadFlag = 2;
		_saveTemporaryState = false;
		break;
	case 28:	// stampShotEnqueue
		debug(8, "o8_kernelSetFunctions: stampShotEnqueue(%d, %d, %d, %d, %d, %d)", args[1], args[2], args[3], args[4], args[5], args[6]);
		stampShotEnqueue(args[1], args[2], args[3], args[4], args[5], args[6]);
		break;
	case 29:	// setKeyScript
		setKeyScriptVars(args[1], args[2]);
		break;
	case 30:	// killAllScriptsButMe
		killAllScriptsExceptCurrent();
		break;
	case 31:	// stopAllVideo
		debug(0, "o8_kernelSetFunctions: stopAllVideo()");
		break;
	case 32:	// writeRegistryValue
		{
		int idx = args[1];
		int value = args[2];
		const char *str = (const char *)getStringAddress(idx);
		if (isUsingOriginalGUI()) {
			if (!strcmp(str, "SFX Volume")) {
				ConfMan.setInt("sfx_volume", value * 2);
			} else if (!strcmp(str, "Voice Volume")) {
				ConfMan.setInt("speech_volume", value * 2);
			} else if (!strcmp(str, "Music Volume")) {
				ConfMan.setInt("music_volume", value * 2);
			} else if (!strcmp(str, "Text Status")) {
				ConfMan.setInt("original_gui_text_status", value);
				ConfMan.setBool("speech_mute", value == 2);
				ConfMan.setBool("subtitles", value > 0);
			} else if (!strcmp(str, "Text Speed")) {
				ConfMan.setInt("original_gui_text_speed", value);
				setTalkSpeed(value);
			} else if (!strcmp(str, "Object Names")) {
				ConfMan.setInt("original_gui_object_labels", value);
				ConfMan.setBool("object_labels", value > 0);
			} else if (!strcmp(str, "Saveload Page")) {
				ConfMan.setInt("original_gui_saveload_page", value);
			}

			ConfMan.flushToDisk();
		}

		debugC(DEBUG_GENERAL,"o8_kernelSetFunctions: writeRegistryValue(%s, %d)", str, value);
		}
		break;
	case 33:	// paletteSetIntensity
		debug(0, "o8_kernelSetFunctions: paletteSetIntensity(%d, %d)", args[1], args[2]);
		break;
	case 34:	// queryQuit
		if (isUsingOriginalGUI()) {
			// Create an artificial CTRL-C keyPress
			_keyPressed = Common::KEYCODE_c;
			_keyPressed.flags |= Common::KBD_CTRL;
			_quitFromScriptCmd = true;
		} else {
			if (ConfMan.getBool("confirm_exit"))
				confirmExitDialog();
			else
				quitGame();
		}
		break;
	case 108:	// buildPaletteShadow
		setShadowPalette(args[1], args[2], args[3], args[4], args[5], args[6]);
		break;
	case 109:	// setPaletteShadow
		setShadowPalette(0, args[1], args[2], args[3], args[4], args[5]);
		break;
	case 118:	// blastShadowObject
		enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 3);
		break;
	case 119:	// superBlastObject
		enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 0);
		break;

	default:
		error("o8_kernelSetFunctions: default case 0x%x (len = %d)", args[0], len);
	}
}

void ScummEngine_v8::o8_kernelGetFunctions() {
	int args[30];
	int len = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 0x73:	// getWalkBoxAt
		push(getSpecialBox(args[1], args[2]));
		break;
	case 0x74:	// isPointInBox
		push(checkXYInBoxBounds(args[3], args[1], args[2]));
		break;
	case 0xD3:		// getKeyState
		push(getKeyState(args[1]));
		break;
	case 0xCE:		// getRGBSlot
		push(remapPaletteColor(args[1], args[2], args[3], -1));
		break;
	case 0xD7:		// getBox
		push(checkXYInBoxBounds(args[3], args[1], args[2]));
		break;
	case 0xD8: {		// findBlastObject
		int x = args[1] + (camera._cur.x & 7);
		int y = args[2] + _screenTop;
		BlastObject *eo;

		for (int i = _blastObjectQueuePos - 1; i >= 0; i--) {
			eo = &_blastObjectQueue[i];

			if (eo->rect.contains(x, y) && !getClass(eo->number, kObjectClassUntouchable)) {
				push(eo->number);
				return;
			}
		}
		push(0);
		break;
	}
	case 0xD9: {   // actorHit - used, for example, to detect ship collision
	               // during ship-to-ship combat.
		Actor *a = derefActor(args[1], "actorHit");
		push(a->actorHitTest(args[2], args[3] + _screenTop));
		break;
	}
	case 0xDA:		// lipSyncWidth
		push(_imuseDigital->getCurVoiceLipSyncWidth());
		break;
	case 0xDB:		// lipSyncHeight
		push(_imuseDigital->getCurVoiceLipSyncHeight());
		break;
	case 0xDC:		// actorTalkAnimation
		{
		Actor *a = derefActor(args[1], "actorTalkAnimation");
		push(a->_talkStartFrame);
		}
		break;
	case 0xDD:		// getGroupSfxVol
		push(_imuseDigital->diMUSEGetSFXGroupVol());
		break;
	case 0xDE:		// getGroupVoiceVol
		push(_imuseDigital->diMUSEGetVoiceGroupVol());
		break;
	case 0xDF:		// getGroupMusicVol
		push(_imuseDigital->diMUSEGetMusicGroupVol());
		break;
	case 0xE0:		// readRegistryValue
		{
		int idx = args[1];
		const char *str = (const char *)getStringAddress(idx);
		if (!strcmp(str, "SFX Volume")) {
			push(ConfMan.getInt("sfx_volume") / 2);
		} else if (!strcmp(str, "Voice Volume")) {
			push(ConfMan.getInt("speech_volume") / 2);
		} else if (!strcmp(str, "Music Volume")) {
			push(ConfMan.getInt("music_volume") / 2);
		} else if (!strcmp(str, "Text Status")) {
			if (ConfMan.hasKey("original_gui_text_status", _targetName) && isUsingOriginalGUI()) {
				push(ConfMan.getInt("original_gui_text_status"));
			} else if (ConfMan.hasKey("subtitles", _targetName)) {
				push(ConfMan.getBool("subtitles"));
			} else {
				push(-1); // Default value
			}
		} else if (!strcmp(str, "Text Speed")) {
			if (ConfMan.hasKey("original_gui_text_speed", _targetName) && isUsingOriginalGUI()) {
				push(ConfMan.getInt("original_gui_text_speed"));
			} else {
				push(-1); // Default value
			}
		} else if (!strcmp(str, "Object Names")) {
			if (ConfMan.hasKey("original_gui_object_labels", _targetName) && isUsingOriginalGUI()) {
				push(ConfMan.getInt("original_gui_object_labels"));
			} else if (ConfMan.hasKey("object_labels", _targetName)) {
				push(ConfMan.getBool("object_labels"));
			} else {
				push(-1); // Default value
			}
		} else if (!strcmp(str, "Saveload Page")) {
			if (ConfMan.hasKey("original_gui_saveload_page", _targetName) && isUsingOriginalGUI()) {
				push(ConfMan.getInt("original_gui_saveload_page"));
			} else {
				push(-1); // Default value
			}
		} else { // Use defaults
			push(-1);
		}

		debugC(DEBUG_GENERAL,"o8_kernelGetFunctions: readRegistryValue(%s)", str);
		}
		break;
	case 0xE1:		// imGetMusicPosition
		push(_imuseDigital->getCurMusicPosInMs());
		break;
	case 0xE2:		// musicLipSyncWidth
		push(_imuseDigital->getCurMusicLipSyncWidth(args[1]));
		break;
	case 0xE3:		// musicLipSyncHeight
		push(_imuseDigital->getCurMusicLipSyncHeight(args[1]));
		break;
	default:
		error("o8_kernelGetFunctions: default case 0x%x (len = %d)", args[0], len);
	}

}

void ScummEngine_v8::o8_getActorChore() {
	int actnum = pop();
	Actor *a = derefActor(actnum, "o8_getActorChore");
	push(a->_frame);
}

void ScummEngine_v8::o8_getActorZPlane() {
	int actnum = pop();
	Actor *a = derefActor(actnum, "o8_getActorZPlane");

	int z = a->_forceClip;
	if (z == 100) {
		z = getMaskFromBox(a->_walkbox);
		if (z > _gdi->_numZBuffer - 1)
			z = _gdi->_numZBuffer - 1;
	}

	push(z);
}


void ScummEngine_v8::o8_getObjectImageX() {
	int i = getObjectIndex(pop());
	assert(i);
	push(_objs[i].x_pos);
}

void ScummEngine_v8::o8_getObjectImageY() {
	int i = getObjectIndex(pop());
	assert(i);
	push(_objs[i].y_pos);
}

void ScummEngine_v8::o8_getObjectImageWidth() {
	int i = getObjectIndex(pop());
	assert(i);
	push(_objs[i].width);
}

void ScummEngine_v8::o8_getObjectImageHeight() {
	int i = getObjectIndex(pop());
	assert(i);
	push(_objs[i].height);
}

void ScummEngine_v8::o8_getStringWidth() {
	int charset = pop();
	int oldID = _charset->getCurID();
	int width;
	const byte *msg = _scriptPointer;
	byte transBuf[512];

	// Skip to the next instruction
	_scriptPointer += resStrLen(_scriptPointer) + 1;

	convertMessageToString(msg, transBuf, 512);
	msg = transBuf;

	// Temporary set the specified charset id
	_charset->setCurID(charset);
	// Determine the strings width
	width = _charset->getStringWidth(0, msg);
	// Revert to old font
	_charset->setCurID(oldID);

	push(width);
}

void ScummEngine_v8::o8_drawObject() {
	int state = pop();
	int y = pop();
	int x = pop();
	int obj = pop();
	setObjectState(obj, state, x, y);
}

} // End of namespace Scumm
