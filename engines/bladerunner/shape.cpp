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

bool Shape::load(Common::SeekableReadStream *stream) {
	_width = stream->readUint32LE();
	_height = stream->readUint32LE();
	uint32 size = stream->readUint32LE();

	if (size != (uint32)(_width * _height * 2)) {
		warning("Shape::load size mismatch (w %d, h %d, sz %d)", _width, _height, size);
		return false;
	}

	// Enfoce a reasonable size limit
	if (_width >= 2048 || _height >= 2048) {
		warning("Shape::load shape too big (%d, %d)", _width, _height);
	}

	_data = new byte[size];

	if (stream->read(_data, size) != size) {
		warning("Shape::load error reading shape (w %d, h %d, sz %d)", _width, _height, size);
		return false;
	}

	return true;
}

Shape::~Shape() {
	delete[] _data;
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

Shapes::Shapes(BladeRunnerEngine *vm) {
	_vm = vm;
}

Shapes::~Shapes() {
	unload();
}

bool Shapes::load(const Common::String &container) {
	unload();

	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->getResourceStream(container));
	if (!stream) {
		warning("Shape::open failed to open '%s'", container.c_str());
		return false;
	}

	uint32 count = stream->readUint32LE();

	_shapes.resize(count);

	for (uint32 i = 0; i < count; ++i) {
		if (!_shapes[i].load(stream.get())) {
			return false;
		}
	}

	return true;
}

void Shapes::unload() {
	_shapes.clear();
}


} // End of namespace BladeRunner
