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
#include "ultima/ultima8/world/minimap.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/texture.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/kernel/mouse.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MiniMapGump)

static const uint BACKGROUND_COLOR = 0;
static const uint NORMAL_COLOR = 53;
static const uint HIGHLIGHT_COLOR = 52;
static const uint KEY_COLOR = 255;

MiniMapGump::MiniMapGump(int x, int y) : ResizableGump(x, y, 120, 120), _minimaps(), _ax(0), _ay(0) {
	setMinSize(60, 60);
}

MiniMapGump::MiniMapGump() : ResizableGump(), _minimaps(), _ax(0), _ay(0) {
	setMinSize(60, 60);
}

MiniMapGump::~MiniMapGump(void) {
	Common::HashMap<uint32, MiniMap *>::iterator iter;
	for (iter = _minimaps.begin(); iter != _minimaps.end(); ++iter) {
		delete iter->_value;
	}
}

void MiniMapGump::run() {
	Gump::run();

	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	int mapChunkSize = currentmap->getChunkSize();

	MainActor *actor = getMainActor();
	if (!actor || actor->isDead())
		return;

	uint32 mapNum = currentmap->getNum();
	MiniMap *minimap = _minimaps[mapNum];
	if (!minimap) {
		minimap = new MiniMap(mapNum);
		_minimaps[mapNum] = minimap;
	}

	Common::Point p = minimap->getItemLocation(*actor, mapChunkSize);

	// Skip map update if location has not changed
	if (p.x == _ax && p.y == _ay)
		return;

	_ax = p.x;
	_ay = p.y;

	minimap->update(*currentmap);
}

void MiniMapGump::generate() {
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	currentmap->setWholeMapFast();

	uint32 mapNum = currentmap->getNum();

	MiniMap *minimap = _minimaps[mapNum];
	if (!minimap) {
		minimap = new MiniMap(mapNum);
		_minimaps[mapNum] = minimap;
	}
	minimap->update(*currentmap);
}

void MiniMapGump::clear() {
	Common::HashMap<uint32, MiniMap *>::iterator iter;
	for (iter = _minimaps.begin(); iter != _minimaps.end(); ++iter) {
		delete iter->_value;
	}
	_minimaps.clear();
}

bool MiniMapGump::dump(const Common::Path &filename) const {
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();

	uint32 mapNum = currentmap->getNum();

	MiniMap *minimap = _minimaps[mapNum];
	return minimap ? minimap->dump(filename) : false;	
}

void MiniMapGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Palette *pal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	uint32 *map = pal->_native;

	uint32 color = map[NORMAL_COLOR];
	if (_dragPosition != Gump::CENTER || _mousePosition != Gump::CENTER)
		color = map[HIGHLIGHT_COLOR];

	// Draw the border
	surf->frameRect(_dims, color);

	// Dimensions minus border
	Rect dims = _dims;
	dims.grow(-1);

	// Fill the background
	surf->fillRect(dims, map[BACKGROUND_COLOR]);

	// Center on avatar
	int sx = _ax - dims.width() / 2;
	int sy = _ay - dims.height() / 2;
	int dx = 1;
	int dy = 1;

	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	uint32 mapNum = currentmap->getNum();

	MiniMap *minimap = _minimaps[mapNum];
	if (!minimap) {
		minimap = new MiniMap(mapNum);
		_minimaps[mapNum] = minimap;
	}

	const Graphics::Surface *ms = minimap->getSurface();
	Common::Rect r(sx, sy, sx + dims.width(), sy + dims.height());

	if (r.left < 0) {
		dx -= r.left;
		r.left = 0;
	}
	if (r.right > ms->w) {
		r.right = ms->w;
	}

	if (r.top < 0) {
		dy -= r.top;
		r.top = 0;
	}
	if (r.bottom > ms->h) {
		r.bottom = ms->h;
	}

	if (!r.isEmpty()) {
		surf->CrossKeyBlitMap(*ms, r, dx, dy, map, KEY_COLOR);
	}

	int32 ax = _ax - sx;
	int32 ay = _ay - sy;

	// Paint the avatar position marker
	surf->drawLine(ax - 1, ay + 1, ax, ay + 1, color);
	surf->drawLine(ax + 1, ay - 1, ax + 1, ay, color);
	surf->drawLine(ax + 2, ay + 1, ax + 3, ay + 1, color);
	surf->drawLine(ax + 1, ay + 2, ax + 1, ay + 3, color);
}

Gump *MiniMapGump::onMouseDown(int button, int32 mx, int32 my) {
	Gump *handled = Gump::onMouseDown(button, mx, my);
	if (handled)
		return handled;

	// only interested in left clicks
	if (button == Mouse::BUTTON_LEFT)
		return this;

	return nullptr;
}

void MiniMapGump::onMouseDouble(int button, int32 mx, int32 my) {
	if (button == Mouse::BUTTON_LEFT) {
		HideGump();
	}
}

void MiniMapGump::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_minimaps.size()));
	Common::HashMap<uint32, MiniMap *>::const_iterator iter;
	for (iter = _minimaps.begin(); iter != _minimaps.end(); ++iter) {
		const MiniMap *minimap = iter->_value;
		ws->writeUint32LE(iter->_key);
		minimap->save(ws);
	}
}

bool MiniMapGump::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Gump::loadData(rs, version))
		return false;

	_ax = 0;
	_ay = 0;

	clear();

	if (version >= 6) {
		uint32 mapcount = rs->readUint32LE();
		for (uint32 i = 0; i < mapcount; ++i) {
			uint32 mapNum = rs->readUint32LE();
			MiniMap *minimap = new MiniMap(mapNum);
			if (!minimap->load(rs, version))
				return false;
			_minimaps[mapNum] = minimap;
		}
	}
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
