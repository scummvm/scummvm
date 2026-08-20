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
#include "ultima/ultima1/views/dialog.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

// Colors used
constexpr int COLOR_BORDER = 1;  // blue - thick outer band
constexpr int COLOR_EDGE = 15;   // white - thin inner highlight line
constexpr int COLOR_NOTCH = 0;   // black - corner notch pixels

void Dialog::drawFrame() {
	auto s = getSurface();

	// Thick outer border band - each edge flush against both the screen edge
	// and the inner highlight line drawn just inside it
	s.fillRect(Common::Rect(0, 0, 320, 7), COLOR_BORDER);
	s.fillRect(Common::Rect(0, 7, 7, 193), COLOR_BORDER);
	s.fillRect(Common::Rect(313, 7, 320, 193), COLOR_BORDER);
	s.fillRect(Common::Rect(0, 193, 320, 200), COLOR_BORDER);

	// Thin inner highlight line
	s.drawLine(7, 7, 312, 7, COLOR_EDGE);
	s.drawLine(7, 7, 7, 192, COLOR_EDGE);
	s.drawLine(312, 7, 312, 192, COLOR_EDGE);
	s.drawLine(7, 192, 312, 192, COLOR_EDGE);

	// Diagonal-notched corners, cut as a growing staircase into each corner
	for (int idx = 1; idx <= 4; ++idx) {
		s.drawLine(idx, 0, 0, idx, COLOR_NOTCH);
		s.drawLine(319 - idx, 0, 319, idx, COLOR_NOTCH);
		s.drawLine(0, 199 - idx, idx, 199, COLOR_NOTCH);
		s.drawLine(319, 199 - idx, 319 - idx, 199, COLOR_NOTCH);
	}

	static const Common::Point NOTCH_POINTS[] = {
		Common::Point(0, 0), Common::Point(0, 5), Common::Point(5, 0),
		Common::Point(319, 0), Common::Point(314, 0), Common::Point(319, 5),
		Common::Point(0, 199), Common::Point(0, 194), Common::Point(5, 199),
		Common::Point(319, 199), Common::Point(319, 194), Common::Point(314, 199)
	};
	for (int i = 0; i < ARRAYSIZE(NOTCH_POINTS); ++i)
		s.drawLine(NOTCH_POINTS[i].x, NOTCH_POINTS[i].y, NOTCH_POINTS[i].x, NOTCH_POINTS[i].y, COLOR_NOTCH);
}

void Dialog::drawGameDividers() {
	auto s = getSurface();

	// Erase the frame's bottom section (its border band, the lower
	// portion of the side bands, and the bottom corners) - the dividers
	// below replace it, running all the way to the screen edges
	s.fillRect(Common::Rect(0, 153, 320, 200), 0);

	// Horizontal divider separating the map viewport (above) from the
	// command/message log and stats panel (below). The bar and its
	// bottom edge span the full screen width, but the top edge stops at
	// the viewport's own inner border, meeting up with its side edges
	s.fillRect(Common::Rect(0, 153, 320, 159), COLOR_BORDER);
	s.drawLine(7, 152, 312, 152, COLOR_EDGE);
	s.drawLine(0, 159, 240, 159, COLOR_EDGE);
	s.drawLine(247, 159, 319, 159, COLOR_EDGE);

	// Vertical divider between the command/message log (left) and the
	// stats panel (right), running to the bottom of the screen
	s.fillRect(Common::Rect(241, 153, 247, 200), COLOR_BORDER);
	s.drawLine(240, 159, 240, 199, COLOR_EDGE);
	s.drawLine(247, 159, 247, 199, COLOR_EDGE);
}

// Per-column vertical extent of an 8x8 right-pointing triangle, tip at
// column 7 - column 0 is the full-height flat edge, tapering down to a
// single pixel at the tip. Used to fill the triangle solid before the
// diagonal edge highlights are drawn on top
static const byte ARROW_TOP[8] = { 0, 0, 1, 1, 2, 2, 3, 3 };
static const byte ARROW_BOTTOM[8] = { 7, 6, 6, 5, 5, 4, 4, 3 };

void Dialog::drawRightArrow(const Common::Point &pt) {
	auto s = getSurface();
	int px = pt.x * 8, py = pt.y * 8;

	s.fillRect(TextRect(pt.x, pt.y, pt.x, pt.y), COLOR_NOTCH);

	for (int i = 0; i < 8; ++i)
		s.drawLine(px + i, py + ARROW_TOP[i], px + i, py + ARROW_BOTTOM[i], COLOR_BORDER);

	s.drawLine(px, py, px + 7, py + 3, COLOR_EDGE);
	s.drawLine(px + 7, py + 3, px, py + 7, COLOR_EDGE);
}

void Dialog::drawLeftArrow(const Common::Point &pt) {
	auto s = getSurface();
	int px = pt.x * 8, py = pt.y * 8;

	s.fillRect(TextRect(pt.x, pt.y, pt.x, pt.y), COLOR_NOTCH);

	for (int i = 0; i < 8; ++i)
		s.drawLine(px + 7 - i, py + ARROW_TOP[i], px + 7 - i, py + ARROW_BOTTOM[i], COLOR_BORDER);

	s.drawLine(px + 7, py, px, py + 3, COLOR_EDGE);
	s.drawLine(px, py + 3, px + 7, py + 7, COLOR_EDGE);
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
