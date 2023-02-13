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

#include "graphics/cursorman.h"
#include "mm/mm1/utils/mouse.h"

namespace MM {
namespace MM1 {

void Mouse::loadCursors() {
	_sprites.load("mouse.icn");
}

void Mouse::setCursor(int cursorId) {
	Shared::Xeen::XSurface cursor;
	_sprites.draw(cursor, cursorId, Common::Point(0, 0), Shared::Xeen::SPRFLAG_RESIZE);

	CursorMan.replaceCursor(cursor.getPixels(), cursor.w, cursor.h, 0, 0, 0);
	showCursor();
}

void Mouse::showCursor() {
	CursorMan.showMouse(true);
}

void Mouse::hideCursor() {
	CursorMan.showMouse(false);
}

bool Mouse::isCursorVisible() const {
	return CursorMan.isVisible();
}

} // namespace MM1
} // namespace MM
