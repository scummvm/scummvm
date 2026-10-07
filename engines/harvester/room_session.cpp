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
#include "harvester/harvester.h"
#include "harvester/player.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

RoomSession::RoomSession(HarvesterEngine &engine, RoomSceneResources &scene, EntityManager *entityManager)
	: _engine(engine), _scene(scene), _entityManager(entityManager) {
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
	Player::syncAnimationPlacement(_playerState);
}

void RoomSession::syncCurrentRoomRuntimeState() {
	Script *script = _engine.getScript();
	if (!script)
		return;

	debugC(3, kDebugRoom, "Harvester: syncing room runtime state room='%s' entities=%d",
		_scene.state.roomName.c_str(), _entityManager != nullptr);

	for (const RegionRecord &baseRegion : script->getRegions()) {
		if (!baseRegion.roomName.equalsIgnoreCase(_scene.state.roomName))
			continue;

		const bool enabled =
			findSceneRegionByName(_scene.sceneRegions, baseRegion.regionName) != nullptr;
		(void)script->setRuntimeRegionEnabled(baseRegion.regionName, enabled);
	}

	if (_entityManager) {
		for (AnimRecord &anim : _scene.state.roomAnimations) {
			Entity *entity = _entityManager->findSceneEntityByName(anim.animName);
			if (!entity)
				continue;
			(void)script->syncRuntimeAnimState(anim.animName,
				entity->isAnimationEnabled(), anim.visible, entity->getCurrentFrame());
		}
	}
	for (const MonsterRecord &monster : _scene.state.roomMonsters)
		(void)script->syncRuntimeMonsterRecord(monster);
	if (!_entityManager)
		return;

	for (const TimerRecord &baseTimer : script->getTimers()) {
		Entity *entity = _entityManager->findSceneEntityByName(baseTimer.timerName);
		if (!entity || entity->getClassId() != kRuntimeEntityClassTimer)
			continue;

		const TimerRecord *runtimeTimer = script->findRuntimeTimerRecord(baseTimer.timerName);
		TimerRecord timer = runtimeTimer ? *runtimeTimer : baseTimer;
		timer.currentValue = entity->getTimerCurrentValue();
		timer.enabled = entity->isTimerEnabled();
		timer.looping = entity->isTimerLooping();
		timer.global = entity->isTimerGlobal();
		(void)script->syncRuntimeTimerRecord(timer);
	}
}

void RoomSession::captureCurrentSaveState() {
	Player::syncAnimationPlacement(_playerState);
	syncCurrentRoomRuntimeState();
	const int facing = _playerState.facing >= 0 ? _playerState.facing : _scene.state.playerFacing;
	_engine.captureCurrentSaveRoomState(_scene.state.entranceName, _scene.state.roomName,
		_playerState.centerX, _playerState.bottomY, (int)_playerState.z, facing,
		_scene.state.discNumber, _engine.getMusicPath());
}

bool RoomSession::isTimerMaterializedInCurrentRoom(const Common::String &timerName) const {
	for (const TimerRecord &timer : _scene.state.roomTimers) {
		if (timer.timerName.equalsIgnoreCase(timerName))
			return true;
	}

	return false;
}

void RoomSession::syncGlobalTimerEntities(const Common::Array<TimerRecord> &previousTimerRecords,
		bool includeCurrentRoomTimers) {
	Script *script = _engine.getScript();
	if (!script || !_entityManager || previousTimerRecords.empty())
		return;

	for (const TimerRecord &previousTimer : previousTimerRecords) {
		const TimerRecord *runtimeTimer =
			script->findRuntimeTimerRecord(previousTimer.timerName);
		if (!runtimeTimer || !runtimeTimer->global ||
				(!includeCurrentRoomTimers &&
					isTimerMaterializedInCurrentRoom(runtimeTimer->timerName))) {
			continue;
		}

		Entity *entity = _entityManager->findSceneEntityByName(runtimeTimer->timerName);
		if (entity && entity->getClassId() != kRuntimeEntityClassTimer)
			continue;

		if (!entity) {
			(void)script->syncRuntimeTimerRecord(previousTimer);
			debugC(1, kDebugRoom,
				"Harvester: global timer mutation ignored current_room='%s' timer='%s' owner_room='%s' reason='missing live timer entity'",
				_scene.state.roomName.c_str(), previousTimer.timerName.c_str(),
				previousTimer.arg1.c_str());
			continue;
		}

		if (entity->getTimerInitialValue() == runtimeTimer->initialValue &&
				entity->isTimerEnabled() == runtimeTimer->enabled &&
				entity->isTimerLooping() == runtimeTimer->looping &&
				entity->isTimerGlobal() == runtimeTimer->global) {
			continue;
		}

		_entityManager->configureSceneTimerEntity(*entity,
			runtimeTimer->initialValue, runtimeTimer->currentValue,
			runtimeTimer->enabled, runtimeTimer->looping, runtimeTimer->global);
		debugC(1, kDebugRoom,
			"Harvester: global timer synced current_room='%s' timer='%s' owner_room='%s' current=%d initial=%d enabled=%d loop=%d",
			_scene.state.roomName.c_str(), runtimeTimer->timerName.c_str(),
			runtimeTimer->arg1.c_str(), runtimeTimer->currentValue,
			runtimeTimer->initialValue, runtimeTimer->enabled,
			runtimeTimer->looping);
	}
}

} // End of namespace Harvester
