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
#include "harvester/cheats.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"
#include "harvester/scene/interaction.h"
#include "harvester/player/player.h"

namespace Harvester {

const int kRoomMonsterAnimationRate = 17;
const int kNativeMonsterAttackAnimationRate = 30;
const uint32 kNativeMonsterAttackCooldownBaseTicks = 50;
const int kDefaultMonsterAttackContactFrameOffset = 2;
const int kMuckeyAttackContactFrameOffset = 7;
const float kNativeMonsterPursuitZTolerance = 2.0f;
const float kNativeMonsterHorizontalWaypointTolerance = 50.0f;

namespace {

struct RoomMonsterFacingAnimationRange {
	RoomMonsterFacingAnimationRange() {}
	RoomMonsterFacingAnimationRange(int walkFirstFrame, int walkLastFrame, int idleFrame)
		: walkFirstFrame(walkFirstFrame), walkLastFrame(walkLastFrame), idleFrame(idleFrame) {}

	int walkFirstFrame = 0;
	int walkLastFrame = 0;
	int idleFrame = 0;
};

RoomMonsterFacingAnimationRange resolveRoomMonsterFacingAnimationRange(int facing) {
	switch (facing) {
	case 0:
		return RoomMonsterFacingAnimationRange(0x00, 0x09, 0x3b);
	case 1:
		return RoomMonsterFacingAnimationRange(0x0f, 0x18, 0x0e);
	case 2:
		return RoomMonsterFacingAnimationRange(0x2d, 0x36, 0x2c);
	case 3:
		return RoomMonsterFacingAnimationRange(0x1e, 0x27, 0x28);
	default:
		return RoomMonsterFacingAnimationRange(0x00, 0x09, 0x3b);
	}
}

} // End of anonymous namespace

struct Combat::MonsterPursuitResult {
	bool moved;
	int playerCenterX;
	int monsterCenterX;
	int liveCenterDx;
	int absLiveCenterDx;
	float zDelta;
	int engageDistance;
};

bool setRoomMonsterAnimation(Entity &entity, int facing, bool walking) {
	const RoomMonsterFacingAnimationRange range = resolveRoomMonsterFacingAnimationRange(facing);
	if (walking) {
		const bool changed = entity.getAnimationRate() != kRoomMonsterAnimationRate ||
			entity.getCurrentFrame() < range.walkFirstFrame ||
			entity.getCurrentFrame() > range.walkLastFrame;
		entity.setAnimationRate(kRoomMonsterAnimationRate);
		entity.setAnimationFrameRange(range.walkFirstFrame, range.walkLastFrame, true);
		entity.setAnimationEnabled(true);
		return changed;
	}

	const int standingFrame = entity.hasOpaqueFramesInRange(range.idleFrame, range.idleFrame)
		? range.idleFrame
		: range.walkFirstFrame;
	const bool changed = entity.getAnimationRate() != 0 ||
		entity.getCurrentFrame() != standingFrame;
	entity.setAnimationRate(0);
	entity.setAnimationFrameRange(standingFrame, standingFrame, false);
	entity.setCurrentFrame(standingFrame);
	entity.setAnimationEnabled(false);
	return changed;
}

bool resolveMonsterAttackAnimationRange(HarvesterEngine &engine,
		const Entity &entity, int actorCenterX, int targetCenterX,
		AttackAnimationRange &range) {
	// Class-6 attackers pick one of the 0x16..0x1b attack states, which map to
	// the 0x50..0x8b strike banks. The 0x8c..0x9d banks are hit reactions, not attacks.
	static const AttackAnimationRange kLeftAttackRanges[3] = {
		AttackAnimationRange(0x64, 0x6d, 1),
		AttackAnimationRange(0x5a, 0x63, 1),
		AttackAnimationRange(0x50, 0x59, 1)
	};
	static const AttackAnimationRange kRightAttackRanges[3] = {
		AttackAnimationRange(0x82, 0x8b, 2),
		AttackAnimationRange(0x78, 0x81, 2),
		AttackAnimationRange(0x6e, 0x77, 2)
	};

	const AttackAnimationRange *candidates =
		targetCenterX < actorCenterX ? kLeftAttackRanges : kRightAttackRanges;
	int availableCandidateIndices[3];
	int availableCandidateCount = 0;
	for (int i = 0; i < 3; ++i) {
		if (!runtimeEntityHasDrawableFrameRange(entity, candidates[i].firstFrame, candidates[i].lastFrame))
			continue;
		availableCandidateIndices[availableCandidateCount++] = i;
	}

	if (availableCandidateCount == 0)
		return false;

	const int selectedIndex = availableCandidateIndices[
		availableCandidateCount > 1 ? engine.getRandomNumber(availableCandidateCount - 1) : 0];
	range = candidates[selectedIndex];
	return true;
}

int stepTowardsRoomCombatInt(int current, int target, int step) {
	if (step <= 0 || current == target)
		return current;
	if (current < target)
		return MIN(current + step, target);
	return MAX(current - step, target);
}

bool playRandomRoomAttackSound(HarvesterEngine &engine, const Common::String &sound1,
		const Common::String &sound2, const Common::String &sound3) {
	const Common::String *availableSounds[3];
	uint availableCount = 0;
	if (!sound1.empty())
		availableSounds[availableCount++] = &sound1;
	if (!sound2.empty())
		availableSounds[availableCount++] = &sound2;
	if (!sound3.empty())
		availableSounds[availableCount++] = &sound3;
	if (availableCount == 0)
		return false;

	const uint soundIndex = availableCount > 1
		? engine.getRandomNumber(availableCount - 1)
		: 0;
	return engine.playSound(*availableSounds[soundIndex]);
}

int mapRoomDepthToScreenYForCombat(const RoomSetupState &state, float z, int fallbackY) {
	if (state.roomMaxZScreenY < 0 || state.roomMinZScreenY < state.roomMaxZScreenY)
		return fallbackY;
	if (state.roomMaxZ == state.roomMinZ)
		return CLIP<int>(fallbackY, state.roomMaxZScreenY, state.roomMinZScreenY);

	const float clampedZ = clampRoomDepthForEvent(state, z);
	const float offset = (static_cast<float>(state.roomMaxZ) - clampedZ) *
		static_cast<float>(state.roomMinZScreenY - state.roomMaxZScreenY) /
		static_cast<float>(state.roomMaxZ - state.roomMinZ);
	return CLIP<int>(state.roomMaxZScreenY + roundRoomCombatFloat(offset),
		state.roomMaxZScreenY, state.roomMinZScreenY);
}

int resolveMonsterAttackContactFrameOffset(const MonsterRecord &monster) {
	return monster.monsterName.equalsIgnoreCase("MUCKEY")
		? kMuckeyAttackContactFrameOffset
		: kDefaultMonsterAttackContactFrameOffset;
}

Common::Error Combat::updateRoomMonsterCombat(bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	if (!_entityManager)
		return Common::kNoError;

	Script &script = *_engine.getScript();
	const uint32 now = Player::getRuntimeClockTicks();
	const uint32 moveInterval = MAX<uint32>(1, 100U / static_cast<uint32>(kRoomMonsterAnimationRate));
	const int depthStep = MAX<int>(1, roundRoomCombatFloat(
		_scene.state.roomZVelocityStep > 0.0f ? _scene.state.roomZVelocityStep : 1.0f));
	bool playerAlive = script.getPlayerCurrentHitPoints() > 0;

	resizeCombatStates();

	const NpcCombatUpdateResult npcResult = updateRoomNpcCombat(needsRedraw);
	if (npcResult.finalizedDeath)
		return npcResult.error;

	for (uint i = 0; i < _scene.state.roomMonsters.size(); ++i) {
		MonsterRecord &monster = _scene.state.roomMonsters[i];
		MonsterCombatState &combatState = _monsterCombatStates[i];
		Entity *entity = findSceneRuntimeEntity(_entityManager, monster.monsterName);
		if (!entity || !monster.visible) {
			clearRoomMonsterCombatState(combatState);
			continue;
		}
		if (combatState.deathActive) {
			if (entity->getCurrentFrame() == combatState.deathLastFrame)
				return finalizeMonsterDeathTransition(script, monster, *entity, combatState, needsRedraw);
			continue;
		}
		if (monster.currentHitPoints <= 0) {
			clearRoomMonsterCombatState(combatState);
			continue;
		}

		if (!monster.active || !playerState.entity || !playerAlive) {
			clearRoomMonsterCombatState(combatState);
			needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, false) || needsRedraw;
			continue;
		}

		if (combatState.hitActive) {
			updateMonsterHitReaction(script, monster, *entity, combatState, needsRedraw);
			continue;
		}

		if (combatState.attackActive) {
			if (updateMonsterAttack(script, monster, *entity, combatState, playerAlive, needsRedraw))
				return Common::kNoError;
			continue;
		}

		const bool animationDrivesMovement =
			entity->isAnimationEnabled() && entity->getAnimationRate() != 0;
		if (animationDrivesMovement) {
			combatState.nextMovementTick = 0;
			if (!combatState.allowInitialPursuitStep && !entity->didAnimationAdvanceLastTick())
				continue;
		} else {
			if (combatState.nextMovementTick != 0 && (int32)(now - combatState.nextMovementTick) < 0)
				continue;
			combatState.nextMovementTick = now + moveInterval;
		}
		combatState.allowInitialPursuitStep = false;

		const MonsterPursuitResult pursuit = updateMonsterPursuit(
			script, monster, *entity, combatState, depthStep, needsRedraw);
		if (pursuit.moved)
			continue;
		beginMonsterAttack(monster, *entity, combatState, pursuit, now, needsRedraw);
	}

	return Common::kNoError;
}

bool Combat::updateMonsterAttack(Script &script, const MonsterRecord &monster,
		Entity &entity, MonsterCombatState &combatState, bool &playerAlive, bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	const int currentFrame = entity.getCurrentFrame();
	if (!combatState.attackSoundPlayed &&
			monster.attackSoundTriggerFrame >= 0 &&
			currentFrame >= combatState.attackFirstFrame + monster.attackSoundTriggerFrame) {
		(void)playRandomRoomAttackSound(_engine,
			monster.attackSound1, monster.attackSound2, monster.attackSound3);
		combatState.attackSoundPlayed = true;
	}
	if (!combatState.attackContactResolved &&
			combatState.attackContactFrame >= 0 &&
			currentFrame >= combatState.attackContactFrame) {
		const bool projectile = (monster.damageType & 4) != 0;
		const bool hasTrackedPlayerTarget =
			playerState.entity &&
			!combatState.attackTargetName.empty() &&
			playerState.entity->getName().equalsIgnoreCase(combatState.attackTargetName);
		const bool hasContact = playerAlive && !playerState.deathActive && hasTrackedPlayerTarget &&
			(projectile || entity.overlapsEntity(*playerState.entity));
		if (hasContact || projectile) {
			combatState.attackContactResolved = true;
			combatState.attackContactFrame = -1;
		}
		if (!hasContact) {
			if (entity.didAnimationAdvanceLastTick()) {
				debugC(3, kDebugCombat,
					"Harvester: combat monster attack miss monster='%s' frame=%d player_hp=%d reason='no live contact'",
					monster.monsterName.c_str(), currentFrame, script.getPlayerCurrentHitPoints());
			}
		} else {
			const int playerHitPointsBefore = script.getPlayerCurrentHitPoints();
			const bool damageDisabled = _cheats.isPlayerDamageDisabled();
			const bool changed = damageDisabled
				? false
				: script.adjustPlayerCurrentHitPoints(-monster.damageAmount);
			const int playerHitPointsAfter = script.getPlayerCurrentHitPoints();
			const int damageLanded = playerHitPointsBefore - playerHitPointsAfter;
			debugC(1, kDebugCombat,
				"Harvester: combat monster attack hit monster='%s' frame=%d damage=%d damage_type='%s' player_hp=%d->%d changed=%d damage_disabled=%d",
				monster.monsterName.c_str(), currentFrame, monster.damageAmount,
				Player::describeCombatDamageType(monster.damageType),
				playerHitPointsBefore, playerHitPointsAfter, changed,
				damageDisabled ? 1 : 0);
			if (damageLanded > 0 && playerState.entity) {
				_combatEffects.spawnCombatDamagePopup(*playerState.entity, playerState.entity->getName(), damageLanded);
				_player.cancelIdleAnimation();
				if (playerHitPointsAfter > 0 &&
						_player.startHitAnimation(_engine, combatState.attackFirstFrame)) {
					_callbacks.notePlayerActivity();
				}
			}
			if (changed && playerHitPointsAfter <= 0) {
				playerAlive = false;
				_callbacks.stopPlayerRegionInteraction();
				_callbacks.startPlayerDefeatSequence(
					"combat_monster_hit", monster.monsterName, monster.damageType);
				return true;
			}
			needsRedraw = true;
		}
	}
	if (currentFrame == combatState.attackLastFrame) {
		clearRoomMonsterAttackState(combatState);
		needsRedraw = setRoomMonsterAnimation(entity, monster.facing, false) || needsRedraw;
	}
	return false;
}

Combat::MonsterPursuitResult Combat::updateMonsterPursuit(Script &script, MonsterRecord &monster,
		Entity &entity, MonsterCombatState &combatState, int depthStep, bool &needsRedraw) const {
	const RoomPlayerState &playerState = _player.getState();
	const int horizontalStepBase = 8;
	const int previousX = monster.posX;
	const int previousY = monster.posY;
	const int previousZ = monster.posZ;
	const Common::Rect playerRect = playerState.entity->getScreenRect();
	const Common::Rect monsterRect = entity.getScreenRect();
	const int playerCenterX = playerRect.left + playerRect.width() / 2;
	const int monsterCenterX = monsterRect.left + monsterRect.width() / 2;
	const int liveCenterDx = playerCenterX - monsterCenterX;
	const int absLiveCenterDx = ABS(liveCenterDx);
	const int playerTargetZ = roundRoomCombatFloat(playerState.z);
	const float zDelta = playerState.z - static_cast<float>(monster.posZ);
	const float absZDelta = zDelta >= 0.0f ? zDelta : -zDelta;
	const int engageDistance = MAX(0, monster.engageDistance);
	const int liveWaypointTolerance = roundRoomCombatFloat(
		Player::computeDepthScale(_scene.state,
			MIN<float>(playerState.z, static_cast<float>(monster.posZ))) * kNativeMonsterHorizontalWaypointTolerance);
	bool attemptedMoveX = false;
	bool attemptedMoveZ = false;
	if (absZDelta > kNativeMonsterPursuitZTolerance) {
		attemptedMoveZ = true;
		monster.posZ = stepTowardsRoomCombatInt(monster.posZ, playerTargetZ, depthStep);
		monster.posZ = static_cast<int>(clampRoomDepthForEvent(_scene.state, static_cast<float>(monster.posZ)));
		monster.posY = mapRoomDepthToScreenYForCombat(_scene.state, static_cast<float>(monster.posZ), monster.posY);
	} else if (absLiveCenterDx > engageDistance) {
		// Native class-6 pursuit stores a target frame-left X, not a center X.
		const int desiredFrameLeftX = monsterCenterX < playerCenterX
			? playerRect.right + engageDistance - monsterRect.width() / 2
			: playerRect.left - engageDistance + monsterRect.width() / 2;
		if (ABS(monsterRect.left - desiredFrameLeftX) > liveWaypointTolerance) {
			attemptedMoveX = true;
			const int step = MAX<int>(1, roundRoomCombatFloat(
				Player::computeDepthScale(_scene.state, static_cast<float>(monster.posZ)) * static_cast<float>(horizontalStepBase)));
			const int monsterFrameLeftOffset = monsterRect.left - monster.posX;
			const int desiredLogicalCenterX = desiredFrameLeftX - monsterFrameLeftOffset;
			monster.posX = stepTowardsRoomCombatInt(monster.posX, desiredLogicalCenterX, step);
			monster.posX = CLIP<int>(monster.posX, monster.screenMinXBound, monster.screenMaxXBound);
		}
	}

	const bool moved = monster.posX != previousX || monster.posY != previousY || monster.posZ != previousZ;
	const MonsterPursuitResult result = {
		moved, playerCenterX, monsterCenterX, liveCenterDx, absLiveCenterDx, zDelta, engageDistance
	};
	const int stalledDirections = moved ? 0 : ((attemptedMoveX ? 1 : 0) | (attemptedMoveZ ? 2 : 0));
	const bool stallChanged = combatState.loggedPursuitStallDirections != stalledDirections;
	combatState.loggedPursuitStallDirections = stalledDirections;
	if (moved) {
		const int movedX = monster.posX - previousX;
		const int movedY = monster.posY - previousY;
		if (ABS(movedX) > ABS(movedY))
			monster.facing = movedX < 0 ? 1 : 2;
		else if (movedY != 0)
			monster.facing = movedY < 0 ? 3 : 0;
		(void)applyRoomActorPlacement(_scene.state, entity, monster.posX, monster.posY, static_cast<float>(monster.posZ));
		if (monster.posZ != previousZ)
			_entityManager->reinsertSceneEntity(&entity);
		(void)script.syncRuntimeMonsterRecord(monster);
		debugC(3, kDebugCombat,
			"Harvester: combat monster chase move monster='%s' live_center_dx=%d z_delta=%.2f engage=%d waypoint_tol=%d from=(%d,%d,z=%d) to=(%d,%d,z=%d) facing=%d",
			monster.monsterName.c_str(), liveCenterDx, static_cast<double>(zDelta), engageDistance, liveWaypointTolerance,
			previousX, previousY, previousZ, monster.posX, monster.posY, monster.posZ, monster.facing);
		needsRedraw = true;
		needsRedraw = setRoomMonsterAnimation(entity, monster.facing, true) || needsRedraw;
		return result;
	}

	if (stalledDirections != 0 && stallChanged) {
		debugC(2, kDebugCombat,
			"Harvester: combat monster chase stalled monster='%s' live_center_dx=%d z_delta=%.2f engage=%d waypoint_tol=%d pos=(%d,%d,z=%d) attempted_x=%d attempted_z=%d",
			monster.monsterName.c_str(), liveCenterDx, static_cast<double>(zDelta), engageDistance, liveWaypointTolerance,
			monster.posX, monster.posY, monster.posZ, attemptedMoveX, attemptedMoveZ);
	}
	return result;
}

void Combat::beginMonsterAttack(MonsterRecord &monster, Entity &entity, MonsterCombatState &combatState,
		const MonsterPursuitResult &pursuit, uint32 now, bool &needsRedraw) const {
	const RoomPlayerState &playerState = _player.getState();
	needsRedraw = setRoomMonsterAnimation(entity, monster.facing, false) || needsRedraw;
	const bool closeEnoughForAttack = isWithinNativeMonsterAttackEntryRange(
		entity, static_cast<float>(monster.posZ), *playerState.entity, playerState.z, pursuit.engageDistance);
	// Class-6 monsters store `now + random(99)` when they arm an attack, then
	// apply a 0x32-tick gate when deciding whether a later strike may start.
	// A fresh zeroed spawn therefore attacks immediately once it reaches engage range.
	const uint32 attackCooldownDeadline =
		combatState.attackCooldownSeedTick + kNativeMonsterAttackCooldownBaseTicks;
	const bool attackCooldownExpired = combatState.attackCooldownSeedTick == 0 ||
		(int32)(now - attackCooldownDeadline) > 0;
	const int closeRangeBypassDistance = MAX(1, pursuit.engageDistance / 3);
	if (!closeEnoughForAttack)
		return;
	if (!attackCooldownExpired && pursuit.absLiveCenterDx > closeRangeBypassDistance)
		return;

	AttackAnimationRange range;
	if (!resolveMonsterAttackAnimationRange(_engine, entity, pursuit.monsterCenterX, pursuit.playerCenterX, range)) {
		debugC(1, kDebugCombat,
			"Harvester: combat monster attack skipped monster='%s' reason='no native attack bank' target_side=%s",
			monster.monsterName.c_str(), pursuit.playerCenterX < pursuit.monsterCenterX ? "left" : "right");
		return;
	}
	combatState.attackActive = true;
	combatState.attackFirstFrame = range.firstFrame;
	combatState.attackLastFrame = range.lastFrame;
	combatState.attackContactFrame =
		range.firstFrame + resolveMonsterAttackContactFrameOffset(monster);
	combatState.attackResumeFacing = range.resumeFacing;
	combatState.attackSoundPlayed =
		monster.attackSound1.empty() && monster.attackSound2.empty() && monster.attackSound3.empty();
	combatState.attackContactResolved = false;
	combatState.attackTargetName = playerState.entity->getName();
	combatState.attackCooldownSeedTick = now + _engine.getRandomNumber(99);
	combatState.loggedPursuitStallDirections = 0;
	monster.facing = range.resumeFacing;
	entity.setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	entity.setAnimationRate(kNativeMonsterAttackAnimationRate);
	entity.setAnimationEnabled(true);
	entity.setCurrentFrame(range.firstFrame);
	debugC(2, kDebugCombat,
		"Harvester: combat monster attack start monster='%s' target='%s'",
		monster.monsterName.c_str(), combatState.attackTargetName.c_str());
	debugC(3, kDebugCombat,
		"Harvester: combat monster attack timing monster='%s' target='%s' frames=%d..%d contact=%d resume_facing=%d live_center_dx=%d z_delta=%.2f engage=%d rate=%d attack_seed_tick=%u close_bypass=%d",
		monster.monsterName.c_str(), combatState.attackTargetName.c_str(), range.firstFrame, range.lastFrame,
		combatState.attackContactFrame, range.resumeFacing, pursuit.liveCenterDx, static_cast<double>(pursuit.zDelta),
		pursuit.engageDistance, kNativeMonsterAttackAnimationRate, combatState.attackCooldownSeedTick,
		closeRangeBypassDistance);
	needsRedraw = true;
}

} // End of namespace Harvester
