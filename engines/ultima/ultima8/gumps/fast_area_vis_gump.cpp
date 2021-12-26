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

#include "ultima/ultima8/gumps/fast_area_vis_gump.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item.h"
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

	// Put a red dot where the avatar is
	Item *avatar = getItem(1);
	if (avatar) {
		int32 x, y, z;
		avatar->getLocation(x, y, z);
		int chunksize = currentmap->getChunkSize();
		x /= chunksize;
		y /= chunksize;
		if (x >= 0 && x < MAP_NUM_CHUNKS && y >= 0 && y < MAP_NUM_CHUNKS)
			surf->Fill32(0xFFFF1010, x + 1, y + 1, 1, 1);
	}
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
