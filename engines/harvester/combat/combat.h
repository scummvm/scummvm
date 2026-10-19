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

#ifndef HARVESTER_COMBAT_H
#define HARVESTER_COMBAT_H

#include "common/error.h"
#include "harvester/scene/room_support.h"

namespace Harvester {

class CheatSystem;
class Entity;
class EntityManager;
class HarvesterEngine;
class CombatEffects;
class InteractionCallbacks;
class Interaction;
class SceneEntities;
class Player;

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

struct AttackAnimationRange {
	AttackAnimationRange() {}
	constexpr AttackAnimationRange(int firstFrame, int lastFrame, int resumeFacing)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing) {}

	int firstFrame = 0;
	int lastFrame = 0;
	int resumeFacing = 0;
};

struct DeathAnimationRange {
	DeathAnimationRange() {}
	DeathAnimationRange(int firstFrame, int lastFrame)
		: firstFrame(firstFrame), lastFrame(lastFrame) {}

	int firstFrame = -1;
	int lastFrame = -1;
};

struct HitAnimationRange {
	HitAnimationRange() {}
	HitAnimationRange(int firstFrame, int lastFrame, int resumeFacing, int knockbackX)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing),
		  knockbackX(knockbackX) {}

	int firstFrame = -1;
	int lastFrame = -1;
	int resumeFacing = -1;
	int knockbackX = 0;
};

struct MonsterCombatState {
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
	int loggedPursuitStallDirections = 0;
	bool allowInitialPursuitStep = true;
	uint32 nextMovementTick = 0;
	uint32 attackCooldownSeedTick = 0;
};

struct NpcCombatState {
	bool deathActive = false;
	int deathFirstFrame = -1;
	int deathLastFrame = -1;
	int deathDamageType = 0;
};

struct NpcCombatUpdateResult {
	Common::Error error = Common::kNoError;
	bool finalizedDeath = false;
};

class Combat {
public:
	Combat(HarvesterEngine &engine, SceneState &scene, Player &player,
		EntityManager *entityManager, SceneEntities &sceneRuntime, CombatEffects &combatEffects,
		const CheatSystem &cheats, InteractionCallbacks &callbacks, Interaction &interactionProcessor);
	Combat(const Combat &) = delete;
	Combat &operator=(const Combat &) = delete;

	void initializeCombatStates();
	// Keep prepared state separate until scene-entity mutations succeed.
	Common::Array<MonsterCombatState> prepareMonsterCombatStatesForSceneChange(
		const Common::Array<MonsterRecord> &previousMonsters, const Common::Array<MonsterRecord> &updatedMonsters) const;
	void commitMonsterCombatStatesForSceneChange(Common::Array<MonsterCombatState> &&states);
	void resetForSceneRefresh();

	void capturePlayerAttackTargetForKeyboard(bool attackLeft, bool attackRight, bool attackUp, bool attackDown) const;
	MonsterRecord *resolveKillTarget(const Common::Point &mousePos) const;
	// The caller clears the captured target before resolving the room hover state.
	void capturePlayerAttackTarget(const Common::Point &mousePos, const NpcRecord *hoverNpc) const;
	void captureOverlappingNpcAttackTarget() const;
	NpcRecord *selectNpcAttackContactTarget(Entity *&npcEntity) const;
	MonsterRecord *selectMonsterAttackContactTarget(Entity *&monsterEntity) const;

	Common::Error killDebugActiveMonster(const Common::Point &mousePos, bool &needsRedraw);

	Common::Error updateRoomMonsterCombat(bool &needsRedraw);
	Common::Error resolvePlayerAttackContact(bool &needsRedraw);

private:
	Common::Error resolvePlayerNpcAttackContact(Script &script, int damageType, bool &needsRedraw);
	Common::Error resolvePlayerMonsterAttackContact(Script &script,
		int damageType, int damageAmount, bool &needsRedraw);

	struct MonsterPursuitResult;

	// Returns true when player defeat must end the entire combat update.
	bool updateMonsterAttack(Script &script, const MonsterRecord &monster,
		Entity &entity, MonsterCombatState &combatState, bool &playerAlive, bool &needsRedraw);
	MonsterPursuitResult updateMonsterPursuit(Script &script, MonsterRecord &monster,
		Entity &entity, MonsterCombatState &combatState, int depthStep, bool &needsRedraw) const;
	void beginMonsterAttack(MonsterRecord &monster, Entity &entity, MonsterCombatState &combatState,
		const MonsterPursuitResult &pursuit, uint32 now, bool &needsRedraw) const;

	void resizeCombatStates();
	bool queueInnerSanctumHerrillDeathAfterMuck(const MonsterRecord &monster) const;
	// Called after resizeCombatStates; NPC finalization ends the entire combat update.
	NpcCombatUpdateResult updateRoomNpcCombat(bool &needsRedraw);

	bool beginNpcDeathTransition(NpcRecord &npc, Entity &npcEntity, NpcCombatState &combatState,
		int deathDamageType, bool &needsRedraw) const;
	Common::Error finalizeNpcDeathTransition(NpcRecord &npc, NpcCombatState &combatState, bool &needsRedraw) const;

	void startMonsterHitReaction(MonsterRecord &monster, Entity &monsterEntity, MonsterCombatState *combatState,
		int attackerAttackFirstFrame, bool &needsRedraw) const;

	void updateMonsterHitReaction(Script &script, MonsterRecord &monster,
		Entity &entity, MonsterCombatState &combatState, bool &needsRedraw) const;
	void startMonsterDeathAnimation(const MonsterRecord &monster, Entity &monsterEntity,
		MonsterCombatState &combatState, const DeathAnimationRange &deathRange, int deathDamageType) const;
	Common::Error finalizeMonsterDeathTransition(Script &script, MonsterRecord &monster,
		Entity &entity, MonsterCombatState &combatState, bool &needsRedraw) const;

	static Entity *findSceneRuntimeEntity(EntityManager *entityManager, const Common::String &name);
	static NpcRecord *findRoomNpcRecordByName(RoomSetupState &state, const Common::String &name);
	static Entity *selectMonsterEntity(EntityManager *entityManager, const MonsterRecord &monster);

	MonsterRecord *findMonsterTargetAtPoint(const Common::Point &point) const;
	MonsterRecord *findOverlappingMonsterTarget() const;
	NpcRecord *findOverlappingNpcTarget() const;
	bool playerAttackCanReachTarget(Entity *targetEntity, int engageDistance = 0) const;
	MonsterRecord *findProjectileContactMonsterTarget() const;

	HarvesterEngine &_engine;
	// Refresh assigns new room data into this scene object.
	SceneState &_scene;
	Player &_player;
	EntityManager *const _entityManager;
	SceneEntities &_sceneRuntime;
	CombatEffects &_combatEffects;
	const CheatSystem &_cheats;
	InteractionCallbacks &_callbacks;
	Interaction &_interactionProcessor;
	Common::Array<MonsterCombatState> _monsterCombatStates;
	Common::Array<NpcCombatState> _npcCombatStates;
};

struct HitEffectState {
	Common::String entityName;
	Common::String followTargetName;
	Common::Point anchorPoint;
	float renderZ = 0.0f;
	bool finished = false;
};

struct CombatDamagePopupState {
	Common::String followTargetName;
	Common::Point anchorPoint;
	uint32 startTick = 0;
	int damageAmount = 0;
};

class CombatEffects {
public:
	explicit CombatEffects(EntityManager *entityManager);

	void clearHitEffects();
	bool hasDamagePopups() const { return !_damagePopupStates.empty(); }
	bool spawnCombatHitEffect(Entity &sourceEntity, const Common::String &followTargetName);
	void spawnCombatDamagePopup(Entity &targetEntity, const Common::String &followTargetName, int damageAmount);
	bool syncCombatHitEffects();
	void drawCombatDamagePopups(Graphics::Screen &screen, bool combatDebugEnabled);
	bool pruneCombatHitEffects();
	bool pruneCombatDamagePopups();

private:
	static void resolveHitEffectAnchor(const Entity &targetEntity, Common::Point &anchorPoint, float &renderZ);
	static void resolveCombatDamagePopupAnchor(const Entity &targetEntity, Common::Point &anchorPoint);

	EntityManager *_entityManager;
	Common::Array<HitEffectState> _hitEffectStates;
	Common::Array<CombatDamagePopupState> _damagePopupStates;
	uint _nextCombatEffectId = 0;
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

const char *describeCombatTargetClass(int classId);
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
	AttackAnimationRange &range);
bool resolveMonsterDeathAnimationRange(const Entity &entity, int facing,
	int deathDamageType, bool goreEnabled, DeathAnimationRange &range);
bool resolveNpcDeathAnimationRange(const Entity &entity, bool hasMonsterfyTarget,
	int deathDamageType, bool goreEnabled, DeathAnimationRange &range);
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
	int attackerAttackFirstFrame, HitAnimationRange &range);
float clampRoomDepthForEvent(const RoomSetupState &state, float z);
int roundRoomCombatFloat(float value);
int mapRoomDepthToScreenYForCombat(const RoomSetupState &state, float z, int fallbackY);
int resolveMonsterAttackContactFrameOffset(const MonsterRecord &monster);
void clearRoomMonsterCombatState(MonsterCombatState &combatState);
void clearRoomNpcCombatState(NpcCombatState &combatState);
void clearRoomMonsterAttackState(MonsterCombatState &combatState);
void clearRoomMonsterHitState(MonsterCombatState &combatState);

} // End of namespace Harvester

#endif // HARVESTER_COMBAT_H
