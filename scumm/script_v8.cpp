/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm/actor.h"
#include "scumm/akos.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"
#include "scumm/smush/smush_player.h"
#include "sound/mixer.h"


#define OPCODE(x)	{ &Scumm_v8::x, #x }

void Scumm_v8::setupOpcodes() {
	static const OpcodeEntryV8 opcodes[256] = {
		/* 00 */
		OPCODE(o6_invalid),
		OPCODE(o6_pushWord),
		OPCODE(o6_pushWordVar),
		OPCODE(o6_wordArrayRead),
		/* 04 */
		OPCODE(o6_wordArrayIndexedRead),
		OPCODE(o6_dup),
		OPCODE(o6_pop),
		OPCODE(o6_not),
		/* 08 */
		OPCODE(o6_eq),
		OPCODE(o6_neq),
		OPCODE(o6_gt),
		OPCODE(o6_lt),
		/* 0C */
		OPCODE(o6_le),
		OPCODE(o6_ge),
		OPCODE(o6_add),
		OPCODE(o6_sub),
		/* 10 */
		OPCODE(o6_mul),
		OPCODE(o6_div),
		OPCODE(o6_land),
		OPCODE(o6_lor),
		/* 14 */
		OPCODE(o6_band),
		OPCODE(o6_bor),
		OPCODE(o8_mod),
		OPCODE(o6_invalid),
		/* 18 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 1C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 20 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 24 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 28 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 2C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 30 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 34 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 38 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 3C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 40 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 44 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 48 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 50 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 54 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 58 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 5C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 60 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 64 */
		OPCODE(o6_jumpTrue),
		OPCODE(o6_jumpFalse),
		OPCODE(o6_jump),
		OPCODE(o6_breakHere),
		/* 68 */
		OPCODE(o6_delayFrames),
		OPCODE(o8_wait),
		OPCODE(o6_delay),
		OPCODE(o6_delaySeconds),
		/* 6C */
		OPCODE(o6_delayMinutes),
		OPCODE(o6_writeWordVar),
		OPCODE(o6_wordVarInc),
		OPCODE(o6_wordVarDec),
		/* 70 */
		OPCODE(o8_dim),
		OPCODE(o6_wordArrayWrite),
		OPCODE(o6_wordArrayInc),
		OPCODE(o6_wordArrayDec),
		/* 74 */
		OPCODE(o8_dim2),
		OPCODE(o6_wordArrayIndexedWrite),
		OPCODE(o8_arrayOps),
		OPCODE(o6_invalid),
		/* 78 */
		OPCODE(o6_invalid),
		OPCODE(o6_startScriptEx),
		OPCODE(o6_startScript),
		OPCODE(o6_stopObjectCode),
		/* 7C */
		OPCODE(o6_stopScript),
		OPCODE(o6_jumpToScript),
		OPCODE(o6_dummy),				// O_RETURN boils down to a NOP
		OPCODE(o6_startObjectEx),
		/* 80 */
		OPCODE(o6_stopObjectScript),
		OPCODE(o6_cutscene),
		OPCODE(o6_endCutscene),
		OPCODE(o6_freezeUnfreeze),
		/* 84 */
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_stopSentence),
		OPCODE(o6_invalid),
		/* 88 */
		OPCODE(o6_invalid),
		OPCODE(o6_setClass),
		OPCODE(o6_setState),
		OPCODE(o6_setOwner),
		/* 8C */
		OPCODE(o6_panCameraTo),
		OPCODE(o6_actorFollowCamera),
		OPCODE(o6_setCameraAt),
		OPCODE(o6_printActor),
		/* 90 */
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		OPCODE(o6_talkEgo),
		OPCODE(o6_printLine),
		/* 94 */
		OPCODE(o6_printCursor),
		OPCODE(o6_printDebug),
		OPCODE(o6_printSystem),
		OPCODE(o8_blastText),
		/* 98 */
		OPCODE(o8_drawObject),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 9C */
		OPCODE(o8_cursorCommand),
		OPCODE(o6_loadRoom),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_walkActorToObj),
		/* A0 */
		OPCODE(o6_walkActorTo),
		OPCODE(o6_putActorInRoom),
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_faceActor),
		/* A4 */
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		OPCODE(o6_pickupObject),
		OPCODE(o6_setBoxFlags),
		/* A8 */
		OPCODE(o8_createBoxMatrix),
		OPCODE(o6_invalid),
		OPCODE(o8_resourceRoutines),
		OPCODE(o8_roomOps),
		/* AC */
		OPCODE(o8_actorOps),
		OPCODE(o8_cameraOps),
		OPCODE(o8_verbOps),
		OPCODE(o6_startSound),
		/* B0 */
		OPCODE(o6_startMusic),
		OPCODE(o6_stopSound),
		OPCODE(o6_soundKludge),
		OPCODE(o8_system),
		/* B4 */
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_setObjectName),
		OPCODE(o6_getDateTime),
		OPCODE(o6_drawBox),
		/* B8 */
		OPCODE(o6_invalid),
		OPCODE(o8_startVideo),
		OPCODE(o8_kernelSetFunctions),
		OPCODE(o6_invalid),
		/* BC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C8 */
		OPCODE(o6_startScriptQuick),	// FIXME: are these really the "quick" (=recursive) variants,
		OPCODE(o6_startObjectQuick),	// or aren't these maybe supposed to be the "plain" versions, too?
		OPCODE(o6_pickOneOf),
		OPCODE(o6_pickOneOfDefault),
		/* CC */
		OPCODE(o6_invalid),
		OPCODE(o6_isAnyOf),
		OPCODE(o6_getRandomNumber),
		OPCODE(o6_getRandomNumberRange),
		/* D0 */
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_getState),
		OPCODE(o6_getOwner),
		OPCODE(o6_isScriptRunning),
		/* D4 */
		OPCODE(o6_invalid),
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_abs),
		OPCODE(o6_invalid),
		/* D8 */
		OPCODE(o8_kernelGetFunctions),
		OPCODE(o6_isActorInBox),
		OPCODE(o6_getVerbEntrypoint),
		OPCODE(o6_getActorFromXY),
		/* DC */
		OPCODE(o6_findObject),
		OPCODE(o6_getVerbFromXY),
		OPCODE(o6_invalid),
		OPCODE(o6_findInventory),
		/* E0 */
		OPCODE(o6_getInventoryCount),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_getActorRoom),
		OPCODE(o6_getActorWalkBox),
		/* E4 */
		OPCODE(o6_getActorMoving),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorLayer),
		/* E8 */
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getActorWidth),
		OPCODE(o6_getObjectNewDir),
		OPCODE(o6_getObjectX),
		/* EC */
		OPCODE(o6_getObjectY),
		OPCODE(o8_getActorChore),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distPtPt),
		/* F0 */
		OPCODE(o8_getObjectImageX),
		OPCODE(o8_getObjectImageY),
		OPCODE(o8_getObjectImageWidth),
		OPCODE(o8_getObjectImageHeight),
		/* F4 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o8_getStringWidth),
		OPCODE(o8_getActorZPlane),
		/* F8 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* FC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};
	
	_opcodesV8 = opcodes;
}

void Scumm_v8::executeOpcode(byte i) {
	OpcodeProcV8 op = _opcodesV8[i].proc;
	(this->*op) ();
}

const char *Scumm_v8::getOpcodeDesc(byte i) {
	return _opcodesV8[i].desc;
}

// In V8, the word size is 4 byte, not 2 bytes as in V6/V7 games
uint Scumm_v8::fetchScriptWord() {
	int a;
	if (*_lastCodePtr + sizeof(MemBlkHeader) != _scriptOrgPointer) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	a = READ_LE_UINT32(_scriptPointer);
	_scriptPointer += 4;
	return a;
}

int Scumm_v8::fetchScriptWordSigned() {
	return (int32)fetchScriptWord();
}

int Scumm_v8::readVar(uint var) {
	debug(9, "readvar(%d)", var);

	if (!(var & 0xF0000000)) {
		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
		return _scummVars[var];
	}

	if (var & 0x80000000) {
		var &= 0x7FFFFFFF;
		checkRange(_numBitVariables - 1, 0, var, "Bit variable %d out of range(r)");
		return (_bitVars[var >> 3] & (1 << (var & 7))) ? 1 : 0;
	}

	if (var & 0x40000000) {
		var &= 0xFFFFFFF;
		checkRange(25, 0, var, "Local variable %d out of range(r)");
		return vm.localvar[_currentScript][var];
	}

	error("Illegal varbits (r)");
	return -1;
}

void Scumm_v8::writeVar(uint var, int value) {
	debug(9, "writeVar(%d, %d)", var, value);

	if (!(var & 0xF0000000)) {
		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(w)");

		_scummVars[var] = value;

		if ((_varwatch == (int)var) || (_varwatch == 0)) {
			if (vm.slot[_currentScript].number < 100)
				debug(1, "vars[%d] = %d (via script-%d)", var, value, vm.slot[_currentScript].number);
			else
				debug(1, "vars[%d] = %d (via room-%d-%d)", var, value, _currentRoom, vm.slot[_currentScript].number);
		}
		return;
	}

	if (var & 0x80000000) {
		var &= 0x7FFFFFFF;
		checkRange(_numBitVariables - 1, 0, var, "Bit variable %d out of range(w)");

		if (value)
			_bitVars[var >> 3] |= (1 << (var & 7));
		else
			_bitVars[var >> 3] &= ~(1 << (var & 7));
		return;
	}

	if (var & 0x40000000) {
		var &= 0xFFFFFFF;
		checkRange(25, 0, var, "Local variable %d out of range(w)");
		vm.localvar[_currentScript][var] = value;
		return;
	}

	error("Illegal varbits (w)");
}

void Scumm_v8::decodeParseString(int m, int n) {
	byte b;

	b = fetchScriptByte();

	switch (b) {
	case 0xC8:
		setStringVars(m);
		if (n)
			_actorToPrintStrFor = pop();
		break;
	case 0xC9:
		_string[m].t_xpos = _string[m].xpos;
		_string[m].t_ypos = _string[m].ypos;
		_string[m].t_center = _string[m].center;
		_string[m].t_overhead = _string[m].overhead;
		_string[m].t_no_talk_anim = _string[m].no_talk_anim;
		_string[m].t_right = _string[m].right;
		_string[m].t_color = _string[m].color;
		_string[m].t_charset = _string[m].charset;
		break;
	case 0xCA:
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case 0xCB:
		_string[m].color = pop();
		break;
	case 0xCC:
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case 0xCD:		// SO_PRINT_CHARSET Set print character set
		_string[m].charset = pop();
		break;
	case 0xCE:
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case 0xCF:
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case 0xD0:		// SO_PRINT_MUMBLE
		_string[m].no_talk_anim = true;
		break;
	case 0xD1:
		_messagePtr = translateTextAndPlaySpeech(_scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer)+ 1;

		switch (m) {
		case 0:
			actorTalk();
			break;
		case 1:
			drawString(1);
			break;
		case 2:
			unkMessage1();
			break;
		case 3:
			unkMessage2();
			break;
		case 5:{
			byte buffer[256];
			_msgPtrToAdd = buffer;
			addMessageToStack(_messagePtr);
			enqueueText(buffer, _string[m].xpos, _string[m].ypos, _string[m].color, _string[m].charset, _string[m].center);
			}
			break;
		}
		break;
	case 0xD2:		// SO_PRINT_WRAP Set print wordwrap
		//warning("decodeParseString: SO_PRINT_WRAP");
		break;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

void Scumm_v8::o8_mod() {
	int a = pop();
	push(pop() % a);
}

void Scumm_v8::o8_wait() {
	int actnum;
	int offs = -2;
	Actor *a;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0x1E:		// SO_WAIT_FOR_ACTOR Wait for actor (to finish current action?)
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o8_wait:SO_WAIT_FOR_ACTOR");
		if (a->isInCurrentRoom() && a->moving)
			break;
		return;
	case 0x1F:		// SO_WAIT_FOR_MESSAGE Wait for message
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case 0x20:		// SO_WAIT_FOR_CAMERA Wait for camera (to finish current action?)
		if (camera._dest != camera._cur)
			break;
		return;
	case 0x21:		// SO_WAIT_FOR_SENTENCE
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
			break;
		}
		if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;
		break;
	case 0x22:		// SO_WAIT_FOR_ANIMATION
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o8_wait:SO_WAIT_FOR_ANIMATION");
		if (a->isInCurrentRoom() && a->needRedraw)
			break;
		return;
	case 0x23:		// SO_WAIT_FOR_TURN
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o8_wait:SO_WAIT_FOR_TURN");
		if (a->isInCurrentRoom() && a->moving & MF_TURN)
			break;
		return;
	default:
		error("o8_wait: default case 0x%x", subOp);
	}

	_scriptPointer += offs;
	o6_breakHere();
}

void Scumm_v8::o8_dim() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	
	switch (subOp) {
	case 0x0A:		// SO_ARRAY_SCUMMVAR
		defineArray(array, 5, 0, pop());
		break;
	case 0x0B:		// SO_ARRAY_STRING
		defineArray(array, 4, 0, pop());
		break;
	case 0x0C:		// SO_ARRAY_UNDIM
		nukeArray(array);
		break;
	default:
		error("o8_dim: default case 0x%x", subOp);
	}
}

void Scumm_v8::o8_dim2() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord(), a, b;
	
	switch (subOp) {
	case 0x0A:		// SO_ARRAY_SCUMMVAR
		b = pop();
		a = pop();
		defineArray(array, 5, a, b);
		break;
	case 0x0B:		// SO_ARRAY_STRING
		b = pop();
		a = pop();
		defineArray(array, 4, a, b);
		break;
	case 0x0C:		// SO_ARRAY_UNDIM
		nukeArray(array);
		break;
	default:
		error("o8_dim2: default case 0x%x", subOp);
	}
}

void Scumm_v8::o8_arrayOps() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	int b, c, d, len;
	ArrayHeader *ah;
	int list[128];
	
	switch (subOp) {
	case 0x14:		// SO_ASSIGN_STRING
		b = pop();
		len = resStrLen(_scriptPointer);
		c = defineArray(array, 4, 0, len + 1);
		ah = (ArrayHeader *)getResourceAddress(rtString, c);
		copyScriptString(ah->data + b);
		break;
	case 0x15:		// SO_ASSIGN_SCUMMVAR_LIST
		b = pop();
		len = getStackList(list, ARRAYSIZE(list));
		d = readVar(array);
		if (d == 0) {
			defineArray(array, 5, 0, b + len);
		}
		while (--len >= 0) {
			writeArray(array, 0, b + len, list[len]);
		}
		break;
	case 0x16:		// SO_ASSIGN_2DIM_LIST
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

void Scumm_v8::o8_blastText() {
	// FIXME
	decodeParseString(5, 0);
}

void Scumm_v8::o8_cursorCommand() {
	byte subOp = fetchScriptByte();
	int a, i;
	int args[16];

	switch (subOp) {
	case 0xDC:		// SO_CURSOR_ON Turn cursor on
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 0xDD:		// SO_CURSOR_OFF Turn cursor off
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case 0xDE:		// SO_CURSOR_SOFT_ON Turn soft cursor on
		_cursor.state++;
			// FIXME is this check right? see bug #739229
		if (_cursor.state > 1)
			warning("Cursor state %d greater than 1 in script", _cursor.state);
		verbMouseOver(0);
		break;
	case 0xDF:		// SO_CURSOR_SOFT_OFF Turn soft cursor off
		_cursor.state--;
		verbMouseOver(0);
		break;
	case 0xE0:		// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 0xE1:		// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 0xE2:		// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 0xE3:		// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	case 0xE4:		// SO_CURSOR_IMAGE Set cursor image
		{
			int idx = pop();
			int room, obj;
			obj = popRoomAndObj(&room);
			setCursorImg(obj, room, idx);
		}
		break;
	case 0xE5:		// SO_CURSOR_HOTSPOT Set cursor hotspot
		a = pop();
		setCursorHotspot(pop(), a);
		break;
	case 0xE6:		// SO_CURSOR_TRANSPARENT Set cursor transparent color
		makeCursorColorTransparent(pop());
		break;
	case 0xE7: {		// SO_CHARSET_SET
		int charset = pop();
		warning("Set userface charset to %d", charset);
//		loadCharset(charset);
		break;
	}
	case 0xE8:		// SO_CHARSET_COLOR
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1].t_charset][i] = (unsigned char)args[i];
		break;
	case 0xE9: 		// SO_CURSOR_PUT
		{
		int y = pop();
		int x = pop();

		_system->warp_mouse(x, y);
		}
		break;
	default:
		error("o8_cursorCommand: default case 0x%x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void Scumm_v8::o8_createBoxMatrix() {
	int i;
	Actor *a;

	createBoxMatrix();

	for (i = 1; i < _numActors; i++) {
		a = &_actors[i];
		if (a && a->isInCurrentRoom())
			a->adjustActorPos();
	}
}

void Scumm_v8::o8_resourceRoutines() {
	byte subOp = fetchScriptByte();
	int resid = pop();

	switch (subOp) {
	case 0x3C:		// SO_HEAP_LOAD_CHARSET Load character set to heap
		ensureResourceLoaded(rtCharset, resid);	// FIXME - is this correct?
		break;
	case 0x3D:		// SO_HEAP_LOAD_COSTUME Load costume to heap
		ensureResourceLoaded(rtCostume, resid);
		break;
	case 0x3E:		// SO_HEAP_LOAD_OBJECT Load object to heap
		{
		int room = getObjectRoom(resid);
		loadFlObject(resid, room);
		}
		break;
	case 0x3F:		// SO_HEAP_LOAD_ROOM Load room to heap
		ensureResourceLoaded(rtRoom, resid);
		break;
	case 0x40:		// SO_HEAP_LOAD_SCRIPT Load script to heap
		ensureResourceLoaded(rtScript, resid);
		break;
	case 0x41:		// SO_HEAP_LOAD_SOUND Load sound to heap
		ensureResourceLoaded(rtSound, resid);
		break;

	case 0x42:		// SO_HEAP_LOCK_COSTUME Lock costume in heap
		lock(rtCostume, resid);
		break;
	case 0x43:		// SO_HEAP_LOCK_ROOM Lock room in heap
		lock(rtRoom, resid);
		break;
	case 0x44:		// SO_HEAP_LOCK_SCRIPT Lock script in heap
		lock(rtScript, resid);
		break;
	case 0x45:		// SO_HEAP_LOCK_SOUND Lock sound in heap
		lock(rtSound, resid);
		break;
	case 0x46:		// SO_HEAP_UNLOCK_COSTUME Unlock costume
		unlock(rtCostume, resid);
		break;
	case 0x47:		// SO_HEAP_UNLOCK_ROOM Unlock room
		unlock(rtRoom, resid);
		break;
	case 0x48:		// SO_HEAP_UNLOCK_SCRIPT Unlock script
		unlock(rtScript, resid);
		break;
	case 0x49:		// SO_HEAP_UNLOCK_SOUND Unlock sound
		unlock(rtSound, resid);
		break;
	case 0x4A:		// SO_HEAP_NUKE_COSTUME Remove costume from heap
		setResourceCounter(rtCostume, resid, 0x7F);
		break;
	case 0x4B:		// SO_HEAP_NUKE_ROOM Remove room from heap
		setResourceCounter(rtRoom, resid, 0x7F);
		break;
	case 0x4C:		// SO_HEAP_NUKE_SCRIPT Remove script from heap
		setResourceCounter(rtScript, resid, 0x7F);
		break;
	case 0x4D:		// SO_HEAP_NUKE_SOUND Remove sound from heap
		setResourceCounter(rtSound, resid, 0x7F);
		break;
	default:
		error("o8_resourceRoutines: default case 0x%x", subOp);
	}
}

void Scumm_v8::o8_roomOps() {
	byte subOp = fetchScriptByte();
	int a, b, c, d, e;
	
	switch (subOp) {
	case 0x52:		// SO_ROOM_PALETTE Set room palette
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;
	case 0x55:		// SO_ROOM_INTENSITY Set room intensity
		// Not used in CMI???
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;
	case 0x57:		// SO_ROOM_FADE Fade room
		a = pop();
		if (a) {
			_switchRoomEffect = (byte)(a);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;
	case 0x58:		// SO_ROOM_RGB_INTENSITY Set room color intensity
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;
	case 0x59:		// SO_ROOM_TRANSFORM Transform room
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		warning("o8_roomOps: unimplemented case SO_ROOM_TRANSFORM(%d,%d,%d,%d)", a, b, c, d);
		break;
	case 0x5A:		// SO_ROOM_CYCLE_SPEED Set palette cycling speed
	case 0x5B:		// SO_ROOM_COPY_PALETTE Copy palette
		error("o8_roomOps: unimplemented case %d", subOp);
		break;
	case 0x5C:		// SO_ROOM_NEW_PALETTE New palette
		a = pop();
		setPalette(a);
		break;
	case 0x5D:		// SO_ROOM_SAVE_GAME Save game
		_saveLoadCompatible = true;
		_saveLoadSlot = 1;
		_saveLoadFlag = 1;
		break;
	case 0x5E:		// SO_ROOM_LOAD_GAME Load game
		_saveLoadCompatible = true;
		_saveLoadSlot = 1;
		_saveLoadFlag = 2;
		break;
	case 0x5F:		// SO_ROOM_SATURATION Set saturation of room colors
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

void Scumm_v8::o8_actorOps() {
	byte subOp = fetchScriptByte();
	Actor *a;
	int i, j;

	if (subOp == 0x7A) {
		_curActor = pop();
		//printf("Setting current actor to %d\n", _curActor);
		return;
	}

	a = derefActorSafe(_curActor, "o8_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case 0x64:		// SO_ACTOR_COSTUME Set actor costume
		a->setActorCostume(pop());
		break;
	case 0x65:		// SO_ACTOR_STEP_DIST Set actor width of steps
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case 0x67:		// SO_ACTOR_ANIMATION_DEFAULT Set actor animation to default
		a->initFrame = 1;
		a->walkFrame = 2;
		a->standFrame = 3;
		a->talkStartFrame = 4;
		a->talkStopFrame = 5;
		break;
	case 0x68:		// SO_ACTOR_ANIMATION_INIT Initialize animation
		a->initFrame = pop();
		break;
	case 0x69:		// SO_ACTOR_ANIMATION_TALK Set actor animation to talk animation
		a->talkStopFrame = pop();
		a->talkStartFrame = pop();
		break;
	case 0x6A:		// SO_ACTOR_ANIMATION_WALK Set actor animation to walk animation
		a->walkFrame = pop();
		break;
	case 0x6B:		// SO_ACTOR_ANIMATION_STAND Set actor animation to standing animation
		a->standFrame = pop();
		break;
	case 0x6C:		// SO_ACTOR_ANIMATION_SPEED Set speed of animation
		a->setAnimSpeed(pop());
		break;
	case 0x6D:		// SO_ACTOR_DEFAULT
		a->initActor(0);
		break;
	case 0x6E:		// SO_ACTOR_ELEVATION
		i = pop();
		if (i != a->elevation) {
			a->elevation = i;
			a->needRedraw = true;
		}
		break;
	case 0x6F:		// SO_ACTOR_PALETTE Set actor palette
		j = pop();
		i = pop();
		checkRange(31, 0, i, "Illegal palette slot %d");
		a->palette[i] = j;
		a->needRedraw = true;
		break;
	case 0x70:		// SO_ACTOR_TALK_COLOR Set actor talk color
		a->talkColor = pop();
		break;
	case 0x71:		// SO_ACTOR_NAME Set name of actor
		loadPtrToResource(rtActorName, a->number, NULL);
		break;
	case 0x72:		// SO_ACTOR_WIDTH Set width of actor
		a->width = pop();
		break;
	case 0x73:		// SO_ACTOR_SCALE Set scaling of actor
		a->scalex = a->scaley = pop();
		a->needRedraw = true;
		break;
	case 0x74:		// SO_ACTOR_NEVER_ZCLIP ?
		a->forceClip = 0;
		break;
	case 0x75:		// SO_ACTOR_ALWAYS_ZCLIP ?
		a->forceClip = pop();
		// V8 uses 255 where we used to use 100
		if (a->forceClip == 255)
			a->forceClip = 100;
		break;
	case 0x76:		// SO_ACTOR_IGNORE_BOXES Make actor ignore boxes
		a->ignoreBoxes = true;
		a->forceClip = 100;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->room);
		break;
	case 0x77:		// SO_ACTOR_FOLLOW_BOXES Make actor follow boxes
		a->ignoreBoxes = false;
		a->forceClip = 100;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->room);
		break;
	case 0x78:		// SO_ACTOR_SPECIAL_DRAW
		a->shadow_mode = pop();
		break;
	case 0x79:		// SO_ACTOR_TEXT_OFFSET Set text offset relative to actor
		a->talkPosY = pop();
		a->talkPosX = pop();
		break;
//	case 0x7A:		// SO_ACTOR_INIT Set current actor (handled above)
	case 0x7B:		// SO_ACTOR_VARIABLE Set actor variable
		// FIXME - is this right??
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case 0x7C:		// SO_ACTOR_IGNORE_TURNS_ON Make actor ignore turns
		warning("ignoreTurns not yet implemented");
		a->ignoreTurns = true;
		break;
	case 0x7D:		// SO_ACTOR_IGNORE_TURNS_OFF Make actor follow turns
		a->ignoreTurns = false;
		break;
	case 0x7E:		// SO_ACTOR_NEW New actor
		a->initActor(2);
		break;
	case 0x7F:		// SO_ACTOR_DEPTH Set actor Z position
		a->layer = pop();
		break;
	case 0x80:		// SO_ACTOR_STOP
		a->stopActorMoving();
		break;
	case 0x81:		// SO_ACTOR_FACE Make actor face angle
		a->moving &= ~MF_TURN;
		a->setDirection(pop());
		break;
	case 0x82:		// SO_ACTOR_TURN Turn actor
		a->turnToDirection(pop());
		break;
	case 0x83:		// SO_ACTOR_WALK_SCRIPT Set walk script for actor?
		a->walkScript = pop();
		break;
	case 0x84:		// SO_ACTOR_TALK_SCRIPT Set talk script for actor?
		a->talkScript = pop();
		break;
	case 0x85:		// SO_ACTOR_WALK_PAUSE
		a->moving |= MF_FROZEN;
		break;
	case 0x86:		// SO_ACTOR_WALK_RESUME
		a->moving &= ~MF_FROZEN;
		break;
	case 0x87:		// SO_ACTOR_VOLUME Set volume of actor speech
		// TODO - implement this!
		i = pop();
		warning("o8_actorOps: setActorVolume(%d) not implemented", i);
		break;
	case 0x88:		// SO_ACTOR_FREQUENCY Set frequency of actor speech
		a->talkFrequency = pop();
		break;
	case 0x89:		// SO_ACTOR_PAN
		// FIXME: This should be stored in savegames.
		// 0 = left, 64 = middle, 127 = right.
		a->talkPan = pop();

		// If the actor is talking at the moment, adjust the panning
		// on the current talk channel handle. (If the handle is 0,
		// setChannelPan() won't do anything.)

		if (_actorToPrintStrFor == a->number)
			_mixer->setChannelPan(_sound->_talkChannelHandle,
				(a->talkPan != 64) ? 2 * a->talkPan - 127 : 0);

		break;
	default:
		error("o8_actorOps: default case 0x%x", subOp);
	}
}

void Scumm_v8::o8_cameraOps() {
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x32:		// SO_CAMERA_PAUSE
		//warning("freezeCamera NYI");
		break;
	case 0x33:		// SO_CAMERA_RESUME
		//warning("unfreezeCamera NYI");
		break;
	default:
		error("o8_cameraOps: default case 0x%x", subOp);
	}
}

void Scumm_v8::o8_verbOps() {
	byte subOp = fetchScriptByte();
	VerbSlot *vs = NULL;
	int slot, a, b;

	_verbRedraw = true;

	if (subOp == 0x96) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		checkRange(_maxVerbs - 1, 0, _curVerbSlot, "Illegal new verb slot %d");
		//printf("Setting current actor to %d\n", _curActor);
		return;
	}

	assert(0 <= _curVerbSlot && _curVerbSlot < _maxVerbs);
	vs = &_verbs[_curVerbSlot];
	assert(vs);

	switch (subOp) {
	case 0x96:		// SO_VERB_INIT Choose verb number for editing
		// handled above!
		break;
	case 0x97:		// SO_VERB_NEW New verb
		if (_curVerbSlot == 0) {
			for (slot = 1; slot < _maxVerbs; slot++) {
				if (_verbs[slot].verbid == 0)
					break;
			}
			if (slot >= _maxVerbs) {
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
		vs->charset_nr = _string[0].t_charset;
		vs->curmode = 0;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		break;
	case 0x98:		// SO_VERB_DELETE Delete verb
		killVerb(_curVerbSlot);
		break;
	case 0x99:		// SO_VERB_NAME Set verb name
		loadPtrToResource(rtVerb, _curVerbSlot, NULL);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 0x9A:		// SO_VERB_AT Set verb (X,Y) placement
		vs->y = pop();
		vs->x = pop();
		break;
	case 0x9B:		// SO_VERB_ON Turn verb on
		vs->curmode = 1;
		break;
	case 0x9C:		// SO_VERB_OFF Turn verb off
		vs->curmode = 0;
		break;
	case 0x9D:		// SO_VERB_COLOR Set verb color
		vs->color = pop();
		break;
	case 0x9E:		// SO_VERB_HICOLOR Set verb highlighted color
		vs->hicolor = pop();
		break;
	case 0xA0:		// SO_VERB_DIMCOLOR Set verb dimmed (disabled) color
		vs->dimcolor = pop();
		break;
	case 0xA1:		// SO_VERB_DIM
		vs->curmode = 2;
		break;
	case 0xA2:		// SO_VERB_KEY Set keypress to associate with verb
		vs->key = pop();
		break;
	case 0xA3:		// SO_VERB_IMAGE Set verb image
		b = pop();
		a = pop();
		if (_curVerbSlot && a != vs->imgindex) {
			setVerbObject(b, a, _curVerbSlot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case 0xA4:		// SO_VERB_NAME_STR Set verb name
		a = pop();
		if (a == 0) {
			loadPtrToResource(rtVerb, _curVerbSlot, (const byte *)"");
		} else {
			loadPtrToResource(rtVerb, _curVerbSlot, getStringAddress(a));
		}
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 0xA5:		// SO_VERB_CENTER Center verb
		vs->center = 1;
		break;
	case 0xA6:		// SO_VERB_CHARSET Choose charset for verb
		// FIXME - TODO
		vs->charset_nr = pop();
		break;
	case 0xA7:		// SO_VERB_LINE_SPACING Choose linespacing for verb
		// FIXME - TODO
		// Note: it seems that var596 stores the "line spacing". It is used by various
		// scripts that place verbs for that.
		// Also, var595 contains the vertical position at which to start placing verbs (330)
		pop();
		break;
	default:
		error("o8_verbops: default case 0x%x", subOp);
	}
}

void Scumm_v8::o8_system() {
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x28:		// SO_SYSTEM_RESTART Restart game
		restart();
		break;
	case 0x29:		// SO_SYSTEM_QUIT Quit game
		shutDown();
		break;
	default:
		error("o8_system: invalid case 0x%x", subOp);
	}
}


void Scumm_v8::o8_startVideo() {
	int len = resStrLen(_scriptPointer);

	debug(4, "o8_startVideo(%s/%s)", getGameDataPath(), (const char*)_scriptPointer);

	SmushPlayer *sp = new SmushPlayer(this, 83333, !_noSubtitles);
	sp->play((const char*)_scriptPointer, getGameDataPath());
	delete sp;

	_scriptPointer += len + 1;
}

void Scumm_v8::o8_kernelSetFunctions() {
	// TODO
	Actor *a;
	int args[30];
	int len = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 11: {	// lockObject
		int objidx = getObjectIndex(args[1]);
		if (objidx == -1) {
			warning("Cannot find object %d to lock", args[1]);
			break;
		}

		lock(rtFlObject, objidx);

//		if (ObjData.field28 != 0) {
//			ObjData.field32 = 1;
//		}
		break;
	}
	case 12: {	// unlockObject
		int objidx = getObjectIndex(args[1]);
		if (objidx == -1) {
			warning("Cannot find object %d to unlock", args[1]);
			break;
		}

		unlock(rtFlObject, objidx);

//		if (ObjData.field28 != 0) {
//			ObjData.field32 = 0;
//		}
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
//		warning("o8_kernelSetFunctions: setBannerColors(%d, %d, %d, %d)", args[1], args[2], args[3], args[4]);
		break;
	case 23:	// setActorChoreLimbFrame
		// FIXME: This is critical, and is the cause of the Cannon "too many scripts" crash
		// This opcode is used a lot in script 28.
		// The problem here is that args[4] is always 0, as it is computed from
		// lipSyncWidth and lipSyncHeight, which we currently don't support. As a result,
		// actors will currently not move their mouth at all!
//		warning("o8_kernelSetFunctions: setActorChoreLimbFrame(%d, %d, %d, %d)", args[1], args[2], args[3], args[4]);
		a = derefActor(args[1], "o8_kernelSetFunctions:setActorChoreLimbFrame");

		a->startAnimActor(args[2]);
		a->animateLimb(args[3], args[4]);
		
		break;
	case 24:	// clearTextQueue
		// TODO - clearTextQueue. Maybe this should just call removeBlastTexts() ?
		warning("o8_kernelSetFunctions: clearTextQueue()");
		break;
	case 25: {	// saveGameReadName
		SaveFileManager *mgr = _system->get_savefile_manager();
		char *address = (char*)getStringAddress(args[2]);
		char name[30];

		if (!address) {
			warning("o8_kernelSetFunctions: saveGameReadName failed finding slot string %d", args[2]);
			break;
		}
		getSavegameName(args[1] - 1, name, mgr);
		if (strlen(name) > 0 && strlen(name) < 30)
			strcpy(address, name);
		break;
	}
	case 26: {	// saveGame?
		//SaveFileManager *mgr = _system->get_savefile_manager();
		//char *address = (char*)getStringAddress(args[2]);
		char address[30];
		warning("o8_kernelSetFunctions: saveGame?(%d, %s)", args[1], address);
		break;
	}
	case 27: {	// FIXME: This doesn't work
			// saveGameRead
		_saveLoadSlot = args[1];
		_saveLoadFlag = 2;
		_saveLoadCompatible = false;
		warning("Sgl: %d", args[1]);
		break;
	}
	case 28:	// saveGameStampScreenshot
		warning("o8_kernelSetFunctions: saveGameStampScreenshot(%d, %d, %d, %d, %d, %d)", args[1], args[2], args[3], args[4], args[5], args[6]);
		break;
	case 29:	// setKeyScript
		_keyScriptKey = args[1];
		_keyScriptNo = args[2];
		break;
	case 30:	// killAllScriptsButMe
		warning("o8_kernelSetFunctions: killAllScriptsButMe()");
		killAllScriptsExceptCurrent();
		break;
	case 31:	// stopAllVideo
		warning("o8_kernelSetFunctions: stopAllVideo()");
		break;
	case 32:	// writeRegistryValue
		warning("o8_kernelSetFunctions: writeRegistryValue(%d, %d)", args[1], args[2]);
		break;
	case 33:	// paletteSetIntensity
		warning("o8_kernelSetFunctions: paletteSetIntensity(%d, %d)", args[1], args[2]);
		break;
	case 34:	// queryQuit
		warning("o8_kernelSetFunctions: queryQuit()");
		break;
	case 108:	// buildPaletteShadow
		setupShadowPalette(args[1], args[2], args[3], args[4], args[5], args[6]);
		break;
	case 109:	// setPaletteShadow
		setupShadowPalette(0, args[1], args[2], args[3], args[4], args[5]);
		break;
	case 118:	// blastShadowObject
		enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 3);
		break;
	case 119:	// superBlastObject
		enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 0);
		break;

	default:
		warning("o8_kernelSetFunctions: default case 0x%x (len = %d)", args[0], len);
	}
}

void Scumm_v8::o8_kernelGetFunctions() {
	// TODO
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
		switch(args[1]) {
		// Stub out a few specific cases, just to make things less noisy

		// To actually implement this, we may need a new OSystem call to do
		// asyncronous keyboard state checking...
		case 0x14B:	//	Left Arrow depressed?
		case 0x14D:	//	Right Arrow depressed?
		case 0x09:	// 	Tab depressed (fire in ship combat)
			push(0);
			break;
		default:
			warning("getKeyState(0x%X)", args[1]);
			push(0);
			break;
		}
		break;
	case 0xCE:		// getRGBSlot
	case 0xD7:		// getBox
		push(0);
		warning("o8_kernelGetFunctions: default case 0x%x (len = %d)", args[0], len);
		break;
	case 0xD8: {		// findBlastObject
		int x = args[1] + (camera._cur.x & 7);
		int y = args[2] + _screenTop;
		BlastObject *eo;

		for (int i = _blastObjectQueuePos - 1; i >= 0; i--) {
			eo = &_blastObjectQueue[i];

			if (eo->posX <= x && eo->width + eo->posX > x &&
			    eo->posY <= y && eo->height + eo->posY > y) {
                		if (!getClass(eo->number, kObjectClassUntouchable)) {
					push(eo->number);
					return;
				}
			}
		}
		push(0);
		break;
	}
	case 0xD9: {   // actorHit - used, for example, to detect ship collision
	               // during ship-to-ship combat.
		Actor *a = derefActor(args[1], "actorHit");
		AkosRenderer *ar = (AkosRenderer *) _costumeRenderer;
		bool old_need_redraw = a->needRedraw;

		ar->_actorHitX = args[2];
		ar->_actorHitY = args[3] + _screenTop;
		ar->_actorHitMode = true;
		ar->_actorHitResult = false;

		a->needRedraw = true;
		a->drawActorCostume();
		a->needRedraw = old_need_redraw;

		ar->_actorHitMode = false;
		push(ar->_actorHitResult);
		break;
	}
	case 0xDA:		// lipSyncWidth
	case 0xDB:		// lipSyncHeight
		// TODO - get lip sync data for the currently active voice
		// HACK - return random values for now, to make things look half decent
		push(_rnd.getRandomNumber(255));
		break;
	case 0xDC:		// actorTalkAnimation
		{
		Actor *a = derefActor(args[1], "actorTalkAnimation");
		push(a->talkStartFrame);
		}
		break;
	case 0xDD:		// getMasterSFXVol
		push(_sound->_sound_volume_sfx / 2);
		break;
	case 0xDE:		// getMasterVoiceVol
		push(_sound->_sound_volume_sfx / 2);
		break;
	case 0xDF:		// getMasterMusicVol
		push(_sound->_sound_volume_music / 2);
		break;
	case 0xE0:		// readRegistryValue
		{
		int array = args[1];
		// FIXME - hack: for some reasons the wrong variable ID arrives here, compared to the
		// scripts. Probably a wrong push/pop somewhere. For now override to correct value.
		array = 658;
		ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));
		if (!strcmp((char *)ah->data, "Saveload Page") || !strcmp((char *)ah->data, "Object Names"))
			push(1);
		else
			push(0);
		}
		break;
	case 0xE1:		// imGetMusicPosition
		warning("o8_kernelGetFunctions: imGetMusicPosition(stub)");
		// FIXME - get this stuff to be properly implemented
		push(_sound->_bundleMusicPosition);
		break;
	case 0xE2:		// musicLipSyncWidth
	case 0xE3:		// musicLipSyncHeight
		// TODO - get lip sync data for the currently active music
		//warning("o8_kernelGetFunctions: musicLipSync(case 0x%x, len = %d)", args[0], len);
		push(_rnd.getRandomNumber(255));
		break;
	default:
		error("o8_kernelGetFunctions: default case 0x%x (len = %d)", args[0], len);
	}

}

void Scumm_v8::o8_getActorChore() {
	int actnum = pop();
	Actor *a = derefActor(actnum, "o8_getActorChore");

	// FIXME: This is a hack for the cannon scene, as something isn't quite right
	// here yet..
	if ((_roomResource == 10) && (vm.slot[_currentScript].number == 2021)) {
		push(11);
		return;
	}

	push(a->frame);
}

void Scumm_v8::o8_getActorZPlane() {
	int actnum = pop();
	Actor *a = derefActor(actnum, "o8_getActorZPlane");

	int z = a->forceClip;
	if (z == 100) {
		z = getMaskFromBox(a->walkbox);
		if (z > gdi._numZBuffer)
			z = gdi._numZBuffer;
	}

	push(z);
}


void Scumm_v8::o8_getObjectImageX() {
	int i = getObjectIndex(pop());
	push(_objs[i].x_pos);
}

void Scumm_v8::o8_getObjectImageY() {
	int i = getObjectIndex(pop());
	push(_objs[i].y_pos);
}

void Scumm_v8::o8_getObjectImageWidth() {
	int i = getObjectIndex(pop());
	push(_objs[i].width);
}

void Scumm_v8::o8_getObjectImageHeight() {
	int i = getObjectIndex(pop());
	push(_objs[i].height);
}

void Scumm_v8::o8_getStringWidth() {
	int charset = pop();
	int oldID = _charset->getCurID(); 
	int width;
	const byte *msg = _scriptPointer;
	
	// Skip to the next instruction
	_scriptPointer += resStrLen(_scriptPointer) + 1;

	if (msg[0] == '/') {
		translateText(msg, _transText);
		msg = _transText;
	} 


	// Temporary set the specified charset id
	_charset->setCurID(_string[charset].charset);
	// Determine the strings width
	width = _charset->getStringWidth(0, msg);
	// Revert to old font
	_charset->setCurID(oldID);

	push(width);
}

void Scumm_v8::o8_drawObject() {
	int state = pop(), y = pop(), x = pop(), obj = pop(), objnum = getObjectIndex(obj);
	ObjectData *od;

	if (!objnum)
		return;

	od = &_objs[objnum];
	if (x != 0x7FFFFFFF) {
		od->x_pos = x;
		od->y_pos = y;
		debug(1, "setting position: 0x%X b 0x%X", x, y);
	}

	addObjectToDrawQue(objnum);

	if (state == 255 || state == 254)
		warning("o8_drawObject(%d, %d, %d, %d): extended attributes unimplemented", x, y, objnum, state);
	else
		warning("o8_drawObject(%d, %d, %d, %d)", x, y, objnum, state);

	putState(obj, state);
}
