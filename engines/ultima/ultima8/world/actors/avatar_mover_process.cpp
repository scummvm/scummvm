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

#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/targeted_anim_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/misc/direction_util.h"

namespace Ultima {
namespace Ultima8 {

AvatarMoverProcess::AvatarMoverProcess() : Process(),
		_lastAttack(0), _idleTime(0), _movementFlags(0) {
	_type = 1; // CONSTANT! (type 1 = persistent)
}


AvatarMoverProcess::~AvatarMoverProcess() {
}

void AvatarMoverProcess::run() {
	Actor *avatar = getControlledActor();
	assert(avatar);

	// busy, so don't move
	if (avatar->isBusy()) {
		_idleTime = 0;
		return;
	}

	if (avatar->getLastAnim() == Animation::hang) {
		handleHangingMode();
		return;
	}

	// falling, so don't move
	if (avatar->getGravityPID() != 0) {
		Process *proc = Kernel::get_instance()->getProcess(avatar->getGravityPID());
		if (!proc || !proc->is_active()) {
			warning("FIXME: Removing stale gravity pid %d from Avatar.", avatar->getGravityPID());
			avatar->setGravityPID(0);
		} else {
			_idleTime = 0;
			return;
		}
	}

	// not in fast area, don't move (can happen for some death sequences
	// in Crusader)
	if (!avatar->hasFlags(Item::FLG_FASTAREA))
		return;

	bool combatRun = avatar->hasActorFlags(Actor::ACT_COMBATRUN);
	if (avatar->isInCombat() && !combatRun)
		handleCombatMode();
	else
		handleNormalMode();
}


bool AvatarMoverProcess::checkTurn(Direction direction, bool moving) {
	Actor *avatar = getControlledActor();
	Direction curdir = avatar->getDir();
	bool combat = avatar->isInCombat() && !avatar->hasActorFlags(Actor::ACT_COMBATRUN);

	// Note: don't need to turn if moving backward in combat stance
	// CHECKME: currently, first turn in the right direction
	if (direction != curdir && !(combat && Direction_Invert(direction) == curdir)) {
		Animation::Sequence lastanim = avatar->getLastAnim();

		if (moving &&
				(lastanim == Animation::walk || lastanim == Animation::run ||
				 lastanim == Animation::combatStand ||
				 (GAME_IS_CRUSADER && (lastanim == Animation::startRunSmallWeapon ||
				 lastanim == Animation::combatRunSmallWeapon))) &&
				(ABS(direction - curdir) + 2) % 16 <= 4) {
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

void AvatarMoverProcess::turnToDirection(Direction direction) {
	Actor *avatar = getControlledActor();
	uint16 turnpid = avatar->turnTowardDir(direction);
	if (turnpid)
		waitFor(turnpid);
}

void AvatarMoverProcess::slowFromRun(Direction direction) {
	Actor *avatar = getControlledActor();
	ProcId walkpid = avatar->doAnim(Animation::walk, direction);
	ProcId standpid = avatar->doAnimAfter(Animation::stand, direction, walkpid);
	waitFor(standpid);
}

void AvatarMoverProcess::putAwayWeapon(Direction direction) {
	Actor *avatar = getControlledActor();
	ProcId anim1 = avatar->doAnim(Animation::unreadyWeapon, direction);
	ProcId anim2 = avatar->doAnimAfter(Animation::stand, direction, anim1);
	waitFor(anim2);
}

bool AvatarMoverProcess::standUpIfNeeded(Direction direction) {
	Actor *avatar = getControlledActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	bool stasis = Ultima8Engine::get_instance()->isAvatarInStasis();

	if (lastanim == Animation::die || lastanim == Animation::fallBackwards) {
		if (!stasis) {
			waitFor(avatar->doAnim(Animation::standUp, direction));
		}
		return true;
	}
	return false;
}

void AvatarMoverProcess::getMovementFlagAxes(int &x, int &y) {
	y = 0;
	x = 0;
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
}

Direction AvatarMoverProcess::getTurnDirForTurnFlags(Direction direction, DirectionMode dirmode) {
	if (hasMovementFlags(MOVE_TURN_LEFT | MOVE_PENDING_TURN_LEFT)) {
		direction = Direction_OneLeft(direction, dirmode);
	}

	if (hasMovementFlags(MOVE_TURN_RIGHT | MOVE_PENDING_TURN_RIGHT)) {
		direction = Direction_OneRight(direction, dirmode);
	}
	return direction;
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
}

bool AvatarMoverProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_lastAttack = rs->readUint32LE();
	_idleTime = rs->readUint32LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
