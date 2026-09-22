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

#include "ultima/ultima2/views/space_map.h"
#include "ultima/ultima2/logic/space_logic.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

// The stars fill a 256x128 area, offset from the top left corner
constexpr int VIEW_OFFSET = 16;
constexpr int COLOR_STAR = 3;

// Animation steps per game frame while hyperwarping. The original's speed
// depends on the CPU, so this is just tuned to feel right
constexpr int WARP_STEPS_PER_TICK = 2;

SpaceMap::SpaceMap() : View("SpaceMap") {
	setBounds(TextRect(0, 0, 39, 24));
}

void SpaceMap::seedStars() {
	_seedA = 0x3B;
	_seedB = 0x67;
}

byte SpaceMap::nextStarCoord() {
	int sum = _seedA + 9;
	int carry = sum >> 8;
	byte next = (byte)(sum + _seedB + carry);

	_seedB = _seedA;
	_seedA = next;
	return next;
}

bool SpaceMap::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_GAMEPLAY);

	seedStars();
	for (int i = 0; i < STAR_COUNT; ++i) {
		_starX[i] = nextStarCoord();
		_starY[i] = nextStarCoord() & 0x7F;
	}

	_shipX = 0x80;
	_shipY = 0x40;
	_speed = 0x80;
	startWarp();
	return View::msgFocus(msg);
}

bool SpaceMap::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return View::msgUnfocus(msg);
}

bool SpaceMap::msgAction(const ActionMessage &msg) {
	if (!busy())
		g_engine->_logic->action(msg._action);
	return true;
}

bool SpaceMap::msgKeypress(const KeypressMessage &msg) {
	if (!busy())
		g_engine->_logic->keypress(msg);
	return true;
}

void SpaceMap::plot(Shared::Gfx::GfxSurface &s, int x, int y, int color) {
	s.setPixel(x + VIEW_OFFSET, y + VIEW_OFFSET, color);
}

void SpaceMap::drawMarker(Shared::Gfx::GfxSurface &s, int color) {
	for (int i = -3; i <= 3; ++i) {
		plot(s, _shipX, (byte)(_shipY + i), color);
		if (i != 0)
			plot(s, (byte)(_shipX + i), _shipY, color);
	}
}

void SpaceMap::setShip(byte x, byte y) {
	auto s = getSurface();
	drawMarker(s, 0);

	_shipX = x;
	_shipY = y;
	drawMarker(s, COLOR_STAR);
}

void SpaceMap::draw() {
	auto s = getSurface();
	s.fillRect(Common::Rect(0, 0, 320, 160), 0);

	for (int i = 0; i < STAR_COUNT; ++i)
		plot(s, _starX[i], _starY[i], COLOR_STAR);
	drawMarker(s, COLOR_STAR);

	// Draw the HUD stats
	View::draw();
}

void SpaceMap::animate() {
	auto s = getSurface();
	drawMarker(s, COLOR_STAR);

	for (int i = 0; i < STAR_COUNT; ++i) {
		byte sx = _starX[i], sy = _starY[i];
		byte nx = 0, ny = 0;
		bool respawn = false;

		// Each star moves away from the steering position, faster the
		// further out it is, and is replaced by a new one once off the edge
		byte dx = sx - _shipX;
		if (sx >= _shipX) {
			int t = sx + (dx >> 4) + 1;
			if (t > 255)
				respawn = true;
			else
				nx = t;
		} else {
			int step = ((byte)~dx >> 4) + 1;
			if (sx < step)
				respawn = true;
			else
				nx = sx - step;
		}

		if (!respawn) {
			byte dy = sy - _shipY;
			if (sy >= _shipY) {
				byte t = sy + (dy >> 4) + 1;
				if (t & 0x80)
					respawn = true;
				else
					ny = t;
			} else {
				int step = ((byte)~dy >> 4) + 1;
				byte t = sy - step;
				if (t & 0x80)
					respawn = true;
				else
					ny = t;
			}
		}

		if (respawn) {
			nx = nextStarCoord();
			ny = nextStarCoord() & 0x7F;
		}

		// At speed zero the old positions are left behind, drawing streaks
		if (_speed != 0)
			plot(s, sx, sy, 0);

		_starX[i] = nx;
		_starY[i] = ny;
		plot(s, nx, ny, COLOR_STAR);
	}

	// Flag the entire surface area needing updates
	s.markAllDirty();
}

void SpaceMap::startWarp() {
	g_engine->baseView()->findView("Commands")->send(GameMessage("TEXT", "HYPERWARP ENGAGED!\n"));

	_speed = 0;
	_counter = 0x80;
	seedStars();
	_phase = PHASE_WARP_TRAILS;
}

void SpaceMap::startSpinDown() {
	seedStars();
	_phase = PHASE_SPIN_DOWN;
}

bool SpaceMap::tick() {
	auto *commands = g_engine->baseView()->findView("Commands");
	if (commands)
		commands->tick();

	int steps = (_phase == PHASE_IDLE) ? 1 : WARP_STEPS_PER_TICK;
	for (int i = 0; i < steps; ++i) {
		animate();

		switch (_phase) {
		case PHASE_WARP_TRAILS:
			if (--_counter == 0) {
				auto s = getSurface();
				_counter = 0x80;
				_speed = 1;
				_stats.draw();
				seedStars();
				_phase = PHASE_WARP_ACCELERATE;
			}
			break;

		case PHASE_WARP_ACCELERATE:
			if (--_counter == 0) {
				++_speed;
				_phase = PHASE_WARP_RUSH;
			}
			break;

		case PHASE_WARP_RUSH:
			_speed += 2;
			if (_speed & 0x80) {
				_phase = PHASE_IDLE;
				if (auto *logic = dynamic_cast<Logic::SpaceLogic *>(_G(logic).get()))
					logic->warpFinished();
				return true;
			}
			break;

		case PHASE_SPIN_DOWN:
			if (--_speed == 2) {
				_counter = 0x40;
				seedStars();
				_phase = PHASE_SPIN_HOLD;
			}
			break;

		case PHASE_SPIN_HOLD:
			if (--_counter == 0)
				startWarp();
			break;

		default:
			break;
		}
	}

	return true;
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
