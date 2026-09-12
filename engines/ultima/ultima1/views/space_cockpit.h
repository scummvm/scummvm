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

#ifndef ULTIMA1_VIEWS_SPACE_COCKPIT_H
#define ULTIMA1_VIEWS_SPACE_COCKPIT_H

#include "ultima/ultima1/views/map.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/shared/gfx/gfx_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Shared::Messages;

/**
 * The first-person cockpit view of outer space - the panning warp starfield
 * inside the cockpit frame, with the aiming crosshair at the centre (this
 * is also where enemy-craft combat happens). Paired with SpaceCockpitLogic;
 * swapped in from SpaceMap by the View command
 */
class SpaceCockpit : public Map {
private:
	enum HyperjumpState {
		HYPERJUMP_IDLE,
		HYPERJUMP_RAMP_UP,
		HYPERJUMP_WARP,
		HYPERJUMP_RAMP_DOWN
	};

	HyperjumpState _hyperjumpState = HYPERJUMP_IDLE;
	int _hyperjumpTicks = 0;
	int _hyperjumpDX = 0, _hyperjumpDY = 0;
	int _hyperjumpOrigSpeed = 0;

	/**
	 * Draws the static cockpit frame around the viewport (drawCockpitFrame)
	 */
	void drawCockpitFrame(Shared::Gfx::GfxSurface &s);

	/**
	 * Advances whichever phase of the hyperjump animation is running -
	 * ramping the display speed up to 8, streaking the starfield hard for
	 * a bit, then ramping back down to the original speed and handing off
	 * to SpaceCockpitLogic::completeHyperjump()
	 */
	void tickHyperjump();

public:
	SpaceCockpit() : Map("SpaceCockpit") {}
	~SpaceCockpit() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	bool tick() override;

	/**
	 * Any action/keypress while a hyperjump is in progress aborts it
	 * instead of being handled normally
	 */
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;

	/**
	 * Secret mouse-aiming shortcut (not part of the original): moving the
	 * mouse stops any keyboard-driven panning and snaps the crosshair
	 * straight to the pointer, and clicking fires - a much easier way to
	 * line up on enemy ships
	 */
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;

	/**
	 * Kicks off the animated hyperjump sequence heading towards the sector
	 * at the given offset (dx/dy each -1/0/1) - called by
	 * SpaceCockpitLogic::hyperjump() once fuel/heading are validated
	 */
	void startHyperjump(int dx, int dy);
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
