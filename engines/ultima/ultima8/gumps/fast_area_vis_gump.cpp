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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/fast_area_vis_gump.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/graphics/render_surface.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(FastAreaVisGump)

FastAreaVisGump::FastAreaVisGump(void) : Gump(0, 0, MAP_NUM_CHUNKS + 2, MAP_NUM_CHUNKS + 2, 0, FLAG_DRAGGABLE | FLAG_DONT_SAVE, LAYER_NORMAL) {
}

FastAreaVisGump::~FastAreaVisGump(void) {
}

void FastAreaVisGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();

	surf->Fill32(0xFF0000, 0, 0, MAP_NUM_CHUNKS + 2, MAP_NUM_CHUNKS + 2);
	surf->Fill32(0, 1, 1, MAP_NUM_CHUNKS, MAP_NUM_CHUNKS);


	for (int yp = 0; yp < MAP_NUM_CHUNKS; yp++)
		for (int xp = 0; xp < MAP_NUM_CHUNKS; xp++)
			if (currentmap->isChunkFast(xp, yp)) surf->Fill32(0xFFFFFFFF, xp + 1, yp + 1, 1, 1);
}

uint16 FastAreaVisGump::TraceObjId(int32 mx, int32 my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);

	if (!objId_ || objId_ == 65535)
		if (PointOnGump(mx, my))
			objId_ = getObjId();

	return objId_;
}

} // End of namespace Ultima8
} // End of namespace Ultima
