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

#include "ultima/ultima8/gumps/minimap_gump.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_sorter.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MiniMapGump)


static const int MINMAPGUMP_SCALE = 8;

MiniMapGump::MiniMapGump(int x, int y) :
	Gump(x, y, MAP_NUM_CHUNKS * 2 + 2, MAP_NUM_CHUNKS * 2 + 2, 0,
	     FLAG_DRAGGABLE, LAYER_NORMAL), _minimap(), _lastMapNum(0) {
	_minimap.create((MAP_NUM_CHUNKS * MINMAPGUMP_SCALE), (MAP_NUM_CHUNKS * MINMAPGUMP_SCALE),
		Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
}

MiniMapGump::MiniMapGump() : Gump() , _lastMapNum(0){
	_minimap.create((MAP_NUM_CHUNKS * MINMAPGUMP_SCALE), (MAP_NUM_CHUNKS * MINMAPGUMP_SCALE),
		Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
}

MiniMapGump::~MiniMapGump(void) {
}

void MiniMapGump::run() {
	Gump::run();

	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	int mapChunkSize = currentmap->getChunkSize();

	if (currentmap->getNum() != _lastMapNum) {
		_lastMapNum = currentmap->getNum();
		_minimap.clear();
	}

	// Draw into the map surface
	for (int yv = 0; yv < MAP_NUM_CHUNKS; yv++) {
		for (int xv = 0; xv < MAP_NUM_CHUNKS; xv++) {
			if (currentmap->isChunkFast(xv, yv)) {
				for (int j = 0; j < MINMAPGUMP_SCALE; j++) for (int i = 0; i < MINMAPGUMP_SCALE; i++) {
					int x = xv * MINMAPGUMP_SCALE + i;
					int y = yv * MINMAPGUMP_SCALE + j;
					uint32 val = _minimap.getPixel(x, y);
					if (val == 0) {
						val = sampleAtPoint(currentmap,
							xv * mapChunkSize + mapChunkSize / (MINMAPGUMP_SCALE * 2) + (mapChunkSize * i) / MINMAPGUMP_SCALE,
							yv * mapChunkSize + mapChunkSize / (MINMAPGUMP_SCALE * 2) + (mapChunkSize * j) / MINMAPGUMP_SCALE);
						_minimap.setPixel(x, y, val);
					}
				}
			}
		}
	}
}

void MiniMapGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	int mapChunkSize = currentmap->getChunkSize();

	// Draw the yellow border
	surf->Fill32(0xFFFFAF00, 0, 0, _dims.width(), 1);
	surf->Fill32(0xFFFFAF00, 0, 1, 1, _dims.height());
	surf->Fill32(0xFFFFAF00, 1, _dims.bottom - 1, _dims.width(), 1);
	surf->Fill32(0xFFFFAF00, _dims.right - 1, 1, 1, _dims.height());

	// Dimensions minus border
	Rect dims = _dims;
	dims.grow(-1);

	// Fill the background
	surf->Fill32(0xFF000000, dims.left, dims.top, dims.width(), dims.height());

	// Center on avatar
	int sx = 0, sy = 0, ox = 0, oy = 0, lx = 0, ly = 0;

	MainActor *av = getMainActor();
	int32 ax, ay, az;
	av->getLocation(ax, ay, az);

	ax = ax / (mapChunkSize / MINMAPGUMP_SCALE);
	ay = ay / (mapChunkSize / MINMAPGUMP_SCALE);

	sx = ax - (mapChunkSize / (4 * 2));
	sy = ay - (mapChunkSize / (4 * 2));
	ax = ax - sx;
	ay = ay - sy;

	if (sx < 0) {
		ox = -sx;
	} else if ((sx + dims.width()) > _minimap.w) {
		lx = (sx + dims.width()) - _minimap.w;
	}

	if (sy < 0) {
		oy = -sy;
	} else if ((sy + dims.height()) > _minimap.h) {
		ly = (sy + dims.height()) - _minimap.h;
	}

	surf->Blit(&_minimap, sx + ox, sy + oy, dims.width() - (ox + lx), dims.height() - (oy + ly), 1 + ox, 1 + oy);

	// Paint the avatar position marker
	surf->Fill32(0xFFFFFF00, 1 + ax - 2, 1 + ay + 0, 2, 1);
	surf->Fill32(0xFFFFFF00, 1 + ax + 0, 1 + ay - 2, 1, 2);
	surf->Fill32(0xFFFFFF00, 1 + ax + 1, 1 + ay + 0, 2, 1);
	surf->Fill32(0xFFFFFF00, 1 + ax + 0, 1 + ay + 1, 1, 2);
}

uint32 MiniMapGump::sampleAtPoint(CurrentMap *currentmap, int x, int y) {
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
			val = _minimap.format.RGBToColor(0x00, 0x00, 0x00);
		}
	}
	return val;
}


uint32 MiniMapGump::sampleAtPoint(const Item *item, int x, int y)
{
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

	// Screenspace bounding box bottom x_ coord (RNB x_ coord)
	int sx = (ix - iy) / 4;
	// Screenspace bounding box bottom extent  (RNB y_ coord)
	int sy = (ix + iy) / 8 + idz;

	uint16 r = 0, g = 0, b = 0, c = 0;

	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			if (!frame->hasPoint(i - sx, j - sy)) continue;

			byte r2, g2, b2;
			UNPACK_RGB8(pal->_native_untransformed[frame->getPixelAtPoint(i - sx, j - sy)], r2, g2, b2);
			r += RenderSurface::_gamma22toGamma10[r2];
			g += RenderSurface::_gamma22toGamma10[g2];
			b += RenderSurface::_gamma22toGamma10[b2];
			c++;
		}
	}

	if (c > 0) {
		return _minimap.format.RGBToColor(RenderSurface::_gamma10toGamma22[r / c], RenderSurface::_gamma10toGamma22[g / c], RenderSurface::_gamma10toGamma22[b / c]);
	}

	return 0;
}

void MiniMapGump::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);
}

bool MiniMapGump::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Gump::loadData(rs, version))
		return false;

	_lastMapNum = 0;
	_minimap.clear();

	return true;
}

uint16 MiniMapGump::TraceObjId(int32 mx, int32 my) {
	uint16 objId = Gump::TraceObjId(mx, my);

	if (!objId || objId == 65535)
		if (PointOnGump(mx, my))
			objId = getObjId();

	return objId;
}

} // End of namespace Ultima8
} // End of namespace Ultima
