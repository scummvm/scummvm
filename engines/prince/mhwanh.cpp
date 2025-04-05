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

#include "common/stream.h"

#include "graphics/surface.h"

#include "prince/mhwanh.h"

namespace Prince {

MhwanhDecoder::MhwanhDecoder() : _surface(nullptr), _palette(0) {
}

MhwanhDecoder::~MhwanhDecoder() {
	destroy();
}

void MhwanhDecoder::destroy() {
	if (_surface != nullptr) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
	_palette.clear();
}

bool MhwanhDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();
	stream.seek(0);
	stream.skip(0x20);
	// Read the palette
	_palette.resize(kPaletteColorCount, false);
	for (uint16 i = 0; i < kPaletteColorCount; i++) {
		byte r = stream.readByte();
		byte g = stream.readByte();
		byte b = stream.readByte();
		_palette.set(i, r, g, b);
	}

	_surface = new Graphics::Surface();
	_surface->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	for (int h = 0; h < 480; h++) {
		stream.read(_surface->getBasePtr(0, h), 640);
	}

	return true;
}

} // End of namespace Prince
