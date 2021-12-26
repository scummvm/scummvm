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

#include "engines/util.h"
#include "ultima/shared/gfx/screen.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

Screen::Screen(): Graphics::Screen(320, 200), _cursor(nullptr), _drawCursor(false) {
	initGraphics(320, 200);
}

void Screen::update() {
	_drawCursor = false;

	if (_cursor) {
		// Check whether the area the cursor occupies will be being updated
		Common::Rect cursorBounds = _cursor->getBounds();
		for (Common::List<Common::Rect>::iterator i = _dirtyRects.begin(); i != _dirtyRects.end(); ++i) {
			const Common::Rect &r = *i;
			if (r.intersects(cursorBounds)) {
				addDirtyRect(cursorBounds);
				_drawCursor = true;
				break;
			}
		}
	}

	Graphics::Screen::update();
}

void Screen::updateScreen() {
	if (_drawCursor)
		_cursor->draw();

	Graphics::Screen::updateScreen();
}


} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
