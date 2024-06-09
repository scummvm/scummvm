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

#include "common/config-manager.h"

#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/actors/combat_process.h"
#include "ultima/ultima8/world/actors/auto_firer_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/gfx/palette_fader_process.h"
#include "ultima/ultima8/world/create_item_process.h"
#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

//#define WATCHACTOR 1

#ifdef WATCHACTOR
static const int watchactor = WATCHACTOR;
#endif

DEFINE_RUNTIME_CLASSTYPE_CODE(ActorAnimProcess)

ActorAnimProcess::ActorAnimProcess() : Process(), _tracker(nullptr),
	_dir(dir_north), _action(Animation::walk), _steps(0), _firstFrame(true),
	_currentStep(0), _repeatCounter(0), _animAborted(false),
	_attackedSomething(false), _interpolate(false) {
}

ActorAnimProcess::ActorAnimProcess(Actor *actor, Animation::Sequence action,
								   Direction dir, uint32 steps) :
		_dir(dir), _action(action), _steps(steps), _tracker(nullptr),
		_firstFrame(true), _currentStep(0), _repeatCounter(0),
		_animAborted(false), _attackedSomething(false), _interpolate(false)  {
	assert(actor);
	_itemNum = actor->getObjId();

	_type = ACTOR_ANIM_PROC_TYPE;
#ifdef WATCHACTOR
	if (_itemNum == watchactor)
		debugC(kDebugActor, "Animation [%u] ActorAnimProcess created (%u, %d, %d) steps %u",
			  Kernel::get_instance()->getFrameNum(), _itemNum, _action, _dir, _steps);
#endif
}

bool ActorAnimProcess::init() {
	_repeatCounter = 0;
	_animAborted = false;
	_attackedSomething = false;

	_interpolate = Ultima8Engine::get_instance()->isInterpolationEnabled();

	Actor *actor = getActor(_itemNum);
	assert(actor);

	if (_dir == dir_current)
		_dir = actor->getDir();

	if (!actor->hasFlags(Item::FLG_FASTAREA)) {
		// not in the fast area? Can't play an animation then.
		// (If we do, the actor will likely fall because the floor is gone.)

#ifdef WATCHACTOR
	if (_itemNum == watchactor)
		debugC(kDebugActor, "Animation [%u] ActorAnimProcess %u init failed (actor %u not fast)",
			  Kernel::get_instance()->getFrameNum(), getPid(), _itemNum);
#endif

		return false;
	}

	if (actor->hasActorFlags(Actor::ACT_ANIMLOCK)) {
		//! What do we do if actor was already animating?
		//! don't do this animation or kill the previous one?
		//! Or maybe wait until the previous one finishes?

		warning("ActorAnimProcess [%u]: ANIMLOCK set on actor %u, skipping anim (%d, %d)",
			getPid(), _itemNum, _action, _dir);

		// for now, just don't play this one.
		return false;
	}

	_tracker = new AnimationTracker();
	if (!_tracker->init(actor, _action, _dir)) {
		delete _tracker;
		_tracker = nullptr;

#ifdef WATCHACTOR
	if (_itemNum == watchactor)
		debugC(kDebugActor, "Animation [%u] ActorAnimProcess %u init failed (tracker not fast)",
			  Kernel::get_instance()->getFrameNum(), getPid());
#endif

		return false;
	}

	actor->setActorFlag(Actor::ACT_ANIMLOCK);

	actor->_lastAnim = _action;
	actor->_direction = _dir;


#ifdef WATCHACTOR
	if (_itemNum == watchactor)
		debugC(kDebugActor, "Animation [%u] ActorAnimProcess %u initalized (%u, %d, %d) steps %d",
			  Kernel::get_instance()->getFrameNum(), getPid(), _itemNum, _action, _dir, _steps);
#endif

	return true;
}


void ActorAnimProcess::run() {
	if (_firstFrame) {
		bool ret = init();
		if (!ret) {
			// initialization failed
			terminateDeferred();
			return;
		}
	}

	if (_animAborted) {
		terminate();
		return;
	}

	assert(_tracker);

	if (!_firstFrame)
		_repeatCounter++;
	if (_repeatCounter > _tracker->getAnimAction()->getFrameRepeat())
		_repeatCounter = 0;

	Actor *a = getActor(_itemNum);
	if (!a) {
		// actor gone
		terminate();
		return;
	}

	_firstFrame = false;

	if (!a->hasFlags(Item::FLG_FASTAREA)) {
		// not in the fast area? Kill the animation then.
		//! TODO: Decide if this is the right move.
		//  Animation could do one of three things: pause, move
		//  without allowing actor to fall, or pretend to move and
		//  complete the entire movement as the actor reappears
		//  in fast area (still may need to pause when
		//  AnimationTracker is done.)
#ifdef WATCHACTOR
		if (_itemNum == watchactor)
			debugC(kDebugActor, "Animation [%u] ActorAnimProcess left fastarea; terminating",
				  Kernel::get_instance()->getFrameNum());
#endif
		terminate();
		return;
	}

	bool resultVal = true;
	if (_repeatCounter == 0) {
		// next step:
		Point3 pt = a->getLocation();
		resultVal = _tracker->stepFrom(pt);
		_tracker->updateActorFlags();
		_currentStep++;

		if (!resultVal) {
			// check possible error conditions

			if (_tracker->isDone() || (_steps && _currentStep >= _steps)) {
				// all done
#ifdef WATCHACTOR
				if (_itemNum == watchactor)
					debugC(kDebugActor, "Animation [%u] ActorAnimProcess done; terminating",
						  Kernel::get_instance()->getFrameNum());
#endif

				// TODO: there are _three_ places where we can fall; clean up
				if (_tracker->isUnsupported() && pt.z > 0) {
#ifdef WATCHACTOR
					if (_itemNum == watchactor) {
						debugC(kDebugActor, "Animation [%u] ActorAnimProcess falling at end",
							  Kernel::get_instance()->getFrameNum());
					}
#endif
					int32 dx, dy, dz;
					_tracker->getSpeed(dx, dy, dz);
					a->hurl(dx, dy, dz, 2);
				}

				terminate();
				return;
			}

			if (_tracker->isBlocked() &&
				!_tracker->getAnimAction()->hasFlags(AnimAction::AAF_UNSTOPPABLE)) {
				// FIXME: For blocked large steps we may still want to do
				//        a partial move. (But how would that work with
				//        repeated frames?)

#ifdef WATCHACTOR
				if (_itemNum == watchactor)
					debugC(kDebugActor, "Animation [%u] ActorAnimProcess blocked; terminating",
						  Kernel::get_instance()->getFrameNum());
#endif

				if (_tracker->isUnsupported() && pt.z > 0) {
#ifdef WATCHACTOR
					if (_itemNum == watchactor) {
						debugC(kDebugActor, "Animation [%u] ActorAnimProcess falling from blocked",
							  Kernel::get_instance()->getFrameNum());
					}
#endif
					// no inertia here because we just crashed into something
					a->fall();
				}


				terminate();
				return;
			}
		}

		const AnimFrame *curframe = _tracker->getAnimFrame();
		if (curframe) {
			if (curframe->_sfx) {
				AudioProcess *audioproc = AudioProcess::get_instance();
				if (audioproc) audioproc->playSFX(curframe->_sfx, 0x60, _itemNum, 0);
			}

			if (curframe->_flags & AnimFrame::AFF_SPECIAL) {
				// Flag to trigger a special action
				// E.g.: play draw/sheathe SFX for avatar when weapon equipped,
				// throw skull-fireball when ghost attacks, ...
				doSpecial();
			} else if (curframe->_flags & AnimFrame::AFF_HURTY && GAME_IS_CRUSADER) {
				a->tookHitCru();
			} else if (curframe->is_cruattack() && GAME_IS_CRUSADER) {
				doFireWeaponCru(a, curframe);
			}
		}


		// attacking?
		if (!_attackedSomething) {
			ObjId hit = _tracker->hitSomething();
			if (hit) {
				_attackedSomething = true;
				Item *hit_item = getItem(hit);
				assert(hit_item);
				hit_item->receiveHit(_itemNum, Direction_Invert(_dir), 0, 0);
				doHitSpecial(hit_item);
			}
		}
	}

	Point3 pt = a->getLocation();
	Point3 pt2;

	if (_interpolate) {
		// Apply interpolated position on repeated frames
		pt2 = _tracker->getInterpolatedPosition(_repeatCounter);
		if (pt == pt2) {
			pt = _tracker->getInterpolatedPosition(_repeatCounter + 1);
			a->collideMove(pt.x, pt.y, pt.z, false, true); // forced move
			a->setFrame(_tracker->getFrame());
#ifdef WATCHACTOR
		} else {
			if (_itemNum == watchactor) {
				debugC(kDebugActor, "Animation [%u] ActorAnimProcess moved, so aborting this frame.",
						Kernel::get_instance()->getFrameNum());
			}
#endif // WATCHACTOR
		}
	} else {
		// Just move the whole distance on frame 0 of the repeat.
		if (_repeatCounter == 0) {
			pt2 = _tracker->getPosition();
			a->collideMove(pt2.x, pt2.y, pt2.z, false, true); // forced move
			a->setFrame(_tracker->getFrame());
		} else {
			pt2 = pt;
		}
	}

	// Did we just leave the fast area?
	if (!a->hasFlags(Item::FLG_FASTAREA)) {
#ifdef WATCHACTOR
		if (_itemNum == watchactor)
			debugC(kDebugActor, "Animation [%u] ActorAnimProcess left fastarea; terminating",
				  Kernel::get_instance()->getFrameNum());
#endif
		terminate();
		return;
	}

#ifdef WATCHACTOR
	if (_itemNum == watchactor) {
		Common::String info;
		if (_tracker->isDone())
			info += "D";
		if (_tracker->isBlocked())
			info += "B";
		if (_tracker->isUnsupported())
			info += "U";
		if (_tracker->hitSomething())
			info += "H";

		debugC(kDebugActor, "Animation [%u] ActorAnimProcess showing frame (%d, %d, %d)-(%d, %d, %d) shp (%u, %u) sfx %d rep %d flg %04X %s",
			  Kernel::get_instance()->getFrameNum(), pt.x, pt.y, pt.z, pt2.x, pt2.y, pt2.z,
			  a->getShape(), _tracker->getFrame(), _tracker->getAnimFrame()->_sfx,
			  _repeatCounter, _tracker->getAnimFrame()->_flags, info.c_str());
	}
#endif


	if (_repeatCounter == _tracker->getAnimAction()->getFrameRepeat()) {
		if (_tracker->isUnsupported() && pt.z > 0) {
			_animAborted = !_tracker->getAnimAction()->hasFlags(AnimAction::AAF_UNSTOPPABLE);

#ifdef WATCHACTOR
			if (_itemNum == watchactor) {
				debugC(kDebugActor, "Animation [%u] ActorAnimProcess falling from repeat",
					  Kernel::get_instance()->getFrameNum());
			}
#endif

			int32 dx, dy, dz;
			_tracker->getSpeed(dx, dy, dz);
			if (GAME_IS_CRUSADER) {
				// HACK: Hurl people a bit less hard in crusader until
				// the movement bugs are fixed to make them fall less..
				dx /= 4;
				dy /= 4;
				dz /= 4;
			}
			a->hurl(dx, dy, dz, 2);

			// Note: do not wait for the fall to finish: this breaks
			// the scene where Devon kills Mordea
			return;
		}
	}
}

void ActorAnimProcess::doSpecial() {
	Actor *a = getActor(_itemNum);
	assert(a);

	// All this stuff is U8 specific.
	if (!GAME_IS_U8)
		return;

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	// play SFX when Avatar draws/sheathes weapon
	if (_itemNum == kMainActorId &&
			(_action == Animation::readyWeapon || _action == Animation::unreadyWeapon) &&
			a->getEquip(ShapeInfo::SE_WEAPON) != 0) {
		int sfx = rs.getRandomBit() ? 0x51 : 0x52; // constants!
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(sfx, 0x60, 1, 0);
		return;
	}

	// ghosts
	if (a->getShape() == 0x19b) {
		Actor *hostile = nullptr;
		if (_action == Animation::attack) {
			// fireball on attack
			unsigned int skullcount = a->countNearby(0x19d, 6 * 256);
			if (skullcount > 5) return;

			Actor *skull = Actor::createActor(0x19d, 0);
			if (!skull) return;
			skull->setFlag(Item::FLG_FAST_ONLY);
			Point3 pt = a->getLocation();
			Direction dirNum = a->getDir();
			skull->move(pt.x + 32 * Direction_XFactor(dirNum), pt.y + 32 * Direction_XFactor(dirNum), pt.z);
			hostile = skull;
		} else if (a->getMapNum() != 54) { // Khumash-Gor doesn't summon ghouls
			// otherwise, summon ghoul
			unsigned int ghoulcount = a->countNearby(0x8e, 8 * 256);
			if (ghoulcount > 2) return;

			Point3 pt = a->getLocation();
			pt.x += rs.getRandomNumberRngSigned(-3 * 256, 3 * 256);
			pt.y += rs.getRandomNumberRngSigned(-3 * 256, 3 * 256);

			Actor *ghoul = Actor::createActor(0x8e, 0);
			if (!ghoul) return;
			ghoul->setFlag(Item::FLG_FAST_ONLY);
			if (!ghoul->canExistAt(pt, true)) {
				ghoul->destroy();
				return;
			}
			ghoul->move(pt);
			ghoul->doAnim(Animation::standUp, dir_north);
			hostile = ghoul;
		}

		if (hostile) {
			// Note: only happens in U8, so activity num is not important.
			hostile->setInCombat(0);
			CombatProcess *hostilecp = hostile->getCombatProcess();
			CombatProcess *cp = a->getCombatProcess();
			if (hostilecp && cp)
				hostilecp->setTarget(cp->getTarget());
		}

		return;
	}

	// ghost's fireball
	if (a->getShape() == 0x19d) {
		Actor *av = getMainActor();
		if (a->getRange(*av) < 96) {
			a->setActorFlag(Actor::ACT_DEAD);
			a->explode(0, true); // explode if close to the avatar
		}
		return;
	}

	// play PC/NPC footsteps
	bool playavfootsteps = ConfMan.getBool("footsteps");
	if (_itemNum != kMainActorId || playavfootsteps) {
		int32 xd, yd, zd;
		Point3 pt = a->getLocation();
		a->getFootpadWorld(xd, yd, zd);
		Box start(pt.x, pt.y, pt.z, xd, yd, zd);

		pt = _tracker->getPosition();
		Box target(pt.x, pt.y, pt.z, xd, yd, zd);

		CurrentMap *cm = World::get_instance()->getCurrentMap();
		PositionInfo info = cm->getPositionInfo(target, start, a->getShapeInfo()->_flags, _itemNum);
		if (info.supported && info.land) {
			uint32 floor = info.land->getShape();
			bool running = (_action == Animation::run);
			bool splash = false;
			int sfx = 0;
			switch (floor) { // lots of constants!!
			case 0x03:
			case 0x04:
			case 0x09:
			case 0x0B:
			case 0x5C:
			case 0x5E:
				sfx = 0x2B;
				break;
			case 0x7E:
			case 0x80:
				sfx = 0xCD;
				splash = true;
				break;
			case 0xA1:
			case 0xA2:
			case 0xA3:
			case 0xA4:
				sfx = (running ? 0x99 : 0x91);
				break;
			default:
				sfx = (running ? 0x97 : 0x90);
				break;
			}

			if (sfx) {
				AudioProcess *audioproc = AudioProcess::get_instance();
				if (audioproc)
					audioproc->playSFX(sfx, 0x60, _itemNum, 0, false, 0x10000 + rs.getRandomNumber(0x1FFF) - 0x1000);
			}

			if (splash) {
				pt = a->getLocation();
				Process *sp = new SpriteProcess(475, 0, 7, 1, 1, pt.x, pt.y, pt.z);
				Kernel::get_instance()->addProcess(sp);
			}
		}
	}

}

void ActorAnimProcess::doFireWeaponCru(Actor *a, const AnimFrame *f) {
	assert(a);
	assert(f);
	if (!f->is_cruattack())
		return;

	const Item *wpn = getItem(a->getActiveWeapon());
	if (!wpn)
		return;
	const ShapeInfo *wpninfo = wpn->getShapeInfo();
	if (!wpninfo || !wpninfo->_weaponInfo)
		return;

	if (a->getObjId() == kMainActorId && wpninfo->_weaponInfo->_damageType == 6) {
		Process *auto_firer = new AutoFirerProcess();
		Kernel::get_instance()->addProcess(auto_firer);
	}

	a->fireWeapon(f->cru_attackx(), f->cru_attacky(), f->cru_attackz(),
				  a->getDir(), wpninfo->_weaponInfo->_damageType, true);

	AudioProcess *audioproc = AudioProcess::get_instance();
	if (audioproc)
		audioproc->playSFX(wpninfo->_weaponInfo->_sound, 0x80, a->getObjId(), 0, false);
}



void ActorAnimProcess::doHitSpecial(Item *hit) {
	Actor *a = getActor(_itemNum);
	assert(a);

	Actor *attacked = dynamic_cast<Actor *>(hit);

	if (_itemNum == 1 && _action == Animation::attack) {
		// some magic weapons have some special effects

		AudioProcess *audioproc = AudioProcess::get_instance();

		MainActor *av = getMainActor();
		ObjId weaponid = av->getEquip(ShapeInfo::SE_WEAPON);
		Item *weapon = getItem(weaponid);

		if (!weapon) return;

		Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
		uint32 weaponshape = weapon->getShape();

		switch (weaponshape) {
		case 0x32F: // magic hammer
			if (audioproc) audioproc->playSFX(23, 0x60, 1, 0, false,
				                                  0x10000 + rs.getRandomNumber(0x1FFF) - 0x1000);
			break;
		case 0x330: { // Slayer
			// if we killed somebody, thunder&lightning
			if (attacked && attacked->hasActorFlags(Actor::ACT_DEAD)) {
				// calling intrinsic...
				PaletteFaderProcess::I_lightningBolt(0, 0);
				int sfx;
				switch (rs.getRandomNumber(2)) {
				case 0:
					sfx = 91;
					break;
				case 1:
					sfx = 94;
					break;
				default:
					sfx = 96;
					break;
				}
				if (audioproc) audioproc->playSFX(sfx, 0x60, 1, 0);
			}
			break;
		}
		case 0x331: { // Flame Sting
			int sfx = 33;
			if (rs.getRandomBit())
				sfx = 101;
			if (audioproc) audioproc->playSFX(sfx, 0x60, 1, 0, false,
				                                  0x10000 + rs.getRandomNumber(0x1FFF) - 0x1000);

			Point3 pt = a->getLocation();
			// 1: create flame sprite
			// 2: create flame object
			// 3: wait
			// 4a: destroy flame object
			// 4b: create douse-flame sprite
			Kernel *kernel = Kernel::get_instance();

			int32 fx, fy, fz;
			fx = pt.x + 96 * Direction_XFactor(_dir);
			fy = pt.y + 96 * Direction_YFactor(_dir);
			fz = pt.z;

			// CONSTANTS!! (lots of them)

			SpriteProcess *sp1 = new SpriteProcess(480, 0, 9, 1, 2, fx, fy, fz);
			kernel->addProcess(sp1);

			DelayProcess *dp1 = new DelayProcess(3);
			ProcId dp1id = kernel->addProcess(dp1);

			CreateItemProcess *cip = new CreateItemProcess(400, 0, 0,
			        Item::FLG_FAST_ONLY,
			        0, 0, 0, fx, fy, fz);
			ProcId cipid = kernel->addProcess(cip);

			DelayProcess *dp2 = new DelayProcess(rs.getRandomNumberRng(60, 120)); //2-4s
			ProcId dp2id = kernel->addProcess(dp2);

			DestroyItemProcess *dip = new DestroyItemProcess(0);
			kernel->addProcess(dip);

			SpriteProcess *sp2 = new SpriteProcess(381, 0, 9, 1, 1,
			                                       fx, fy, fz, true);
			kernel->addProcess(sp2);

			cip->waitFor(dp1id);
			dp2->waitFor(cipid);
			dip->waitFor(dp2id);
			sp2->waitFor(dp2id);

			break;
		}
		default:
			break;
		}

		return ;
	}

}

void ActorAnimProcess::terminate() {
#ifdef WATCHACTOR
	if (_itemNum == watchactor)
		debugC(kDebugActor, "Animation [%u] ActorAnimProcess %u terminating",
			  Kernel::get_instance()->getFrameNum(), getPid());
#endif

	Actor *a = getActor(_itemNum);
	if (a) {
		if (_tracker) { // if we were really animating...
			a->clearActorFlag(Actor::ACT_ANIMLOCK);
			if (_tracker->getAnimAction()->hasFlags(AnimAction::AAF_DESTROYACTOR)) {
				// destroy the actor
#ifdef WATCHACTOR
				if (_itemNum == watchactor)
					debugC(kDebugActor, "Animation [%u] ActorAnimProcess destroying actor %u",
						  Kernel::get_instance()->getFrameNum(), _itemNum);
#endif
				Process *vanishproc = new DestroyItemProcess(a);
				Kernel::get_instance()->addProcess(vanishproc);

				return;
			}
		}
	}

	delete _tracker;

	Process::terminate();
}

Common::String ActorAnimProcess::dumpInfo() const {
	return Process::dumpInfo() +
		Common::String::format(", _action: %d, _dir: %d", _action, _dir);
}

void ActorAnimProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	uint8 ff = _firstFrame ? 1 : 0;
	ws->writeByte(ff);
	uint8 ab = _animAborted ? 1 : 0;
	ws->writeByte(ab);
	uint8 attacked = _attackedSomething ? 1 : 0;
	ws->writeByte(attacked);
	ws->writeByte(static_cast<uint8>(Direction_ToUsecodeDir(_dir)));
	ws->writeUint16LE(static_cast<uint16>(_action));
	ws->writeUint16LE(static_cast<uint16>(_steps));
	ws->writeUint16LE(static_cast<uint16>(_repeatCounter));
	ws->writeUint16LE(static_cast<uint16>(_currentStep));

	if (_tracker) {
		ws->writeByte(1);
		_tracker->save(ws);
	} else
		ws->writeByte(0);
}

bool ActorAnimProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_firstFrame = (rs->readByte() != 0);
	_animAborted = (rs->readByte() != 0);
	_attackedSomething = (rs->readByte() != 0);
	_dir = Direction_FromUsecodeDir(rs->readByte());
	_action = static_cast<Animation::Sequence>(rs->readUint16LE());
	_steps = rs->readUint16LE();
	_repeatCounter = rs->readUint16LE();
	_currentStep = rs->readUint16LE();

	assert(_tracker == nullptr);
	if (rs->readByte() != 0) {
		_tracker = new AnimationTracker();
		if (!_tracker->load(rs, version))
			return false;
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
