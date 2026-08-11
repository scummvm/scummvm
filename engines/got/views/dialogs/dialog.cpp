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

#include "got/views/dialogs/dialog.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

Dialog::Dialog(const Common::String &name) : View(name) {
	_bounds.setBorderSize(16);
}

void Dialog::draw() {
	// Clear the inner content first
	GfxSurface s = getSurface(true);
	s.clear(215);

	s = getSurface();
	assert((s.w % 16) == 0 && (s.h % 16) == 0);

	// Draw four corners
	s.simpleBlitFrom(_G(bgPics[192]), Common::Point(0, 0));
	s.simpleBlitFrom(_G(bgPics[193]), Common::Point(_bounds.width() - 16, 0));
	s.simpleBlitFrom(_G(bgPics[194]), Common::Point(0, _bounds.height() - 16));
	s.simpleBlitFrom(_G(bgPics[195]), Common::Point(_bounds.width() - 16, _bounds.height() - 16));

	// Draw top/bottom horizontal lines
	for (int x = 16; x < _bounds.width() - 16; x += 16) {
		s.simpleBlitFrom(_G(bgPics[196]), Common::Point(x, 0));
		s.simpleBlitFrom(_G(bgPics[197]), Common::Point(x, _bounds.height() - 16));
	}

	// Draw left/right vertical lines
	for (int y = 16; y < _bounds.height() - 16; y += 16) {
		s.simpleBlitFrom(_G(bgPics[198]), Common::Point(0, y));
		s.simpleBlitFrom(_G(bgPics[199]), Common::Point(_bounds.width() - 16, y));
	}
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
