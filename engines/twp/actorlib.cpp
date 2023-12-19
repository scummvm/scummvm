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
#include "twp/util.h"
#include "twp/scenegraph.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

// Sets the transparency for an actor's image in [0.0..1.0]
static SQInteger actorAlpha(HSQUIRRELVM v) {
	Object *actor = sqobj(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	float alpha;
	if (SQ_FAILED(sqget(v, 3, alpha)))
		return sq_throwerror(v, "failed to get alpha");
	debug("actorAlpha(%s, %f)", actor->_key.c_str(), alpha);
	actor->_node->setAlpha(alpha);
	return 0;
}

static SQInteger actorAnimationFlags(HSQUIRRELVM v) {
	Object *actor = sqobj(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	sqpush(v, actor->_animFlags);
	return 1;
}

static SQInteger actorAnimationNames(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");

	HSQOBJECT table;
	if (SQ_FAILED(sqget(v, 3, table)))
		return sq_throwerror(v, "failed to get table");
	if (!sq_istable(table))
		return sq_throwerror(v, "failed to get animation table");

	Common::String head;
	Common::String stand;
	Common::String walk;
	Common::String reach;
	sqgetf(table, "head", head);
	sqgetf(table, "stand", stand);
	sqgetf(table, "walk", walk);
	sqgetf(table, "reach", reach);
	actor->setAnimationNames(head, stand, walk, reach);
	return 0;
}

// Moves the specified actor to the room and x, y coordinates specified.
// Also makes the actor face to given direction (options are: FACE_FRONT, FACE_BACK, FACE_LEFT, FACE_RIGHT).
// If using a spot, moves the player to the spot as specified in a Wimpy file.
static SQInteger actorAt(HSQUIRRELVM v) {
	SQInteger numArgs = sq_gettop(v);
	switch (numArgs) {
	case 3: {
		Object *actor = sqactor(v, 2);
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
		Object *spot = sqobj(v, 3);
		if (spot) {
			Math::Vector2d pos = spot->_node->getPos() + spot->_usePos;
			actor->setRoom(spot->_room);
			actor->stopWalking();
			debug("actorAt %s at %s, room '%s'", actor->_key.c_str(), spot->_key.c_str(), spot->_room->_name.c_str());
			actor->_node->setPos(pos);
			actor->setFacing(getFacing(spot->_useDir, actor->getFacing()));
		} else {
			Room *room = sqroom(v, 3);
			if (!room)
				return sq_throwerror(v, "failed to get spot or room");
			debug("actorAt %s room '%s'", actor->_key.c_str(), room->_name.c_str());
			actor->stopWalking();
			actor->setRoom(room);
		}
		return 0;
	}
	case 4: {
		Object *actor = sqactor(v, 2);
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
		int x, y;
		if (SQ_FAILED(sqget(v, 3, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 4, y)))
			return sq_throwerror(v, "failed to get y");
		debug("actorAt %s room %d, %d", actor->_key.c_str(), x, y);
		actor->stopWalking();
		actor->_node->setPos(Math::Vector2d(x, y));
		return 0;
	}
	case 5:
	case 6: {
		Object *actor = sqactor(v, 2);
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
		Room *room = sqroom(v, 3);
		if (!room)
			return sq_throwerror(v, "failed to get room");
		int x, y;
		if (SQ_FAILED(sqget(v, 4, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 5, y)))
			return sq_throwerror(v, "failed to get y");
		int dir = 0;
		if ((numArgs == 6) && SQ_FAILED(sqget(v, 6, dir)))
			return sq_throwerror(v, "failed to get direction");
		debug("actorAt %s, pos = (%d,%d), dir = %d", actor->_key.c_str(), x, y, dir);
		actor->stopWalking();
		actor->_node->setPos(Math::Vector2d(x, y));
		actor->setFacing(getFacing(dir, actor->getFacing()));
		actor->setRoom(room);
		return 0;
	}
	default:
		return sq_throwerror(v, "invalid number of arguments");
	}
}

static SQInteger actorBlinkRate(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	float min;
	if (SQ_FAILED(sqget(v, 3, min)))
		return sq_throwerror(v, "failed to get min");
	float max;
	if (SQ_FAILED(sqget(v, 4, max)))
		return sq_throwerror(v, "failed to get max");
	actor->blinkRate(min, max);
	return 0;
}

// Adjusts the colour of the actor.
//
// . code-block:: Squirrel
// actorColor(coroner, 0xc0c0c0)
static SQInteger actorColor(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger c;
	if (SQ_FAILED(sq_getinteger(v, 3, &c)))
		return sq_throwerror(v, "failed to get color");
	actor->_node->setColor(Color::rgb(c));
	return 0;
}

// Sets the actor's costume to the (JSON) filename animation file.
// If the actor is expected to preform the standard walk, talk, stand, reach animations, they need to exist in the file.
// If a sheet is given, this is a sprite sheet containing all the images needed for the animation.
static SQInteger actorCostume(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");

	Common::String name;
	if (SQ_FAILED(sqget(v, 3, name)))
		return sq_throwerror(v, "failed to get name");

	Common::String sheet;
	if (sq_gettop(v) == 4)
		sqget(v, 4, sheet);
	debug("Actor costume %s %s", name.c_str(), sheet.c_str());
	actor->setCostume(name, sheet);
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

// Makes the actor face a given direction.
// Directions are: FACE_FRONT, FACE_BACK, FACE_LEFT, FACE_RIGHT.
// Similar to actorTurnTo, but will not animate the change, it will instantly be in the specified direction.
static SQInteger actorFace(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger nArgs = sq_gettop(v);
	if (nArgs == 2) {
		Facing dir = actor->getFacing();
		sqpush(v, (int)dir);
		return 1;
	}

	if (sq_gettype(v, 3) == OT_INTEGER) {
		int dir = 0;
		if (SQ_FAILED(sqget(v, 3, dir)))
			return sq_throwerror(v, "failed to get direction");
		// FACE_FLIP ?
		if (dir == 0x10) {
			Facing facing = flip(actor->getFacing());
			actor->setFacing(facing);
		} else {
			actor->setFacing((Facing)dir);
		}
	} else {
		Object *actor2 = sqactor(v, 3);
		if (!actor2)
			return sq_throwerror(v, "failed to get actor to face to");
		Facing facing = getFacingToFaceTo(actor, actor2);
		actor->setFacing(facing);
	}
	return 0;
}

static SQInteger actorHidden(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	int hidden = 0;
	if (SQ_FAILED(sqget(v, 3, hidden)))
		return sq_throwerror(v, "failed to get hidden");
	if (hidden && (g_engine->_actor == actor)) {
		g_engine->follow(nullptr);
	}
	actor->_node->setVisible(hidden == 0);
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
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Room *room = actor->_room;
	if (!room) {
		sq_pushnull(v);
	} else {
		sqpush(v, room->_table);
	}
	return 1;
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

// Creates a new actor from a table.
//
// An actor is defined in the DefineActors.nut file.
static SQInteger createActor(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) != OT_TABLE)
		return sq_throwerror(v, "failed to get a table");

	HSQUIRRELVM vm = g_engine->getVm();
	Object *actor = Object::createActor();
	sq_resetobject(&actor->_table);
	sq_getstackobj(v, 2, &actor->_table);
	sq_addref(vm, &actor->_table);
	setId(actor->_table, newActorId());

	Common::String key;
	sqgetf(actor->_table, "_key", key);
	actor->_key = key;

	debug("Create actor %s %d", key.c_str(), actor->getId());
	actor->_node = new ActorNode(actor);
	actor->_nodeAnim = new Anim(actor);
	actor->_node->addChild(actor->_nodeAnim);
	g_engine->_actors.push_back(actor);

	sq_pushobject(v, actor->_table);
	return 1;
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

// returns true if the specified actor is currently in the screen.
static SQInteger isActorOnScreen(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get actor/object");

	if (obj->_room != g_engine->_room) {
		sqpush(v, false);
	} else {
		Math::Vector2d pos = obj->_node->getPos() - g_engine->getGfx().cameraPos();
		Math::Vector2d size = g_engine->getGfx().camera();
		bool isOnScreen = Common::Rect(0.0f, 0.0f, size.getX(), size.getY()).contains(pos.getX(), pos.getY());
		sqpush(v, isOnScreen);
	}
	return 1;
}

static SQInteger isActorSelectable(HSQUIRRELVM v) {
	warning("TODO: isActorSelectable not implemented");
	return 0;
}

// If an actor is specified, returns true otherwise returns false.
static SQInteger is_actor(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	sqpush(v, actor != nullptr);
	return 1;
}

// Returns an array with every single actor that has been defined in the game so far, including non-player characters.
// See also masterRoomArray.
static SQInteger masterActorArray(HSQUIRRELVM v) {
	sq_newarray(v, 0);
	for (int i = 0; i < g_engine->_actors.size(); i++) {
		Object *actor = g_engine->_actors[i];
		sqpush(v, actor->_table);
		sq_arrayappend(v, -2);
	}
	return 1;
}

static SQInteger mumbleLine(HSQUIRRELVM v) {
	warning("TODO: mumbleLine not implemented");
	return 0;
}

static SQInteger stopTalking(HSQUIRRELVM v) {
	warning("TODO: stopTalking not implemented");
	return 0;
}

// Causes the actor to become the selected actor.
// If they are in the same room as the last selected actor the camera will pan over to them.
// If they are in a different room, the camera will cut to the new room.
// The UI will change to reflect the new actor and their inventory.
static SQInteger selectActor(HSQUIRRELVM v) {
	g_engine->setActor(sqobj(v, 2));
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
