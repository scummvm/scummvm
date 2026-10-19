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
#include "harvester/scene/scene_entities.h"

namespace Harvester {

const int kNativeMonsterHitAnimationRate = 5;
const int kNativeCombatHitKnockbackDistance = 18;
const int kNativeCombatHitKnockbackDecayStep = 3;
const char *const kInnerSanctumRoomName = "INNERSANCTUM";
const char *const kHerrillLogNpcName = "HERRILL_LOG";
const char *const kMuckeyMonsterName = "MUCKEY";
const char *const kActivateInnerSanctumDoorActionTag = "ACTV_INNERSAN_DOOR";
const int kNativeDefaultNpcDeathDamageType = 1;

bool shouldRetainNpcDeathEntityInCurrentRoom(const NpcRecord &npc) {
	return npc.monsterfyTargetName.empty();
}

bool isRetainedCurrentRoomNpcDeathRecord(const NpcRecord &npc) {
	return npc.deathOrMonsterfyFlag &&
		npc.monsterfyTargetName.empty() &&
		npc.deathDamageType != 0 &&
		npc.runtimeState >= 0;
}

bool runtimeEntityHasFrameRange(const Entity &entity, int firstFrame, int lastFrame) {
	if (!entity.hasFrames() || firstFrame < 0 || lastFrame < firstFrame)
		return false;

	return static_cast<uint>(lastFrame) < entity.getFrameCount();
}

bool runtimeEntityHasDrawableFrameRange(const Entity &entity, int firstFrame, int lastFrame) {
	return runtimeEntityHasFrameRange(entity, firstFrame, lastFrame) &&
		entity.hasOpaqueFramesInRange(firstFrame, lastFrame);
}

bool resolveMonsterDeathAnimationRange(const Entity &entity, int facing,
		int deathDamageType, bool goreEnabled, DeathAnimationRange &range) {
	const bool preferLeftBank = facing == 1;
	const DeathAnimationRange rightBludge(0xb0, 0xb9);
	const DeathAnimationRange leftBludge(0xba, 0xc3);
	const DeathAnimationRange rightSlash(0x100, 0x109);
	const DeathAnimationRange leftSlash(0x10a, 0x113);
	const DeathAnimationRange rightProjectile(0x114, 0x11d);
	const DeathAnimationRange leftProjectile(0x11e, 0x127);

	const DeathAnimationRange primaryBludge = preferLeftBank ? leftBludge : rightBludge;
	const DeathAnimationRange primarySlash = preferLeftBank ? leftSlash : rightSlash;
	const DeathAnimationRange primaryProjectile = preferLeftBank ? leftProjectile : rightProjectile;
	const DeathAnimationRange fallbackBludge = preferLeftBank ? rightBludge : leftBludge;
	const DeathAnimationRange fallbackSlash = preferLeftBank ? rightSlash : leftSlash;
	const DeathAnimationRange fallbackProjectile = preferLeftBank ? rightProjectile : leftProjectile;

	auto chooseIfAvailable = [&](const DeathAnimationRange &candidate) {
		if (!runtimeEntityHasDrawableFrameRange(entity, candidate.firstFrame, candidate.lastFrame))
			return false;
		range = candidate;
		return true;
	};

	if ((!goreEnabled || deathDamageType == 1) &&
			(chooseIfAvailable(primaryBludge) || chooseIfAvailable(fallbackBludge)))
		return true;
	if (deathDamageType == 2 &&
			(chooseIfAvailable(primarySlash) || chooseIfAvailable(fallbackSlash)))
		return true;
	if (deathDamageType == 4 &&
			(chooseIfAvailable(primaryProjectile) || chooseIfAvailable(fallbackProjectile)))
		return true;

	return chooseIfAvailable(primaryBludge) || chooseIfAvailable(primarySlash) ||
		chooseIfAvailable(primaryProjectile) || chooseIfAvailable(fallbackBludge) ||
		chooseIfAvailable(fallbackSlash) || chooseIfAvailable(fallbackProjectile);
}

bool resolveNpcDeathAnimationRange(const Entity &entity, bool hasMonsterfyTarget,
		int deathDamageType, bool goreEnabled, DeathAnimationRange &range) {
	const DeathAnimationRange goreBank(0x3c, 0x45);
	const DeathAnimationRange bludgeBank(0x46, 0x4f);
	const bool preferGoreBank = hasMonsterfyTarget || (goreEnabled && deathDamageType != 1);

	auto chooseIfAvailable = [&](const DeathAnimationRange &candidate) {
		if (!runtimeEntityHasFrameRange(entity, candidate.firstFrame, candidate.lastFrame))
			return false;
		range = candidate;
		return true;
	};

	if (preferGoreBank && chooseIfAvailable(goreBank))
		return true;
	if (chooseIfAvailable(bludgeBank))
		return true;

	return chooseIfAvailable(goreBank);
}

bool resolveRoomMonsterHitAnimationRange(const Entity &entity,
		int attackerAttackFirstFrame, HitAnimationRange &range) {
	HitAnimationRange candidate;
	switch (attackerAttackFirstFrame) {
	case 0x64:
		candidate = HitAnimationRange(0x92, 0x94, 2, -kNativeCombatHitKnockbackDistance);
		break;
	case 0x5a:
		candidate = HitAnimationRange(0x8f, 0x91, 2, -kNativeCombatHitKnockbackDistance);
		break;
	case 0x50:
		candidate = HitAnimationRange(0x8c, 0x8e, 2, -kNativeCombatHitKnockbackDistance);
		break;
	case 0x82:
		candidate = HitAnimationRange(0x9b, 0x9d, 1, kNativeCombatHitKnockbackDistance);
		break;
	case 0x78:
		candidate = HitAnimationRange(0x98, 0x9a, 1, kNativeCombatHitKnockbackDistance);
		break;
	case 0x6e:
		candidate = HitAnimationRange(0x95, 0x97, 1, kNativeCombatHitKnockbackDistance);
		break;
	default:
		return false;
	}

	if (!runtimeEntityHasDrawableFrameRange(entity, candidate.firstFrame, candidate.lastFrame))
		return false;

	range = candidate;
	return true;
}

NpcCombatUpdateResult Combat::updateRoomNpcCombat(bool &needsRedraw) {
	NpcCombatUpdateResult result;
	for (uint i = 0; i < _scene.state.roomNpcs.size(); ++i) {
		NpcRecord &npc = _scene.state.roomNpcs[i];
		NpcCombatState &combatState = _npcCombatStates[i];
		Entity *entity = findSceneRuntimeEntity(_entityManager, npc.npcName);
		if (!entity || !npc.visible || npc.deathOrMonsterfyFlag) {
			clearRoomNpcCombatState(combatState);
			continue;
		}
		if (!combatState.deathActive && npc.active && npc.deathDamageType != 0) {
			if (beginNpcDeathTransition(npc, *entity, combatState, npc.deathDamageType, needsRedraw))
				continue;

			debugC(1, kDebugCombat,
				"Harvester: room npc death fallback target='%s' damage_type=%d on_death='%s' reason='no death bank'",
				npc.npcName.c_str(), npc.deathDamageType, npc.onDeathActionTag.c_str());
			combatState.deathDamageType = npc.deathDamageType;
			combatState.deathLastFrame = entity->getCurrentFrame();
			result.finalizedDeath = true;
			result.error = finalizeNpcDeathTransition(npc, combatState, needsRedraw);
			return result;
		}
		if (!combatState.deathActive)
			continue;
		if (entity->getCurrentFrame() != combatState.deathLastFrame)
			continue;

		entity->setAnimationRate(0);
		entity->setAnimationFrameRange(combatState.deathLastFrame, combatState.deathLastFrame, false);
		entity->setCurrentFrame(combatState.deathLastFrame);
		entity->setAnimationEnabled(false);
		result.finalizedDeath = true;
		result.error = finalizeNpcDeathTransition(npc, combatState, needsRedraw);
		return result;
	}
	return result;
}

bool Combat::queueInnerSanctumHerrillDeathAfterMuck(const MonsterRecord &monster) const {
	if (!monster.roomName.equalsIgnoreCase(kInnerSanctumRoomName) ||
			!monster.monsterName.equalsIgnoreCase(kMuckeyMonsterName) ||
			!monster.onDeathActionTag.equalsIgnoreCase(kActivateInnerSanctumDoorActionTag)) {
		return false;
	}

	Script *script = _engine.getScript();
	NpcRecord *herrill = findRoomNpcRecordByName(_scene.state, kHerrillLogNpcName);
	if (!script || !herrill || !herrill->visible ||
			herrill->deathOrMonsterfyFlag || herrill->deathDamageType != 0) {
		return false;
	}

	// There's a hard-coded HERRILL_LOG removal path
	// while the Inner Sanctum Muckey combat state is active; queue the ordinary
	// live NPC death so the room's timer delay can play Herrill's death bank.
	if (!script->queueRuntimeNpcDeathOrMonsterfy(
			kHerrillLogNpcName, kNativeDefaultNpcDeathDamageType)) {
		return false;
	}

	herrill->deathDamageType = kNativeDefaultNpcDeathDamageType;
	debugC(1, kDebugCombat,
		"Harvester: queued Inner Sanctum Herrill death after monster='%s'",
		monster.monsterName.c_str());
	return true;
}

bool Combat::beginNpcDeathTransition(NpcRecord &npc, Entity &npcEntity, NpcCombatState &combatState,
		int deathDamageType, bool &needsRedraw) const {
	DeathAnimationRange deathRange;
	if (!resolveNpcDeathAnimationRange(npcEntity, !npc.monsterfyTargetName.empty(),
			deathDamageType, _engine.isGoreEnabled(), deathRange))
		return false;

	combatState.deathActive = true;
	combatState.deathFirstFrame = deathRange.firstFrame;
	combatState.deathLastFrame = deathRange.lastFrame;
	combatState.deathDamageType = deathDamageType;
	npc.deathDamageType = deathDamageType;
	npcEntity.setAnimationFrameRange(deathRange.firstFrame, deathRange.lastFrame, false);
	npcEntity.setAnimationRate(npc.frameDelay > 0 ? npc.frameDelay : 0);
	npcEntity.setAnimationEnabled(true);
	npcEntity.setCurrentFrame(deathRange.firstFrame);
	if (!npc.audioPath.empty())
		(void)_engine.playSound(npc.audioPath);
	debugC(1, kDebugCombat,
		"Harvester: combat npc death start target='%s' damage_type=%d frames=%d..%d monsterfy='%s' on_death='%s'",
		npc.npcName.c_str(), deathDamageType, deathRange.firstFrame, deathRange.lastFrame,
		npc.monsterfyTargetName.c_str(), npc.onDeathActionTag.c_str());
	needsRedraw = true;
	return true;
}

Common::Error Combat::finalizeNpcDeathTransition(NpcRecord &npc, NpcCombatState &combatState, bool &needsRedraw) const {
	Script *script = _engine.getScript();
	Entity *npcEntity = findSceneRuntimeEntity(_entityManager, npc.npcName);
	// State 0x35 leaves non-monsterfied NPCs as a dead state-0x38 actor
	// in the current room, while the NpcRecord death flag suppresses later setup.
	const bool retainDeathEntity =
		shouldRetainNpcDeathEntityInCurrentRoom(npc) &&
		combatState.deathLastFrame >= 0 &&
		npcEntity;
	const int monsterfyPosZ = (!npc.monsterfyTargetName.empty() && npcEntity)
		? roundRoomCombatFloat(npcEntity->getZ())
		: Script::kNoMonsterfyPosZOverride;
	if (retainDeathEntity) {
		npcEntity->setAnimationRate(0);
		npcEntity->setAnimationFrameRange(combatState.deathLastFrame, combatState.deathLastFrame, false);
		npcEntity->setCurrentFrame(combatState.deathLastFrame);
		npcEntity->setAnimationEnabled(false);
		npcEntity->setHitTestMode(kEntityHitTestNone);
		npcEntity->setVisible(true);
	} else {
		_sceneRuntime.removeSceneEntityByName(npc.npcName);
	}
	npc.active = false;
	npc.visible = false;
	npc.savedVisible = false;
	npc.runtimeSpawned = retainDeathEntity;
	npc.runtimeState = retainDeathEntity ? combatState.deathLastFrame : -1;
	npc.deathOrMonsterfyFlag = true;
	if (combatState.deathDamageType != 0)
		npc.deathDamageType = combatState.deathDamageType;
	const bool runtimeChanged = script
		? script->finalizeRuntimeNpcDeathOrMonsterfy(
			npc.npcName, npc.deathDamageType, false, -1, monsterfyPosZ)
		: false;
	debugC(1, kDebugCombat,
		"Harvester: combat npc death complete target='%s' damage_type=%d last_frame=%d preserve_runtime_actor=%d monsterfy='%s' on_death='%s'",
		npc.npcName.c_str(), combatState.deathDamageType, combatState.deathLastFrame,
		retainDeathEntity, npc.monsterfyTargetName.c_str(), npc.onDeathActionTag.c_str());
	clearRoomNpcCombatState(combatState);

	InteractionResult interaction;
	interaction.mutatedRuntimeState = runtimeChanged;
	interaction.visualRuntimeStateChanged = runtimeChanged;
	if (script && !npc.onDeathActionTag.empty()) {
		InteractionResult deathInteraction;
		if (script->executeActionTag(npc.onDeathActionTag, deathInteraction, true,
				ScriptExecutionContext::forNpcDeath(npc))) {
			interaction = Common::move(deathInteraction);
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
	}

	return _interactionProcessor.handleCombatInteraction(interaction, needsRedraw);
}

void Combat::startMonsterHitReaction(MonsterRecord &monster, Entity &monsterEntity, MonsterCombatState *combatState,
		int attackerAttackFirstFrame, bool &needsRedraw) const {
	if (_combatEffects.spawnCombatHitEffect(monsterEntity, monster.monsterName))
		needsRedraw = true;

	if (!combatState) {
		(void)playRandomRoomAttackSound(_engine,
			monster.hitSound1, monster.hitSound2, monster.hitSound3);
		return;
	}

	HitAnimationRange range;
	if (!resolveRoomMonsterHitAnimationRange(monsterEntity, attackerAttackFirstFrame, range)) {
		(void)playRandomRoomAttackSound(_engine,
			monster.hitSound1, monster.hitSound2, monster.hitSound3);
		debugC(1, kDebugCombat,
			"Harvester: combat monster hit reaction skipped target='%s' attack_frame=%d reason='no native hit bank'",
			monster.monsterName.c_str(), attackerAttackFirstFrame);
		return;
	}

	clearRoomMonsterAttackState(*combatState);
	clearRoomMonsterHitState(*combatState);
	combatState->hitActive = true;
	combatState->hitFirstFrame = range.firstFrame;
	combatState->hitLastFrame = range.lastFrame;
	combatState->hitResumeFacing = range.resumeFacing;
	combatState->hitKnockbackRemainingX = range.knockbackX;
	combatState->hitKnockbackDecayStep = kNativeCombatHitKnockbackDecayStep;
	combatState->hitSoundPlayed =
		monster.hitSound1.empty() && monster.hitSound2.empty() && monster.hitSound3.empty();
	combatState->nextMovementTick = 0;
	monster.facing = range.resumeFacing;
	monsterEntity.setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	monsterEntity.setAnimationRate(kNativeMonsterHitAnimationRate);
	monsterEntity.setAnimationEnabled(true);
	monsterEntity.setCurrentFrame(range.firstFrame);
	if (!combatState->hitSoundPlayed && monster.hitSoundTriggerFrame <= 0) {
		(void)playRandomRoomAttackSound(_engine,
			monster.hitSound1, monster.hitSound2, monster.hitSound3);
		combatState->hitSoundPlayed = true;
	}
	debugC(1, kDebugCombat,
		"Harvester: combat monster hit reaction target='%s' attack_frame=%d frames=%d..%d resume_facing=%d knockback=%d rate=%d",
		monster.monsterName.c_str(), attackerAttackFirstFrame, range.firstFrame, range.lastFrame,
		range.resumeFacing, range.knockbackX, kNativeMonsterHitAnimationRate);
	needsRedraw = true;
}

void Combat::startMonsterDeathAnimation(const MonsterRecord &monster, Entity &monsterEntity,
		MonsterCombatState &combatState, const DeathAnimationRange &deathRange, int deathDamageType) const {
	combatState.deathActive = true;
	combatState.deathFirstFrame = deathRange.firstFrame;
	combatState.deathLastFrame = deathRange.lastFrame;
	combatState.deathDamageType = deathDamageType;
	combatState.nextMovementTick = 0;
	monsterEntity.setAnimationFrameRange(deathRange.firstFrame, deathRange.lastFrame, false);
	monsterEntity.setAnimationRate(kRoomMonsterAnimationRate);
	monsterEntity.setAnimationEnabled(true);
	monsterEntity.setCurrentFrame(deathRange.firstFrame);
	if (!monster.deathSound.empty())
		(void)_engine.playSound(monster.deathSound);
}

Common::Error Combat::finalizeMonsterDeathTransition(Script &script, MonsterRecord &monster,
		Entity &entity, MonsterCombatState &combatState, bool &needsRedraw) const {
	combatState.deathActive = false;
	monster.active = false;
	monster.visible = true;
	monster.runtimeSpawned = true;
	monster.runtimeState = entity.getCurrentFrame();
	entity.setAnimationRate(0);
	entity.setAnimationFrameRange(monster.runtimeState, monster.runtimeState, false);
	entity.setCurrentFrame(monster.runtimeState);
	entity.setAnimationEnabled(false);
	(void)script.syncRuntimeMonsterRecord(monster);
	debugC(1, kDebugCombat,
		"Harvester: combat monster death complete target='%s' damage_type=%d corpse_frame=%d on_death='%s'",
		monster.monsterName.c_str(), combatState.deathDamageType,
		monster.runtimeState,
		monster.onDeathActionTag.c_str());
	clearRoomMonsterCombatState(combatState);
	const bool herrillDeathQueued = queueInnerSanctumHerrillDeathAfterMuck(monster);

	InteractionResult interaction;
	interaction.mutatedRuntimeState = true;
	interaction.visualRuntimeStateChanged = true;
	if (!monster.onDeathActionTag.empty()) {
		InteractionResult deathInteraction;
		if (script.executeActionTag(
				monster.onDeathActionTag, deathInteraction, true, ScriptExecutionContext::forMonsterDeath(monster))) {
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

void Combat::updateMonsterHitReaction(Script &script, MonsterRecord &monster,
		Entity &entity, MonsterCombatState &combatState, bool &needsRedraw) const {
	const int currentFrame = entity.getCurrentFrame();
	if (!combatState.hitSoundPlayed &&
			monster.hitSoundTriggerFrame >= 0 &&
			currentFrame >= combatState.hitFirstFrame + monster.hitSoundTriggerFrame) {
		(void)playRandomRoomAttackSound(_engine,
			monster.hitSound1, monster.hitSound2, monster.hitSound3);
		combatState.hitSoundPlayed = true;
	}
	if (combatState.hitKnockbackRemainingX != 0) {
		const int step = MAX(1, ABS(combatState.hitKnockbackDecayStep));
		const int knockbackStep = combatState.hitKnockbackRemainingX > 0
			? MIN(combatState.hitKnockbackRemainingX, step)
			: MAX(combatState.hitKnockbackRemainingX, -step);
		const int previousX = monster.posX;
		monster.posX = CLIP<int>(
			monster.posX + knockbackStep, monster.screenMinXBound, monster.screenMaxXBound);
		if (monster.posX != previousX) {
			(void)applyRoomActorPlacement(
				_scene.state, entity, monster.posX, monster.posY, static_cast<float>(monster.posZ));
			(void)script.syncRuntimeMonsterRecord(monster);
			debugC(1, kDebugCombat,
				"Harvester: combat monster hit knockback target='%s' from_x=%d to_x=%d remaining=%d",
				monster.monsterName.c_str(), previousX, monster.posX,
				combatState.hitKnockbackRemainingX - (monster.posX - previousX));
			needsRedraw = true;
			combatState.hitKnockbackRemainingX -= monster.posX - previousX;
		} else {
			combatState.hitKnockbackRemainingX = 0;
		}
	}
	if (currentFrame == combatState.hitLastFrame) {
		clearRoomMonsterHitState(combatState);
		needsRedraw = setRoomMonsterAnimation(entity, monster.facing, false) || needsRedraw;
	}
}

} // End of namespace Harvester
