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

#include "twp/sqgame.h"
#include "twp/twp.h"
#include "twp/room.h"
#include "twp/object.h"
#include "twp/squtil.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

static SQInteger actorAlpha(HSQUIRRELVM v) {
	warning("TODO: actorAlpha not implemented");
	return 0;
}

static SQInteger actorAnimationFlags(HSQUIRRELVM v) {
	warning("TODO: actorAnimationFlags not implemented");
	return 0;
}

static SQInteger actorAnimationNames(HSQUIRRELVM v) {
	warning("TODO: actorAnimationNames not implemented");
	return 0;
}

static SQInteger actorAt(HSQUIRRELVM v) {
	warning("TODO: actorAt not implemented");
	return 0;
}

static SQInteger actorBlinkRate(HSQUIRRELVM v) {
	warning("TODO: actorBlinkRate not implemented");
	return 0;
}

static SQInteger actorColor(HSQUIRRELVM v) {
	warning("TODO: actorColor not implemented");
	return 0;
}

static SQInteger actorCostume(HSQUIRRELVM v) {
	warning("TODO: actorCostume not implemented");
	return 0;
}

static SQInteger actorDistanceTo(HSQUIRRELVM v) {
	warning("TODO: actorDistanceTo not implemented");
	return 0;
}

static SQInteger actorDistanceWithin(HSQUIRRELVM v) {
	warning("TODO: actorDistanceWithin not implemented");
	return 0;
}

static SQInteger actorFace(HSQUIRRELVM v) {
	warning("TODO: actorFace not implemented");
	return 0;
}

static SQInteger actorHidden(HSQUIRRELVM v) {
	warning("TODO: actorHidden not implemented");
	return 0;
}

static SQInteger actorInTrigger(HSQUIRRELVM v) {
	warning("TODO: actorInTrigger not implemented");
	return 0;
}

static SQInteger actorInWalkbox(HSQUIRRELVM v) {
	warning("TODO: actorInWalkbox not implemented");
	return 0;
}

static SQInteger actorRoom(HSQUIRRELVM v) {
	warning("TODO: actorRoom not implemented");
	return 0;
}

static SQInteger actorHideLayer(HSQUIRRELVM v) {
	warning("TODO: actorHideLayer not implemented");
	return 0;
}

static SQInteger actorShowLayer(HSQUIRRELVM v) {
	warning("TODO: actorShowLayer not implemented");
	return 0;
}

static SQInteger actorSlotSelectable(HSQUIRRELVM v) {
	warning("TODO: actorSlotSelectable not implemented");
	return 0;
}

static SQInteger actorLockFacing(HSQUIRRELVM v) {
	warning("TODO: actorLockFacing not implemented");
	return 0;
}

static SQInteger actorPosX(HSQUIRRELVM v) {
	warning("TODO: actorPosX not implemented");
	return 0;
}

static SQInteger actorPosY(HSQUIRRELVM v) {
	warning("TODO: actorPosY not implemented");
	return 0;
}

static SQInteger actorPlayAnimation(HSQUIRRELVM v) {
	warning("TODO: actorPlayAnimation not implemented");
	return 0;
}

static SQInteger actorRenderOffset(HSQUIRRELVM v) {
	warning("TODO: actorRenderOffset not implemented");
	return 0;
}

static SQInteger actorStand(HSQUIRRELVM v) {
	warning("TODO: actorStand not implemented");
	return 0;
}

static SQInteger actorStopWalking(HSQUIRRELVM v) {
	warning("TODO: actorStopWalking not implemented");
	return 0;
}

static SQInteger actorTalkColors(HSQUIRRELVM v) {
	warning("TODO: actorTalkColors not implemented");
	return 0;
}

static SQInteger actorTalking(HSQUIRRELVM v) {
	warning("TODO: actorTalking not implemented");
	return 0;
}

static SQInteger actorTurnTo(HSQUIRRELVM v) {
	warning("TODO: actorTurnTo not implemented");
	return 0;
}

static SQInteger actorTalkOffset(HSQUIRRELVM v) {
	warning("TODO: actorTalkOffset not implemented");
	return 0;
}

static SQInteger actorUsePos(HSQUIRRELVM v) {
	warning("TODO: actorUsePos not implemented");
	return 0;
}

static SQInteger actorUseWalkboxes(HSQUIRRELVM v) {
	warning("TODO: actorUseWalkboxes not implemented");
	return 0;
}

static SQInteger actorVolume(HSQUIRRELVM v) {
	warning("TODO: actorVolume not implemented");
	return 0;
}

static SQInteger actorWalkForward(HSQUIRRELVM v) {
	warning("TODO: actorWalkForward not implemented");
	return 0;
}

static SQInteger actorWalking(HSQUIRRELVM v) {
	warning("TODO: actorWalking not implemented");
	return 0;
}

static SQInteger actorWalkSpeed(HSQUIRRELVM v) {
	warning("TODO: actorWalkSpeed not implemented");
	return 0;
}

static SQInteger actorWalkTo(HSQUIRRELVM v) {
	warning("TODO: actorWalkTo not implemented");
	return 0;
}

static SQInteger addSelectableActor(HSQUIRRELVM v) {
	warning("TODO: addSelectableActor not implemented");
	return 0;
}

static SQInteger createActor(HSQUIRRELVM v) {
	warning("TODO: createActor not implemented");
	return 0;
}

static SQInteger flashSelectableActor(HSQUIRRELVM v) {
	warning("TODO: flashSelectableActor not implemented");
	return 0;
}

static SQInteger sayLine(HSQUIRRELVM v) {
	warning("TODO: sayLine not implemented");
	return 0;
}

static SQInteger sayLineAt(HSQUIRRELVM v) {
	warning("TODO: sayLineAt not implemented");
	return 0;
}

static SQInteger isActorOnScreen(HSQUIRRELVM v) {
	warning("TODO: isActorOnScreen not implemented");
	return 0;
}

static SQInteger isActorSelectable(HSQUIRRELVM v) {
	warning("TODO: isActorSelectable not implemented");
	return 0;
}

static SQInteger is_actor(HSQUIRRELVM v) {
	warning("TODO: is_actor not implemented");
	return 0;
}

static SQInteger masterActorArray(HSQUIRRELVM v) {
	warning("TODO: masterActorArray not implemented");
	return 0;
}

static SQInteger mumbleLine(HSQUIRRELVM v) {
	warning("TODO: mumbleLine not implemented");
	return 0;
}

static SQInteger stopTalking(HSQUIRRELVM v) {
	warning("TODO: stopTalking not implemented");
	return 0;
}

static SQInteger selectActor(HSQUIRRELVM v) {
	warning("TODO: selectActor not implemented");
	return 0;
}

static SQInteger triggerActors(HSQUIRRELVM v) {
	warning("TODO: triggerActors not implemented");
	return 0;
}

static SQInteger verbUIColors(HSQUIRRELVM v) {
	warning("TODO: verbUIColors not implemented");
	return 0;
}

void sqgame_register_actorlib(HSQUIRRELVM v) {
  regFunc(v, actorAnimationFlags, "actorAnimationFlags");
  regFunc(v, actorAnimationNames, "actorAnimationNames");
  regFunc(v, actorAlpha, "actorAlpha");
  regFunc(v, actorAt, "actorAt");
  regFunc(v, actorBlinkRate, "actorBlinkRate");
  regFunc(v, actorColor, "actorColor");
  regFunc(v, actorCostume, "actorCostume");
  regFunc(v, actorDistanceTo, "actorDistanceTo");
  regFunc(v, actorDistanceWithin, "actorDistanceWithin");
  regFunc(v, actorFace, "actorFace");
  regFunc(v, actorHidden, "actorHidden");
  regFunc(v, actorHideLayer, "actorHideLayer");
  regFunc(v, actorInTrigger, "actorInTrigger");
  regFunc(v, actorInWalkbox, "actorInWalkbox");
  regFunc(v, actorLockFacing, "actorLockFacing");
  regFunc(v, actorPlayAnimation, "actorPlayAnimation");
  regFunc(v, actorPosX, "actorPosX");
  regFunc(v, actorPosY, "actorPosY");
  regFunc(v, actorRenderOffset, "actorRenderOffset");
  regFunc(v, actorRoom, "actorRoom");
  regFunc(v, actorShowLayer, "actorShowLayer");
  regFunc(v, actorSlotSelectable, "actorSlotSelectable");
  regFunc(v, actorStand, "actorStand");
  regFunc(v, actorStopWalking, "actorStopWalking");
  regFunc(v, actorTalkColors, "actorTalkColors");
  regFunc(v, actorTalking, "actorTalking");
  regFunc(v, actorTalkOffset, "actorTalkOffset");
  regFunc(v, actorTurnTo, "actorTurnTo");
  regFunc(v, actorUsePos, "actorUsePos");
  regFunc(v, actorUseWalkboxes, "actorUseWalkboxes");
  regFunc(v, actorVolume, "actorVolume");
  regFunc(v, actorWalking, "actorWalking");
  regFunc(v, actorWalkForward, "actorWalkForward");
  regFunc(v, actorWalkSpeed, "actorWalkSpeed");
  regFunc(v, actorWalkTo, "actorWalkTo");
  regFunc(v, addSelectableActor, "addSelectableActor");
  regFunc(v, createActor, "createActor");
  regFunc(v, flashSelectableActor, "flashSelectableActor");
  regFunc(v, is_actor, "is_actor");
  regFunc(v, isActorOnScreen, "isActorOnScreen");
  regFunc(v, isActorSelectable, "isActorSelectable");
  regFunc(v, mumbleLine, "mumbleLine");
  regFunc(v, masterActorArray, "masterActorArray");
  regFunc(v, sayLine, "sayLine");
  regFunc(v, sayLineAt, "sayLineAt");
  regFunc(v, selectActor, "selectActor");
  regFunc(v, stopTalking, "stopTalking");
  regFunc(v, triggerActors, "triggerActors");
  regFunc(v, verbUIColors, "verbUIColors");
}
} // namespace Twp
