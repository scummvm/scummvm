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

static SQInteger actorShowHideLayer(HSQUIRRELVM v, bool visible) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::String layer;
	if (SQ_FAILED(sqget(v, 3, layer)))
		return sq_throwerror(v, "failed to get layer");
	actor->showLayer(layer, visible);
	return 0;
}

static SQInteger actorHideLayer(HSQUIRRELVM v) {
	return actorShowHideLayer(v, false);
}

static SQInteger actorShowLayer(HSQUIRRELVM v) {
	return actorShowHideLayer(v, true);
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
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	sqpush(v, actor->_node->getPos().getX());
	return 1;
}

static SQInteger actorPosY(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	sqpush(v, actor->_node->getPos().getY());
	return 1;
}

// Plays the specified animation from the player's costume JSON filename.
// If YES loop the animation. Default is NO.
static SQInteger actorPlayAnimation(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Common::String animation;
	if (SQ_FAILED(sqget(v, 3, animation)))
		return sq_throwerror(v, "failed to get animation");
	int loop = 0;
	if ((sq_gettop(v) >= 4) && (SQ_FAILED(sqget(v, 4, loop))))
		return sq_throwerror(v, "failed to get loop");
	debug("Play anim %s %s loop=%s", actor->_key.c_str(), animation.c_str(), loop ? "yes" : "no");
	actor->play(animation, loop != 0);
	return 0;
}

// Sets the rendering offset of the actor to x and y.
//
// A rendering offset of 0,0 would cause them to be rendered from the middle of their image.
// Actor's are typically adjusted so they are rendered from the middle of the bottom of their feet.
// To maintain sanity, it is best if all actors have the same image size and are all adjust the same, but this is not a requirement.
static SQInteger actorRenderOffset(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	int x, y;
	if (SQ_FAILED(sqget(v, 3, x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sqget(v, 4, y)))
		return sq_throwerror(v, "failed to get y");
	actor->_node->setRenderOffset(Math::Vector2d(x, y));
	return 0;
}

static SQInteger actorStand(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
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
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	actor->stopWalking();
	actor->stand();
	return 0;
}

// Set the text color of the specified actor's text that appears when they speak.
static SQInteger actorTalkColors(HSQUIRRELVM v) {
	Object *actor = sqobj(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	int color;
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
	Object *actor = nullptr;
	if (sq_gettop(v) == 2) {
		actor = sqobj(v, 2);
		if (!actor) {
			sqpush(v, false);
			return 1;
		}
	} else {
		actor = g_engine->_actor;
	}
	bool isTalking = actor && actor->getTalking() && actor->getTalking()->isEnabled();
	sqpush(v, isTalking);
	return 1;
}

static SQInteger actorTurnTo(HSQUIRRELVM v) {
	warning("TODO: actorTurnTo not implemented");
	return 0;
}

// Specifies the offset that will be applied to the actor's speech text that appears on screen.
static SQInteger actorTalkOffset(HSQUIRRELVM v) {
	Object *actor = sqobj(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	int x, y;
	if (SQ_FAILED(sqget(v, 3, x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sqget(v, 4, y)))
		return sq_throwerror(v, "failed to get y");
	actor->_talkOffset = Math::Vector2d(x, y);
	return 0;
}

static SQInteger actorUsePos(HSQUIRRELVM v) {
	Math::Vector2d usePos;
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	Object *obj = sqobj(v, 3);
	if (!obj)
		usePos = Math::Vector2d();
	else
		usePos = obj->_usePos;
	if (sq_gettop(v) == 4) {
		int dir;
		if (SQ_FAILED(sqget(v, 4, dir)))
			return sq_throwerror(v, "failed to get direction");
		else
			actor->_useDir = (Direction)dir;
	}
	actor->_usePos = usePos;
	return 0;
}

// Specifies whether the actor needs to abide by walkboxes or not.
//
// . code-block:: Squirrel
// actorUseWalkboxes(coroner, NO)
static SQInteger actorUseWalkboxes(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	int useWalkboxes = 1;
	if (SQ_FAILED(sqget(v, 3, useWalkboxes)))
		return sq_throwerror(v, "failed to get useWalkboxes");
	actor->_useWalkboxes = useWalkboxes != 0;
	return 0;
}

static SQInteger actorVolume(HSQUIRRELVM v) {
	Object *actor = sqactor(v, 2);
	if (!actor)
		return sq_throwerror(v, "failed to get actor");
	float volume = 0.0f;
	if (SQ_FAILED(sqget(v, 3, volume)))
		return sq_throwerror(v, "failed to get volume");
	actor->_volume = volume;
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

static SQInteger sayOrMumbleLine(HSQUIRRELVM v) {
	Object *obj;
	int index;
	Common::StringArray texts;
	if (sq_gettype(v, 2) == OT_TABLE) {
		obj = sqobj(v, 2);
		index = 3;
	} else {
		index = 2;
		obj = g_engine->_actor;
	}

	if (sq_gettype(v, index) == OT_ARRAY) {
		HSQOBJECT arr;
		sq_getstackobj(v, index, &arr);
		sqgetitems(arr, [&](HSQOBJECT item) { texts.push_back(sq_objtostring(&item)); });
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
	debug("sayline: {obj.key}, {texts}");
	obj->say(texts, obj->_talkColor);
	return 0;
}

static void stopTalking() {
	for (auto it = g_engine->_room->_layers.begin(); it != g_engine->_room->_layers.end(); it++) {
		Layer *layer = *it;
		for (auto it2 = layer->_objects.begin(); it2 != layer->_objects.end(); it2++) {
			(*it2)->stopTalking();
		}
	}
}

// Causes an actor to say a line of dialog and play the appropriate talking animations.
// In the first example, the actor ray will say the line.
// In the second, the selected actor will say the line.
// In the third example, the first line is displayed, then the second one.
// See also:
// - `mumbleLine method`
static SQInteger sayLine(HSQUIRRELVM v) {
	stopTalking();
	return sayOrMumbleLine(v);
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
			stopTalking();
		} else {
			Object *actor = sqobj(v, 2);
			if (!actor)
				return sq_throwerror(v, "failed to get actor/object");
			actor->stopTalking();
		}
	} else if (nArgs == 1) {
		g_engine->_actor->stopTalking();
	}
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

// Returns an array of all the actors that are currently within a specified trigger box.
//
// . code-block:: Squirrel
// local stepsArray = triggerActors(AStreet.bookStoreLampTrigger)
// if (stepsArray.len()) {    // someone's on the steps
// }
static SQInteger triggerActors(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	sq_newarray(v, 0);
	for (auto it = g_engine->_actors.begin(); it != g_engine->_actors.end(); it++) {
		Object* actor = *it;
		if (obj->contains(actor->_node->getPos())) {
			sq_pushobject(v, actor->_table);
			sq_arrayappend(v, -2);
		}
	}
	return 1;
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
