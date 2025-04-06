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

#include "ultima/ultima8/world/actors/cru_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/kernel/delay_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CruAvatarMoverProcess)

static const int REBEL_BASE_MAP = 40;

CruAvatarMoverProcess::CruAvatarMoverProcess() : AvatarMoverProcess(),
_avatarAngle(-1), _SGA1Loaded(false), _nextFireTick(0), _lastNPCAlertTick(0) {
}


CruAvatarMoverProcess::~CruAvatarMoverProcess() {
}

static bool _isAnimRunningWalking(Animation::Sequence anim) {
	return (anim == Animation::run || anim == Animation::combatRunSmallWeapon ||
			anim == Animation::walk);
}

void CruAvatarMoverProcess::run() {

	// Even when we are not doing anything (because we're waiting for an anim)
	// we check if the combat angle needs updating - this keeps it smooth.

	const Actor *avatar = getControlledActor();

	// Controlled actor may have gone
	if (!avatar)
		return;

	// When in combat and not running, update the angle.
	// Otherwise, angle is kept as -1 and direction is just actor dir.
	if (avatar->isInCombat() && (avatar->getLastAnim() != Animation::run)) {
		if (_avatarAngle < 0) {
			_avatarAngle = Direction_ToCentidegrees(avatar->getDir());
		}
		if (!hasMovementFlags(MOVE_FORWARD | MOVE_JUMP | MOVE_STEP)) {
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
		// Check for a turn request while running or walking.  This only happens
		// once per arrow keydown, so clear the flag.
		if (_isAnimRunningWalking(avatar->getLastAnim())
			&& hasMovementFlags(MOVE_FORWARD)
			&& (hasMovementFlags(MOVE_TURN_LEFT) || hasMovementFlags(MOVE_TURN_RIGHT) ||
				hasMovementFlags(MOVE_PENDING_TURN_LEFT) || hasMovementFlags(MOVE_PENDING_TURN_RIGHT))) {
			Kernel *kernel = Kernel::get_instance();
			// Stop the current animation and turn now.
			kernel->killProcesses(avatar->getObjId(), ActorAnimProcess::ACTOR_ANIM_PROC_TYPE, true);

			Direction curdir = avatar->getDir();
			Animation::Sequence anim = hasMovementFlags(MOVE_RUN) ? Animation::run : Animation::walk;
			DirectionMode dirmode = avatar->animDirMode(anim);
			Direction dir = getTurnDirForTurnFlags(curdir, dirmode);
			clearMovementFlag(MOVE_TURN_LEFT | MOVE_TURN_RIGHT |
							  MOVE_PENDING_TURN_LEFT | MOVE_PENDING_TURN_RIGHT);
			step(anim, dir);
			return;
		}
	}

	// Pending turns shouldn't stick around.
	clearMovementFlag(MOVE_PENDING_TURN_LEFT | MOVE_PENDING_TURN_RIGHT);

	// Now do the regular process
	AvatarMoverProcess::run();
}

void CruAvatarMoverProcess::clearMovementFlag(uint32 mask) {
	// Set a pending turn if we haven't already cleared the turn
	if ((mask & MOVE_TURN_LEFT) && hasMovementFlags(MOVE_TURN_LEFT))
		setMovementFlag(MOVE_PENDING_TURN_LEFT);
	else if ((mask & MOVE_TURN_RIGHT) && hasMovementFlags(MOVE_TURN_RIGHT))
		setMovementFlag(MOVE_PENDING_TURN_RIGHT);

	AvatarMoverProcess::clearMovementFlag(mask);
}

void CruAvatarMoverProcess::handleHangingMode() {
	// No hanging in crusader, this shouldn't happen?
	assert(false);
}

static bool _isAnimRunningJumping(Animation::Sequence anim) {
	return (anim == Animation::run || anim == Animation::combatRunSmallWeapon ||
			anim == Animation::combatRunLargeWeapon || anim == Animation::jumpForward);
}

static bool _isAnimStartRunning(Animation::Sequence anim) {
	return (anim == Animation::startRun || anim == Animation::startRunSmallWeapon /*||
			// don't test this as it overlaps with kneel :(
			anim == Animation::startRunLargeWeapon*/);
}

bool CruAvatarMoverProcess::checkOneShotMove(Direction direction) {
	Actor *avatar = getControlledActor();
	MainActor *mainactor = dynamic_cast<MainActor *>(avatar);

	static const MovementFlags oneShotFlags[] = {
		MOVE_ROLL_LEFT, MOVE_ROLL_RIGHT,
		MOVE_STEP_LEFT, MOVE_STEP_RIGHT,
		MOVE_STEP_FORWARD, MOVE_STEP_BACK,
		MOVE_SHORT_JUMP, MOVE_TOGGLE_CROUCH
	};

	static const Animation::Sequence oneShotAnims[] = {
		Animation::combatRollLeft, Animation::combatRollRight,
		Animation::slideLeft, Animation::slideRight,
		Animation::advance, Animation::retreat,
		Animation::jumpForward, Animation::kneelStartCru
	};

	static const Animation::Sequence oneShotKneelingAnims[] = {
		Animation::kneelCombatRollLeft, Animation::kneelCombatRollRight,
		Animation::slideLeft, Animation::slideRight,
		Animation::kneelingAdvance, Animation::kneelingRetreat,
		Animation::jumpForward, Animation::kneelEndCru
	};

	for (int i = 0; i < ARRAYSIZE(oneShotFlags); i++) {
		if (hasMovementFlags(oneShotFlags[i])) {
			Animation::Sequence anim = (avatar->isKneeling() ?
							oneShotKneelingAnims[i] : oneShotAnims[i]);

			// All the animations should finish with gun drawn, *except*
			// jump which should finish with gun stowed.  For other cases we should
			// toggle.
			bool incombat = avatar->isInCombat();
			bool isjump = (anim == Animation::jumpForward);
			if (mainactor && (incombat == isjump)) {
				mainactor->toggleInCombat();
			}

			clearMovementFlag(oneShotFlags[i]);

			if (anim == Animation::advance || anim == Animation::retreat ||
				anim == Animation::kneelingAdvance || anim == Animation::kneelingRetreat) {
				step(anim, direction);
			} else {
				avatar->doAnim(anim, direction);
			}
			return true;
		}
	}

	return false;
}

void CruAvatarMoverProcess::handleCombatMode() {
	Actor *avatar = getControlledActor();
	MainActor *mainactor = dynamic_cast<MainActor *>(avatar);
	const Animation::Sequence lastanim = avatar->getLastAnim();
	Direction direction = (_avatarAngle >= 0 ? Direction_FromCentidegrees(_avatarAngle) : avatar->getDir());
	const Direction curdir = avatar->getDir();
	const bool stasis = Ultima8Engine::get_instance()->isAvatarInStasis();

	if (avatar->getMapNum() == REBEL_BASE_MAP) {
		avatar->clearInCombat();
		return;
	}

	// never idle when in combat
	_idleTime = 0;

	if (stasis)
		return;

	if (checkOneShotMove(direction))
		return;

	if (hasMovementFlags(MOVE_FORWARD)) {
		Animation::Sequence nextanim;
		if (hasMovementFlags(MOVE_STEP)) {
			nextanim = avatar->isKneeling() ?
							Animation::kneelingAdvance : Animation::advance;
		} else if (hasMovementFlags(MOVE_RUN) && avatar->hasAnim(Animation::combatRunSmallWeapon)) {
			// Take a step before running
			if (lastanim == Animation::walk || _isAnimRunningJumping(lastanim) || _isAnimStartRunning(lastanim))
				nextanim = Animation::combatRunSmallWeapon;
			else
				nextanim = Animation::startRunSmallWeapon;
		} else if (hasMovementFlags(MOVE_JUMP) && avatar->hasAnim(Animation::jumpForward)) {
			if (lastanim == Animation::walk || lastanim == Animation::run || lastanim == Animation::combatRunSmallWeapon)
				nextanim = Animation::jumpForward;
			else
				nextanim = Animation::jump;
			// Jump always ends out of combat
			avatar->clearInCombat();
		} else if (avatar->isKneeling()) {
			avatar->doAnim(Animation::kneelEndCru, direction);
			avatar->clearActorFlag(Actor::ACT_KNEELING);
			return;
		} else {
			// moving forward from combat stows weapon
			nextanim = Animation::walk;
			if (mainactor)
				mainactor->toggleInCombat();
		}

		// Ensure the dir we are about to use is valid
		if (avatar->animDirMode(nextanim) == dirmode_8dirs)
			direction = static_cast<Direction>(direction - (static_cast<uint32>(direction) % 2));

		// don't check weapon here, Avatar can go straight from drawn-weapon to
		// walking forward.
		step(nextanim, direction);
		return;
	} else if (hasMovementFlags(MOVE_BACK)) {
		Animation::Sequence nextanim;
		if (hasMovementFlags(MOVE_JUMP)) {
			if (!avatar->isKneeling() && avatar->hasAnim(Animation::kneelStartCru)) {
				nextanim = Animation::kneelStartCru;
				avatar->setActorFlag(Actor::ACT_KNEELING);
			} else {
				// Do nothing if already kneeling
				return;
			}
		} else {
			nextanim = Animation::retreat;
		}
		step(nextanim, direction);
		return;
	} else if (hasMovementFlags(MOVE_STEP)) {
		if (avatar->isKneeling()) {
			avatar->doAnim(Animation::kneelEndCru, direction);
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
				avatar->doAnim(Animation::kneelCombatRollLeft, direction);
			else
				avatar->doAnim(Animation::combatRollLeft, direction);
			return;
		} else if (hasMovementFlags(MOVE_TURN_RIGHT)) {
			if (avatar->isKneeling())
				avatar->doAnim(Animation::kneelCombatRollRight, direction);
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
		if ((lastanim == Animation::run || lastanim == Animation::combatRunSmallWeapon) && !hasMovementFlags(MOVE_RUN)) {
			// want to go back to combat mode  from run
			nextanim = Animation::stopRunningAndDrawSmallWeapon;
		} else if (hasMovementFlags(MOVE_BACK)) {
			nextanim = Animation::retreat;
			nextdir = Direction_Invert(direction);
		}

		if (hasMovementFlags(MOVE_RUN)) {
			nextanim = Animation::combatRunSmallWeapon;
		}

		Animation::Sequence wpnanim = Animation::checkWeapon(nextanim, lastanim);
		step(wpnanim, nextdir);
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

	if (hasMovementFlags(MOVE_ATTACKING) && !hasMovementFlags(MOVE_FORWARD | MOVE_BACK)) {
		tryAttack();
		return;
	}

	if (_isAnimRunningJumping(lastanim) || _isAnimStartRunning(idleanim)) {
		idleanim = Animation::stopRunningAndDrawSmallWeapon;
	}

	// Not doing anything in particular? stand.
	if (lastanim != idleanim) {
		Animation::Sequence nextanim = Animation::checkWeapon(idleanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
	}
}

void CruAvatarMoverProcess::handleNormalMode() {
	Actor *avatar = getControlledActor();
	MainActor *mainactor = dynamic_cast<MainActor *>(avatar);
	const Animation::Sequence lastanim = avatar->getLastAnim();
	Direction direction = avatar->getDir();
	const bool stasis = Ultima8Engine::get_instance()->isAvatarInStasis();
	const bool rebelBase = (avatar->getMapNum() == REBEL_BASE_MAP);

	if (!rebelBase && hasMovementFlags(MOVE_STEP | MOVE_JUMP) && hasMovementFlags(MOVE_ANY_DIRECTION | MOVE_TURN_LEFT | MOVE_TURN_RIGHT)) {
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
		if (mainactor)
			mainactor->toggleInCombat();
	}

	if (!hasMovementFlags(MOVE_ANY_DIRECTION) && lastanim == Animation::run) {
		// if we were running, slow to a walk before stopping
		// (even in stasis)
		Animation::Sequence nextanim;
		if (rebelBase) {
			nextanim = Animation::stand;
		} else {
			nextanim = Animation::stopRunningAndDrawSmallWeapon;
			// Robots don't slow down from  running
			if (!avatar->hasAnim(nextanim))
				nextanim = Animation::stand;
		}
		waitFor(avatar->doAnim(nextanim, direction));
		avatar->setInCombat(0);
		return;
	}

	// can't do any new actions if in stasis
	if (stasis)
		return;

	if (checkOneShotMove(direction))
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
			|| lastanim == Animation::startRunSmallWeapon
			|| lastanim == Animation::combatRunSmallWeapon
			|| lastanim == Animation::walk) {
			// keep running
			nextanim = Animation::run;
		} else {
			// start running
			nextanim = Animation::startRun;
		}
	}

	if (hasMovementFlags(MOVE_FORWARD)) {
		step(nextanim, direction);
		return;
	}

	if (!rebelBase && hasMovementFlags(MOVE_BACK)) {
		if (mainactor)
			mainactor->toggleInCombat();
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
	if (avatar->isBusy())
		return;

	if (hasMovementFlags(MOVE_ATTACKING) && !hasMovementFlags(MOVE_FORWARD | MOVE_BACK)) {
		tryAttack();
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
	Actor *avatar = getControlledActor();

	// For "start run" animations, don't call it a success unless we can actually run
	Animation::Sequence testaction = _isAnimStartRunning(action) ? Animation::run : action;

	Animation::Result res = avatar->tryAnim(testaction, direction);
	Animation::Result initialres = res;

	if (res != Animation::SUCCESS) {
		World *world = World::get_instance();
		const CurrentMap *currentmap = world->getCurrentMap();

		// Search right/left gradually increasing distance to see if we can make the move work.

		Direction dir_right = Direction_TurnByDelta(direction, 4, dirmode_16dirs);
		Direction dir_left = Direction_TurnByDelta(direction, -4, dirmode_16dirs);
		Point3 origpt = avatar->getLocation();

		int32 dims[3];
		avatar->getFootpadWorld(dims[0], dims[1], dims[2]);

		// Double the values in original to match our coordinate space
		static const int ADJUSTMENTS[] = {0x20, 0x20, 0x40, 0x40, 0x60, 0x60,
			0x80, 0x80, 0xA0, 0xA0};

		for (int i = 0; i < ARRAYSIZE(ADJUSTMENTS); i++) {
			Direction testdir = (i % 2 ? dir_left : dir_right);
			int32 x = origpt.x + Direction_XFactor(testdir) * ADJUSTMENTS[i];
			int32 y = origpt.y + Direction_YFactor(testdir) * ADJUSTMENTS[i];
			int32 z = origpt.z;

			//
			// Check if we can slide from the original point to a different
			// start point (otherwise we might pop through walls, lasers, etc).
			// This is like Item::collideMove, but we want to stop on any blockers
			// and not trigger any events
			//
			bool startvalid = true;
			Std::list<CurrentMap::SweepItem> collisions;
			Point3 end(x, y, z);
			avatar->setLocation(origpt);
			currentmap->sweepTest(origpt, end, dims, avatar->getShapeInfo()->_flags,
								  avatar->getObjId(), true, &collisions);
			for (const auto &collision : collisions) {
				if (!collision._touching && collision._blocking) {
					startvalid = false;
					break;
				}
			}

			if (startvalid) {
				avatar->setLocation(x, y, z);
				res = avatar->tryAnim(testaction, direction);
				if (res == Animation::SUCCESS) {
					// move to starting point for real (trigger fast area updates etc)
					avatar->setLocation(origpt);
					avatar->move(x, y, z);
					break;
				}
			}
		}

		if (res != Animation::SUCCESS) {
			// reset location and result (in case it's END_OFF_LAND now)
			// couldn't find a better move.
			avatar->setLocation(origpt);
			res = initialres;
		}
	}

	if ((action == Animation::step || action == Animation::run ||
		 action == Animation::startRun || action == Animation::walk)
		&& res == Animation::FAILURE) {
		action = Animation::stand;
	}
	else if ((action == Animation::advance || action == Animation::retreat ||
		 action == Animation::combatRunSmallWeapon ||
		 action == Animation::startRunSmallWeapon)
		&& res == Animation::FAILURE) {
		action = Animation::combatStand;
	}

	bool moving = _isAnimRunningWalking(action);

	if (checkTurn(direction, moving))
		return;

	//debug(6, "Cru avatar step: picked action %d dir %d (test result %d)", action, direction, res);
	avatar->doAnim(action, direction);
}

void CruAvatarMoverProcess::tryAttack() {
	// Don't do it while this process is waiting
	if (is_suspended())
		return;

	Actor *avatar = getControlledActor();
	if (!avatar || avatar->getMapNum() == REBEL_BASE_MAP || avatar->isBusy())
		return;

	Item *wpn = getItem(avatar->getActiveWeapon());
	if (!wpn || !wpn->getShapeInfo() || !wpn->getShapeInfo()->_weaponInfo)
		return;

	Kernel *kernel = Kernel::get_instance();
	if (kernel->getTickNum() < _nextFireTick)
		return;

	if (!avatar->isInCombat()) {
		avatar->setInCombat(0);
	}

	AudioProcess *audio = AudioProcess::get_instance();
	const WeaponInfo *wpninfo = wpn->getShapeInfo()->_weaponInfo;

	if (avatar->getObjId() != kMainActorId) {
		// Non-avatar NPCs never need to reload or run out of energy.
		Animation::Sequence fireanim = (avatar->isKneeling() ?
										Animation::kneelAndFire : Animation::attack);
		waitFor(avatar->doAnim(fireanim, avatar->getDir()));
		return;
	}

	int shotsleft;
	if (wpninfo->_ammoShape) {
		shotsleft = wpn->getQuality();
	} else if (wpninfo->_energyUse) {
		shotsleft = avatar->getMana() / wpninfo->_energyUse;
	} else {
		shotsleft = 1;
	}

	if (!shotsleft) {
		Item *ammo = avatar->getFirstItemWithShape(wpninfo->_ammoShape, true);
		if (ammo) {
			// reload now
			// SGA1 is special, it reloads every shot.
			if (wpn->getShape() == 0x332)
				_SGA1Loaded = true;

			wpn->setQuality(wpninfo->_clipSize);
			ammo->setQuality(ammo->getQuality() - 1);
			if (ammo->getQuality() == 0)
				ammo->destroy();

			if (wpninfo->_reloadSound) {
				audio->playSFX(0x2a, 0x80, avatar->getObjId(), 1);
			}
			if (avatar->getObjId() == kMainActorId && !avatar->isKneeling()) {
				avatar->doAnim(Animation::reloadSmallWeapon, dir_current);
			}

			_nextFireTick = kernel->getTickNum() + 15;
		} else {
			// no shots left
			audio->playSFX(0x2a, 0x80, avatar->getObjId(), 1);
			_nextFireTick = kernel->getTickNum() + 20;
		}
	} else {
		// Check for SGA1 reload anim (which happens every shot)
		if (wpn->getShape() == 0x332 && !avatar->isKneeling() && !_SGA1Loaded) {
			if (wpninfo->_reloadSound) {
				audio->playSFX(0x2a, 0x80, avatar->getObjId(), 1);
			}
			if (avatar->getObjId() == kMainActorId) {
				avatar->doAnim(Animation::reloadSmallWeapon, dir_current);
			}
			_SGA1Loaded = true;
		} else {
			Direction dir = avatar->getDir();
			// Fire event happens from animation
			Animation::Sequence fireanim = (avatar->isKneeling() ?
											Animation::kneelAndFire : Animation::attack);
			uint16 fireanimpid = avatar->doAnim(fireanim, dir);

			if (wpn->getShape() == 0x332)
				_SGA1Loaded = false;

			// Use a shot up
			if (wpninfo->_ammoShape) {
				wpn->setQuality(shotsleft - 1);
			} else if (wpninfo->_energyUse) {
				avatar->setMana(avatar->getMana() - wpninfo->_energyUse);
			}

			// Check if we should alert nearby NPCs
			checkForAlertingNPCs();

			if (wpninfo->_shotDelay) {
				_nextFireTick = kernel->getTickNum() + wpninfo->_shotDelay;
			} else {
				waitFor(fireanimpid);
			}
		}
	}
}

void CruAvatarMoverProcess::checkForAlertingNPCs() {
	uint32 nowtick = Kernel::get_instance()->getTickNum();
	if (nowtick - _lastNPCAlertTick < 240)
		return;

	_lastNPCAlertTick = nowtick;
	uint16 controllednpc = World::get_instance()->getControlledNPCNum();
	for (int i = 2; i < 256; i++) {
		if (i == controllednpc)
			continue;

		Actor *a = getActor(i);
		if (!a || a->isDead() || !a->isOnScreen())
			continue;

		if (!a->isInCombat()) {
			uint16 currentactivity = a->getCurrentActivityNo();
			uint16 activity2 = a->getDefaultActivity(2);
			if (currentactivity == activity2) {
				// note: original game also seems to check surrendering flag here?
				if (currentactivity == 8) {
					// Was guarding, attack!
					a->setActivity(5);
				}
			} else {
				uint16 range = 0;
				uint32 npcshape = a->getShape();
				if (npcshape == 0x2f5 || npcshape == 0x2f6 || npcshape == 0x2f7 ||
					(GAME_IS_REMORSE && (npcshape == 0x595 || npcshape == 0x597)) ||
					(GAME_IS_REGRET && (npcshape == 0x344 || npcshape == 0x384))) {
					Actor *c = getActor(controllednpc);
					if (c)
						range = a->getRangeIfVisible(*c);
				} else {
					range = 1;
				}
				if (range) {
					a->setActivity(a->getDefaultActivity(2));
				}
			}
		} else {
			// Was guarding, attack!
			a->setActivity(5);
		}
	}
}

void CruAvatarMoverProcess::saveData(Common::WriteStream *ws) {
	AvatarMoverProcess::saveData(ws);
	ws->writeSint32LE(_avatarAngle);
	ws->writeByte(_SGA1Loaded ? 1 : 0);

	// We don't bother saving _lastNPCAlertTick or _nextFireTick, they both
	// will get reset to 0 which will behave almost identically in practice.
}

bool CruAvatarMoverProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!AvatarMoverProcess::loadData(rs, version)) return false;
	_avatarAngle = rs->readSint32LE();
	_SGA1Loaded = (rs->readByte() != 0);
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
