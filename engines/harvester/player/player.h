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

#include "harvester/scene/room_support.h"

namespace Harvester {

class EntityManager;

namespace PlayerMovement {
struct MovementContext;
struct MovementPosition;
}

struct RoomPlayerState {
	Entity *entity = nullptr;
	int centerX = 0;
	int bottomY = 0;
	float z = 0.0f;
	int facing = -1;
	int combatLoadout = 0;
	int animationRate = 17;
	int evasiveState = -1;
	int evasiveFirstFrame = -1;
	int evasiveLastFrame = -1;
	int evasiveXStep = 0;
	Common::Rect evasiveFrameBounds;
	uint32 nextMovementTick = 0;
	int keyboardWalkRequest = -1;
	int walkHorizontalDirection = 0;
	int walkDepthDirection = 0;
	bool hasMoveTarget = false;
	int targetX = 0;
	int targetBottomY = 0;
	float targetZ = 0.0f;
	bool moveTargetXReached = false;
	bool moveTargetZReached = false;
	Common::String currentBlockerName;

	Common::String upwardBlockerHistory;
	Common::String downwardBlockerHistory;
	Common::String leftwardBlockerHistory;
	Common::String rightwardBlockerHistory;
	bool turnActive = false;
	int turnTargetFacing = -1;
	int turnFirstFrame = -1;
	int turnLastFrame = -1;
	int turnEndFrame = -1;
	bool turnPlayBackwards = false;
	bool attackActive = false;
	int attackFirstFrame = -1;
	int attackLastFrame = -1;
	int attackContactFrame = -1;
	int attackResumeFacing = -1;
	bool attackSoundPlayed = false;
	int attackSoundPlaybackFrame = -1;
	bool attackContactResolved = false;

	Common::String attackTargetName;
	int attackTargetClassId = -1;
	bool hitActive = false;
	int hitFirstFrame = -1;
	int hitLastFrame = -1;
	int hitResumeFacing = -1;
	int hitKnockbackRemainingX = 0;
	int hitKnockbackDecayStep = 0;
	bool deathActive = false;
	int deathFirstFrame = -1;
	int deathLastFrame = -1;
	int deathDamageType = 0;
};

struct RoomIdleAnimationState {
	Entity *entity = nullptr;
	int restoreFacing = -1;
	uint32 activityTick = 0;
	uint32 resetTick = 0;
	uint32 triggerTick = 0;
	bool active = false;
	bool loopStarted = false;
	bool exiting = false;
};

class Player {
public:
	Player() = default;
	Player(const Player &) = delete;
	Player &operator=(const Player &) = delete;

	const RoomPlayerState &getState() const { return _state; }
	const RoomIdleAnimationState &getIdleState() const { return _idleState; }
	void initialize(const RoomSetupState &state, Entity *entity, int combatLoadout);
	bool isCombatLocked() const;
	void cancelMovement();
	void cancelMovementForAttack();
	bool cancelAttackAnimation();
	void clearAttackTarget();
	void captureAttackTarget(const Common::String &name, int classId);
	void markAttackContactResolved();
	void resetIdleState();
	void noteActivity();
	void cancelIdleAnimation();
	void applyGotoXZ(const RoomSetupState &state, EntityManager *entityManager, int x, int z);
	void rebindAfterSceneRefresh(HarvesterEngine &engine, const RoomSetupState &state,
		EntityManager *entityManager, bool preservePlayerPlacement);
	void syncAnimationPlacement();
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

	void setMoveTarget(const RoomSetupState &state, int targetX, float targetZ);
	void setRegionMoveTarget(const RoomSetupState &state, int targetX, float targetZ);
	void setMoveTargetFromScreenPoint(const RoomSetupState &state, int targetX, int targetBottomY);
	bool resolveBlockedStartupSpawn(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Array<AnimRecord> &sceneAnimations);
	static int resolveRegionTargetX(const RoomSetupState &state, const RegionRecord &region,
		const RoomPlayerState &playerState);
	static float resolveRegionTargetZ(const RegionRecord &region);
	void syncCombatLoadout(HarvesterEngine &engine, const RoomSetupState &state);
	bool syncCombatLoadoutVisual(HarvesterEngine &engine, const RoomSetupState &state, int loadout);
	bool setIdleAnimation(int facing);
	bool updateAnimationRate(bool speedUp);
	bool startEvasiveAnimation(bool moveLeft, bool moveRight, bool moveUp, bool moveDown);
	bool updateEvasiveAnimation(HarvesterEngine &engine);
	bool startAttackAnimation(const RoomSetupState &state, const Common::Point &mousePos);
	bool startKeyboardAttackAnimation(const RoomSetupState &state,
		bool attackLeft, bool attackRight, bool attackUp, bool attackDown);
	bool updateAttackAnimationState(HarvesterEngine &engine);
	bool startHitAnimation(HarvesterEngine &engine, int monsterAttackFirstFrame);
	bool updateHitAnimationState(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Array<AnimRecord> &sceneAnimations);
	bool startDeathAnimation(int damageType, bool goreEnabled);
	bool updateDeathAnimationState();
	bool startTurnAnimation(int targetFacing);
	bool updateTurnAnimationState();
	bool stepMoveTarget(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Array<AnimRecord> &sceneAnimations,
		int regionFacing = -1);
	bool stepKeyboardMovement(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Array<AnimRecord> &sceneAnimations,
		bool moveLeft, bool moveRight, bool moveUp, bool moveDown);
	bool requestIdleAnimationExit(const RoomSetupState &state);
	bool startIdleAnimation(HarvesterEngine &engine, const RoomSetupState &state);
	bool updateIdleAnimation(const RoomSetupState &state);

private:
	bool finishIdleAnimation(const RoomSetupState &state);

	bool stopMoveTargetStep(const PlayerMovement::MovementContext &context,
		const PlayerMovement::MovementPosition &candidate, int horizontalDirection, int depthDirection,
		const Entity *blocker);

	RoomPlayerState _state;
	RoomIdleAnimationState _idleState;
};

} // End of namespace Harvester

#endif // HARVESTER_PLAYER_H
