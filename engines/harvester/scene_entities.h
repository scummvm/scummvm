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

#ifndef HARVESTER_SCENE_ENTITIES_H
#define HARVESTER_SCENE_ENTITIES_H

#include "harvester/room_support.h"
#include "harvester/entity.h"

namespace Harvester {

// Borrows the materialized room state; live entities remain owned by EntityManager.
class SceneEntities {
public:
	SceneEntities(RoomSetupState &state, Script *script, ResourceManager *resources, EntityManager *entityManager);

	bool loadSceneResources(const RoomSetupState &state, SceneState &scene) const;
	bool reconcileSceneEntitiesInPlace(const SceneState &previousScene,
		RoomSetupState &updatedState, const SceneState &updatedScene);
	void syncCurrentRoomRuntimeState(const Common::Array<RegionRecord> &sceneRegions);
	void syncGlobalTimerEntities(const Common::Array<TimerRecord> &previousTimerRecords,
		bool includeCurrentRoomTimers = false);
	void populateRoomRegions();
	void populateRoomObjects(const Common::Array<ObjectRecord> &drawableObjects);
	void populateRoomNpcs();
	void populateRoomMonsters();
	void restoreRoomMonsterAnimations();

	void removeSceneEntityByName(const Common::String &entityName);
	static Common::Rect getSceneObjectBounds(const ObjectRecord &object);
	static Common::String resolveSceneObjectSpritePathLocal(const ObjectRecord &object);
	void spawnSceneObjectEntityFromRecord(const ObjectRecord &object);
	void spawnSceneRegionEntityFromRecord(const RegionRecord &region);
	static bool shouldSpawnRoomNpcEntity(const NpcRecord &npc);
	static bool shouldSpawnRoomMonsterEntity(const MonsterRecord &monster);
	Entity *spawnSceneNpcEntityFromRecord(const NpcRecord &npc);
	Entity *spawnSceneMonsterEntityFromRecord(const RoomSetupState &roomState, const MonsterRecord &monster);

private:
	bool isTimerMaterializedInCurrentRoom(const Common::String &timerName) const;
	bool isInteractiveSceneHotspot(const ObjectRecord &object) const;
	EntityClass resolveSceneObjectClassLocal(const ObjectRecord &object, const Entity *entity) const;
	static Common::Rect getSceneRegionBounds(const RegionRecord &region);
	static bool applyRoomNpcPlacement(Entity &entity, const NpcRecord &npc);

	RoomSetupState &_state;
	Script *_script;
	ResourceManager *_resources;
	EntityManager *_entityManager;
};

} // End of namespace Harvester

#endif // HARVESTER_SCENE_ENTITIES_H
