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
#include "ultima/ultima8/world/actors/cru_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/actors/targeted_anim_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CruAvatarMoverProcess)

CruAvatarMoverProcess::CruAvatarMoverProcess() : AvatarMoverProcess() {
}


CruAvatarMoverProcess::~CruAvatarMoverProcess() {
}


void CruAvatarMoverProcess::handleHangingMode() {
	// No hanging in crusader, this shouldn't happen?
	assert(false);
}

void CruAvatarMoverProcess::handleCombatMode() {
	MainActor *avatar = getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	Direction direction = avatar->getDir();
	bool stasis = Ultima8Engine::get_instance()->isAvatarInStasis();

	// never idle when in combat
	_idleTime = 0;

	// If Avatar has fallen down, stand up
	if (lastanim == Animation::die || lastanim == Animation::fallBackwards) {
		if (!stasis) {
			waitFor(avatar->doAnim(Animation::standUp, direction));
		}
		return;
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	bool moving = (lastanim == Animation::advance || lastanim == Animation::retreat);

	DirectionMode dirmode = avatar->animDirMode(Animation::combatStand);

	//  if we are trying to move, allow change direction only after move occurs to avoid spinning
	if (moving || !hasMovementFlags(MOVE_FORWARD | MOVE_BACK)) {
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			direction = Direction_OneLeft(direction, dirmode);
		}

		if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			direction = Direction_OneRight(direction, dirmode);
		}
	}

	if (hasMovementFlags(MOVE_FORWARD)) {
		Animation::Sequence nextanim;
		if (hasMovementFlags(MOVE_STEP)) {
			nextanim = Animation::advance;
		} else if (hasMovementFlags(MOVE_RUN)) {
			// Take a step before running
			avatar->toggleInCombat();
			if (lastanim != Animation::startRun)
				nextanim = Animation::startRun;
			else
				nextanim = Animation::run;
		} else {
			// moving from combat stows weapon
			nextanim = Animation::walk;
			avatar->toggleInCombat();
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		step(nextanim, direction);
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
		Direction nextdir = Direction_Get(y, x, dirmode_8dirs);

		if (checkTurn(nextdir, true))
			return;

		Animation::Sequence nextanim;
		if (lastanim == Animation::run) {
			// want to run while in combat mode?
			// first sheath weapon
			nextanim = Animation::readyWeapon;
		} else if (Direction_Invert(direction) == nextdir) {
			nextanim = Animation::retreat;
			nextdir = direction;
		}

		if (hasMovementFlags(MOVE_RUN)) {
			// Take a step before running
			nextanim = Animation::startRun;
			avatar->toggleInCombat();
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		step(nextanim, nextdir);
		return;
	}

	if (checkTurn(direction, false))
		return;

	// not doing anything in particular? stand
	if (lastanim != Animation::combatStand) {
		Animation::Sequence nextanim = Animation::combatStand;
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
	}
}

void CruAvatarMoverProcess::handleNormalMode() {
	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	MainActor *avatar = getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	Direction direction = avatar->getDir();
	bool stasis = guiapp->isAvatarInStasis();

	// Store current idle time. (Also see end of function.)
	uint32 currentIdleTime = _idleTime;
	_idleTime = 0;

	// User toggled combat while in combatRun
	if (avatar->isInCombat()) {
		avatar->clearActorFlag(Actor::ACT_COMBATRUN);
		avatar->toggleInCombat();
	}

	// If Avatar has fallen down do nothing
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

	if (!hasMovementFlags(MOVE_ANY_DIRECTION)) {
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
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	if (hasMovementFlags(MOVE_JUMP) && hasMovementFlags(MOVE_ANY_DIRECTION)) {
		clearMovementFlag(MOVE_JUMP);

		Animation::Sequence nextanim = Animation::jump;
		// check if we need to do a running jump
		if (lastanim == Animation::run || lastanim == Animation::runningJump) {
			nextanim = Animation::runningJump;
		}
		else if (avatar->hasActorFlags(Actor::ACT_AIRWALK)) {
			nextanim = Animation::airwalkJump;
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
		return;
	}

	if (hasMovementFlags(MOVE_JUMP)) {
		clearMovementFlag(MOVE_JUMP);

		Animation::Sequence nextanim = Animation::jumpUp;

		if (nextanim == Animation::jump) {
			jump(Animation::jump, direction);
		} else {
			nextanim = Animation::checkWeapon(nextanim, lastanim);
			waitFor(avatar->doAnim(nextanim, direction));
		}
		return;
	}

	bool moving = (lastanim == Animation::step || lastanim == Animation::run || lastanim == Animation::walk);

	DirectionMode dirmode = avatar->animDirMode(Animation::step);

	//  if we are trying to move, allow change direction only after move occurs to avoid spinning
	if (moving || !hasMovementFlags(MOVE_FORWARD | MOVE_BACK)) {
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			direction = Direction_OneLeft(direction, dirmode);
		}

		if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			direction = Direction_OneRight(direction, dirmode);
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
		step(nextanim, Direction_Invert(direction));

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
		direction = Direction_Get(y, x, dirmode_8dirs);
		step(nextanim, direction);
		return;
	}

	if (checkTurn(direction, moving))
		return;

	// doing another animation?
	if (Kernel::get_instance()->getNumProcesses(1, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE))
		return;

	// if we were running, slow to a walk before stopping
	if (lastanim == Animation::run) {
		waitFor(avatar->doAnim(Animation::walk, direction));
		return;
	}

	// not doing anything in particular? stand
	if (lastanim != Animation::stand && currentIdleTime == 0) {
		waitFor(avatar->doAnim(Animation::stand, direction));
		return;
	}

	// idle
	_idleTime = currentIdleTime + 1;
}

void CruAvatarMoverProcess::step(Animation::Sequence action, Direction direction,
                              bool adjusted) {
	MainActor *avatar = getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();

	Animation::Result res = avatar->tryAnim(action, direction);

	if (res != Animation::SUCCESS) {
		World *world = World::get_instance();
		CurrentMap *currentmap = world->getCurrentMap();

		// Search right/left gradually increasing distance to see if we can make the move work.

		Direction dir_right = Direction_TurnByDelta(direction, 4, dirmode_16dirs);
		Direction dir_left = Direction_TurnByDelta(direction, -4, dirmode_16dirs);
		Point3 origpt;
		avatar->getLocation(origpt);
		static const int ADJUSTMENTS[] = {0x10, 0x10, 0x20, 0x20, 0x30, 0x30,
			0x40, 0x40, 0x50, 0x50};

		for (int i = 0; i < ARRAYSIZE(ADJUSTMENTS); i++) {
			Direction testdir = (i % 2 ? dir_left : dir_right);
			int32 x = origpt.x + Direction_XFactor(testdir) * ADJUSTMENTS[i];
			int32 y = origpt.y + Direction_YFactor(testdir) * ADJUSTMENTS[i];
			int32 z = origpt.z;
			if (currentmap->isValidPosition(x, y, z, avatar->getShape(), avatar->getObjId(),
											nullptr, nullptr, nullptr)) {
				avatar->setLocation(x, y, z);
				res = avatar->tryAnim(action, direction);
				if (res == Animation::SUCCESS)
					break;
			}
		}

		if (res != Animation::SUCCESS) {
			// reset location, couldn't move.
			avatar->setLocation(origpt.x, origpt.y, origpt.z);
		}
	}

	if ((action == Animation::step || action == Animation::advance ||
		 action == Animation::retreat || action == Animation::run ||
		 action == Animation::startRun || action == Animation::walk)
		&& res == Animation::FAILURE) {
		action = Animation::stand;
	}

	bool moving = (action == Animation::run || action == Animation::walk);

	if (checkTurn(direction, moving))
		return;

	debug(6, "Cru avatar step: picked action %d dir %d (test result %d)", action, direction, res);
	action = Animation::checkWeapon(action, lastanim);
	waitFor(avatar->doAnim(action, direction));
}

void CruAvatarMoverProcess::jump(Animation::Sequence action, Direction direction) {
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

	waitFor(avatar->doAnim(Animation::jump, direction));
}

bool CruAvatarMoverProcess::canAttack() {
	MainActor *avatar = getMainActor();
	return avatar->isInCombat();
}

void CruAvatarMoverProcess::tryAttack() {
	MainActor *avatar = getMainActor();
	Direction dir = avatar->getDir();
	if (!avatar->isInCombat()) {
		avatar->setInCombat(0);
		waitFor(avatar->doAnim(Animation::readyWeapon, dir));
	} else {
		if (canAttack()) {
			waitFor(avatar->doAnim(Animation::attack, dir));
			// FIXME: put some real values in here.
			int32 xs, ys, zs;
			avatar->getFootpadWorld(xs, ys, zs);
			avatar->fireWeapon(xs / 2, ys / 2, zs / 2, dir, 1, 1);
		}
	}
}

void CruAvatarMoverProcess::saveData(Common::WriteStream *ws) {
	AvatarMoverProcess::saveData(ws);
}

bool CruAvatarMoverProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!AvatarMoverProcess::loadData(rs, version)) return false;
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
