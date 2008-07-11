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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

#include "common/savefile.h"

namespace Scumm {

#define OPCODE(x)	_OPCODE(ScummEngine_v5, x)

void ScummEngine_v5::setupOpcodes() {
	static const OpcodeEntryV5 opcodes[256] = {
		/* 00 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o5_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 04 */
		OPCODE(o5_isGreaterEqual),
		OPCODE(o5_drawObject),
		OPCODE(o5_getActorElevation),
		OPCODE(o5_setState),
		/* 08 */
		OPCODE(o5_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getVerbEntrypoint),
		/* 0C */
		OPCODE(o5_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_getObjectState),
		/* 10 */
		OPCODE(o5_getObjectOwner),
		OPCODE(o5_animateActor),
		OPCODE(o5_panCameraTo),
		OPCODE(o5_actorOps),
		/* 14 */
		OPCODE(o5_print),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o5_and),
		/* 18 */
		OPCODE(o5_jumpRelative),
		OPCODE(o5_doSentence),
		OPCODE(o5_move),
		OPCODE(o5_multiply),
		/* 1C */
		OPCODE(o5_startSound),
		OPCODE(o5_ifClassOfIs),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_isActorInBox),
		/* 20 */
		OPCODE(o5_stopMusic),
		OPCODE(o5_putActor),
		OPCODE(o5_getAnimCounter),
		OPCODE(o5_getActorY),
		/* 24 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_setVarRange),
		OPCODE(o5_stringOps),
		/* 28 */
		OPCODE(o5_equalZero),
		OPCODE(o5_setOwnerOf),
		OPCODE(o5_startScript),
		OPCODE(o5_delayVariable),
		/* 2C */
		OPCODE(o5_cursorCommand),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_delay),
		OPCODE(o5_ifNotState),
		/* 30 */
		OPCODE(o5_matrixOps),
		OPCODE(o5_getInventoryCount),
		OPCODE(o5_setCameraAt),
		OPCODE(o5_roomOps),
		/* 34 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o5_startObject),
		/* 38 */
		OPCODE(o5_lessOrEqual),
		OPCODE(o5_doSentence),
		OPCODE(o5_subtract),
		OPCODE(o5_getActorScale),
		/* 3C */
		OPCODE(o5_stopSound),
		OPCODE(o5_findInventory),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox),
		/* 40 */
		OPCODE(o5_cutscene),
		OPCODE(o5_putActor),
		OPCODE(o5_chainScript),
		OPCODE(o5_getActorX),
		/* 44 */
		OPCODE(o5_isLess),
		OPCODE(o5_drawObject),
		OPCODE(o5_increment),
		OPCODE(o5_setState),
		/* 48 */
		OPCODE(o5_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getVerbEntrypoint),
		/* 4C */
		OPCODE(o5_soundKludge),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_ifState),
		/* 50 */
		OPCODE(o5_pickupObjectOld),
		OPCODE(o5_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o5_actorOps),
		/* 54 */
		OPCODE(o5_setObjectName),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o5_or),
		/* 58 */
		OPCODE(o5_beginOverride),
		OPCODE(o5_doSentence),
		OPCODE(o5_add),
		OPCODE(o5_divide),
		/* 5C */
		OPCODE(o5_oldRoomEffect),
		OPCODE(o5_setClass),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_isActorInBox),
		/* 60 */
		OPCODE(o5_freezeScripts),
		OPCODE(o5_putActor),
		OPCODE(o5_stopScript),
		OPCODE(o5_getActorFacing),
		/* 64 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_getClosestObjActor),
		OPCODE(o5_getStringWidth),
		/* 68 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o5_setOwnerOf),
		OPCODE(o5_startScript),
		OPCODE(o5_debug),
		/* 6C */
		OPCODE(o5_getActorWidth),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_stopObjectScript),
		OPCODE(o5_ifNotState),
		/* 70 */
		OPCODE(o5_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o5_roomOps),
		/* 74 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o5_startObject),
		/* 78 */
		OPCODE(o5_isGreater),
		OPCODE(o5_doSentence),
		OPCODE(o5_verbOps),
		OPCODE(o5_getActorWalkBox),
		/* 7C */
		OPCODE(o5_isSoundRunning),
		OPCODE(o5_findInventory),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox),
		/* 80 */
		OPCODE(o5_breakHere),
		OPCODE(o5_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 84 */
		OPCODE(o5_isGreaterEqual),
		OPCODE(o5_drawObject),
		OPCODE(o5_getActorElevation),
		OPCODE(o5_setState),
		/* 88 */
		OPCODE(o5_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getVerbEntrypoint),
		/* 8C */
		OPCODE(o5_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_getObjectState),
		/* 90 */
		OPCODE(o5_getObjectOwner),
		OPCODE(o5_animateActor),
		OPCODE(o5_panCameraTo),
		OPCODE(o5_actorOps),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o5_and),
		/* 98 */
		OPCODE(o5_systemOps),
		OPCODE(o5_doSentence),
		OPCODE(o5_move),
		OPCODE(o5_multiply),
		/* 9C */
		OPCODE(o5_startSound),
		OPCODE(o5_ifClassOfIs),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_isActorInBox),
		/* A0 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o5_putActor),
		OPCODE(o5_getAnimCounter),
		OPCODE(o5_getActorY),
		/* A4 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_setVarRange),
		OPCODE(o5_saveLoadVars),
		/* A8 */
		OPCODE(o5_notEqualZero),
		OPCODE(o5_setOwnerOf),
		OPCODE(o5_startScript),
		OPCODE(o5_saveRestoreVerbs),
		/* AC */
		OPCODE(o5_expression),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_wait),
		OPCODE(o5_ifNotState),
		/* B0 */
		OPCODE(o5_matrixOps),
		OPCODE(o5_getInventoryCount),
		OPCODE(o5_setCameraAt),
		OPCODE(o5_roomOps),
		/* B4 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o5_startObject),
		/* B8 */
		OPCODE(o5_lessOrEqual),
		OPCODE(o5_doSentence),
		OPCODE(o5_subtract),
		OPCODE(o5_getActorScale),
		/* BC */
		OPCODE(o5_stopSound),
		OPCODE(o5_findInventory),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox),
		/* C0 */
		OPCODE(o5_endCutscene),
		OPCODE(o5_putActor),
		OPCODE(o5_chainScript),
		OPCODE(o5_getActorX),
		/* C4 */
		OPCODE(o5_isLess),
		OPCODE(o5_drawObject),
		OPCODE(o5_decrement),
		OPCODE(o5_setState),
		/* C8 */
		OPCODE(o5_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getVerbEntrypoint),
		/* CC */
		OPCODE(o5_pseudoRoom),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_ifState),
		/* D0 */
		OPCODE(o5_pickupObjectOld),
		OPCODE(o5_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o5_actorOps),
		/* D4 */
		OPCODE(o5_setObjectName),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o5_or),
		/* D8 */
		OPCODE(o5_printEgo),
		OPCODE(o5_doSentence),
		OPCODE(o5_add),
		OPCODE(o5_divide),
		/* DC */
		OPCODE(o5_oldRoomEffect),
		OPCODE(o5_setClass),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_isActorInBox),
		/* E0 */
		OPCODE(o5_freezeScripts),
		OPCODE(o5_putActor),
		OPCODE(o5_stopScript),
		OPCODE(o5_getActorFacing),
		/* E4 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_getClosestObjActor),
		OPCODE(o5_getStringWidth),
		/* E8 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o5_setOwnerOf),
		OPCODE(o5_startScript),
		OPCODE(o5_debug),
		/* EC */
		OPCODE(o5_getActorWidth),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_stopObjectScript),
		OPCODE(o5_ifNotState),
		/* F0 */
		OPCODE(o5_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o5_roomOps),
		/* F4 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o5_startObject),
		/* F8 */
		OPCODE(o5_isGreater),
		OPCODE(o5_doSentence),
		OPCODE(o5_verbOps),
		OPCODE(o5_getActorWalkBox),
		/* FC */
		OPCODE(o5_isSoundRunning),
		OPCODE(o5_findInventory),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox)
	};

	_opcodesV5 = opcodes;
}

#define PARAM_1 0x80
#define PARAM_2 0x40
#define PARAM_3 0x20

void ScummEngine_v5::executeOpcode(byte i) {
	OpcodeProcV5 op = _opcodesV5[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v5::getOpcodeDesc(byte i) {
	return _opcodesV5[i].desc;
}

int ScummEngine_v5::getVar() {
	return readVar(fetchScriptWord());
}

int ScummEngine_v5::getVarOrDirectByte(byte mask) {
	if (_opcode & mask)
		return getVar();
	return fetchScriptByte();
}

int ScummEngine_v5::getVarOrDirectWord(byte mask) {
	if (_opcode & mask)
		return getVar();
	return (int16)fetchScriptWord();
}

void ScummEngine_v5::o5_actorFollowCamera() {
	actorFollowCamera(getVarOrDirectByte(0x80));
}

void ScummEngine_v5::o5_actorFromPos() {
	int x, y;
	getResultPos();
	x = getVarOrDirectWord(PARAM_1);
	y = getVarOrDirectWord(PARAM_2);
	setResult(getActorFromPos(x, y));
}

void ScummEngine_v5::o5_actorOps() {
	static const byte convertTable[20] =
		{ 1, 0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20 };
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_actorOps");
	int i, j;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		if (_game.features & GF_SMALL_HEADER)
			_opcode = (_opcode & 0xE0) | convertTable[(_opcode & 0x1F) - 1];

		switch (_opcode & 0x1F) {
		case 0:										/* dummy case */
			getVarOrDirectByte(PARAM_1);
			break;
		case 1:			// SO_COSTUME
			a->setActorCostume(getVarOrDirectByte(PARAM_1));
			break;
		case 2:			// SO_STEP_DIST
			i = getVarOrDirectByte(PARAM_1);
			j = getVarOrDirectByte(PARAM_2);
			a->setActorWalkSpeed(i, j);
			break;
		case 3:			// SO_SOUND
			a->_sound[0] = getVarOrDirectByte(PARAM_1);
			break;
		case 4:			// SO_WALK_ANIMATION
			a->_walkFrame = getVarOrDirectByte(PARAM_1);
			break;
		case 5:			// SO_TALK_ANIMATION
			a->_talkStartFrame = getVarOrDirectByte(PARAM_1);
			a->_talkStopFrame = getVarOrDirectByte(PARAM_2);
			break;
		case 6:			// SO_STAND_ANIMATION
			a->_standFrame = getVarOrDirectByte(PARAM_1);
			break;
		case 7:			// SO_ANIMATION
			getVarOrDirectByte(PARAM_1);
			getVarOrDirectByte(PARAM_2);
			getVarOrDirectByte(PARAM_3);
			break;
		case 8:			// SO_DEFAULT
			a->initActor(0);
			break;
		case 9:			// SO_ELEVATION
			a->setElevation(getVarOrDirectWord(PARAM_1));
			break;
		case 10:		// SO_ANIMATION_DEFAULT
			a->_initFrame = 1;
			a->_walkFrame = 2;
			a->_standFrame = 3;
			a->_talkStartFrame = 4;
			a->_talkStopFrame = 5;
			break;
		case 11:		// SO_PALETTE
			i = getVarOrDirectByte(PARAM_1);
			j = getVarOrDirectByte(PARAM_2);
			assertRange(0, i, 31, "o5_actorOps: palette slot");
			a->setPalette(i, j);
			break;
		case 12:		// SO_TALK_COLOR
			a->_talkColor = getVarOrDirectByte(PARAM_1);
			break;
		case 13:		// SO_ACTOR_NAME
			loadPtrToResource(rtActorName, a->_number, NULL);
			break;
		case 14:		// SO_INIT_ANIMATION
			a->_initFrame = getVarOrDirectByte(PARAM_1);
			break;
		case 16:		// SO_ACTOR_WIDTH
			a->_width = getVarOrDirectByte(PARAM_1);
			break;
		case 17:		// SO_ACTOR_SCALE
			if (_game.version == 4) {
				i = j = getVarOrDirectByte(PARAM_1);
			} else {
				i = getVarOrDirectByte(PARAM_1);
				j = getVarOrDirectByte(PARAM_2);
			}

			a->_boxscale = i;
			a->setScale(i, j);
			break;
		case 18:		// SO_NEVER_ZCLIP
			a->_forceClip = 0;
			break;
		case 19:		// SO_ALWAYS_ZCLIP
			a->_forceClip = getVarOrDirectByte(PARAM_1);
			break;
		case 20:		// SO_IGNORE_BOXES
		case 21:		// SO_FOLLOW_BOXES
			a->_ignoreBoxes = !(_opcode & 1);
			a->_forceClip = 0;
			if (a->isInCurrentRoom())
				a->putActor();
			break;

		case 22:		// SO_ANIMATION_SPEED
			a->setAnimSpeed(getVarOrDirectByte(PARAM_1));
			break;
		case 23:		// SO_SHADOW
			a->_shadowMode = getVarOrDirectByte(PARAM_1);
			break;
		default:
			error("o5_actorOps: default case %d", _opcode & 0x1F);
		}
	}
}

void ScummEngine_v5::o5_setClass() {
	int obj = getVarOrDirectWord(PARAM_1);
	int newClass;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		newClass = getVarOrDirectWord(PARAM_1);
		if (newClass == 0) {
			// Class '0' means: clean all class data
			_classData[obj] = 0;
			if ((_game.features & GF_SMALL_HEADER) && obj <= _numActors) {
				Actor *a = derefActor(obj, "o5_setClass");
				a->_ignoreBoxes = false;
				a->_forceClip = 0;
			}
		} else
			putClass(obj, newClass, (newClass & 0x80) ? true : false);
	}
}

void ScummEngine_v5::o5_add() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);

	// WORKAROUND bug #770065: This works around a script bug in LoomCD. To
	// understand the reasoning behind this, compare script 210 and 218 in
	// room 20. Apparently they made a mistake when converting the absolute
	// delays into relative ones.
	if (_game.id == GID_LOOM && _game.version == 4 && vm.slot[_currentScript].number == 210 && _currentRoom == 20 && _resultVarNumber == 0x4000) {
		switch (a) {
		// Fix for the Var[250] == 11 case
		case 138:
			a = 145;
			break;
		case 324:
			a = 324 - 138;
			break;
		// Fixes for the Var[250] == 14 case
		case 130:
			a = 170;
			break;
		case 342:
			a = 342 - 130 + 15;	// Small extra adjustment for the "OUCH"
			break;
		case 384:
			a -= 342;
			break;
		case 564:
			a -= 384;
			break;
		}
	}

	setResult(readVar(_resultVarNumber) + a);
}

void ScummEngine_v5::o5_and() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	setResult(readVar(_resultVarNumber) & a);
}

void ScummEngine_v5::o5_animateActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int anim = getVarOrDirectByte(PARAM_2);

	// WORKAROUND bug #820357: This seems to be yet another script bug which
	// the original engine let slip by. For details, refer to the tracker item.
	if (_game.id == GID_INDY4 && vm.slot[_currentScript].number == 206 && _currentRoom == 17 && (act == 31 || act == 86)) {
		return;
	}

	// WORKAROUND bug #859513: While on mars, going outside without your helmet
	// (or missing some other part of your "space suite" will cause your
	// character to complain ("I can't breathe."). Unfortunately, this is
	// coupled with an animate command, making it very difficult to return to
	// safety (from where you came). The following hack works around this by
	// ignoring that particular turn command.
	if (_game.id == GID_ZAK && _currentRoom == 182 && anim == 246 &&
			((_game.version < 3 && vm.slot[_currentScript].number == 82)
			|| (_game.version == 3 && vm.slot[_currentScript].number == 131))) {
		return;
	}

	Actor *a = derefActor(act, "o5_animateActor");
	a->animateActor(anim);
}

void ScummEngine_v5::o5_breakHere() {
	updateScriptPtr();
	_currentScript = 0xFF;
}

void ScummEngine_v5::o5_chainScript() {
	int vars[16];
	int script;
	int cur;

	script = getVarOrDirectByte(PARAM_1);

	getWordVararg(vars);

	cur = _currentScript;

	// WORKAROUND bug #743314: Work around a bug in script 33 in Indy3 VGA.
	// That script is used for the fist fights in the Zeppelin. It uses
	// Local[5], even though that is never set to any value. But script 33 is
	// called via chainScript by script 32, and in there Local[5] is defined
	// to the  actor ID of the opposing soldier. So, we copy that value over
	// to the Local[5] variable of script 33.
	// FIXME: This workaround is meant for Indy3 VGA, but we make no checks
	// to exclude the EGA/Mac/FM-TOWNS versions. Maybe we should check those
	// (by looking for GF_OLD256 && !platform=FM-TOWNS
	if (_game.id == GID_INDY3 && vm.slot[cur].number == 32 && script == 33) {
		vars[5] = vm.localvar[cur][5];
	}

	vm.slot[cur].number = 0;
	vm.slot[cur].status = ssDead;
	_currentScript = 0xFF;

	runScript(script, vm.slot[cur].freezeResistant, vm.slot[cur].recursive, vars);
}

void ScummEngine_v5::o5_cursorCommand() {
	int i, j, k;
	int table[16];
	switch ((_opcode = fetchScriptByte()) & 0x1F) {
	case 1:			// SO_CURSOR_ON
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 2:			// SO_CURSOR_OFF
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case 3:			// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 4:			// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 5:			// SO_CURSOR_SOFT_ON
		_cursor.state++;
		verbMouseOver(0);
		break;
	case 6:			// SO_CURSOR_SOFT_OFF
		_cursor.state--;
		verbMouseOver(0);
		break;
	case 7:			// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 8:			// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	case 10:		// SO_CURSOR_IMAGE
		i = getVarOrDirectByte(PARAM_1);	// Cursor number
		j = getVarOrDirectByte(PARAM_2);	// Charset letter to use
		redefineBuiltinCursorFromChar(i, j);
		break;
	case 11:		// SO_CURSOR_HOTSPOT
		i = getVarOrDirectByte(PARAM_1);
		j = getVarOrDirectByte(PARAM_2);
		k = getVarOrDirectByte(PARAM_3);
		redefineBuiltinCursorHotspot(i, j, k);
		break;
	case 12:		// SO_CURSOR_SET
		i = getVarOrDirectByte(PARAM_1);
		if (i >= 0 && i <= 3)
			_currentCursor = i;
		else
			error("SO_CURSOR_SET: unsupported cursor id %d", i);
		break;
	case 13:		// SO_CHARSET_SET
		initCharset(getVarOrDirectByte(PARAM_1));
		break;
	case 14:											/* unk */
		if (_game.version == 3) {
			/*int a = */ getVarOrDirectByte(PARAM_1);
			/*int b = */ getVarOrDirectByte(PARAM_2);
			// This is some kind of "init charset" opcode. However, we don't have to do anything
			// in here, as our initCharset automatically calls loadCharset for GF_SMALL_HEADER,
			// games if needed.
		} else {
			getWordVararg(table);
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)table[i];
		}
		break;
	}

	if (_game.version >= 4) {
		VAR(VAR_CURSORSTATE) = _cursor.state;
		VAR(VAR_USERPUT) = _userPut;
	}
}

void ScummEngine_v5::o5_cutscene() {
	int args[16];
	getWordVararg(args);
	beginCutscene(args);
}

void ScummEngine_v5::o5_endCutscene() {
	endCutscene();
}

void ScummEngine_v5::o5_debug() {
	int a = getVarOrDirectWord(PARAM_1);
	debugC(DEBUG_GENERAL, "o5_debug(%d)", a);
}

void ScummEngine_v5::o5_decrement() {
	getResultPos();
	setResult(readVar(_resultVarNumber) - 1);
}

void ScummEngine_v5::o5_delay() {
	int delay = fetchScriptByte();
	delay |= fetchScriptByte() << 8;
	delay |= fetchScriptByte() << 16;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	o5_breakHere();
}

void ScummEngine_v5::o5_delayVariable() {
	vm.slot[_currentScript].delay = getVar();
	vm.slot[_currentScript].status = ssPaused;
	o5_breakHere();
}

void ScummEngine_v5::o5_divide() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	if (a == 0) {
		error("Divide by zero");
		setResult(0);
	} else
		setResult(readVar(_resultVarNumber) / a);
}

void ScummEngine_v5::o5_doSentence() {
	int verb;

	verb = getVarOrDirectByte(PARAM_1);
	if (verb == 0xFE) {
		_sentenceNum = 0;
		stopScript(VAR(VAR_SENTENCE_SCRIPT));
		clearClickedStatus();
		return;
	}

	int objectA = getVarOrDirectWord(PARAM_2);
	int objectB = getVarOrDirectWord(PARAM_3);
	doSentence(verb, objectA, objectB);
}

void ScummEngine_v5::o5_drawBox() {
	int x, y, x2, y2, color;

	x = getVarOrDirectWord(PARAM_1);
	y = getVarOrDirectWord(PARAM_2);

	_opcode = fetchScriptByte();
	x2 = getVarOrDirectWord(PARAM_1);
	y2 = getVarOrDirectWord(PARAM_2);
	color = getVarOrDirectByte(PARAM_3);

	drawBox(x, y, x2, y2, color);
}

void ScummEngine_v5::o5_drawObject() {
	int state, obj, idx, i;
	ObjectData *od;
	uint16 x, y, w, h;
	int xpos, ypos;

	state = 1;
	xpos = ypos = 255;
	obj = getVarOrDirectWord(PARAM_1);

	if (_game.features & GF_SMALL_HEADER) {
		xpos = getVarOrDirectWord(PARAM_2);
		ypos = getVarOrDirectWord(PARAM_3);
	} else {
		_opcode = fetchScriptByte();
		switch (_opcode & 0x1F) {
		case 1:										/* draw at */
			xpos = getVarOrDirectWord(PARAM_1);
			ypos = getVarOrDirectWord(PARAM_2);
			break;
		case 2:										/* set state */
			state = getVarOrDirectWord(PARAM_1);
			break;
		case 0x1F:									/* neither */
			break;
		default:
			error("o5_drawObject: unknown subopcode %d", _opcode & 0x1F);
		}
	}

	idx = getObjectIndex(obj);
	if (idx == -1)
		return;

	od = &_objs[idx];
	if (xpos != 0xFF) {
		od->walk_x += (xpos * 8) - od->x_pos;
		od->x_pos = xpos * 8;
		od->walk_y += (ypos * 8) - od->y_pos;
		od->y_pos = ypos * 8;
	}
	addObjectToDrawQue(idx);

	x = od->x_pos;
	y = od->y_pos;
	w = od->width;
	h = od->height;

	i = _numLocalObjects - 1;
	do {
		if (_objs[i].obj_nr && _objs[i].x_pos == x && _objs[i].y_pos == y && _objs[i].width == w && _objs[i].height == h)
			putState(_objs[i].obj_nr, 0);
	} while (--i);

	putState(obj, state);
}

void ScummEngine_v5::o5_getStringWidth() {
	int string, width = 0;
	byte *ptr;

	getResultPos();
	string = getVarOrDirectByte(PARAM_1);
	ptr = getResourceAddress(rtString, string);
	assert(ptr);

	width = _charset->getStringWidth(0, ptr);

	setResult(width);
}

enum StringIds {
	// The string IDs used by Indy3 to store the episode resp. series IQ points.
	// Note that we save the episode IQ points but load the series IQ points,
	// which matches the original Indy3 save/load code. See also the notes
	// on Feature Request #1666521.
	STRINGID_IQ_EPISODE = 7,
	STRINGID_IQ_SERIES = 9
};

void ScummEngine_v5::o5_saveLoadVars() {
	// The KIXX XL release of Monkey Island 2 (Amiga disk) used this opcode
	// as dummy, in order to remove copy protection and keep level selection.
	if (_game.version == 5)
		return;

	if (fetchScriptByte() == 1)
		saveVars();
	else
		loadVars();
}

void ScummEngine_v5::saveVars() {
	int a, b;

	while ((_opcode = fetchScriptByte()) != 0) {
		switch (_opcode & 0x1F) {
		case 0x01: // write a range of variables
			getResultPos();
			a = _resultVarNumber;
			getResultPos();
			b = _resultVarNumber;
			debug(0, "stub saveVars: vars %d -> %d", a, b);
			break;
		case 0x02: // write a range of string variables
			a = getVarOrDirectByte(PARAM_1);
			b = getVarOrDirectByte(PARAM_2);

			if (a == STRINGID_IQ_EPISODE && b == STRINGID_IQ_EPISODE) {
				if (_game.id == GID_INDY3) {
					saveIQPoints();
				}
				break;
			}
			// FIXME: changing savegame-names not supported
			break;
		case 0x03: // open file
			a = resStrLen(_scriptPointer);
			strncpy(_saveLoadVarsFilename, (const char *)_scriptPointer, a);
			_saveLoadVarsFilename[a] = '\0';
			_scriptPointer += a + 1;
			break;
		case 0x04:
			return;
		case 0x1F: // close file
			_saveLoadVarsFilename[0] = '\0';
			return;
		}
	}
}

void ScummEngine_v5::loadVars() {
	int a, b;

	while ((_opcode = fetchScriptByte()) != 0) {
		switch (_opcode & 0x1F) {
		case 0x01: // read a range of variables
			getResultPos();
			a = _resultVarNumber;
			getResultPos();
			b = _resultVarNumber;
			debug(0, "stub loadVars: vars %d -> %d", a, b);
			break;
		case 0x02: // read a range of string variables
			a = getVarOrDirectByte(PARAM_1);
			b = getVarOrDirectByte(PARAM_2);

			int slot;
			int slotSize;
			byte* slotContent;
			int savegameId;
			bool avail_saves[100];

			if (a == STRINGID_IQ_SERIES && b == STRINGID_IQ_SERIES) {
				// Zak256 loads the IQ script-slot but does not use it -> ignore it
				if (_game.id == GID_INDY3) {
					loadIQPoints();
				}
				break;
			}

			listSavegames(avail_saves, ARRAYSIZE(avail_saves));
			for (slot = a; slot <= b; ++slot) {
				slotSize = getResourceSize(rtString, slot);
				slotContent = getResourceAddress(rtString, slot);

				// load savegame names
				savegameId = slot - a + 1;
				Common::String name;
				if (avail_saves[savegameId] && getSavegameName(savegameId, name)) {
					int pos;
					const char *ptr = name.c_str();
					// slotContent ends with {'\0','@'} -> max. length = slotSize-2
					for (pos = 0; pos < slotSize - 2; ++pos) {
						if (!ptr[pos])
							break;
						// replace special characters
						if (ptr[pos] >= 32 && ptr[pos] <= 122 && ptr[pos] != 64)
							slotContent[pos] = ptr[pos];
						else
							slotContent[pos] = '_';
					}
					slotContent[pos] = '\0';
				} else {
					slotContent[0] = '\0';
				}
			}
			break;
		case 0x03: // open file
			a = resStrLen(_scriptPointer);
			strncpy(_saveLoadVarsFilename, (const char *)_scriptPointer, a);
			_saveLoadVarsFilename[a] = '\0';
			_scriptPointer += a + 1;
			break;
		case 0x04:
			return;
		case 0x1F: // close file
			_saveLoadVarsFilename[0] = '\0';
			return;
		}
	}
}

void ScummEngine_v5::saveIQPoints() {
	// save Indy3 IQ-points
	Common::OutSaveFile *file;
	Common::String filename = _targetName + ".iq";

	file = _saveFileMan->openForSaving(filename.c_str());
	if (file != NULL) {
		int size = getResourceSize(rtString, STRINGID_IQ_EPISODE);
		byte *ptr = getResourceAddress(rtString, STRINGID_IQ_EPISODE);
		file->write(ptr, size);
		delete file;
	}
}

void ScummEngine_v5::loadIQPoints() {
	// load Indy3 IQ-points
	Common::InSaveFile *file;
	Common::String filename = _targetName + ".iq";

	file = _saveFileMan->openForLoading(filename.c_str());
	if (file != NULL) {
		int size = getResourceSize(rtString, STRINGID_IQ_SERIES);
		byte *ptr = getResourceAddress(rtString, STRINGID_IQ_SERIES);
		byte *tmp = (byte*)malloc(size);
		int nread = file->read(tmp, size);
		if (nread == size) {
			memcpy(ptr, tmp, size);
		}
		free(tmp);
		delete file;
	}
}

void ScummEngine_v5::o5_expression() {
	int dst, i;

	_scummStackPos = 0;
	getResultPos();
	dst = _resultVarNumber;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0x1F) {
		case 1:										/* varordirect */
			push(getVarOrDirectWord(PARAM_1));
			break;
		case 2:										/* add */
			i = pop();
			push(i + pop());
			break;
		case 3:										/* sub */
			i = pop();
			push(pop() - i);
			break;
		case 4:										/* mul */
			i = pop();
			push(i * pop());
			break;
		case 5:										/* div */
			i = pop();
			if (i == 0)
				error("Divide by zero");
			push(pop() / i);
			break;
		case 6:										/* normal opcode */
			_opcode = fetchScriptByte();
			executeOpcode(_opcode);
			push(_scummVars[0]);
			break;
		}
	}

	_resultVarNumber = dst;
	setResult(pop());
}

void ScummEngine_v5::o5_faceActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int obj = getVarOrDirectWord(PARAM_2);
	Actor *a = derefActor(act, "o5_faceActor");
	a->faceToObject(obj);
}

void ScummEngine_v5::o5_findInventory() {
	getResultPos();
	int x = getVarOrDirectByte(PARAM_1);
	int y = getVarOrDirectByte(PARAM_2);
	setResult(findInventory(x, y));
}

void ScummEngine_v5::o5_findObject() {
	getResultPos();
	int x = getVarOrDirectByte(PARAM_1);
	int y = getVarOrDirectByte(PARAM_2);
	setResult(findObject(x, y));
}

void ScummEngine_v5::o5_freezeScripts() {
	int scr = getVarOrDirectByte(PARAM_1);

	if (scr != 0)
		freezeScripts(scr);
	else
		unfreezeScripts();
}

void ScummEngine_v5::o5_getActorCostume() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorCostume");
	setResult(a->_costume);
}

void ScummEngine_v5::o5_getActorElevation() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorElevation");
	setResult(a->getElevation());
}

void ScummEngine_v5::o5_getActorFacing() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorFacing");
	setResult(newDirToOldDir(a->getFacing()));
}

void ScummEngine_v5::o5_getActorMoving() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorMoving");
	setResult(a->_moving);
}

void ScummEngine_v5::o5_getActorRoom() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	// WORKAROUND bug #746349. This is a really odd bug in either the script
	// or in our script engine. Might be a good idea to investigate this
	// further by e.g. looking at the FOA engine a bit closer.
	if (_game.id == GID_INDY4 && _roomResource == 94 && vm.slot[_currentScript].number == 206 && !isValidActor(act)) {
		setResult(0);
		return;
	}

	Actor *a = derefActor(act, "o5_getActorRoom");
	setResult(a->_room);
}

void ScummEngine_v5::o5_getActorScale() {
	Actor *a;

	// INDY3 uses this opcode for waitForActor
	if (_game.id == GID_INDY3) {
		const byte *oldaddr = _scriptPointer - 1;
		a = derefActor(getVarOrDirectByte(PARAM_1), "o5_getActorScale (wait)");
		if (a->_moving) {
			_scriptPointer = oldaddr;
			o5_breakHere();
		}
		return;
	}

	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	a = derefActor(act, "o5_getActorScale");
	setResult(a->_scalex);
}

void ScummEngine_v5::o5_getActorWalkBox() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorWalkBox");
	setResult(a->_walkbox);
}

void ScummEngine_v5::o5_getActorWidth() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorWidth");
	setResult(a->_width);
}

void ScummEngine_v5::o5_getActorX() {
	int a;
	getResultPos();

	if ((_game.id == GID_INDY3) && !(_game.platform == Common::kPlatformMacintosh))
		a = getVarOrDirectByte(PARAM_1);
	else
		a = getVarOrDirectWord(PARAM_1);

	setResult(getObjX(a));
}

void ScummEngine_v5::o5_getActorY() {
	int a;
	getResultPos();

	if ((_game.id == GID_INDY3) && !(_game.platform == Common::kPlatformMacintosh)) {
		a = getVarOrDirectByte(PARAM_1);

		// WORKAROUND bug #636433 (can't get into Zeppelin)
		if (_roomResource == 36) {
			setResult(getObjY(a) - 1);
			return;
		}
	} else
		a = getVarOrDirectWord(PARAM_1);

	setResult(getObjY(a));
}

void ScummEngine_v5::o5_saveLoadGame() {
	getResultPos();
	byte a = getVarOrDirectByte(PARAM_1);
	byte slot = a & 0x1F;
	byte result = 0;

	// Slot numbers in older games start with 0, in newer games with 1
	if (_game.version <= 2)
		slot++;

	if ((_game.id == GID_MANIAC) && (_game.version <= 1)) {
		// Convert older load/save screen
		// 1 Load
		// 2 Save
		slot = 1;
		if (a == 1)
			_opcode = 0x40;
		else if ((a == 2) || (_game.platform == Common::kPlatformNES))
			_opcode = 0x80;
	} else {
		_opcode = a & 0xE0;
	}

	switch (_opcode) {
	case 0x00: // num slots available
		result = 100;
		break;
	case 0x20: // drive
		if (_game.id == GID_INDY3) {
			// 0 = hard drive
			// 1 = disk drive
			result = 0;
		} else {
			// set current drive
			result = 1;
		}
		break;
	case 0x40: // load
		if (loadState(slot, _saveTemporaryState))
			result = 3; // sucess
		else
			result = 5; // failed to load
		break;
	case 0x80: // save
		//if (saveState(slot, _saveTemporaryState))
		//	result = 0; // sucess
		//else
			result = 2; // failed to save
		break;
	case 0xC0: // test if save exists
		Common::InSaveFile *file;
		bool avail_saves[100];
		char filename[256];

		listSavegames(avail_saves, ARRAYSIZE(avail_saves));
		makeSavegameName(filename, slot, false);
		if (avail_saves[slot] && (file = _saveFileMan->openForLoading(filename))) {
			result = 6; // save file exists
			delete file;
		} else
			result = 7; // save file does not exist
		break;
	default:
		error("o5_saveLoadGame: unknown subopcode %d", _opcode);
	}

	setResult(result);
}

void ScummEngine_v5::o5_getAnimCounter() {
	if (_game.version == 3) {
		o5_saveLoadGame();
		return;
	}

	getResultPos();

	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getAnimCounter");
	setResult(a->_cost.animCounter);
}

void ScummEngine_v5::o5_getClosestObjActor() {
	int obj;
	int act;
	int dist;

	// This code can't detect any actors farther away than 255 units
	// (pixels in newer games, characters in older ones.) But this is
	// perfectly OK, as it is exactly how the original behaved.

	int closest_obj = 0xFF, closest_dist = 0xFF;

	getResultPos();

	act = getVarOrDirectWord(PARAM_1);
	obj = VAR(VAR_ACTOR_RANGE_MAX);

	do {
		dist = getObjActToObjActDist(act, obj);
		if (dist < closest_dist) {
			closest_dist = dist;
			closest_obj = obj;
		}
	} while (--obj >= VAR(VAR_ACTOR_RANGE_MIN));

	setResult(closest_obj);
}

void ScummEngine_v5::o5_getDist() {
	int o1, o2;
	int r;
	getResultPos();
	o1 = getVarOrDirectWord(PARAM_1);
	o2 = getVarOrDirectWord(PARAM_2);
	r = getObjActToObjActDist(o1, o2);

	// FIXME: MI2 race workaround, see bug #597022. We never quite figured out
	// what the real cause of this, or if it maybe occurs in the original, too...
	if (_game.id == GID_MONKEY2 && vm.slot[_currentScript].number == 40 && r < 60)
		r = 60;

	// WORKAROUND bug #795937
	if ((_game.id == GID_MONKEY_EGA || _game.id == GID_PASS) && o1 == 1 && o2 == 307  && vm.slot[_currentScript].number == 205 && r == 2)
		r = 3;

	setResult(r);
}

void ScummEngine_v5::o5_getInventoryCount() {
	getResultPos();
	setResult(getInventoryCount(getVarOrDirectByte(PARAM_1)));
}

void ScummEngine_v5::o5_getObjectOwner() {
	getResultPos();
	setResult(getOwner(getVarOrDirectWord(PARAM_1)));
}

void ScummEngine_v5::o5_getObjectState() {
	if (_game.features & GF_SMALL_HEADER) {
		o5_ifState();
	} else {
		getResultPos();
		setResult(getState(getVarOrDirectWord(PARAM_1)));
	}
}

void ScummEngine_v5::o5_ifState() {
	int a = getVarOrDirectWord(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	if (getState(a) != b)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_v5::o5_ifNotState() {
	int a = getVarOrDirectWord(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	if (getState(a) == b)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_v5::o5_getRandomNr() {
	getResultPos();
	setResult(_rnd.getRandomNumber(getVarOrDirectByte(PARAM_1)));
}

void ScummEngine_v5::o5_isScriptRunning() {
	getResultPos();
	setResult(isScriptRunning(getVarOrDirectByte(PARAM_1)));
}

void ScummEngine_v5::o5_getVerbEntrypoint() {
	int a, b;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	b = getVarOrDirectWord(PARAM_2);

	setResult(getVerbEntrypoint(a, b));
}

void ScummEngine_v5::o5_ifClassOfIs() {
	int act, cls, b = 0;
	bool cond = true;

	act = getVarOrDirectWord(PARAM_1);

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		cls = getVarOrDirectWord(PARAM_1);
		b = getClass(act, cls);
		if (cls & 0x80 && !b || !(cls & 0x80) && b)
			cond = false;
	}
	if (cond)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v5::o5_increment() {
	getResultPos();
	setResult(readVar(_resultVarNumber) + 1);
}

void ScummEngine_v5::o5_isActorInBox() {
	int act = getVarOrDirectByte(PARAM_1);
	int box = getVarOrDirectByte(PARAM_2);
	Actor *a = derefActor(act, "o5_isActorInBox");

	if (!checkXYInBoxBounds(box, a->getRealPos().x, a->getRealPos().y))
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_v5::o5_isEqual() {
	int16 a, b;
	int var;

	if (_game.version <= 2)
		var = fetchScriptByte();
	else
		var = fetchScriptWord();
	a = readVar(var);
	b = getVarOrDirectWord(PARAM_1);

	// HACK: See bug report #602348. The sound effects for Largo's screams
	// are only played on type 5 soundcards. However, there is at least one
	// other sound effect (the bartender spitting) which is only played on
	// type 3 soundcards.

	if (_game.id == GID_MONKEY2 && var == VAR_SOUNDCARD && b == 5)
		b = a;

	// HACK: To allow demo script of Maniac Mansion V2
	// The camera x position is only 100, instead of 180, after game title name scrolls.
	if (_game.id == GID_MANIAC && _game.version == 2 && (_game.features & GF_DEMO) && isScriptRunning(173) && b == 180)
		b = 100;

	if (b == a)
		ignoreScriptWord();
	else
		o5_jumpRelative();

}

void ScummEngine_v5::o5_isGreater() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);
	if (b > a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v5::o5_isGreaterEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);
	if (b >= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v5::o5_isLess() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);
	if (b < a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v5::o5_lessOrEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);

	// WORKAROUND bug #820507 : Work around a bug in Indy3Town.
	if (_game.id == GID_INDY3 && (_game.platform == Common::kPlatformFMTowns) &&
	    (vm.slot[_currentScript].number == 200 || vm.slot[_currentScript].number == 203) &&
	    _currentRoom == 70 && b == -256) {
		o5_jumpRelative();
		return;
	}

	if (b <= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v5::o5_isNotEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);
	if (b != a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v5::o5_notEqualZero() {
	int a = getVar();
	if (a != 0)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v5::o5_equalZero() {
	int a = getVar();
	if (a == 0)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v5::o5_jumpRelative() {
	// Note that calling fetchScriptWord() will also modify _scriptPointer,
	// so *don't* do this: _scriptPointer += (int16)fetchScriptWord();
	//
	// I'm not enough of a language lawyer to say for certain that this is
	// undefined, but I do know that GCC 4.0 doesn't think it means what
	// we want it to mean.

	int16 offset = (int16)fetchScriptWord();
	_scriptPointer += offset;
}

void ScummEngine_v5::o5_lights() {
	int a, b, c;

	a = getVarOrDirectByte(PARAM_1);
	b = fetchScriptByte();
	c = fetchScriptByte();

	if (c == 0)
		VAR(VAR_CURRENT_LIGHTS) = a;
	else if (c == 1) {
		_flashlight.xStrips = a;
		_flashlight.yStrips = b;
	}
	_fullRedraw = true;
}

void ScummEngine_v5::o5_loadRoom() {
	int room;

	room = getVarOrDirectByte(PARAM_1);

	// For small header games, we only call startScene if the room
	// actually changed. This avoid unwanted (wrong) fades in Zak256
	// and others. OTOH, it seems to cause a problem in newer games.
	if (!(_game.features & GF_SMALL_HEADER) || room != _currentRoom)
		startScene(room, 0, 0);

	_fullRedraw = true;
}

void ScummEngine_v5::o5_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y;
	int x2, y2, dir, oldDir;

	obj = getVarOrDirectWord(PARAM_1);
	room = getVarOrDirectByte(PARAM_2);

	a = derefActor(VAR(VAR_EGO), "o5_loadRoomWithEgo");

	a->putActor(room);
	oldDir = a->getFacing();
	_egoPositioned = false;

	x = (int16)fetchScriptWord();
	y = (int16)fetchScriptWord();

	VAR(VAR_WALKTO_OBJ) = obj;
	startScene(a->_room, a, obj);
	VAR(VAR_WALKTO_OBJ) = 0;

	if (_game.version <= 4) {
		if (whereIsObject(obj) != WIO_ROOM)
			error("o5_loadRoomWithEgo: Object %d is not in room %d", obj, _currentRoom);
		if (!_egoPositioned) {
			getObjectXYPos(obj, x2, y2, dir);
			a->putActor(x2, y2, _currentRoom);
			if (a->getFacing() == oldDir)
				a->setDirection(dir + 180);
		}
		a->_moving = 0;
	}

	// This is based on disassembly
	camera._cur.x = camera._dest.x = a->getPos().x;
	if ((_game.id == GID_ZAK || _game.id == GID_LOOM) && (_game.platform == Common::kPlatformFMTowns)) {
		setCameraAt(a->getPos().x, a->getPos().y);
	}
	setCameraFollows(a);

	_fullRedraw = true;

	if (x != -1) {
		a->startWalkActor(x, y, -1);
	}
}

void ScummEngine_v5::o5_matrixOps() {
	int a, b;

	if (_game.version == 3) {
		a = getVarOrDirectByte(PARAM_1);
		b = fetchScriptByte();
		setBoxFlags(a, b);
		return;
	}

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		setBoxFlags(a, b);
		break;
	case 2:
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		setBoxScale(a, b);
		break;
	case 3:
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		setBoxScale(a, (b - 1) | 0x8000);
		break;
	case 4:
		createBoxMatrix();
		break;
	}
}

void ScummEngine_v5::o5_move() {
	getResultPos();
	setResult(getVarOrDirectWord(PARAM_1));
}

void ScummEngine_v5::o5_multiply() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	setResult(readVar(_resultVarNumber) * a);
}

void ScummEngine_v5::o5_or() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	setResult(readVar(_resultVarNumber) | a);
}

void ScummEngine_v5::o5_beginOverride() {
	if (fetchScriptByte() != 0)
		beginOverride();
	else
		endOverride();
}

void ScummEngine_v5::o5_panCameraTo() {
	panCameraTo(getVarOrDirectWord(PARAM_1), 0);
}

void ScummEngine_v5::o5_pickupObject() {
	int obj, room;
	if (_game.version == 3 || _game.version == 4) {
		o5_drawObject();
		return;
	}

	obj = getVarOrDirectWord(PARAM_1);
	room = getVarOrDirectByte(PARAM_2);
	if (room == 0)
		room = _roomResource;
	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
	runInventoryScript(1);
}

void ScummEngine_v5::o5_print() {
	_actorToPrintStrFor = getVarOrDirectByte(PARAM_1);
	decodeParseString();
}

void ScummEngine_v5::o5_printEgo() {
	_actorToPrintStrFor = (byte)VAR(VAR_EGO);
	decodeParseString();
}

void ScummEngine_v5::o5_pseudoRoom() {
	int i = fetchScriptByte(), j;
	while ((j = fetchScriptByte()) != 0) {
		if (j >= 0x80) {
			_resourceMapper[j & 0x7F] = i;
		}
	}
}

void ScummEngine_v5::o5_putActor() {
	int x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o5_putActor");
	x = getVarOrDirectWord(PARAM_2);
	y = getVarOrDirectWord(PARAM_3);
	a->putActor(x, y);
}

void ScummEngine_v5::o5_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o5_putActorAtObject");
	obj = getVarOrDirectWord(PARAM_2);
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj, x, y);
	else {
		x = 240;
		y = 120;
	}
	a->putActor(x, y);
}

void ScummEngine_v5::o5_putActorInRoom() {
	Actor *a;
	int act = getVarOrDirectByte(PARAM_1);
	int room = getVarOrDirectByte(PARAM_2);

	a = derefActor(act, "o5_putActorInRoom");

	if (a->_visible && _currentRoom != room && getTalkingActor() == a->_number) {
		stopTalk();
	}
	a->_room = room;
	if (!room)
		a->putActor(0, 0, 0);
}

void ScummEngine_v5::o5_systemOps() {
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 1:		// SO_RESTART
		restart();
		break;
	case 2:		// SO_PAUSE
		pauseGame();
		break;
	case 3:		// SO_QUIT
		quitGame();
		break;
	default:
		error("o5_systemOps: unknown subopcode %d", subOp);
	}
}

void ScummEngine_v5::o5_resourceRoutines() {
	const ResTypes resType[4] = { rtScript, rtSound, rtCostume, rtRoom };
	int resid = 0;
	int foo, bar;

	_opcode = fetchScriptByte();
	if (_opcode != 17)
		resid = getVarOrDirectByte(PARAM_1);
	if (!(_game.platform == Common::kPlatformFMTowns)) {
		// FIXME - this probably can be removed eventually, I don't think the following
		// check will ever be triggered, but then I could be wrong and it's better
		// to play it safe.
		if ((_opcode & 0x3F) != (_opcode & 0x1F))
			error("Oops, this shouldn't happen: o5_resourceRoutines opcode %d", _opcode);
	}

	int op = _opcode & 0x3F;

	// FIXME: Sound resources are currently missing
	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine &&
		(op == 2 || op == 6)) {
			return;
	}

	switch (op) {
	case 1:			// SO_LOAD_SCRIPT
	case 2:			// SO_LOAD_SOUND
	case 3:			// SO_LOAD_COSTUME
		ensureResourceLoaded(resType[op - 1], resid);
		break;
	case 4:			// SO_LOAD_ROOM
		if (_game.version == 3) {
			ensureResourceLoaded(rtRoom, resid);
			if (resid > 0x7F)
				resid = _resourceMapper[resid & 0x7F];

			if (_currentRoom != resid) {
				_res->setResourceCounter(rtRoom, resid, 1);
			}
		} else
			ensureResourceLoaded(rtRoom, resid);
		break;

	case 5:			// SO_NUKE_SCRIPT
	case 6:			// SO_NUKE_SOUND
	case 7:			// SO_NUKE_COSTUME
	case 8:			// SO_NUKE_ROOM
		if (_game.id == GID_ZAK && (_game.platform == Common::kPlatformFMTowns))
			error("o5_resourceRoutines %d should not occur in Zak256", op);
		else
			_res->setResourceCounter(resType[op-5], resid, 0x7F);
		break;
	case 9:			// SO_LOCK_SCRIPT
		if (resid >= _numGlobalScripts)
			break;
		_res->lock(rtScript, resid);
		break;
	case 10:		// SO_LOCK_SOUND
		// FIXME: Sound resources are currently missing
		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)
			break;
		_res->lock(rtSound, resid);
		break;
	case 11:		// SO_LOCK_COSTUME
		_res->lock(rtCostume, resid);
		break;
	case 12:		// SO_LOCK_ROOM
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->lock(rtRoom, resid);
		break;

	case 13:		// SO_UNLOCK_SCRIPT
		if (resid >= _numGlobalScripts)
			break;
		_res->unlock(rtScript, resid);
		break;
	case 14:		// SO_UNLOCK_SOUND
		// FIXME: Sound resources are currently missing
		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)
			break;
		_res->unlock(rtSound, resid);
		break;
	case 15:		// SO_UNLOCK_COSTUME
		_res->unlock(rtCostume, resid);
		break;
	case 16:		// SO_UNLOCK_ROOM
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->unlock(rtRoom, resid);
		break;

	case 17:		// SO_CLEAR_HEAP
		//heapClear(0);
		//unkHeapProc2(0, 0);
		break;
	case 18:		// SO_LOAD_CHARSET
		loadCharset(resid);
		break;
	case 19:		// SO_NUKE_CHARSET
		nukeCharset(resid);
		break;
	case 20:		// SO_LOAD_OBJECT
		loadFlObject(getVarOrDirectWord(PARAM_2), resid);
		break;

	// TODO: For the following see also Hibarnatus' information on bug #805691.
	case 32:
		// TODO (apparently never used in FM-TOWNS)
		debug(0, "o5_resourceRoutines %d not yet handled (script %d)", op,  vm.slot[_currentScript].number);
		break;
	case 33:
		// TODO (apparently never used in FM-TOWNS)
		debug(0, "o5_resourceRoutines %d not yet handled (script %d)", op,  vm.slot[_currentScript].number);
		break;
	case 35:
		// TODO: Might be used to set CD volume in FM-TOWNS Loom
		foo = getVarOrDirectByte(PARAM_2);
		debug(0, "o5_resourceRoutines %d not yet handled (script %d)", op,  vm.slot[_currentScript].number);
		break;
	case 36:
		// TODO: Sets the loudness of a sound resource. Used in Indy3 and Zak.
		foo = getVarOrDirectByte(PARAM_2);
		bar = fetchScriptByte();
		debug(0, "o5_resourceRoutines %d not yet handled (script %d)", op,  vm.slot[_currentScript].number);
		break;
	case 37:
		// TODO: Sets the pitch of a sound resource (pitch = foo - center semitones.
		// "center" is at 0x32 in the sfx resource (always 0x3C in zak256, but sometimes different in Indy3).
		foo = getVarOrDirectByte(PARAM_2);
		debug(0, "o5_resourceRoutines %d not yet handled (script %d)", op,  vm.slot[_currentScript].number);
		break;

	default:
		error("o5_resourceRoutines: default case %d", op);
	}
}

void ScummEngine_v5::o5_roomOps() {
	int a = 0, b = 0, c, d, e;

	if (_game.version == 3 && _game.platform != Common::kPlatformPCEngine) {
		a = getVarOrDirectWord(PARAM_1);
		b = getVarOrDirectWord(PARAM_2);
	}

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:		// SO_ROOM_SCROLL
		if (_game.version != 3 || _game.platform == Common::kPlatformPCEngine) {
			a = getVarOrDirectWord(PARAM_1);
			b = getVarOrDirectWord(PARAM_2);
		}
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
	case 2:		// SO_ROOM_COLOR
		if (_game.features & GF_SMALL_HEADER) {
			if (_game.version != 3 || _game.platform == Common::kPlatformPCEngine) {
				a = getVarOrDirectWord(PARAM_1);
				b = getVarOrDirectWord(PARAM_2);
			}
			assertRange(0, a, 256, "o5_roomOps: 2: room color slot");
			_roomPalette[b] = a;
			_fullRedraw = true;
		} else {
			error("room-color is no longer a valid command");
		}
		break;

	case 3:		// SO_ROOM_SCREEN
		if (_game.version != 3 || _game.platform == Common::kPlatformPCEngine) {
			a = getVarOrDirectWord(PARAM_1);
			b = getVarOrDirectWord(PARAM_2);
		}
		initScreens(a, b);
		break;
	case 4:		// SO_ROOM_PALETTE
		if (_game.features & GF_SMALL_HEADER) {
			if (_game.version != 3 || _game.platform == Common::kPlatformPCEngine) {
				a = getVarOrDirectWord(PARAM_1);
				b = getVarOrDirectWord(PARAM_2);
			}
			assertRange(0, a, 256, "o5_roomOps: 4: room color slot");
			_shadowPalette[b] = a;
			setDirtyColors(b, b);
		} else {
			a = getVarOrDirectWord(PARAM_1);
			b = getVarOrDirectWord(PARAM_2);
			c = getVarOrDirectWord(PARAM_3);
			_opcode = fetchScriptByte();
			d = getVarOrDirectByte(PARAM_1);
			setPalColor(d, a, b, c);	/* index, r, g, b */
		}
		break;
	case 5:		// SO_ROOM_SHAKE_ON
		setShake(1);
		break;
	case 6:		// SO_ROOM_SHAKE_OFF
		setShake(0);
		break;
	case 7:		// SO_ROOM_SCALE
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		_opcode = fetchScriptByte();
		c = getVarOrDirectByte(PARAM_1);
		d = getVarOrDirectByte(PARAM_2);
		_opcode = fetchScriptByte();
		e = getVarOrDirectByte(PARAM_2);
		setScaleSlot(e - 1, 0, b, a, 0, d, c);
		break;
	case 8:		// SO_ROOM_INTENSITY
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		c = getVarOrDirectByte(PARAM_3);
		darkenPalette(a, a, a, b, c);
		break;
	case 9:		// SO_ROOM_SAVEGAME
		_saveLoadFlag = getVarOrDirectByte(PARAM_1);
		_saveLoadSlot = getVarOrDirectByte(PARAM_2);
		_saveLoadSlot = 99;					/* use this slot */
		_saveTemporaryState = true;
		break;
	case 10:	// SO_ROOM_FADE
		a = getVarOrDirectWord(PARAM_1);
		if (a) {
			if (_game.platform == Common::kPlatformFMTowns) {
				switch (a) {
				case 8: // compose kMainVirtScreen over a screen buffer
				case 9: // call 0x110:0x20 _ax=0x601 _edx=2
				case 10: // call 0x110:0x20 _ax=0x601 _edx=3
				case 11: // clear screen 0x1C:0x45000 sizeof(640 * 320)
				case 12: // call 0x110:0x20 _ax=0x601 _edx=0
				case 13: // call 0x110:0x20 _ax=0x601 _edx=1
				case 16: // enable clearing of a screen buffer in drawBitmap()
				case 17: // disable clearing of a screen buffer in drawBitmap()
				case 18: // clear a screen buffer
				case 19: // enable palette operations (palManipulate(), cyclePalette() etc.)
				case 20: // disable palette operations
				case 21: // disable clearing of screen 0x1C:0x5000 sizeof(640 * 320) in initScreens()
				case 22: // enable clearing of screen 0x1C:0x5000 sizeof(640 * 320) in initScreens()
				case 30:
					debug(0, "o5_roomOps: unhandled FM-TOWNS fadeEffect %d", a);
					return;
				}
			}
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;
	case 11:	// SO_RGB_ROOM_INTENSITY
		a = getVarOrDirectWord(PARAM_1);
		b = getVarOrDirectWord(PARAM_2);
		c = getVarOrDirectWord(PARAM_3);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(PARAM_1);
		e = getVarOrDirectByte(PARAM_2);
		darkenPalette(a, b, c, d, e);
		break;
	case 12:	// SO_ROOM_SHADOW
		a = getVarOrDirectWord(PARAM_1);
		b = getVarOrDirectWord(PARAM_2);
		c = getVarOrDirectWord(PARAM_3);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(PARAM_1);
		e = getVarOrDirectByte(PARAM_2);
		setShadowPalette(a, b, c, d, e, 0, 256);
		break;

	case 13:	// SO_SAVE_STRING
		{
			Common::OutSaveFile *file;
			char filename[256], *s;

			a = getVarOrDirectByte(PARAM_1);
			s = filename;
			while ((*s++ = fetchScriptByte()));

			file = _saveFileMan->openForSaving(filename);
			if (file != NULL) {
				byte *ptr;
				ptr = getResourceAddress(rtString, a);
				file->write(ptr, resStrLen(ptr) + 1);
				delete file;
				VAR(VAR_SOUNDRESULT) = 0;
			}
			break;
		}
	case 14:	// SO_LOAD_STRING
		{
			Common::InSaveFile *file;
			char filename[256], *s;

			a = getVarOrDirectByte(PARAM_1);
			s = filename;
			while ((*s++ = fetchScriptByte()));

			file = _saveFileMan->openForLoading(filename);
			if (file != NULL) {
				byte *ptr;
				int len = 256, cnt = 0;
				ptr = (byte *)malloc(len);
				while (ptr) {
					int r = file->read(ptr + cnt, len - cnt);
					cnt += r;
					if (cnt < len)
						break;
					len *= 2;
					ptr = (byte *)realloc(ptr, len);
				}
				ptr[cnt] = '\0';
				loadPtrToResource(rtString, a, ptr);
				free(ptr);
				delete file;
			}
			break;
		}
	case 15:	// SO_ROOM_TRANSFORM
		a = getVarOrDirectByte(PARAM_1);
		_opcode = fetchScriptByte();
		b = getVarOrDirectByte(PARAM_1);
		c = getVarOrDirectByte(PARAM_2);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(PARAM_1);
		palManipulateInit(a, b, c, d);
		break;

	case 16:	// SO_CYCLE_SPEED
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		assertRange(1, a, 16, "o5_roomOps: 16: color cycle");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;
	default:
		error("o5_roomOps: unknown subopcode %d", _opcode & 0x1F);
	}
}

void ScummEngine_v5::o5_saveRestoreVerbs() {
	int a, b, c, slot, slot2;

	_opcode = fetchScriptByte();

	a = getVarOrDirectByte(PARAM_1);
	b = getVarOrDirectByte(PARAM_2);
	c = getVarOrDirectByte(PARAM_3);

	switch (_opcode) {
	case 1:		// SO_SAVE_VERBS
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
	case 2:		// SO_RESTORE_VERBS
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
	case 3:		// SO_DELETE_VERBS
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot)
				killVerb(slot);
			a++;
		}
		break;
	default:
		error("o5_saveRestoreVerbs: unknown subopcode %d", _opcode);
	}
}

void ScummEngine_v5::o5_setCameraAt() {
	setCameraAtEx(getVarOrDirectWord(PARAM_1));
}

void ScummEngine_v5::o5_setObjectName() {
	int obj = getVarOrDirectWord(PARAM_1);
	setObjectName(obj);
}

void ScummEngine_v5::o5_setOwnerOf() {
	int obj, owner;

	obj = getVarOrDirectWord(PARAM_1);
	owner = getVarOrDirectByte(PARAM_2);

	setOwnerOf(obj, owner);
}

void ScummEngine_v5::o5_setState() {
	int obj, state;
	obj = getVarOrDirectWord(PARAM_1);
	state = getVarOrDirectByte(PARAM_2);
	putState(obj, state);
	markObjectRectAsDirty(obj);
	if (_bgNeedsRedraw)
		clearDrawObjectQueue();
}

void ScummEngine_v5::o5_setVarRange() {
	int a, b;

	getResultPos();
	a = fetchScriptByte();
	do {
		if (_opcode & 0x80)
			b = fetchScriptWordSigned();
		else
			b = fetchScriptByte();

		setResult(b);
		_resultVarNumber++;
	} while (--a);

	// Macintosh verison of indy3ega used different interface, so adjust values.
	if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformMacintosh) {
		VAR(68) = 0;
		VAR(69) = 0;
		VAR(70) = 168;
		VAR(71) = 0;
		VAR(72) = 168;
		VAR(73) = 0;
		VAR(74) = 168;
		VAR(75) = 0;
		VAR(76) = 176;
		VAR(77) = 176;
		VAR(78) = 184;
		VAR(79) = 184;
		VAR(80) = 192;
		VAR(81) = 192;
	}
}

void ScummEngine_v5::o5_startMusic() {
	if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {
		// In FM-TOWNS games this is some kind of Audio CD status query function.
		// See also bug #762589 (thanks to Hibernatus for providing the information).
		getResultPos();
		int b = getVarOrDirectByte(PARAM_1);
		int result = 0;
		switch (b) {
		case 0:
			result = _sound->pollCD() == 0;
			break;
		case 0xFC:
			// TODO: Unpause (resume) audio track. We'll have to extend Sound and OSystem for this.
			break;
		case 0xFD:
			// TODO: Pause audio track. We'll have to extend Sound and OSystem for this.
			break;
		case 0xFE:
			result = _sound->getCurrentCDSound();
			break;
		case 0xFF:
			// TODO: Might return current CD volume in FM-TOWNS Loom. See also bug #805691.
			break;
		default:
			// TODO: return track length in seconds. We'll have to extend Sound and OSystem for this.
			// To check scummvm returns the right track length you
			// can look at the global script #9 (0x888A in 49.LFL).
			break;
		}
		debugC(DEBUG_GENERAL,"o5_startMusic(%d)", b);
		setResult(result);
	} else {
		_sound->addSoundToQueue(getVarOrDirectByte(PARAM_1));
	}
}

void ScummEngine_v5::o5_startSound() {
	const byte *oldaddr = _scriptPointer - 1;
	int sound = getVarOrDirectByte(PARAM_1);

	// WORKAROUND: In the scene where Largo is talking to Mad Marty, the
	// Woodtick music often resumes before Largo's theme has finished. As
	// far as I can tell, this is a script bug.
	if (_game.id == GID_MONKEY2 && sound == 110 && _sound->isSoundRunning(151)) {
		debug(1, "Delaying Woodtick music until Largo's theme has finished");
		_scriptPointer = oldaddr;
		o5_breakHere();
		return;
	}

	if (VAR_MUSIC_TIMER != 0xFF)
		VAR(VAR_MUSIC_TIMER) = 0;
	_sound->addSoundToQueue(sound);
}

void ScummEngine_v5::o5_stopMusic() {
	_sound->stopAllSounds();
}

void ScummEngine_v5::o5_stopSound() {
	_sound->stopSound(getVarOrDirectByte(PARAM_1));
}

void ScummEngine_v5::o5_isSoundRunning() {
	int snd;
	getResultPos();
	snd = getVarOrDirectByte(PARAM_1);
	if (snd)
		snd = _sound->isSoundRunning(snd);
	setResult(snd);
}

void ScummEngine_v5::o5_soundKludge() {
	int items[16];

	if (_game.features & GF_SMALL_HEADER) {	// Is WaitForSentence in SCUMM V3
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
		} else if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;

		_scriptPointer--;
		o5_breakHere();
		return;
	}

	int num = getWordVararg(items);
	_sound->soundKludge(items, num);
}

void ScummEngine_v5::o5_startObject() {
	int obj, script;
	int data[16];

	obj = getVarOrDirectWord(PARAM_1);
	script = getVarOrDirectByte(PARAM_2);

	getWordVararg(data);
	runObjectScript(obj, script, 0, 0, data);
}

void ScummEngine_v5::o5_startScript() {
	int op, script;
	int data[16];

	op = _opcode;
	script = getVarOrDirectByte(PARAM_1);

	getWordVararg(data);

	// WORKAROUND bug #1290485: Script 171 loads a complete room resource,
	// instead of the actual script, causing invalid opcode cases
	if (_game.id == GID_ZAK && _game.platform == Common::kPlatformFMTowns && script == 171)
		return;

	// Method used by original games to skip copy protection scheme
	if (!_copyProtection) {
		// Copy protection was disabled in LucasArts Classic Adventures (PC Disk)
		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPC && _game.version == 3 && _currentRoom == 69 && script == 201)
			script = 205;
		// Copy protection was disabled in KIXX XL release (Amiga Disk) and
		// in LucasArts Classic Adventures (PC Disk)
		if (_game.id == GID_MONKEY_VGA && script == 152)
			return;
		// Copy protection was disabled in LucasArts Mac CD Game Pack II (Macintosh CD)
		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformMacintosh && script == 155)
			return;
	}

	runScript(script, (op & 0x20) != 0, (op & 0x40) != 0, data);
}

void ScummEngine_v5::o5_stopObjectCode() {
	stopObjectCode();
}

void ScummEngine_v5::o5_stopObjectScript() {
	stopObjectScript(getVarOrDirectWord(PARAM_1));
}

void ScummEngine_v5::o5_stopScript() {
	const byte *oldaddr = _scriptPointer - 1;
	int script;

	script = getVarOrDirectByte(PARAM_1);

	if (_game.id == GID_INDY4 && script == 164 &&
		_roomResource == 50 && vm.slot[_currentScript].number == 213 && VAR(VAR_HAVE_MSG)) {
		// WORKAROUND bug #1308033: Due to a script bug, a line of text is skipped
		// which Indy is supposed to speak when he finds Orichalcum in some old
		// bones in the caves below Crete.
		_scriptPointer = oldaddr;
		o5_breakHere();
		return;
	}

	if (!script)
		stopObjectCode();
	else
		stopScript(script);
}

void ScummEngine_v5::o5_stringOps() {
	int a, b, c, i;
	byte *ptr;

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:											/* loadstring */
		loadPtrToResource(rtString, getVarOrDirectByte(PARAM_1), NULL);
		break;
	case 2:											/* copystring */
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		_res->nukeResource(rtString, a);
		ptr = getResourceAddress(rtString, b);
		if (ptr)
			loadPtrToResource(rtString, a, ptr);
		break;
	case 3:											/* set string char */
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		c = getVarOrDirectByte(PARAM_3);
		ptr = getResourceAddress(rtString, a);
		if (ptr == NULL)
			error("String %d does not exist", a);
		ptr[b] = c;
		break;

	case 4:											/* get string char */
		getResultPos();
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		ptr = getResourceAddress(rtString, a);
		if (ptr == NULL)
			error("String %d does not exist", a);
		setResult(ptr[b]);
		break;

	case 5:											/* create empty string */
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		_res->nukeResource(rtString, a);
		if (b) {
			ptr = _res->createResource(rtString, a, b);
			if (ptr) {
				for (i = 0; i < b; i++)
					ptr[i] = 0;
			}
		}
		break;
	}
}

void ScummEngine_v5::o5_subtract() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	setResult(readVar(_resultVarNumber) - a);
}

void ScummEngine_v5::o5_verbOps() {
	int verb, slot;
	VerbSlot *vs;
	int a, b;
	byte *ptr;

	verb = getVarOrDirectByte(PARAM_1);

	slot = getVerbSlot(verb, 0);
	assertRange(0, slot, _numVerbs - 1, "new verb slot");

	vs = &_verbs[slot];
	vs->verbid = verb;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0x1F) {
		case 1:		// SO_VERB_IMAGE
			a = getVarOrDirectWord(PARAM_1);
			if (slot) {
				setVerbObject(_roomResource, a, slot);
				vs->type = kImageVerbType;
			}
			break;
		case 2:		// SO_VERB_NAME
			loadPtrToResource(rtVerb, slot, NULL);
			if (slot == 0)
				_res->nukeResource(rtVerb, slot);
			vs->type = kTextVerbType;
			vs->imgindex = 0;
			break;
		case 3:		// SO_VERB_COLOR
			vs->color = getVarOrDirectByte(PARAM_1);
			break;
		case 4:		// SO_VERB_HICOLOR
			vs->hicolor = getVarOrDirectByte(PARAM_1);
			break;
		case 5:		// SO_VERB_AT
			vs->curRect.left = getVarOrDirectWord(PARAM_1);
			vs->curRect.top = getVarOrDirectWord(PARAM_2);
			// Macintosh version of indy3ega used different interface, so adjust values.
			if ((_game.platform == Common::kPlatformMacintosh) && (_game.id == GID_INDY3)) {
				switch (verb) {
				case 1:
				case 2:
				case 9:
					vs->curRect.left += 16;
					break;
				case 10:
				case 11:
				case 12:
					vs->curRect.left += 36;
					break;
				case 4:
				case 5:
				case 8:
					vs->curRect.left += 60;
					break;
				case 13:
				case 32:
				case 33:
				case 34:
					vs->curRect.left += 90;
					break;
				case 107:
					vs->curRect.left -= 54;
					vs->curRect.top += 16;
					break;
				case 108:
					vs->curRect.left -= 54;
					vs->curRect.top += 8;
					break;
				}
			} else	if (_game.id == GID_LOOM && _game.version == 4) {
			// FIXME: hack loom notes into right spot
				if ((verb >= 90) && (verb <= 97)) {	// Notes
					switch (verb) {
					case 90:
					case 91:
						vs->curRect.top -= 7;
						break;
					case 92:
						vs->curRect.top -= 6;
						break;
					case 93:
						vs->curRect.top -= 4;
						break;
					case 94:
						vs->curRect.top -= 3;
						break;
					case 95:
						vs->curRect.top -= 1;
						break;
					case 97:
						vs->curRect.top -= 5;
					}
				}
			}
			break;
		case 6:		// SO_VERB_ON
			vs->curmode = 1;
			break;
		case 7:		// SO_VERB_OFF
			vs->curmode = 0;
			break;
		case 8:		// SO_VERB_DELETE
			killVerb(slot);
			break;
		case 9:		// SO_VERB_NEW
			slot = getVerbSlot(verb, 0);
			if (slot == 0) {
				for (slot = 1; slot < _numVerbs; slot++) {
					if (_verbs[slot].verbid == 0)
						break;
				}
				if (slot == _numVerbs)
					error("Too many verbs");
			}
			vs = &_verbs[slot];
			vs->verbid = verb;
			vs->color = 2;
			vs->hicolor = (_game.version == 3) ? 14 : 0;
			vs->dimcolor = 8;
			vs->type = kTextVerbType;
			vs->charset_nr = _string[0]._default.charset;
			vs->curmode = 0;
			vs->saveid = 0;
			vs->key = 0;
			vs->center = 0;
			vs->imgindex = 0;
			break;

		case 16:	// SO_VERB_DIMCOLOR
			vs->dimcolor = getVarOrDirectByte(PARAM_1);
			break;
		case 17:	// SO_VERB_DIM
			vs->curmode = 2;
			break;
		case 18:	// SO_VERB_KEY
			vs->key = getVarOrDirectByte(PARAM_1);
			break;
		case 19:	// SO_VERB_CENTER
			vs->center = 1;
			break;
		case 20:	// SO_VERB_NAME_STR
			ptr = getResourceAddress(rtString, getVarOrDirectWord(PARAM_1));
			if (!ptr)
				_res->nukeResource(rtVerb, slot);
			else {
				loadPtrToResource(rtVerb, slot, ptr);
			}
			if (slot == 0)
				_res->nukeResource(rtVerb, slot);
			vs->type = kTextVerbType;
			vs->imgindex = 0;
			break;
		case 22:										/* assign object */
			a = getVarOrDirectWord(PARAM_1);
			b = getVarOrDirectByte(PARAM_2);
			if (slot && vs->imgindex != a) {
				setVerbObject(b, a, slot);
				vs->type = kImageVerbType;
				vs->imgindex = a;
			}
			break;
		case 23:										/* set back color */
			vs->bkcolor = getVarOrDirectByte(PARAM_1);
			break;
		default:
			error("o5_verbOps: unknown subopcode %d", _opcode & 0x1F);
		}
	}

	// Force redraw of the modified verb slot
	drawVerb(slot, 0);
	verbMouseOver(0);
}

void ScummEngine_v5::o5_wait() {
	const byte *oldaddr = _scriptPointer - 1;

	if ((_game.id == GID_INDY3) && !(_game.platform == Common::kPlatformMacintosh)) {
		_opcode = 2;
	} else
		_opcode = fetchScriptByte();

	switch (_opcode & 0x1F) {
	case 1:		// SO_WAIT_FOR_ACTOR
		{
			Actor *a = derefActorSafe(getVarOrDirectByte(PARAM_1), "o5_wait");
			if (a && a->_moving)
				break;
			return;
		}
	case 2:		// SO_WAIT_FOR_MESSAGE
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case 3:		// SO_WAIT_FOR_CAMERA
		if (camera._cur.x / 8 != camera._dest.x / 8)
			break;
		return;
	case 4:		// SO_WAIT_FOR_SENTENCE
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
			break;
		}
		if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;
		break;
	default:
		error("o5_wait: unknown subopcode %d", _opcode & 0x1F);
		return;
	}

	_scriptPointer = oldaddr;
	o5_breakHere();
}

void ScummEngine_v5::o5_walkActorTo() {
	int x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o5_walkActorTo");
	x = getVarOrDirectWord(PARAM_2);
	y = getVarOrDirectWord(PARAM_3);
	a->startWalkActor(x, y, -1);
}

void ScummEngine_v5::o5_walkActorToActor() {
	int x, y;
	Actor *a, *a2;
	int nr = getVarOrDirectByte(PARAM_1);
	int nr2 = getVarOrDirectByte(PARAM_2);
	int dist = fetchScriptByte();

	if (_game.id == GID_LOOM && _game.version == 4 && nr == 1 && nr2 == 0 &&
		dist == 255 && vm.slot[_currentScript].number == 98) {
		// WORKAROUND bug #743615: LoomCD script 98 contains this:
		//   walkActorToActor(1,0,255)
		// Once again this is either a script bug, or there is some hidden
		// or unknown meaning to this odd walk request...
		return;
	}

	if (_game.id == GID_INDY4 && nr == 1 && nr2 == 106 &&
		dist == 255 && vm.slot[_currentScript].number == 210) {
		// WORKAROUND bug: Work around an invalid actor bug when using the
		// camel in Fate of Atlantis, the "wits" path. The room-65-210 script
		// contains this:
		//   walkActorToActor(1,106,255)
		// Once again this is either a script bug, or there is some hidden
		// or unknown meaning to this odd walk request...
		return;
	}

	a = derefActor(nr, "o5_walkActorToActor");
	if (!a->isInCurrentRoom())
		return;

	a2 = derefActor(nr2, "o5_walkActorToActor(2)");
	if (!a2->isInCurrentRoom())
		return;

	if (_game.version <= 2) {
		dist *= V12_X_MULTIPLIER;
	} else if (dist == 0xFF) {
		dist = a->_scalex * a->_width / 0xFF;
		dist += (a2->_scalex * a2->_width / 0xFF) / 2;
	}
	x = a2->getPos().x;
	y = a2->getPos().y;
	if (x < a->getPos().x)
		x += dist;
	else
		x -= dist;

	if (_game.version <= 2) {
		x /= V12_X_MULTIPLIER;
		y /= V12_Y_MULTIPLIER;
	}
	if (_game.version <= 3) {
		AdjustBoxResult abr = a->adjustXYToBeInBox(x, y);
		x = abr.x;
		y = abr.y;
	}
	a->startWalkActor(x, y, -1);
}

void ScummEngine_v5::o5_walkActorToObject() {
	int obj;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o5_walkActorToObject");
	obj = getVarOrDirectWord(PARAM_2);
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		int x, y, dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	}
}

int ScummEngine_v5::getWordVararg(int *ptr) {
	int i;

	for (i = 0; i < 16; i++)
		ptr[i] = 0;

	i = 0;
	while ((_opcode = fetchScriptByte()) != 0xFF) {
		ptr[i++] = getVarOrDirectWord(PARAM_1);
	}
	return i;
}

void ScummEngine_v5::decodeParseString() {
	int textSlot;

	switch (_actorToPrintStrFor) {
	case 252:
		textSlot = 3;
		break;
	case 253:
		textSlot = 2;
		break;
	case 254:
		textSlot = 1;
		break;
	default:
		textSlot = 0;
	}

	_string[textSlot].loadDefault();

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0xF) {
		case 0:		// SO_AT
			_string[textSlot].xpos = getVarOrDirectWord(PARAM_1);
			_string[textSlot].ypos = getVarOrDirectWord(PARAM_2);
			_string[textSlot].overhead = false;
			break;
		case 1:		// SO_COLOR
			_string[textSlot].color = getVarOrDirectByte(PARAM_1);
			break;
		case 2:		// SO_CLIPPED
			_string[textSlot].right = getVarOrDirectWord(PARAM_1);
			break;
		case 3:		// SO_ERASE
			{
			int w = getVarOrDirectWord(PARAM_1);
			int h = getVarOrDirectWord(PARAM_2);
			// restoreCharsetBg(xpos, xpos + w, ypos, ypos + h)
			error("ScummEngine_v5::decodeParseString: Unhandled case 3: %d, %d", w, h);
			}
			break;
		case 4:		// SO_CENTER
			_string[textSlot].center = true;
			_string[textSlot].overhead = false;
			break;
		case 6:		// SO_LEFT
			if (_game.version == 3) {
				_string[textSlot].height = getVarOrDirectWord(PARAM_1);
			} else {
				_string[textSlot].center = false;
				_string[textSlot].overhead = false;
			}
			break;
		case 7:		// SO_OVERHEAD
			_string[textSlot].overhead = true;
			break;
		case 8:{	// SO_SAY_VOICE
				int offset = (uint16)getVarOrDirectWord(PARAM_1);
				int delay = (uint16)getVarOrDirectWord(PARAM_2);

				if (_game.id == GID_LOOM && _game.version == 4) {
					if (offset == 0 && delay == 0) {
						VAR(VAR_MUSIC_TIMER) = 0;
						_sound->stopCD();
					} else {
						// Loom specified the offset from the start of the CD;
						// thus we have to subtract the length of the first track
						// (22500 frames) plus the 2 second = 150 frame leadin.
						// I.e. in total 22650 frames.
						offset = (int)(offset * 7.5 - 22500 - 2*75);

						// Slightly increase the delay (5 frames = 1/25 of a second).
						// This noticably improves the experience in Loom CD.
						delay = (int)(delay * 7.5 + 5);

						_sound->playCDTrack(1, 0, offset, delay);
					}
				} else {
					error("ScummEngine_v5::decodeParseString: Unhandled case 8");
				}
			}
			break;
		case 15:{	// SO_TEXTSTRING
				const int len = resStrLen(_scriptPointer);

				if (_game.id == GID_LOOM && strcmp((const char *) _scriptPointer, "I am Choas.") == 0) {
					// WORKAROUND: This happens when Chaos introduces
					// herself to bishop Mandible. Of all the places to put
					// a typo...
					printString(textSlot, (const byte *) "I am Chaos.");
				} else if (_game.id == GID_INDY4 && _roomResource == 23 && vm.slot[_currentScript].number == 167 &&
						len == 24 && 0==memcmp(_scriptPointer+16, "pregod", 6)) {
					// WORKAROUND for bug #1621210.
					byte tmpBuf[25];
					memcpy(tmpBuf, _scriptPointer, 25);
					if (tmpBuf[22] == '8')
						strcpy((char *)tmpBuf+16, "^18^");
					else
						strcpy((char *)tmpBuf+16, "^19^");
					printString(textSlot, tmpBuf);
				} else {
					printString(textSlot, _scriptPointer);
				}
				_scriptPointer += len + 1;
			}


			// In SCUMM V1-V3, there were no 'default' values for the text slot
			// values. Hence to achieve correct behaviour, we have to keep the
			// 'default' values in sync with the active values.
			//
			// Note: This is needed for Indy3 (Grail Diary). It's also needed
			// for Loom, or the lines Bobbin speaks during the intro are put
			// at position 0,0.
			//
			// Note: We can't use saveDefault() here because we only want to
			// save the position and color. In particular, we do not want to
			// save the 'center' flag. See bug #933168.
			if (_game.version <= 3) {
				_string[textSlot]._default.xpos = _string[textSlot].xpos;
				_string[textSlot]._default.ypos = _string[textSlot].ypos;
				_string[textSlot]._default.height = _string[textSlot].height;
				_string[textSlot]._default.color = _string[textSlot].color;
			}
			return;
		default:
			error("ScummEngine_v5::decodeParseString: Unhandled case %d", _opcode & 0xF);
		}
	}

	_string[textSlot].saveDefault();
}

void ScummEngine_v5::o5_oldRoomEffect() {
	int a;

	_opcode = fetchScriptByte();
	if ((_opcode & 0x1F) == 3) {
		a = getVarOrDirectWord(PARAM_1);

#if 1
		if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {
			// FIXME / TODO: OK the first thing to note is: at least in Zak256,
			// maybe also in other games, this opcode does a bit more. I added
			// some stubs here, but somebody with a full IDA or more knowledge
			// about this will have to fill in the gaps. At least now we know
			// that something is missing here :-)

			if (a == 4) {
				//printf("o5_oldRoomEffect ODDBALL: _opcode = 0x%x, a = 0x%x\n", _opcode, a);
				// No idea what byte_2FCCF is, but it's a globale boolean flag.
				// I only add it here as a temporary hack to make the pseudo code compile.
				// Maybe it is just there as a reentry protection guard, given
				// how it is used? It might also correspond to _screenEffectFlag.
				int byte_2FCCF = 0;

				// For now, we force a redraw of the screen background. This
				// way the Zak end credits seem to work mostly correct.
				VirtScreen *vs = &_virtscr[kMainVirtScreen];
				restoreBackground(Common::Rect(0, vs->topline, vs->w, vs->topline + vs->h));
				vs->setDirtyRange(0, vs->h);
				updateDirtyScreen(kMainVirtScreen);

				if (byte_2FCCF) {
					// Here now "sub_1C44" is called, which sets byte_2FCCF to 0 then
					// calls yet another sub (which also reads byte_2FCCF):

					byte_2FCCF = 0;
					//call sub_0BB3


					// Now sub_085C is called. This is quite simply: it sets
					// 0xF000 bytes. starting at 0x40000 to 0. No idea what that
					// buffer is, maybe a screen buffer, though. Note that
					// 0xF000 = 320*192.
					// Maybe this is also the charset mask being cleaned?

					// call sub_085C


					// And then sub_1C54 is called, which is almost identical to
					// the above sub_1C44, only it sets byte_2FCCF to 1:

					byte_2FCCF = 1;
					// call sub_0BB3

				} else {
					// Here only sub_085C is called (see comment above)

					// call sub_085C
				}
			return;
			}
#endif

		}
		if (a) {
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
	}
}

void ScummEngine_v5::o5_pickupObjectOld() {
	int obj = getVarOrDirectWord(PARAM_1);

	if (obj < 1) {
		error("pickupObjectOld received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	/* Don't take an */
		return;											/* object twice */

	// debug(0, "adding %d from %d to inventoryOld", obj, _currentRoom);
	addObjectToInventory(obj, _roomResource);
	markObjectRectAsDirty(obj);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	clearDrawObjectQueue();
	runInventoryScript(1);
}

#undef PARAM_1
#undef PARAM_2
#undef PARAM_3

} // End of namespace Scumm
