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
#include "ultima/ultima8/misc/direction.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarMoverProcess)

AvatarMoverProcess::AvatarMoverProcess() : Process(),
		_lastFrame(0), _lastAttack(0), _idleTime(0),
		_lastHeadShakeAnim(Animation::lookLeft),
		_movementFlags(0) {
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
	if (kernel->getNumProcesses(1, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE) > 0) {
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

	bool combatRun = avatar->hasActorFlags(Actor::ACT_COMBATRUN);
	if (avatar->isInCombat() && !combatRun)
		handleCombatMode();
	else
		handleNormalMode();
}

void AvatarMoverProcess::handleHangingMode() {
	bool stasis = Ultima8Engine::get_instance()->isAvatarInStasis();

	_idleTime = 0;

	if (stasis)
		return;

	bool m0clicked = false;
	//bool m1clicked = false;
	if (!_mouseButton[0].isState(MBS_HANDLED) &&
		!_mouseButton[0].curWithinDblClkTimeout()) {
		m0clicked = true;
		_mouseButton[0].setState(MBS_HANDLED);
	}
	if (!_mouseButton[1].isState(MBS_HANDLED) &&
	    !_mouseButton[1].curWithinDblClkTimeout()) {
		//m1clicked = true;
		_mouseButton[1].setState(MBS_HANDLED);
	}

	// if left mouse is down, try to climb up

	if (_mouseButton[0].isState(MBS_DOWN) &&
	        (!_mouseButton[0].isState(MBS_HANDLED) || m0clicked)) {
		_mouseButton[0].setState(MBS_HANDLED);
		_mouseButton[0]._lastDown = 0;
		MainActor *avatar = getMainActor();

		if (avatar->tryAnim(Animation::climb40, 8) == Animation::SUCCESS) {
			avatar->ensureGravityProcess()->terminate();
			waitFor(avatar->doAnim(Animation::climb40, 8));
		}
	}
}

void AvatarMoverProcess::handleCombatMode() {
	Mouse *mouse = Mouse::get_instance();
	MainActor *avatar = getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	int32 direction = avatar->getDir();
	bool stasis = Ultima8Engine::get_instance()->isAvatarInStasis();

	int32 mx, my;
	mouse->getMouseCoords(mx, my);
	unsigned int mouselength = mouse->getMouseLength(mx, my);

	int32 mousedir = mouse->getMouseDirectionWorld(mx, my);

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
	        !_mouseButton[0].isState(MBS_DOWN)) {
		waitFor(avatar->doAnim(Animation::stopBlock, direction));
		return;
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	bool m0clicked = false;
	bool m1clicked = false;

	if (!_mouseButton[0].isState(MBS_HANDLED) &&
	    !_mouseButton[0].curWithinDblClkTimeout()) {
		m0clicked = true;
		_mouseButton[0].setState(MBS_HANDLED);
	}

	if (!_mouseButton[1].isState(MBS_HANDLED) &&
	    !_mouseButton[1].curWithinDblClkTimeout()) {
		m1clicked = true;
		_mouseButton[1].setState(MBS_HANDLED);
	}

	if (!_mouseButton[0].isState(MBS_DOWN)) {
		clearMovementFlag(MOVE_MOUSE_DIRECTION);
	}

	if (_mouseButton[0].isState(MBS_DOWN) &&
	        _mouseButton[0].isState(MBS_HANDLED) && _mouseButton[0]._lastDown > 0) {
		// left click-and-hold = block
		if (lastanim == Animation::startBlock)
			return;

//		pout << "AvatarMover: combat block" << Std::endl;

		if (checkTurn(mousedir, false))
			return;

		waitFor(avatar->doAnim(Animation::startBlock, mousedir));
		return;
	}

	if (_mouseButton[0].isUnhandledDoubleClick()) {
		_mouseButton[0].setState(MBS_HANDLED);
		_mouseButton[0]._lastDown = 0;

		if (canAttack()) {
			// double left click = attack
//			pout << "AvatarMover: combat attack" << Std::endl;

			if (checkTurn(mousedir, true))
				return;

			waitFor(avatar->doAnim(Animation::attack, mousedir));
			_lastAttack = _lastFrame;

			// attacking gives str/dex
			avatar->accumulateStr(1 + (getRandom() % 2));
			avatar->accumulateDex(2 + (getRandom() % 2));
		}

		return;
	}

	if (_mouseButton[1].isUnhandledDoubleClick()) {
		_mouseButton[1].setState(MBS_HANDLED);
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

			if (checkTurn(mousedir, false))
				return;

			waitFor(avatar->doAnim(Animation::kick, mousedir));
			_lastAttack = _lastFrame;

			// kicking gives str/dex
			avatar->accumulateStr(1 + (getRandom() % 2));
			avatar->accumulateDex(2 + (getRandom() % 2));
		}

		return;
	}

	if (_mouseButton[1].isState(MBS_DOWN) && _mouseButton[1].isState(MBS_HANDLED)) {
		// Note: Orginal game allowed a move animation on a single right click.
		// This implementation needs right mouse to be held. 
		setMovementFlag(MOVE_MOUSE_DIRECTION);

		if (checkTurn(mousedir, true))
			return;

		//!! TODO: check if you can actually take this step
		int32 nextdir = mousedir;
		Animation::Sequence nextanim;

		if (lastanim == Animation::run) {
			// want to run while in combat mode?
			// first sheath weapon
			nextanim = Animation::readyWeapon;
		} else if (ABS(direction - mousedir) == 4) {
			nextanim = Animation::retreat;
			nextdir = direction;
		} else {
			nextanim = Animation::advance;
		}

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
		if (checkTurn(mousedir, false))
			return;

	bool moving = (lastanim == Animation::advance || lastanim == Animation::retreat);

	//  if we are trying to move, allow change direction only after move occurs to avoid spinning
	if (moving || !hasMovementFlags(MOVE_FORWARD | MOVE_BACK)) {
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			direction = (direction + 7) % 8;
		}

		if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			direction = (direction + 1) % 8;
		}
	}

	if (hasMovementFlags(MOVE_FORWARD)) {
		Animation::Sequence nextanim = Animation::advance;

		if (lastanim == Animation::run) {
			// want to run while in combat mode?
			// first sheath weapon
			nextanim = Animation::readyWeapon;
		} 

		if (hasMovementFlags(MOVE_RUN)) {
			// Take a step before running
			nextanim = Animation::walk;
			avatar->setActorFlag(Actor::ACT_COMBATRUN);
			avatar->toggleInCombat();
			MusicProcess::get_instance()->playCombatMusic(110); // CONSTANT!!
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
		return;
	}

	if (hasMovementFlags(MOVE_BACK)) {
		waitFor(avatar->doAnim(Animation::retreat, direction));
		return;
	}

	int y = 0;
	int x = 0;
	if (hasMovementFlags(MOVE_UP)) {
		y++;
	}
	if (hasMovementFlags(MOVE_DOWN)) {
		y--;
	}
	if (hasMovementFlags(MOVE_LEFT)) {
		x--;
	}
	if (hasMovementFlags(MOVE_RIGHT)) {
		x++;
	}

	if (x != 0 || y != 0) {
		int32 nextdir =  Get_direction(y, x);

		if (checkTurn(nextdir, true))
			return;

		Animation::Sequence nextanim;
		if (lastanim == Animation::run) {
			// want to run while in combat mode?
			// first sheath weapon
			nextanim = Animation::readyWeapon;
		} else if (ABS(direction - nextdir) == 4) {
			nextanim = Animation::retreat;
			nextdir = direction;
		} else {
			nextanim = Animation::advance;
		}

		if (hasMovementFlags(MOVE_RUN)) {
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

	if (checkTurn(direction, false))
		return;

	// not doing anything in particular? stand
	// TODO: make sure falling works properly.
	if (lastanim != Animation::combatStand) {
		Animation::Sequence nextanim = Animation::combatStand;
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
	}
}

void AvatarMoverProcess::handleNormalMode() {
	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	Mouse *mouse = Mouse::get_instance();
	MainActor *avatar = getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	int32 direction = avatar->getDir();
	bool stasis = guiapp->isAvatarInStasis();
	bool combatRun = avatar->hasActorFlags(Actor::ACT_COMBATRUN);

	int32 mx, my;
	mouse->getMouseCoords(mx, my);
	unsigned int mouselength = mouse->getMouseLength(mx, my);
	int32 mousedir = mouse->getMouseDirectionWorld(mx, my);

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
			waitFor(avatar->doAnim(Animation::standUp, direction));
		}
		return;
	}

	// If still in combat stance, sheathe weapon
	if (!stasis && Animation::isCombatAnim(lastanim)) {
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
	if (!_mouseButton[0].isState(MBS_HANDLED) &&
		!_mouseButton[0].curWithinDblClkTimeout()) {
		m0clicked = true;
		_mouseButton[0].setState(MBS_HANDLED);
	}

	if (!_mouseButton[1].isState(MBS_HANDLED) &&
	    !_mouseButton[1].curWithinDblClkTimeout()) {
		m1clicked = true;
		_mouseButton[1].setState(MBS_HANDLED);
	}

	if (!_mouseButton[1].isState(MBS_DOWN)) {
		clearMovementFlag(MOVE_MOUSE_DIRECTION);
	}

	if (_mouseButton[1].isState(MBS_DOWN) && _mouseButton[1].isState(MBS_HANDLED)) {
		// Note: Orginal game allowed a move animation on a single right click.
		// This implementation needs right mouse to be held. 
		setMovementFlag(MOVE_MOUSE_DIRECTION);
	}

	if (!hasMovementFlags(MOVE_ANY_DIRECTION)) {
		// if we were running in combat mode, slow to a walk, draw weapon
		// (even in stasis)
		if (combatRun) {
			avatar = getMainActor();
			avatar->clearActorFlag(Actor::ACT_COMBATRUN);
			avatar->toggleInCombat();

			// If we were running, slow to a walk before drawing weapon.
			// Note: Original game did not check last animation and always took an extra walk.
			if (lastanim == Animation::run || lastanim == Animation::runningJump) {
				ProcId walkpid = avatar->doAnim(Animation::walk, direction);
				ProcId drawpid = avatar->doAnim(Animation::readyWeapon, direction);
				Process *drawproc = Kernel::get_instance()->getProcess(drawpid);
				drawproc->waitFor(walkpid);
				waitFor(drawpid);
				return;
			}

			waitFor(avatar->doAnim(Animation::readyWeapon, direction));
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
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	// both mouse buttons down and not yet handled, check for jump.
	if (!_mouseButton[0].isState(MBS_HANDLED) && !_mouseButton[1].isState(MBS_HANDLED)) {
		// Take action if both were clicked within
		// double-click timeout of each other.
		// notice these are all unsigned.
		uint32 down = _mouseButton[1]._curDown;
		if (_mouseButton[0]._curDown < down) {
			down = down - _mouseButton[0]._curDown;
		} else {
			down = _mouseButton[0]._curDown - down;
		}

		if (down < DOUBLE_CLICK_TIMEOUT) {
			// Both buttons pressed within the timeout
			_mouseButton[0].setState(MBS_HANDLED);
			_mouseButton[1].setState(MBS_HANDLED);
			setMovementFlag(MOVE_JUMP);
		}
	}

	if ((!_mouseButton[0].isState(MBS_HANDLED) || m0clicked) && hasMovementFlags(MOVE_ANY_DIRECTION | MOVE_STEP)) {
		_mouseButton[0].setState(MBS_HANDLED);
		// We got a left mouse down while already moving in any direction or holding the step button.
		// CHECKME: check what needs to happen when keeping left pressed
		setMovementFlag(MOVE_JUMP);
	}

	if (_mouseButton[1].isUnhandledDoubleClick()) {
		Gump *desktopgump = Ultima8Engine::get_instance()->getDesktopGump();
		if (desktopgump->TraceObjId(mx, my) == 1) {
			// double right click on avatar = toggle combat mode
			_mouseButton[1].setState(MBS_HANDLED);
			_mouseButton[1]._lastDown = 0;

			avatar->toggleInCombat();
			waitFor(avatar->doAnim(Animation::readyWeapon, direction));
			return;
		}
	}

	if (hasMovementFlags(MOVE_JUMP) && hasMovementFlags(MOVE_ANY_DIRECTION)) {
		clearMovementFlag(MOVE_JUMP);

		if (hasMovementFlags(MOVE_MOUSE_DIRECTION)) {
			if (checkTurn(mousedir, false))
				return;
		}

		Animation::Sequence nextanim = Animation::jump;
		// check if we need to do a running jump
		if (lastanim == Animation::run || lastanim == Animation::runningJump) {
			nextanim = Animation::runningJump;
		}
		else if (avatar->hasActorFlags(Actor::ACT_AIRWALK)) {
			nextanim = Animation::airwalkJump;
		}
		else if ((hasMovementFlags(MOVE_MOUSE_DIRECTION) && mouselength == 0) || hasMovementFlags(MOVE_STEP)) {
			nextanim = Animation::jumpUp;
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
		return;
	}

	if (hasMovementFlags(MOVE_JUMP)) {
		clearMovementFlag(MOVE_JUMP);

		if (checkTurn(mousedir, false))
			return;

		Animation::Sequence nextanim = Animation::jumpUp;
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
			jump(Animation::jump, direction);
		}
		else {
			if (nextanim != Animation::jumpUp) {
				// climbing gives str/dex
				avatar->accumulateStr(2 + nextanim - Animation::climb16);
				avatar->accumulateDex(2 * (2 + nextanim - Animation::climb16));
			}
			nextanim = Animation::checkWeapon(nextanim, lastanim);
			waitFor(avatar->doAnim(nextanim, direction));
		}
		return;
	}
	
	if (hasMovementFlags(MOVE_MOUSE_DIRECTION)) {
		Animation::Sequence nextanim = Animation::step;

		if (mouselength == 1) {
			nextanim = Animation::walk;
		} else if (mouselength == 2) {
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
		if (checkTurn(mousedir, false))
			return;

	bool moving = (lastanim == Animation::step || lastanim == Animation::run || lastanim == Animation::walk);

	//  if we are trying to move, allow change direction only after move occurs to avoid spinning
	if (moving || !hasMovementFlags(MOVE_FORWARD | MOVE_BACK)) {
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			direction = (direction + 7) % 8;
		}

		if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			direction = (direction + 1) % 8;
		}
	}

	Animation::Sequence nextanim = Animation::walk;

	if (hasMovementFlags(MOVE_STEP)) {
		nextanim = Animation::step;
	} else if (hasMovementFlags(MOVE_RUN)) {
		if (lastanim == Animation::run
			    || lastanim == Animation::runningJump
			    || lastanim == Animation::walk)
			nextanim = Animation::run;
		else
			nextanim = Animation::walk;
	}

	if (hasMovementFlags(MOVE_FORWARD)) {
		step(nextanim, direction);
		return;
	}

	if (hasMovementFlags(MOVE_BACK)) {
		step(nextanim, (direction + 4) % 8);

		// flip to move forward once turned
		setMovementFlag(MOVE_FORWARD);
		return;
	}

	int y = 0;
	int x = 0;
	if (hasMovementFlags(MOVE_UP)) {
		y++;
	}
	if (hasMovementFlags(MOVE_DOWN)) {
		y--;
	}
	if (hasMovementFlags(MOVE_LEFT)) {
		x--;
	}
	if (hasMovementFlags(MOVE_RIGHT)) {
		x++;
	}

	if (x != 0 || y != 0) {
		direction = Get_direction(y, x);
		step(nextanim, direction);
		return;
	}

	if (checkTurn(direction, moving))
		return;

	// doing another animation?
	if (Kernel::get_instance()->getNumProcesses(1, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE))
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
			return;
		}
	}

	// if we were running, slow to a walk before stopping
	if (lastanim == Animation::run) {
		waitFor(avatar->doAnim(Animation::walk, direction));
	}

	// not doing anything in particular? stand
	if (lastanim != Animation::stand) {
		waitFor(avatar->doAnim(Animation::stand, direction));
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
		debug(6, "Step: end off land dir %d, try other dir", stepdir);
		int altdir1 = (stepdir + 1) % 8;
		int altdir2 = (stepdir + 7) % 8;

		res = avatar->tryAnim(action, altdir1);
		if (res == Animation::FAILURE ||
		        (action == Animation::step && res == Animation::END_OFF_LAND)) {
			debug(6, "Step: end off land dir %d, altdir1 %d failed, try altdir2 %d", stepdir, altdir1, altdir2);
			res = avatar->tryAnim(action, altdir2);
			if (res == Animation::FAILURE ||
			        (action == Animation::step && res == Animation::END_OFF_LAND)) {
				// Can't walk in this direction.
				// Try to take a smaller step

				if (action == Animation::walk) {
					debug(6, "Step: end off land both altdirs failed, smaller step (step)");
					step(Animation::step, direction, true);
					return;
				} else if (action == Animation::run) {
					debug(6, "Step: end off land both altdirs failed, smaller step (walk)");
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
		if (checkTurn(stepdir, false))
			return;
		debug(6, "Step: end off land both altdirs failed, keep balance.");
		waitFor(avatar->doAnim(Animation::keepBalance, stepdir));
		return;
	}

	if (action == Animation::step && res == Animation::FAILURE) {
		action = Animation::stand;
	}


	bool moving = (action == Animation::run || action == Animation::walk);

	if (checkTurn(stepdir, moving))
		return;

	debug(6, "Step: step ok: action %d dir %d", action, stepdir);
	action = Animation::checkWeapon(action, lastanim);
	waitFor(avatar->doAnim(action, stepdir));
}

void AvatarMoverProcess::jump(Animation::Sequence action, int direction) {
	MainActor *avatar = getMainActor();

	// running jump
	if (action == Animation::runningJump) {
		waitFor(avatar->doAnim(action, direction));
		return;
	}

	// airwalk
	if (avatar->hasActorFlags(Actor::ACT_AIRWALK) &&
	        action == Animation::jump) {
		waitFor(avatar->doAnim(Animation::airwalkJump, direction));
		return;
	}

	bool targeting;
	SettingManager::get_instance()->get("targetedjump", targeting);

	if (targeting) {
		Mouse *mouse = Mouse::get_instance();
		int32 coords[3];
		int32 mx, my;
		mouse->getMouseCoords(mx, my);
		GameMapGump *gameMap = Ultima8Engine::get_instance()->getGameMapGump();
		// We need the Gump's x/y for TraceCoordinates
		gameMap->ScreenSpaceToGump(mx, my);
		ObjId targetId = gameMap->TraceCoordinates(mx, my, coords);
		Item *target = getItem(targetId);

		int32 ax, ay, az;
		avatar->getCentre(ax, ay, az);

		int32 xrange = abs(ax - coords[0]);
		int32 yrange = abs(ay - coords[1]);
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
	bool combatRun = avatar->hasActorFlags(Actor::ACT_COMBATRUN);
	int curdir = avatar->getDir();
	int stepDelta;
	bool combat = avatar->isInCombat() && !combatRun;
	Animation::Sequence turnanim;
	Animation::Sequence standanim = Animation::stand;

	// note curdir-direction can be negative, hence + 8 % 8
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

	// Create a sequence of turn animations from
	// our current direction to the new one
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
	int curdir = avatar->getDir();
	bool combat = avatar->isInCombat() && !avatar->hasActorFlags(Actor::ACT_COMBATRUN);

	// Note: don't need to turn if moving backward in combat stance
	// CHECKME: currently, first turn in the right direction
	if (direction != curdir && !(
	            combat && ABS(direction - curdir) == 4)) {
		Animation::Sequence lastanim = avatar->getLastAnim();

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

void AvatarMoverProcess::onMouseDown(int button, int32 mx, int32 my) {
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
	_mouseButton[bid].setState(MBS_DOWN);
	_mouseButton[bid].clearState(MBS_HANDLED);
}

void AvatarMoverProcess::onMouseUp(int button) {
	int bid = 0;

	if (button == Shared::BUTTON_LEFT) {
		bid = 0;
	} else if (button == Shared::BUTTON_RIGHT) {
		bid = 1;
	} else {
		CANT_HAPPEN_MSG("invalid MouseUp passed to AvatarMoverProcess");
	}

	_mouseButton[bid].clearState(MBS_DOWN);
}


void AvatarMoverProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(_lastAttack);
	ws->writeUint32LE(_idleTime);
	ws->writeUint16LE(static_cast<uint8>(_lastHeadShakeAnim));
}

bool AvatarMoverProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_lastAttack = rs->readUint32LE();
	_idleTime = rs->readUint32LE();
	_lastHeadShakeAnim = static_cast<Animation::Sequence>(rs->readUint16LE());

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
