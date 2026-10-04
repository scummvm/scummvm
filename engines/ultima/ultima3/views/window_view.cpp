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

#include "ultima/ultima3/views/window_view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int BORDER_TOP = 10;
constexpr int BORDER_BOTTOM = 23;
constexpr int BORDER_RIGHT = 39;

void WindowView::clearWindow(GfxSurface &s) {
	s.fillRect(Common::Rect(8, 11 * 8, 39 * 8, 23 * 8), 0);
}

void WindowView::drawBorder(GfxSurface &s) {
	// Alternating black and magenta pixels, filling whole 8x8 cells
	auto cell = [&](int col, int row) {
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < 8; ++x)
				s.setPixel(col * 8 + x, row * 8 + y, (x & 1) ? 2 : 0);
	};

	s.fillRect(Common::Rect(0, 24 * 8, 320, 25 * 8), 0);

	for (int col = 0; col <= BORDER_RIGHT; ++col)
		cell(col, BORDER_BOTTOM);
	for (int col = 1; col < BORDER_RIGHT; ++col)
		cell(col, BORDER_TOP);
	for (int row = BORDER_TOP; row <= BORDER_BOTTOM; ++row) {
		cell(0, row);
		cell(BORDER_RIGHT, row);
	}

	// setPixel doesn't flag the changed area for updating on screen
	s.addDirtyRect(Common::Rect(0, BORDER_TOP * 8, 320, 25 * 8));
}

void WindowView::drawSpacePrompt(GfxSurface &s) {
	s.writeString(Common::Point(12, 23), "\x10Press <Space>\x11");
}

bool WindowView::isSpaceKey(const KeypressMessage &msg) {
	return msg.ascii == ' ';
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
