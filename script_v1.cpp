/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

void Scumm::setupOpcodes()
{
	static const OpcodeProc opcode_list[] = {
		/* 00 */
		&Scumm::o5_stopObjectCode,
		&Scumm::o5_putActor,
		&Scumm::o5_startMusic,
		&Scumm::o5_getActorRoom,
		/* 04 */
		&Scumm::o5_isGreaterEqual,
		&Scumm::o5_drawObject,
		&Scumm::o5_getActorElevation,
		&Scumm::o5_setState,
		/* 08 */
		&Scumm::o5_isNotEqual,
		&Scumm::o5_faceActor,
		&Scumm::o5_startScript,
		&Scumm::o5_getVerbEntrypoint,
		/* 0C */
		&Scumm::o5_resourceRoutines,
		&Scumm::o5_walkActorToActor,
		&Scumm::o5_putActorAtObject,
		&Scumm::o5_getObjectState,
		/* 10 */
		&Scumm::o5_getObjectOwner,
		&Scumm::o5_animateActor,
		&Scumm::o5_panCameraTo,
		&Scumm::o5_actorSet,
		/* 14 */
		&Scumm::o5_print,
		&Scumm::o5_actorFromPos,
		&Scumm::o5_getRandomNr,
		&Scumm::o5_and,
		/* 18 */
		&Scumm::o5_jumpRelative,
		&Scumm::o5_doSentence,
		&Scumm::o5_move,
		&Scumm::o5_multiply,
		/* 1C */
		&Scumm::o5_startSound,
		&Scumm::o5_ifClassOfIs,
		&Scumm::o5_walkActorTo,
		&Scumm::o5_isActorInBox,
		/* 20 */
		&Scumm::o5_stopMusic,
		&Scumm::o5_putActor,
		&Scumm::o5_getAnimCounter,
		&Scumm::o5_getActorY,
		/* 24 */
		&Scumm::o5_loadRoomWithEgo,
		&Scumm::o5_pickupObject,
		&Scumm::o5_setVarRange,
		&Scumm::o5_stringOps,
		/* 28 */
		&Scumm::o5_equalZero,
		&Scumm::o5_setOwnerOf,
		&Scumm::o5_startScript,
		&Scumm::o5_delayVariable,
		/* 2C */
		&Scumm::o5_cursorCommand,
		&Scumm::o5_putActorInRoom,
		&Scumm::o5_delay,
		&Scumm::o5_getObjectState,
		/* 30 */
		&Scumm::o5_matrixOps,
		&Scumm::o5_getInventoryCount,
		&Scumm::o5_setCameraAt,
		&Scumm::o5_roomOps,
		/* 34 */
		&Scumm::o5_getDist,
		&Scumm::o5_findObject,
		&Scumm::o5_walkActorToObject,
		&Scumm::o5_startObject,
		/* 38 */
		&Scumm::o5_lessOrEqual,
		&Scumm::o5_doSentence,
		&Scumm::o5_subtract,
		&Scumm::o5_getActorScale,
		/* 3C */
		&Scumm::o5_stopSound,
		&Scumm::o5_findInventory,
		&Scumm::o5_walkActorTo,
		&Scumm::o5_drawBox,
		/* 40 */
		&Scumm::o5_cutscene,
		&Scumm::o5_putActor,
		&Scumm::o5_chainScript,
		&Scumm::o5_getActorX,
		/* 44 */
		&Scumm::o5_isLess,
		&Scumm::o5_drawObject,
		&Scumm::o5_increment,
		&Scumm::o5_setState,
		/* 48 */
		&Scumm::o5_isEqual,
		&Scumm::o5_faceActor,
		&Scumm::o5_startScript,
		&Scumm::o5_getVerbEntrypoint,
		/* 4C */
		&Scumm::o5_soundKludge,
		&Scumm::o5_walkActorToActor,
		&Scumm::o5_putActorAtObject,
		&Scumm::o5_badOpcode,
		/* 50 */
		&Scumm::o5_pickupObjectOld,
		&Scumm::o5_animateActor,
		&Scumm::o5_actorFollowCamera,
		&Scumm::o5_actorSet,
		/* 54 */
		&Scumm::o5_setObjectName,
		&Scumm::o5_actorFromPos,
		&Scumm::o5_getActorMoving,
		&Scumm::o5_or,
		/* 58 */
		&Scumm::o5_overRide,
		&Scumm::o5_doSentence,
		&Scumm::o5_add,
		&Scumm::o5_divide,
		/* 5C */
		&Scumm::o5_oldRoomEffect,
		&Scumm::o5_actorSetClass,
		&Scumm::o5_walkActorTo,
		&Scumm::o5_isActorInBox,
		/* 60 */
		&Scumm::o5_freezeScripts,
		&Scumm::o5_putActor,
		&Scumm::o5_stopScript,
		&Scumm::o5_getActorFacing,
		/* 64 */
		&Scumm::o5_loadRoomWithEgo,
		&Scumm::o5_pickupObject,
		&Scumm::o5_getClosestObjActor,
		&Scumm::o5_dummy,
		/* 68 */
		&Scumm::o5_getScriptRunning,
		&Scumm::o5_setOwnerOf,
		&Scumm::o5_startScript,
		&Scumm::o5_debug,
		/* 6C */
		&Scumm::o5_getActorWidth,
		&Scumm::o5_putActorInRoom,
		&Scumm::o5_stopObjectScript,
		&Scumm::o5_badOpcode,
		/* 70 */
		&Scumm::o5_lights,
		&Scumm::o5_getActorCostume,
		&Scumm::o5_loadRoom,
		&Scumm::o5_roomOps,
		/* 74 */
		&Scumm::o5_getDist,
		&Scumm::o5_findObject,
		&Scumm::o5_walkActorToObject,
		&Scumm::o5_startObject,
		/* 78 */
		&Scumm::o5_isGreater,				/* less? */
		&Scumm::o5_doSentence,
		&Scumm::o5_verbOps,
		&Scumm::o5_getActorWalkBox,
		/* 7C */
		&Scumm::o5_isSoundRunning,
		&Scumm::o5_findInventory,
		&Scumm::o5_walkActorTo,
		&Scumm::o5_drawBox,
		/* 80 */
		&Scumm::o5_breakHere,
		&Scumm::o5_putActor,
		&Scumm::o5_startMusic,
		&Scumm::o5_getActorRoom,
		/* 84 */
		&Scumm::o5_isGreaterEqual,	/* less equal? */
		&Scumm::o5_drawObject,
		&Scumm::o5_getActorElevation,
		&Scumm::o5_setState,
		/* 88 */
		&Scumm::o5_isNotEqual,
		&Scumm::o5_faceActor,
		&Scumm::o5_startScript,
		&Scumm::o5_getVerbEntrypoint,
		/* 8C */
		&Scumm::o5_resourceRoutines,
		&Scumm::o5_walkActorToActor,
		&Scumm::o5_putActorAtObject,
		&Scumm::o5_getObjectState,
		/* 90 */
		&Scumm::o5_getObjectOwner,
		&Scumm::o5_animateActor,
		&Scumm::o5_panCameraTo,
		&Scumm::o5_actorSet,
		/* 94 */
		&Scumm::o5_print,
		&Scumm::o5_actorFromPos,
		&Scumm::o5_getRandomNr,
		&Scumm::o5_and,
		/* 98 */
		&Scumm::o5_quitPauseRestart,
		&Scumm::o5_doSentence,
		&Scumm::o5_move,
		&Scumm::o5_multiply,
		/* 9C */
		&Scumm::o5_startSound,
		&Scumm::o5_ifClassOfIs,
		&Scumm::o5_walkActorTo,
		&Scumm::o5_isActorInBox,
		/* A0 */
		&Scumm::o5_stopObjectCode,
		&Scumm::o5_putActor,
		&Scumm::o5_getAnimCounter,
		&Scumm::o5_getActorY,
		/* A4 */
		&Scumm::o5_loadRoomWithEgo,
		&Scumm::o5_pickupObject,
		&Scumm::o5_setVarRange,
		&Scumm::o5_dummy,
		/* A8 */
		&Scumm::o5_notEqualZero,
		&Scumm::o5_setOwnerOf,
		&Scumm::o5_startScript,
		&Scumm::o5_saveRestoreVerbs,
		/* AC */
		&Scumm::o5_expression,
		&Scumm::o5_putActorInRoom,
		&Scumm::o5_wait,
		&Scumm::o5_badOpcode,
		/* B0 */
		&Scumm::o5_matrixOps,
		&Scumm::o5_getInventoryCount,
		&Scumm::o5_setCameraAt,
		&Scumm::o5_roomOps,
		/* B4 */
		&Scumm::o5_getDist,
		&Scumm::o5_findObject,
		&Scumm::o5_walkActorToObject,
		&Scumm::o5_startObject,
		/* B8 */
		&Scumm::o5_lessOrEqual,
		&Scumm::o5_doSentence,
		&Scumm::o5_subtract,
		&Scumm::o5_getActorScale,
		/* BC */
		&Scumm::o5_stopSound,
		&Scumm::o5_findInventory,
		&Scumm::o5_walkActorTo,
		&Scumm::o5_drawBox,
		/* C0 */
		&Scumm::o5_endCutscene,
		&Scumm::o5_putActor,
		&Scumm::o5_chainScript,
		&Scumm::o5_getActorX,
		/* C4 */
		&Scumm::o5_isLess,
		&Scumm::o5_drawObject,
		&Scumm::o5_decrement,
		&Scumm::o5_setState,
		/* C8 */
		&Scumm::o5_isEqual,
		&Scumm::o5_faceActor,
		&Scumm::o5_startScript,
		&Scumm::o5_getVerbEntrypoint,
		/* CC */
		&Scumm::o5_pseudoRoom,
		&Scumm::o5_walkActorToActor,
		&Scumm::o5_putActorAtObject,
		&Scumm::o5_badOpcode,
		/* D0 */
		&Scumm::o5_pickupObjectOld,
		&Scumm::o5_animateActor,
		&Scumm::o5_actorFollowCamera,
		&Scumm::o5_actorSet,
		/* D4 */
		&Scumm::o5_setObjectName,
		&Scumm::o5_actorFromPos,
		&Scumm::o5_getActorMoving,
		&Scumm::o5_or,
		/* D8 */
		&Scumm::o5_printEgo,
		&Scumm::o5_doSentence,
		&Scumm::o5_add,
		&Scumm::o5_divide,
		/* DC */
		&Scumm::o5_badOpcode,
		&Scumm::o5_actorSetClass,
		&Scumm::o5_walkActorTo,
		&Scumm::o5_isActorInBox,
		/* E0 */
		&Scumm::o5_freezeScripts,
		&Scumm::o5_putActor,
		&Scumm::o5_stopScript,
		&Scumm::o5_getActorFacing,
		/* E4 */
		&Scumm::o5_loadRoomWithEgo,
		&Scumm::o5_pickupObject,
		&Scumm::o5_getClosestObjActor,
		&Scumm::o5_dummy,
		/* E8 */
		&Scumm::o5_getScriptRunning,
		&Scumm::o5_setOwnerOf,
		&Scumm::o5_startScript,
		&Scumm::o5_debug,
		/* EC */
		&Scumm::o5_getActorWidth,
		&Scumm::o5_putActorInRoom,
		&Scumm::o5_stopObjectScript,
		&Scumm::o5_badOpcode,
		/* F0 */
		&Scumm::o5_lights,
		&Scumm::o5_getActorCostume,
		&Scumm::o5_loadRoom,
		&Scumm::o5_roomOps,
		/* F4 */
		&Scumm::o5_getDist,
		&Scumm::o5_findObject,
		&Scumm::o5_walkActorToObject,
		&Scumm::o5_startObject,
		/* F8 */
		&Scumm::o5_isGreater,
		&Scumm::o5_doSentence,
		&Scumm::o5_verbOps,
		&Scumm::o5_getActorWalkBox,
		/* FC */
		&Scumm::o5_isSoundRunning,
		&Scumm::o5_findInventory,
		&Scumm::o5_walkActorTo,
		&Scumm::o5_drawBox
	};

	static const char *opcode_lookup[] = {
		/* 00 */
		"o5_stopObjectCode",
		"o5_putActor",
		"o5_startMusic",
		"o5_getActorRoom",
		/* 04 */
		"o5_isGreaterEqual",
		"o5_drawObject",
		"o5_getActorElevation",
		"o5_setState",
		/* 08 */
		"o5_isNotEqual",
		"o5_faceActor",
		"o5_startScript",
		"o5_getVerbEntrypoint",
		/* 0C */
		"o5_resourceRoutines",
		"o5_walkActorToActor",
		"o5_putActorAtObject",
		"o5_getObjectState",
		/* 10 */
		"o5_getObjectOwner",
		"o5_animateActor",
		"o5_panCameraTo",
		"o5_actorSet",
		/* 14 */
		"o5_print",
		"o5_actorFromPos",
		"o5_getRandomNr",
		"o5_and",
		/* 18 */
		"o5_jumpRelative",
		"o5_doSentence",
		"o5_move",
		"o5_multiply",
		/* 1C */
		"o5_startSound",
		"o5_ifClassOfIs",
		"o5_walkActorTo",
		"o5_isActorInBox",
		/* 20 */
		"o5_stopMusic",
		"o5_putActor",
		"o5_getAnimCounter",
		"o5_getActorY",
		/* 24 */
		"o5_loadRoomWithEgo",
		"o5_pickupObject",
		"o5_setVarRange",
		"o5_stringOps",
		/* 28 */
		"o5_equalZero",
		"o5_setOwnerOf",
		"o5_startScript",
		"o5_delayVariable",
		/* 2C */
		"o5_cursorCommand",
		"o5_putActorInRoom",
		"o5_delay",
		"o5_badOpcode",
		/* 30 */
		"o5_matrixOps",
		"o5_getInventoryCount",
		"o5_setCameraAt",
		"o5_roomOps",
		/* 34 */
		"o5_getDist",
		"o5_findObject",
		"o5_walkActorToObject",
		"o5_startObject",
		/* 38 */
		"o5_lessOrEqual",
		"o5_doSentence",
		"o5_subtract",
		"o5_getActorScale",
		/* 3C */
		"o5_stopSound",
		"o5_findInventory",
		"o5_walkActorTo",
		"o5_drawBox",
		/* 40 */
		"o5_cutscene",
		"o5_putActor",
		"o5_chainScript",
		"o5_getActorX",
		/* 44 */
		"o5_isLess",
		"o5_badOpcode",
		"o5_increment",
		"o5_setState",
		/* 48 */
		"o5_isEqual",
		"o5_faceActor",
		"o5_startScript",
		"o5_getVerbEntrypoint",
		/* 4C */
		"o5_soundKludge",
		"o5_walkActorToActor",
		"o5_putActorAtObject",
		"o5_badOpcode",
		/* 50 */
		"o5_pickupObjectOld",
		"o5_animateActor",
		"o5_actorFollowCamera",
		"o5_actorSet",
		/* 54 */
		"o5_setObjectName",
		"o5_actorFromPos",
		"o5_getActorMoving",
		"o5_or",
		/* 58 */
		"o5_overRide",
		"o5_doSentence",
		"o5_add",
		"o5_divide",
		/* 5C */
		"o5_oldRoomEffect",
		"o5_actorSetClass",
		"o5_walkActorTo",
		"o5_isActorInBox",
		/* 60 */
		"o5_freezeScripts",
		"o5_putActor",
		"o5_stopScript",
		"o5_getActorFacing",
		/* 64 */
		"o5_loadRoomWithEgo",
		"o5_pickupObject",
		"o5_getClosestObjActor",
		"o5_dummy",
		/* 68 */
		"o5_getScriptRunning",
		"o5_setOwnerOf",
		"o5_startScript",
		"o5_debug",
		/* 6C */
		"o5_getActorWidth",
		"o5_putActorInRoom",
		"o5_stopObjectScript",
		"o5_badOpcode",
		/* 70 */
		"o5_lights",
		"o5_getActorCostume",
		"o5_loadRoom",
		"o5_roomOps",
		/* 74 */
		"o5_getDist",
		"o5_findObject",
		"o5_walkActorToObject",
		"o5_startObject",
		/* 78 */
		"o5_isGreater",							/* less? */
		"o5_doSentence",
		"o5_verbOps",
		"o5_getActorWalkBox",
		/* 7C */
		"o5_isSoundRunning",
		"o5_findInventory",
		"o5_walkActorTo",
		"o5_drawBox",
		/* 80 */
		"o5_breakHere",
		"o5_putActor",
		"o5_startMusic",
		"o5_getActorRoom",
		/* 84 */
		"o5_isGreaterEqual",				/* less equal? */
		"o5_drawObject",
		"o5_getActorElevation",
		"o5_setState",
		/* 88 */
		"o5_isNotEqual",
		"o5_faceActor",
		"o5_startScript",
		"o5_getVerbEntrypoint",
		/* 8C */
		"o5_resourceRoutines",
		"o5_walkActorToActor",
		"o5_putActorAtObject",
		"o5_getObjectState",
		/* 90 */
		"o5_getObjectOwner",
		"o5_animateActor",
		"o5_panCameraTo",
		"o5_actorSet",
		/* 94 */
		"o5_print",
		"o5_actorFromPos",
		"o5_getRandomNr",
		"o5_and",
		/* 98 */
		"o5_quitPauseRestart",
		"o5_doSentence",
		"o5_move",
		"o5_multiply",
		/* 9C */
		"o5_startSound",
		"o5_ifClassOfIs",
		"o5_walkActorTo",
		"o5_isActorInBox",
		/* A0 */
		"o5_stopObjectCode",
		"o5_putActor",
		"o5_getAnimCounter",
		"o5_getActorY",
		/* A4 */
		"o5_loadRoomWithEgo",
		"o5_pickupObject",
		"o5_setVarRange",
		"o5_dummy",
		/* A8 */
		"o5_notEqualZero",
		"o5_setOwnerOf",
		"o5_startScript",
		"o5_saveRestoreVerbs",
		/* AC */
		"o5_expression",
		"o5_putActorInRoom",
		"o5_wait",
		"o5_badOpcode",
		/* B0 */
		"o5_matrixOps",
		"o5_getInventoryCount",
		"o5_setCameraAt",
		"o5_roomOps",
		/* B4 */
		"o5_getDist",
		"o5_findObject",
		"o5_walkActorToObject",
		"o5_startObject",
		/* B8 */
		"o5_lessOrEqual",
		"o5_doSentence",
		"o5_subtract",
		"o5_getActorScale",
		/* BC */
		"o5_stopSound",
		"o5_findInventory",
		"o5_walkActorTo",
		"o5_drawBox",
		/* C0 */
		"o5_endCutscene",
		"o5_putActor",
		"o5_chainScript",
		"o5_getActorX",
		/* C4 */
		"o5_isLess",
		"o5_badOpcode",
		"o5_decrement",
		"o5_setState",
		/* C8 */
		"o5_isEqual",
		"o5_faceActor",
		"o5_startScript",
		"o5_getVerbEntrypoint",
		/* CC */
		"o5_pseudoRoom",
		"o5_walkActorToActor",
		"o5_putActorAtObject",
		"o5_badOpcode",
		/* D0 */
		"o5_pickupObjectOld",
		"o5_animateActor",
		"o5_actorFollowCamera",
		"o5_actorSet",
		/* D4 */
		"o5_setObjectName",
		"o5_actorFromPos",
		"o5_getActorMoving",
		"o5_or",
		/* D8 */
		"o5_printEgo",
		"o5_doSentence",
		"o5_add",
		"o5_divide",
		/* DC */
		"o5_badOpcode",
		"o5_actorSetClass",
		"o5_walkActorTo",
		"o5_isActorInBox",
		/* E0 */
		"o5_freezeScripts",
		"o5_putActor",
		"o5_stopScript",
		"o5_getActorFacing",
		/* E4 */
		"o5_loadRoomWithEgo",
		"o5_pickupObject",
		"o5_getClosestObjActor",
		"o5_dummy",
		/* E8 */
		"o5_getScriptRunning",
		"o5_setOwnerOf",
		"o5_startScript",
		"o5_debug",
		/* EC */
		"o5_getActorWidth",
		"o5_putActorInRoom",
		"o5_stopObjectScript",
		"o5_badOpcode",
		/* F0 */
		"o5_lights",
		"o5_getActorCostume",
		"o5_loadRoom",
		"o5_roomOps",
		/* F4 */
		"o5_getDist",
		"o5_findObject",
		"o5_walkActorToObject",
		"o5_startObject",
		/* F8 */
		"o5_isGreater",
		"o5_doSentence",
		"o5_verbOps",
		"o5_getActorWalkBox",
		/* FC */
		"o5_isSoundRunning",
		"o5_findInventory",
		"o5_walkActorTo",
		"o5_drawBox",
	};


	_opcodes = opcode_list;
	_opcodes_lookup = opcode_lookup;
}

void Scumm::o5_actorFollowCamera()
{
	actorFollowCamera(getVarOrDirectByte(0x80));
}

void Scumm::o5_actorFromPos()
{
	int x, y;
	getResultPos();
	x = getVarOrDirectWord(0x80);
	y = getVarOrDirectWord(0x40);
	setResult(getActorFromPos(x, y));
}

void Scumm::o5_actorSet()
{
	static const byte convertTable[20] =
		{ 1, 0, 0, 2, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20 };
	int act = getVarOrDirectByte(0x80);
	Actor *a;
	int i, j;

	if (act == 0)
		act = 1;

	a = derefActorSafe(act, "actorSet");

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		if (_features & GF_SMALL_HEADER)
			_opcode = (_opcode & 0xE0) | convertTable[(_opcode & 0x1F) - 1];

		if (!a)
			return;

		switch (_opcode & 0x1F) {
		case 0:										/* dummy case */
			getVarOrDirectByte(0x80);
			break;
		case 1:										/* costume */
			setActorCostume(a, getVarOrDirectByte(0x80));
			break;
		case 2:										/* walkspeed */
			i = getVarOrDirectByte(0x80);
			j = getVarOrDirectByte(0x40);
			setActorWalkSpeed(a, i, j);
			break;
		case 3:										/* sound */
			a->sound[0] = getVarOrDirectByte(0x80);
			break;
		case 4:										/* walkanim */
			a->walkFrame = getVarOrDirectByte(0x80);
			break;
		case 5:										/* talkanim */
			a->talkFrame1 = getVarOrDirectByte(0x80);
			a->talkFrame2 = getVarOrDirectByte(0x40);
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
			initActor(a, 0);
			break;
		case 9:										/* elevation */
			a->elevation = getVarOrDirectWord(0x80);
			a->needRedraw = true;
			a->needBgReset = true;
			break;
		case 10:										/* defaultanims */
			a->initFrame = 1;
			a->walkFrame = 2;
			a->standFrame = 3;
			a->talkFrame1 = 4;
			a->talkFrame2 = 5;
			break;
		case 11:										/* palette */
			i = getVarOrDirectByte(0x80);
			j = getVarOrDirectByte(0x40);
			checkRange(31, 0, i, "Illegal palet slot %d");
			a->palette[i] = j;
			a->needRedraw = true;
			break;
		case 12:										/* talk color */
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
			break;
		case 16:										/* width */
			a->width = getVarOrDirectByte(0x80);
			break;
		case 17:										/* scale */
			a->scalex = getVarOrDirectByte(0x80);
			a->scaley = getVarOrDirectByte(0x40);
			if (a->scalex > 255 || a->scaley > 255)
				error("Setting an bad actor scale!");
			a->needRedraw = true;
			a->needBgReset = true;
			break;
		case 18:										/* neverzclip */
			a->forceClip = 0;
			break;
		case 19:										/* setzclip */
			a->forceClip = getVarOrDirectByte(0x80);
			break;
		case 20:										/* ignoreboxes */
			a->ignoreBoxes = 1;
			a->forceClip = 0;
		FixRoom:
			if (a->room == _currentRoom)
				putActor(a, a->x, a->y, a->room);
			break;
		case 21:										/* followboxes */
			a->ignoreBoxes = 0;
			a->forceClip = 0;
			goto FixRoom;

		case 22:										/* animspeed */
			a->animSpeed = getVarOrDirectByte(0x80);
			a->animProgress = 0;
			break;
		case 23:										/* unk2 */
			a->shadow_mode = getVarOrDirectByte(0x80);	/* shadow mode */
			break;
		default:
			warning("o5_actorSet: default case");
		}
	}
}

void Scumm::o5_actorSetClass()
{
	int act = getVarOrDirectWord(0x80);
	int newClass;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		newClass = getVarOrDirectWord(0x80);
		if (newClass == 0) {
			_classData[act] = 0;
			continue;
		}

		if (newClass & 0x80)
			putClass(act, newClass, 1);
		else
			putClass(act, newClass, 0);
	}
}

void Scumm::o5_add()
{
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) + a);
}

void Scumm::o5_and()
{
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) & a);
}

void Scumm::o5_animateActor()
{
	int act, anim;

	act = getVarOrDirectByte(0x80);
	anim = getVarOrDirectByte(0x40);
	animateActor(act, anim);
}

void Scumm::o5_badOpcode()
{
	error("Scumm opcode %d illegal", _opcode);
}

void Scumm::o5_breakHere()
{
	updateScriptPtr();
	_currentScript = 0xFF;
}

void Scumm::o5_chainScript()
{
	int16 vars[16];
	int data;
	int cur;

	data = getVarOrDirectByte(0x80);

	getWordVararg(vars);

	cur = _currentScript;

	if (vm.slot[cur].cutsceneOverride != 0)
		error("Script %d chaining with active cutscene/override");

	vm.slot[cur].number = 0;
	vm.slot[cur].status = 0;
	_currentScript = 0xFF;

	runScript(data, vm.slot[cur].unk1, vm.slot[cur].unk2, vars);
}

void Scumm::o5_cursorCommand()
{
	int i, j, k;
	int16 table[16];

	switch ((_opcode = fetchScriptByte()) & 0x1F) {
	case 1:											/* cursor show */
		_cursorState = 1;
		verbMouseOver(0);
		break;
	case 2:											/* cursor hide */
		_cursorState = 0;
		verbMouseOver(0);
		break;
	case 3:											/* userput on */
		_userPut = 1;
		break;
	case 4:											/* userput off */
		_userPut = 0;
		break;
	case 5:											/* cursor soft on */
		_cursorState++;
		if (_cursorState > 1) {
			error("Cursor state greater than 1 in script");
		}
		verbMouseOver(0);
		break;
	case 6:											/* cursor soft off */
		_cursorState--;
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
		if (_gameId != GID_LOOM256)
			setCursorImg(i, j, 1);
		break;
	case 11:											/* set cursor hotspot */
		i = getVarOrDirectByte(0x80);
		j = getVarOrDirectByte(0x40);
		k = getVarOrDirectByte(0x20);
		setCursorHotspot2(j, k);
		break;

	case 12:											/* init cursor */
		setCursor(getVarOrDirectByte(0x80));
		break;
	case 13:											/* init charset */
		initCharset(getVarOrDirectByte(0x80));
		break;
	case 14:											/* unk */
		getWordVararg(table);
		for (i = 0; i < 16; i++)
			charset._colorMap[i] = _charsetData[string[1].t_charset][i] =
				(unsigned char)table[i];
		break;
	}

	_vars[VAR_CURSORSTATE] = _cursorState;
	_vars[VAR_USERPUT] = _userPut;
}

void Scumm::o5_cutscene()
{
	int16 args[16];
	getWordVararg(args);
	cutscene(args);
}

void Scumm::o5_endCutscene()
{
	endCutscene();
}

void Scumm::o5_debug()
{
	getVarOrDirectWord(0x80);
}

void Scumm::o5_decrement()
{
	getResultPos();
	setResult(readVar(_resultVarNumber) - 1);
}

void Scumm::o5_delay()
{
	int delay = fetchScriptByte();
	delay |= fetchScriptByte() << 8;
	delay |= fetchScriptByte() << 16;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o5_breakHere();
}

void Scumm::o5_delayVariable()
{
	vm.slot[_currentScript].delay = readVar(fetchScriptWord());
	vm.slot[_currentScript].status = 1;
	o5_breakHere();
}

void Scumm::o5_divide()
{
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	if (a == 0) {
		error("Divide by zero");
		setResult(0);
	} else
		setResult(readVar(_resultVarNumber) / a);
}

void Scumm::o5_doSentence()
{
	int a, b;
	SentenceTab *st;

	a = getVarOrDirectByte(0x80);
	if (a == 0xFE) {
		_sentenceNum = 0;
		stopScriptNr(_vars[VAR_SENTENCE_SCRIPT]);
		clearClickedStatus();
		return;
	}

	st = &sentence[_sentenceNum++];

	st->unk5 = a;
	st->unk4 = getVarOrDirectWord(0x40);
	b = st->unk3 = getVarOrDirectWord(0x20);
	if (b == 0) {
		st->unk2 = 0;
	} else {
		st->unk2 = 1;
	}
	st->unk = 0;
}

void Scumm::o5_drawBox()
{
	int x, y, x2, y2, color;

	x = getVarOrDirectWord(0x80);
	y = getVarOrDirectWord(0x40);

	_opcode = fetchScriptByte();
	x2 = getVarOrDirectWord(0x80);
	y2 = getVarOrDirectWord(0x40);
	color = getVarOrDirectByte(0x20);

	drawBox(x, y, x2, y2, color);
}

void Scumm::o5_drawObject()
{
	int state, obj, idx, i;
	ObjectData *od;
	uint16 x, y, w, h;
	int xpos, ypos;

	state = 1;
	xpos = ypos = 255;
	obj = getVarOrDirectWord(0x80);

	if (_features & GF_SMALL_HEADER) {
		int temp = getVarOrDirectWord(0x40);
		getVarOrDirectWord(0x20);		// Room

		idx = getObjectIndex(obj);
		if (idx == -1)
			return;
		od = &_objs[idx];
		xpos = ypos = 255;
		if (temp != 0xFF) {
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

		i = _numObjectsInRoom;
		do {
			if (_objs[i].x_pos == x && _objs[i].y_pos == y && _objs[i].width == w
					&& _objs[i].height == h)
				putState(_objs[i].obj_nr, 0);
		} while (--i);

		putState(obj, state);
		return;
	}

	switch ((_opcode = fetchScriptByte()) & 0x1F) {
	case 1:											/* draw at */
		xpos = getVarOrDirectWord(0x80);
		ypos = getVarOrDirectWord(0x40);
		break;
	case 2:											/* set state */
		state = getVarOrDirectWord(0x80);
		break;
	case 0x1F:										/* neither */
		break;
	default:
		error("o5_drawObject: default case");
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

	i = _numObjectsInRoom;
	do {
		if (_objs[i].x_pos == x && _objs[i].y_pos == y
				&& _objs[i].width == w && _objs[i].height == h)
			putState(_objs[i].obj_nr, 0);
	} while (--i);

	putState(obj, state);
}

void Scumm::o5_dummy()
{
	/* nothing */
}


void Scumm::o5_expression()
{
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
			(this->*(getOpcode(_opcode))) ();
			push(_vars[0]);
			break;
		}
	}

	_resultVarNumber = dst;
	setResult(pop());
}

void Scumm::o5_faceActor()
{
	int act, obj;
	act = getVarOrDirectByte(0x80);
	obj = getVarOrDirectWord(0x40);
	faceActorToObj(act, obj);
}

void Scumm::o5_findInventory()
{
	int t;
	getResultPos();
	t = getVarOrDirectByte(0x80);
	setResult(findInventory(t, getVarOrDirectByte(0x40)));
}

void Scumm::o5_findObject()
{
	int t;
	getResultPos();
	t = getVarOrDirectWord(0x80);
	setResult(findObject(t, getVarOrDirectWord(0x40)));
}

void Scumm::o5_freezeScripts()
{
	int scr = getVarOrDirectByte(0x80);

	if (scr != 0)
		freezeScripts(scr);
	else
		unfreezeScripts();
}

void Scumm::o5_getActorCostume()
{
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorCostume")->
						costume);
}

void Scumm::o5_getActorElevation()
{
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorElevation")->
						elevation);
}

void Scumm::o5_getActorFacing()
{
	getResultPos();
	setResult(newDirToOldDir
						(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorFacing")->
						 facing));
}

void Scumm::o5_getActorMoving()
{
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorMoving")->
						moving);
}

void Scumm::o5_getActorRoom()
{
	int temp;
	Actor *act;
	getResultPos();
	temp = getVarOrDirectByte(0x80);
	
	act = derefActorSafe(temp, "o5_getActorRoom");
	if (!act) 
		return;

	setResult(act->room);
}

void Scumm::o5_getActorScale()
{
	if (_gameId == GID_INDY3_256) {
		getVarOrDirectByte(0x80);		/*FIXME: missing stuff here */
		return;
	}
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorScale")->
						scalex);
}

void Scumm::o5_getActorWalkBox()
{
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorWalkbox")->
						walkbox);
}

void Scumm::o5_getActorWidth()
{
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorWidth")->
						width);
}

void Scumm::o5_getActorX()
{
	int actor;
	getResultPos();
	actor = getVarOrDirectWord(0x80);
	setResult(getObjX(actor));
}

void Scumm::o5_getActorY()
{
	getResultPos();
	setResult(getObjY(getVarOrDirectWord(0x80)));
}

void Scumm::o5_getAnimCounter()
{
	getResultPos();
	setResult(derefActorSafe
						(getVarOrDirectByte(0x80),
						 "o5_getActorAnimCounter")->cost.animCounter1);
}

void Scumm::o5_getClosestObjActor()
{
	int obj;
	int act;
	int closest_obj = 0xFF, closest_dist = 0xFF;
	int dist;

	getResultPos();

	act = getVarOrDirectWord(0x80);
	obj = _vars[VAR_V5_OBJECT_HI];

	do {
		dist = getObjActToObjActDist(act, obj);
		if (dist < closest_dist) {
			closest_dist = dist;
			closest_obj = obj;
		}
	} while (--obj >= _vars[VAR_V5_OBJECT_LO]);

	setResult(closest_dist);
}

void Scumm::o5_getDist()
{
	int o1, o2;
	int r;
	getResultPos();
	o1 = getVarOrDirectWord(0x80);
	o2 = getVarOrDirectWord(0x40);
	r = getObjActToObjActDist(o1, o2);

	/* Fix for monkey 2, dunno what's wrong in scummvm */
	if (_gameId == GID_MONKEY2 && vm.slot[_currentScript].number == 40
			&& r < 60)
		r = 60;

	setResult(r);
}

void Scumm::o5_getInventoryCount()
{
	getResultPos();
	setResult(getInventoryCount(getVarOrDirectByte(0x80)));
}

void Scumm::o5_getObjectOwner()
{
	getResultPos();
	setResult(getOwner(getVarOrDirectWord(0x80)));
}

void Scumm::o5_getObjectState()
{
	if (_features & GF_SMALL_HEADER) {
		int a = getVarOrDirectWord(0x80);
		int b = getVarOrDirectByte(0x40);

		if ((getState(a) & 0xF0 >> 4) != b)
			o5_jumpRelative();
		else
			ignoreScriptWord();
	} else {
		getResultPos();
		setResult(getState(getVarOrDirectWord(0x80)));
	}
}

void Scumm::o5_getRandomNr()
{
	getResultPos();
	setResult(getRandomNumber(getVarOrDirectByte(0x80) + 1));
}

void Scumm::o5_getScriptRunning()
{
	getResultPos();
	setResult(isScriptRunning(getVarOrDirectByte(0x80)));
}

void Scumm::o5_getVerbEntrypoint()
{
	int a, b;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	b = getVarOrDirectWord(0x40);

	setResult(getVerbEntrypoint(a, b));
}

void Scumm::o5_ifClassOfIs()
{
	int act, cls, b;
	bool cond = true;

	act = getVarOrDirectWord(0x80);

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		cls = getVarOrDirectWord(0x80);
		b = getClass(act, cls);

		if (cls & 0x80 && !b || !(cls & 0x80) && b)
			cond = false;
	}
	if (cond)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm::o5_increment()
{
	getResultPos();
	setResult(readVar(_resultVarNumber) + 1);
}

void Scumm::o5_isActorInBox()
{
	int box;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_isActorInBox");
	box = getVarOrDirectByte(0x40);

	if (!checkXYInBoxBounds(box, a->x, a->y))
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm::o5_isEqual()
{
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b == a)
		ignoreScriptWord();
	else
		o5_jumpRelative();

}

void Scumm::o5_isGreater()
{
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b > a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm::o5_isGreaterEqual()
{
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b >= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm::o5_isLess()
{
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);

	if (b < a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm::o5_lessOrEqual()
{
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b <= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm::o5_isNotEqual()
{
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b != a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm::o5_notEqualZero()
{
	int a = readVar(fetchScriptWord());
	if (a != 0)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm::o5_equalZero()
{
	int a = readVar(fetchScriptWord());
	if (a == 0)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm::o5_isSoundRunning()
{
	int snd;
	getResultPos();
	snd = getVarOrDirectByte(0x80);
	if (snd)
		snd = isSoundRunning(snd);
	setResult(snd);
}

void Scumm::o5_jumpRelative()
{
	_scriptPointer += (int16) fetchScriptWord();
}

void Scumm::o5_lights()
{
	int a, b, c;

	a = getVarOrDirectByte(0x80);
	b = fetchScriptByte();
	c = fetchScriptByte();

/*	if (c==0)
		_vars[VAR_V5_DRAWFLAGS] = a;
	else if (c==1) {*/
	warning("o5_lights: lights not implemented");
//  }
	_fullRedraw = 1;
}

void Scumm::o5_loadRoom()
{
	int room;

	/* Begin: Autosave 
	   _saveLoadSlot = 0;
	   sprintf(_saveLoadName, "Autosave", _saveLoadSlot);
	   _saveLoadFlag = 1;
	   _saveLoadCompatible = false;
	   End: Autosave */

	room = getVarOrDirectByte(0x80);
	startScene(room, 0, 0);
	_fullRedraw = 1;
}

void Scumm::o5_loadRoomWithEgo()
{
	int obj, room, x, y;
	Actor *a;

	/* Begin: Autosave 
	   _saveLoadSlot = 0;
	   sprintf(_saveLoadName, "Autosave", _saveLoadSlot);
	   _saveLoadFlag = 1;
	   _saveLoadCompatible = false;
	   End: Autosave */

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);

	a = derefActorSafe(_vars[VAR_EGO], "o5_loadRoomWithEgo");

	/* Warning: used previously _xPos, _yPos from a previous update of those */
	putActor(a, a->x, a->y, room);

	x = (int16) fetchScriptWord();
	y = (int16) fetchScriptWord();

	_egoPositioned = false;

	_vars[VAR_WALKTO_OBJ] = obj;
	startScene(a->room, a, obj);
	_vars[VAR_WALKTO_OBJ] = 0;

	camera._dest.x = camera._cur.x = a->x;
	setCameraFollows(a);

	_fullRedraw = 1;

	if (x != -1) {
		startWalkActor(a, x, y, -1);
	}
}

void Scumm::o5_matrixOps()
{
	int a, b;

	if (_features & GF_OLD256) {
		a = getVarOrDirectByte(0x80);
		b = fetchScriptByte();
		if (b == 0x40)							// Lock Box
			setBoxFlags(a, 0x80);
		else
			setBoxFlags(a, 0);
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

void Scumm::o5_move()
{
	getResultPos();
	setResult(getVarOrDirectWord(0x80));
}

void Scumm::o5_multiply()
{
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) * a);
}


void Scumm::o5_or()
{
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) | a);
}

void Scumm::o5_overRide()
{
	if (fetchScriptByte() != 0)
		beginOverride();
	else
		endOverride();
}

void Scumm::o5_panCameraTo()
{
	panCameraTo(getVarOrDirectWord(0x80), 0);
}

void Scumm::o5_pickupObject()
{
	int obj, room;

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);
	if (room == 0)
		room = _roomResource;
	addObjectToInventory(obj, room);
	putOwner(obj, _vars[VAR_EGO]);
	putClass(obj, 32, 1);
	putState(obj, 1);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
	runHook(1);
}

void Scumm::o5_print()
{
	_actorToPrintStrFor = getVarOrDirectByte(0x80);
	decodeParseString();
}

void Scumm::o5_printEgo()
{
	_actorToPrintStrFor = (unsigned char)_vars[VAR_EGO];
	decodeParseString();
}

void Scumm::o5_pseudoRoom()
{
	int i = fetchScriptByte(), j;
	while ((j = fetchScriptByte()) != 0) {
		if (j >= 0x80) {
			_resourceMapper[j & 0x7F] = i;
		}
	}
}

void Scumm::o5_putActor()
{
	int x, y;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_putActor");
	if (!a)
		return;
	x = getVarOrDirectWord(0x40);
	y = getVarOrDirectWord(0x20);

	putActor(a, x, y, a->room);
}


void Scumm::o5_putActorAtObject()
{
	int obj;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_putActorAtObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj);
	else {
		_xPos = 240;
		_yPos = 120;
	}
	putActor(a, _xPos, _yPos, a->room);
}

void Scumm::o5_putActorInRoom()
{
	int room;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_putActorInRoom");
	room = getVarOrDirectByte(0x40);
	if (a->visible && _currentRoom != room
			&& _vars[VAR_TALK_ACTOR] == a->number) {
		clearMsgQueue();
	}
	a->room = room;
	if (!room)
		putActor(a, 0, 0, 0);
}

void Scumm::o5_quitPauseRestart()
{
	switch (fetchScriptByte()) {
	case 1:
		pauseGame(false);
		break;
	case 3:
		shutDown(0);
		break;
	}
}

void Scumm::o5_resourceRoutines()
{
	int resid = 0;

	_opcode = fetchScriptByte();
	if (_opcode != 17)
		resid = getVarOrDirectByte(0x80);
	if (_features & GF_OLD256)		/*FIXME: find a better way to implement this */
		_opcode &= 0x3F;
	switch (_opcode & 0x1F) {
	case 1:											/* load script */
		ensureResourceLoaded(rtScript, resid);
		break;
	case 2:											/* load sound */
		ensureResourceLoaded(rtSound, resid);
		break;
	case 3:											/* load costume */
		ensureResourceLoaded(rtCostume, resid);
		break;
	case 4:											/* load room */
		if (_features & GF_OLD256)
			ensureResourceLoaded(rtScript, resid & 0x7F);	/*FIXME: missing stuff... */
		else
			ensureResourceLoaded(rtRoom, resid);
		break;
	case 5:											/* nuke script */
		setResourceCounter(rtScript, resid, 0x7F);
		break;
	case 6:											/* nuke sound */
		setResourceCounter(rtSound, resid, 0x7F);
		break;
	case 7:											/* nuke costume */
		setResourceCounter(rtCostume, resid, 0x7F);
		break;
	case 8:											/* nuke room */
		setResourceCounter(rtRoom, resid, 0x7F);
		break;
	case 9:											/* lock script */
		if (resid >= _numGlobalScripts)
			break;
		lock(rtScript, resid);
		break;
	case 10:											/* lock sound */
		lock(rtSound, resid);
		break;
	case 11:											/* lock costume */
		lock(rtCostume, resid);
		break;
	case 12:											/* lock room */
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		lock(rtRoom, resid);
		break;
	case 13:											/* unlock script */
		if (resid >= _numGlobalScripts)
			break;
		unlock(rtScript, resid);
		break;
	case 14:											/* unlock sound */
		unlock(rtSound, resid);
		break;
	case 15:											/* unlock costume */
		unlock(rtCostume, resid);
		break;
	case 16:											/* unlock room */
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		unlock(rtRoom, resid);
		break;
	case 17:											/* clear heap */
		heapClear(0);
		unkHeapProc2(0, 0);
		break;
	case 18:											/* load charset */
		loadCharset(resid);
		break;
	case 19:											/* nuke charset */
		nukeCharset(resid);
		break;
	case 20:											/* load fl object */
		loadFlObject(getVarOrDirectWord(0x40), resid);
		break;
	default:
		warning("Unknown o5_resourcesroutine: %d", _opcode & 0x1F);
		break;
	}
}

void Scumm::o5_roomOps()
{
	int a = 0, b = 0, c, d, e;

	if (_features & GF_OLD256) {
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		if (_gameId == GID_INDY3_256 && a == 16 && b == 0) {	/* FIXME */
			// Set screen height
			c = fetchScriptByte();
			d = fetchScriptByte();
			e = fetchScriptByte();
			initScreens(0, a, 320, c);
			return;
		}
	}

	_opcode = fetchScriptByte();

	switch (_opcode & 0x1F) {
	case 1:											/* room scroll */
		if (!(_features & GF_OLD256)) {
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectWord(0x40);
		}
		if (a < 160)
			a = 160;
		if (b < 160)
			b = 160;
		if (a > _scrWidth - 160)
			a = _scrWidth - 160;
		if (b > _scrWidth - 160)
			b = _scrWidth - 160;
		_vars[VAR_CAMERA_MIN_X] = a;
		_vars[VAR_CAMERA_MAX_X] = b;
		break;
	case 2:											/* room color */
		if (_features & GF_SMALL_HEADER) {
			if (!(_features & GF_OLD256)) {
				a = getVarOrDirectWord(0x80);
				b = getVarOrDirectWord(0x40);
			}
			checkRange(256, 0, a, "o5_roomOps: 2: Illegal room color slot (%d)");
			_currentPalette[a] = b;
			_fullRedraw = 1;
		} else {
			error("room-color is no longer a valid command");
		}
		break;

	case 3:											/* set screen */
		if (!(_features & GF_OLD256)) {
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectWord(0x40);
		}
		initScreens(0, a, 320, b);
		break;
	case 4:											/* set palette color */
		if (_features & GF_SMALL_HEADER) {
			if (!(_features & GF_OLD256)) {
				a = getVarOrDirectWord(0x80);
				b = getVarOrDirectWord(0x40);
			}
			checkRange(256, 0, a, "o5_roomOps: 2: Illegal room color slot (%d)");
			_currentPalette[a] = b;		/*FIXME: should be shadow palette */
			//        _fullRedraw = 1;
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
	case 8:											/* room scale? */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		c = getVarOrDirectByte(0x20);
		darkenPalette(b, c, a, a, a);
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
			_switchRoomEffect = (byte)(a);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			screenEffect(_newEffect);
		}
		break;
	case 11:											/* ? */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		e = getVarOrDirectByte(0x40);
		darkenPalette(d, e, a, b, c);
		break;
	case 12:											/* ? */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		e = getVarOrDirectByte(0x40);
		unkRoomFunc3(d, e, a, b, c);
		break;

	case 13:{										/* save-string */
			char buf[256], *s;
			a = getVarOrDirectByte(0x80);
			s = buf;
			while ((*s++ = fetchScriptByte()));
			warning("roomops:13 save-string(%d,\"%s\") not implemented", a, buf);
			break;
		}
	case 14:											/* load-string */
		char buf[256], *s;
		a = getVarOrDirectByte(0x80);
		s = buf;
		while ((*s++ = fetchScriptByte()));
		warning("roomops:14 load-string(%d,\"%s\") not implemented", a, buf);
		break;
	case 15:											/* palmanip? */
		a = getVarOrDirectByte(0x80);
		_opcode = fetchScriptByte();
		b = getVarOrDirectByte(0x80);
		c = getVarOrDirectByte(0x40);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		unkRoomFunc4(b, c, a, d, 1);
		break;

	case 16:											/* ? */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		if (a < 1)
			a = 1;										/* FIXME: ZAK256 */
		checkRange(16, 1, a, "o5_roomOps: 16: color cycle out of range (%d)");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;
	}
}

void Scumm::o5_saveRestoreVerbs()
{
	int a, b, c, slot, slot2;

	_opcode = fetchScriptByte();

	a = getVarOrDirectByte(0x80);
	b = getVarOrDirectByte(0x40);
	c = getVarOrDirectByte(0x20);

	switch (_opcode) {
	case 1:											/* hide verbs */
		if (_gameId == GID_ZAK256)	// FIXME?
			return;

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
		error("o5_saveRestoreVerbs: invalid opcode");
	}
}

void Scumm::o5_setCameraAt()
{
	setCameraAtEx(getVarOrDirectWord(0x80));
}

void Scumm::o5_setObjectName()
{
	int obj = getVarOrDirectWord(0x80);
	int size;
	int a;
	int i;
	byte *name;
	unsigned char work[255];

	if (obj < NUM_ACTORS)
		error("Can't set actor %d name with new-name-of", obj);

	if (!getOBCDFromObject(obj))
		error("Can't set name of object %d", obj);

	name = getObjOrActorName(obj);
	size = getResourceDataSize(name);
	i = 0;

	while ((a = fetchScriptByte()) != 0) {
		work[i++] = a;

		if (a == 0xFF) {
			work[i++] = fetchScriptByte();
			work[i++] = fetchScriptByte();
			work[i++] = fetchScriptByte();
		}

	}

	if (i >= size) {
		work[i] = 0;
		warning("New name of object %d too long (old *%s* new *%s*)",
						obj, name, work);
		i = size - 1;
	}

	work[i] = 0;
	strcpy((char *)name, (char *)work);
	runHook(0);
}

void Scumm::o5_setOwnerOf()
{
	int obj, owner;

	obj = getVarOrDirectWord(0x80);
	owner = getVarOrDirectByte(0x40);

	setOwnerOf(obj, owner);
}

void Scumm::o5_setState()
{
	int obj, state;
	obj = getVarOrDirectWord(0x80);
	state = getVarOrDirectByte(0x40);
	putState(obj, state);
	removeObjectFromRoom(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void Scumm::o5_setVarRange()
{
	int a, b;

	getResultPos();
	a = fetchScriptByte();
	do {
		if (_opcode & 0x80)
			b = fetchScriptWord();
		else
			b = fetchScriptByte();

		setResult(b);
		_resultVarNumber++;
	} while (--a);
}

void Scumm::o5_soundKludge()
{
	int16 items[15];
	int i;

	if (_features & GF_SMALL_HEADER)	// Is dummy function in
		return;											// SCUMM V3

	for (i = 0; i < 15; i++)
		items[i] = 0;

	getWordVararg(items);

	soundKludge(items);
}

void Scumm::o5_startMusic()
{
	addSoundToQueue(getVarOrDirectByte(0x80));
}

void Scumm::o5_startObject()
{
	int obj, script;
	int16 data[16];

	obj = getVarOrDirectWord(0x80);
	script = getVarOrDirectByte(0x40);

	getWordVararg(data);
	runVerbCode(obj, script, 0, 0, data);
}

void Scumm::o5_startScript()
{
	int op, script;
	int16 data[16];
	int a, b;

	op = _opcode;
	script = getVarOrDirectByte(0x80);

	getWordVararg(data);

	a = b = 0;
	if (op & 0x40)
		b = 1;
	if (op & 0x20)
		a = 1;

	runScript(script, a, b, data);
}

void Scumm::o5_startSound()
{
	addSoundToQueue(getVarOrDirectByte(0x80));
}

void Scumm::o5_stopMusic()
{
	stopAllSounds();
}

void Scumm::o5_stopObjectCode()
{
	stopObjectCode();
}

void Scumm::o5_stopObjectScript()
{
	stopObjectScript(getVarOrDirectWord(0x80));
}

void Scumm::o5_stopScript()
{
	int script;

	script = getVarOrDirectByte(0x80);
	if (!script)
		stopObjectCode();		
	else
		stopScriptNr(script);
}

void Scumm::o5_stopSound()
{
	stopSound(getVarOrDirectByte(0x80));
}

void Scumm::o5_stringOps()
{
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
		ptr = getResourceAddress(rtString, a);
		if (!(_gameId == GID_LOOM256)) {	/* FIXME - LOOM256 */
			if (ptr == NULL)
				error("String %d does not exist", a);
			c = getVarOrDirectByte(0x20);
			ptr[b] = c;
		} else
			getVarOrDirectByte(0x20);

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

void Scumm::o5_subtract()
{
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) - a);
}

void Scumm::o5_verbOps()
{
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
				vs->type = 1;
			}
			break;
		case 2:										/* load from code */
			loadPtrToResource(rtVerb, slot, NULL);
			if (slot == 0)
				nukeResource(rtVerb, slot);
			vs->type = 0;
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
				if ((verb >= 90) && (verb <= 97)) { // Notes
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
			vs->type = 0;
			vs->charset_nr = string[0].t_charset;
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
			vs->type = 0;
			vs->imgindex = 0;
			break;
		case 22:										/* assign object */
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectByte(0x40);
			if (slot && vs->imgindex != a) {
				setVerbObject(b, a, slot);
				vs->type = 1;
				vs->imgindex = a;
			}
			break;
		case 23:										/* set back color */
			vs->bkcolor = getVarOrDirectByte(0x80);
			break;
		}
	}
	drawVerb(slot, 0);
	verbMouseOver(0);
}

void Scumm::o5_wait()
{
	byte *oldaddr;


	oldaddr = _scriptPointer - 1;

	if (_opcode == 0xAE && _gameId == GID_INDY3_256) {
		_opcode = 2;
	} else
		_opcode = fetchScriptByte();

	switch (_opcode & 0x1F) {
	case 1:											/* wait for actor */
		if (derefActorSafe(getVarOrDirectByte(0x80), "o5_wait")->moving)
			break;
		return;
	case 2:											/* wait for message */
		if ((_currentRoom == 0) && (_gameId == GID_ZAK256))	// Bypass Zak256 script hang
			return;

		if (_vars[VAR_HAVE_MSG])
			break;
		return;
	case 3:											/* wait for camera */
		if (camera._cur.x >> 3 != camera._dest.x >> 3)
			break;
		return;
	case 4:											/* wait for sentence */
		if (_sentenceNum) {
			if (sentence[_sentenceNum - 1].unk &&
					!isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
				return;
			break;
		}
		if (!isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
			return;
		break;
	default:
		error("o5_wait: default case");
		return;
	}

	_scriptPointer = oldaddr;
	o5_breakHere();
}

void Scumm::o5_walkActorTo()
{
	int x, y;
	Actor *a;
	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_walkActorTo");
	x = getVarOrDirectWord(0x40);
	y = getVarOrDirectWord(0x20);
	startWalkActor(a, x, y, -1);
}

void Scumm::o5_walkActorToActor()
{
	int b, x, y;
	Actor *a, *a2;
	int nr;
	int nr2 = getVarOrDirectByte(0x80);
	a = derefActorSafe(nr2, "o5_walkActorToActor");
	if (!a)
		return;

	if (a->room != _currentRoom) {
		getVarOrDirectByte(0x40);
		fetchScriptByte();
		return;
	}

	nr = getVarOrDirectByte(0x40);
	if (nr == 106 && _gameId == GID_INDY4) {
		warning("Bypassing Indy4 bug");
		fetchScriptByte();
		return;
	}
	// warning("walk actor %d to actor %d", nr, nr2);
	a2 = derefActorSafe(nr, "o5_walkActorToActor(2)");
	if (!a2)
		return;

	if (a2->room != _currentRoom) {
		fetchScriptByte();
		return;
	}
	b = fetchScriptByte();				/* distance from actor */
	if (b == 0xFF) {
		b = a2->scalex * a->width / 0xFF;
		b = b + b / 2;
	}
	x = a2->x;
	y = a2->y;
	if (x < a->x)
		x += b;
	else
		x -= b;

	startWalkActor(a, x, y, -1);
}

void Scumm::o5_walkActorToObject()
{
	int obj;
	Actor *a;

	// warning("walk object to object");

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_walkActorToObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		getObjectXYPos(obj);
		startWalkActor(a, _xPos, _yPos, _dir);
	}
}

int Scumm::getWordVararg(int16 * ptr)
{
	int i;

	for (i = 0; i < 15; i++)
		ptr[i] = 0;

	i = 0;
	while ((_opcode = fetchScriptByte()) != 0xFF) {
		ptr[i++] = getVarOrDirectWord(0x80);
	}
	return i;
}

int Scumm::getVarOrDirectWord(byte mask)
{
	if (_opcode & mask)
		return readVar(fetchScriptWord());
	return (int16) fetchScriptWord();
}

int Scumm::getVarOrDirectByte(byte mask)
{
	if (_opcode & mask)
		return readVar(fetchScriptWord());
	return fetchScriptByte();
}

void Scumm::decodeParseString()
{
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

	string[textSlot].xpos = string[textSlot].t_xpos;
	string[textSlot].ypos = string[textSlot].t_ypos;
	string[textSlot].center = string[textSlot].t_center;
	string[textSlot].overhead = string[textSlot].t_overhead;
	string[textSlot].right = string[textSlot].t_right;
	string[textSlot].color = string[textSlot].t_color;
	string[textSlot].charset = string[textSlot].t_charset;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0xF) {
		case 0:										/* set string xy */
			string[textSlot].xpos = getVarOrDirectWord(0x80);
			string[textSlot].ypos = getVarOrDirectWord(0x40);
			string[textSlot].overhead = false;
			break;
		case 1:										/* color */
			string[textSlot].color = getVarOrDirectByte(0x80);
			break;
		case 2:										/* right */
			string[textSlot].right = getVarOrDirectWord(0x80);
			break;
		case 4:										/* center */
			string[textSlot].center = true;
			string[textSlot].overhead = false;
			break;
		case 6:										/* left */
			string[textSlot].center = false;
			string[textSlot].overhead = false;
			break;
		case 7:										/* overhead */
			string[textSlot].overhead = true;
			break;
		case 8:{										/* play loom talkie sound - use in other games ? */
				int x = getVarOrDirectWord(0x80);
				int offset;
				int delay;

				if (x != 0)
					offset = (int)((x & 0xffff) * 7.5 - 22650);
				else
					offset = 0;
				delay = (int)((getVarOrDirectWord(0x40) & 0xffff) * 7.5);
				if (_gameId == GID_LOOM256)
					_system->play_cdrom(1, 0, offset, delay);
				else
					warning("parseString: 8");
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
			_scriptPointer = _messagePtr;
			return;
		default:
			return;
		}
	}

	string[textSlot].t_xpos = string[textSlot].xpos;
	string[textSlot].t_ypos = string[textSlot].ypos;
	string[textSlot].t_center = string[textSlot].center;
	string[textSlot].t_overhead = string[textSlot].overhead;
	string[textSlot].t_right = string[textSlot].right;
	string[textSlot].t_color = string[textSlot].color;
	string[textSlot].t_charset = string[textSlot].charset;
}

void Scumm::o5_oldRoomEffect()
{
	int a;

	_opcode = fetchScriptByte();
	if ((_opcode & 0x1F) == 3) {
		a = getVarOrDirectWord(0x80);
	}
	warning("Unsupported oldRoomEffect");
}

void Scumm::o5_pickupObjectOld()
{
	int obj = getVarOrDirectWord(0x80);

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	/* Don't take an */
		return;									/* object twice */

	// warning("adding %d from %d to inventoryOld", obj, _currentRoom);
	addObjectToInventory(obj, _currentRoom);	
	removeObjectFromRoom(obj);
	putOwner(obj, _vars[VAR_EGO]);
	putClass(obj, 32, 1);
	putState(obj, 1);
	clearDrawObjectQueue();
	runHook(1);
}
