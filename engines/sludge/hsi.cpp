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

#include "common/debug.h"
#include "common/stream.h"

#include "graphics/surface.h"

#include "sludge/hsi.h"
#include "sludge/sludge.h"

namespace Sludge {

HSIDecoder::HSIDecoder() : _surface(nullptr), _reserve(-1) {
}

HSIDecoder::~HSIDecoder() {
	destroy();
}

void HSIDecoder::destroy() {
	if (_surface != nullptr) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
}

bool HSIDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();
	int32 transCol = _reserve > 0 ? -1 : 63519;
	int n;
	uint16 width = stream.readUint16BE();
	debugC(2, kSludgeDebugGraphics, "picWidth : %i", width);
	uint16 height = stream.readUint16BE();
	debugC(2, kSludgeDebugGraphics, "picHeight : %i", height);

	_surface = new Graphics::Surface();
	_surface->create(width, height, *g_sludge->getScreenPixelFormat());
	for (uint16 y = 0; y < height; y++) {
		uint16 x = 0;
		while (x < width) {
			uint16 c = stream.readUint16BE();
			if (c & 32) {
				n = stream.readByte() + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n--) {
				byte *target = (byte *)_surface->getBasePtr(x, y);
				if (_reserve != -1 && (c == transCol || c == 2015)) {
					target[0] = (byte)0;
					target[1] = (byte)0;
					target[2] = (byte)0;
					target[3] = (byte)0;
				} else {
					target[0] = (byte)255;
					g_sludge->getOrigPixelFormat()->colorToRGB(c, target[3], target[2], target[1]);
				}
				x++;
			}
		}
	}
	return true;
}

} // End of namespace Sludge
