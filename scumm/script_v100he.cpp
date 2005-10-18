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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"

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
		OPCODE(o90_xor),
		/* 0C */
		OPCODE(o6_setCameraAt),
		OPCODE(o6_actorFollowCamera),
		OPCODE(o6_loadRoom),
		OPCODE(o6_panCameraTo),
		/* 10 */
		OPCODE(o6_invalid),
		OPCODE(o100_jumpToScript),
		OPCODE(o6_setClass),
		OPCODE(o60_closeFile),
		/* 14 */
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o72_setFilePath),
		OPCODE(o100_loadSBNG),
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
		OPCODE(o6_drawBox),
		/* 24 */
		OPCODE(o72_drawWizImage),
		OPCODE(o80_drawWizPolygon),
		OPCODE(o100_drawLine),
		OPCODE(o100_drawObject),
		/* 28 */
		OPCODE(o6_dup),
		OPCODE(o90_dup_n),
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
		OPCODE(o100_setSpriteGroupInfo),
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
		OPCODE(o90_kernelSetFunctions),
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
		OPCODE(o100_redim2dimArray),
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
		OPCODE(o72_printWizImage),
		OPCODE(o6_printLine),
		OPCODE(o6_printSystem),
		/* 58 */
		OPCODE(o6_printText),
		OPCODE(o100_jumpToScriptUnk),
		OPCODE(o100_startScriptUnk),
		OPCODE(o6_pseudoRoom),
		/* 5C */
		OPCODE(o6_pushByte),
		OPCODE(o72_pushDWord),
		OPCODE(o72_getScriptString),
		OPCODE(o6_pushWord),
		/* 60 */
		OPCODE(o6_pushWordVar),
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_putActorAtXY),
		OPCODE(o6_invalid),
		/* 64 */
		OPCODE(o100_redimArray),
		OPCODE(o72_rename),
		OPCODE(o6_stopObjectCode),
		OPCODE(o80_localizeArrayToRoom),
		/* 68 */
		OPCODE(o100_roomOps),
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o72_talkActor),
		/* 6C */
		OPCODE(o72_talkEgo),
		OPCODE(o6_invalid),
		OPCODE(o70_seekFilePos),
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
		OPCODE(o100_setSpriteInfo),
		OPCODE(o6_stampObject),
		OPCODE(o72_startObject),
		/* 7C */
		OPCODE(o100_startScript),
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
		OPCODE(o100_systemOps),
		OPCODE(o6_invalid),
		OPCODE(o72_setTimer),
		OPCODE(o100_cursorCommand),
		/* 8C */
		OPCODE(o100_videoOps),
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
		OPCODE(o90_getActorData),
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
		OPCODE(o90_getDistanceBetweenPoints),
		/* A4 */
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_invalid),
		OPCODE(o90_cond),
		OPCODE(o90_cos),
		/* A8 */
		OPCODE(o6_invalid),
		OPCODE(o80_getFileSize),
		OPCODE(o6_getActorFromXY),
		OPCODE(o72_findAllObjects),
		/* AC */
		OPCODE(o90_findAllObjectsWithClassOf),
		OPCODE(o6_invalid),
		OPCODE(o6_findInventory),
		OPCODE(o72_findObject),
		/* B0 */
		OPCODE(o72_findObjectWithClassOf),
		OPCODE(o70_polygonHit),
		OPCODE(o90_getLinesIntersectionPoint),
		OPCODE(o90_fontUnk),
		/* B4 */
		OPCODE(o72_getNumFreeArrays),
		OPCODE(o72_getArrayDimSize),
		OPCODE(o100_isResourceLoaded),
		OPCODE(o100_getResourceSize),
		/* B8 */
		OPCODE(o100_getSpriteGroupInfo),
		OPCODE(o6_invalid),
		OPCODE(o100_getWizData),
		OPCODE(o6_isActorInBox),
		/* BC */
		OPCODE(o6_isAnyOf),
		OPCODE(o6_getInventoryCount),
		OPCODE(o90_kernelGetFunctions),
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
		OPCODE(o72_getSoundElapsedTime),
		OPCODE(o6_isSoundRunning),
		OPCODE(o80_getSoundVar),
		OPCODE(o100_getSpriteInfo),
		/* DC */
		OPCODE(o90_sqrt),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		OPCODE(o6_getState),
		/* E0 */
		OPCODE(o70_compareString),
		OPCODE(o70_copyString),
		OPCODE(o70_appendString),
		OPCODE(o70_concatString),
		/* E4 */
		OPCODE(o70_getStringLen),
		OPCODE(o70_getStringLenForWidth),
		OPCODE(o80_stringToInt),
		OPCODE(o70_getCharIndexInString),
		/* E8 */
		OPCODE(o70_getStringWidth),
		OPCODE(o60_readFilePos),
		OPCODE(o72_getTimer),
		OPCODE(o6_getVerbEntrypoint),
		/* EC */
		OPCODE(o100_getVideoData),
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
	case 0:
		// freddicove Ru Updated
		// FIXME: check stack parameters
		debug(0,"o100_actorOps: case 0 UNHANDLED");
		break;
	case 3:
		pop();
		pop();
		pop();
		break;
	case 4:		// SO_ANIMATION_SPEED
		a->setAnimSpeed(pop());
		break;
	case 6:
		j = pop();
		i = pop();
		a->putActor(i, j, a->_room);
		break;
	case 8:
		a->_drawToBackBuf = false;
		a->_needRedraw = true;
		a->_needBgReset = true;
		break;
	case 9:
		a->drawActorToBackBuf(a->_pos.x, a->_pos.y);
		break;
	case 14:
		a->_charset = pop();
		break;
	case 18:
		a->_clipOverride.bottom = pop();
		a->_clipOverride.right = pop();
		a->_clipOverride.top = pop();
		a->_clipOverride.left = pop();
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
		copyScriptString(string, sizeof(string));
		loadPtrToResource(rtActorName, a->_number, string);
		break;
	case 53:		// SO_ACTOR_NEW
		a->initActor(2);
		break;
	case 57:		// SO_PALETTE
		j = pop();
		i = pop();
		checkRange(255, 0, i, "o100_actorOps: Illegal palette slot %d");
		a->remapActorPaletteColor(i, j);
		a->_needRedraw = true;
		break;
	case 59:
		a->_layer = pop();
		a->_needRedraw = true;
		break;
	case 63:
		a->_hePaletteNum = pop();
		a->_needRedraw = true;
		break;
	case 65:		// SO_SCALE
		i = pop();
		a->setScale(i, i);
		break;
	case 70:		// SO_SHADOW
		a->_shadowMode = pop();
		a->_needRedraw = true;
		debug(0, "o100_actorOps: Set actor XMAP idx to %d", a->_shadowMode);
		break;
	case 74:		// SO_STEP_DIST
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case 78:
		{
		copyScriptString(string, sizeof(string));
		int slot = pop();

		int len = resStrLen(string) + 1;
		memcpy(a->_heTalkQueue[slot].sentence, string, len);

		a->_heTalkQueue[slot].posX = a->_talkPosX;
		a->_heTalkQueue[slot].posY = a->_talkPosY;
		a->_heTalkQueue[slot].color = a->_talkColor;
		}
		break;
	case 83:		// SO_ACTOR_VARIABLE
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case 87:		// SO_ALWAYS_ZCLIP
		a->_forceClip = pop();
		break;
	case 89:		// SO_NEVER_ZCLIP
		a->_forceClip = 0;
		break;
	case 128:
		_actorClipOverride.bottom = pop();
		_actorClipOverride.right = pop();
		_actorClipOverride.top = pop();
		_actorClipOverride.left = pop();
		break;
	case 130:		// SO_SOUND
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->_sound[i] = args[i];
		break;
	case 131:		// SO_ACTOR_WIDTH
		a->_width = pop();
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
		a->_ignoreBoxes = 0;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->_room);
		break;
	case 135:		// SO_IGNORE_BOXES
		a->_ignoreBoxes = 1;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->_room);
		break;
	case 136:		// SO_ACTOR_IGNORE_TURNS_OFF
		a->_ignoreTurns = false;
		break;
	case 137:		// SO_ACTOR_IGNORE_TURNS_ON
		a->_ignoreTurns = true;
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
		a->_talkColor = pop();
		break;
	case 142:
		k = pop();
		a->_heNoTalkAnimation = 1;
		a->setTalkCondition(k);
		debug(1,"o100_actorOps: case 24 (%d)", k);
		break;
	case 143:		// SO_TEXT_OFFSET
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case 144:		// SO_WALK_ANIMATION
		a->_walkFrame = pop();
		break;
	default:
		error("o100_actorOps: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_arrayOps() {
	ArrayHeader *ah;
	byte string[1024];
	int dim1end, dim1start, dim2end, dim2start;
	int id, len, b, c, list[128];
	int offs, tmp, tmp2;
	uint tmp3;

	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	debug(1,"o100_arrayOps: array %d case %d", array, subOp);

	switch (subOp) {
	case 35:
		decodeScriptString(string);
		len = resStrLen(string);
		ah = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(ah->data, string, len);
		break;
	case 77:			// SO_ASSIGN_STRING
		copyScriptString(string, sizeof(string));
		len = resStrLen(string);
		ah = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(ah->data, string, len);
		break;

	case 128:		// SO_ASSIGN_2DIM_LIST
		len = getStackList(list, ARRAYSIZE(list));
		id = readVar(array);
		if (id == 0)
			error("Must DIM a two dimensional array before assigning");
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
			defineArray(array, kDwordArray, 0, 0, 0, b + c - 1);
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
			int a2_dim1end = pop();
			int a2_dim1start = pop();
			int a2_dim2end = pop();
			int a2_dim2start = pop();
			int array2 = fetchScriptWord();
			int a1_dim1end = pop();
			int a1_dim1start = pop();
			int a1_dim2end = pop();
			int a1_dim2start = pop();
			if (a1_dim1end - a1_dim1start != a2_dim1end - a2_dim1start || a2_dim2end - a2_dim2start != a1_dim2end - a1_dim2start) {
				error("Source and dest ranges size are mismatched");
			}
			copyArray(array, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end, array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
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

		offs = (b >= c) ? 1 : -1;
		tmp2 = c;
		tmp3 = c - b + 1;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, tmp2);
				if (--tmp3 == 0) {
					tmp2 = c;
					tmp3 = c - b + 1;
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

void ScummEngine_v100he::o100_jumpToScript() {
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == 128 || flags == 129), (flags == 130 || flags == 129), args);
}

void ScummEngine_v100he::o100_loadSBNG() {
	// Loads SBNG sound resource
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0:
		_heSBNGId = pop();
		break;
	case 53:
		//loadSBNG(_heSBNGId, -1);
		break;
	case 92:
		// dummy case
		break;
	case 128:
		//loadSBNG(_heSBNGId, pop();
		pop();
		break;
	default:
		error("o100_loadSBNG: default case %d", subOp);
	}
	debug(1,"o100_loadSBNG stub (%d)",subOp);
}

void ScummEngine_v100he::o100_dim2dimArray() {
	int data, dim1end, dim2end;

	byte type = fetchScriptByte();
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

	dim1end = pop();
	dim2end = pop();
	defineArray(fetchScriptWord(), data, 0, dim2end, 0, dim1end);
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

void ScummEngine_v100he::o100_drawLine() {
	int id, unk1, unk2, x, x1, y1;

	unk2 = pop();
	id = pop();
	unk1 = pop();
	x = pop();
	y1 = pop();
	x1 = pop();

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 1:
		drawLine(x1, y1, x, unk1, unk2, 2, id);
		break;
	case 20:
		drawLine(x1, y1, x, unk1, unk2, 1, id);
		break;
	case 40:
		drawLine(x1, y1, x, unk1, unk2, 3, id);
		break;
	default:
		error("o100_drawLine: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_drawObject() {
	byte subOp = fetchScriptByte();
	int state, y, x;

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
		y = x = -100;
		break;
	default:
		error("o100_drawObject: default case %d", subOp);
	}

	int object = pop();
	int objnum = getObjectIndex(object);
	if (objnum == -1)
		return;

	if (y != -100 && x != -100) {
		_objs[objnum].x_pos = x * 8;
		_objs[objnum].y_pos = y * 8;
	}

	if (state != -1) {
		addObjectToDrawQue(objnum);
		putState(object, state);
	}
}

void ScummEngine_v100he::o100_setSpriteGroupInfo() {
	byte string[260];
	int type, value1, value2, value3, value4;

	byte subOp = fetchScriptByte();

	debug(1,"o100_setSpriteGroupInfo (%d)", subOp);
	switch (subOp) {
	case 0:
		_curSpriteGroupId = pop();
		break;
	case 6:
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->setGroupPosition(_curSpriteGroupId, value1, value2);
		break;
	case 18:
		value4 = pop();
		value3 = pop();
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->setGroupBounds(_curSpriteGroupId, value1, value2, value3, value4);
		break;
	case 38:
		type = pop() - 1;
		switch (type) {
		case 0:
			value2 = pop();
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->moveGroupMembers(_curSpriteGroupId, value1, value2);
			break;
		case 1:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersPriority(_curSpriteGroupId, value1);
			break;
		case 2:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersGroup(_curSpriteGroupId, value1);
			break;
		case 3:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersUpdateType(_curSpriteGroupId, value1);
			break;
		case 4:
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersResetSprite(_curSpriteGroupId);
			break;
		case 5:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersAnimationSpeed(_curSpriteGroupId, value1);
			break;
		case 6:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersAutoAnimFlag(_curSpriteGroupId, value1);
			break;
		case 7:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersShadow(_curSpriteGroupId, value1);
			break;
		default:
			error("o100_setSpriteGroupInfo subOp 38: Unknown case %d", subOp);
		}
		break;
	case 40:
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->setGroupImage(_curSpriteGroupId, value1);
		break;
	case 49:
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->moveGroup(_curSpriteGroupId, value1, value2);
		break;
	case 52:
		copyScriptString(string, sizeof(string));
		break;
	case 53:
		if (!_curSpriteGroupId)
			break;

		_sprite->resetGroup(_curSpriteGroupId);
		break;
	case 54:
		// dummy case
		pop();
		pop();
		break;
	case 59:
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->setGroupPriority(_curSpriteGroupId, value1);
		break;
	case 60:
		type = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		switch (type) {
		case 0:
			_sprite->setGroupScale_x_ratio_mul(_curSpriteGroupId, value1);
			break;
		case 1:
			_sprite->setGroupScale_x_ratio_div(_curSpriteGroupId, value1);
			break;
		case 2:
			_sprite->setGroupScale_y_ratio_mul(_curSpriteGroupId, value1);
			break;
		case 3:
			_sprite->setGroupScale_y_ratio_div(_curSpriteGroupId, value1);
			break;
		default:
			error("o100_setSpriteGroupInfo subOp 60: Unknown case %d", subOp);
		}
		break;
	case 89:
		if (!_curSpriteGroupId)
			break;

		_sprite->resetGroupBounds(_curSpriteGroupId);
		break;
	default:
		error("o100_setSpriteGroupInfo: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_resourceRoutines() {
	int objidx, room;

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
			room = getObjectRoom(_heResId);
			loadFlObject(_heResId, room);
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
			if (objidx == -1)
				break;
			res.lock(rtFlObject, _objs[objidx].fl_object_index);
		} else {
			res.lock(_heResType, _heResId);
		}
		break;
	case 133:
		if (_heResType == rtCharset)
			nukeCharset(_heResId);
		else
			res.nukeResource(_heResType, _heResId);
		break;
	case 134:
	case 135:
		// Heap related
		break;
	case 136:
		if (_heResType == rtScript && _heResId >= _numGlobalScripts)
			break;

		//queueLoadResource(_heResType, _heResId);
		break;
	case 137:
		if (_heResType == rtScript && _heResId >= _numGlobalScripts)
			break;

		if (_heResType == rtFlObject) {
			objidx = getObjectIndex(_heResId);
			if (objidx == -1)
				break;
			res.unlock(rtFlObject, _objs[objidx].fl_object_index);
		} else {
			res.unlock(_heResType, _heResId);
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
		_wizParams.field_184 = 0;
		_wizParams.field_180 = 0;
		_wizParams.spriteId = 0;
		_wizParams.spriteGroup = 0;
		break;
	case 2:
		_wizParams.processFlags |= kWPFRotate;
		_wizParams.angle = pop();
		break;
	case 6:
	case 132:
		_wizParams.processFlags |= kWPFSetPos;
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		break;
	case 7:
		_wizParams.processFlags |= kWPFMaskImg;
		_wizParams.sourceImage = pop();
		break;
	case 11:
		_wizParams.processFlags |= kWPFClipBox | 0x100;
		_wizParams.processMode = 2;
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		_wizParams.compType = pop();
		break;
	case 18:
		_wizParams.processFlags |= kWPFClipBox;
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		break;
	case 21:
		b = pop();
		a = pop();
		_wizParams.processFlags |= kWPFRemapPalette;
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
		// Dummy case
		pop();
		break;
	case 39:
		_wizParams.processFlags |= kWPFUseDefImgHeight;
		_wizParams.resDefImgH = pop();
		break;
	case 47:
		_wizParams.processFlags |= kWPFUseFile;
		_wizParams.processMode = 3;
		copyScriptString(_wizParams.filename, sizeof(_wizParams.filename));
		break;
	case 53:
		_wizParams.processMode = 8;
		break;
	case 54:
		_wizParams.processFlags |= 0x100000;
		_wizParams.field_180 = pop();
		_wizParams.field_184 = pop();
		break;
	case 55:
		_wizParams.img.flags = pop();
		_wizParams.img.state = pop();
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		_wizParams.spriteId = 0;
		_wizParams.spriteGroup = 0;
		_wizParams.img.resNum = pop();
		_wiz->displayWizImage(&_wizParams.img);
		break;
	case 57:
		_wizParams.processFlags |= kWPFPaletteNum;
		_wizParams.img.palette = pop();
		break;
	case 58:
		_wizParams.processFlags |= 0x1000 | 0x100 | 0x2;
		_wizParams.processMode = 7;
		_wizParams.field_168 = pop();
		_wizParams.field_164 = pop();
		_wizParams.compType = pop();
		break;
	case 64:
		_wizParams.processFlags |= kWPFUseFile;
		_wizParams.processMode = 4;
		copyScriptString(_wizParams.filename, sizeof(_wizParams.filename));
		_wizParams.fileWriteMode = pop();
		break;
	case 65:
		_wizParams.processFlags |= kWPFScaled;
		_wizParams.scale = pop();
		break;
	case 67:
		_wizParams.processFlags |= kWPFNewFlags;
		_wizParams.img.flags |= pop();
		break;
	case 68:
		_wizParams.processFlags |= kWPFNewFlags | kWPFSetPos | 2;
		_wizParams.img.flags |= kWIFIsPolygon;
		_wizParams.field_164 = _wizParams.img.y1 = _wizParams.img.x1 = pop();
		break;
	case 70:
		_wizParams.processFlags |= kWPFShadow;
		_wizParams.img.shadow = pop();
		break;
	case 73:
		_wizParams.processFlags |= kWPFNewState;
		_wizParams.img.state = pop();
		break;
	case 84:
		_wizParams.processFlags |= kWPFUseDefImgWidth;
		_wizParams.resDefImgW = pop();
		break;
	case 92:
		if (_wizParams.img.resNum)
			_wiz->processWizImage(&_wizParams);
		break;
	case 128:
		_wizParams.field_239D = pop();
		_wizParams.field_2399 = pop();
		_wizParams.field_23A5 = pop();
		_wizParams.field_23A1 = pop();
		copyScriptString(_wizParams.string2, sizeof(_wizParams.string2));
		_wizParams.processMode = 15;
		break;
	case 129:
		_wizParams.processMode = 14;
		break;
	case 130:
		_wizParams.processMode = 16;
		_wizParams.field_23AD = pop();
		_wizParams.field_23A9 = pop();
		copyScriptString(_wizParams.string1, sizeof(_wizParams.string1));
		break;
	case 131:
		_wizParams.processMode = 13;
		break;
	case 133:
		_wizParams.processMode = 17;
		_wizParams.field_23CD = pop();
		_wizParams.field_23C9 = pop();
		_wizParams.field_23C5 = pop();
		_wizParams.field_23C1 = pop();
		_wizParams.field_23BD = pop();
		_wizParams.field_23B9 = pop();
		_wizParams.field_23B5 = pop();
		_wizParams.field_23B1 = pop();
		break;
	case 134:
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 12;
		_wizParams.fillColor = pop();
		_wizParams.box2.top = _wizParams.box2.bottom = pop();
		_wizParams.box2.left = _wizParams.box2.right = pop();
		break;
	case 135:
		_wizParams.processFlags |= kWPFDstResNum;
		_wizParams.dstResNum = pop();
		break;
	case 136:
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 10;
		_wizParams.fillColor = pop();
		_wizParams.box2.bottom = pop();
		_wizParams.box2.right = pop();
		_wizParams.box2.top = pop();
		_wizParams.box2.left = pop();
		break;
	case 137:
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 11;
		_wizParams.fillColor = pop();
		_wizParams.box2.top = _wizParams.box2.bottom = pop();
		_wizParams.box2.left = _wizParams.box2.right = pop();
		break;
	case 138:
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 9;
		_wizParams.fillColor = pop();
		_wizParams.box2.bottom = pop();
		_wizParams.box2.right = pop();
		_wizParams.box2.top = pop();
		_wizParams.box2.left = pop();
		break;
	default:
		error("o100_wizImageOps: Unknown case %d", subOp);
	}
	debug(1,"o100_wizImageOps (%d)", subOp);
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

void ScummEngine_v100he::o100_redim2dimArray() {
	int a, b, c, d;
	d = pop();
	c = pop();
	b = pop();
	a = pop();

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 42:
		redimArray(fetchScriptWord(), a, b, c, d, kIntArray);
		break;
	case 43:
		redimArray(fetchScriptWord(), a, b, c, d, kDwordArray);
		break;
	case 45:
		redimArray(fetchScriptWord(), a, b, c, d, kByteArray);
		break;
	default:
		error("o100_redim2dimArray: default type %d", subOp);
	}
}

void ScummEngine_v100he::o100_paletteOps() {
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0:
		_hePaletteNum = pop();
		break;
	case 20:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		if (_hePaletteNum != 0) {
			for (; a <= b; ++a) {
				setHEPaletteColor(_hePaletteNum, a, c, d, e);
			}
		}
		break;
	case 25:
		a = pop();
		if (_hePaletteNum != 0) {
			setHEPaletteFromCostume(_hePaletteNum, a);
		}
		break;
	case 40:
		b = pop();
		a = pop();
		if (_hePaletteNum != 0) {
			setHEPaletteFromImage(_hePaletteNum, a, b);
		}
		break;
	case 53:
		if (_hePaletteNum != 0) {
			restoreHEPalette(_hePaletteNum);
		}
		break;
	case 57:
		a = pop();
		if (_hePaletteNum != 0) {
			copyHEPalette(_hePaletteNum, a);
		}
		break;
	case 63:
		b = pop();
		a = pop();
		if (_hePaletteNum != 0) {
			setHEPaletteFromRoom(_hePaletteNum, a, b);
		}
		break;
	case 81:
		c = pop();
		b = pop();
		a = pop();
		if (_hePaletteNum) {
			for (; a <= b; ++a) {
				copyHEPaletteColor(_hePaletteNum, a, c);
			}
		}
		break;
	case 92:
		_hePaletteNum = 0;
		break;
	default:
		error("o100_paletteOps: Unknown case %d", subOp);
	}
	debug(1, "o100_paletteOps (%d)", subOp);
}

void ScummEngine_v100he::o100_jumpToScriptUnk() {
	int args[25];
	int script, cycle;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	cycle = pop();
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == 128 || flags == 129), (flags == 130 || flags == 129), args, cycle);
}

void ScummEngine_v100he::o100_startScriptUnk() {
	int args[25];
	int script, cycle;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	cycle = pop();
	script = pop();
	flags = fetchScriptByte();
	runScript(script, (flags == 128 || flags == 129), (flags == 130 || flags == 129), args, cycle);
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
		setPalette(a);
		break;

	case 135:
		b = pop();
		a = pop();
		setRoomPalette(a, b);
		break;

	case 136:		// SO_ROOM_SAVEGAME
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		break;

	case 137:
		copyScriptString(filename, sizeof(filename));
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
	byte filename[260];
	int var, value;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 6:
		_heSndFlags |= 16;
		_heSndOffset = pop();
		break;
	case 47:
		copyScriptString(filename, sizeof(filename));
		_heSndSoundId = pop();
		if (_heSndSoundId)
			debug(0, "Load sound %d from file %s\n", _heSndSoundId, filename);
		break;
	case 55:
		_heSndFlags |= 8;
		break;
	case 83:
		value = pop();
		var = pop();
		_heSndSoundId = pop();
		debug(0,"o100_startSound: case 29 (snd %d, var %d, value %d)", _heSndSoundId, var, value);
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
	case 132: // Music
	case 134: // Sound
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

void ScummEngine_v100he::o100_setSpriteInfo() {
	int args[16];
	int spriteId, n;
	int32 tmp[2];
	byte string[80];

	byte subOp = fetchScriptByte();

	debug(1,"o100_setSpriteInfo (%d)", subOp);
	switch (subOp) {
	case 0:
		_curMaxSpriteId = pop();
		_curSpriteId = pop();

		if (_curSpriteId > _curMaxSpriteId)
			SWAP(_curSpriteId, _curMaxSpriteId);
		break;
	case 2:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteAngle(spriteId, args[0]);
		break;
	case 3:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteFlagAutoAnim(spriteId, args[0]);
		break;
	case 4:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteAnimSpeed(spriteId, args[0]);
		break;
	case 6:
		args[1] = pop();
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpritePosition(spriteId, args[0], args[1]);
		break;
	case 7:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteSourceImage(spriteId, args[0]);
		break;
	case 16:
		n = getStackList(args, ARRAYSIZE(args));
		if (_curSpriteId != 0 && _curMaxSpriteId != 0 && n != 0) {
			int *p = &args[n - 1];
			do {
				int code = *p;
				if (code == 0) {
					for (int i = _curSpriteId; i <= _curMaxSpriteId; ++i) {
						_sprite->setSpriteResetClass(i);
					}
				} else if (code & 0x80) {
					for (int i = _curSpriteId; i <= _curMaxSpriteId; ++i) {
						_sprite->setSpriteSetClass(i, code & 0x7F, 1);
					}
				} else {
					for (int i = _curSpriteId; i <= _curMaxSpriteId; ++i) {
						_sprite->setSpriteSetClass(i, code & 0x7F, 0);
					}
				}
				--p;
			} while (--n);
		}
		break;
	case 32:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteFlagEraseType(spriteId, args[0]);
		break;
	case 38:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteGroup(spriteId, args[0]);
		break;
	case 40:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteImage(spriteId, args[0]);
		break;
	case 48:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteMaskImage(spriteId, args[0]);
		break;
	case 49:
		args[1] = pop();
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->moveSprite(spriteId, args[0], args[1]);
		break;
	case 52:
		copyScriptString(string, sizeof(string));
		break;
	case 53:
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->resetSprite(spriteId);
		break;
	case 54:
		args[1] = pop();
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteGeneralProperty(spriteId, args[0], args[1]);
		break;
	case 57:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpritePalette(spriteId, args[0]);
		break;
	case 59:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpritePriority(spriteId, args[0]);
		break;
	case 60:
		args[1] = pop();
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			switch(args[1]) {
			case 0:
				_sprite->setSpriteFlagXFlipped(spriteId, args[0]);
				break;
			case 1:
				_sprite->setSpriteFlagYFlipped(spriteId, args[0]);
				break;
			case 2:
				_sprite->setSpriteFlagActive(spriteId, args[0]);
				break;
			case 3:
				_sprite->setSpriteFlagDoubleBuffered(spriteId, args[0]);
				break;
			case 4:
				_sprite->setSpriteFlagRemapPalette(spriteId, args[0]);
				break;
			default:
				break;
			}
		break;
	case 61:
		_sprite->resetTables(true);
		break;
	case 65:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteScale(spriteId, args[0]);
		break;
	case 70:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteShadow(spriteId, args[0]);
		break;
	case 73:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteImageState(spriteId, args[0]);
		break;
	case 74:
		args[1] = pop();
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteDist(spriteId, args[0], args[1]);
		break;
	case 75:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++) {
			_sprite->getSpriteDist(spriteId, tmp[0], tmp[1]);
			_sprite->setSpriteDist(spriteId, args[0], tmp[1]);
		}
		break;
	case 76:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++) {
			_sprite->getSpriteDist(spriteId, tmp[0], tmp[1]);
			_sprite->setSpriteDist(spriteId, tmp[0], args[0]);
		}
		break;
	case 82:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteFlagUpdateType(spriteId, args[0]);
		break;
	case 83:
		args[1] = pop();
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteUserValue(spriteId, args[0], args[1]);
		break;
	case 88:
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteField84(spriteId, args[0]);
		break;
	case 89:
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteField84(spriteId, 0);
		break;
	default:
		error("o100_setSpriteInfo: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_startScript() {
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	runScript(script, (flags == 128 || flags == 129), (flags == 130 || flags == 129), args);
}

void ScummEngine_v100he::o100_systemOps() {
	byte string[1024];

	byte subOp = fetchScriptByte();
	subOp -= 61;

	switch (subOp) {
	case 0:
		restart();
		break;
	case 67:
		clearDrawObjectQueue();
		break;
	case 71:
		// Confirm shutdown
		shutDown();
		break;
	case 72:
		shutDown();
		break;
	case 73:
		copyScriptString(string, sizeof(string));
		debug(0, "Start game (%s)", string);
		break;
	case 74:
		copyScriptString(string, sizeof(string));
		debug(0, "Start executable (%s)", string);
		break;
	case 75:
		gdi.copyVirtScreenBuffers(Common::Rect(_screenWidth, _screenHeight));
		updatePalette();
		break;
	default:
		error("o100_systemOps invalid case %d", subOp);
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
	case 0x81:
		a = pop();
		_wiz->loadWizCursor(a);
		break;
	case 0x82:
		pop();
		a = pop();
		_wiz->loadWizCursor(a);
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
		error("o100_cursorCommand: default case %x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void ScummEngine_v100he::o100_videoOps() {
	// Uses Bink video
	int subOp = fetchScriptByte();

	switch (subOp) {
	case 0:
		memset(_videoParams.filename, 0, sizeof(_videoParams.filename));
		_videoParams.unk2 = pop();
		break;
	case 19:
		_videoParams.status = 19;
		break;
	case 40:
		_videoParams.wizResNum = pop();
		if (_videoParams.wizResNum)
			_videoParams.flags |= 2;
		break;
	case 47:
		copyScriptString(_videoParams.filename, sizeof(_videoParams.filename));
		_videoParams.status = 47;
		break;
	case 67:
		_videoParams.flags |= pop();
		break;
	case 92:
		if (_videoParams.status == 47) {
			// Start video
			if (_videoParams.flags == 0)
				_videoParams.flags = 4;

			if (_videoParams.flags == 2) {
				// result = startVideo(_videoParams.filename, _videoParams.flags, _videoParams.wizResNum);
				// VAR(119) = result;
			} else {
				// result = startVideo(_videoParams.filename, _videoParams.flags);
				// VAR(119) = result;
			}
		} else if (_videoParams.status == 19) {
			// Stop video
		}
		break;
	default:
		error("o100_videoOps: unhandled case %d", subOp);
	}

	debug(0,"o100_videoOps stub (%d)", subOp);
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
		if (a->_moving)
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

void ScummEngine_v100he::o100_isResourceLoaded() {
	// Reports percentage of resource loaded by queue
	int type;

	byte subOp = fetchScriptByte();
	/* int idx = */ pop();

	switch (subOp) {
	case 25:
		type = rtCostume;
		break;
	case 40:
		type = rtImage;
		break;
	case 62:
		type = rtRoom;
		break;
	case 66:
		type = rtScript;
		break;
	case 72:
		type = rtSound;
		break;
	default:
		error("o100_isResourceLoaded: default case %d", subOp);
	}

	push(100);
}

void ScummEngine_v100he::o100_getResourceSize() {
	const byte *ptr;
	int size, type;

	int resid = pop();
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
		push (getSoundResourceSize(resid));
		return;
	default:
		error("o100_getResourceSize: default type %d", subOp);
	}

	ptr = getResourceAddress(type, resid);
	assert(ptr);
	size = READ_BE_UINT32(ptr + 4) - 8;
	push(size);
}

void ScummEngine_v100he::o100_getSpriteGroupInfo() {
	int32 tx, ty;
	int spriteGroupId, type;

	byte subOp = fetchScriptByte();

	debug(1,"o100_getSpriteGroupInfo (%d)", subOp);
	switch (subOp) {
	case 5:
		spriteGroupId = pop();
		if (spriteGroupId)
			push(getGroupSpriteArray(spriteGroupId));
		else
			push(0);
		break;
	case 40:
		spriteGroupId = pop();
		if (spriteGroupId)
			push(_sprite->getGroupDstResNum(spriteGroupId));
		else
			push(0);
		break;
	case 54:
		// TODO: U32 related
		pop();
		pop();
		push(0);
		break;
	case 59:
		spriteGroupId = pop();
		if (spriteGroupId)
			push(_sprite->getGroupPriority(spriteGroupId));
		else
			push(0);
		break;
	case 60:
		type = pop();
		spriteGroupId = pop();
		if (spriteGroupId) {
			switch(type) {
			case 0:
				push(_sprite->getGroupScale_x_ratio_mul(spriteGroupId));
				break;
			case 1:
				push(_sprite->getGroupScale_x_ratio_div(spriteGroupId));
				break;
			case 2:
				push(_sprite->getGroupScale_y_ratio_mul(spriteGroupId));
				break;
			case 3:
				push(_sprite->getGroupScale_y_ratio_div(spriteGroupId));
				break;
			default:
				push(0);
			}
		} else {
			push(0);
		}
		break;
	case 85:
		spriteGroupId = pop();
		if (spriteGroupId) {
			_sprite->getGrouptx_ty(spriteGroupId, tx, ty);
			push(tx);
		} else {
			push(0);
		}
		break;
	case 86:
		spriteGroupId = pop();
		if (spriteGroupId) {
			_sprite->getGrouptx_ty(spriteGroupId, tx, ty);
			push(ty);
		} else {
			push(0);
		}
		break;
	default:
		error("o100_getSpriteGroupInfo: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_getWizData() {
	byte filename[4096];
	int resId, state, type;
	int32 w, h;
	int16 x, y;

	byte subOp = fetchScriptByte();
	subOp -= 20;

	switch (subOp) {
	case 0:
		y = pop();
		x = pop();
		state = pop();
		resId = pop();
		push(_wiz->getWizPixelColor(resId, state, x, y, 0));
		break;
	case 6:
		resId = pop();
		push(_wiz->getWizImageStates(resId));
		break;
	case 13:
		y = pop();
		x = pop();
		state = pop();
		resId = pop();
		push(_wiz->isWizPixelNonTransparent(resId, state, x, y, 0));
		break;
	case 19:
		state = pop();
		resId = pop();
		_wiz->getWizImageDim(resId, state, w, h);
		push(h);
		break;
	case 34:
		type = pop();
		state = pop();
		resId = pop();
		push(_wiz->getWizImageData(resId, state, type));
		break;
	case 64:
		state = pop();
		resId = pop();
		_wiz->getWizImageDim(resId, state, w, h);
		push(w);
		break;
	case 65:
		state = pop();
		resId = pop();
		_wiz->loadImgSpot(resId, state, x, y);
		push(x);
		break;
	case 66:
		state = pop();
		resId = pop();
		_wiz->loadImgSpot(resId, state, x, y);
		push(y);
		break;
	case 111:
		pop();
		copyScriptString(filename, sizeof(filename));
		pop();
		push(0);
		debug(0, "o100_getWizData() case 111 unhandled");
		break;
	case 112:
		h = pop();
		w = pop();
		y = pop();
		x = pop();
		state = pop();
		resId = pop();
		if (x == -1 && y == -1 && w == -1 && h == -1) {
			_wiz->getWizImageDim(resId, state, w, h);
			x = 0;
			y = 0;
		}
		push(computeWizHistogram(resId, state, x, y, w, h));
		break;
	default:
		error("o100_getWizData: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_getPaletteData() {
	int b, c, d, e;
	int palSlot, color;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 13:
		c = pop();
		b = pop();
		push(getHEPaletteColorComponent(1, b, c));
		break;
	case 20:
		color = pop();
		palSlot = pop();
		push(getHEPaletteColor(palSlot, color));
		break;
	case 33:
		e = pop();
		d = pop();
		palSlot = pop();
		pop();
		c = pop();
		b = pop();
		push(getHEPaletteSimilarColor(palSlot, b, c, d, e));
		break;
	case 53:
		pop();
		c = pop();
		c = MAX(0, c);
		c = MIN(c, 255);
		b = pop();
		b = MAX(0, b);
		b = MIN(b, 255);
		push(getHEPaletteSimilarColor(1, b, c, 10, 245));
		break;
	case 73:
		c = pop();
		b = pop();
		palSlot = pop();
		push(getHEPaletteColorComponent(palSlot, b, c));
		break;
	default:
		error("o100_getPaletteData: Unknown case %d", subOp);
	}
	debug(0, "o100_getPaletteData stub (%d)", subOp);
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

void ScummEngine_v100he::o100_getSpriteInfo() {
	int args[16];
	int spriteId, flags, groupId, type;
	int32 x, y;

	byte subOp = fetchScriptByte();

	debug(1,"o100_getSpriteInfo (%d)", subOp);
	switch (subOp) {
	case 3:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteFlagAutoAnim(spriteId));
		else
			push(0);
		break;
	case 4:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteAnimSpeed(spriteId));
		else
			push(1);
		break;
	case 7:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteSourceImage(spriteId));
		else
			push(0);
		break;
	case 16:
		flags = getStackList(args, ARRAYSIZE(args));
		spriteId = pop();
		if (spriteId) {
			push(_sprite->getSpriteClass(spriteId, flags, args));
		} else {
			push(0);
		}
		break;
	case 26:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteImageStateCount(spriteId));
		else
			push(0);
		break;
	case 30:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteDisplayX(spriteId));
		else
			push(0);
		break;
	case 31:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteDisplayY(spriteId));
		else
			push(0);
		break;
	case 32:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteFlagEraseType(spriteId));
		else
			push(1);
		break;
	case 33:
		flags = getStackList(args, ARRAYSIZE(args));
		type = pop();
		groupId = pop();
		y = pop();
		x = pop();
		push(_sprite->findSpriteWithClassOf(x, y, groupId, type, flags, args));
		break;
	case 38:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteGroup(spriteId));
		else
			push(0);
		break;
	case 39:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpriteImageDim(spriteId, x, y);
			push(y);
		} else {
			push(0);
		}
		break;
	case 40:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteImage(spriteId));
		else
			push(0);
		break;
	case 48:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteMaskImage(spriteId));
		else
			push(0);
		break;
	case 54:
		flags = pop();
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteGeneralProperty(spriteId, flags));
		else
			push(0);
		break;
	case 57:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpritePalette(spriteId));
		else
			push(0);
		break;
	case 59:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpritePriority(spriteId));
		else
			push(0);
		break;
	case 60:
		flags = pop();
		spriteId = pop();
		if (spriteId) {
			switch(flags) {
			case 0:
				push(_sprite->getSpriteFlagXFlipped(spriteId));
				break;
			case 1:
				push(_sprite->getSpriteFlagYFlipped(spriteId));
				break;
			case 2:
				push(_sprite->getSpriteFlagActive(spriteId));
				break;
			case 3:
				push(_sprite->getSpriteFlagDoubleBuffered(spriteId));
				break;
			case 4:
				push(_sprite->getSpriteFlagRemapPalette(spriteId));
				break;
			default:
				push(0);
			}
		} else {
			push(0);
		}
		break;
	case 65:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteScale(spriteId));
		else
			push(0);
		break;
	case 70:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteShadow(spriteId));
		else
			push(0);
		break;
	case 73:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteImageState(spriteId));
		else
			push(0);
		break;
	case 75:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpriteDist(spriteId, x, y);
			push(x);
		} else {
			push(0);
		}
		break;
	case 76:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpriteDist(spriteId, x, y);
			push(y);
		} else {
			push(0);
		}
		break;
	case 82:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteFlagUpdateType(spriteId));
		else
			push(0);
		break;
	case 83:
		pop();
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteUserValue(spriteId));
		else
			push(0);
		break;
	case 84:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpriteImageDim(spriteId, x, y);
			push(x);
		} else {
			push(0);
		}
		break;
	case 85:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpritePosition(spriteId, x, y);
			push(x);
		} else {
			push(0);
		}
		break;
	case 86:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpritePosition(spriteId, x, y);
			push(y);
		} else {
			push(0);
		}
		break;
	default:
		error("o100_getSpriteInfo: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_getVideoData() {
	// Uses Bink video
	int subOp = fetchScriptByte();
	subOp -= 26;

	switch (subOp) {
	case 0:
		pop();
		break;
	case 13:
		pop();
		break;
	case 14:
		pop();
		break;
	case 28:
		pop();
		pop();
		break;
	case 47:
		pop();
		break;
	case 58:
		pop();
		break;
	default:
		error("o100_getVideoData: unhandled case %d", subOp);
	}

	push(-1);
	debug(0,"o100_getVideoData stub (%d)", subOp);
}

void ScummEngine_v100he::decodeParseString(int m, int n) {
	Actor *a;
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
		byte *dataPtr = getResourceAddress(rtTalkie, pop());
		byte *text = findWrappedBlock(MKID('TEXT'), dataPtr, 0, 0);
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
		if (n) {
			_actorToPrintStrFor = pop();
			if (_actorToPrintStrFor != 0xFF) {
				a = derefActor(_actorToPrintStrFor, "decodeParseString");
				_string[0].color = a->_talkColor;
			}
		}
		break;
	case 92:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case %d", b);
	}
}

} // End of namespace Scumm
