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

#include "harvester/player/player.h"
#include "harvester/player/player_movement.h"
#include "harvester/combat/combat.h"

#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"

namespace Harvester {

using namespace PlayerMovement;

namespace {

static const char *const kPlayerActorResourcePath = "1:/GRAPHIC/MONSTERS/PC/PC0.ABM";
static const float kRoomPlayerAttackUpperYOffset = 144.44f;
static const float kRoomPlayerAttackMidYOffset = 75.36f;
static const int kPlayerAttackSoundTriggerFrameOffset = 1;

struct PlayerAttackAnimationRange {
	PlayerAttackAnimationRange() {}
	PlayerAttackAnimationRange(int firstFrame, int lastFrame, int resumeFacing)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing) {}

	int firstFrame = 0;
	int lastFrame = 0;
	int resumeFacing = 0;
};

struct PlayerAttackSoundSet {
	uint soundCount;
	const char *soundPaths[3];
};

struct PlayerCombatLoadoutTuning {
	const char *label;
	int damageType;
	int damageAmount;
	int contactFrameOffset;
};

static const PlayerAttackSoundSet kPlayerAttackSoundSets[] = {
	{ 3, { "2:/sound/effects/swoosh1.wav", "2:/sound/effects/swoosh2.wav", "2:/sound/effects/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 1, { "2:/SOUND/EFFECTS/NAILGUN1.wav", nullptr, nullptr } },
	{ 1, { "1:/sound/effects/shotgun2.wav", nullptr, nullptr } },
	{ 1, { "2:/SOUND/EFFECTS/gunshot.wav", nullptr, nullptr } },
	{ 1, { "2:/SOUND/EFFECTS/gunshot.wav", nullptr, nullptr } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/sound/effects/swoosh1.wav", "2:/sound/effects/swoosh2.wav", "2:/sound/effects/swoosh3.wav" } },
	{ 1, { "1:/sound/effects/CLN_atk0.wav", nullptr, nullptr } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh3.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh1.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } },
	{ 3, { "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh2.wav", "2:/SOUND/EFFECTS/swoosh3.wav" } }
};

static const PlayerCombatLoadoutTuning kPlayerCombatLoadoutTunings[] = {
	{ "FISTS", 1, 1, 5 },
	{ "CLEAVER", 2, 4, 2 },
	{ "NAILGUN", 4, 2, 2 },
	{ "SHOTGUN", 4, 7, 2 },
	{ "9GUN", 4, 6, 2 },
	{ "38GUN", 4, 6, 2 },
	{ "TOMAHAWK", 2, 3, 2 },
	{ "KNIFE", 2, 3, 2 },
	{ "FLAIL", 2, 3, 2 },
	{ "HANDAXE", 2, 3, 2 },
	{ "WRENCH", 1, 3, 2 },
	{ "PITCHFORK", 2, 3, 2 },
	{ "SCYTHE", 2, 4, 2 },
	{ "SWORD", 2, 5, 2 },
	{ "CHAINSAW", 2, 8, 2 },
	{ "HARVEST_BLADE", 2, 3, 2 },
	{ "SHOVEL", 1, 3, 2 },
	{ "FIREAXE", 2, 3, 2 },
	{ "BAT", 1, 3, 2 },
	{ "RAZOR", 2, 3, 2 },
	{ "POOLSTICK", 1, 2, 2 }
};

static const PlayerAttackSoundSet *resolvePlayerAttackSoundSet(int loadout) {
	if (loadout < 0 || loadout >= (int)ARRAYSIZE(kPlayerAttackSoundSets))
		return nullptr;

	if (kPlayerAttackSoundSets[loadout].soundCount == 0)
		return nullptr;

	return &kPlayerAttackSoundSets[loadout];
}

static const PlayerCombatLoadoutTuning &resolvePlayerCombatLoadoutTuning(int loadout) {
	static const PlayerCombatLoadoutTuning kDefaultTuning = { "FISTS", 1, 1, 5 };

	if (loadout < 0 || loadout >= (int)ARRAYSIZE(kPlayerCombatLoadoutTunings))
		return kDefaultTuning;

	return kPlayerCombatLoadoutTunings[loadout];
}

static bool playPlayerAttackSound(HarvesterEngine &engine, int loadout) {
	const PlayerAttackSoundSet *soundSet = resolvePlayerAttackSoundSet(loadout);
	if (!soundSet)
		return false;

	uint soundIndex = 0;
	if (soundSet->soundCount > 1)
		soundIndex = engine.getRandomNumber(soundSet->soundCount - 1);

	const char *soundPath = soundSet->soundPaths[soundIndex];
	return soundPath && engine.playSound(soundPath);
}

static bool loadoutUsesConsumableCombatResource(int loadout) {
	switch (loadout) {
	case 2:
	case 3:
	case 4:
	case 5:
	case 14:
		return true;
	default:
		return false;
	}
}

static bool resolvePlayerAttackAnimationRange(const RoomSetupState &state,
		const RoomPlayerState &playerState, const Common::Point &mousePos,
		PlayerAttackAnimationRange &range) {
	if (!playerState.entity)
		return false;

	const float depthScale = Player::computeDepthScale(state, playerState.z);
	const float upperThresholdY =
		(float)playerState.bottomY - depthScale * kRoomPlayerAttackUpperYOffset;
	const float midThresholdY =
		(float)playerState.bottomY - depthScale * kRoomPlayerAttackMidYOffset;

	if (mousePos.x < playerState.centerX) {
		if ((float)mousePos.y < upperThresholdY)
			range = PlayerAttackAnimationRange(0x50, 0x59, 1);
		else if ((float)mousePos.y < midThresholdY)
			range = PlayerAttackAnimationRange(0x5a, 0x63, 1);
		else
			range = PlayerAttackAnimationRange(0x64, 0x6d, 1);
	} else {
		if ((float)mousePos.y < upperThresholdY)
			range = PlayerAttackAnimationRange(0x6e, 0x77, 2);
		else if ((float)mousePos.y < midThresholdY)
			range = PlayerAttackAnimationRange(0x78, 0x81, 2);
		else
			range = PlayerAttackAnimationRange(0x82, 0x8b, 2);
	}

	return true;
}

static bool resolveKeyboardAttackAnimationRange(const RoomPlayerState &playerState,
		bool attackLeft, bool attackRight, bool attackUp, bool attackDown,
		PlayerAttackAnimationRange &range) {
	if (attackRight) {
		range = PlayerAttackAnimationRange(0x78, 0x81, 2);
		return true;
	}
	if (attackLeft) {
		range = PlayerAttackAnimationRange(0x5a, 0x63, 1);
		return true;
	}
	if (attackUp) {
		if (playerState.facing == 2) {
			range = PlayerAttackAnimationRange(0x6e, 0x77, 2);
			return true;
		}
		if (playerState.facing == 1) {
			range = PlayerAttackAnimationRange(0x50, 0x59, 1);
			return true;
		}
		return false;
	}
	if (attackDown) {
		if (playerState.facing == 2) {
			range = PlayerAttackAnimationRange(0x82, 0x8b, 2);
			return true;
		}
		if (playerState.facing == 1) {
			range = PlayerAttackAnimationRange(0x64, 0x6d, 1);
			return true;
		}
		return false;
	}

	return false;
}

static bool startResolvedAttackAnimation(RoomPlayerState &playerState,
		const PlayerAttackAnimationRange &range) {
	if (!playerState.entity || playerState.attackActive || playerState.hitActive)
		return false;

	playerState.hasMoveTarget = false;
	playerState.evasiveState = -1;
	playerState.evasiveXStep = 0;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.turnFirstFrame = -1;
	playerState.turnLastFrame = -1;
	playerState.turnEndFrame = -1;
	playerState.turnPlayBackwards = false;
	playerState.attackActive = true;
	playerState.attackFirstFrame = range.firstFrame;
	playerState.attackLastFrame = range.lastFrame;
	playerState.attackContactFrame =
		range.firstFrame + Player::resolveCombatLoadoutContactFrameOffset(playerState.combatLoadout);
	playerState.attackResumeFacing = range.resumeFacing;
	playerState.attackSoundPlayed = resolvePlayerAttackSoundSet(playerState.combatLoadout) == nullptr;
	playerState.attackSoundPlaybackFrame = playerState.attackSoundPlayed
		? -1
		: (range.firstFrame + kPlayerAttackSoundTriggerFrameOffset);
	playerState.attackContactResolved = false;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setAnimationRate(playerState.animationRate);
	playerState.entity->setCurrentFrame(range.firstFrame);
	return true;
}

} // End of anonymous namespace

void Player::cancelMovementForAttack() {
	cancelMovement();
	_state.turnFirstFrame = -1;
	_state.turnLastFrame = -1;
	_state.turnEndFrame = -1;
	_state.turnPlayBackwards = false;
}

bool Player::cancelAttackAnimation() {
	RoomPlayerState &playerState = _state;
	if (!playerState.attackActive)
		return false;

	playerState.attackActive = false;
	playerState.attackFirstFrame = -1;
	playerState.attackLastFrame = -1;
	playerState.attackContactFrame = -1;
	if (!playerState.entity)
		return false;
	playerState.attackSoundPlayed = false;
	playerState.attackSoundPlaybackFrame = -1;
	playerState.attackContactResolved = false;
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	const int resumeFacing = playerState.attackResumeFacing >= 0
		? playerState.attackResumeFacing
		: (playerState.facing >= 0 ? playerState.facing : 0);
	playerState.attackResumeFacing = -1;
	return setIdleAnimation(resumeFacing);
}

void Player::clearAttackTarget() {
	_state.attackTargetName.clear();
	_state.attackTargetClassId = -1;
}

void Player::captureAttackTarget(const Common::String &name, int classId) {
	_state.attackTargetName = name;
	_state.attackTargetClassId = classId;
}

void Player::markAttackContactResolved() {
	_state.attackContactResolved = true;
}

Common::String Player::resolveCombatLoadoutResourcePath(int loadout) {
	if (loadout <= 0)
		return Common::String(kPlayerActorResourcePath);

	return Common::String::format("1:/GRAPHIC/MONSTERS/PC/PC%02d.ABM", loadout);
}

const char *Player::describeCombatLoadout(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).label;
}

int Player::resolveCombatLoadoutDamageAmount(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).damageAmount;
}

int Player::resolveCombatLoadoutDamageType(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).damageType;
}

const char *Player::describeCombatDamageType(int damageType) {
	switch (damageType) {
	case 1:
		return "BLUDGE";
	case 2:
		return "SLASH";
	case 4:
		return "PROJ";
	default:
		return "UNKNOWN";
	}
}

int Player::resolveCombatLoadoutContactFrameOffset(int loadout) {
	return resolvePlayerCombatLoadoutTuning(loadout).contactFrameOffset;
}

bool Player::isProjectileCombatLoadout(int loadout) {
	return resolveCombatLoadoutDamageType(loadout) == 4;
}

void Player::syncCombatLoadout(HarvesterEngine &engine, const RoomSetupState &state) {
	if (_state.entity) {
		(void)syncCombatLoadoutVisual(engine, state, engine.getScript()->getPlayerCombatLoadout());
	} else if (engine.getScript()) {
		_state.combatLoadout = engine.getScript()->getPlayerCombatLoadout();
	}
}

bool Player::syncCombatLoadoutVisual(HarvesterEngine &engine, const RoomSetupState &state, int loadout) {
	RoomPlayerState &playerState = _state;
	if (!playerState.entity)
		return false;

	ResourceManager *resources = engine.getResources();
	if (!resources)
		return false;

	const int clampedLoadout = CLIP<int>(loadout, 0, 0x14);
	if (playerState.combatLoadout == clampedLoadout)
		return false;

	const Common::String resourcePath = resolveCombatLoadoutResourcePath(clampedLoadout);
	if (!playerState.entity->loadAbmResource(*resources, resourcePath))
		return false;

	playerState.combatLoadout = clampedLoadout;
	playerState.attackActive = false;
	playerState.attackFirstFrame = -1;
	playerState.attackLastFrame = -1;
	playerState.attackContactFrame = -1;
	playerState.attackResumeFacing = -1;
	playerState.attackSoundPlayed = false;
	playerState.attackSoundPlaybackFrame = -1;
	playerState.attackContactResolved = false;
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	playerState.hitActive = false;
	playerState.hitFirstFrame = -1;
	playerState.hitLastFrame = -1;
	playerState.hitResumeFacing = -1;
	playerState.hitKnockbackRemainingX = 0;
	playerState.hitKnockbackDecayStep = 0;
	(void)setIdleAnimation(playerState.facing >= 0 ? playerState.facing : 0);
	if (applyRoomActorPlacement(state, *playerState.entity,
			playerState.centerX, playerState.bottomY, playerState.z)) {
		reinsertPlayerSceneEntity(engine, playerState);
	}
	debugC(1, kDebugPlayer,
		"Harvester: player combat loadout visual loadout=%d weapon='%s' damage=%d damage_type='%s' contact=%d resource='%s' facing=%d pos=(%d,%d,z=%.2f)",
		playerState.combatLoadout, describeCombatLoadout(playerState.combatLoadout),
		resolveCombatLoadoutDamageAmount(playerState.combatLoadout),
		describeCombatDamageType(resolveCombatLoadoutDamageType(playerState.combatLoadout)),
		resolveCombatLoadoutContactFrameOffset(playerState.combatLoadout), resourcePath.c_str(), playerState.facing,
		playerState.centerX, playerState.bottomY, (double)playerState.z);
	return true;
}

bool Player::startAttackAnimation(const RoomSetupState &state, const Common::Point &mousePos) {
	RoomPlayerState &playerState = _state;
	PlayerAttackAnimationRange range;
	if (!resolvePlayerAttackAnimationRange(state, playerState, mousePos, range))
		return false;
	if (!startResolvedAttackAnimation(playerState, range))
		return false;
	debugC(1, kDebugPlayer,
		"Harvester: player attack animation frames=%d..%d resume_facing=%d cursor=(%d,%d)",
		range.firstFrame, range.lastFrame, range.resumeFacing, mousePos.x, mousePos.y);
	return true;
}

bool Player::startKeyboardAttackAnimation(const RoomSetupState &state,
		bool attackLeft, bool attackRight, bool attackUp, bool attackDown) {
	RoomPlayerState &playerState = _state;
	(void)state;

	PlayerAttackAnimationRange range;
	if (!resolveKeyboardAttackAnimationRange(playerState, attackLeft, attackRight, attackUp, attackDown, range))
		return false;
	if (!startResolvedAttackAnimation(playerState, range))
		return false;
	debugC(1, kDebugPlayer,
		"Harvester: player keyboard attack animation frames=%d..%d resume_facing=%d input=(L=%d R=%d U=%d D=%d) facing=%d",
		range.firstFrame, range.lastFrame, range.resumeFacing,
		attackLeft, attackRight, attackUp, attackDown, playerState.facing);
	return true;
}

bool Player::updateAttackAnimationState(HarvesterEngine &engine) {
	RoomPlayerState &playerState = _state;
	if (!playerState.attackActive || !playerState.entity)
		return false;
	bool changed = false;
	if (!playerState.attackSoundPlayed &&
			playerState.attackSoundPlaybackFrame >= 0 &&
			playerState.entity->getCurrentFrame() >= playerState.attackSoundPlaybackFrame) {
		if (loadoutUsesConsumableCombatResource(playerState.combatLoadout)) {
			Script *script = engine.getScript();
			if (!script || !script->consumePlayerCombatResourceUnit(playerState.combatLoadout)) {
				debugC(1, kDebugCombat,
					"Harvester: combat player attack dry fire loadout=%d weapon='%s'",
					playerState.combatLoadout,
					describeCombatLoadout(playerState.combatLoadout));
				playerState.attackContactResolved = true;
				playerState.attackContactFrame = -1;
				playerState.attackTargetName.clear();
				playerState.attackTargetClassId = -1;
			} else {
				changed = true;
				(void)playPlayerAttackSound(engine, playerState.combatLoadout);
			}
		} else {
			(void)playPlayerAttackSound(engine, playerState.combatLoadout);
		}
		playerState.attackSoundPlayed = true;
	}
	if (playerState.entity->getCurrentFrame() != playerState.attackLastFrame)
		return changed;

	const int resumeFacing = playerState.attackResumeFacing >= 0
		? playerState.attackResumeFacing
		: (playerState.facing >= 0 ? playerState.facing : 0);
	playerState.attackActive = false;
	playerState.attackFirstFrame = -1;
	playerState.attackLastFrame = -1;
	playerState.attackContactFrame = -1;
	playerState.attackResumeFacing = -1;
	playerState.attackSoundPlayed = false;
	playerState.attackSoundPlaybackFrame = -1;
	playerState.attackContactResolved = false;
	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;
	return setIdleAnimation(resumeFacing) || changed;
}

} // End of namespace Harvester
