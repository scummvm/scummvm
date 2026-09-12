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
	return Map::msgUnfocus(msg);
}

void SpaceCockpit::draw() {
	Map::draw();

	auto s = getSurface();
	s.fillRect(Rect(8, 8, 312, 152), 0);

	drawCockpitFrame(s);

	const Data::SpaceStarfield &sf = _G(starfield);

	// The aiming crosshair marking the viewport centre
	s.drawLine(sf._centerX - 2, sf._centerY, sf._centerX + 2, sf._centerY, STAR_COLOR);
	s.drawLine(sf._centerX, sf._centerY - 2, sf._centerX, sf._centerY + 2, STAR_COLOR);

	// The warp stars
	for (int i = 0; i < Data::SPACE_STAR_COUNT; ++i) {
		int x = sf._centerX + sf._starX[i];
		int y = sf._centerY + sf._starY[i];
		if (Data::SpaceStarfield::withinView(x, y))
			s.setPixel(x, y, STAR_COLOR);
	}

	// The brief laser-bolt flash from a shot just fired, converging on the
	// crosshair from both bottom corners of the viewport
	if (_laserFlashTicks > 0) {
		s.drawLine(LASER_ORIGIN_LEFT_X, LASER_ORIGIN_Y, sf._centerX, sf._centerY, LASER_COLOR);
		s.drawLine(LASER_ORIGIN_RIGHT_X, LASER_ORIGIN_Y, sf._centerX, sf._centerY, LASER_COLOR);
	}
}

bool SpaceCockpit::msgMouseMove(const MouseMoveMessage &msg) {
	Data::SpaceStarfield &sf = _G(starfield);
	sf._panX = 0;
	sf._panY = 0;
	sf._centerX = (int16)CLIP<int>(msg._pos.x, 21, 299);
	sf._centerY = (int16)CLIP<int>(msg._pos.y, 21, 139);
	redraw();
	return true;
}

bool SpaceCockpit::msgMouseDown(const MouseDownMessage &msg) {
	_G(logic)->action(KEYBIND_FIRE);
	return true;
}

void SpaceCockpit::fireLaser() {
	_laserFlashTicks = LASER_FLASH_TICKS;
	redraw();
}

// Ticks per +/-1 step while ramping the display speed up/down
constexpr int HYPERJUMP_SPEED_STEP_TICKS = 2;
// How long (in ticks) to hold at full warp, streaking the starfield
constexpr int HYPERJUMP_WARP_TICKS = 30;
// Extra starfield advances per tick while at full warp, on top of the
// normal per-frame one - this is the "speeded up" part of the animation
constexpr int HYPERJUMP_WARP_ADVANCES = 5;

void SpaceCockpit::startHyperjump(int dx, int dy) {
	_hyperjumpState = HYPERJUMP_RAMP_UP;
	_hyperjumpTicks = 0;
	_hyperjumpDX = dx;
	_hyperjumpDY = dy;
	_hyperjumpOrigSpeed = _G(cockpitSpeed);

	// Stop any manual steering - the animation takes over the view
	_G(starfield)._panX = 0;
	_G(starfield)._panY = 0;
}

void SpaceCockpit::tickHyperjump() {
	++_hyperjumpTicks;

	switch (_hyperjumpState) {
	case HYPERJUMP_RAMP_UP:
		if (_hyperjumpTicks % HYPERJUMP_SPEED_STEP_TICKS == 0 && _G(cockpitSpeed) < 8)
			++_G(cockpitSpeed);
		if (_G(cockpitSpeed) >= 8) {
			_hyperjumpState = HYPERJUMP_WARP;
			_hyperjumpTicks = 0;

			auto *logic = dynamic_cast<Logic::SpaceCockpitLogic *>(_G(logic).get());
			if (logic)
				logic->hyperjumpLightspeed();
		}
		break;

	case HYPERJUMP_WARP:
		for (int i = 0; i < HYPERJUMP_WARP_ADVANCES; ++i)
			_G(starfield).advance();
		if (_hyperjumpTicks >= HYPERJUMP_WARP_TICKS) {
			_hyperjumpState = HYPERJUMP_RAMP_DOWN;
			_hyperjumpTicks = 0;
		}
		break;

	case HYPERJUMP_RAMP_DOWN:
		if (_hyperjumpTicks % HYPERJUMP_SPEED_STEP_TICKS == 0 && _G(cockpitSpeed) > _hyperjumpOrigSpeed)
			--_G(cockpitSpeed);
		if (_G(cockpitSpeed) <= _hyperjumpOrigSpeed) {
			_hyperjumpState = HYPERJUMP_IDLE;
			auto *logic = dynamic_cast<Logic::SpaceCockpitLogic *>(_G(logic).get());
			if (logic)
				logic->completeHyperjump(_hyperjumpDX, _hyperjumpDY);
		}
		break;

	default:
		break;
	}

	redraw();
}

bool SpaceCockpit::tick() {
	if (_hyperjumpState != HYPERJUMP_IDLE)
		tickHyperjump();

	if (_laserFlashTicks > 0 && --_laserFlashTicks == 0)
		redraw();

	return Map::tick();
}

static void abortHyperjumpAndNotify(int origSpeed) {
	_G(cockpitSpeed) = origSpeed;
	auto *logic = dynamic_cast<Logic::SpaceCockpitLogic *>(_G(logic).get());
	if (logic)
		logic->abortHyperjump();
}

bool SpaceCockpit::msgAction(const ActionMessage &msg) {
	if (_hyperjumpState != HYPERJUMP_IDLE) {
		_hyperjumpState = HYPERJUMP_IDLE;
		abortHyperjumpAndNotify(_hyperjumpOrigSpeed);
		return true;
	}

	return Map::msgAction(msg);
}

bool SpaceCockpit::msgKeypress(const KeypressMessage &msg) {
	if (_hyperjumpState != HYPERJUMP_IDLE) {
		_hyperjumpState = HYPERJUMP_IDLE;
		abortHyperjumpAndNotify(_hyperjumpOrigSpeed);
		return true;
	}

	return Map::msgKeypress(msg);
}

void SpaceCockpit::drawCockpitFrame(Shared::Gfx::GfxSurface &s) {
	// The ~7px surround around the viewport
	s.fillRect(Rect(8, 8, 312, 16), FRAME_COLOR);
	s.fillRect(Rect(8, 144, 312, 152), FRAME_COLOR);
	s.fillRect(Rect(8, 16, 16, 145), FRAME_COLOR);
	s.fillRect(Rect(304, 16, 312, 145), FRAME_COLOR);

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
