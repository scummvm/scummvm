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

CruAvatarMoverProcess::CruAvatarMoverProcess() : AvatarMoverProcess(), _avatarAngle(0) {
}


CruAvatarMoverProcess::~CruAvatarMoverProcess() {
}


void CruAvatarMoverProcess::run() {

	// Even when we are not doing anything (because we're waiting for an anim)
	// we check if the combat angle needs updating - this keeps it smooth.

	const MainActor *avatar = getMainActor();
	assert(avatar);

	if (avatar->isInCombat() && !hasMovementFlags(MOVE_FORWARD | MOVE_BACK | MOVE_JUMP | MOVE_STEP)) {
		// See comment on _avatarAngle in header about these constants
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			if (hasMovementFlags(MOVE_RUN))
				_avatarAngle -= 375;
			else
				_avatarAngle -= 150;

			if (_avatarAngle < 0)
				_avatarAngle += 36000;
		}
		if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			if (hasMovementFlags(MOVE_RUN))
				_avatarAngle += 375;
			else
				_avatarAngle += 150;

			_avatarAngle = _avatarAngle % 36000;
		}
	}

	// Now do the regular process
	AvatarMoverProcess::run();
}


void CruAvatarMoverProcess::handleHangingMode() {
	// No hanging in crusader, this shouldn't happen?
	assert(false);
}

void CruAvatarMoverProcess::handleCombatMode() {
	MainActor *avatar = getMainActor();
	const Animation::Sequence lastanim = avatar->getLastAnim();
	Direction direction = Direction_FromCentidegrees(_avatarAngle);
	const Direction curdir = avatar->getDir();
	const bool stasis = Ultima8Engine::get_instance()->isAvatarInStasis();

	// never idle when in combat
	_idleTime = 0;

	// If Avatar has fallen down, stand up
	if (standUpIfNeeded(direction)) {
		return;
	} else if (stasis) {
		return;
	} else if (hasMovementFlags(MOVE_FORWARD)) {
		Animation::Sequence nextanim;
		if (hasMovementFlags(MOVE_STEP)) {
			nextanim = avatar->hasActorFlags(Actor::ACT_KNEELING) ?
							Animation::kneelingAdvance : Animation::advance;
		} else if (hasMovementFlags(MOVE_RUN)) {
			// Take a step before running
			avatar->toggleInCombat();
			if (lastanim != Animation::startRun)
				nextanim = Animation::startRun;
			else
				nextanim = Animation::run;
		} else if (hasMovementFlags(MOVE_JUMP)) {
			avatar->toggleInCombat();
			nextanim = Animation::jumpForward;
		} else if (avatar->hasActorFlags(Actor::ACT_KNEELING)) {
			nextanim = Animation::stopKneeling;
			avatar->clearActorFlag(Actor::ACT_KNEELING);
		} else {
			// moving from combat stows weapon
			nextanim = Animation::walk;
			avatar->toggleInCombat();
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		step(nextanim, direction);
		return;
	} else if (hasMovementFlags(MOVE_BACK)) {
		Animation::Sequence nextanim;
		if (hasMovementFlags(MOVE_JUMP)) {
			nextanim = Animation::startKneeling;
			avatar->setActorFlag(Actor::ACT_KNEELING);
		} else {
			nextanim = avatar->hasActorFlags(Actor::ACT_KNEELING) ?
							Animation::kneelingRetreat : Animation::retreat;
		}
		waitFor(avatar->doAnim(nextanim, direction));
		return;
	} else if (hasMovementFlags(MOVE_STEP)) {
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			avatar->doAnim(Animation::slideLeft, direction);
			return;
		} else if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			avatar->doAnim(Animation::slideRight, direction);
			return;
		}
	} else if (hasMovementFlags(MOVE_JUMP)) {
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			//direction = Direction_TurnByDelta(direction, 4, dirmode_16dirs);
			avatar->doAnim(Animation::combatRollLeft, direction);
			return;
		} else if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			//direction = Direction_TurnByDelta(direction, -4, dirmode_16dirs);
			avatar->doAnim(Animation::combatRollRight, direction);
			return;
		}
	}

	int x, y;
	getMovementFlagAxes(x, y);
	if (x != 0 || y != 0) {
		Direction nextdir = Direction_FromCentidegrees(_avatarAngle);

		if (checkTurn(nextdir, true))
			return;

		Animation::Sequence nextanim = Animation::combatStand;
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

	Animation::Sequence idleanim = avatar->hasActorFlags(Actor::ACT_KNEELING) ?
						Animation::kneel : Animation::combatStand;

	if (curdir != direction) {
		// Slight hack: don't "wait" for this - we want to keep turning smooth,
		// and the process will not do anything else if an anim is active, so
		// it's safe.
		avatar->doAnim(idleanim, direction);
		return;
	}

	// not doing anything in particular? stand
	if (lastanim != idleanim) {
		Animation::Sequence nextanim = Animation::checkWeapon(idleanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
	}
}

void CruAvatarMoverProcess::handleNormalMode() {
	MainActor *avatar = getMainActor();
	const Animation::Sequence lastanim = avatar->getLastAnim();
	Direction direction = avatar->getDir();
	const bool stasis = Ultima8Engine::get_instance()->isAvatarInStasis();

	// Store current idle time. (Also see end of function.)
	uint32 currentIdleTime = _idleTime;
	_idleTime = 0;

	// User toggled combat while in combatRun
	if (avatar->isInCombat()) {
		avatar->clearActorFlag(Actor::ACT_COMBATRUN);
		avatar->toggleInCombat();
	}

	// In normal mode the internal angle is set based on the avatar direction
	_avatarAngle = Direction_ToCentidegrees(direction);

	// If Avatar has fallen down and not dead, get up!
	if (standUpIfNeeded(direction))
		return;

	// If still in combat stance, sheathe weapon
	if (!stasis && Animation::isCombatAnimU8(lastanim)) {
		putAwayWeapon(direction);
		return;
	}

	if (!hasMovementFlags(MOVE_ANY_DIRECTION) && lastanim == Animation::run) {
		// if we were running, slow to a walk before stopping
		// (even in stasis)
		slowFromRun(direction);
		return;
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	if (hasMovementFlags(MOVE_JUMP) && hasMovementFlags(MOVE_FORWARD)) {
		Animation::Sequence nextanim = Animation::jump;
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
		return;
	}

	bool moving = (lastanim == Animation::step || lastanim == Animation::run || lastanim == Animation::walk);

	DirectionMode dirmode = avatar->animDirMode(Animation::step);

	//  if we are trying to move, allow change direction only after move occurs to avoid spinning
	if (moving || !hasMovementFlags(MOVE_FORWARD | MOVE_BACK)) {
		direction = getTurnDirForTurnFlags(direction, dirmode);
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

	int x, y;
	getMovementFlagAxes(x, y);

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
			// Note: we don't actually need the blocker output, just add the parameter
			// for compilers that can't tell nullptr from 0..
			const Item *blocker;
			if (currentmap->isValidPosition(x, y, z, avatar->getShape(), avatar->getObjId(),
											nullptr, nullptr, &blocker)) {
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

bool CruAvatarMoverProcess::canAttack() {
	MainActor *avatar = getMainActor();
	return avatar->isInCombat();
}

void CruAvatarMoverProcess::tryAttack() {
	MainActor *avatar = getMainActor();
	Direction dir = avatar->getDir();
	if (!avatar->isInCombat()) {
		avatar->setInCombat(0);
		if (!avatar->hasActorFlags(Actor::ACT_WEAPONREADY))
			waitFor(avatar->doAnim(Animation::readyWeapon, dir));
	} else {
		if (canAttack()) {
			// Fire event happens from animation
			waitFor(avatar->doAnim(Animation::attack, dir));
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
