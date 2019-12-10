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

#include "ultima8/world/gravity_process.h"

#include "ultima8/world/actors/actor.h"
#include "ultima8/audio/audio_process.h"
#include "ultima8/world/current_map.h"
#include "ultima8/kernel/kernel.h"
#include "ultima8/world/world.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(GravityProcess, Process)

GravityProcess::GravityProcess()
	: Process() {

}

GravityProcess::GravityProcess(Item *item, int gravity_)
	: xspeed(0), yspeed(0), zspeed(0) {
	assert(item);

	gravity = gravity_;
	item_num = item->getObjId();

	type = 0x203; // CONSTANT!
}

void GravityProcess::init() {
	Item *item = getItem(item_num);
	assert(item);

	item->setGravityPID(getPid());

	Actor *actor = p_dynamic_cast<Actor *>(item);
	if (actor) {
		actor->setFallStart(actor->getZ());
	}
}

void GravityProcess::move(int xs, int ys, int zs) {
	xspeed += xs;
	yspeed += ys;
	zspeed += zs;
}

void GravityProcess::setGravity(int gravity_) {
	// only apply gravity if stronger than current gravity
	//!!! is this correct?
	if (gravity_ > gravity)
		gravity = gravity_;
}

void GravityProcess::run() {
	// move item in (xs,ys,zs) direction
	Item *item = getItem(item_num);
	if (!item) {
		terminate();
		return;
	}

	Actor *actor = p_dynamic_cast<Actor *>(item);
	if (actor && actor->getFallStart() < actor->getZ()) {
		actor->setFallStart(actor->getZ());
	}

	// What to do:
	//   - check if item can move to the given position
	//     (in intervals in the z direction, since the z movement
	//      can be quite large)
	//   - if item can move, move it
	//   - if item can't move, it hit something:
	//     - bounce off the item (need to consider FLG_LOW_FRICTION?)
	//     - call the relevant events: hit/gothit ?

	int32 ix, iy, iz;
	item->getLocation(ix, iy, iz);
	int32 ixd, iyd, izd;
	item->getFootpadWorld(ixd, iyd, izd);

	int32 tx, ty, tz;
	tx = ix + xspeed;
	ty = iy + yspeed;
	tz = iz + zspeed;

	bool clipped = false;

	// Clip to region. This doesn't work
#if 0
	if (tx < 0 && ix >= 0) {
		int32 scale = (ix - tx) >> 0x8;
		tx = 0;
		ty = iy + ((yspeed * scale) >> 0x2000);
		tz = iz + ((zspeed * scale) >> 0x2000);
		clipped = true;
	}
	if (ty < 0 && iy >= 0) {
		int32 scale = (iy - ty) >> 0x8;
		tx = ix + ((xspeed * scale) >> 0x2000);
		ty = 0;
		tz = iz + ((zspeed * scale) >> 0x2000);
		clipped = true;
	}
	if (tz < 0 && iz >= 0) {
		int32 scale = (iz - tz) >> 0x8;
		tx = ix + ((xspeed * scale) >> 0x2000);
		ty = iy + ((yspeed * scale) >> 0x2000);
		tz = 0;
		clipped = true;
	}
#endif

//#define BOUNCE_DIAG

	ObjId hititemid;
	uint8 dirs;
	int32 dist = item->collideMove(tx, ty, tz, false, false, &hititemid, &dirs);

	if (dist == 0x4000 && !clipped) {
		// normal move
		zspeed -= gravity;
		return;
	}

	// TODO: after doing a partial move we may still want to do another
	// partial move in this ::run() call to move the full speed this frame.
	// The effect might not be visible enough to be worth the trouble though.

	// Item was blocked


	// We behave differently depending on which direction was blocked.
	// We only consider stopping to bounce when blocked purely in the
	// downward-Z direction. Other directions always bounce, reducing speed in
	// the blocked directions


	// only blocked going down?
	if (dirs == 4 && zspeed < 0) {

		// If it landed on top of hititem and hititem is not land, the item
		// should always bounce.

		bool terminate = true;
		Item *hititem = getItem(hititemid);
		if (zspeed < -2 && !p_dynamic_cast<Actor *>(item)) {
#ifdef BOUNCE_DIAG
			pout << "item " << item_num << " bounce ["
			     << Kernel::get_instance()->getFrameNum()
			     << "]: hit " << hititem->getObjId() << std::endl;
#endif

			if (!hititem->getShapeInfo()->is_land() || zspeed < -2 * gravity) {
				// Bounce!
				terminate = false;
#ifdef BOUNCE_DIAG
				int xspeedold = xspeed;
				int yspeedold = yspeed;
				int zspeedold = zspeed;
#endif
				zspeed = 0 - zspeed / 3;
				int approx_v = ABS(xspeed) + ABS(yspeed) + zspeed;

				// Apply an impulse on the x/y plane in a random direction
				// in a 180 degree pie around the orginal vector in x/y
				double heading_r = atan2((double)yspeed, (double)xspeed);
				double deltah_r = static_cast<double>(rand())
				                  * M_PI / RAND_MAX - M_PI / 2;
#ifdef BOUNCE_DIAG
				double headingold_r = heading_r;
#endif
				heading_r += deltah_r;
				if (heading_r > M_PI) heading_r -= 2 * M_PI;
				if (heading_r < -M_PI) heading_r += 2 * M_PI;
				yspeed += static_cast<int>(sin(heading_r) *
				                           static_cast<double>(approx_v));
				xspeed += static_cast<int>(cos(heading_r) *
				                           static_cast<double>(approx_v));

				if (hititem->getShapeInfo()->is_land()) {
					// Bouncing off land; this bounce approximates what's
					// seen in the original U8 when the key thrown by
					// Kilandra's daughters ghost lands on the grass.
					xspeed /= 4;
					yspeed /= 4;
					zspeed /= 2;
					if (zspeed == 0) terminate = true;
				} else {
					// Not on land; this bounce approximates what's seen
					// in the original U8 when Kilandra's daughters ghost
					// throws a key at the Avatar's head
					if (ABS(yspeed) > 2) yspeed /= 2;
					if (ABS(xspeed) > 2) xspeed /= 2;
				}
#ifdef BOUNCE_DIAG
				pout << "item " << item_num << " bounce ["
				     << Kernel::get_instance()->getFrameNum()
				     << "]: speed was (" << xspeedold << ","
				     << yspeedold << "," << zspeedold << ") new zspeed "
				     << zspeed << " heading " << headingold_r
				     << " impulse " << heading_r << " ("
				     << (xspeed - xspeedold) << "," << (yspeed - yspeedold)
				     << "), terminate: " << terminate << std::endl;
#endif
			} else {
#ifdef BOUNCE_DIAG
				pout << "item " << item_num << " bounce ["
				     << Kernel::get_instance()->getFrameNum()
				     << "]: no bounce" << std::endl;
#endif
			}
		} else {
#ifdef BOUNCE_DIAG
			pout << "item " << item_num << " bounce ["
			     << Kernel::get_instance()->getFrameNum()
			     << "]: slow hit" << std::endl;
#endif
		}
		if (terminate) {
			item->clearFlag(Item::FLG_BOUNCING);
			terminateDeferred();
		} else {
			item->setFlag(Item::FLG_BOUNCING);
		}
		fallStopped();

	} else {

		// blocked in some other direction than strictly downward

		// invert and decrease speed in all blocked directions

#ifdef BOUNCE_DIAG
		int xspeedold = xspeed;
		int yspeedold = yspeed;
		int zspeedold = zspeed;
#endif

		if (dirs & 1)
			xspeed = -xspeed / 2;
		if (dirs & 2)
			yspeed = -yspeed / 2;
		if (dirs & 4)
			zspeed = -zspeed / 2;

#ifdef BOUNCE_DIAG
		pout << "item " << item_num << " bounce ["
		     << Kernel::get_instance()->getFrameNum()
		     << "]: speed was (" << xspeedold << ","
		     << yspeedold << "," << zspeedold << ") new speed ("
		     << xspeed << "," << yspeed << "," << zspeed << ")" << std::endl;
#endif

		item->setFlag(Item::FLG_BOUNCING);

	}
}


void GravityProcess::terminate() {
	//signal item GravityProcess is gone
	Item *item = getItem(item_num);
	if (item) {
		item->setGravityPID(0);

		// no longer bouncing
		item->clearFlag(Item::FLG_BOUNCING);
	}

	Process::terminate();
}

void GravityProcess::fallStopped() {
	// actors take a hit if they fall
	// CHECKME: might need to do a 'die' animation even if actor is dead

	Actor *actor = getActor(item_num);
	if (actor && !actor->isDead()) {
		int height = actor->getFallStart() - actor->getZ();

		if (height >= 80) {
			int damage = 0;

			if (height < 104) {
				// medium fall: take some damage
				damage = (height - 72) / 4;
			} else {
				// high fall: die
				damage = actor->getHP();
			}

			actor->receiveHit(0, actor->getDir(), damage,
			                  WeaponInfo::DMG_FALLING | WeaponInfo::DMG_PIERCE);

			// 'ooof'
			AudioProcess *audioproc = AudioProcess::get_instance();
			if (audioproc) audioproc->playSFX(51, 250, item_num, 0); // CONSTANT!
		}

		if (!actor->isDead() && actor->getLastAnim() != Animation::die) {

			// play land animation, overriding other animations
			Kernel::get_instance()->killProcesses(item_num, 0xF0, false); // CONSTANT!
			ProcId lpid = actor->doAnim(Animation::land, 8);

			if (actor->isInCombat()) {
				// need to get back to a combat stance to prevent weapon from
				// being drawn again
				ProcId spid = actor->doAnim(Animation::combatStand, 8);
				Process *sp = Kernel::get_instance()->getProcess(spid);
				sp->waitFor(lpid);
			}
		}
	}
}

void GravityProcess::dumpInfo() {
	Process::dumpInfo();

	pout << "gravity: " << gravity << ", speed: (" << xspeed << ","
	     << yspeed << "," << zspeed << ")" << std::endl;
}


void GravityProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(gravity));
	ods->write4(static_cast<uint32>(xspeed));
	ods->write4(static_cast<uint32>(yspeed));
	ods->write4(static_cast<uint32>(zspeed));
}

bool GravityProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	gravity = static_cast<int>(ids->read4());
	xspeed = static_cast<int>(ids->read4());
	yspeed = static_cast<int>(ids->read4());
	zspeed = static_cast<int>(ids->read4());

	return true;
}

} // End of namespace Ultima8
