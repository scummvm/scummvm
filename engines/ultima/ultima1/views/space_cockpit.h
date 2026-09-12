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
	/**
	 * Draws the static cockpit frame around the viewport (drawCockpitFrame)
	 */
	void drawCockpitFrame(Shared::Gfx::GfxSurface &s);

public:
	SpaceCockpit() : Map("SpaceCockpit") {}
	~SpaceCockpit() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;

	/**
	 * Secret mouse-aiming shortcut (not part of the original): moving the
	 * mouse stops any keyboard-driven panning and snaps the crosshair
	 * straight to the pointer, and clicking fires - a much easier way to
	 * line up on enemy ships
	 */
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
