/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#define OPCODE(x)	{ &Scumm_v5::x, #x }

void Scumm_v5::setupOpcodes() {
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
		OPCODE(o5_actorSet),
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
		OPCODE(o5_actorSet),
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
		OPCODE(o5_actorSet),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o5_and),
		/* 98 */
		OPCODE(o5_quitPauseRestart),
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
		OPCODE(o5_actorSet),
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

void Scumm_v5::executeOpcode(byte i) {
	OpcodeProcV5 op = _opcodesV5[i].proc;
	(this->*op) ();
}

const char *Scumm_v5::getOpcodeDesc(byte i) {
	return _opcodesV5[i].desc;
}

int Scumm_v5::getVar() {
	return readVar(fetchScriptWord());
}

int Scumm_v5::getVarOrDirectByte(byte mask) {
	if (_opcode & mask)
		return getVar();
	return fetchScriptByte();
}

int Scumm_v5::getVarOrDirectWord(byte mask) {
	if (_opcode & mask)
		return getVar();
	return (int16)fetchScriptWord();
}

void Scumm_v5::o5_actorFollowCamera() {
	actorFollowCamera(getVarOrDirectByte(0x80));
}

void Scumm_v5::o5_actorFromPos() {
	int x, y;
	getResultPos();
	x = getVarOrDirectWord(0x80);
	y = getVarOrDirectWord(0x40);
	setResult(getActorFromPos(x, y));
}

void Scumm_v5::o5_actorSet() {
	static const byte convertTable[20] =
		{ 1, 0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20 };
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o5_actorSet");
	int i, j;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		if (_features & GF_SMALL_HEADER)
			_opcode = (_opcode & 0xE0) | convertTable[(_opcode & 0x1F) - 1];

		switch (_opcode & 0x1F) {
		case 0:										/* dummy case */
			getVarOrDirectByte(0x80);
			break;
		case 1:										/* costume */
			a->setActorCostume(getVarOrDirectByte(0x80));
			break;
		case 2:										/* walkspeed */
			i = getVarOrDirectByte(0x80);
			j = getVarOrDirectByte(0x40);
			a->setActorWalkSpeed(i, j);
			break;
		case 3:										/* sound */
			a->sound[0] = getVarOrDirectByte(0x80);
			break;
		case 4:										/* walkanim */
			a->walkFrame = getVarOrDirectByte(0x80);
			break;
		case 5:										/* talkanim */
			a->talkStartFrame = getVarOrDirectByte(0x80);
			a->talkStopFrame = getVarOrDirectByte(0x40);
			break;
		case 6:										/* standanim */
			a->standFrame = getVarOrDirectByte(0x80);
			break;
		case 7:										/* ignore */
			getVarOrDirectByte(0x80);
			getVarOrDirectByte(0x40);
			getVarOrDirectByte(0x20);
			break;
		case 8:										/* init */
			a->initActor(0);
			break;
		case 9:										/* elevation */
			a->elevation = getVarOrDirectWord(0x80);
			a->needRedraw = true;
			break;
		case 10:										/* defaultanims */
			a->initFrame = 1;
			a->walkFrame = 2;
			a->standFrame = 3;
			a->talkStartFrame = 4;
			a->talkStopFrame = 5;
			break;
		case 11:										/* palette */
			i = getVarOrDirectByte(0x80);
			j = getVarOrDirectByte(0x40);
			checkRange(31, 0, i, "Illegal palette slot %d");
			a->palette[i] = j;
			a->needRedraw = true;
			break;
		case 12:										/* talk color */

			// Zak256 (and possibly other games) uses actor 0 to
			// indicate that it's the default talk color that is
			// to be changed.

			if (act == 0)
				_string[0].color = getVarOrDirectByte(0x80);
			else
				a->talkColor = getVarOrDirectByte(0x80);
			break;
		case 13:										/* name */
			loadPtrToResource(rtActorName, a->number, NULL);
			break;
		case 14:										/* initanim */
			a->initFrame = getVarOrDirectByte(0x80);
			break;
		case 15:										/* unk */
			error("o5_actorset:unk not implemented");
#if 0
			int args[16] =
				{
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				};
			getWordVararg(args);
			for (i = 0; i < 16; i++)
				if (args[i] != 0xFF)
					a->palette[i] = args[i];
#endif
			break;
		case 16:										/* width */
			a->width = getVarOrDirectByte(0x80);
			break;
		case 17:										/* scale */
			if ((_gameId == GID_MONKEY_VGA) || (_gameId == GID_MONKEY_EGA) || (_gameId == GID_PASS)) {
				a->scalex = a->scaley = getVarOrDirectByte(0x80);
			} else {
				a->scalex = getVarOrDirectByte(0x80);
				a->scaley = getVarOrDirectByte(0x40);
			}

			a->needRedraw = true;
			break;
		case 18:										/* neverzclip */
			a->forceClip = 0;
			break;
		case 19:										/* setzclip */
			a->forceClip = getVarOrDirectByte(0x80);
			break;
		case 20:										/* ignoreboxes */
		case 21:										/* followboxes */
			a->ignoreBoxes = !(_opcode & 1);
			a->forceClip = 0;
			if (a->isInCurrentRoom())
				a->putActor(a->x, a->y, a->room);
			break;

		case 22:										/* animspeed */
			a->setAnimSpeed(getVarOrDirectByte(0x80));
			break;
		case 23:										/* shadow mode */
			a->shadow_mode = getVarOrDirectByte(0x80);
			break;
		default:
			warning("o5_actorSet: default case");
		}
	}
}

void Scumm_v5::o5_setClass() {
	int obj = getVarOrDirectWord(0x80);
	int newClass;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		newClass = getVarOrDirectWord(0x80);
		if (newClass == 0) {
			// Class '0' means: clean all class data
			_classData[obj] = 0;
			if ((_features & GF_SMALL_HEADER) && obj <= _numActors) {
				Actor *a = derefActor(obj, "o5_setClass");
				a->ignoreBoxes = false;
				a->forceClip = 0;
			}
		} else
			putClass(obj, newClass, (newClass & 0x80) ? true : false);
	}
}

void Scumm_v5::o5_add() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) + a);
}

void Scumm_v5::o5_and() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) & a);
}

void Scumm_v5::o5_animateActor() {
	int act = getVarOrDirectByte(0x80);
	int anim = getVarOrDirectByte(0x40);
	Actor *a = derefActor(act, "o5_animateActor");
	a->animateActor(anim);
}

void Scumm_v5::o5_breakHere() {
	updateScriptPtr();
	_currentScript = 0xFF;
}

void Scumm_v5::o5_chainScript() {
	int vars[16];
	int script;
	int cur;

	script = getVarOrDirectByte(0x80);

	getWordVararg(vars);

	cur = _currentScript;

	// WORKAROUND bug #743314: Work around a bug in script 33 in Indy3 VGA.
	// That script is used for the fist fights in the Zeppeling. It uses
	// Local[5], even though that is never set to any value. But script 33 is
	// called via chainScript by script 32, and in there Local[5] is defined
	// to the  actor ID of the opposing soldier. So, we copy that value over
	// to the Local[5] variable of script 33.
	if (_gameId == GID_INDY3 && vm.slot[cur].number == 32 && script == 33) {
		vars[5] = vm.localvar[cur][5];
	}

	vm.slot[cur].number = 0;
	vm.slot[cur].status = 0;
	_currentScript = 0xFF;

	runScript(script, vm.slot[cur].freezeResistant, vm.slot[cur].recursive, vars);
}

void Scumm_v5::o5_cursorCommand() {
	int i, j, k;
	int table[16];
	switch ((_opcode = fetchScriptByte()) & 0x1F) {
	case 1:											/* cursor show */
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 2:											/* cursor hide */
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case 3:											/* userput on */
		_userPut = 1;
		break;
	case 4:											/* userput off */
		_userPut = 0;
		break;
	case 5:											/* cursor soft on */
		_cursor.state++;
		verbMouseOver(0);
		break;
	case 6:											/* cursor soft off */
		_cursor.state--;
		verbMouseOver(0);
		break;
	case 7:											/* userput soft on */
		_userPut++;
		break;
	case 8:											/* userput soft off */
		_userPut--;
		break;
	case 10:											/* set cursor img */
		i = getVarOrDirectByte(0x80);
		j = getVarOrDirectByte(0x40);
		// cursor image in both Looms is based on image from charset
		// omit for now.
		// FIXME: Actually: is this opcode ever called by a non-Loom game?
		// Which V3-V5 game besides Loom makes use of custom cursors, ever?
		if (_gameId != GID_LOOM && _gameId != GID_LOOM256) {
			warning("setCursorImg called - tell Fingolfin where you saw this!");
			setCursorImg(i, j, 1);
		}
		break;
	case 11:											/* set cursor hotspot */
		i = getVarOrDirectByte(0x80);
		j = getVarOrDirectByte(0x40);
		k = getVarOrDirectByte(0x20);
		setCursorHotspot(j, k);
		break;
	case 12:											/* init cursor */
		setCursor(getVarOrDirectByte(0x80));
		break;
	case 13:											/* init charset */
		initCharset(getVarOrDirectByte(0x80));
		break;
	case 14:											/* unk */
		if (_gameId == GID_LOOM || _gameId == GID_INDY3) {
			/*int a = */ getVarOrDirectByte(0x80);
			/*int b = */ getVarOrDirectByte(0x40);
			// This is some kind of "init charset" opcode. However, we don't have to do anything
			// in here, as our initCharset automatically calls loadCharset for GF_SMALL_HEADER,
			// games if needed.
		} else {
			getWordVararg(table);
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[1].t_charset][i] = (unsigned char)table[i];
		}
		break;
	}

	if (!(_gameId == GID_LOOM || _gameId == GID_INDY3)) {
		VAR(VAR_CURSORSTATE) = _cursor.state;
		VAR(VAR_USERPUT) = _userPut;
	}
}

void Scumm_v5::o5_cutscene() {
	int args[16];
	getWordVararg(args);
	beginCutscene(args);
}

void Scumm_v5::o5_endCutscene() {
	endCutscene();
}

void Scumm_v5::o5_debug() {
	int a = getVarOrDirectWord(0x80);
	debug(1, "o5_debug(%d)", a);
}

void Scumm_v5::o5_decrement() {
	getResultPos();
	setResult(readVar(_resultVarNumber) - 1);
}

void Scumm_v5::o5_delay() {
	int delay = fetchScriptByte();
	delay |= fetchScriptByte() << 8;
	delay |= fetchScriptByte() << 16;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o5_breakHere();
}

void Scumm_v5::o5_delayVariable() {
	vm.slot[_currentScript].delay = getVar();
	vm.slot[_currentScript].status = 1;
	o5_breakHere();
}

void Scumm_v5::o5_divide() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	if (a == 0) {
		error("Divide by zero");
		setResult(0);
	} else
		setResult(readVar(_resultVarNumber) / a);
}

void Scumm_v5::o5_doSentence() {
	int verb;
	SentenceTab *st;

	verb = getVarOrDirectByte(0x80);
	if (verb == 0xFE) {
		_sentenceNum = 0;
		stopScript(VAR(VAR_SENTENCE_SCRIPT));
		clearClickedStatus();
		return;
	}

	st = &_sentence[_sentenceNum++];

	st->verb = verb;
	st->objectA = getVarOrDirectWord(0x40);
	st->objectB = getVarOrDirectWord(0x20);
	st->preposition = (st->objectB != 0);
	st->freezeCount = 0;
}

void Scumm_v5::o5_drawBox() {
	int x, y, x2, y2, color;

	x = getVarOrDirectWord(0x80);
	y = getVarOrDirectWord(0x40);

	_opcode = fetchScriptByte();
	x2 = getVarOrDirectWord(0x80);
	y2 = getVarOrDirectWord(0x40);
	color = getVarOrDirectByte(0x20);

	drawBox(x, y, x2, y2, color);
}

void Scumm_v5::o5_drawObject() {
	int state, obj, idx, i;
	ObjectData *od;
	uint16 x, y, w, h;
	int xpos, ypos;

	state = 1;
	xpos = ypos = 255;
	obj = getVarOrDirectWord(0x80);

	if (_features & GF_SMALL_HEADER) {
		xpos = getVarOrDirectWord(0x40);
		ypos = getVarOrDirectWord(0x20);
	} else {
		_opcode = fetchScriptByte();
		switch (_opcode & 0x1F) {
		case 1:										/* draw at */
			xpos = getVarOrDirectWord(0x80);
			ypos = getVarOrDirectWord(0x40);
			break;
		case 2:										/* set state */
			state = getVarOrDirectWord(0x80);
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
		od->walk_x += (xpos << 3) - od->x_pos;
		od->x_pos = xpos << 3;
		od->walk_y += (ypos << 3) - od->y_pos;
		od->y_pos = ypos << 3;
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

void Scumm_v5::o5_getStringWidth() {
	// TODO - not sure if this is correct... needs testing
	int string, width = 0;
	byte *ptr;
	
	getResultPos();
	string = getVarOrDirectByte(0x80);
	ptr = getResourceAddress(rtString, string);
	assert(ptr);

	width = _charset->getStringWidth(0, ptr);
	
	setResult(width);
	warning("o5_getStringWidth, result %d", width);
}

void Scumm_v5::o5_saveLoadVars() {
	// TODO
	if (fetchScriptByte() == 1) 
		saveVars();
	else
		loadVars();
}

void Scumm_v5::saveVars() {
	int a, b;

	while ((_opcode = fetchScriptByte()) != 0) {
		switch (_opcode & 0x1F) {
			case 0x01: // write a range of variables
				getResultPos();
			        a = _resultVarNumber;
				getResultPos();
			        b = _resultVarNumber;
				warning("stub saveVars: vars %d -> %d", a, b);
				break;

			case 0x02: // write a range of string variables
				a = getVarOrDirectByte(0x80);
				b = getVarOrDirectByte(0x40);
				warning("stub saveVars: strings %d -> %d", a, b);
				break;
			case 0x03: // open file
				a = resStrLen(_scriptPointer);
				warning("stub saveVars to %s", _scriptPointer);
				_scriptPointer += a + 1;
				break;
			case 0x1F: // close file
				warning("stub saveVars close file");
				break;
		}

	}
}

void Scumm_v5::loadVars() {
	int a, b;

	hexdump(_scriptPointer, 64);
	while ((_opcode = fetchScriptByte()) != 0) {
		switch (_opcode & 0x1F) {
			case 0x01: // read a range of variables
				getResultPos();
			        a = _resultVarNumber;
				getResultPos();
			        b = _resultVarNumber;
				warning("stub loadVars: vars %d -> %d", a, b);
				break;
			case 0x02: // read a range of string variables
				a = getVarOrDirectByte(0x80);
				b = getVarOrDirectByte(0x40);
				warning("stub loadVars: strings %d -> %d", a, b);
				break;
			case 0x03: // open file
				a = resStrLen(_scriptPointer);
				warning("stub loadVars from %s", _scriptPointer);
				_scriptPointer += a + 1;
				break;
			case 0x1F: // close file
				warning("stub loadVars close file");
				break;
		}
				
	}

}

void Scumm_v5::o5_expression() {
	int dst, i;

	_scummStackPos = 0;
	getResultPos();
	dst = _resultVarNumber;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0x1F) {
		case 1:										/* varordirect */
			push(getVarOrDirectWord(0x80));
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

void Scumm_v5::o5_faceActor() {
	int act = getVarOrDirectByte(0x80);
	int obj = getVarOrDirectWord(0x40);
	Actor *a = derefActor(act, "o5_faceActor");
	a->faceToObject(obj);
}

void Scumm_v5::o5_findInventory() {
	getResultPos();
	int x = getVarOrDirectByte(0x80);
	int y = getVarOrDirectByte(0x40);
	setResult(findInventory(x, y));
}

void Scumm_v5::o5_findObject() {
	getResultPos();
	int x = getVarOrDirectByte(0x80);
	int y = getVarOrDirectByte(0x40);
	setResult(findObject(x, y));
}

void Scumm_v5::o5_freezeScripts() {
	int scr = getVarOrDirectByte(0x80);

	if (scr != 0)
		freezeScripts(scr);
	else
		unfreezeScripts();
}

void Scumm_v5::o5_getActorCostume() {
	getResultPos();
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o5_getActorCostume");
	setResult(a->costume);
}

void Scumm_v5::o5_getActorElevation() {
	getResultPos();
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o5_getActorElevation");
	setResult(a->elevation);
}

void Scumm_v5::o5_getActorFacing() {
	getResultPos();
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o5_getActorFacing");
	setResult(newDirToOldDir(a->getFacing()));
}

void Scumm_v5::o5_getActorMoving() {
	getResultPos();
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o5_getActorMoving");
	setResult(a->moving);
}

void Scumm_v5::o5_getActorRoom() {
	getResultPos();
	int act = getVarOrDirectByte(0x80);
	// WORKAROUND bug #746349. This is a really odd bug in either the script
	// or in our script engine. Might be a good idea to investigate this
	// further by e.g. looking at the FOA engine a bit closer.
	if (_gameId == GID_INDY4 && _roomResource == 94 && vm.slot[_currentScript].number == 206 && act > _numActors) {
		setResult(0);
		return;
	}
	
	Actor *a = derefActor(act, "o5_getActorRoom");
	setResult(a->room);
}

void Scumm_v5::o5_getActorScale() {
	Actor *a;
	
	// dummy opcode in the loom
	if (_gameId == GID_LOOM)
		return;

	// INDY3 uses this opcode as a wait_for_actor();
	if (_gameId == GID_INDY3) {
		const byte *oldaddr = _scriptPointer - 1;
		a = derefActor(getVarOrDirectByte(0x80), "o5_getActorScale (wait)");
		if (a->moving) {
			_scriptPointer = oldaddr;
			o5_breakHere();
		}
		return;
	}

	getResultPos();
	int act = getVarOrDirectByte(0x80);
	a = derefActor(act, "o5_getActorScale");
	setResult(a->scalex);
}

void Scumm_v5::o5_getActorWalkBox() {
	getResultPos();
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o5_getActorWalkBox");
	setResult(a->walkbox);
}

void Scumm_v5::o5_getActorWidth() {
	getResultPos();
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o5_getActorWidth");
	setResult(a->width);
}

void Scumm_v5::o5_getActorX() {
	int a;
	getResultPos();

	if ((_gameId == GID_INDY3) && !(_features & GF_MACINTOSH))
		a = getVarOrDirectByte(0x80);
	else
		a = getVarOrDirectWord(0x80);

	setResult(getObjX(a));
}

void Scumm_v5::o5_getActorY() {
	int a;
	getResultPos();

	if ((_gameId == GID_INDY3) && !(_features & GF_MACINTOSH)) {
		a = getVarOrDirectByte(0x80);

		// WORKAROUND bug #636433 (can't get into Zeppelin) 
		if (_roomResource == 36) {
			setResult(getObjY(a) - 1);
			return;
		}
	} else
		a = getVarOrDirectWord(0x80);

	setResult(getObjY(a));
}

void Scumm_v5::o5_saveLoadGame() {
	getResultPos();
	byte a = getVarOrDirectByte(0x80);
	byte slot = (a & 0x1F) + 1;
	byte result = 0;
		
	if ((_gameId == GID_MANIAC) && (_version == 1)) {
		// Convert older load/save screen
		// 1 Load
		// 2 Save
		slot = 1;
		if (a == 1)
			_opcode = 0x40;
		else if (a == 2)
			_opcode = 0x80;
	} else
		_opcode = a & 0xE0;

	switch (_opcode) {
		case 0x00: // num slots available
			result = 100;
			break;
		case 0x20: // dos drive?
			result = 0;
			break;
		case 0x40: // load 
			if (loadState(slot, _saveLoadCompatible))
				result = 3; // sucess
			else
				result = 5; // failed to load
			break;
		case 0x80: // save
			if (saveState(slot, _saveLoadCompatible))
				result = 0;
			else
				result = 2;
			break;
		case 0xC0: // test if save exists
			bool avail_saves[100];
			char filename[256];
			SaveFileManager *mgr = _system->get_savefile_manager();
			listSavegames(avail_saves, ARRAYSIZE(avail_saves), mgr);
			makeSavegameName(filename, slot, false);
			if (avail_saves[slot] && (mgr->open_savefile(filename, getSavePath(), false)))
				result = 6; // save file exists
			else
				result = 7; // save file does not exist
			break;
	}
	setResult(result);
}
		
void Scumm_v5::o5_getAnimCounter() {
	if (_version == 3) {
		o5_saveLoadGame();
		return;
	}
		
	getResultPos();

	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o5_getAnimCounter");
	setResult(a->cost.animCounter);
}

void Scumm_v5::o5_getClosestObjActor() {
	int obj;
	int act;
	int dist;

	// This is a bit odd: We can't detect any actors farther away than
	// 255 units (pixels in newer games, characters in older ones.) To
	// fix this, we also need to change getObjActToObjActDist(), since
	// it returns 255 to indicate that it can't find the actor, and make
	// sure we don't break o5_getDist() in the process.
	//
	// But we probably won't have to.

	int closest_obj = 0xFF, closest_dist = 0xFF;

	getResultPos();

	act = getVarOrDirectWord(0x80);
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

void Scumm_v5::o5_getDist() {
	int o1, o2;
	int r;
	getResultPos();
	o1 = getVarOrDirectWord(0x80);
	o2 = getVarOrDirectWord(0x40);
	r = getObjActToObjActDist(o1, o2);

	// FIXME: MI2 race workaround, see bug #597022
	if (_gameId == GID_MONKEY2 && vm.slot[_currentScript].number == 40 && r < 60) 
		r = 60; 

	setResult(r);
}

void Scumm_v5::o5_getInventoryCount() {
	getResultPos();
	setResult(getInventoryCount(getVarOrDirectByte(0x80)));
}

void Scumm_v5::o5_getObjectOwner() {
	getResultPos();
	setResult(getOwner(getVarOrDirectWord(0x80)));
}

void Scumm_v5::o5_getObjectState() {
	if (_features & GF_SMALL_HEADER) {
		o5_ifState();
	} else {
		getResultPos();
		setResult(getState(getVarOrDirectWord(0x80)));
	}
}

void Scumm_v5::o5_ifState() {
	int a = getVarOrDirectWord(0x80);
	int b = getVarOrDirectByte(0x40);

	if (getState(a) != b)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v5::o5_ifNotState() {
	int a = getVarOrDirectWord(0x80);
	int b = getVarOrDirectByte(0x40);

	if (getState(a) == b)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v5::o5_getRandomNr() {
	getResultPos();
	setResult(_rnd.getRandomNumber(getVarOrDirectByte(0x80)));
}

void Scumm_v5::o5_isScriptRunning() {
	getResultPos();
	setResult(isScriptRunning(getVarOrDirectByte(0x80)));
}

void Scumm_v5::o5_getVerbEntrypoint() {
	int a, b;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	b = getVarOrDirectWord(0x40);

	setResult(getVerbEntrypoint(a, b));
}

void Scumm_v5::o5_ifClassOfIs() {
	int act, cls, b = 0;
	bool cond = true;

	act = getVarOrDirectWord(0x80);

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		cls = getVarOrDirectWord(0x80);

		if (!cls) // FIXME: Ender can't remember why this is here,
			b = false;  // but it fixes an oddball zak256 crash
		else
			b = getClass(act, cls);

		if (cls & 0x80 && !b || !(cls & 0x80) && b)
			cond = false;
	}
	if (cond)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_increment() {
	getResultPos();
	setResult(readVar(_resultVarNumber) + 1);
}

void Scumm_v5::o5_isActorInBox() {
	int act = getVarOrDirectByte(0x80);
	int box = getVarOrDirectByte(0x40);
	Actor *a = derefActor(act, "o5_isActorInBox");

	if (!checkXYInBoxBounds(box, a->x, a->y))
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v5::o5_isEqual() {
	int16 a, b;
	int var;

	if (_version <= 2)
		var = fetchScriptByte();
	else
		var = fetchScriptWord();
	a = readVar(var);
	b = getVarOrDirectWord(0x80);

	// HACK: See bug report #602348. The sound effects for Largo's screams
	// are only played on type 5 soundcards. However, there is at least one
	// other sound effect (the bartender spitting) which is only played on
	// type 3 soundcards.

	if (_gameId == GID_MONKEY2 && var == VAR_SOUNDCARD && b == 5)
		b = a;

	if (b == a)
		ignoreScriptWord();
	else
		o5_jumpRelative();

}

void Scumm_v5::o5_isGreater() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(0x80);
	if (b > a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_isGreaterEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(0x80);
	if (b >= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_isLess() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(0x80);

	if (b < a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_lessOrEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(0x80);
	if (b <= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_isNotEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(0x80);
	if (b != a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_notEqualZero() {
	int a = getVar();
	if (a != 0)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_equalZero() {
	int a = getVar();
	if (a == 0)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_jumpRelative() {
	_scriptPointer += (int16)fetchScriptWord();
}

void Scumm_v5::o5_lights() {
	int a, b, c;

	a = getVarOrDirectByte(0x80);
	b = fetchScriptByte();
	c = fetchScriptByte();

	if (c == 0)
		VAR(VAR_CURRENT_LIGHTS) = a;
	else if (c == 1) {
		_flashlight.xStrips = a;
		_flashlight.yStrips = b;
	}
	_fullRedraw = 1;
}

void Scumm_v5::o5_loadRoom() {
	int room;

	room = getVarOrDirectByte(0x80);
	
	// For small header games, we only call startScene if the room
	// actually changed. This avoid unwanted (wrong) fades in Zak256
	// and others. OTOH, it seems to cause a problem in newer games.
	if (!(_features & GF_SMALL_HEADER) || room != _currentRoom)
		startScene(room, 0, 0);

	// FIXME: Incredibly nasty evil hack to fix bug #770699 (During meeting
	// with Guru, script 42 changes between room 0 and room 19 to create
	// 'some time later' effects. On switching back to room 19, the camera
	// reverts to 0,0 - Added for 0.5.0, should be fixed properly
	if (_gameId == GID_ZAK && (vm.slot[_currentScript].number == 42) && (room == 19))
		setCameraAt(480, 0);

	_fullRedraw = 1;
}

void Scumm_v5::o5_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y;

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);

	a = derefActor(VAR(VAR_EGO), "o5_loadRoomWithEgo");

	a->putActor(0, 0, room);
	_egoPositioned = false;

	x = (int16)fetchScriptWord();
	y = (int16)fetchScriptWord();

	VAR(VAR_WALKTO_OBJ) = obj;
	startScene(a->room, a, obj);
	VAR(VAR_WALKTO_OBJ) = 0;

	// FIXME: Can this be removed?
	camera._cur.x = a->x;

	setCameraAt(a->x, a->y);
	setCameraFollows(a);

	_fullRedraw = 1;

	if (x != -1) {
		a->startWalkActor(x, y, -1);
	}
}

void Scumm_v5::o5_matrixOps() {
	int a, b;

	if (_version == 3) {
		a = getVarOrDirectByte(0x80);
		b = fetchScriptByte();
		setBoxFlags(a, b);
		return;
	}

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxFlags(a, b);
		break;
	case 2:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxScale(a, b);
		break;
	case 3:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxScale(a, (b - 1) | 0x8000);
		break;
	case 4:
		createBoxMatrix();
		break;
	}
}

void Scumm_v5::o5_move() {
	getResultPos();
	setResult(getVarOrDirectWord(0x80));
}

void Scumm_v5::o5_multiply() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) * a);
}

void Scumm_v5::o5_or() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) | a);
}

void Scumm_v5::o5_beginOverride() {
	if (fetchScriptByte() != 0)
		beginOverride();
	else
		endOverride();
}

void Scumm_v5::o5_panCameraTo() {
	panCameraTo(getVarOrDirectWord(0x80), 0);
}

void Scumm_v5::o5_pickupObject() {
	int obj, room;
	if (_version == 3 || _version == 4) {
		o5_drawObject();
		return;
	}

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);
	if (room == 0)
		room = _roomResource;
	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
	runInventoryScript(1);
}

void Scumm_v5::o5_print() {
	_actorToPrintStrFor = getVarOrDirectByte(0x80);
	decodeParseString();
}

void Scumm_v5::o5_printEgo() {
	_actorToPrintStrFor = (byte)VAR(VAR_EGO);
	decodeParseString();
}

void Scumm_v5::o5_pseudoRoom() {
	int i = fetchScriptByte(), j;
	while ((j = fetchScriptByte()) != 0) {
		if (j >= 0x80) {
			_resourceMapper[j & 0x7F] = i;
		}
	}
}

void Scumm_v5::o5_putActor() {
	int x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(0x80), "o5_putActor");
	x = getVarOrDirectWord(0x40);
	y = getVarOrDirectWord(0x20);
	a->putActor(x, y, a->room);
}

void Scumm_v5::o5_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(0x80), "o5_putActorAtObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj, x, y);
	else {
		x = 240;
		y = 120;
	}
	a->putActor(x, y, a->room);
}

void Scumm_v5::o5_putActorInRoom() {
	Actor *a;
	int act = getVarOrDirectByte(0x80);
	int room = getVarOrDirectByte(0x40);

	a = derefActor(act, "o5_putActorInRoom");
	
	// WORKAROUND: Fix for Bug #770710
	// This fixes conflicting actor usage by two different scripts
	// in MI1 Demo (PC GID_MONKEY_EGA or Amiga GID_MONKEY_VGA). The
	// exit script for the LucasArts logo screen attempts to reset an
	// actor that has already been set up for the next scene by the
	// boot script. The fix cannot be used as a general behavior 
	// because it does cause GFX glitches in other games that try to
	// do legitimate cleanup of actors that are in another room.
	// (The Indy3 "Indy at Donovan's" cutscene, for instance.)
	if (_gameId == GID_MONKEY_EGA || _gameId == GID_MONKEY_VGA) {
		if (room == 0 && a->room != _currentRoom && a->room != room && _currentRoom != room) {
			warning ("o5_putActorInRoom (%d [%d], %d) ignored", act, a->room, room);
			return;
		}
	}

	if (a->visible && _currentRoom != room && VAR(VAR_TALK_ACTOR) == a->number) {
		clearMsgQueue();
	}
	a->room = room;
	if (!room)
		a->putActor(0, 0, 0);
}

void Scumm_v5::o5_quitPauseRestart() {
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 1:		// Restart
		restart();
		break;
	case 2:		// Pause
		pauseGame();
		break;
	case 3:		// Quit
		shutDown();
		break;
	default:
		error("o5_quitPauseRestart: unknown subopcode %d", subOp);
	}
}

void Scumm_v5::o5_resourceRoutines() {
	const ResTypes resType[4] = { rtScript, rtSound, rtCostume, rtRoom };
	int resid = 0;
	int foo, bar;

	_opcode = fetchScriptByte();
	if (_opcode != 17)
		resid = getVarOrDirectByte(0x80);
	if (!(_features & GF_FMTOWNS)) {
		// FIXME - this probably can be removed eventually, I don't think the following
		// check will ever be triggered, but then I could be wrong and it's better
		// to play it safe.
		if ((_opcode & 0x3F) != (_opcode & 0x1F))
			error("Oops, this shouldn't happen: o5_resourceRoutines opcode %d", _opcode);
	}

	int op = _opcode & 0x3F;

	switch (_opcode & 0x3F) {
	case 1:											// load script
	case 2:											// load sound
	case 3:											// load costume
		ensureResourceLoaded(resType[op-1], resid);
		break;
	case 4:											// load room 
		if (_version == 3) {
			ensureResourceLoaded(rtRoom, resid);
			if (resid > 0x7F)
				resid = _resourceMapper[resid & 0x7F];

			if (_currentRoom != resid) {
				res.flags[rtRoom][resid] |= 1;
			}
		} else
			ensureResourceLoaded(rtRoom, resid);
		break;

	case 5:											// nuke script
	case 6:											// nuke sound
	case 7:											// nuke costume
	case 8:											// nuke room
		if (_gameId == GID_ZAK256)
			warning("o5_resourceRoutines %d should not occur in Zak256", op);
		else
			setResourceCounter(resType[op-5], resid, 0x7F);
		break;
	case 9:											// lock script
		if (resid >= _numGlobalScripts)
			break;
		lock(rtScript, resid);
		break;
	case 10:											// lock sound
		lock(rtSound, resid);
		break;
	case 11:											// lock costume
		lock(rtCostume, resid);
		break;
	case 12:											// lock room
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		lock(rtRoom, resid);
		break;

	case 13:											// unlock script
		if (resid >= _numGlobalScripts)
			break;
		unlock(rtScript, resid);
		break;
	case 14:											// unlock sound
		unlock(rtSound, resid);
		break;
	case 15:											// unlock costume
		unlock(rtCostume, resid);
		break;
	case 16:											// unlock room
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		unlock(rtRoom, resid);
		break;

	case 17:											// clear heap
		//heapClear(0);
		//unkHeapProc2(0, 0);
		break;
	case 18:											// load charset
		loadCharset(resid);
		break;
	case 19:											// nuke charset
		nukeCharset(resid);
		break;
	case 20:											// load fl object
		loadFlObject(getVarOrDirectWord(0x40), resid);
		break;

	case 32:
		// TODO
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode & 0x3F,  vm.slot[_currentScript].number);
		break;
	case 33:
		// TODO
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode & 0x3F,  vm.slot[_currentScript].number);
		break;
	case 35:
		// TODO
		foo = getVarOrDirectByte(0x40);
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode & 0x3F,  vm.slot[_currentScript].number);
		break;
	case 36:
		// TODO
		foo = getVarOrDirectByte(0x40);
		bar = fetchScriptByte();
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode & 0x3F,  vm.slot[_currentScript].number);
		break;
	case 37:
		// TODO
		foo = getVarOrDirectByte(0x40);
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode & 0x3F,  vm.slot[_currentScript].number);
		break;

	default:
		warning("Unknown o5_resourceRoutines: %d", _opcode & 0x3F);
		break;
	}
}

void Scumm_v5::o5_roomOps() {
	int a = 0, b = 0, c, d, e;

	if (_version == 3) {
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
	}

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:											/* room scroll */
		if (_version != 3) {
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectWord(0x40);
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
	case 2:											/* room color */
		if (_features & GF_SMALL_HEADER) {
			if (_version != 3) {
				a = getVarOrDirectWord(0x80);
				b = getVarOrDirectWord(0x40);
			}
			checkRange(256, 0, a, "o5_roomOps: 2: Illegal room color slot (%d)");
		 	if (_features & GF_16COLOR) {
				_roomPalette[b] = a;
				_fullRedraw = true;
			} else {
				_shadowPalette[b] = a;
				setDirtyColors(b, b);
			}
		} else {
			error("room-color is no longer a valid command");
		}
		break;

	case 3:											/* set screen */
		if (_version != 3) {
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectWord(0x40);
		}
		initScreens(0, a, _screenWidth, b);
		break;
	case 4:											/* set palette color */
		if (_features & GF_SMALL_HEADER) {
			if (_version != 3) {
				a = getVarOrDirectWord(0x80);
				b = getVarOrDirectWord(0x40);
			}
			checkRange(256, 0, a, "o5_roomOps: 2: Illegal room color slot (%d)");
			_shadowPalette[b] = a;
			setDirtyColors(b, b);
		} else {
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectWord(0x40);
			c = getVarOrDirectWord(0x20);
			_opcode = fetchScriptByte();
			d = getVarOrDirectByte(0x80);
			setPalColor(d, a, b, c);	/* index, r, g, b */
		}
		break;
	case 5:											/* shake on */
		setShake(1);
		break;
	case 6:											/* shake off */
		setShake(0);
		break;
	case 7:											/* room scale for old games */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		_opcode = fetchScriptByte();
		c = getVarOrDirectByte(0x80);
		d = getVarOrDirectByte(0x40);
		_opcode = fetchScriptByte();
		e = getVarOrDirectByte(0x40);
		setScaleItem(e - 1, b, a, d, c);
		break;
	case 8:											/* room scale? */
		if (_features & GF_SMALL_HEADER) {
			if (_version != 3) {
				a = getVarOrDirectWord(0x80);
				b = getVarOrDirectWord(0x40);
			}
			c = getVarOrDirectWord(0x20);
		} else {
			a = getVarOrDirectByte(0x80);
			b = getVarOrDirectByte(0x40);
			c = getVarOrDirectByte(0x20);
		}
		darkenPalette(a, a, a, b, c);
		break;
	case 9:											/* ? */
		_saveLoadFlag = getVarOrDirectByte(0x80);
		_saveLoadSlot = getVarOrDirectByte(0x40);
		_saveLoadSlot = 99;					/* use this slot */
		_saveLoadCompatible = true;
		break;
	case 10:											/* ? */
		a = getVarOrDirectWord(0x80);
		if (a) {
			_switchRoomEffect = (byte)(a&0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;
	case 11:											/* ? */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		e = getVarOrDirectByte(0x40);
		darkenPalette(a, b, c, d, e);
		break;
	case 12:											/* ? */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		e = getVarOrDirectByte(0x40);
		setupShadowPalette(a, b, c, d, e);
		break;

	case 13:{										/* save-string */
			SaveFile *file;
			char filename[256], *s;

			a = getVarOrDirectByte(0x80);

			s = filename;
			while ((*s++ = fetchScriptByte()));

			SaveFileManager *mgr = _system->get_savefile_manager();
			file = mgr->open_savefile(filename, getSavePath(), true);
			if (file != NULL) {
				byte *ptr;
				ptr = getResourceAddress(rtString, a);
				file->write(ptr, resStrLen(ptr) + 1);
				delete file;
			}
			delete mgr;
			break;
		}
	case 14:{										/* load-string */
			SaveFile *file;
			char filename[256], *s;

			a = getVarOrDirectByte(0x80);

			s = filename;
			while ((*s++ = fetchScriptByte()));

			SaveFileManager *mgr = _system->get_savefile_manager();
			file = mgr->open_savefile(filename, getSavePath(), false);
			if (file != NULL) {
				byte *ptr;
				int len = 256, cnt = 0;
				ptr = (byte *)malloc(len);
				while (ptr) {
				  int r = file->read(ptr+cnt, len-cnt);
				  if ((cnt += r) < len) break;
				  ptr = (byte *)realloc(ptr, len<<=1);
				}
				ptr[cnt] = '\0';
				loadPtrToResource(rtString, a, ptr);
				free(ptr);
			}
			break;
		}
	case 15:											/* palmanip */
		a = getVarOrDirectByte(0x80);
		_opcode = fetchScriptByte();
		b = getVarOrDirectByte(0x80);
		c = getVarOrDirectByte(0x40);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		palManipulateInit(b, c, a, d);
		break;

	case 16:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		if (a < 1)
			a = 1;										/* FIXME: ZAK256 */
		checkRange(16, 1, a, "o5_roomOps: 16: color cycle out of range (%d)");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;
	default:
		error("o5_roomOps: unknown subopcode %d", _opcode & 0x1F);
	}
}

void Scumm_v5::o5_saveRestoreVerbs() {
	int a, b, c, slot, slot2;

	_opcode = fetchScriptByte();

	a = getVarOrDirectByte(0x80);
	b = getVarOrDirectByte(0x40);
	c = getVarOrDirectByte(0x20);

	switch (_opcode) {
	case 1:											/* hide verbs */
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
	case 2:											/* show verbs */
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
	case 3:											/* kill verbs */
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

void Scumm_v5::o5_setCameraAt() {
	setCameraAtEx(getVarOrDirectWord(0x80));
}

void Scumm_v5::o5_setObjectName() {
	int obj = getVarOrDirectWord(0x80);
	int size;
	int a;
	int i = 0;
	byte *name = NULL;
	unsigned char work[256];
	
	// Read in new name
	while ((a = fetchScriptByte()) != 0) {
		work[i++] = a;
		if (a == 0xFF) {
			work[i++] = fetchScriptByte();
			work[i++] = fetchScriptByte();
			work[i++] = fetchScriptByte();
		}
	}
	work[i++] = 0;

	if (obj < _numActors)
		error("Can't set actor %d name with new-name-of", obj);

	// TODO: Would be nice if we used rtObjectName resource for pre-V6
	// games, too. The only problem with that which I can see is that this
	// would break savegames. I.e. it would require yet another change to
	// the save/load system.

	byte *objptr;
	objptr = getOBCDFromObject(obj);
	if (objptr == NULL) {
		// WORKAROUND bug #587553: This is an odd one and looks more like
		// an actual bug in the original script. Usually we would error
		warning("Can't find OBCD to rename object %d to %s", obj, work);
		return;
	}

	if (_features & GF_SMALL_HEADER) {
		byte offset = 0;

		if (_features & GF_OLD_BUNDLE)
			offset = *(objptr + 16);
		else
			offset = *(objptr + 18);

		size = READ_LE_UINT16(objptr) - offset;
		name = objptr + offset;
	} else {
		name = 0;
#if 0
		name = findResourceData(MKID('OBNA'), objptr);
#else
		// FIXME: we can't use findResourceData anymore, because it returns const
		// data, while this function *must* return a non-const pointer. That is so
		// because in o2_setObjectName / o5_setObjectName we directly modify this
		// data. Now, we could add a non-const version of findResourceData, too
		// (C++ makes that easy); but this here is really the *only* place in all
		// of ScummVM where it wold be needed! That seems kind of a waste...
		//
		// So for now, I duplicate some code from findResourceData / findResource
		// here. However, a much nicer solution might be (with stress on "might")
		// to use the same technique as in V6 games: that is, use a seperate
		// resource for changed names. That would be the cleanest solution, but
		// might proof to be infeasible, as it might lead to unforseen regressions.
		
		uint32 tag = MKID('OBNA');
		byte *searchin = objptr;
		uint32 curpos, totalsize;
	
		assert(searchin);
	
		searchin += 4;
		totalsize = READ_BE_UINT32(searchin);
		curpos = 8;
		searchin += 4;
	
		while (curpos < totalsize) {
			if (READ_UINT32(searchin) == tag) {
				name = searchin + _resourceHeaderSize;
				break;
			}
	
			size = READ_BE_UINT32(searchin + 4);
			if ((int32)size <= 0) {
				error("(%c%c%c%c) Not found in %d... illegal block len %d",
							tag & 0xFF, (tag >> 8) & 0xFF, (tag >> 16) & 0xFF, (tag >> 24) & 0xFF, 0, size);
			}
	
			curpos += size;
			searchin += size;
		}
#endif
		size = getResourceDataSize(name);
	}
	
	if (name == 0)
		return;		// Silently bail out
	

	if (i > size) {
		warning("New name of object %d too long: old 's' (%d), new '%s' (%d))",
				obj, name, i, work, size);
		i = size;
	}

	memcpy(name, work, i);
	runInventoryScript(0);
}

void Scumm_v5::o5_setOwnerOf() {
	int obj, owner;

	obj = getVarOrDirectWord(0x80);
	owner = getVarOrDirectByte(0x40);

	setOwnerOf(obj, owner);
}

void Scumm_v5::o5_setState() {
	int obj, state;
	obj = getVarOrDirectWord(0x80);
	state = getVarOrDirectByte(0x40);
	putState(obj, state);
	removeObjectFromRoom(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void Scumm_v5::o5_setVarRange() {
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
}

void Scumm_v5::o5_startMusic() {
	if (_features & GF_FMTOWNS) {
		// In FM Towns games this is some kind of Audio CD status query function.
		// See also bug #762589 (thanks to Hibernatus for providing the information).
		getResultPos();
		int b = getVarOrDirectByte(0x80);
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
			// Unknown, but apparently never used.
			break;
		default:
			// TODO: return track length in seconds. We'll have to extend Sound and OSystem for this.
			// To check scummvm returns the right track length you 
			// can look at the global script #9 (0x888A in 49.LFL). 
			break;
		}
		debug(4,"o5_startMusic(%d)", b);
		setResult(result);
	} else {
		_sound->addSoundToQueue(getVarOrDirectByte(0x80));
	}
}

void Scumm_v5::o5_startSound() {
	VAR(VAR_MUSIC_TIMER) = 0;
	_sound->addSoundToQueue(getVarOrDirectByte(0x80));
}

void Scumm_v5::o5_stopMusic() {
	_sound->stopAllSounds();
}

void Scumm_v5::o5_stopSound() {
	_sound->stopSound(getVarOrDirectByte(0x80));
}

void Scumm_v5::o5_isSoundRunning() {
	int snd;
	getResultPos();
	snd = getVarOrDirectByte(0x80);
	if (snd)
		snd = _sound->isSoundRunning(snd);
	setResult(snd);
}

void Scumm_v5::o5_soundKludge() {
	int items[16];
	int i;

	if (_features & GF_SMALL_HEADER) {	// Is WaitForSentence in SCUMM V3
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
		} else if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;

		_scriptPointer--;
		o5_breakHere();
		return;
	}

	for (i = 0; i < 16; i++)
		items[i] = 0;

	int num = getWordVararg(items);

	_sound->soundKludge(items, num);
}

void Scumm_v5::o5_startObject() {
	int obj, script;
	int data[16];

	obj = getVarOrDirectWord(0x80);
	script = getVarOrDirectByte(0x40);

	getWordVararg(data);
	runObjectScript(obj, script, 0, 0, data);
}

void Scumm_v5::o5_startScript() {
	int op, script;
	int data[16];

	op = _opcode;
	script = getVarOrDirectByte(0x80);

	getWordVararg(data);

	runScript(script, (op & 0x20) != 0, (op & 0x40) != 0, data);
}

void Scumm_v5::o5_stopObjectCode() {
	stopObjectCode();
}

void Scumm_v5::o5_stopObjectScript() {
	stopObjectScript(getVarOrDirectWord(0x80));
}

void Scumm_v5::o5_stopScript() {
	int script;

	script = getVarOrDirectByte(0x80);

	if ((_gameId == GID_ZAK) && (_roomResource == 7) && (vm.slot[_currentScript].number == 10001)) {
		// FIXME: Nasty hack for bug #771499
		// Don't let the exit script for room 7 stop the buy script (24),
		// switching to the number selection keypad (script 15)
		if ((script == 24) && isScriptRunning(15))
			return;
	}

	if (!script)
		stopObjectCode();
	else
		stopScript(script);
}

void Scumm_v5::o5_stringOps() {
	int a, b, c, i;
	byte *ptr;

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:											/* loadstring */
		loadPtrToResource(rtString, getVarOrDirectByte(0x80), NULL);
		break;
	case 2:											/* copystring */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		nukeResource(rtString, a);
		ptr = getResourceAddress(rtString, b);
		if (ptr)
			loadPtrToResource(rtString, a, ptr);
		break;
	case 3:											/* set string char */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		c = getVarOrDirectByte(0x20);
		ptr = getResourceAddress(rtString, a);
		if (_gameId != GID_LOOM256) {	/* FIXME - LOOM256 */
			if (ptr == NULL)
				error("String %d does not exist", a);
			ptr[b] = c;
		}

		break;

	case 4:											/* get string char */
		getResultPos();
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		ptr = getResourceAddress(rtString, a);
		if (ptr == NULL)
			error("String %d does not exist", a);
		setResult(ptr[b]);
		break;

	case 5:											/* create empty string */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		nukeResource(rtString, a);
		if (b) {
			ptr = createResource(rtString, a, b);
			if (ptr) {
				for (i = 0; i < b; i++)
					ptr[i] = 0;
			}
		}
		break;
	}
}

void Scumm_v5::o5_subtract() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) - a);
}

void Scumm_v5::o5_verbOps() {
	int verb, slot;
	VerbSlot *vs;
	int a, b;
	byte *ptr;

	verb = getVarOrDirectByte(0x80);

	slot = getVerbSlot(verb, 0);
	checkRange(_maxVerbs - 1, 0, slot, "Illegal new verb slot %d");

	vs = &_verbs[slot];
	vs->verbid = verb;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0x1F) {
		case 1:										/* load image */
			a = getVarOrDirectWord(0x80);
			if (slot) {
				setVerbObject(_roomResource, a, slot);
				vs->type = kImageVerbType;
			}
			break;
		case 2:										/* load from code */
			loadPtrToResource(rtVerb, slot, NULL);
			if (slot == 0)
				nukeResource(rtVerb, slot);
			vs->type = kTextVerbType;
			vs->imgindex = 0;
			break;
		case 3:										/* color */
			vs->color = getVarOrDirectByte(0x80);
			break;
		case 4:										/* set hi color */
			vs->hicolor = getVarOrDirectByte(0x80);
			break;
		case 5:										/* set xy */
			vs->x = getVarOrDirectWord(0x80);
			vs->y = getVarOrDirectWord(0x40);
			// FIXME: hack loom notes into right spot
			if (_gameId == GID_LOOM256) {
				if ((verb >= 90) && (verb <= 97)) {	// Notes
					switch (verb) {
					case 90:
					case 91:
						vs->y -= 7;
						break;
					case 92:
						vs->y -= 6;
						break;
					case 93:
						vs->y -= 4;
						break;
					case 94:
						vs->y -= 3;
						break;
					case 95:
						vs->y -= 1;
						break;
					case 97:
						vs->y -= 5;
					}
				}
			}
			break;
		case 6:										/* set on */
			vs->curmode = 1;
			break;
		case 7:										/* set off */
			vs->curmode = 0;
			break;
		case 8:										/* delete */
			killVerb(slot);
			break;
		case 9:										/* new */
			slot = getVerbSlot(verb, 0);
			if (slot == 0) {
				for (slot = 1; slot < _maxVerbs; slot++) {
					if (_verbs[slot].verbid == 0)
						break;
				}
				if (slot == _maxVerbs)
					error("Too many verbs");
			}
			vs = &_verbs[slot];
			vs->verbid = verb;
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

		case 16:										/* set dim color */
			vs->dimcolor = getVarOrDirectByte(0x80);
			break;
		case 17:										/* dim */
			vs->curmode = 2;
			break;
		case 18:										/* set key */
			vs->key = getVarOrDirectByte(0x80);
			break;
		case 19:										/* set center */
			vs->center = 1;
			break;
		case 20:										/* set to string */
			ptr = getResourceAddress(rtString, getVarOrDirectWord(0x80));
			if (!ptr)
				nukeResource(rtVerb, slot);
			else {
				loadPtrToResource(rtVerb, slot, ptr);
			}
			if (slot == 0)
				nukeResource(rtVerb, slot);
			vs->type = kTextVerbType;
			vs->imgindex = 0;
			break;
		case 22:										/* assign object */
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectByte(0x40);
			if (slot && vs->imgindex != a) {
				setVerbObject(b, a, slot);
				vs->type = kImageVerbType;
				vs->imgindex = a;
			}
			break;
		case 23:										/* set back color */
			vs->bkcolor = getVarOrDirectByte(0x80);
			break;
		default:
			error("o5_verbOps: unknown subopcode %d", _opcode & 0x1F);
		}
	}
	drawVerb(slot, 0);
	verbMouseOver(0);
}

void Scumm_v5::o5_wait() {
	const byte *oldaddr = _scriptPointer - 1;

	if ((_gameId == GID_INDY3) && !(_features & GF_MACINTOSH)) {
		_opcode = 2;
	} else
		_opcode = fetchScriptByte();

	switch (_opcode & 0x1F) {
	case 1:	{										/* wait for actor */
			Actor *a = derefActorSafe(getVarOrDirectByte(0x80), "o5_wait");
			if (a && a->isInCurrentRoom() && a->moving)
				break;
			return;
		}
	case 2:											/* wait for message */
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case 3:											/* wait for camera */
		if (camera._cur.x >> 3 != camera._dest.x >> 3)
			break;
		return;
	case 4:											/* wait for sentence */
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

void Scumm_v5::o5_walkActorTo() {
	int x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(0x80), "o5_walkActorTo");
	x = getVarOrDirectWord(0x40);
	y = getVarOrDirectWord(0x20);
	a->startWalkActor(x, y, -1);
}

void Scumm_v5::o5_walkActorToActor() {
	int x, y;
	Actor *a, *a2;
	int nr = getVarOrDirectByte(0x80);
	int nr2 = getVarOrDirectByte(0x40);
	int dist = fetchScriptByte();

	if (nr == 106 && _gameId == GID_INDY4) {
		warning("Bypassing Indy4 bug");
		return;
	}
	
	if (_gameId == GID_LOOM256 && nr == 1 && nr2 == 0 &&
		dist == 255 && vm.slot[_currentScript].number == 98) {
		// WORKAROUND bug #743615: LoomCD script 98 contains this:
		//   walkActorToActor(1,0,255)
		// Once again this is either a script bug, or there is some hidden
		// or unknown meaning to this odd walk request...
		return;
	}

	if (_gameId == GID_INDY4 && nr == 1 && nr2 == 106 &&
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

	if (_version <= 2)
		dist *= 8;
	else if (dist == 0xFF) {
		dist = a->scalex * a->width / 0xFF;
		dist += (a2->scalex * a2->width / 0xFF) / 2;
	}
	x = a2->x;
	y = a2->y;
	if (x < a->x)
		x += dist;
	else
		x -= dist;

	if (_version <= 3) {
		AdjustBoxResult abr = a->adjustXYToBeInBox(x, y);
		x = abr.x;
		y = abr.y;
	}
	a->startWalkActor(x, y, -1);
}

void Scumm_v5::o5_walkActorToObject() {
	int obj;
	Actor *a;

	a = derefActor(getVarOrDirectByte(0x80), "o5_walkActorToObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		int x, y, dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	}
}

int Scumm_v5::getWordVararg(int *ptr) {
	int i;

	for (i = 0; i < 16; i++)
		ptr[i] = 0;

	i = 0;
	while ((_opcode = fetchScriptByte()) != 0xFF) {
		ptr[i++] = getVarOrDirectWord(0x80);
	}
	return i;
}

void Scumm_v5::decodeParseString() {
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

	setStringVars(textSlot);

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0xF) {
		case 0:										/* set string xy */
			_string[textSlot].xpos = getVarOrDirectWord(0x80);
			_string[textSlot].ypos = getVarOrDirectWord(0x40);
			_string[textSlot].overhead = false;
			break;
		case 1:										/* color */
			_string[textSlot].color = getVarOrDirectByte(0x80);
			break;
		case 2:										/* clipping */
			_string[textSlot].right = getVarOrDirectWord(0x80);
			break;
		case 3:										/* erase */
			{
			int a = getVarOrDirectWord(0x80);
			int b = getVarOrDirectWord(0x40);
			warning("Scumm_v5::decodeParseString: Unhandled case 3: %d, %d", a, b);
			}
			break;
		case 4:										/* center */
			_string[textSlot].center = true;
			_string[textSlot].overhead = false;
			break;
		case 6:										/* left */
			// FIXME: not sure if GF_OLD_BUNDLE is the right thing to check...
			// but Loom needs this, for sure.
			if (_version == 3) {
				// FIXME: this value seems to be some kind of override
				// for text spacing?!?
				/* int a = */ getVarOrDirectWord(0x80);
				
			} else {
				_string[textSlot].center = false;
				_string[textSlot].overhead = false;
			}
			break;
		case 7:										/* overhead */
			_string[textSlot].overhead = true;
			break;
		case 8:{									/* play loom talkie sound - used in other games ? */
				int offset = (uint16)getVarOrDirectWord(0x80);
				int delay = (uint16)getVarOrDirectWord(0x40);

				if (_gameId == GID_LOOM256) {
					VAR(VAR_MUSIC_TIMER) = 0;
					if (offset == 0 && delay == 0) {
						_sound->stopCD();
					} else {
						// Loom specified the offset from the start of the CD;
						// thus we have to subtract the length of the first track
						// (22500 frames) plus the 2 second = 150 frame leadin.
						// I.e. in total 22650 frames.
						offset = (int)(offset * 7.5 - 22650);

						// Slightly increase the delay (5 frames = 1/25 of a second).
						// This noticably improves the experience in Loom CD.
						delay = (int)(delay * 7.5 + 5);
						
						_sound->playCDTrack(1, 0, offset, delay);
					}
				} else {
					warning("parseString: 8");
				}
			}
			break;
		case 15:
			_messagePtr = _scriptPointer;
			switch (textSlot) {
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

			// FIXME: Store positions, this is needed for Indy3 (Grail Diary)..
			// I don't believe this is the correct fix, may cause other problems
			// later in the game.
 			//
 			// It's also needed for Loom, or the lines Bobbin
 			// speaks during the intro are put at position 0,0.
 			// In addition, Loom needs to remember the text colour.
			if (_gameId == GID_LOOM || _gameId == GID_INDY3) {
				_string[textSlot].t_xpos = _string[textSlot].xpos;
				_string[textSlot].t_ypos = _string[textSlot].ypos;
 				_string[textSlot].t_color = _string[textSlot].color;
			}

			_scriptPointer = _messagePtr;
			return;
		default:
			warning("Scumm_v5::decodeParseString: Unhandled case %d", _opcode & 0xF);
			return;
		}
	}

	_string[textSlot].t_xpos = _string[textSlot].xpos;
	_string[textSlot].t_ypos = _string[textSlot].ypos;
	_string[textSlot].t_center = _string[textSlot].center;
	_string[textSlot].t_overhead = _string[textSlot].overhead;
	_string[textSlot].t_right = _string[textSlot].right;
	_string[textSlot].t_color = _string[textSlot].color;
	_string[textSlot].t_charset = _string[textSlot].charset;
}

void Scumm_v5::o5_oldRoomEffect() {
	int a;

	_opcode = fetchScriptByte();
	if ((_opcode & 0x1F) == 3) {
		a = getVarOrDirectWord(0x80);

#if 1
		if (_features & GF_FMTOWNS) {
			// FIXME / TODO: OK the first thing to note is: at least in Zak256,
			// maybe also in other games, this opcode does a bit more. I added
			// some stubs here, but somebody with a full IDA or more knowledge
			// about this will have to fill in the gaps. At least now we know
			// that something is missing here :-)
		
			if (a == 4) {
printf("o5_oldRoomEffect ODDBALL: _opcode = 0x%x, a = 0x%x\n", _opcode, a);
				// No idea what byte_2FCCF is, but it's a globale boolean flag.
				// I only add it here as a temporary hack to make the pseudo code compile.
				// Maybe it is just there as a reentry protection guard, given
				// how it is used? It might also correspond to _screenEffectFlag.
				int byte_2FCCF = 0;

				// For now, we force a redraw of the screen background. This 
				// Makes the Zak end credits work more or less correctly.
				VirtScreen *vs = &virtscr[0];
				restoreBG(ScummVM::Rect(0,vs->topline, vs->width, vs->topline + vs->height));
				virtscr[0].setDirtyRange(0, virtscr[0].height);
				updateDirtyScreen(0);
				
				if (byte_2FCCF) {
					// Here now "sub_1C44" is called, which sets byte_2FCCF to 0 then
					// calls yet another sub (which also reads byte_2FCCF):

					byte_2FCCF = 0;
					//call    sub_0BB3
					
					
					// Now sub_085C is called. This is quite simply: it sets 
					// 0xF000 bytes. starting at 0x40000 to 0. No idea what that
					// buffer is, maybe a screen buffer, though. Note that
					// 0xF000 = 320*192.
					// Maybe this is also the charset mask being cleaned?
					
					// call sub_085C

					
					// And then sub_1C54 is called, which is almost identical to
					// the above sub_1C44, only it sets byte_2FCCF to 1:
					
					byte_2FCCF = 1;
					// call    sub_0BB3

				} else {
					// Here only sub_085C is called (see comment above) 

					// call    sub_085C
				}
			return;
			}
#endif

		}
		if (a) {
			_switchRoomEffect = (byte)(a&0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
	}
}

void Scumm_v5::o5_pickupObjectOld() {
	int obj = getVarOrDirectWord(0x80);

	if (obj < 1) {
		error("pickupObjectOld received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	/* Don't take an */
		return;											/* object twice */

	// warning("adding %d from %d to inventoryOld", obj, _currentRoom);
	addObjectToInventory(obj, _roomResource);
	removeObjectFromRoom(obj);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	clearDrawObjectQueue();
	runInventoryScript(1);
}
