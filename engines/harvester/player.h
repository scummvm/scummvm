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
	static float computeDepthScale(const RoomSetupState &state, float z);
	static int resolveFacingFrame(int facing);
	static const char *describeCombatLoadout(int loadout);
	static const char *describeCombatDamageType(int damageType);
	static Common::String resolveCombatLoadoutResourcePath(int loadout);
	static int resolveCombatLoadoutDamageAmount(int loadout);
	static int resolveCombatLoadoutDamageType(int loadout);
	static int resolveCombatLoadoutContactFrameOffset(int loadout);
	static bool isProjectileCombatLoadout(int loadout);
	static bool supportsMovementBand(const RoomSetupState &state);
	static uint32 getRuntimeClockTicks();
	static bool isIdleAnimationExcludedRoom(const Common::String &roomName);
	static void updateIdleTrigger(RoomIdleAnimationState &idleState);
	static void setMoveTarget(const RoomSetupState &state, RoomPlayerState &playerState,
		int targetX, float targetZ);
	static void setRegionMoveTarget(const RoomSetupState &state, RoomPlayerState &playerState,
		int targetX, float targetZ);
	static void setMoveTargetFromScreenPoint(const RoomSetupState &state,
		RoomPlayerState &playerState, int targetX, int targetBottomY);
	static bool resolveBlockedStartupSpawn(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState);
	static int resolveRegionTargetX(const RoomSetupState &state, const RegionRecord &region,
		const RoomPlayerState &playerState);
	static float resolveRegionTargetZ(const RegionRecord &region);
	static bool syncCombatLoadoutVisual(HarvesterEngine &engine, const RoomSetupState &state,
		RoomPlayerState &playerState, int loadout);
	static bool setIdleAnimation(RoomPlayerState &playerState, int facing);
	static bool startAttackAnimation(const RoomSetupState &state,
		RoomPlayerState &playerState, const Common::Point &mousePos);
	static bool startKeyboardAttackAnimation(const RoomSetupState &state,
		RoomPlayerState &playerState, bool attackLeft, bool attackRight, bool attackUp, bool attackDown);
	static bool updateAttackAnimationState(HarvesterEngine &engine, RoomPlayerState &playerState);
	static bool startHitAnimation(HarvesterEngine &engine, RoomPlayerState &playerState,
		int monsterAttackFirstFrame);
	static bool updateHitAnimationState(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState);
	static bool startDeathAnimation(RoomPlayerState &playerState, int damageType, bool goreEnabled);
	static bool updateDeathAnimationState(RoomPlayerState &playerState);
	static bool startTurnAnimation(RoomPlayerState &playerState, int targetFacing);
	static bool updateTurnAnimationState(RoomPlayerState &playerState);
	static bool stepMoveTarget(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState);
	static bool stepKeyboardMovement(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState, bool moveLeft, bool moveRight, bool moveUp, bool moveDown);
	static bool requestIdleAnimationExit(const RoomSetupState &state,
		RoomPlayerState &playerState, RoomIdleAnimationState &idleState);
	static bool startIdleAnimation(HarvesterEngine &engine, const RoomSetupState &state,
		RoomPlayerState &playerState, RoomIdleAnimationState &idleState);
	static bool updateIdleAnimation(const RoomSetupState &state, RoomPlayerState &playerState,
		RoomIdleAnimationState &idleState);
};

} // End of namespace Harvester

#endif // HARVESTER_PLAYER_H
