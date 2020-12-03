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

#include "ultima/ultima8/world/gravity_process.h"

#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(GravityProcess)

GravityProcess::GravityProcess()
	: Process(), _xSpeed(0), _ySpeed(0), _zSpeed(0), _gravity(0) {

}

GravityProcess::GravityProcess(Item *item, int gravity)
	: _xSpeed(0), _ySpeed(0), _zSpeed(0), _gravity(gravity) {
	assert(item);

	_itemNum = item->getObjId();
	_type = 0x203; // CONSTANT!
}

void GravityProcess::init() {
	Item *item = getItem(_itemNum);
	assert(item);

	item->setGravityPID(getPid());

	Actor *actor = dynamic_cast<Actor *>(item);
	if (actor) {
		actor->setFallStart(actor->getZ());
	}
}

void GravityProcess::move(int xs, int ys, int zs) {
	_xSpeed += xs;
	_ySpeed += ys;
	_zSpeed += zs;
}

void GravityProcess::setGravity(int gravity) {
	// only apply _gravity if stronger than current _gravity
	//!!! is this correct?
	if (gravity > _gravity)
		_gravity = gravity;
}

void GravityProcess::run() {
	// move item in (xs,ys,zs) direction
	Item *item = getItem(_itemNum);
	if (!item) {
		terminate();
		return;
	}

	Actor *actor = dynamic_cast<Actor *>(item);
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
	tx = ix + _xSpeed;
	ty = iy + _ySpeed;
	tz = iz + _zSpeed;

	bool clipped = false;

	// Clip to region. This doesn't work
#if 0
	if (tx < 0 && ix >= 0) {
		int32 scale = (ix - tx) >> 0x8;
		tx = 0;
		ty = iy + ((_ySpeed * scale) >> 0x2000);
		tz = iz + ((_zSpeed * scale) >> 0x2000);
		clipped = true;
	}
	if (ty < 0 && iy >= 0) {
		int32 scale = (iy - ty) >> 0x8;
		tx = ix + ((_xSpeed * scale) >> 0x2000);
		ty = 0;
		tz = iz + ((_zSpeed * scale) >> 0x2000);
		clipped = true;
	}
	if (tz < 0 && iz >= 0) {
		int32 scale = (iz - tz) >> 0x8;
		tx = ix + ((_xSpeed * scale) >> 0x2000);
		ty = iy + ((_ySpeed * scale) >> 0x2000);
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
		_zSpeed -= _gravity;
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
	if (dirs == 4 && _zSpeed < 0) {

		// If it landed on top of hititem and hititem is not land, the item
		// should always bounce.

		bool termFlag = true;
		Item *hititem = getItem(hititemid);
		if (!hititem)
			return; // shouldn't happen..
		if (_zSpeed < -2 && !dynamic_cast<Actor *>(item)) {
#ifdef BOUNCE_DIAG
			pout << "item " << _itemNum << " bounce ["
			     << Kernel::get_instance()->getFrameNum()
			     << "]: hit " << hititem->getObjId() << Std::endl;
#endif

			if (!hititem->getShapeInfo()->is_land() || _zSpeed < -2 * _gravity) {
				// Bounce!
				termFlag = false;
#ifdef BOUNCE_DIAG
				int xspeedold = _xSpeed;
				int yspeedold = _ySpeed;
				int zspeedold = _zSpeed;
#endif
				_zSpeed = 0 - _zSpeed / 3;
				int approx_v = ABS(_xSpeed) + ABS(_ySpeed) + _zSpeed;

				// Apply an impulse on the x/y plane in a random direction
				// in a 180 degree pie around the orginal vector in x/y
				double heading_r = atan2((double)_ySpeed, (double)_xSpeed);
				double deltah_r = static_cast<double>(getRandom())
				                  * M_PI / RAND_MAX - M_PI / 2;
#ifdef BOUNCE_DIAG
				double headingold_r = heading_r;
#endif
				heading_r += deltah_r;
				if (heading_r > M_PI) heading_r -= 2 * M_PI;
				if (heading_r < -M_PI) heading_r += 2 * M_PI;
				_ySpeed += static_cast<int>(sin(heading_r) *
				                           static_cast<double>(approx_v));
				_xSpeed += static_cast<int>(cos(heading_r) *
				                           static_cast<double>(approx_v));

				if (hititem->getShapeInfo()->is_land()) {
					// Bouncing off land; this bounce approximates what's
					// seen in the original U8 when the key thrown by
					// Kilandra's daughters ghost lands on the grass.
					_xSpeed /= 4;
					_ySpeed /= 4;
					_zSpeed /= 2;
					if (_zSpeed == 0) termFlag = true;
				} else {
					// Not on land; this bounce approximates what's seen
					// in the original U8 when Kilandra's daughters ghost
					// throws a key at the Avatar's head
					if (ABS(_ySpeed) > 2) _ySpeed /= 2;
					if (ABS(_xSpeed) > 2) _xSpeed /= 2;
				}
#ifdef BOUNCE_DIAG
				pout << "item " << _itemNum << " bounce ["
				     << Kernel::get_instance()->getFrameNum()
				     << "]: speed was (" << xspeedold << ","
				     << yspeedold << "," << zspeedold << ") new _zSpeed "
				     << _zSpeed << " heading " << headingold_r
				     << " impulse " << heading_r << " ("
				     << (_xSpeed - xspeedold) << "," << (_ySpeed - yspeedold)
				     << "), termFlag: " << termFlag << Std::endl;
#endif
			} else {
#ifdef BOUNCE_DIAG
				pout << "item " << _itemNum << " bounce ["
				     << Kernel::get_instance()->getFrameNum()
				     << "]: no bounce" << Std::endl;
#endif
			}
		} else {
#ifdef BOUNCE_DIAG
			pout << "item " << _itemNum << " bounce ["
			     << Kernel::get_instance()->getFrameNum()
			     << "]: slow hit" << Std::endl;
#endif
		}
		if (termFlag) {
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
		int xspeedold = _xSpeed;
		int yspeedold = _ySpeed;
		int zspeedold = _zSpeed;
#endif

		if (dirs & 1)
			_xSpeed = -_xSpeed / 2;
		if (dirs & 2)
			_ySpeed = -_ySpeed / 2;
		if (dirs & 4)
			_zSpeed = -_zSpeed / 2;

#ifdef BOUNCE_DIAG
		pout << "item " << _itemNum << " bounce ["
		     << Kernel::get_instance()->getFrameNum()
		     << "]: speed was (" << xspeedold << ","
		     << yspeedold << "," << zspeedold << ") new speed ("
		     << _xSpeed << "," << _ySpeed << "," << _zSpeed << ")" << Std::endl;
#endif

		item->setFlag(Item::FLG_BOUNCING);

	}
}


void GravityProcess::terminate() {
	//signal item GravityProcess is gone
	Item *item = getItem(_itemNum);
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

	Actor *actor = getActor(_itemNum);
	if (actor && !actor->isDead()) {
		int height = actor->getFallStart() - actor->getZ();
		if (GAME_IS_U8)
			actorFallStoppedU8(actor, height);
		else
			actorFallStoppedCru(actor, height);
	}
}

void GravityProcess::actorFallStoppedU8(Actor *actor, int height) {
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
		if (audioproc) audioproc->playSFX(51, 250, _itemNum, 0); // CONSTANT!
	}

	if (!actor->isDead() && actor->getLastAnim() != Animation::die) {
		Kernel *kernel = Kernel::get_instance();

		// play land animation, overriding other animations
		kernel->killProcesses(_itemNum, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE, false); // CONSTANT!
		ProcId lpid = actor->doAnim(Animation::land, dir_current);

		if (actor->isInCombat()) {
			// need to get back to a combat stance to prevent weapon from
			// being drawn again
			ProcId spid = actor->doAnim(Animation::combatStand, dir_current);
			Process *sp = kernel->getProcess(spid);
			sp->waitFor(lpid);
		}
	}
}

void GravityProcess::actorFallStoppedCru(Actor *actor, int height) {
	Animation::Sequence lastanim = actor->getLastAnim();
	Kernel *kernel = Kernel::get_instance();

	if (height / 8 > 2 &&
		(lastanim != Animation::anotherJump &&
		 lastanim != Animation::slowCombatRollLeft &&
		 lastanim != Animation::slowCombatRollRight &&
		 lastanim != Animation::combatRollLeft &&
		 lastanim != Animation::combatRollRight &&
		 lastanim != Animation::run &&
		 lastanim != Animation::jumpForward &&
		 lastanim != Animation::unknownAnim30 &&
		 lastanim != Animation::runWithLargeWeapon)) {
		// play land animation, overriding other animations
		kernel->killProcesses(_itemNum, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE, false); // CONSTANT!
		ProcId lpid = actor->doAnim(Animation::jumpLanding, dir_current);

		Animation::Sequence nextanim = actor->isInCombat() ? Animation::combatStand : Animation::stand;
		ProcId spid = actor->doAnim(nextanim, dir_current);
		Process *sp = kernel->getProcess(spid);
		sp->waitFor(lpid);

		// 'ooof' (Note: same sound no is used in No Remorse and No Regret)
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x8f, 250, _itemNum, 0); // CONSTANT!
	} else {
		Process *currentanim = kernel->findProcess(_itemNum, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE);
		if (currentanim) {
			// TODO: Is this the right thing?
			currentanim->wakeUp(0);
		}
	}
}

void GravityProcess::dumpInfo() const {
	Process::dumpInfo();

	pout << "_gravity: " << _gravity << ", speed: (" << _xSpeed << ","
	     << _ySpeed << "," << _zSpeed << ")" << Std::endl;
}


void GravityProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_gravity));
	ws->writeUint32LE(static_cast<uint32>(_xSpeed));
	ws->writeUint32LE(static_cast<uint32>(_ySpeed));
	ws->writeUint32LE(static_cast<uint32>(_zSpeed));
}

bool GravityProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_gravity = static_cast<int>(rs->readUint32LE());
	_xSpeed = static_cast<int>(rs->readUint32LE());
	_ySpeed = static_cast<int>(rs->readUint32LE());
	_zSpeed = static_cast<int>(rs->readUint32LE());

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
