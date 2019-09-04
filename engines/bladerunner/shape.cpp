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

#include "bladerunner/shape.h"

#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/util.h"

#include "graphics/surface.h"

namespace BladeRunner {

Shape::Shape(BladeRunnerEngine *vm) {
	_vm     = vm;
	_data   = nullptr;
	_width  = 0;
	_height = 0;
}

Shape::~Shape() {
	delete[] _data;
}

bool Shape::open(const Common::String &container, int index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->getResourceStream(container));
	if (!stream) {
		warning("Shape::open failed to open '%s'", container.c_str());
		return false;
	}

	uint32 count = stream->readUint32LE();
	if (index < 0 || (uint32)index >= count) {
		warning("Shape::open invalid index %d (count %u)", index, count);
		return false;
	}

	uint32 size = 0, width = 0, height = 0;
	for (int i = 0; i <= index; ++i) {
		width  = stream->readUint32LE();
		height = stream->readUint32LE();
		size   = stream->readUint32LE();

		if (size != width * height * 2) {
			warning("Shape::open size mismatch (w %d, h %d, sz %d)", width, height, size);
			return false;
		}

		if (i != index) {
			stream->skip(size);
		}
	}

	// Enfoce a reasonable size limit
	if (width >= 2048 || height >= 2048) {
		warning("Shape::open shape too big (%d, %d)", width, height);
	}

	_width  = width;
	_height = height;
	_data   = new byte[size];

	if (stream->read(_data, size) != size) {
		warning("Shape::open error reading shape %d (w %d, h %d, sz %d)", index, width, height, size);
		return false;
	}

	return true;
}

void Shape::draw(Graphics::Surface &surface, int x, int y) const {
	int src_x = CLIP(-x, 0, _width);
	int src_y = CLIP(-y, 0, _height);

	int dst_x = CLIP<int>(x, 0, surface.w);
	int dst_y = CLIP<int>(y, 0, surface.h);

	int rect_w = MIN(CLIP(_width + x, 0, _width), surface.w - x);
	int rect_h = MIN(CLIP(_height + y, 0, _height), surface.h - y);

	if (rect_w == 0 || rect_h == 0) {
		return;
	}

	const uint8 *src_p = _data + 2 * (src_y * _width + src_x);

	for (int yi = 0; yi != rect_h; ++yi) {
		for (int xi = 0; xi != rect_w; ++xi) {
			uint16 shpColor = READ_LE_UINT16(src_p);
			src_p += 2;

			uint8 a, r, g, b;
			getGameDataColor(shpColor, a, r, g, b);

			if (!a) {
				// Ignore the alpha in the output as it is inversed in the input
				void *dstPtr = surface.getBasePtr(CLIP(dst_x + xi, 0, surface.w - 1), CLIP(dst_y + yi, 0, surface.h - 1));
				drawPixel(surface, dstPtr, surface.format.RGBToColor(r, g, b));
			}
		}
		src_p += 2 * (_width - rect_w);
	}
}

} // End of namespace BladeRunner
