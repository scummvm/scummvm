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

#include "ultima/ultima8/world/missile_tracker.h"

#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

MissileTracker::MissileTracker(Item *item, int32 sx, int32 sy, int32 sz,
                               int32 tx, int32 ty, int32 tz,
                               int32 speed, int32 gravity_) {
	objid = item->getObjId();
	destx = tx;
	desty = ty;
	destz = tz;
	gravity = gravity_;

	init(sx, sy, sz, speed);
}

MissileTracker::MissileTracker(Item *item, int32 tx, int32 ty, int32 tz,
                               int32 speed, int32 gravity_) {
	assert(item->getParent() == 0);

	objid = item->getObjId();
	destx = tx;
	desty = ty;
	destz = tz;
	gravity = gravity_;

	int32 x, y, z;
	item->getLocation(x, y, z);

	init(x, y, z, speed);
}

void MissileTracker::init(int32 x, int32 y, int32 z, int32 speed) {
	int range = ABS(x - destx) + ABS(y - desty);

	// rounded division: range/speed
	frames = (range + (speed / 2)) / speed;

	/*

	Item's vertical trajectory:

	z_{i+1} = z_i + s_i
	s_{i+1} = s_i - g

	(z_i = vertical position after i frames,
	 s_i = vertical speed after i frames, g = gravity)

	So:

	z_i = z + sum_{j=0}^{i-1} ( s_0 - jg)
	    = z + is_0 - 1/2 i(i-1)g

	Conclusion: if we want to reach the destination vertical level in i frames,
	we need to set

	s_0 = ((1/2 gi(i-1)) + z_i-z) / i

	*/

	if (frames > 0) {
		speedz = ((gravity * frames * (frames - 1) / 2) + destz - z) / frames;

		// check if vertical speed isn't too high
		if (speedz > speed / 4) {
			if (gravity == 0 || (speed / (4 * gravity)) <= frames) {
				if (speed >= 4 && (destz - z) / (speed / 4) > frames)
					frames = (destz - z) / (speed / 4);
			} else {
				frames = speed / (4 * gravity);
			}
		}

		speedz = ((gravity * frames * (frames - 1) / 2) + destz - z) / frames;

		// horizontal speed is easier: just divide distance by frames
		speedx = ((destx - x) + (frames / 2)) / frames;
		speedy = ((desty - y) + (frames / 2)) / frames;

#if 0
		pout.printf("MissileTracker: from (%d,%d,%d) to (%d,%d,%d)\n", x, y, z, destx, desty, destz);
		pout.printf("speed: %d, gravity: %d, frames: %d\n", speed, gravity, frames);
		pout.printf("resulting speed: (%d,%d,%d)\n", speedx, speedy, speedz);
#endif
	} else {

		// no significant horizontal movement
		if (destz > z)
			speedz = speed / 4;
		else
			speedz = -speed / 4;

	}
}

MissileTracker::~MissileTracker() {

}

bool MissileTracker::isPathClear() {
	int32 start[3];
	int32 end[3];
	int32 dims[3];
	int32 sx, sy, sz;

	sx = speedx;
	sy = speedy;
	sz = speedz;

	World *world = World::get_instance();
	CurrentMap *map = world->getCurrentMap();
	Item *item = getItem(objid);

	item->getFootpadWorld(dims[0], dims[1], dims[2]);
	item->getLocation(start[0], start[1], start[2]);

	for (int f = 0; f < frames; ++f) {
		end[0] = start[0] + sx;
		end[1] = start[1] + sy;
		end[2] = start[2] + sz;

		// Do the sweep test
		Std::list<CurrentMap::SweepItem> collisions;
		Std::list<CurrentMap::SweepItem>::iterator it;
		map->sweepTest(start, end, dims, item->getShapeInfo()->flags, objid,
		               false, &collisions);

		int32 hit = 0x4000;
		for (it = collisions.begin(); it != collisions.end(); it++) {
			if (it->blocking && !it->touching) {
				hit = it->hit_time;
				break;
			}
		}
		if (hit != 0x4000) {
			// didn't reach end of this path segment
			return false;
		}

		sz -= gravity;
		for (int i = 0; i < 3; ++i) start[i] = end[i];
	}

	return true;
}


void MissileTracker::launchItem() {
	Item *item = getItem(objid);
	if (!item) return;

	item->hurl(speedx, speedy, speedz, gravity);
}

} // End of namespace Ultima8
} // End of namespace Ultima
