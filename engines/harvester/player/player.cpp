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
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/entity.h"

namespace Harvester {

using namespace PlayerMovement;

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

} // End of namespace Harvester
