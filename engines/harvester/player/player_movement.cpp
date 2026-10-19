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

#include "harvester/player/player_movement.h"

#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"

namespace Harvester {
namespace PlayerMovement {

static const float kRoomPlayerHorizontalMoveBase = 8.0f;
const float kRoomDepthCompareEpsilon = 0.01f;
static const int kRoomPlayerMinOpaqueLeftX = 4;
static const int kRoomPlayerMaxOpaqueRightX = 0x27c;

static int roundToInt(float value) {
	return value >= 0.0f ? (int)floorf(value + 0.5f) : (int)ceilf(value - 0.5f);
}

PlayerAnimationRange resolvePlayerAnimationRange(int facing) {
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

int clampRoomMovementY(const RoomSetupState &state, int screenY) {
	if (!Player::supportsMovementBand(state))
		return screenY;

	return CLIP<int>(screenY, state.roomMaxZScreenY, state.roomMinZScreenY);
}

float mapRoomScreenYToDepth(const RoomSetupState &state, int screenY) {
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

float clampRoomDepth(const RoomSetupState &state, float z) {
	if (!Player::supportsMovementBand(state))
		return z;

	return CLIP<float>(z,
		(float)MIN(state.roomMinZ, state.roomMaxZ),
		(float)MAX(state.roomMinZ, state.roomMaxZ));
}

int mapRoomDepthToScreenY(const RoomSetupState &state, float z) {
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

int computeRoomPlayerHorizontalStep(const RoomSetupState &state, float z) {
	return MAX<int>(1, (int)(Player::computeDepthScale(state, z) * kRoomPlayerHorizontalMoveBase));
}

float computeRoomPlayerDepthStep(const RoomSetupState &state) {
	return state.roomZVelocityStep > 0.0f ? state.roomZVelocityStep : 1.0f;
}

int computePlayerFrameLeftX(const RoomPlayerState &playerState) {
	return playerState.entity ? playerState.entity->getScreenRect().left : playerState.centerX;
}

float computePlayerRenderZ(const RoomPlayerState &playerState) {
	return playerState.entity ? playerState.entity->getZ() : playerState.z;
}

float clampPlayerMovementDepth(const RoomSetupState &state,
		const RoomPlayerState &playerState, float z) {
	const float placementOffset = playerState.z - computePlayerRenderZ(playerState);
	return clampRoomDepth(state, z - placementOffset) + placementOffset;
}

int clampPlayerCenterXToNativeBounds(const RoomPlayerState &playerState, int centerX) {
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

bool consumePlayerMovementTick(RoomPlayerState &playerState) {
	if (playerState.entity &&
			playerState.entity->isAnimationEnabled() &&
			playerState.entity->getAnimationRate() != 0) {
		playerState.nextMovementTick = 0;
		return playerState.entity->didAnimationAdvanceLastTick();
	}

	const uint32 interval = computeAnimationTickInterval(playerState.animationRate);
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

static bool shouldIgnoreMovementBlocker(const Entity &entity, const Common::String *ignoredBlockerName) {
	return ignoredBlockerName && !ignoredBlockerName->empty() &&
		entity.getName().equalsIgnoreCase(*ignoredBlockerName);
}

static bool isSameBlockerName(const Common::String &history, const Entity &blocker) {
	return !history.empty() && history.equalsIgnoreCase(blocker.getName());
}

static void clearPlayerBlockerHistory(RoomPlayerState &playerState) {
	playerState.currentBlockerName.clear();
	playerState.upwardBlockerHistory.clear();
	playerState.downwardBlockerHistory.clear();
	playerState.leftwardBlockerHistory.clear();
	playerState.rightwardBlockerHistory.clear();
}

void setPlayerCurrentBlocker(RoomPlayerState &playerState, const Entity *blocker) {
	if (blocker) {
		playerState.currentBlockerName = blocker->getName();
		return;
	}

	clearPlayerBlockerHistory(playerState);
}

static const Entity *findPlayerRoomBlocker(const MovementContext &context, const Entity &playerEntity,
		const Common::String *ignoredBlockerName) {
	EntityManager *entityManager = context.engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	for (const ObjectRecord &object : context.sceneObjects) {
		const Entity *entity = entityManager->findSceneEntityByName(object.objectName);
		if (entity && !shouldIgnoreMovementBlocker(*entity, ignoredBlockerName) &&
				playerEntity.overlapsEntity(*entity)) {
			return entity;
		}
	}

	for (const AnimRecord &anim : context.sceneAnimations) {
		const Entity *entity = entityManager->findSceneEntityByName(anim.animName);
		if (entity && !shouldIgnoreMovementBlocker(*entity, ignoredBlockerName) &&
				playerEntity.overlapsEntity(*entity)) {
			return entity;
		}
	}

	return nullptr;
}

void refreshPlayerCurrentBlocker(const MovementContext &context) {
	RoomPlayerState &playerState = context.playerState;
	if (!playerState.entity) {
		clearPlayerBlockerHistory(playerState);
		return;
	}

	setPlayerCurrentBlocker(playerState, findPlayerRoomBlocker(context, *playerState.entity, nullptr));
}

void reinsertPlayerSceneEntity(HarvesterEngine &engine, const RoomPlayerState &playerState) {
	if (!playerState.entity)
		return;

	EntityManager *entityManager = engine.getRuntimeEntities();
	if (entityManager &&
			entityManager->findSceneEntityByName(playerState.entity->getName()) == playerState.entity) {
		entityManager->reinsertSceneEntity(playerState.entity);
	}
}

static void applyPlayerMovementPlacement(const RoomSetupState &state, RoomPlayerState &playerState,
		int candidateCenterX, int candidateBottomY, float candidateZ) {
	Entity &entity = *playerState.entity;
	const float renderZ = candidateZ - floorf(MAX<float>(entity.getZExtent(), 0.0f) * 0.5f);
	entity.setPosition(entity.getX() + candidateCenterX - playerState.centerX,
		entity.getY() + candidateBottomY - playerState.bottomY, renderZ);
	entity.setDepthScale(Player::computeDepthScale(state, renderZ));
}

bool isPlayerMovementBlocked(const MovementContext &context, const MovementPosition &candidate,
		const Common::String *ignoredBlockerName, const Entity **blockerOut) {
	const RoomSetupState &state = context.state;
	RoomPlayerState &playerState = context.playerState;
	if (!playerState.entity)
		return true;

	if (blockerOut)
		*blockerOut = nullptr;

	Entity &entity = *playerState.entity;
	const int oldX = entity.getX();
	const int oldY = entity.getY();
	const float oldZ = entity.getZ();
	const float oldScale = entity.getDepthScale();
	applyPlayerMovementPlacement(state, playerState, candidate.centerX, candidate.bottomY, candidate.z);

	const Entity *blocker = findPlayerRoomBlocker(context, entity, ignoredBlockerName);
	entity.setPosition(oldX, oldY, oldZ);
	entity.setDepthScale(oldScale);
	if (blockerOut)
		*blockerOut = blocker;
	return blocker != nullptr;
}

bool tryApplyPlayerMovement(const MovementContext &context, MovementPosition candidate,
		const Common::String *ignoredBlockerName, const Entity **blockerOut) {
	HarvesterEngine &engine = context.engine;
	const RoomSetupState &state = context.state;
	RoomPlayerState &playerState = context.playerState;
	if (!playerState.entity)
		return false;

	if (blockerOut)
		*blockerOut = nullptr;

	candidate.centerX = clampPlayerCenterXToNativeBounds(playerState, candidate.centerX);
	candidate.z = clampRoomDepth(state, candidate.z);
	if (candidate.centerX == playerState.centerX &&
			candidate.bottomY == playerState.bottomY &&
			fabsf(candidate.z - playerState.z) <= kRoomDepthCompareEpsilon) {
		return false;
	}
	if (isPlayerMovementBlocked(context, candidate, ignoredBlockerName, blockerOut)) {
		return false;
	}

	applyPlayerMovementPlacement(state, playerState, candidate.centerX, candidate.bottomY, candidate.z);
	playerState.z = candidate.z;
	syncPlayerAnimationPlacement(playerState);

	reinsertPlayerSceneEntity(engine, playerState);
	return true;
}

bool isPathfindingDebugEnabled() {
	return g_engine && g_engine->isPathfindingDebugEnabled();
}

static void logPlayerBlockerHistory(const RoomSetupState &state,
		const RoomPlayerState &playerState, const Entity &blocker,
		const char *direction) {
	if (!isPathfindingDebugEnabled())
		return;

	debugC(1, kDebugPathfinding,
		"Harvester: pathfinding blocker history room='%s' blocker='%s' direction='%s' histories=(up='%s',down='%s',left='%s',right='%s')",
		state.roomName.c_str(), blocker.getName().c_str(), direction,
		playerState.upwardBlockerHistory.c_str(),
		playerState.downwardBlockerHistory.c_str(),
		playerState.leftwardBlockerHistory.c_str(),
		playerState.rightwardBlockerHistory.c_str());
}

static bool recordNativePrimaryWalkBlockerHistory(const RoomSetupState &state,
		RoomPlayerState &playerState, const Entity &blocker,
		int horizontalDirection, int depthDirection) {
	if (horizontalDirection < 0) {
		const int frameLeftX = computePlayerFrameLeftX(playerState);
		const Common::Rect blockerRect = blocker.getScreenRect();
		if (isSameBlockerName(playerState.rightwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.upwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.downwardBlockerHistory, blocker) ||
				frameLeftX <= blockerRect.left) {
			return false;
		}

		playerState.leftwardBlockerHistory = blocker.getName();
		playerState.moveTargetXReached = true;
		logPlayerBlockerHistory(state, playerState, blocker, "left");
		return true;
	}

	if (horizontalDirection > 0) {
		const int frameLeftX = computePlayerFrameLeftX(playerState);
		const Common::Rect blockerRect = blocker.getScreenRect();
		if (isSameBlockerName(playerState.leftwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.upwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.downwardBlockerHistory, blocker) ||
				blockerRect.left <= frameLeftX) {
			return false;
		}

		playerState.rightwardBlockerHistory = blocker.getName();
		playerState.moveTargetXReached = true;
		logPlayerBlockerHistory(state, playerState, blocker, "right");
		return true;
	}

	if (depthDirection > 0) {
		if (isSameBlockerName(playerState.rightwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.leftwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.downwardBlockerHistory, blocker)) {
			return false;
		}

		playerState.upwardBlockerHistory = blocker.getName();
		playerState.moveTargetZReached = true;
		logPlayerBlockerHistory(state, playerState, blocker, "upward");
		return true;
	}

	if (depthDirection < 0) {
		if (isSameBlockerName(playerState.rightwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.upwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.leftwardBlockerHistory, blocker)) {
			return false;
		}

		playerState.downwardBlockerHistory = blocker.getName();
		playerState.moveTargetZReached = true;
		logPlayerBlockerHistory(state, playerState, blocker, "downward");
		return true;
	}

	return false;
}

static void playPlayerBlockedWalkSounds(HarvesterEngine &engine, const RoomSetupState &state,
		const RoomPlayerState &playerState, const Entity &blocker) {
	const Entity &entity = *playerState.entity;
	const PlayerAnimationRange range = resolvePlayerAnimationRange(playerState.facing);
	if (entity.getAnimationRate() != playerState.animationRate ||
			entity.getCurrentFrame() < range.walkFirstFrame ||
			entity.getCurrentFrame() > range.walkLastFrame)
		return;

	const int frameLeftX = computePlayerFrameLeftX(playerState);
	if ((playerState.facing == 1 && frameLeftX < 30) ||
			(playerState.facing == 2 && frameLeftX >= 611) ||
			(playerState.facing == 0 && entity.getZ() <= state.roomMinZ) ||
			(playerState.facing == 3 && entity.getZ() >= state.roomMaxZ))
		return;

	(void)engine.playSound("2:/SOUND/EFFECTS/FOOTLFT1.WAV");
	(void)engine.playSound("2:/SOUND/EFFECTS/FOOTRHT1.WAV");
	debugC(2, kDebugPlayer, "Harvester: player blocked walk sound blocker='%s' facing=%d",
		blocker.getName().c_str(), playerState.facing);
}

bool tryApplyPlayerWalkMovement(const MovementContext &context, const MovementPosition &candidate,
		int horizontalDirection, int depthDirection, const Entity **blockerOut) {
	HarvesterEngine &engine = context.engine;
	const RoomSetupState &state = context.state;
	RoomPlayerState &playerState = context.playerState;
	const Entity *blocker = findPlayerRoomBlocker(context, *playerState.entity, nullptr);
	setPlayerCurrentBlocker(playerState, blocker);
	if (blockerOut)
		*blockerOut = nullptr;
	if (blocker && recordNativePrimaryWalkBlockerHistory(state, playerState, *blocker,
			horizontalDirection, depthDirection)) {
		playPlayerBlockedWalkSounds(engine, state, playerState, *blocker);
		if (blockerOut)
			*blockerOut = blocker;
		return false;
	}

	if (candidate.centerX == playerState.centerX &&
			candidate.bottomY == playerState.bottomY &&
			fabsf(candidate.z - playerState.z) <= kRoomDepthCompareEpsilon)
		return false;

	applyPlayerMovementPlacement(state, playerState, candidate.centerX, candidate.bottomY, candidate.z);
	playerState.z = candidate.z;
	syncPlayerAnimationPlacement(playerState);
	reinsertPlayerSceneEntity(engine, playerState);
	return true;
}

void syncPlayerAnimationPlacement(RoomPlayerState &playerState) {
	if (!playerState.entity)
		return;

	const Common::Rect rect = playerState.entity->getScreenRect();
	const int centerX = rect.left + rect.width() / 2;
	if (playerState.centerX != centerX || playerState.bottomY != rect.bottom) {
		debugC(3, kDebugPlayer,
			"Harvester: player frame bounds frame=%d base=(%d,%d) rect=(%d,%d)-(%d,%d) z=%.2f",
			playerState.entity->getCurrentFrame(), playerState.entity->getX(), playerState.entity->getY(),
			rect.left, rect.top, rect.right, rect.bottom, (double)playerState.z);
	}
	playerState.centerX = centerX;
	playerState.bottomY = rect.bottom;
}

int resolveFacingFromMoveDirection(int horizontalDirection, int depthDirection,
		int fallbackFacing) {
	if (horizontalDirection < 0)
		return 1;
	if (horizontalDirection > 0)
		return 2;
	if (depthDirection < 0)
		return 3;
	if (depthDirection > 0)
		return 0;
	return fallbackFacing >= 0 ? fallbackFacing : 0;
}

bool setPlayerWalkAnimation(RoomPlayerState &playerState, int facing) {
	if (!playerState.entity)
		return false;

	const PlayerAnimationRange range = resolvePlayerAnimationRange(facing);
	const int currentFrame = playerState.entity->getCurrentFrame();
	const bool alreadyWalking = playerState.facing == facing &&
		playerState.entity->getAnimationRate() == playerState.animationRate &&
		currentFrame >= range.walkFirstFrame && currentFrame <= range.walkLastFrame;
	if (alreadyWalking)
		return false;

	playerState.facing = facing;
	playerState.entity->setAnimationFrameRange(range.walkFirstFrame, range.walkLastFrame, true);
	playerState.entity->setAnimationRate(playerState.animationRate);
	debugC(1, kDebugPlayer,
		"Harvester: player walk animation facing=%d frames=%d..%d rate=%d",
		facing, range.walkFirstFrame, range.walkLastFrame, playerState.animationRate);
	return true;
}

} // End of namespace PlayerMovement

using namespace PlayerMovement;

bool Player::resolveBlockedStartupSpawn(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Array<AnimRecord> &sceneAnimations) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity)
		return false;

	const MovementContext movementContext = { engine, state, sceneObjects, sceneAnimations, playerState };
	if (!isPlayerMovementBlocked(movementContext, { playerState.centerX, playerState.bottomY, playerState.z })) {
		return false;
	}

	const int originalCenterX = playerState.centerX;
	const int originalBottomY = playerState.bottomY;
	const int preferredDirection = playerState.facing == 1 ? -1 : 1;
	for (int distance = 1; distance < 640; ++distance) {
		int previousCandidateX = originalCenterX;
		for (int pass = 0; pass < 2; ++pass) {
			const int direction = pass == 0 ? preferredDirection : -preferredDirection;
			const int candidateX = clampPlayerCenterXToNativeBounds(
				playerState, originalCenterX + direction * distance);
			if (candidateX == originalCenterX || candidateX == previousCandidateX)
				continue;
			previousCandidateX = candidateX;
			if (isPlayerMovementBlocked(movementContext, { candidateX, playerState.bottomY, playerState.z })) {
				continue;
			}

			playerState.centerX = candidateX;
			if (!applyRoomActorPlacement(state, *playerState.entity,
					playerState.centerX, playerState.bottomY, playerState.z)) {
				return false;
			}
			reinsertPlayerSceneEntity(engine, playerState);

			debugC(1, kDebugPlayer,
				"Harvester: adjusted blocked startup spawn room='%s' facing=%d from=(%d,%d,z=%.2f) to=(%d,%d,z=%.2f)",
				state.roomName.c_str(), playerState.facing,
				originalCenterX, originalBottomY, (double)playerState.z,
				playerState.centerX, playerState.bottomY, (double)playerState.z);
			return true;
		}
	}

	debugC(1, kDebugPlayer,
		"Harvester: startup spawn remains blocked room='%s' pos=(%d,%d,z=%.2f)",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY,
		(double)playerState.z);
	return false;
}

} // End of namespace Harvester
