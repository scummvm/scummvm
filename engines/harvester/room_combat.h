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

#ifndef HARVESTER_ROOM_COMBAT_H
#define HARVESTER_ROOM_COMBAT_H

#include "harvester/room_support.h"

namespace Harvester {

class Entity;
class HarvesterEngine;

extern const int kRoomMonsterAnimationRate;
extern const int kNativeMonsterAttackAnimationRate;
extern const int kNativeMonsterHitAnimationRate;
extern const uint32 kNativeMonsterAttackCooldownBaseTicks;
extern const float kNativeNpcMonsterZExtent;
extern const int kRoomNpcAmbientLastFrame;
extern const float kNativeMonsterPursuitZTolerance;
extern const float kNativeMonsterHorizontalWaypointTolerance;
extern const uint32 kNativeKeyboardAttackRepeatTicks;
extern const int kNativeAttackSideWindow;
extern const int kNativeCombatHitKnockbackDecayStep;
extern const char *const kNativeHitEffectResourcePath;
extern const int kNativeHitEffectAnimationRate;
extern const float kNativeHitEffectRenderZBias;
extern const uint32 kCombatDebugDamagePopupDurationMs;
extern const int kCombatDebugDamagePopupRisePixels;
extern const int kCombatDebugDamagePopupVerticalGap;
extern const char *const kInnerSanctumRoomName;
extern const char *const kHerrillLogNpcName;
extern const char *const kMuckeyMonsterName;
extern const char *const kActivateInnerSanctumDoorActionTag;
extern const int kNativeDefaultNpcDeathDamageType;

struct RoomAttackAnimationRange {
	RoomAttackAnimationRange() {}
	RoomAttackAnimationRange(int firstFrame, int lastFrame, int resumeFacing)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing) {}

	int firstFrame = 0;
	int lastFrame = 0;
	int resumeFacing = 0;
};

struct RoomDeathAnimationRange {
	RoomDeathAnimationRange() {}
	RoomDeathAnimationRange(int firstFrame, int lastFrame)
		: firstFrame(firstFrame), lastFrame(lastFrame) {}

	int firstFrame = -1;
	int lastFrame = -1;
};

struct RoomHitAnimationRange {
	RoomHitAnimationRange() {}
	RoomHitAnimationRange(int firstFrame, int lastFrame, int resumeFacing, int knockbackX)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing),
		  knockbackX(knockbackX) {}

	int firstFrame = -1;
	int lastFrame = -1;
	int resumeFacing = -1;
	int knockbackX = 0;
};

struct RoomMonsterCombatState {
	bool attackActive = false;
	int attackFirstFrame = -1;
	int attackLastFrame = -1;
	int attackContactFrame = -1;
	int attackResumeFacing = -1;
	bool attackSoundPlayed = false;
	bool attackContactResolved = false;
	Common::String attackTargetName;
	bool hitActive = false;
	int hitFirstFrame = -1;
	int hitLastFrame = -1;
	int hitResumeFacing = -1;
	int hitKnockbackRemainingX = 0;
	int hitKnockbackDecayStep = 0;
	bool hitSoundPlayed = false;
	bool deathActive = false;
	int deathFirstFrame = -1;
	int deathLastFrame = -1;
	int deathDamageType = 0;
	bool allowInitialPursuitStep = true;
	uint32 nextMovementTick = 0;
	uint32 attackCooldownSeedTick = 0;
};

struct RoomNpcCombatState {
	bool deathActive = false;
	int deathFirstFrame = -1;
	int deathLastFrame = -1;
	int deathDamageType = 0;
};

struct RoomHitEffectState {
	Common::String entityName;
	Common::String followTargetName;
	Common::Point anchorPoint;
	float renderZ = 0.0f;
	bool finished = false;
};

struct RoomCombatDamagePopupState {
	Common::String followTargetName;
	Common::Point anchorPoint;
	uint32 startTick = 0;
	int damageAmount = 0;
};

struct CombatLoadoutHudInfo {
	uint iconIndex = 0;
	int maxVisibleCount = 0;
	const char *unitLabel = nullptr;
};

struct ScopedDeferredLiveNpcDeathTransitions {
	ScopedDeferredLiveNpcDeathTransitions(Script *script) : _script(script) {
		if (_script)
			_script->pushDeferredLiveNpcDeathTransitions();
	}

	~ScopedDeferredLiveNpcDeathTransitions() {
		if (_script)
			_script->popDeferredLiveNpcDeathTransitions();
	}

	Script *_script;
};

bool shouldRetainNpcDeathEntityInCurrentRoom(const NpcRecord &npc);
bool isRetainedCurrentRoomNpcDeathRecord(const NpcRecord &npc);
bool resolveCombatLoadoutHudInfo(int loadout, CombatLoadoutHudInfo &info);
Common::String buildCombatLoadoutStatusMessage(Script &script, const ObjectRecord &object,
	int previousLoadout, int currentLoadout);
bool setRoomMonsterAnimation(Entity &entity, int facing, bool walking);
bool runtimeEntityHasFrameRange(const Entity &entity, int firstFrame, int lastFrame);
bool runtimeEntityHasDrawableFrameRange(const Entity &entity, int firstFrame, int lastFrame);
bool resolveMonsterAttackAnimationRange(HarvesterEngine &engine,
	const Entity &entity, int actorCenterX, int targetCenterX,
	RoomAttackAnimationRange &range);
bool resolveMonsterDeathAnimationRange(const Entity &entity, int facing,
	int deathDamageType, bool goreEnabled, RoomDeathAnimationRange &range);
bool resolveNpcDeathAnimationRange(const Entity &entity, bool hasMonsterfyTarget,
	int deathDamageType, bool goreEnabled, RoomDeathAnimationRange &range);
int stepTowardsRoomCombatInt(int current, int target, int step);
int computeRuntimeEntityHorizontalGap(const Entity &left, const Entity &right);
bool doRuntimeEntityDepthExtentsOverlap(const Entity &first, const Entity &second);
bool areCombatantsWithinRoomCombatReach(const RoomSetupState &state,
	const Entity &attacker, float attackerZ,
	const Entity &target, float targetZ, int engageDistance);
bool isWithinNativeMonsterAttackEntryRange(const Entity &monster, float monsterZ,
	const Entity &player, float playerZ, int engageDistance);
bool playRandomRoomAttackSound(HarvesterEngine &engine, const Common::String &sound1,
	const Common::String &sound2, const Common::String &sound3);
bool resolveRoomMonsterHitAnimationRange(const Entity &entity,
	int attackerAttackFirstFrame, RoomHitAnimationRange &range);
float clampRoomDepthForEvent(const RoomSetupState &state, float z);
int roundRoomCombatFloat(float value);
int mapRoomDepthToScreenYForCombat(const RoomSetupState &state, float z, int fallbackY);
int resolveMonsterAttackContactFrameOffset(const MonsterRecord &monster);
void clearRoomMonsterCombatState(RoomMonsterCombatState &combatState);
void clearRoomNpcCombatState(RoomNpcCombatState &combatState);
void clearRoomMonsterAttackState(RoomMonsterCombatState &combatState);
void clearRoomMonsterHitState(RoomMonsterCombatState &combatState);

} // End of namespace Harvester

#endif // HARVESTER_ROOM_COMBAT_H
