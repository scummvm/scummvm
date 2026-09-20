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

#ifndef ULTIMA2_DATA_MAP_H
#define ULTIMA2_DATA_MAP_H

#include "common/array.h"
#include "common/serializer.h"
#include "common/str.h"
#include "ultima/ultima2/data/tiles.h"
#include "ultima/ultima2/data/map_monsters.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

constexpr int MAP_WIDTH = 64;
constexpr int MAP_HEIGHT = 64;

// Both build "MAPXnn"/"MONXnn" from the same two map-number digits
Common::String mapFilename(int mapEra, int mapType);
Common::String monsterFilename(int mapEra, int mapType);
Common::String talkFilename(int mapEra, int mapType);

/**
 * A planet, village, town, or castle's map: a flat 64x64 grid of tiles,
 * plus its monster/NPC slots. Towers and dungeons share the same on-disk
 * mapX??/monX?? files but use a different internal layout for the map
 * data itself -- see MapDungeon.
 */
struct Map {
	TileId _tiles[MAP_HEIGHT][MAP_WIDTH] = {};
	MapMonsters _monsters;
	Common::Array<Common::String> _talk;

private:
	void loadTalk(int mapEra, int mapType);

	/**
	 * Loads a tower or dungeon and its monsters, and enters it
	 */
	void loadDungeon(int mapEra, int mapType);

	/**
	 * Switches to the given map view, first setting up the game view if
	 * that isn't already showing
	 */
	void showMapView(const Common::String &mapViewName);

public:
	void load(int mapEra, int mapType);

	/**
	 * Saves or restores the current map's contents and monsters as they
	 * are now, including changes made during play. Loading expects the
	 * map to have been loaded first
	 */
	void synchronize(Common::Serializer &s, bool isDungeon);

	/**
	 * Returns the given line of the map's NPC dialogue, or an empty string
	 * if there isn't one
	 */
	Common::String talkString(int index) const {
		return (index >= 0 && index < (int)_talk.size()) ? _talk[index] : Common::String();
	}

	TileId tileAt(int x, int y) const {
		return _tiles[y][x];
	}
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
