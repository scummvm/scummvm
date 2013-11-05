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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "prince/cursor.h"

#include "common/debug.h"
#include "common/stream.h"

namespace Prince {

Cursor::Cursor() : _surface(NULL) {
}

Cursor::~Cursor() {
	delete _surface;
	_surface = NULL;
}

bool Cursor::loadFromStream(Common::SeekableReadStream &stream) {
	stream.skip(4);
	uint16 w = stream.readUint16LE();
	uint16 h = stream.readUint16LE();

	_surface = new Graphics::Surface();
	_surface->create(w, h, Graphics::PixelFormat::createFormatCLUT8());

	for (int ih = 0; ih < h; ++ih) {
		stream.read(_surface->getBasePtr(0, ih), w);
	}
	return true;
}

}

/* vim: set tabstop=4 noexpandtab: */
