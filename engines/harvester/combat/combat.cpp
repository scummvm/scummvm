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

#include "harvester/combat/combat.h"

#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/scene/scene_entities.h"

namespace Harvester {

const float kNativeNpcMonsterZExtent = 5.0f;
const int kRoomNpcAmbientLastFrame = 0x3b;
const uint32 kNativeKeyboardAttackRepeatTicks = 25;

const char *describeCombatTargetClass(int classId) {
	switch (classId) {
	case kEntityClassNpc:
		return "npc";
	case kEntityClassMonster:
		return "monster";
	case kEntityClassPlayer:
		return "player";
	default:
		return "none";
	}
}

Combat::Combat(HarvesterEngine &engine, SceneState &scene, Player &player,
		EntityManager *entityManager, SceneEntities &sceneRuntime, CombatEffects &combatEffects,
		const CheatSystem &cheats, InteractionCallbacks &callbacks, Interaction &interactionProcessor)
	: _engine(engine), _scene(scene), _player(player), _entityManager(entityManager),
	  _sceneRuntime(sceneRuntime), _combatEffects(combatEffects), _cheats(cheats),
	  _callbacks(callbacks), _interactionProcessor(interactionProcessor) {
}

void Combat::initializeCombatStates() {
	_monsterCombatStates.resize(_scene.state.roomMonsters.size());
	_npcCombatStates.resize(_scene.state.roomNpcs.size());
	debugC(1, kDebugCombat, "Harvester: initialized room combat states monsters=%u npcs=%u",
		_monsterCombatStates.size(), _npcCombatStates.size());
}

void Combat::resizeCombatStates() {
	if (_npcCombatStates.size() != _scene.state.roomNpcs.size())
		_npcCombatStates.resize(_scene.state.roomNpcs.size());
	if (_monsterCombatStates.size() != _scene.state.roomMonsters.size())
		_monsterCombatStates.resize(_scene.state.roomMonsters.size());
}

Common::Array<MonsterCombatState> Combat::prepareMonsterCombatStatesForSceneChange(
		const Common::Array<MonsterRecord> &previousMonsters, const Common::Array<MonsterRecord> &updatedMonsters) const {
	Common::Array<MonsterCombatState> updatedMonsterCombatStates;
	updatedMonsterCombatStates.resize(updatedMonsters.size());
	for (uint i = 0; i < updatedMonsters.size(); ++i) {
		const MonsterRecord &updatedMonster = updatedMonsters[i];
		for (uint j = 0; j < previousMonsters.size() &&
				j < _monsterCombatStates.size(); ++j) {
			const MonsterRecord &previousMonster = previousMonsters[j];
			if (!previousMonster.monsterName.equalsIgnoreCase(updatedMonster.monsterName))
				continue;
			if (sameMonsterEntityState(previousMonster, updatedMonster))
				updatedMonsterCombatStates[i] = _monsterCombatStates[j];
			break;
		}
	}

	return updatedMonsterCombatStates;
}

void Combat::commitMonsterCombatStatesForSceneChange(Common::Array<MonsterCombatState> &&states) {
	_monsterCombatStates = Common::move(states);
	debugC(2, kDebugCombat, "Harvester: committed room monster combat states count=%u", _monsterCombatStates.size());
}

void Combat::resetForSceneRefresh() {
	// NPC death state survives refresh and is resized by index at the next combat update.
	_monsterCombatStates.clear();
	_monsterCombatStates.resize(_scene.state.roomMonsters.size());
	debugC(2, kDebugCombat, "Harvester: reset room monster combat states count=%u retained_npcs=%u",
		_monsterCombatStates.size(), _npcCombatStates.size());
}

void clearRoomMonsterCombatState(MonsterCombatState &combatState) {
	combatState = MonsterCombatState();
}

void clearRoomNpcCombatState(NpcCombatState &combatState) {
	combatState = NpcCombatState();
}

void clearRoomMonsterAttackState(MonsterCombatState &combatState) {
	combatState.attackActive = false;
	combatState.attackFirstFrame = -1;
	combatState.attackLastFrame = -1;
	combatState.attackContactFrame = -1;
	combatState.attackResumeFacing = -1;
	combatState.attackSoundPlayed = false;
	combatState.attackContactResolved = false;
	combatState.attackTargetName.clear();
}

void clearRoomMonsterHitState(MonsterCombatState &combatState) {
	combatState.hitActive = false;
	combatState.hitFirstFrame = -1;
	combatState.hitLastFrame = -1;
	combatState.hitResumeFacing = -1;
	combatState.hitKnockbackRemainingX = 0;
	combatState.hitKnockbackDecayStep = 0;
	combatState.hitSoundPlayed = false;
}

} // End of namespace Harvester
