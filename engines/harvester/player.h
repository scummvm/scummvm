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

#ifndef HARVESTER_PLAYER_H
#define HARVESTER_PLAYER_H

#include "harvester/room_support.h"

namespace Harvester {

class Player {
public:
	static float computeDepthScale(const StartupRoomSetupState &state, float z);
	static int resolveFacingFrame(int facing);
	static Common::String resolveCombatLoadoutResourcePath(int loadout);
	static int resolveCombatLoadoutDamageAmount(int loadout);
	static int resolveCombatLoadoutDamageType(int loadout);
	static int resolveCombatLoadoutContactFrameOffset(int loadout);
	static bool isProjectileCombatLoadout(int loadout);
	static bool supportsMovementBand(const StartupRoomSetupState &state);
	static uint32 getRuntimeClockTicks();
	static bool isIdleAnimationExcludedRoom(const Common::String &roomName);
	static void updateIdleTrigger(StartupRoomIdleAnimationState &idleState);
	static void setMoveTarget(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
		int targetX, float targetZ);
	static void setMoveTargetFromScreenPoint(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, int targetX, int targetBottomY);
	static int resolveRegionTargetX(const StartupRegionRecord &region,
		const StartupRoomPlayerState &playerState);
	static float resolveRegionTargetZ(const StartupRegionRecord &region);
	static bool syncCombatLoadoutVisual(HarvesterEngine &engine, const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, int loadout);
	static bool setIdleAnimation(StartupRoomPlayerState &playerState, int facing);
	static bool startAttackAnimation(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, const Common::Point &mousePos);
	static bool startKeyboardAttackAnimation(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, bool attackLeft, bool attackRight, bool attackUp, bool attackDown);
	static bool updateAttackAnimationState(HarvesterEngine &engine, StartupRoomPlayerState &playerState);
	static bool startTurnAnimation(StartupRoomPlayerState &playerState, int targetFacing);
	static bool updateTurnAnimationState(StartupRoomPlayerState &playerState);
	static bool stepMoveTarget(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState);
	static bool stepKeyboardMovement(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState, bool moveLeft, bool moveRight, bool moveUp, bool moveDown);
	static bool requestIdleAnimationExit(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState);
	static bool startIdleAnimation(HarvesterEngine &engine, const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState);
	static bool updateIdleAnimation(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
		StartupRoomIdleAnimationState &idleState);
};

} // End of namespace Harvester

#endif // HARVESTER_PLAYER_H
