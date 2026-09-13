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

#include "ultima/ultima2/logic/overworld_logic.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

bool OverworldLogic::isWalkable(Data::TileId tile) const {
	switch (tile) {
	case Data::TILE_WATER:
	case Data::TILE_MOUNTAIN:
		return false;
	case Data::TILE_HORSE:
	case Data::TILE_SHIP:
	case Data::TILE_AIRPLANE:
	case Data::TILE_ROCKET:
	case Data::TILE_SHIELD:
	case Data::TILE_SWORD:
	case Data::TILE_FORCEFIELD:
	case Data::TILE_ROAD:
		return true;
	default:
		return tile <= Data::TILE_SIGNPOST;
	}
}

bool OverworldLogic::move(Data::Direction dir) {
	Data::Savegame &sg = _G(savegame);

	int newX = sg._mapX, newY = sg._mapY;
	const char *dirName;
	switch (dir) {
	case Data::DIR_UP:
		newY = (newY + Data::MAP_HEIGHT - 1) % Data::MAP_HEIGHT;
		dirName = "NORTH";
		break;
	case Data::DIR_DOWN:
		newY = (newY + 1) % Data::MAP_HEIGHT;
		dirName = "SOUTH";
		break;
	case Data::DIR_LEFT:
		newX = (newX + Data::MAP_WIDTH - 1) % Data::MAP_WIDTH;
		dirName = "WEST";
		break;
	case Data::DIR_RIGHT:
		newX = (newX + 1) % Data::MAP_WIDTH;
		dirName = "EAST";
		break;
	default:
		return true;
	}

	writeString("%s", dirName);

	if (sg._legParalysisTurns > 0) {
		writeString("--PARALIZED!--INVALID MOVE!\n");
		return true;
	}

	Data::TileId destTile = _G(map).tileAt(newX, newY);

	if (destTile == Data::TILE_SWAMP) {
		if (!sg.deductHP(5)) {
			playerDied();
			return false;
		}
	} else if (destTile == Data::TILE_FORCEFIELD) {
		if (sg._items[Data::ITEM_RING] != 0) {
			writeString("\nRING PROTECTS FROM FIELD!\n");
		} else {
			writeString("\nFIELD CAUSES 1000 DAMAGE!\n");
			if (!sg.deductHP(1000)) {
				playerDied();
				return false;
			}
		}
	}

	if (!sg.deductFood(25)) {
		playerDied();
		return false;
	}

	if (!isWalkable(destTile)) {
		writeString("--INVALID MOVE!\n");
		return true;
	}

	writeString("\n");
	sg._mapX = newX;
	sg._mapY = newY;
	return true;
}

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima
