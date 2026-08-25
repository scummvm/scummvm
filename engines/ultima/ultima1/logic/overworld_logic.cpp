
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

#include "ultima/ultima1/logic/overworld_logic.h"
#include "ultima/ultima1/core/strings.h"
#include "ultima/ultima1/data/entity.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

enum {
	IMP_WATER, IMP_MOUNTAINS, IMP_WOODS, IMP_FRIGATES_WATER, IMP_RAFTS_WATER,
	IMP_BLOCKED_BY, IMP_MOVE_ON_LAND = 6
};

constexpr int OVERWORLD_MID_Y = Data::OVERWORLD_HEIGHT / 2 - 1;

static const char *IMPASSABLE_REASONS[] = {
	"You can't walk on water!",
	"Mountains are impassable!",
	"Aircars can't pass woods!",
	"Frigates like water!",
	"Rafts like water!"
};

const int8 DELTA_X[5] = { 0, -1, 1, 0, 0 };
const int8 DELTA_Y[5] = { 0, 0, 0, -1, 1 };

void OverworldLogic::action(int action) {
	switch (action) {
	case KEYBIND_UP:
		move(Data::DIR_UP);
		break;
	case KEYBIND_DOWN:
		move(Data::DIR_DOWN);
		break;
	case KEYBIND_LEFT:
		move(Data::DIR_LEFT);
		break;
	case KEYBIND_RIGHT:
		move(Data::DIR_RIGHT);
		break;
	default:
		break;
	}
}

void OverworldLogic::keypress(Common::KeyCode keycode) {

}

void OverworldLogic::move(Data::Direction dir) {
	if (!moveCheck(dir))
		return;

	int continentNum = g_engine->_map.getContinentAt(g_engine->_player._position);
	if (g_engine->_player._transportType <= Data::TRANSPORT_CART)
		playFX(4);

	// Write direction traveled
	writeString(Common::String::format("%s\n", Data::DIRECTION_NAMES[dir]));

	switch (dir) {
	case Data::DIR_LEFT:
		moveLeft();
		break;
	case Data::DIR_RIGHT:
		moveRight();
		break;
	case Data::DIR_UP:
		moveUp();
		break;
	case Data::DIR_DOWN:
		moveDown();
		break;
	}
}

bool OverworldLogic::moveCheck(Data::Direction dir) {
	int xp = getViewportX(Data::MAP_VISIBLE_CENTER_X + DELTA_X[dir]);
	int yp = getViewportY(Data::MAP_VISIBLE_CENTER_Y + DELTA_Y[dir]);
	int tile = getTileAt(xp, yp, 1);
	int transport = g_engine->_player._transportType;

	if (tile >= Data::TILE_FIRST_MONSTER && tile <= Data::TILE_LAST_MONSTER) {
		Common::String msg = Common::String::format("Blocked by a");
		const char *monsterName = Data::OVERWORLD_MONSTERS[(tile - Data::TILE_FIRST_MONSTER) / 2];
		if (isVowel(*monsterName))
			msg += '\n';
		msg += ' ';

		if (tile == Data::TILE_WANDERING_WARLOCK)
			msg += '\n';
		msg += monsterName;
		msg += "!\n";

		writeString(msg);
		g_engine->playFX(0);
		return false;

	} else if (tile == Data::TILE_MOUNTAINS) {
		impassable(IMP_MOUNTAINS);
		return false;

	} else if (tile == Data::TILE_OCEAN) {
		if (transport != Data::TRANSPORT_FRIGATE && transport != Data::TRANSPORT_RAFT &&
				transport != Data::TRANSPORT_AIRCAR) {
			impassable(IMP_WATER);
			return false;
		}
	} else if (tile == Data::TILE_WOODS && Data::TRANSPORT_AIRCAR) {
		impassable(IMP_WOODS);
		return false;

	} else if (transport == Data::TRANSPORT_FRIGATE && tile != Data::TILE_OCEAN) {
		impassable(IMP_FRIGATES_WATER);
		return false;

	} else if (transport == Data::TRANSPORT_RAFT && tile != Data::TILE_OCEAN) {
		impassable(IMP_RAFTS_WATER);
		return false;

	} else if (transport >= Data::TRANSPORT_SHUTTLE) {
		impassable(IMP_MOVE_ON_LAND);
		return false;
	}

	return true;
}

int OverworldLogic::getViewportX(int xp) const {
	int x = g_engine->_player._position.x - Data::MAP_VISIBLE_CENTER_X + xp;
	if (x < 0)
		x += Data::OVERWORLD_WIDTH;
	else if (x >= Data::OVERWORLD_WIDTH)
		x -= Data::OVERWORLD_WIDTH;

	return x;
}

int OverworldLogic::getViewportY(int yp) const {
	int y = g_engine->_player._position.y - Data::MAP_VISIBLE_CENTER_Y + yp;
	if (y < 0)
		y += Data::OVERWORLD_HEIGHT;
	else if (y >= Data::OVERWORLD_HEIGHT)
		y -= Data::OVERWORLD_HEIGHT;

	return y;
}

int OverworldLogic::getTileAt(int x, int y, int creatureIndex) const {
	int entityIndex = g_engine->_player.getEntityAt(x, y, creatureIndex);

	if (entityIndex >= 0)
		return g_engine->_player._overworldEntities[entityIndex]._type;

	return g_engine->_map.getMapTile(x, y);
}

void OverworldLogic::impassable(int reason) {
	writeString(Common::String::format("%s\n", IMPASSABLE_REASONS[reason]));
	playFX(0);
}

void OverworldLogic::moveLeft() {
	// Change overworld position
	Common::Point &pt = g_engine->_player._position;
	if (pt.x > 0) {
		pt.x--;
	} else {
		pt.x = Data::OVERWORLD_WIDTH - 1;
		pt.y = (pt.y >= OVERWORLD_MID_Y) ? pt.y - OVERWORLD_MID_Y : pt.y + OVERWORLD_MID_Y;
	}

	// Shift the existing columns right by one, to make room for a new
	// column of tiles at the left edge
	auto &tiles = g_engine->_map._mapTilesId;
	for (int col = Data::MAP_VISIBLE_WIDTH - 2; col >= 0; --col) {
		for (int row = 0; row < Data::MAP_VISIBLE_HEIGHT; ++row)
			tiles[row][col + 1] = tiles[row][col];
	}

	// Fill in the new left-hand column
	for (int row = 0; row < Data::MAP_VISIBLE_HEIGHT; ++row) {
		int x = getViewportX(0);
		int y = getViewportY(row);
		tiles[row][0] = g_engine->_map.getMapTile(x, y);
	}
}

void OverworldLogic::moveRight() {
	// Change overworld position
	Common::Point &pt = g_engine->_player._position;
	if (pt.x != Data::OVERWORLD_WIDTH - 1) {
		pt.x++;
	} else {
		pt.y = (pt.y >= OVERWORLD_MID_Y) ? pt.y - OVERWORLD_MID_Y : pt.y + OVERWORLD_MID_Y;
		pt.x = 0;
	}

	// Shift the existing columns left by one, to make room for a new
	// column of tiles at the right edge
	auto &tiles = g_engine->_map._mapTilesId;
	for (int col = 1; col < Data::MAP_VISIBLE_WIDTH; ++col) {
		for (int row = 0; row < Data::MAP_VISIBLE_HEIGHT; ++row)
			tiles[row][col - 1] = tiles[row][col];
	}

	// Fill in the new right-hand column
	for (int row = 0; row < Data::MAP_VISIBLE_HEIGHT; ++row) {
		int x = getViewportX(Data::MAP_VISIBLE_WIDTH - 1);
		int y = getViewportY(row);
		tiles[row][Data::MAP_VISIBLE_WIDTH - 1] = g_engine->_map.getMapTile(x, y);
	}
}

void OverworldLogic::moveUp() {
	// Change overworld position
	Common::Point &pt = g_engine->_player._position;
	pt.y = (pt.y == 0) ? Data::OVERWORLD_HEIGHT - 1 : pt.y - 1;

	// Shift the existing rows down by one, to make room for a new
	// row of tiles at the top edge
	auto &tiles = g_engine->_map._mapTilesId;
	for (int row = Data::MAP_VISIBLE_HEIGHT - 2; row >= 0; --row) {
		for (int col = 0; col < Data::MAP_VISIBLE_WIDTH; ++col)
			tiles[row + 1][col] = tiles[row][col];
	}

	// Fill in the new top row
	for (int col = 0; col < Data::MAP_VISIBLE_WIDTH; ++col) {
		int x = getViewportX(col);
		int y = getViewportY(0);
		tiles[0][col] = g_engine->_map.getMapTile(x, y);
	}
}

void OverworldLogic::moveDown() {
	// Change overworld position
	Common::Point &pt = g_engine->_player._position;
	pt.y = (pt.y == Data::OVERWORLD_HEIGHT - 1) ? 0 : pt.y + 1;

	// Shift the existing rows up by one, to make room for a new
	// row of tiles at the bottom edge
	auto &tiles = g_engine->_map._mapTilesId;
	for (int row = 1; row < Data::MAP_VISIBLE_HEIGHT; ++row) {
		for (int col = 0; col < Data::MAP_VISIBLE_WIDTH; ++col)
			tiles[row - 1][col] = tiles[row][col];
	}

	// Fill in the new bottom row
	for (int col = 0; col < Data::MAP_VISIBLE_WIDTH; ++col) {
		int x = getViewportX(col);
		int y = getViewportY(Data::MAP_VISIBLE_HEIGHT - 1);
		tiles[Data::MAP_VISIBLE_HEIGHT - 1][col] = g_engine->_map.getMapTile(x, y);
	}
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
