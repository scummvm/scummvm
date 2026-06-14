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

#include "harvester/room_combat.h"

#include "common/util.h"
#include "harvester/harvester.h"
#include "harvester/player.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

const int kRoomMonsterAnimationRate = 17;
const int kNativeMonsterAttackAnimationRate = 4;
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
		RoomAttackAnimationRange &range) {
	// Native class-6 attackers pick one of the 0x16..0x1b attack states, which map to
	// the 0x50..0x8b strike banks. The 0x8c..0x9d banks are hit reactions, not attacks.
	static const RoomAttackAnimationRange kLeftAttackRanges[3] = {
		RoomAttackAnimationRange(0x64, 0x6d, 1),
		RoomAttackAnimationRange(0x5a, 0x63, 1),
		RoomAttackAnimationRange(0x50, 0x59, 1)
	};
	static const RoomAttackAnimationRange kRightAttackRanges[3] = {
		RoomAttackAnimationRange(0x82, 0x8b, 2),
		RoomAttackAnimationRange(0x78, 0x81, 2),
		RoomAttackAnimationRange(0x6e, 0x77, 2)
	};

	const RoomAttackAnimationRange *candidates =
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
		int deathDamageType, bool goreEnabled, RoomDeathAnimationRange &range) {
	const bool preferLeftBank = facing == 1;
	const RoomDeathAnimationRange rightBludge(0xb0, 0xb9);
	const RoomDeathAnimationRange leftBludge(0xba, 0xc3);
	const RoomDeathAnimationRange rightSlash(0x100, 0x109);
	const RoomDeathAnimationRange leftSlash(0x10a, 0x113);
	const RoomDeathAnimationRange rightProjectile(0x114, 0x11d);
	const RoomDeathAnimationRange leftProjectile(0x11e, 0x127);

	const RoomDeathAnimationRange primaryBludge = preferLeftBank ? leftBludge : rightBludge;
	const RoomDeathAnimationRange primarySlash = preferLeftBank ? leftSlash : rightSlash;
	const RoomDeathAnimationRange primaryProjectile = preferLeftBank ? leftProjectile : rightProjectile;
	const RoomDeathAnimationRange fallbackBludge = preferLeftBank ? rightBludge : leftBludge;
	const RoomDeathAnimationRange fallbackSlash = preferLeftBank ? rightSlash : leftSlash;
	const RoomDeathAnimationRange fallbackProjectile = preferLeftBank ? rightProjectile : leftProjectile;

	auto chooseIfAvailable = [&](const RoomDeathAnimationRange &candidate) {
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
		int deathDamageType, bool goreEnabled, RoomDeathAnimationRange &range) {
	const RoomDeathAnimationRange goreBank(0x3c, 0x45);
	const RoomDeathAnimationRange bludgeBank(0x46, 0x4f);
	const bool preferGoreBank = hasMonsterfyTarget || (goreEnabled && deathDamageType != 1);

	auto chooseIfAvailable = [&](const RoomDeathAnimationRange &candidate) {
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
		int attackerAttackFirstFrame, RoomHitAnimationRange &range) {
	RoomHitAnimationRange candidate;
	switch (attackerAttackFirstFrame) {
	case 0x64:
		candidate = RoomHitAnimationRange(0x92, 0x94, 2, -kNativeCombatHitKnockbackDistance);
		break;
	case 0x5a:
		candidate = RoomHitAnimationRange(0x8f, 0x91, 2, -kNativeCombatHitKnockbackDistance);
		break;
	case 0x50:
		candidate = RoomHitAnimationRange(0x8c, 0x8e, 2, -kNativeCombatHitKnockbackDistance);
		break;
	case 0x82:
		candidate = RoomHitAnimationRange(0x9b, 0x9d, 1, kNativeCombatHitKnockbackDistance);
		break;
	case 0x78:
		candidate = RoomHitAnimationRange(0x98, 0x9a, 1, kNativeCombatHitKnockbackDistance);
		break;
	case 0x6e:
		candidate = RoomHitAnimationRange(0x95, 0x97, 1, kNativeCombatHitKnockbackDistance);
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

void clearRoomMonsterCombatState(RoomMonsterCombatState &combatState) {
	combatState = RoomMonsterCombatState();
}

void clearRoomNpcCombatState(RoomNpcCombatState &combatState) {
	combatState = RoomNpcCombatState();
}

void clearRoomMonsterAttackState(RoomMonsterCombatState &combatState) {
	combatState.attackActive = false;
	combatState.attackFirstFrame = -1;
	combatState.attackLastFrame = -1;
	combatState.attackContactFrame = -1;
	combatState.attackResumeFacing = -1;
	combatState.attackSoundPlayed = false;
	combatState.attackContactResolved = false;
	combatState.attackTargetName.clear();
}

void clearRoomMonsterHitState(RoomMonsterCombatState &combatState) {
	combatState.hitActive = false;
	combatState.hitFirstFrame = -1;
	combatState.hitLastFrame = -1;
	combatState.hitResumeFacing = -1;
	combatState.hitKnockbackRemainingX = 0;
	combatState.hitKnockbackDecayStep = 0;
	combatState.hitSoundPlayed = false;
}

} // End of namespace Harvester
