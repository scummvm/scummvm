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
// update_actor_runtime_state (0x4d750), states 0x16..0x1b: the block at
// 0x53525 forces a three-centisecond interval by assigning animation rate 0x1e.
const int kNativeMonsterAttackAnimationRate = 30;
const int kNativeMonsterHitAnimationRate = 5;
const uint32 kNativeMonsterAttackCooldownBaseTicks = 50;
const float kNativeNpcMonsterZExtent = 5.0f;
const int kRoomNpcAmbientLastFrame = 0x3b;
const int kDefaultMonsterAttackContactFrameOffset = 2;
const int kMuckeyAttackContactFrameOffset = 7;
const float kNativeMonsterPursuitZTolerance = 2.0f;
const float kNativeMonsterHorizontalWaypointTolerance = 50.0f;
const uint32 kNativeKeyboardAttackRepeatTicks = 25;
const int kNativeAttackSideWindow = 30;
const int kNativeCombatHitKnockbackDistance = 18;
const int kNativeCombatHitKnockbackDecayStep = 3;
const char *const kNativeHitEffectResourcePath = "blood.abm";
const int kNativeHitEffectAnimationRate = 10;
const float kNativeHitEffectRenderZBias = 0.01f;
const uint32 kCombatDebugDamagePopupDurationMs = 750;
const int kCombatDebugDamagePopupRisePixels = 18;
const int kCombatDebugDamagePopupVerticalGap = 6;
const char *const kInnerSanctumRoomName = "INNERSANCTUM";
const char *const kHerrillLogNpcName = "HERRILL_LOG";
const char *const kMuckeyMonsterName = "MUCKEY";
const char *const kActivateInnerSanctumDoorActionTag = "ACTV_INNERSAN_DOOR";
const int kNativeDefaultNpcDeathDamageType = 1;

namespace {

static Entity *findSceneRuntimeEntity(EntityManager *entityManager, const Common::String &name) {
	return entityManager ? entityManager->findSceneEntityByName(name) : nullptr;
}

static MonsterRecord *findRoomMonsterRecordByName(RoomSetupState &state, const Common::String &name) {
	for (MonsterRecord &monster : state.roomMonsters) {
		if (monster.monsterName.equalsIgnoreCase(name))
			return &monster;
	}
	return nullptr;
}

static NpcRecord *findRoomNpcRecordByName(RoomSetupState &state, const Common::String &name) {
	for (NpcRecord &npc : state.roomNpcs) {
		if (npc.npcName.equalsIgnoreCase(name))
			return &npc;
	}
	return nullptr;
}

static int findRoomNpcIndexByName(const RoomSetupState &state, const Common::String &name) {
	for (uint i = 0; i < state.roomNpcs.size(); ++i) {
		if (state.roomNpcs[i].npcName.equalsIgnoreCase(name))
			return (int)i;
	}

	return -1;
}

static int findRoomMonsterIndexByName(const RoomSetupState &state, const Common::String &name) {
	for (uint i = 0; i < state.roomMonsters.size(); ++i) {
		if (state.roomMonsters[i].monsterName.equalsIgnoreCase(name))
			return (int)i;
	}

	return -1;
}

static bool isKillableMonster(const MonsterRecord *monster) {
	return monster && monster->active && monster->visible && monster->currentHitPoints > 0;
}

static Entity *selectMonsterEntity(EntityManager *entityManager, const MonsterRecord &monster) {
	Entity *entity = findSceneRuntimeEntity(entityManager, monster.monsterName);
	return (entity && entity->isVisible()) ? entity : nullptr;
}

enum NativeAttackSide {
	kNativeAttackSideNone,
	kNativeAttackSideLeft,
	kNativeAttackSideRight
};

enum NativeAttackBand {
	kNativeAttackBandMid,
	kNativeAttackBandUpper,
	kNativeAttackBandLower
};

static bool resolveNativePlayerAttackGeometry(int attackFirstFrame, NativeAttackSide &attackSide, NativeAttackBand &attackBand) {
	attackSide = kNativeAttackSideNone;
	attackBand = kNativeAttackBandMid;

	switch (attackFirstFrame) {
	case 0x50:
		attackSide = kNativeAttackSideLeft;
		attackBand = kNativeAttackBandUpper;
		return true;
	case 0x5a:
		attackSide = kNativeAttackSideLeft;
		attackBand = kNativeAttackBandMid;
		return true;
	case 0x64:
		attackSide = kNativeAttackSideLeft;
		attackBand = kNativeAttackBandLower;
		return true;
	case 0x6e:
		attackSide = kNativeAttackSideRight;
		attackBand = kNativeAttackBandUpper;
		return true;
	case 0x78:
		attackSide = kNativeAttackSideRight;
		attackBand = kNativeAttackBandMid;
		return true;
	case 0x82:
		attackSide = kNativeAttackSideRight;
		attackBand = kNativeAttackBandLower;
		return true;
	default:
		return false;
	}
}

static byte findNearestPaletteColor(const byte *palette, byte red, byte green, byte blue) {
	if (!palette)
		return 0;

	byte bestIndex = 0;
	uint32 bestDistance = 0xffffffffu;
	for (int i = 0; i < 256; ++i) {
		const int paletteOffset = i * 3;
		const int dr = (int)palette[paletteOffset] - red;
		const int dg = (int)palette[paletteOffset + 1] - green;
		const int db = (int)palette[paletteOffset + 2] - blue;
		const uint32 distance = (uint32)(dr * dr + dg * dg + db * db);
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

int computeRectAxisGap(int minA, int maxA, int minB, int maxB) {
	if (maxA <= minB)
		return minB - maxA;
	if (maxB <= minA)
		return minA - maxB;
	return 0;
}

} // End of anonymous namespace

float clampRoomDepthForEvent(const RoomSetupState &state, float z) {
	return CLIP<float>(z,
		(float)MIN(state.roomMinZ, state.roomMaxZ),
		(float)MAX(state.roomMinZ, state.roomMaxZ));
}

bool shouldRetainNpcDeathEntityInCurrentRoom(const NpcRecord &npc) {
	return npc.monsterfyTargetName.empty();
}

bool isRetainedCurrentRoomNpcDeathRecord(const NpcRecord &npc) {
	return npc.deathOrMonsterfyFlag &&
		npc.monsterfyTargetName.empty() &&
		npc.deathDamageType != 0 &&
		npc.runtimeState >= 0;
}

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

bool runtimeEntityHasFrameRange(const Entity &entity, int firstFrame, int lastFrame) {
	if (!entity.hasFrames() || firstFrame < 0 || lastFrame < firstFrame)
		return false;

	return (uint)lastFrame < entity.getFrameCount();
}

bool runtimeEntityHasDrawableFrameRange(const Entity &entity, int firstFrame, int lastFrame) {
	return runtimeEntityHasFrameRange(entity, firstFrame, lastFrame) &&
		entity.hasOpaqueFramesInRange(firstFrame, lastFrame);
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

int stepTowardsRoomCombatInt(int current, int target, int step) {
	if (step <= 0 || current == target)
		return current;
	if (current < target)
		return MIN(current + step, target);
	return MAX(current - step, target);
}

int computeRuntimeEntityHorizontalGap(const Entity &left, const Entity &right) {
	const Common::Rect leftRect = left.getScreenRect();
	const Common::Rect rightRect = right.getScreenRect();
	return computeRectAxisGap(leftRect.left, leftRect.right, rightRect.left, rightRect.right);
}

bool doRuntimeEntityDepthExtentsOverlap(const Entity &first, const Entity &second) {
	const float firstZMin = first.getZ();
	const float firstZMax = firstZMin + first.getZExtent();
	const float secondZMin = second.getZ();
	const float secondZMax = secondZMin + second.getZExtent();
	return !(firstZMax < secondZMin || secondZMax < firstZMin);
}

bool areCombatantsWithinRoomCombatReach(const RoomSetupState &state,
		const Entity &attacker, float attackerZ,
		const Entity &target, float targetZ, int engageDistance) {
	if (attacker.overlapsEntity(target))
		return true;

	const float zDelta = attackerZ >= targetZ ? attackerZ - targetZ : targetZ - attackerZ;
	if (zDelta > kNativeMonsterPursuitZTolerance)
		return false;

	const float nearZ = attackerZ < targetZ ? attackerZ : targetZ;
	const int nativeTolerance = roundRoomCombatFloat(
		Player::computeDepthScale(state, nearZ) * kNativeMonsterHorizontalWaypointTolerance);
	return computeRuntimeEntityHorizontalGap(attacker, target) <= MAX(MAX(0, engageDistance), nativeTolerance);
}

bool isWithinNativeMonsterAttackEntryRange(const Entity &monster, float monsterZ,
		const Entity &player, float playerZ, int engageDistance) {
	const float zDelta = monsterZ >= playerZ ? monsterZ - playerZ : playerZ - monsterZ;
	if (zDelta > kNativeMonsterPursuitZTolerance)
		return false;

	const Common::Rect monsterRect = monster.getScreenRect();
	const Common::Rect playerRect = player.getScreenRect();
	const int monsterCenterX = monsterRect.left + monsterRect.width() / 2;
	const int playerCenterX = playerRect.left + playerRect.width() / 2;
	return ABS(playerCenterX - monsterCenterX) <= MAX(0, engageDistance);
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

int roundRoomCombatFloat(float value) {
	return value >= 0.0f ? (int)(value + 0.5f) : (int)(value - 0.5f);
}

int mapRoomDepthToScreenYForCombat(const RoomSetupState &state, float z, int fallbackY) {
	if (state.roomMaxZScreenY < 0 || state.roomMinZScreenY < state.roomMaxZScreenY)
		return fallbackY;
	if (state.roomMaxZ == state.roomMinZ)
		return CLIP<int>(fallbackY, state.roomMaxZScreenY, state.roomMinZScreenY);

	const float clampedZ = clampRoomDepthForEvent(state, z);
	const float offset = ((float)state.roomMaxZ - clampedZ) *
		(float)(state.roomMinZScreenY - state.roomMaxZScreenY) /
		(float)(state.roomMaxZ - state.roomMinZ);
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

MonsterRecord *Combat::findMonsterTargetAtPoint(RoomSetupState &state, EntityManager *entityManager, const Common::Point &point) const {
	MonsterRecord *bestMonster = nullptr;
	int bestZ = -0x7fffffff;
	int bestTop = -0x7fffffff;
	for (MonsterRecord &monster : state.roomMonsters) {
		if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
			continue;

		Entity *entity = findSceneRuntimeEntity(entityManager, monster.monsterName);
		if (!entity || !entity->isVisible())
			continue;

		const Common::Rect entityRect = entity->getScreenRect();
		if (!entityRect.contains(point))
			continue;

		if (!bestMonster || monster.posZ > bestZ ||
				(monster.posZ == bestZ && entityRect.top > bestTop)) {
			bestMonster = &monster;
			bestZ = monster.posZ;
			bestTop = entityRect.top;
		}
	}

	return bestMonster;
}

MonsterRecord *Combat::findOverlappingMonsterTarget(RoomSetupState &state, const RoomPlayerState &playerState,
		EntityManager *entityManager) const {
	if (!playerState.entity)
		return nullptr;

	for (MonsterRecord &monster : state.roomMonsters) {
		if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
			continue;

		Entity *entity = findSceneRuntimeEntity(entityManager, monster.monsterName);
		if (entity && areCombatantsWithinRoomCombatReach(state,
				*playerState.entity, playerState.z, *entity, (float)monster.posZ, monster.engageDistance))
			return &monster;
	}

	return nullptr;
}

NpcRecord *Combat::findOverlappingNpcTarget(RoomSetupState &state, const RoomPlayerState &playerState, EntityManager *entityManager) const {
	if (!playerState.entity)
		return nullptr;

	for (NpcRecord &npc : state.roomNpcs) {
		if (!npc.visible || npc.deathOrMonsterfyFlag || npc.deathDamageType != 0)
			continue;

		Entity *entity = findSceneRuntimeEntity(entityManager, npc.npcName);
		if (entity && areCombatantsWithinRoomCombatReach(state,
				*playerState.entity, playerState.z, *entity, entity->getZ(), 0))
			return &npc;
	}

	return nullptr;
}

bool Combat::playerAttackCanReachTarget(RoomSetupState &state, const RoomPlayerState &playerState,
		Entity *targetEntity, int engageDistance) const {
	if (!playerState.entity || !targetEntity)
		return false;
	if (Player::isProjectileCombatLoadout(playerState.combatLoadout))
		return true;

	return areCombatantsWithinRoomCombatReach(state,
		*playerState.entity, playerState.z, *targetEntity, targetEntity->getZ(), engageDistance);
}

MonsterRecord *Combat::findProjectileContactMonsterTarget(RoomSetupState &state, const RoomPlayerState &playerState,
		EntityManager *entityManager) const {
	if (!playerState.entity)
		return nullptr;

	NativeAttackSide attackSide = kNativeAttackSideNone;
	NativeAttackBand attackBand = kNativeAttackBandMid;
	if (!resolveNativePlayerAttackGeometry(playerState.attackFirstFrame, attackSide, attackBand))
		return nullptr;

	const Common::Rect playerRect = playerState.entity->getScreenRect();
	const int playerLeft = playerRect.left;
	const int playerRight = playerRect.right;
	const int playerCenterX = playerLeft + playerRect.width() / 2;
	MonsterRecord *bestMonster = nullptr;
	Entity *bestEntity = nullptr;
	int bestLeadingEdge = attackSide == kNativeAttackSideLeft ? -0x7fffffff : 0x7fffffff;

	for (MonsterRecord &monster : state.roomMonsters) {
		if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
			continue;

		Entity *entity = findSceneRuntimeEntity(entityManager, monster.monsterName);
		if (!entity || !entity->isVisible() ||
				!doRuntimeEntityDepthExtentsOverlap(*playerState.entity, *entity)) {
			continue;
		}

		const Common::Rect targetRect = entity->getScreenRect();
		const int targetLeft = targetRect.left;
		const int targetCenterX = targetLeft + targetRect.width() / 2;
		bool matchesDirection = false;
		if (attackSide == kNativeAttackSideLeft) {
			if (attackBand == kNativeAttackBandMid)
				matchesDirection = targetLeft < playerCenterX;
			else
				matchesDirection =
					targetLeft < playerCenterX && targetCenterX > playerLeft - kNativeAttackSideWindow;
			if (!matchesDirection || targetLeft <= bestLeadingEdge)
				continue;
		} else {
			if (attackBand == kNativeAttackBandMid)
				matchesDirection = playerCenterX < targetLeft;
			else
				matchesDirection =
					playerCenterX < targetLeft && targetCenterX < playerRight + kNativeAttackSideWindow;
			if (!matchesDirection || targetLeft >= bestLeadingEdge)
				continue;
		}

		bestMonster = &monster;
		bestEntity = entity;
		bestLeadingEdge = targetLeft;
	}

	if (bestMonster && bestEntity) {
		debugC(1, kDebugCombat,
			"Harvester: combat player projectile contact fallback target='%s' attack_frame=%d player_rect=(%d,%d)-(%d,%d) target_rect=(%d,%d)-(%d,%d)",
			bestMonster->monsterName.c_str(), playerState.attackFirstFrame,
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			bestEntity->getScreenRect().left, bestEntity->getScreenRect().top,
			bestEntity->getScreenRect().right, bestEntity->getScreenRect().bottom);
	}

	return bestMonster;
}

void Combat::capturePlayerAttackTargetForKeyboard(RoomSetupState &state, RoomPlayerState &playerState,
		EntityManager *entityManager, bool attackLeft, bool attackRight, bool attackUp, bool attackDown) const {
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	if (!playerState.entity)
		return;

	NativeAttackSide attackSide = kNativeAttackSideNone;
	NativeAttackBand attackBand = kNativeAttackBandMid;
	if (attackRight) {
		attackSide = kNativeAttackSideRight;
		attackBand = kNativeAttackBandMid;
	} else if (attackLeft) {
		attackSide = kNativeAttackSideLeft;
		attackBand = kNativeAttackBandMid;
	} else if (attackUp) {
		if (playerState.facing == 2) {
			attackSide = kNativeAttackSideRight;
			attackBand = kNativeAttackBandUpper;
		} else if (playerState.facing == 1) {
			attackSide = kNativeAttackSideLeft;
			attackBand = kNativeAttackBandUpper;
		}
	} else if (attackDown) {
		if (playerState.facing == 2) {
			attackSide = kNativeAttackSideRight;
			attackBand = kNativeAttackBandLower;
		} else if (playerState.facing == 1) {
			attackSide = kNativeAttackSideLeft;
			attackBand = kNativeAttackBandLower;
		}
	}
	if (attackSide == kNativeAttackSideNone) {
		debugC(1, kDebugCombat,
			"Harvester: combat player keyboard target capture class='none' input=(L=%d R=%d U=%d D=%d) facing=%d reason='no native attack family'",
			attackLeft, attackRight, attackUp, attackDown, playerState.facing);
		return;
	}

	const Common::Rect playerRect = playerState.entity->getScreenRect();
	const int playerLeft = playerRect.left;
	const int playerRight = playerRect.right;
	// Keyboard selection uses the cached center; projectile fallback uses the frame rectangle.
	const int playerCenterX = playerState.centerX;
	Entity *bestEntity = nullptr;
	Common::String bestName;
	int bestClassId = -1;
	int bestLeadingEdge = attackSide == kNativeAttackSideLeft ? -0x7fffffff : 0x7fffffff;

	auto considerTarget = [&](Entity *targetEntity, const Common::String &targetName, int classId) {
		if (!targetEntity || !targetEntity->isVisible() ||
				!doRuntimeEntityDepthExtentsOverlap(*playerState.entity, *targetEntity)) {
			return;
		}

		const Common::Rect targetRect = targetEntity->getScreenRect();
		const int targetLeft = targetRect.left;
		const int targetCenterX = targetLeft + (targetRect.width() / 2);
		bool matchesDirection = false;
		if (attackSide == kNativeAttackSideLeft) {
			if (attackBand == kNativeAttackBandMid)
				matchesDirection = targetLeft < playerCenterX;
			else
				matchesDirection =
					targetLeft < playerCenterX && targetCenterX > playerLeft - kNativeAttackSideWindow;
			if (!matchesDirection || targetLeft <= bestLeadingEdge)
				return;
		} else {
			if (attackBand == kNativeAttackBandMid)
				matchesDirection = playerCenterX < targetLeft;
			else
				matchesDirection =
					playerCenterX < targetLeft && targetCenterX < playerRight + kNativeAttackSideWindow;
			if (!matchesDirection || targetLeft >= bestLeadingEdge)
				return;
		}

		bestEntity = targetEntity;
		bestName = targetName;
		bestClassId = classId;
		bestLeadingEdge = targetLeft;
	};

	for (NpcRecord &npc : state.roomNpcs) {
		if (!npc.visible || npc.deathOrMonsterfyFlag || npc.deathDamageType != 0)
			continue;
		considerTarget(findSceneRuntimeEntity(entityManager, npc.npcName), npc.npcName, kEntityClassNpc);
	}
	for (MonsterRecord &monster : state.roomMonsters) {
		if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
			continue;
		considerTarget(findSceneRuntimeEntity(entityManager, monster.monsterName), monster.monsterName, kEntityClassMonster);
	}

	if (!bestEntity) {
		debugC(1, kDebugCombat,
			"Harvester: combat player keyboard target capture class='none' input=(L=%d R=%d U=%d D=%d) facing=%d reason='no directional target'",
			attackLeft, attackRight, attackUp, attackDown, playerState.facing);
		return;
	}

	playerState.attackTargetName = bestName;
	playerState.attackTargetClassId = bestClassId;
	debugC(1, kDebugCombat,
		"Harvester: combat player keyboard target capture class='%s' name='%s' input=(L=%d R=%d U=%d D=%d) facing=%d target_rect=(%d,%d)-(%d,%d)",
		describeCombatTargetClass(bestClassId), bestName.c_str(),
		attackLeft, attackRight, attackUp, attackDown, playerState.facing,
		bestEntity->getScreenRect().left, bestEntity->getScreenRect().top,
		bestEntity->getScreenRect().right, bestEntity->getScreenRect().bottom);
}

MonsterRecord *Combat::resolveKillTarget(RoomSetupState &state, const RoomPlayerState &playerState,
		EntityManager *entityManager, const Common::Point &mousePos) const {
	if (playerState.attackTargetClassId == kEntityClassMonster) {
		MonsterRecord *monster = findRoomMonsterRecordByName(state, playerState.attackTargetName);
		if (isKillableMonster(monster) && selectMonsterEntity(entityManager, *monster))
			return monster;
	}

	if (MonsterRecord *monster = findMonsterTargetAtPoint(state, entityManager, mousePos)) {
		if (isKillableMonster(monster) && selectMonsterEntity(entityManager, *monster))
			return monster;
	}

	if (playerState.entity) {
		for (uint i = 0; i < state.roomMonsters.size() && i < _monsterCombatStates.size(); ++i) {
			MonsterRecord &monster = state.roomMonsters[i];
			const MonsterCombatState &combatState = _monsterCombatStates[i];
			if (!combatState.attackActive || !isKillableMonster(&monster) || !selectMonsterEntity(entityManager, monster))
				continue;
			if (!combatState.attackTargetName.empty() &&
					playerState.entity->getName().equalsIgnoreCase(combatState.attackTargetName)) {
				return &monster;
			}
		}
	}

	if (!playerState.entity)
		return nullptr;

	const Common::Rect playerRect = playerState.entity->getScreenRect();
	const int playerCenterX = playerRect.left + playerRect.width() / 2;
	MonsterRecord *bestMonster = nullptr;
	int bestScore = 0x7fffffff;
	for (MonsterRecord &monster : state.roomMonsters) {
		if (!isKillableMonster(&monster))
			continue;

		Entity *entity = selectMonsterEntity(entityManager, monster);
		if (!entity)
			continue;

		const Common::Rect monsterRect = entity->getScreenRect();
		const int monsterCenterX = monsterRect.left + monsterRect.width() / 2;
		const int score = ABS(monsterCenterX - playerCenterX) +
			ABS(monster.posZ - roundRoomCombatFloat(playerState.z)) * 8;
		if (!bestMonster || score < bestScore) {
			bestMonster = &monster;
			bestScore = score;
		}
	}

	return bestMonster;
}

void Combat::capturePlayerAttackTarget(RoomSetupState &state, RoomPlayerState &playerState,
		EntityManager *entityManager, const Common::Point &mousePos, const NpcRecord *hoverNpc) const {
	if (hoverNpc) {
		playerState.attackTargetName = hoverNpc->npcName;
		playerState.attackTargetClassId = kEntityClassNpc;
		debugC(1, kDebugCombat,
			"Harvester: combat player target capture class='npc' name='%s' cursor=(%d,%d)",
			playerState.attackTargetName.c_str(), mousePos.x, mousePos.y);
		return;
	}

	if (MonsterRecord *monster = findMonsterTargetAtPoint(state, entityManager, mousePos)) {
		playerState.attackTargetName = monster->monsterName;
		playerState.attackTargetClassId = kEntityClassMonster;
		debugC(1, kDebugCombat,
			"Harvester: combat player target capture class='monster' name='%s' cursor=(%d,%d)",
			playerState.attackTargetName.c_str(), mousePos.x, mousePos.y);
		return;
	}

	debugC(1, kDebugCombat,
		"Harvester: combat player target capture class='none' cursor=(%d,%d)",
		mousePos.x, mousePos.y);
}

void Combat::captureOverlappingNpcAttackTarget(RoomSetupState &state, RoomPlayerState &playerState, EntityManager *entityManager) const {
	if (playerState.attackTargetClassId < 0 && !Player::isProjectileCombatLoadout(playerState.combatLoadout)) {
		if (NpcRecord *fallbackNpc = findOverlappingNpcTarget(state, playerState, entityManager)) {
			playerState.attackTargetName = fallbackNpc->npcName;
			playerState.attackTargetClassId = kEntityClassNpc;
		}
	}
}

NpcRecord *Combat::selectNpcAttackContactTarget(RoomSetupState &state, const RoomPlayerState &playerState,
		EntityManager *entityManager, Entity *&npcEntity) const {
	NpcRecord *npc = findRoomNpcRecordByName(state, playerState.attackTargetName);
	npcEntity = npc ? findSceneRuntimeEntity(entityManager, npc->npcName) : nullptr;
	if (!npc || !npcEntity || !playerAttackCanReachTarget(state, playerState, npcEntity))
		npc = Player::isProjectileCombatLoadout(playerState.combatLoadout) ? nullptr : findOverlappingNpcTarget(state, playerState, entityManager);
	npcEntity = npc ? findSceneRuntimeEntity(entityManager, npc->npcName) : nullptr;
	return npc;
}

MonsterRecord *Combat::selectMonsterAttackContactTarget(RoomSetupState &state, RoomPlayerState &playerState,
		EntityManager *entityManager, Entity *&monsterEntity) const {
	MonsterRecord *monster = nullptr;
	if (playerState.attackTargetClassId == kEntityClassMonster)
		monster = findRoomMonsterRecordByName(state, playerState.attackTargetName);
	monsterEntity = monster ? findSceneRuntimeEntity(entityManager, monster->monsterName) : nullptr;
	if (!monster || !monsterEntity || !playerAttackCanReachTarget(state, playerState, monsterEntity, monster->engageDistance)) {
		if (!Player::isProjectileCombatLoadout(playerState.combatLoadout)) {
			monster = findOverlappingMonsterTarget(state, playerState, entityManager);
		} else if (playerState.attackTargetClassId < 0) {
			monster = findProjectileContactMonsterTarget(state, playerState, entityManager);
			if (monster) {
				playerState.attackTargetName = monster->monsterName;
				playerState.attackTargetClassId = kEntityClassMonster;
			}
		} else {
			monster = nullptr;
		}
		monsterEntity = monster ? findSceneRuntimeEntity(entityManager, monster->monsterName) : nullptr;
	}
	return monster;
}

NpcCombatUpdateResult Combat::updateRoomNpcCombat(HarvesterEngine &engine, RoomSetupState &state,
		EntityManager *entityManager, SceneEntities &sceneRuntime,
		Interaction &interactionProcessor, bool &needsRedraw) {
	NpcCombatUpdateResult result;
	for (uint i = 0; i < state.roomNpcs.size(); ++i) {
		NpcRecord &npc = state.roomNpcs[i];
		NpcCombatState &combatState = _npcCombatStates[i];
		Entity *entity = findSceneRuntimeEntity(entityManager, npc.npcName);
		if (!entity || !npc.visible || npc.deathOrMonsterfyFlag) {
			clearRoomNpcCombatState(combatState);
			continue;
		}
		if (!combatState.deathActive && npc.active && npc.deathDamageType != 0) {
			if (beginNpcDeathTransition(engine, npc, *entity, combatState, npc.deathDamageType, needsRedraw))
				continue;

			debugC(1, kDebugCombat,
				"Harvester: room npc death fallback target='%s' damage_type=%d on_death='%s' reason='no death bank'",
				npc.npcName.c_str(), npc.deathDamageType, npc.onDeathActionTag.c_str());
			combatState.deathDamageType = npc.deathDamageType;
			combatState.deathLastFrame = entity->getCurrentFrame();
			result.finalizedDeath = true;
			result.error = finalizeNpcDeathTransition(engine, entityManager, sceneRuntime, npc, combatState, interactionProcessor, needsRedraw);
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
		result.error = finalizeNpcDeathTransition(engine, entityManager, sceneRuntime, npc, combatState, interactionProcessor, needsRedraw);
		return result;
	}
	return result;
}

bool Combat::queueInnerSanctumHerrillDeathAfterMuck(HarvesterEngine &engine, RoomSetupState &state, const MonsterRecord &monster) const {
	if (!monster.roomName.equalsIgnoreCase(kInnerSanctumRoomName) ||
			!monster.monsterName.equalsIgnoreCase(kMuckeyMonsterName) ||
			!monster.onDeathActionTag.equalsIgnoreCase(kActivateInnerSanctumDoorActionTag)) {
		return false;
	}

	Script *script = engine.getScript();
	NpcRecord *herrill = findRoomNpcRecordByName(state, kHerrillLogNpcName);
	if (!script || !herrill || !herrill->visible ||
			herrill->deathOrMonsterfyFlag || herrill->deathDamageType != 0) {
		return false;
	}

	// Native update_actor_runtime_state has a hard-coded HERRILL_LOG removal path
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

bool Combat::beginNpcDeathTransition(HarvesterEngine &engine,
		NpcRecord &npc, Entity &npcEntity, NpcCombatState &combatState, int deathDamageType, bool &needsRedraw) const {
	DeathAnimationRange deathRange;
	if (!resolveNpcDeathAnimationRange(npcEntity, !npc.monsterfyTargetName.empty(),
			deathDamageType, engine.isGoreEnabled(), deathRange))
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
		(void)engine.playSound(npc.audioPath);
	debugC(1, kDebugCombat,
		"Harvester: combat npc death start target='%s' damage_type=%d frames=%d..%d monsterfy='%s' on_death='%s'",
		npc.npcName.c_str(), deathDamageType, deathRange.firstFrame, deathRange.lastFrame,
		npc.monsterfyTargetName.c_str(), npc.onDeathActionTag.c_str());
	needsRedraw = true;
	return true;
}

Common::Error Combat::finalizeNpcDeathTransition(HarvesterEngine &engine,
		EntityManager *entityManager, SceneEntities &sceneRuntime, NpcRecord &npc, NpcCombatState &combatState,
		Interaction &interactionProcessor, bool &needsRedraw) const {
	Script *script = engine.getScript();
	Entity *npcEntity = findSceneRuntimeEntity(entityManager, npc.npcName);
	// Native state 0x35 leaves non-monsterfied NPCs as a dead state-0x38 actor
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
		sceneRuntime.removeSceneEntityByName(npc.npcName);
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
		if (script->executeActionTag(npc.onDeathActionTag, deathInteraction, true, npc.roomName)) {
			interaction = Common::move(deathInteraction);
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
	}

	return interactionProcessor.handleCombatInteraction(interaction, needsRedraw);
}

void Combat::startMonsterHitReaction(HarvesterEngine &engine,
		CombatEffects &combatEffects, MonsterRecord &monster, Entity &monsterEntity, MonsterCombatState *combatState,
		int attackerAttackFirstFrame, bool &needsRedraw) const {
	if (combatEffects.spawnCombatHitEffect(monsterEntity, monster.monsterName))
		needsRedraw = true;

	if (!combatState) {
		(void)playRandomRoomAttackSound(engine,
			monster.hitSound1, monster.hitSound2, monster.hitSound3);
		return;
	}

	HitAnimationRange range;
	if (!resolveRoomMonsterHitAnimationRange(monsterEntity, attackerAttackFirstFrame, range)) {
		(void)playRandomRoomAttackSound(engine,
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
		(void)playRandomRoomAttackSound(engine,
			monster.hitSound1, monster.hitSound2, monster.hitSound3);
		combatState->hitSoundPlayed = true;
	}
	debugC(1, kDebugCombat,
		"Harvester: combat monster hit reaction target='%s' attack_frame=%d frames=%d..%d resume_facing=%d knockback=%d rate=%d",
		monster.monsterName.c_str(), attackerAttackFirstFrame, range.firstFrame, range.lastFrame,
		range.resumeFacing, range.knockbackX, kNativeMonsterHitAnimationRate);
	needsRedraw = true;
}

Common::Error Combat::killDebugActiveMonster(HarvesterEngine &engine, RoomSetupState &state,
		RoomPlayerState &playerState, EntityManager *entityManager, const Common::Point &mousePos,
		CombatEffects &combatEffects, Interaction &interactionProcessor, bool &needsRedraw) {
	if (!engine.isCombatDebugEnabled())
		return Common::kNoError;
	MonsterRecord *monster = resolveKillTarget(state, playerState, entityManager, mousePos);
	Entity *monsterEntity = monster ? findSceneRuntimeEntity(entityManager, monster->monsterName) : nullptr;
	if (!monster || !monsterEntity || !monsterEntity->isVisible()) {
		debugC(1, kDebugCombat,
			"Harvester: debug combat kill skipped reason='no active monster' cursor=(%d,%d)",
			mousePos.x, mousePos.y);
		return Common::kNoError;
	}

	Script *script = engine.getScript();
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
		combatEffects.spawnCombatDamagePopup(*monsterEntity, monster->monsterName, damageLanded);

	if (playerState.attackTargetClassId == kEntityClassMonster &&
			playerState.attackTargetName.equalsIgnoreCase(monster->monsterName)) {
		playerState.attackTargetName.clear();
		playerState.attackTargetClassId = -1;
	}

	debugC(1, kDebugCombat,
		"Harvester: debug combat kill target='%s' damage=%d damage_type=%d cursor=(%d,%d)",
		monster->monsterName.c_str(), damageLanded, debugDamageType, mousePos.x, mousePos.y);

	const int monsterIndex = findRoomMonsterIndexByName(state, monster->monsterName);
	MonsterCombatState *combatState =
		monsterIndex >= 0 && monsterIndex < (int)_monsterCombatStates.size()
			? &_monsterCombatStates[(uint)monsterIndex]
			: nullptr;
	if (combatState)
		clearRoomMonsterCombatState(*combatState);
	(void)script->syncRuntimeMonsterRecord(*monster);

	DeathAnimationRange deathRange;
	if (combatState && resolveMonsterDeathAnimationRange(*monsterEntity, monster->facing,
			debugDamageType, engine.isGoreEnabled(), deathRange)) {
		combatState->deathActive = true;
		combatState->deathFirstFrame = deathRange.firstFrame;
		combatState->deathLastFrame = deathRange.lastFrame;
		combatState->deathDamageType = debugDamageType;
		combatState->nextMovementTick = 0;
		monsterEntity->setAnimationFrameRange(deathRange.firstFrame, deathRange.lastFrame, false);
		monsterEntity->setAnimationRate(kRoomMonsterAnimationRate);
		monsterEntity->setAnimationEnabled(true);
		monsterEntity->setCurrentFrame(deathRange.firstFrame);
		if (!monster->deathSound.empty())
			(void)engine.playSound(monster->deathSound);
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
				monster->onDeathActionTag, deathInteraction, true, monster->roomName)) {
			interaction = Common::move(deathInteraction);
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
	}

	return interactionProcessor.handleCombatInteraction(interaction, needsRedraw);
}

Common::Error Combat::updateRoomMonsterCombat(HarvesterEngine &engine, RoomSetupState &state,
		RoomPlayerState &playerState, RoomIdleAnimationState &idleState, EntityManager *entityManager,
		SceneEntities &sceneRuntime, CombatEffects &combatEffects, const CheatSystem &cheats,
		InteractionCallbacks &callbacks, Interaction &interactionProcessor, bool &needsRedraw) {
	if (!entityManager)
		return Common::kNoError;

	Script &script = *engine.getScript();
	const uint32 now = Player::getRuntimeClockTicks();
	const uint32 moveInterval = MAX<uint32>(1, 100U / (uint32)kRoomMonsterAnimationRate);
	const int horizontalStepBase = 8;
	const int depthStep = MAX<int>(1, roundRoomCombatFloat(
		state.roomZVelocityStep > 0.0f ? state.roomZVelocityStep : 1.0f));
	bool playerAlive = script.getPlayerCurrentHitPoints() > 0;

	resizeCombatStates(state);

	const NpcCombatUpdateResult npcResult = updateRoomNpcCombat(engine, state,
		entityManager, sceneRuntime, interactionProcessor, needsRedraw);
	if (npcResult.finalizedDeath)
		return npcResult.error;

	for (uint i = 0; i < state.roomMonsters.size(); ++i) {
		MonsterRecord &monster = state.roomMonsters[i];
		MonsterCombatState &combatState = _monsterCombatStates[i];
		Entity *entity = findSceneRuntimeEntity(entityManager, monster.monsterName);
		if (!entity || !monster.visible) {
			clearRoomMonsterCombatState(combatState);
			continue;
		}
		if (combatState.deathActive) {
			if (entity->getCurrentFrame() == combatState.deathLastFrame) {
				combatState.deathActive = false;
				monster.active = false;
				monster.visible = true;
				monster.runtimeSpawned = true;
				monster.runtimeState = entity->getCurrentFrame();
				entity->setAnimationRate(0);
				entity->setAnimationFrameRange(monster.runtimeState, monster.runtimeState, false);
				entity->setCurrentFrame(monster.runtimeState);
				entity->setAnimationEnabled(false);
				(void)script.syncRuntimeMonsterRecord(monster);
				debugC(1, kDebugCombat,
					"Harvester: combat monster death complete target='%s' damage_type=%d corpse_frame=%d on_death='%s'",
					monster.monsterName.c_str(), combatState.deathDamageType,
					monster.runtimeState,
					monster.onDeathActionTag.c_str());
				clearRoomMonsterCombatState(combatState);
				const bool herrillDeathQueued = queueInnerSanctumHerrillDeathAfterMuck(engine, state, monster);

				InteractionResult interaction;
				interaction.mutatedRuntimeState = true;
				interaction.visualRuntimeStateChanged = true;
				if (!monster.onDeathActionTag.empty()) {
					InteractionResult deathInteraction;
					if (script.executeActionTag(
							monster.onDeathActionTag, deathInteraction, true, monster.roomName)) {
						interaction = Common::move(deathInteraction);
						interaction.mutatedRuntimeState = true;
						interaction.visualRuntimeStateChanged = true;
					}
				}
				if (herrillDeathQueued) {
					interaction.mutatedRuntimeState = true;
					interaction.visualRuntimeStateChanged = true;
				}

				return interactionProcessor.handleCombatInteraction(interaction, needsRedraw);
			}
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
			const int currentFrame = entity->getCurrentFrame();
			if (!combatState.hitSoundPlayed &&
					monster.hitSoundTriggerFrame >= 0 &&
					currentFrame >= combatState.hitFirstFrame + monster.hitSoundTriggerFrame) {
				(void)playRandomRoomAttackSound(engine,
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
						state, *entity, monster.posX, monster.posY, (float)monster.posZ);
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
				needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, false) || needsRedraw;
			}
			continue;
		}

		if (combatState.attackActive) {
			const int currentFrame = entity->getCurrentFrame();
			if (!combatState.attackSoundPlayed &&
					monster.attackSoundTriggerFrame >= 0 &&
					currentFrame >= combatState.attackFirstFrame + monster.attackSoundTriggerFrame) {
				(void)playRandomRoomAttackSound(engine,
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
					const bool damageDisabled = cheats.isPlayerDamageDisabled();
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
						combatEffects.spawnCombatDamagePopup(*playerState.entity, playerState.entity->getName(), damageLanded);
						if (idleState.entity) {
							idleState.entity->setVisible(false);
							idleState.entity->setAnimationRate(0);
						}
						idleState.active = false;
						idleState.loopStarted = false;
						idleState.exiting = false;
						idleState.restoreFacing = -1;
						playerState.entity->setVisible(true);
						if (playerHitPointsAfter > 0 &&
								Player::startHitAnimation(engine, playerState, combatState.attackFirstFrame)) {
							callbacks.notePlayerActivity();
						}
					}
					if (changed && playerHitPointsAfter <= 0) {
						playerAlive = false;
						callbacks.stopPlayerRegionInteraction();
						callbacks.startPlayerDefeatSequence(
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
		const float zDelta = playerState.z - (float)monster.posZ;
		const float absZDelta = zDelta >= 0.0f ? zDelta : -zDelta;
		const int engageDistance = MAX(0, monster.engageDistance);
		const int liveWaypointTolerance = roundRoomCombatFloat(
			Player::computeDepthScale(state,
				MIN<float>(playerState.z, (float)monster.posZ)) * kNativeMonsterHorizontalWaypointTolerance);
		bool attemptedMoveX = false;
		bool attemptedMoveZ = false;
		if (absZDelta > kNativeMonsterPursuitZTolerance) {
			attemptedMoveZ = true;
			monster.posZ = stepTowardsRoomCombatInt(monster.posZ, playerTargetZ, depthStep);
			monster.posZ = (int)clampRoomDepthForEvent(state, (float)monster.posZ);
			monster.posY = mapRoomDepthToScreenYForCombat(state, (float)monster.posZ, monster.posY);
		} else if (absLiveCenterDx > engageDistance) {
			// Native class-6 pursuit stores a target frame-left X, not a center X.
			const int desiredFrameLeftX = monsterCenterX < playerCenterX
				? playerRect.right + engageDistance - monsterRect.width() / 2
				: playerRect.left - engageDistance + monsterRect.width() / 2;
			if (ABS(monsterRect.left - desiredFrameLeftX) > liveWaypointTolerance) {
				attemptedMoveX = true;
				const int step = MAX<int>(1, roundRoomCombatFloat(
					Player::computeDepthScale(state, (float)monster.posZ) * (float)horizontalStepBase));
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
			(void)applyRoomActorPlacement(state, *entity, monster.posX, monster.posY, (float)monster.posZ);
			if (monster.posZ != previousZ)
				entityManager->reinsertSceneEntity(entity);
			(void)script.syncRuntimeMonsterRecord(monster);
			debugC(3, kDebugCombat,
				"Harvester: combat monster chase move monster='%s' live_center_dx=%d z_delta=%.2f engage=%d waypoint_tol=%d from=(%d,%d,z=%d) to=(%d,%d,z=%d) facing=%d",
				monster.monsterName.c_str(), liveCenterDx, (double)zDelta, engageDistance, liveWaypointTolerance,
				previousX, previousY, previousZ, monster.posX, monster.posY, monster.posZ, monster.facing);
			needsRedraw = true;
			needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, true) || needsRedraw;
			continue;
		}

		if (stalledDirections != 0 && stallChanged) {
			debugC(2, kDebugCombat,
				"Harvester: combat monster chase stalled monster='%s' live_center_dx=%d z_delta=%.2f engage=%d waypoint_tol=%d pos=(%d,%d,z=%d) attempted_x=%d attempted_z=%d",
				monster.monsterName.c_str(), liveCenterDx, (double)zDelta, engageDistance, liveWaypointTolerance,
				monster.posX, monster.posY, monster.posZ, attemptedMoveX, attemptedMoveZ);
		}

		needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, false) || needsRedraw;
		const bool closeEnoughForAttack = isWithinNativeMonsterAttackEntryRange(
			*entity, (float)monster.posZ, *playerState.entity, playerState.z, engageDistance);
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
		if (!resolveMonsterAttackAnimationRange(engine, *entity, monsterCenterX, playerCenterX, range)) {
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
		combatState.attackCooldownSeedTick = now + engine.getRandomNumber(99);
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
			combatState.attackContactFrame, range.resumeFacing, liveCenterDx, (double)zDelta,
			engageDistance, kNativeMonsterAttackAnimationRate, combatState.attackCooldownSeedTick,
			closeRangeBypassDistance);
		needsRedraw = true;
	}

	return Common::kNoError;
}

Common::Error Combat::resolvePlayerAttackContact(HarvesterEngine &engine,
		RoomSetupState &state, RoomPlayerState &playerState,
		EntityManager *entityManager, SceneEntities &sceneRuntime,
		CombatEffects &combatEffects,
		Interaction &interactionProcessor, bool &needsRedraw) {
	if (!playerState.attackActive || playerState.attackContactResolved || !playerState.entity ||
			playerState.attackContactFrame < 0 ||
			playerState.entity->getCurrentFrame() < playerState.attackContactFrame) {
		return Common::kNoError;
	}

	playerState.attackContactResolved = true;
	const int damageType = Player::resolveCombatLoadoutDamageType(playerState.combatLoadout);
	const int damageAmount = Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout);
	debugC(1, kDebugCombat,
		"Harvester: combat player attack contact frame=%d loadout=%d weapon='%s' damage=%d damage_type='%s' target_class='%s' target='%s'",
		playerState.entity->getCurrentFrame(), playerState.combatLoadout,
		Player::describeCombatLoadout(playerState.combatLoadout), damageAmount,
		Player::describeCombatDamageType(damageType),
		describeCombatTargetClass(playerState.attackTargetClassId), playerState.attackTargetName.c_str());
	Script *script = engine.getScript();
	if (!script)
		return Common::kNoError;

	captureOverlappingNpcAttackTarget(state, playerState, entityManager);
	if (playerState.attackTargetClassId == kEntityClassNpc) {
		Entity *npcEntity = nullptr;
		NpcRecord *npc = selectNpcAttackContactTarget(state, playerState, entityManager, npcEntity);
		if (!npc) {
			debugC(1, kDebugCombat,
				"Harvester: combat player attack missed class='npc' target='%s' reason='no reachable npc'",
				playerState.attackTargetName.c_str());
			return Common::kNoError;
		}

		const bool runtimeChanged =
			script->queueRuntimeNpcDeathOrMonsterfy(npc->npcName, damageType);
		const bool jimmyFlagChanged = npc->npcName.equalsIgnoreCase("JIMMY")
			? script->setRuntimeFlagValue("JIMMY_ATTACKED", true)
			: false;
		const int npcIndex = findRoomNpcIndexByName(state, npc->npcName);
		NpcCombatState *combatState =
			npcIndex >= 0 && npcIndex < (int)_npcCombatStates.size()
				? &_npcCombatStates[(uint)npcIndex]
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
			if (combatEffects.spawnCombatHitEffect(*npcEntity, followTargetName))
				needsRedraw = true;
		}
		if (runtimeChanged && combatState && npcEntity &&
				beginNpcDeathTransition(engine, *npc, *npcEntity, *combatState, damageType, needsRedraw))
			return Common::kNoError;
		if (runtimeChanged) {
			debugC(1, kDebugCombat,
				"Harvester: combat npc defeated target='%s' damage_type=%d on_death='%s' fallback='no death bank'",
				npc->npcName.c_str(), damageType, npc->onDeathActionTag.c_str());
			NpcCombatState fallbackState;
			fallbackState.deathDamageType = damageType;
			return finalizeNpcDeathTransition(engine, entityManager, sceneRuntime, *npc, combatState ? *combatState : fallbackState, interactionProcessor, needsRedraw);
		}

		return Common::kNoError;
	}

	Entity *monsterEntity = nullptr;
	MonsterRecord *monster = selectMonsterAttackContactTarget(state, playerState, entityManager, monsterEntity);
	if (!monster || !monsterEntity) {
		debugC(1, kDebugCombat,
			"Harvester: combat player attack missed class='monster' target='%s' reason='no reachable monster'",
			playerState.attackTargetName.c_str());
		return Common::kNoError;
	}

	const int hitPointsBefore = monster->currentHitPoints;
	monster->currentHitPoints = MAX(0,
		monster->currentHitPoints - damageAmount);
	const int damageLanded = hitPointsBefore - monster->currentHitPoints;
	const int monsterIndex = findRoomMonsterIndexByName(state, monster->monsterName);
	MonsterCombatState *combatState =
		monsterIndex >= 0 && monsterIndex < (int)_monsterCombatStates.size()
			? &_monsterCombatStates[(uint)monsterIndex]
			: nullptr;
	debugC(1, kDebugCombat,
		"Harvester: combat player monster hit target='%s' loadout=%d weapon='%s' damage=%d damage_type='%s' hp=%d->%d reachable=1",
		monster->monsterName.c_str(), playerState.combatLoadout,
		Player::describeCombatLoadout(playerState.combatLoadout), damageAmount,
		Player::describeCombatDamageType(damageType), hitPointsBefore, monster->currentHitPoints);
	if (damageLanded > 0) {
		combatEffects.spawnCombatDamagePopup(*monsterEntity, monster->monsterName, damageLanded);
		startMonsterHitReaction(engine, combatEffects, *monster, *monsterEntity, combatState, playerState.attackFirstFrame, needsRedraw);
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
			damageType, engine.isGoreEnabled(), deathRange)) {
		combatState->deathActive = true;
		combatState->deathFirstFrame = deathRange.firstFrame;
		combatState->deathLastFrame = deathRange.lastFrame;
		combatState->deathDamageType = damageType;
		combatState->nextMovementTick = 0;
		monsterEntity->setAnimationFrameRange(deathRange.firstFrame, deathRange.lastFrame, false);
		monsterEntity->setAnimationRate(kRoomMonsterAnimationRate);
		monsterEntity->setAnimationEnabled(true);
		monsterEntity->setCurrentFrame(deathRange.firstFrame);
		if (!monster->deathSound.empty())
			(void)engine.playSound(monster->deathSound);
		debugC(1, kDebugCombat,
			"Harvester: combat monster death start target='%s' damage_type=%d frames=%d..%d gore=%d on_death='%s'",
			monster->monsterName.c_str(), damageType,
			deathRange.firstFrame, deathRange.lastFrame,
			engine.isGoreEnabled(), monster->onDeathActionTag.c_str());
		needsRedraw = true;
		return Common::kNoError;
	}

	monster->active = false;
	monster->visible = false;
	const bool herrillDeathQueued = queueInnerSanctumHerrillDeathAfterMuck(engine, state, *monster);
	debugC(1, kDebugCombat,
		"Harvester: combat monster defeated target='%s' on_death='%s' fallback='no death bank'",
		monster->monsterName.c_str(), monster->onDeathActionTag.c_str());
	InteractionResult interaction;
	interaction.mutatedRuntimeState = true;
	interaction.visualRuntimeStateChanged = true;
	if (!monster->onDeathActionTag.empty()) {
		InteractionResult deathInteraction;
		if (script->executeActionTag(
				monster->onDeathActionTag, deathInteraction, true, monster->roomName)) {
			interaction = Common::move(deathInteraction);
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
	}
	if (herrillDeathQueued) {
		interaction.mutatedRuntimeState = true;
		interaction.visualRuntimeStateChanged = true;
	}

	return interactionProcessor.handleCombatInteraction(interaction, needsRedraw);
}

void Combat::initializeCombatStates(const RoomSetupState &state) {
	_monsterCombatStates.resize(state.roomMonsters.size());
	_npcCombatStates.resize(state.roomNpcs.size());
	debugC(1, kDebugCombat, "Harvester: initialized room combat states monsters=%u npcs=%u",
		_monsterCombatStates.size(), _npcCombatStates.size());
}

void Combat::resizeCombatStates(const RoomSetupState &state) {
	if (_npcCombatStates.size() != state.roomNpcs.size())
		_npcCombatStates.resize(state.roomNpcs.size());
	if (_monsterCombatStates.size() != state.roomMonsters.size())
		_monsterCombatStates.resize(state.roomMonsters.size());
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

void Combat::resetForSceneRefresh(const RoomSetupState &state) {
	// NPC death state survives refresh and is resized by index at the next combat update.
	_monsterCombatStates.clear();
	_monsterCombatStates.resize(state.roomMonsters.size());
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
		(uint)_hitEffectStates.size(), _nextCombatEffectId);
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
		const int rise = (int)(
			clampedElapsed * (uint32)kCombatDebugDamagePopupRisePixels /
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
