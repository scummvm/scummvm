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

#include "ultima/ultima1/views/space_cockpit.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

constexpr int FRAME_COLOR = 7;		// light grey cockpit surround
constexpr int STAR_COLOR = 15;		// white stars / crosshair
constexpr int RIVET_COLOR = 0;		// black detail punched into the frame

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
