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

#include "ultima/ultima8/world/gravity_process.h"

#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"

namespace Ultima {
namespace Ultima8 {

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

	// There should never be more than one gravity on an object
	assert(item->getGravityPID() == _pid);

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

	Point3 pt = item->getLocation();

	// Shouldn't go negative as original did not allow it
	if (pt.z <= 0 && _zSpeed < 0) {
		terminateDeferred();
		fallStopped();
		return;
	}

	int32 tx, ty, tz;
	tx = pt.x + _xSpeed;
	ty = pt.y + _ySpeed;
	tz = pt.z + _zSpeed;

	if (tz < 0)
		tz = 0;

//#define BOUNCE_DIAG

	ObjId hititemid;
	uint8 dirs;
	int32 dist = item->collideMove(tx, ty, tz, false, false, &hititemid, &dirs);

	if (dist == 0x4000 && !hititemid) {
		// didn't hit anything.
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

		CurrentMap *cm = World::get_instance()->getCurrentMap();
		Box target = item->getWorldBox();
		Box empty;
		PositionInfo info = cm->getPositionInfo(target, empty, item->getShapeInfo()->_flags, _itemNum);
		if (!info.valid || !info.supported) {
			// Reset speed and continue to slip off
			termFlag = false;
			_zSpeed = 0;

			pt = item->getCentre();
			target = hititem->getWorldBox();
			if (ABS(_xSpeed) < 16) {
				if (pt.x + 16 > target._x)
					_xSpeed = 16;
				else if (pt.x - 16 < target._x - target._xd)
					_xSpeed = -16;
			}

			if (ABS(_ySpeed) < 16) {
				if (pt.y + 16 > target._y)
					_ySpeed = 16;
				else if (pt.y - 16 < target._y - target._yd)
					_ySpeed = -16;
			}
		} else if (_zSpeed < -2 && !actor) {
#ifdef BOUNCE_DIAG
			debugC(kDebugObject, "item %u bounce [%u]: hit %u",
				_itemNum, Kernel::get_instance()->getFrameNum(), hititem->getObjId());
#endif

			if (GAME_IS_U8 && (!hititem->getShapeInfo()->is_land() || _zSpeed < -2 * _gravity)) {
				// Bounce!
				termFlag = false;
#ifdef BOUNCE_DIAG
				int xspeedold = _xSpeed;
				int yspeedold = _ySpeed;
				int zspeedold = _zSpeed;
#endif
				_zSpeed = 0 - _zSpeed / 3;
				int approx_v = ABS(_xSpeed) + ABS(_ySpeed) + _zSpeed;

				Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();

				// Apply an impulse on the x/y plane in a random direction
				// in a 180 degree pie around the original vector in x/y
				double heading_r = atan2((double)_ySpeed, (double)_xSpeed);
				double deltah_r = static_cast<double>(rs.getRandomNumber(UINT_MAX)) * M_PI / UINT_MAX - M_PI / 2.0;
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
					if (_zSpeed == 0)
						termFlag = true;
				} else {
					// Not on land; this bounce approximates what's seen
					// in the original U8 when Kilandra's daughters ghost
					// throws a key at the Avatar's head
					if (ABS(_ySpeed) > 2) _ySpeed /= 2;
					if (ABS(_xSpeed) > 2) _xSpeed /= 2;
				}
#ifdef BOUNCE_DIAG
				debugc(kDebugObject, "item %u bounce [%u]: speed was (%d, %d, %d) new _zSpeed %d heading %lf impulse %lf (%d, %d), termFlag: %d",
					  _itemNum, Kernel::get_instance()->getFrameNum(),
					  xspeedold, yspeedold, zspeedold
					  _zSpeed, headingold_r, heading_r,
					  (_xSpeed - xspeedold), (_ySpeed - yspeedold), termFlag);
#endif
			} else {
#ifdef BOUNCE_DIAG
				debugC(kDebugObject, "item %u bounce [%u]: no bounce",
					  _itemNum, Kernel::get_instance()->getFrameNum());
#endif
			}
		} else {
#ifdef BOUNCE_DIAG
			debugC(kDebugObject, "item %u bounce [%u]: slow hit",
				  _itemNum, Kernel::get_instance()->getFrameNum());
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
		debugC(kDebugObject, "item %u bounce [%u]: speed was (%d, %d, %d) new speed (%d, %d, %d)",
			  _itemNum, Kernel::get_instance()->getFrameNum(),
			  xspeedold, yspeedold, zspeedold ,
			  _xSpeed, _ySpeed, _zSpeed);
#endif

		item->setFlag(Item::FLG_BOUNCING);

	}
}


void GravityProcess::terminate() {
	//signal item GravityProcess is gone
	Item *item = getItem(_itemNum);
	if (item) {
		// This is strange, but not impossible (one terminates
		// and another starts before terminate() gets called).
		// Don't reset the item's gravityPID in this case.
		if (item->getGravityPID() != 0 && item->getGravityPID() != _pid)
			warning("GravityProcess::terminate %d on item %d which now has gravityPID %d",
					_pid, _itemNum, item->getGravityPID());
		else
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

	if (!actor->isDead() && actor->getLastAnim() != Animation::die && !actor->hasActorFlags(Actor::ACT_STUNNED)) {
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

	if (!actor->isDead() &&
		height / 8 > 2 &&
		(lastanim != Animation::quickJumpCru &&
		 lastanim != Animation::combatRollLeft &&
		 lastanim != Animation::combatRollRight &&
		 lastanim != Animation::kneelCombatRollLeft &&
		 lastanim != Animation::kneelCombatRollRight &&
		 lastanim != Animation::run &&
		 lastanim != Animation::jumpForward &&
		 lastanim != Animation::combatRunSmallWeapon &&
		 lastanim != Animation::combatRunLargeWeapon)) {
		// play land animation, overriding other animations
		kernel->killProcesses(_itemNum, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE, false); // CONSTANT!
		ProcId lpid = actor->doAnim(Animation::jumpLanding, dir_current);

		Animation::Sequence nextanim = actor->isInCombat() ? Animation::combatStand : Animation::stand;
		actor->doAnimAfter(nextanim, dir_current, lpid);

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

Common::String GravityProcess::dumpInfo() const {
	return Process::dumpInfo() +
		Common::String::format(", _gravity: %d, speed: (%d, %d, %d)", _gravity, _xSpeed, _ySpeed, _zSpeed);
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
