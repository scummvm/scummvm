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
 * $Header
 *
 */


#include "stdafx.h"

#include "common/config-manager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"
#include "scumm/smush/smush_player.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

namespace Scumm {

#define OPCODE(x)	{ &ScummEngine_v6he::x, #x }

void ScummEngine_v6he::setupOpcodes() {
	static const OpcodeEntryV6he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o6_pushByteVar),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayRead),
		OPCODE(o6_wordArrayRead),
		/* 08 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayIndexedRead),
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
		OPCODE(o6_writeByteVar),
		OPCODE(o6_writeWordVar),
		/* 44 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayWrite),
		OPCODE(o6_wordArrayWrite),
		/* 48 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayIndexedWrite),
		OPCODE(o6_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteVarInc),
		OPCODE(o6_wordVarInc),
		/* 50 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayInc),
		OPCODE(o6_wordArrayInc),
		/* 54 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteVarDec),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayDec),
		OPCODE(o6_wordArrayDec),
		/* 5C */
		OPCODE(o6_if),
		OPCODE(o6_ifNot),
		OPCODE(o6_startScript),
		OPCODE(o6_startScriptQuick),
		/* 60 */
		OPCODE(o6_startObject),
		OPCODE(o6_drawObject),
		OPCODE(o6_drawObjectAt),
		OPCODE(o6_drawBlastObject),
		/* 64 */
		OPCODE(o6_setBlastObjectWindow),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_endCutscene),
		/* 68 */
		OPCODE(o6_cutscene),
		OPCODE(o6_stopMusic),
		OPCODE(o6_freezeUnfreeze),
		OPCODE(o6_cursorCommand),
		/* 6C */
		OPCODE(o6_breakHere),
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_setClass),
		OPCODE(o6_getState),
		/* 70 */
		OPCODE(o6_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o6_startSound),
		OPCODE(o6_stopSound),
		OPCODE(o6_startMusic),
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
		OPCODE(o6_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o6_getActorRoom),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		OPCODE(o6_getObjectOldDir),
		/* 90 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_findInventory),
		OPCODE(o6_getInventoryCount),
		/* 94 */
		OPCODE(o6_getVerbFromXY),
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_setObjectName),
		/* 98 */
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_setBoxFlags),
		OPCODE(o6_createBoxMatrix),
		OPCODE(o6_resourceRoutines),
		/* 9C */
		OPCODE(o6_roomOps),
		OPCODE(o6_actorOps),
		OPCODE(o6_verbOps),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o6_findObject),
		OPCODE(o6_pseudoRoom),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getVerbEntrypoint),
		/* A4 */
		OPCODE(o6_arrayOps),
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o6_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o6_soundKludge),
		OPCODE(o6_isAnyOf),
		OPCODE(o6_quitPauseRestart),
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
		OPCODE(o6_dimArray),
		OPCODE(o6_dummy),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		/* C0 */
		OPCODE(o6_dim2dimArray),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_abs),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* C8 */
		OPCODE(o6_kernelGetFunctions),
		OPCODE(o6_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* D0 */
		OPCODE(o6_getDateTime),
		OPCODE(o6_stopTalking),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o6_shuffle),
		OPCODE(o6_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o6_closeFile),
		OPCODE(o6_openFile),
		OPCODE(o6_readFile),
		/* DC */
		OPCODE(o6_writeFile),
		OPCODE(o6_findAllObjects),
		OPCODE(o6_deleteFile),
		OPCODE(o6_rename),
		/* E0 */
		OPCODE(o6_setVolume),
		OPCODE(o6_unknownE1),
		OPCODE(o6_localizeArray),
		OPCODE(o6_pickVarRandom),
		/* E4 */
		OPCODE(o6_unknownE4),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o6_seekFile),
		OPCODE(o6_unknownEA),
		OPCODE(o6_invalid),
		/* EC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_unknownEE),
		OPCODE(o6_invalid),
		/* F0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_readINI),
		/* F4 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o6_invalid),
		OPCODE(o6_unknownF9),
		OPCODE(o6_unknownFA),
		OPCODE(o6_invalid),
		/* FC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesV6he = opcodes;
}

void ScummEngine_v6he::executeOpcode(byte i) {
	OpcodeProcV6he op = _opcodesV6he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v6he::getOpcodeDesc(byte i) {
	return _opcodesV6he[i].desc;
}

void ScummEngine_v6he::o6_setState() {
	int state = pop();
	int obj = pop();

	if (state & 0x8000) {
		state = state & 0x7F00;
		putState(obj, state);
		return;
	}

	putState(obj, state);
	markObjectRectAsDirty(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void ScummEngine_v6he::o6_startSound() {
	if (_gameId != GID_PUTTDEMO) {
		// Seems to range between 952 - 9000
		int offset = pop();
		debug(2, "o6_startSound: offset %d", offset);
	}

	_sound->addSoundToQueue(pop());
}

void ScummEngine_v6he::o6_roomOps() {
	int a, b, c, d, e;
	byte op;

	op = fetchScriptByte();

	switch (op) {
	case 172:		// SO_ROOM_SCROLL
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

	case 174:		// SO_ROOM_SCREEN
		b = pop();
		a = pop();
		initScreens(a, b);
		break;

	case 175:		// SO_ROOM_PALETTE
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case 176:		// SO_ROOM_SHAKE_ON
		setShake(1);
		break;

	case 177:		// SO_ROOM_SHAKE_OFF
		setShake(0);
		break;

	case 179:		// SO_ROOM_INTENSITY
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;

	case 180:		// SO_ROOM_SAVEGAME
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		break;

	case 181:		// SO_ROOM_FADE
		a = pop();
		if (a) {
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;

	case 182:		// SO_RGB_ROOM_INTENSITY
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case 183:		// SO_ROOM_SHADOW
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setupShadowPalette(a, b, c, d, e);
		break;

	case 184:		// SO_SAVE_STRING
		error("save string not implemented");
		break;

	case 185:		// SO_LOAD_STRING
		error("load string not implemented");
		break;

	case 186:		// SO_ROOM_TRANSFORM
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		palManipulateInit(a, b, c, d);
		break;

	case 187:		// SO_CYCLE_SPEED
		b = pop();
		a = pop();
		checkRange(16, 1, a, "o6_roomOps: 187: color cycle out of range (%d)");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;

	case 213:		// SO_ROOM_NEW_PALETTE
		a = pop();
		setPalette(a);
		break;
	case 220:
		a = pop();
		b = pop();
		warning("o6_roomops:220 (%d, %d): unimplemented", a, b);
		break;
	case 221:
		int len;
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
		_saveLoadFlag = pop();
		_saveLoadSlot = 1;
		_saveTemporaryState = true;
		break;
	default:
		error("o6_roomOps: default case %d", op);
	}
}

void ScummEngine_v6he::o6_actorOps() {
	Actor *a;
	int i, j, k;
	int args[8];
	byte b;

	b = fetchScriptByte();
	if (b == 197) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o6_actorOps");
	if (!a)
		return;

	switch (b) {
	case 76:		// SO_COSTUME
		a->setActorCostume(pop());
		break;
	case 77:		// SO_STEP_DIST
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case 78:		// SO_SOUND
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->sound[i] = args[i];
		break;
	case 79:		// SO_WALK_ANIMATION
		a->walkFrame = pop();
		break;
	case 80:		// SO_TALK_ANIMATION
		a->talkStopFrame = pop();
		a->talkStartFrame = pop();
		break;
	case 81:		// SO_STAND_ANIMATION
		a->standFrame = pop();
		break;
	case 82:		// SO_ANIMATION
		// dummy case in scumm6
		pop();
		pop();
		pop();
		break;
	case 83:		// SO_DEFAULT
		a->initActor(0);
		break;
	case 84:		// SO_ELEVATION
		a->setElevation(pop());
		break;
	case 85:		// SO_ANIMATION_DEFAULT
		a->initFrame = 1;
		a->walkFrame = 2;
		a->standFrame = 3;
		a->talkStartFrame = 4;
		a->talkStopFrame = 5;
		break;
	case 86:		// SO_PALETTE
		j = pop();
		i = pop();
		checkRange(255, 0, i, "Illegal palette slot %d");
		if (_gameId != GID_PUTTDEMO)
			a->remapActorPaletteColor(i, j);
		else
			a->setPalette(i, j);
		break;
	case 87:		// SO_TALK_COLOR
		a->talkColor = pop();
		break;
	case 88:		// SO_ACTOR_NAME
		loadPtrToResource(rtActorName, a->number, NULL);
		break;
	case 89:		// SO_INIT_ANIMATION
		a->initFrame = pop();
		break;
	case 91:		// SO_ACTOR_WIDTH
		a->width = pop();
		break;
	case 92:		// SO_SCALE
		i = pop();
		a->setScale(i, i);
		break;
	case 93:		// SO_NEVER_ZCLIP
		a->forceClip = 0;
		break;
	case 225:		// SO_ALWAYS_ZCLIP
	case 94:		// SO_ALWAYS_ZCLIP
		a->forceClip = pop();
		break;
	case 95:		// SO_IGNORE_BOXES
		a->ignoreBoxes = 1;
		a->forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->room);
		break;
	case 96:		// SO_FOLLOW_BOXES
		a->ignoreBoxes = 0;
		a->forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->room);
		break;
	case 97:		// SO_ANIMATION_SPEED
		a->setAnimSpeed(pop());
		break;
	case 98:		// SO_SHADOW
		a->shadow_mode = pop();
		break;
	case 99:		// SO_TEXT_OFFSET
		a->talkPosY = pop();
		a->talkPosX = pop();
		break;
	case 198:		// SO_ACTOR_VARIABLE
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case 215:		// SO_ACTOR_IGNORE_TURNS_ON
		a->ignoreTurns = true;
		break;
	case 216:		// SO_ACTOR_IGNORE_TURNS_OFF
		a->ignoreTurns = false;
		break;
	case 217:		// SO_ACTOR_NEW
		a->initActor(2);
		break;
	case 218:		
		{
			// TODO: this opcode is used in the putt-putt fun pack, in 'checkers" mini game
			warning("o6_actorOps():218 partially unimplemented");

			int top_actor = a->top;
			int bottom_actor = a->bottom;
			a->forceClip = 1;
			a->needRedraw = true;
			a->drawActorCostume();
			a->forceClip = 0;
			a->needRedraw = true;
			a->drawActorCostume();
			a->needRedraw = false;

			if (a->top > top_actor)
				a->top = top_actor;
			if (a->bottom < bottom_actor)
				a->bottom = bottom_actor;

			//FIXME Trigger redraw
			a->bottom = top_actor;
		}
		break;
	default:
		error("o6_actorOps: default case %d", b);
	}
}

void ScummEngine_v6he::o6_verbOps() {
	int slot, a, b;
	VerbSlot *vs;
	byte op;

	op = fetchScriptByte();
	if (op == 196) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		checkRange(_numVerbs - 1, 0, _curVerbSlot, "Illegal new verb slot %d");
		return;
	}
	vs = &_verbs[_curVerbSlot];
	slot = _curVerbSlot;
	switch (op) {
	case 124:		// SO_VERB_IMAGE
		a = pop();
		if (_curVerbSlot) {
			setVerbObject(_roomResource, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case 125:		// SO_VERB_NAME
		loadPtrToResource(rtVerb, slot, NULL);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 126:		// SO_VERB_COLOR
		vs->color = pop();
		break;
	case 127:		// SO_VERB_HICOLOR
		vs->hicolor = pop();
		break;
	case 128:		// SO_VERB_AT
		vs->curRect.top = pop();
		vs->curRect.left = pop();
		break;
	case 129:		// SO_VERB_ON
		vs->curmode = 1;
		break;
	case 130:		// SO_VERB_OFF
		vs->curmode = 0;
		break;
	case 131:		// SO_VERB_DELETE
		slot = getVerbSlot(pop(), 0);
		killVerb(slot);
		break;
	case 132:		// SO_VERB_NEW
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
		vs->charset_nr = _string[0].t_charset;
		vs->curmode = 0;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		break;
	case 133:		// SO_VERB_DIMCOLOR
		vs->dimcolor = pop();
		break;
	case 134:		// SO_VERB_DIM
		vs->curmode = 2;
		break;
	case 135:		// SO_VERB_KEY
		vs->key = pop();
		break;
	case 136:		// SO_VERB_CENTER
		vs->center = 1;
		break;
	case 137:		// SO_VERB_NAME_STR
		a = pop();
		if (a == 0) {
			loadPtrToResource(rtVerb, slot, (const byte *)"");
		} else {
			loadPtrToResource(rtVerb, slot, getStringAddress(a));
		}
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 139:		// SO_VERB_IMAGE_IN_ROOM
		b = pop();
		a = pop();
		if (slot && a != vs->imgindex) {
			setVerbObject(b, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case 140:		// SO_VERB_BAKCOLOR
		vs->bkcolor = pop();
		break;
	case 255:
		drawVerb(slot, 0);
		verbMouseOver(0);
		break;
	default:
		error("o6_verbops: default case %d", op);
	}
}

void ScummEngine_v6he::o6_wait() {
	int actnum;
	int offs = -2;
	Actor *a;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 168:		// SO_WAIT_FOR_ACTOR Wait for actor
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o6_wait:168");
		if (a->moving)
			break;
		return;
	case 169:		// SO_WAIT_FOR_MESSAGE Wait for message
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case 170:		// SO_WAIT_FOR_CAMERA Wait for camera
		if (camera._cur.x / 8 != camera._dest.x / 8)
			break;
		return;
	case 171:		// SO_WAIT_FOR_SENTENCE
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
			break;
		}
		if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;
		break;
	default:
		error("o6_wait: default case 0x%x", subOp);
	}

	_scriptPointer += offs;
	o6_breakHere();
}

void ScummEngine_v6he::o6_soundKludge() {
	int list[16];
	getStackList(list, ARRAYSIZE(list));
}

void ScummEngine_v6he::o6_dummy() {
	stopObjectCode();
}

void ScummEngine_v6he::o6_drawBlastObject() {
	int args[16];
	getStackList(args, ARRAYSIZE(args));
	pop();
	pop();
	pop();
	pop();
	pop();
}

void ScummEngine_v6he::o6_setBlastObjectWindow() {
	pop();
	pop();
	pop();
	pop();
}

void ScummEngine_v6he::o6_kernelSetFunctions() {
	int args[29];
	int num;

	num = getStackList(args, ARRAYSIZE(args));

		switch (args[0]) {
		case 1:
			// Used to restore images when decorating cake in
			// Fatty Bear's Birthday Surprise
			warning("o6_kernelSetFunctions: stub1()");
			break;
		default:
			error("o6_kernelSetFunctions: default case %d (param count %d)", args[0], num);
			break;
		}
}

void ScummEngine_v6he::o6_kernelGetFunctions() {
	int args[29];
	getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 1:
		// Used to store images when decorating cake in
		// Fatty Bear's Birthday Surprise
		warning("o6_kernelGetFunctions: stub1()");
		push(0);
		break;
	default:
		error("o6_kernelGetFunctions: default case %d", args[0]);
	}
}

void ScummEngine_v6he::o6_stampObject() {
	int object, x, y, state;
	
	state = pop();
	if (state == 0) {
		state = 1;
	}
	y = pop();
	x = pop();
	object = pop();
	int objnum = getObjectIndex(object);
	if (objnum == -1)
		return;
	_objs[objnum].x_pos = x * 8;
	_objs[objnum].y_pos = y * 8;
	putState(object, state);
	drawObject(objnum, 0);
}

void ScummEngine_v6he::o6_openFile() {
	int mode, len, slot, l, r;
	byte filename[100];

	_msgPtrToAdd = filename;
	_messagePtr = _scriptPointer;
	addMessageToStack(_messagePtr);

	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;
	
	for (r = strlen((char*)filename); r != 0; r--) {
		if (filename[r - 1] == '\\')
			break;
	}
	
	mode = pop();
	slot = -1;
	for (l = 0; l < 17; l++) {
		if (_hFileTable[l].isOpen() == false) {
			slot = l;
			break;
		}
	}

	if (slot != -1) {
		if (mode == 1)
			_hFileTable[slot].open((char*)filename + r, getGameDataPath(), File::kFileReadMode);
		else if (mode == 2)
			_hFileTable[slot].open((char*)filename + r, getGameDataPath(), File::kFileWriteMode);
		else
			error("o6_openFile(): wrong open file mode");

		warning("%d = o6_openFile(\"%s\", %d)", slot, filename + r, mode);
	}
	push(slot);
}

void ScummEngine_v6he::o6_closeFile() {
	int slot = pop();
	_hFileTable[slot].close();
	warning("o6_closeFile(%d)", slot);
}

void ScummEngine_v6he::o6_deleteFile() {
	int len, r;
	byte filename[100];

	_msgPtrToAdd = filename;
	_messagePtr = _scriptPointer;
	addMessageToStack(_messagePtr);

	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	for (r = strlen((char*)filename); r != 0; r--) {
		if (filename[r - 1] == '\\')
			break;
	}

	warning("stub o6_deleteFile(\"%s\")", filename + r);
}

void ScummEngine_v6he::o6_rename() {
	int len, r1, r2;
	byte filename[100],filename2[100];

	_msgPtrToAdd = filename;
	_messagePtr = _scriptPointer;
	addMessageToStack(_messagePtr);

	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	for (r1 = strlen((char*)filename); r1 != 0; r1--) {
		if (filename[r1 - 1] == '\\')
			break;
	}

	_msgPtrToAdd = filename2;
	_messagePtr = _scriptPointer;
	addMessageToStack(_messagePtr);

	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	for (r2 = strlen((char*)filename2); r2 != 0; r2--) {
		if (filename2[r2 - 1] == '\\')
			break;
	}

	warning("stub o6_rename(\"%s\" to \"%s\")", filename + r1, filename2 + r2);
}

int ScummEngine_v6he::readFileToArray(int slot, int32 size) {
	if (size == 0)
		size = _hFileTable[slot].size() - _hFileTable[slot].pos();
	writeVar(0, 0);
	defineArray(0, 3, 0, size);
	byte *ptr = getResourceAddress(rtString, readVar(0));
	_hFileTable[slot].read(ptr, size);
	return readVar(0);
}

void ScummEngine_v6he::o6_readFile() {
	int32 size = pop();
	int slot = pop();

	if (size == -2) {
		push(_hFileTable[slot].readUint16LE());
	} else if (size == -1) {
		push(_hFileTable[slot].readByte());
	} else {
		push(readFileToArray(slot, size));
	}
	warning("o6_readFile(%d, %d)", slot, size);
}

void ScummEngine_v6he::writeFileFromArray(int slot, int resID) {
	byte *ptr = getResourceAddress(rtString, resID);
	// FIXME: hack for proper size: / 2 - 5
	int32 size = getResourceSize(rtString, resID) / 2 - 5;
	_hFileTable[slot].write(ptr, size);
}

void ScummEngine_v6he::o6_writeFile() {
	int32 size = pop();
	int16 resID = pop();
	int slot = pop();

	if (size == -2) {
		_hFileTable[slot].writeUint16LE(resID);
	} else if (size == -1) {
		_hFileTable[slot].writeByte(resID);
	} else {
		writeFileFromArray(slot, resID);
	}
	warning("o6_writeFile(%d, %d)", slot, resID);
}

void ScummEngine_v6he::o6_setVolume() {
	byte subOp = fetchScriptByte();
	int soundVolumeMaster;
	int volume = pop();
	switch (subOp) {
	case 222:
		if (_imuse)
			_imuse->set_music_volume(volume);
		else
			_mixer->setMusicVolume(volume);
		break;
	case 224:
		soundVolumeMaster = ConfMan.getInt("master_volume");
		_mixer->setVolume(volume * soundVolumeMaster / 255);
		break;
	}
}

void ScummEngine_v6he::o6_unknownFA() {
	int len, a = fetchScriptByte();
	
	len = resStrLen(_scriptPointer);
	warning("stub o6_unknownFA(%d, \"%s\")", a, _scriptPointer);
	_scriptPointer += len + 1;
}

void ScummEngine_v6he::o6_seekFile() {
	int a, b, c;
	a = pop();
	b = pop();
	c = pop();

	switch (a) {
	case 1:
		//seekWrapper(c, b, 0, 0);
		break;
	case 2:
		//seekWrapper(c, b, ?, 1);
		break;
	default:
		break;
	}

	warning("stub o6_seekFile(%d, %d, %d)", a, b, c);
}

void ScummEngine_v6he::o6_unknownEA() {
	int edi, esi, eax;
	edi = pop();
	esi = pop();

	if (edi ==  0) {
		eax = esi;
		esi = edi;
		edi = eax;
	}

	eax = fetchScriptByte();
	switch (eax) {
	case 199:
		unknownEA_func(5, esi, edi, fetchScriptWord(), eax);
		break;
	case 202:
		unknownEA_func(3, esi, edi, fetchScriptWord(), eax);
		break;
	default:
		break;
	}
}

void ScummEngine_v6he::unknownEA_func(int a, int b, int c, int d, int e) {
	// Used in mini game at Cosmic Dust Diner in puttmoon
	warning("unknownEA_func(%d, %d, %d, %d, %d) stub", a, b, c, d, e);
}

void ScummEngine_v6he::o6_unknownEE() {
	int a;
	a = pop();
	loadPtrToResource(rtVerb, _curVerbSlot, getStringAddress(a));
}

void ScummEngine_v6he::o6_readINI() {
	int len;

	len = resStrLen(_scriptPointer);
	warning("stub o6_readINI(\"%s\")", _scriptPointer);
	_scriptPointer += len + 1;
	pop();
	push(0);
	
}

void ScummEngine_v6he::o6_localizeArray() {
	int stringID = pop();

	if (stringID < _numArray) {
		_baseArrays[stringID][0] = (byte)_currentScript;
	} else {
		warning("o6_localizeArray(%d): too big scriptID", stringID);
	}
}

void ScummEngine_v6he::o6_unknownF9() {
	// File related
	int len, r;
	byte filename[100];

	_msgPtrToAdd = filename;
	_messagePtr = _scriptPointer;
	addMessageToStack(_messagePtr);

	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	for (r = strlen((char*)filename); r != 0; r--) {
		if (filename[r - 1] == '\\')
			break;
	}

	warning("stub o6_unknownF9(\"%s\")", filename + r);
}

void ScummEngine_v6he::decodeParseString(int m, int n) {
	byte b;
	int c;

	b = fetchScriptByte();

	switch (b) {
	case 65:		// SO_AT
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case 66:		// SO_COLOR
		_string[m].color = pop();
		break;
	case 67:		// SO_CLIPPED
		_string[m].right = pop();
		break;
	case 69:		// SO_CENTER
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case 71:		// SO_LEFT
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case 72:		// SO_OVERHEAD
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case 73:		// SO_SAY_VOICE
		error("decodeParseString: case 73");
		break;
	case 74:		// SO_MUMBLE
		_string[m].no_talk_anim = true;
		break;
	case 75:		// SO_TEXTSTRING
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
		}
		return;
	case 0xF9:
		c = pop();
		if (c == 1) {
			_string[m].color = pop();
		} else {	
			push(c);
			int args[16];
			getStackList(args, ARRAYSIZE(args));
		}
		warning("decodeParseString case 0xF9 stub");
		return;
	case 0xFE:
		setStringVars(m);
		if (n)
			_actorToPrintStrFor = pop();
		return;
	case 0xFF:
		_string[m].t_xpos = _string[m].xpos;
		_string[m].t_ypos = _string[m].ypos;
		_string[m].t_center = _string[m].center;
		_string[m].t_overhead = _string[m].overhead;
		_string[m].t_no_talk_anim = _string[m].no_talk_anim;
		_string[m].t_right = _string[m].right;
		_string[m].t_color = _string[m].color;
		_string[m].t_charset = _string[m].charset;
		return;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

} // End of namespace Scumm
