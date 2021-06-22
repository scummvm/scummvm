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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/grim/cursor.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/gfx_base.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"

const static int numCursors = 8;

namespace Grim {

Cursor::Cursor(GrimEngine *vm) : _position(320, 210),
								 _curCursor(0), _persistentCursor(-1) {
	_bitmaps = new Bitmap *[numCursors]();
	loadAvailableCursors();
}

void Cursor::loadAvailableCursors() {
	for (int i = 0; i < numCursors; ++i) {
		Common::String fn = Common::String::format("cursor%d.tga", i);
		_bitmaps[i] = nullptr;
		if (SearchMan.hasFile(fn)) {
			_bitmaps[i] = Bitmap::create(fn.c_str());
			_bitmaps[i]->_data->load();
			_bitmaps[i]->_data->_hasTransparency = true;
		}
	}
	_hotspotx = _hotspoty = 15;
	CursorMan.showMouse(false);
}

void Cursor::reload() {
	loadAvailableCursors();
}

Cursor::~Cursor() {
	delete[] _bitmaps;
	_bitmaps = nullptr;
}

void Cursor::setPersistent(int id, int x, int y) {
	_persistentCursor = id;
	_persistentPosition.x = x;
	_persistentPosition.y = y;
}

void Cursor::draw() {
	if (_curCursor >= 0 && _bitmaps[_curCursor])
		_bitmaps[_curCursor]->draw(_position.x - _hotspotx, _position.y - _hotspoty);
	if (_persistentCursor >= 0 && _bitmaps[_persistentCursor] != nullptr)
		_bitmaps[_persistentCursor]->draw(_persistentPosition.x - _hotspotx,
										  _persistentPosition.y - _hotspoty);
}

} // namespace Grim
