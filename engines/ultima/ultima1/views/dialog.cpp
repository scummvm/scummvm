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
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

// Colors used
constexpr int COLOR_BORDER = 1;  // blue - thick outer band
constexpr int COLOR_EDGE = 15;   // white - thin inner highlight line
constexpr int COLOR_NOTCH = 0;   // black - corner notch pixels

Dialog::Dialog(const Common::String &name) : View(name) {
	setBounds(TextRect(0, 0, 39, 19));
}

void Dialog::draw() {
	auto s = getSurface();
	s.clear();

	// Thick outer border band - each edge flush against both the screen edge
	// and the inner highlight line drawn just inside it
	s.fillRect(Common::Rect(0, 0, 320, 7), COLOR_BORDER);
	s.fillRect(Common::Rect(0, 7, 7, 159), COLOR_BORDER);
	s.fillRect(Common::Rect(313, 7, 320, 159), COLOR_BORDER);
	s.fillRect(Common::Rect(0, 152, 320, 160), COLOR_BORDER);

	// Thin inner highlight line
	s.frameRect(Common::Rect(7, 7, 313, 153), COLOR_EDGE);
	s.drawLine(0, 159, 240, 159, COLOR_EDGE);
	s.drawLine(247, 159, 319, 159, COLOR_EDGE);

	// Diagonal-notched corners, cut as a growing staircase into each corner
	for (int idx = 1; idx <= 4; ++idx) {
		s.drawLine(idx, 0, 0, idx, COLOR_NOTCH);
		s.drawLine(319 - idx, 0, 319, idx, COLOR_NOTCH);
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

bool Dialog::tick() {
	g_engine->findView("Game")->tick();
	return Shared::Gfx::View::tick();
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
