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
 * GNU General Public License for more detailsvvvvv.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <htvvvtp://www.gnu.org/licenses/>.
 *
 */

#include "common/util.h"
#include "ultima/ultima1/views/space_cockpit.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/logic/space_cockpit_logic.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

constexpr int FRAME_COLOR = 7;		// light grey cockpit surround
constexpr int STAR_COLOR = 15;		// white stars / crosshair
constexpr int RIVET_COLOR = 0;		// black detail punched into the frame
constexpr int LASER_COLOR = 12;	// light red laser-bolt flash

// How many ticks the laser-bolt flash from fireLaser() stays on-screen
constexpr int LASER_FLASH_TICKS = 3;

// The two fixed points at the bottom corners of the viewport the laser
// bolts converge in from (handleFireCommand's own (20,140)/(300,140))
constexpr int LASER_ORIGIN_LEFT_X = 20, LASER_ORIGIN_RIGHT_X = 300, LASER_ORIGIN_Y = 140;

bool SpaceCockpit::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_SPACE);
	_G(starfield).reset();
	return Map::msgFocus(msg);
}

bool SpaceCockpit::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);

	_hyperjumpState = HYPERJUMP_IDLE;
	_trails.free();

	return Map::msgUnfocus(msg);
}

void SpaceCockpit::draw() {
	Map::draw();

	auto s = getSurface();
	s.fillRect(Common::Rect(8, 8, 312, 152), 0);

	drawCockpitFrame(s);

	const Data::SpaceStarfield &sf = _G(starfield);

	// The aiming crosshair marking the viewport centre
	s.drawLine(sf._centerX - 2, sf._centerY, sf._centerX + 2, sf._centerY, STAR_COLOR);
	s.drawLine(sf._centerX, sf._centerY - 2, sf._centerX, sf._centerY + 2, STAR_COLOR);

	if (_hyperjumpState == HYPERJUMP_WARP_IN || _hyperjumpState == HYPERJUMP_WARP_OUT) {
		// The stars along with all the trails behind them
		s.transBlitFrom(_trails, Common::Point(0, 0), 0);
	} else {
		// The warp stars
		for (int i = 0; i < Data::SPACE_STAR_COUNT; ++i) {
			int x = sf._centerX + sf._starX[i];
			int y = sf._centerY + sf._starY[i];
			if (Data::SpaceStarfield::withinView(x, y))
				s.setPixel(x, y, STAR_COLOR);
		}
	}

	// The enemy fighter currently being engaged, if any
	if (_G(cockpitTargetX) != 0) {
		const Graphics::ManagedSurface &fighter = _G(map).enemyFighterTiles()[_G(cockpitTargetStage)];
		s.transBlitFrom(fighter, Common::Point(_G(cockpitTargetX), _G(cockpitTargetY)), 0);
	}

	// The brief laser-bolt flash from a shot just fired, converging on the
	// crosshair from both bottom corners of the viewport
	if (_laserFlashTicks > 0) {
		s.drawLine(LASER_ORIGIN_LEFT_X, LASER_ORIGIN_Y, sf._centerX, sf._centerY, LASER_COLOR);
		s.drawLine(LASER_ORIGIN_RIGHT_X, LASER_ORIGIN_Y, sf._centerX, sf._centerY, LASER_COLOR);
	}

	// The brief laser-bolt flash from the current target firing back
	if (_alienFlashTicks > 0)
		s.drawLine(_alienFlashX1, _alienFlashY1, _alienFlashX2, _alienFlashY2, LASER_COLOR);
}

bool SpaceCockpit::msgMouseMove(const MouseMoveMessage &msg) {
	// The warp's stars are measured from the centre, so leave it where it is
	if (_hyperjumpState != HYPERJUMP_IDLE)
		return true;

	Data::SpaceStarfield &sf = _G(starfield);
	sf._panX = 0;
	sf._panY = 0;
	sf._centerX = (int16)CLIP<int>(msg._pos.x, 21, 299);
	sf._centerY = (int16)CLIP<int>(msg._pos.y, 21, 139);
	redraw();
	return true;
}

bool SpaceCockpit::msgMouseDown(const MouseDownMessage &msg) {
	if (!cancelHyperjump())
		_G(logic)->action(KEYBIND_FIRE);
	return true;
}

void SpaceCockpit::fireLaser() {
	_laserFlashTicks = LASER_FLASH_TICKS;
	redraw();
}

void SpaceCockpit::alienFireFlash(int x1, int y1, int x2, int y2) {
	_alienFlashTicks = LASER_FLASH_TICKS;
	_alienFlashX1 = x1; _alienFlashY1 = y1;
	_alienFlashX2 = x2; _alienFlashY2 = y2;
	redraw();
}

constexpr int HYPERJUMP_MAX_SPEED = Data::SPACE_COCKPIT_MAX_SPEED;

// Frames cruised at each speed, depending on whether the ship started out
// slow or fast
constexpr int HYPERJUMP_SLOW_LEVEL_FRAMES = 11;
constexpr int HYPERJUMP_FAST_LEVEL_FRAMES = 16;
constexpr int HYPERJUMP_FAST_SPEED = 4;

// Frames of each half of the warp: the first leaves the trails, the second
// erases them again
constexpr int HYPERJUMP_WARP_IN_FRAMES = 201;
constexpr int HYPERJUMP_WARP_OUT_FRAMES = 200;

// The warp itself runs flat out in the original, so this is chosen to take a
// few seconds
constexpr int HYPERJUMP_WARP_FRAMES_PER_TICK = 4;

void SpaceCockpit::startHyperjump(int dx, int dy) {
	_hyperjumpState = HYPERJUMP_ACCELERATE;
	_hyperjumpDX = dx;
	_hyperjumpDY = dy;
	_hyperjumpOrigSpeed = _G(cockpitSpeed);
	_hyperjumpLevelFrames = (_hyperjumpOrigSpeed < HYPERJUMP_FAST_SPEED) ?
		HYPERJUMP_SLOW_LEVEL_FRAMES : HYPERJUMP_FAST_LEVEL_FRAMES;
	_hyperjumpFrames = _hyperjumpLevelFrames;
	_hyperjumpBudget = 0;

	// Stop any manual steering - the animation takes over the view
	_G(starfield)._panX = 0;
	_G(starfield)._panY = 0;
}

void SpaceCockpit::startWarp() {
	// The ship has stopped, as far as the stars are concerned
	_G(cockpitSpeed) = 0;

	_trails.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_trails.clear(0);

	auto *logic = dynamic_cast<Logic::SpaceCockpitLogic *>(_G(logic).get());
	if (logic)
		logic->hyperjumpLightspeed();

	_hyperjumpSeed = g_engine->getRandomNumber(0x7FFFFF);
	_G(starfield).reseed(_hyperjumpSeed);
	_hyperjumpState = HYPERJUMP_WARP_IN;
	_hyperjumpFrames = HYPERJUMP_WARP_IN_FRAMES;
}

void SpaceCockpit::plotTrail(int x, int y, int color) {
	if (x >= 0 && x < _trails.w && y >= 0 && y < _trails.h)
		*(byte *)_trails.getBasePtr(x, y) = (byte)color;
}

void SpaceCockpit::finishHyperjump() {
	_hyperjumpState = HYPERJUMP_IDLE;
	_G(cockpitSpeed) = _hyperjumpOrigSpeed;

	auto *logic = dynamic_cast<Logic::SpaceCockpitLogic *>(_G(logic).get());
	if (logic)
		logic->completeHyperjump(_hyperjumpDX, _hyperjumpDY);
}

void SpaceCockpit::hyperjumpFrame() {
	Data::SpaceStarfield &sf = _G(starfield);

	switch (_hyperjumpState) {
	case HYPERJUMP_ACCELERATE:
		sf.advance();

		if (--_hyperjumpFrames == 0) {
			if (_G(cockpitSpeed) < HYPERJUMP_MAX_SPEED) {
				++_G(cockpitSpeed);
				_hyperjumpFrames = _hyperjumpLevelFrames;
			} else {
				startWarp();
			}
		}
		break;

	case HYPERJUMP_WARP_IN:
		// Nothing is erased, so every star leaves its whole path behind it
		sf.advance();
		for (int i = 0; i < Data::SPACE_STAR_COUNT; ++i) {
			if (sf._visible[i])
				plotTrail(sf._prevX[i], sf._prevY[i], STAR_COLOR);
		}

		if (--_hyperjumpFrames == 0) {
			// Replaying the same stars, each wipes out the pixels it left
			sf.reseed(_hyperjumpSeed);
			_hyperjumpState = HYPERJUMP_WARP_OUT;
			_hyperjumpFrames = HYPERJUMP_WARP_OUT_FRAMES;
		}
		break;

	case HYPERJUMP_WARP_OUT:
		for (int i = 0; i < Data::SPACE_STAR_COUNT; ++i) {
			if (Data::SpaceStarfield::withinView(sf._prevX[i], sf._prevY[i]))
				plotTrail(sf._prevX[i], sf._prevY[i], 0);
		}

		sf.advance();
		for (int i = 0; i < Data::SPACE_STAR_COUNT; ++i) {
			if (sf._visible[i])
				plotTrail(sf._prevX[i], sf._prevY[i], STAR_COLOR);
		}

		if (--_hyperjumpFrames == 0) {
			_trails.free();
			_hyperjumpBudget = 0;

			if (_hyperjumpOrigSpeed < HYPERJUMP_MAX_SPEED) {
				_G(cockpitSpeed) = HYPERJUMP_MAX_SPEED;
				_hyperjumpState = HYPERJUMP_DECELERATE;
				_hyperjumpFrames = _hyperjumpLevelFrames;
			} else {
				finishHyperjump();
			}
		}
		break;

	case HYPERJUMP_DECELERATE:
		sf.advance();

		if (--_hyperjumpFrames == 0) {
			if (_G(cockpitSpeed) - 1 > _hyperjumpOrigSpeed) {
				--_G(cockpitSpeed);
				_hyperjumpFrames = _hyperjumpLevelFrames;
			} else {
				finishHyperjump();
			}
		}
		break;

	default:
		break;
	}
}

void SpaceCockpit::tickHyperjump() {
	_hyperjumpBudget += Data::SPACE_COCKPIT_VBLANKS_PER_TICK;
	int warpFrames = HYPERJUMP_WARP_FRAMES_PER_TICK;

	while (_hyperjumpState != HYPERJUMP_IDLE) {
		if (_hyperjumpState == HYPERJUMP_WARP_IN || _hyperjumpState == HYPERJUMP_WARP_OUT) {
			if (warpFrames-- == 0)
				break;
		} else {
			int delay = Data::SPACE_COCKPIT_FRAME_DELAY[_G(cockpitSpeed)];
			if (delay > _hyperjumpBudget)
				break;
			_hyperjumpBudget -= delay;
		}

		hyperjumpFrame();
	}

	redraw();
}

bool SpaceCockpit::tick() {
	if (_hyperjumpState != HYPERJUMP_IDLE)
		tickHyperjump();

	if (_laserFlashTicks > 0 && --_laserFlashTicks == 0)
		redraw();
	if (_alienFlashTicks > 0 && --_alienFlashTicks == 0)
		redraw();

	return Map::tick();
}

bool SpaceCockpit::cancelHyperjump() {
	if (_hyperjumpState == HYPERJUMP_IDLE)
		return false;

	_hyperjumpState = HYPERJUMP_IDLE;
	_trails.free();
	_G(cockpitSpeed) = _hyperjumpOrigSpeed;

	auto *logic = dynamic_cast<Logic::SpaceCockpitLogic *>(_G(logic).get());
	if (logic)
		logic->abortHyperjump();
	return true;
}

bool SpaceCockpit::msgAction(const ActionMessage &msg) {
	return cancelHyperjump() || Map::msgAction(msg);
}

bool SpaceCockpit::msgKeypress(const KeypressMessage &msg) {
	return cancelHyperjump() || Map::msgKeypress(msg);
}

void SpaceCockpit::drawCockpitFrame(Graphics::Views::GfxSurface &s) {
	// The ~7px surround around the viewport
	s.fillRect(Common::Rect(8, 8, 312, 16), FRAME_COLOR);
	s.fillRect(Common::Rect(8, 144, 312, 152), FRAME_COLOR);
	s.fillRect(Common::Rect(8, 16, 16, 145), FRAME_COLOR);
	s.fillRect(Common::Rect(304, 16, 312, 145), FRAME_COLOR);

	// Rivet detail along the top and bottom
	for (int i = 1; i < 20; ++i) {
		int px = i * 15 + 10;
		s.setPixel(px, 10, RIVET_COLOR);
		s.setPixel(px, 13, RIVET_COLOR);
		s.setPixel(px, 146, RIVET_COLOR);
		s.setPixel(px, 149, RIVET_COLOR);
	}
	// ...and down the sides
	for (int i = 1; i < 10; ++i) {
		int py = i * 15 + 3;
		s.setPixel(10, py, RIVET_COLOR);
		s.setPixel(13, py, RIVET_COLOR);
		s.setPixel(306, py, RIVET_COLOR);
		s.setPixel(309, py, RIVET_COLOR);
	}
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
