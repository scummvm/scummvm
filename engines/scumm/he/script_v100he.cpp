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

#ifdef ENABLE_HE

#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/he/animation_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/he/sprite_he.h"
#include "scumm/util.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v100he, x)

void ScummEngine_v100he::setupOpcodes() {
	/* 00 */
	OPCODE(0x00, o100_actorOps);
	OPCODE(0x01, o6_add);
	OPCODE(0x02, o6_faceActor);
	OPCODE(0x03, o90_sortArray);
	/* 04 */
	OPCODE(0x04, o100_arrayOps);
	OPCODE(0x05, o6_band);
	OPCODE(0x06, o6_bor);
	OPCODE(0x07, o6_breakHere);
	/* 08 */
	OPCODE(0x08, o6_delayFrames);
	OPCODE(0x09, o90_shl);
	OPCODE(0x0a, o90_shr);
	OPCODE(0x0b, o90_xor);
	/* 0C */
	OPCODE(0x0c, o6_setCameraAt);
	OPCODE(0x0d, o6_actorFollowCamera);
	OPCODE(0x0e, o6_loadRoom);
	OPCODE(0x0f, o6_panCameraTo);
	/* 10 */
	OPCODE(0x10, o72_captureWizImage);
	OPCODE(0x11, o100_jumpToScript);
	OPCODE(0x12, o6_setClass);
	OPCODE(0x13, o60_closeFile);
	/* 14 */
	OPCODE(0x14, o6_loadRoomWithEgo);
	OPCODE(0x16, o72_createDirectory);
	OPCODE(0x17, o100_createSound);
	/* 18 */
	OPCODE(0x18, o6_cutscene);
	OPCODE(0x19, o6_pop);
	OPCODE(0x1a, o72_traceStatus);
	OPCODE(0x1b, o6_wordVarDec);
	/* 1C */
	OPCODE(0x1c, o6_wordArrayDec);
	OPCODE(0x1d, o72_deleteFile);
	OPCODE(0x1e, o100_dim2dimArray);
	OPCODE(0x1f, o100_dimArray);
	/* 20 */
	OPCODE(0x20, o6_div);
	OPCODE(0x21, o6_animateActor);
	OPCODE(0x22, o6_doSentence);
	OPCODE(0x23, o6_drawBox);
	/* 24 */
	OPCODE(0x24, o72_drawWizImage);
	OPCODE(0x25, o80_drawWizPolygon);
	OPCODE(0x26, o100_drawLine);
	OPCODE(0x27, o100_drawObject);
	/* 28 */
	OPCODE(0x28, o6_dup);
	OPCODE(0x29, o90_dup_n);
	OPCODE(0x2a, o6_endCutscene);
	OPCODE(0x2b, o6_stopObjectCode);
	/* 2C */
	OPCODE(0x2c, o6_stopObjectCode);
	OPCODE(0x2d, o6_eq);
	OPCODE(0x2e, o100_floodFill);
	OPCODE(0x2f, o6_freezeUnfreeze);
	/* 30 */
	OPCODE(0x30, o6_ge);
	OPCODE(0x31, o6_getDateTime);
	OPCODE(0x32, o100_setSpriteGroupInfo);
	OPCODE(0x33, o6_gt);
	/* 34 */
	OPCODE(0x34, o100_resourceRoutines);
	OPCODE(0x35, o6_if);
	OPCODE(0x36, o6_ifNot);
	OPCODE(0x37, o100_wizImageOps);
	/* 38 */
	OPCODE(0x38, o72_isAnyOf);
	OPCODE(0x39, o6_wordVarInc);
	OPCODE(0x3a, o6_wordArrayInc);
	OPCODE(0x3b, o6_jump);
	/* 3C */
	OPCODE(0x3c, o90_kernelSetFunctions);
	OPCODE(0x3d, o6_land);
	OPCODE(0x3e, o6_le);
	OPCODE(0x3f, o60_localizeArrayToScript);
	/* 40 */
	OPCODE(0x40, o6_wordArrayRead);
	OPCODE(0x41, o6_wordArrayIndexedRead);
	OPCODE(0x42, o6_lor);
	OPCODE(0x43, o6_lt);
	/* 44 */
	OPCODE(0x44, o90_mod);
	OPCODE(0x45, o6_mul);
	OPCODE(0x46, o6_neq);
	OPCODE(0x47, o100_dim2dim2Array);
	/* 48 */
	OPCODE(0x49, o100_redim2dimArray);
	OPCODE(0x4a, o6_not);
	/* 4C */
	OPCODE(0x4c, o6_beginOverride);
	OPCODE(0x4d, o6_endOverride);
	OPCODE(0x4e, o72_resetCutscene);
	OPCODE(0x4f, o6_setOwner);
	/* 50 */
	OPCODE(0x50, o100_paletteOps);
	OPCODE(0x51, o70_pickupObject);
	OPCODE(0x52, o71_polygonOps);
	OPCODE(0x53, o6_pop);
	/* 54 */
	OPCODE(0x54, o6_printDebug);
	OPCODE(0x55, o72_printWizImage);
	OPCODE(0x56, o6_printLine);
	OPCODE(0x57, o6_printSystem);
	/* 58 */
	OPCODE(0x58, o6_printText);
	OPCODE(0x59, o100_jumpToScriptUnk);
	OPCODE(0x5a, o100_startScriptUnk);
	OPCODE(0x5b, o6_pseudoRoom);
	/* 5C */
	OPCODE(0x5c, o6_pushByte);
	OPCODE(0x5d, o72_pushDWord);
	OPCODE(0x5e, o72_getScriptString);
	OPCODE(0x5f, o6_pushWord);
	/* 60 */
	OPCODE(0x60, o6_pushWordVar);
	OPCODE(0x61, o6_putActorAtObject);
	OPCODE(0x62, o6_putActorAtXY);
	/* 64 */
	OPCODE(0x64, o100_redimArray);
	OPCODE(0x65, o72_rename);
	OPCODE(0x66, o6_stopObjectCode);
	OPCODE(0x67, o80_localizeArrayToRoom);
	/* 68 */
	OPCODE(0x68, o100_roomOps);
	OPCODE(0x69, o6_printActor);
	OPCODE(0x6a, o6_printEgo);
	OPCODE(0x6b, o72_talkActor);
	/* 6C */
	OPCODE(0x6c, o72_talkEgo);
	OPCODE(0x6e, o60_seekFilePos);
	OPCODE(0x6f, o6_setBoxFlags);
	/* 70 */
	OPCODE(0x71, o6_setBoxSet);
	OPCODE(0x72, o100_setSystemMessage);
	OPCODE(0x73, o6_shuffle);
	/* 74 */
	OPCODE(0x74, o6_delay);
	OPCODE(0x75, o6_delayMinutes);
	OPCODE(0x76, o6_delaySeconds);
	OPCODE(0x77, o100_soundOps);
	/* 78 */
	OPCODE(0x78, o80_sourceDebug);
	OPCODE(0x79, o100_setSpriteInfo);
	OPCODE(0x7a, o6_stampObject);
	OPCODE(0x7b, o72_startObject);
	/* 7C */
	OPCODE(0x7c, o100_startScript);
	OPCODE(0x7d, o6_startScriptQuick);
	OPCODE(0x7e, o80_setState);
	OPCODE(0x7f, o6_stopObjectScript);
	/* 80 */
	OPCODE(0x80, o6_stopScript);
	OPCODE(0x81, o6_stopSentence);
	OPCODE(0x82, o6_stopSound);
	OPCODE(0x83, o6_stopTalking);
	/* 84 */
	OPCODE(0x84, o6_writeWordVar);
	OPCODE(0x85, o6_wordArrayWrite);
	OPCODE(0x86, o6_wordArrayIndexedWrite);
	OPCODE(0x87, o6_sub);
	/* 88 */
	OPCODE(0x88, o100_systemOps);
	OPCODE(0x8a, o72_setTimer);
	OPCODE(0x8b, o100_cursorCommand);
	/* 8C */
	OPCODE(0x8c, o100_videoOps);
	OPCODE(0x8d, o100_wait);
	OPCODE(0x8e, o6_walkActorToObj);
	OPCODE(0x8f, o6_walkActorTo);
	/* 90 */
	OPCODE(0x90, o100_writeFile);
	OPCODE(0x91, o72_writeINI);
	OPCODE(0x92, o80_writeConfigFile);
	OPCODE(0x93, o6_abs);
	/* 94 */
	OPCODE(0x94, o6_getActorWalkBox);
	OPCODE(0x95, o6_getActorCostume);
	OPCODE(0x96, o6_getActorElevation);
	OPCODE(0x97, o6_getObjectOldDir);
	/* 98 */
	OPCODE(0x98, o6_getActorMoving);
	OPCODE(0x99, o90_getActorData);
	OPCODE(0x9a, o6_getActorRoom);
	OPCODE(0x9b, o6_getActorScaleX);
	/* 9C */
	OPCODE(0x9c, o6_getAnimateVariable);
	OPCODE(0x9d, o6_getActorWidth);
	OPCODE(0x9e, o6_getObjectX);
	OPCODE(0x9f, o6_getObjectY);
	/* A0 */
	OPCODE(0xa0, o90_atan2);
	OPCODE(0xa1, o90_getSegmentAngle);
	OPCODE(0xa2, o90_getActorAnimProgress);
	OPCODE(0xa3, o90_getDistanceBetweenPoints);
	/* A4 */
	OPCODE(0xa4, o6_ifClassOfIs);
	OPCODE(0xa6, o90_cond);
	OPCODE(0xa7, o90_cos);
	/* A8 */
	OPCODE(0xa8, o100_debugInput);
	OPCODE(0xa9, o80_getFileSize);
	OPCODE(0xaa, o6_getActorFromXY);
	OPCODE(0xab, o72_findAllObjects);
	/* AC */
	OPCODE(0xac, o90_findAllObjectsWithClassOf);
	OPCODE(0xad, o71_findBox);
	OPCODE(0xae, o6_findInventory);
	OPCODE(0xaf, o72_findObject);
	/* B0 */
	OPCODE(0xb0, o72_findObjectWithClassOf);
	OPCODE(0xb1, o71_polygonHit);
	OPCODE(0xb2, o90_getLinesIntersectionPoint);
	OPCODE(0xb3, o90_fontUnk);
	/* B4 */
	OPCODE(0xb4, o72_getNumFreeArrays);
	OPCODE(0xb5, o72_getArrayDimSize);
	OPCODE(0xb6, o100_isResourceLoaded);
	OPCODE(0xb7, o100_getResourceSize);
	/* B8 */
	OPCODE(0xb8, o100_getSpriteGroupInfo);
	OPCODE(0xba, o100_getWizData);
	OPCODE(0xbb, o6_isActorInBox);
	/* BC */
	OPCODE(0xbc, o6_isAnyOf);
	OPCODE(0xbd, o6_getInventoryCount);
	OPCODE(0xbe, o90_kernelGetFunctions);
	OPCODE(0xbf, o90_max);
	/* C0 */
	OPCODE(0xc0, o90_min);
	OPCODE(0xc1, o72_getObjectImageX);
	OPCODE(0xc2, o72_getObjectImageY);
	OPCODE(0xc3, o6_isRoomScriptRunning);
	/* C4 */
	OPCODE(0xc4, o90_getObjectData);
	OPCODE(0xc5, o72_openFile);
	OPCODE(0xc6, o90_getPolygonOverlap);
	OPCODE(0xc7, o6_getOwner);
	/* C8 */
	OPCODE(0xc8, o100_getPaletteData);
	OPCODE(0xc9, o6_pickOneOf);
	OPCODE(0xca, o6_pickOneOfDefault);
	OPCODE(0xcb, o80_pickVarRandom);
	/* CC */
	OPCODE(0xcc, o72_getPixel);
	OPCODE(0xcd, o6_distObjectObject);
	OPCODE(0xce, o6_distObjectPt);
	OPCODE(0xcf, o6_distPtPt);
	/* D0 */
	OPCODE(0xd0, o6_getRandomNumber);
	OPCODE(0xd1, o6_getRandomNumberRange);
	OPCODE(0xd3, o100_readFile);
	/* D4 */
	OPCODE(0xd4, o72_readINI);
	OPCODE(0xd5, o80_readConfigFile);
	OPCODE(0xd6, o6_isScriptRunning);
	OPCODE(0xd7, o90_sin);
	/* D8 */
	OPCODE(0xd8, o72_getSoundPosition);
	OPCODE(0xd9, o6_isSoundRunning);
	OPCODE(0xda, o80_getSoundVar);
	OPCODE(0xdb, o100_getSpriteInfo);
	/* DC */
	OPCODE(0xdc, o90_sqrt);
	OPCODE(0xdd, o6_startObjectQuick);
	OPCODE(0xde, o6_startScriptQuick2);
	OPCODE(0xdf, o6_getState);
	/* E0 */
	OPCODE(0xe0, o71_compareString);
	OPCODE(0xe1, o71_copyString);
	OPCODE(0xe2, o71_appendString);
	OPCODE(0xe3, o71_concatString);
	/* E4 */
	OPCODE(0xe4, o70_getStringLen);
	OPCODE(0xe5, o71_getStringLenForWidth);
	OPCODE(0xe6, o80_stringToInt);
	OPCODE(0xe7, o71_getCharIndexInString);
	/* E8 */
	OPCODE(0xe8, o71_getStringWidth);
	OPCODE(0xe9, o60_readFilePos);
	OPCODE(0xea, o72_getTimer);
	OPCODE(0xeb, o6_getVerbEntrypoint);
	/* EC */
	OPCODE(0xec, o100_getVideoData);
	/* F0 */
	/* F4 */
	/* F8 */
	/* FC */
}

void ScummEngine_v100he::o100_actorOps() {
	ActorHE *a;
	int i, j, k;
	int args[32];
	byte string[256];

	byte subOp = fetchScriptByte();
	if (subOp == SO_ACTOR_INIT) {
		_curActor = pop();
		return;
	}

	a = (ActorHE *)derefActorSafe(_curActor, "o100_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case SO_INIT:
		// freddicove Ru Updated
		// FIXME: check stack parameters
		debug(0,"o100_actorOps: case 0 UNHANDLED");
		break;
	case SO_ANIMATION:
		pop();
		pop();
		pop();
		break;
	case SO_ANIMATION_SPEED:
		a->setAnimSpeed(pop());
		break;
	case SO_AT:
		j = pop();
		i = pop();
		a->putActor(i, j);
		break;
	case SO_BACKGROUND_OFF:
		a->_drawToBackBuf = false;
		a->_needRedraw = true;
		a->_needBgReset = true;
		break;
	case SO_BACKGROUND_ON:
		a->drawActorToBackBuf(a->getPos().x, a->getPos().y);
		break;
	case SO_CHARSET_SET:
		a->_charset = pop();
		break;
	case SO_CLIPPED:
		a->_clipOverride.bottom = pop();
		a->_clipOverride.right = pop();
		a->_clipOverride.top = pop();
		a->_clipOverride.left = pop();
		adjustRect(a->_clipOverride);
		break;
	case SO_CONDITION:
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; ++i) {
			a->setUserCondition(args[i] & 0x7F, args[i] & 0x80);
		}
		break;
	case SO_COSTUME:
		a->setActorCostume(pop());
		break;
	case SO_DEFAULT:
		a->initActor(0);
		break;
	case SO_ERASE:
		k = pop();
		a->setHEFlag(1, k);
		break;
	case SO_NAME:
		copyScriptString(string, sizeof(string));
		loadPtrToResource(rtActorName, a->_number, string);
		break;
	case SO_NEW:
		a->initActor(2);
		break;
	case SO_PALETTE:
		j = pop();
		i = pop();
		assertRange(0, i, 255, "palette slot");
		a->remapActorPaletteColor(i, j);
		a->_needRedraw = true;
		break;
	case SO_PRIORITY:
		a->_layer = pop();
		a->_needRedraw = true;
		break;
	case SO_ROOM_PALETTE:
		a->_hePaletteNum = pop();
		a->_needRedraw = true;
		break;
	case SO_SCALE:
		i = pop();
		a->setScale(i, i);
		break;
	case SO_SHADOW:
		a->_heXmapNum = pop();
		a->_needRedraw = true;
		break;
	case SO_STEP_DIST:
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case SO_TALKIE:
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
	case SO_VARIABLE:
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case SO_ALWAYS_ZCLIP:
		a->_forceClip = pop();
		break;
	case SO_NEVER_ZCLIP:
		a->_forceClip = 0;
		break;
	case SO_ACTOR_DEFAULT_CLIPPED:
		_actorClipOverride.bottom = pop();
		_actorClipOverride.right = pop();
		_actorClipOverride.top = pop();
		_actorClipOverride.left = pop();
		adjustRect(_actorClipOverride);
		break;
	case SO_ACTOR_SOUNDS:
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->_sound[i] = args[i];
		break;
	case SO_ACTOR_WIDTH:
		a->_width = pop();
		break;
	case SO_ANIMATION_DEFAULT:
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case SO_ELEVATION:
		a->setElevation(pop());
		break;
	case SO_FOLLOW_BOXES:
		a->_ignoreBoxes = 0;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_IGNORE_BOXES:
		a->_ignoreBoxes = 1;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_ACTOR_IGNORE_TURNS_OFF:
		a->_ignoreTurns = false;
		break;
	case SO_ACTOR_IGNORE_TURNS_ON:
		a->_ignoreTurns = true;
		break;
	case SO_INIT_ANIMATION:
		a->_initFrame = pop();
		break;
	case SO_STAND_ANIMATION:
		a->_standFrame = pop();
		break;
	case SO_TALK_ANIMATION:
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case SO_TALK_COLOR:
		a->_talkColor = pop();
		break;
	case SO_TALK_CONDITION:
		k = pop();
		if (k == 0)
			k = _rnd.getRandomNumberRng(1, 10);
		a->_heNoTalkAnimation = 1;
		a->setTalkCondition(k);
		break;
	case SO_TEXT_OFFSET:
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case SO_WALK_ANIMATION:
		a->_walkFrame = pop();
		break;
	default:
		error("o100_actorOps: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_arrayOps() {
	byte *data;
	byte string[1024];
	int dim1end, dim1start, dim2end, dim2start;
	int id, len, b, c, list[128];
	int offs, tmp, tmp2;
	uint tmp3, type;

	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	debug(9,"o100_arrayOps: array %d case %d", array, subOp);

	switch (subOp) {
	case SO_FORMATTED_STRING:
		decodeScriptString(string);
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;
	case SO_STRING:
		copyScriptString(string, sizeof(string));
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;

	case SO_ASSIGN_2DIM_LIST:
		len = getStackList(list, ARRAYSIZE(list));
		id = readVar(array);
		if (id == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, len, list[len]);
		}
		break;
	case SO_ASSIGN_INT_LIST:
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
	case SO_COMPLEX_ARRAY_ASSIGNMENT:
		len = getStackList(list, ARRAYSIZE(list));
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);

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
	case SO_COMPLEX_ARRAY_COPY_OPERATION:
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
	case SO_COMPLEX_ARRAY_MATH_OPERATION:
		{
			// Used by room 2 script 2180 in Moonbase Commander (modify-line-of-sight)
			int array2 = fetchScriptWord();
			int array1 = fetchScriptWord();
			type = pop();
			int a1_dim1end = pop();
			int a1_dim1start = pop();
			int a1_dim2end = pop();
			int a1_dim2start = pop();
			int a2_dim1end = pop();
			int a2_dim1start = pop();
			int a2_dim2end = pop();
			int a2_dim2start = pop();
			dim1end = pop();
			dim1start = pop();
			dim2end = pop();
			dim2start = pop();

			debug(0, "Complex: %d = %d[%d to %d][%d to %d] %c %d[%d to %d][%d to %d]", array,
				array1, a1_dim1start, a1_dim2end, a1_dim1start, a1_dim2end,
				" +-&|^"[type],
				array2, a2_dim1start, a2_dim2end, a2_dim1start, a2_dim2end);

			int a12_num = a1_dim2end - a1_dim2start + 1;
			int a11_num = a1_dim1end - a1_dim1start + 1;
			int a22_num = a2_dim2end - a2_dim2start + 1;
			int a21_num = a2_dim1end - a2_dim1start + 1;
			int d12_num = dim2end - dim2start + 1;
			int d11_num = dim1end - dim1start + 1;

			id = readVar(array);
			if (id == 0) {
				defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
			}
			if (a12_num != a22_num || a12_num != d12_num || a11_num != a21_num || a11_num != d11_num) {
				error("Operation size mismatch (%d vs %d)(%d vs %d)", a12_num, a22_num, a11_num, a21_num);
			}

			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start, ++a2_dim2start, ++dim2start) {
				int a2dim1 = a2_dim1start;
				int a1dim1 = a1_dim1start;
				int dim1 = dim1start;
				for (; a1dim1 <= a1_dim1end; ++a1dim1, ++a2dim1, ++dim1) {
					int val1 = readArray(array1, a1_dim2start, a1dim1);
					int val2 = readArray(array2, a2_dim2start, a2dim1);
					int res;

					switch (type) {
					case 1: // Addition
						res = val2 + val1;
						break;
					case 2: // Subtraction
						res = val2 - val1;
						break;
					case 3: // Binary AND
						res = val2 & val1;
						break;
					case 4: // Binary OR
						res = val2 | val1;
						break;
					case 5: // Binary XOR
						res = val2 ^ val1;
						break;
					default:
						error("o100_arrayOps: case 132 unknown type %d)", type);
					}
					writeArray(array, dim2start, dim1, res);
				}
			}
			break;
		}
	case SO_RANGE_ARRAY_ASSIGNMENT:
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
		checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);

		offs = (b >= c) ? 1 : -1;
		tmp2 = c;
		tmp3 = ABS(c - b) + 1;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, tmp2);
				if (--tmp3 == 0) {
					tmp2 = c;
					tmp3 = ABS(c - b) + 1;
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

void ScummEngine_v100he::o100_createSound() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0:
		_heSndResId = pop();
		break;
	case 53:
		createSound(_heSndResId, -1);
		break;
	case 92:
		// dummy case
		break;
	case 128:
		createSound(_heSndResId, pop());
		break;
	default:
		error("o100_createSound: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_dim2dimArray() {
	int data, dim1end, dim2end;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_BIT:
		data = kBitArray;
		break;
	case SO_INT:
		data = kIntArray;
		break;
	case SO_DWORD:
		data = kDwordArray;
		break;
	case SO_NIBBLE:
		data = kNibbleArray;
		break;
	case SO_BYTE:
		data = kByteArray;
		break;
	case SO_STRING:
		data = kStringArray;
		break;
	default:
		error("o100_dim2dimArray: default case %d", subOp);
	}

	dim1end = pop();
	dim2end = pop();
	defineArray(fetchScriptWord(), data, 0, dim2end, 0, dim1end);
}

void ScummEngine_v100he::o100_dimArray() {
	int data;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_BIT:
		data = kBitArray;
		break;
	case SO_INT:
		data = kIntArray;
		break;
	case SO_DWORD:
		data = kDwordArray;
		break;
	case SO_NIBBLE:
		data = kNibbleArray;
		break;
	case SO_BYTE:
		data = kByteArray;
		break;
	case SO_STRING:
		data = kStringArray;
		break;
	case SO_UNDIM_ARRAY:
		nukeArray(fetchScriptWord());
		return;
	default:
		error("o100_dimArray: default case %d", subOp);
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
	case SO_ACTOR:
		drawLine(x1, y1, x, unk1, unk2, 2, id);
		break;
	case SO_COLOR:
		drawLine(x1, y1, x, unk1, unk2, 1, id);
		break;
	case SO_IMAGE:
		drawLine(x1, y1, x, unk1, unk2, 3, id);
		break;
	default:
		error("o100_drawLine: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_drawObject() {
	int state, y, x;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_AT:
		state = 1;
		y = pop();
		x = pop();
		break;
	case SO_AT_IMAGE:
		state = pop();
		y = pop();
		x = pop();
		break;
	case SO_IMAGE:
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

void ScummEngine_v100he::o100_floodFill() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_INIT:
		_floodFillParams.reset();
		_floodFillParams.box.left = 0;
		_floodFillParams.box.top = 0;
		_floodFillParams.box.right = 639;
		_floodFillParams.box.bottom = 479;
		adjustRect(_floodFillParams.box);
		break;
	case SO_AT:
		_floodFillParams.y = pop();
		_floodFillParams.x = pop();
		break;
	case SO_CLIPPED:
		_floodFillParams.box.bottom = pop();
		_floodFillParams.box.right = pop();
		_floodFillParams.box.top = pop();
		_floodFillParams.box.left = pop();
		adjustRect(_floodFillParams.box);
		break;
	case SO_COLOR:
		_floodFillParams.flags = pop();
		break;
	case SO_SET_FLAGS:
		pop();
		break;
	case SO_END:
		floodFill(&_floodFillParams, this);
		break;
	default:
		error("o100_floodFill: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_setSpriteGroupInfo() {
	byte string[260];
	int type, value1, value2, value3, value4;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_INIT: // 0
		_curSpriteGroupId = pop();
		break;
	case SO_AT: // 6
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->setGroupPosition(_curSpriteGroupId, value1, value2);
		break;
	case SO_CLIPPED: // 18
		value4 = pop();
		value3 = pop();
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->setGroupBounds(_curSpriteGroupId, value1, value2, value3, value4);
		break;
	case SO_GROUP: // 38
		type = pop() - 1;
		switch (type) {
		case 0: // SPRGRPOP_MOVE
			value2 = pop();
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->moveGroupMembers(_curSpriteGroupId, value1, value2);
			break;
		case 1: // SPRGRPOP_ORDER
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersPriority(_curSpriteGroupId, value1);
			break;
		case 2: // SPRGRPOP_NEW_GROUP
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersGroup(_curSpriteGroupId, value1);
			break;
		case 3: // SPRGRPOP_UPDATE_TYPE
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersUpdateType(_curSpriteGroupId, value1);
			break;
		case 4: // SPRGRPOP_NEW
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersResetSprite(_curSpriteGroupId);
			break;
		case 5: // SPRGRPOP_ANIMATION_SPEED
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersAnimationSpeed(_curSpriteGroupId, value1);
			break;
		case 6: // SPRGRPOP_ANIMATION_TYPE
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersAutoAnimFlag(_curSpriteGroupId, value1);
			break;
		case 7: // SPRGRPOP_SHADOW
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			_sprite->setGroupMembersShadow(_curSpriteGroupId, value1);
			break;
		default:
			error("o100_setSpriteGroupInfo subOp 38: Unknown case %d", subOp);
		}
		break;
	case SO_IMAGE: // 40
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->setGroupImage(_curSpriteGroupId, value1);
		break;
	case SO_MOVE: // 49
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->moveGroup(_curSpriteGroupId, value1, value2);
		break;
	case SO_NAME: // 52
		copyScriptString(string, sizeof(string));
		break;
	case SO_NEW: // 53
		if (!_curSpriteGroupId)
			break;

		_sprite->resetGroup(_curSpriteGroupId);
		break;
	case SO_NEW_GENERAL_PROPERTY: // 54
		// dummy case
		pop();
		pop();
		break;
	case SO_PRIORITY: // 59
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		_sprite->setGroupPriority(_curSpriteGroupId, value1);
		break;
	case SO_PROPERTY: // 60
		type = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		switch (type) {
		case 0: // SPRGRPPROP_XMUL
			_sprite->setGroupXMul(_curSpriteGroupId, value1);
			break;
		case 1: // SPRGRPPROP_XDIV
			_sprite->setGroupXDiv(_curSpriteGroupId, value1);
			break;
		case 2: // SPRGRPPROP_YMUL
			_sprite->setGroupYMul(_curSpriteGroupId, value1);
			break;
		case 3: // SPRGRPPROP_YDIV
			_sprite->setGroupYDiv(_curSpriteGroupId, value1);
			break;
		default:
			error("o100_setSpriteGroupInfo subOp 60: Unknown case %d", subOp);
		}
		break;
	case SO_NEVER_ZCLIP: // 89
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
	case SO_CHARSET_SET:
		_heResType = rtCharset;
		_heResId = pop();
		break;
	case SO_COSTUME:
		_heResType = rtCostume;
		_heResId = pop();
		break;
	case SO_FLOBJECT:
		_heResType = rtFlObject;
		_heResId = pop();
		break;
	case SO_IMAGE:
		_heResType = rtImage;
		_heResId = pop();
		break;
	case SO_LOAD:
		if (_heResType == rtFlObject) {
			room = getObjectRoom(_heResId);
			loadFlObject(_heResId, room);
		} else if (_heResType == rtCharset) {
			loadCharset(_heResId);
		} else {
			ensureResourceLoaded(_heResType, _heResId);
		}
		break;
	case SO_ROOM:
		_heResType = rtRoom;
		_heResId = pop();
		break;
	case SO_SCRIPT:
		_heResType = rtScript;
		_heResId = pop();
		break;
	case SO_SOUND:
		_heResType = rtSound;
		_heResId = pop();
		break;
	case SO_CLEAR_HEAP:
		// TODO: Clear Heap
		warning("STUB: o100_resourceRoutines: clear Heap");
		break;
	case SO_PRELOAD_FLUSH:
		// Dummy case
		break;
	case SO_LOCK:
		if (_heResType == rtScript && _heResId >= _numGlobalScripts)
			break;

		if (_heResType == rtFlObject) {
			objidx = getObjectIndex(_heResId);
			if (objidx == -1)
				break;
			_res->lock(rtFlObject, _objs[objidx].fl_object_index);
		} else {
			_res->lock(_heResType, _heResId);
		}
		break;
	case SO_NUKE:
		if (_heResType == rtCharset)
			nukeCharset(_heResId);
		else
			_res->nukeResource(_heResType, _heResId);
		break;
	case SO_OFF_HEAP:
	case SO_ON_HEAP:
		// Heap related
		break;
	case SO_PRELOAD:
		if (_heResType == rtScript && _heResId >= _numGlobalScripts)
			break;

		//queueLoadResource(_heResType, _heResId);
		break;
	case SO_UNLOCK:
		if (_heResType == rtScript && _heResId >= _numGlobalScripts)
			break;

		if (_heResType == rtFlObject) {
			objidx = getObjectIndex(_heResId);
			if (objidx == -1)
				break;
			_res->unlock(rtFlObject, _objs[objidx].fl_object_index);
		} else {
			_res->unlock(_heResType, _heResId);
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
	case SO_INIT: // 0
		_wizParams.img.resNum = pop();
		_wizParams.processMode = 0;
		_wizParams.processFlags = 0;
		_wizParams.remapNum = 0;
		_wizParams.img.flags = 0;
		_wizParams.params1 = 0;
		_wizParams.params2 = 0;
		_wizParams.spriteId = 0;
		_wizParams.spriteGroup = 0;
		break;
	case SO_ANGLE: // 2
		_wizParams.processFlags |= kWPFRotate;
		_wizParams.angle = pop();
		break;
	case SO_AT: // 6
	case SO_CURSOR_HOTSPOT: // 132
		_wizParams.processFlags |= kWPFSetPos;
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		break;
	case SO_AT_IMAGE: // 7
		_wizParams.processFlags |= kWPFMaskImg;
		_wizParams.sourceImage = pop();
		break;
	case SO_CAPTURE: // 11
		_wizParams.processFlags |= kWPFClipBox | 0x100;
		_wizParams.processMode = 2;
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		_wizParams.compType = pop();
		adjustRect(_wizParams.box);
		break;
	case SO_CLIPPED: // 18
		_wizParams.processFlags |= kWPFClipBox;
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		adjustRect(_wizParams.box);
		break;
	case SO_COLOR_LIST: // 21
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
	case SO_DRAW: // 29
		_wizParams.processMode = 1;
		break;
	case SO_GENERAL_CLIP_RECT: // 36
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		break;
	case SO_GENERAL_CLIP_STATE: // 37
		// Dummy case
		pop();
		break;
	case SO_HEIGHT: // 39
		_wizParams.processFlags |= kWPFUseDefImgHeight;
		_wizParams.resDefImgH = pop();
		break;
	case SO_LOAD: // 47
		_wizParams.processFlags |= kWPFUseFile;
		_wizParams.processMode = 3;
		copyScriptString(_wizParams.filename, sizeof(_wizParams.filename));
		break;
	case SO_NEW: // 53
		_wizParams.processMode = 8;
		break;
	case SO_NEW_GENERAL_PROPERTY: // 54
		_wizParams.processFlags |= kWPFParams;
		_wizParams.params1 = pop();
		_wizParams.params2 = pop();
		break;
	case SO_NOW: // 55
		_wizParams.img.flags = pop();
		_wizParams.img.state = pop();
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		_wizParams.spriteId = 0;
		_wizParams.spriteGroup = 0;
		_wizParams.img.resNum = pop();
		_wiz->displayWizImage(&_wizParams.img);
		break;
	case SO_PALETTE: // 57
		_wizParams.processFlags |= kWPFPaletteNum;
		_wizParams.img.palette = pop();
		break;
	case SO_POLY_TO_POLY: // 58
		_wizParams.processFlags |= 0x1000 | 0x100 | 0x2;
		_wizParams.processMode = 7;
		_wizParams.polygonId2 = pop();
		_wizParams.polygonId1 = pop();
		_wizParams.compType = pop();
		break;
	case SO_SAVE: // 64
		_wizParams.processFlags |= kWPFUseFile;
		_wizParams.processMode = 4;
		copyScriptString(_wizParams.filename, sizeof(_wizParams.filename));
		_wizParams.fileWriteMode = pop();
		break;
	case SO_SCALE: // 65
		_wizParams.processFlags |= kWPFScaled;
		_wizParams.scale = pop();
		break;
	case SO_SET_FLAGS: // 67
		_wizParams.processFlags |= kWPFNewFlags;
		_wizParams.img.flags |= pop();
		break;
	case SO_SET_POLYGON: // 68
		_wizParams.processFlags |= kWPFNewFlags | kWPFSetPos | 2;
		_wizParams.img.flags |= kWIFIsPolygon;
		_wizParams.polygonId1 = _wizParams.img.y1 = _wizParams.img.x1 = pop();
		break;
	case SO_SHADOW: // 70
		_wizParams.processFlags |= kWPFShadow;
		_wizParams.img.shadow = pop();
		break;
	case SO_STATE: // 73
		_wizParams.processFlags |= kWPFNewState;
		_wizParams.img.state = pop();
		break;
	case SO_WIDTH: // 84
		_wizParams.processFlags |= kWPFUseDefImgWidth;
		_wizParams.resDefImgW = pop();
		break;
	case SO_END: // 92
		if (_wizParams.img.resNum)
			_wiz->processWizImage(&_wizParams);
		break;
	case SO_FONT_CREATE: // 128
		_wizParams.processMode = 15;
		_wizParams.fontProperties.bgColor = pop();
		_wizParams.fontProperties.fgColor = pop();
		_wizParams.fontProperties.size = pop();
		_wizParams.fontProperties.style = pop();
		copyScriptString(_wizParams.fontProperties.fontName, sizeof(_wizParams.fontProperties.fontName));
		break;
	case SO_FONT_END: // 129
		_wizParams.processMode = 14;
		break;
	case SO_FONT_RENDER: // 130
		_wizParams.processMode = 16;
		_wizParams.fontProperties.yPos = pop();
		_wizParams.fontProperties.xPos = pop();
		copyScriptString(_wizParams.fontProperties.string, sizeof(_wizParams.fontProperties.string));
		break;
	case SO_FONT_START: // 131
		_wizParams.processMode = 13;
		break;
	case SO_RENDER_ELLIPSE: // 133
		_wizParams.processMode = 17;
		_wizParams.ellipseProperties.color = pop();
		_wizParams.ellipseProperties.lod = pop();
		_wizParams.ellipseProperties.ky = pop();
		_wizParams.ellipseProperties.kx = pop();
		_wizParams.ellipseProperties.qy = pop();
		_wizParams.ellipseProperties.qx = pop();
		_wizParams.ellipseProperties.py = pop();
		_wizParams.ellipseProperties.px = pop();
		break;
	case SO_RENDER_FLOOD_FILL: // 134
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 12;
		_wizParams.fillColor = pop();
		_wizParams.box2.top = _wizParams.box2.bottom = pop();
		_wizParams.box2.left = _wizParams.box2.right = pop();
		adjustRect(_wizParams.box2);
		break;
	case SO_RENDER_INTO_IMAGE: // 135
		_wizParams.processFlags |= kWPFDstResNum;
		_wizParams.dstResNum = pop();
		break;
	case SO_RENDER_LINE: // 136
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 10;
		_wizParams.fillColor = pop();
		_wizParams.box2.bottom = pop();
		_wizParams.box2.right = pop();
		_wizParams.box2.top = pop();
		_wizParams.box2.left = pop();
		adjustRect(_wizParams.box2);
		break;
	case SO_RENDER_PIXEL: // 137
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 11;
		_wizParams.fillColor = pop();
		_wizParams.box2.top = _wizParams.box2.bottom = pop();
		_wizParams.box2.left = _wizParams.box2.right = pop();
		adjustRect(_wizParams.box2);
		break;
	case SO_RENDER_RECTANGLE: // 138
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 9;
		_wizParams.fillColor = pop();
		_wizParams.box2.bottom = pop();
		_wizParams.box2.right = pop();
		_wizParams.box2.top = pop();
		_wizParams.box2.left = pop();
		adjustRect(_wizParams.box2);
		break;
	default:
		error("o100_wizImageOps: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_dim2dim2Array() {
	int data, dim1start, dim1end, dim2start, dim2end;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_BIT:
		data = kBitArray;
		break;
	case SO_INT:
		data = kIntArray;
		break;
	case SO_DWORD:
		data = kDwordArray;
		break;
	case SO_NIBBLE:
		data = kNibbleArray;
		break;
	case SO_BYTE:
		data = kByteArray;
		break;
	case SO_STRING:
		data = kStringArray;
		break;
	default:
		error("o100_dim2dim2Array: default case %d", subOp);
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
	case SO_INT:
		redimArray(fetchScriptWord(), a, b, c, d, kIntArray);
		break;
	case SO_DWORD:
		redimArray(fetchScriptWord(), a, b, c, d, kDwordArray);
		break;
	case SO_BYTE:
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
	case SO_INIT:
		_hePaletteNum = pop();
		break;
	case SO_COLOR:
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
	case SO_COSTUME:
		a = pop();
		if (_hePaletteNum != 0) {
			setHEPaletteFromCostume(_hePaletteNum, a);
		}
		break;
	case SO_IMAGE:
		b = pop();
		a = pop();
		if (_hePaletteNum != 0) {
			setHEPaletteFromImage(_hePaletteNum, a, b);
		}
		break;
	case SO_NEW:
		if (_hePaletteNum != 0) {
			restoreHEPalette(_hePaletteNum);
		}
		break;
	case SO_PALETTE:
		a = pop();
		if (_hePaletteNum != 0) {
			copyHEPalette(_hePaletteNum, a);
		}
		break;
	case SO_ROOM_PALETTE:
		b = pop();
		a = pop();
		if (_hePaletteNum != 0) {
			setHEPaletteFromRoom(_hePaletteNum, a, b);
		}
		break;
	case SO_TO:
		c = pop();
		b = pop();
		a = pop();
		if (_hePaletteNum) {
			for (; a <= b; ++a) {
				copyHEPaletteColor(_hePaletteNum, a, c);
			}
		}
		break;
	case SO_END:
		_hePaletteNum = 0;
		break;
	default:
		error("o100_paletteOps: Unknown case %d", subOp);
	}
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
	case SO_INT:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kIntArray);
		break;
	case SO_DWORD:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kDwordArray);
		break;
	case SO_BYTE:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kByteArray);
		break;
	default:
		error("o100_redimArray: default type %d", subOp);
	}
}

void ScummEngine_v100he::o100_roomOps() {
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_ROOM_PALETTE:
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case SO_OBJECT_ORDER:
		b = pop();
		a = pop();
		swapObjects(a, b);
		break;

	case SO_ROOM_COPY_PALETTE:
		a = pop();
		b = pop();
		if (_game.features & GF_16BIT_COLOR)
			copyHEPaletteColor(1, a, b);
		else
			copyPalColor(a, b);
		break;

	case SO_ROOM_FADE:
		// Defaults to 1 but doesn't use fade effects
		a = pop();
		break;

	case SO_ROOM_INTENSITY:
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;

	case SO_ROOM_INTENSITY_RGB:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case SO_ROOM_NEW_PALETTE:
		a = pop();
		setCurrentPalette(a);
		break;

	case SO_ROOM_PALETTE_IN_ROOM:
		b = pop();
		a = pop();
		setRoomPalette(a, b);
		break;

	case SO_ROOM_SAVEGAME:
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		break;

	case SO_ROOM_SAVEGAME_BY_NAME:
		byte buffer[256];

		copyScriptString((byte *)buffer, sizeof(buffer));

		_saveLoadFileName = (char *)buffer;
		debug(1, "o100_roomOps: case 137: filename %s", _saveLoadFileName.c_str());

		_saveLoadFlag = pop();
		_saveLoadSlot = 255;
		_saveTemporaryState = true;
		break;

	case SO_ROOM_SCREEN:
		b = pop();
		a = pop();
		initScreens(a, _screenHeight);
		break;

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

	default:
		error("o100_roomOps: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_setSystemMessage() {
	byte name[1024];

	copyScriptString(name, sizeof(name));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_TITLE_BAR: // Set Window Caption
		// TODO: The 'name' string can contain non-ASCII data. This can lead to
		// problems, because the encoding used for "name" is not clear,
		//
		// Possible fixes/workarounds:
		// - Simply stop using this. It's a rather unimportant "feature" anyway.
		// - Try to translate the text to UTF-32.
		//_system->setWindowCaption(Common::U32String((const char *)name));
		break;
	case SO_PAUSE_TITLE: // Set Version
		debug(1,"o100_setSystemMessage: (%d) %s", subOp, name);
		break;
	default:
		error("o100_setSystemMessage: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_soundOps() {
	byte filename[260];
	int var, value;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_AT:
		_heSndFlags |= HE_SND_OFFSET;
		_heSndOffset = pop();
		break;
	case SO_LOAD:
		copyScriptString(filename, sizeof(filename));
		_heSndSoundId = pop();
		if (_heSndSoundId)
			debug(0, "Load sound %d from file %s\n", _heSndSoundId, filename);
		break;
	case SO_NOW:
		_heSndFlags |= HE_SND_QUICK_START;
		break;
	case SO_VARIABLE:
		value = pop();
		var = pop();
		_heSndSoundId = pop();
		((SoundHE *)_sound)->setSoundVar(_heSndSoundId, var, value);
		break;
	case SO_END:
		_sound->addSoundToQueue(_heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags, _heSndSoundFreq, _heSndPan, _heSndVol);
		break;
	case SO_SOUND_ADD:
		_heSndFlags |= HE_SND_APPEND;
		break;
	case SO_SOUND_CHANNEL:
		_heSndChannel = pop();
		break;
	case SO_SOUND_FREQUENCY:
		_heSndFlags |= HE_SND_FREQUENCY;
		_heSndSoundFreq = pop();
		break;
	case SO_SOUND_LOOPING:
		_heSndFlags |= HE_SND_LOOP;
		break;
	case SO_SOUND_MODIFY:
	case SO_SOUND_START:
		_heSndSoundId = pop();
		_heSndOffset = 0;
		_heSndSoundFreq = 11025;
		_heSndChannel = VAR(VAR_SOUND_CHANNEL);
		_heSndFlags = 0;
		break;
	case SO_SOUND_PAN:
		_heSndFlags |= HE_SND_PAN;
		_heSndPan = pop();
		break;
	case SO_SOUND_SOFT:
		_heSndFlags |= HE_SND_SOFT_SOUND;
		break;
	case SO_SOUND_VOLUME:
		_heSndFlags |= HE_SND_VOL;
		_heSndVol = pop();
		break;
	default:
		error("o100_soundOps invalid case %d", subOp);
	}
}

void ScummEngine_v100he::o100_setSpriteInfo() {
	int args[16];
	int spriteId, n;
	int32 tmp[2];
	byte string[80];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_INIT: // 0
		_curMaxSpriteId = pop();
		_curSpriteId = pop();

		if (_curSpriteId > _curMaxSpriteId)
			SWAP(_curSpriteId, _curMaxSpriteId);
		break;
	case SO_ANGLE: // 2
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteAngle(spriteId, args[0]);
		break;
	case SO_ANIMATION: // 3
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteFlagAutoAnim(spriteId, args[0]);
		break;
	case SO_ANIMATION_SPEED: // 4
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteAnimSpeed(spriteId, args[0]);
		break;
	case SO_AT: // 6
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
	case SO_AT_IMAGE: // 7
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteSourceImage(spriteId, args[0]);
		break;
	case SO_CLASS: // 16
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
	case SO_ERASE: // 32
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteFlagEraseType(spriteId, args[0]);
		break;
	case SO_GROUP: // 38
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteGroup(spriteId, args[0]);
		break;
	case SO_IMAGE: // 40
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteImage(spriteId, args[0]);
		break;
	case SO_MASK: // 48
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteMaskImage(spriteId, args[0]);
		break;
	case SO_MOVE: // 49
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
	case SO_NAME: // 52
		copyScriptString(string, sizeof(string));
		break;
	case SO_NEW: // 53
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->resetSprite(spriteId);
		break;
	case SO_NEW_GENERAL_PROPERTY: // 54
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
	case SO_PALETTE: // 57
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpritePalette(spriteId, args[0]);
		break;
	case SO_PRIORITY: // 59
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpritePriority(spriteId, args[0]);
		break;
	case SO_PROPERTY: // 60
		args[1] = pop();
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			switch (args[1]) {
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
				warning("Unknown sprite property %d for sprite %d", args[0], spriteId);
				break;
			}
		break;
	case SO_RESTART: // 61
		_sprite->resetTables(true);
		break;
	case SO_SCALE: // 65
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteScale(spriteId, args[0]);
		break;
	case SO_SHADOW: // 70
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteShadow(spriteId, args[0]);
		break;
	case SO_STATE: // 73
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteImageState(spriteId, args[0]);
		break;
	case SO_STEP_DIST: // 74
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
	case SO_STEP_DIST_X: // 75
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
	case SO_STEP_DIST_Y: // 76
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
	case SO_UPDATE: // 82
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteFlagUpdateType(spriteId, args[0]);
		break;
	case SO_VARIABLE: // 83
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
	case SO_IMAGE_ZCLIP: // 88
		args[0] = pop();
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteZBuffer(spriteId, args[0]);
		break;
	case SO_NEVER_ZCLIP: // 89
		if (_curSpriteId > _curMaxSpriteId)
			break;
		spriteId = _curSpriteId;
		if (!spriteId)
			spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
			_sprite->setSpriteZBuffer(spriteId, 0);
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

	switch (subOp) {
	case SO_RESTART:
		restart();
		break;
	case SO_FLUSH_OBJECT_DRAW_QUEUE:
		clearDrawObjectQueue();
		break;
	case SO_QUIT:
		// Confirm shutdown
		if (_game.id == GID_MOONBASE)
			// Moonbase uses this subOp to quit the game (The confirmation dialog
			// exists inside the game).
			quitGame();
		else
			confirmExitDialog();
		break;
	case SO_QUIT_QUIT:
		quitGame();
		break;
	case SO_RESTART_STRING:
		copyScriptString(string, sizeof(string));
		debug(0, "Start game (%s)", string);
		break;
	case SO_START_SYSTEM_STRING:
		copyScriptString(string, sizeof(string));
		debug(0, "Start executable (%s)", string);
		break;
	case SO_UPDATE_SCREEN:
		restoreBackgroundHE(Common::Rect(_screenWidth, _screenHeight));
		updatePalette();
		break;
	default:
		error("o100_systemOps invalid case %d", subOp);
	}
}

void ScummEngine_v100he::o100_cursorCommand() {
	int a, b, i;
	int args[16];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_CHARSET_SET:
		initCharset(pop());
		break;
	case SO_CHARSET_COLOR:
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
		break;
	case SO_CURSOR_IMAGE:
	case SO_CURSOR_COLOR_IMAGE:
		a = pop();
		_wiz->loadWizCursor(a, 0);
		break;
	case SO_CURSOR_COLOR_PAL_IMAGE:
		b = pop();
		a = pop();
		_wiz->loadWizCursor(a, b);
		break;
	case SO_CURSOR_ON:		// Turn cursor on
		_cursor.state = 1;
		break;
	case SO_CURSOR_OFF:		// Turn cursor off
		_cursor.state = 0;
		break;
	case SO_CURSOR_SOFT_ON:		// Turn soft cursor on
		_cursor.state++;
		if (_cursor.state > 1)
			error("o100_cursorCommand: Cursor state greater than 1 in script");
		break;

	case SO_CURSOR_SOFT_OFF:		// Turn soft cursor off
		_cursor.state--;
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
	default:
		error("o100_cursorCommand: default case %x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void ScummEngine_v100he::o100_videoOps() {
	// Uses Bink video
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_INIT:
		memset(_videoParams.filename, 0, sizeof(_videoParams.filename));
		_videoParams.status = 0;
		_videoParams.flags = 0;
		_videoParams.number = pop();
		_videoParams.wizResNum = 0;

		if (_videoParams.number != 1 && _videoParams.number != -1)
			warning("o100_videoOps: number: %d", _videoParams.number);
		break;
	case SO_CLOSE:
		_videoParams.status = SO_CLOSE;
		break;
	case SO_IMAGE:
		_videoParams.wizResNum = pop();
		if (_videoParams.wizResNum)
			_videoParams.flags |= 2;
		break;
	case SO_LOAD:
		copyScriptString(_videoParams.filename, sizeof(_videoParams.filename));
		_videoParams.status = SO_LOAD;
		break;
	case SO_SET_FLAGS:
		_videoParams.flags |= pop();
		break;
	case SO_END:
		if (_videoParams.status == SO_LOAD) {
			// Start video
			if (_videoParams.flags == 0)
				_videoParams.flags = 4;

			if (_videoParams.flags & 2) {
				VAR(119) = _moviePlay->load(convertFilePath(_videoParams.filename), _videoParams.flags, _videoParams.wizResNum);
			} else {
				VAR(119) = _moviePlay->load(convertFilePath(_videoParams.filename), _videoParams.flags);
			}
		} else if (_videoParams.status == SO_CLOSE) {
			// Stop video
			_moviePlay->close();
		}
		break;
	default:
		error("o100_videoOps: unhandled case %d", subOp);
	}

	debug(1,"o100_videoOps stub (%d)", subOp);
}

void ScummEngine_v100he::o100_wait() {
	int actnum;
	int offs = -2;
	Actor *a;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_WAIT_FOR_ACTOR:		// Wait for actor
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o100_wait:168");
		if (a->_moving)
			break;
		return;
	case SO_WAIT_FOR_CAMERA:		// Wait for camera
		if (camera._cur.x / 8 != camera._dest.x / 8)
			break;
		return;
	case SO_WAIT_FOR_MESSAGE:		// Wait for message
		if (VAR(VAR_HAVE_MSG))
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
	default:
		error("o100_wait: default case 0x%x", subOp);
	}

	_scriptPointer += offs;
	o6_breakHere();
}

void ScummEngine_v100he::o100_writeFile() {
	int32 resID = pop();
	int slot = pop();

	byte subOp = fetchScriptByte();

	assert(_hOutFileTable[slot]);
	switch (subOp) {
	case SO_ARRAY:
		fetchScriptByte();
		writeFileFromArray(slot, resID);
		break;
	case SO_INT:
		_hOutFileTable[slot]->writeUint16LE(resID);
		break;
	case SO_DWORD:
		_hOutFileTable[slot]->writeUint32LE(resID);
		break;
	case SO_BYTE:
		_hOutFileTable[slot]->writeByte(resID);
		break;
	default:
		error("o100_writeFile: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_debugInput() {
	// Backyard Baseball 2003 / Basketball / Football 2002
	// use older o72_debugInput code
	if (_game.heversion == 101) {
		ScummEngine_v72he::o72_debugInput();
		return;
	}

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_INIT:
		copyScriptString(_debugInputBuffer, sizeof(_debugInputBuffer));
		break;
	case SO_COUNT:
		pop();
		break;
	case SO_DEFAULT:
		copyScriptString(_debugInputBuffer, sizeof(_debugInputBuffer));
		break;
	case SO_TITLE_BAR:
		copyScriptString(_debugInputBuffer, sizeof(_debugInputBuffer));
		break;
	case SO_END:
		debugInput(_debugInputBuffer);
		break;
	default:
		error("o100_debugInput: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_isResourceLoaded() {
	// Reports percentage of resource loaded by queue
	ResType type;

	byte subOp = fetchScriptByte();
	int idx = pop();

	switch (subOp) {
	case SO_COSTUME:
		type = rtCostume;
		break;
	case SO_IMAGE:
		type = rtImage;
		break;
	case SO_ROOM:
		type = rtRoom;
		break;
	case SO_SCRIPT:
		type = rtScript;
		break;
	case SO_SOUND:
		type = rtSound;
		break;
	default:
		error("o100_isResourceLoaded: default case %d", subOp);
	}
	debug(7, "o100_isResourceLoaded(%d,%d)", type, idx);

	push(100);
}

void ScummEngine_v100he::o100_getResourceSize() {
	const byte *ptr;
	int size;
	ResType type;

	int resid = pop();
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_COSTUME:
		type = rtCostume;
		break;
	case SO_IMAGE:
		type = rtImage;
		break;
	case SO_ROOM:
		type = rtRoomImage;
		break;
	case SO_SCRIPT:
		type = rtScript;
		break;
	case SO_SOUND:
		push(getSoundResourceSize(resid));
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

	warning("o100_getSpriteGroupInfo, subop %d", subOp);

	switch (subOp) {
	case SO_ARRAY:
		spriteGroupId = pop();
		if (spriteGroupId)
			push(getGroupSpriteArray(spriteGroupId));
		else
			push(0);
		break;
	case SO_IMAGE:
		spriteGroupId = pop();
		if (spriteGroupId)
			push(_sprite->getGroupDstResNum(spriteGroupId));
		else
			push(0);
		break;
	case SO_NEW_GENERAL_PROPERTY:
		// TODO: U32 related
		pop();
		pop();
		push(0);
		warning("STUB: o100_getSpriteGroupInfo, subop 54");
		break;
	case SO_PRIORITY:
		spriteGroupId = pop();
		if (spriteGroupId)
			push(_sprite->getGroupPriority(spriteGroupId));
		else
			push(0);
		break;
	case SO_PROPERTY:
		type = pop();
		spriteGroupId = pop();
		if (spriteGroupId) {
			switch (type) {
			case 0: // SPRGRPPROP_XMUL
				push(_sprite->getGroupXMul(spriteGroupId));
				break;
			case 1: // SPRGRPPROP_XDIV
				push(_sprite->getGroupXDiv(spriteGroupId));
				break;
			case 2: // SPRGRPPROP_YMUL
				push(_sprite->getGroupYMul(spriteGroupId));
				break;
			case 3: // SPRGRPPROP_YDIV
				push(_sprite->getGroupYDiv(spriteGroupId));
				break;
			default:
				push(0);
			}
		} else {
			push(0);
		}
		break;
	case SO_XPOS:
		spriteGroupId = pop();
		if (spriteGroupId) {
			_sprite->getGroupPosition(spriteGroupId, tx, ty);
			push(tx);
		} else {
			push(0);
		}
		break;
	case SO_YPOS:
		spriteGroupId = pop();
		if (spriteGroupId) {
			_sprite->getGroupPosition(spriteGroupId, tx, ty);
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
	int32 x, y;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_COLOR:
		y = pop();
		x = pop();
		state = pop();
		resId = pop();
		push(_wiz->getWizPixelColor(resId, state, x, y));
		break;
	case SO_COUNT:
		resId = pop();
		push(_wiz->getWizImageStates(resId));
		break;
	case SO_FIND:
		y = pop();
		x = pop();
		state = pop();
		resId = pop();
		push(_wiz->isWizPixelNonTransparent(resId, state, x, y, 0));
		break;
	case SO_HEIGHT:
		state = pop();
		resId = pop();
		_wiz->getWizImageDim(resId, state, w, h);
		push(h);
		break;
	case SO_NEW_GENERAL_PROPERTY:
		type = pop();
		state = pop();
		resId = pop();
		push(_wiz->getWizImageData(resId, state, type));
		break;
	case SO_WIDTH:
		state = pop();
		resId = pop();
		_wiz->getWizImageDim(resId, state, w, h);
		push(w);
		break;
	case SO_XPOS:
		state = pop();
		resId = pop();
		_wiz->getWizImageSpot(resId, state, x, y);
		push(x);
		break;
	case SO_YPOS:
		state = pop();
		resId = pop();
		_wiz->getWizImageSpot(resId, state, x, y);
		push(y);
		break;
	case SO_FONT_START:
		pop();
		copyScriptString(filename, sizeof(filename));
		pop();
		push(0);
		debug(0, "o100_getWizData() case 111 unhandled");
		break;
	case SO_HISTOGRAM:
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
	int c, d, e;
	int r, g, b;
	int palSlot, color;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_CHANNEL:
		c = pop();
		b = pop();
		if (_game.features & GF_16BIT_COLOR)
			push(getHEPalette16BitColorComponent(b, c));
		else
			push(getHEPaletteColorComponent(1, b, c));
		break;
	case SO_COLOR:
		color = pop();
		palSlot = pop();
		push(getHEPaletteColor(palSlot, color));
		break;
	case SO_FIND:
		e = pop();
		d = pop();
		palSlot = pop();
		b = pop();
		g = pop();
		r = pop();
		push(getHEPaletteSimilarColor(palSlot, r, g, d, e));
		break;
	case SO_NEW:
		b = pop();
		b = MAX(0, b);
		b = MIN(b, 255);
		g = pop();
		g = MAX(0, g);
		g = MIN(g, 255);
		r = pop();
		r = MAX(0, r);
		r = MIN(r, 255);
		if (_game.features & GF_16BIT_COLOR) {
			push(get16BitColor(r, g, b));
		} else {
			push(getHEPaletteSimilarColor(1, r, g, 10, 245));
		}
		break;
	case SO_STATE:
		c = pop();
		b = pop();
		palSlot = pop();
		push(getHEPaletteColorComponent(palSlot, b, c));
		break;
	default:
		error("o100_getPaletteData: Unknown case %d", subOp);
	}
}

void ScummEngine_v100he::o100_readFile() {
	int slot, val;
	int32 size;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_ARRAY:
		fetchScriptByte();
		size = pop();
		slot = pop();
		val = readFileToArray(slot, size);
		push(val);
		break;
	case SO_INT:
		slot = pop();
		assert(_hInFileTable[slot]);
		val = _hInFileTable[slot]->readUint16LE();
		push(val);
		break;
	case SO_DWORD:
		slot = pop();
		assert(_hInFileTable[slot]);
		val = _hInFileTable[slot]->readUint32LE();
		push(val);
		break;
	case SO_BYTE:
		slot = pop();
		assert(_hInFileTable[slot]);
		val = _hInFileTable[slot]->readByte();
		push(val);
		break;
	default:
		error("o100_readFile: default case %d", subOp);
	}
}

void ScummEngine_v100he::o100_getSpriteInfo() {
	int args[16];
	int spriteId, flags, groupId, type;
	int32 x, y;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_ANIMATION:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteFlagAutoAnim(spriteId));
		else
			push(0);
		break;
	case SO_ANIMATION_SPEED:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteAnimSpeed(spriteId));
		else
			push(1);
		break;
	case SO_AT_IMAGE:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteSourceImage(spriteId));
		else
			push(0);
		break;
	case SO_CLASS:
		flags = getStackList(args, ARRAYSIZE(args));
		spriteId = pop();
		if (spriteId) {
			push(_sprite->getSpriteClass(spriteId, flags, args));
		} else {
			push(0);
		}
		break;
	case SO_COUNT:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteImageStateCount(spriteId));
		else
			push(0);
		break;
	case SO_DRAW_XPOS:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteDisplayX(spriteId));
		else
			push(0);
		break;
	case SO_DRAW_YPOS:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteDisplayY(spriteId));
		else
			push(0);
		break;
	case SO_ERASE:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteFlagEraseType(spriteId));
		else
			push(1);
		break;
	case SO_FIND:
		flags = getStackList(args, ARRAYSIZE(args));
		type = pop();
		groupId = pop();
		y = pop();
		x = pop();
		push(_sprite->findSpriteWithClassOf(x, y, groupId, type, flags, args));
		break;
	case SO_GROUP:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteGroup(spriteId));
		else
			push(0);
		break;
	case SO_HEIGHT:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpriteImageDim(spriteId, x, y);
			push(y);
		} else {
			push(0);
		}
		break;
	case SO_IMAGE:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteImage(spriteId));
		else
			push(0);
		break;
	case SO_MASK:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteMaskImage(spriteId));
		else
			push(0);
		break;
	case SO_NEW_GENERAL_PROPERTY:
		flags = pop();
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteGeneralProperty(spriteId, flags));
		else
			push(0);
		break;
	case SO_PALETTE:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpritePalette(spriteId));
		else
			push(0);
		break;
	case SO_PRIORITY:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpritePriority(spriteId));
		else
			push(0);
		break;
	case SO_PROPERTY:
		flags = pop();
		spriteId = pop();
		if (spriteId) {
			switch (flags) {
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
	case SO_SCALE:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteScale(spriteId));
		else
			push(0);
		break;
	case SO_SHADOW:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteShadow(spriteId));
		else
			push(0);
		break;
	case SO_STATE:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteImageState(spriteId));
		else
			push(0);
		break;
	case SO_STEP_DIST_X:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpriteDist(spriteId, x, y);
			push(x);
		} else {
			push(0);
		}
		break;
	case SO_STEP_DIST_Y:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpriteDist(spriteId, x, y);
			push(y);
		} else {
			push(0);
		}
		break;
	case SO_UPDATE:
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteFlagUpdateType(spriteId));
		else
			push(0);
		break;
	case SO_VARIABLE:
		pop();
		spriteId = pop();
		if (spriteId)
			push(_sprite->getSpriteUserValue(spriteId));
		else
			push(0);
		break;
	case SO_WIDTH:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpriteImageDim(spriteId, x, y);
			push(x);
		} else {
			push(0);
		}
		break;
	case SO_XPOS:
		spriteId = pop();
		if (spriteId) {
			_sprite->getSpritePosition(spriteId, x, y);
			push(x);
		} else {
			push(0);
		}
		break;
	case SO_YPOS:
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
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_COUNT:
		pop();
		push(_moviePlay->getFrameCount());
		break;
	case SO_HEIGHT:
		pop();
		push(_moviePlay->getHeight());
		break;
	case SO_IMAGE:
		pop();
		push(_moviePlay->getImageNum());
		break;
	case SO_NEW_GENERAL_PROPERTY:
		debug(0, "o100_getVideoData: subOp 28 stub (%d, %d)", pop(), pop());
		push(0);
		break;
	case SO_STATE:
		pop();
		push(_moviePlay->getCurFrame());
		break;
	case SO_WIDTH:
		pop();
		push(_moviePlay->getWidth());
		break;
	default:
		error("o100_getVideoData: unhandled case %d", subOp);
	}
}

void ScummEngine_v100he::decodeParseString(int m, int n) {
	Actor *a;
	int i, colors, size;
	int args[31];
	byte name[1024];

	byte b = fetchScriptByte();

	switch (b) {
	case SO_AT:
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case SO_CENTER:
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case SO_CLIPPED:
		_string[m].right = pop();
		break;
	case SO_COLOR:
		_string[m].color = pop();
		break;
	case SO_COLOR_LIST:
		colors = pop();
		if (colors == 1) {
			_string[m].color = pop();
		} else {
			push(colors);
			getStackList(args, ARRAYSIZE(args));
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[m]._default.charset][i] = (unsigned char)args[i];
			_string[m].color = _charsetColorMap[0];
		}
		break;
	case SO_FORMATTED_STRING:
		decodeScriptString(name, true);
		printString(m, name);
		break;
	case SO_LEFT:
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case SO_MUMBLE:
		_string[m].no_talk_anim = true;
		break;
	case SO_OVERHEAD:
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case SO_TALKIE:
		{
		byte *dataPtr = getResourceAddress(rtTalkie, pop());
		byte *text = findWrappedBlock(MKTAG('T','E','X','T'), dataPtr, 0, 0);
		size = getResourceDataSize(text);
		memcpy(name, text, size);
		printString(m, name);
		}
		break;
	case SO_TEXTSTRING:
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case SO_BASEOP:
		_string[m].loadDefault();
		if (n) {
			_actorToPrintStrFor = pop();
			if (_actorToPrintStrFor != 0xFF) {
				a = derefActor(_actorToPrintStrFor, "decodeParseString");
				_string[m].color = a->_talkColor;
			}
		}
		break;
	case SO_END:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case %d", b);
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
