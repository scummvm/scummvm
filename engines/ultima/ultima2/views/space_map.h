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

#ifndef ULTIMA2_VIEWS_SPACE_MAP_H
#define ULTIMA2_VIEWS_SPACE_MAP_H

#include "ultima/shared/gfx/view.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

using namespace Shared::Messages;

/**
 * The starfield seen from a rocket in flight, with the steering marker and
 * the fuel and hyperwarp coordinates readout. Also runs the hyperwarp
 * animation, which passes through several phases of stars streaking and
 * then speeding past
 */
class SpaceMap : public Shared::Gfx::View {
private:
	enum Phase {
		PHASE_IDLE, PHASE_WARP_TRAILS, PHASE_WARP_ACCELERATE, PHASE_WARP_RUSH,
		PHASE_SPIN_DOWN, PHASE_SPIN_HOLD
	};

	static const int STAR_COUNT = 64;

	byte _starX[STAR_COUNT] = {};
	byte _starY[STAR_COUNT] = {};
	byte _seedA = 0, _seedB = 0;
	byte _shipX = 0x80, _shipY = 0x40;
	byte _speed = 0x80;
	Phase _phase = PHASE_IDLE;
	int _counter = 0;

	void seedStars();
	void plot(Shared::Gfx::GfxSurface &s, int x, int y, int color);
	void drawMarker(Shared::Gfx::GfxSurface &s, int color);
	void drawHud(Shared::Gfx::GfxSurface &s);

	/**
	 * Moves every star one step away from the ship's steering position
	 */
	void animate();

	/**
	 * Starts the animation that runs the ship's hyperwarp drive
	 */
	void startWarp();

public:
	SpaceMap();
	~SpaceMap() override {}

	/**
	 * Returns true while a hyperwarp is under way
	 */
	bool busy() const {
		return _phase != PHASE_IDLE;
	}

	/**
	 * Moves the steering marker
	 */
	void setShip(byte x, byte y);

	/**
	 * Winds the ship down to a stop ready for a hyperwarp, then warps
	 */
	void startSpinDown();

	/**
	 * Returns the next value from the starfield's pseudo-random sequence
	 */
	byte nextStarCoord();

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
