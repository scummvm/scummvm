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

#include "common/util.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

const char *SPACE_COCKPIT_DIRECTION_NAMES[5] = { nullptr, "Left", "Right", "Climb", "Dive" };

int SpaceMap::getRandomNumber(int minVal, int maxVal) {
	// seed = ror16(seed + 0x9248, 3)
	_randomSeed = (uint16)(_randomSeed + 0x9248);
	_randomSeed = (uint16)((_randomSeed >> 3) | (_randomSeed << 13));

	int clampedMax = (maxVal > 255) ? 255 : maxVal;
	int range = clampedMax - minVal;
	int result = _randomSeed & 0x3ff;
	while (result > range)
		result -= range;

	return (result + minVal) & 0xff;
}

void SpaceMap::setup() {
	_randomSeed = (uint16)((int)_G(savegame)._randomSeed * 3 / 2);

	for (int x = 0; x < SPACE_SECTOR_COUNT; ++x)
		for (int y = 0; y < SPACE_SECTOR_COUNT; ++y)
			_sectors[x][y] = SpaceMapCell();

	for (int x = 0; x < SPACE_SECTOR_COUNT; ++x)
		for (int y = 0; y < SPACE_SECTOR_COUNT; ++y)
			setupSector(x, y);

	// Sector (3,3) is always the space station: a fixed hazard, a fixed
	// docking anchor, and no roaming enemies
	SpaceMapCell &station = _sectors[SPACE_STATION_X][SPACE_STATION_Y];
	station._enemyCount = 0;
	station._hazardX = 140;
	station._hazardY = 70;
	station._anchorX = 200;
	station._anchorY = 100;

	// TODO: setupSpaceMap also parks two fighters (ships[0]/[1]) at the
	// station and places the player's own shuttle (ships[3], fuel/shield
	// from _savegame._shipFuel/_shipShield). Needed for docking and combat,
	// not for the sector scan
}

void SpaceMap::setupSector(int x, int y) {
	SpaceMapCell &cell = _sectors[x][y];

	for (int i = 0; i < SPACE_SHIPS_PER_SECTOR; ++i)
		cell._ships[i]._shipType = SHIP_NONE;

	cell._enemyCount = getRandomNumber(0, 240) / 40;
	if (cell._enemyCount > 3)
		cell._enemyCount = 0;

	int hazardX = 0, hazardY = 0, anchorX = 0, anchorY = 0;

	// ~38% of sectors have a star / heat hazard
	if (getRandomNumber(1, 200) < 77) {
		hazardX = getRandomNumber(50, 200);
		hazardY = getRandomNumber(50, 120);
	}
	cell._hazardX = hazardX;
	cell._hazardY = hazardY;

	// ~38% have an encounter anchor - discarded again if it would sit too
	// close to the hazard on either axis
	if (getRandomNumber(1, 200) < 77) {
		anchorX = getRandomNumber(50, 200);
		anchorY = getRandomNumber(50, 100);

		if (ABS(anchorX - hazardX) < 40 || ABS(anchorY - hazardY) < 40) {
			anchorX = 0;
			anchorY = 0;
		}
	}
	cell._anchorX = anchorX;
	cell._anchorY = anchorY;

	// TODO: with a non-zero anchor, setupSpaceSector then clusters 1-2
	// encounter ships around it (using the DELTA_X/DELTA_Y parking tables
	// and the checkOverlap* helpers). Not needed for the sector scan
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
