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

#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"

namespace Harvester {

using namespace PlayerMovement;

namespace {

static const float kRoomPlayerHorizontalTargetSlackBase = 50.0f;
static const float kRoomPlayerDepthTargetSlack = 8.0f;
static const int kRoomRegionTargetXBias = 10;

static void resetPlayerMoveTargetProgress(RoomPlayerState &playerState) {
	playerState.moveTargetXReached = false;
	playerState.moveTargetZReached = false;
}

static int resolveMoveTargetHorizontalDirection(const RoomSetupState &state,
		const RoomPlayerState &playerState, int regionFacing) {
	const float slack = Player::computeDepthScale(state, computePlayerRenderZ(playerState)) *
		kRoomPlayerHorizontalTargetSlackBase;
	const float frameLeftX = (float)computePlayerFrameLeftX(playerState);
	if (frameLeftX < (float)playerState.targetX - slack || regionFacing == 2)
		return 1;
	if ((float)playerState.targetX + slack < frameLeftX || regionFacing == 1)
		return -1;

	return 0;
}

static int resolveMoveTargetDepthDirection(const RoomPlayerState &playerState, int regionFacing) {
	const float z = computePlayerRenderZ(playerState);
	if (playerState.targetZ < z - kRoomPlayerDepthTargetSlack || regionFacing == 0)
		return 1;
	if (z + kRoomPlayerDepthTargetSlack < playerState.targetZ || regionFacing == 3)
		return -1;

	return 0;
}

static bool hasPlayerReachedMoveTarget(const RoomPlayerState &playerState) {
	return playerState.moveTargetXReached && playerState.moveTargetZReached;
}

static void updatePlayerMoveTargetProgress(const RoomSetupState &state,
		RoomPlayerState &playerState, int regionFacing) {
	if (!playerState.moveTargetXReached &&
			resolveMoveTargetHorizontalDirection(state, playerState, regionFacing) == 0) {
		playerState.moveTargetXReached = true;
	}
	if (!playerState.moveTargetZReached &&
			resolveMoveTargetDepthDirection(playerState, regionFacing) == 0) {
		playerState.moveTargetZReached = true;
	}
}

static bool isHorizontalDirectionBlockedByHistory(const RoomPlayerState &playerState,
		int horizontalDirection) {
	if (horizontalDirection < 0)
		return !playerState.leftwardBlockerHistory.empty();
	if (horizontalDirection > 0)
		return !playerState.rightwardBlockerHistory.empty();
	return false;
}

static bool isDepthDirectionBlockedByHistory(const RoomPlayerState &playerState,
		int depthDirection) {
	if (depthDirection > 0)
		return !playerState.upwardBlockerHistory.empty();
	if (depthDirection < 0)
		return !playerState.downwardBlockerHistory.empty();
	return false;
}

static const char *describeBlockedDirection(int horizontalDirection, int depthDirection) {
	if (horizontalDirection < 0)
		return "left";
	if (horizontalDirection > 0)
		return "right";
	if (depthDirection > 0)
		return "upward";
	if (depthDirection < 0)
		return "downward";
	return "";
}

static void setMoveTargetInternal(const RoomSetupState &state, RoomPlayerState &playerState,
		int targetX, float targetZ, int targetBottomY) {
	playerState.hasMoveTarget = true;
	playerState.nextMovementTick = 0;
	playerState.targetX = CLIP<int>(targetX, 0, 639);
	playerState.targetBottomY = clampRoomMovementY(state, targetBottomY);
	playerState.targetZ = targetZ;
	resetPlayerMoveTargetProgress(playerState);
	debugC(1, kDebugPlayer,
		"Harvester: player move target room='%s' current=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f)",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ);
	if (isPathfindingDebugEnabled()) {
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding target set room='%s' movement_band=(y=%d..%d,z=%d..%d) current=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f)",
			state.roomName.c_str(), state.roomMaxZScreenY, state.roomMinZScreenY,
			state.roomMinZ, state.roomMaxZ,
			playerState.centerX, playerState.bottomY, (double)playerState.z,
			playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ);
	}
}

struct TargetWalkDirection {
	int horizontal;
	int depth;
};

static TargetWalkDirection resolveTargetWalkDirection(HarvesterEngine &engine, const RoomSetupState &state,
		const RoomPlayerState &playerState, int regionFacing, bool updateTarget) {
	int horizontalDirection = updateTarget
		? (playerState.moveTargetXReached ? 0 : resolveMoveTargetHorizontalDirection(state, playerState, regionFacing))
		: playerState.walkHorizontalDirection;
	if (horizontalDirection != 0 && isHorizontalDirectionBlockedByHistory(playerState, horizontalDirection)) {
		if (engine.isPathfindingDebugEnabled()) {
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding direction held by blocker history room='%s' direction='%s' history='%s'",
				state.roomName.c_str(),
				describeBlockedDirection(horizontalDirection, 0),
				horizontalDirection < 0
					? playerState.leftwardBlockerHistory.c_str()
					: playerState.rightwardBlockerHistory.c_str());
		}
		horizontalDirection = 0;
	}
	int depthDirection = updateTarget
		? (playerState.moveTargetZReached ? 0 : resolveMoveTargetDepthDirection(playerState, regionFacing))
		: playerState.walkDepthDirection;
	if (depthDirection != 0 && isDepthDirectionBlockedByHistory(playerState, depthDirection)) {
		if (engine.isPathfindingDebugEnabled()) {
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding direction held by blocker history room='%s' direction='%s' history='%s'",
				state.roomName.c_str(),
				describeBlockedDirection(0, depthDirection),
				depthDirection > 0
					? playerState.upwardBlockerHistory.c_str()
					: playerState.downwardBlockerHistory.c_str());
		}
		depthDirection = 0;
	}
	return { horizontalDirection, depthDirection };
}

static MovementPosition computeMoveTargetStep(const RoomSetupState &state,
		const RoomPlayerState &playerState, const TargetWalkDirection &direction) {
	int candidateCenterX = playerState.centerX;
	const int candidateBottomY = playerState.bottomY;
	float candidateZ = playerState.z;
	if (direction.horizontal != 0) {
		const int horizontalStep = computeRoomPlayerHorizontalStep(state, computePlayerRenderZ(playerState));
		candidateCenterX = clampPlayerCenterXToNativeBounds(
			playerState, playerState.centerX + direction.horizontal * horizontalStep);
		candidateZ = clampPlayerMovementDepth(state, playerState, playerState.z - direction.depth);
	} else if (direction.depth != 0) {
		const float depthStep = computeRoomPlayerDepthStep(state);
		const bool depthBoundaryReached = direction.depth < 0
			? computePlayerRenderZ(playerState) >= state.roomMaxZ
			: computePlayerRenderZ(playerState) <= state.roomMinZ;
		if (!depthBoundaryReached) {
			candidateZ = clampPlayerMovementDepth(state, playerState, playerState.z - direction.depth * depthStep);
		}
	}

	return { candidateCenterX, candidateBottomY, candidateZ };
}

static void logAppliedMoveTargetStep(const MovementContext &context,
		int previousCenterX, int previousBottomY, int actualFacing, bool moved) {
	HarvesterEngine &engine = context.engine;
	const RoomSetupState &state = context.state;
	const RoomPlayerState &playerState = context.playerState;
	if (engine.isPathfindingDebugEnabled()) {
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding applied room='%s' previous=(%d,%d) current=(%d,%d,z=%.2f) facing=%d",
			state.roomName.c_str(), previousCenterX, previousBottomY,
			playerState.centerX, playerState.bottomY, (double)playerState.z,
			actualFacing);
	}
	debugC(playerState.hasMoveTarget ? 2 : 1, kDebugPlayer,
		"Harvester: player move step room='%s' pos=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f) facing=%d frame=%d active=%d moved=%d",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ,
		playerState.facing, playerState.entity->getCurrentFrame(),
		playerState.hasMoveTarget, moved);
}

} // End of anonymous namespace

void Player::setMoveTarget(const RoomSetupState &state, int targetX, float targetZ) {
	RoomPlayerState &playerState = _state;
	setMoveTargetInternal(state, playerState, targetX, clampRoomDepth(state, targetZ),
		mapRoomDepthToScreenY(state, targetZ));
}

void Player::setRegionMoveTarget(const RoomSetupState &state, int targetX, float targetZ) {
	RoomPlayerState &playerState = _state;
	setMoveTargetInternal(state, playerState, targetX, targetZ,
		mapRoomDepthToScreenY(state, targetZ));
}

void Player::setMoveTargetFromScreenPoint(const RoomSetupState &state, int targetX, int targetBottomY) {
	RoomPlayerState &playerState = _state;
	const int clampedTargetBottomY = clampRoomMovementY(state, targetBottomY);
	setMoveTargetInternal(state, playerState, targetX,
		mapRoomScreenYToDepth(state, clampedTargetBottomY), clampedTargetBottomY);
}

int Player::resolveRegionTargetX(const RoomSetupState &state, const RegionRecord &region,
		const RoomPlayerState &playerState) {
	const Common::Rect bounds(region.left, region.top, region.right, region.bottom);
	int targetX = bounds.left + bounds.width() / 2;
	if (!playerState.entity)
		return CLIP<int>(targetX, 0, 639);

	const int frameLeftX = computePlayerFrameLeftX(playerState);
	if (targetX + kRoomRegionTargetXBias < frameLeftX)
		targetX -= kRoomRegionTargetXBias;

	return CLIP<int>(targetX, 0, 639);
}

float Player::resolveRegionTargetZ(const RegionRecord &region) {
	if (region.desiredFacing == 0)
		return (float)region.maxZ;
	if (region.desiredFacing == 3)
		return (float)region.minZ;

	return (float)(region.minZ + (region.maxZ - region.minZ) / 2);
}

bool Player::stepMoveTarget(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Array<AnimRecord> &sceneAnimations,
		int regionFacing) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity || !playerState.hasMoveTarget || playerState.turnActive ||
			playerState.hitActive || playerState.evasiveState >= 0)
		return false;

	const MovementContext movementContext = { engine, state, sceneObjects, sceneAnimations, playerState };
	refreshPlayerCurrentBlocker(movementContext);

	const PlayerAnimationRange range = resolvePlayerAnimationRange(playerState.facing);
	const int currentFrame = playerState.entity->getCurrentFrame();
	const bool walking = playerState.entity->getAnimationRate() == playerState.animationRate &&
		currentFrame >= range.walkFirstFrame && currentFrame <= range.walkLastFrame;
	const bool updateTarget = !walking || currentFrame == range.walkLastFrame;
	if (updateTarget)
		updatePlayerMoveTargetProgress(state, playerState, regionFacing);
	if (hasPlayerReachedMoveTarget(playerState)) {
		playerState.hasMoveTarget = false;
		return setIdleAnimation(playerState.facing >= 0 ? playerState.facing : 0);
	}

	const int previousCenterX = playerState.centerX;
	const int previousBottomY = playerState.bottomY;
	const TargetWalkDirection direction = resolveTargetWalkDirection(
		engine, state, playerState, regionFacing, updateTarget);
	if (direction.horizontal == 0 && direction.depth == 0) {
		playerState.hasMoveTarget = false;
		return setIdleAnimation(playerState.facing >= 0 ? playerState.facing : 0);
	}
	const int desiredFacing = resolveFacingFromMoveDirection(
		direction.horizontal, direction.depth, playerState.facing);
	if (regionFacing >= 0 && desiredFacing != playerState.facing &&
			startTurnAnimation(desiredFacing)) {
		debugC(2, kDebugPlayer,
			"Harvester: player region target turn room='%s' required_facing=%d movement_facing=%d target=(%d,z=%.2f)",
			state.roomName.c_str(), regionFacing, desiredFacing,
			playerState.targetX, (double)playerState.targetZ);
		return true;
	}
	if (!consumePlayerMovementTick(playerState))
		return false;

	const MovementPosition candidate = computeMoveTargetStep(state, playerState, direction);

	if (engine.isPathfindingDebugEnabled()) {
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding step room='%s' current=(%d,%d,z=%.2f) target=(x=%d,y=%d,z=%.2f) progress=(x=%d,z=%d) direction=(x=%d,z=%d) candidate=(%d,%d,z=%.2f)",
			state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
			playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ,
			playerState.moveTargetXReached, playerState.moveTargetZReached,
			direction.horizontal, direction.depth,
			candidate.centerX, candidate.bottomY, (double)candidate.z);
	}

	const Entity *blocker = nullptr;
	bool moved = tryApplyPlayerWalkMovement(movementContext, candidate,
		direction.horizontal, direction.depth, &blocker);
	if (!moved)
		return stopMoveTargetStep(movementContext, candidate, direction.horizontal, direction.depth, blocker);

	const int actualFacing = desiredFacing;
	playerState.walkHorizontalDirection = direction.horizontal;
	playerState.walkDepthDirection = direction.depth;
	(void)setPlayerWalkAnimation(playerState, actualFacing);
	if (walking && currentFrame == range.walkLastFrame) {
		playerState.entity->setCurrentFrame(resolvePlayerAnimationRange(actualFacing).walkFirstFrame);
		debugC(2, kDebugPlayer,
			"Harvester: player target walk bank restart facing=%d frame=%d render_z=%.2f direction=(%d,%d)",
			actualFacing, playerState.entity->getCurrentFrame(), (double)computePlayerRenderZ(playerState),
			direction.horizontal, direction.depth);
	}
	syncAnimationPlacement();
	refreshPlayerCurrentBlocker(movementContext);
	logAppliedMoveTargetStep(movementContext, previousCenterX, previousBottomY, actualFacing, moved);
	return true;
}

bool Player::stopMoveTargetStep(const PlayerMovement::MovementContext &context,
		const PlayerMovement::MovementPosition &candidate, int horizontalDirection, int depthDirection,
		const Entity *blocker) {
	HarvesterEngine &engine = context.engine;
	const RoomSetupState &state = context.state;
	RoomPlayerState &playerState = _state;
	if (engine.isPathfindingDebugEnabled()) {
		const Common::Rect blockerRect = blocker ? blocker->getScreenRect() : Common::Rect();
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding blocked room='%s' candidate=(%d,%d,z=%.2f) blocker='%s' class=0x%x rect=(%d,%d)-(%d,%d) z=%.2f z_extent=%.2f",
			state.roomName.c_str(), candidate.centerX, candidate.bottomY, (double)candidate.z,
			blocker ? blocker->getName().c_str() : "",
			blocker ? blocker->getClassId() : -1,
			blockerRect.left, blockerRect.top, blockerRect.right, blockerRect.bottom,
			blocker ? (double)blocker->getZ() : 0.0,
			blocker ? (double)blocker->getZExtent() : 0.0);
	}
	setPlayerCurrentBlocker(playerState, blocker);
	if (blocker) {
		return setIdleAnimation(resolveFacingFromMoveDirection(horizontalDirection, depthDirection, playerState.facing));
	}

	playerState.hasMoveTarget = false;
	int idleFacing = playerState.facing;
	if (idleFacing < 0) {
		if (horizontalDirection != 0)
			idleFacing = horizontalDirection < 0 ? 1 : 2;
		else
			idleFacing = depthDirection < 0 ? 3 : 0;
	}
	return setIdleAnimation(idleFacing);
}

} // End of namespace Harvester
