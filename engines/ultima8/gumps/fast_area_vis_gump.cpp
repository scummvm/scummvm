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

#include "ultima8/misc/pent_include.h"
#include "fast_area_vis_gump.h"
#include "ultima8/kernel/gui_app.h"
#include "ultima8/world/world.h"
#include "ultima8/world/current_map.h"
#include "ultima8/graphics/render_surface.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(FastAreaVisGump, Gump);

FastAreaVisGump::FastAreaVisGump(void) : Gump(0, 0, MAP_NUM_CHUNKS + 2, MAP_NUM_CHUNKS + 2, 0, FLAG_DRAGGABLE | FLAG_DONT_SAVE, LAYER_NORMAL) {
}

FastAreaVisGump::~FastAreaVisGump(void) {
}

void FastAreaVisGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();

	surf->Fill32(0xFF0000, 0, 0, MAP_NUM_CHUNKS + 2, MAP_NUM_CHUNKS + 2);
	surf->Fill32(0, 1, 1, MAP_NUM_CHUNKS, MAP_NUM_CHUNKS);


	for (int y = 0; y < MAP_NUM_CHUNKS; y++)
		for (int x = 0; x < MAP_NUM_CHUNKS; x++)
			if (currentmap->isChunkFast(x, y)) surf->Fill32(0xFFFFFFFF, x + 1, y + 1, 1, 1);
}

void FastAreaVisGump::ConCmd_toggle(const Console::ArgvType &argv) {
	GUIApp *app = GUIApp::get_instance();
	Gump *desktop = app->getDesktopGump();
	Gump *favg = desktop->FindGump(FastAreaVisGump::ClassType);

	if (!favg) {
		favg = new FastAreaVisGump;
		favg->InitGump(0);
		favg->setRelativePosition(TOP_RIGHT, -4, 4);
	} else {
		favg->Close();
	}
}

uint16 FastAreaVisGump::TraceObjId(int mx, int my) {
	uint16 objid = Gump::TraceObjId(mx, my);

	if (!objid || objid == 65535)
		if (PointOnGump(mx, my))
			objid = getObjId();

	return objid;
}
