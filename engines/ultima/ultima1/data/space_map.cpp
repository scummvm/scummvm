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

const int16 SPACE_FACING_DX[4] = { -1, 1, 0, 0 };
const int16 SPACE_FACING_DY[4] = { 0, 0, -1, 1 };

// Edge order: 0=left, 1=right, 2=top, 3=bottom. ShipType order: Shuttle,
// LargeFighter, SmallFighter
const int16 SPACE_DOCK_DELTA_X[SPACE_DOCK_EDGE_COUNT][3] = {
	{ -17, -15, -15 },
	{ 18, 18, 18 },
	{ 0, 1, 2 },
	{ 0, 1, 2 }
};
const int16 SPACE_DOCK_DELTA_Y[SPACE_DOCK_EDGE_COUNT][3] = {
	{ 1, 1, 1 },
	{ 1, 1, 1 },
	{ -17, -16, -16 },
	{ 18, 17, 17 }
};
const SpaceShipFacing SPACE_DOCK_EDGE_FACING[SPACE_DOCK_EDGE_COUNT] = {
	FACING_RIGHT, FACING_LEFT, FACING_DOWN, FACING_UP
};
const char *SPACE_DOCK_EDGE_NAMES[SPACE_DOCK_EDGE_COUNT] = { "left", "right", "top", "bottom" };

int SpaceMapCell::dockedEdge(int shipIndex) const {
	const SpaceMapShip &ship = _ships[shipIndex];
	if (ship._shipType == SHIP_NONE)
		return -1;

	for (int edge = 0; edge < SPACE_DOCK_EDGE_COUNT; ++edge) {
		int expectedX = _anchorX + SPACE_DOCK_DELTA_X[edge][ship._shipType];
		int expectedY = _anchorY + SPACE_DOCK_DELTA_Y[edge][ship._shipType];
		if (ship._x == expectedX && ship._y == expectedY && ship._facing == SPACE_DOCK_EDGE_FACING[edge])
			return edge;
	}

	return -1;
}

int SpaceMapCell::shipAtEdge(int edge) const {
	for (int i = 0; i < SPACE_SHIPS_PER_SECTOR; ++i) {
		if (dockedEdge(i) == edge)
			return i;
	}

	return -1;
}

int16 shipFuelCapacity(int shipType) {
	switch (shipType) {
	case SHIP_SHUTTLE: return 1000;
	case SHIP_LARGE_FIGHTER: return 5000;
	case SHIP_SMALL_FIGHTER: return 2500;
	default: return 0;
	}
}

int16 shipShieldCapacity(int shipType) {
	switch (shipType) {
	case SHIP_SHUTTLE: return 1000;
	case SHIP_LARGE_FIGHTER: return 1000;
	case SHIP_SMALL_FIGHTER: return 5000;
	default: return 0;
	}
}

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

	// Two fighters are permanently parked at the station, docked at fixed
	// offsets from the anchor (setupSpaceMap's own DELTA_X/DELTA_Y-derived
	// positions for the bottom and left docking edges respectively)
	SpaceMapShip &smallFighter = station._ships[0];
	smallFighter._shipType = SHIP_SMALL_FIGHTER;
	smallFighter._fuel = 2500;
	smallFighter._shield = 5000;
	smallFighter._x = (int16)(station._anchorX + 2);
	smallFighter._y = (int16)(station._anchorY + 17);
	smallFighter._facing = FACING_UP;

	SpaceMapShip &largeFighter = station._ships[1];
	largeFighter._shipType = SHIP_LARGE_FIGHTER;
	largeFighter._fuel = 5000;
	largeFighter._shield = 1000;
	largeFighter._x = (int16)(station._anchorX - 15);
	largeFighter._y = (int16)(station._anchorY + 1);
	largeFighter._facing = FACING_RIGHT;

	// The player's own shuttle (ships[2]) sits at a fixed dock position of
	// its own, not derived from the anchor - see SpaceLogic::entering()
}

void SpaceMapShip::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_x);
	s.syncAsSint16LE(_y);
	s.syncAsSint16LE(_shipType);
	s.syncAsSint16LE(_fuel);
	s.syncAsSint16LE(_shield);
	s.syncAsSint16LE(_facing);
}

void SpaceMapCell::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_enemyCount);
	s.syncAsSint16LE(_hazardX);
	s.syncAsSint16LE(_hazardY);
	s.syncAsSint16LE(_anchorX);
	s.syncAsSint16LE(_anchorY);

	for (int i = 0; i < SPACE_SHIPS_PER_SECTOR; ++i)
		_ships[i].synchronize(s);
}

void SpaceMap::synchronize(Common::Serializer &s) {
	for (int x = 0; x < SPACE_SECTOR_COUNT; ++x)
		for (int y = 0; y < SPACE_SECTOR_COUNT; ++y)
			_sectors[x][y].synchronize(s);
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

void SpaceStarfield::spawnStar(int index) {
	// A random quadrant and a random offset magnitude, so the star starts
	// somewhere near an edge and heads outward
	int quadrant = g_engine->getRandomNumber(0, 255) / 64;
	int dx = g_engine->getRandomNumber(2, 255) / 2;
	int dy = g_engine->getRandomNumber(4, 255) / 4;

	if (quadrant == 1 || quadrant == 2)
		dx = -dx;
	if (quadrant > 1)
		dy = -dy;

	// If that would already be off-view, park it far away so next frame
	// just respawns it
	if (!withinView(_centerX + dx, _centerY + dy)) {
		dx = 5000;
		dy = 5000;
	}

	_starX[index] = (int16)dx;
	_starY[index] = (int16)dy;
}

void SpaceStarfield::reset() {
	_centerX = 0x9f;
	_centerY = 0x4f;
	_panX = _panY = 0;

	for (int i = 0; i < SPACE_STAR_COUNT; ++i)
		spawnStar(i);
}

void SpaceStarfield::advance() {
	for (int i = 0; i < SPACE_STAR_COUNT; ++i) {
		int stepX = _starX[i] / 8;
		int stepY = _starY[i] / 6;

		// Always move at least one pixel in the star's own direction
		if (stepX == 0)
			stepX = (_starX[i] > 0) - (_starX[i] < 0);
		if (stepY == 0)
			stepY = (_starY[i] > 0) - (_starY[i] < 0);

		_starX[i] = (int16)(_starX[i] + stepX);
		_starY[i] = (int16)(_starY[i] + stepY);

		if (!withinView(_centerX + _starX[i], _centerY + _starY[i]))
			spawnStar(i);
	}

	// Pan the centre, stopping dead (and dropping the pan) at a viewport edge
	_centerX = (int16)(_centerX + _panX);
	if (_centerX >= 300 || _centerX <= 20) {
		_centerX = (int16)(_centerX - _panX);
		_panX = 0;
	}
	_centerY = (int16)(_centerY + _panY);
	if (_centerY >= 140 || _centerY <= 20) {
		_centerY = (int16)(_centerY - _panY);
		_panY = 0;
	}
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
