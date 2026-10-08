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
#include "harvester/monster.h"
#include "harvester/player.h"
#include "harvester/room_combat.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

static const char *const kInventoryOwnerName = "INVENTORY";

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

void RoomSession::removeSceneEntityByName(const Common::String &entityName) {
	if (!_entityManager)
		return;

	Entity *entity = _entityManager->detachSceneEntityByName(entityName);
	delete entity;
}

Common::Rect RoomSession::getSceneObjectBounds(const ObjectRecord &object) {
	if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
		return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);
	return Common::Rect();
}

Common::String RoomSession::resolveSceneObjectSpritePathLocal(const ObjectRecord &object) {
	const bool atInitialPlacement = object.currentX == object.initialX &&
		object.currentY == object.initialY &&
		object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
	if (!object.altSpritePath.empty() &&
			(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)))
		return object.altSpritePath;
	return object.spritePath;
}

bool RoomSession::isInteractiveSceneHotspot(const ObjectRecord &object) const {
	if (object.operatable || !object.actionTag.empty())
		return true;

	Script *script = _engine.getScript();
	if (!script)
		return false;
	if (script->isPickupObject(object))
		return true;

	ResolvedText inspectText;
	return script->resolveObjectInspectText(object, inspectText);
}

RuntimeEntityClass RoomSession::resolveSceneObjectClassLocal(const ObjectRecord &object, const Entity *entity) const {
	if (entity) {
		return ((!_scene.state.backgroundObjectName.empty() &&
				object.objectName.equalsIgnoreCase(_scene.state.backgroundObjectName)) ||
			(object.initialX == 0 && object.initialY == 0 &&
				entity->getBoundsWidth() == 640 && entity->getBoundsHeight() == 480))
			? kRuntimeEntityClassBackground
			: kRuntimeEntityClassObject;
	}

	return isInteractiveSceneHotspot(object)
		? kRuntimeEntityClassRectHotspot
		: kRuntimeEntityClassDisabledHotspot;
}

void RoomSession::spawnSceneObjectEntityFromRecord(const ObjectRecord &object) {
	if (!_entityManager)
		return;

	const Common::String spritePath = resolveSceneObjectSpritePathLocal(object);
	const Common::Rect hotspotBounds = getSceneObjectBounds(object);
	Entity *entity = nullptr;
	if (!spritePath.empty() && spritePath.hasSuffixIgnoreCase(".BM")) {
		entity = _entityManager->spawnSceneBitmapEntity(object.objectName, spritePath,
			Common::Point(object.currentX, object.currentY), (float)object.currentZ);
	} else if (!hotspotBounds.isEmpty()) {
		entity = _entityManager->spawnSceneHotspotEntity(object.objectName, hotspotBounds,
			(float)object.currentZ);
	}
	if (!entity)
		return;

	entity->setClassId(resolveSceneObjectClassLocal(
		object, entity->hasFrames() ? entity : nullptr));
	entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
	entity->setZExtent((float)object.zExtent);
	debugC(3, kDebugRoom, "Harvester: scene object spawned object='%s' sprite='%s' class=0x%x",
		object.objectName.c_str(), spritePath.c_str(), entity->getClassId());
}

Common::Rect RoomSession::getSceneRegionBounds(const RegionRecord &region) {
	if (region.right > region.left && region.bottom > region.top)
		return Common::Rect(region.left, region.top, region.right + 1, region.bottom + 1);
	return Common::Rect();
}

void RoomSession::spawnSceneRegionEntityFromRecord(const RegionRecord &region) {
	if (!_entityManager)
		return;

	const Common::Rect bounds = getSceneRegionBounds(region);
	if (bounds.isEmpty())
		return;

	Entity *entity = _entityManager->spawnSceneHotspotEntity(
		region.regionName, bounds, (float)region.minZ);
	if (!entity)
		return;

	entity->setClassId(kRuntimeEntityClassRectHotspot19);
	entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
	entity->setZExtent((float)MAX(0, region.maxZ - region.minZ));
	if (!region.cursorEnabled)
		entity->setHitTestMode(kRuntimeEntityHitTestNone);
	debugC(1, kDebugRoom,
		"Harvester: scene region spawned room='%s' region='%s' class=0x%x bounds=(%d,%d)-(%d,%d) z=[%d,%d] facing=%d cursor=%d action='%s'",
		region.roomName.c_str(), region.regionName.c_str(), entity->getClassId(),
		bounds.left, bounds.top, bounds.right, bounds.bottom,
		region.minZ, region.maxZ, region.desiredFacing, region.cursorEnabled, region.actionTag.c_str());
}

bool RoomSession::shouldSpawnRoomNpcEntity(const NpcRecord &npc) {
	return npc.visible && !npc.deathOrMonsterfyFlag;
}

bool RoomSession::shouldSpawnRoomMonsterEntity(const MonsterRecord &monster) {
	return monster.visible;
}

bool RoomSession::applyRoomNpcPlacement(Entity &entity, const NpcRecord &npc) {
	int width = 0;
	int height = 0;
	int xOffset = 0;
	int yOffset = 0;
	if (!entity.getCurrentFrameMetrics(width, height, xOffset, yOffset))
		return false;

	const float renderZ =
		(float)npc.posZ - floorf(MAX<float>(entity.getZExtent(), 0.0f) * 0.5f);
	entity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
	entity.setPosition(npc.posX - xOffset - width / 2, npc.posY - height - yOffset, renderZ);
	return true;
}

Entity *RoomSession::spawnSceneNpcEntityFromRecord(const NpcRecord &npc) {
	const bool shouldSpawnNpc = shouldSpawnRoomNpcEntity(npc);
	const bool preservedCorpse = !npc.active &&
		npc.runtimeSpawned &&
		npc.deathDamageType != 0 &&
		npc.runtimeState >= 0;
	if (!_entityManager || !shouldSpawnNpc)
		return nullptr;

	Entity *entity = _entityManager->spawnSceneActorEntity(
		npc.npcName, npc.modelPath, Common::Point(npc.posX, npc.posY), (float)npc.posZ, 0);
	if (!entity)
		return nullptr;

	entity->setClassId(kRuntimeEntityClassNpc);
	entity->setZExtent(kNativeNpcMonsterZExtent);
	entity->setHitTestMode(preservedCorpse ? kRuntimeEntityHitTestNone : kRuntimeEntityHitTestOpaquePixels);
	entity->setVisible(true);
	if (!applyRoomNpcPlacement(*entity, npc)) {
		removeSceneEntityByName(npc.npcName);
		return nullptr;
	}
	if (preservedCorpse) {
		const int corpseFrame = MIN(entity->getLastFrame(), npc.runtimeState);
		entity->setAnimationFrameRange(corpseFrame, corpseFrame, false);
		entity->setAnimationRate(0);
		entity->setCurrentFrame(corpseFrame);
		entity->setAnimationEnabled(false);
	} else {
		entity->setAnimationFrameRange(0, MIN(entity->getLastFrame(), kRoomNpcAmbientLastFrame), true);
		entity->setAnimationRate(npc.frameDelay > 0 ? npc.frameDelay : 0);
	}
	_entityManager->reinsertSceneEntity(entity);
	return entity;
}

Entity *RoomSession::spawnSceneMonsterEntityFromRecord(const RoomSetupState &roomState, const MonsterRecord &monster) {
	if (!_entityManager || !shouldSpawnRoomMonsterEntity(monster))
		return nullptr;

	Entity *entity = _entityManager->spawnSceneActorEntity(
		monster.monsterName, monster.modelPath,
		Common::Point(monster.posX, monster.posY), (float)monster.posZ,
		Monster::resolveFacingFrame(monster.facing));
	if (!entity)
		return nullptr;

	entity->setClassId(kRuntimeEntityClassMonster);
	entity->setZExtent(kNativeNpcMonsterZExtent);
	entity->setHitTestMode(kRuntimeEntityHitTestNone);
	entity->setVisible(monster.visible);
	Monster::applyAnimation(*entity, monster);
	if (!applyRoomActorPlacement(roomState, *entity, monster.posX, monster.posY, (float)monster.posZ)) {
		removeSceneEntityByName(monster.monsterName);
		return nullptr;
	}
	_entityManager->reinsertSceneEntity(entity);
	return entity;
}

} // End of namespace Harvester
