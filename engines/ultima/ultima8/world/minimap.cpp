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

#include "ultima/ultima8/world/minimap.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/palette.h"

namespace Ultima {
namespace Ultima8 {

MiniMap::MiniMap(uint32 mapNum) : _mapNum(mapNum), _surface() {
	_surface.create((MAP_NUM_CHUNKS * MINMAPGUMP_SCALE), (MAP_NUM_CHUNKS * MINMAPGUMP_SCALE),
					Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
}

MiniMap::~MiniMap() {
	_surface.free();
}

void MiniMap::update(CurrentMap *currentmap) {
	int mapChunkSize = currentmap->getChunkSize();

	// Draw into the map surface
	for (int x = 0; x < _surface.w; x++) {
		for (int y = 0; y < _surface.h; y++) {
			uint32 val = _surface.getPixel(x, y);
			if (val == 0) {
				int cx = x / MINMAPGUMP_SCALE;
				int cy = y / MINMAPGUMP_SCALE;
				if (currentmap->isChunkFast(cx, cy)) {
					int mx = (x * mapChunkSize) / MINMAPGUMP_SCALE;
					int my = (y * mapChunkSize) / MINMAPGUMP_SCALE;

					// Offset produces nicer samples but may need altering
					mx += mapChunkSize / (MINMAPGUMP_SCALE * 2);
					my += mapChunkSize / (MINMAPGUMP_SCALE * 2);

					val = sampleAtPoint(currentmap, mx, my);
					_surface.setPixel(x, y, val);
				}
			}
		}
	}
}

uint32 MiniMap::sampleAtPoint(CurrentMap *currentmap, int x, int y) {
	uint32 val = 0;
	const Item *item = currentmap->traceTopItem(x, y, 1 << 15, -1, 0, ShapeInfo::SI_ROOF | ShapeInfo::SI_OCCL | ShapeInfo::SI_LAND | ShapeInfo::SI_SEA);
	if (item) {
		val = sampleAtPoint(item, x, y);
		if (val == 0) {
			item = currentmap->traceTopItem(x, y, 1 << 15, -1, item->getObjId(), ShapeInfo::SI_ROOF | ShapeInfo::SI_OCCL | ShapeInfo::SI_LAND | ShapeInfo::SI_SEA);
			if (item) {
				val = sampleAtPoint(item, x, y);
			}
		}

		if (val == 0) {
			// set to avoid reprocessing
			val = _surface.format.RGBToColor(0x00, 0x00, 0x00);
		}
	}
	return val;
}

uint32 MiniMap::sampleAtPoint(const Item *item, int x, int y) {
	int32 ix, iy, iz, idx, idy, idz;
	item->getLocation(ix, iy, iz);
	item->getFootpadWorld(idx, idy, idz);

	ix -= x;
	iy -= y;

	const Shape *sh = item->getShapeObject();
	if (!sh)
		return 0;

	const ShapeFrame *frame = sh->getFrame(item->getFrame());
	if (!frame)
		return 0;

	const Palette *pal = sh->getPalette();
	if (!pal)
		return 0;

	if (item->canDrag())
		return 0;

	// Screenspace bounding box bottom x_ coord (RNB x_ coord)
	int sx = (ix - iy) / 4;
	// Screenspace bounding box bottom extent  (RNB y_ coord)
	int sy = (ix + iy) / 8 + idz;

	int w = 2;
	int h = 2;

	// Ensure sample is in bounds of frame
	if (frame->_xoff - sx < 0)
		sx = frame->_xoff;
	else if (frame->_xoff - sx >= frame->_width - w)
		sx = frame->_xoff - frame->_width + w;

	if (frame->_yoff - sy < 0)
		sy = frame->_yoff;
	else if (frame->_yoff - sy >= frame->_height - h)
		sy = frame->_yoff - frame->_height + h;

	uint16 r = 0, g = 0, b = 0, c = 0;

	for (int j = 0; j < w; j++) {
		for (int i = 0; i < h; i++) {
			if (!frame->hasPoint(i - sx, j - sy))
				continue;

			uint8 p = frame->getPixelAtPoint(i - sx, j - sy);
			byte r2 = pal->_palette[p * 3 + 0];
			byte g2 = pal->_palette[p * 3 + 1];
			byte b2 = pal->_palette[p * 3 + 2];
			r += RenderSurface::_gamma22toGamma10[r2];
			g += RenderSurface::_gamma22toGamma10[g2];
			b += RenderSurface::_gamma22toGamma10[b2];
			c++;
		}
	}

	if (c > 0) {
		return _surface.format.RGBToColor(RenderSurface::_gamma10toGamma22[r / c], RenderSurface::_gamma10toGamma22[g / c], RenderSurface::_gamma10toGamma22[b / c]);
	}

	return 0;
}

const Common::Rect MiniMap::getCropBounds() const {
	Common::Rect bounds(_surface.w, _surface.h);
	uint32 mask = _surface.format.ARGBToColor(0x00, 0xFF, 0xFF, 0xFF);

	// Get left
	for (int x = bounds.left; x < bounds.right; x++) {
		for (int y = bounds.top; y < bounds.bottom; y++) {
			uint32 val = _surface.getPixel(x, y);
			if ((val & mask) != 0) {
				bounds.left = x;

				// end loops
				x = bounds.right;
				y = bounds.bottom;
			}
		}
	}

	// Get top
	for (int y = bounds.top; y < bounds.bottom; y++) {
		for (int x = bounds.left; x < bounds.right; x++) {
			uint32 val = _surface.getPixel(x, y);
			if ((val & mask) != 0) {
				bounds.top = y;

				// end loops
				x = bounds.right;
				y = bounds.bottom;
			}
		}
	}

	// Get right
	for (int x = bounds.right - 1; x > bounds.left; x--) {
		for (int y = bounds.bottom - 1; y > bounds.top; y--) {
			uint32 val = _surface.getPixel(x, y);
			if ((val & mask) != 0) {
				bounds.right = x + 1;

				// end loops
				x = bounds.left;
				y = bounds.top;
			}
		}
	}

	// Get bottom
	for (int y = bounds.bottom - 1; y > bounds.top; y--) {
		for (int x = bounds.right - 1; x > bounds.left; x--) {
			uint32 val = _surface.getPixel(x, y);
			if ((val & mask) != 0) {
				bounds.bottom = y + 1;

				// end loops
				x = bounds.left;
				y = bounds.top;
			}
		}
	}

	return bounds;
}

bool MiniMap::load(Common::ReadStream *rs, uint32 version) {
	//_mapNum = rs->readUint32LE();

	Common::Rect bounds;
	bounds.left = rs->readUint16LE();
	bounds.top = rs->readUint16LE();
	bounds.right = rs->readUint16LE();
	bounds.bottom = rs->readUint16LE();

	Graphics::PixelFormat format;
	format.bytesPerPixel = rs->readByte();
	format.rLoss = rs->readByte();
	format.gLoss = rs->readByte();
	format.bLoss = rs->readByte();
	format.aLoss = rs->readByte();
	format.rShift = rs->readByte();
	format.gShift = rs->readByte();
	format.bShift = rs->readByte();
	format.aShift = rs->readByte();

	if (format.bytesPerPixel != 2) {
		error("unsupported minimap texture format %d bpp", format.bytesPerPixel);
		return false;
	}

	uint16 w = _surface.w;
	uint16 h = _surface.h;
	_surface.create(w, h, format);
	for (int y = bounds.top; y < bounds.bottom; ++y) {
		uint16 *pixels = (uint16 *)_surface.getBasePtr(bounds.left, y);
		for (int x = bounds.left; x < bounds.right; ++x) {
			*pixels++ = rs->readUint16LE();
		}
	}
	return true;
}

void MiniMap::save(Common::WriteStream *ws) const {
	//ws->writeUint32LE(_mapNum);

	Common::Rect bounds = getCropBounds();
	ws->writeUint16LE(bounds.left);
	ws->writeUint16LE(bounds.top);
	ws->writeUint16LE(bounds.right);
	ws->writeUint16LE(bounds.bottom);

	// Serialize the PixelFormat
	ws->writeByte(_surface.format.bytesPerPixel);
	ws->writeByte(_surface.format.rLoss);
	ws->writeByte(_surface.format.gLoss);
	ws->writeByte(_surface.format.bLoss);
	ws->writeByte(_surface.format.aLoss);
	ws->writeByte(_surface.format.rShift);
	ws->writeByte(_surface.format.gShift);
	ws->writeByte(_surface.format.bShift);
	ws->writeByte(_surface.format.aShift);

	// Serialize the pixel data
	for (int y = bounds.top; y < bounds.bottom; ++y) {
		const uint16 *pixels = (const uint16 *)_surface.getBasePtr(bounds.left, y);
		for (int x = bounds.left; x < bounds.right; ++x) {
			ws->writeUint16LE(*pixels++);
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
