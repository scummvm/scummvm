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

#include "harvester/room_session.h"

#include "harvester/detection.h"
#include "harvester/flow.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/monster.h"
#include "harvester/player.h"
#include "harvester/room_combat.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

RoomSession::RoomSession(HarvesterEngine &engine, Flow &flow, RoomSceneResources &scene, EntityManager *entityManager,
		InventorySystem &inventory)
	: _engine(engine), _flow(flow), _scene(scene), _entityManager(entityManager), _inventory(inventory),
	  _sceneRuntime(scene.state, engine.getScript(), engine.getResources(), entityManager), _combatEffects(entityManager) {
}

void RoomSession::resetIdleState() {
	_idleState = RoomIdleAnimationState();
	_idleState.activityTick = Player::getRuntimeClockTicks();
	_idleState.resetTick = _idleState.activityTick;
	Player::updateIdleTrigger(_idleState);
}

void RoomSession::notePlayerActivity() {
	_idleState.activityTick = Player::getRuntimeClockTicks();
	Player::updateIdleTrigger(_idleState);
}

bool RoomSession::isPlayerCombatLocked() const {
	return _playerState.attackActive || _playerState.hitActive || _playerState.deathActive;
}

bool RoomSession::cancelPlayerAttackAnimation() {
	if (!_playerState.attackActive)
		return false;

	_playerState.attackActive = false;
	_playerState.attackFirstFrame = -1;
	_playerState.attackLastFrame = -1;
	_playerState.attackContactFrame = -1;
	if (!_playerState.entity)
		return false;
	_playerState.attackSoundPlayed = false;
	_playerState.attackSoundPlaybackFrame = -1;
	_playerState.attackContactResolved = false;
	_playerState.attackTargetName.clear();
	_playerState.attackTargetClassId = -1;
	const int resumeFacing = _playerState.attackResumeFacing >= 0
		? _playerState.attackResumeFacing
		: (_playerState.facing >= 0 ? _playerState.facing : 0);
	_playerState.attackResumeFacing = -1;
	return Player::setIdleAnimation(_playerState, resumeFacing);
}

void RoomSession::stopPlayerRegionInteraction() {
	_movementInput.moveLeft = false;
	_movementInput.moveRight = false;
	_movementInput.moveUp = false;
	_movementInput.moveDown = false;
	_pendingRegionName.clear();
	(void)cancelPlayerAttackAnimation();
	_playerState.hasMoveTarget = false;
	_playerState.turnActive = false;
	_playerState.turnTargetFacing = -1;
	if (_playerState.entity && _playerState.facing >= 0)
		(void)Player::setIdleAnimation(_playerState, _playerState.facing);
	syncPlayerPlacement();
}

void RoomSession::syncPlayerPlacement() {
	Player::syncAnimationPlacement(_playerState);
}

void RoomSession::captureCurrentSaveState() {
	syncPlayerPlacement();
	_sceneRuntime.syncCurrentRoomRuntimeState(_scene.sceneRegions);
	const int facing = _playerState.facing >= 0 ? _playerState.facing : _scene.state.playerFacing;
	_engine.captureCurrentSaveRoomState(_scene.state.entranceName, _scene.state.roomName,
		_playerState.centerX, _playerState.bottomY, (int)_playerState.z, facing,
		_scene.state.discNumber, _engine.getMusicPath());
}

bool RoomSession::applyCurrentRoomRuntimeMutationsInPlace(bool preservePaletteState) {
	Script *script = _engine.getScript();
	ResourceManager *resources = _engine.getResources();
	if (!script || !resources)
		return false;

	byte previousPalette[256 * 3];
	memcpy(previousPalette, _scene.palette, sizeof(previousPalette));
	const float previousPaletteBrightness = _scene.targetPaletteBrightness;
	Common::Array<AudioCommand> entryAudioCommands = _scene.state.audioCommands;
	debugC(2, kDebugRoom, "Harvester: reconciling room runtime mutations room='%s' preserve_palette=%d",
		_scene.state.roomName.c_str(), preservePaletteState);
	RoomSetupState updatedState;
	if (!script->materializeRoomState(
			_scene.state.entranceName, _scene.state.roomName, updatedState, *resources)) {
		return false;
	}
	updatedState.audioCommands = Common::move(entryAudioCommands);

	RoomSceneResources updatedScene;
	if (!_sceneRuntime.loadSceneResources(updatedState, updatedScene))
		return false;
	Common::Array<RoomMonsterCombatState> updatedMonsterCombatStates =
		_combat.prepareMonsterCombatStatesForSceneChange(_scene.state.roomMonsters, updatedState.roomMonsters);

	if (!_sceneRuntime.reconcileSceneEntitiesInPlace(_scene, updatedState, updatedScene))
		return false;

	_scene.state = updatedState;
	_combat.commitMonsterCombatStatesForSceneChange(Common::move(updatedMonsterCombatStates));
	_scene.sceneObjects = updatedScene.sceneObjects;
	_scene.sceneAnimations = updatedScene.sceneAnimations;
	_scene.sceneRegions = updatedScene.sceneRegions;
	const int playerCombatLoadout = script->getPlayerCombatLoadout();
	if (_playerState.entity && _playerState.combatLoadout != playerCombatLoadout &&
			!Player::syncCombatLoadoutVisual(
				_engine, updatedState, _playerState, playerCombatLoadout)) {
		return false;
	}
	if (preservePaletteState) {
		memcpy(_scene.palette, previousPalette, sizeof(_scene.palette));
		_scene.targetPaletteBrightness = previousPaletteBrightness;
	} else {
		memcpy(_scene.palette, updatedScene.palette, sizeof(_scene.palette));
		_scene.targetPaletteBrightness = updatedScene.targetPaletteBrightness;
	}
	captureCurrentSaveState();
	return _inventory.refresh();
}

bool RoomSession::refreshCurrentScene(bool preservePlayerPlacement) {
	debugC(2, kDebugRoom, "Harvester: refreshing room scene room='%s' preserve_player_placement=%d",
		_scene.state.roomName.c_str(), preservePlayerPlacement);
	Common::Array<AudioCommand> entryAudioCommands = _scene.state.audioCommands;
	RoomSetupState refreshedState;
	if (!_engine.getScript()->materializeRoomState(
			_scene.state.entranceName, _scene.state.roomName, refreshedState, *_engine.getResources())) {
		return false;
	}

	refreshedState.audioCommands = Common::move(entryAudioCommands);
	if (!_sceneRuntime.loadSceneResources(refreshedState, _scene))
		return false;
	_combatEffects.clearHitEffects();
	if (!_flow.populateRoomSceneEntities(_scene.state, _scene.sceneObjects, _scene.sceneAnimations))
		return false;
	if (_entityManager) {
		for (const MonsterRecord &monster : _scene.state.roomMonsters) {
			Entity *entity = _entityManager->findSceneEntityByName(monster.monsterName);
			if (entity)
				Monster::applyAnimation(*entity, monster);
		}
	}

	_playerState.entity = _entityManager ? _entityManager->findSceneEntityByName("PLAYER") : nullptr;
	if (_playerState.entity) {
		if (!preservePlayerPlacement || _playerState.facing < 0) {
			_playerState.centerX = _scene.state.playerSpawnX;
			_playerState.bottomY = _scene.state.playerSpawnY;
			_playerState.z = (float)_scene.state.playerSpawnZ;
			_playerState.facing = _scene.state.playerFacing;
		}

		const int facing = _playerState.facing >= 0 ? _playerState.facing : _scene.state.playerFacing;
		(void)Player::setIdleAnimation(_playerState, facing);
		(void)applyRoomActorPlacement(_scene.state, *_playerState.entity,
			_playerState.centerX, _playerState.bottomY, _playerState.z);
	}

	_playerState.hasMoveTarget = false;
	_playerState.turnActive = false;
	_playerState.turnTargetFacing = -1;
	_playerState.attackActive = false;
	_playerState.attackFirstFrame = -1;
	_playerState.attackLastFrame = -1;
	_playerState.attackContactFrame = -1;
	_playerState.attackResumeFacing = -1;
	_playerState.attackSoundPlayed = false;
	_playerState.attackSoundPlaybackFrame = -1;
	_playerState.attackContactResolved = false;
	_playerState.attackTargetName.clear();
	_playerState.attackTargetClassId = -1;
	_playerState.hitActive = false;
	_playerState.hitFirstFrame = -1;
	_playerState.hitLastFrame = -1;
	_playerState.hitResumeFacing = -1;
	_playerState.hitKnockbackRemainingX = 0;
	_playerState.hitKnockbackDecayStep = 0;
	_playerState.combatLoadout =
		_engine.getScript() ? _engine.getScript()->getPlayerCombatLoadout() : 0;
	_combat.resetForSceneRefresh(_scene.state);
	_pendingRegionName.clear();
	resetIdleState();
	_flow.resetCursorAnimationSequence();
	captureCurrentSaveState();
	return _inventory.refresh();
}

} // End of namespace Harvester
