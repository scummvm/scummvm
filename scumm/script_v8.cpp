/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
#include "scumm.h"
#include "actor.h"
#include "charset.h"
#include "intern.h"
#include "sound.h"
#include "verbs.h"

#include "smush/player.h"
#include "smush/scumm_renderer.h"

#include <time.h>


#define OPCODE(x)	{ &Scumm_v8::x, #x }

void Scumm_v8::setupOpcodes()
{
	// TODO: any of the o6_ entries are potentially wrong and pure guesses :-)
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
		OPCODE(o6_stopObjectScript),	// FIXME - is this right?
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
		OPCODE(o8_printLine),
		/* 94 */
		OPCODE(o8_printCursor),
		OPCODE(o8_printDebug),
		OPCODE(o8_printSystem),
		OPCODE(o8_blastText),
		/* 98 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 9C */
		OPCODE(o8_cursorCommand),
		OPCODE(o6_loadRoom),
		OPCODE(o6_loadRoomWithEgo),	// FIXME - this is a pure guess
		OPCODE(o6_walkActorToObj),
		/* A0 */
		OPCODE(o6_walkActorTo),
		OPCODE(o6_putActorInRoom),	// FIXME - this seems to be wrong? at least we get actor indices from 31 to 79 here...
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_faceActor),
		/* A4 */
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		OPCODE(o6_pickupObject),
		OPCODE(o6_setBoxFlags),
		/* A8 */
		OPCODE(o6_createBoxMatrix), // fixme?
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
		OPCODE(o8_soundKludge),
		OPCODE(o8_system),
		/* B4 */
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_setObjectName),
		OPCODE(o8_getDateTime),
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
		OPCODE(o6_startScriptQuick),	// FIXME - this function returns something in V8 !
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_pickOneOf),
		OPCODE(o6_pickOneOfDefault),
		/* CC */
		OPCODE(o6_invalid),
		OPCODE(o6_isAnyOf),
		OPCODE(o6_getRandomNumber),
		OPCODE(o6_getRandomNumberRange),
		/* D0 */
		OPCODE(o6_ifClassOfIs),	// FIXME - this is a guess
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
		OPCODE(o6_getObjectNewDir),		// FIXME: is this right?
		OPCODE(o6_getObjectX),
		/* EC */
		OPCODE(o6_getObjectY),
		OPCODE(o6_getActorAnimCounter1),
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
		OPCODE(o6_invalid),
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

void Scumm_v8::executeOpcode(int i)
{
	OpcodeProcV8 op = _opcodesV8[i].proc;
	(this->*op) ();
}

const char *Scumm_v8::getOpcodeDesc(int i)
{
	return _opcodesV8[i].desc;
}

// In V8, the word size is 4 byte, not 2 bytes as in V6/V7 games
uint Scumm_v8::fetchScriptWord()
{
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

int Scumm_v8::fetchScriptWordSigned()
{
	return (int32)fetchScriptWord();
}

int Scumm_v8::readVar(uint var)
{
	debug(9, "readvar(%d)", var);

	if (!(var & 0xF0000000)) {
		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
		return _vars[var];
	}

	if (var & 0x80000000) {
		var &= 0x7FFFFFFF;
		checkRange(_numBitVariables - 1, 0, var, "Bit variable %d out of range(r)");
		return (_bitVars[var >> 3] & (1 << (var & 7))) ? 1 : 0;
	}

	if (var & 0x40000000) {
		var &= 0xFFFFFFF;
		checkRange(0x10, 0, var, "Local variable %d out of range(r)");
		return vm.localvar[_currentScript][var];
	}

	error("Illegal varbits (r)");
	return -1;
}

void Scumm_v8::writeVar(uint var, int value)
{
	debug(9, "writeVar(%d, %d)", var, value);

	if (!(var & 0xF0000000)) {
		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(w)");

		_vars[var] = value;

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
		checkRange(0x10, 0, var, "Local variable %d out of range(w)");
		vm.localvar[_currentScript][var] = value;
		return;
	}

	error("Illegal varbits (w)");
}

void Scumm_v8::decodeParseString(int m, int n)
{
	byte b;

	b = fetchScriptByte();

	switch (b) {
	case 0xC8:
		setStringVars(m);
		if (n)
			_actorToPrintStrFor = pop();
		return;
	case 0xC9:
		_string[m].t_xpos = _string[m].xpos;
		_string[m].t_ypos = _string[m].ypos;
		_string[m].t_center = _string[m].center;
		_string[m].t_overhead = _string[m].overhead;
		_string[m].t_no_talk_anim = _string[m].no_talk_anim;
		_string[m].t_right = _string[m].right;
		_string[m].t_color = _string[m].color;
		_string[m].t_charset = _string[m].charset;
		return;
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
	case 0xCD: {		// SO_PRINT_CHARSET Set print character set
		// FIXME - TODO
		int charset = pop();
		_string[m].charset = charset;
	}
		break;
	case 0xCE:
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case 0xCF:
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case 0xD2:
	case 0xD0:		// SO_PRINT_MUMBLE
		_string[m].no_talk_anim = true;
		break;
	case 0xD1:
		_messagePtr = _scriptPointer;

		if (_messagePtr[0] == '/') {
			char pointer[20];
			int i, j;

			_scriptPointer += resStrLen(_scriptPointer) + 1;
			translateText(_messagePtr, _transText);
			for (i = 0, j = 0; (_messagePtr[i] != '/' || j == 0) && j < 19; i++) {
				if (_messagePtr[i] != '/')
					pointer[j++] = _messagePtr[i];
			}
			pointer[j] = 0;

			// Stop any talking that's still going on
			if (_sound->_talkChannel > -1)
				_mixer->stop(_sound->_talkChannel);

//			_sound->_talkChannel = _sound->playBundleSound(pointer);
			_messagePtr = _transText;

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
			}
			return;
		} else {
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
			}
			_scriptPointer = _messagePtr;
			return;
		}
		break;
//	case 0xD2:		// SO_PRINT_WRAP Set print wordwrap
//		error("decodeParseString: SO_PRINT_MUMBLE");
//		break;
	default:
		error("decodeParseString: default case %d", b);
	}
}

void Scumm_v8::o8_mod()
{
	int a = pop();
	push(pop() % a);
}

void Scumm_v8::o8_wait()
{
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x1E: {		// SO_WAIT_FOR_ACTOR Wait for actor (to finish current action?)
		int offs = fetchScriptWordSigned();
		if (derefActorSafe(pop(), "o8_wait:SO_WAIT_FOR_ACTOR")->moving) {
			_scriptPointer += offs;
			o6_breakHere();
		}
		return;
	}
	case 0x1F:		// SO_WAIT_FOR_MESSAGE Wait for message
		if (_vars[VAR_HAVE_MSG])
			break;
		return;
	case 0x20:		// SO_WAIT_FOR_CAMERA Wait for camera (to finish current action?)
		if (camera._dest != camera._cur)
			break;
		return;
	case 0x21:		// SO_WAIT_FOR_SENTENCE
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
				return;
			break;
		}
		if (!isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
			return;
		break;
	case 0x22: {		// SO_WAIT_FOR_ANIMATION
		int actnum = pop();
		Actor *a = derefActorSafe(actnum, "o8_wait:SO_WAIT_FOR_ANIMATION");
		int offs = fetchScriptWordSigned();
		if (a && a->isInCurrentRoom() && a->needRedraw) {
			_scriptPointer += offs;
			o6_breakHere();
		}
		return;
	}
	case 0x23: {		// SO_WAIT_FOR_TURN
		int actnum = pop();
		Actor *a = derefActorSafe(actnum, "o8_wait:SO_WAIT_FOR_TURN");
		int offs = fetchScriptWordSigned();
		if (a && a->isInCurrentRoom() && a->moving & MF_TURN) {
			_scriptPointer += offs;
			o6_breakHere();
		}
		return;
	}

	default:
		error("o8_wait: default case %d", subOp);
	}

	_scriptPointer -= 2;
	o6_breakHere();
}

void Scumm_v8::o8_dim()
{
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
		error("o8_dim: default case %d", subOp);
	}
}

void Scumm_v8::o8_dim2()
{
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
		error("o8_dim2: default case %d", subOp);
	}
}

void Scumm_v8::o8_arrayOps()
{
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
		c = pop();
		d = readVar(array);
		if (d == 0) {
			defineArray(array, 5, 0, b + c);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 0x16:		// SO_ASSIGN_2DIM_LIST
		b = pop();
		len = getStackList(list, sizeof(list) / sizeof(list[0]));
		d = readVar(array);
		if (d == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, b + len, list[len]);
		}
		break;
	default:
		error("o8_arrayOps: default case %d (array %d)", subOp, array);
	}
}

void Scumm_v8::o8_printLine()
{
	// FIXME
	decodeParseString(0, 0);
}

void Scumm_v8::o8_printCursor()
{
	// FIXME
	decodeParseString(1, 0);
}

void Scumm_v8::o8_printDebug()
{
	// FIXME
	decodeParseString(2, 0);
}

void Scumm_v8::o8_printSystem()
{
	// FIXME
	decodeParseString(3, 0);
}

void Scumm_v8::o8_blastText()
{
	// FIXME
	decodeParseString(1, 0);
}

void Scumm_v8::o8_cursorCommand()
{
	// TODO
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
		if (_cursor.state > 1)
			error("Cursor state greater than 1 in script");
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
		setCursorHotspot2(pop(), a);
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
		getStackList(args, sizeof(args) / sizeof(args[0]));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1].t_charset][i] = (unsigned char)args[i];
		break;
	case 0xE9: 		// SO_CURSOR_PUT
	        _virtual_mouse_x = pop();
		_virtual_mouse_y = pop();

		mouse.x = _virtual_mouse_x - virtscr[0].xstart;
		mouse.y = _virtual_mouse_y - camera._cur.y + (_realHeight / 2);
		mouse.y += 16;

		_system->set_mouse_pos(mouse.x, mouse.y);
		_system->update_screen();
		warning("warped mouse to (%d, %d) from %d-%d", _virtual_mouse_x, _virtual_mouse_y, _roomResource, vm.slot[_currentScript].number);
		break;
	default:
		error("o8_cursorCommand: default case %d", subOp);
	}

	_vars[VAR_CURSORSTATE] = _cursor.state;
	_vars[VAR_USERPUT] = _userPut;
}

void Scumm_v8::o8_resourceRoutines()
{
	// TODO
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
		error("o8_resourceRoutines: default case %d", subOp);
	}
}

void Scumm_v8::o8_roomOps()
{
	// TODO
	byte subOp = fetchScriptByte();
	int a, b, c, d, e;
	
	switch (subOp) {
	case 0x52:		// SO_ROOM_PALETTE Set room palette
		error("o8_roomOps: default case %d", subOp);
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
	case 0x5A:		// SO_ROOM_CYCLE_SPEED Set palette cycling speed
	case 0x5B:		// SO_ROOM_COPY_PALETTE Copy palette
	case 0x5C:		// SO_ROOM_NEW_PALETTE Create new palette
		error("o8_roomOps: unimplemented case %d", subOp);
		break;
	case 0x5D:		// SO_ROOM_SAVE_GAME Save game
		warning("V8 Save game opcode not implemented");
		break;
	case 0x5E:		// SO_ROOM_LOAD_GAME Load game
		warning("V8 Load game opcode not implemented");
		break;
	case 0x5F:		// SO_ROOM_SATURATION Set saturation of room colors
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		// FIXME - this probably has the same format as for darkenPalette:
		// thre values for R, G, B and a start/end palette range to modify.
		// Now, how on earth does on modify the saturation of a single color channel?
		// Change the hue/saturation of a color, no problem, I know how to do that,
		// but for only a channel alone, I don't even know what that should mean... :-/
//		warning("o8_roomOps: SO_ROOM_SATURATION(%d, %d, %d, %d, %d)", a, b, c, d, e);
		break;
	default:
		error("o8_roomOps: default case %d", subOp);
	}
}

void Scumm_v8::o8_actorOps()
{
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
		a->talkFrame1 = 4;
		a->talkFrame2 = 5;
		break;
	case 0x68:		// SO_ACTOR_ANIMATION_INIT Initialize animation
		a->initFrame = pop();
		break;
	case 0x69:		// SO_ACTOR_ANIMATION_TALK Set actor animation to talk animation
		a->talkFrame2 = pop();
		a->talkFrame1 = pop();
		break;
	case 0x6A:		// SO_ACTOR_ANIMATION_WALK Set actor animation to walk animation
		a->walkFrame = pop();
		break;
	case 0x6B:		// SO_ACTOR_ANIMATION_STAND Set actor animation to standing animation
		a->standFrame = pop();
		break;
	case 0x6C:		// SO_ACTOR_ANIMATION_SPEED Set speed of animation
		a->animSpeed = pop();
		a->animProgress = 0;
		break;
	case 0x6D:		// SO_ACTOR_DEFAULT
		// FIXME - is this right? Or maybe a->initActor(2) ?
		warning("o8_actorOps: SO_ACTOR_DEFAULT");
		a->initActor(0);
		break;
	case 0x6E:		// SO_ACTOR_ELEVATION
		a->elevation = pop();
		a->needRedraw = true;
		a->needBgReset = true;
		break;
	case 0x6F:		// SO_ACTOR_PALETTE Set actor palette
		j = pop();
		i = pop();
		checkRange(31, 0, i, "Illegal palet slot %d");
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
		a->needBgReset = true;
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
			a->putActor(a->x, a->y, a->room);
		break;
	case 0x77:		// SO_ACTOR_FOLLOW_BOXES Make actor follow boxes
		a->ignoreBoxes = false;
		a->forceClip = 100;
		if (a->isInCurrentRoom())
			a->putActor(a->x, a->y, a->room);
		break;
	case 0x78:		// SO_ACTOR_SPECIAL_DRAW
		a->shadow_mode = pop();
		break;
	case 0x79:		// SO_ACTOR_TEXT_OFFSET Set text offset relative to actor
		a->talkPosX = pop();
		a->talkPosY = pop();
		break;
//	case 0x7A:		// SO_ACTOR_INIT Set current actor (handled above)
	case 0x7B:		// SO_ACTOR_VARIABLE Set actor variable
		// FIXME - is this right??
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case 0x7C:		// SO_ACTOR_IGNORE_TURNS_ON Make actor ignore turns
		a->ignoreTurns = true;
		break;
	case 0x7D:		// SO_ACTOR_IGNORE_TURNS_OFF Make actor follow turns
		a->ignoreTurns = false;
		break;
	case 0x7E:		// SO_ACTOR_NEW New actor
		// FIXME - is this right? Or maybe a->initActor(0) ?
		warning("o8_actorOps: SO_ACTOR_NEW");
		a->initActor(2);
		break;
	case 0x7F:		// SO_ACTOR_DEPTH Set actor Z position
		a->layer = pop();
		break;
	case 0x80:		// SO_ACTOR_STOP
		a->stopActorMoving();
		a->startAnimActor(a->standFrame);
		break;
	case 0x81:		// SO_ACTOR_FACE Make actor face angle
		a->moving &= ~MF_TURN;
		a->setDirection(pop());
		break;
	case 0x82:		// SO_ACTOR_TURN Turn actor
		a->turnToDirection(pop());
		break;
	case 0x83:		// SO_ACTOR_WALK_SCRIPT Set walk script for actor?
		a->walk_script = pop();
		break;
	case 0x84:		// SO_ACTOR_TALK_SCRIPT Set talk script for actor?
		a->talk_script = pop();
		break;
	case 0x85:		// SO_ACTOR_WALK_PAUSE
		a->moving |= 0x80;
		break;
	case 0x86:		// SO_ACTOR_WALK_RESUME
		a->moving &= ~0x7f;
		break;
	case 0x87:		// SO_ACTOR_VOLUME Set volume of actor speech
		// TODO - implement this!
		i = pop();
		warning("o8_actorOps: setActorVolume(%d) not implemented", i);
		break;
	case 0x88:		// SO_ACTOR_FREQUENCY Set frequency of actor speech
		// TODO - implement this!
		i = pop();
		warning("o8_actorOps: setActorFrequency(%d) not implemented", i);
		break;
	case 0x89:		// SO_ACTOR_PAN
		// TODO - implement this!
		i = pop();
		warning("o8_actorOps: setActorPan(%d) not implemented", i);
		break;
	default:
		error("o8_actorOps: default case %d", subOp);
	}
}

void Scumm_v8::o8_cameraOps()
{
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
		error("o8_cameraOps: default case %d", subOp);
	}
}

void Scumm_v8::o8_verbOps()
{
	byte subOp = fetchScriptByte();
	VerbSlot *vs = NULL;
	int slot, a, b;

	_verbRedraw = true;

	if (0 <= _curVerbSlot && _curVerbSlot < _maxVerbs)
		vs = &_verbs[_curVerbSlot];
	if (subOp != 0x96)
		assert(vs);

	switch (subOp) {
	case 0x96:		// SO_VERB_INIT Choose verb number for editing
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		checkRange(_maxVerbs - 1, 0, _curVerbSlot, "Illegal new verb slot %d");
		break;
	case 0x97:		// SO_VERB_NEW New verb
		if (_curVerbSlot == 0) {
			for (slot = 1; slot < _maxVerbs; slot++) {
				if (_verbs[slot].verbid == 0)
					break;
			}
			if (slot == _maxVerbs)
				error("Too many verbs");
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
			loadPtrToResource(rtVerb, _curVerbSlot, (byte *)"");
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
		//printf("Set to charset %d\n", vs->charset_nr);
		break;
	case 0xA7:		// SO_VERB_LINE_SPACING Choose linespacing for verb
		// FIXME - TODO
		// Note: it seems that var596 stores the "line spacing". It is used by various
		// scripts that place verbs for that.
		// Also, var595 contains the vertical position at which to start placing verbs (330)
		pop();
		break;
	default:
		error("o8_verbops: default case %d", subOp);
	}
}

void Scumm_v8::o8_soundKludge()
{
	int args[16];
	int num = getStackList(args, sizeof(args) / sizeof(args[0]));

	_sound->soundKludge(args, num);
}

void Scumm_v8::o8_system()
{
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x28:		// SO_SYSTEM_RESTART Restart game
//		pauseGame(false);
//		break;
	case 0x29:		// SO_SYSTEM_QUIT Quit game
//		shutDown(0);
//		break;
	default:
		error("o8_system: default case %d", subOp);
	}
}

void Scumm_v8::o8_getDateTime()
{
	struct tm *t;
	time_t now = time(NULL);
	
	t = localtime(&now);

	_vars[VAR_TIMEDATE_YEAR] = t->tm_year;
	_vars[VAR_TIMEDATE_MONTH] = t->tm_mon;
	_vars[VAR_TIMEDATE_DAY] = t->tm_mday;
	_vars[VAR_TIMEDATE_HOUR] = t->tm_hour;
	_vars[VAR_TIMEDATE_MINUTE] = t->tm_min;
	_vars[VAR_TIMEDATE_SECOND] = t->tm_sec;
}

void Scumm_v8::o8_startVideo()
{
	int len = resStrLen(_scriptPointer);
	
	warning("o8_startVideo(%s/%s)", getGameDataPath(), (char*)_scriptPointer);
	
	//ScummRenderer * sr = new ScummRenderer(this, 1000/14);
	//SmushPlayer * sp = new SmushPlayer(sr);
	//sp->play((char*)_scriptPointer, getGameDataPath());
	
	_scriptPointer += len + 1;
}

void Scumm_v8::o8_kernelSetFunctions()
{
	// TODO
	int args[30];
	int len = getStackList(args, sizeof(args) / sizeof(args[0]));

	switch (args[0]) {
	case 11:	// lockObject
//		warning("o8_kernelSetFunctions: lockObject(%d)", args[1]);
		lock(rtFlObject, args[1]);	// FIXME - no idea if this is right?
//		getObjectIndex(args[1]);
//		if (ObjData.field28 != 0) {
//			ObjData.field32 = 1;
//		}
		break;
	case 12:	// unlockObject
		warning("o8_kernelSetFunctions: unlockObject(%d)", args[1]);
		unlock(rtFlObject, args[1]);	// FIXME - no idea if this is right?
//		getObjectIndex(args[1]);
//		if (ObjData.field28 != 0) {
//			ObjData.field32 = 0;
//		}
		break;
	case 13:	// remapCostume
		derefActorSafe(args[1], "o8_kernelSetFunctions:remapCostume")->remapActorPalette(args[2], args[3], args[4], -1);
		break;
	case 14:	// remapCostumeInsert
		derefActorSafe(args[1], "o8_kernelSetFunctions:remapCostumeInsert")->remapActorPalette(args[2], args[3], args[4], args[5]);
		break;
	case 15:	// setVideoFrameRate
		// not used anymore (was smush frame rate)
		break;
	case 20:	// setBoxScale
		setBoxScale(args[1], args[2]);
		break;
	case 21:	// setScaleSlot
		warning("o8_kernelSetFunctions: setScaleSlot(%d, %d, %d, %d, %d, %d, %d)", args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
		break;
	case 22:	// setBannerColors
//		warning("o8_kernelSetFunctions: setBannerColors(%d, %d, %d, %d)", args[1], args[2], args[3], args[4]);
		break;
	case 23:	// setActorChoreLimbFrame
//		warning("o8_kernelSetFunctions: setActorChoreLimbFrame(%d, %d, %d, %d)", args[1], args[2], args[3], args[4]);
		break;
	case 24:	// clearTextQueue
		warning("o8_kernelSetFunctions: clearTextQueue()");
		break;
	case 25:	// saveGameWrite
		warning("o8_kernelSetFunctions: saveGameWrite(%d, %d)", args[1], args[2]);
		break;
	case 26:	// saveGameRead
		warning("o8_kernelSetFunctions: saveGameRead(%d, %d)", args[1], args[2]);
		break;
	case 27:	// saveGameReadName
		warning("o8_kernelSetFunctions: saveGameReadName(%d)", args[1]);
		break;
	case 28:	// saveGameStampScreenshot
		warning("o8_kernelSetFunctions: saveGameStampScreenshot(%d, %d, %d, %d, %d, %d)", args[1], args[2], args[3], args[4], args[5], args[6]);
		break;
	case 29:	// setKeyScript
		warning("o8_kernelSetFunctions: setKeyScript(%d, %d)", args[1], args[2]);
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
		warning("o8_kernelSetFunctions: default case %d (len = %d)", args[0], len);
	}
}

void Scumm_v8::o8_kernelGetFunctions()
{
	// TODO
	int args[30];
	int len = getStackList(args, sizeof(args) / sizeof(args[0]));

	switch (args[0]) {
	case 0x73:	// getWalkBoxAt
		push(getSpecialBox(args[1], args[2]));
		break;
	case 0x74:	// isPointInBox
		push(checkXYInBoxBounds(args[3], args[1], args[2]));
		break;
	case 0xCE:		// getRGBSlot
	case 0xD3:		// getKeyState
	case 0xD7:		// getBox
		push(0);
		warning("o8_kernelGetFunctions: default case %d (len = %d)", args[0], len);
		break;
	case 0xD8: {		// findBlastObject
		BlastObject *eo;
		int i;

		for (i = _enqueuePos; i >= 0; i--) {
			eo = &_enqueuedObjects[i];
			if (eo->posX <= args[1] && eo->width + eo->posX > args[1] &&
			    eo->posY <= args[2] && eo->height + eo->posY > args[2]) {
				push(eo->number);
				return;
			}
		}

		push(0);
		break;
	}
	case 0xD9:		// actorHit
		push(0);
		warning("o8_kernelGetFunctions: default case %d (len = %d)", args[0], len);
		break;
	case 0xDA:		// lipSyncWidth
	case 0xDB:		// lipSyncHeight
	case 0xDC:		// actorTalkAnimation
		// TODO - these methods are for lip syncing. Not so important right now, though
		push(0);
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
		if (!strcmp((char *)ah->data, "Saveload Page"))
			push(1);
		else
			push(0);
		}
		break;
	default:
		error("o8_kernelGetFunctions: default case %d (len = %d)", args[0], len);
	}

}

void Scumm_v8::o8_getObjectImageX()
{
	int i = getObjectIndex(pop());
	push(_objs[i].x_pos);
}

void Scumm_v8::o8_getObjectImageY()
{
	int i = getObjectIndex(pop());
	push(_objs[i].y_pos);
}

void Scumm_v8::o8_getObjectImageWidth()
{
	int i = getObjectIndex(pop());
	push(_objs[i].width);
}

void Scumm_v8::o8_getObjectImageHeight()
{
	int i = getObjectIndex(pop());
	push(_objs[i].height);
}

void Scumm_v8::o8_getStringWidth()
{
	int charset = pop();
	int len = resStrLen(_scriptPointer);
	int oldID = _charset->getCurID(); 
	int width;
	
	// Temporary set the specified charset id
	_charset->setCurID(charset);
	width = _charset->getStringWidth(0, _scriptPointer);
	_charset->setCurID(oldID);
	
	push(width);
	_scriptPointer += len + 1;
}


/*

From http://scummrev.mixnmojo.com/specs/CMIOpcodes.shtml

000 O_0
001 O_PUSH_NUMBER Push number onto stack.
002 O_PUSH_VARIABLE Push variable value onto stack.
003 O_PUSH_ARRAY_VALUE Push array value onto stack.
004 O_PUSH_ARRAY2_VALUE ?
005 O_DUP Duplicate stack value
006 O_POP Pop value from stack.
007 O_NOT NOT (!)
008 O_EQ Equals (==)
009 O_NEQ Does not equal (!=)
00A O_GT Greater than (>)
00B O_LT Less than (<)
00C O_LEQ Less than or equal (<=)
00D O_GEQ Greater than or equal (>=)
00E O_ADD Add (+)
00F O_SUB Subtract (-)
010 O_MUL Multiply (*)
011 O_DIV Divide (/)
012 O_LAND Logical AND
013 O_LOR Logical OR
014 O_BAND Binary AND
015 O_BOR Binary OR
016 O_MOD Modulus (%)
017 O_23
018 O_24
019 O_25
01A O_26
01B O_27
01C O_28
01D O_29
01E O_30
01F O_31
020 O_32
021 O_33
022 O_34
023 O_35
024 O_36
025 O_37
026 O_38
027 O_39
028 O_40
029 O_41
02A O_42
02B O_43
02C O_44
02D O_45
02E O_46
02F O_47
030 O_48
031 O_49
032 O_50
033 O_51
034 O_52
035 O_53
036 O_54
037 O_55
038 O_56
039 O_57
03A O_58
03B O_59
03C O_60
03D O_61
03E O_62
03F O_63
040 O_64
041 O_65
042 O_66
043 O_67
044 O_68
045 O_69
046 O_70
047 O_71
048 O_72
049 O_73
04A O_74
04B O_75
04C O_76
04D O_77
04E O_78
04F O_79
050 O_80
051 O_81
052 O_82
053 O_83
054 O_84
055 O_85
056 O_86
057 O_87
058 O_88
059 O_89
05A O_90
05B O_91
05C O_92
05D O_93
05E O_94
05F O_95
060 O_96
061 O_97
062 O_98
063 O_99
064 O_IF if ()
065 O_IF_NOT if not ()
066 O_JUMP jump/goto
067 O_BREAK_HERE Break out of script
068 O_BREAK_HERE_VAR
069 O_WAIT_FOR_STUFF Wait Sub opcodes
06A O_SLEEP_JIFFIES Sleep for jiffies (10ths of a second)
06B O_SLEEP_SECONDS Sleep for seconds
06C O_SLEEP_MINUTES Sleep for minutes
06D O_STORE_VARIABLE Assign value to variable
06E O_INC_VARIABLE Increase variable value (++)
06F O_DEC_VARIABLE Decrease variable value (--)
070 O_ARRAY_DIM Set dimension of array
071 O_STORE_ARRAY Assign value to array
072 O_INC_ARRAY Increase array value
073 O_DEC_ARRAY Decrease array value
074 O_ARRAY_DIM2 Set dimensions of 2 dimensional array
075 O_STORE_ARRAY2 Assign value to 2 dimensional array
076 O_ASSIGN_ARRAY ?
077 O_ARRAY_SHUFFLE Shuffle array
078 O_ARRAY_LOCALIZE ?
079 O_START_SCRIPT Start script
07A O_START_SCRIPT_QUICK ?
07B O_END_SCRIPT End script
07C O_STOP_SCRIPT Stop script from running
07D O_CHAIN_SCRIPT ?
07E O_RETURN Return
07F O_START_OBJECT ?
080 O_STOP_OBJECT ?
081 O_CUT_SCENE Start of cutscene (interface off)
082 O_END_CUT_SCENE End of cutscene (interface on)
083 O_FREEZE_SCRIPTS ?
084 O_OVERRIDE ?
085 O_OVERRIDE_OFF ?
086 O_STOP_SENTENCE ?
087 O_DEBUG Set debug mode
088 O_DEBUG_WINDEX Set debug mode with output to external window
089 O_CLASS_OF Set class of script
08A O_STATE_OF ?
08B O_OWNER_OF Set owner of object
08C O_CAMERA_PAN_TO Pan camera to (X,Y)
08D O_CAMERA_FOLLOW Make camera follow character/object
08E O_CAMERA_AT Place camera at specific (X,Y)
08F O_SAY_LINE Talk
090 O_SAY_LINE_DEFAULT Talk using default actor
091 O_SAY_LINE_SIMPLE Talk with less arguments
092 O_SAY_LINE_SIMPLE_DEFAULT Talk with less arguments using default actor
093 O_PRINT_LINE Print a line on screen
094 O_PRINT_CURSOR
095 O_PRINT_DEBUG
096 O_PRINT_SYSTEM
097 O_BLAST_TEXT Text to output to screen
098 O_DRAW_OBJECT Draw object
099 O_153
09A O_BLAST_OBJECT
09B O_155
09C O_USERFACE ?
09D O_CURRENT_ROOM Set current room
09E O_COME_OUT_DOOR
09F O_WALK_ACTOR_TO_OBJECT Walk to object
0A0 O_WALK_ACTOR_TO_XY Walk to coordinate
0A1 O_PUT_ACTOR_AT_XY Put at coordinate
0A2 O_PUT_ACTOR_AT_OBJECT Put at object
0A3 O_FACE_TOWARDS Change facing
0A4 O_DO_ANIMATION Animate
0A5 O_DO_SENTENCE
0A6 O_PICK_UP_OBJECT Pick up object
0A7 O_SET_BOX
0A8 O_SET_BOX_PATH
0A9 O_SET_BOX_SET
0AA O_HEAP_STUFF Heap sub opcodes
0AB O_ROOM_STUFF Room sub opcodes
0AC O_ACTOR_STUFF Actor sub opcodes
0AD O_CAMERA_STUFF Camera sub opcodes
0AE O_VERB_STUFF Verb sub opcodes
0AF O_START_SFX Start sound effect
0B0 O_START_MUSIC Start music
0B1 O_STOP_SOUND Stop sound (effect or music)
0B2 O_SOUND_KLUDGE ?
0B3 O_SYSTEM System sub opcodes
0B4 O_VERB_SETS
0B5 O_NEW_NAME_OF Set new name of object
0B6 O_GET_TIME_DATE Get time and/or date
0B7 O_DRAW_BOX
0B8 O_ACTOBJ_STAMP
0B9 O_START_VIDEO Start cutscene video
0BA O_KLUDGE
0BB O_187
0BC O_188
0BD O_189
0BE O_190
0BF O_191
0C0 O_192
0C1 O_193
0C2 O_194
0C3 O_195
0C4 O_196
0C5 O_197
0C6 O_198
0C7 O_199
0C8 F_START_SCRIPT Start script
0C9 F_START_OBJECT Start object script
0CA F_PICK
0CB F_PICK_DEFAULT
0CC F_PICK_RANDOM
0CD F_IN_SET
0CE F_RANDOM Get random number
0CF F_RANDOM_BETWEEN Get random number between two values
0D0 F_CLASS_OF Get class of script
0D1 F_STATE_OF
0D2 F_OWNER_OF Get owner of object
0D3 F_SCRIPT_RUNNING Test if script is running
0D4 F_OBJECT_RUNNING Test if object is running
0D5 F_SOUND_RUNNING Test if sound is running
0D6 F_ABS Get absolute value
0D7 F_PIXEL
0D8 F_KLUDGE
0D9 F_IN_BOX Test if object is in box
0DA F_VALID_VERB
0DB F_FIND_ACTOR
0DC F_FIND_OBJECT
0DD F_FIND_VERB
0DE F_FIND_ALL_OBJECTS
0DF F_ACTOR_INVENTORY
0E0 F_ACTOR_INVENTORY_COUNT Get number of items in inventory
0E1 F_ACTOR_VARIABLE Get actor variable (property)
0E2 F_ACTOR_ROOM Get current room for actor
0E3 F_ACTOR_BOX Get current box for actor
0E4 F_ACTOR_MOVING Test if actor is moving
0E5 F_ACTOR_COSTUME Get current costume for actor
0E6 F_ACTOR_SCALE Get current scale of actor
0E7 F_ACTOR_DEPTH Get current Z position of actor
0E8 F_ACTOR_ELEVATION Get current actor elevation
0E9 F_ACTOR_WIDTH Get current actor width
0EA F_ACTOBJ_FACING Get current actor/object facing
0EB F_ACTOBJ_X Get X position of actor/object
0EC F_ACTOBJ_Y Get Y position of actor/object
0ED F_ACTOR_CHORE
0EE F_PROXIMITY_2ACTOBJS Get distance between 2 actors/objects
0EF F_PROXIMITY_2POINTS Get distance between 2 points
0F0 F_OBJECT_IMAGE_X Get X position of object image
0F1 F_OBJECT_IMAGE_Y Get Y position of object image
0F2 F_OBJECT_IMAGE_WIDTH Get width of object image
0F3 F_OBJECT_IMAGE_HEIGHT Get height of object image
0F4 F_VERB_X Get X position of verb
0F5 F_VERB_Y Get Y position of verb
0F6 F_STRING_WIDTH
0F7 F_ACTOR_ZPLANE
0F8 O_248
0F9 O_249
0FA O_250
0FB O_251
0FC O_252
0FD O_253
0FE O_254
0FF O_255

The following are subopcodes - just strip the leading 1

100 SO_256
101 SO_257
102 SO_258
103 SO_259
104 SO_260
105 SO_261
106 SO_262
107 SO_263
108 SO_264
109 SO_265
10A SO_ARRAY_SCUMMVAR
10B SO_ARRAY_STRING
10C SO_ARRAY_UNDIM
10D SO_269
10E SO_270
10F SO_271
110 SO_272
111 SO_273
112 SO_274
113 SO_275
114 SO_ASSIGN_STRING
115 SO_ASSIGN_SCUMMVAR_LIST
116 SO_ASSIGN_2DIM_LIST
117 SO_279
118 SO_280
119 SO_281
11A SO_282
11B SO_283
11C SO_284
11D SO_285
11E SO_WAIT_FOR_ACTOR Wait for actor (to finish current action?)
11F SO_WAIT_FOR_MESSAGE Wait for message
120 SO_WAIT_FOR_CAMERA Wait for camera (to finish current action?)
121 SO_WAIT_FOR_SENTENCE
122 SO_WAIT_FOR_ANIMATION
123 SO_WAIT_FOR_TURN
124 SO_292
125 SO_293
126 SO_294
127 SO_295
128 SO_SYSTEM_RESTART Restart game
129 SO_SYSTEM_QUIT Quit game
12A SO_298
12B SO_299
12C SO_300
12D SO_301
12E SO_302
12F SO_303
130 SO_304
131 SO_305
132 SO_CAMERA_PAUSE
133 SO_CAMERA_RESUME
134 SO_308
135 SO_309
136 SO_310
137 SO_311
138 SO_312
139 SO_313
13A SO_314
13B SO_315
13C SO_HEAP_LOAD_CHARSET Load character set to heap
13D SO_HEAP_LOAD_COSTUME Load costume to heap
13E SO_HEAP_LOAD_OBJECT Load object to heap
13F SO_HEAP_LOAD_ROOM Load room to heap
140 SO_HEAP_LOAD_SCRIPT Load script to heap
141 SO_HEAP_LOAD_SOUND Load sound to heap
142 SO_HEAP_LOCK_COSTUME Lock costume in heap
143 SO_HEAP_LOCK_ROOM Lock room in heap
144 SO_HEAP_LOCK_SCRIPT Lock script in heap
145 SO_HEAP_LOCK_SOUND Lock sound in heap
146 SO_HEAP_UNLOCK_COSTUME Unlock costume
147 SO_HEAP_UNLOCK_ROOM Unlock room
148 SO_HEAP_UNLOCK_SCRIPT Unlock script
149 SO_HEAP_UNLOCK_SOUND Unlock sound
14A SO_HEAP_NUKE_COSTUME Remove costume from heap
14B SO_HEAP_NUKE_ROOM Remove room from heap
14C SO_HEAP_NUKE_SCRIPT Remove script from heap
14D SO_HEAP_NUKE_SOUND Remove sound from heap
14E SO_334
14F SO_335
150 SO_336
151 SO_337
152 SO_ROOM_PALETTE Set room palette
153 SO_339
154 SO_340
155 SO_ROOM_INTENSITY Set room intensity
156 SO_342
157 SO_ROOM_FADE Fade room
158 SO_ROOM_RGB_INTENSITY Set room color intensity
159 SO_ROOM_TRANSFORM Transform room
15A SO_ROOM_CYCLE_SPEED Set palette cycling speed
15B SO_ROOM_COPY_PALETTE Copy palette
15C SO_ROOM_NEW_PALETTE Create new palette
15D SO_ROOM_SAVE_GAME Save game
15E SO_ROOM_LOAD_GAME Load game
15F SO_ROOM_SATURATION Set saturation of room colors
160 SO_352
161 SO_353
162 SO_354
163 SO_355
164 SO_ACTOR_COSTUME Set actor costume
165 SO_ACTOR_STEP_DIST Set actor width of steps
166 SO_358
167 SO_ACTOR_ANIMATION_DEFAULT Set actor animation to default
168 SO_ACTOR_ANIMATION_INIT Initialize animation
169 SO_ACTOR_ANIMATION_TALK Set actor animation to talk animation
16A SO_ACTOR_ANIMATION_WALK Set actor animation to walk animation
16B SO_ACTOR_ANIMATION_STAND Set actor animation to standing animation
16C SO_ACTOR_ANIMATION_SPEED Set speed of animation
16D SO_ACTOR_DEFAULT
16E SO_ACTOR_ELEVATION
16F SO_ACTOR_PALETTE Set actor palette
170 SO_ACTOR_TALK_COLOR Set actor talk color
171 SO_ACTOR_NAME Set name of actor
172 SO_ACTOR_WIDTH Set width of actor
173 SO_ACTOR_SCALE Set scaling of actor
174 SO_ACTOR_NEVER_ZCLIP ?
175 SO_ACTOR_ALWAYS_ZCLIP ?
176 SO_ACTOR_IGNORE_BOXES Make actor ignore boxes
177 SO_ACTOR_FOLLOW_BOXES Make actor follow boxes
178 SO_ACTOR_SPECIAL_DRAW
179 SO_ACTOR_TEXT_OFFSET Set text offset relative to actor
17A SO_ACTOR_INIT Initialize actor
17B SO_ACTOR_VARIABLE Set actor variable
17C SO_ACTOR_IGNORE_TURNS_ON Make actor ignore turns
17D SO_ACTOR_IGNORE_TURNS_OFF Make actor follow turns
17E SO_ACTOR_NEW New actor
17F SO_ACTOR_DEPTH Set actor Z position
180 SO_ACTOR_STOP
181 SO_ACTOR_FACE Make actor face angle
182 SO_ACTOR_TURN Turn actor
183 SO_ACTOR_WALK_SCRIPT Set walk script for actor?
184 SO_ACTOR_TALK_SCRIPT Set talk script for actor?
185 SO_ACTOR_WALK_PAUSE
186 SO_ACTOR_WALK_RESUME
187 SO_ACTOR_VOLUME Set volume of actor speech
188 SO_ACTOR_FREQUENCY Set frequency of actor speech
189 SO_ACTOR_PAN
18A SO_394
18B SO_395
18C SO_396
18D SO_397
18E SO_398
18F SO_399
190 SO_400
191 SO_401
192 SO_402
193 SO_403
194 SO_404
195 SO_405
196 SO_VERB_INIT Choose verb number for editing
197 SO_VERB_NEW New verb
198 SO_VERB_DELETE Delete verb
199 SO_VERB_NAME Set verb name
19A SO_VERB_AT Set verb (X,Y) placement
19B SO_VERB_ON Turn verb on
19C SO_VERB_OFF Turn verb off
19D SO_VERB_COLOR Set verb color
19E SO_VERB_HICOLOR Set verb highlighted color
19F SO_415
1A0 SO_VERB_DIMCOLOR Set verb dimmed (disabled) color
1A1 SO_VERB_DIM
1A2 SO_VERB_KEY Set keypress to associate with verb
1A3 SO_VERB_IMAGE Set verb image
1A4 SO_VERB_NAME_STR Set verb name
1A5 SO_VERB_CENTER Center verb
1A6 SO_VERB_CHARSET Choose charset for verb
1A7 SO_VERB_LINE_SPACING Choose linespacing for verb
1A8 SO_424
1A9 SO_425
1AA SO_426
1AB SO_427
1AC SO_428
1AD SO_429
1AE SO_430
1AF SO_431
1B0 SO_432
1B1 SO_433
1B2 SO_434
1B3 SO_435
1B4 SO_VERBS_SAVE
1B5 SO_VERBS_RESTORE
1B6 SO_VERBS_DELETE
1B7 SO_439
1B8 SO_440
1B9 SO_441
1BA SO_442
1BB SO_443
1BC SO_444
1BD SO_445
1BE SO_446
1BF SO_447
1C0 SO_448
1C1 SO_449
1C2 SO_450
1C3 SO_451
1C4 SO_452
1C5 SO_453
1C6 SO_454
1C7 SO_455
1C8 SO_PRINT_BASEOP
1C9 SO_PRINT_END
1CA SO_PRINT_AT Print at coordinates (x,y)
1CB SO_PRINT_COLOR Print color
1CC SO_PRINT_CENTER Center output
1CD SO_PRINT_CHARSET Set print character set
1CE SO_PRINT_LEFT Left justify output
1CF SO_PRINT_OVERHEAD
1D0 SO_PRINT_MUMBLE
1D1 SO_PRINT_STRING Set string to print
1D2 SO_PRINT_WRAP Set print wordwrap
1D3 SO_467
1D4 SO_468
1D5 SO_469
1D6 SO_470
1D7 SO_471
1D8 SO_472
1D9 SO_473
1DA SO_474
1DB SO_475
1DC SO_CURSOR_ON Turn cursor on
1DD SO_CURSOR_OFF Turn cursor off
1DE SO_CURSOR_SOFT_ON Turn soft cursor on
1DF SO_CURSOR_SOFT_OFF Turn soft cursor off
1E0 SO_USERPUT_ON
1E1 SO_USERPUT_OFF
1E2 SO_USERPUT_SOFT_ON
1E3 SO_USERPUT_SOFT_OFF
1E4 SO_CURSOR_IMAGE Set cursor image
1E5 SO_CURSOR_HOTSPOT Set cursor hotspot
1E6 SO_CURSOR_TRANSPARENT Set cursor transparent color
1E7 SO_CHARSET_SET
1E8 SO_CHARSET_COLOR
1E9 SO_CURSOR_PUT
1EA SO_490
1EB SO_491
1EC SO_492
1ED SO_493
1EE SO_494
1EF SO_495
1F0 SO_496
1F1 SO_497
1F2 SO_498
1F3 SO_499
1F4 SO_500
1F5 SO_501
1F6 SO_502
1F7 SO_503
1F8 SO_504
1F9 SO_505
1FA SO_506
1FB SO_507
1FC SO_508
1FD SO_509
1FE SO_510
1FF SO_511

*/
