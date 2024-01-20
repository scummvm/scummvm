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
#include "twp/scenegraph.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

static SQInteger addTrigger(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	sq_resetobject(&obj->_enter);
	sq_resetobject(&obj->_leave);
	if (SQ_FAILED(sqget(v, 3, obj->_enter)))
		return sq_throwerror(v, "failed to get enter");
	sq_addref(g_engine->getVm(), &obj->_enter);
	if (nArgs == 4)
		if (SQ_FAILED(sqget(v, 4, obj->_leave)))
			return sq_throwerror(v, "failed to get leave");
	sq_addref(g_engine->getVm(), &obj->_leave);
	g_engine->_room->_triggers.push_back(obj);
	return 0;
}

static SQInteger clampInWalkbox(HSQUIRRELVM v) {
	SQInteger numArgs = sq_gettop(v);
	Math::Vector2d pos1, pos2;
	if (numArgs == 3) {
		int x = 0;
		if (SQ_FAILED(sqget(v, 2, x)))
			return sq_throwerror(v, "failed to get x");
		int y = 0;
		if (SQ_FAILED(sqget(v, 3, y)))
			return sq_throwerror(v, "failed to get y");
		pos1 = Math::Vector2d(x, y);
		pos2 = pos1;
	} else if (numArgs == 5) {
		int x1 = 0;
		if (SQ_FAILED(sqget(v, 2, x1)))
			return sq_throwerror(v, "failed to get x1");
		int y1 = 0;
		if (SQ_FAILED(sqget(v, 3, y1)))
			return sq_throwerror(v, "failed to get y1");
		pos1 = Math::Vector2d(x1, y1);
		int x2 = 0;
		if (SQ_FAILED(sqget(v, 4, x2)))
			return sq_throwerror(v, "failed to get x2");
		int y2 = 0;
		if (SQ_FAILED(sqget(v, 5, y1)))
			return sq_throwerror(v, "failed to get y2");
		pos2 = Math::Vector2d(x2, y2);
	} else {
		return sq_throwerror(v, "Invalid argument number in clampInWalkbox");
	}
	const Common::Array<Walkbox> &walkboxes = g_engine->_room->_walkboxes;
	for (size_t i = 0; i < walkboxes.size(); i++) {
		const Walkbox &walkbox = walkboxes[i];
		if (walkbox.contains(pos1)) {
			sqpush(v, pos1);
			return 1;
		}
	}
	Math::Vector2d pos = walkboxes[0].getClosestPointOnEdge(pos2);
	sqpush(v, pos);
	return 1;
}

static SQInteger createLight(HSQUIRRELVM v) {
	int color;
	if (SQ_FAILED(sqget(v, 2, color)))
		return sq_throwerror(v, "failed to get color");
	int x;
	if (SQ_FAILED(sqget(v, 3, x)))
		return sq_throwerror(v, "failed to get x");
	int y;
	if (SQ_FAILED(sqget(v, 4, y)))
		return sq_throwerror(v, "failed to get y");
	Light *light = g_engine->_room->createLight(Color::rgb(color), Math::Vector2d(x, y));
	debug("createLight(%d) -> %d", color, light->id);
	sqpush(v, light->id);
	return 1;
}

static SQInteger enableTrigger(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	bool enabled;
	if (SQ_FAILED(sqget(v, 3, enabled)))
		return sq_throwerror(v, "failed to get enabled");
	if (enabled)
		g_engine->_room->_triggers.push_back(obj);
	else {
		int index = find(g_engine->_room->_triggers, obj);
		if (index != -1)
			g_engine->_room->_triggers.remove_at(index);
	}
	return 0;
}

static SQInteger enterRoomFromDoor(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	g_engine->enterRoom(obj->_room, obj);
	return 0;
}

static SQInteger lightBrightness(HSQUIRRELVM v) {
	warning("TODO: lightBrightness not implemented");
	return 0;
}

static SQInteger lightConeDirection(HSQUIRRELVM v) {
	warning("TODO: lightConeDirection not implemented");
	return 0;
}

static SQInteger lightConeAngle(HSQUIRRELVM v) {
	warning("TODO: lightConeAngle not implemented");
	return 0;
}

static SQInteger lightConeFalloff(HSQUIRRELVM v) {
	warning("TODO: lightConeFalloff not implemented");
	return 0;
}

static SQInteger lightCutOffRadius(HSQUIRRELVM v) {
	warning("TODO: lightCutOffRadius not implemented");
	return 0;
}

static SQInteger lightHalfRadius(HSQUIRRELVM v) {
	warning("TODO: lightHalfRadius not implemented");
	return 0;
}

static SQInteger lightTurnOn(HSQUIRRELVM v) {
	warning("TODO: lightTurnOn not implemented");
	return 0;
}

static SQInteger lightZRange(HSQUIRRELVM v) {
	warning("TODO: lightZRange not implemented");
	return 0;
}

static SQInteger defineRoom(HSQUIRRELVM v) {
	// This command is used during the game's boot process.
	// `defineRoom` is called once for every room in the game, passing it the room's room object.
	// If the room has not been defined, it can not be referenced.
	// `defineRoom` is typically called in the the DefineRooms.nut file which loads and defines every room in the game.
	HSQOBJECT table;
	sq_resetobject(&table);
	if (SQ_FAILED(sq_getstackobj(v, 2, &table)))
		return sq_throwerror(v, "failed to get room table");
	Common::String name;
	sqgetf(v, table, "name", name);
	if (name.size() == 0)
		sqgetf(v, table, "background", name);
	Room *room = g_engine->defineRoom(name, table);
	debug("Define room: %s", name.c_str());
	g_engine->_rooms.push_back(room);
	sqpush(v, room->_table);
	return 1;
}

// Creates a new room called name using the specified template.
//
// . code-block:: Squirrel
// for (local room_id = 1; room_id <= HOTEL_ROOMS_PER_FLOOR; room_id++) {
//     local room = definePseudoRoom("HotelRoomA"+((floor_id*100)+room_id), HotelRoomA)
//     local door = floor["hotelHallDoor"+room_id]
//     ...
// }
static SQInteger definePseudoRoom(HSQUIRRELVM v) {
	const SQChar *name;
	if (SQ_FAILED(sqget(v, 2, name)))
		return sq_throwerror(v, "failed to get name");
	HSQOBJECT table;
	sq_resetobject(&table);
	// if this is a pseudo room, we have to clone the table
	// to have a different instance by room
	if (SQ_FAILED(sq_clone(v, 3)))
		return sq_throwerror(v, "failed to clone room table");
	if (SQ_FAILED(sq_getstackobj(v, -1, &table)))
		return sq_throwerror(v, "failed to get room table");

	Room *room = g_engine->defineRoom(name, table, true);
	debug("Define pseudo room: %s", name);
	g_engine->_rooms.push_back(room);
	sqpush(v, room->_table);
	return 1;
}

// Returns the room table for the room specified by the string roomName.
// Useful for returning specific pseudo rooms where the name is composed of text and a variable.
//
// .. code-block:: Squirrel
// local standardRoom = findRoom("HotelRoomA"+keycard.room_num)
static SQInteger findRoom(HSQUIRRELVM v) {
	Common::String name;
	if (SQ_FAILED(sqget(v, 2, name)))
		return sq_throwerror(v, "failed to get name");
	for (size_t i = 0; i < g_engine->_rooms.size(); i++) {
		Room *room = g_engine->_rooms[i];
		if (room->_name == name) {
			sqpush(v, room->_table);
			return 1;
		}
	}
	warning("Room '%s' not found", name.c_str());
	sq_pushnull(v);
	return 1;
}

// Returns an array of all the rooms that are in the game currently.
//
// This is useful for testing.
//
// .. code-block:: Squirrel
// local roomArray = masterRoomArray()
// foreach (room in roomArray) {
//     enterRoomFromDoor(room)
//     breaktime(0.10)
// }
static SQInteger masterRoomArray(HSQUIRRELVM v) {
	sq_newarray(v, 0);
	for (size_t i = 0; i < g_engine->_rooms.size(); i++) {
		Room *room = g_engine->_rooms[i];
		sq_pushobject(v, room->_table);
		sq_arrayappend(v, -2);
	}
	return 1;
}

static SQInteger removeTrigger(HSQUIRRELVM v) {
	if (!g_engine->_room)
		return 0;
	if (sq_gettype(v, 2) == OT_CLOSURE) {
		HSQOBJECT closure;
		sq_resetobject(&closure);
		if (SQ_FAILED(sqget(v, 3, closure)))
			return sq_throwerror(v, "failed to get closure");
		for (size_t i = 0; i < g_engine->_room->_triggers.size(); i++) {
			Object *trigger = g_engine->_room->_triggers[i];
			if ((trigger->_enter._unVal.pClosure == closure._unVal.pClosure) || (trigger->_leave._unVal.pClosure == closure._unVal.pClosure)) {
				g_engine->_room->_triggers.remove_at(i);
				return 0;
			}
		}
	} else {
		Object *obj = sqobj(v, 2);
		if (!obj)
			return sq_throwerror(v, "failed to get object");
        size_t i = find(g_engine->_room->_triggers, obj);
		if (i != (size_t)-1) {
			debug("Remove room trigger: %s(%s)", obj->_name.c_str(), obj->_key.c_str());
			g_engine->_room->_triggers.remove_at(find(g_engine->_room->_triggers, obj));
		}
		return 0;
	}
	return 0;
}

// Returns an array of all the actors in the specified room.
//
// .. code-block:: Squirrel
// local actorInBookstore = roomActors(BookStore)
// if (actorInBookstore.len()>1) { ... }
//
// local spotters = roomActors(currentRoom)
// foreach(actor in spotters) { ...}
static SQInteger roomActors(HSQUIRRELVM v) {
	Room *room = sqroom(v, 2);
	if (!room)
		return sq_throwerror(v, "failed to get room");

	sq_newarray(v, 0);
	for (size_t i = 0; i < g_engine->_actors.size(); i++) {
		Object *actor = g_engine->_actors[i];
		if (actor->_room == room) {
			sqpush(v, actor->_table);
			sq_arrayappend(v, -2);
		}
	}
	return 1;
}

static SQInteger roomEffect(HSQUIRRELVM v) {
	int effect = 0;
	if (SQ_FAILED(sqget(v, 2, effect)))
		return sq_throwerror(v, "failed to get effect");
	RoomEffect roomEffect = (RoomEffect)effect;
	SQInteger nArgs = sq_gettop(v);
	if (roomEffect == RoomEffect::Ghost) {
		if (nArgs == 14) {
			sqget(v, 3, g_engine->_shaderParams.iFade);
			sqget(v, 4, g_engine->_shaderParams.wobbleIntensity);
			sqget(v, 6, g_engine->_shaderParams.shadows.rgba.r);
			sqget(v, 7, g_engine->_shaderParams.shadows.rgba.g);
			sqget(v, 8, g_engine->_shaderParams.shadows.rgba.b);
			sqget(v, 9, g_engine->_shaderParams.midtones.rgba.r);
			sqget(v, 10, g_engine->_shaderParams.midtones.rgba.g);
			sqget(v, 11, g_engine->_shaderParams.midtones.rgba.b);
			sqget(v, 12, g_engine->_shaderParams.highlights.rgba.r);
			sqget(v, 13, g_engine->_shaderParams.highlights.rgba.g);
			sqget(v, 14, g_engine->_shaderParams.highlights.rgba.b);
		} else {
			g_engine->_shaderParams.iFade = 1.f;
			g_engine->_shaderParams.wobbleIntensity = 1.f;
			g_engine->_shaderParams.shadows = Color(-0.3f, 0.f, 0.f);
			g_engine->_shaderParams.midtones = Color(-0.2f, 0.f, 0.1f);
			g_engine->_shaderParams.highlights = Color(0.f, 0.f, 0.2f);
		}
	}
	g_engine->_room->_effect = (RoomEffect)effect;
	return 0;
}

// Fades in or out (FADE_IN, FADE_OUT, FADE_WOBBLE, FADE_WOBBLE_TO_SEPIA) of the current room over the specified duration.
//
// Used for dramatic effect when we want to teleport the player actor to somewhere new, or when starting/ending a cutscene that takes place in another room.
//
// .. code-block:: Squirrel
// roomFade(FADE_OUT, 0.5)
// breaktime(0.5)
// actorAt(currentActor, Alleyway.newLocationSpot)
// cameraFollow(currentActor)
// roomFade(FADE_IN, 0.5)
static SQInteger roomFade(HSQUIRRELVM v) {
	SQInteger fadeType;
	float t;
	if (SQ_FAILED(sqget(v, 2, fadeType)))
		return sq_throwerror(v, "failed to get fadeType");
	if (SQ_FAILED(sqget(v, 3, t)))
		return sq_throwerror(v, "failed to get time");
	FadeEffect effect = FadeEffect::In;
	bool sepia = false;
	switch (fadeType) {
	case FADE_IN:
		effect = FadeEffect::In;
		break;
	case FADE_OUT:
		effect = FadeEffect::Out;
		break;
	case FADE_WOBBLE:
		effect = FadeEffect::Wobble;
		break;
	case FADE_WOBBLE_TO_SEPIA:
		effect = FadeEffect::Wobble;
		sepia = true;
		break;
	default:
		break;
	}
	g_engine->fadeTo(effect, t, sepia);
	return 0;
}

// Makes all layers at the specified zsort value in room visible (YES) or invisible (NO).
// It's also currently the only way to affect parallax layers and can be used for minor animation to turn a layer on and off.
//
// .. code-block:: Squirrel
// roomLayer(GrateEntry, -2, NO)  // Make lights out layer invisible
static SQInteger roomLayer(HSQUIRRELVM v) {
	Room *r = sqroom(v, 2);
	int layer;
	SQInteger enabled;
	if (SQ_FAILED(sqget(v, 3, layer)))
		return sq_throwerror(v, "failed to get layer");
	if (SQ_FAILED(sq_getinteger(v, 4, &enabled)))
		return sq_throwerror(v, "failed to get enabled");
	r->layer(layer)->_node->setVisible(enabled != 0);
	return 0;
}

// Puts a color overlay on the top of the entire room.
//
// Transition from startColor to endColor over duration seconds.
// The endColor remains on screen until changed.
// Note that the actual colour is an 8 digit number, the first two digits (00-ff) represent the transparency, while the last 6 digits represent the actual colour.
// If transparency is set to 00, the overlay is completely see through.
// If startColor is not on the screen already, it will flash to that color before starting the transition.
// If no endColor or duration are provided, it will change instantly to color and remain there.
//
// .. code-block:: Squirrel
// // Make lights in QuickiePal flicker
// roomOverlayColor(0x20dff2cd, 0x20dff2cd, 0.0)
// breaktime(1/60)
// roomOverlayColor(0x00000000, 0x00000000, 0.0)
// breaktime(1/60)
//
// if (currentActor == franklin) {
//     roomOverlayColor(0x800040AA)
// }
static SQInteger roomOverlayColor(HSQUIRRELVM v) {
	int startColor;
	SQInteger numArgs = sq_gettop(v);
	if (SQ_FAILED(sqget(v, 2, startColor)))
		return sq_throwerror(v, "failed to get startColor");
	Room *room = g_engine->_room;
	if (room->_overlayTo)
		room->_overlayTo->disable();
	room->setOverlay(Color::fromRgba(startColor));
	if (numArgs == 4) {
		int endColor;
		if (SQ_FAILED(sqget(v, 3, endColor)))
			return sq_throwerror(v, "failed to get endColor");
		float duration;
		if (SQ_FAILED(sqget(v, 4, duration)))
			return sq_throwerror(v, "failed to get duration");
		debug("start overlay from {rgba(startColor)} to {rgba(endColor)} in {duration}s");
		g_engine->_room->_overlayTo = new OverlayTo(duration, room, Color::fromRgba(endColor));
	}
	return 0;
}

static SQInteger roomRotateTo(HSQUIRRELVM v) {
	warning("TODO: roomRotateTo not implemented");
	return 0;
}

static SQInteger roomSize(HSQUIRRELVM v) {
	Room *room = sqroom(v, 2);
	if (!room)
		return sq_throwerror(v, "failed to get room");
	sqpush(v, room->_roomSize);
	return 1;
}

// Sets walkbox to be hidden (YES) or not (NO).
// If the walkbox is hidden, the actors cannot walk to any point within that area anymore, nor to any walkbox that's connected to it on the other side from the actor.
// Often used on small walkboxes below a gate or door to keep the actor from crossing that boundary if the gate/door is closed.
static SQInteger walkboxHidden(HSQUIRRELVM v) {
	Common::String walkbox;
	if (SQ_FAILED(sqget(v, 2, walkbox)))
		return sq_throwerror(v, "failed to get object or walkbox");
	int hidden = 0;
	if (SQ_FAILED(sqget(v, 3, hidden)))
		return sq_throwerror(v, "failed to get object or hidden");
	g_engine->_room->walkboxHidden(walkbox, hidden != 0);
	return 0;
}

void sqgame_register_roomlib(HSQUIRRELVM v) {
	regFunc(v, addTrigger, "addTrigger");
	regFunc(v, clampInWalkbox, "clampInWalkbox");
	regFunc(v, createLight, "createLight");
	regFunc(v, defineRoom, "defineRoom");
	regFunc(v, definePseudoRoom, "definePseudoRoom");
	regFunc(v, enableTrigger, "enableTrigger");
	regFunc(v, enterRoomFromDoor, "enterRoomFromDoor");
	regFunc(v, findRoom, "findRoom");
	regFunc(v, lightBrightness, "lightBrightness");
	regFunc(v, lightConeAngle, "lightConeAngle");
	regFunc(v, lightConeDirection, "lightConeDirection");
	regFunc(v, lightConeFalloff, "lightConeFalloff");
	regFunc(v, lightCutOffRadius, "lightCutOffRadius");
	regFunc(v, lightHalfRadius, "lightHalfRadius");
	regFunc(v, lightTurnOn, "lightTurnOn");
	regFunc(v, lightZRange, "lightZRange");
	regFunc(v, masterRoomArray, "masterRoomArray");
	regFunc(v, removeTrigger, "removeTrigger");
	regFunc(v, roomActors, "roomActors");
	regFunc(v, roomEffect, "roomEffect");
	regFunc(v, roomFade, "roomFade");
	regFunc(v, roomLayer, "roomLayer");
	regFunc(v, roomRotateTo, "roomRotateTo");
	regFunc(v, roomSize, "roomSize");
	regFunc(v, roomOverlayColor, "roomOverlayColor");
	regFunc(v, walkboxHidden, "walkboxHidden");
}
} // namespace Twp
