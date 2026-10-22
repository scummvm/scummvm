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
#include "harvester/harvester.h"
#include "harvester/scene/interaction.h"
#include "harvester/player/player.h"

namespace Harvester {

namespace {

static int findRoomNpcIndexByName(const RoomSetupState &state, const Common::String &name) {
	for (uint i = 0; i < state.roomNpcs.size(); ++i) {
		if (state.roomNpcs[i].npcName.equalsIgnoreCase(name))
			return static_cast<int>(i);
	}

	return -1;
}

static int findRoomMonsterIndexByName(const RoomSetupState &state, const Common::String &name) {
	for (uint i = 0; i < state.roomMonsters.size(); ++i) {
		if (state.roomMonsters[i].monsterName.equalsIgnoreCase(name))
			return static_cast<int>(i);
	}

	return -1;
}

} // End of anonymous namespace

Common::Error Combat::resolvePlayerAttackContact(bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	if (!playerState.attackActive || playerState.attackContactResolved || !playerState.entity ||
			playerState.attackContactFrame < 0 ||
			playerState.entity->getCurrentFrame() < playerState.attackContactFrame) {
		return Common::kNoError;
	}

	const bool projectile = Player::isProjectileCombatLoadout(playerState.combatLoadout);
	if (projectile)
		_player.markAttackContactResolved();
	const int damageType = Player::resolveCombatLoadoutDamageType(playerState.combatLoadout);
	const int damageAmount = Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout);
	Script *script = _engine.getScript();
	if (!script)
		return Common::kNoError;

	captureOverlappingNpcAttackTarget();
	if (playerState.attackTargetClassId == kEntityClassNpc)
		return resolvePlayerNpcAttackContact(*script, damageType, needsRedraw);
	return resolvePlayerMonsterAttackContact(*script, damageType, damageAmount, needsRedraw);
}

Common::Error Combat::resolvePlayerNpcAttackContact(Script &script, int damageType, bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	Entity *npcEntity = nullptr;
	NpcRecord *npc = selectNpcAttackContactTarget(npcEntity);
	if (!npc) {
		if (playerState.entity->didAnimationAdvanceLastTick()) {
			debugC(3, kDebugCombat,
				"Harvester: combat player attack missed class='npc' target='%s' frame=%d reason='no reachable npc'",
				playerState.attackTargetName.c_str(), playerState.entity->getCurrentFrame());
		}
		return Common::kNoError;
	}

	_player.markAttackContactResolved();
	const bool runtimeChanged =
		script.queueRuntimeNpcDeathOrMonsterfy(npc->npcName, damageType);
	const bool jimmyFlagChanged = npc->npcName.equalsIgnoreCase("JIMMY")
		? script.setRuntimeFlagValue("JIMMY_ATTACKED", true)
		: false;
	const int npcIndex = findRoomNpcIndexByName(_scene.state, npc->npcName);
	NpcCombatState *combatState =
		npcIndex >= 0 && npcIndex < static_cast<int>(_npcCombatStates.size())
			? &_npcCombatStates[static_cast<uint>(npcIndex)]
			: nullptr;
	if (runtimeChanged) {
		npc->deathDamageType = damageType;
		if (combatState)
			clearRoomNpcCombatState(*combatState);
	}
	debugC(1, kDebugCombat,
		"Harvester: combat player npc hit target='%s' damage_type=%d runtime_changed=%d jimmy_flag=%d",
		npc->npcName.c_str(), damageType, runtimeChanged, jimmyFlagChanged);
	if (!runtimeChanged && !jimmyFlagChanged)
		return Common::kNoError;
	if (runtimeChanged && npcEntity) {
		const Common::String followTargetName = npc->monsterfyTargetName;
		if (_combatEffects.spawnCombatHitEffect(*npcEntity, followTargetName))
			needsRedraw = true;
	}
	if (runtimeChanged && combatState && npcEntity &&
			beginNpcDeathTransition(*npc, *npcEntity, *combatState, damageType, needsRedraw))
		return Common::kNoError;
	if (runtimeChanged) {
		debugC(1, kDebugCombat,
			"Harvester: combat npc defeated target='%s' damage_type=%d on_death='%s' fallback='no death bank'",
			npc->npcName.c_str(), damageType, npc->onDeathActionTag.c_str());
		NpcCombatState fallbackState;
		fallbackState.deathDamageType = damageType;
		return finalizeNpcDeathTransition(*npc, combatState ? *combatState : fallbackState, needsRedraw);
	}

	return Common::kNoError;
}

Common::Error Combat::resolvePlayerMonsterAttackContact(Script &script,
		int damageType, int damageAmount, bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	Entity *monsterEntity = nullptr;
	MonsterRecord *monster = selectMonsterAttackContactTarget(monsterEntity);
	if (!monster || !monsterEntity) {
		if (playerState.entity->didAnimationAdvanceLastTick()) {
			debugC(3, kDebugCombat,
				"Harvester: combat player attack missed class='monster' target='%s' frame=%d reason='no reachable monster'",
				playerState.attackTargetName.c_str(), playerState.entity->getCurrentFrame());
		}
		return Common::kNoError;
	}

	_player.markAttackContactResolved();
	const int hitPointsBefore = monster->currentHitPoints;
	monster->currentHitPoints = MAX(0,
		monster->currentHitPoints - damageAmount);
	const int damageLanded = hitPointsBefore - monster->currentHitPoints;
	const int monsterIndex = findRoomMonsterIndexByName(_scene.state, monster->monsterName);
	MonsterCombatState *combatState =
		monsterIndex >= 0 && monsterIndex < static_cast<int>(_monsterCombatStates.size())
			? &_monsterCombatStates[static_cast<uint>(monsterIndex)]
			: nullptr;
	debugC(1, kDebugCombat,
		"Harvester: combat player monster hit target='%s' loadout=%d weapon='%s' damage=%d damage_type='%s' hp=%d->%d reachable=1",
		monster->monsterName.c_str(), playerState.combatLoadout,
		Player::describeCombatLoadout(playerState.combatLoadout), damageAmount,
		Player::describeCombatDamageType(damageType), hitPointsBefore, monster->currentHitPoints);
	if (damageLanded > 0) {
		_combatEffects.spawnCombatDamagePopup(*monsterEntity, monster->monsterName, damageLanded);
		startMonsterHitReaction(*monster, *monsterEntity, combatState, playerState.attackFirstFrame, needsRedraw);
	}
	if (monster->currentHitPoints > 0) {
		(void)script.syncRuntimeMonsterRecord(*monster);
		needsRedraw = true;
		return Common::kNoError;
	}
	if (combatState)
		clearRoomMonsterCombatState(*combatState);
	(void)script.syncRuntimeMonsterRecord(*monster);

	DeathAnimationRange deathRange;
	if (combatState && resolveMonsterDeathAnimationRange(*monsterEntity, monster->facing,
			damageType, _engine.isGoreEnabled(), deathRange)) {
		startMonsterDeathAnimation(*monster, *monsterEntity, *combatState, deathRange, damageType);
		debugC(1, kDebugCombat,
			"Harvester: combat monster death start target='%s' damage_type=%d frames=%d..%d gore=%d on_death='%s'",
			monster->monsterName.c_str(), damageType,
			deathRange.firstFrame, deathRange.lastFrame,
			_engine.isGoreEnabled(), monster->onDeathActionTag.c_str());
		needsRedraw = true;
		return Common::kNoError;
	}

	monster->active = false;
	monster->visible = false;
	const bool herrillDeathQueued = queueInnerSanctumHerrillDeathAfterMuck(*monster);
	debugC(1, kDebugCombat,
		"Harvester: combat monster defeated target='%s' on_death='%s' fallback='no death bank'",
		monster->monsterName.c_str(), monster->onDeathActionTag.c_str());
	InteractionResult interaction;
	interaction.mutatedRuntimeState = true;
	interaction.visualRuntimeStateChanged = true;
	if (!monster->onDeathActionTag.empty()) {
		InteractionResult deathInteraction;
		if (script.executeActionTag(
				monster->onDeathActionTag, deathInteraction, true, ScriptExecutionContext::forMonsterDeath(*monster))) {
			interaction = Common::move(deathInteraction);
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
	}
	if (herrillDeathQueued) {
		interaction.mutatedRuntimeState = true;
		interaction.visualRuntimeStateChanged = true;
	}

	return _interactionProcessor.handleCombatInteraction(interaction, needsRedraw);
}

Common::Error Combat::killDebugActiveMonster(const Common::Point &mousePos, bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	if (!_engine.isCombatDebugEnabled())
		return Common::kNoError;
	MonsterRecord *monster = resolveKillTarget(mousePos);
	Entity *monsterEntity = monster ? findSceneRuntimeEntity(_entityManager, monster->monsterName) : nullptr;
	if (!monster || !monsterEntity || !monsterEntity->isVisible()) {
		debugC(1, kDebugCombat,
			"Harvester: debug combat kill skipped reason='no active monster' cursor=(%d,%d)",
			mousePos.x, mousePos.y);
		return Common::kNoError;
	}

	Script *script = _engine.getScript();
	if (!script) {
		debugC(1, kDebugCombat,
			"Harvester: debug combat kill skipped target='%s' reason='no startup script'",
			monster->monsterName.c_str());
		return Common::kNoError;
	}

	const int debugDamageType = Player::resolveCombatLoadoutDamageType(playerState.combatLoadout);
	const int damageLanded = monster->currentHitPoints;
	monster->currentHitPoints = 0;
	if (damageLanded > 0)
		_combatEffects.spawnCombatDamagePopup(*monsterEntity, monster->monsterName, damageLanded);

	if (playerState.attackTargetClassId == kEntityClassMonster &&
			playerState.attackTargetName.equalsIgnoreCase(monster->monsterName)) {
		_player.clearAttackTarget();
	}

	debugC(1, kDebugCombat,
		"Harvester: debug combat kill target='%s' damage=%d damage_type=%d cursor=(%d,%d)",
		monster->monsterName.c_str(), damageLanded, debugDamageType, mousePos.x, mousePos.y);

	const int monsterIndex = findRoomMonsterIndexByName(_scene.state, monster->monsterName);
	MonsterCombatState *combatState =
		monsterIndex >= 0 && monsterIndex < static_cast<int>(_monsterCombatStates.size())
			? &_monsterCombatStates[static_cast<uint>(monsterIndex)]
			: nullptr;
	if (combatState)
		clearRoomMonsterCombatState(*combatState);
	(void)script->syncRuntimeMonsterRecord(*monster);

	DeathAnimationRange deathRange;
	if (combatState && resolveMonsterDeathAnimationRange(*monsterEntity, monster->facing,
			debugDamageType, _engine.isGoreEnabled(), deathRange)) {
		startMonsterDeathAnimation(*monster, *monsterEntity, *combatState, deathRange, debugDamageType);
		needsRedraw = true;
		return Common::kNoError;
	}

	monster->active = false;
	monster->visible = false;
	InteractionResult interaction;
	interaction.mutatedRuntimeState = true;
	interaction.visualRuntimeStateChanged = true;
	if (!monster->onDeathActionTag.empty()) {
		InteractionResult deathInteraction;
		if (script->executeActionTag(
				monster->onDeathActionTag, deathInteraction, true, ScriptExecutionContext::forMonsterDeath(*monster))) {
			interaction = Common::move(deathInteraction);
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
	}

	return _interactionProcessor.handleCombatInteraction(interaction, needsRedraw);
}

} // End of namespace Harvester
