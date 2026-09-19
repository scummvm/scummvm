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
#include "ultima/ultima2/console.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/data/tiles.h"
#include "ultima/ultima2/logic/overworld_logic.h"

namespace Ultima {
namespace Ultima2 {

Console::Console() : GUI::Debugger() {
	registerCmd("map", WRAP_METHOD(Console, cmdMap));
	registerCmd("tiles", WRAP_METHOD(Console, cmdTiles));
	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));
	registerCmd("locations", WRAP_METHOD(Console, cmdLocations));
	registerCmd("intangible", WRAP_METHOD(Console, cmdIntangible));
	registerCmd("enemy", WRAP_METHOD(Console, cmdEnemy));
	registerCmd("hp", WRAP_METHOD(Console, cmdHP));
	registerCmd("food", WRAP_METHOD(Console, cmdFood));
	registerCmd("gold", WRAP_METHOD(Console, cmdGold));
}

Console::~Console() {
}

bool Console::cmdMap(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("map <era> <map type>\n");
		return true;
	}

	Data::Savegame &sg = _G(savegame);
	int mapEra = atoi(argv[1]);
	int mapType = atoi(argv[2]);

	if (!Common::File::exists(Data::mapFilename(mapEra, mapType).c_str())) {
		debugPrintf("No such map: %s\n", Data::mapFilename(mapEra, mapType).c_str());
		return true;
	}

	if (mapType >= 4) {
		// Towers and dungeons have no view yet, so just dump the loaded cells
		_G(dungeon).load(mapEra, mapType);
		debugPrintf("Loaded dungeon MAPX%d%d\n", mapEra, mapType);

		for (int level = 0; level < Data::DUNGEON_LEVELS; ++level) {
			debugPrintf("Level %d:\n", level);
			for (int y = 0; y < Data::DUNGEON_HEIGHT; ++y) {
				Common::String line;
				for (int x = 0; x < Data::DUNGEON_WIDTH; ++x)
					line += Common::String::format("%02x", _G(dungeon)._cells[level][y][x]);
				debugPrintf("%s\n", line.c_str());
			}
		}
		return true;
	}

	if (sg._mapType == 0) {
		sg._overworldReturnX = sg._mapX;
		sg._overworldReturnY = sg._mapY;
	}
	if (mapType != 0) {
		sg._mapX = 31;
		sg._mapY = 62;
	}

	sg._mapEra = mapEra;
	sg._mapType = mapType;
	_G(map).load(mapEra, mapType);
	_G(logic)->entering();
	return false;
}

bool Console::cmdTiles(int argc, const char **argv) {
	Graphics::Surface tiles[Data::TILE_COUNT];
	Data::loadTiles(tiles);

	debugPrintf("Loaded %d tiles\n", Data::TILE_COUNT);
	for (int t = 0; t < Data::TILE_COUNT; ++t) {
		uint32 sum = 0;
		const byte *pixels = (const byte *)tiles[t].getPixels();
		for (int i = 0; i < Data::TILE_WIDTH * Data::TILE_HEIGHT; ++i)
			sum += pixels[i];

		debugPrintf("Tile %2d: pixel sum=%u\n", t, sum);
	}

	// Tile 6 (Town) has a distinctive twin-tower shape, useful as a
	// visual sanity check that the decode is correct
	debugPrintf("Tile %d (Town):\n", (int)Data::TILE_TOWN);
	const byte *townPixels = (const byte *)tiles[Data::TILE_TOWN].getPixels();
	for (int y = 0; y < Data::TILE_HEIGHT; ++y) {
		Common::String line;
		for (int x = 0; x < Data::TILE_WIDTH; ++x)
			line += (char)('0' + townPixels[y * Data::TILE_WIDTH + x]);
		debugPrintf("%s\n", line.c_str());
	}

	for (int t = 0; t < Data::TILE_COUNT; ++t)
		tiles[t].free();

	return true;
}

bool Console::cmdTeleport(int argc, const char **argv) {
	Data::Savegame &sg = _G(savegame);

	if (argc == 3) {
		sg._mapX = atoi(argv[1]);
		sg._mapY = atoi(argv[2]);
		g_engine->focusedView()->redraw();
	} else if (argc != 1) {
		debugPrintf("teleport [<x> <y>]\n");
		return true;
	}

	debugPrintf("Position: (%d,%d)\n", sg._mapX, sg._mapY);
	return true;
}

bool Console::cmdLocations(int argc, const char **argv) {
	static const struct {
		Data::TileId tile;
		const char *name;
	} LOCATION_TILES[] = {
		{ Data::TILE_VILLAGE, "VILLAGE" },
		{ Data::TILE_TOWN, "TOWN" },
		{ Data::TILE_TOWER, "TOWER" },
		{ Data::TILE_CASTLE, "CASTLE" },
		{ Data::TILE_DUNGEON_ENTRANCE, "DUNGEON" },
		{ Data::TILE_SIGNPOST, "SIGNPOST" }
	};

	Common::Array<Common::Point> positions;
	Common::Array<const char *> names;
	for (int y = 0; y < Data::MAP_HEIGHT; ++y) {
		for (int x = 0; x < Data::MAP_WIDTH; ++x) {
			Data::TileId tile = _G(map).tileAt(x, y);
			for (const auto &entry : LOCATION_TILES) {
				if (tile == entry.tile) {
					positions.push_back(Common::Point(x, y));
					names.push_back(entry.name);
					break;
				}
			}
		}
	}

	if (argc == 2) {
		int index = atoi(argv[1]);
		if (index < 0 || index >= (int)positions.size()) {
			debugPrintf("Invalid location index\n");
			return true;
		}

		_G(savegame)._mapX = positions[index].x;
		_G(savegame)._mapY = positions[index].y;
		g_engine->focusedView()->redraw();
		debugPrintf("Teleported to %d: %s (%d,%d)\n", index, names[index],
			positions[index].x, positions[index].y);
		return true;
	} else if (argc != 1) {
		debugPrintf("locations [<index>]\n");
		return true;
	}

	for (uint i = 0; i < positions.size(); ++i)
		debugPrintf("%2d: %-8s (%d,%d)\n", i, names[i], positions[i].x, positions[i].y);

	return true;
}

bool Console::cmdEnemy(int argc, const char **argv) {
	Data::Savegame &sg = _G(savegame);
	if (sg._mapType != 0) {
		debugPrintf("Only supported on the overworld map\n");
		return true;
	}

	Logic::OverworldLogic *logic = dynamic_cast<Logic::OverworldLogic *>(_G(logic).get());
	assert(logic);

	Data::MapMonsters &monsters = _G(map)._monsters;

	int slot = -1;
	for (int i = 1; i <= 31; ++i) {
		if (!monsters.isActive(i)) {
			slot = i;
			break;
		}
	}
	if (slot < 0) {
		slot = 1;
		monsters._type[slot] = 0;
		debugPrintf("No free monster slots; evicted the monster in slot %d\n", slot);
	}

	int spawnX = -1, spawnY = -1;
	for (int radius = 1; radius <= 10 && spawnX < 0; ++radius) {
		for (int dy = -radius; dy <= radius && spawnX < 0; ++dy) {
			for (int dx = -radius; dx <= radius && spawnX < 0; ++dx) {
				if (MAX(ABS(dx), ABS(dy)) != radius)
					continue;

				int x = (sg._mapX + dx + Data::MAP_WIDTH) % Data::MAP_WIDTH;
				int y = (sg._mapY + dy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;
				if (!logic->isWalkable(_G(map).tileAt(x, y)))
					continue;

				bool occupied = false;
				for (int i = 1; i <= 31; ++i) {
					if (monsters.isActive(i) && monsters._mapX[i] == x && monsters._mapY[i] == y) {
						occupied = true;
						break;
					}
				}
				if (!occupied) {
					spawnX = x;
					spawnY = y;
					break;
				}
			}
		}
	}

	if (spawnX < 0) {
		debugPrintf("Couldn't find a free tile nearby\n");
		return true;
	}

	monsters._mapX[slot] = spawnX;
	monsters._mapY[slot] = spawnY;
	monsters._spellHP[slot] = 0x10;
	monsters._type[slot] = Data::TILE_ORC * 4;
	monsters._glyphTile[slot] = 0;
	monsters._offerFlag[slot] = 0;
	monsters._tempX[slot] = 0;
	monsters._tempY[slot] = 0;

	g_engine->focusedView()->redraw();
	debugPrintf("Spawned enemy in slot %d at (%d,%d)\n", slot, spawnX, spawnY);
	return true;
}

bool Console::cmdIntangible(int argc, const char **argv) {
	_G(intangible) = !_G(intangible);
	debugPrintf("Intangible mode %s\n", _G(intangible) ? "ON" : "OFF");
	return true;
}

bool Console::cmdHP(int argc, const char **argv) {
	_G(savegame)._hp = 9999;
	g_engine->baseView()->findView("Stats")->draw();
	debugPrintf("HP set to 9999\n");
	return true;
}

bool Console::cmdFood(int argc, const char **argv) {
	_G(savegame)._food = 9999;
	_G(savegame)._foodTurnCtr = 0;
	g_engine->baseView()->findView("Stats")->draw();
	debugPrintf("Food set to 9999\n");
	return true;
}

bool Console::cmdGold(int argc, const char **argv) {
	_G(savegame)._gold = 9999;
	g_engine->baseView()->findView("Stats")->draw();
	debugPrintf("Gold set to 9999\n");
	return true;
}

} // namespace Ultima2
} // namespace Ultima
