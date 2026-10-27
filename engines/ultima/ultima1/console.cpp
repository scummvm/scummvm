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

#include "ultima/ultima1/console.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/views/dialog.h"

namespace Ultima {
namespace Ultima1 {

Console::Console() : GUI::Debugger() {
	registerCmd("view", WRAP_METHOD(Console, cmdView));
	registerCmd("map", WRAP_METHOD(Console, cmdMap));
	registerCmd("hits", WRAP_METHOD(Console, cmdHits));
	registerCmd("food", WRAP_METHOD(Console, cmdFood));
	registerCmd("coins", WRAP_METHOD(Console, cmdCoins));
	registerCmd("fuel", WRAP_METHOD(Console, cmdFuel));
	registerCmd("space", WRAP_METHOD(Console, cmdSpace));
	registerCmd("inventory", WRAP_METHOD(Console, cmdInventory));
	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));
	registerCmd("locations", WRAP_METHOD(Console, cmdLocations));
	registerCmd("enemy", WRAP_METHOD(Console, cmdEnemy));
}

Console::~Console() {
}

bool Console::cmdView(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("view <view name>\n");
		return true;
	} else {
		auto *view = g_engine->findView(argv[1]);

		if (!view) {
			debugPrintf("Unknown view name\n");
			return true;
		} else if (dynamic_cast<Views::Dialog *>(view)) {
			g_engine->replaceView("Game", true);
			g_engine->addView(argv[1]);
			return false;
		} else {
			g_engine->replaceView(argv[1], true);
			return false;
		}
	}
}

bool Console::cmdMap(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("map <map num>\n");
		return true;
	} else {
		int location = atoi(argv[1]);

		if (location >= 41 && location < 49) {
			g_engine->send("Pillar", Graphics::Views::GameMessage("PILLAR", location - 41));
		} else {
			if (location == Data::MAP_SPACE) {
				_G(savegame)._shipFuel = 9999;
				_G(savegame)._shipShield = 9999;

				// Make sure docking at the station doesn't kill them outright
				// for want of a vacuum suit
				_G(savegame)._armor[Data::ARMOR_VACUUM_SUIT] = 1;
				_G(savegame)._equippedArmor = Data::ARMOR_VACUUM_SUIT;
			}

			_G(map).load(location);
			_G(logic)->entering();
		}

		return false;
	}
}

bool Console::cmdHits(int argc, const char **argv) {
	_G(savegame)._hits = (argc == 2) ? atoi(argv[1]) : 9999;
	g_engine->baseView()->findView("Stats")->draw();
	return false;
}

bool Console::cmdFood(int argc, const char **argv) {
	_G(savegame)._food = (argc == 2) ? atoi(argv[1]) : 9999;
	g_engine->baseView()->findView("Stats")->draw();
	return false;
}

bool Console::cmdCoins(int argc, const char **argv) {
	_G(savegame)._coins = (argc == 2) ? atoi(argv[1]) : 9999;
	g_engine->baseView()->findView("Stats")->draw();
	return false;
}

bool Console::cmdFuel(int argc, const char **argv) {
	if (_G(map)._mapType == Data::MAPTYPE_SPACE) {
		Data::SpaceMapShip &ship = _G(savegame)._starmap
			._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]
			._ships[_G(savegame)._shipIndex];
		ship._fuel = 9999;
		ship._shield = 9999;
	}

	_G(savegame)._shipFuel = 9999;
	_G(savegame)._shipShield = 9999;

	g_engine->baseView()->findView("Stats")->draw();
	return false;
}

bool Console::cmdSpace(int argc, const char **argv) {
	_G(savegame)._shipFuel = 9999;
	_G(savegame)._shipShield = 9999;

	// Make sure docking at the station doesn't kill them outright
	// for want of a vacuum suit
	_G(savegame)._armor[Data::ARMOR_VACUUM_SUIT] = 1;
	_G(savegame)._equippedArmor = Data::ARMOR_VACUUM_SUIT;

	_G(map).load(Data::MAP_SPACE);
	_G(logic)->entering();

	return false;
}

bool Console::cmdInventory(int argc, const char **argv) {
	Data::Savegame &sg = _G(savegame);

	for (int i = 1; i < Data::ARMOR_COUNT; ++i)
		sg._armor[i] = 9;
	for (int i = 1; i < Data::WEAPON_COUNT; ++i)
		sg._weapons[i] = 9;
	for (int i = 1; i < Data::SPELL_COUNT; ++i)
		sg._spells[i] = 99;
	for (int i = 1; i < Data::TRANSPORT_COUNT; ++i)
		sg._transports[i] = 9;

	sg._redGems = sg._greenGems = sg._blueGem = sg._whiteGem = 1;
	sg._hits = sg._food = sg._coins = 9999;

	sg._equippedWeapon = Data::WEAPON_BLASTER;
	sg._equippedArmor = Data::ARMOR_REFLECT_SUIT;
	sg._equippedSpell = Data::SPELL_KILL;

	g_engine->baseView()->findView("Stats")->draw();
	debugPrintf("Full inventory given; readied %s, %s and %s\n",
		Data::WEAPON_NAMES[sg._equippedWeapon], Data::ARMOR_NAMES[sg._equippedArmor],
		Data::SPELL_NAMES[sg._equippedSpell]);
	return true;
}

// Returns to the overworld if the player is elsewhere, so a position
// change on it makes sense
static void ensureOverworld() {
	if (_G(savegame)._mapNum != Data::MAP_OVERWORLD) {
		_G(map).load(Data::MAP_OVERWORLD);
		_G(logic)->entering();
	}
}

bool Console::cmdTeleport(int argc, const char **argv) {
	Data::Savegame &sg = _G(savegame);

	if (argc == 3) {
		int x = atoi(argv[1]), y = atoi(argv[2]);
		if (x < 0 || x >= Data::OVERWORLD_WIDTH || y < 0 || y >= Data::OVERWORLD_HEIGHT) {
			debugPrintf("Position must be within (0,0) - (%d,%d)\n",
				Data::OVERWORLD_WIDTH - 1, Data::OVERWORLD_HEIGHT - 1);
			return true;
		}

		ensureOverworld();
		sg._overworldPos = Common::Point(x, y);
		g_engine->focusedView()->redraw();
	} else if (argc != 1) {
		debugPrintf("teleport [<x> <y>]\n");
		return true;
	}

	debugPrintf("Position: (%d,%d)\n", sg._overworldPos.x, sg._overworldPos.y);
	return true;
}

bool Console::cmdLocations(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("locations [<number>]\n");
		return true;
	}

	int target = (argc == 2) ? atoi(argv[1]) : 0;
	if (argc == 2 && (target < 1 || target > Data::LOCATION_COUNT)) {
		debugPrintf("Location number must be 1 to %d\n", Data::LOCATION_COUNT);
		return true;
	}

	for (int y = 0; y < Data::OVERWORLD_HEIGHT; ++y) {
		for (int x = 0; x < Data::OVERWORLD_WIDTH; ++x) {
			int location = _G(map).getLocationAt(x, y);
			if (!location)
				continue;

			if (argc == 1) {
				debugPrintf("%2d: %-30s (%d,%d)\n", location, Data::LOCATION_NAMES[location - 1], x, y);
			} else if (location == target) {
				ensureOverworld();
				_G(savegame)._overworldPos = Common::Point(x, y);
				g_engine->focusedView()->redraw();
				debugPrintf("Teleported to %s (%d,%d)\n", Data::LOCATION_NAMES[location - 1], x, y);
				return true;
			}
		}
	}

	return true;
}

bool Console::cmdEnemy(int argc, const char **argv) {
	Data::Savegame &sg = _G(savegame);
	if (sg._mapNum != Data::MAP_OVERWORLD) {
		debugPrintf("Only supported on the overworld map\n");
		return true;
	}

	int typeIdx = (argc == 2) ? atoi(argv[1]) : 9;
	if (argc > 2 || typeIdx < 0 || typeIdx >= Data::MONSTER_TYPE_COUNT) {
		debugPrintf("enemy [<type>], where type is:\n");
		for (int i = 0; i < Data::MONSTER_TYPE_COUNT; ++i)
			debugPrintf("%2d: %s\n", i, Data::OVERWORLD_MONSTERS[i]);
		return true;
	}

	if (_G(overworldEntityCount) >= Data::OVERWORLD_ENTITY_COUNT - 1) {
		debugPrintf("No free entity slots\n");
		return true;
	}

	// Each type only appears on its own home terrain
	int terrain = (typeIdx < 4) ? Data::TILE_OCEAN : (typeIdx < 9) ? Data::TILE_WOODS : Data::TILE_GRASS;

	for (int radius = 2; radius <= 8; ++radius) {
		for (int dy = -radius; dy <= radius; ++dy) {
			for (int dx = -radius; dx <= radius; ++dx) {
				if (MAX(ABS(dx), ABS(dy)) != radius)
					continue;

				int x = sg._overworldPos.x + dx, y = sg._overworldPos.y + dy;
				if (x < 0 || x >= Data::OVERWORLD_WIDTH || y < 0 || y >= Data::OVERWORLD_HEIGHT ||
						_G(map).getTileAt(x, y) != terrain)
					continue;

				++_G(overworldEntityCount);
				auto &e = sg._overworldEntities[_G(overworldEntityCount)];
				e._type = Data::TILE_FIRST_MONSTER + typeIdx * 2;
				e._data = terrain;
				e._x = x;
				e._y = y;
				e._hits = Data::OVERWORLD_MONSTERS_HITS[typeIdx] * 10 + 2;
				++_G(creaturesCount);

				g_engine->focusedView()->redraw();
				debugPrintf("Spawned %s at (%d,%d)\n", Data::OVERWORLD_MONSTERS[typeIdx], x, y);
				return true;
			}
		}
	}

	debugPrintf("No suitable terrain for a %s nearby\n", Data::OVERWORLD_MONSTERS[typeIdx]);
	return true;
}

} // namespace Ultima1
} // namespace Ultima
