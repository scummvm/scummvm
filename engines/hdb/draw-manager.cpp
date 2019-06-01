/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "draw-manager.h"

namespace HDB {

DrawMan::DrawMan() {
	_systemInit = false;
	cursorDisplay = true;
}

bool DrawMan::init() {
	_systemInit = true;
	return true;
}

void DrawMan::loadTile32(char *name, uint32 *length) {
	
}

Graphics::Surface Picture::load(Common::SeekableReadStream *stream) {
	_width = stream->readUint32LE();
	_height = stream->readUint32LE();
	stream->read(_name, 64);
	Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);

	debug(8, "Picture: _width: %d, _height: %d", _width, _height);
	debug(8, "Picture: _name: %s", _name);

	_surface.create(_width, _height, format);
	stream->readUint32LE(); // Skip Win32 Surface

	uint16 *ptr;

	for (uint y = 0; y < _height; y++) {
		ptr = (uint16 *) _surface.getBasePtr(0, y);
		for (uint x = 0; x < _width; x++) {
			*ptr = TO_LE_16(stream->readUint16LE());
			ptr++;
		}
	}

	return _surface;
}

}
