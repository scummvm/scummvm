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

#include "harvester/player/player.h"
#include "harvester/player/player_movement.h"
#include "harvester/combat/combat.h"

#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"

namespace Harvester {

using namespace PlayerMovement;

namespace {

static const int kRoomPlayerHitAnimationRate = 4;
static const int kNativePlayerDeathEntryAnimationRate = 14;
static const int kPlayerHitKnockbackDistance = 18;
static const int kPlayerHitKnockbackDecayStep = 3;

struct PlayerDeathAnimationRange {
	PlayerDeathAnimationRange() {}
	PlayerDeathAnimationRange(int firstFrame, int lastFrame)
		: firstFrame(firstFrame), lastFrame(lastFrame) {}

	int firstFrame = -1;
	int lastFrame = -1;
};

struct PlayerHitAnimationRange {
	PlayerHitAnimationRange() {}
	PlayerHitAnimationRange(int firstFrame, int lastFrame, int resumeFacing, int knockbackX)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing), knockbackX(knockbackX) {}

	int firstFrame = -1;
	int lastFrame = -1;
	int resumeFacing = -1;
	int knockbackX = 0;
};

static const char *const kPlayerHitSoundPaths[] = {
	"1:/sound/effects/PC0_hit0.wav",
	"1:/sound/effects/PC0_hit1.wav",
	"1:/sound/effects/PC0_hit2.wav"
};

static bool playPlayerHitSound(HarvesterEngine &engine) {
	const uint soundIndex = ARRAYSIZE(kPlayerHitSoundPaths) > 1
		? engine.getRandomNumber(ARRAYSIZE(kPlayerHitSoundPaths) - 1)
		: 0;
	return engine.playSound(kPlayerHitSoundPaths[soundIndex]);
}

static bool choosePlayerDeathAnimationRange(const Entity &entity, int firstFrame, int lastFrame,
		PlayerDeathAnimationRange &range) {
	if (!runtimeEntityHasFrameRange(entity, firstFrame, lastFrame))
		return false;

	range = PlayerDeathAnimationRange(firstFrame, lastFrame);
	return true;
}

static bool resolvePlayerDeathAnimationRange(const RoomPlayerState &playerState,
		int damageType, bool goreEnabled, PlayerDeathAnimationRange &range) {
	if (!playerState.entity)
		return false;

	enum PlayerDeathBankFamily {
		kPlayerDeathBankHorizontalRight,
		kPlayerDeathBankHorizontalLeft,
		kPlayerDeathBankVerticalDown,
		kPlayerDeathBankVerticalUp
	};

	PlayerDeathBankFamily family = kPlayerDeathBankHorizontalLeft;
	if (playerState.attackActive) {
		if (playerState.attackFirstFrame >= 0x6e && playerState.attackFirstFrame <= 0x8b)
			family = kPlayerDeathBankHorizontalRight;
		else
			family = kPlayerDeathBankHorizontalLeft;
	} else if (playerState.turnActive) {
		if (playerState.turnFirstFrame >= 0x28)
			family = kPlayerDeathBankHorizontalRight;
		else
			family = kPlayerDeathBankHorizontalLeft;
	} else {
		switch (playerState.facing) {
		case 0:
			family = kPlayerDeathBankVerticalDown;
			break;
		case 2:
			family = kPlayerDeathBankHorizontalRight;
			break;
		case 3:
			family = kPlayerDeathBankVerticalUp;
			break;
		case 1:
		default:
			family = kPlayerDeathBankHorizontalLeft;
			break;
		}
	}

	const auto selectByFamily = [&](int bludgeFirst, int bludgeLast, int slashFirst, int slashLast,
			int projectileFirst, int projectileLast) {
		if ((!goreEnabled || damageType == 1) &&
				choosePlayerDeathAnimationRange(*playerState.entity, bludgeFirst, bludgeLast, range))
			return true;
		if (damageType == 4 &&
				choosePlayerDeathAnimationRange(*playerState.entity, projectileFirst, projectileLast, range))
			return true;
		if (damageType == 2 &&
				choosePlayerDeathAnimationRange(*playerState.entity, slashFirst, slashLast, range))
			return true;

		return choosePlayerDeathAnimationRange(*playerState.entity, bludgeFirst, bludgeLast, range) ||
			choosePlayerDeathAnimationRange(*playerState.entity, slashFirst, slashLast, range) ||
			choosePlayerDeathAnimationRange(*playerState.entity, projectileFirst, projectileLast, range);
	};

	switch (family) {
	case kPlayerDeathBankHorizontalRight:
		return selectByFamily(0xb0, 0xb9, 0x100, 0x109, 0x114, 0x11d);
	case kPlayerDeathBankVerticalDown:
		return selectByFamily(0xc4, 0xcd, 0xd8, 0xe1, 0xec, 0xf5);
	case kPlayerDeathBankVerticalUp:
		return selectByFamily(0xce, 0xd7, 0xe2, 0xeb, 0xf6, 0xff);
	case kPlayerDeathBankHorizontalLeft:
	default:
		return selectByFamily(0xba, 0xc3, 0x10a, 0x113, 0x11e, 0x127);
	}
}

static bool resolvePlayerHitAnimationRange(const Entity &entity,
		int monsterAttackFirstFrame, PlayerHitAnimationRange &range) {
	PlayerHitAnimationRange candidate;
	switch (monsterAttackFirstFrame) {
	case 0x64:
		candidate = PlayerHitAnimationRange(0x92, 0x94, 2, -kPlayerHitKnockbackDistance);
		break;
	case 0x5a:
		candidate = PlayerHitAnimationRange(0x8f, 0x91, 2, -kPlayerHitKnockbackDistance);
		break;
	case 0x50:
		candidate = PlayerHitAnimationRange(0x8c, 0x8e, 2, -kPlayerHitKnockbackDistance);
		break;
	case 0x82:
		candidate = PlayerHitAnimationRange(0x9b, 0x9d, 1, kPlayerHitKnockbackDistance);
		break;
	case 0x78:
		candidate = PlayerHitAnimationRange(0x98, 0x9a, 1, kPlayerHitKnockbackDistance);
		break;
	case 0x6e:
		candidate = PlayerHitAnimationRange(0x95, 0x97, 1, kPlayerHitKnockbackDistance);
		break;
	default:
		return false;
	}

	if (!runtimeEntityHasFrameRange(entity, candidate.firstFrame, candidate.lastFrame))
		return false;

	range = candidate;
	return true;
}

} // End of anonymous namespace

bool Player::startHitAnimation(HarvesterEngine &engine, int monsterAttackFirstFrame) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity || playerState.deathActive)
		return false;

	PlayerHitAnimationRange range;
	if (!resolvePlayerHitAnimationRange(*playerState.entity, monsterAttackFirstFrame, range))
		return false;

	playerState.hasMoveTarget = false;
	playerState.evasiveState = -1;
	playerState.evasiveXStep = 0;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.turnFirstFrame = -1;
	playerState.turnLastFrame = -1;
	playerState.turnEndFrame = -1;
	playerState.turnPlayBackwards = false;
	playerState.attackActive = false;
	playerState.attackFirstFrame = -1;
	playerState.attackLastFrame = -1;
	playerState.attackContactFrame = -1;
	playerState.attackResumeFacing = -1;
	playerState.attackSoundPlayed = false;
	playerState.attackSoundPlaybackFrame = -1;
	playerState.attackContactResolved = false;
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	playerState.nextMovementTick = 0;
	playerState.hitActive = true;
	playerState.hitFirstFrame = range.firstFrame;
	playerState.hitLastFrame = range.lastFrame;
	playerState.hitResumeFacing = range.resumeFacing;
	playerState.hitKnockbackRemainingX = range.knockbackX;
	playerState.hitKnockbackDecayStep = kPlayerHitKnockbackDecayStep;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setAnimationRate(kRoomPlayerHitAnimationRate);
	playerState.entity->setCurrentFrame(range.firstFrame);
	playerState.entity->setAnimationEnabled(true);
	playerState.entity->setVisible(true);
	(void)playPlayerHitSound(engine);
	debugC(1, kDebugCombat,
		"Harvester: player hit animation attack_frame=%d frames=%d..%d resume_facing=%d knockback=%d rate=%d",
		monsterAttackFirstFrame, range.firstFrame, range.lastFrame, range.resumeFacing,
		range.knockbackX, kRoomPlayerHitAnimationRate);
	return true;
}

bool Player::updateHitAnimationState(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Array<AnimRecord> &sceneAnimations) {
	RoomPlayerState &playerState = _state;
	if (!playerState.hitActive || !playerState.entity)
		return false;

	const MovementContext movementContext = { engine, state, sceneObjects, sceneAnimations, playerState };
	bool changed = false;
	if (playerState.hitKnockbackRemainingX != 0) {
		const int step = MAX(1, ABS(playerState.hitKnockbackDecayStep));
		const int knockbackStep = playerState.hitKnockbackRemainingX > 0
			? MIN(playerState.hitKnockbackRemainingX, step)
			: MAX(playerState.hitKnockbackRemainingX, -step);
		if (knockbackStep != 0) {
			if (!tryApplyPlayerMovement(movementContext,
					{ playerState.centerX + knockbackStep, playerState.bottomY, playerState.z })) {
				playerState.hitKnockbackRemainingX = 0;
			} else {
				changed = true;
				playerState.hitKnockbackRemainingX -= knockbackStep;
			}
		}
	}

	if (playerState.entity->getCurrentFrame() != playerState.hitLastFrame)
		return changed;

	const int resumeFacing = playerState.hitResumeFacing >= 0
		? playerState.hitResumeFacing
		: (playerState.facing >= 0 ? playerState.facing : 0);
	playerState.hitActive = false;
	playerState.hitFirstFrame = -1;
	playerState.hitLastFrame = -1;
	playerState.hitResumeFacing = -1;
	playerState.hitKnockbackRemainingX = 0;
	playerState.hitKnockbackDecayStep = 0;
	return setIdleAnimation(resumeFacing) || changed;
}

bool Player::startDeathAnimation(int damageType, bool goreEnabled) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity || playerState.deathActive)
		return false;

	PlayerDeathAnimationRange range;
	if (!resolvePlayerDeathAnimationRange(playerState, damageType, goreEnabled, range))
		return false;

	playerState.hasMoveTarget = false;
	playerState.evasiveState = -1;
	playerState.evasiveXStep = 0;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.turnFirstFrame = -1;
	playerState.turnLastFrame = -1;
	playerState.turnEndFrame = -1;
	playerState.turnPlayBackwards = false;
	playerState.attackActive = false;
	playerState.attackFirstFrame = -1;
	playerState.attackLastFrame = -1;
	playerState.attackContactFrame = -1;
	playerState.attackResumeFacing = -1;
	playerState.attackSoundPlayed = false;
	playerState.attackSoundPlaybackFrame = -1;
	playerState.attackContactResolved = false;
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	playerState.hitActive = false;
	playerState.hitFirstFrame = -1;
	playerState.hitLastFrame = -1;
	playerState.hitResumeFacing = -1;
	playerState.hitKnockbackRemainingX = 0;
	playerState.hitKnockbackDecayStep = 0;
	playerState.nextMovementTick = 0;
	playerState.deathActive = true;
	playerState.deathFirstFrame = range.firstFrame;
	playerState.deathLastFrame = range.lastFrame;
	playerState.deathDamageType = damageType;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setAnimationRate(kNativePlayerDeathEntryAnimationRate);
	playerState.entity->setCurrentFrame(range.firstFrame);
	playerState.entity->setVisible(true);
	debugC(2, kDebugCombat,
		"Harvester: player death animation start damage_type=%d gore=%d facing=%d frames=%d..%d",
		damageType, goreEnabled, playerState.facing, range.firstFrame, range.lastFrame);
	debugC(3, kDebugCombat,
		"Harvester: player death animation timing frames=%d..%d entry_rate=%d entry_interval_ticks=%d running_rate=%d running_interval_ticks=%d",
		range.firstFrame, range.lastFrame,
		kNativePlayerDeathEntryAnimationRate, 100 / kNativePlayerDeathEntryAnimationRate,
		playerState.animationRate, 100 / playerState.animationRate);
	return true;
}

bool Player::updateDeathAnimationState() {
	RoomPlayerState &playerState = _state;
	if (!playerState.deathActive || !playerState.entity)
		return false;

	const int currentFrame = playerState.entity->getCurrentFrame();
	if (currentFrame > playerState.deathFirstFrame &&
			currentFrame < playerState.deathLastFrame &&
			playerState.entity->getAnimationRate() != playerState.animationRate) {
		playerState.entity->setAnimationRate(playerState.animationRate);
		debugC(3, kDebugCombat,
			"Harvester: player death animation running frame=%d rate=%d interval_ticks=%d",
			currentFrame, playerState.animationRate,
			100 / playerState.animationRate);
	}

	if (currentFrame < playerState.deathLastFrame)
		return false;

	playerState.deathActive = false;
	debugC(2, kDebugCombat,
		"Harvester: player death animation complete damage_type=%d frame=%d",
		playerState.deathDamageType, currentFrame);
	return true;
}

} // End of namespace Harvester
