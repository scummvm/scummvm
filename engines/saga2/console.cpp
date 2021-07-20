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

#include "common/file.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "image/png.h"

#include "saga2/saga2.h"
#include "saga2/objects.h"
#include "saga2/player.h"
#include "saga2/mapfeatr.h"
#include "saga2/tile.h"

#include "saga2/console.h"

namespace Saga2 {

#define MAX_MAP_FEATURES 128

extern pCMapFeature mapFeatures[];
extern GameObject *objectList;
extern WorldMapData *mapList;
extern int16 currentMapNum;

Console::Console(Saga2Engine *vm) : GUI::Debugger() {
	_vm = vm;

	registerCmd("kill_protag", WRAP_METHOD(Console, cmdKillProtag));

	registerCmd("kill", WRAP_METHOD(Console, cmdKill));

	registerCmd("godmode", WRAP_METHOD(Console, cmdGodmode));

	registerCmd("obj_name", WRAP_METHOD(Console, cmdObjName));

	registerCmd("name2id", WRAP_METHOD(Console, cmdObjNameToID));

	registerCmd("search_obj", WRAP_METHOD(Console, cmdSearchObj));

	registerCmd("add_obj", WRAP_METHOD(Console, cmdAddObj));

	registerCmd("position", WRAP_METHOD(Console, cmdPosition));

	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));

	registerCmd("goto_place", WRAP_METHOD(Console, cmdGotoPlace));

	registerCmd("list_places", WRAP_METHOD(Console, cmdListPlaces));

	registerCmd("stats", WRAP_METHOD(Console, cmdStats));

	registerCmd("dump_map", WRAP_METHOD(Console, cmdDumpMap));
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
		debugPrintf("Usage: %s <Name index>\n", argv[0]);
	else {
		int32 id = GameObject::nameIndexToID(atoi(argv[1]));
		if (id == -1)
			debugPrintf("Invalid name index!\n");
		else
			debugPrintf("%d\n", id);
	}

	return true;
}

bool Console::cmdSearchObj(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Object name>\n", argv[0]);
	else {
		for (int i = 0; i < objectCount; ++i) {
			Common::String objName = objectList[i].objName();
			objName.toLowercase();
			if (objName.contains(argv[1]))
				debugPrintf("%d: %s\n", i, objectList[i].objName());
		}
	}

	return true;
}

bool Console::cmdAddObj(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <ObjectID>\n", argv[0]);
	else {
		Actor *a = getCenterActor();
		a->placeObject(a->thisID(), atoi(argv[1]));
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

bool Console::cmdStats(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <1/0>\n", argv[0]);
	else {
		bool show = atoi(argv[1]);
		_vm->_showStats = show;
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

bool Console::cmdListPlaces(int argc, const char **argv) {
	if (argc != 1)
		debugPrintf("Usage: %s\n", argv[0]);
	else {
		for (int i = 0; i < MAX_MAP_FEATURES; ++i) {
			if (mapFeatures[i])
				debugPrintf("%d: %s\n", i, mapFeatures[i]->getText());
		}
	}

	return true;
}

bool Console::cmdDumpMap(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Map Size Multiplier>\n", argv[0]);
	else {
		gPixelMap drawMap;
		drawMap.size = _vm->_tileDrawMap.size * atoi(argv[1]);
		//drawMap.size.x = mapList[currentMapNum].mapHeight;
		//drawMap.size.y = mapList[currentMapNum].mapHeight;
		drawMap.data = new uint8[drawMap.bytes()]();
		drawMetaTiles(drawMap);

		Graphics::Surface sur;
		sur.create(drawMap.size.x, drawMap.size.y, Graphics::PixelFormat::createFormatCLUT8());
		sur.setPixels(drawMap.data);

		Common::String pngFile = Common::String::format("%s-mapdump.png", _vm->getMetaEngine()->getName());
		Common::DumpFile dump;
		dump.open(pngFile);

		byte palette[256 * 3];
		g_system->getPaletteManager()->grabPalette(palette, 0, 256);

		Image::writePNG(dump, sur, palette);

		dump.close();

		delete[] drawMap.data;
	}

	return true;
}

}
