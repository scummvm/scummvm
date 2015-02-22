/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/stark/cursor.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

namespace Stark {

Cursor::Cursor(Gfx::Driver *gfx) :
		_gfx(gfx) {
	// TODO: This is just a quick solution to get anything drawn, we will need load-code for the actual pointers.
	_cursorTexture = _gfx->createTextureFromString("X", 0xFF00FF00);
}

Cursor::~Cursor() {
	delete _cursorTexture;
}

void Cursor::setMousePosition(Common::Point pos) {
	_mousePos = pos;
}

void Cursor::render() {
	_gfx->drawSurface(_cursorTexture, _mousePos);
}

} // End of namespace Stark
