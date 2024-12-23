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
#include "common/file.h"

#include "image/png.h"
#include "image/bmp.h"

#include "ultima/ultima8/world/minimap.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/gfx/palette_manager.h"

namespace Ultima {
namespace Ultima8 {

static const uint BLACK_COLOR = 0;
static const uint KEY_COLOR = 255;

MiniMap::MiniMap(uint32 mapNum) : _mapNum(mapNum), _surface() {
	uint16 w = MAP_NUM_CHUNKS * MINMAPGUMP_SCALE;
	uint16 h = MAP_NUM_CHUNKS * MINMAPGUMP_SCALE;
	_surface.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	_surface.fillRect(Common::Rect(w, h), KEY_COLOR);
}

MiniMap::~MiniMap() {
	_surface.free();
}

void MiniMap::update(const CurrentMap &map) {
	int mapChunkSize = map.getChunkSize();

	// Draw into the map surface
	for (int x = 0; x < _surface.w; x++) {
		for (int y = 0; y < _surface.h; y++) {
			uint32 val = _surface.getPixel(x, y);
			if (val == KEY_COLOR) {
				int cx = x / MINMAPGUMP_SCALE;
				int cy = y / MINMAPGUMP_SCALE;
				if (map.isChunkFast(cx, cy)) {
					int mx = (x * mapChunkSize) / MINMAPGUMP_SCALE;
					int my = (y * mapChunkSize) / MINMAPGUMP_SCALE;

					// Offset produces nicer samples but may need altering
					mx += mapChunkSize / (MINMAPGUMP_SCALE * 2);
					my += mapChunkSize / (MINMAPGUMP_SCALE * 2);

					val = sampleAtPoint(map, mx, my);
					_surface.setPixel(x, y, val);
				}
			}
		}
	}
}

Common::Point MiniMap::getItemLocation(const Item &item, unsigned int chunkSize) {
	Point3 pt = item.getLocation();

	pt.x = pt.x / (chunkSize / MINMAPGUMP_SCALE);
	pt.y = pt.y / (chunkSize / MINMAPGUMP_SCALE);
	return Common::Point(pt.x, pt.y);
}

uint32 MiniMap::sampleAtPoint(const CurrentMap &map, int x, int y) {
	Point3 start(x, y, 1 << 15);
	Point3 end(x, y, -1);
	int32 dims[3] = {0, 0, 0};
	uint32 shflags = ShapeInfo::SI_ROOF | ShapeInfo::SI_OCCL | ShapeInfo::SI_LAND | ShapeInfo::SI_SEA;
	Std::list<CurrentMap::SweepItem> collisions;
	if (map.sweepTest(start, end, dims, shflags, 0, false, &collisions)) {
		Std::list<CurrentMap::SweepItem>::const_iterator it;
		for (it = collisions.begin(); it != collisions.end(); it++) {
			const Item *item = getItem(it->_item);
			if (item) {
				const ShapeInfo *si = item->getShapeInfo();
				if (!(si->_flags & shflags) || si->is_editor() || si->is_translucent())
					continue;

				uint32 val = sampleAtPoint(*item, x, y);
				if (val != KEY_COLOR && val != BLACK_COLOR)
					return val;
			}
		}
	}

	// set to avoid reprocessing
	return BLACK_COLOR;
}

uint32 MiniMap::sampleAtPoint(const Item &item, int x, int y) {
	int32 idx, idy, idz;
	Point3 pt = item.getLocation();
	item.getFootpadWorld(idx, idy, idz);

	pt.x -= x;
	pt.y -= y;

	const Shape *sh = item.getShapeObject();
	if (!sh)
		return KEY_COLOR;

	const ShapeFrame *frame = sh->getFrame(item.getFrame());
	if (!frame)
		return KEY_COLOR;

	const Palette *pal = sh->getPalette();
	if (!pal)
		return KEY_COLOR;

	if (item.canDrag())
		return KEY_COLOR;

	// Screenspace bounding box bottom x_ coord (RNB x_ coord)
	int sx = (pt.x - pt.y) / 4;
	// Screenspace bounding box bottom extent  (RNB y_ coord)
	int sy = (pt.x + pt.y) / 8 + idz;

	int w = 3;
	int h = 3;

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

			uint8 p = frame->getPixel(i - sx, j - sy);
			byte r2, g2, b2;
			pal->get(p, r2, g2, b2);
			r += r2;
			g += g2;
			b += b2;
			c++;
		}
	}

	if (c > 0) {
		return pal->findBestColor(r / c, g / c, b / c);
	}

	return KEY_COLOR;
}

const Common::Rect MiniMap::getCropBounds() const {
	Common::Rect bounds(_surface.w, _surface.h);

	// Get left
	for (int x = bounds.left; x < bounds.right; x++) {
		for (int y = bounds.top; y < bounds.bottom; y++) {
			uint32 val = _surface.getPixel(x, y);
			if (val != KEY_COLOR && val != BLACK_COLOR) {
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
			if (val != KEY_COLOR && val != BLACK_COLOR) {
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
			if (val != KEY_COLOR && val != BLACK_COLOR) {
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
			if (val != KEY_COLOR && val != BLACK_COLOR) {
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

	uint16 w = _surface.w;
	uint16 h = _surface.h;
	_surface.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	_surface.fillRect(Common::Rect(w, h), KEY_COLOR);

	if (format.bytesPerPixel == 1) {
		for (int y = bounds.top; y < bounds.bottom; ++y) {
			uint8 *pixels = (uint8 *)_surface.getBasePtr(bounds.left, y);
			for (int x = bounds.left; x < bounds.right; ++x) {
				*pixels++ = rs->readByte();
			}
		}
	} else if (format.bytesPerPixel == 2) {
		// Convert format to palette
		Palette *p = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
		Graphics::PaletteLookup pl(p->data(), p->size());
		for (int y = bounds.top; y < bounds.bottom; ++y) {
			uint8 *pixels = (uint8 *)_surface.getBasePtr(bounds.left, y);
			for (int x = bounds.left; x < bounds.right; ++x) {
				uint16 color = rs->readUint16LE();
				if (color) {
					byte r, g, b;
					format.colorToRGB(color, r, g, b);
					*pixels++ = pl.findBestColor(r, g, b);
				} else {
					*pixels++ = KEY_COLOR;
				}
			}
		}
	} else {
		error("unsupported minimap texture format %d bpp", format.bytesPerPixel);
		return false;
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
		const uint8 *pixels = (const uint8 *)_surface.getBasePtr(bounds.left, y);
		for (int x = bounds.left; x < bounds.right; ++x) {
			ws->writeByte(*pixels++);
		}
	}
}

bool MiniMap::dump(const Common::Path &filename) const {
	Palette *p = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	Common::DumpFile dumpFile;
	bool result = dumpFile.open(filename);
	if (result) {
#ifdef USE_PNG
		result = Image::writePNG(dumpFile, _surface, p->data());
#else
		result = Image::writeBMP(dumpFile, _surface, p->data());
#endif
	}
	return result;
}

} // End of namespace Ultima8
} // End of namespace Ultima
