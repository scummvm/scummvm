/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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

#include "sound/mididrv.h"
#include "sound/mixer.h"

namespace Scumm {

#define OPCODE(x)	{ &ScummEngine_v90he::x, #x }

void ScummEngine_v90he::setupOpcodes() {
	static const OpcodeEntryV90he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o72_pushDWord),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o72_addMessageToStack),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayRead),
		/* 08 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o90_dup),
		OPCODE(o6_wordArrayIndexedRead),
		/* 0C */
		OPCODE(o6_dup),
		OPCODE(o6_not),
		OPCODE(o6_eq),
		OPCODE(o6_neq),
		/* 10 */
		OPCODE(o6_gt),
		OPCODE(o6_lt),
		OPCODE(o6_le),
		OPCODE(o6_ge),
		/* 14 */
		OPCODE(o6_add),
		OPCODE(o6_sub),
		OPCODE(o6_mul),
		OPCODE(o6_div),
		/* 18 */
		OPCODE(o6_land),
		OPCODE(o6_lor),
		OPCODE(o6_pop),
		OPCODE(o72_isAnyOf),
		/* 1C */
		OPCODE(o90_wizImageOps),
		OPCODE(o90_min),
		OPCODE(o90_max),
		OPCODE(o90_sin),
		/* 20 */
		OPCODE(o90_cos),
		OPCODE(o90_sqrt),
		OPCODE(o90_atan2),
		OPCODE(o90_getSegmentAngle),
		/* 24 */
		OPCODE(o6_invalid),
		OPCODE(o90_unknown25),
		OPCODE(o90_unknown26),
		OPCODE(o90_unknown27),
		/* 28 */
		OPCODE(o90_unknown28),
		OPCODE(o90_unknown29),
		OPCODE(o6_invalid),
		OPCODE(o90_startLocalScript),
		/* 2C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o90_unknown2F),
		/* 30 */
		OPCODE(o90_mod),
		OPCODE(o90_unknown31),
		OPCODE(o90_unknown32),
		OPCODE(o6_invalid),
		/* 34 */
		OPCODE(o90_findAllObjectsWithClassOf),
		OPCODE(o90_getPolygonOverlap),
		OPCODE(o90_unknown36),
		OPCODE(o90_dim2dim2Array),
		/* 38 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o90_sortArray),
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
		OPCODE(o6_writeWordVar),
		/* 44 */
		OPCODE(o90_getObjectData),
		OPCODE(o80_unknown45),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayWrite),
		/* 48 */
		OPCODE(o80_unknown48),
		OPCODE(o80_unknown49),
		OPCODE(o80_unknown4A),
		OPCODE(o6_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o80_readConfigFile),
		OPCODE(o80_writeConfigFile),
		OPCODE(o6_wordVarInc),
		/* 50 */
		OPCODE(o72_unknown50),
		OPCODE(o6_invalid),
		OPCODE(o72_findObjectWithClassOf),
		OPCODE(o72_wordArrayInc),
		/* 54 */
		OPCODE(o72_getObjectImageX),
		OPCODE(o72_getObjectImageY),
		OPCODE(o72_captureWizImage),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o72_getTimer),
		OPCODE(o72_setTimer),
		OPCODE(o72_unknown5A),
		OPCODE(o72_wordArrayDec),
		/* 5C */
		OPCODE(o6_if),
		OPCODE(o6_ifNot),
		OPCODE(o72_startScript),
		OPCODE(o6_startScriptQuick),
		/* 60 */
		OPCODE(o72_startObject),
		OPCODE(o72_drawObject),
		OPCODE(o72_printWizImage),
		OPCODE(o72_getArrayDimSize),
		/* 64 */
		OPCODE(o72_getNumFreeArrays),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_endCutscene),
		/* 68 */
		OPCODE(o6_cutscene),
		OPCODE(o6_invalid),
		OPCODE(o6_freezeUnfreeze),
		OPCODE(o80_cursorCommand),
		/* 6C */
		OPCODE(o6_breakHere),
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_setClass),
		OPCODE(o6_getState),
		/* 70 */
		OPCODE(o80_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o70_startSound),
		OPCODE(o6_stopSound),
		OPCODE(o6_invalid),
		OPCODE(o6_stopObjectScript),
		/* 78 */
		OPCODE(o6_panCameraTo),
		OPCODE(o6_actorFollowCamera),
		OPCODE(o6_setCameraAt),
		OPCODE(o6_loadRoom),
		/* 7C */
		OPCODE(o6_stopScript),
		OPCODE(o6_walkActorToObj),
		OPCODE(o6_walkActorTo),
		OPCODE(o6_putActorAtXY),
		/* 80 */
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_faceActor),
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		/* 84 */
		OPCODE(o70_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o70_getActorRoom),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		OPCODE(o6_getObjectOldDir),
		/* 90 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_findInventory),
		OPCODE(o6_getInventoryCount),
		/* 94 */
		OPCODE(o90_getPaletteData),
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_setObjectName),
		/* 98 */
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_setBoxFlags),
		OPCODE(o6_invalid),
		OPCODE(o70_resourceRoutines),
		/* 9C */
		OPCODE(o72_roomOps),
		OPCODE(o72_actorOps),
		OPCODE(o90_paletteOps),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o72_findObject),
		OPCODE(o6_pseudoRoom),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getVerbEntrypoint),
		/* A4 */
		OPCODE(o72_arrayOps),
		OPCODE(o90_unknownA5),
		OPCODE(o80_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o60_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o90_getActorAnimProgress),
		/* AC */
		OPCODE(o80_drawWizPolygon),
		OPCODE(o6_isAnyOf),
		OPCODE(o70_quitPauseRestart),
		OPCODE(o6_isActorInBox),
		/* B0 */
		OPCODE(o6_delay),
		OPCODE(o6_delaySeconds),
		OPCODE(o6_delayMinutes),
		OPCODE(o6_stopSentence),
		/* B4 */
		OPCODE(o6_printLine),
		OPCODE(o6_printCursor),
		OPCODE(o6_printDebug),
		OPCODE(o6_printSystem),
		/* B8 */
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		OPCODE(o6_talkEgo),
		/* BC */
		OPCODE(o72_dimArray),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		/* C0 */
		OPCODE(o72_dim2dimArray),
		OPCODE(o72_traceStatus),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_abs),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* C8 */
		OPCODE(o72_kernelGetFunctions),
		OPCODE(o70_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o72_drawWizImage),
		OPCODE(o72_unknownCF),
		/* D0 */
		OPCODE(o6_getDateTime),
		OPCODE(o6_stopTalking),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o6_shuffle),
		OPCODE(o72_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o60_closeFile),
		OPCODE(o72_openFile),
		OPCODE(o72_readFile),
		/* DC */
		OPCODE(o72_writeFile),
		OPCODE(o72_findAllObjects),
		OPCODE(o72_deleteFile),
		OPCODE(o60_rename),
		/* E0 */
		OPCODE(o80_unknownE0),
		OPCODE(o72_getPixel),
		OPCODE(o60_localizeArray),
		OPCODE(o80_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o60_seekFilePos),
		OPCODE(o72_redimArray),
		OPCODE(o60_readFilePos),
		/* EC */
		OPCODE(o72_unknownEC),
		OPCODE(o72_unknownED),
		OPCODE(o70_stringLen),
		OPCODE(o72_unknownEF),
		/* F0 */
		OPCODE(o72_unknownF0),
		OPCODE(o72_unknownF1),
		OPCODE(o72_checkGlobQueue),
		OPCODE(o72_readINI),
		/* F4 */
		OPCODE(o72_writeINI),
		OPCODE(o72_unknownF5),
		OPCODE(o72_unknownF6),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o72_getResourceSize),
		OPCODE(o72_setFilePath),
		OPCODE(o72_unknownFA),
		OPCODE(o70_polygonOps),
		/* FC */
		OPCODE(o70_polygonHit),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesV90he = opcodes;
}

void ScummEngine_v90he::executeOpcode(byte i) {
	OpcodeProcV90he op = _opcodesV90he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v90he::getOpcodeDesc(byte i) {
	return _opcodesV90he[i].desc;
}

void ScummEngine_v90he::o90_dup() {
	int a, num;

	num = fetchScriptWord();
	for (int i = 0; i < num; i++) {
		a = pop();
		push(a);
		push(a);
	}
}

void ScummEngine_v90he::o90_min() {
	int a = pop();
	int b = pop();

	if (b < a) {
		push(b);
	} else {
		push(a);
	}
}

void ScummEngine_v90he::o90_max() {
	int a = pop();
	int b = pop();

	if (b > a) {
		push(b);
	} else {
		push(a);
	}
}

void ScummEngine_v90he::o90_sin() {
	double a = pop() * PI / 180.;
	push((int)(sin(a) * 100000));	
}

void ScummEngine_v90he::o90_cos() {
	double a = pop() * PI / 180.;
	push((int)(cos(a) * 100000));	
}

void ScummEngine_v90he::o90_sqrt() {
	int i = pop();
	if (i < 2) {
		push(i);
	} else {
		push((int)sqrt(i + 1));
	}
}

void ScummEngine_v90he::o90_atan2() {
	int y = pop();
	int x = pop();
	int a = (int)(atan2(y, x) * 180. / PI);
	if (a < 0) {
		a += 360;
	}
	push(a);
}

void ScummEngine_v90he::o90_getSegmentAngle() {
	int y1 = pop();
	int x1 = pop();
	int dy = y1 - pop();
	int dx = x1 - pop();
	int a = (int)(atan2(dy, dx) * 180. / PI);
	if (a < 0) {
		a += 360;
	}
	push(a);
}

void ScummEngine_v90he::o90_startLocalScript() {
	int args[16];
	int script, entryp;
	int flags;
	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	flags = fetchScriptByte();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v90he::drawWizComplexPolygon(int resnum, int state, int po_x, int po_y, int arg14, int angle, int zoom, const Common::Rect *r) {
	Common::Point pts[4];
	uint32 w, h;
	getWizImageDim(resnum, state, w, h);

	pts[1].x = pts[2].x = w / 2 - 1;
	pts[0].x = pts[0].y = pts[1].y = pts[3].x = -w / 2;
	pts[2].y = pts[3].y = h / 2 - 1;

	// transform points
	if (zoom != 256) {
		for (int i = 0; i < 4; ++i) {
			pts[i].x = pts[i].x * zoom / 256;
			pts[i].y = pts[i].y * zoom / 256;
		}
	}
	if (angle != 0) {
		double alpha = angle * PI / 180.;
		double cos_alpha = cos(alpha);
		double sin_alpha = sin(alpha);
		for (int i = 0; i < 4; ++i) {
			int16 x = pts[i].x;
			int16 y = pts[i].y;
			pts[i].x = (int16)(x * cos_alpha - y * sin_alpha);
			pts[i].y = (int16)(y * cos_alpha + x * sin_alpha);
		}
	}
	for (int i = 0; i < 4; ++i) {
		pts[i].x += po_x;
		pts[i].y += po_y;
	}
	// XXX drawWizPolygonPoints(resnum, state, pts, r, VAR(117));
	warning("ScummEngine_v90he::drawWizComplexPolygon() partially implemented");
}

void ScummEngine_v90he::displayWizComplexImage(const WizParameters *params) {
	// XXX merge with ScummEngine_v72he::displayWizImage
	int zoom = 256;
	if (params->processFlags & 0x8) {
		zoom = params->zoom;
	}
	int rotationAngle = 0;
	if (params->processFlags & 0x10) {
		rotationAngle = params->angle;
	}
	int state = 0;
	if (params->processFlags & 0x400) {
		state = params->img.state;
	}
	int flags = 0;
	if (params->processFlags & 0x20) {
		flags = params->img.flags;
	}
	int po_x = 0;
	int po_y = 0;
	if (params->processFlags & 0x1) {
		po_x = params->img.x1;
		po_y = params->img.y1;
	}
	int unk = 0;
	if (params->processFlags & 0x4) {
		unk = params->unk_15C;
	}
	const Common::Rect *r = NULL;
	if (params->processFlags & 0x200) {
		r = &params->box;
	}

	if (_fullRedraw) {
		assert(_wizImagesNum < ARRAYSIZE(_wizImages));
		WizImage *pwi = &_wizImages[_wizImagesNum];
		pwi->resNum = params->img.resNum;
		pwi->x1 = po_x;
		pwi->y1 = po_y;
		pwi->state = state;
		pwi->flags = flags;
		pwi->unk = unk;
		++_wizImagesNum;
	} else if (params->processFlags & 0x18) {
		drawWizComplexPolygon(params->img.resNum, state, po_x, po_y, unk, rotationAngle, zoom, r);
	} else if (flags & 0x40) {
		drawWizPolygon(params->img.resNum, state, po_x, flags); // XXX , VAR(117));
	} else {
		// XXX flags 0x200, 0x24
		WizImage wi;
		wi.resNum = params->img.resNum;
		wi.x1 = po_x;
		wi.y1 = po_y;
		wi.state = state;
		wi.flags = flags;
		wi.unk = unk;
		drawWizImage(rtImage, &wi);
	}
}

void ScummEngine_v90he::processWizImage(const WizParameters *params) {
	debug(1, "ScummEngine_v90he::processWizImage()");
	switch (params->processMode) {
	case 1:
		displayWizComplexImage(params);
		break;
	case 2:
	case 3:
	case 4:
	case 6:
	// HE 99+
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		warning("unhandled processWizImage mode %d", params->processMode);
		break;
	default:
		warning("invalid processWizImage mode %d", params->processMode);
	}
}

void ScummEngine_v90he::o90_wizImageOps() {
	int a, b;
	int subOp = fetchScriptByte();
	subOp -= 46;

	switch (subOp) {
	case -14: // HE99+
		_wizParams.processFlags |= 0x20;
		pop();
		break;
	case -13: // HE99+
		_wizParams.processFlags |= 0x40;
		pop();
		break;
	case 0:
		pop();
		break;
	case 1:
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		break;
	case 2:
		_wizParams.processMode = 1;
		break;
	case 3:
		_wizParams.processFlags |= 0x800;
		_wizParams.processMode = 3;
		copyScriptString(_wizParams.filename);
		break;
	case 4:
		_wizParams.processFlags |= 0x800;
		_wizParams.processMode = 4;
		copyScriptString(_wizParams.filename);
		_wizParams.unk_14C = pop();
		break;
	case 5:
		_wizParams.processFlags |= 0x300;
		_wizParams.processMode = 2;
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		_wizParams.unk_148 = pop();
		break;
	case 6:
		_wizParams.processFlags |= 0x400;
		_wizParams.img.state = pop();
		break;
	case 7:
		_wizParams.processFlags |= 0x10;
		_wizParams.angle = pop();
		break;
	case 8:
		_wizParams.processFlags |= 0x20;
		_wizParams.img.flags = pop();
		break;
	case 10:
		_wizParams.img.flags = pop();
		_wizParams.img.state = pop();
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		_wizParams.img.resNum = pop();
		displayWizImage(&_wizParams.img);
		break;
	case 11:
		_wizParams.img.resNum = pop();
		_wizParams.processMode = 0;
		_wizParams.processFlags = 0;
		_wizParams.remapPos = 0;
		_wizParams.img.flags = 0;
		break;
	case 16: // HE99+
		_wizParams.processFlags |= 0x80000;
		pop();
		break;
	case 19:
		_wizParams.processFlags |= 1;
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		break;
	case 20:
	case 203: // HE98+
		b = pop();
		a = pop();
		_wizParams.processFlags |= 0x40;
		_wizParams.processMode = 6;
		if (_wizParams.remapPos == 0) {
			memset(_wizParams.remapBuf2, 0, sizeof(_wizParams.remapBuf2));
		} else {
			assert(_wizParams.remapPos < ARRAYSIZE(_wizParams.remapBuf2));
			_wizParams.remapBuf2[_wizParams.remapPos] = a;
			_wizParams.remapBuf1[a] = b;
			++_wizParams.remapPos;
		}
		break;
	case 21:
		_wizParams.processFlags |= 0x200;
		_wizParams.box.bottom = pop();
		_wizParams.box.right = pop();
		_wizParams.box.top = pop();
		_wizParams.box.left = pop();
		break;
	case 31: // HE99+
		_wizParams.processFlags |= 2;
		break;
	case 40: // HE99+
		_wizParams.processFlags |= 0x80;
		pop();
		break;
	case 46:
		_wizParams.processFlags |= 8;
		_wizParams.zoom = pop();
		break;
	case 52:
		_wizParams.processFlags |= 4;
		_wizParams.unk_15C = pop();
		break;
	case 85: // HE99+
		_wizParams.processFlags |= 0x1102;
		_wizParams.processMode = 7;
		break;
	case 87: // HE99+
		_wizParams.processFlags |= 0x60000;
		_wizParams.processMode = 9;
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 88: // HE99+
		_wizParams.processFlags |= 0x60000;
		_wizParams.processMode = 10;
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 89: // HE99+
		_wizParams.processFlags |= 0x60000;
		_wizParams.processMode = 11;
		pop();
		pop();
		pop();
		break;
	case 90: // HE99+
		_wizParams.processFlags |= 0x60000;
		_wizParams.processMode = 12;
		pop();
		pop();
		pop();
		break;
	case 91: // HE99+
		_wizParams.processFlags |= 0x10000;
		pop();
		break;
	case 108:
		_wizParams.processFlags |= 1;
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		break;
	case 171: // HE99+
		_wizParams.processMode = 8;
		break;
	case 200:
		_wizParams.processFlags |= 0x23;
		_wizParams.img.flags |= 0x40;
		_wizParams.unk_160 = _wizParams.img.y1 = _wizParams.img.x1 = pop();
		break;
	case 209:
		processWizImage(&_wizParams);
		break;
	default:
		error("o90_wizImageOps: unhandled case %d", subOp);
	}

	debug(1,"o90_wizImageOps stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown25() {
	int args[16];
	int subOp = fetchScriptByte();
	subOp -= 30;

	debug(1,"o90_unknown25 stub (%d)", subOp);
	switch (subOp) {
	case 0:
		pop();
		break;
	case 1:
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
		pop();
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
	case 9:
		pop();
		break;
	case 12:
		pop();
		pop();
		break;
	case 13:
		pop();
		break;
	case 15:
		if (_heversion == 99) {
			getStackList(args, ARRAYSIZE(args));
			pop();
			pop();
			pop();
			pop();
		} else if (_heversion == 98) {
			pop();
			pop();
			pop();
			pop();
		} else {
			pop();
			pop();
			pop();
		}
		break;
	case 22:
		pop();
		break;
	case 33:
		pop();
		break;
	case 38:
		pop();
		break;
	case 52:
		pop();
		break;
	case 62:
		pop();
		break;
	case 67:
		pop();
		break;
	case 68:
		pop();
		break;
	case 94:
		pop();
		break;
	case 95:
		getStackList(args, ARRAYSIZE(args));
		pop();
		break;
	case 168:
		pop();
		pop();
		break;
	default:
		error("o90_unknown25: Unknown case %d", subOp);
	}
	push(0);


}

void ScummEngine_v90he::o90_unknown26() {
	// Incomplete
	int args[16];
	int subOp = fetchScriptByte();
	subOp -= 34;

	switch (subOp) {
	case 0:
		pop();
		break;
	case 1:
		pop();
		break;
	case 3:
		pop();
		break;
	case 8:
		pop();
		pop();
		break;
	case 9:
		pop();
		break;
	case 10:
		pop();
		pop();
		break;
	case 18:
		pop();
		break;
	case 19:
		pop();
		break;
	case 23:
		pop();
		if (_gameId == GID_FREDDI4 || _heversion >= 99)
			pop();
		break;
	case 28: // HE99+
		pop();
		break;
	case 29:
		pop();
		break;
	case 31:
		pop();
		pop();
		break;
	case 34:
		pop();
		break;
	case 43:
		pop();
		pop();
		break;
	case 48:
		pop();
		break;
	case 52: // HE 98+
		pop();
		break;
	case 58: // HE 99+
		pop();
		break;
	case 63: // HE 98+
		pop();
		break;
	case 64:
		pop();
		break;
	case 90:
		pop();
		break;
	case 91:
		getStackList(args, ARRAYSIZE(args));
		break;
	case 105: // HE99+
		pop();
		pop();
		break;
	case 124:
		break;
	case 164:
		pop();
		pop();
		break;
	case 183:
		break;
	default:
		error("o90_unknown26: Unknown case %d", subOp);
	}
	debug(1,"o90_unknown26 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown27() {
	int subOp = fetchScriptByte();
	switch (subOp) {
	case 8: // HE 99+
		pop();
		break;
	case 30:
		pop();
		break;
	case 31:
		pop();
		break;
	case 42: // HE 99+
		pop();
		pop();
		break;
	case 43:
		pop();
		break;
	case 63: // HE 99+
		pop();
		break;
	case 139: // HE 99+
		pop();
		pop();
		break;
	default:
		error("o90_unknown27: Unknown case %d", subOp);
	}
	push(0);

	debug(1,"o90_unknown27 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown28() {
	// Incomplete
	int subOp = fetchScriptByte();
	subOp -= 37;

	switch (subOp) {
	case 6:
		pop();
		break;
	case 7:
		pop();
		pop();
		break;
	case 20:
		pop();
		break;
	case 28:
		pop();
		pop();
		break;
	case 30:
		pop();
		pop();
		pop();
		pop();
		break;
	case 180:
		break;
	default:
		error("o90_unknown28: Unknown case %d", subOp);
	}
	debug(1,"o90_unknown28 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown29() {
	int state, resId;
	uint32 w, h;
	int16 x, y;

	int subOp = fetchScriptByte();
	subOp -= 30;

	switch (subOp) {
	case 0:
		state = pop();
		resId = pop();
		loadImgSpot(resId, state, x, y);
		push(x);
		break;
	case 1:
		state = pop();
		resId = pop();
		loadImgSpot(resId, state, x, y);
		push(y);
		break;
	case 2:
		state = pop();
		resId = pop();
		getWizImageDim(resId, state, w, h);
		push(w);
		break;
	case 3:
		state = pop();
		resId = pop();
		getWizImageDim(resId, state, w, h);
		push(h);
		break;
	case 6:
		pop();
		push(0);
		break;
	case 15:
		pop();
		pop();
		pop();
		pop();
		push(0);
		break;
	case 36:
		pop();
		pop();
		pop();
		pop();
		push(0);
		break;
	case 100:
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		push(0);
		break;
	default:
		error("o90_unknown29: Unknown case %d", subOp);
	}

	debug(1,"o90_unknown29 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown2F() {
	int subOp = fetchScriptByte();
	subOp -= 54;

	switch (subOp) {
	case 0:
		pop();
		break;
	case 3:
		break;
	case 11:
		pop();
		pop();
		break;
	case 12:
		pop();
		break;
	case 13:
		pop();
		pop();
		pop();
		pop();
		break;
	case 201:
		break;
	default:
		error("o90_unknown2F: Unknown case %d", subOp);
	}
	debug(1,"o90_unknown2F stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown31() {
	int a = pop() << 2;
	push(pop() << a);
}

void ScummEngine_v90he::o90_unknown32() {
	int a = pop() << 2;
	push(pop() >> a);
}

void ScummEngine_v90he::o90_mod() {
	int a = pop();
	if (a == 0)
		error("modulus by zero");
	push(pop() % a);
}

void ScummEngine_v90he::o90_findAllObjectsWithClassOf() {
	int args[16];
	int cond, num, cls, tmp;
	bool b;

	num = getStackList(args, ARRAYSIZE(args));
	int room = pop();
	int j = 1;

	if (room != _currentRoom)
		warning("o90_findAllObjectsWithClassOf: current room is not %d", room);

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, _numLocalObjects + 1);
	for (int i = 1; i < _numLocalObjects; i++) {
		cond = 1;
		tmp = num;
		while (--tmp >= 0) {
			cls = args[tmp];
			b = getClass(i, cls);
			if ((cls & 0x80 && !b) || (!(cls & 0x80) && b))
				cond = 0;
		}

		if (cond)
			writeArray(0, 0, j++, _objs[i].obj_nr);
	}
	writeArray(0, 0, 0, j);
	
	push(readVar(0));
}

void ScummEngine_v90he::o90_getPolygonOverlap() {
	int args1[32];
	int args2[32];

	int n1 = getStackList(args1, ARRAYSIZE(args1));
	getStackList(args2, ARRAYSIZE(args2));

	int subOp = pop();

	switch (subOp) {
	case 1:
		{
			Common::Rect r(args1[0], args1[1], args1[2] + 1, args1[3] + 1);
			Common::Point p(args2[0], args2[1]);
			push(r.contains(p) ? 1 : 0);
		}
		break;
	case 2:
		{
			int dx = args2[0] - args1[0];
			int dy = args2[1] - args1[1];
			int dist = dx * dx + dy * dy;
			if (dist >= 2) {
				dist = (int)sqrt(dist + 1);
			}
			push((dist > args1[2]) ? 1 : 0);
		}
		break;
	case 3:
		{
			Common::Rect r1(args1[0], args1[1], args1[2] + 1, args1[3] + 1);
			Common::Rect r2(args2[0], args2[1], args2[2] + 1, args2[3] + 1);
			push(r2.intersects(r1) ? 1 : 0);			
		}
		break;
	case 4:
		{
			int dx = args2[0] - args1[0];
			int dy = args2[1] - args1[1];
			int dist = dx * dx + dy * dy;
			if (dist >= 2) {
				dist = (int)sqrt(dist + 1);
			}
			push((dist < args1[2] && dist < args2[2]) ? 1 : 0);
		}
		break;
	case 5:
		{
			assert((n1 & 1) == 0);
			n1 /= 2;
			if (n1 == 0) {
				push(0);
			} else {
				WizPolygon wp;
				memset(&wp, 0, sizeof(wp));
				wp.numVerts = n1;
				assert(n1 < ARRAYSIZE(wp.vert));
				for (int i = 0; i < n1; ++i) {
					wp.vert[i].x = args1[i * 2 + 0];
					wp.vert[i].y = args1[i * 2 + 1];
				}
				push(polygonContains(wp, args2[0], args2[1]) ? 1 : 0);
			}
		}
		break;
	// HE 98+
	case 6:
	case 7:
	case 8:
	case 9:
		push(0);
		break;
	default:
		error("o90_getPolygonOverlap: default case %d", subOp);
	}
}

void ScummEngine_v90he::o90_unknown36() {
	int a = pop();
	int b = pop();
	int c = pop();

	if (!c)
		b = a;
	push(b);
}

void ScummEngine_v90he::o90_dim2dim2Array() {
	int data, dim1start, dim1end, dim2start, dim2end;
	int type = fetchScriptByte();

	switch (type) {
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 6:
		data = kDwordArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	default:
		error("o90_dim2dim2Array: default case %d", type);
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

void ScummEngine_v90he::o90_sortArray() {
	// Sorts array via qsort
	int subOp = fetchScriptByte();

	switch (subOp) {
	case 129:
		fetchScriptWord();
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	default:
		error("o90_sortArray: Unknown case %d", subOp);
	}
	debug(1,"o90_sortArray stub (%d)", subOp);
}

void ScummEngine_v90he::o90_getObjectData() {
	// Object releated
	int subOp = fetchScriptByte();
	subOp -= 32;

	switch (subOp) {
	case 0:
		if (_heObjectNum == -1)
			push(0);
		else
			push(_objs[_heObjectNum].width);
		break;
	case 1:
		if (_heObjectNum == -1)
			push(0);
		else
			push(_objs[_heObjectNum].height);
		break;
	case 4:
		push(0);
		break;
	case 6:
		if (_heObjectNum == -1)
			push(0);
		else
			push(_objs[_heObjectNum].x_pos);
		break;
	case 7:
		if (_heObjectNum == -1)
			push(0);
		else
			push(_objs[_heObjectNum].y_pos);
		break;
	case 20:
		push(getState(_heObject));
		break;
	case 25:
		_heObject = pop();
		_heObjectNum = getObjectIndex(_heObject);
		break;
	case 107:
		// Dummy case
		pop();
		push(0);
		break;
	default:
		error("o90_getObjectData: Unknown case %d", subOp);
	}
	debug(1,"o90_getObjectData stub (%d)", subOp);
}

void ScummEngine_v90he::o90_getPaletteData() {
	int subOp = fetchScriptByte();
	subOp -= 45;

	switch (subOp) {
	case 0:
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 7:
		pop();
		pop();
		pop();
		break;
	case 21:
		pop();
		pop();
		break;
	case 87:
		pop();
		pop();
		break;
	case 172:
		pop();
		pop();
		pop();
		break;
	default:
		error("o90_getPaletteData: Unknown case %d", subOp);
	}
	push(0);
	debug(1,"o90_getPaletteData stub (%d)", subOp);
}

void ScummEngine_v90he::o90_paletteOps() {
	int idx, state;

	byte subOp = fetchScriptByte();
	subOp -= 57;

	switch (subOp) {
	case 0:
		_hePaletteNum = pop();
		break;
	case 6:
		{
		state = pop();
		idx = pop();
		const uint8 *dataPtr = getResourceAddress(rtImage, idx);
		const uint8 *pal = findWrappedBlock(MKID('RGBS'), dataPtr, state, 0);
		assert(pal);
		}
		break;
	case 9:
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 13:
		pop();
		pop();
		pop();
		break;
	case 19: //HE99+
		pop();
		break;
	case 29:
		pop();
		break;
	case 118:
		pop();
		pop();
		break;
	case 160:
		break;
	case 198:
		_hePaletteNum = 0;
		break;
	default:
		error("o90_paletteOps: Unknown case %d", subOp);
	}
	debug(0,"o90_paletteOps stub (%d)", subOp);
}



void ScummEngine_v90he::o90_unknownA5() {
	int subOp = fetchScriptByte();

	switch (subOp) {
	case 42:
		if (pop() - 1 == 0)
			pop();
		break;
	case 57:
		break;
	default:
		error("o90_unknownA5: Unknown case %d", subOp);
	}

	push(0);
	debug(1,"o90_unknownA5 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_getActorAnimProgress() {
	Actor *a = derefActor(pop(), "o90_getActorAnimProgress");
	push(a->getAnimProgress());
}

} // End of namespace Scumm
