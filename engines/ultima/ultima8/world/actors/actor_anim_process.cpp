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

#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/graphics/anim_dat.h"
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/gravity_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/world/actors/combat_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/graphics/palette_fader_process.h"
#include "ultima/ultima8/world/create_item_process.h"
#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/get_object.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

//#define WATCHACTOR 1

#ifdef WATCHACTOR
static const int watchactor = WATCHACTOR;
#endif

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(ActorAnimProcess, Process)

ActorAnimProcess::ActorAnimProcess() : Process(), _tracker(nullptr) {

}

ActorAnimProcess::ActorAnimProcess(Actor *actor_, Animation::Sequence action_,
                                   uint32 dir_, uint32 steps_) :
		_dir(dir_), _action(action_), _steps(steps_), _tracker(nullptr),
		_firstFrame(true), _currentStep(0) {
	assert(actor_);
	_itemNum = actor_->getObjId();

	_type = 0x00F0; // CONSTANT !
}

bool ActorAnimProcess::init() {
	_repeatCounter = 0;
	_animAborted = false;
	_attackedSomething = false;

	Actor *actor = getActor(_itemNum);
	assert(actor);

	if (_dir == 8)
		_dir = actor->getDir();

	if (_dir > 7) {
		// invalid direction
		return false;
	}

	if (!(actor->getFlags() & Item::FLG_FASTAREA)) {
		// not in the fast area? Can't play an animation then.
		// (If we do, the actor will likely fall because the floor is gone.)
		return false;
	}

	if (actor->getActorFlags() & Actor::ACT_ANIMLOCK) {
		//! What do we do if actor was already animating?
		//! don't do this animation or kill the previous one?
		//! Or maybe wait until the previous one finishes?

		perr << "ActorAnimProcess [" << getPid() << "]: ANIMLOCK set on actor "
		     << _itemNum << Std::endl;

		// for now, just don't play this one.
		return false;
	}

	_tracker = new AnimationTracker();
	if (!_tracker->init(actor, _action, _dir)) {
		delete _tracker;
		_tracker = nullptr;
		return false;
	}

	actor->setActorFlag(Actor::ACT_ANIMLOCK);

	actor->_lastAnim = _action;
	actor->_direction = _dir;


#ifdef WATCHACTOR
	if (_itemNum == watchactor)
		pout << "Animation [" << Kernel::get_instance()->getFrameNum()
		     << "] ActorAnimProcess " << getPid() << " created ("
		     << _action << "," << _dir << ") _steps " << _steps << Std::endl;
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
	if (_repeatCounter > _tracker->getAnimAction()->_frameRepeat)
		_repeatCounter = 0;

	Actor *a = getActor(_itemNum);
	if (!a) {
		// actor gone
		terminate();
		return;
	}

	_firstFrame = false;

	if (!(a->getFlags() & Item::FLG_FASTAREA)) {
		// not in the fast area? Kill the animation then.
		//! TODO: Decide if this is the right move.
		//  Animation could do one of three things: pause, move
		//  without allowing actor to fall, or pretend to move and
		//  complete the entire movement as the actor reappears
		//  in fast area (still may need to pause when
		//  AnimationTracker is done.)
#ifdef WATCHACTOR
		if (_itemNum == watchactor)
			pout << "Animation ["
			     << Kernel::get_instance()->getFrameNum()
			     << "] ActorAnimProcess left fastarea; terminating"
			     << Std::endl;
#endif
		terminate();
		return;
	}

	bool resultVal = true;
	if (_repeatCounter == 0) {
		// next step:
		int32 x, y, z;
		a->getLocation(x, y, z);
		resultVal = _tracker->stepFrom(x, y, z);
		_tracker->updateActorFlags();
		_currentStep++;

		if (!resultVal) {
			// check possible error conditions

			if (_tracker->isDone() || (_steps && _currentStep >= _steps)) {
				// all done
#ifdef WATCHACTOR
				if (_itemNum == watchactor)
					pout << "Animation ["
					     << Kernel::get_instance()->getFrameNum()
					     << "] ActorAnimProcess done; terminating"
					     << Std::endl;
#endif

				// TODO: there are _three_ places where we can fall; clean up
				if (_tracker->isUnsupported()) {
#ifdef WATCHACTOR
					if (_itemNum == watchactor) {
						pout << "Animation ["
						     << Kernel::get_instance()->getFrameNum()
						     << "] falling" << Std::endl;
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
			        !(_tracker->getAnimAction()->_flags & AnimAction::AAF_UNSTOPPABLE)) {
				// FIXME: For blocked large _steps we may still want to do
				//        a partial move. (But how would that work with
				//        repeated frames?)

#ifdef WATCHACTOR
				if (_itemNum == watchactor)
					pout << "Animation ["
					     << Kernel::get_instance()->getFrameNum()
					     << "] ActorAnimProcess blocked; terminating"
					     << Std::endl;
#endif

				if (_tracker->isUnsupported()) {
#ifdef WATCHACTOR
					if (_itemNum == watchactor) {
						pout << "Animation ["
						     << Kernel::get_instance()->getFrameNum()
						     << "] falling" << Std::endl;
					}
#endif
					// no inertia here because we just crashed into something
					a->fall();
				}


				terminate();
				return;
			}
		}

		AnimFrame *curframe = _tracker->getAnimFrame();
		if (curframe && curframe->_sfx) {
			AudioProcess *audioproc = AudioProcess::get_instance();
			if (audioproc) audioproc->playSFX(curframe->_sfx, 0x60, _itemNum, 0);
		}

		if (curframe && (curframe->_flags & AnimFrame::AFF_SPECIAL)) {
			// Flag to trigger a special _action
			// E.g.: play draw/sheathe SFX for avatar when weapon equipped,
			// throw skull-fireball when ghost attacks, ...
			doSpecial();
		}


		// attacking?
		if (!_attackedSomething) {
			ObjId hit = _tracker->hitSomething();
			if (hit) {
				_attackedSomething = true;
				Item *hit_item = getItem(hit);
				assert(hit_item);
				hit_item->receiveHit(_itemNum, (_dir + 4) % 8, 0, 0);
				doHitSpecial(hit_item);
			}
		}
	}

	int32 x, y, z, x2, y2, z2;
	a->getLocation(x, y, z);
	_tracker->getInterpolatedPosition(x2, y2, z2, _repeatCounter);
	if (x == x2 && y == y2 && z == z2) {
		_tracker->getInterpolatedPosition(x, y, z, _repeatCounter + 1);
		a->collideMove(x, y, z, false, true); // forced move
		a->setFrame(_tracker->getFrame());
	} else {
#ifdef WATCHACTOR
		if (_itemNum == watchactor) {
			pout << "Animation [" << Kernel::get_instance()->getFrameNum()
			     << "] moved, so aborting this frame." << Std::endl;
		}
#endif
	}

	// Did we just leave the fast area?
	if (!(a->getFlags() & Item::FLG_FASTAREA)) {
#ifdef WATCHACTOR
		if (_itemNum == watchactor)
			pout << "Animation ["
			     << Kernel::get_instance()->getFrameNum()
			     << "] ActorAnimProcess left fastarea; terminating"
			     << Std::endl;
#endif
		terminate();
		return;
	}

#ifdef WATCHACTOR
	if (_itemNum == watchactor) {
		pout << "Animation [" << Kernel::get_instance()->getFrameNum()
		     << "] showing frame (" << x << "," << y << "," << z << ")"
		     << " shape (" << a->getShape() << "," << _tracker->getFrame()
		     << ") sfx " << _tracker->getAnimFrame()->sfx
		     << " rep " << _repeatCounter << " ";

		if (_tracker->isDone()) pout << "D";
		if (_tracker->isBlocked()) pout << "B";
		if (_tracker->isUnsupported()) pout << "U";
		if (_tracker->hitSomething()) pout << "H";
		pout << Std::endl;
	}
#endif


	if (_repeatCounter == _tracker->getAnimAction()->_frameRepeat) {
		if (_tracker->isUnsupported()) {
			_animAborted = true;

#ifdef WATCHACTOR
			if (_itemNum == watchactor) {
				pout << "Animation [" << Kernel::get_instance()->getFrameNum()
				     << "] falling" << Std::endl;
			}
#endif

			int32 dx, dy, dz;
			_tracker->getSpeed(dx, dy, dz);
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

	// play SFX when Avatar draws/sheathes weapon
	if (_itemNum == 1 && (_action == Animation::readyWeapon ||
	                      _action == Animation::unreadyWeapon) &&
	        a->getEquip(ShapeInfo::SE_WEAPON) != 0) {
		int sfx = (getRandom() % 2) ? 0x51 : 0x52; // constants!
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
			int32 x, y, z;
			a->getLocation(x, y, z);
			int dirNum = a->getDir();
			skull->move(x + 32 * x_fact[dirNum], y + 32 * y_fact[dirNum], z);
			hostile = skull;
		} else if (a->getMapNum() != 54) { // Khumash-Gor doesn't summon ghouls
			// otherwise, summon ghoul
			unsigned int ghoulcount = a->countNearby(0x8e, 8 * 256);
			if (ghoulcount > 2) return;

			int32 x, y, z;
			a->getLocation(x, y, z);
			x += (getRandom() % (6 * 256)) - 3 * 256;
			y += (getRandom() % (6 * 256)) - 3 * 256;

			Actor *ghoul = Actor::createActor(0x8e, 0);
			if (!ghoul) return;
			ghoul->setFlag(Item::FLG_FAST_ONLY);
			if (!ghoul->canExistAt(x, y, z, true)) {
				ghoul->destroy();
				return;
			}
			ghoul->move(x, y, z);
			ghoul->doAnim(Animation::standUp, 0);
			hostile = ghoul;
		}

		if (hostile) {
			hostile->setInCombat();
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
			a->explode(); // explode if close to the avatar
		}
		return;
	}

	// play PC/NPC footsteps
	SettingManager *settingman = SettingManager::get_instance();
	bool playavfootsteps;
	settingman->get("footsteps", playavfootsteps);
	if (_itemNum != 1 || playavfootsteps) {
		UCList itemlist(2);
		LOOPSCRIPT(script, LS_TOKEN_TRUE);
		CurrentMap *cm = World::get_instance()->getCurrentMap();

		// find items directly below
		cm->surfaceSearch(&itemlist, script, sizeof(script), a, false, true);
		if (itemlist.getSize() == 0) return;

		Item *f = getItem(itemlist.getuint16(0));
		assert(f);

		uint32 floor = f->getShape();
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
			if (audioproc) audioproc->playSFX(sfx, 0x60, _itemNum, 0, false, 0x10000 + (getRandom() & 0x1FFF) - 0x1000);
		}

		if (splash) {
			int32 x, y, z;
			a->getLocation(x, y, z);
			Process *sp = new SpriteProcess(475, 0, 7, 1, 1, x, y, z);
			Kernel::get_instance()->addProcess(sp);
		}
	}

}


void ActorAnimProcess::doHitSpecial(Item *hit) {
	Actor *a = getActor(_itemNum);
	assert(a);

	Actor *attacked = p_dynamic_cast<Actor *>(hit);

	if (_itemNum == 1 && _action == Animation::attack) {
		// some magic weapons have some special effects

		AudioProcess *audioproc = AudioProcess::get_instance();

		MainActor *av = getMainActor();
		ObjId weaponid = av->getEquip(ShapeInfo::SE_WEAPON);
		Item *weapon = getItem(weaponid);

		if (!weapon) return;

		uint32 weaponshape = weapon->getShape();

		switch (weaponshape) {
		case 0x32F: // magic hammer
			if (audioproc) audioproc->playSFX(23, 0x60, 1, 0, false,
				                                  0x10000 + (getRandom() & 0x1FFF) - 0x1000);
			break;
		case 0x330: { // Slayer
			// if we killed somebody, thunder&lightning
			if (attacked && (attacked->getActorFlags() & Actor::ACT_DEAD)) {
				// calling intrinsic...
				PaletteFaderProcess::I_lightningBolt(0, 0);
				int sfx;
				switch (getRandom() % 3) {
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
			if (getRandom() % 2 == 0) sfx = 101;
			if (audioproc) audioproc->playSFX(sfx, 0x60, 1, 0, false,
				                                  0x10000 + (getRandom() & 0x1FFF) - 0x1000);

			int32 x, y, z;
			a->getLocation(x, y, z);
			// 1: create flame sprite
			// 2: create flame object
			// 3: wait
			// 4a: destroy flame object
			// 4b: create douse-flame sprite
			Kernel *kernel = Kernel::get_instance();

			int32 fx, fy, fz;
			fx = x + 96 * x_fact[_dir];
			fy = y + 96 * y_fact[_dir];
			fz = z;

			// CONSTANTS!! (lots of them)

			SpriteProcess *sp1 = new SpriteProcess(480, 0, 9, 1, 2, fx, fy, fz);
			kernel->addProcess(sp1);

			DelayProcess *dp1 = new DelayProcess(3);
			ProcId dp1id = kernel->addProcess(dp1);

			CreateItemProcess *cip = new CreateItemProcess(400, 0, 0,
			        Item::FLG_FAST_ONLY,
			        0, 0, 0, fx, fy, fz);
			ProcId cipid = kernel->addProcess(cip);

			DelayProcess *dp2 = new DelayProcess(60 + (getRandom() % 60)); //2-4s
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
		pout << "Animation ["
		     << Kernel::get_instance()->getFrameNum()
		     << "] ActorAnimProcess terminating"
		     << Std::endl;
#endif
	Actor *a = getActor(_itemNum);
	if (a) {
		if (_tracker) { // if we were really animating...
			a->clearActorFlag(Actor::ACT_ANIMLOCK);
			if (_tracker->getAnimAction()->_flags & AnimAction::AAF_DESTROYACTOR) {
				// destroy the actor
#ifdef WATCHACTOR
				if (_itemNum == watchactor)
					pout << "Animation ["
					     << Kernel::get_instance()->getFrameNum()
					     << "] ActorAnimProcess destroying actor " << _itemNum
					     << Std::endl;
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

void ActorAnimProcess::dumpInfo() const {
	Process::dumpInfo();
	pout << "_action: " << _action << ", _dir: " << _dir << Std::endl;
}

void ActorAnimProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	uint8 ff = _firstFrame ? 1 : 0;
	ods->write1(ff);
	uint8 ab = _animAborted ? 1 : 0;
	ods->write1(ab);
	uint8 attacked = _attackedSomething ? 1 : 0;
	ods->write1(attacked);
	ods->write1(static_cast<uint8>(_dir));
	ods->write2(static_cast<uint16>(_action));
	ods->write2(static_cast<uint16>(_steps));
	ods->write2(static_cast<uint16>(_repeatCounter));
	ods->write2(static_cast<uint16>(_currentStep));

	if (_tracker) {
		ods->write1(1);
		_tracker->save(ods);
	} else
		ods->write1(0);
}

bool ActorAnimProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	_firstFrame = (ids->read1() != 0);
	_animAborted = (ids->read1() != 0);
	_attackedSomething = (ids->read1() != 0);
	_dir = ids->read1();
	_action = static_cast<Animation::Sequence>(ids->read2());
	_steps = ids->read2();
	_repeatCounter = ids->read2();
	_currentStep = ids->read2();

	assert(_tracker == nullptr);
	if (ids->read1() != 0) {
		_tracker = new AnimationTracker();
		if (!_tracker->load(ids, version))
			return false;
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
