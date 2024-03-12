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

#include "common/file.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "image/png.h"

#include "saga2/saga2.h"
#include "saga2/automap.h"
#include "saga2/objects.h"
#include "saga2/player.h"
#include "saga2/mapfeatr.h"
#include "saga2/tile.h"

#include "saga2/console.h"

namespace Saga2 {

extern GameObject *objectList;
extern WorldMapData *mapList;
extern AutoMap *pAutoMap;

Console::Console(Saga2Engine *vm) : GUI::Debugger() {
	_vm = vm;

	registerCmd("kill_protag", WRAP_METHOD(Console, cmdKillProtag));
	registerCmd("kill", WRAP_METHOD(Console, cmdKill));
	registerCmd("godmode", WRAP_METHOD(Console, cmdGodmode));
	registerCmd("obj_name", WRAP_METHOD(Console, cmdObjName));
	registerCmd("nid2id", WRAP_METHOD(Console, cmdObjNameIndexToID));
	registerCmd("search", WRAP_METHOD(Console, cmdSearchObj));
	registerCmd("add", WRAP_METHOD(Console, cmdAddObj));
	registerCmd("position", WRAP_METHOD(Console, cmdPosition));
	registerCmd("teleport_on_click", WRAP_METHOD(Console, cmdTeleportOnClick));
	registerCmd("teleport_on_map", WRAP_METHOD(Console, cmdTeleportOnMap));
	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));
	registerCmd("teleport_place", WRAP_METHOD(Console, cmdTeleportPlace));
	registerCmd("teleport_to_npc", WRAP_METHOD(Console, cmdTeleportToNPC));
	registerCmd("teleport_npc", WRAP_METHOD(Console, cmdTeleportNPC));
	registerCmd("teleport_npc_here", WRAP_METHOD(Console, cmdTeleportNPCHere));
	registerCmd("teleport_party_here", WRAP_METHOD(Console, cmdTeleportPartyHere));
	registerCmd("save_loc", WRAP_METHOD(Console, cmdSaveLoc));
	registerCmd("load_loc", WRAP_METHOD(Console, cmdLoadLoc));
	registerCmd("goto_place", WRAP_METHOD(Console, cmdGotoPlace));
	registerCmd("list_places", WRAP_METHOD(Console, cmdListPlaces));
	registerCmd("stats", WRAP_METHOD(Console, cmdStats));
	registerCmd("status_msg", WRAP_METHOD(Console, cmdStatusMsg));
	registerCmd("dump_map", WRAP_METHOD(Console, cmdDumpMap));
	registerCmd("play_music", WRAP_METHOD(Console, cmdPlayMusic));
	registerCmd("play_voice", WRAP_METHOD(Console, cmdPlayVoice));
	registerCmd("invis", WRAP_METHOD(Console, cmdInvisibility));
	registerCmd("map_cheat", WRAP_METHOD(Console, cmdMapCheat));
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

bool Console::cmdObjNameIndexToID(int argc, const char **argv) {
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
		Common::String name = argv[1];
		Common::Array<ObjectID> array = GameObject::nameToID(name);
		Common::String type;

		if (array.size() == 0)
			debugPrintf("No objects found!\n");
		else {
			for (uint i = 0; i < array.size(); ++i) {
				ObjectID id = array[i];

				GameObject *obj = GameObject::objectAddress(id);

				if (isObject(obj))
					type = "OBJECT";
				else if (isActor(obj))
					type = "ACTOR";
				else if (isWorld(obj))
					type = "WORLD";
				else
					type = "???";

				debugPrintf("%s: %d (%s)\n", obj->objName(), id, type.c_str());
			}
		}
	}

	return true;
}

bool Console::cmdAddObj(int argc, const char **argv) {
	if (argc == 2) {
		Actor *a = getCenterActor();
		a->placeObject(a->thisID(), atoi(argv[1]));
	} else if (argc == 3) {
		Actor *a = getCenterActor();
		int num = atoi(argv[2]);
		a->placeObject(a->thisID(), atoi(argv[1]), true, num);
	} else
		debugPrintf("Usage: %s <ObjectID> <num = 1>\n", argv[0]);

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

bool Console::cmdStatusMsg(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <1/0>\n", argv[0]);
	else {
		bool show = atoi(argv[1]);
		_vm->_showStatusMsg = show;
	}

	return true;
}

bool Console::cmdTeleportOnClick(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <1/0>\n", argv[0]);
	else {
		bool teleport = atoi(argv[1]);
		_vm->_teleportOnClick = teleport;
	}

	return true;
}

bool Console::cmdTeleportOnMap(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <1/0>\n", argv[0]);
	else {
		bool teleport = atoi(argv[1]);
		_vm->_teleportOnMap = teleport;
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

bool Console::cmdTeleportToNPC(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Actor ID>\n", argv[0]);
	else {
		ObjectID id = atoi(argv[1]);
		Actor *a = getCenterActor();
		Actor *b = (Actor *)GameObject::objectAddress(id);

		a->setLocation(b->getLocation());
	}

	return true;
}

bool Console::cmdTeleportNPC(int argc, const char **argv) {
	if (argc != 5)
		debugPrintf("Usage: %s <Actor ID> <u> <v> <z>\n", argv[0]);
	else {
		ObjectID id = atoi(argv[1]);
		Actor *a = (Actor *)GameObject::objectAddress(id);

		TilePoint loc;
		loc.u = atoi(argv[2]);
		loc.v = atoi(argv[3]);
		loc.z = atoi(argv[4]);

		a->setLocation(loc);
	}

	return true;
}

bool Console::cmdTeleportNPCHere(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Actor ID>\n", argv[0]);
	else {
		ObjectID id = atoi(argv[1]);
		Actor *a = (Actor *)GameObject::objectAddress(id);

		a->setLocation(getCenterActor()->getLocation());
	}

	return true;
}

bool Console::cmdTeleportPartyHere(int argc, const char **argv) {
	if (argc != 1)
		debugPrintf("Usage: %s\n", argv[0]);
	else {
		TilePoint loc = getCenterActor()->getLocation();

		for (ObjectID id = ActorBaseID; id < ActorBaseID + kPlayerActors; ++id) {
			Actor *p = (Actor *)GameObject::objectAddress(id);
			p->setLocation(loc);
		}
	}

	return true;
}

bool Console::cmdTeleportPlace(int argc, const char **argv) {
	if (argc < 1)
		debugPrintf("Usage: %s <place id>/<place name>\n", argv[0]);
	else {
		int placenum = -1;

		if (!Common::isDigit(argv[1][0])) {
			// First, assemble the name
			Common::String place = argv[1];

			for (int i = 2; i < argc; i++)
				place += Common::String(" ") + argv[i];

			for (uint i = 0; i < g_vm->_mapFeatures.size(); ++i) {
				if (g_vm->_mapFeatures[i] && !place.compareToIgnoreCase(g_vm->_mapFeatures[i]->getText())) {
					placenum = i;
					break;
				}
			}
		} else {
			placenum = atoi(argv[1]);
		}

		if (placenum == -1) {
			debugPrintf("Unknown place\n");
			return true;
		}

		Actor *a = getCenterActor();
		a->setLocation(g_vm->_mapFeatures[placenum]->getLocation());
	}

	return true;
}

bool Console::cmdSaveLoc(int argc, const char **argv) {
	if (argc != 1)
		debugPrintf("Usage: %s\n", argv[0]);
	else {
		Actor *a = getCenterActor();
		_savedLoc = a->getLocation();
	}

	return true;
}

bool Console::cmdLoadLoc(int argc, const char **argv) {
	if (argc != 1)
		debugPrintf("Usage: %s\n", argv[0]);
	else {
		Actor *a = getCenterActor();

		if (_savedLoc.u != 0 || _savedLoc.v != 0 || _savedLoc.z != 0)
			a->setLocation(_savedLoc);
		else
			debugPrintf("Location not saved!\n");
	}

	return true;
}

bool Console::cmdGotoPlace(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <place id>\n", argv[0]);
	else {
		uint placeID = atoi(argv[1]);

		if (placeID > g_vm->_mapFeatures.size()) {
			debugPrintf("Invalid place id > %d", g_vm->_mapFeatures.size());
			return true;
		}

		int u = g_vm->_mapFeatures[placeID]->getU();
		int v = g_vm->_mapFeatures[placeID]->getV();

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
		for (uint i = 0; i < g_vm->_mapFeatures.size(); ++i) {
			if (g_vm->_mapFeatures[i])
				debugPrintf("%d: %s\n", i, g_vm->_mapFeatures[i]->getText());
		}
	}

	return true;
}

bool Console::cmdDumpMap(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Map Size Multiplier>\n", argv[0]);
	else {
		gPixelMap drawMap;
		drawMap._size = _vm->_tileDrawMap._size * atoi(argv[1]);
		drawMap._data = new uint8[drawMap.bytes()]();
		drawMetaTiles(drawMap);

		Graphics::Surface sur;
		sur.create(drawMap._size.x, drawMap._size.y, Graphics::PixelFormat::createFormatCLUT8());
		sur.setPixels(drawMap._data);

		Common::Path pngFile(Common::String::format("%s-mapdump.png", _vm->getMetaEngine()->getName()));
		Common::DumpFile dump;
		dump.open(pngFile);

		byte palette[256 * 3];
		g_system->getPaletteManager()->grabPalette(palette, 0, 256);

		Image::writePNG(dump, sur, palette);

		dump.close();

		delete[] drawMap._data;
	}

	return true;
}

bool Console::cmdPlayMusic(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Music Index>\n", argv[0]);
	else {
		int32 currentID = atoi(argv[1]);
		playMusic(MKTAG('X', 'M', 'I', currentID));
	}

	return true;
}

bool Console::cmdPlayVoice(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <Voice ID>\n", argv[0]);
	else {
		int32 soundID = READ_BE_INT32(argv[1]);
		playVoice(soundID);
	}

	return true;
}

bool Console::cmdInvisibility(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <1/0>\n", argv[0]);
	else {
		bool inv = atoi(argv[1]);
		for (ObjectID id = ActorBaseID; id < ActorBaseID + kPlayerActors; ++id) {
			Actor *p = (Actor *)GameObject::objectAddress(id);
			if (inv)
				p->setEffect(kActorInvisible, true);
			else
				p->setEffect(kActorInvisible, false);
		}
	}

	return true;
}

bool Console::cmdMapCheat(int argc, const char **argv) {
	if (argc != 2)
		debugPrintf("Usage: %s <1/0>\n", argv[0]);
	else {
		bool cheat = atoi(argv[1]);
		if (pAutoMap) {
			pAutoMap->setCheatFlag(cheat);
		}
	}

	return true;
}

}
