/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/config-manager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"

namespace Scumm {

#define OPCODE(x)	_OPCODE(ScummEngine_v100he, x)

void ScummEngine_v100he::setupOpcodes() {
	static const OpcodeEntryV100he opcodes[256] = {
		/* 00 */
		OPCODE(o100_actorOps),
		OPCODE(o6_add),
		OPCODE(o6_faceActor),
		OPCODE(o90_sortArray),
		/* 04 */
		OPCODE(o100_arrayOps),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		OPCODE(o6_breakHere),
		/* 08 */
		OPCODE(o6_delayFrames),
		OPCODE(o90_shl),
		OPCODE(o90_shr),
		OPCODE(o6_invalid),
		/* 0C */
		OPCODE(o6_setCameraAt),
		OPCODE(o6_actorFollowCamera),
		OPCODE(o6_loadRoom),
		OPCODE(o6_panCameraTo),
		/* 10 */
		OPCODE(o6_invalid),
		OPCODE(o72_jumpToScript),
		OPCODE(o6_setClass),
		OPCODE(o60_closeFile),
		/* 14 */
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o72_setFilePath),
		OPCODE(o6_invalid),
		/* 18 */
		OPCODE(o6_cutscene),
		OPCODE(o6_pop),
		OPCODE(o72_traceStatus),
		OPCODE(o6_wordVarDec),
		/* 1C */
		OPCODE(o6_wordArrayDec),
		OPCODE(o72_deleteFile),
		OPCODE(o100_dim2dimArray),
		OPCODE(o100_dimArray),
		/* 20 */
		OPCODE(o6_div),
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		OPCODE(o80_drawBox),
		/* 24 */
		OPCODE(o72_drawWizImage),
		OPCODE(o80_drawWizPolygon),
		OPCODE(o6_invalid),
		OPCODE(o100_drawObject),
		/* 28 */
		OPCODE(o6_dup),
		OPCODE(o90_dup),
		OPCODE(o6_endCutscene),
		OPCODE(o6_stopObjectCode),
		/* 2C */
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_eq),
		OPCODE(o6_invalid),
		OPCODE(o6_freezeUnfreeze),
		/* 30 */
		OPCODE(o6_ge),
		OPCODE(o6_getDateTime),
		OPCODE(o100_unknown28),
		OPCODE(o6_gt),
		/* 34 */
		OPCODE(o100_resourceRoutines),
		OPCODE(o6_if),
		OPCODE(o6_ifNot),
		OPCODE(o100_wizImageOps),
		/* 38 */
		OPCODE(o72_isAnyOf),
		OPCODE(o6_wordVarInc),
		OPCODE(o6_wordArrayInc),
		OPCODE(o6_jump),
		/* 3C */
		OPCODE(o70_kernelSetFunctions),
		OPCODE(o6_land),
		OPCODE(o6_le),
		OPCODE(o60_localizeArrayToScript),
		/* 40 */
		OPCODE(o6_wordArrayRead),
		OPCODE(o6_wordArrayIndexedRead),
		OPCODE(o6_lor),
		OPCODE(o6_lt),
		/* 44 */
		OPCODE(o90_mod),
		OPCODE(o6_mul),
		OPCODE(o6_neq),
		OPCODE(o100_dim2dim2Array),
		/* 48 */
		OPCODE(o6_setObjectName),
		OPCODE(o6_invalid),
		OPCODE(o6_not),
		OPCODE(o6_invalid),
		/* 4C */
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o72_resetCutscene),
		OPCODE(o6_setOwner),
		/* 50 */
		OPCODE(o100_paletteOps),
		OPCODE(o70_pickupObject),
		OPCODE(o70_polygonOps),
		OPCODE(o6_pop),
		/* 54 */
		OPCODE(o6_printDebug),
		OPCODE(o6_invalid),
		OPCODE(o6_printLine),
		OPCODE(o6_printSystem),
		/* 58 */
		OPCODE(o6_printCursor),
		OPCODE(o90_jumpToScriptUnk),
		OPCODE(o90_startScriptUnk),
		OPCODE(o6_pseudoRoom),
		/* 5C */
		OPCODE(o6_pushByte),
		OPCODE(o72_pushDWord),
		OPCODE(o72_addMessageToStack),
		OPCODE(o6_pushWord),
		/* 60 */
		OPCODE(o6_pushWordVar),
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_putActorAtXY),
		OPCODE(o6_invalid),
		/* 64 */
		OPCODE(o100_redimArray),
		OPCODE(o60_rename),
		OPCODE(o6_stopObjectCode),
		OPCODE(o80_localizeArrayToRoom),
		/* 68 */
		OPCODE(o100_roomOps),
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		/* 6C */
		OPCODE(o6_talkEgo),
		OPCODE(o6_invalid),
		OPCODE(o60_seekFilePos),
		OPCODE(o6_setBoxFlags),
		/* 70 */
		OPCODE(o6_invalid),
		OPCODE(o6_setBoxSet),
		OPCODE(o72_setWindowCaption),
		OPCODE(o6_shuffle),
		/* 74 */
		OPCODE(o6_delay),
		OPCODE(o6_delayMinutes),
		OPCODE(o6_delaySeconds),
		OPCODE(o100_startSound),
		/* 78 */
		OPCODE(o6_invalid),
		OPCODE(o100_unknown26),
		OPCODE(o6_stampObject),
		OPCODE(o72_startObject),
		/* 7C */
		OPCODE(o72_startScript),
		OPCODE(o6_startScriptQuick),
		OPCODE(o80_setState),
		OPCODE(o6_stopObjectScript),
		/* 80 */
		OPCODE(o6_stopScript),
		OPCODE(o6_stopSentence),
		OPCODE(o6_stopSound),
		OPCODE(o6_stopTalking),
		/* 84 */
		OPCODE(o6_writeWordVar),
		OPCODE(o6_wordArrayWrite),
		OPCODE(o6_wordArrayIndexedWrite),
		OPCODE(o6_sub),
		/* 88 */
		OPCODE(o100_quitPauseRestart),
		OPCODE(o6_invalid),
		OPCODE(o72_setTimer),
		OPCODE(o100_cursorCommand),
		/* 8C */
		OPCODE(o6_invalid),
		OPCODE(o100_wait),
		OPCODE(o6_walkActorToObj),
		OPCODE(o6_walkActorTo),
		/* 90 */
		OPCODE(o100_writeFile),
		OPCODE(o72_writeINI),
		OPCODE(o80_writeConfigFile),
		OPCODE(o6_abs),
		/* 94 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getObjectOldDir),
		/* 98 */
		OPCODE(o6_getActorMoving),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorRoom),
		OPCODE(o6_getActorScaleX),
		/* 9C */
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_getActorWidth),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		/* A0 */
		OPCODE(o90_atan2),
		OPCODE(o90_getSegmentAngle),
		OPCODE(o90_getActorAnimProgress),
		OPCODE(o6_invalid),
		/* A4 */
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_invalid),
		OPCODE(o90_cond),
		OPCODE(o90_cos),
		/* A8 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorFromXY),
		OPCODE(o72_findAllObjects),
		/* AC */
		OPCODE(o90_findAllObjectsWithClassOf),
		OPCODE(o6_invalid),
		OPCODE(o6_findInventory),
		OPCODE(o70_findObject),
		/* B0 */
		OPCODE(o72_findObjectWithClassOf),
		OPCODE(o70_polygonHit),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* B4 */
		OPCODE(o72_getNumFreeArrays),
		OPCODE(o72_getArrayDimSize),
		OPCODE(o72_checkGlobQueue),
		OPCODE(o100_getResourceSize),
		/* B8 */
		OPCODE(o100_unknown27),
		OPCODE(o6_invalid),
		OPCODE(o100_unknown29),
		OPCODE(o6_isActorInBox),
		/* BC */
		OPCODE(o6_isAnyOf),
		OPCODE(o6_getInventoryCount),
		OPCODE(o72_kernelGetFunctions),
		OPCODE(o90_max),
		/* C0 */
		OPCODE(o90_min),
		OPCODE(o72_getObjectImageX),
		OPCODE(o72_getObjectImageY),
		OPCODE(o6_isRoomScriptRunning),
		/* C4 */
		OPCODE(o90_getObjectData),
		OPCODE(o72_openFile),
		OPCODE(o90_getPolygonOverlap),
		OPCODE(o6_getOwner),
		/* C8 */
		OPCODE(o100_getPaletteData),
		OPCODE(o6_pickOneOf),
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o80_pickVarRandom),
		/* CC */
		OPCODE(o72_getPixel),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* D0 */
		OPCODE(o6_getRandomNumber),
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o100_readFile),
		/* D4 */
		OPCODE(o72_readINI),
		OPCODE(o80_readConfigFile),
		OPCODE(o6_isScriptRunning),
		OPCODE(o90_sin),
		/* D8 */
		OPCODE(o72_getSoundElapsedTimeOfSound),
		OPCODE(o6_isSoundRunning),
		OPCODE(o80_getSoundVar),
		OPCODE(o100_unknown25),
		/* DC */
		OPCODE(o90_sqrt),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		OPCODE(o6_getState),
		/* E0 */
		OPCODE(o70_compareString),
		OPCODE(o72_copyString),
		OPCODE(o72_appendString),
		OPCODE(o72_concatString),
		/* E4 */
		OPCODE(o70_getStringLen),
		OPCODE(o70_getStringLenForWidth),
		OPCODE(o6_invalid),
		OPCODE(o70_getCharIndexInString),
		/* E8 */
		OPCODE(o70_getStringWidth),
		OPCODE(o60_readFilePos),
		OPCODE(o72_getTimer),
		OPCODE(o6_getVerbEntrypoint),
		/* EC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F4 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
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

	_opcodesV100he = opcodes;
}

void ScummEngine_v100he::executeOpcode(byte i) {
	OpcodeProcV100he op = _opcodesV100he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v100he::getOpcodeDesc(byte i) {
	return _opcodesV100he[i].desc;
}

void ScummEngine_v100he::o100_actorOps() {
	Actor *a;
	int i, j, k;
	int args[32];
	byte subOp;
	byte string[256];

	subOp = fetchScriptByte();
	if (subOp == 129) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o100_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case 3:
		pop();
		pop();
		pop();
		break;
	case 4:		// SO_ANIMATION_SPEED
		a->setAnimSpeed(pop());
		break;
	case 6:
		i = pop();
		j = pop();
		a->putActor(i, j, a->room);
		break;
	case 8:
		a->drawToBackBuf = false;
		a->needRedraw = true;
		a->needBgReset = true;
		break;
	case 9:		
		{
			int top_actor = a->top;
			int bottom_actor = a->bottom;
			a->drawToBackBuf = true;
			a->needRedraw = true;
			a->drawActorCostume();
			a->drawToBackBuf = false;
			a->needRedraw = true;
			a->drawActorCostume();
			a->needRedraw = false;

			if (a->top > top_actor)
				a->top = top_actor;
			if (a->bottom < bottom_actor)
				a->bottom = bottom_actor;

		}
		break;
	case 14:
		a->charset = pop();
		break;
	case 18:
	case 128:
		_actorClipOverride.bottom = pop();
		_actorClipOverride.right = pop();
		_actorClipOverride.top = pop();
		_actorClipOverride.left = pop();
		break;
	case 22:
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; ++i) {
			a->setUserCondition(args[i] & 0x7F, args[i] & 0x80);
		}
		debug(1,"o100_actorOps: case 21 (%d)", k);
		break;
	case 25:		// SO_COSTUME
		a->setActorCostume(pop());
		break;
	case 27:		// SO_DEFAULT
		a->initActor(0);
		break;
	case 32:
		i = pop();
		debug(1,"o100_actorOps: case 32 (%d)", i);
		break;
	case 52:		// SO_ACTOR_NAME
		copyScriptString(string);
		loadPtrToResource(rtActorName, a->number, string);
		break;
	case 53:		// SO_ACTOR_NEW
		a->initActor(2);
		break;
	case 57:		// SO_PALETTE
		j = pop();
		i = pop();
		checkRange(255, 0, i, "o100_actorOps: Illegal palette slot %d");
		a->remapActorPaletteColor(i, j);
		break;
	case 59:
		// Uses reverse order of layering, so we adjust
		a->_layer = -pop();
		break;
	case 63:
		a->hePaletteNum = pop();
		break;
	case 65:		// SO_SCALE
		i = pop();
		a->setScale(i, i);
		break;
	case 70:		// SO_SHADOW
		a->_shadowMode = pop();
		debug(0, "o100_actorOps: Set actor XMAP idx to %d", a->_shadowMode);
		break;
	case 74:		// SO_STEP_DIST
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case 78:
		{
		copyScriptString(string);
		int slot = pop();

		int len = resStrLen(string) + 1;
		addMessageToStack(string, a->heTalkQueue[slot].sentence, len);

		a->heTalkQueue[slot].posX = a->talkPosX;
		a->heTalkQueue[slot].posY = a->talkPosY;
		a->heTalkQueue[slot].color = a->talkColor;
		}
		break;
	case 83:		// SO_ACTOR_VARIABLE
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case 87:		// SO_ALWAYS_ZCLIP
		a->forceClip = pop();
		break;
	case 89:		// SO_NEVER_ZCLIP
		a->forceClip = 0;
		break;
	case 130:		// SO_SOUND
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->sound[i] = args[i];
		break;
	case 131:		// SO_ACTOR_WIDTH
		a->width = pop();
		break;
	case 132:		// SO_ANIMATION_DEFAULT
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case 133:		// SO_ELEVATION
		a->setElevation(pop());
		break;
	case 134:		// SO_FOLLOW_BOXES
		a->ignoreBoxes = 0;
		a->forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->room);
		break;
	case 135:		// SO_IGNORE_BOXES
		a->ignoreBoxes = 1;
		a->forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->room);
		break;
	case 136:		// SO_ACTOR_IGNORE_TURNS_OFF
		a->ignoreTurns = false;
		break;
	case 137:		// SO_ACTOR_IGNORE_TURNS_ON
		a->ignoreTurns = true;
		break;
	case 138:		// SO_INIT_ANIMATION
		a->_initFrame = pop();
		break;
	case 139:		// SO_STAND_ANIMATION
		a->_standFrame = pop();
		break;
	case 140:		// SO_TALK_ANIMATION
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case 141:		// SO_TALK_COLOR
		a->talkColor = pop();
		break;
	case 142:
		k = pop();
		a->heNoTalkAnimation = 1;
		a->setTalkCondition(k);
		debug(1,"o100_actorOps: case 24 (%d)", k);
		break;
	case 143:		// SO_TEXT_OFFSET
		a->talkPosY = pop();
		a->talkPosX = pop();
		break;
	case 144:		// SO_WALK_ANIMATION
		a->_walkFrame = pop();
		break;
	default:
		error("o100_actorOps: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_arrayOps() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	int offs, tmp, tmp2, tmp3;
	int dim1end, dim1start, dim2end, dim2start;
	int id, len, b, c;
	ArrayHeader *ah;
	int list[128];
	byte string[2048];

	debug(1,"o100_arrayOps: case %d", subOp);
	switch (subOp) {
	case 35:			// SO_ASSIGN_STRING
		decodeScriptString(string);
		len = resStrLen(string) + 1;
		ah = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(ah->data, string, len);
		break;
	case 77:			// SO_ASSIGN_STRING
		copyScriptString(string);
		len = resStrLen(string) + 1;
		ah = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(ah->data, string, len);
		break;

	case 128:		// SO_ASSIGN_2DIM_LIST
		len = getStackList(list, ARRAYSIZE(list));
		id = readVar(array);
		if (id == 0)
			error("o100_arrayOps: Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, len, list[len]);
		}
		break;
	case 129:		// SO_ASSIGN_INT_LIST
		b = pop();
		c = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, 0, 0, 0, b + c);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 130:
		len = getStackList(list, ARRAYSIZE(list));
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		tmp2 = 0;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, list[tmp2++]);
				if (tmp2 == len)
					tmp2 = 0;
				tmp++;
			}
			dim2start++;
		}
		break;
	case 131:
		{
		// TODO
		//Array1
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		//Array2
		array = fetchScriptWord();
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		}
		break;
	case 133:
		b = pop();
		c = pop();
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}

		len = c - b;
		len |= dim2end;
		len = len - dim2end + 1;
		offs = (b >= c) ? 1 : -1;
		tmp2 = c;
		tmp3 = len;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, tmp2);
				if (--tmp3 == 0) {
					tmp2 = c;
					tmp3 = len;
				} else {
					tmp2 += offs;
				}
				tmp++;
			}
			dim2start++;
		}
		break;
	default:
		error("o100_arrayOps: default case %d (array %d)", subOp, array);
	}
}

void ScummEngine_v100he::o100_dim2dimArray() {
	int a, b, data;
	int type = fetchScriptByte();
	switch (type) {
	case 41:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 42:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 43:		
		data = kDwordArray;
		break;
	case 44:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 45:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 77:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	default:
		error("o100_dim2dimArray: default case %d", type);
	}

	b = pop();
	a = pop();
	defineArray(fetchScriptWord(), data, 0, a, 0, b);
}

void ScummEngine_v100he::o100_dimArray() {
	int data;
	int type = fetchScriptByte();

	switch (type) {
	case 41:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 42:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 43:
		data = kDwordArray;
		break;
	case 44:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 45:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 77:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	case 135:		// SO_UNDIM_ARRAY
		nukeArray(fetchScriptWord());
		return;
	default:
		error("o100_dimArray: default case %d", type);
	}

	defineArray(fetchScriptWord(), data, 0, 0, 0, pop());
}

void ScummEngine_v100he::o100_drawObject() {
	byte subOp = fetchScriptByte();
	int state = 0, y = -1, x = -1;

	switch (subOp) {
	case 6:
		state = 1;
		y = pop();
		x = pop();
		break;
	case 7:
		state = pop();
		y = pop();
		x = pop();
		break;
	case 40:
		state = pop();
		if (state == 0)
			state = 1;
		break;
	default:
		error("o100_drawObject: default case %d", subOp);
	}

	int object = pop();
	int objnum = getObjectIndex(object);
	if (objnum == -1)
		return;

	if (y != -1 && x != -1) {
		_objs[objnum].x_pos = x * 8;
		_objs[objnum].y_pos = y * 8;
	}

	if (state != -1) {
		addObjectToDrawQue(objnum);
		putState(object, state);
	}
}

void ScummEngine_v100he::o100_unknown28() {
	byte string[260];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0:
		pop();
		break;
	case 6:
		pop();
		pop();
		break;
	case 18:
		pop();
		pop();
		pop();
		pop();
		break;
	case 38:
		switch (pop()) {
		case 1:
			pop();
			pop();
			break;
		case 2:
			pop();
			break;
		case 3:
			pop();
			break;
		case 4:
			pop();
			break;
		case 5:
			break;
		case 6:
			pop();
			break;
		case 7:
			pop();
			break;
		case 8:
			pop();
			break;
		default:
			error("o100_unknown28 subOp 38: Unknown case %d", subOp);
		}
		break;
	case 40:
		pop();
		break;
	case 49:
		pop();
		pop();
		break;
	case 52:
		copyScriptString(string);
		break;
	case 53:
		break;
	case 54:
		pop();
		pop();
		break;
	case 59:
		pop();
		break;
	case 60:
		pop();
		pop();
		break;
	case 89:
		break;
	default:
		error("o100_unknown28: Unknown case %d", subOp);
	}
	debug(1,"o100_unknown28 stub (%d)", subOp);
}

void ScummEngine_v100he::o100_resourceRoutines() {
	int obj, objidx, room;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 14:
		_heResType = rtCharset;
		_heResId = pop();
		break;
	case 25:
		_heResType = rtCostume;
		_heResId = pop();
		break;
	case 34:
		_heResType = rtFlObject;
		_heResId = pop();
		break;
	case 40:
		_heResType = rtImage;
		_heResId = pop();
		break;
	case 47:
		if (_heResType == rtFlObject) {
			obj = _heResId;
			room= getObjectRoom(obj);
			loadFlObject(obj, room);
		} else if (_heResType == rtCharset) {
			loadCharset(_heResId);
		} else {
			ensureResourceLoaded(_heResType, _heResId);
		}
		break;
	case 62:
		_heResType = rtRoom;
		_heResId = pop();
		break;
	case 66:
		_heResType = rtScript;
		_heResId = pop();
		break;
	case 72:
		_heResType = rtSound;
		_heResId = pop();
		break;
	case 128:
		break;
	case 132:
		if (_heResType == rtScript && _heResId >= _numGlobalScripts)
			break;

		if (_heResType == rtFlObject) {
			objidx = getObjectIndex(_heResId);
			//assert(objidx != -1);
			//lock(_heResType, objidx);
		} else {
			lock(_heResType, _heResId);
		}
		break;
	case 133:
		if (_heResType == rtCharset)
			nukeCharset(_heResId);
		else
			nukeResource(_heResType, _heResId);
		break;
	case 134:
	case 135:
		// Heap related
		break;
	case 136:
		debug(5,"stub queueload (%d) resource %d", _heResType, _heResId);
		break;
	case 137:
		if (_heResType == rtScript && _heResId >= _numGlobalScripts)
			break;

		if (_heResType == rtFlObject) {
			objidx = getObjectIndex(_heResId);
			//assert(objidx != -1);
			//unlock(_heResType, objidx);
		} else {
			unlock(_heResType, _heResId);
		}
		break;
	default:
		error("o100_resourceRoutines: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_wizImageOps() {
	int a, b;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0:
		_wizParams.img.resNum = pop();
		_wizParams.processMode = 0;
		_wizParams.processFlags = 0;
		_wizParams.remapNum = 0;
		_wizParams.img.flags = 0;
		break;
	case 2:
		_wizParams.processFlags |= 0x10;
		_wizParams.angle = pop();
		break;
	case 6:
		_wizParams.processFlags |= 1;
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		break;
	case 7:
		_wizParams.processFlags |= 0x80000;
		pop();
		break;
	case 11:
		_wizParams.processFlags |= 0x300;
		_wizParams.processMode = 2;
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		_wizParams.compType = pop();
		break;
	case 18:
		_wizParams.processFlags |= 0x200;
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		break;
	case 21:
		b = pop();
		a = pop();
		_wizParams.processFlags |= 0x40;
		_wizParams.processMode = 6;
		if (_wizParams.remapNum == 0) {
			memset(_wizParams.remapIndex, 0, sizeof(_wizParams.remapIndex));
		} else {
			assert(_wizParams.remapNum < ARRAYSIZE(_wizParams.remapIndex));
			_wizParams.remapIndex[_wizParams.remapNum] = a;
			_wizParams.remapColor[a] = b;
			++_wizParams.remapNum;
		}
		break;
	case 29:
		_wizParams.processMode = 1;
		break;
	case 36:
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		break;
	case 37:
		pop();
		break;
	case 39:
		_wizParams.processFlags |= 0x4000;
		pop();
		break;
	case 47:
		_wizParams.processFlags |= 0x800;
		_wizParams.processMode = 3;
		copyScriptString(_wizParams.filename);
		break;
	case 53:
		_wizParams.processMode = 8;
		break;
	case 54:
		_wizParams.processFlags |= 0x10000;
		pop();
		pop();
		break;
	case 55:
		_wizParams.img.flags = pop();
		_wizParams.img.state = pop();
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		_wizParams.img.resNum = pop();
		displayWizImage(&_wizParams.img);
		break;
	case 57:
		_wizParams.processFlags |= 0x8000;
		pop();
		break;
	case 58:
		_wizParams.processFlags |= 0x1102;
		_wizParams.processMode = 7;
		pop();
		pop();
		_wizParams.compType = pop();
	case 64:
		_wizParams.processFlags |= 0x800;
		_wizParams.processMode = 4;
		copyScriptString(_wizParams.filename);
		_wizParams.unk_14C = pop();
		break;
	case 65:
		_wizParams.processFlags |= 8;
		_wizParams.zoom = pop();
		break;
	case 67:
		_wizParams.processFlags |= 0x20;
		_wizParams.img.flags = pop();
		break;
	case 68:
		_wizParams.processFlags |= 0x23;
		_wizParams.img.flags |= 0x40;
		_wizParams.unk_160 = _wizParams.img.y1 = _wizParams.img.x1 = pop();
		break;
	case 70:
		_wizParams.processFlags |= 4;
		_wizParams.unk_15C = pop();
		break;
	case 73:
		_wizParams.processFlags |= 0x400;
		_wizParams.img.state = pop();
		break;
	case 84:
		_wizParams.processFlags |= 0x2000;
		pop();
		break;
	case 92:
		processWizImage(&_wizParams);
		break;
	case 128:
		pop();
		pop();
		pop();
		pop();
		//copyScriptString(?.filename);
		_wizParams.processMode = 15;
		break;
	case 129:
		_wizParams.processMode = 14;
		break;
	case 130:
		_wizParams.processMode = 16;
		pop();
		pop();
		//copyScriptString(?.filename);
		break;
	case 131:
		_wizParams.processMode = 13;
		break;
	case 133:
		_wizParams.processMode = 16;
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 134:
		_wizParams.processFlags |= 0x60000;
		_wizParams.processMode = 12;
		pop();
		pop();
		pop();
		break;
	case 135:
		_wizParams.processFlags |= 0x10000;
		pop();
		break;
	case 136:
		_wizParams.processFlags |= 0x60000;
		_wizParams.processMode = 10;
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 137:
		_wizParams.processFlags |= 0x60000;
		_wizParams.processMode = 11;
		pop();
		pop();
		pop();
		break;
	case 138:
		_wizParams.processFlags |= 0x60000;
		_wizParams.processMode = 9;
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	default:
		error("o100_wizImageOps: Unknown case %d", subOp);
	}
	debug(1,"o100_wizImageOps stub (%d)", subOp);
}

void ScummEngine_v100he::o100_dim2dim2Array() {
	int data, dim1start, dim1end, dim2start, dim2end;
	int type = fetchScriptByte();

	switch (type) {
	case 41:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 42:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 43:
		data = kDwordArray;
		break;
	case 44:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 45:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 77:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	default:
		error("o100_dim2dim2Array: default case %d", type);
	}

	if (pop() == 2) {
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
	} else {
		dim2end = pop();
		dim2start = pop();
		dim1end = pop();
		dim1start = pop();
	}

	defineArray(fetchScriptWord(), data, dim2start, dim2end, dim1start, dim1end);
}

void ScummEngine_v100he::o100_paletteOps() {
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0:
		_hePaletteNum = pop();
		break;
	case 20:
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 25:
		pop();
		break;
	case 40:
		pop();
		pop();
		break;
	case 53:
		break;
	case 57:
		pop();
		break;
	case 63:
		pop();
		pop();
		break;
	case 81:
		pop();
		pop();
		pop();
		break;
	case 92:
		_hePaletteNum = 0;
		break;
	default:
		error("o100_paletteOps: Unknown case %d", subOp);
	}
	debug(0,"o100_paletteOps stub (%d)", subOp);
}

void ScummEngine_v100he::o100_redimArray() {
	int newX, newY;
	newY = pop();
	newX = pop();

	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 42:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kIntArray);
		break;
	case 43:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kDwordArray);
		break;
	case 45:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kByteArray);
		break;
	default:
		error("o100_redimArray: default type %d", subOp);
	}
}

void ScummEngine_v100he::o100_roomOps() {
	int a, b, c, d, e;
	byte op;
	byte filename[100];

	op = fetchScriptByte();

	switch (op) {
	case 63:		// SO_ROOM_PALETTE
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case 129:
		b = pop();
		a = pop();
		swapObjects(a, b);
		break;

	case 130:
		a = pop();
		b = pop();
		copyPalColor(a, b);
		break;

	case 131:		// SO_ROOM_FADE
		// Defaults to 1 but doesn't use fade effects
		a = pop();
		break;

	case 132:		// SO_ROOM_INTENSITY
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;

	case 133:		// SO_RGB_ROOM_INTENSITY
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case 134:		// SO_ROOM_NEW_PALETTE
		a = pop();
		setPalette(a, _roomResource);
		break;

	case 135:
		b = pop();
		a = pop();
		setPalette(a, b);
		break;

	case 136:		// SO_ROOM_SAVEGAME
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		break;

	case 137:
		copyScriptString(filename);
		_saveLoadFlag = pop();
		_saveLoadSlot = 1;
		_saveTemporaryState = true;
		break;

	case 138:		// SO_ROOM_SCREEN
		b = pop();
		a = pop();
		initScreens(a, _screenHeight);
		break;

	case 139:		// SO_ROOM_SCROLL
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

	default:
		error("o100_roomOps: default case %d", op);
	}
}

void ScummEngine_v100he::o100_startSound() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 6:
		_heSndFlags |= 16;
		pop();
		break;
	case 92:
		debug(0, "o100_startSound (ID %d, Offset %d, Channel %d, Flags %d)", _heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags);
		_sound->addSoundToQueue(_heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags);
		break;
	case 128:
		_heSndFlags |= 2;
		break;
	case 129:
		_heSndChannel = pop();
		break;
	case 130:
		_heSndFlags |= 40;
		pop();
		break;
	case 131:
		_heSndFlags |= 4;
		break;
	case 132:
	case 134:
		_heSndSoundId = pop();
		_heSndOffset = 0;
		_heSndSoundFreq = 11025;
		_heSndChannel = VAR(VAR_MUSIC_CHANNEL);
		break;
	case 133:
		_heSndFlags |= 80;
		pop();
		break;
	case 135:
		_heSndFlags |= 4;
		break;
	case 136:
		_heSndFlags |= 20;
		pop();
		break;
	default:
		error("o100_startSound invalid case %d", subOp);
	}
}

void ScummEngine_v100he::o100_unknown26() {
	int args[16];
	byte string[80];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0:
		pop();
		pop();
		break;
	case 2:
		pop();
		break;
	case 3:
		pop();
		break;
	case 4:
		pop();
		break;
	case 6:
		pop();
		pop();
		break;
	case 7:
		pop();
		break;
	case 16:
		getStackList(args, ARRAYSIZE(args));
		break;
	case 32:
		pop();
		break;
	case 38:
		pop();
		break;
	case 40:
		pop();
		break;
	case 48:
		pop();
		break;
	case 49:
		pop();
		pop();
		break;
	case 52:
		copyScriptString(string);
		break;
	case 53:
		break;
	case 54:
		pop();
		pop();
		break;
	case 57:
		pop();
		break;
	case 59:
		pop();
		break;
	case 60:
		pop();
		pop();
		break;
	case 61:
		break;
	case 65:
		pop();
		break;
	case 70:
		pop();
		break;
	case 73:
		pop();
		break;
	case 74:
		pop();
		pop();
		break;
	case 75:
		pop();
		break;
	case 76:
		pop();
		break;
	case 82:
		pop();
		break;
	case 83:
		pop();
		pop();
		break;
	case 88:
		pop();
		break;
	case 89:
		break;
	default:
		error("o100_unknown26: Unknown case %d", subOp);
	}
	debug(1,"o100_unknown26 stub (%d)", subOp);
}

void ScummEngine_v100he::o100_quitPauseRestart() {
	byte subOp = fetchScriptByte();
	subOp -= 61;

	switch (subOp) {
	case 67:
		clearDrawObjectQueue();
		break;
	case 72:
		shutDown();
		break;
	case 73:		// SO_RESTART
		restart();
		break;
	case 75:
		// Clear screen
		// Update palette
		break;
	default:
		error("o100_quitPauseRestart invalid case %d", subOp);
	}
}

void ScummEngine_v100he::o100_cursorCommand() {
	int a, i;
	int args[16];
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0xE:		// SO_CHARSET_SET
		initCharset(pop());
		break;
	case 0xF:		// SO_CHARSET_COLOR
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
		break;
	case 0x80:
		a = pop();
		loadWizCursor(a, rtInventory, 0);
		break;
	case 0x81:
		a = pop();
		loadWizCursor(a, rtInventory, 0);
		break;
	case 0x82:
		pop();
		a = pop();
		loadWizCursor(a, rtInventory, 1);
		break;
	case 0x86:		// SO_CURSOR_ON Turn cursor on
		_cursor.state = 1;
		break;
	case 0x87:		// SO_CURSOR_OFF Turn cursor off
		_cursor.state = 0;
		break;
	case 0x88:		// SO_CURSOR_SOFT_ON Turn soft cursor on
		_cursor.state++;
		if (_cursor.state > 1)
			error("o100_cursorCommand: Cursor state greater than 1 in script");
		break;

	case 0x89:		// SO_CURSOR_SOFT_OFF Turn soft cursor off
		_cursor.state--;
		break;
	case 0x8B:		// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 0x8C:		// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 0x8D:		// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 0x8E:		// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	default:
		warning("o100_cursorCommand: default case %x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void ScummEngine_v100he::o100_wait() {
	int actnum;
	int offs = -2;
	Actor *a;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 128:		// SO_WAIT_FOR_ACTOR Wait for actor
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o100_wait:168");
		if (a->moving)
			break;
		return;
	case 129:		// SO_WAIT_FOR_CAMERA Wait for camera
		if (camera._cur.x / 8 != camera._dest.x / 8)
			break;
		return;
	case 130:		// SO_WAIT_FOR_MESSAGE Wait for message
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case 131:		// SO_WAIT_FOR_SENTENCE
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
			break;
		}
		if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;
		break;
	default:
		error("o100_wait: default case 0x%x", subOp);
	}

	_scriptPointer += offs;
	o6_breakHere();
}

void ScummEngine_v100he::o100_writeFile() {
	int16 resID = pop();
	int slot = pop();
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 5:
		fetchScriptByte();
		writeFileFromArray(slot, resID);
		break;
	case 42:
		_hFileTable[slot].writeUint16LE(resID);
		break;
	case 43:
		_hFileTable[slot].writeUint32LE(resID);
		break;
	case 45:
		_hFileTable[slot].writeByte(resID);
		break;
	default:
		error("o100_writeFile: default case %d", subOp);
	}
	debug(1, "o100_writeFile: slot %d, subOp %d", slot, subOp);
}

void ScummEngine_v100he::o100_getResourceSize() {
	int size = 0, type;

	int idx = pop();
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 25:
		type = rtCostume;
		break;
	case 40:
		type = rtImage;
		break;
	case 62:
		type = rtRoomImage;
		break;
	case 66:
		type = rtScript;
		break;
	case 72:
		if (idx > _numSounds) {
			// TODO Music resource size
			push(100);
			return;
		}
		type = rtSound;
		break;
	default:
		error("o100_getResourceSize: default type %d", subOp);
	}

	const byte *ptr = getResourceAddress(type, idx);
	if (ptr)
		size = READ_BE_UINT32(ptr + 4) - 8;
	push(size);
}

void ScummEngine_v100he::o100_unknown27() {
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 5:
		pop();
		break;
	case 48:
		pop();
		break;
	case 54:
		pop();
		pop();
		break;
	case 59:
		pop();
		break;
	case 60:
		pop();
		pop();
		break;
	case 85:
		pop();
		break;
	case 86:
		pop();
		break;
	default:
		error("o100_unknown27: Unknown case %d", subOp);
	}
	push(0);

	debug(1,"o100_unknown27 stub (%d)", subOp);
}

void ScummEngine_v100he::o100_unknown29() {
	int state, resId;
	int32 w, h;
	int16 x, y;

	byte subOp = fetchScriptByte();
	subOp -= 20;
	
	switch (subOp) {
	case 0:
		pop();
		pop();
		pop();
		pop();
		push(0);
		warning("o100_unknown29() case 0 unhandled");
		break;		
	case 6:
		resId = pop();
		push(getWizImageStates(resId));
		break;
	case 13:
		pop();
		pop();
		pop();
		pop();
		push(0);
		warning("o100_unknown29() case 13 unhandled");
		break;
	case 19:
		state = pop();
		resId = pop();
		getWizImageDim(resId, state, w, h);
		push(h);
		break;
	case 34:
		pop();
		pop();
		pop();
		push(0);
		warning("o100_unknown29() case 34 unhandled");
		break;		
	case 64:
		state = pop();
		resId = pop();
		getWizImageDim(resId, state, w, h);
		push(w);
		break;
	case 65:
		state = pop();
		resId = pop();
		loadImgSpot(resId, state, x, y);
		push(x);
		break;	
	case 66:
		state = pop();
		resId = pop();
		loadImgSpot(resId, state, x, y);
		push(y);
		break;
	case 111:
		pop();
		pop();
		push(0);
		warning("o100_unknown29() case 111 unhandled");
		break;
	case 112:
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		push(0);
		warning("o100_unknown29() case 112 unhandled");
		break;
	default:
		error("o100_unknown29: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_getPaletteData() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 13:
		pop();
		pop();
		break;
	case 20:
		pop();
		pop();
		break;
	case 33:
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 53:
		pop();
		pop();
		pop();
		break;
	case 73:
		pop();
		pop();
		pop();
		break;
	default:
		error("o100_getPaletteData: Unknown case %d", subOp);
	}
	push(0);
	debug(1,"o100_getPaletteData stub (%d)", subOp);
}

void ScummEngine_v100he::o100_readFile() {
	int slot, val;
	int32 size;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 5:
		fetchScriptByte();
		size = pop();
		slot = pop();
		val = readFileToArray(slot, size);
		push(val);
		break;
	case 42:
		slot = pop();
		val = _hFileTable[slot].readUint16LE();
		push(val);
		break;
	case 43:
		slot = pop();
		val = _hFileTable[slot].readUint32LE();
		push(val);
		break;
	case 45:
		slot = pop();
		val = _hFileTable[slot].readByte();
		push(val);
		break;
	default:
		error("o100_readFile: default case %d", subOp);
	}
	debug(1, "o100_readFile: slot %d, subOp %d val %d", slot, subOp, val);
}

void ScummEngine_v100he::o100_unknown25() {
	int args[16];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 3:
		pop();
		break;
	case 4:
		pop();
		break;
	case 7:
		pop();
		break;
	case 16:
		getStackList(args, ARRAYSIZE(args));
		pop();
		break;
	case 26:
		pop();
		break;
	case 30:
		pop();
		break;
	case 31:
		pop();
		break;
	case 32:
		pop();
		break;
	case 33:
		getStackList(args, ARRAYSIZE(args));
		pop();
		pop();
		pop();
		pop();
		break;
	case 38:
		pop();
		break;
	case 39:
		pop();
		break;
	case 40:
		pop();
		break;
	case 48:
		pop();
		break;
	case 54:
		pop();
		pop();
		break;
	case 57:
		pop();
		break;
	case 59:
		pop();
		break;
	case 60:
		pop();
		pop();
		break;
	case 65:
		pop();
		break;
	case 70:
		pop();
		break;
	case 73:
		pop();
		break;
	case 75:
		pop();
		break;
	case 76:
		pop();
		break;
	case 82:
		pop();
		break;
	case 83:
		pop();
		pop();
		break;
	case 84:
		pop();
		break;
	case 85:
		pop();
		break;
	case 86:
		pop();
		break;
	default:
		error("o100_unknown25: Unknown case %d", subOp);
	}
	push(0);
	debug(1,"o100_unknown25 stub (%d)", subOp);
}

void ScummEngine_v100he::decodeParseString(int m, int n) {
	int i, colors, size;
	int args[31];
	byte name[1024];

	byte b = fetchScriptByte();

	switch (b) {
	case 6:		// SO_AT
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case 12:		// SO_CENTER
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case 18:		// SO_CLIPPED
		_string[m].right = pop();
		break;
	case 20:		// SO_COLOR
		_string[m].color = pop();
		break;
	case 21:
		colors = pop();
		if (colors == 1) {
			_string[m].color = pop();
		} else {	
			push(colors);
			getStackList(args, ARRAYSIZE(args));
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
			_string[m].color = _charsetColorMap[0];
		}
		break;
	case 35:
		decodeScriptString(name, true);
		printString(m, name);
		break;
	case 46:		// SO_LEFT
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case 51:		// SO_MUMBLE
		_string[m].no_talk_anim = true;
		break;
	case 56:		// SO_OVERHEAD
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case 78:
		{
		const byte *dataPtr = getResourceAddress(rtTalkie, pop());
		const byte *text = findWrappedBlock(MKID('TEXT'), dataPtr, 0, 0);
		size = getResourceDataSize(text);
		memcpy(name, text, size);
		printString(m, name);
		}
		break;
	case 79:		// SO_TEXTSTRING
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case 91:
		_string[m].loadDefault();
		if (n)
			_actorToPrintStrFor = pop();
		break;
	case 92:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case %d", b);
	}
}

} // End of namespace Scumm
