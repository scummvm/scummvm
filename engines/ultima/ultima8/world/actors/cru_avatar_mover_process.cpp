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

#include "ultima/ultima8/world/actors/cru_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/misc/direction_util.h"

namespace Ultima {
namespace Ultima8 {

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

	// When not in combat the angle is kept as -1
	if (avatar->isInCombat()) {
		if (_avatarAngle < 0) {
			_avatarAngle = Direction_ToCentidegrees(avatar->getDir());
		}
		if (!hasMovementFlags(MOVE_FORWARD | MOVE_BACK | MOVE_JUMP | MOVE_STEP)) {
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
	} else {
		_avatarAngle = -1;
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
	Direction direction = (_avatarAngle >= 0 ? Direction_FromCentidegrees(_avatarAngle) : avatar->getDir());
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
			nextanim = avatar->isKneeling() ?
							Animation::kneelingAdvance : Animation::advance;
		} else if (hasMovementFlags(MOVE_RUN)) {
			// Take a step before running
			if (lastanim != Animation::startRunWithLargeWeapon && lastanim != Animation::run)
				nextanim = Animation::startRunWithLargeWeapon;
			else
				nextanim = Animation::run;
		} else if (hasMovementFlags(MOVE_JUMP)) {
			if (lastanim == Animation::walk || lastanim == Animation::run)
				nextanim = Animation::jumpForward;
			else
				nextanim = Animation::jump;
			// Jump always ends out of combat
			avatar->clearInCombat();
		} else if (avatar->isKneeling()) {
			nextanim = Animation::stopKneeling;
			avatar->clearActorFlag(Actor::ACT_KNEELING);
		} else {
			// moving forward from combat stows weapon
			nextanim = Animation::walk;
			avatar->toggleInCombat();
		}

		// don't check weapon here, Avatar can go straight from drawn-weapon to
		// walking forward.
		step(nextanim, direction);
		return;
	} else if (hasMovementFlags(MOVE_BACK)) {
		Animation::Sequence nextanim;
		if (hasMovementFlags(MOVE_JUMP)) {
			if (!avatar->isKneeling()) {
				nextanim = Animation::startKneeling;
				avatar->setActorFlag(Actor::ACT_KNEELING);
			} else {
				// Do nothing if already kneeling
				return;
			}
		} else {
			nextanim = Animation::retreat;
		}
		waitFor(avatar->doAnim(nextanim, direction));
		return;
	} else if (hasMovementFlags(MOVE_STEP)) {
		if (avatar->isKneeling()) {
			avatar->doAnim(Animation::stopKneeling, direction);
			return;
		} else {
			if (hasMovementFlags(MOVE_TURN_LEFT)) {
				avatar->doAnim(Animation::slideLeft, direction);
				return;
			} else if (hasMovementFlags(MOVE_TURN_RIGHT)) {
				avatar->doAnim(Animation::slideRight, direction);
				return;
			}
		}
	} else if (hasMovementFlags(MOVE_JUMP)) {
		if (hasMovementFlags(MOVE_TURN_LEFT)) {
			if (avatar->isKneeling())
				avatar->doAnim(Animation::slowCombatRollLeft, direction);
			else
				avatar->doAnim(Animation::combatRollLeft, direction);
			return;
		} else if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			if (avatar->isKneeling())
				avatar->doAnim(Animation::slowCombatRollRight, direction);
			else
				avatar->doAnim(Animation::combatRollRight, direction);
			return;
		}
	}

	int x, y;
	getMovementFlagAxes(x, y);
	if (x != 0 || y != 0) {
		Direction nextdir = (_avatarAngle >= 0 ? Direction_FromCentidegrees(_avatarAngle) : avatar->getDir());

		if (checkTurn(nextdir, true))
			return;

		Animation::Sequence nextanim = Animation::combatStand;
		if (lastanim == Animation::run && !hasMovementFlags(MOVE_RUN)) {
			// want to go back to combat mode from run
			nextanim = Animation::stopRunningAndDrawWeapon;
		} else if (hasMovementFlags(MOVE_BACK)) {
			nextanim = Animation::retreat;
			nextdir = Direction_Invert(direction);
		}

		if (hasMovementFlags(MOVE_RUN)) {
			// Take a step before running.  Don't clear combat mode in Cruasder
			// - running always finishes with drawing weapon
			nextanim = Animation::run;
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		step(nextanim, nextdir);
		return;
	}

	Animation::Sequence idleanim = avatar->isKneeling() ?
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

	if (hasMovementFlags(MOVE_STEP | MOVE_JUMP) && hasMovementFlags(MOVE_ANY_DIRECTION | MOVE_TURN_LEFT | MOVE_TURN_RIGHT)) {
		// All jump and step movements in crusader are handled identically
		// whether starting from combat mode or not.
		avatar->setInCombat(0);
		handleCombatMode();
		return;
	}

	// Store current idle time. (Also see end of function.)
	uint32 currentIdleTime = _idleTime;
	_idleTime = 0;

	// User toggled combat while in combatRun
	if (avatar->isInCombat()) {
		avatar->clearActorFlag(Actor::ACT_COMBATRUN);
		avatar->toggleInCombat();
	}

	// If Avatar has fallen down and not dead, get up!
	if (standUpIfNeeded(direction))
		return;

	if (!hasMovementFlags(MOVE_ANY_DIRECTION) && lastanim == Animation::run) {
		// if we were running, slow to a walk before stopping
		// (even in stasis)
		waitFor(avatar->doAnim(Animation::stopRunningAndDrawWeapon, direction));
		avatar->setInCombat(0);
		avatar->clearActorFlag(Actor::ACT_COMBATRUN);
		return;
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	bool moving = (lastanim == Animation::step || lastanim == Animation::run || lastanim == Animation::walk);

	DirectionMode dirmode = avatar->animDirMode(Animation::step);

	//  if we are trying to move, allow change direction only after move occurs to avoid spinning
	if (moving || !hasMovementFlags(MOVE_FORWARD | MOVE_BACK)) {
		direction = getTurnDirForTurnFlags(direction, dirmode);
	}

	Animation::Sequence nextanim = Animation::walk;

	if (hasMovementFlags(MOVE_RUN)) {
		if (lastanim == Animation::run
			|| lastanim == Animation::startRun
			|| lastanim == Animation::startRunWithLargeWeapon
			|| lastanim == Animation::walk) {
			// keep running
			nextanim = Animation::run;
			avatar->setActorFlag(Actor::ACT_COMBATRUN);
		} else {
			// start running
			nextanim = Animation::startRun;
			avatar->setActorFlag(Actor::ACT_COMBATRUN);
		}
	}

	if (hasMovementFlags(MOVE_FORWARD)) {
		step(nextanim, direction);
		return;
	}

	if (hasMovementFlags(MOVE_BACK)) {
		avatar->toggleInCombat();
		step(Animation::retreat, direction);
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

	Animation::Result res = avatar->tryAnim(action, direction);
	Animation::Result initialres = res;

	if (res != Animation::SUCCESS) {
		World *world = World::get_instance();
		const CurrentMap *currentmap = world->getCurrentMap();

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
				if (res == Animation::SUCCESS) {
					// move to starting point for real (trigger fast area updates etc)
					avatar->setLocation(origpt.x, origpt.y, origpt.z);
					avatar->move(x, y, z);
					break;
				}
			}
		}

		if (res != Animation::SUCCESS) {
			// reset location and result (in case it's END_OFF_LAND now)
			// couldn't find a better move.
			avatar->setLocation(origpt.x, origpt.y, origpt.z);
			res = initialres;
		}
	}

	if ((action == Animation::step || action == Animation::advance ||
		 action == Animation::retreat || action == Animation::run ||
		 action == Animation::startRunWithLargeWeapon ||
		 action == Animation::startRun || action == Animation::walk)
		&& res == Animation::FAILURE) {
		action = Animation::stand;
	}

	bool moving = (action == Animation::run || action == Animation::walk);

	if (checkTurn(direction, moving))
		return;

	//debug(6, "Cru avatar step: picked action %d dir %d (test result %d)", action, direction, res);
	waitFor(avatar->doAnim(action, direction));
}

void CruAvatarMoverProcess::tryAttack() {
	MainActor *avatar = getMainActor();
	Direction dir = avatar->getDir();
	if (!avatar->isInCombat()) {
		avatar->setInCombat(0);
	}
	// Fire event happens from animation
	Animation::Sequence fireanim = (avatar->isKneeling() ?
									Animation::kneelAndFire : Animation::attack);
	waitFor(avatar->doAnim(fireanim, dir));
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
