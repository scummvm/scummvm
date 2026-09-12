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
			g_engine->send("Pillar", Shared::Messages::GameMessage("PILLAR", location - 41));
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

} // namespace Ultima1
} // namespace Ultima
