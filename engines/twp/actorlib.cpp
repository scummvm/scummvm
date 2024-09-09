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

#include "twp/twp.h"
#include "twp/detection.h"
#include "twp/hud.h"
#include "twp/object.h"
#include "twp/resmanager.h"
#include "twp/room.h"
#include "twp/sqgame.h"
#include "twp/squtil.h"

namespace Twp {

// Sets the transparency for an actor's image in [0.0..1.0]
static SQInteger actorAlpha(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqobj(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	float alpha;
	if (SQ_FAILED(sqget(v, 3, alpha)))
		return sq_throwerror(v, "failed to get alpha");
	debugC(kDebugActScript, "actorAlpha(%s, %f)", actor->_key.c_str(), alpha);
	actor->_node->setAlpha(alpha);
	return 0;
}

static SQInteger actorAnimationFlags(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqobj(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	sqpush(v, actor->_animFlags);
	return 1;
}

static SQInteger actorAnimationNames(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
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
	if (sqrawexists(table, "head") && SQ_FAILED(sqgetf(table, "head", head)))
		return sq_throwerror(v, "failed to get head");
	if (sqrawexists(table, "stand") && SQ_FAILED(sqgetf(table, "stand", stand)))
		return sq_throwerror(v, "failed to get stand");
	if (sqrawexists(table, "walk") && SQ_FAILED(sqgetf(table, "walk", walk)))
		return sq_throwerror(v, "failed to get walk");
	if (sqrawexists(table, "reach") && SQ_FAILED(sqgetf(table, "reach", reach)))
		return sq_throwerror(v, "failed to get reach");
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
		Common::SharedPtr<Object> actor = sqactor(v, 2);
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
		Common::SharedPtr<Object> spot = sqobj(v, 3);
		if (spot) {
			Math::Vector2d pos(spot->_node->getPos() + spot->_usePos);
			Object::setRoom(actor, spot->_room);
			actor->stopWalking();
			debugC(kDebugActScript, "actorAt %s at %s (%d, %d), room '%s'", actor->_key.c_str(), spot->_key.c_str(), (int)pos.getX(), (int)pos.getY(), spot->_room->_name.c_str());
			actor->_node->setPos(pos);
			actor->setFacing(getFacing(spot->_useDir, actor->getFacing()));
		} else {
			Common::SharedPtr<Room> room = sqroom(v, 3);
			if (!room)
				return sq_throwerror(v, "failed to get spot or room");
			debugC(kDebugActScript, "actorAt %s room '%s'", actor->_key.c_str(), room->_name.c_str());
			actor->stopWalking();
			Object::setRoom(actor, room);
		}
		return 0;
	}
	case 4: {
		Common::SharedPtr<Object> actor = sqactor(v, 2);
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
		SQInteger x, y;
		if (SQ_FAILED(sqget(v, 3, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 4, y)))
			return sq_throwerror(v, "failed to get y");
		debugC(kDebugActScript, "actorAt %s room %lld, %lld", actor->_key.c_str(), x, y);
		actor->stopWalking();
		actor->_node->setPos(Math::Vector2d(x, y));
		return 0;
	}
	case 5:
	case 6: {
		Common::SharedPtr<Object> actor = sqactor(v, 2);
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
		Common::SharedPtr<Room> room = sqroom(v, 3);
		if (!room)
			return sq_throwerror(v, "failed to get room");
		SQInteger x, y;
		if (SQ_FAILED(sqget(v, 4, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 5, y)))
			return sq_throwerror(v, "failed to get y");
		SQInteger dir = 0;
		if ((numArgs == 6) && SQ_FAILED(sqget(v, 6, dir)))
			return sq_throwerror(v, "failed to get direction");
		debugC(kDebugActScript, "actorAt %s, pos = (%lld,%lld), dir = %lld", actor->_key.c_str(), x, y, dir);
		actor->stopWalking();
		actor->_node->setPos(Math::Vector2d(x, y));
		actor->setFacing(getFacing(dir, actor->getFacing()));
		Object::setRoom(actor, room);
		return 0;
	}
	default:
		return sq_throwerror(v, "invalid number of arguments");
	}
}

static SQInteger actorBlinkRate(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	float min;
	if (SQ_FAILED(sqget(v, 3, min)))
		return sq_throwerror(v, "failed to get min");
	float max;
	if (SQ_FAILED(sqget(v, 4, max)))
		return sq_throwerror(v, "failed to get max");
	Object::blinkRate(actor, min, max);
	return 0;
}

// Adjusts the colour of the actor.
//
// . code-block:: Squirrel
// actorColor(coroner, 0xc0c0c0)
static SQInteger actorColor(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger c;
	if (SQ_FAILED(sqget(v, 3, c)))
		return sq_throwerror(v, "failed to get color");
	actor->_node->setColor(Color::fromRgba(c));
	return 0;
}

// Sets the actor's costume to the (JSON) filename animation file.
// If the actor is expected to perform the standard walk, talk, stand, reach animations, they need to exist in the file.
// If a sheet is given, this is a sprite sheet containing all the images needed for the animation.
static SQInteger actorCostume(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");

	Common::String name;
	if (SQ_FAILED(sqget(v, 3, name)))
		return sq_throwerror(v, "failed to get name");

	Common::String sheet;
	if ((sq_gettop(v) == 4) && SQ_FAILED(sqget(v, 4, sheet)))
		return sq_throwerror(v, "failed to get sheet");
	debugC(kDebugActScript, "Actor costume %s %s", name.c_str(), sheet.c_str());
	actor->setCostume(name, sheet);
	return 0;
}

static SQInteger actorDistanceTo(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::SharedPtr<Object> obj;
	if (sq_gettop(v) == 3) {
		obj = sqobj(v, 3);
		if (!obj)
			return sq_throwerror(v, "failed to get object");
	} else {
		obj = g_twp->_actor;
	}
	sqpush(v, distance(actor->_node->getPos(), obj->getUsePos()));
	return 1;
}

static SQInteger actorDistanceWithin(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	if (nArgs == 3) {
		Common::SharedPtr<Object> actor1(g_twp->_actor);
		Common::SharedPtr<Object> actor2(sqactor(v, 2));
		if (!actor2)
			return sq_throwerror(v, "failed to get actor");
		Common::SharedPtr<Object> obj = sqobj(v, 3);
		if (!obj)
			return sq_throwerror(v, "failed to get spot");
		if (actor1->_room != actor2->_room)
			return false;
		// not sure about this, needs to be check one day ;)
		sqpush(v, distance(actor1->_node->getAbsPos(), obj->getUsePos()) < distance(actor2->_node->getAbsPos(), obj->getUsePos()));
		return 1;
	}

	if (nArgs == 4) {
		Common::SharedPtr<Object> actor(sqactor(v, 2));
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
		Common::SharedPtr<Object> obj(sqobj(v, 3));
		if (!obj)
			return sq_throwerror(v, "failed to get object");
		SQInteger dist;
		if (SQ_FAILED(sqget(v, 4, dist)))
			return sq_throwerror(v, "failed to get distance");
		if (actor->_room != obj->_room)
			return false;
		sqpush(v, distance(actor->_node->getAbsPos(), obj->getUsePos()) < dist);
		return 1;
	}
	return sq_throwerror(v, "actorDistanceWithin not implemented");
}

// Makes the actor face a given direction.
// Directions are: FACE_FRONT, FACE_BACK, FACE_LEFT, FACE_RIGHT.
// Similar to actorTurnTo, but will not animate the change, it will instantly be in the specified direction.
static SQInteger actorFace(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger nArgs = sq_gettop(v);
	if (nArgs == 2) {
		Facing dir = actor->getFacing();
		sqpush(v, (int)dir);
		return 1;
	}

	if (sq_gettype(v, 3) == OT_INTEGER) {
		SQInteger dir = 0;
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
		Common::SharedPtr<Object> actor2 = sqactor(v, 3);
		if (!actor2)
			return sq_throwerror(v, "failed to get actor to face to");
		Facing facing = getFacingToFaceTo(actor, actor2);
		actor->setFacing(facing);
	}
	return 0;
}

static SQInteger actorHidden(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger hidden = 0;
	if (SQ_FAILED(sqget(v, 3, hidden)))
		return sq_throwerror(v, "failed to get hidden");
	if (hidden && (g_twp->_actor == actor)) {
		g_twp->follow(nullptr);
	}
	actor->_node->setVisible(hidden == 0);
	return 0;
}

// Returns an array of all the actors that are currently within a specified trigger box.
//
// . code-block:: Squirrel
// local stepsArray = triggerActors(AStreet.bookStoreLampTrigger)
// if (stepsArray.len()) {    // someone's on the steps
// }
static SQInteger actorInTrigger(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::SharedPtr<Object> obj = sqobj(v, 3);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	bool inside = obj->contains(actor->_node->getAbsPos());
	sqpush(v, inside);
	return 1;
}

// Returns true if the specified actor is inside the specified walkbox from the wimpy file.
//
// . code-block:: Squirrel
// sheriffsOfficeJailDoor =
// {
//     name = "jail door"
//     actorInWalkbox(currentActor, "jail")
//     verbOpen = function()
//     {
//         if (jail_door_state == OPEN) {
//             sayLine("The door is already open.")
//         } else {
//             if (actorInWalkbox(currentActor, "jail")) {
//                 sayLine("I can't open it from in here.")
//                 return
//             } else {
//                startthread(openJailDoor)
//             }
//         }
//     }
// }
static SQInteger actorInWalkbox(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::String name;
	if (SQ_FAILED(sqget(v, 3, name)))
		return sq_throwerror(v, "failed to get name");
	for (const auto &walkbox : g_twp->_room->_walkboxes) {
		if (walkbox._name == name) {
			if (walkbox.contains(actor->_node->getAbsPos())) {
				sqpush(v, true);
				return 1;
			}
		}
	}
	sqpush(v, false);
	return 1;
}

static SQInteger actorRoom(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::SharedPtr<Room> room = actor->_room;
	if (!room) {
		sq_pushnull(v);
	} else {
		sqpush(v, room->_table);
	}
	return 1;
}

static SQInteger actorShowHideLayer(HSQUIRRELVM v, bool visible) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::String layer;
	if (SQ_FAILED(sqget(v, 3, layer)))
		return sq_throwerror(v, "failed to get layer");
	actor->showLayer(layer, visible);
	if (!visible) {
		for (int i = 1; i <= 6; i++) {
			actor->showLayer(Common::String::format("%s%d", layer.c_str(), i), false);
		}
	}
	return 0;
}

static SQInteger actorHideLayer(HSQUIRRELVM v) {
	return actorShowHideLayer(v, false);
}

static SQInteger actorShowLayer(HSQUIRRELVM v) {
	return actorShowHideLayer(v, true);
}

static SQInteger actorSlotSelectable(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	switch (nArgs) {
	case 2: {
		SQInteger selectable;
		if (SQ_FAILED(sqget(v, 2, selectable)))
			return sq_throwerror(v, "failed to get selectable");
		switch (selectable) {
		case OFF:
			g_twp->_actorSwitcher._mode &= (~asOn);
			break;
		case ON:
			g_twp->_actorSwitcher._mode |= asOn;
			break;
		case TEMP_UNSELECTABLE:
			g_twp->_actorSwitcher._mode |= asTemporaryUnselectable;
			break;
		case TEMP_SELECTABLE:
			g_twp->_actorSwitcher._mode &= ~asTemporaryUnselectable;
			break;
		default:
			return sq_throwerror(v, "invalid selectable value");
		}
		return 0;
	}
	case 3: {
		bool selectable;
		if (SQ_FAILED(sqget(v, 3, selectable)))
			return sq_throwerror(v, "failed to get selectable");
		if (sq_gettype(v, 2) == OT_INTEGER) {
			SQInteger slot;
			if (SQ_FAILED(sqget(v, 2, slot)))
				return sq_throwerror(v, "failed to get slot");
			g_twp->_hud->_actorSlots[slot - 1].selectable = selectable;
		} else {
			Common::SharedPtr<Object> actor = sqactor(v, 2);
			if (!actor)
				return sq_throwerror(v, "failed to get actor");
			Common::String key;
			if (SQ_FAILED(sqgetf(actor->_table, "_key", key)))
				return sq_throwerror(v, "failed to get actor key");
			debugC(kDebugActScript, "actorSlotSelectable(%s, %s)", key.c_str(), selectable ? "yes" : "no");
			ActorSlot *slot = g_twp->_hud->actorSlot(actor);
			if (!slot)
				warning("slot for actor %s not found", key.c_str());
			else
				slot->selectable = selectable;
		}
		return 0;
	}
	default:
		return sq_throwerror(v, "invalid number of arguments");
	}
}

// If a direction is specified: makes the actor face a given direction, which cannot be changed no matter what the player does.
// Directions are: FACE_FRONT, FACE_BACK, FACE_LEFT, FACE_RIGHT.
// If "NO" is specified, it removes all locking and allows the actor to change its facing direction based on player input or otherwise.
static SQInteger actorLockFacing(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	switch (sq_gettype(v, 3)) {
	case OT_INTEGER: {
		SQInteger facing = 0;
		if (SQ_FAILED(sqget(v, 3, facing)))
			return sq_throwerror(v, "failed to get facing");
		if (facing == 0)
			actor->resetLockFacing();
		else
			actor->lockFacing(facing);
	} break;
	case OT_TABLE: {
		HSQOBJECT obj;
		SQInteger back = static_cast<SQInteger>(Facing::FACE_BACK);
		SQInteger front = static_cast<SQInteger>(Facing::FACE_FRONT);
		SQInteger left = static_cast<SQInteger>(Facing::FACE_LEFT);
		SQInteger right = static_cast<SQInteger>(Facing::FACE_RIGHT);
		SQInteger reset = 0;
		sq_getstackobj(v, 3, &obj);
		if (sqrawexists(obj, "back") && SQ_FAILED(sqgetf(v, obj, "back", back)))
			return sq_throwerror(v, "failed to get verb back");
		if (sqrawexists(obj, "front") && SQ_FAILED(sqgetf(v, obj, "front", front)))
			return sq_throwerror(v, "failed to get verb front");
		if (sqrawexists(obj, "left") && SQ_FAILED(sqgetf(v, obj, "left", left)))
			return sq_throwerror(v, "failed to get verb left");
		if (sqrawexists(obj, "right") && SQ_FAILED(sqgetf(v, obj, "right", right)))
			return sq_throwerror(v, "failed to get verb right");
		if (sqrawexists(obj, "reset") && SQ_FAILED(sqgetf(v, obj, "reset", reset)))
			return sq_throwerror(v, "failed to get verb reset");
		if (reset != 0)
			actor->resetLockFacing();
		else
			actor->lockFacing((Facing)left, (Facing)right, (Facing)front, (Facing)back);
	} break;
	default:
		return sq_throwerror(v, "unknown facing type");
	}
	return 0;
}

static SQInteger actorPosX(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	sqpush(v, actor->_node->getPos().getX());
	return 1;
}

static SQInteger actorPosY(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	sqpush(v, actor->_node->getPos().getY());
	return 1;
}

// Plays the specified animation from the player's costume JSON filename.
// If YES loop the animation. Default is NO.
static SQInteger actorPlayAnimation(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::String animation;
	if (SQ_FAILED(sqget(v, 3, animation)))
		return sq_throwerror(v, "failed to get animation");
	SQInteger loop = 0;
	if ((sq_gettop(v) >= 4) && (SQ_FAILED(sqget(v, 4, loop))))
		return sq_throwerror(v, "failed to get loop");
	debugC(kDebugActScript, "Play anim %s %s loop=%s", actor->_key.c_str(), animation.c_str(), loop ? "yes" : "no");
	actor->play(animation, loop != 0);
	return 0;
}

// Sets the rendering offset of the actor to x and y.
//
// A rendering offset of 0,0 would cause them to be rendered from the middle of their image.
// Actor's are typically adjusted so they are rendered from the middle of the bottom of their feet.
// To maintain sanity, it is best if all actors have the same image size and are all adjust the same, but this is not a requirement.
static SQInteger actorRenderOffset(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger x, y;
	if (SQ_FAILED(sqget(v, 3, x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sqget(v, 4, y)))
		return sq_throwerror(v, "failed to get y");
	actor->_node->setRenderOffset(Math::Vector2d(x, y));
	return 0;
}

static SQInteger actorStand(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	actor->stand();
	return 0;
}

// Makes the specified actor stop moving immediately.
//
// . code-block:: Squirrel
// actorStopWalking(currentActor)
// actorStopWalking(postalworker)
static SQInteger actorStopWalking(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	actor->stopWalking();
	actor->stand();
	return 0;
}

// Set the text color of the specified actor's text that appears when they speak.
static SQInteger actorTalkColors(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqobj(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger color;
	if (SQ_FAILED(sqget(v, 3, color)))
		return sq_throwerror(v, "failed to get talk color");
	actor->_talkColor = Color::rgb(color);
	return 0;
}

// If an actor is specified, returns true if that actor is currently talking.
// If no actor is specified, returns true if the player's current actor is currently talking.
//
// . code-block:: Squirrel
// actorTalking()
// actorTalking(vo)
static SQInteger actorTalking(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor;
	if (sq_gettop(v) == 2) {
		actor = sqobj(v, 2);
	} else {
		actor = g_twp->_actor;
	}
	bool isTalking = actor && actor->getTalking() && actor->getTalking()->isEnabled();
	sqpush(v, isTalking);
	return 1;
}

// Turn to the pos, dir, object or actor over 2 frames.
static SQInteger actorTurnTo(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	if (sq_gettype(v, 3) == OT_INTEGER) {
		SQInteger facing = 0;
		if (SQ_FAILED(sqget(v, 3, facing)))
			return sq_throwerror(v, "failed to get facing");
		actor->turn((Facing)facing);
	} else {
		Common::SharedPtr<Object> obj = sqobj(v, 3);
		if (!obj)
			return sq_throwerror(v, "failed to get object to face to");
		Object::turn(actor, obj);
	}
	return 0;
}

// Specifies the offset that will be applied to the actor's speech text that appears on screen.
static SQInteger actorTalkOffset(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqobj(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger x, y;
	if (SQ_FAILED(sqget(v, 3, x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sqget(v, 4, y)))
		return sq_throwerror(v, "failed to get y");
	actor->_talkOffset = Math::Vector2d(x, y);
	return 0;
}

static SQInteger actorUsePos(HSQUIRRELVM v) {
	Math::Vector2d usePos;
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::SharedPtr<Object> obj = sqobj(v, 3);
	if (!obj)
		usePos = Math::Vector2d();
	else
		usePos = obj->_usePos;
	if (sq_gettop(v) == 4) {
		SQInteger dir;
		if (SQ_FAILED(sqget(v, 4, dir)))
			return sq_throwerror(v, "failed to get direction");
		else
			actor->_useDir = (Direction)dir;
	}
	actor->_usePos = Common::move(usePos);
	return 0;
}

// Specifies whether the actor needs to abide by walkboxes or not.
//
// . code-block:: Squirrel
// actorUseWalkboxes(coroner, NO)
static SQInteger actorUseWalkboxes(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger useWalkboxes = 1;
	if (SQ_FAILED(sqget(v, 3, useWalkboxes)))
		return sq_throwerror(v, "failed to get useWalkboxes");
	actor->_useWalkboxes = useWalkboxes != 0;
	return 0;
}

static SQInteger actorVolume(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	float volume = 0.0f;
	if (SQ_FAILED(sqget(v, 3, volume)))
		return sq_throwerror(v, "failed to get volume");
	actor->_volume = volume;
	return 0;
}

// Gets the specified actor to walk forward the distance specified.
//
// . code-block:: Squirrel
// script sheriffOpening2() {
//     cutscene(@() {
//         actorAt(sheriff, CityHall.spot1)
//         actorWalkForward(currentActor, 50)
//         ...
//     }
// }
static SQInteger actorWalkForward(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger dist;
	if (SQ_FAILED(sqget(v, 3, dist)))
		return sq_throwerror(v, "failed to get dist");
	Math::Vector2d dir;
	switch (actor->getFacing()) {
	case Facing::FACE_FRONT:
		dir = Math::Vector2d(0, -dist);
		break;
	case Facing::FACE_BACK:
		dir = Math::Vector2d(0, dist);
		break;
	case Facing::FACE_LEFT:
		dir = Math::Vector2d(-dist, 0);
		break;
	case Facing::FACE_RIGHT:
		dir = Math::Vector2d(dist, 0);
		break;
	}
	Object::walk(actor, (actor->_node->getAbsPos() + dir));
	return 0;
}

// Returns true if the specified actor is currently walking.
// If no actor is specified, then returns true if the current player character is walking.
//
// . code-block:: Squirrel
// script _startWriting() {
//    if (!actorWalking(this)) {
//        if (notebookOpen == NO) {
//            actorPlayAnimation(reyes, "start_writing", NO)
//            breaktime(0.30)
//        }
//        ...
//    }
//}
static SQInteger actorWalking(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	Common::SharedPtr<Object> actor;
	if (nArgs == 1) {
		actor = g_twp->_actor;
	} else if (nArgs == 2) {
		actor = sqactor(v, 2);
	}
	sqpush(v, actor && actor->isWalking());
	return 1;
}

// Sets the walk speed of an actor.
//
// The numbers are in pixel's per second.
// The vertical movement is typically half (or more) than the horizontal movement to simulate depth in the 2D world.
static SQInteger actorWalkSpeed(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	SQInteger x, y;
	if (SQ_FAILED(sqget(v, 3, x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sqget(v, 4, y)))
		return sq_throwerror(v, "failed to get y");
	actor->_walkSpeed = Math::Vector2d(x, y);
	return 0;
}

// Tells the specified actor to walk to an x/y position or to an actor position or to an object position.
static SQInteger actorWalkTo(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	if (nArgs == 3) {
		Common::SharedPtr<Object> obj = sqobj(v, 3);
		if (!obj)
			return sq_throwerror(v, "failed to get actor or object");
		else
			Object::walk(actor, obj);
	} else if ((nArgs == 4) || (nArgs == 5)) {
		SQInteger x, y;
		if (SQ_FAILED(sqget(v, 3, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 4, y)))
			return sq_throwerror(v, "failed to get y");
		SQInteger facing = 0;
		if (nArgs == 5) {
			if (SQ_FAILED(sqget(v, 5, facing)))
				return sq_throwerror(v, "failed to get dir");
		}
		Object::walk(actor, Math::Vector2d(x, y), facing);
	} else {
		return sq_throwerror(v, "invalid number of arguments in actorWalkTo");
	}
	return 0;
}

static SQInteger addSelectableActor(HSQUIRRELVM v) {
	SQInteger slot;
	if (SQ_FAILED(sqget(v, 2, slot)))
		return sq_throwerror(v, "failed to get slot");
	Common::SharedPtr<Object> actor = sqactor(v, 3);
	g_twp->_hud->_actorSlots[slot - 1].actor = actor;
	return 0;
}

// Creates a new actor from a table.
//
// An actor is defined in the DefineActors.nut file.
static SQInteger createActor(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) != OT_TABLE)
		return sq_throwerror(v, "failed to get a table");

	HSQUIRRELVM vm = g_twp->getVm();
	Common::SharedPtr<Object> actor = Object::createActor();
	sq_resetobject(&actor->_table);
	sq_getstackobj(v, 2, &actor->_table);
	sq_addref(vm, &actor->_table);
	const int id = g_twp->_resManager->newActorId();
	setId(actor->_table, id);
	g_twp->_resManager->_allObjects[id] = actor;

	Common::String key;
	if (sqrawexists(actor->_table, "_key") && SQ_FAILED(sqgetf(actor->_table, "_key", key))) {
		return sq_throwerror(v, "failed to get actor key");
	}
	actor->_key = key;

	debugC(kDebugActScript, "Create actor %s %d", key.c_str(), actor->getId());
	actor->_nodeAnim->remove();
	actor->_node->remove();
	actor->_node = Common::SharedPtr<Node>(new ActorNode(actor));
	actor->_nodeAnim = Common::SharedPtr<Anim>(new Anim(actor.get()));
	actor->_node->addChild(actor->_nodeAnim.get());
	g_twp->_actors.push_back(actor);

	sq_pushobject(v, actor->_table);
	return 1;
}

static SQInteger flashSelectableActor(HSQUIRRELVM v) {
	SQInteger time = 0;
	if (SQ_FAILED(sqget(v, 2, time)))
		return sq_throwerror(v, "failed to get time");
	g_twp->flashSelectableActor(time);
	return 0;
}

struct GetStrings {
	explicit GetStrings(Common::StringArray &texts) : _texts(texts) {}

	void operator()(HSQOBJECT item) {
		_texts.push_back(sq_objtostring(&item));
	}

private:
	Common::StringArray &_texts;
};

static SQInteger sayOrMumbleLine(HSQUIRRELVM v) {
	Common::SharedPtr<Object> obj;
	int index;
	Common::StringArray texts;
	if (sq_gettype(v, 2) == OT_TABLE) {
		obj = sqobj(v, 2);
		index = 3;
	} else {
		index = 2;
		obj = g_twp->_actor;
	}

	if (sq_gettype(v, index) == OT_ARRAY) {
		HSQOBJECT arr;
		sq_getstackobj(v, index, &arr);
		sqgetitems(arr, GetStrings(texts));
	} else {
		int numIds = sq_gettop(v) - index + 1;
		for (int i = 0; i < numIds; i++) {
			if (sq_gettype(v, index + i) != OT_NULL) {
				Common::String text;
				if (SQ_FAILED(sqget(v, index + i, text)))
					return sq_throwerror(v, "failed to get text");
				texts.push_back(text);
			}
		}
	}
	debugC(kDebugActScript, "sayline: %s, %s", obj->_key.c_str(), join(texts, "|").c_str());
	Object::say(obj, texts, obj->_talkColor);
	return 0;
}

// Causes an actor to say a line of dialog and play the appropriate talking animations.
// In the first example, the actor ray will say the line.
// In the second, the selected actor will say the line.
// In the third example, the first line is displayed, then the second one.
// See also:
// - `mumbleLine method`
static SQInteger sayLine(HSQUIRRELVM v) {
	g_twp->stopTalking();
	return sayOrMumbleLine(v);
}

// Say a line of dialog and play the appropriate talking animations.
// In the first example, the actor ray will say the line.
// In the second, the selected actor will say the line.
// In the third example, the first line is displayed, then the second one.
// See also:
// - `mumbleLine method`
static SQInteger sayLineAt(HSQUIRRELVM v) {
	SQInteger x, y;
	Common::String text;
	float duration = -1.0f;
	Common::SharedPtr<Object> actor;
	if (SQ_FAILED(sqget(v, 2, x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sqget(v, 3, y)))
		return sq_throwerror(v, "failed to get y");
	Color color;
	if (sq_gettype(v, 4) == OT_INTEGER) {
		SQInteger c;
		if (SQ_FAILED(sqget(v, 4, c)))
			return sq_throwerror(v, "failed to get color");
		color = Color::rgb(c);
	} else {
		actor = sqactor(v, 4);
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
		color = actor->_talkColor;
	}

	if (SQ_FAILED(sqget(v, 5, duration)))
		return sq_throwerror(v, "failed to get duration");
	if (SQ_FAILED(sqget(v, 6, text)))
		return sq_throwerror(v, "failed to get text");

	debugC(kDebugActScript, "saylineAt: (%lld,%lld) text=%s color=%s duration=%f", x, y, text.c_str(), color.toStr().c_str(), duration);
	g_twp->sayLineAt(Math::Vector2d(x, y), color, actor, duration, text);
	return 0;
}

// returns true if the specified actor is currently in the screen.
static SQInteger isActorOnScreen(HSQUIRRELVM v) {
	Common::SharedPtr<Object> obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get actor/object");

	if (obj->_room != g_twp->_room) {
		sqpush(v, false);
	} else {
		Math::Vector2d pos = obj->_node->getPos() - g_twp->getGfx().cameraPos();
		Math::Vector2d size = g_twp->getGfx().camera();
		bool isOnScreen = Common::Rect(0.0f, 0.0f, size.getX(), size.getY()).contains(pos.getX(), pos.getY());
		sqpush(v, isOnScreen);
	}
	return 1;
}

static SQInteger isActorSelectable(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	ActorSlot *slot = g_twp->_hud->actorSlot(actor);
	bool selectable = slot && slot->selectable;
	sqpush(v, selectable);
	return 1;
}

// If an actor is specified, returns true otherwise returns false.
static SQInteger is_actor(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor(sqactor(v, 2));
	sqpush(v, actor != nullptr);
	return 1;
}

// Returns an array with every single actor that has been defined in the game so far, including non-player characters.
// See also masterRoomArray.
static SQInteger masterActorArray(HSQUIRRELVM v) {
	sq_newarray(v, 0);
	for (auto actor : g_twp->_actors) {
		sqpush(v, actor->_table);
		sq_arrayappend(v, -2);
	}
	return 1;
}

// Makes actor say a line or multiple lines.
// Unlike sayLine this line will not interrupt any other talking on the screen.
// Cannot be interrupted by normal sayLines.
// See also:
// - `sayLine method`.
static SQInteger mumbleLine(HSQUIRRELVM v) {
	return sayOrMumbleLine(v);
}

// Stops all the current sayLines or mumbleLines that the actor is currently saying or are queued to be said.
// Passing ALL will stop anyone who is talking to stop.
// If no parameter is passed, it will stop the currentActor talking.
static SQInteger stopTalking(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	if (nArgs == 2) {
		if (sq_gettype(v, 2) == OT_INTEGER) {
			g_twp->stopTalking();
		} else {
			Common::SharedPtr<Object> actor = sqobj(v, 2);
			if (!actor)
				return sq_throwerror(v, "failed to get actor/object");
			actor->stopTalking();
		}
	} else if (nArgs == 1) {
		g_twp->_actor->stopTalking();
	}
	return 0;
}

// Causes the actor to become the selected actor.
// If they are in the same room as the last selected actor the camera will pan over to them.
// If they are in a different room, the camera will cut to the new room.
// The UI will change to reflect the new actor and their inventory.
static SQInteger selectActor(HSQUIRRELVM v) {
	g_twp->setActor(sqobj(v, 2));
	return 0;
}

// Returns an array of all the actors that are currently within a specified trigger box.
//
// . code-block:: Squirrel
// local stepsArray = triggerActors(AStreet.bookStoreLampTrigger)
// if (stepsArray.len()) {    // someone's on the steps
// }
static SQInteger triggerActors(HSQUIRRELVM v) {
	Common::SharedPtr<Object> obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	sq_newarray(v, 0);
	for (auto actor : g_twp->_actors) {
		if (obj->contains(actor->_node->getPos())) {
			sq_pushobject(v, actor->_table);
			sq_arrayappend(v, -2);
		}
	}
	return 1;
}

static SQInteger verbUIColors(HSQUIRRELVM v) {
	SQInteger actorSlot;
	if (SQ_FAILED(sqget(v, 2, actorSlot)))
		return sq_throwerror(v, "failed to get actorSlot");
	HSQOBJECT table;
	if (SQ_FAILED(sqget(v, 3, table)))
		return sq_throwerror(v, "failed to get table");
	if (!sq_istable(table))
		return sq_throwerror(v, "failed to get verb definitionTable");

	// get mandatory colors
	SQInteger
		sentence = 0,
		verbNormal = 0,
		verbNormalTint = 0,
		verbHighlight = 0,
		verbHighlightTint = 0,
		inventoryFrame = 0,
		inventoryBackground = 0;
	if (sqrawexists(table, "sentence") && SQ_FAILED(sqgetf(table, "sentence", sentence)))
		return sq_throwerror(v, "failed to get sentence");
	if (sqrawexists(table, "verbNormal") && SQ_FAILED(sqgetf(table, "verbNormal", verbNormal)))
		return sq_throwerror(v, "failed to get verbNormal");
	if (sqrawexists(table, "verbNormalTint") && SQ_FAILED(sqgetf(table, "verbNormalTint", verbNormalTint)))
		return sq_throwerror(v, "failed to get verbNormalTint");
	if (sqrawexists(table, "verbHighlight") && SQ_FAILED(sqgetf(table, "verbHighlight", verbHighlight)))
		return sq_throwerror(v, "failed to get verbHighlight");
	if (sqrawexists(table, "verbHighlightTint") && SQ_FAILED(sqgetf(table, "verbHighlightTint", verbHighlightTint)))
		return sq_throwerror(v, "failed to get verbHighlightTint");
	if (sqrawexists(table, "inventoryFrame") && SQ_FAILED(sqgetf(table, "inventoryFrame", inventoryFrame)))
		return sq_throwerror(v, "failed to get inventoryFrame");
	if (sqrawexists(table, "inventoryBackground") && SQ_FAILED(sqgetf(table, "inventoryBackground", inventoryBackground)))
		return sq_throwerror(v, "failed to get inventoryBackground");

	// get optional colors
	SQInteger retroNormal = verbNormal;
	SQInteger retroHighlight = verbNormalTint;
	SQInteger dialogNormal = verbNormal;
	SQInteger dialogHighlight = verbHighlight;
	if (sqrawexists(table, "retroNormal") && SQ_FAILED(sqgetf(table, "retroNormal", retroNormal)))
		return sq_throwerror(v, "failed to get retroNormal");
	if (sqrawexists(table, "retroHighlight") && SQ_FAILED(sqgetf(table, "retroHighlight", retroHighlight)))
		return sq_throwerror(v, "failed to get retroHighlight");
	if (sqrawexists(table, "dialogNormal") && SQ_FAILED(sqgetf(table, "dialogNormal", dialogNormal)))
		return sq_throwerror(v, "failed to get dialogNormal");
	if (sqrawexists(table, "dialogHighlight") && SQ_FAILED(sqgetf(table, "dialogHighlight", dialogHighlight)))
		return sq_throwerror(v, "failed to get dialogHighlight");

	g_twp->_hud->_actorSlots[actorSlot - 1].verbUiColors =
		VerbUiColors(
			Color::rgb(sentence),
			Color::rgb(verbNormal),
			Color::rgb(verbNormalTint),
			Color::rgb(verbHighlight),
			Color::rgb(verbHighlightTint),
			Color::rgb(dialogNormal),
			Color::rgb(dialogHighlight),
			Color::rgb(inventoryFrame),
			Color::rgb(inventoryBackground),
			Color::rgb(retroNormal),
			Color::rgb(retroHighlight));
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
