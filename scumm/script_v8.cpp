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
#include "intern.h"

/*
 * NO, we do NOT support CMI yet :-) This file is mostly a placeholder and a place
 * to grow real support in. For now, only a few opcodes are implemented, and they
 * might even be wrong... so don't hold your breath.
 */

#define OPCODE(x)	{ &Scumm_v8::x, #x }

void Scumm_v8::setupOpcodes()
{
	static const OpcodeEntryV8 opcodes[256] = {
		/* 00 */
		OPCODE(o8_invalid),
		OPCODE(o8_pushNumber),
		OPCODE(o8_pushVariable),
		OPCODE(o8_pushArrayValue),
		/* 04 */
		OPCODE(o8_pushArrayValueIndexed),
		OPCODE(o8_dup),
		OPCODE(o8_pop),
		OPCODE(o6_not),
		/* 08 */
		OPCODE(o8_eq),
		OPCODE(o8_neq),
		OPCODE(o8_gt),
		OPCODE(o8_lt),
		/* 0C */
		OPCODE(o8_leq),
		OPCODE(o8_geq),
		OPCODE(o8_add),
		OPCODE(o8_sub),
		/* 10 */
		OPCODE(o8_mul),
		OPCODE(o8_div),
		OPCODE(o8_land),
		OPCODE(o8_lor),
		/* 14 */
		OPCODE(o8_band),
		OPCODE(o8_bor),
		OPCODE(o8_mod),
		OPCODE(o8_invalid),
		/* 18 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 1C */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 20 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 24 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 28 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 2C */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 30 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 34 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 38 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 3C */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 40 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 44 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 48 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 4C */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 50 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 54 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 58 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 5C */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 60 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* 64 */
		OPCODE(o8_if),
		OPCODE(o8_ifNot),
		OPCODE(o8_jump),
		OPCODE(o8_breakHere),
		/* 68 */
		OPCODE(o8_breakHereVar),
		OPCODE(o8_waitForStuff),
		OPCODE(o8_sleepJiffies),
		OPCODE(o8_sleepSeconds),
		/* 6C */
		OPCODE(o8_sleepMinutes),
		OPCODE(o8_storeVariable),
		OPCODE(o8_incVariable),
		OPCODE(o8_decVariable),
		/* 70 */
		OPCODE(o8_arrayDim),
		OPCODE(o8_storeArray),
		OPCODE(o8_incArray),
		OPCODE(o8_decArray),
		/* 74 */
		OPCODE(o8_arrayDim2),
		OPCODE(o8_storeArrayIndexed),
		OPCODE(o8_assignArray),
		OPCODE(o8_arrayShuffle),
		/* 78 */
		OPCODE(o8_arrayLocalize),
		OPCODE(o8_startScript),
		OPCODE(o8_startScriptQuick),
		OPCODE(o8_endScript),
		/* 7C */
		OPCODE(o8_stopScript),
		OPCODE(o8_chainScript),
		OPCODE(o8_return),
		OPCODE(o8_startObject),
		/* 80 */
		OPCODE(o8_stopObject),
		OPCODE(o8_cutscene),
		OPCODE(o8_endCutscene),
		OPCODE(o8_freezeScripts),
		/* 84 */
		OPCODE(o8_beginOverride),
		OPCODE(o8_endOverride),
		OPCODE(o8_stopSentence),
		OPCODE(o8_debug),
		/* 88 */
		OPCODE(o8_debugWin),
		OPCODE(o8_classOf),
		OPCODE(o8_stateOf),
		OPCODE(o8_ownerOf),
		/* 8C */
		OPCODE(o8_cameraPanTo),
		OPCODE(o8_cameraFollow),
		OPCODE(o8_cameraAt),
		OPCODE(o8_sayLine),
		/* 90 */
		OPCODE(o8_sayLineDefault),
		OPCODE(o8_sayLineSimple),
		OPCODE(o8_sayLineSimpleDefault),
		OPCODE(o8_printLine),
		/* 94 */
		OPCODE(o8_printCursor),
		OPCODE(o8_printDebug),
		OPCODE(o8_printSystem),
		OPCODE(o8_blastText),
		/* 98 */
		OPCODE(o8_drawObject),
		OPCODE(o8_invalid),
		OPCODE(o8_blastObject),
		OPCODE(o8_invalid),
		/* 9C */
		OPCODE(o8_userFace),
		OPCODE(o8_currentRoom),
		OPCODE(o8_comeOutDoor),
		OPCODE(o8_walkActorToObject),
		/* A0 */
		OPCODE(o8_walkActorToXY),
		OPCODE(o8_putActorAtXY),
		OPCODE(o8_putActorAtObject),
		OPCODE(o8_faceTowards),
		/* A4 */
		OPCODE(o8_doAnimation),
		OPCODE(o8_doSentence),
		OPCODE(o8_pickUpObject),
		OPCODE(o8_setBox),
		/* A8 */
		OPCODE(o8_setBoxPath),
		OPCODE(o8_setBoxSet),
		OPCODE(o8_heapStuff),
		OPCODE(o8_roomStuff),
		/* AC */
		OPCODE(o8_actorStuff),
		OPCODE(o8_cameraStuff),
		OPCODE(o8_verbStuff),
		OPCODE(o8_startSfx),
		/* B0 */
		OPCODE(o8_startMusic),
		OPCODE(o8_stopSound),
		OPCODE(o8_soundKludge),
		OPCODE(o8_system),
		/* B4 */
		OPCODE(o8_verbSets),
		OPCODE(o8_newNameOf),
		OPCODE(o8_getTimeDate),
		OPCODE(o8_drawBox),
		/* B8 */
		OPCODE(o8_actObjStamp),
		OPCODE(o8_startVideo),
		OPCODE(o8_kludge),
		OPCODE(o8_invalid),
		/* BC */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* C0 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* C4 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* C8 */
		OPCODE(o6_startScript),
		OPCODE(o6_startObject),
		OPCODE(o8_pick),
		OPCODE(o8_pickDefault),
		/* CC */
		OPCODE(o8_pickRandom),
		OPCODE(o8_inSet),
		OPCODE(o8_getRandomNumber),
		OPCODE(o8_getRandomNumberRange),
		/* D0 */
		OPCODE(o8_classOf),
		OPCODE(o8_stateOf),
		OPCODE(o8_ownerOf),
		OPCODE(o8_scriptRunning),
		/* D4 */
		OPCODE(o8_objectRunning),
		OPCODE(o8_soundRunning),
		OPCODE(o8_abs),
		OPCODE(o8_pixel),
		/* D8 */
		OPCODE(o8_kludge),
		OPCODE(o8_inBox),
		OPCODE(o8_validVerb),
		OPCODE(o8_findActor),
		/* DC */
		OPCODE(o8_findObject),
		OPCODE(o8_findVerb),
		OPCODE(o8_findAllObjects),
		OPCODE(o8_actorInvertory),
		/* E0 */
		OPCODE(o8_actorInvertoryCount),
		OPCODE(o8_actorVariable),
		OPCODE(o8_actorRoom),
		OPCODE(o8_actorBox),
		/* E4 */
		OPCODE(o8_actorMoving),
		OPCODE(o8_actorCostume),
		OPCODE(o8_actorScale),
		OPCODE(o8_actorDepth),
		/* E8 */
		OPCODE(o8_actorElevation),
		OPCODE(o8_actorWidth),
		OPCODE(o8_actObjFacing),
		OPCODE(o8_actObjX),
		/* EC */
		OPCODE(o8_actObjY),
		OPCODE(o8_actorChore),
		OPCODE(o8_proximity2ActObjs),
		OPCODE(o8_proximity2Points),
		/* F0 */
		OPCODE(o8_objectImageX),
		OPCODE(o8_objectImageY),
		OPCODE(o8_objectImageWidth),
		OPCODE(o8_objectImageHeight),
		/* F4 */
		OPCODE(o8_verbX),
		OPCODE(o8_verbY),
		OPCODE(o8_stringWidth),
		OPCODE(o8_actorZPlane),
		/* F8 */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		/* FC */
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
		OPCODE(o8_invalid),
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

uint32 Scumm_v8::fetchScriptUint32()
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

int Scumm_v8::fetchScriptInt32()
{
	return (int32)fetchScriptUint32();
}

void Scumm_v8::o8_invalid()
{
	error("Invalid opcode '%x' at %x", _opcode, _scriptPointer - _scriptOrgPointer);
}

void Scumm_v8::o8_pushNumber()
{
}

void Scumm_v8::o8_pushVariable()
{
}

void Scumm_v8::o8_pushArrayValue()
{
}

void Scumm_v8::o8_pushArrayValueIndexed()
{
}

void Scumm_v8::o8_dup()
{
}

void Scumm_v8::o8_pop()
{
}

void Scumm_v8::o8_not()
{
}

void Scumm_v8::o8_eq()
{
}

void Scumm_v8::o8_neq()
{
}

void Scumm_v8::o8_gt()
{
}

void Scumm_v8::o8_lt()
{
}

void Scumm_v8::o8_leq()
{
}

void Scumm_v8::o8_geq()
{
}

void Scumm_v8::o8_add()
{
}

void Scumm_v8::o8_sub()
{
}

void Scumm_v8::o8_mul()
{
}

void Scumm_v8::o8_div()
{
}

void Scumm_v8::o8_land()
{
}

void Scumm_v8::o8_lor()
{
}

void Scumm_v8::o8_band()
{
}

void Scumm_v8::o8_mod()
{
}

void Scumm_v8::o8_bor()
{
}

void Scumm_v8::o8_if()
{
}

void Scumm_v8::o8_ifNot()
{
}

void Scumm_v8::o8_jump()
{
}

void Scumm_v8::o8_breakHere()
{
}

void Scumm_v8::o8_breakHereVar()
{
}

void Scumm_v8::o8_waitForStuff()
{
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x1E:		// SO_WAIT_FOR_ACTOR Wait for actor (to finish current action?)
	case 0x1F:		// SO_WAIT_FOR_MESSAGE Wait for message
	case 0x20:		// SO_WAIT_FOR_CAMERA Wait for camera (to finish current action?)
	case 0x21:		// SO_WAIT_FOR_SENTENCE
	case 0x22:		// SO_WAIT_FOR_ANIMATION
	case 0x23:		// SO_WAIT_FOR_TURN
	default:
		error("o8_wait: default case %d", subOp);
	}
}

void Scumm_v8::o8_sleepJiffies()
{
}

void Scumm_v8::o8_sleepSeconds()
{
}

void Scumm_v8::o8_sleepMinutes()
{
}

void Scumm_v8::o8_storeVariable()
{
}

void Scumm_v8::o8_incVariable()
{
}

void Scumm_v8::o8_decVariable()
{
}

void Scumm_v8::o8_arrayDim()
{
}

void Scumm_v8::o8_storeArray()
{
}

void Scumm_v8::o8_incArray()
{
}

void Scumm_v8::o8_decArray()
{
}

void Scumm_v8::o8_arrayDim2()
{
}

void Scumm_v8::o8_storeArrayIndexed()
{
}

void Scumm_v8::o8_assignArray()
{
}

void Scumm_v8::o8_arrayShuffle()
{
}

void Scumm_v8::o8_arrayLocalize()
{
}

void Scumm_v8::o8_startScript()
{
}

void Scumm_v8::o8_startScriptQuick()
{
}

void Scumm_v8::o8_endScript()
{
}

void Scumm_v8::o8_stopScript()
{
}

void Scumm_v8::o8_chainScript()
{
}

void Scumm_v8::o8_return()
{
}

void Scumm_v8::o8_startObject()
{
}

void Scumm_v8::o8_stopObject()
{
}

void Scumm_v8::o8_cutscene()
{
}

void Scumm_v8::o8_endCutscene()
{
}

void Scumm_v8::o8_freezeScripts()
{
}

void Scumm_v8::o8_beginOverride()
{
}

void Scumm_v8::o8_endOverride()
{
}

void Scumm_v8::o8_stopSentence()
{
}

void Scumm_v8::o8_debug()
{
}

void Scumm_v8::o8_debugWin()
{
}

void Scumm_v8::o8_cameraPanTo()
{
}

void Scumm_v8::o8_cameraFollow()
{
}

void Scumm_v8::o8_cameraAt()
{
}

void Scumm_v8::o8_sayLine()
{
}

void Scumm_v8::o8_sayLineDefault()
{
}

void Scumm_v8::o8_sayLineSimple()
{
}

void Scumm_v8::o8_sayLineSimpleDefault()
{
}

void Scumm_v8::o8_printLine()
{
}

void Scumm_v8::o8_printCursor()
{
}

void Scumm_v8::o8_printDebug()
{
}

void Scumm_v8::o8_printSystem()
{
}

void Scumm_v8::o8_blastText()
{
}

void Scumm_v8::o8_drawObject()
{
}

void Scumm_v8::o8_blastObject()
{
}

void Scumm_v8::o8_userFace()
{
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0xDC:		// SO_CURSOR_ON Turn cursor on
	case 0xDD:		// SO_CURSOR_OFF Turn cursor off
	case 0xDE:		// SO_CURSOR_SOFT_ON Turn soft cursor on
	case 0xDF:		// SO_CURSOR_SOFT_OFF Turn soft cursor off
	case 0xE0:		// SO_USERPUT_ON
	case 0xE1:		// SO_USERPUT_OFF
	case 0xE2:		// SO_USERPUT_SOFT_ON
	case 0xE3:		// SO_USERPUT_SOFT_OFF
	case 0xE4:		// SO_CURSOR_IMAGE Set cursor image
	case 0xE5:		// SO_CURSOR_HOTSPOT Set cursor hotspot
	case 0xE6:		// SO_CURSOR_TRANSPARENT Set cursor transparent color
	case 0xE7:		// SO_CHARSET_SET
	case 0xE8:		// SO_CHARSET_COLOR
	case 0xE9:		// SO_CURSOR_PUT
	default:
		error("o8_cursorCommand: default case %d", subOp);
	}
}

void Scumm_v8::o8_currentRoom()
{
}

void Scumm_v8::o8_comeOutDoor()
{
}

void Scumm_v8::o8_walkActorToObject()
{
}

void Scumm_v8::o8_walkActorToXY()
{
}

void Scumm_v8::o8_putActorAtXY()
{
}

void Scumm_v8::o8_putActorAtObject()
{
}

void Scumm_v8::o8_faceTowards()
{
}

void Scumm_v8::o8_doAnimation()
{
}

void Scumm_v8::o8_doSentence()
{
}

void Scumm_v8::o8_pickUpObject()
{
}

void Scumm_v8::o8_setBox()
{
}

void Scumm_v8::o8_setBoxPath()
{
}

void Scumm_v8::o8_setBoxSet()
{
}

void Scumm_v8::o8_heapStuff()
{
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x3C:		// SO_HEAP_LOAD_CHARSET Load character set to heap
	case 0x3D:		// SO_HEAP_LOAD_COSTUME Load costume to heap
	case 0x3E:		// SO_HEAP_LOAD_OBJECT Load object to heap
	case 0x3F:		// SO_HEAP_LOAD_ROOM Load room to heap
	case 0x40:		// SO_HEAP_LOAD_SCRIPT Load script to heap
	case 0x41:		// SO_HEAP_LOAD_SOUND Load sound to heap
	case 0x42:		// SO_HEAP_LOCK_COSTUME Lock costume in heap
	case 0x43:		// SO_HEAP_LOCK_ROOM Lock room in heap
	case 0x44:		// SO_HEAP_LOCK_SCRIPT Lock script in heap
	case 0x45:		// SO_HEAP_LOCK_SOUND Lock sound in heap
	case 0x46:		// SO_HEAP_UNLOCK_COSTUME Unlock costume
	case 0x47:		// SO_HEAP_UNLOCK_ROOM Unlock room
	case 0x48:		// SO_HEAP_UNLOCK_SCRIPT Unlock script
	case 0x49:		// SO_HEAP_UNLOCK_SOUND Unlock sound
	case 0x4A:		// SO_HEAP_NUKE_COSTUME Remove costume from heap
	case 0x4B:		// SO_HEAP_NUKE_ROOM Remove room from heap
	case 0x4C:		// SO_HEAP_NUKE_SCRIPT Remove script from heap
	case 0x4D:		// SO_HEAP_NUKE_SOUND Remove sound from heap
	default:
		error("o8_heapStuff: default case %d", subOp);
	}
}

void Scumm_v8::o8_roomStuff()
{
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x52:		// SO_ROOM_PALETTE Set room palette
//	case 0x53:		// SO_339
//	case 0x54:		// SO_340
	case 0x55:		// SO_ROOM_INTENSITY Set room intensity
//	case 0x56:		// SO_342
	case 0x57:		// SO_ROOM_FADE Fade room
	case 0x58:		// SO_ROOM_RGB_INTENSITY Set room color intensity
	case 0x59:		// SO_ROOM_TRANSFORM Transform room
	case 0x5A:		// SO_ROOM_CYCLE_SPEED Set palette cycling speed
	case 0x5B:		// SO_ROOM_COPY_PALETTE Copy palette
	case 0x5C:		// SO_ROOM_NEW_PALETTE Create new palette
	case 0x5D:		// SO_ROOM_SAVE_GAME Save game
	case 0x5E:		// SO_ROOM_LOAD_GAME Load game
	case 0x5F:		// SO_ROOM_SATURATION Set saturation of room colors
	default:
		error("o8_roomStuff: default case %d", subOp);
	}
}

void Scumm_v8::o8_actorStuff()
{
	// TODO
	byte subOp = fetchScriptByte();
	Actor *a;

	a = derefActorSafe(_curActor, "o8_actorStuff");
	if (!a)
		return;

	switch (subOp) {
	case 0x64:		// SO_ACTOR_COSTUME Set actor costume
	case 0x65:		// SO_ACTOR_STEP_DIST Set actor width of steps
//	case 0x66:		// SO_358
	case 0x67:		// SO_ACTOR_ANIMATION_DEFAULT Set actor animation to default
	case 0x68:		// SO_ACTOR_ANIMATION_INIT Initialize animation
	case 0x69:		// SO_ACTOR_ANIMATION_TALK Set actor animation to talk animation
	case 0x6A:		// SO_ACTOR_ANIMATION_WALK Set actor animation to walk animation
	case 0x6B:		// SO_ACTOR_ANIMATION_STAND Set actor animation to standing animation
	case 0x6C:		// SO_ACTOR_ANIMATION_SPEED Set speed of animation
	case 0x6D:		// SO_ACTOR_DEFAULT
	case 0x6E:		// SO_ACTOR_ELEVATION
	case 0x6F:		// SO_ACTOR_PALETTE Set actor palette
	case 0x70:		// SO_ACTOR_TALK_COLOR Set actor talk color
	case 0x71:		// SO_ACTOR_NAME Set name of actor
	case 0x72:		// SO_ACTOR_WIDTH Set width of actor
	case 0x73:		// SO_ACTOR_SCALE Set scaling of actor
	case 0x74:		// SO_ACTOR_NEVER_ZCLIP ?
	case 0x75:		// SO_ACTOR_ALWAYS_ZCLIP ?
	case 0x76:		// SO_ACTOR_IGNORE_BOXES Make actor ignore boxes
	case 0x77:		// SO_ACTOR_FOLLOW_BOXES Make actor follow boxes
	case 0x78:		// SO_ACTOR_SPECIAL_DRAW
	case 0x79:		// SO_ACTOR_TEXT_OFFSET Set text offset relative to actor
	case 0x7A:		// SO_ACTOR_INIT Initialize actor
	case 0x7B:		// SO_ACTOR_VARIABLE Set actor variable
	case 0x7C:		// SO_ACTOR_IGNORE_TURNS_ON Make actor ignore turns
	case 0x7D:		// SO_ACTOR_IGNORE_TURNS_OFF Make actor follow turns
	case 0x7E:		// SO_ACTOR_NEW New actor
	case 0x7F:		// SO_ACTOR_DEPTH Set actor Z position
	case 0x80:		// SO_ACTOR_STOP
	case 0x81:		// SO_ACTOR_FACE Make actor face angle
	case 0x82:		// SO_ACTOR_TURN Turn actor
	case 0x83:		// SO_ACTOR_WALK_SCRIPT Set walk script for actor?
	case 0x84:		// SO_ACTOR_TALK_SCRIPT Set talk script for actor?
	case 0x85:		// SO_ACTOR_WALK_PAUSE
	case 0x86:		// SO_ACTOR_WALK_RESUME
	case 0x87:		// SO_ACTOR_VOLUME Set volume of actor speech
	case 0x88:		// SO_ACTOR_FREQUENCY Set frequency of actor speech
	case 0x89:		// SO_ACTOR_PAN
	default:
		error("o8_actorStuff: default case %d", subOp);
	}
}

void Scumm_v8::o8_cameraStuff()
{
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x32:		// SO_CAMERA_PAUSE
	case 0x33:		// SO_CAMERA_RESUME
	default:
		error("o8_cameraStuff: default case %d", subOp);
	}
}

void Scumm_v8::o8_verbStuff()
{
	// TODO
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 0x96:		// SO_VERB_INIT Choose verb number for editing
	case 0x97:		// SO_VERB_NEW New verb
	case 0x98:		// SO_VERB_DELETE Delete verb
	case 0x99:		// SO_VERB_NAME Set verb name
	case 0x9A:		// SO_VERB_AT Set verb (X,Y) placement
	case 0x9B:		// SO_VERB_ON Turn verb on
	case 0x9C:		// SO_VERB_OFF Turn verb off
	case 0x9D:		// SO_VERB_COLOR Set verb color
	case 0x9E:		// SO_VERB_HICOLOR Set verb highlighted color
//	case 0x9F:		// SO_415
	case 0xA0:		// SO_VERB_DIMCOLOR Set verb dimmed (disabled) color
	case 0xA1:		// SO_VERB_DIM
	case 0xA2:		// SO_VERB_KEY Set keypress to associate with verb
	case 0xA3:		// SO_VERB_IMAGE Set verb image
	case 0xA4:		// SO_VERB_NAME_STR Set verb name
	case 0xA5:		// SO_VERB_CENTER Center verb
	case 0xA6:		// SO_VERB_CHARSET Choose charset for verb
	case 0xA7:		// SO_VERB_LINE_SPACING Choose linespacing for verb
	default:
		error("o8_verbStuff: default case %d", subOp);
	}
}

void Scumm_v8::o8_startSfx()
{
}

void Scumm_v8::o8_startMusic()
{
}

void Scumm_v8::o8_stopSound()
{
}

void Scumm_v8::o8_soundKludge()
{
}

void Scumm_v8::o8_system()
{
}

void Scumm_v8::o8_verbSets()
{
}

void Scumm_v8::o8_newNameOf()
{
}

void Scumm_v8::o8_getTimeDate()
{
}

void Scumm_v8::o8_drawBox()
{
}

void Scumm_v8::o8_actObjStamp()
{
}

void Scumm_v8::o8_startVideo()
{
}

void Scumm_v8::o8_pick()
{
}

void Scumm_v8::o8_pickDefault()
{
}

void Scumm_v8::o8_pickRandom()
{
}

void Scumm_v8::o8_inSet()
{
}

void Scumm_v8::o8_getRandomNumber()
{
}

void Scumm_v8::o8_getRandomNumberRange()
{
}

void Scumm_v8::o8_classOf()
{
}

void Scumm_v8::o8_stateOf()
{
}

void Scumm_v8::o8_ownerOf()
{
}

void Scumm_v8::o8_scriptRunning()
{
}

void Scumm_v8::o8_objectRunning()
{
}

void Scumm_v8::o8_soundRunning()
{
}

void Scumm_v8::o8_abs()
{
}

void Scumm_v8::o8_pixel()
{
}

void Scumm_v8::o8_kludge()
{
}

void Scumm_v8::o8_inBox()
{
}

void Scumm_v8::o8_validVerb()
{
}

void Scumm_v8::o8_findActor()
{
}

void Scumm_v8::o8_findObject()
{
}

void Scumm_v8::o8_findVerb()
{
}

void Scumm_v8::o8_findAllObjects()
{
}

void Scumm_v8::o8_actorInvertory()
{
}

void Scumm_v8::o8_actorInvertoryCount()
{
}

void Scumm_v8::o8_actorVariable()
{
}

void Scumm_v8::o8_actorRoom()
{
}

void Scumm_v8::o8_actorBox()
{
}

void Scumm_v8::o8_actorMoving()
{
}

void Scumm_v8::o8_actorCostume()
{
}

void Scumm_v8::o8_actorScale()
{
}

void Scumm_v8::o8_actorDepth()
{
}

void Scumm_v8::o8_actorElevation()
{
}

void Scumm_v8::o8_actorWidth()
{
}

void Scumm_v8::o8_actObjFacing()
{
}

void Scumm_v8::o8_actObjX()
{
}

void Scumm_v8::o8_actObjY()
{
}

void Scumm_v8::o8_actorChore()
{
}

void Scumm_v8::o8_proximity2ActObjs()
{
}

void Scumm_v8::o8_proximity2Points()
{
}

void Scumm_v8::o8_objectImageX()
{
}

void Scumm_v8::o8_objectImageY()
{
}

void Scumm_v8::o8_objectImageWidth()
{
}

void Scumm_v8::o8_objectImageHeight()
{
}

void Scumm_v8::o8_verbX()
{
}

void Scumm_v8::o8_verbY()
{
}

void Scumm_v8::o8_stringWidth()
{
}

void Scumm_v8::o8_actorZPlane()
{
}

/*

From http://scummrev.mixnmojo.com/specs/CMIOpcodes.shtml

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