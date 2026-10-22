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

static const int kRoomPlayerWalkAnimationRate = 17;
static const int kRoomPlayerFastAnimationRate = 30;

static int applyEvasiveFrameMovement(Entity &entity, const Common::Rect &previousBounds, int xStep) {
	const int dx = MIN(MAX(xStep, 20 - previousBounds.left), 619 - previousBounds.right);
	entity.setPosition(entity.getX() + dx, entity.getY(), entity.getZ());
	return dx;
}

struct PlayerTurnAnimationRange {
	PlayerTurnAnimationRange() {}
	PlayerTurnAnimationRange(int firstFrame, int lastFrame, bool playBackwards)
		: firstFrame(firstFrame), lastFrame(lastFrame), playBackwards(playBackwards) {}

	int firstFrame = 0;
	int lastFrame = 0;
	bool playBackwards = false;
};

static bool resolvePlayerTurnAnimationRange(int fromFacing, int toFacing, PlayerTurnAnimationRange &range) {
	// Desired-state transitions play these banks forward or backward depending on the source facing.
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

} // End of anonymous namespace

bool Player::setIdleAnimation(int facing) {
	RoomPlayerState &playerState = _state;
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
	if (!runtimeEntityHasFrameRange(*playerState.entity, firstFrame, lastFrame))
		return false;
	playerState.evasiveState = actorState;
	playerState.evasiveFirstFrame = firstFrame;
	playerState.evasiveLastFrame = lastFrame;
	playerState.evasiveXStep = backwards
		? (int)(playerState.walkHorizontalDirection * playerState.entity->getDepthScale() * 8.0f) : 0;
	playerState.walkHorizontalDirection = 0;
	playerState.walkDepthDirection = 0;
	playerState.nextMovementTick = 0;
	const Common::Rect previousBounds = playerState.entity->getScreenRect();
	playerState.entity->setAnimationFrameRange(firstFrame, lastFrame, false);
	playerState.entity->setPlayBackwards(backwards);
	playerState.entity->setAnimationRate(playerState.animationRate);
	playerState.entity->setCurrentFrame(backwards ? lastFrame : firstFrame, true);
	(void)applyEvasiveFrameMovement(*playerState.entity, previousBounds, playerState.evasiveXStep);
	playerState.evasiveFrameBounds = playerState.entity->getScreenRect();
	syncAnimationPlacement();
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
	const int dx = applyEvasiveFrameMovement(entity, playerState.evasiveFrameBounds, playerState.evasiveXStep);
	playerState.evasiveFrameBounds = entity.getScreenRect();
	syncAnimationPlacement();
	reinsertPlayerSceneEntity(engine, playerState);
	if (dx != 0) {
		debugC(3, kDebugPlayer, "Harvester: player evasive step state=0x%x frame=%d dx=%d",
			actorState, entity.getCurrentFrame(), dx);
	}
	const bool backwards = actorState == 0x08 || actorState == 0x0f;
	if (entity.getCurrentFrame() == (backwards ? playerState.evasiveFirstFrame : playerState.evasiveLastFrame)) {
		debugC(2, kDebugPlayer, "Harvester: player evasive complete state=0x%x facing=%d",
			actorState, playerState.facing);
		return setIdleAnimation(playerState.facing);
	}

	const int frameLeft = computePlayerFrameLeftX(playerState);
	if ((actorState == 0x08 && frameLeft > 20) || (actorState == 0x0f && frameLeft < 500)) {
		playerState.evasiveXStep = (int)(entity.getDepthScale() * (actorState == 0x08 ? -8.0f : 8.0f));
	} else if (entity.getCurrentFrame() > playerState.evasiveFirstFrame + 2 &&
			((actorState == 0x12 && frameLeft > 36) || (actorState == 0x13 && frameLeft < 616))) {
		playerState.evasiveXStep = (int)(entity.getDepthScale() * (actorState == 0x12 ? -16.0f : 16.0f));
	}
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

} // End of namespace Harvester
