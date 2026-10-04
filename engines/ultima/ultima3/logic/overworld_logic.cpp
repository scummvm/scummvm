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

#include "ultima/ultima3/logic/overworld_logic.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {

constexpr byte TILE_WATER = 0;
constexpr byte TILE_MOUNTAINS = 4;
constexpr byte TILE_SHIP_WATER = 12;
constexpr byte TILE_FIRE = 0x20;
constexpr byte TILE_FORCE_FIELD = 0x21;
constexpr byte TILE_WHIRLPOOL = 0x22;
constexpr byte TILE_SHALLOWS = 0x3E;
constexpr byte FIRST_BLOCKING_TILE = 12;

// Bits of a character's marks and cards that protect against terrain
constexpr byte MARK_FIRE = 0x10;
constexpr byte MARK_FORCE = 0x20;

constexpr int FIRE_DAMAGE = 99;
constexpr int FORCE_FIELD_DAMAGE = 50;

bool OverworldLogic::isShipBlockedByWind(Direction dir) const {
	if (_G(savegame)._transport != TRANSPORT_SHIP)
		return false;

	return _G(windDirection) == 0 || _G(windDirection) == (byte)dir;
}

bool OverworldLogic::isTerrainBlocked(byte tile) {
	Data::Savegame &save = _G(savegame);

	if (save._transport == TRANSPORT_SHIP)
		return tile != TILE_WATER && tile != TILE_SHIP_WATER;

	switch (tile) {
	case TILE_FIRE:
		// The whole party needs the Mark of Fire to cross it. Otherwise the
		// first without it is burnt, and the fire can't be crossed
		_G(effects).flashViewport();
		for (int slot = 0; slot < save._partySize; ++slot) {
			if (!(save.partyMember(slot)._marksAndCards & MARK_FIRE)) {
				damageCharacter(slot, FIRE_DAMAGE);
				_G(effects).flashSlot(slot);
				return true;
			}
		}
		return false;

	case TILE_FORCE_FIELD:
		// Anyone without the Mark of Force is hurt, but it can be crossed
		for (int slot = 0; slot < save._partySize; ++slot) {
			Data::RosterEntry &e = save.partyMember(slot);

			if (e.isAlive() && !(e._marksAndCards & MARK_FORCE)) {
				damageCharacter(slot, FORCE_FIELD_DAMAGE);
				_G(effects).flashSlot(slot);
			}
		}
		return false;

	case TILE_WHIRLPOOL:
	case TILE_SHALLOWS:
		return false;

	default:
		return tile >= FIRST_BLOCKING_TILE || tile == TILE_WATER || tile == TILE_MOUNTAINS;
	}
}

bool OverworldLogic::move(Direction dir) {
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

} // namespace Ultima3
} // namespace Ultima
