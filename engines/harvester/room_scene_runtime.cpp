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

#include "harvester/room_scene_runtime.h"

#include "harvester/detection.h"
#include "harvester/monster.h"
#include "harvester/player.h"
#include "harvester/room_combat.h"

namespace Harvester {

static const char *const kInventoryOwnerName = "INVENTORY";

static bool isBackgroundSceneObject(const RoomSetupState &state,
		const ObjectRecord &object, const Entity &entity) {
	if (!state.backgroundObjectName.empty() &&
			object.objectName.equalsIgnoreCase(state.backgroundObjectName)) {
		return true;
	}

	return object.initialX == 0 && object.initialY == 0 &&
		entity.getBoundsWidth() == 640 && entity.getBoundsHeight() == 480;
}

static bool isInteractiveSceneHotspot(const ObjectRecord &object, Script *script) {
	if (object.operatable || !object.actionTag.empty())
		return true;
	if (!script)
		return false;
	if (script->isPickupObject(object))
		return true;

	ResolvedText inspectText;
	return script->resolveObjectInspectText(object, inspectText);
}

static int resolveSceneObjectClass(const RoomSetupState &state,
		const ObjectRecord &object, const Entity *entity, Script *script) {
	if (entity)
		return isBackgroundSceneObject(state, object, *entity)
			? kRuntimeEntityClassBackground
			: kRuntimeEntityClassObject;

	return isInteractiveSceneHotspot(object, script)
		? kRuntimeEntityClassRectHotspot
		: kRuntimeEntityClassDisabledHotspot;
}

RoomSceneRuntime::RoomSceneRuntime(RoomSetupState &state, Script *script, ResourceManager *resources, EntityManager *entityManager)
	: _state(state), _script(script), _resources(resources), _entityManager(entityManager) {
	debugC(1, kDebugRoom, "Harvester: created room scene runtime room='%s' entities=%d",
		_state.roomName.c_str(), _entityManager != nullptr);
}

bool RoomSceneRuntime::loadSceneResources(const RoomSetupState &state, RoomSceneResources &scene) const {
	return _resources && loadRoomSceneResources(state, *_resources, scene);
}

void RoomSceneRuntime::syncCurrentRoomRuntimeState(const Common::Array<RegionRecord> &sceneRegions) {
	Script *script = _script;
	if (!script)
		return;

	debugC(3, kDebugRoom, "Harvester: syncing room runtime state room='%s' entities=%d",
		_state.roomName.c_str(), _entityManager != nullptr);

	for (const RegionRecord &baseRegion : script->getRegions()) {
		if (!baseRegion.roomName.equalsIgnoreCase(_state.roomName))
			continue;

		const bool enabled =
			findSceneRegionByName(sceneRegions, baseRegion.regionName) != nullptr;
		(void)script->setRuntimeRegionEnabled(baseRegion.regionName, enabled);
	}

	if (_entityManager) {
		for (AnimRecord &anim : _state.roomAnimations) {
			Entity *entity = _entityManager->findSceneEntityByName(anim.animName);
			if (!entity)
				continue;
			(void)script->syncRuntimeAnimState(anim.animName,
				entity->isAnimationEnabled(), anim.visible, entity->getCurrentFrame());
		}
	}
	for (const MonsterRecord &monster : _state.roomMonsters)
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

bool RoomSceneRuntime::isTimerMaterializedInCurrentRoom(const Common::String &timerName) const {
	for (const TimerRecord &timer : _state.roomTimers) {
		if (timer.timerName.equalsIgnoreCase(timerName))
			return true;
	}

	return false;
}

void RoomSceneRuntime::syncGlobalTimerEntities(const Common::Array<TimerRecord> &previousTimerRecords,
		bool includeCurrentRoomTimers) {
	Script *script = _script;
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
				_state.roomName.c_str(), previousTimer.timerName.c_str(),
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
			_state.roomName.c_str(), runtimeTimer->timerName.c_str(),
			runtimeTimer->arg1.c_str(), runtimeTimer->currentValue,
			runtimeTimer->initialValue, runtimeTimer->enabled,
			runtimeTimer->looping);
	}
}

void RoomSceneRuntime::removeSceneEntityByName(const Common::String &entityName) {
	if (!_entityManager)
		return;

	Entity *entity = _entityManager->detachSceneEntityByName(entityName);
	delete entity;
}

Common::Rect RoomSceneRuntime::getSceneObjectBounds(const ObjectRecord &object) {
	if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
		return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);
	return Common::Rect();
}

Common::String RoomSceneRuntime::resolveSceneObjectSpritePathLocal(const ObjectRecord &object) {
	const bool atInitialPlacement = object.currentX == object.initialX &&
		object.currentY == object.initialY &&
		object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
	if (!object.altSpritePath.empty() &&
			(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)))
		return object.altSpritePath;
	return object.spritePath;
}

bool RoomSceneRuntime::isInteractiveSceneHotspot(const ObjectRecord &object) const {
	if (object.operatable || !object.actionTag.empty())
		return true;

	Script *script = _script;
	if (!script)
		return false;
	if (script->isPickupObject(object))
		return true;

	ResolvedText inspectText;
	return script->resolveObjectInspectText(object, inspectText);
}

RuntimeEntityClass RoomSceneRuntime::resolveSceneObjectClassLocal(const ObjectRecord &object, const Entity *entity) const {
	if (entity) {
		return ((!_state.backgroundObjectName.empty() &&
				object.objectName.equalsIgnoreCase(_state.backgroundObjectName)) ||
			(object.initialX == 0 && object.initialY == 0 &&
				entity->getBoundsWidth() == 640 && entity->getBoundsHeight() == 480))
			? kRuntimeEntityClassBackground
			: kRuntimeEntityClassObject;
	}

	return isInteractiveSceneHotspot(object)
		? kRuntimeEntityClassRectHotspot
		: kRuntimeEntityClassDisabledHotspot;
}

void RoomSceneRuntime::spawnSceneObjectEntityFromRecord(const ObjectRecord &object) {
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

Common::Rect RoomSceneRuntime::getSceneRegionBounds(const RegionRecord &region) {
	if (region.right > region.left && region.bottom > region.top)
		return Common::Rect(region.left, region.top, region.right + 1, region.bottom + 1);
	return Common::Rect();
}

void RoomSceneRuntime::spawnSceneRegionEntityFromRecord(const RegionRecord &region) {
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

bool RoomSceneRuntime::shouldSpawnRoomNpcEntity(const NpcRecord &npc) {
	return npc.visible && !npc.deathOrMonsterfyFlag;
}

bool RoomSceneRuntime::shouldSpawnRoomMonsterEntity(const MonsterRecord &monster) {
	return monster.visible;
}

bool RoomSceneRuntime::applyRoomNpcPlacement(Entity &entity, const NpcRecord &npc) {
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

Entity *RoomSceneRuntime::spawnSceneNpcEntityFromRecord(const NpcRecord &npc) {
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

Entity *RoomSceneRuntime::spawnSceneMonsterEntityFromRecord(const RoomSetupState &roomState, const MonsterRecord &monster) {
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

void RoomSceneRuntime::populateRoomRegions() {
	if (!_entityManager)
		return;

	for (const RegionRecord &region : _state.roomRegions) {
		const Common::Rect regionBounds = getSceneRegionBounds(region);
		if (regionBounds.isEmpty())
			continue;

		Entity *entity = _entityManager->spawnSceneHotspotEntity(
			region.regionName, regionBounds, (float)region.minZ);
		if (!entity) {
			debugC(1, kDebugRoom,
				"Harvester: scene region skipped room='%s' region='%s' bounds=(%d,%d)-(%d,%d) z=[%d,%d]",
				_state.roomName.c_str(), region.regionName.c_str(),
				regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
				region.minZ, region.maxZ);
			continue;
		}

		entity->setClassId(kRuntimeEntityClassRectHotspot19);
		entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
		entity->setZExtent((float)MAX(0, region.maxZ - region.minZ));
		if (!region.cursorEnabled)
			entity->setHitTestMode(kRuntimeEntityHitTestNone);
		debugC(1, kDebugRoom,
			"Harvester: scene region spawned room='%s' region='%s' class=0x%x bounds=(%d,%d)-(%d,%d) z=[%d,%d] facing=%d cursor=%d action='%s'",
			_state.roomName.c_str(), region.regionName.c_str(), entity->getClassId(),
			regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
			region.minZ, region.maxZ, region.desiredFacing, region.cursorEnabled, region.actionTag.c_str());
	}
}

void RoomSceneRuntime::populateRoomObjects(const Common::Array<ObjectRecord> &drawableObjects) {
	if (!_entityManager)
		return;

	for (const ObjectRecord &object : drawableObjects) {
		Entity *entity = nullptr;
		const Common::String spritePath = resolveSceneObjectSpritePathLocal(object);
		const Common::Rect hotspotBounds = getRoomObjectHotspotBounds(object);
		if (!spritePath.empty() && spritePath.hasSuffixIgnoreCase(".BM")) {
			entity = _entityManager->spawnSceneBitmapEntity(object.objectName, spritePath,
				Common::Point(object.currentX, object.currentY), (float)object.currentZ);
		} else {
			if (!hotspotBounds.isEmpty())
				entity = _entityManager->spawnSceneHotspotEntity(object.objectName, hotspotBounds, (float)object.currentZ);
		}

		if (!entity) {
			debugC(1, kDebugRoom,
				"Harvester: scene entity skipped room='%s' object='%s' resolved='%s' bounds=(%d,%d)-(%d,%d) reason='%s'",
				_state.roomName.c_str(), object.objectName.c_str(), spritePath.c_str(),
				hotspotBounds.left, hotspotBounds.top, hotspotBounds.right, hotspotBounds.bottom,
				spritePath.empty() || !spritePath.hasSuffixIgnoreCase(".BM")
					? "no_bitmap_and_no_hotspot_bounds"
					: "spawn_failed");
			debug(1, "Harvester: unable to spawn room object entity '%s' from '%s'",
				object.objectName.c_str(), spritePath.c_str());
			continue;
		}

		entity->setClassId(resolveSceneObjectClass(
			_state, object, entity->hasFrames() ? entity : nullptr, _script));
		entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
		entity->setZExtent((float)object.zExtent);
		const Common::Rect entityRect = entity->getScreenRect();
		debugC(1, kDebugRoom,
			"Harvester: scene entity spawned room='%s' object='%s' type='%s' class=0x%x pos=(%d,%d,z=%.2f) rect=(%d,%d)-(%d,%d) sprite='%s' action='%s' operatable=%d ident='%s'",
			_state.roomName.c_str(), object.objectName.c_str(),
			entity->hasFrames() ? "bitmap" : "hotspot", entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(),
			entityRect.left, entityRect.top, entityRect.right, entityRect.bottom,
			spritePath.c_str(), object.actionTag.c_str(), object.operatable, object.identTextKey.c_str());
	}
}

void RoomSceneRuntime::populateRoomNpcs() {
	if (!_entityManager)
		return;

	for (const NpcRecord &npc : _state.roomNpcs) {
		Entity *entity = _entityManager->spawnSceneActorEntity(npc.npcName,
			npc.modelPath, Common::Point(npc.posX, npc.posY), (float)npc.posZ, 0);
		if (!entity) {
			debug(1, "Harvester: unable to spawn room npc entity '%s' from '%s'",
				npc.npcName.c_str(), npc.modelPath.c_str());
			continue;
		}

		entity->setClassId(kRuntimeEntityClassNpc);
		entity->setZExtent(kNativeNpcMonsterZExtent);
		const bool isCorpse = !npc.active &&
			npc.runtimeSpawned &&
			npc.deathDamageType != 0 &&
			npc.runtimeState >= 0;
		entity->setHitTestMode(isCorpse ? kRuntimeEntityHitTestNone : kRuntimeEntityHitTestOpaquePixels);
		// Native spawn positions frame 0 before selecting ambient or corpse frame banks.
		if (!applyRoomNpcPlacement(*entity, npc)) {
			debug(1, "Harvester: unable to apply room npc placement for '%s'",
				npc.npcName.c_str());
		}
		if (isCorpse) {
			const int corpseFrame = MIN(entity->getLastFrame(), npc.runtimeState);
			entity->setAnimationFrameRange(corpseFrame, corpseFrame, false);
			entity->setAnimationRate(0);
			entity->setCurrentFrame(corpseFrame);
			entity->setAnimationEnabled(false);
		} else {
			// Native spawn_npc_entity_from_record seeds the passive room-NPC loop as frames 0..0x3b.
			entity->setAnimationFrameRange(0, MIN(entity->getLastFrame(), kRoomNpcAmbientLastFrame), true);
			if (npc.frameDelay > 0)
				entity->setAnimationRate(npc.frameDelay);
		}
		_entityManager->reinsertSceneEntity(entity);
		debugC(1, kDebugRoom,
			"Harvester: scene npc spawned room='%s' npc='%s' class=0x%x pos=(%d,%d,z=%.2f) frame_delay=%d model='%s' active=%d visible=%d",
			_state.roomName.c_str(), npc.npcName.c_str(), entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(),
			npc.frameDelay, npc.modelPath.c_str(), npc.active, npc.visible);
	}
}

void RoomSceneRuntime::populateRoomMonsters() {
	if (!_entityManager)
		return;

	for (const MonsterRecord &monster : _state.roomMonsters) {
		Entity *entity = _entityManager->spawnSceneActorEntity(monster.monsterName,
			monster.modelPath, Common::Point(monster.posX, monster.posY), (float)monster.posZ,
			Monster::resolveFacingFrame(monster.facing));
		if (!entity) {
			debug(1, "Harvester: unable to spawn room monster entity '%s' from '%s'",
				monster.monsterName.c_str(), monster.modelPath.c_str());
			continue;
		}

		entity->setClassId(kRuntimeEntityClassMonster);
		entity->setZExtent(kNativeNpcMonsterZExtent);
		entity->setHitTestMode(kRuntimeEntityHitTestNone);
		entity->setVisible(monster.visible);
		// Native restores a saved monster corpse frame before set_entity_screen_position().
		Monster::applyAnimation(*entity, monster);
		if (!applyRoomActorPlacement(_state, *entity, monster.posX, monster.posY, (float)monster.posZ)) {
			debug(1, "Harvester: unable to apply room monster placement for '%s'",
				monster.monsterName.c_str());
		}
		_entityManager->reinsertSceneEntity(entity);
		debugC(1, kDebugRoom,
			"Harvester: scene monster spawned room='%s' monster='%s' class=0x%x pos=(%d,%d,z=%.2f) facing=%d hp=%d/%d damage=%d engage=%d damage_type='%s' model='%s' active=%d visible=%d",
			_state.roomName.c_str(), monster.monsterName.c_str(), entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(), monster.facing,
			monster.currentHitPoints, monster.initialHitPoints,
			monster.damageAmount, monster.engageDistance, Player::describeCombatDamageType(monster.damageType),
			monster.modelPath.c_str(), monster.active, monster.visible);
	}
}

} // End of namespace Harvester
