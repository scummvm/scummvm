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

#include "ultima/ultima3/logic/location_logic.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {

constexpr byte TILE_WATER = 0;
constexpr byte TILE_MOUNTAINS = 4;
constexpr byte TILE_SHIP_WATER = 12;
constexpr byte TILE_FORCE_FIELD = 0x20;
constexpr byte TILE_LAVA = 0x21;
constexpr byte TILE_MOON_GATE = 0x22;
constexpr byte TILE_SHALLOWS = 0x3E;
constexpr byte FIRST_BLOCKING_TILE = 12;

// Bits of a character's marks and cards that protect against terrain
constexpr byte MARK_FORCE = 0x10;
constexpr byte MARK_FIRE = 0x20;

// Map cells the party's mounts are left in, and the highest cell they can
// be left on, which is the ground they can travel over
constexpr byte CELL_WATER = 0;
constexpr byte CELL_GRASS = 4;
constexpr byte CELL_HORSE = TRANSPORT_HORSE * 4;
constexpr byte CELL_SHIP = TRANSPORT_SHIP * 4;

constexpr int FORCE_FIELD_DAMAGE = 99;
constexpr int LAVA_DAMAGE = 50;

bool LocationLogic::isShipBlockedByWind(Direction dir) const {
	if (_G(savegame)._transport != TRANSPORT_SHIP)
		return false;

	return _G(windDirection) == 0 || _G(windDirection) == (byte)dir;
}

bool LocationLogic::isTerrainBlocked(byte tile) {
	Data::Savegame &save = _G(savegame);

	if (save._transport == TRANSPORT_SHIP)
		return tile != TILE_WATER && tile != TILE_SHIP_WATER;

	switch (tile) {
	case TILE_FORCE_FIELD:
		// The whole party needs the Mark of Force to cross it. Otherwise the
		// first without it is hurt, and the field can't be crossed
		_G(effects).flashViewport();
		for (int slot = 0; slot < save._partySize; ++slot) {
			if (!(save.partyMember(slot)._marksAndCards & MARK_FORCE)) {
				damageCharacter(slot, FORCE_FIELD_DAMAGE);
				_G(effects).flashSlot(slot);
				return true;
			}
		}
		return false;

	case TILE_LAVA:
		// Anyone without the Mark of Fire is burnt, but it can be crossed
		for (int slot = 0; slot < save._partySize; ++slot) {
			Data::RosterEntry &e = save.partyMember(slot);

			if (e.isAlive() && !(e._marksAndCards & MARK_FIRE)) {
				damageCharacter(slot, LAVA_DAMAGE);
				_G(effects).flashSlot(slot);
			}
		}
		return false;

	case TILE_MOON_GATE:
	case TILE_SHALLOWS:
		return false;

	default:
		return tile >= FIRST_BLOCKING_TILE || tile == TILE_WATER || tile == TILE_MOUNTAINS;
	}
}

bool LocationLogic::move(Direction dir) {
	Data::Savegame &save = _G(savegame);
	int dx = (dir == DIR_EAST) ? 1 : (dir == DIR_WEST) ? -1 : 0;
	int dy = (dir == DIR_SOUTH) ? 1 : (dir == DIR_NORTH) ? -1 : 0;

	if (isShipBlockedByWind(dir))
		return false;
	if (isTerrainBlocked(_G(map).tile(save._posX + dx, save._posY + dy)))
		return false;

	save._posX = (save._posX + dx) & (Data::MAP_SIZE - 1);
	save._posY = (save._posY + dy) & (Data::MAP_SIZE - 1);
	return true;
}

// The towns, castles and dungeons on the world map, with the entrance of each
struct Entrance {
	const char *_filename;
	byte _x, _y;
};

static const Entrance ENTRANCES[] = {
	{ "BRITISH.ULT", 45, 18 }, { "EXODUS.ULT", 10, 53 }, { "LCB.ULT", 46, 19 },
	{ "MOON.ULT", 6, 13 }, { "YEW.ULT", 34, 16 }, { "MONTOR_E.ULT", 49, 58 },
	{ "MONTOR_W.ULT", 47, 58 }, { "GREY.ULT", 7, 44 }, { "DAWN.ULT", 37, 53 },
	{ "DEVIL.ULT", 18, 31 }, { "FAWN.ULT", 30, 2 }, { "DEATH.ULT", 56, 31 }
};

constexpr byte TILE_TOWN = 6;
constexpr byte TILE_CASTLE = 7;

// Where the party starts out in each kind of location
constexpr byte TOWN_START_X = 0x01;
constexpr byte TOWN_START_Y = 0x20;
constexpr byte CASTLE_START_X = 0x20;
constexpr byte CASTLE_START_Y = 0x3E;

const char *LocationLogic::enter() {
	Data::Savegame &save = _G(savegame);
	if (save._location != Data::LOCATION_SOSARIA)
		return nullptr;

	const Entrance *entrance = nullptr;
	for (uint i = 0; i < ARRAYSIZE(ENTRANCES) && !entrance; ++i) {
		if (ENTRANCES[i]._x == save._posX && ENTRANCES[i]._y == save._posY)
			entrance = &ENTRANCES[i];
	}

	byte tile = _G(map).tile(save._posX, save._posY);
	if (!entrance || (tile != TILE_TOWN && tile != TILE_CASTLE) || !Data::Map::exists(entrance->_filename))
		return nullptr;

	_G(worldMap) = _G(map);
	save._worldX = save._posX;
	save._worldY = save._posY;
	_G(map).load(entrance->_filename);

	if (tile == TILE_TOWN) {
		save._location = Data::LOCATION_TOWN;
		save._posX = TOWN_START_X;
		save._posY = TOWN_START_Y;
		return "Towne!\n";
	} else {
		save._location = Data::LOCATION_CASTLE;
		save._posX = CASTLE_START_X;
		save._posY = CASTLE_START_Y;
		return "Castle!\n";
	}
}

bool LocationLogic::isAtExit() const {
	const Data::Savegame &save = _G(savegame);
	return save._location >= Data::LOCATION_TOWN && (save._posX == 0 || save._posY == 0);
}

void LocationLogic::exitToWorld() {
	Data::Savegame &save = _G(savegame);

	_G(map) = _G(worldMap);
	save._location = Data::LOCATION_SOSARIA;
	save._posX = save._worldX;
	save._posY = save._worldY;
}

void LocationLogic::teleportRandomly() {
	Data::Savegame &save = _G(savegame);
	Graphics::Views::Events *events = Graphics::Views::g_events;

	for (;;) {
		int x = events->getRandomNumber(Data::MAP_SIZE - 1);
		int y = events->getRandomNumber(Data::MAP_SIZE - 1);
		byte cell = _G(map).cell(x, y);

		// Not into whirlpools, lava or force fields. As in the original, the
		// map cell rather than its tile is what's tested for being passable
		if (cell == 0x30 || cell == 0x80 || cell == 0x84 || isTerrainBlocked(cell))
			continue;

		save._posX = x;
		save._posY = y;
		return;
	}
}

const char *LocationLogic::board() {
	Data::Savegame &save = _G(savegame);
	if (save._transport != TRANSPORT_ON_FOOT)
		return nullptr;

	byte cell = _G(map).cell(save._posX, save._posY);
	if (cell == CELL_HORSE) {
		_G(map).setCell(save._posX, save._posY, CELL_GRASS);
		save._transport = TRANSPORT_HORSE;
		return "Mount Horse!\n";
	}
	if (cell == CELL_SHIP) {
		_G(map).setCell(save._posX, save._posY, CELL_WATER);
		save._transport = TRANSPORT_SHIP;
		return "Board Frigate!\n";
	}

	return nullptr;
}

LocationLogic::ExitResult LocationLogic::exitVehicle() {
	Data::Savegame &save = _G(savegame);
	if (save._transport == TRANSPORT_ON_FOOT)
		return EXIT_NOT_RIDING;

	if (_G(map).cell(save._posX, save._posY) > CELL_GRASS)
		return EXIT_NOT_HERE;

	_G(map).setCell(save._posX, save._posY, save._transport * 4);
	save._transport = TRANSPORT_ON_FOOT;
	return EXIT_DONE;
}

} // namespace Ultima3
} // namespace Ultima
