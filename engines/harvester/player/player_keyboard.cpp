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

namespace Harvester {

using namespace PlayerMovement;

namespace {

static bool resolveKeyboardWalkInput(RoomPlayerState &playerState, const PlayerAnimationRange &range,
		int currentFrame, bool keyboardWalking, int &horizontalInput, int &verticalInput) {
	if (keyboardWalking) {
		if (horizontalInput != 0 || verticalInput != 0)
			playerState.keyboardWalkRequest = resolveFacingFromMoveDirection(
				horizontalInput, verticalInput, playerState.facing);
		if (!playerState.entity->didAnimationAdvanceLastTick())
			return false;
		const int stepFacing = currentFrame < range.walkLastFrame
			? playerState.facing : playerState.keyboardWalkRequest;
		horizontalInput = stepFacing == 1 ? -1 : stepFacing == 2 ? 1 : 0;
		verticalInput = stepFacing == 3 ? -1 : stepFacing == 0 ? 1 : 0;
	} else {
		playerState.keyboardWalkRequest = -1;
	}
	return true;
}

struct KeyboardWalkStep {
	KeyboardWalkStep(const MovementPosition &candidate, bool depthBoundaryReached)
		: candidate(candidate), depthBoundaryReached(depthBoundaryReached) {}

	MovementPosition candidate;
	bool depthBoundaryReached;
};

static KeyboardWalkStep computeKeyboardWalkStep(const RoomSetupState &state, const RoomPlayerState &playerState,
		int horizontalInput, int verticalInput) {
	const int horizontalStep = computeRoomPlayerHorizontalStep(state, computePlayerRenderZ(playerState));
	const float depthStep = computeRoomPlayerDepthStep(state);
	int candidateCenterX = clampPlayerCenterXToNativeBounds(
		playerState, playerState.centerX + horizontalInput * horizontalStep);
	const int candidateBottomY = playerState.bottomY;
	float candidateZ = playerState.z;
	bool depthBoundaryReached = false;
	if (verticalInput != 0) {

		depthBoundaryReached = verticalInput < 0
			? computePlayerRenderZ(playerState) >= state.roomMaxZ
			: computePlayerRenderZ(playerState) <= state.roomMinZ;
		if (!depthBoundaryReached) {
			candidateZ = clampPlayerMovementDepth(state, playerState, playerState.z - verticalInput * depthStep);
		}
	}
	return { { candidateCenterX, candidateBottomY, candidateZ }, depthBoundaryReached };
}

} // End of anonymous namespace

bool Player::stepKeyboardMovement(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Array<AnimRecord> &sceneAnimations,
		bool moveLeft, bool moveRight, bool moveUp, bool moveDown) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity || !supportsMovementBand(state))
		return false;
	if (playerState.turnActive || playerState.hitActive || playerState.evasiveState >= 0)
		return false;

	const MovementContext movementContext = { engine, state, sceneObjects, sceneAnimations, playerState };
	int horizontalInput = (moveRight ? 1 : 0) - (moveLeft ? 1 : 0);
	int verticalInput = (moveDown ? 1 : 0) - (moveUp ? 1 : 0);
	const PlayerAnimationRange range = resolvePlayerAnimationRange(playerState.facing);
	const int currentFrame = playerState.entity->getCurrentFrame();
	const bool keyboardWalking = !playerState.hasMoveTarget &&
		playerState.entity->getAnimationRate() == playerState.animationRate &&
		currentFrame >= range.walkFirstFrame && currentFrame <= range.walkLastFrame;
	if (!resolveKeyboardWalkInput(playerState, range, currentFrame, keyboardWalking, horizontalInput, verticalInput))
		return false;
	if (horizontalInput == 0 && verticalInput == 0) {
		if (playerState.hasMoveTarget || playerState.facing < 0)
			return false;
		return setIdleAnimation(playerState.facing);
	}

	const KeyboardWalkStep step = computeKeyboardWalkStep(state, playerState, horizontalInput, verticalInput);
	if (step.candidate.centerX == playerState.centerX &&
			step.candidate.bottomY == playerState.bottomY &&
			fabsf(step.candidate.z - playerState.z) <= kRoomDepthCompareEpsilon) {
		playerState.hasMoveTarget = false;
		if (step.depthBoundaryReached) {
			const bool stopped = setIdleAnimation(playerState.facing >= 0 ? playerState.facing : 0);
			if (stopped) {
				debugC(3, kDebugPathfinding,
					"Harvester: player keyboard depth boundary room='%s' input_y=%d pos=(%d,%d,z=%.2f) bounds=(%d,%d)",
					state.roomName.c_str(), verticalInput,
					playerState.centerX, playerState.bottomY, (double)playerState.z,
					state.roomMinZ, state.roomMaxZ);
			}
			return stopped;
		}
		return setIdleAnimation(playerState.facing >= 0 ? playerState.facing : 0);
	}

	const int desiredFacing = resolveFacingFromMoveDirection(
		horizontalInput, verticalInput, playerState.facing);
	if (desiredFacing != playerState.facing && startTurnAnimation(desiredFacing)) {
		playerState.keyboardWalkRequest = -1;
		return true;
	}

	if (!consumePlayerMovementTick(playerState))
		return false;

	playerState.hasMoveTarget = false;
	bool moved = tryApplyPlayerWalkMovement(movementContext,
		step.candidate,
		horizontalInput, verticalInput);
	if (!moved)
		return setIdleAnimation(playerState.facing >= 0 ? playerState.facing : 0);

	const int actualFacing = desiredFacing;
	playerState.walkHorizontalDirection = actualFacing == 1 ? -1 : actualFacing == 2 ? 1 : 0;
	playerState.walkDepthDirection = actualFacing == 3 ? -1 : actualFacing == 0 ? 1 : 0;
	(void)setPlayerWalkAnimation(playerState, actualFacing);
	if (keyboardWalking && currentFrame == range.walkLastFrame) {
		playerState.keyboardWalkRequest = -1;
		playerState.entity->setCurrentFrame(resolvePlayerAnimationRange(actualFacing).walkFirstFrame);
		debugC(2, kDebugPlayer,
			"Harvester: player keyboard walk bank restart facing=%d frame=%d",
			actualFacing, playerState.entity->getCurrentFrame());
	}
	syncAnimationPlacement();
	debugC(2, kDebugPlayer,
		"Harvester: player keyboard move room='%s' input=(%d,%d) step=(%d,%d) pos=(%d,%d,z=%.2f) facing=%d frame=%d moved=%d",
		state.roomName.c_str(), (moveRight ? 1 : 0) - (moveLeft ? 1 : 0),
		(moveDown ? 1 : 0) - (moveUp ? 1 : 0), horizontalInput, verticalInput,
		playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.facing, playerState.entity->getCurrentFrame(), moved);
	return true;
}

} // End of namespace Harvester
