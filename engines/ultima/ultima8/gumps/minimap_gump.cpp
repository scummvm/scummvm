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
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/kernel/mouse.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MiniMapGump)

MiniMapGump::MiniMapGump(int x, int y) :
	Gump(x, y, 120, 120, 0, FLAG_DRAGGABLE, LAYER_NORMAL), _minimaps(), _ax(0), _ay(0) {
}

MiniMapGump::MiniMapGump() : Gump(), _minimaps(), _ax(0), _ay(0) {
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

	int32 ax, ay, az;
	actor->getLocation(ax, ay, az);

	ax = ax / (mapChunkSize / MINMAPGUMP_SCALE);
	ay = ay / (mapChunkSize / MINMAPGUMP_SCALE);

	// Skip map update if location has not changed
	if (ax == _ax && ay == _ay)
		return;

	_ax = ax;
	_ay = ay;

	uint32 mapNum = currentmap->getNum();

	MiniMap *minimap = _minimaps[mapNum];
	if (!minimap) {
		minimap = new MiniMap(mapNum);
		_minimaps[mapNum] = minimap;
	}
	minimap->update(currentmap);
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
	minimap->update(currentmap);
}

void MiniMapGump::clear() {
	Common::HashMap<uint32, MiniMap *>::iterator iter;
	for (iter = _minimaps.begin(); iter != _minimaps.end(); ++iter) {
		delete iter->_value;
	}
	_minimaps.clear();
}

void MiniMapGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Draw the yellow border
	surf->Fill32(0xFFFFAF00, 0, 0, _dims.width(), 1);
	surf->Fill32(0xFFFFAF00, 0, 1, 1, _dims.height());
	surf->Fill32(0xFFFFAF00, 1, _dims.bottom - 1, _dims.width(), 1);
	surf->Fill32(0xFFFFAF00, _dims.right - 1, 1, 1, _dims.height());

	// Dimensions minus border
	Common::Rect dims(_dims.left, _dims.top, _dims.right, _dims.bottom);
	dims.grow(-1);

	// Fill the background
	surf->Fill32(0xFF000000, dims.left, dims.top, dims.width(), dims.height());

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

	Graphics::ManagedSurface ms(minimap->getSurface(), DisposeAfterUse::NO);
	Common::Rect r(sx, sy, sx + dims.width(), sy + dims.height());

	if (r.left < 0) {
		dx -= r.left;
		r.left = 0;
	}
	if (r.right > ms.w) {
		r.right = ms.w;
	}

	if (r.top < 0) {
		dy -= r.top;
		r.top = 0;
	}
	if (r.bottom > ms.h) {
		r.bottom = ms.h;
	}

	if (!r.isEmpty()) {
		surf->Blit(ms, r, dx, dy);
	}

	int32 ax = _ax - sx;
	int32 ay = _ay - sy;

	// Paint the avatar position marker
	surf->Fill32(0xFFFFFF00, 1 + ax - 2, 1 + ay + 0, 2, 1);
	surf->Fill32(0xFFFFFF00, 1 + ax + 0, 1 + ay - 2, 1, 2);
	surf->Fill32(0xFFFFFF00, 1 + ax + 1, 1 + ay + 0, 2, 1);
	surf->Fill32(0xFFFFFF00, 1 + ax + 0, 1 + ay + 1, 1, 2);
}

Gump *MiniMapGump::onMouseDown(int button, int32 mx, int32 my) {
	Gump *handled = Gump::onMouseDown(button, mx, my);
	if (handled)
		return handled;

	// only interested in left clicks
	if (button == Shared::BUTTON_LEFT)
		return this;

	return nullptr;
}

void MiniMapGump::onMouseDouble(int button, int32 mx, int32 my) {
	if (button == Shared::BUTTON_LEFT) {
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
