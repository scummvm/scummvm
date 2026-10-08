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

#ifndef HARVESTER_ROOM_SESSION_H
#define HARVESTER_ROOM_SESSION_H

#include "harvester/room_combat.h"
#include "harvester/room_scene_runtime.h"
#include "harvester/room_support.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

class EntityManager;
class Flow;
class InventorySystem;

struct RoomMovementInputState {
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
};

// Scoped to one runRoomLoop iteration, including time spent in nested closeups.
// Entity pointers remain owned by EntityManager.
class RoomSession {
public:
	RoomSession(HarvesterEngine &engine, Flow &flow, RoomSceneResources &scene, EntityManager *entityManager,
		InventorySystem &inventory);

	RoomPlayerState &getPlayerState() { return _playerState; }
	Common::Array<RoomMonsterCombatState> &getMonsterCombatStates() { return _monsterCombatStates; }
	RoomSceneRuntime &getSceneRuntime() { return _sceneRuntime; }
	RoomCombatEffects &getCombatEffects() { return _combatEffects; }
	RoomIdleAnimationState &getIdleState() { return _idleState; }
	RoomMovementInputState &getMovementInput() { return _movementInput; }
	Common::String &getPendingRegionName() { return _pendingRegionName; }

	void resetIdleState();
	void notePlayerActivity();
	bool isPlayerCombatLocked() const;
	bool cancelPlayerAttackAnimation();
	void stopPlayerRegionInteraction();
	void syncPlayerPlacement();
	void captureCurrentSaveState();
	bool applyCurrentRoomRuntimeMutationsInPlace(bool preservePaletteState);
	bool refreshCurrentScene(bool preservePlayerPlacement);

private:
	HarvesterEngine &_engine;
	Flow &_flow;
	RoomSceneResources &_scene;
	EntityManager *_entityManager;
	InventorySystem &_inventory;
	RoomSceneRuntime _sceneRuntime;
	RoomPlayerState _playerState;
	Common::Array<RoomMonsterCombatState> _monsterCombatStates;
	RoomCombatEffects _combatEffects;
	RoomIdleAnimationState _idleState;
	RoomMovementInputState _movementInput;
	Common::String _pendingRegionName;
};

} // End of namespace Harvester

#endif // HARVESTER_ROOM_SESSION_H
