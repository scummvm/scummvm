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

#ifndef HARVESTER_PLAYER_MOVEMENT_H
#define HARVESTER_PLAYER_MOVEMENT_H

#include "harvester/player/player.h"

namespace Harvester {
namespace PlayerMovement {

struct MovementContext {
	HarvesterEngine &engine;
	const RoomSetupState &state;
	const Common::Array<ObjectRecord> &sceneObjects;
	const Common::Array<AnimRecord> &sceneAnimations;
	RoomPlayerState &playerState;
};

struct MovementPosition {
	int centerX;
	int bottomY;
	float z;
};

struct PlayerAnimationRange {
	PlayerAnimationRange() {}
	PlayerAnimationRange(int walkFirstFrame, int walkLastFrame, int idleFrame)
		: walkFirstFrame(walkFirstFrame), walkLastFrame(walkLastFrame), idleFrame(idleFrame) {}

	int walkFirstFrame = 0;
	int walkLastFrame = 0;
	int idleFrame = 0;
};

extern const float kRoomDepthCompareEpsilon;

PlayerAnimationRange resolvePlayerAnimationRange(int facing);
int clampRoomMovementY(const RoomSetupState &state, int screenY);
float mapRoomScreenYToDepth(const RoomSetupState &state, int screenY);
float clampRoomDepth(const RoomSetupState &state, float z);
int mapRoomDepthToScreenY(const RoomSetupState &state, float z);
int computeRoomPlayerHorizontalStep(const RoomSetupState &state, float z);
float computeRoomPlayerDepthStep(const RoomSetupState &state);
int computePlayerFrameLeftX(const RoomPlayerState &playerState);
float computePlayerRenderZ(const RoomPlayerState &playerState);
float clampPlayerMovementDepth(const RoomSetupState &state,
		const RoomPlayerState &playerState, float z);
int clampPlayerCenterXToNativeBounds(const RoomPlayerState &playerState, int centerX);
bool consumePlayerMovementTick(RoomPlayerState &playerState);
void setPlayerCurrentBlocker(RoomPlayerState &playerState, const Entity *blocker);
void refreshPlayerCurrentBlocker(const MovementContext &context);
void reinsertPlayerSceneEntity(HarvesterEngine &engine, const RoomPlayerState &playerState);
bool isPlayerMovementBlocked(const MovementContext &context, const MovementPosition &candidate,
		const Common::String *ignoredBlockerName = nullptr, const Entity **blockerOut = nullptr);
bool tryApplyPlayerMovement(const MovementContext &context, MovementPosition candidate,
		const Common::String *ignoredBlockerName = nullptr, const Entity **blockerOut = nullptr);
bool tryApplyPlayerWalkMovement(const MovementContext &context, const MovementPosition &candidate,
		int horizontalDirection, int depthDirection, const Entity **blockerOut = nullptr);
bool isPathfindingDebugEnabled();
void syncPlayerAnimationPlacement(RoomPlayerState &playerState);

int resolveFacingFromMoveDirection(int horizontalDirection, int depthDirection, int fallbackFacing);
bool setPlayerWalkAnimation(RoomPlayerState &playerState, int facing);

} // End of namespace PlayerMovement
} // End of namespace Harvester

#endif // HARVESTER_PLAYER_MOVEMENT_H
