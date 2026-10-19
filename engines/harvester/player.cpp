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

#include "harvester/player.h"
#include "harvester/player_movement.h"
#include "harvester/combat.h"

#include "common/debug.h"
#include "common/system.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/entity.h"

namespace Harvester {

using namespace PlayerMovement;

namespace {

static bool setPlayerIdleAnimation(RoomPlayerState &playerState, int facing);
static void updatePlayerIdleTrigger(RoomIdleAnimationState &idleState);

static const char *const kPlayerActorResourcePath = "1:/GRAPHIC/MONSTERS/PC/PC0.ABM";
static const int kRoomPlayerWalkAnimationRate = 17;
static const int kRoomPlayerFastAnimationRate = 30;
static const int kRoomPlayerHitAnimationRate = 4;
static const int kNativePlayerDeathEntryAnimationRate = 14;
static const float kRoomPlayerAttackUpperYOffset = 144.44f;
static const float kRoomPlayerAttackMidYOffset = 75.36f;
static const char *const kPlayerIdleAnimationEntityName = "IDLE_ANIM";
static const char *const kPlayerIdleAnimationResourcePath = "1:/GRAPHIC/ROOMANIM/PCLOUN02.ABM";
static const uint32 kRoomPlayerIdleDelayTicks = 3000;
static const int kPlayerHitKnockbackDistance = 18;
static const int kPlayerHitKnockbackDecayStep = 3;
static const int kRoomPlayerIdleAnimationRate = 14;
static const int kRoomPlayerIdleLoopStartFrame = 0x0f;
static const int kRoomPlayerIdleLoopLastFrame = 0xb2;
static const int kRoomPlayerIdleExitLastFrame = 0xbf;
static const int kRoomPlayerIdleYOffset = 4;

struct PlayerTurnAnimationRange {
	PlayerTurnAnimationRange() {}
	PlayerTurnAnimationRange(int firstFrame, int lastFrame, bool playBackwards)
		: firstFrame(firstFrame), lastFrame(lastFrame), playBackwards(playBackwards) {}

	int firstFrame = 0;
	int lastFrame = 0;
	bool playBackwards = false;
};

struct PlayerAttackAnimationRange {
	PlayerAttackAnimationRange() {}
	PlayerAttackAnimationRange(int firstFrame, int lastFrame, int resumeFacing)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing) {}

	int firstFrame = 0;
	int lastFrame = 0;
	int resumeFacing = 0;
};

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

struct PlayerAttackSoundSet {
	uint soundCount;
	const char *soundPaths[3];
};

struct PlayerCombatLoadoutTuning {
	const char *label;
	int damageType;
	int damageAmount;
	int contactFrameOffset;
};

static const int kPlayerAttackSoundTriggerFrameOffset = 1;
static const PlayerAttackSoundSet kPlayerAttackSoundSets[] = {
	{ 3, { "2:/sound/effects/swoosh1.wav", "2:/sound/effects/swoosh2.wav", "2:/sound/effects/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 1, { "2:/SOUND/EFFECTS/NAILGUN1.wav", nullptr, nullptr } },
	{ 1, { "1:/sound/effects/shotgun2.wav", nullptr, nullptr } },
	{ 1, { "2:/SOUND/EFFECTS/gunshot.wav", nullptr, nullptr } },
	{ 1, { "2:/SOUND/EFFECTS/gunshot.wav", nullptr, nullptr } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/sound/effects/swoosh1.wav", "2:/sound/effects/swoosh2.wav", "2:/sound/effects/swoosh3.wav" } },
	{ 1, { "1:/sound/effects/CLN_atk0.wav", nullptr, nullptr } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh3.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } }
};

static const char *const kPlayerHitSoundPaths[] = {
	"1:/sound/effects/PC0_hit0.wav",
	"1:/sound/effects/PC0_hit1.wav",
	"1:/sound/effects/PC0_hit2.wav"
};

static const PlayerCombatLoadoutTuning kPlayerCombatLoadoutTunings[] = {
	{ "FISTS", 1, 1, 5 },
	{ "CLEAVER", 2, 4, 2 },
	{ "NAILGUN", 4, 2, 2 },
	{ "SHOTGUN", 4, 7, 2 },
	{ "9GUN", 4, 6, 2 },
	{ "38GUN", 4, 6, 2 },
	{ "TOMAHAWK", 2, 3, 2 },
	{ "KNIFE", 2, 3, 2 },
	{ "FLAIL", 2, 3, 2 },
	{ "HANDAXE", 2, 3, 2 },
	{ "WRENCH", 1, 3, 2 },
	{ "PITCHFORK", 2, 3, 2 },
	{ "SCYTHE", 2, 4, 2 },
	{ "SWORD", 2, 5, 2 },
	{ "CHAINSAW", 2, 8, 2 },
	{ "HARVEST_BLADE", 2, 3, 2 },
	{ "SHOVEL", 1, 3, 2 },
	{ "FIREAXE", 2, 3, 2 },
	{ "BAT", 1, 3, 2 },
	{ "RAZOR", 2, 3, 2 },
	{ "POOLSTICK", 1, 2, 2 }
};

static bool resolvePlayerTurnAnimationRange(int fromFacing, int toFacing, PlayerTurnAnimationRange &range) {
	// Native desired-state transitions play these banks forward or backward depending on the source facing.
	if (fromFacing == 0 && toFacing == 1) {
		range = PlayerTurnAnimationRange{ 0x0a, 0x0e, false };
		return true;
	}
	if (fromFacing == 1 && toFacing == 0) {
		range = PlayerTurnAnimationRange{ 0x0a, 0x0e, true };
		return true;
	}
	if (fromFacing == 1 && toFacing == 3) {
		range = PlayerTurnAnimationRange{ 0x19, 0x1d, false };
		return true;
	}
	if (fromFacing == 3 && toFacing == 1) {
		range = PlayerTurnAnimationRange{ 0x19, 0x1d, true };
		return true;
	}
	if (fromFacing == 3 && toFacing == 2) {
		range = PlayerTurnAnimationRange{ 0x28, 0x2c, false };
		return true;
	}
	if (fromFacing == 2 && toFacing == 3) {
		range = PlayerTurnAnimationRange{ 0x28, 0x2c, true };
		return true;
	}
	if (fromFacing == 2 && toFacing == 0) {
		range = PlayerTurnAnimationRange{ 0x37, 0x3b, false };
		return true;
	}
	if (fromFacing == 0 && toFacing == 2) {
		range = PlayerTurnAnimationRange{ 0x37, 0x3b, true };
		return true;
	}

	return false;
}

static const PlayerAttackSoundSet *resolvePlayerAttackSoundSet(int loadout) {
	if (loadout < 0 || loadout >= (int)ARRAYSIZE(kPlayerAttackSoundSets))
		return nullptr;

	if (kPlayerAttackSoundSets[loadout].soundCount == 0)
		return nullptr;

	return &kPlayerAttackSoundSets[loadout];
}

static const PlayerCombatLoadoutTuning &resolvePlayerCombatLoadoutTuning(int loadout) {
	static const PlayerCombatLoadoutTuning kDefaultTuning = { "FISTS", 1, 1, 5 };

	if (loadout < 0 || loadout >= (int)ARRAYSIZE(kPlayerCombatLoadoutTunings))
		return kDefaultTuning;

	return kPlayerCombatLoadoutTunings[loadout];
}

static bool playPlayerAttackSound(HarvesterEngine &engine, int loadout) {
	const PlayerAttackSoundSet *soundSet = resolvePlayerAttackSoundSet(loadout);
	if (!soundSet)
		return false;

	uint soundIndex = 0;
	if (soundSet->soundCount > 1)
		soundIndex = engine.getRandomNumber(soundSet->soundCount - 1);

	const char *soundPath = soundSet->soundPaths[soundIndex];
	return soundPath && engine.playSound(soundPath);
}

static bool loadoutUsesConsumableCombatResource(int loadout) {
	switch (loadout) {
	case 2:
	case 3:
	case 4:
	case 5:
	case 14:
		return true;
	default:
		return false;
	}
}

static bool playPlayerHitSound(HarvesterEngine &engine) {
	const uint soundIndex = ARRAYSIZE(kPlayerHitSoundPaths) > 1
		? engine.getRandomNumber(ARRAYSIZE(kPlayerHitSoundPaths) - 1)
		: 0;
	return engine.playSound(kPlayerHitSoundPaths[soundIndex]);
}

static bool playerEntityHasFrameRange(const Entity &entity, int firstFrame, int lastFrame) {
	if (!entity.hasFrames() || firstFrame < 0 || lastFrame < firstFrame)
		return false;

	return (uint)lastFrame < entity.getFrameCount();
}

static bool choosePlayerDeathAnimationRange(const Entity &entity, int firstFrame, int lastFrame,
		PlayerDeathAnimationRange &range) {
	if (!playerEntityHasFrameRange(entity, firstFrame, lastFrame))
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

static bool resolvePlayerAttackAnimationRange(const RoomSetupState &state,
		const RoomPlayerState &playerState, const Common::Point &mousePos,
		PlayerAttackAnimationRange &range) {
	if (!playerState.entity)
		return false;

	const float depthScale = Player::computeDepthScale(state, playerState.z);
	const float upperThresholdY =
		(float)playerState.bottomY - depthScale * kRoomPlayerAttackUpperYOffset;
	const float midThresholdY =
		(float)playerState.bottomY - depthScale * kRoomPlayerAttackMidYOffset;

	if (mousePos.x < playerState.centerX) {
		if ((float)mousePos.y < upperThresholdY)
			range = PlayerAttackAnimationRange(0x50, 0x59, 1);
		else if ((float)mousePos.y < midThresholdY)
			range = PlayerAttackAnimationRange(0x5a, 0x63, 1);
		else
			range = PlayerAttackAnimationRange(0x64, 0x6d, 1);
	} else {
		if ((float)mousePos.y < upperThresholdY)
			range = PlayerAttackAnimationRange(0x6e, 0x77, 2);
		else if ((float)mousePos.y < midThresholdY)
			range = PlayerAttackAnimationRange(0x78, 0x81, 2);
		else
			range = PlayerAttackAnimationRange(0x82, 0x8b, 2);
	}

	return true;
}

static bool resolveKeyboardAttackAnimationRange(const RoomPlayerState &playerState,
		bool attackLeft, bool attackRight, bool attackUp, bool attackDown,
		PlayerAttackAnimationRange &range) {
	if (attackRight) {
		range = PlayerAttackAnimationRange(0x78, 0x81, 2);
		return true;
	}
	if (attackLeft) {
		range = PlayerAttackAnimationRange(0x5a, 0x63, 1);
		return true;
	}
	if (attackUp) {
		if (playerState.facing == 2) {
			range = PlayerAttackAnimationRange(0x6e, 0x77, 2);
			return true;
		}
		if (playerState.facing == 1) {
			range = PlayerAttackAnimationRange(0x50, 0x59, 1);
			return true;
		}
		return false;
	}
	if (attackDown) {
		if (playerState.facing == 2) {
			range = PlayerAttackAnimationRange(0x82, 0x8b, 2);
			return true;
		}
		if (playerState.facing == 1) {
			range = PlayerAttackAnimationRange(0x64, 0x6d, 1);
			return true;
		}
		return false;
	}

	return false;
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

	if (!playerEntityHasFrameRange(entity, candidate.firstFrame, candidate.lastFrame))
		return false;

	range = candidate;
	return true;
}

static bool startResolvedAttackAnimation(RoomPlayerState &playerState,
		const PlayerAttackAnimationRange &range) {
	if (!playerState.entity || playerState.attackActive || playerState.hitActive)
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
	playerState.attackActive = true;
	playerState.attackFirstFrame = range.firstFrame;
	playerState.attackLastFrame = range.lastFrame;
	playerState.attackContactFrame =
		range.firstFrame + Player::resolveCombatLoadoutContactFrameOffset(playerState.combatLoadout);
	playerState.attackResumeFacing = range.resumeFacing;
	playerState.attackSoundPlayed = resolvePlayerAttackSoundSet(playerState.combatLoadout) == nullptr;
	playerState.attackSoundPlaybackFrame = playerState.attackSoundPlayed
		? -1
		: (range.firstFrame + kPlayerAttackSoundTriggerFrameOffset);
	playerState.attackContactResolved = false;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setAnimationRate(playerState.animationRate);
	playerState.entity->setCurrentFrame(range.firstFrame);
	return true;
}

static void notePlayerIdleReset(RoomIdleAnimationState &idleState) {
	idleState.resetTick = Player::getRuntimeClockTicks();
	updatePlayerIdleTrigger(idleState);
}

static void positionPlayerIdleAnimationEntity(const RoomSetupState &state,
		const RoomPlayerState &playerState, Entity &idleEntity) {
	if (!playerState.entity)
		return;

	idleEntity.setAnchorMode(kEntityAnchorTopLeft);
	idleEntity.setDepthScale(Player::computeDepthScale(state, playerState.z));
	idleEntity.setPosition(playerState.entity->getX(),
		playerState.entity->getY() + kRoomPlayerIdleYOffset, playerState.z);
}

static Entity *ensurePlayerIdleAnimationEntity(HarvesterEngine &engine,
		const RoomSetupState &state, const RoomPlayerState &playerState,
		RoomIdleAnimationState &idleState) {
	if (idleState.entity || !playerState.entity)
		return idleState.entity;

	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	idleState.entity = entityManager->spawnSceneAnimationEntity(
		kPlayerIdleAnimationEntityName, kPlayerIdleAnimationResourcePath,
		Common::Point(playerState.entity->getX(),
			playerState.entity->getY() + kRoomPlayerIdleYOffset),
		playerState.z, 0, false, false, false, false, false);
	if (!idleState.entity)
		return nullptr;

	positionPlayerIdleAnimationEntity(state, playerState, *idleState.entity);
	idleState.entity->setVisible(false);
	debugC(1, kDebugPlayer,
		"Harvester: spawned player idle animation '%s' room='%s' frames=0..%d",
		kPlayerIdleAnimationResourcePath, state.roomName.c_str(), idleState.entity->getLastFrame());
	return idleState.entity;
}

static bool finishPlayerIdleAnimation(const RoomSetupState &state, RoomPlayerState &playerState,
		RoomIdleAnimationState &idleState) {
	if (!idleState.active)
		return false;

	if (idleState.entity) {
		idleState.entity->setVisible(false);
		idleState.entity->setAnimationRate(0);
		idleState.entity->setCurrentFrame(0);
		positionPlayerIdleAnimationEntity(state, playerState, *idleState.entity);
	}
	if (playerState.entity) {
		playerState.entity->setVisible(true);
		if (idleState.restoreFacing >= 0)
			(void)setPlayerIdleAnimation(playerState, idleState.restoreFacing);
	}

	idleState.active = false;
	idleState.loopStarted = false;
	idleState.exiting = false;
	idleState.restoreFacing = -1;
	notePlayerIdleReset(idleState);
	debugC(1, kDebugPlayer,
		"Harvester: player idle animation finished room='%s' facing=%d",
		state.roomName.c_str(), playerState.facing);
	return true;
}

static void updatePlayerIdleTrigger(RoomIdleAnimationState &idleState) {
	idleState.triggerTick = MAX(idleState.activityTick, idleState.resetTick) + kRoomPlayerIdleDelayTicks;
}

static bool setPlayerIdleAnimation(RoomPlayerState &playerState, int facing) {
	if (!playerState.entity)
		return false;

	const PlayerAnimationRange range = resolvePlayerAnimationRange(facing);
	const bool changed = playerState.facing != facing ||
		playerState.entity->getCurrentFrame() != range.idleFrame ||
		playerState.entity->getAnimationRate() != 0;
	playerState.facing = facing;
	playerState.evasiveState = -1;
	playerState.evasiveXStep = 0;
	playerState.nextMovementTick = 0;
	playerState.keyboardWalkRequest = -1;
	playerState.walkHorizontalDirection = 0;
	playerState.walkDepthDirection = 0;
	playerState.entity->setAnimationRate(0);
	playerState.entity->setAnimationFrameRange(range.idleFrame, range.idleFrame, false);
	playerState.entity->setCurrentFrame(range.idleFrame);
	if (changed) {
		debugC(1, kDebugPlayer,
			"Harvester: player idle animation facing=%d frame=%d",
			facing, range.idleFrame);
	}
	return changed;
}

} // End of anonymous namespace

void Player::initialize(const RoomSetupState &state, Entity *entity, int combatLoadout) {
	RoomPlayerState &playerState = _state;
	playerState.entity = entity;
	playerState.centerX = state.playerSpawnX;
	playerState.bottomY = state.playerSpawnY;
	playerState.z = (float)state.playerSpawnZ;
	playerState.facing = state.playerFacing;
	playerState.combatLoadout = combatLoadout;
	playerState.evasiveState = -1;
	playerState.evasiveXStep = 0;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
}

bool Player::isCombatLocked() const {
	return _state.attackActive || _state.hitActive || _state.deathActive || _state.evasiveState >= 0;
}

void Player::cancelMovement() {
	_state.hasMoveTarget = false;
	_state.evasiveState = -1;
	_state.evasiveXStep = 0;
	_state.turnActive = false;
	_state.turnTargetFacing = -1;
}

void Player::cancelMovementForAttack() {
	cancelMovement();
	_state.turnFirstFrame = -1;
	_state.turnLastFrame = -1;
	_state.turnEndFrame = -1;
	_state.turnPlayBackwards = false;
}

bool Player::cancelAttackAnimation() {
	RoomPlayerState &playerState = _state;
	if (!playerState.attackActive)
		return false;

	playerState.attackActive = false;
	playerState.attackFirstFrame = -1;
	playerState.attackLastFrame = -1;
	playerState.attackContactFrame = -1;
	if (!playerState.entity)
		return false;
	playerState.attackSoundPlayed = false;
	playerState.attackSoundPlaybackFrame = -1;
	playerState.attackContactResolved = false;
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	const int resumeFacing = playerState.attackResumeFacing >= 0
		? playerState.attackResumeFacing
		: (playerState.facing >= 0 ? playerState.facing : 0);
	playerState.attackResumeFacing = -1;
	return setIdleAnimation(resumeFacing);
}

void Player::clearAttackTarget() {
	_state.attackTargetName.clear();
	_state.attackTargetClassId = -1;
}

void Player::captureAttackTarget(const Common::String &name, int classId) {
	_state.attackTargetName = name;
	_state.attackTargetClassId = classId;
}

void Player::markAttackContactResolved() {
	_state.attackContactResolved = true;
}

void Player::resetIdleState() {
	RoomIdleAnimationState &idleState = _idleState;
	idleState = RoomIdleAnimationState();
	idleState.activityTick = Player::getRuntimeClockTicks();
	idleState.resetTick = idleState.activityTick;
	updatePlayerIdleTrigger(idleState);
}

void Player::noteActivity() {
	RoomIdleAnimationState &idleState = _idleState;
	idleState.activityTick = Player::getRuntimeClockTicks();
	updatePlayerIdleTrigger(idleState);
}

void Player::cancelIdleAnimation() {
	RoomPlayerState &playerState = _state;
	RoomIdleAnimationState &idleState = _idleState;
	if (idleState.entity) {
		idleState.entity->setVisible(false);
		idleState.entity->setAnimationRate(0);
	}
	idleState.active = false;
	idleState.loopStarted = false;
	idleState.exiting = false;
	idleState.restoreFacing = -1;
	if (playerState.entity)
		playerState.entity->setVisible(true);
}

void Player::applyGotoXZ(const RoomSetupState &state, EntityManager *entityManager, int x, int z) {
	RoomPlayerState &playerState = _state;
	playerState.hasMoveTarget = false;
	playerState.evasiveState = -1;
	playerState.evasiveXStep = 0;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.centerX = x;
	playerState.z = (float)z;
	playerState.bottomY = mapRoomDepthToScreenYForCombat(
		state, playerState.z, playerState.bottomY);
	Common::Rect screenRect;
	bool hasScreenRect = false;
	float depthScale = 0.0f;
	if (playerState.entity) {
		(void)applyRoomActorPlacement(state, *playerState.entity,
			playerState.centerX, playerState.bottomY, playerState.z);
		if (entityManager)
			entityManager->reinsertSceneEntity(playerState.entity);
		screenRect = playerState.entity->getScreenRect();
		hasScreenRect = true;
		depthScale = playerState.entity->getDepthScale();
	}
	debugC(1, kDebugPlayer,
		"Harvester: PC_GOTO_XZ applied x=%d z=%d pos=(%d,%d,z=%.2f) screen_rect=(%d,%d)-(%d,%d) depth_scale=%.3f",
		x, z, playerState.centerX, playerState.bottomY, (double)playerState.z,
		hasScreenRect ? screenRect.left : 0, hasScreenRect ? screenRect.top : 0,
		hasScreenRect ? screenRect.right : 0, hasScreenRect ? screenRect.bottom : 0,
		(double)depthScale);
}

void Player::rebindAfterSceneRefresh(HarvesterEngine &engine, const RoomSetupState &state,
		EntityManager *entityManager, bool preservePlayerPlacement) {
	RoomPlayerState &playerState = _state;
	playerState.entity = entityManager ? entityManager->findSceneEntityByName("PLAYER") : nullptr;
	if (playerState.entity) {
		if (!preservePlayerPlacement || playerState.facing < 0) {
			playerState.centerX = state.playerSpawnX;
			playerState.bottomY = state.playerSpawnY;
			playerState.z = (float)state.playerSpawnZ;
			playerState.facing = state.playerFacing;
		}

		const int facing = playerState.facing >= 0 ? playerState.facing : state.playerFacing;
		(void)setIdleAnimation(facing);
		(void)applyRoomActorPlacement(state, *playerState.entity,
			playerState.centerX, playerState.bottomY, playerState.z);
	}

	// Death state survives a scene refresh.
	playerState.hasMoveTarget = false;
	playerState.evasiveState = -1;
	playerState.evasiveXStep = 0;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
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
	debugC(2, kDebugPlayer, "Harvester: rebound player after scene refresh room='%s' entity=%d preserve_placement=%d",
		state.roomName.c_str(), playerState.entity != nullptr, preservePlayerPlacement);
	playerState.combatLoadout = engine.getScript() ? engine.getScript()->getPlayerCombatLoadout() : 0;
}

void Player::syncAnimationPlacement() {
	syncPlayerAnimationPlacement(_state);
}

float Player::computeDepthScale(const RoomSetupState &state, float z) {
	float scale = 1.0f;
	if (state.roomPerspectiveScale != 0.0f) {
		scale -= (z - (float)state.roomFullScaleZ) * state.roomPerspectiveScale;
		if (scale < 0.0f)
			scale = 1.0f;
	}

	return scale;
}

int Player::resolveFacingFrame(int facing) {
	switch (facing) {
	case 0:
		return 0x3b;
	case 1:
		return 0x0e;
	case 2:
		return 0x2c;
	case 3:
		return 0x28;
	default:
		return 0x0e;
	}
}

Common::String Player::resolveCombatLoadoutResourcePath(int loadout) {
	if (loadout <= 0)
		return Common::String(kPlayerActorResourcePath);

	return Common::String::format("1:/GRAPHIC/MONSTERS/PC/PC%02d.ABM", loadout);
}

const char *Player::describeCombatLoadout(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).label;
}

int Player::resolveCombatLoadoutDamageAmount(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).damageAmount;
}

int Player::resolveCombatLoadoutDamageType(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).damageType;
}

const char *Player::describeCombatDamageType(int damageType) {
	switch (damageType) {
	case 1:
		return "BLUDGE";
	case 2:
		return "SLASH";
	case 4:
		return "PROJ";
	default:
		return "UNKNOWN";
	}
}

int Player::resolveCombatLoadoutContactFrameOffset(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).contactFrameOffset;
}

bool Player::isProjectileCombatLoadout(int loadout) {
	return resolveCombatLoadoutDamageType(loadout) == 4;
}

bool Player::supportsMovementBand(const RoomSetupState &state) {
	return state.roomMaxZScreenY >= 0 &&
		state.roomMinZScreenY >= state.roomMaxZScreenY;
}

uint32 Player::getRuntimeClockTicks() {
	return Harvester::getRuntimeClockTicks();
}

bool Player::isIdleAnimationExcludedRoom(const Common::String &roomName) {
	static const char *const kExcludedRooms[] = {
		"JAWS",
		"SUPPLY1",
		"SUPPLY2",
		"SUPPLY3",
		"LAVAPIT",
		"RMNBATH",
		"SERGENTRM",
		"CHAP2"
	};

	for (const char *excludedRoom : kExcludedRooms) {
		if (roomName.equalsIgnoreCase(excludedRoom))
			return true;
	}

	return false;
}


void Player::syncCombatLoadout(HarvesterEngine &engine, const RoomSetupState &state) {
	if (_state.entity) {
		(void)syncCombatLoadoutVisual(engine, state, engine.getScript()->getPlayerCombatLoadout());
	} else if (engine.getScript()) {
		_state.combatLoadout = engine.getScript()->getPlayerCombatLoadout();
	}
}

bool Player::syncCombatLoadoutVisual(HarvesterEngine &engine, const RoomSetupState &state, int loadout) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity)
		return false;

	ResourceManager *resources = engine.getResources();
	if (!resources)
		return false;

	const int clampedLoadout = CLIP<int>(loadout, 0, 0x14);
	if (playerState.combatLoadout == clampedLoadout)
		return false;

	const Common::String resourcePath = resolveCombatLoadoutResourcePath(clampedLoadout);
	if (!playerState.entity->loadAbmResource(*resources, resourcePath))
		return false;

	playerState.combatLoadout = clampedLoadout;
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
	(void)setIdleAnimation(playerState.facing >= 0 ? playerState.facing : 0);
	if (applyRoomActorPlacement(state, *playerState.entity,
			playerState.centerX, playerState.bottomY, playerState.z)) {
		reinsertPlayerSceneEntity(engine, playerState);
	}
	debugC(1, kDebugPlayer,
		"Harvester: player combat loadout visual loadout=%d weapon='%s' damage=%d damage_type='%s' contact=%d resource='%s' facing=%d pos=(%d,%d,z=%.2f)",
		playerState.combatLoadout, describeCombatLoadout(playerState.combatLoadout),
		resolveCombatLoadoutDamageAmount(playerState.combatLoadout),
		describeCombatDamageType(resolveCombatLoadoutDamageType(playerState.combatLoadout)),
		resolveCombatLoadoutContactFrameOffset(playerState.combatLoadout), resourcePath.c_str(), playerState.facing,
		playerState.centerX, playerState.bottomY, (double)playerState.z);
	return true;
}

bool Player::setIdleAnimation(int facing) {
	return setPlayerIdleAnimation(_state, facing);
}

bool Player::updateAnimationRate(bool speedUp) {
	RoomPlayerState &playerState = _state;
	const int rate = speedUp ? kRoomPlayerFastAnimationRate : kRoomPlayerWalkAnimationRate;
	if (playerState.animationRate == rate)
		return false;
	playerState.animationRate = rate;
	if (playerState.entity && !playerState.hitActive && !playerState.deathActive &&
			playerState.entity->getAnimationRate() != 0)
		playerState.entity->setAnimationRate(rate);
	debugC(2, kDebugPlayer, "Harvester: player actor rate=%d", rate);
	return true;
}

bool Player::startEvasiveAnimation(bool moveLeft, bool moveRight, bool moveUp, bool moveDown) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity || playerState.attackActive || playerState.hitActive || playerState.deathActive ||
			playerState.evasiveState >= 0)
		return false;
	playerState.hasMoveTarget = false;
	playerState.keyboardWalkRequest = -1;
	if (playerState.turnActive)
		return false;
	const PlayerAnimationRange walk = resolvePlayerAnimationRange(playerState.facing);
	const int frame = playerState.entity->getCurrentFrame();
	if (playerState.entity->getAnimationRate() != 0 &&
			frame >= walk.walkFirstFrame && frame < walk.walkLastFrame)
		return false;

	// Right Shift branch:
	// vertical requests select duck/leap banks; horizontal requests can override them.
	int actorState = -1;
	if (playerState.facing == 1 || playerState.facing == 2) {
		if (moveUp)
			actorState = playerState.facing == 2 ? 0x12 : 0x13;
		else if (moveDown)
			actorState = playerState.facing == 2 ? 0x14 : 0x15;
		if (moveLeft) {
			if (playerState.facing == 2)
				actorState = 0x08;
		} else if (moveRight && playerState.facing == 1) {
			actorState = 0x0f;
		}
	}
	if (actorState < 0)
		return false;

	const bool backwards = actorState == 0x08 || actorState == 0x0f;
	const int firstFrame = actorState == 0x08 ? 0x2d : actorState == 0x0f ? 0x0f :
		0x3c + (actorState - 0x12) * 5;
	const int lastFrame = firstFrame + (backwards ? 9 : 4);
	if (!playerEntityHasFrameRange(*playerState.entity, firstFrame, lastFrame))
		return false;
	playerState.evasiveState = actorState;
	playerState.evasiveFirstFrame = firstFrame;
	playerState.evasiveLastFrame = lastFrame;
	playerState.evasiveXStep = backwards
		? (int)(playerState.walkHorizontalDirection * playerState.entity->getDepthScale() * 8.0f) : 0;
	playerState.walkHorizontalDirection = 0;
	playerState.walkDepthDirection = 0;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationFrameRange(firstFrame, lastFrame, false);
	playerState.entity->setPlayBackwards(backwards);
	playerState.entity->setAnimationRate(playerState.animationRate);
	playerState.entity->setCurrentFrame(backwards ? lastFrame : firstFrame);
	debugC(2, kDebugPlayer, "Harvester: player evasive state=0x%x facing=%d frames=%d..%d backwards=%d",
		actorState, playerState.facing, firstFrame, lastFrame, backwards);
	return true;
}

bool Player::updateEvasiveAnimation(HarvesterEngine &engine) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity || playerState.evasiveState < 0 ||
			!playerState.entity->didAnimationAdvanceLastTick())
		return false;

	Entity &entity = *playerState.entity;
	const int actorState = playerState.evasiveState;
	if (playerState.evasiveXStep != 0) {
		entity.setPosition(entity.getX() + playerState.evasiveXStep, entity.getY(), entity.getZ());
		syncAnimationPlacement();
		reinsertPlayerSceneEntity(engine, playerState);
		debugC(3, kDebugPlayer, "Harvester: player evasive step state=0x%x frame=%d dx=%d",
			actorState, entity.getCurrentFrame(), playerState.evasiveXStep);
	}
	const bool backwards = actorState == 0x08 || actorState == 0x0f;
	if (entity.getCurrentFrame() == (backwards ? playerState.evasiveFirstFrame : playerState.evasiveLastFrame)) {
		debugC(2, kDebugPlayer, "Harvester: player evasive complete state=0x%x facing=%d",
			actorState, playerState.facing);
		return setIdleAnimation(playerState.facing);
	}

	const int frameLeft = computePlayerFrameLeftX(playerState);
	if ((actorState == 0x08 && frameLeft > 20) || (actorState == 0x0f && frameLeft < 620)) {
		playerState.evasiveXStep = (int)(entity.getDepthScale() * (actorState == 0x08 ? -8.0f : 8.0f));
	} else if (entity.getCurrentFrame() > playerState.evasiveFirstFrame + 2 &&
			((actorState == 0x12 && frameLeft > 36) || (actorState == 0x13 && frameLeft < 616))) {
		playerState.evasiveXStep = (int)(entity.getDepthScale() * (actorState == 0x12 ? -16.0f : 16.0f));
	}
	return true;
}

bool Player::startAttackAnimation(const RoomSetupState &state, const Common::Point &mousePos) {
	RoomPlayerState &playerState = _state;
	PlayerAttackAnimationRange range;
	if (!resolvePlayerAttackAnimationRange(state, playerState, mousePos, range))
		return false;
	if (!startResolvedAttackAnimation(playerState, range))
		return false;
	debugC(1, kDebugPlayer,
		"Harvester: player attack animation frames=%d..%d resume_facing=%d cursor=(%d,%d)",
		range.firstFrame, range.lastFrame, range.resumeFacing, mousePos.x, mousePos.y);
	return true;
}

bool Player::startKeyboardAttackAnimation(const RoomSetupState &state,
		bool attackLeft, bool attackRight, bool attackUp, bool attackDown) {
	RoomPlayerState &playerState = _state;
	(void)state;

	PlayerAttackAnimationRange range;
	if (!resolveKeyboardAttackAnimationRange(playerState, attackLeft, attackRight, attackUp, attackDown, range))
		return false;
	if (!startResolvedAttackAnimation(playerState, range))
		return false;
	debugC(1, kDebugPlayer,
		"Harvester: player keyboard attack animation frames=%d..%d resume_facing=%d input=(L=%d R=%d U=%d D=%d) facing=%d",
		range.firstFrame, range.lastFrame, range.resumeFacing,
		attackLeft, attackRight, attackUp, attackDown, playerState.facing);
	return true;
}

bool Player::updateAttackAnimationState(HarvesterEngine &engine) {
	RoomPlayerState &playerState = _state;
	if (!playerState.attackActive || !playerState.entity)
		return false;
	bool changed = false;
	if (!playerState.attackSoundPlayed &&
			playerState.attackSoundPlaybackFrame >= 0 &&
			playerState.entity->getCurrentFrame() >= playerState.attackSoundPlaybackFrame) {
		if (loadoutUsesConsumableCombatResource(playerState.combatLoadout)) {
			Script *script = engine.getScript();
			if (!script || !script->consumePlayerCombatResourceUnit(playerState.combatLoadout)) {
				debugC(1, kDebugCombat,
					"Harvester: combat player attack dry fire loadout=%d weapon='%s'",
					playerState.combatLoadout,
					describeCombatLoadout(playerState.combatLoadout));
				playerState.attackContactResolved = true;
				playerState.attackContactFrame = -1;
				playerState.attackTargetName.clear();
				playerState.attackTargetClassId = -1;
			} else {
				changed = true;
				(void)playPlayerAttackSound(engine, playerState.combatLoadout);
			}
		} else {
			(void)playPlayerAttackSound(engine, playerState.combatLoadout);
		}
		playerState.attackSoundPlayed = true;
	}
	if (playerState.entity->getCurrentFrame() != playerState.attackLastFrame)
		return changed;

	const int resumeFacing = playerState.attackResumeFacing >= 0
		? playerState.attackResumeFacing
		: (playerState.facing >= 0 ? playerState.facing : 0);
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
	return setIdleAnimation(resumeFacing) || changed;
}

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

bool Player::startTurnAnimation(int targetFacing) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity || playerState.hitActive || playerState.evasiveState >= 0 ||
			playerState.facing < 0 || playerState.facing == targetFacing)
		return false;

	int nextFacing = targetFacing;
	if ((playerState.facing == 1 && targetFacing == 2) ||
			(playerState.facing == 2 && targetFacing == 1))
		nextFacing = 0;
	else if ((playerState.facing == 0 && targetFacing == 3) ||
			(playerState.facing == 3 && targetFacing == 0))
		nextFacing = 2;
	PlayerTurnAnimationRange range;
	if (playerState.facing == 3 && targetFacing == 0) {
		range = PlayerTurnAnimationRange{ 0x28, 0x28, false };
	} else if (!resolvePlayerTurnAnimationRange(playerState.facing, nextFacing, range)) {
		return false;
	}

	playerState.turnActive = true;
	playerState.turnTargetFacing = nextFacing;
	playerState.turnFirstFrame = range.firstFrame;
	playerState.turnLastFrame = range.lastFrame;
	playerState.turnEndFrame = range.playBackwards ? range.firstFrame : range.lastFrame;
	playerState.turnPlayBackwards = range.playBackwards;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setPlayBackwards(range.playBackwards);
	playerState.entity->setAnimationRate(playerState.animationRate);
	playerState.entity->setCurrentFrame(range.playBackwards ? range.lastFrame : range.firstFrame);
	debugC(1, kDebugPlayer,
		"Harvester: player turn animation from=%d to=%d frames=%d..%d backwards=%d rate=%d requested_facing=%d",
		playerState.facing, nextFacing, range.firstFrame, range.lastFrame,
		range.playBackwards, playerState.animationRate, targetFacing);
	return true;
}

bool Player::updateTurnAnimationState() {
	RoomPlayerState &playerState = _state;
	if (!playerState.turnActive || !playerState.entity)
		return false;
	if (playerState.entity->getCurrentFrame() != playerState.turnEndFrame)
		return false;

	playerState.turnActive = false;
	playerState.facing = playerState.turnTargetFacing;
	playerState.turnTargetFacing = -1;
	playerState.turnFirstFrame = -1;
	playerState.turnLastFrame = -1;
	playerState.turnEndFrame = -1;
	playerState.turnPlayBackwards = false;
	playerState.entity->setAnimationRate(0);
	debugC(1, kDebugPlayer,
		"Harvester: player turn complete facing=%d frame=%d",
		playerState.facing, playerState.entity->getCurrentFrame());
	return true;
}

bool Player::requestIdleAnimationExit(const RoomSetupState &state) {
	RoomPlayerState &playerState = _state;
	RoomIdleAnimationState &idleState = _idleState;
	if (!idleState.active || idleState.exiting || !idleState.entity)
		return false;

	const int lastFrame = idleState.entity->getLastFrame();
	const int exitStartFrame = MIN(lastFrame, kRoomPlayerIdleLoopLastFrame);
	const int exitLastFrame = MIN(lastFrame, kRoomPlayerIdleExitLastFrame);
	if (exitLastFrame <= exitStartFrame)
		return finishPlayerIdleAnimation(state, playerState, idleState);

	positionPlayerIdleAnimationEntity(state, playerState, *idleState.entity);
	idleState.exiting = true;
	idleState.entity->setAnimationFrameRange(exitStartFrame, exitLastFrame, false);
	idleState.entity->setCurrentFrame(exitStartFrame);
	idleState.entity->setAnimationRate(kRoomPlayerIdleAnimationRate);
	debugC(1, kDebugPlayer,
		"Harvester: player idle animation exit room='%s' frames=%d..%d rate=%d",
		state.roomName.c_str(), exitStartFrame, exitLastFrame, kRoomPlayerIdleAnimationRate);
	return true;
}

bool Player::startIdleAnimation(HarvesterEngine &engine, const RoomSetupState &state) {
	RoomPlayerState &playerState = _state;
	RoomIdleAnimationState &idleState = _idleState;
	if (!playerState.entity || idleState.active || isIdleAnimationExcludedRoom(state.roomName))
		return false;

	Entity *idleEntity = ensurePlayerIdleAnimationEntity(engine, state, playerState, idleState);
	if (!idleEntity)
		return false;

	const int lastFrame = idleEntity->getLastFrame();
	const int loopLastFrame = MIN(lastFrame, kRoomPlayerIdleLoopLastFrame);
	positionPlayerIdleAnimationEntity(state, playerState, *idleEntity);
	idleState.restoreFacing = playerState.facing;
	idleState.active = true;
	idleState.loopStarted = false;
	idleState.exiting = false;
	idleEntity->setVisible(true);
	idleEntity->setAnimationFrameRange(0, loopLastFrame, false);
	idleEntity->setCurrentFrame(0);
	idleEntity->setAnimationRate(kRoomPlayerIdleAnimationRate);
	playerState.hasMoveTarget = false;
	playerState.evasiveState = -1;
	playerState.evasiveXStep = 0;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.entity->setVisible(false);
	debugC(1, kDebugPlayer,
		"Harvester: player idle animation start room='%s' facing=%d frames=%d..%d rate=%d trigger_tick=%u",
		state.roomName.c_str(), idleState.restoreFacing, 0, loopLastFrame,
		kRoomPlayerIdleAnimationRate, idleState.triggerTick);
	return true;
}

bool Player::updateIdleAnimation(const RoomSetupState &state) {
	RoomPlayerState &playerState = _state;
	RoomIdleAnimationState &idleState = _idleState;
	if (!idleState.active || !idleState.entity)
		return false;

	const int lastFrame = idleState.entity->getLastFrame();
	const int loopLastFrame = MIN(lastFrame, kRoomPlayerIdleLoopLastFrame);
	if (!idleState.exiting && !idleState.loopStarted &&
			idleState.entity->getCurrentFrame() >= loopLastFrame) {
		const int loopStartFrame = MIN(lastFrame, kRoomPlayerIdleLoopStartFrame);
		idleState.loopStarted = true;
		if (loopStartFrame < loopLastFrame) {
			idleState.entity->setAnimationFrameRange(loopStartFrame, loopLastFrame, true);
			idleState.entity->setCurrentFrame(loopStartFrame);
			idleState.entity->setAnimationRate(kRoomPlayerIdleAnimationRate);
		}
		debugC(1, kDebugPlayer,
			"Harvester: player idle animation loop room='%s' frames=%d..%d rate=%d",
			state.roomName.c_str(), loopStartFrame, loopLastFrame, kRoomPlayerIdleAnimationRate);
		return true;
	}

	if (!idleState.exiting)
		return false;

	const int exitLastFrame = MIN(lastFrame, kRoomPlayerIdleExitLastFrame);
	if (idleState.entity->getCurrentFrame() < exitLastFrame)
		return false;

	return finishPlayerIdleAnimation(state, playerState, idleState);
}

} // End of namespace Harvester
