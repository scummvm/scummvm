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

#include "harvester/combat.h"

#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/screen.h"
#include "harvester/cheats.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/player.h"
#include "harvester/interaction.h"
#include "harvester/scene_entities.h"
#include "harvester/entity.h"

namespace Harvester {

const int kRoomMonsterAnimationRate = 17;
const int kNativeMonsterAttackAnimationRate = 30;
const uint32 kNativeMonsterAttackCooldownBaseTicks = 50;
const float kNativeNpcMonsterZExtent = 5.0f;
const int kRoomNpcAmbientLastFrame = 0x3b;
const int kDefaultMonsterAttackContactFrameOffset = 2;
const int kMuckeyAttackContactFrameOffset = 7;
const float kNativeMonsterPursuitZTolerance = 2.0f;
const float kNativeMonsterHorizontalWaypointTolerance = 50.0f;
const uint32 kNativeKeyboardAttackRepeatTicks = 25;
const char *const kNativeHitEffectResourcePath = "blood.abm";
const int kNativeHitEffectAnimationRate = 10;
const float kNativeHitEffectRenderZBias = 0.01f;
const uint32 kCombatDebugDamagePopupDurationMs = 750;
const int kCombatDebugDamagePopupRisePixels = 18;
const int kCombatDebugDamagePopupVerticalGap = 6;

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

static byte findNearestPaletteColor(const byte *palette, byte red, byte green, byte blue) {
	if (!palette)
		return 0;

	byte bestIndex = 0;
	uint32 bestDistance = 0xffffffffu;
	for (int i = 0; i < 256; ++i) {
		const int paletteOffset = i * 3;
		const int dr = static_cast<int>(palette[paletteOffset]) - red;
		const int dg = static_cast<int>(palette[paletteOffset + 1]) - green;
		const int db = static_cast<int>(palette[paletteOffset + 2]) - blue;
		const uint32 distance = static_cast<uint32>(dr * dr + dg * dg + db * db);
		if (distance < bestDistance) {
			bestDistance = distance;
			bestIndex = (byte)i;
		}
	}

	return bestIndex;
}

static void drawShadowedRoomText(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &text, int x, int y, byte textColor, byte shadowColor) {
	if (text.empty())
		return;

	const int width = font.getStringWidth(text);
	font.drawString(&screen, text, x + 1, y + 1, width, shadowColor);
	font.drawString(&screen, text, x, y, width, textColor);
}


struct RoomMonsterFacingAnimationRange {
	RoomMonsterFacingAnimationRange() {}
	RoomMonsterFacingAnimationRange(int walkFirstFrame, int walkLastFrame, int idleFrame)
		: walkFirstFrame(walkFirstFrame), walkLastFrame(walkLastFrame), idleFrame(idleFrame) {}

	int walkFirstFrame = 0;
	int walkLastFrame = 0;
	int idleFrame = 0;
};

Common::String resolveCombatLoadoutStatusLabel(Script &script, const ObjectRecord &object) {
	Common::String label = script.resolveInventoryTooltipText(object);
	if (!label.empty()) {
		for (uint i = 0; i < label.size(); ++i) {
			if (label[i] == '_')
				label.setChar(' ', i);
		}

		uint trimAt = label.size();
		for (uint i = 0; i < label.size(); ++i) {
			if (label[i] == ',' || label[i] == ';') {
				trimAt = i;
				break;
			}
		}
		label = label.substr(0, trimAt);
		while (!label.empty() && label.lastChar() == ' ')
			label.deleteLastChar();
		if (!label.empty())
			return label;
	}

	label = script.resolveObjectLabel(object);
	if (!label.empty())
		return label;

	label = object.objectName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

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

bool resolveCombatLoadoutHudInfo(int loadout, CombatLoadoutHudInfo &info) {
	switch (loadout) {
	case 2:
		info.iconIndex = 2;
		info.maxVisibleCount = 16;
		info.unitLabel = "nails";
		return true;
	case 3:
		info.iconIndex = 1;
		info.maxVisibleCount = 16;
		info.unitLabel = "shells";
		return true;
	case 4:
		info.iconIndex = 0;
		info.maxVisibleCount = 8;
		info.unitLabel = "bullets";
		return true;
	case 5:
		info.iconIndex = 0;
		info.maxVisibleCount = 6;
		info.unitLabel = "bullets";
		return true;
	case 14:
		info.iconIndex = 3;
		info.maxVisibleCount = 16;
		info.unitLabel = "gas";
		return true;
	default:
		return false;
	}
}

Common::String buildCombatLoadoutStatusMessage(Script &script, const ObjectRecord &object,
		int previousLoadout, int currentLoadout) {
	const Common::String label = resolveCombatLoadoutStatusLabel(script, object);
	if (label.empty())
		return Common::String();

	if (currentLoadout == 0 && previousLoadout != 0)
		return Common::String::format("Disarming %s...", label.c_str());

	CombatLoadoutHudInfo info;
	if (resolveCombatLoadoutHudInfo(currentLoadout, info))
		return Common::String::format("Arming %s, %d %s...", label.c_str(),
			script.getPlayerCombatResourceCount(currentLoadout), info.unitLabel);

	return Common::String::format("Arming %s...", label.c_str());
}

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
	// Native class-6 attackers pick one of the 0x16..0x1b attack states, which map to
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

Common::Error Combat::updateRoomMonsterCombat(bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	if (!_entityManager)
		return Common::kNoError;

	Script &script = *_engine.getScript();
	const uint32 now = Player::getRuntimeClockTicks();
	const uint32 moveInterval = MAX<uint32>(1, 100U / static_cast<uint32>(kRoomMonsterAnimationRate));
	const int horizontalStepBase = 8;
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
			const int currentFrame = entity->getCurrentFrame();
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
				combatState.attackContactResolved = true;
				combatState.attackContactFrame = -1;
				const bool hasTrackedPlayerTarget =
					playerState.entity &&
					!combatState.attackTargetName.empty() &&
					playerState.entity->getName().equalsIgnoreCase(combatState.attackTargetName);
				if (!playerAlive || !hasTrackedPlayerTarget) {
					debugC(1, kDebugCombat,
						"Harvester: combat monster attack miss monster='%s' frame=%d player_hp=%d reason='no live target'",
						monster.monsterName.c_str(), currentFrame, script.getPlayerCurrentHitPoints());
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
						return Common::kNoError;
					}
				}
				needsRedraw = true;
			}
			if (currentFrame == combatState.attackLastFrame) {
				clearRoomMonsterAttackState(combatState);
				needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, false) || needsRedraw;
			}
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

		const int previousX = monster.posX;
		const int previousY = monster.posY;
		const int previousZ = monster.posZ;
		const Common::Rect playerRect = playerState.entity->getScreenRect();
		const Common::Rect monsterRect = entity->getScreenRect();
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
			(void)applyRoomActorPlacement(_scene.state, *entity, monster.posX, monster.posY, static_cast<float>(monster.posZ));
			if (monster.posZ != previousZ)
				_entityManager->reinsertSceneEntity(entity);
			(void)script.syncRuntimeMonsterRecord(monster);
			debugC(3, kDebugCombat,
				"Harvester: combat monster chase move monster='%s' live_center_dx=%d z_delta=%.2f engage=%d waypoint_tol=%d from=(%d,%d,z=%d) to=(%d,%d,z=%d) facing=%d",
				monster.monsterName.c_str(), liveCenterDx, static_cast<double>(zDelta), engageDistance, liveWaypointTolerance,
				previousX, previousY, previousZ, monster.posX, monster.posY, monster.posZ, monster.facing);
			needsRedraw = true;
			needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, true) || needsRedraw;
			continue;
		}

		if (stalledDirections != 0 && stallChanged) {
			debugC(2, kDebugCombat,
				"Harvester: combat monster chase stalled monster='%s' live_center_dx=%d z_delta=%.2f engage=%d waypoint_tol=%d pos=(%d,%d,z=%d) attempted_x=%d attempted_z=%d",
				monster.monsterName.c_str(), liveCenterDx, static_cast<double>(zDelta), engageDistance, liveWaypointTolerance,
				monster.posX, monster.posY, monster.posZ, attemptedMoveX, attemptedMoveZ);
		}

		needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, false) || needsRedraw;
		const bool closeEnoughForAttack = isWithinNativeMonsterAttackEntryRange(
			*entity, static_cast<float>(monster.posZ), *playerState.entity, playerState.z, engageDistance);
		// Class-6 monsters store `now + random(99)` when they arm an attack, then
		// apply a 0x32-tick gate when deciding whether a later strike may start.
		// A fresh zeroed spawn therefore attacks immediately once it reaches engage range.
		const uint32 attackCooldownDeadline =
			combatState.attackCooldownSeedTick + kNativeMonsterAttackCooldownBaseTicks;
		const bool attackCooldownExpired = combatState.attackCooldownSeedTick == 0 ||
			(int32)(now - attackCooldownDeadline) > 0;
		const int closeRangeBypassDistance = MAX(1, engageDistance / 3);
		if (!closeEnoughForAttack)
			continue;
		if (!attackCooldownExpired && absLiveCenterDx > closeRangeBypassDistance)
			continue;

		AttackAnimationRange range;
		if (!resolveMonsterAttackAnimationRange(_engine, *entity, monsterCenterX, playerCenterX, range)) {
			debugC(1, kDebugCombat,
				"Harvester: combat monster attack skipped monster='%s' reason='no native attack bank' target_side=%s",
				monster.monsterName.c_str(), playerCenterX < monsterCenterX ? "left" : "right");
			continue;
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
		entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
		entity->setAnimationRate(kNativeMonsterAttackAnimationRate);
		entity->setAnimationEnabled(true);
		entity->setCurrentFrame(range.firstFrame);
		debugC(2, kDebugCombat,
			"Harvester: combat monster attack start monster='%s' target='%s'",
			monster.monsterName.c_str(), combatState.attackTargetName.c_str());
		debugC(3, kDebugCombat,
			"Harvester: combat monster attack timing monster='%s' target='%s' frames=%d..%d contact=%d resume_facing=%d live_center_dx=%d z_delta=%.2f engage=%d rate=%d attack_seed_tick=%u close_bypass=%d",
			monster.monsterName.c_str(), combatState.attackTargetName.c_str(), range.firstFrame, range.lastFrame,
			combatState.attackContactFrame, range.resumeFacing, liveCenterDx, static_cast<double>(zDelta),
			engageDistance, kNativeMonsterAttackAnimationRate, combatState.attackCooldownSeedTick,
			closeRangeBypassDistance);
		needsRedraw = true;
	}

	return Common::kNoError;
}

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
	if (playerState.attackTargetClassId == kEntityClassNpc) {
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
			script->queueRuntimeNpcDeathOrMonsterfy(npc->npcName, damageType);
		const bool jimmyFlagChanged = npc->npcName.equalsIgnoreCase("JIMMY")
			? script->setRuntimeFlagValue("JIMMY_ATTACKED", true)
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

	// update_actor_runtime_state (0x4d750) latches melee contact only after a hit.
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
		(void)script->syncRuntimeMonsterRecord(*monster);
		needsRedraw = true;
		return Common::kNoError;
	}
	if (combatState)
		clearRoomMonsterCombatState(*combatState);
	(void)script->syncRuntimeMonsterRecord(*monster);

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
		if (script->executeActionTag(
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

CombatEffects::CombatEffects(EntityManager *entityManager) : _entityManager(entityManager) {
	debugC(1, kDebugCombat, "Harvester: created room combat effects entities=%d", _entityManager != nullptr);
}

void CombatEffects::clearHitEffects() {
	debugC(2, kDebugCombat, "Harvester: resetting room blood effects records=%u next_id=%u",
		static_cast<uint>(_hitEffectStates.size()), _nextCombatEffectId);
	_hitEffectStates.clear();
	_nextCombatEffectId = 0;
}

void CombatEffects::resolveHitEffectAnchor(const Entity &targetEntity, Common::Point &anchorPoint, float &renderZ) {
	const Common::Rect targetRect = targetEntity.getScreenRect();
	anchorPoint.x = targetRect.left + targetRect.width() / 2;
	anchorPoint.y = targetRect.top + targetRect.height() / 3;
	renderZ = targetEntity.getZ() - kNativeHitEffectRenderZBias;
}

void CombatEffects::resolveCombatDamagePopupAnchor(const Entity &targetEntity, Common::Point &anchorPoint) {
	const Common::Rect targetRect = targetEntity.getScreenRect();
	anchorPoint.x = targetRect.left + targetRect.width() / 2;
	anchorPoint.y = targetRect.top - kCombatDebugDamagePopupVerticalGap;
}

bool CombatEffects::spawnCombatHitEffect(Entity &sourceEntity, const Common::String &followTargetName) {
	if (!_entityManager)
		return false;

	HitEffectState effectState;
	effectState.entityName = Common::String::format("__combat_blood_%u", _nextCombatEffectId++);
	effectState.followTargetName = followTargetName;
	resolveHitEffectAnchor(sourceEntity, effectState.anchorPoint, effectState.renderZ);

	Entity *effectEntity = _entityManager->spawnSceneAnimationEntity(
		effectState.entityName, kNativeHitEffectResourcePath, effectState.anchorPoint,
		effectState.renderZ, kNativeHitEffectAnimationRate, true, true, false,
		false, false, 0);
	if (!effectEntity) {
		debugC(1, kDebugCombat, "Harvester: unable to spawn blood effect entity='%s' target='%s'",
			effectState.entityName.c_str(), followTargetName.c_str());
		return false;
	}

	_hitEffectStates.push_back(Common::move(effectState));
	return true;
}

void CombatEffects::spawnCombatDamagePopup(Entity &targetEntity, const Common::String &followTargetName, int damageAmount) {
	if (damageAmount <= 0)
		return;

	CombatDamagePopupState popupState;
	popupState.followTargetName = followTargetName;
	popupState.startTick = Player::getRuntimeClockTicks();
	popupState.damageAmount = damageAmount;
	resolveCombatDamagePopupAnchor(targetEntity, popupState.anchorPoint);
	_damagePopupStates.push_back(Common::move(popupState));
}

bool CombatEffects::syncCombatHitEffects() {
	if (!_entityManager)
		return false;

	bool changed = false;
	for (HitEffectState &effectState : _hitEffectStates) {
		Entity *effectEntity = _entityManager->findSceneEntityByName(effectState.entityName);
		if (!effectEntity)
			continue;

		if (!effectState.followTargetName.empty()) {
			Entity *targetEntity =
				_entityManager->findSceneEntityByName(effectState.followTargetName);
			if (targetEntity && targetEntity->isVisible()) {
				resolveHitEffectAnchor(*targetEntity, effectState.anchorPoint, effectState.renderZ);
			}
		}

		if (effectEntity->getX() != effectState.anchorPoint.x ||
				effectEntity->getY() != effectState.anchorPoint.y ||
				fabsf(effectEntity->getZ() - effectState.renderZ) > 0.0001f) {
			effectEntity->setPosition(
				effectState.anchorPoint.x, effectState.anchorPoint.y, effectState.renderZ);
			_entityManager->reinsertSceneEntity(effectEntity);
			changed = true;
		}
	}

	return changed;
}

void CombatEffects::drawCombatDamagePopups(Graphics::Screen &screen, bool combatDebugEnabled) {
	if (!combatDebugEnabled || _damagePopupStates.empty())
		return;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		return;

	byte displayPalette[256 * 3];
	screen.getPalette(displayPalette);
	const byte white = findNearestPaletteColor(displayPalette, 0xff, 0xff, 0xff);
	const byte black = findNearestPaletteColor(displayPalette, 0x00, 0x00, 0x00);
	const uint32 now = Player::getRuntimeClockTicks();
	for (CombatDamagePopupState &popupState : _damagePopupStates) {
		if (_entityManager && !popupState.followTargetName.empty()) {
			Entity *targetEntity =
				_entityManager->findSceneEntityByName(popupState.followTargetName);
			if (targetEntity && targetEntity->isVisible())
				resolveCombatDamagePopupAnchor(*targetEntity, popupState.anchorPoint);
		}

		const uint32 elapsed = now - popupState.startTick;
		const uint32 clampedElapsed = MIN<uint32>(elapsed, kCombatDebugDamagePopupDurationMs);
		const int rise = static_cast<int>(
			clampedElapsed * static_cast<uint32>(kCombatDebugDamagePopupRisePixels) /
			kCombatDebugDamagePopupDurationMs);
		const Common::String popupText = Common::String::format("+%d", popupState.damageAmount);
		const int textWidth = font->getStringWidth(popupText);
		const int drawX = CLIP<int>(popupState.anchorPoint.x - textWidth / 2,
			0, MAX(0, screen.w - textWidth));
		const int drawY = CLIP<int>(
			popupState.anchorPoint.y - font->getFontHeight() - rise,
			0, MAX(0, screen.h - font->getFontHeight()));
		drawShadowedRoomText(screen, *font, popupText, drawX, drawY, white, black);
	}
}

bool CombatEffects::pruneCombatHitEffects() {
	bool removedAny = false;
	for (uint i = 0; i < _hitEffectStates.size();) {
		const Common::String effectName = _hitEffectStates[i].entityName;
		Entity *effectEntity = _entityManager
			? _entityManager->findSceneEntityByName(effectName)
			: nullptr;
		if (!effectEntity) {
			_hitEffectStates.remove_at(i);
			continue;
		}

		const bool finished =
			!effectEntity->isAnimationEnabled() &&
			effectEntity->getCurrentFrame() == effectEntity->getLastFrame();
		if (finished && _hitEffectStates[i].finished) {
			Entity *removedEntity = _entityManager->detachSceneEntityByName(effectName);
			delete removedEntity;
			_hitEffectStates.remove_at(i);
			removedAny = true;
			continue;
		}

		_hitEffectStates[i].finished = finished;
		++i;
	}

	return removedAny;
}

bool CombatEffects::pruneCombatDamagePopups() {
	const uint32 now = Player::getRuntimeClockTicks();
	bool removedAny = false;
	for (uint i = 0; i < _damagePopupStates.size();) {
		if (now - _damagePopupStates[i].startTick < kCombatDebugDamagePopupDurationMs) {
			++i;
			continue;
		}

		_damagePopupStates.remove_at(i);
		removedAny = true;
	}

	return removedAny;
}

} // End of namespace Harvester
