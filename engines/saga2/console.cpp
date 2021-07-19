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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "saga2/saga2.h"
#include "saga2/objects.h"
#include "saga2/player.h"
#include "saga2/mapfeatr.h"

#include "saga2/console.h"

namespace Saga2 {

extern pCMapFeature mapFeatures[];

Console::Console(Saga2Engine *vm) : GUI::Debugger() {
	_vm = vm;

	registerCmd("kill_protag", WRAP_METHOD(Console, cmdKillProtag));

	registerCmd("kill", WRAP_METHOD(Console, cmdKill));

	registerCmd("godmode", WRAP_METHOD(Console, cmdGodmode));

	registerCmd("obj_name", WRAP_METHOD(Console, cmdObjName));

	registerCmd("name2id", WRAP_METHOD(Console, cmdObjNameToID));

	registerCmd("position", WRAP_METHOD(Console, cmdPosition));

	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));

	registerCmd("goto_place", WRAP_METHOD(Console, cmdGotoPlace));
}

Console::~Console() {
}

bool Console::cmdKillProtag(int argc, const char **argv) {
	debugPrintf("Killing protagonist\n");

	Actor *protag = (Actor *)GameObject::objectAddress(ActorBaseID);
	protag->getStats()->vitality = 0;

	return true;
}

bool Console::cmdKill(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Actor id>\n", argv[0]);
	else {
		Actor *a = (Actor *)GameObject::objectAddress(atoi(argv[1]));
		if (a)
			a->getStats()->vitality = 0;
	}

	return true;
}

bool Console::cmdGodmode(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s\n <1/0>\n", argv[0]);
	else {
		for (ObjectID id = ActorBaseID; id < ActorBaseID + kPlayerActors; ++id) {
			Actor *a = (Actor *)GameObject::objectAddress(id);
			a->_godmode = atoi(argv[1]);
		}
	}

	return true;
}

bool Console::cmdObjName(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Object id>\n", argv[0]);
	else {
		GameObject *obj = GameObject::objectAddress(atoi(argv[1]));
		if (obj)
			debugPrintf("%s\n", obj->objName());
	}

	return true;
}

bool Console::cmdObjNameToID(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Name index>", argv[0]);
	else {
		int32 id = GameObject::nameIndexToID(atoi(argv[1]));
		if (id == -1)
			debugPrintf("Invalid name index!\n");
		else
			debugPrintf("%d\n", id);
	}

	return true;
}

bool Console::cmdPosition(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <1/0>\n", argv[0]);
	else {
		bool show = atoi(argv[1]);
		_vm->_showPosition = show;
	}

	return true;
}

bool Console::cmdTeleport(int argc, const char **argv) {
	if (argc != 4)
		debugPrintf("Usage: %s <u> <v> <z>\n", argv[0]);
	else {
		int u = atoi(argv[1]);
		int v = atoi(argv[2]);
		int z = atoi(argv[3]);

		Actor *a = getCenterActor();
		a->setLocation(TilePoint(u, v, z));
	}

	return true;
}

bool Console::cmdGotoPlace(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <place id>\n", argv[0]);
	else {
		int placeID = atoi(argv[1]);
		int u = mapFeatures[placeID]->getU();
		int v = mapFeatures[placeID]->getV();

		Actor *a = getCenterActor();

		int du = u - a->getLocation().u;
		int dv = v - a->getLocation().v;

		for (ObjectID id = ActorBaseID; id < ActorBaseID + kPlayerActors; ++id) {
			Actor *p = (Actor *)GameObject::objectAddress(id);
			TilePoint curLoc = p->getLocation();
			p->setLocation(TilePoint(curLoc.u + du, curLoc.v + dv, 8));
		}
	}

	return true;
}

}
