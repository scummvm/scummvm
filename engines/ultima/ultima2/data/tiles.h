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

#ifndef ULTIMA2_DATA_TILES_H
#define ULTIMA2_DATA_TILES_H

#include "common/archive.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

constexpr int TILE_COUNT = 64 + 1;
constexpr int TILE_WIDTH = 16;
constexpr int TILE_HEIGHT = 16;

enum TileId {
	kTileWater = 0,
	kTileSwamp = 1,
	kTileGrass = 2,
	kTileForest = 3,
	kTileMountain = 4,
	kTileUnknown5 = 5,
	kTileTown = 6,
	kTileTower = 7,
	kTileCastle = 8,
	kTileDungeonEntrance = 9,
	kTileSignpost = 10,
	kTileSeaMonster = 11,
	kTileOrc = 12,
	kTileDaemon = 13,
	kTileDevil = 14,
	kTileBalron = 15,
	kTileMinax = 16,
	kTileHorse = 17,
	kTileShip = 18,
	kTileAirplane = 19,
	kTileRocket = 20,
	kTileShield = 21,
	kTileSword = 22,
	kTileForcefield = 23,
	kTileGuard = 24,
	kTileJester = 25,
	kTileShopkeep = 26,
	kTileUnknown27 = 27,
	kTileRoad = 28,
	kTileEmpty = 29,
	kTileWall = 30,
	kTileEmptyCounterSpace = 31,
	kTileLetterA = 32,
	kTileLetterB = 33,
	kTileLetterC = 34,
	kTileLetterD = 35,
	kTileLetterE = 36,
	kTileLetterF = 37,
	kTileLetterG = 38,
	kTileLetterH = 39,
	kTileLetterIDoor = 40,
	kTileLetterJ = 41,
	kTileLetterK = 42,
	kTileLetterL = 43,
	kTileLetterM = 44,
	kTileLetterN = 45,
	kTileLetterO = 46,
	kTileLetterP = 47,
	kTileMoongate = 48,
	kTileLetterR = 49,
	kTileLetterS = 50,
	kTileLetterT = 51,
	kTileLetterU = 52,
	kTileLetterV = 53,
	kTileLetterW = 54,
	kTileLetterX = 55,
	kTileLetterY = 56,
	kTileLetterZ = 57,
	kTileCounterEndRight = 58,
	kTileCounterEndLeft = 59,
	kTileFighter = 60,
	kTileCleric = 61,
	kTileMage = 62,
	kTileThief = 63,
	// Special tiles
	kTileCircle = 64
};

/**
 * This archive class virtualizes converting the CGA tile data from
 * the ultimaii.exe file into a flat pixel (palette indexed) for each tile.
 * This will hopefully make it easier in the future if anyone wants to
 * create an enhanced graphic pack.. they'll just need to drop a 'tiles'
 * file in the game folder and implement a replacement palette for the game.
 */
class Tiles : public Common::Archive {
public:
	~Tiles() override {
	}

	/**
	 * Check if a member with the given @p name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.  This returns "true" for both files and directories.
	 */
	virtual bool hasFile(const Common::Path &path) const {
		return path.baseName().equalsIgnoreCase("tiles");
	}

	/**
	 * Add all members of the Archive to the list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return The number of names added to list.
	 */
	virtual int listMembers(Common::ArchiveMemberList &list) const {
		return 0;
	}

	/**
	 * Return an ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 *
	 * @return The newly created input stream.
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
