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
#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/actors/targeted_anim_process.h"
#include "ultima/ultima8/world/actors/avatar_gravity_process.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarMoverProcess, Process)

AvatarMoverProcess::AvatarMoverProcess() : Process() {
	_lastFrame = 0;
	_lastAttack = 0;
	_idleTime = 0;
	_lastHeadShakeAnim = Animation::lookLeft;
	_mouseButton[0]._state = MBS_HANDLED | MBS_RELHANDLED;
	_mouseButton[1]._state = MBS_HANDLED | MBS_RELHANDLED;
	_mouseButton[0]._curDown = 0;
	_mouseButton[1]._curDown = 0;
	_mouseButton[0]._lastDown = 0;
	_mouseButton[1]._lastDown = 0;
	_type = 1; // CONSTANT! (type 1 = persistent)
}


AvatarMoverProcess::~AvatarMoverProcess() {
}

void AvatarMoverProcess::run() {
	Kernel *kernel = Kernel::get_instance();
	uint32 framenum = kernel->getFrameNum();

	// only run once per frame
	if (framenum == _lastFrame)
		return;
	_lastFrame = framenum;


	MainActor *avatar = getMainActor();

	// busy, so don't move
	if (kernel->getNumProcesses(1, 0x00F0) > 0) {
		_idleTime = 0;
		return;
	}


	if (avatar->getLastAnim() == Animation::hang) {
		handleHangingMode();
		return;
	}

	// falling, so don't move
	if (avatar->getGravityPID() != 0) {
		_idleTime = 0;
		return;
	}

	bool combatRun = (avatar->getActorFlags() & Actor::ACT_COMBATRUN) != 0;
	if (avatar->isInCombat() && !combatRun)
		handleCombatMode();
	else
		handleNormalMode();
}

void AvatarMoverProcess::handleHangingMode() {
	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	MainActor *avatar = getMainActor();
	uint32 now = g_system->getMillis();
	bool stasis = guiapp->isAvatarInStasis();

	_idleTime = 0;

	if (stasis) return;

	bool m0clicked = false;
	//bool m1clicked = false;
	if (!(_mouseButton[0]._state & MBS_HANDLED) &&
	        now - _mouseButton[0]._curDown > DOUBLE_CLICK_TIMEOUT) {
		m0clicked = true;
		_mouseButton[0]._state |= MBS_HANDLED;
	}
	if (!(_mouseButton[1]._state & MBS_HANDLED) &&
	        now - _mouseButton[1]._curDown > DOUBLE_CLICK_TIMEOUT) {
		//m1clicked = true;
		_mouseButton[1]._state |= MBS_HANDLED;
	}
	if (!(_mouseButton[0]._state & MBS_RELHANDLED))
		_mouseButton[0]._state |= MBS_RELHANDLED;
	if (!(_mouseButton[1]._state & MBS_RELHANDLED))
		_mouseButton[1]._state |= MBS_RELHANDLED;


	// if left mouse is down, try to climb up

	if ((_mouseButton[0]._state & MBS_DOWN) &&
	        (!(_mouseButton[0]._state & MBS_HANDLED) || m0clicked)) {
		_mouseButton[0]._state |= MBS_HANDLED;
		_mouseButton[0]._lastDown = 0;

		if (avatar->tryAnim(Animation::climb40, 8) == Animation::SUCCESS) {
			avatar->ensureGravityProcess()->terminate();
			waitFor(avatar->doAnim(Animation::climb40, 8));
		}
	}
}

void AvatarMoverProcess::handleCombatMode() {
	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	Mouse *mouse = Mouse::get_instance();
	MainActor *avatar = getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	Animation::Sequence nextanim = Animation::walk;
	int32 direction = avatar->getDir();
	uint32 now = g_system->getMillis();
	bool stasis = guiapp->isAvatarInStasis();

	int32 mx, my;
	mouse->getMouseCoords(mx, my);
	unsigned int mouselength = mouse->getMouseLength(mx, my);

	// adjust to world direction
	int32 mousedir = (mouse->getMouseDirection(mx, my) + 7) % 8;

	// never idle when in combat
	_idleTime = 0;

	// If Avatar has fallen down, stand up.
	if (lastanim == Animation::die || lastanim == Animation::fallBackwards) {
		if (!stasis)
			waitFor(avatar->doAnim(Animation::standUp, mousedir));
		return;
	}

	// if we were blocking, and no longer holding the mouse, stop
	if (lastanim == Animation::startBlock &&
	        !(_mouseButton[0]._state & MBS_DOWN)) {
//		pout << "AvatarMover: combat stop blocking" << Std::endl;
		waitFor(avatar->doAnim(Animation::stopBlock, direction));
		return;
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	bool m0clicked = false;
	bool m1clicked = false;

	if (!(_mouseButton[0]._state & MBS_HANDLED) &&
	        now - _mouseButton[0]._curDown > DOUBLE_CLICK_TIMEOUT) {
		m0clicked = true;
		_mouseButton[0]._state |= MBS_HANDLED;
	}

	if (!(_mouseButton[1]._state & MBS_HANDLED) &&
	        now - _mouseButton[1]._curDown > DOUBLE_CLICK_TIMEOUT) {
		m1clicked = true;
		_mouseButton[1]._state |= MBS_HANDLED;
	}

	if (!(_mouseButton[0]._state & MBS_RELHANDLED))
		_mouseButton[0]._state |= MBS_RELHANDLED;

	if (!(_mouseButton[1]._state & MBS_RELHANDLED))
		_mouseButton[1]._state |= MBS_RELHANDLED;

	if ((_mouseButton[0]._state & MBS_DOWN) &&
	        (_mouseButton[0]._state & MBS_HANDLED) && _mouseButton[0]._lastDown > 0) {
		// left click-and-hold = block
		if (lastanim == Animation::startBlock)
			return;

//		pout << "AvatarMover: combat block" << Std::endl;

		if (checkTurn(mousedir, false)) return;

		waitFor(avatar->doAnim(Animation::startBlock, mousedir));
		return;
	}

	if ((_mouseButton[0]._state & MBS_DOWN) &&
	        !(_mouseButton[0]._state & MBS_HANDLED) &&
	        _mouseButton[0]._curDown - _mouseButton[0]._lastDown <= DOUBLE_CLICK_TIMEOUT) {
		_mouseButton[0]._state |= MBS_HANDLED;
		_mouseButton[0]._lastDown = 0;

		if (canAttack()) {
			// double left click = attack
//			pout << "AvatarMover: combat attack" << Std::endl;

			if (checkTurn(mousedir, true)) return;

			waitFor(avatar->doAnim(Animation::attack, mousedir));
			_lastAttack = _lastFrame;

			// attacking gives str/dex
			avatar->accumulateStr(1 + (getRandom() % 2));
			avatar->accumulateDex(2 + (getRandom() % 2));
		}

		return;
	}

	if ((_mouseButton[1]._state & MBS_DOWN) &&
	        !(_mouseButton[1]._state & MBS_HANDLED) &&
	        _mouseButton[1]._curDown - _mouseButton[1]._lastDown <= DOUBLE_CLICK_TIMEOUT) {
		_mouseButton[1]._state |= MBS_HANDLED;
		_mouseButton[1]._lastDown = 0;

		Gump *desktopgump = Ultima8Engine::get_instance()->getDesktopGump();
		if (desktopgump->TraceObjId(mx, my) == 1) {
			// double right click on avatar = toggle combat mode
			avatar->toggleInCombat();
			waitFor(avatar->doAnim(Animation::unreadyWeapon, direction));
			return;
		}

		if (canAttack()) {
			// double right click = kick
//			pout << "AvatarMover: combat kick" << Std::endl;

			if (checkTurn(mousedir, false)) return;

			waitFor(avatar->doAnim(Animation::kick, mousedir));
			_lastAttack = _lastFrame;

			// kicking gives str/dex
			avatar->accumulateStr(1 + (getRandom() % 2));
			avatar->accumulateDex(2 + (getRandom() % 2));
		}

		return;
	}

	if ((_mouseButton[1]._state & MBS_DOWN) &&
	        (_mouseButton[1]._state & MBS_HANDLED) && _mouseButton[1]._lastDown > 0) {
		// right mouse button is down long enough to act on it
		// if facing right direction, walk
		//!! TODO: check if you can actually take this step

		if (checkTurn(mousedir, true)) return;

		int32 nextdir = mousedir;

		if (lastanim == Animation::run) {
			// want to run while in combat mode?
			// first sheath weapon
			nextanim = Animation::readyWeapon;
		} else if (ABS(direction - mousedir) == 4) {
			nextanim = Animation::retreat;
			nextdir = direction;
		} else
			nextanim = Animation::advance;

		if (mouselength == 2) {
			// Take a step before running
			nextanim = Animation::walk;
			avatar->setActorFlag(Actor::ACT_COMBATRUN);
			avatar->toggleInCombat();
			MusicProcess::get_instance()->playCombatMusic(110); // CONSTANT!!
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
		return;
	}

	// if clicked, turn in mouse direction
	if (m0clicked || m1clicked)
		if (checkTurn(mousedir, false)) return;

	// not doing anything in particular? stand
	// TODO: make sure falling works properly.
	if (lastanim != Animation::combatStand) {
		nextanim = Animation::combatStand;
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
	}
}

void AvatarMoverProcess::handleNormalMode() {
	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	Mouse *mouse = Mouse::get_instance();
	MainActor *avatar = getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	Animation::Sequence nextanim = Animation::walk;
	int32 direction = avatar->getDir();
	uint32 now = g_system->getMillis();
	bool stasis = guiapp->isAvatarInStasis();
	bool combatRun = (avatar->getActorFlags() & Actor::ACT_COMBATRUN) != 0;

	int32 mx, my;
	mouse->getMouseCoords(mx, my);
	unsigned int mouselength = mouse->getMouseLength(mx, my);

	// adjust to world direction
	int32 mousedir = (mouse->getMouseDirection(mx, my) + 7) % 8;

	// Store current idle time. (Also see end of function.)
	uint32 currentIdleTime = _idleTime;
	_idleTime = 0;

	// User toggled combat while in combatRun
	if (avatar->isInCombat()) {
		avatar->clearActorFlag(Actor::ACT_COMBATRUN);
		avatar->toggleInCombat();
	}

	// If Avatar has fallen down, stand up.
	if (lastanim == Animation::die || lastanim == Animation::fallBackwards) {
		if (!stasis) {
//			pout << "AvatarMover: standing up" << Std::endl;
			waitFor(avatar->doAnim(Animation::standUp, direction));
		}
		return;
	}

	// If still in combat stance, sheathe weapon
	if (!stasis && Animation::isCombatAnim(lastanim)) {
//		pout << "AvatarMover: sheathing weapon" << Std::endl;

		ProcId anim1 = avatar->doAnim(Animation::unreadyWeapon, direction);
		ProcId anim2 = avatar->doAnim(Animation::stand, direction);
		Process *anim2p = Kernel::get_instance()->getProcess(anim2);
		anim2p->waitFor(anim1);
		waitFor(anim2);

		return;
	}

	bool m0clicked = false;
	bool m1clicked = false;

	// check mouse state to see what needs to be done
	if (!(_mouseButton[0]._state & MBS_HANDLED) &&
	        now - _mouseButton[0]._curDown > DOUBLE_CLICK_TIMEOUT) {
		m0clicked = true;
		_mouseButton[0]._state |= MBS_HANDLED;
	}

	if (!(_mouseButton[1]._state & MBS_HANDLED) &&
	        now - _mouseButton[1]._curDown > DOUBLE_CLICK_TIMEOUT) {
		m1clicked = true;
		_mouseButton[1]._state |= MBS_HANDLED;
	}

	// see if mouse was just released
	if (!(_mouseButton[0]._state & MBS_RELHANDLED))
		_mouseButton[0]._state |= MBS_RELHANDLED; // don't care about left

	if (!(_mouseButton[1]._state & MBS_RELHANDLED)) {
		_mouseButton[1]._state |= MBS_RELHANDLED;

		// if we were running in combat mode, slow to a walk, draw weapon
		// (even in stasis)
		if (combatRun) {
			avatar = getMainActor();
			avatar->clearActorFlag(Actor::ACT_COMBATRUN);
			avatar->toggleInCombat();
			ProcId walkpid = avatar->doAnim(Animation::walk, direction);
			ProcId drawpid = avatar->doAnim(Animation::readyWeapon, direction);
			Process *drawproc = Kernel::get_instance()->getProcess(drawpid);
			drawproc->waitFor(walkpid);
			waitFor(drawpid);
			return;
		}

		// if we were running, slow to a walk before stopping
		// (even in stasis)
		if (lastanim == Animation::run) {
			ProcId walkpid = avatar->doAnim(Animation::walk, direction);
			ProcId standpid = avatar->doAnim(Animation::stand, direction);
			Process *standproc = Kernel::get_instance()->getProcess(standpid);
			standproc->waitFor(walkpid);
			waitFor(standpid);
			return;
		}

		// TODO: if we were hanging, fall

		// otherwise, stand
		if (!stasis)
			waitFor(avatar->doAnim(Animation::stand, direction));
		return;
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	// both mouse buttons down
	if (!(_mouseButton[0]._state & MBS_HANDLED) &&
	        !(_mouseButton[1]._state & MBS_HANDLED)) {
		// notice these are all unsigned.
		uint32 down = _mouseButton[1]._curDown;
		if (_mouseButton[0]._curDown < down) {
			down = down - _mouseButton[0]._curDown;
		} else {
			down = _mouseButton[0]._curDown - down;
		}

		if (down < DOUBLE_CLICK_TIMEOUT) {
			_mouseButton[0]._state |= MBS_HANDLED;
			_mouseButton[1]._state |= MBS_HANDLED;
			// We got a left mouse down.
			// Note that this automatically means right was down too.

			if (checkTurn(mousedir, false)) return;

			nextanim = Animation::jumpUp;
			if (mouselength > 0) {
				nextanim = Animation::jump;
			}
			// check if there's something we can climb up onto here

			Animation::Sequence climbanim = Animation::climb72;
			while (climbanim >= Animation::climb16) {
				if (avatar->tryAnim(climbanim, direction) ==
				        Animation::SUCCESS) {
					nextanim = climbanim;
				}
				climbanim = static_cast<Animation::Sequence>(climbanim - 1);
			}

			if (nextanim == Animation::jump) {
				jump(nextanim, direction);
				return;
			}

			if (nextanim != Animation::jumpUp) {
				// climbing gives str/dex
				avatar->accumulateStr(2 + nextanim - Animation::climb16);
				avatar->accumulateDex(2 * (2 + nextanim - Animation::climb16));
			}

			nextanim = Animation::checkWeapon(nextanim, lastanim);
			waitFor(avatar->doAnim(nextanim, direction));
			return;
		}
	}

	if ((!(_mouseButton[0]._state & MBS_HANDLED) || m0clicked) &&
	        (_mouseButton[1]._state & MBS_DOWN)) {
		_mouseButton[0]._state |= MBS_HANDLED;
		// We got a left mouse down.
		// Note that this automatically means right was down at the time too.

		if (checkTurn(mousedir, false)) return;

		nextanim = Animation::jumpUp;

		// check if we need to do a running jump
		if (lastanim == Animation::run ||
		        lastanim == Animation::runningJump) {
			pout << "AvatarMover: running jump" << Std::endl;
			jump(Animation::runningJump, direction);
			return;
		} else if (mouselength > 0) {
			pout << "AvatarMover: jump" << Std::endl;
			jump(Animation::jump, direction);
			return;
		}
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
		return;

		// CHECKME: check what needs to happen when keeping left pressed
	}

	if ((_mouseButton[1]._state & MBS_DOWN) &&
	        !(_mouseButton[1]._state & MBS_HANDLED) &&
	        _mouseButton[1]._curDown - _mouseButton[1]._lastDown <= DOUBLE_CLICK_TIMEOUT) {
		Gump *desktopgump = Ultima8Engine::get_instance()->getDesktopGump();
		if (desktopgump->TraceObjId(mx, my) == 1) {
			// double right click on avatar = toggle combat mode
			_mouseButton[1]._state |= MBS_HANDLED;
			_mouseButton[1]._lastDown = 0;

			avatar->toggleInCombat();
			waitFor(avatar->doAnim(Animation::readyWeapon, direction));
			return;
		}
	}

	if ((_mouseButton[1]._state & MBS_DOWN) &&
	        (_mouseButton[1]._state & MBS_HANDLED)) {
		// right mouse button is down long enough to act on it
		// if facing right direction, walk

		nextanim = Animation::step;

		if (mouselength == 1)
			nextanim = Animation::walk;

		if (mouselength == 2) {
			if (lastanim == Animation::run
			        || lastanim == Animation::runningJump
			        || lastanim == Animation::walk)
				nextanim = Animation::run;
			else
				nextanim = Animation::walk;
		}

		step(nextanim, mousedir);
		return;
	}

	if (m1clicked)
		if (checkTurn(mousedir, false)) return;

	// doing another animation?
	if (Kernel::get_instance()->getNumProcesses(1, 0x00F0))
		return;

	// idle
	_idleTime = currentIdleTime + 1;

	// currently shaking head?
	if (lastanim == Animation::lookLeft || lastanim == Animation::lookRight) {
		if ((getRandom() % 1500) + 30 < _idleTime) {
			_lastHeadShakeAnim = lastanim;
			waitFor(avatar->doAnim(Animation::stand, direction));
			_idleTime = 0;
			return;
		}
	} else {
		if ((getRandom() % 3000) + 150 < _idleTime) {
			if (getRandom() % 5 == 0)
				nextanim = _lastHeadShakeAnim;
			else if (_lastHeadShakeAnim == Animation::lookLeft)
				nextanim = Animation::lookRight;
			else
				nextanim = Animation::lookLeft;
			waitFor(avatar->doAnim(nextanim, direction));
			_idleTime = 0;
		}
	}
}

void AvatarMoverProcess::step(Animation::Sequence action, int direction,
                              bool adjusted) {
	assert(action == Animation::step || action == Animation::walk ||
	       action == Animation::run);

	MainActor *avatar = getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();

	Animation::Result res = avatar->tryAnim(action, direction);

	int stepdir = direction;

	if (res == Animation::FAILURE ||
	        (action == Animation::step && res == Animation::END_OFF_LAND)) {
		int altdir1 = (stepdir + 1) % 8;
		int altdir2 = (stepdir + 7) % 8;

		res = avatar->tryAnim(action, altdir1);
		if (res == Animation::FAILURE ||
		        (action == Animation::step && res == Animation::END_OFF_LAND)) {
			res = avatar->tryAnim(action, altdir2);
			if (res == Animation::FAILURE ||
			        (action == Animation::step && res == Animation::END_OFF_LAND)) {
				// Can't walk in this direction.
				// Try to take a smaller step

				if (action == Animation::walk) {
					step(Animation::step, direction, true);
					return;
				} else if (action == Animation::run) {
					step(Animation::walk, direction, true);
					return;
				}

			} else {
				stepdir = altdir2;
			}
		} else {
			stepdir = altdir1;
		}


	}

	if (action == Animation::step && res == Animation::END_OFF_LAND &&
	        lastanim != Animation::keepBalance && !adjusted) {
		if (checkTurn(stepdir, false)) return;
		waitFor(avatar->doAnim(Animation::keepBalance, stepdir));
		return;
	}

	if (action == Animation::step && res == Animation::FAILURE) {
		action = Animation::stand;
	}


	bool moving = (action == Animation::run || action == Animation::walk);

	if (checkTurn(stepdir, moving)) return;

	action = Animation::checkWeapon(action, lastanim);
	waitFor(avatar->doAnim(action, stepdir));
}

void AvatarMoverProcess::jump(Animation::Sequence action, int direction) {
	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	Mouse *mouse = Mouse::get_instance();
	MainActor *avatar = getMainActor();
	int32 mx, my;
	mouse->getMouseCoords(mx, my);

	// running jump
	if (action == Animation::runningJump) {
		waitFor(avatar->doAnim(action, direction));
		return;
	}

	// airwalk
	if ((avatar->getActorFlags() & Actor::ACT_AIRWALK) &&
	        action == Animation::jump) {
		waitFor(avatar->doAnim(Animation::airwalkJump, direction));
		return;
	}

	bool targeting;
	SettingManager::get_instance()->get("targetedjump", targeting);

	if (targeting) {
		int32 coords[3];
		GameMapGump *gameMap = guiapp->getGameMapGump();
		// We need the Gump's x/y for TraceCoordinates
		gameMap->ScreenSpaceToGump(mx, my);
		ObjId targetId = gameMap->TraceCoordinates(mx, my, coords);
		Item *target = getItem(targetId);

		int32 ax, ay, az;
		avatar->getCentre(ax, ay, az);

		int32 xrange = ax - coords[0];
		if (xrange < 0) xrange = -xrange;
		int32 yrange = ay - coords[1];
		if (yrange < 0) yrange = -yrange;
		int maxrange = avatar->getStr() * 32;

		if (target && target->getShapeInfo()->is_land() &&
		        xrange < maxrange && yrange < maxrange) {
			// Original also only lets you jump at the Z_FACE
			Process *p = new TargetedAnimProcess(avatar, Animation::jumpUp,
			                                     direction, coords);
			waitFor(Kernel::get_instance()->addProcess(p));
			return;
		}
		// invalid target or out of range
		waitFor(avatar->doAnim(Animation::shakeHead, direction));
	} else {
		waitFor(avatar->doAnim(Animation::jump, direction));
	}
}

void AvatarMoverProcess::turnToDirection(int direction) {
	MainActor *avatar = getMainActor();
	bool combatRun = (avatar->getActorFlags() & Actor::ACT_COMBATRUN) != 0;
	int curdir = avatar->getDir();
	int stepDelta;
	bool combat = avatar->isInCombat() && !combatRun;
	Animation::Sequence turnanim;
	Animation::Sequence standanim = Animation::stand;

	if ((curdir - direction + 8) % 8 < 4) {
		stepDelta = -1;
		turnanim = Animation::lookLeft;
	} else {
		stepDelta = 1;
		turnanim = Animation::lookRight;
	}

	if (combat) {
		turnanim = Animation::combatStand;
		standanim = Animation::combatStand;
	}

	ProcId prevpid = 0;

	for (int dir = curdir; dir != direction;) {
		ProcId animpid = avatar->doAnim(turnanim, dir);

		if (prevpid) {
			Process *proc = Kernel::get_instance()->getProcess(animpid);
			assert(proc);
			proc->waitFor(prevpid);
		}

		prevpid = animpid;

		dir = (dir + stepDelta + 8) % 8;
	}

	ProcId animpid = avatar->doAnim(standanim, direction);

	if (prevpid) {
		Process *proc = Kernel::get_instance()->getProcess(animpid);
		assert(proc);
		proc->waitFor(prevpid);
	}

	waitFor(animpid);
}

bool AvatarMoverProcess::checkTurn(int direction, bool moving) {
	MainActor *avatar = getMainActor();
	bool combatRun = (avatar->getActorFlags() & Actor::ACT_COMBATRUN) != 0;
	int curdir = avatar->getDir();
	bool combat = avatar->isInCombat() && !combatRun;
	Animation::Sequence lastanim = avatar->getLastAnim();

	// Note: don't need to turn if moving backward in combat stance
	// CHECKME: currently, first turn in the right direction
	if (direction != curdir && !(
	            combat && ABS(direction - curdir) == 4)) {
		if (moving &&
		        (lastanim == Animation::walk || lastanim == Animation::run ||
		         lastanim == Animation::combatStand) &&
		        (ABS(direction - curdir) + 1 % 8 <= 2)) {
			// don't need to explicitly do a turn animation
			return false;
		}

		if (moving && lastanim == Animation::run) {
			// slow down to a walk first
			waitFor(avatar->doAnim(Animation::walk, curdir));
			return true;
		}

		turnToDirection(direction);
		return true;
	}

	return false;
}

bool AvatarMoverProcess::canAttack() {
	MainActor *avatar = getMainActor();
	return (_lastFrame > _lastAttack + (25 - avatar->getDex()));
}

void AvatarMoverProcess::OnMouseDown(int button, int32 mx, int32 my) {
	int bid = 0;

	switch (button) {
	case Shared::BUTTON_LEFT: {
		bid = 0;
		break;
	}
	case Shared::BUTTON_RIGHT: {
		bid = 1;
		break;
	}
	default:
		CANT_HAPPEN_MSG("invalid MouseDown passed to AvatarMoverProcess");
		break;
	};

	_mouseButton[bid]._lastDown = _mouseButton[bid]._curDown;
	_mouseButton[bid]._curDown = g_system->getMillis();
	_mouseButton[bid]._state |= MBS_DOWN;
	_mouseButton[bid]._state &= ~MBS_HANDLED;
}

void AvatarMoverProcess::OnMouseUp(int button) {
	int bid = 0;

	if (button == Shared::BUTTON_LEFT) {
		bid = 0;
	} else if (button == Shared::BUTTON_RIGHT) {
		bid = 1;
	} else {
		CANT_HAPPEN_MSG("invalid MouseUp passed to AvatarMoverProcess");
	}

	_mouseButton[bid]._state &= ~MBS_DOWN;
	_mouseButton[bid]._state &= ~MBS_RELHANDLED;
}


void AvatarMoverProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(_lastAttack);
	ods->write4(_idleTime);
	ods->write2(static_cast<uint8>(_lastHeadShakeAnim));
}

bool AvatarMoverProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	_lastAttack = ids->read4();
	_idleTime = ids->read4();
	_lastHeadShakeAnim = static_cast<Animation::Sequence>(ids->read2());

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
