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

#ifndef ULTIMA1_DATA_SPACE_MAP_H
#define ULTIMA1_DATA_SPACE_MAP_H

#include "common/scummsys.h"
#include "common/serializer.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

// The galaxy is a 7x7 grid of sectors
constexpr int SPACE_SECTOR_COUNT = 7;

// The station sector, where the player arrives and can dock/land
constexpr int SPACE_STATION_X = 3;
constexpr int SPACE_STATION_Y = 3;

// Up to 4 ships can be present in a sector at once (parked fighters, the
// player's own ship, spawned attackers)
constexpr int SPACE_SHIPS_PER_SECTOR = 4;

// Direction names shown in the cockpit view, indexed by Data::Direction
// (1-4): the arrow keys read as Left / Right / Climb / Dive rather than the
// overhead view's West / East / North / South
extern const char *SPACE_COCKPIT_DIRECTION_NAMES[5];

// Ship types (SHIP_* in the disassembly)
enum SpaceShipType {
	SHIP_NONE = -1,
	SHIP_SHUTTLE = 0,			// the only type that can land back on Sosaria
	SHIP_LARGE_FIGHTER = 1,		// "Fighter 1" - high fuel
	SHIP_SMALL_FIGHTER = 2		// "Fighter 2" - heavy shields
};

/**
 * A single ship occupying one of a sector's ship slots
 */
struct SpaceMapShip {
	int16 _x = 0, _y = 0;
	int16 _shipType = SHIP_NONE;
	int16 _fuel = 0;
	int16 _shield = 0;
	int16 _facing = 0;

	void synchronize(Common::Serializer &s);
};

/**
 * One sector of the galaxy grid
 */
struct SpaceMapCell {
	// Hostile ships that will attack in this sector (0-3)
	int16 _enemyCount = 0;
	// A star / heat hazard - flying within 25 units destroys the ship.
	// (0, 0) means none in this sector
	int16 _hazardX = 0, _hazardY = 0;
	// The docking / encounter anchor that any ships in the sector cluster
	// around (the space station itself in sector 3,3). (0, 0) means the
	// sector has no ships
	int16 _anchorX = 0, _anchorY = 0;

	SpaceMapShip _ships[SPACE_SHIPS_PER_SECTOR];

	void synchronize(Common::Serializer &s);
};

/**
 * The whole galaxy. Regenerated from the savegame's random seed each time
 * the player lifts off into space (it isn't itself part of the savegame),
 * matching the original SPACE.EXE's setupSpaceMap
 */
struct SpaceMap {
private:
	uint16 _randomSeed = 0;

	/**
	 * The original's own LFSR, seeded from the savegame - kept separate
	 * from the engine RNG so a given seed always lays out the same galaxy
	 */
	int getRandomNumber(int minVal, int maxVal);

	/**
	 * Rolls the contents of one sector (setupSpaceSector)
	 */
	void setupSector(int x, int y);

public:
	// Indexed [x][y], matching the original's column-major layout
	SpaceMapCell _sectors[SPACE_SECTOR_COUNT][SPACE_SECTOR_COUNT];

	/**
	 * (Re)generates the whole galaxy
	 */
	void setup();

	/**
	 * Synchronizes the current state of every sector - called from the
	 * savegame while the player is actually in space, since the galaxy
	 * isn't just regenerated from the seed once ships/fuel/hazards diverge
	 */
	void synchronize(Common::Serializer &s);
};

// The cockpit view's warp starfield holds this many stars
constexpr int SPACE_STAR_COUNT = 40;

/**
 * The panning warp starfield drawn in the first-person cockpit view -
 * SPACE.EXE's setupStars / advanceStarfield. Purely visual (not saved)
 */
struct SpaceStarfield {
	// The viewport centre the stars radiate from and the aiming crosshair
	// sits at (viewCenterX/Y), and the pan velocity applied to it each
	// frame while steering (viewPanDeltaX/Y)
	int16 _centerX = 0x9f, _centerY = 0x4f;
	int16 _panX = 0, _panY = 0;

	// Each star is an offset from the centre that accelerates outward
	int16 _starX[SPACE_STAR_COUNT] = {};
	int16 _starY[SPACE_STAR_COUNT] = {};

	/**
	 * Fresh random stars, centre and pan back to their defaults (setupStars)
	 */
	void reset();

	/**
	 * One frame: every star drifts further from the centre (respawning at
	 * an edge once it leaves the viewport), then the centre pans by the
	 * current pan velocity, which is zeroed on hitting a viewport edge
	 * (advanceStarfield, normal-cruise path)
	 */
	void advance();

	/**
	 * True if a screen point is inside the cockpit viewport (starWithinView)
	 */
	static bool withinView(int x, int y) {
		return x > 20 && x < 300 && y > 20 && y < 140;
	}

private:
	void spawnStar(int index);
};

} // namespace Data
} // namespace Ultima1
} // namespace Ultima

#endif
