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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "ultima/ultima0/views/title.h"
#include "ultima/ultima0/metaengine.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

void Title::draw() {
	auto s = getSurface();
	s.writeString(Common::Point(20, 8), "Ultima 0 - Akalabeth!", Graphics::kTextAlignCenter);

	const int selected = getColor(255, 0, 128);
	const int white = getColor(255, 255, 255);

	s.setColor(_highlightedOption == 0 ? selected : white);
	s.writeString(Common::Point(20, 16), "Introduction", Graphics::kTextAlignCenter);
	s.setColor(_highlightedOption == 1 ? selected : white);
	s.writeString(Common::Point(20, 17), "Create a Character", Graphics::kTextAlignCenter);
	s.setColor(_highlightedOption == 2 ? selected : white);
	s.writeString(Common::Point(20, 18), "Acknowledgements", Graphics::kTextAlignCenter);
	s.setColor(_highlightedOption == 3 ? selected : white);
	s.writeString(Common::Point(20, 19), "Journey Onwards", Graphics::kTextAlignCenter);
}

bool Title::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_UP:
		_highlightedOption = _highlightedOption ? _highlightedOption - 1 : 3;
		redraw();
		break;
	case KEYBIND_DOWN:
		_highlightedOption = (_highlightedOption + 1) % 4;
		redraw();
		break;
	default:
		break;
	}

	return true;
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
