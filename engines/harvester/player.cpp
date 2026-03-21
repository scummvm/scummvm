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

#include <math.h>

#include "harvester/player.h"

#include "common/debug.h"
#include "common/system.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

namespace {

static const char *const kPlayerActorResourcePath = "1:/GRAPHIC/MONSTERS/PC/PC0.ABM";
static const float kRoomPlayerHorizontalMoveBase = 8.0f;
static const int kRoomRegionTargetXBias = 10;
static const float kRoomDepthCompareEpsilon = 0.01f;
static const int kRoomPlayerWalkAnimationRate = 17;
static const int kRoomPlayerAttackAnimationRate = kRoomPlayerWalkAnimationRate;
static const int kRoomPlayerMinOpaqueLeftX = 4;
static const int kRoomPlayerMaxOpaqueRightX = 0x27c;
static const float kRoomPlayerAttackUpperYOffset = 144.44f;
static const float kRoomPlayerAttackMidYOffset = 75.36f;
static const char *const kPlayerIdleAnimationEntityName = "IDLE_ANIM";
static const char *const kPlayerIdleAnimationResourcePath = "1:/GRAPHIC/ROOMANIM/PCLOUN02.ABM";
static const uint32 kRuntimeClockDivisorMs = 10;
static const uint32 kRoomPlayerIdleDelayTicks = 3000;
// Native run_harvester_main_loop sets EBX=0x0e before spawning PCLOUN02.ABM.
static const int kRoomPlayerIdleAnimationRate = 14;
static const int kRoomPlayerIdleLoopStartFrame = 0x0f;
static const int kRoomPlayerIdleLoopLastFrame = 0xb2;
static const int kRoomPlayerIdleExitLastFrame = 0xbf;
static const int kRoomPlayerIdleYOffset = 4;

struct PlayerAnimationRange {
	PlayerAnimationRange() {}
	PlayerAnimationRange(int walkFirstFrame, int walkLastFrame, int idleFrame)
		: walkFirstFrame(walkFirstFrame), walkLastFrame(walkLastFrame), idleFrame(idleFrame) {}

	int walkFirstFrame = 0;
	int walkLastFrame = 0;
	int idleFrame = 0;
};

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

struct PlayerAttackSoundSet {
	uint soundCount;
	const char *soundPaths[3];
};

struct PlayerCombatLoadoutTuning {
	int damageType;
	int damageAmount;
	int contactFrameOffset;
};

// Recovered from spawn_player_combat_avatar and update_actor_runtime_state:
// the native room-combat path commits player attack audio on first_frame + 1.
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

static const PlayerCombatLoadoutTuning kPlayerCombatLoadoutTunings[] = {
	{ 1, 1, 5 },
	{ 2, 4, 2 },
	{ 4, 2, 2 },
	{ 4, 7, 2 },
	{ 4, 6, 2 },
	{ 4, 6, 2 },
	{ 2, 3, 2 },
	{ 2, 3, 2 },
	{ 2, 3, 2 },
	{ 2, 3, 2 },
	{ 1, 3, 2 },
	{ 2, 3, 2 },
	{ 2, 4, 2 },
	{ 2, 5, 2 },
	{ 2, 4, 2 },
	{ 2, 8, 2 },
	{ 1, 3, 2 },
	{ 2, 3, 2 },
	{ 1, 3, 2 },
	{ 2, 3, 2 },
	{ 1, 2, 2 }
};

static int roundToInt(float value) {
	return value >= 0.0f ? (int)floorf(value + 0.5f) : (int)ceilf(value - 0.5f);
}

static PlayerAnimationRange resolvePlayerAnimationRange(int facing) {
	switch (facing) {
	case 0:
		return PlayerAnimationRange{ 0x00, 0x09, 0x3b };
	case 1:
		return PlayerAnimationRange{ 0x0f, 0x18, 0x0e };
	case 2:
		return PlayerAnimationRange{ 0x2d, 0x36, 0x2c };
	case 3:
		return PlayerAnimationRange{ 0x1e, 0x27, 0x28 };
	default:
		return PlayerAnimationRange{ 0x00, 0x09, 0x3b };
	}
}

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
	static const PlayerCombatLoadoutTuning kDefaultTuning = { 1, 1, 2 };

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
	return soundPath && engine.playStartupSound(soundPath);
}

static bool resolvePlayerAttackAnimationRange(const StartupRoomSetupState &state,
		const StartupRoomPlayerState &playerState, const Common::Point &mousePos,
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

static int clampRoomMovementY(const StartupRoomSetupState &state, int screenY) {
	if (!Player::supportsMovementBand(state))
		return screenY;

	return CLIP<int>(screenY, state.roomMaxZScreenY, state.roomMinZScreenY);
}

static float mapRoomScreenYToDepth(const StartupRoomSetupState &state, int screenY) {
	if (!Player::supportsMovementBand(state))
		return (float)state.playerSpawnZ;

	if (screenY <= state.roomMaxZScreenY)
		return (float)state.roomMaxZ;
	if (screenY >= state.roomMinZScreenY)
		return (float)state.roomMinZ;

	return (float)(state.roomMaxZ - ((state.roomMaxZ - state.roomMinZ) *
		(screenY - state.roomMaxZScreenY)) /
		(state.roomMinZScreenY - state.roomMaxZScreenY));
}

static float clampRoomDepth(const StartupRoomSetupState &state, float z) {
	if (!Player::supportsMovementBand(state))
		return z;

	return CLIP<float>(z,
		(float)MIN(state.roomMinZ, state.roomMaxZ),
		(float)MAX(state.roomMinZ, state.roomMaxZ));
}

static int mapRoomDepthToScreenY(const StartupRoomSetupState &state, float z) {
	if (!Player::supportsMovementBand(state))
		return state.playerSpawnY;

	if (state.roomMaxZ == state.roomMinZ)
		return clampRoomMovementY(state, state.playerSpawnY);

	const float clampedZ = clampRoomDepth(state, z);
	const float offset = ((float)state.roomMaxZ - clampedZ) *
		(float)(state.roomMinZScreenY - state.roomMaxZScreenY) /
		(float)(state.roomMaxZ - state.roomMinZ);
	return clampRoomMovementY(state, state.roomMaxZScreenY + roundToInt(offset));
}

static int resolveFacingFromMovementDelta(int dx, int dy) {
	if (ABS(dx) > ABS(dy))
		return dx < 0 ? 1 : 2;
	if (dy < 0)
		return 3;
	return 0;
}

static int resolveFacingFromRoomMovement(const StartupRoomSetupState &state,
		int fromCenterX, float fromZ, int toCenterX, float toZ) {
	return resolveFacingFromMovementDelta(
		toCenterX - fromCenterX,
		mapRoomDepthToScreenY(state, toZ) - mapRoomDepthToScreenY(state, fromZ));
}

static int computeRoomPlayerHorizontalStep(const StartupRoomSetupState &state, float z) {
	return MAX<int>(1, roundToInt(Player::computeDepthScale(state, z) * kRoomPlayerHorizontalMoveBase));
}

static float computeRoomPlayerDepthStep(const StartupRoomSetupState &state) {
	return state.roomZVelocityStep > 0.0f ? state.roomZVelocityStep : 1.0f;
}

static int clampPlayerCenterXToNativeBounds(const StartupRoomPlayerState &playerState, int centerX) {
	if (!playerState.entity)
		return CLIP<int>(centerX, 0, 639);

	int width = 0;
	int height = 0;
	int xOffset = 0;
	int yOffset = 0;
	if (!playerState.entity->getCurrentFrameMetrics(width, height, xOffset, yOffset))
		return CLIP<int>(centerX, 0, 639);

	const int minCenterX = kRoomPlayerMinOpaqueLeftX + width / 2;
	const int maxCenterX = MAX(minCenterX,
		kRoomPlayerMaxOpaqueRightX - (width - width / 2));
	return CLIP<int>(centerX, minCenterX, maxCenterX);
}

static uint32 computeAnimationTickInterval(int rate) {
	return rate > 0 ? MAX<uint32>(1, 100U / (uint32)rate) : 0;
}

static bool consumePlayerMovementTick(StartupRoomPlayerState &playerState) {
	const uint32 interval = computeAnimationTickInterval(kRoomPlayerWalkAnimationRate);
	if (interval == 0) {
		playerState.nextMovementTick = 0;
		return true;
	}

	const uint32 now = Player::getRuntimeClockTicks();
	if (playerState.nextMovementTick != 0 &&
			(int32)(now - playerState.nextMovementTick) < 0) {
		return false;
	}

	playerState.nextMovementTick = now + interval;
	return true;
}

static int stepTowardsInt(int current, int target, int step) {
	if (step <= 0 || current == target)
		return current;
	if (current < target)
		return MIN(current + step, target);
	return MAX(current - step, target);
}

static float stepTowardsFloat(float current, float target, float step) {
	if (step <= 0.0f || fabsf(target - current) <= kRoomDepthCompareEpsilon)
		return target;
	if (current < target)
		return MIN(current + step, target);
	return MAX(current - step, target);
}

static bool doesPlayerOverlapRoomBlocker(HarvesterEngine &engine,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations, const RuntimeEntity &playerEntity) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;

	for (const StartupObjectRecord &object : sceneObjects) {
		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(object.objectName);
		if (entity && playerEntity.overlapsEntity(*entity))
			return true;
	}

	for (const StartupAnimRecord &anim : sceneAnimations) {
		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(anim.animName);
		if (entity && playerEntity.overlapsEntity(*entity))
			return true;
	}

	return false;
}

static bool isPlayerMovementBlocked(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState, int candidateCenterX, float candidateZ) {
	if (!playerState.entity)
		return true;

	const int oldCenterX = playerState.centerX;
	const int oldBottomY = playerState.bottomY;
	const float oldZ = playerState.z;
	const int candidateBottomY = mapRoomDepthToScreenY(state, candidateZ);
	if (!applyRoomActorPlacement(state, *playerState.entity,
			candidateCenterX, candidateBottomY, candidateZ)) {
		(void)applyRoomActorPlacement(state, *playerState.entity, oldCenterX, oldBottomY, oldZ);
		return true;
	}

	const bool blocked = doesPlayerOverlapRoomBlocker(engine, sceneObjects, sceneAnimations, *playerState.entity);
	(void)applyRoomActorPlacement(state, *playerState.entity, oldCenterX, oldBottomY, oldZ);
	return blocked;
}

static bool tryApplyPlayerMovement(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState, int candidateCenterX, float candidateZ) {
	if (!playerState.entity)
		return false;

	candidateCenterX = clampPlayerCenterXToNativeBounds(playerState, candidateCenterX);
	candidateZ = clampRoomDepth(state, candidateZ);
	if (candidateCenterX == playerState.centerX &&
			fabsf(candidateZ - playerState.z) <= kRoomDepthCompareEpsilon) {
		return false;
	}
	if (isPlayerMovementBlocked(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, candidateZ)) {
		return false;
	}

	playerState.centerX = candidateCenterX;
	playerState.z = candidateZ;
	playerState.bottomY = mapRoomDepthToScreenY(state, candidateZ);
	return applyRoomActorPlacement(state, *playerState.entity,
		playerState.centerX, playerState.bottomY, playerState.z);
}

static void notePlayerIdleReset(StartupRoomIdleAnimationState &idleState) {
	idleState.resetTick = Player::getRuntimeClockTicks();
	Player::updateIdleTrigger(idleState);
}

static bool setPlayerWalkAnimation(StartupRoomPlayerState &playerState, int facing) {
	if (!playerState.entity)
		return false;

	const PlayerAnimationRange range = resolvePlayerAnimationRange(facing);
	const int currentFrame = playerState.entity->getCurrentFrame();
	const bool alreadyWalking = playerState.facing == facing &&
		playerState.entity->getAnimationRate() == kRoomPlayerWalkAnimationRate &&
		currentFrame >= range.walkFirstFrame && currentFrame <= range.walkLastFrame;
	if (alreadyWalking)
		return false;

	playerState.facing = facing;
	playerState.entity->setAnimationFrameRange(range.walkFirstFrame, range.walkLastFrame, true);
	playerState.entity->setAnimationRate(kRoomPlayerWalkAnimationRate);
	debugC(1, kDebugScene,
		"Harvester: player walk animation facing=%d frames=%d..%d rate=%d",
		facing, range.walkFirstFrame, range.walkLastFrame, kRoomPlayerWalkAnimationRate);
	return true;
}

static void positionPlayerIdleAnimationEntity(const StartupRoomSetupState &state,
		const StartupRoomPlayerState &playerState, RuntimeEntity &idleEntity) {
	if (!playerState.entity)
		return;

	idleEntity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
	idleEntity.setDepthScale(Player::computeDepthScale(state, playerState.z));
	idleEntity.setPosition(playerState.entity->getX(),
		playerState.entity->getY() + kRoomPlayerIdleYOffset, playerState.z);
}

static RuntimeEntity *ensurePlayerIdleAnimationEntity(HarvesterEngine &engine,
		const StartupRoomSetupState &state, const StartupRoomPlayerState &playerState,
		StartupRoomIdleAnimationState &idleState) {
	if (idleState.entity || !playerState.entity)
		return idleState.entity;

	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	idleState.entity = runtimeEntities->spawnSceneAnimationEntity(
		kPlayerIdleAnimationEntityName, kPlayerIdleAnimationResourcePath,
		Common::Point(playerState.entity->getX(),
			playerState.entity->getY() + kRoomPlayerIdleYOffset),
		playerState.z, 0, false, false, false, false, false);
	if (!idleState.entity)
		return nullptr;

	positionPlayerIdleAnimationEntity(state, playerState, *idleState.entity);
	idleState.entity->setVisible(false);
	debugC(1, kDebugScene,
		"Harvester: spawned player idle animation '%s' room='%s' frames=0..%d",
		kPlayerIdleAnimationResourcePath, state.roomName.c_str(), idleState.entity->getLastFrame());
	return idleState.entity;
}

static bool finishPlayerIdleAnimation(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
		StartupRoomIdleAnimationState &idleState) {
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
			(void)Player::setIdleAnimation(playerState, idleState.restoreFacing);
	}

	idleState.active = false;
	idleState.loopStarted = false;
	idleState.exiting = false;
	idleState.restoreFacing = -1;
	notePlayerIdleReset(idleState);
	debugC(1, kDebugScene,
		"Harvester: player idle animation finished room='%s' facing=%d",
		state.roomName.c_str(), playerState.facing);
	return true;
}

} // End of anonymous namespace

float Player::computeDepthScale(const StartupRoomSetupState &state, float z) {
	float scale = 1.0f;
	if (state.roomPerspectiveScale != 0.0f) {
		scale -= (z - (float)state.roomFullScaleZ) * state.roomPerspectiveScale;
		if (scale <= 0.0f)
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

int Player::resolveCombatLoadoutDamageAmount(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).damageAmount;
}

int Player::resolveCombatLoadoutDamageType(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).damageType;
}

int Player::resolveCombatLoadoutContactFrameOffset(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).contactFrameOffset;
}

bool Player::isProjectileCombatLoadout(int loadout) {
	return resolveCombatLoadoutDamageType(loadout) == 4;
}

bool Player::supportsMovementBand(const StartupRoomSetupState &state) {
	return state.roomMaxZScreenY >= 0 &&
		state.roomMinZScreenY >= state.roomMaxZScreenY;
}

uint32 Player::getRuntimeClockTicks() {
	return g_system ? (g_system->getMillis() / kRuntimeClockDivisorMs) : 0;
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

void Player::updateIdleTrigger(StartupRoomIdleAnimationState &idleState) {
	idleState.triggerTick = MAX(idleState.activityTick, idleState.resetTick) + kRoomPlayerIdleDelayTicks;
}

void Player::setMoveTarget(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
		int targetX, float targetZ) {
	playerState.hasMoveTarget = true;
	playerState.nextMovementTick = 0;
	playerState.targetX = clampPlayerCenterXToNativeBounds(playerState, targetX);
	playerState.targetZ = clampRoomDepth(state, targetZ);
	debugC(1, kDebugScene,
		"Harvester: player move target room='%s' current=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f)",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, mapRoomDepthToScreenY(state, playerState.targetZ), (double)playerState.targetZ);
}

void Player::setMoveTargetFromScreenPoint(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, int targetX, int targetBottomY) {
	setMoveTarget(state, playerState, targetX,
		mapRoomScreenYToDepth(state, clampRoomMovementY(state, targetBottomY)));
}

int Player::resolveRegionTargetX(const StartupRegionRecord &region,
		const StartupRoomPlayerState &playerState) {
	const Common::Rect bounds(region.left, region.top, region.right + 1, region.bottom + 1);
	int targetX = bounds.left + bounds.width() / 2;
	if (playerState.entity && targetX + kRoomRegionTargetXBias < playerState.entity->getScreenRect().right)
		targetX -= kRoomRegionTargetXBias;
	return CLIP<int>(targetX, 0, 639);
}

float Player::resolveRegionTargetZ(const StartupRegionRecord &region) {
	if (region.desiredFacing == 0)
		return (float)region.maxZ;
	if (region.desiredFacing == 3)
		return (float)region.minZ;

	return (float)(region.minZ + (region.maxZ - region.minZ) / 2);
}

bool Player::syncCombatLoadoutVisual(HarvesterEngine &engine, const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, int loadout) {
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
	playerState.attackSoundFrame = -1;
	playerState.attackContactResolved = false;
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	(void)setIdleAnimation(playerState, playerState.facing >= 0 ? playerState.facing : 0);
	(void)applyRoomActorPlacement(state, *playerState.entity,
		playerState.centerX, playerState.bottomY, playerState.z);
	debugC(1, kDebugScene,
		"Harvester: player combat loadout visual loadout=%d resource='%s' facing=%d pos=(%d,%d,z=%.2f)",
		playerState.combatLoadout, resourcePath.c_str(), playerState.facing,
		playerState.centerX, playerState.bottomY, (double)playerState.z);
	return true;
}

bool Player::setIdleAnimation(StartupRoomPlayerState &playerState, int facing) {
	if (!playerState.entity)
		return false;

	const PlayerAnimationRange range = resolvePlayerAnimationRange(facing);
	const bool changed = playerState.facing != facing ||
		playerState.entity->getCurrentFrame() != range.idleFrame ||
		playerState.entity->getAnimationRate() != 0;
	playerState.facing = facing;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationRate(0);
	playerState.entity->setAnimationFrameRange(range.idleFrame, range.idleFrame, false);
	playerState.entity->setCurrentFrame(range.idleFrame);
	if (changed) {
		debugC(1, kDebugScene,
			"Harvester: player idle animation facing=%d frame=%d",
			facing, range.idleFrame);
	}
	return changed;
}

bool Player::startAttackAnimation(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, const Common::Point &mousePos) {
	if (!playerState.entity || playerState.attackActive)
		return false;

	PlayerAttackAnimationRange range;
	if (!resolvePlayerAttackAnimationRange(state, playerState, mousePos, range))
		return false;

	playerState.hasMoveTarget = false;
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
		range.firstFrame + resolveCombatLoadoutContactFrameOffset(playerState.combatLoadout);
	playerState.attackResumeFacing = range.resumeFacing;
	playerState.attackSoundPlayed = resolvePlayerAttackSoundSet(playerState.combatLoadout) == nullptr;
	playerState.attackSoundFrame = playerState.attackSoundPlayed
		? -1
		: (range.firstFrame + kPlayerAttackSoundTriggerFrameOffset);
	playerState.attackContactResolved = false;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setAnimationRate(kRoomPlayerAttackAnimationRate);
	playerState.entity->setCurrentFrame(range.firstFrame);
	debugC(1, kDebugScene,
		"Harvester: player attack animation frames=%d..%d resume_facing=%d cursor=(%d,%d)",
		range.firstFrame, range.lastFrame, range.resumeFacing, mousePos.x, mousePos.y);
	return true;
}

bool Player::updateAttackAnimationState(HarvesterEngine &engine,
		StartupRoomPlayerState &playerState) {
	if (!playerState.attackActive || !playerState.entity)
		return false;
	if (!playerState.attackSoundPlayed &&
			playerState.attackSoundFrame >= 0 &&
			playerState.entity->getCurrentFrame() >= playerState.attackSoundFrame) {
		(void)playPlayerAttackSound(engine, playerState.combatLoadout);
		playerState.attackSoundPlayed = true;
	}
	if (playerState.entity->getCurrentFrame() != playerState.attackLastFrame)
		return false;

	const int resumeFacing = playerState.attackResumeFacing >= 0
		? playerState.attackResumeFacing
		: (playerState.facing >= 0 ? playerState.facing : 0);
	playerState.attackActive = false;
	playerState.attackFirstFrame = -1;
	playerState.attackLastFrame = -1;
	playerState.attackContactFrame = -1;
	playerState.attackResumeFacing = -1;
	playerState.attackSoundPlayed = false;
	playerState.attackSoundFrame = -1;
	playerState.attackContactResolved = false;
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	return setIdleAnimation(playerState, resumeFacing);
}

bool Player::startTurnAnimation(StartupRoomPlayerState &playerState, int targetFacing) {
	if (!playerState.entity || playerState.facing < 0 || playerState.facing == targetFacing)
		return false;

	PlayerTurnAnimationRange range;
	if (!resolvePlayerTurnAnimationRange(playerState.facing, targetFacing, range))
		return false;

	playerState.turnActive = true;
	playerState.turnTargetFacing = targetFacing;
	playerState.turnFirstFrame = range.firstFrame;
	playerState.turnLastFrame = range.lastFrame;
	playerState.turnEndFrame = range.playBackwards ? range.firstFrame : range.lastFrame;
	playerState.turnPlayBackwards = range.playBackwards;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setPlayBackwards(range.playBackwards);
	playerState.entity->setAnimationRate(kRoomPlayerWalkAnimationRate);
	playerState.entity->setCurrentFrame(range.playBackwards ? range.lastFrame : range.firstFrame);
	debugC(1, kDebugScene,
		"Harvester: player turn animation from=%d to=%d frames=%d..%d backwards=%d rate=%d",
		playerState.facing, targetFacing, range.firstFrame, range.lastFrame,
		range.playBackwards, kRoomPlayerWalkAnimationRate);
	return true;
}

bool Player::updateTurnAnimationState(StartupRoomPlayerState &playerState) {
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
	debugC(1, kDebugScene,
		"Harvester: player turn complete facing=%d frame=%d",
		playerState.facing, playerState.entity->getCurrentFrame());
	return true;
}

bool Player::stepMoveTarget(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState) {
	if (!playerState.entity || !playerState.hasMoveTarget || playerState.turnActive)
		return false;

	if (playerState.centerX == playerState.targetX &&
			fabsf(playerState.z - playerState.targetZ) <= kRoomDepthCompareEpsilon) {
		playerState.hasMoveTarget = false;
		return setIdleAnimation(playerState, playerState.facing >= 0 ? playerState.facing : 0);
	}

	const int previousCenterX = playerState.centerX;
	const float previousZ = playerState.z;
	const int targetBottomY = mapRoomDepthToScreenY(state, playerState.targetZ);
	const int moveFacing = resolveFacingFromMovementDelta(
		playerState.targetX - playerState.centerX, targetBottomY - playerState.bottomY);
	if (!consumePlayerMovementTick(playerState))
		return false;
	const int horizontalStep = computeRoomPlayerHorizontalStep(state, playerState.z);
	const float depthStep = computeRoomPlayerDepthStep(state);
	const int candidateCenterX = stepTowardsInt(playerState.centerX, playerState.targetX, horizontalStep);
	const float candidateZ = stepTowardsFloat(playerState.z, playerState.targetZ, depthStep);

	bool moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
		playerState, candidateCenterX, candidateZ);
	if (!moved && candidateCenterX != previousCenterX) {
		moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, previousZ);
	}
	if (!moved && fabsf(candidateZ - previousZ) > kRoomDepthCompareEpsilon) {
		moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, previousCenterX, candidateZ);
	}
	if (!moved) {
		playerState.hasMoveTarget = false;
		return setIdleAnimation(playerState, playerState.facing >= 0 ? playerState.facing : moveFacing);
	}

	const int actualFacing = resolveFacingFromRoomMovement(
		state, previousCenterX, previousZ, playerState.centerX, playerState.z);
	(void)setPlayerWalkAnimation(playerState, actualFacing);
	if (playerState.centerX == playerState.targetX &&
			fabsf(playerState.z - playerState.targetZ) <= kRoomDepthCompareEpsilon) {
		playerState.bottomY = mapRoomDepthToScreenY(state, playerState.targetZ);
		playerState.z = playerState.targetZ;
		playerState.hasMoveTarget = false;
		(void)setIdleAnimation(playerState, actualFacing);
	}
	debugC(playerState.hasMoveTarget ? 2 : 1, kDebugScene,
		"Harvester: player move step room='%s' pos=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f) facing=%d frame=%d active=%d moved=%d",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, mapRoomDepthToScreenY(state, playerState.targetZ), (double)playerState.targetZ,
		playerState.facing, playerState.entity->getCurrentFrame(), playerState.hasMoveTarget, moved);
	return true;
}

bool Player::stepKeyboardMovement(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState, bool moveLeft, bool moveRight, bool moveUp, bool moveDown) {
	if (!playerState.entity || !supportsMovementBand(state))
		return false;

	const int horizontalInput = (moveRight ? 1 : 0) - (moveLeft ? 1 : 0);
	const int verticalInput = (moveDown ? 1 : 0) - (moveUp ? 1 : 0);
	if (horizontalInput == 0 && verticalInput == 0)
		return false;
	if (playerState.turnActive)
		return false;

	const int previousCenterX = playerState.centerX;
	const float previousZ = playerState.z;
	const int horizontalStep = computeRoomPlayerHorizontalStep(state, playerState.z);
	const float depthStep = computeRoomPlayerDepthStep(state);
	int candidateCenterX = clampPlayerCenterXToNativeBounds(
		playerState, playerState.centerX + horizontalInput * horizontalStep);
	float candidateZ = playerState.z;
	if (verticalInput != 0) {
		const float positiveZ = clampRoomDepth(state, playerState.z + depthStep);
		const float negativeZ = clampRoomDepth(state, playerState.z - depthStep);
		const int positiveY = mapRoomDepthToScreenY(state, positiveZ);
		const int negativeY = mapRoomDepthToScreenY(state, negativeZ);
		if (verticalInput < 0) {
			if (positiveY < playerState.bottomY)
				candidateZ = positiveZ;
			else if (negativeY < playerState.bottomY)
				candidateZ = negativeZ;
		} else {
			if (positiveY > playerState.bottomY)
				candidateZ = positiveZ;
			else if (negativeY > playerState.bottomY)
				candidateZ = negativeZ;
		}
	}
	if (candidateCenterX == playerState.centerX &&
			fabsf(candidateZ - playerState.z) <= kRoomDepthCompareEpsilon) {
		return false;
	}

	const int desiredFacing = resolveFacingFromRoomMovement(
		state, playerState.centerX, playerState.z, candidateCenterX, candidateZ);
	if (desiredFacing != playerState.facing && startTurnAnimation(playerState, desiredFacing))
		return true;

	if (!consumePlayerMovementTick(playerState))
		return false;

	playerState.hasMoveTarget = false;
	bool moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
		playerState, candidateCenterX, candidateZ);
	if (!moved && candidateCenterX != previousCenterX) {
		moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, previousZ);
	}
	if (!moved && fabsf(candidateZ - previousZ) > kRoomDepthCompareEpsilon) {
		moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, previousCenterX, candidateZ);
	}
	if (!moved)
		return false;

	const int actualFacing = resolveFacingFromRoomMovement(
		state, previousCenterX, previousZ, playerState.centerX, playerState.z);
	(void)setPlayerWalkAnimation(playerState, actualFacing);
	debugC(2, kDebugScene,
		"Harvester: player keyboard move room='%s' input=(%d,%d) pos=(%d,%d,z=%.2f) facing=%d frame=%d moved=%d",
		state.roomName.c_str(), horizontalInput, verticalInput,
		playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.facing, playerState.entity->getCurrentFrame(), moved);
	return true;
}

bool Player::requestIdleAnimationExit(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState) {
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
	debugC(1, kDebugScene,
		"Harvester: player idle animation exit room='%s' frames=%d..%d rate=%d",
		state.roomName.c_str(), exitStartFrame, exitLastFrame, kRoomPlayerIdleAnimationRate);
	return true;
}

bool Player::startIdleAnimation(HarvesterEngine &engine, const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState) {
	if (!playerState.entity || idleState.active || isIdleAnimationExcludedRoom(state.roomName))
		return false;

	RuntimeEntity *idleEntity = ensurePlayerIdleAnimationEntity(engine, state, playerState, idleState);
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
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.entity->setVisible(false);
	debugC(1, kDebugScene,
		"Harvester: player idle animation start room='%s' facing=%d frames=%d..%d rate=%d trigger_tick=%u",
		state.roomName.c_str(), idleState.restoreFacing, 0, loopLastFrame,
		kRoomPlayerIdleAnimationRate, idleState.triggerTick);
	return true;
}

bool Player::updateIdleAnimation(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState) {
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
		debugC(1, kDebugScene,
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
