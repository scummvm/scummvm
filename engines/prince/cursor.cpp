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

#include "prince/cursor.h"

#include "common/debug.h"

namespace Prince {

Cursor::Cursor() : _surface(nullptr) {
}

Cursor::~Cursor() {
	if (_surface != nullptr) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
}

bool Cursor::loadStream(Common::SeekableReadStream &stream) {
	stream.skip(4);
	uint16 width = stream.readUint16LE();
	uint16 height = stream.readUint16LE();

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	for (int h = 0; h < height; h++) {
		stream.read(_surface->getBasePtr(0, h), width);
	}
	return true;
}

} // End of namespace Prince
