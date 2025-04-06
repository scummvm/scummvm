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

#include "ultima/ultima.h"
#include "ultima/ultima8/misc/debugger.h"

#include "ultima/ultima8/world/missile_tracker.h"

#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

MissileTracker::MissileTracker(const Item *item, ObjId owner,
							   int32 sx, int32 sy, int32 sz,
							   int32 tx, int32 ty, int32 tz,
							   int32 speed, int32 gravity) :
		_owner(owner), _destX(tx), _destY(ty), _destZ(tz), _gravity(gravity) {
	_objId = item->getObjId();

	init(sx, sy, sz, speed);
}

MissileTracker::MissileTracker(const Item *item, ObjId owner,
							   int32 tx, int32 ty, int32 tz,
							   int32 speed, int32 gravity) :
		_owner(owner), _destX(tx), _destY(ty), _destZ(tz), _gravity(gravity)  {
	assert(item->getParent() == 0);

	_objId = item->getObjId();

	Point3 pt = item->getLocation();
	init(pt.x, pt.y, pt.z, speed);
}

void MissileTracker::init(int32 x, int32 y, int32 z, int32 speed) {
	int range = ABS(x - _destX) + ABS(y - _destY);

	// rounded division: range/speed
	_frames = (range + (speed / 2)) / speed;

	/*

	Item's vertical trajectory:

	z_{i+1} = z_i + s_i
	s_{i+1} = s_i - g

	(z_i = vertical position after i _frames,
	 s_i = vertical speed after i _frames, g = _gravity)

	So:

	z_i = z + sum_{j=0}^{i-1} ( s_0 - jg)
	    = z + is_0 - 1/2 i(i-1)g

	Conclusion: if we want to reach the destination vertical level in i _frames,
	we need to set

	s_0 = ((1/2 gi(i-1)) + z_i-z) / i

	*/

	if (_frames > 0) {
		_speedZ = ((_gravity * _frames * (_frames - 1) / 2) + _destZ - z) / _frames;

		// check if vertical speed isn't too high
		if (_speedZ > speed / 4) {
			if (_gravity == 0 || (speed / (4 * _gravity)) <= _frames) {
				if (speed >= 4 && (_destZ - z) / (speed / 4) > _frames)
					_frames = (_destZ - z) / (speed / 4);
			} else {
				_frames = speed / (4 * _gravity);
			}
		}

		_speedZ = ((_gravity * _frames * (_frames - 1) / 2) + _destZ - z) / _frames;

		// horizontal speed is easier: just divide distance by _frames
		_speedX = ((_destX - x) + (_frames / 2)) / _frames;
		_speedY = ((_destY - y) + (_frames / 2)) / _frames;

		debugC(kDebugCollision, "MissileTracker: from (%d,%d,%d) to (%d,%d,%d)", x, y, z, _destX, _destY, _destZ);
		debugC(kDebugCollision, "speed: %d, _gravity: %d, _frames: %d", speed, _gravity, _frames);
		debugC(kDebugCollision, "resulting speed: (%d,%d,%d)", _speedX, _speedY, _speedZ);
	} else {

		// no significant horizontal movement
		if (_destZ > z)
			_speedZ = speed / 4;
		else
			_speedZ = -speed / 4;

	}
}

MissileTracker::~MissileTracker() {
}

bool MissileTracker::isPathClear() const {
	Point3 start;
	Point3 end;
	int32 dims[3];
	int32 sx, sy, sz;

	sx = _speedX;
	sy = _speedY;
	sz = _speedZ;

	World *world = World::get_instance();
	CurrentMap *map = world->getCurrentMap();
	Item *item = getItem(_objId);

	if (!item) {
		// Item disappeared? shouldn't happen, but call the path clear.
		return true;
	}

	item->getFootpadWorld(dims[0], dims[1], dims[2]);
	start = item->getLocation();

	for (int f = 0; f < _frames; ++f) {
		end.x = start.x + sx;
		end.y = start.y + sy;
		end.z = start.z + sz;

		// Do the sweep test
		Std::list<CurrentMap::SweepItem> collisions;
		map->sweepTest(start, end, dims, item->getShapeInfo()->_flags, _objId,
		               false, &collisions);

		int32 hit = 0x4000;
		for (const auto &collision : collisions) {
			if (collision._blocking && !collision._touching && collision._item != _owner) {
				hit = collision._hitTime;
				break;
			}
		}
		if (hit != 0x4000) {
			// didn't reach end of this path segment
			return false;
		}

		sz -= _gravity;
		start = end;
	}

	return true;
}


void MissileTracker::launchItem() {
	Item *item = getItem(_objId);
	if (!item) return;

	item->hurl(_speedX, _speedY, _speedZ, _gravity);
}

} // End of namespace Ultima8
} // End of namespace Ultima
