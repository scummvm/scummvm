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
#include "graphics/views/gfx_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Graphics::Views;

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
		HYPERJUMP_ACCELERATE,	// Cruising at each speed up to the maximum
		HYPERJUMP_WARP_IN,		// Stars streak outward, leaving their trails behind them
		HYPERJUMP_WARP_OUT,		// The same stars replayed, erasing the trails again
		HYPERJUMP_DECELERATE	// Cruising back down to the original speed
	};

	HyperjumpState _hyperjumpState = HYPERJUMP_IDLE;
	int _hyperjumpDX = 0, _hyperjumpDY = 0;
	int _hyperjumpOrigSpeed = 0;
	int _hyperjumpLevelFrames = 0;		// Frames cruised at each speed
	int _hyperjumpFrames = 0;			// Frames left in the current speed or warp phase
	int _hyperjumpBudget = 0;			// Vertical retraces left to spend on cruise frames
	uint32 _hyperjumpSeed = 0;

	// The pixels the stars have left behind during the warp
	Graphics::ManagedSurface _trails;

	// Ticks left to keep showing the laser-bolt flash from fireLaser() (0 =
	// not showing)
	int _laserFlashTicks = 0;

	// Ticks left to keep showing the incoming shot flash from
	// alienFireFlash() (0 = not showing), and its endpoints
	int _alienFlashTicks = 0;
	int _alienFlashX1 = 0, _alienFlashY1 = 0, _alienFlashX2 = 0, _alienFlashY2 = 0;

	/**
	 * Draws the static cockpit frame around the viewport (drawCockpitFrame)
	 */
	void drawCockpitFrame(Graphics::Views::GfxSurface &s);

	/**
	 * Runs however many frames of the hyperjump animation are due: cruising
	 * up through the speeds, then a warp that stops erasing the stars so
	 * they fill the screen with trails, replayed to wipe them away again,
	 * then cruising back down and handing off to
	 * SpaceCockpitLogic::completeHyperjump()
	 */
	void tickHyperjump();

	/**
	 * Runs a single frame of whichever phase is under way
	 */
	void hyperjumpFrame();

	/**
	 * Starts the warp once the ship has reached full speed
	 */
	void startWarp();

	/**
	 * Ends the hyperjump animation, and carries on with the jump
	 */
	void finishHyperjump();

	/**
	 * Marks a pixel of the warp trails, or erases it with color 0
	 */
	void plotTrail(int x, int y, int color);

	/**
	 * Stops a hyperjump in progress, if there is one
	 * @returns		True if there was one
	 */
	bool cancelHyperjump();

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

	/**
	 * Briefly flashes twin laser-bolt lines in from the bottom corners of
	 * the viewport, converging on the crosshair - called by
	 * SpaceCockpitLogic::fire() for each shot
	 */
	void fireLaser();

	/**
	 * Briefly flashes a single laser-bolt line between the given points -
	 * called by SpaceCockpitLogic::alienFiresBack() for the current
	 * target's return shot
	 */
	void alienFireFlash(int x1, int y1, int x2, int y2);
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
