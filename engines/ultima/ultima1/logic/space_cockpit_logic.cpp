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
#include "ultima/ultima1/logic/space_cockpit_logic.h"
#include "ultima/ultima1/logic/space_map_logic.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/views/space_cockpit.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

// The Fire command's fuel cost (handleFireCommand)
constexpr int FIRE_FUEL_COST = 10;

constexpr int FIRE_HIT_XP = 100;
constexpr int SPACE_ACE_KILLS = 20;

// The viewport window an encounter target must stay inside; drifting past
// this (cockpitPerFrame) counts as passing by unengaged
constexpr int TARGET_MIN_X = 30, TARGET_MAX_X = 280;
constexpr int TARGET_MIN_Y = 24, TARGET_MAX_Y = 125;

// How many frames an encounter waits between rolls to advance its approach
// stage, and the roll range/threshold - both scale with cockpit speed
// (cockpitPerFrame)
constexpr int TARGET_APPROACH_TIMER_THRESHOLD = 15;
constexpr int TARGET_APPROACH_ROLL_THRESHOLD = 20;

// alienFiresBack's own odds: a ~1.5% chance per frame of a shot at all
// (roll < ALIEN_FIRE_ROLL_THRESHOLD out of ALIEN_FIRE_ROLL_MAX), and then a
// ~25% chance (reusing the SAME roll used to pick the shot's origin edge -
// only a shot arriving from the bottom edge can actually hit) of it landing
constexpr int ALIEN_FIRE_ROLL_MAX = 200;
constexpr int ALIEN_FIRE_ROLL_THRESHOLD = 4;
constexpr int ALIEN_FIRE_EDGE_ROLL_MAX = 255;
constexpr int ALIEN_FIRE_HIT_THRESHOLD = 0x40;
constexpr int ALIEN_FIRE_DAMAGE = 0x141;

void SpaceCockpitLogic::setSpeed(int speed) {
	writeString("Speed %d\n", speed);

	int reqFuel = ABS(speed - _G(cockpitSpeed)) * 4;
	if (reqFuel > shipFuel()) {
		writeString("Not enough fuel!\n");
	} else {
		_G(cockpitSpeed) = speed;
		subtractFuel(reqFuel);
	}

	prompt();
}

void SpaceCockpitLogic::keypress(Common::KeyCode keycode) {
	if (keycode >= Common::KEYCODE_1 && keycode <= Common::KEYCODE_8) {
		setSpeed(keycode - Common::KEYCODE_0);
		return;
	}

	SpaceLogic::keypress(keycode);
}

void SpaceCockpitLogic::tick() {
	// The original advances the starfield every cockpit frame; at 20fps
	// that's far too fast, so only every 3rd tick
	if (++_tickCounter < 3)
		return;
	_tickCounter = 0;

	_G(starfield).advance();
	tickEncounter();
	redrawMap();
}

void SpaceCockpitLogic::tickEncounter() {
	if (_G(cockpitTargetX) == 0)
		return;

	Data::SpaceStarfield &sf = _G(starfield);
	int deltaX = sf._centerX - _G(cockpitTargetX);
	int deltaY = sf._centerY - _G(cockpitTargetY);

	// The target only holds its screen position while the crosshair is
	// actively tracking it - otherwise it drifts a further pixel away each
	// frame, so lining up a shot means keeping the crosshair on it
	int stepX = (deltaX > 0) - (deltaX < 0);
	int stepY = (deltaY > 0) - (deltaY < 0);
	_G(cockpitTargetX) -= stepX;
	_G(cockpitTargetY) -= stepY;

	if (_G(cockpitTargetX) < TARGET_MIN_X || _G(cockpitTargetX) > TARGET_MAX_X ||
			_G(cockpitTargetY) < TARGET_MIN_Y || _G(cockpitTargetY) > TARGET_MAX_Y) {
		// Passed by unengaged
		setupSectorEnemies();
	} else if (_G(cockpitApproachTimer) <= TARGET_APPROACH_TIMER_THRESHOLD) {
		++_G(cockpitApproachTimer);
	} else {
		_G(cockpitApproachTimer) = 0;
		if (getRandomNumber(1, (_G(cockpitSpeed) + 3) * 10) > TARGET_APPROACH_ROLL_THRESHOLD) {
			if (++_G(cockpitTargetStage) >= Data::SPACE_TARGET_STAGE_COUNT)
				setupSectorEnemies();
		}
	}

	// Still runs even if the above just picked a fresh target (it starts at
	// stage 0, which alienFiresBack's own gate always skips) or cleared it
	alienFiresBack();
}

void SpaceCockpitLogic::alienFiresBack() {
	if (getRandomNumber(1, ALIEN_FIRE_ROLL_MAX) >= ALIEN_FIRE_ROLL_THRESHOLD)
		return;
	if (_G(cockpitTargetStage) == 0 || _G(cockpitTargetX) == 0)
		return;

	writeString("Alien fires!\n");

	// The same roll both picks which viewport edge the shot comes in from
	// and (only the bottom-edge range) whether it actually lands
	int roll = getRandomNumber(1, ALIEN_FIRE_EDGE_ROLL_MAX);
	int originX, originY;
	if (roll < 0x40) {
		originX = 159; originY = 135;		// bottom edge
	} else if (roll < 0x80) {
		originX = 159; originY = 20;		// top edge
	} else if (roll <= 0xC0) {
		originX = 290; originY = 79;		// right edge
	} else {
		originX = 20; originY = 79;		// left edge
	}

	int stage = _G(cockpitTargetStage);
	int endX = _G(cockpitTargetX) + Data::SPACE_TARGET_HIT_MAX_X[stage] / 2;
	int endY = _G(cockpitTargetY) + Data::SPACE_TARGET_HIT_MAX_Y[stage];

	Views::SpaceCockpit *view = dynamic_cast<Views::SpaceCockpit *>(g_engine->findView("SpaceCockpit"));
	assert(view);
	view->alienFireFlash(originX, originY, endX, endY);
	playFX(8);

	if (roll < ALIEN_FIRE_HIT_THRESHOLD) {
		playFX(2);
		writeString("You've been hit!\n");

		subtractShields(MIN(shipShields(), ALIEN_FIRE_DAMAGE));

		if (shipShields() == 0) {
			writeString("Thy shield is drained!\n");
			death();
			return;
		}
	}

	prompt();
}

bool SpaceCockpitLogic::move(Data::Direction dir) {
	writeString("%s\n", Data::SPACE_COCKPIT_DIRECTION_NAMES[dir]);

	if (shipFuel() < 2) {
		writeString("Not enough fuel!\n");
		return true;
	}
	subtractFuel(2);

	// Steer by setting the starfield pan velocity - the view's per-frame
	// update slides the centre and clamps it at the viewport edges
	Data::SpaceStarfield &sf = _G(starfield);
	switch (dir) {
	case Data::DIR_LEFT:
		sf._panX = -4;
		sf._panY = 0;
		break;
	case Data::DIR_RIGHT:
		sf._panX = 4;
		sf._panY = 0;
		break;
	case Data::DIR_UP:
		sf._panY = -4;
		sf._panX = 0;
		break;
	case Data::DIR_DOWN:
		sf._panY = 4;
		sf._panX = 0;
		break;
	default:
		break;
	}

	return true;
}

bool SpaceCockpitLogic::fire() {
	writeString("Fire!\n");

	if (shipFuel() < FIRE_FUEL_COST) {
		writeString("Thou hast not enough fuel!\n");
		return true;
	}
	subtractFuel(FIRE_FUEL_COST);

	// Flash a laser bolt in from each side of the viewport, converging on
	// the crosshair
	Views::SpaceCockpit *view = dynamic_cast<Views::SpaceCockpit *>(g_engine->findView("SpaceCockpit"));
	assert(view);
	view->fireLaser();
	playFX(8);

	int stage = _G(cockpitTargetStage);
	int deltaX = _G(starfield)._centerX - _G(cockpitTargetX);
	int deltaY = _G(starfield)._centerY - _G(cockpitTargetY);
	bool hit = _G(cockpitTargetX) != 0 &&
		deltaX >= Data::SPACE_TARGET_HIT_MIN_X[stage] && deltaX <= Data::SPACE_TARGET_HIT_MAX_X[stage] &&
		deltaY >= Data::SPACE_TARGET_HIT_MIN_Y[stage] && deltaY <= Data::SPACE_TARGET_HIT_MAX_Y[stage];

	if (hit) {
		playFX(2);
		writeString("Hit!!!\n");
		_G(savegame)._experience += FIRE_HIT_XP;
		++_G(savegame)._enemyVessels;

		if (_G(savegame)._enemyVessels == SPACE_ACE_KILLS)
			writeString("Thou hast achieved the rank of Space Ace!\n");
		else if (_G(savegame)._enemyVessels > SPACE_ACE_KILLS)
			writeString("Thou art still a Space Ace!\n");

		Data::SpaceMapCell &cell = _G(savegame)._starmap
			._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];
		--cell._enemyCount;
		setupSectorEnemies();
		redrawStats();
	}

	return true;
}

bool SpaceCockpitLogic::pass() {
	// Re-centers the starscape and stops any drift
	writeString("Center\n");

	Data::SpaceStarfield &sf = _G(starfield);
	sf._centerX = 0x9f;
	sf._centerY = 0x4f;
	sf._panX = sf._panY = 0;
	redrawMap();
	return true;
}

bool SpaceCockpitLogic::hyperjump() {
	// Left open (no newline) so the animation can append " Lightspeed!" to
	// the same line once it hits full scrolling speed
	writeString("HyperJump...");

	if (shipFuel() < 100) {
		writeString("Not enough fuel!\n");
		return true;
	}

	// Heads one sector over in whichever direction the ship is currently
	// facing on the overhead map - the original derives this from which
	// edge of the sector the ship exited, but facing is the equivalent
	// concept here, and doesn't require the ship to actually be moving
	Data::SpaceMapShip &ship = _G(savegame)._starmap
		._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]
		._ships[_G(savegame)._shipIndex];
	int dx = Data::SPACE_FACING_DX[ship._facing];
	int dy = Data::SPACE_FACING_DY[ship._facing];

	Views::SpaceCockpit *view = dynamic_cast<Views::SpaceCockpit *>(g_engine->findView("SpaceCockpit"));
	assert(view);
	view->startHyperjump(dx, dy);

	// The animation owns the commands area until it finishes or is
	// interrupted - suppress the usual immediate end-of-turn/prompt
	return false;
}

void SpaceCockpitLogic::setupSectorEnemies() {
	Data::SpaceMapCell &cell = _G(savegame)._starmap
		._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];

	if (cell._enemyCount > 0) {
		_G(cockpitTargetX) = getRandomNumber(40, 280);
		_G(cockpitTargetY) = getRandomNumber(40, 120);
		_G(cockpitTargetStage) = 0;
	} else {
		_G(cockpitTargetX) = 0;
		_G(cockpitTargetY) = 0;
	}
}

void SpaceCockpitLogic::hyperjumpLightspeed() {
	writeString(" Lightspeed!");
}

void SpaceCockpitLogic::completeHyperjump(int dx, int dy) {
	subtractFuel(100);

	int newSectorX = (_G(savegame)._sectorX + dx + Data::SPACE_SECTOR_COUNT) % Data::SPACE_SECTOR_COUNT;
	int newSectorY = (_G(savegame)._sectorY + dy + Data::SPACE_SECTOR_COUNT) % Data::SPACE_SECTOR_COUNT;

	Data::SpaceMapCell &oldCell = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];
	Data::SpaceMapCell &newCell = _G(savegame)._starmap._sectors[newSectorX][newSectorY];

	// Find a free ship slot in the destination sector to arrive in
	int freeSlot = -1;
	for (int i = 0; i < Data::SPACE_SHIPS_PER_SECTOR; ++i) {
		if (newCell._ships[i]._shipType == Data::SHIP_NONE) {
			freeSlot = i;
			break;
		}
	}

	resetLine();
	if (freeSlot == -1) {
		writeString("No room to arrive in that sector!\n");
	} else {
		// Move the player's ship record into the new sector's slot, vacating
		// the old one, and reposition to the same fixed point space entry uses
		Data::SpaceMapShip ship = oldCell._ships[_G(savegame)._shipIndex];
		oldCell._ships[_G(savegame)._shipIndex] = Data::SpaceMapShip();

		ship._x = 260;
		ship._y = 90;
		ship._facing = Data::FACING_LEFT;
		newCell._ships[freeSlot] = ship;

		_G(savegame)._sectorX = (int16)newSectorX;
		_G(savegame)._sectorY = (int16)newSectorY;
		_G(savegame)._shipIndex = (int16)freeSlot;
		setupSectorEnemies();

		writeString("\x10HyperJump completed.\n");
	}

	redrawMap();
	redrawStats();
	prompt();
}

void SpaceCockpitLogic::abortHyperjump() {
	resetLine();
	writeString("Hyperjump aborted!\n");
	redrawMap();
	prompt();
}

bool SpaceCockpitLogic::view() {
	writeString("View\n");

	// TODO: the original refuses this switch back to the overhead view
	// while any enemies remain in the sector ("Thou must eliminate all
	// enemy craft first!")
	_G(logic) = Common::SharedPtr<Logic>(new SpaceMapLogic());
	g_engine->replaceView("SpaceMap");
	return true;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
