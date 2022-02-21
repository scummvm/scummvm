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

#include "bladerunner/shape.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/mouse.h"

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

void Shape::draw(Graphics::Surface &surface, int x, int y, uint8 drawModeBitFlags) const {
	int src_x = CLIP(-x, 0, _width);
	int src_y = CLIP(-y, 0, _height);

	int dst_x = CLIP<int>(x, 0, surface.w);
	int dst_y = CLIP<int>(y, 0, surface.h);

	int rect_w = MIN(CLIP(_width + x, 0, _width), surface.w - x);
	int rect_h = MIN(CLIP(_height + y, 0, _height), surface.h - y);

	if (rect_w <= 0 || rect_h <= 0) {
		// Checking here for negative values also,
		// prevents segmentation fault (in the for loop below)
		return;
	}

	const uint8 *src_p = _data + 2 * (src_y * _width + src_x);

	uint16 shpColor = 0;
	uint32 surfaceColorRGBPrev = 0;
	uint32 newSurfaceColorRGB = 0;
	uint8 a, r, g, b;
	uint8 rPrev, gPrev, bPrev;
	uint16 rgb16bitPrev = 0;
	uint16 rgb16bitAdd = 0;
	for (int yi = 0; yi != rect_h; ++yi) {
		for (int xi = 0; xi != rect_w; ++xi) {
			shpColor = READ_LE_UINT16(src_p);
			src_p += 2;

			getGameDataColor(shpColor, a, r, g, b);

			if (!a) {
				// Ignore the alpha in the output as it is inversed in the input
				void *dstPtr = surface.getBasePtr(CLIP(dst_x + xi, 0, surface.w - 1), CLIP(dst_y + yi, 0, surface.h - 1));
				if (drawModeBitFlags & Mouse::MouseDrawFlags::SPECIAL) {
					// It seems that the additive mode was supposed to be used only for cursor shapes
					// From testing, the only cursor shape that seems to work with it is the green rotating cursor
					// We add extra code here to cover the cases of the beta crosshairs cursor
					// being drawn a different color based on bullet power
					// The code for creating the specific color is custom.
					if (drawModeBitFlags & Mouse::MouseDrawFlags::REDCROSSHAIRS) {
						newSurfaceColorRGB = surface.format.RGBToColor((b & 0x8B) | (g >> 1), 0, 0);
					} else if (drawModeBitFlags & Mouse::MouseDrawFlags::YELLOWCROSSHAIRS) {
						newSurfaceColorRGB = surface.format.RGBToColor(b & 0xDF, (b & 0xA5) | (g >> 1), 0);
					} else if (drawModeBitFlags & Mouse::MouseDrawFlags::BLUECROSSHAIRS) {
						newSurfaceColorRGB = surface.format.RGBToColor(r, g, b);
					} else {
						// Additive modes
						getPixel(surface, dstPtr, surfaceColorRGBPrev);
						if (drawModeBitFlags & Mouse::MouseDrawFlags::ADDITIVE_MODE0) {
							// This code makes the cursor semi-transparent
							// but it may not be what the disassembly of the original was going for.
							newSurfaceColorRGB = surface.format.RGBToColor(r, g, b);
							newSurfaceColorRGB = (((uint16)surfaceColorRGBPrev >> 1) & 0xFBEF)
												  + (((uint16)newSurfaceColorRGB >> 1) & 0xFBEF);
						} else if (drawModeBitFlags & Mouse::MouseDrawFlags::ADDITIVE_MODE1) {
							// This code may be closer to what the disassembly of the original was doing
							// for additive draw mode but it doesn't look well.
							surface.format.colorToRGB(surfaceColorRGBPrev, rPrev, gPrev, bPrev);
							rgb16bitPrev = (  ((uint16)(rPrev >> 3) << 10)
							                | ((uint16)(gPrev >> 3) <<  5)
							                | ((uint16)(bPrev >> 3)));
							rgb16bitAdd = (((uint16)rgb16bitPrev >> 1) & 0xFBEF)
							                      + ((shpColor >> 1) & 0xFBEF);
							getGameDataColor(rgb16bitAdd, a, r, g, b);
							newSurfaceColorRGB = surface.format.RGBToColor(r, g, b);
						}
					}
				} else {
					newSurfaceColorRGB = surface.format.RGBToColor(r, g, b);
				}
				drawPixel(surface, dstPtr, newSurfaceColorRGB);
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
