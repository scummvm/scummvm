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

static const char *const kPlayerIdleAnimationEntityName = "IDLE_ANIM";
static const char *const kPlayerIdleAnimationResourcePath = "1:/GRAPHIC/ROOMANIM/PCLOUN02.ABM";
static const uint32 kRoomPlayerIdleDelayTicks = 3000;
static const int kRoomPlayerIdleAnimationRate = 14;
static const int kRoomPlayerIdleLoopStartFrame = 0x0f;
static const int kRoomPlayerIdleLoopLastFrame = 0xb2;
static const int kRoomPlayerIdleExitLastFrame = 0xbf;
static const int kRoomPlayerIdleYOffset = 4;

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


bool Player::setIdleAnimation(int facing) {
	return setPlayerIdleAnimation(_state, facing);
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
