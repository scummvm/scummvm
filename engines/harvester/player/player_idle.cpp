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

#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"

namespace Harvester {

namespace {

static const char *const kPlayerIdleAnimationEntityName = "IDLE_ANIM";
static const char *const kPlayerIdleAnimationResourcePath = "1:/GRAPHIC/ROOMANIM/PCLOUN02.ABM";
static const uint32 kRoomPlayerIdleDelayTicks = 3000;
static const int kRoomPlayerIdleAnimationRate = 14;
static const int kRoomPlayerIdleLoopStartFrame = 0x0f;
static const int kRoomPlayerIdleLoopLastFrame = 0xb2;
static const int kRoomPlayerIdleExitLastFrame = 0xbf;
static const int kRoomPlayerIdleYOffset = 4;

static void updatePlayerIdleTrigger(RoomIdleAnimationState &idleState) {
	idleState.triggerTick = MAX(idleState.activityTick, idleState.resetTick) + kRoomPlayerIdleDelayTicks;
}

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

} // End of anonymous namespace

bool Player::finishIdleAnimation(const RoomSetupState &state) {
	RoomPlayerState &playerState = _state;
	RoomIdleAnimationState &idleState = _idleState;
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
			(void)setIdleAnimation(idleState.restoreFacing);
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

bool Player::requestIdleAnimationExit(const RoomSetupState &state) {
	RoomPlayerState &playerState = _state;
	RoomIdleAnimationState &idleState = _idleState;
	if (!idleState.active || idleState.exiting || !idleState.entity)
		return false;

	const int lastFrame = idleState.entity->getLastFrame();
	const int exitStartFrame = MIN(lastFrame, kRoomPlayerIdleLoopLastFrame);
	const int exitLastFrame = MIN(lastFrame, kRoomPlayerIdleExitLastFrame);
	if (exitLastFrame <= exitStartFrame)
		return finishIdleAnimation(state);

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

	return finishIdleAnimation(state);
}

} // End of namespace Harvester
