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

static SQInteger addTrigger(HSQUIRRELVM v) {
	warning("TODO: addTrigger not implemented");
	return 0;
}

static SQInteger clampInWalkbox(HSQUIRRELVM v) {
	warning("TODO: clampInWalkbox not implemented");
	return 0;
}

static SQInteger createLight(HSQUIRRELVM v) {
	warning("TODO: createLight not implemented");
	return 0;
}

static SQInteger enableTrigger(HSQUIRRELVM v) {
	warning("TODO: enableTrigger not implemented");
	return 0;
}

static SQInteger enterRoomFromDoor(HSQUIRRELVM v) {
	warning("TODO: enterRoomFromDoor not implemented");
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

static SQInteger definePseudoRoom(HSQUIRRELVM v) {
	warning("TODO: definePseudoRoom not implemented");
	return 0;
}

static SQInteger findRoom(HSQUIRRELVM v) {
	warning("TODO: findRoom not implemented");
	return 0;
}

static SQInteger masterRoomArray(HSQUIRRELVM v) {
	warning("TODO: masterRoomArray not implemented");
	return 0;
}

static SQInteger removeTrigger(HSQUIRRELVM v) {
	warning("TODO: removeTrigger not implemented");
	return 0;
}

static SQInteger roomActors(HSQUIRRELVM v) {
	warning("TODO: roomActors not implemented");
	return 0;
}

static SQInteger roomEffect(HSQUIRRELVM v) {
	warning("TODO: roomEffect not implemented");
	return 0;
}

static SQInteger roomFade(HSQUIRRELVM v) {
	warning("TODO: roomFade not implemented");
	return 0;
}

static SQInteger roomLayer(HSQUIRRELVM v) {
	warning("TODO: roomLayer not implemented");
	return 0;
}

static SQInteger roomOverlayColor(HSQUIRRELVM v) {
	warning("TODO: roomOverlayColor not implemented");
	return 0;
}

static SQInteger roomRotateTo(HSQUIRRELVM v) {
	warning("TODO: roomRotateTo not implemented");
	return 0;
}

static SQInteger roomSize(HSQUIRRELVM v) {
	warning("TODO: roomSize not implemented");
	return 0;
}

static SQInteger walkboxHidden(HSQUIRRELVM v) {
	warning("TODO: walkboxHidden not implemented");
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
