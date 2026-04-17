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

#include <math.h>

#include "harvester/player.h"

#include "common/debug.h"
#include "common/system.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

namespace {

static const char *const kPlayerActorResourcePath = "1:/GRAPHIC/MONSTERS/PC/PC0.ABM";
static const float kRoomPlayerHorizontalMoveBase = 8.0f;
static const float kRoomPlayerHorizontalTargetSlackBase = 50.0f;
static const float kRoomPlayerDepthTargetSlack = 8.0f;
static const int kRoomRegionTargetXBias = 10;
static const float kRoomDepthCompareEpsilon = 0.01f;
static const int kRoomPlayerWalkAnimationRate = 17;
static const int kRoomPlayerVerticalScreenStep = 1;
static const int kRoomPlayerAttackAnimationRate = kRoomPlayerWalkAnimationRate;
static const int kRoomPlayerHitAnimationRate = 4;
static const int kRoomPlayerDeathAnimationRate = 4;
static const int kRoomPlayerMinOpaqueLeftX = 4;
static const int kRoomPlayerMaxOpaqueRightX = 0x27c;
static const float kRoomPlayerAttackUpperYOffset = 144.44f;
static const float kRoomPlayerAttackMidYOffset = 75.36f;
static const char *const kPlayerIdleAnimationEntityName = "IDLE_ANIM";
static const char *const kPlayerIdleAnimationResourcePath = "1:/GRAPHIC/ROOMANIM/PCLOUN02.ABM";
static const uint32 kRuntimeClockDivisorMs = 10;
static const uint32 kRoomPlayerIdleDelayTicks = 3000;
static const int kPlayerHitKnockbackDistance = 18;
static const int kPlayerHitKnockbackDecayStep = 3;
// Native run_harvester_main_loop sets EBX=0x0e before spawning PCLOUN02.ABM.
static const int kRoomPlayerIdleAnimationRate = 14;
static const int kRoomPlayerIdleLoopStartFrame = 0x0f;
static const int kRoomPlayerIdleLoopLastFrame = 0xb2;
static const int kRoomPlayerIdleExitLastFrame = 0xbf;
static const int kRoomPlayerIdleYOffset = 4;

struct PlayerAnimationRange {
	PlayerAnimationRange() {}
	PlayerAnimationRange(int walkFirstFrame, int walkLastFrame, int idleFrame)
		: walkFirstFrame(walkFirstFrame), walkLastFrame(walkLastFrame), idleFrame(idleFrame) {}

	int walkFirstFrame = 0;
	int walkLastFrame = 0;
	int idleFrame = 0;
};

struct PlayerTurnAnimationRange {
	PlayerTurnAnimationRange() {}
	PlayerTurnAnimationRange(int firstFrame, int lastFrame, bool playBackwards)
		: firstFrame(firstFrame), lastFrame(lastFrame), playBackwards(playBackwards) {}

	int firstFrame = 0;
	int lastFrame = 0;
	bool playBackwards = false;
};

struct PlayerAttackAnimationRange {
	PlayerAttackAnimationRange() {}
	PlayerAttackAnimationRange(int firstFrame, int lastFrame, int resumeFacing)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing) {}

	int firstFrame = 0;
	int lastFrame = 0;
	int resumeFacing = 0;
};

struct PlayerDeathAnimationRange {
	PlayerDeathAnimationRange() {}
	PlayerDeathAnimationRange(int firstFrame, int lastFrame)
		: firstFrame(firstFrame), lastFrame(lastFrame) {}

	int firstFrame = -1;
	int lastFrame = -1;
};

struct PlayerHitAnimationRange {
	PlayerHitAnimationRange() {}
	PlayerHitAnimationRange(int firstFrame, int lastFrame, int resumeFacing, int knockbackX)
		: firstFrame(firstFrame), lastFrame(lastFrame), resumeFacing(resumeFacing), knockbackX(knockbackX) {}

	int firstFrame = -1;
	int lastFrame = -1;
	int resumeFacing = -1;
	int knockbackX = 0;
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

// The live player avatar seeds these from RenderEntityRuntime.player_combat_loadout_id (+0x11bc)
// into attack_damage_amount (+0x1180) and attack_contact_frame_offset (+0x113c).
// Recovered from spawn_player_combat_avatar and update_actor_runtime_state:
// the native room-combat path commits player attack audio on first_frame + 1.
static const int kPlayerAttackSoundTriggerFrameOffset = 1;
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

static const char *const kPlayerHitSoundPaths[] = {
	"1:/sound/effects/PC0_hit0.wav",
	"1:/sound/effects/PC0_hit1.wav",
	"1:/sound/effects/PC0_hit2.wav"
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

static int roundToInt(float value) {
	return value >= 0.0f ? (int)floorf(value + 0.5f) : (int)ceilf(value - 0.5f);
}

static PlayerAnimationRange resolvePlayerAnimationRange(int facing) {
	switch (facing) {
	case 0:
		return PlayerAnimationRange{ 0x00, 0x09, 0x3b };
	case 1:
		return PlayerAnimationRange{ 0x0f, 0x18, 0x0e };
	case 2:
		return PlayerAnimationRange{ 0x2d, 0x36, 0x2c };
	case 3:
		return PlayerAnimationRange{ 0x1e, 0x27, 0x28 };
	default:
		return PlayerAnimationRange{ 0x00, 0x09, 0x3b };
	}
}

static bool resolvePlayerTurnAnimationRange(int fromFacing, int toFacing, PlayerTurnAnimationRange &range) {
	// Native desired-state transitions play these banks forward or backward depending on the source facing.
	if (fromFacing == 0 && toFacing == 1) {
		range = PlayerTurnAnimationRange{ 0x0a, 0x0e, false };
		return true;
	}
	if (fromFacing == 1 && toFacing == 0) {
		range = PlayerTurnAnimationRange{ 0x0a, 0x0e, true };
		return true;
	}
	if (fromFacing == 1 && toFacing == 3) {
		range = PlayerTurnAnimationRange{ 0x19, 0x1d, false };
		return true;
	}
	if (fromFacing == 3 && toFacing == 1) {
		range = PlayerTurnAnimationRange{ 0x19, 0x1d, true };
		return true;
	}
	if (fromFacing == 3 && toFacing == 2) {
		range = PlayerTurnAnimationRange{ 0x28, 0x2c, false };
		return true;
	}
	if (fromFacing == 2 && toFacing == 3) {
		range = PlayerTurnAnimationRange{ 0x28, 0x2c, true };
		return true;
	}
	if (fromFacing == 2 && toFacing == 0) {
		range = PlayerTurnAnimationRange{ 0x37, 0x3b, false };
		return true;
	}
	if (fromFacing == 0 && toFacing == 2) {
		range = PlayerTurnAnimationRange{ 0x37, 0x3b, true };
		return true;
	}

	return false;
}

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

static bool playPlayerHitSound(HarvesterEngine &engine) {
	const uint soundIndex = ARRAYSIZE(kPlayerHitSoundPaths) > 1
		? engine.getRandomNumber(ARRAYSIZE(kPlayerHitSoundPaths) - 1)
		: 0;
	return engine.playSound(kPlayerHitSoundPaths[soundIndex]);
}

static bool runtimeEntityHasFrameRange(const Entity &entity, int firstFrame, int lastFrame) {
	if (!entity.hasFrames() || firstFrame < 0 || lastFrame < firstFrame)
		return false;

	return (uint)lastFrame < entity.getFrameCount();
}

static bool choosePlayerDeathAnimationRange(const Entity &entity, int firstFrame, int lastFrame,
		PlayerDeathAnimationRange &range) {
	if (!runtimeEntityHasFrameRange(entity, firstFrame, lastFrame))
		return false;

	range = PlayerDeathAnimationRange(firstFrame, lastFrame);
	return true;
}

static bool resolvePlayerDeathAnimationRange(const RoomPlayerState &playerState,
		int damageType, bool goreEnabled, PlayerDeathAnimationRange &range) {
	if (!playerState.entity)
		return false;

	// Native update_actor_runtime_state selects the player death bank from the
	// live combat state family before the terminal 0x38 game-over handoff.
	enum PlayerDeathBankFamily {
		kPlayerDeathBankHorizontalRight,
		kPlayerDeathBankHorizontalLeft,
		kPlayerDeathBankVerticalDown,
		kPlayerDeathBankVerticalUp
	};

	PlayerDeathBankFamily family = kPlayerDeathBankHorizontalLeft;
	if (playerState.attackActive) {
		if (playerState.attackFirstFrame >= 0x6e && playerState.attackFirstFrame <= 0x8b)
			family = kPlayerDeathBankHorizontalRight;
		else
			family = kPlayerDeathBankHorizontalLeft;
	} else if (playerState.turnActive) {
		if (playerState.turnFirstFrame >= 0x28)
			family = kPlayerDeathBankHorizontalRight;
		else
			family = kPlayerDeathBankHorizontalLeft;
	} else {
		switch (playerState.facing) {
		case 0:
			family = kPlayerDeathBankVerticalDown;
			break;
		case 2:
			family = kPlayerDeathBankHorizontalRight;
			break;
		case 3:
			family = kPlayerDeathBankVerticalUp;
			break;
		case 1:
		default:
			family = kPlayerDeathBankHorizontalLeft;
			break;
		}
	}

	const auto selectByFamily = [&](int bludgeFirst, int bludgeLast, int slashFirst, int slashLast,
			int projectileFirst, int projectileLast) {
		if ((!goreEnabled || damageType == 1) &&
				choosePlayerDeathAnimationRange(*playerState.entity, bludgeFirst, bludgeLast, range))
			return true;
		if (damageType == 4 &&
				choosePlayerDeathAnimationRange(*playerState.entity, projectileFirst, projectileLast, range))
			return true;
		if (damageType == 2 &&
				choosePlayerDeathAnimationRange(*playerState.entity, slashFirst, slashLast, range))
			return true;

		return choosePlayerDeathAnimationRange(*playerState.entity, bludgeFirst, bludgeLast, range) ||
			choosePlayerDeathAnimationRange(*playerState.entity, slashFirst, slashLast, range) ||
			choosePlayerDeathAnimationRange(*playerState.entity, projectileFirst, projectileLast, range);
	};

	switch (family) {
	case kPlayerDeathBankHorizontalRight:
		return selectByFamily(0xb0, 0xb9, 0x100, 0x109, 0x114, 0x11d);
	case kPlayerDeathBankVerticalDown:
		return selectByFamily(0xc4, 0xcd, 0xd8, 0xe1, 0xec, 0xf5);
	case kPlayerDeathBankVerticalUp:
		return selectByFamily(0xce, 0xd7, 0xe2, 0xeb, 0xf6, 0xff);
	case kPlayerDeathBankHorizontalLeft:
	default:
		return selectByFamily(0xba, 0xc3, 0x10a, 0x113, 0x11e, 0x127);
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
	// Native update_player_combat_avatar_state uses scan-code 0x1d as the attack modifier:
	// Left/Right pick the side mid attacks directly, while Up/Down only choose upper/lower
	// attacks when the current actor state is already in the matching horizontal family.
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

static bool resolvePlayerHitAnimationRange(const Entity &entity,
		int monsterAttackFirstFrame, PlayerHitAnimationRange &range) {
	PlayerHitAnimationRange candidate;
	switch (monsterAttackFirstFrame) {
	case 0x64:
		candidate = PlayerHitAnimationRange(0x92, 0x94, 2, -kPlayerHitKnockbackDistance);
		break;
	case 0x5a:
		candidate = PlayerHitAnimationRange(0x8f, 0x91, 2, -kPlayerHitKnockbackDistance);
		break;
	case 0x50:
		candidate = PlayerHitAnimationRange(0x8c, 0x8e, 2, -kPlayerHitKnockbackDistance);
		break;
	case 0x82:
		candidate = PlayerHitAnimationRange(0x9b, 0x9d, 1, kPlayerHitKnockbackDistance);
		break;
	case 0x78:
		candidate = PlayerHitAnimationRange(0x98, 0x9a, 1, kPlayerHitKnockbackDistance);
		break;
	case 0x6e:
		candidate = PlayerHitAnimationRange(0x95, 0x97, 1, kPlayerHitKnockbackDistance);
		break;
	default:
		return false;
	}

	if (!runtimeEntityHasFrameRange(entity, candidate.firstFrame, candidate.lastFrame))
		return false;

	range = candidate;
	return true;
}

static bool startResolvedAttackAnimation(RoomPlayerState &playerState,
		const PlayerAttackAnimationRange &range) {
	if (!playerState.entity || playerState.attackActive || playerState.hitActive)
		return false;

	playerState.hasMoveTarget = false;
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
	playerState.entity->setAnimationRate(kRoomPlayerAttackAnimationRate);
	playerState.entity->setCurrentFrame(range.firstFrame);
	return true;
}

static int clampRoomMovementY(const RoomSetupState &state, int screenY) {
	if (!Player::supportsMovementBand(state))
		return screenY;

	return CLIP<int>(screenY, state.roomMaxZScreenY, state.roomMinZScreenY);
}

static float mapRoomScreenYToDepth(const RoomSetupState &state, int screenY) {
	if (!Player::supportsMovementBand(state))
		return (float)state.playerSpawnZ;

	if (screenY <= state.roomMaxZScreenY)
		return (float)state.roomMaxZ;
	if (screenY >= state.roomMinZScreenY)
		return (float)state.roomMinZ;

	return (float)(state.roomMaxZ - ((state.roomMaxZ - state.roomMinZ) *
		(screenY - state.roomMaxZScreenY)) /
		(state.roomMinZScreenY - state.roomMaxZScreenY));
}

static float clampRoomDepth(const RoomSetupState &state, float z) {
	if (!Player::supportsMovementBand(state))
		return z;

	return CLIP<float>(z,
		(float)MIN(state.roomMinZ, state.roomMaxZ),
		(float)MAX(state.roomMinZ, state.roomMaxZ));
}

static int mapRoomDepthToScreenY(const RoomSetupState &state, float z) {
	if (!Player::supportsMovementBand(state))
		return state.playerSpawnY;

	if (state.roomMaxZ == state.roomMinZ)
		return clampRoomMovementY(state, state.playerSpawnY);

	const float clampedZ = clampRoomDepth(state, z);
	const float offset = ((float)state.roomMaxZ - clampedZ) *
		(float)(state.roomMinZScreenY - state.roomMaxZScreenY) /
		(float)(state.roomMaxZ - state.roomMinZ);
	return clampRoomMovementY(state, state.roomMaxZScreenY + roundToInt(offset));
}

static int resolveFacingFromMovementDelta(int dx, int dy) {
	if (ABS(dx) > ABS(dy))
		return dx < 0 ? 1 : 2;
	if (dy < 0)
		return 3;
	return 0;
}

static int resolveFacingFromRoomMovement(
		int fromCenterX, int fromBottomY, int toCenterX, int toBottomY) {
	return resolveFacingFromMovementDelta(
		toCenterX - fromCenterX,
		toBottomY - fromBottomY);
}

static int computeRoomPlayerHorizontalStep(const RoomSetupState &state, float z) {
	return MAX<int>(1, roundToInt(Player::computeDepthScale(state, z) * kRoomPlayerHorizontalMoveBase));
}

static float computeRoomPlayerDepthStep(const RoomSetupState &state) {
	return state.roomZVelocityStep > 0.0f ? state.roomZVelocityStep : 1.0f;
}

static int computeRoomPlayerVerticalScreenStep() {
	// Native spawn_player_combat_avatar seeds player runtime +0x1140 with 1,
	// and update_actor_runtime_state copies abs(+0x1140) into frame_y_step for
	// the vertical walk states instead of remapping screen Y from depth.
	return kRoomPlayerVerticalScreenStep;
}

static int computePlayerFrameLeftX(const RoomPlayerState &playerState) {
	if (!playerState.entity)
		return playerState.centerX;

	int width = 0;
	int height = 0;
	int xOffset = 0;
	int yOffset = 0;
	if (!playerState.entity->getCurrentFrameMetrics(width, height, xOffset, yOffset))
		return playerState.centerX;

	return playerState.centerX - width / 2;
}

static void resetPlayerMoveTargetProgress(RoomPlayerState &playerState) {
	playerState.moveTargetXReached = false;
	playerState.moveTargetZReached = false;
}

static int resolveMoveTargetHorizontalDirection(const RoomSetupState &state,
		const RoomPlayerState &playerState) {
	const float slack = Player::computeDepthScale(state, playerState.z) *
		kRoomPlayerHorizontalTargetSlackBase;
	const float frameLeftX = (float)computePlayerFrameLeftX(playerState);
	if (frameLeftX < (float)playerState.targetX - slack)
		return 1;
	if ((float)playerState.targetX + slack < frameLeftX)
		return -1;

	return 0;
}

static int resolveMoveTargetDepthDirection(const RoomPlayerState &playerState) {
	if (playerState.targetZ < playerState.z - kRoomPlayerDepthTargetSlack)
		return 1;
	if (playerState.z + kRoomPlayerDepthTargetSlack < playerState.targetZ)
		return -1;
	if (playerState.moveTargetRequiresScreenY && playerState.bottomY < playerState.targetBottomY)
		return 1;
	if (playerState.moveTargetRequiresScreenY && playerState.bottomY > playerState.targetBottomY)
		return -1;

	return 0;
}

static bool hasPlayerReachedMoveTarget(const RoomPlayerState &playerState) {
	return playerState.moveTargetXReached && playerState.moveTargetZReached;
}

static void updatePlayerMoveTargetProgress(const RoomSetupState &state,
		RoomPlayerState &playerState) {
	if (!playerState.moveTargetXReached &&
			resolveMoveTargetHorizontalDirection(state, playerState) == 0) {
		playerState.moveTargetXReached = true;
	}
	if (!playerState.moveTargetZReached &&
			resolveMoveTargetDepthDirection(playerState) == 0) {
		playerState.moveTargetZReached = true;
	}
}

static int clampPlayerCenterXToNativeBounds(const RoomPlayerState &playerState, int centerX) {
	if (!playerState.entity)
		return CLIP<int>(centerX, 0, 639);

	int width = 0;
	int height = 0;
	int xOffset = 0;
	int yOffset = 0;
	if (!playerState.entity->getCurrentFrameMetrics(width, height, xOffset, yOffset))
		return CLIP<int>(centerX, 0, 639);

	const int minCenterX = kRoomPlayerMinOpaqueLeftX + width / 2;
	const int maxCenterX = MAX(minCenterX,
		kRoomPlayerMaxOpaqueRightX - (width - width / 2));
	return CLIP<int>(centerX, minCenterX, maxCenterX);
}

static uint32 computeAnimationTickInterval(int rate) {
	return rate > 0 ? MAX<uint32>(1, 100U / (uint32)rate) : 0;
}

static bool consumePlayerMovementTick(RoomPlayerState &playerState) {
	if (playerState.entity &&
			playerState.entity->isAnimationEnabled() &&
			playerState.entity->getAnimationRate() != 0) {
		playerState.nextMovementTick = 0;
		return playerState.entity->didAnimationAdvanceLastTick();
	}

	const uint32 interval = computeAnimationTickInterval(kRoomPlayerWalkAnimationRate);
	if (interval == 0) {
		playerState.nextMovementTick = 0;
		return true;
	}

	const uint32 now = Player::getRuntimeClockTicks();
	if (playerState.nextMovementTick != 0 &&
			(int32)(now - playerState.nextMovementTick) < 0) {
		return false;
	}

	playerState.nextMovementTick = now + interval;
	return true;
}

static float stepTowardsFloat(float current, float target, float step) {
	if (step <= 0.0f || fabsf(target - current) <= kRoomDepthCompareEpsilon)
		return target;
	if (current < target)
		return MIN(current + step, target);
	return MAX(current - step, target);
}

static bool shouldIgnoreMovementBlocker(const Entity &entity, const Common::String *ignoredBlockerName) {
	return ignoredBlockerName && !ignoredBlockerName->empty() &&
		entity.getName().equalsIgnoreCase(*ignoredBlockerName);
}

static bool isSameBlockerName(const Common::String &history, const Entity &blocker) {
	return !history.empty() && history.equalsIgnoreCase(blocker.getName());
}

static void clearPlayerBlockerHistory(RoomPlayerState &playerState) {
	playerState.currentBlockerName.clear();
	playerState.upwardBlockerHistory.clear();
	playerState.downwardBlockerHistory.clear();
	playerState.leftwardBlockerHistory.clear();
	playerState.rightwardBlockerHistory.clear();
}

static void setPlayerCurrentBlocker(RoomPlayerState &playerState, const Entity *blocker) {
	if (blocker) {
		playerState.currentBlockerName = blocker->getName();
		return;
	}

	clearPlayerBlockerHistory(playerState);
}

static const Entity *findPlayerRoomBlocker(HarvesterEngine &engine,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations, const Entity &playerEntity,
		const Common::String *ignoredBlockerName) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	for (const ObjectRecord &object : sceneObjects) {
		const Entity *entity = entityManager->findSceneEntityByName(object.objectName);
		if (entity && !shouldIgnoreMovementBlocker(*entity, ignoredBlockerName) &&
				playerEntity.overlapsEntity(*entity)) {
			return entity;
		}
	}

	for (const AnimRecord &anim : sceneAnimations) {
		const Entity *entity = entityManager->findSceneEntityByName(anim.animName);
		if (entity && !shouldIgnoreMovementBlocker(*entity, ignoredBlockerName) &&
				playerEntity.overlapsEntity(*entity)) {
			return entity;
		}
	}

	return nullptr;
}

static void refreshPlayerCurrentBlocker(HarvesterEngine &engine,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState) {
	if (!playerState.entity) {
		clearPlayerBlockerHistory(playerState);
		return;
	}

	setPlayerCurrentBlocker(playerState, findPlayerRoomBlocker(engine, sceneObjects,
		sceneAnimations, *playerState.entity, nullptr));
}

static void reinsertPlayerSceneEntity(HarvesterEngine &engine, const RoomPlayerState &playerState) {
	if (!playerState.entity)
		return;

	EntityManager *entityManager = engine.getRuntimeEntities();
	if (entityManager &&
			entityManager->findSceneEntityByName(playerState.entity->getName()) == playerState.entity) {
		entityManager->reinsertSceneEntity(playerState.entity);
	}
}

static bool isPlayerMovementBlocked(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState, int candidateCenterX, int candidateBottomY, float candidateZ,
		const Common::String *ignoredBlockerName = nullptr, const Entity **blockerOut = nullptr) {
	if (!playerState.entity)
		return true;

	if (blockerOut)
		*blockerOut = nullptr;

	const int oldCenterX = playerState.centerX;
	const int oldBottomY = playerState.bottomY;
	const float oldZ = playerState.z;
	candidateBottomY = clampRoomMovementY(state, candidateBottomY);
	if (!applyRoomActorPlacement(state, *playerState.entity,
			candidateCenterX, candidateBottomY, candidateZ)) {
		(void)applyRoomActorPlacement(state, *playerState.entity, oldCenterX, oldBottomY, oldZ);
		return true;
	}

	const Entity *blocker = findPlayerRoomBlocker(engine, sceneObjects, sceneAnimations,
		*playerState.entity, ignoredBlockerName);
	(void)applyRoomActorPlacement(state, *playerState.entity, oldCenterX, oldBottomY, oldZ);
	if (blockerOut)
		*blockerOut = blocker;
	return blocker != nullptr;
}

static bool tryApplyPlayerMovement(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState, int candidateCenterX, int candidateBottomY, float candidateZ,
		const Common::String *ignoredBlockerName = nullptr, const Entity **blockerOut = nullptr) {
	if (!playerState.entity)
		return false;

	if (blockerOut)
		*blockerOut = nullptr;

	candidateCenterX = clampPlayerCenterXToNativeBounds(playerState, candidateCenterX);
	candidateBottomY = clampRoomMovementY(state, candidateBottomY);
	candidateZ = clampRoomDepth(state, candidateZ);
	if (candidateCenterX == playerState.centerX &&
			candidateBottomY == playerState.bottomY &&
			fabsf(candidateZ - playerState.z) <= kRoomDepthCompareEpsilon) {
		return false;
	}
	if (isPlayerMovementBlocked(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, candidateBottomY, candidateZ,
			ignoredBlockerName, blockerOut)) {
		return false;
	}

	playerState.centerX = candidateCenterX;
	playerState.bottomY = candidateBottomY;
	playerState.z = candidateZ;
	if (!applyRoomActorPlacement(state, *playerState.entity,
			playerState.centerX, playerState.bottomY, playerState.z)) {
		return false;
	}

	reinsertPlayerSceneEntity(engine, playerState);
	return true;
}

static bool tryApplyPlayerMovementWithFallbacks(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState,
		int previousCenterX, int previousBottomY, float previousZ,
		int candidateCenterX, int candidateBottomY, float candidateZ) {
	if (tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, candidateBottomY, candidateZ)) {
		return true;
	}
	if (tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, previousBottomY, candidateZ)) {
		return true;
	}
	if (tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, candidateBottomY, previousZ)) {
		return true;
	}
	if (tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, previousBottomY, previousZ)) {
		return true;
	}
	if (tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, previousCenterX, candidateBottomY, candidateZ)) {
		return true;
	}
	if (tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, previousCenterX, candidateBottomY, previousZ)) {
		return true;
	}
	return tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
		playerState, previousCenterX, previousBottomY, candidateZ);
}

static bool isPathfindingDebugEnabled() {
	return g_engine && g_engine->isPathfindingDebugEnabled();
}

static bool isHorizontalDirectionBlockedByHistory(const RoomPlayerState &playerState,
		int horizontalDirection) {
	if (horizontalDirection < 0)
		return !playerState.leftwardBlockerHistory.empty();
	if (horizontalDirection > 0)
		return !playerState.rightwardBlockerHistory.empty();
	return false;
}

static bool isDepthDirectionBlockedByHistory(const RoomPlayerState &playerState,
		int depthDirection) {
	if (depthDirection > 0)
		return !playerState.upwardBlockerHistory.empty();
	if (depthDirection < 0)
		return !playerState.downwardBlockerHistory.empty();
	return false;
}

static const char *describeBlockedDirection(int horizontalDirection, int depthDirection) {
	if (horizontalDirection < 0)
		return "left";
	if (horizontalDirection > 0)
		return "right";
	if (depthDirection > 0)
		return "upward";
	if (depthDirection < 0)
		return "downward";
	return "";
}

static int resolveFacingFromMoveDirection(int horizontalDirection, int depthDirection,
		int fallbackFacing) {
	if (horizontalDirection < 0)
		return 1;
	if (horizontalDirection > 0)
		return 2;
	if (depthDirection < 0)
		return 3;
	if (depthDirection > 0)
		return 0;
	return fallbackFacing >= 0 ? fallbackFacing : 0;
}

static void logPlayerBlockerHistory(const RoomSetupState &state,
		const RoomPlayerState &playerState, const Entity &blocker,
		const char *direction) {
	if (!isPathfindingDebugEnabled())
		return;

	debugC(1, kDebugPathfinding,
		"Harvester: pathfinding blocker history room='%s' blocker='%s' direction='%s' histories=(up='%s',down='%s',left='%s',right='%s')",
		state.roomName.c_str(), blocker.getName().c_str(), direction,
		playerState.upwardBlockerHistory.c_str(),
		playerState.downwardBlockerHistory.c_str(),
		playerState.leftwardBlockerHistory.c_str(),
		playerState.rightwardBlockerHistory.c_str());
}

static bool recordNativePrimaryWalkBlockerHistory(const RoomSetupState &state,
		RoomPlayerState &playerState, const Entity &blocker,
		int horizontalDirection, int depthDirection) {
	if (horizontalDirection < 0) {
		const int frameLeftX = computePlayerFrameLeftX(playerState);
		const Common::Rect blockerRect = blocker.getScreenRect();
		if (isSameBlockerName(playerState.rightwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.upwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.downwardBlockerHistory, blocker) ||
				frameLeftX <= blockerRect.left) {
			return false;
		}

		playerState.leftwardBlockerHistory = blocker.getName();
		playerState.moveTargetXReached = true;
		logPlayerBlockerHistory(state, playerState, blocker, "left");
		return true;
	}

	if (horizontalDirection > 0) {
		const int frameLeftX = computePlayerFrameLeftX(playerState);
		const Common::Rect blockerRect = blocker.getScreenRect();
		if (isSameBlockerName(playerState.leftwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.upwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.downwardBlockerHistory, blocker) ||
				blockerRect.left <= frameLeftX) {
			return false;
		}

		playerState.rightwardBlockerHistory = blocker.getName();
		playerState.moveTargetXReached = true;
		logPlayerBlockerHistory(state, playerState, blocker, "right");
		return true;
	}

	if (depthDirection > 0) {
		if (isSameBlockerName(playerState.rightwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.leftwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.downwardBlockerHistory, blocker)) {
			return false;
		}

		playerState.upwardBlockerHistory = blocker.getName();
		playerState.moveTargetZReached = true;
		logPlayerBlockerHistory(state, playerState, blocker, "upward");
		return true;
	}

	if (depthDirection < 0) {
		if (isSameBlockerName(playerState.rightwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.upwardBlockerHistory, blocker) ||
				isSameBlockerName(playerState.leftwardBlockerHistory, blocker)) {
			return false;
		}

		playerState.downwardBlockerHistory = blocker.getName();
		playerState.moveTargetZReached = true;
		logPlayerBlockerHistory(state, playerState, blocker, "downward");
		return true;
	}

	return false;
}

static void setMoveTargetInternal(const RoomSetupState &state, RoomPlayerState &playerState,
		int targetX, float targetZ, int targetBottomY, bool requireScreenY) {
	playerState.hasMoveTarget = true;
	playerState.nextMovementTick = 0;
	playerState.targetX = CLIP<int>(targetX, 0, 639);
	playerState.targetBottomY = clampRoomMovementY(state, targetBottomY);
	playerState.targetZ = clampRoomDepth(state, targetZ);
	playerState.moveTargetRequiresScreenY = requireScreenY;
	resetPlayerMoveTargetProgress(playerState);
	debugC(1, kDebugPlayer,
		"Harvester: player move target room='%s' current=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f)",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ);
	if (isPathfindingDebugEnabled()) {
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding target set room='%s' movement_band=(y=%d..%d,z=%d..%d) current=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f)",
			state.roomName.c_str(), state.roomMaxZScreenY, state.roomMinZScreenY,
			state.roomMinZ, state.roomMaxZ,
			playerState.centerX, playerState.bottomY, (double)playerState.z,
			playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ);
	}
}

static void notePlayerIdleReset(RoomIdleAnimationState &idleState) {
	idleState.resetTick = Player::getRuntimeClockTicks();
	Player::updateIdleTrigger(idleState);
}

static bool setPlayerWalkAnimation(RoomPlayerState &playerState, int facing) {
	if (!playerState.entity)
		return false;

	const PlayerAnimationRange range = resolvePlayerAnimationRange(facing);
	const int currentFrame = playerState.entity->getCurrentFrame();
	const bool alreadyWalking = playerState.facing == facing &&
		playerState.entity->getAnimationRate() == kRoomPlayerWalkAnimationRate &&
		currentFrame >= range.walkFirstFrame && currentFrame <= range.walkLastFrame;
	if (alreadyWalking)
		return false;

	playerState.facing = facing;
	playerState.entity->setAnimationFrameRange(range.walkFirstFrame, range.walkLastFrame, true);
	playerState.entity->setAnimationRate(kRoomPlayerWalkAnimationRate);
	debugC(1, kDebugPlayer,
		"Harvester: player walk animation facing=%d frames=%d..%d rate=%d",
		facing, range.walkFirstFrame, range.walkLastFrame, kRoomPlayerWalkAnimationRate);
	return true;
}

static void positionPlayerIdleAnimationEntity(const RoomSetupState &state,
		const RoomPlayerState &playerState, Entity &idleEntity) {
	if (!playerState.entity)
		return;

	idleEntity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
	idleEntity.setDepthScale(Player::computeDepthScale(state, playerState.z));
	idleEntity.setPosition(playerState.entity->getX(),
		playerState.entity->getY() + kRoomPlayerIdleYOffset, playerState.z);
}

static Entity *ensurePlayerIdleAnimationEntity(HarvesterEngine &engine,
		const RoomSetupState &state, const RoomPlayerState &playerState,
		RoomIdleAnimationState &idleState) {
	if (idleState.entity || !playerState.entity)
		return idleState.entity;

	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	idleState.entity = entityManager->spawnSceneAnimationEntity(
		kPlayerIdleAnimationEntityName, kPlayerIdleAnimationResourcePath,
		Common::Point(playerState.entity->getX(),
			playerState.entity->getY() + kRoomPlayerIdleYOffset),
		playerState.z, 0, false, false, false, false, false);
	if (!idleState.entity)
		return nullptr;

	positionPlayerIdleAnimationEntity(state, playerState, *idleState.entity);
	idleState.entity->setVisible(false);
	debugC(1, kDebugPlayer,
		"Harvester: spawned player idle animation '%s' room='%s' frames=0..%d",
		kPlayerIdleAnimationResourcePath, state.roomName.c_str(), idleState.entity->getLastFrame());
	return idleState.entity;
}

static bool finishPlayerIdleAnimation(const RoomSetupState &state, RoomPlayerState &playerState,
		RoomIdleAnimationState &idleState) {
	if (!idleState.active)
		return false;

	if (idleState.entity) {
		idleState.entity->setVisible(false);
		idleState.entity->setAnimationRate(0);
		idleState.entity->setCurrentFrame(0);
		positionPlayerIdleAnimationEntity(state, playerState, *idleState.entity);
	}
	if (playerState.entity) {
		playerState.entity->setVisible(true);
		if (idleState.restoreFacing >= 0)
			(void)Player::setIdleAnimation(playerState, idleState.restoreFacing);
	}

	idleState.active = false;
	idleState.loopStarted = false;
	idleState.exiting = false;
	idleState.restoreFacing = -1;
	notePlayerIdleReset(idleState);
	debugC(1, kDebugPlayer,
		"Harvester: player idle animation finished room='%s' facing=%d",
		state.roomName.c_str(), playerState.facing);
	return true;
}

} // End of anonymous namespace

float Player::computeDepthScale(const RoomSetupState &state, float z) {
	float scale = 1.0f;
	if (state.roomPerspectiveScale != 0.0f) {
		scale -= (z - (float)state.roomFullScaleZ) * state.roomPerspectiveScale;
		if (scale <= 0.0f)
			scale = 1.0f;
	}

	return scale;
}

int Player::resolveFacingFrame(int facing) {
	switch (facing) {
	case 0:
		return 0x3b;
	case 1:
		return 0x0e;
	case 2:
		return 0x2c;
	case 3:
		return 0x28;
	default:
		return 0x0e;
	}
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

bool Player::supportsMovementBand(const RoomSetupState &state) {
	return state.roomMaxZScreenY >= 0 &&
		state.roomMinZScreenY >= state.roomMaxZScreenY;
}

uint32 Player::getRuntimeClockTicks() {
	return g_system ? (g_system->getMillis() / kRuntimeClockDivisorMs) : 0;
}

bool Player::isIdleAnimationExcludedRoom(const Common::String &roomName) {
	static const char *const kExcludedRooms[] = {
		"JAWS",
		"SUPPLY1",
		"SUPPLY2",
		"SUPPLY3",
		"LAVAPIT",
		"RMNBATH",
		"SERGENTRM",
		"CHAP2"
	};

	for (const char *excludedRoom : kExcludedRooms) {
		if (roomName.equalsIgnoreCase(excludedRoom))
			return true;
	}

	return false;
}

void Player::updateIdleTrigger(RoomIdleAnimationState &idleState) {
	idleState.triggerTick = MAX(idleState.activityTick, idleState.resetTick) + kRoomPlayerIdleDelayTicks;
}

void Player::setMoveTarget(const RoomSetupState &state, RoomPlayerState &playerState,
		int targetX, float targetZ) {
	setMoveTargetInternal(state, playerState, targetX, targetZ,
		mapRoomDepthToScreenY(state, targetZ), false);
}

void Player::setRegionMoveTarget(const RoomSetupState &state, RoomPlayerState &playerState,
		int targetX, float targetZ) {
	setMoveTargetInternal(state, playerState, targetX, targetZ,
		mapRoomDepthToScreenY(state, targetZ), true);
}

void Player::setMoveTargetFromScreenPoint(const RoomSetupState &state,
		RoomPlayerState &playerState, int targetX, int targetBottomY) {
	const int clampedTargetBottomY = clampRoomMovementY(state, targetBottomY);
	setMoveTargetInternal(state, playerState, targetX,
		mapRoomScreenYToDepth(state, clampedTargetBottomY), clampedTargetBottomY, false);
}

bool Player::resolveBlockedStartupSpawn(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState) {
	if (!playerState.entity)
		return false;

	if (!isPlayerMovementBlocked(engine, state, sceneObjects, sceneAnimations,
			playerState, playerState.centerX, playerState.bottomY, playerState.z)) {
		return false;
	}

	const int originalCenterX = playerState.centerX;
	const int originalBottomY = playerState.bottomY;
	const int preferredDirection = playerState.facing == 1 ? -1 : 1;
	for (int distance = 1; distance < 640; ++distance) {
		int previousCandidateX = originalCenterX;
		for (int pass = 0; pass < 2; ++pass) {
			const int direction = pass == 0 ? preferredDirection : -preferredDirection;
			const int candidateX = clampPlayerCenterXToNativeBounds(
				playerState, originalCenterX + direction * distance);
			if (candidateX == originalCenterX || candidateX == previousCandidateX)
				continue;
			previousCandidateX = candidateX;
			if (isPlayerMovementBlocked(engine, state, sceneObjects, sceneAnimations,
					playerState, candidateX, playerState.bottomY, playerState.z)) {
				continue;
			}

			playerState.centerX = candidateX;
			if (!applyRoomActorPlacement(state, *playerState.entity,
					playerState.centerX, playerState.bottomY, playerState.z)) {
				return false;
			}
			reinsertPlayerSceneEntity(engine, playerState);

			debugC(1, kDebugPlayer,
				"Harvester: adjusted blocked startup spawn room='%s' facing=%d from=(%d,%d,z=%.2f) to=(%d,%d,z=%.2f)",
				state.roomName.c_str(), playerState.facing,
				originalCenterX, originalBottomY, (double)playerState.z,
				playerState.centerX, playerState.bottomY, (double)playerState.z);
			return true;
		}
	}

	debugC(1, kDebugPlayer,
		"Harvester: startup spawn remains blocked room='%s' pos=(%d,%d,z=%.2f)",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY,
		(double)playerState.z);
	return false;
}

int Player::resolveRegionTargetX(const RoomSetupState &state, const RegionRecord &region,
		const RoomPlayerState &playerState) {
	const Common::Rect bounds(region.left, region.top, region.right + 1, region.bottom + 1);
	int targetX = bounds.left + bounds.width() / 2;
	if (!playerState.entity)
		return CLIP<int>(targetX, 0, 639);

	const int frameLeftX = computePlayerFrameLeftX(playerState);
	if (targetX + kRoomRegionTargetXBias < frameLeftX)
		targetX -= kRoomRegionTargetXBias;

	const int targetSlack = MAX<int>(1,
		roundToInt(Player::computeDepthScale(state, playerState.z) *
			kRoomPlayerHorizontalTargetSlackBase));
	if (region.desiredFacing == 1)
		targetX = MIN<int>(targetX, region.right - targetSlack);
	else if (region.desiredFacing == 2)
		targetX = MAX<int>(targetX, region.left + targetSlack);
	return CLIP<int>(targetX, 0, 639);
}

float Player::resolveRegionTargetZ(const RegionRecord &region) {
	if (region.desiredFacing == 0)
		return (float)region.maxZ;
	if (region.desiredFacing == 3)
		return (float)region.minZ;

	return (float)(region.minZ + (region.maxZ - region.minZ) / 2);
}

bool Player::syncCombatLoadoutVisual(HarvesterEngine &engine, const RoomSetupState &state,
		RoomPlayerState &playerState, int loadout) {
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
	(void)setIdleAnimation(playerState, playerState.facing >= 0 ? playerState.facing : 0);
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

bool Player::setIdleAnimation(RoomPlayerState &playerState, int facing) {
	if (!playerState.entity)
		return false;

	const PlayerAnimationRange range = resolvePlayerAnimationRange(facing);
	const bool changed = playerState.facing != facing ||
		playerState.entity->getCurrentFrame() != range.idleFrame ||
		playerState.entity->getAnimationRate() != 0;
	playerState.facing = facing;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationRate(0);
	playerState.entity->setAnimationFrameRange(range.idleFrame, range.idleFrame, false);
	playerState.entity->setCurrentFrame(range.idleFrame);
	if (changed) {
		debugC(1, kDebugPlayer,
			"Harvester: player idle animation facing=%d frame=%d",
			facing, range.idleFrame);
	}
	return changed;
}

bool Player::startAttackAnimation(const RoomSetupState &state,
		RoomPlayerState &playerState, const Common::Point &mousePos) {
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
		RoomPlayerState &playerState, bool attackLeft, bool attackRight, bool attackUp, bool attackDown) {
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

bool Player::updateAttackAnimationState(HarvesterEngine &engine,
		RoomPlayerState &playerState) {
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
	return setIdleAnimation(playerState, resumeFacing) || changed;
}

bool Player::startHitAnimation(HarvesterEngine &engine, RoomPlayerState &playerState,
		int monsterAttackFirstFrame) {
	if (!playerState.entity || playerState.deathActive)
		return false;

	PlayerHitAnimationRange range;
	if (!resolvePlayerHitAnimationRange(*playerState.entity, monsterAttackFirstFrame, range))
		return false;

	playerState.hasMoveTarget = false;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.turnFirstFrame = -1;
	playerState.turnLastFrame = -1;
	playerState.turnEndFrame = -1;
	playerState.turnPlayBackwards = false;
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
	playerState.nextMovementTick = 0;
	playerState.hitActive = true;
	playerState.hitFirstFrame = range.firstFrame;
	playerState.hitLastFrame = range.lastFrame;
	playerState.hitResumeFacing = range.resumeFacing;
	playerState.hitKnockbackRemainingX = range.knockbackX;
	playerState.hitKnockbackDecayStep = kPlayerHitKnockbackDecayStep;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setAnimationRate(kRoomPlayerHitAnimationRate);
	playerState.entity->setCurrentFrame(range.firstFrame);
	playerState.entity->setAnimationEnabled(true);
	playerState.entity->setVisible(true);
	(void)playPlayerHitSound(engine);
	debugC(1, kDebugCombat,
		"Harvester: player hit animation attack_frame=%d frames=%d..%d resume_facing=%d knockback=%d rate=%d",
		monsterAttackFirstFrame, range.firstFrame, range.lastFrame, range.resumeFacing,
		range.knockbackX, kRoomPlayerHitAnimationRate);
	return true;
}

bool Player::updateHitAnimationState(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState) {
	if (!playerState.hitActive || !playerState.entity)
		return false;

	bool changed = false;
	if (playerState.hitKnockbackRemainingX != 0) {
		const int step = MAX(1, ABS(playerState.hitKnockbackDecayStep));
		const int knockbackStep = playerState.hitKnockbackRemainingX > 0
			? MIN(playerState.hitKnockbackRemainingX, step)
			: MAX(playerState.hitKnockbackRemainingX, -step);
		if (knockbackStep != 0) {
			if (!tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
					playerState, playerState.centerX + knockbackStep, playerState.bottomY, playerState.z)) {
				playerState.hitKnockbackRemainingX = 0;
			} else {
				changed = true;
				playerState.hitKnockbackRemainingX -= knockbackStep;
			}
		}
	}

	if (playerState.entity->getCurrentFrame() != playerState.hitLastFrame)
		return changed;

	const int resumeFacing = playerState.hitResumeFacing >= 0
		? playerState.hitResumeFacing
		: (playerState.facing >= 0 ? playerState.facing : 0);
	playerState.hitActive = false;
	playerState.hitFirstFrame = -1;
	playerState.hitLastFrame = -1;
	playerState.hitResumeFacing = -1;
	playerState.hitKnockbackRemainingX = 0;
	playerState.hitKnockbackDecayStep = 0;
	return setIdleAnimation(playerState, resumeFacing) || changed;
}

bool Player::startDeathAnimation(RoomPlayerState &playerState, int damageType, bool goreEnabled) {
	if (!playerState.entity || playerState.deathActive)
		return false;

	PlayerDeathAnimationRange range;
	if (!resolvePlayerDeathAnimationRange(playerState, damageType, goreEnabled, range))
		return false;

	playerState.hasMoveTarget = false;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.turnFirstFrame = -1;
	playerState.turnLastFrame = -1;
	playerState.turnEndFrame = -1;
	playerState.turnPlayBackwards = false;
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
	playerState.nextMovementTick = 0;
	playerState.deathActive = true;
	playerState.deathFirstFrame = range.firstFrame;
	playerState.deathLastFrame = range.lastFrame;
	playerState.deathDamageType = damageType;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setAnimationRate(kRoomPlayerDeathAnimationRate);
	playerState.entity->setCurrentFrame(range.firstFrame);
	playerState.entity->setVisible(true);
	debugC(1, kDebugCombat,
		"Harvester: player death animation start damage_type=%d gore=%d facing=%d frames=%d..%d",
		damageType, goreEnabled, playerState.facing, range.firstFrame, range.lastFrame);
	return true;
}

bool Player::updateDeathAnimationState(RoomPlayerState &playerState) {
	if (!playerState.deathActive || !playerState.entity)
		return false;
	if (playerState.entity->getCurrentFrame() < playerState.deathLastFrame)
		return false;

	playerState.deathActive = false;
	debugC(1, kDebugCombat,
		"Harvester: player death animation complete damage_type=%d frame=%d",
		playerState.deathDamageType, playerState.entity->getCurrentFrame());
	return true;
}

bool Player::startTurnAnimation(RoomPlayerState &playerState, int targetFacing) {
	if (!playerState.entity || playerState.hitActive ||
			playerState.facing < 0 || playerState.facing == targetFacing)
		return false;

	PlayerTurnAnimationRange range;
	if (!resolvePlayerTurnAnimationRange(playerState.facing, targetFacing, range))
		return false;

	playerState.turnActive = true;
	playerState.turnTargetFacing = targetFacing;
	playerState.turnFirstFrame = range.firstFrame;
	playerState.turnLastFrame = range.lastFrame;
	playerState.turnEndFrame = range.playBackwards ? range.firstFrame : range.lastFrame;
	playerState.turnPlayBackwards = range.playBackwards;
	playerState.nextMovementTick = 0;
	playerState.entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
	playerState.entity->setPlayBackwards(range.playBackwards);
	playerState.entity->setAnimationRate(kRoomPlayerWalkAnimationRate);
	playerState.entity->setCurrentFrame(range.playBackwards ? range.lastFrame : range.firstFrame);
	debugC(1, kDebugPlayer,
		"Harvester: player turn animation from=%d to=%d frames=%d..%d backwards=%d rate=%d",
		playerState.facing, targetFacing, range.firstFrame, range.lastFrame,
		range.playBackwards, kRoomPlayerWalkAnimationRate);
	return true;
}

bool Player::updateTurnAnimationState(RoomPlayerState &playerState) {
	if (!playerState.turnActive || !playerState.entity)
		return false;
	if (playerState.entity->getCurrentFrame() != playerState.turnEndFrame)
		return false;

	playerState.turnActive = false;
	playerState.facing = playerState.turnTargetFacing;
	playerState.turnTargetFacing = -1;
	playerState.turnFirstFrame = -1;
	playerState.turnLastFrame = -1;
	playerState.turnEndFrame = -1;
	playerState.turnPlayBackwards = false;
	playerState.entity->setAnimationRate(0);
	debugC(1, kDebugPlayer,
		"Harvester: player turn complete facing=%d frame=%d",
		playerState.facing, playerState.entity->getCurrentFrame());
	return true;
}

bool Player::stepMoveTarget(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState) {
	if (!playerState.entity || !playerState.hasMoveTarget || playerState.turnActive || playerState.hitActive)
		return false;

	refreshPlayerCurrentBlocker(engine, sceneObjects, sceneAnimations, playerState);

	updatePlayerMoveTargetProgress(state, playerState);
	if (hasPlayerReachedMoveTarget(playerState)) {
		playerState.hasMoveTarget = false;
		return setIdleAnimation(playerState, playerState.facing >= 0 ? playerState.facing : 0);
	}

	const int previousCenterX = playerState.centerX;
	const int previousBottomY = playerState.bottomY;
	int horizontalDirection = playerState.moveTargetXReached
		? 0
		: resolveMoveTargetHorizontalDirection(state, playerState);
	if (horizontalDirection != 0 && isHorizontalDirectionBlockedByHistory(playerState, horizontalDirection)) {
		if (engine.isPathfindingDebugEnabled()) {
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding direction held by blocker history room='%s' direction='%s' history='%s'",
				state.roomName.c_str(),
				describeBlockedDirection(horizontalDirection, 0),
				horizontalDirection < 0
					? playerState.leftwardBlockerHistory.c_str()
					: playerState.rightwardBlockerHistory.c_str());
		}
		horizontalDirection = 0;
	}
	int depthDirection = horizontalDirection == 0 && !playerState.moveTargetZReached
		? resolveMoveTargetDepthDirection(playerState)
		: 0;
	if (depthDirection != 0 && isDepthDirectionBlockedByHistory(playerState, depthDirection)) {
		if (engine.isPathfindingDebugEnabled()) {
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding direction held by blocker history room='%s' direction='%s' history='%s'",
				state.roomName.c_str(),
				describeBlockedDirection(0, depthDirection),
				depthDirection > 0
					? playerState.upwardBlockerHistory.c_str()
					: playerState.downwardBlockerHistory.c_str());
		}
		depthDirection = 0;
	}
	if (horizontalDirection == 0 && depthDirection == 0) {
		playerState.hasMoveTarget = false;
		return setIdleAnimation(playerState, playerState.facing >= 0 ? playerState.facing : 0);
	}
	if (!consumePlayerMovementTick(playerState))
		return false;

	int candidateCenterX = playerState.centerX;
	int candidateBottomY = playerState.bottomY;
	float candidateZ = playerState.z;
	if (horizontalDirection != 0) {
		const int horizontalStep = computeRoomPlayerHorizontalStep(state, playerState.z);
		candidateCenterX = clampPlayerCenterXToNativeBounds(
			playerState, playerState.centerX + horizontalDirection * horizontalStep);
	} else if (depthDirection != 0) {
		const int verticalStep = computeRoomPlayerVerticalScreenStep();
		const float depthStep = computeRoomPlayerDepthStep(state);
		candidateBottomY = clampRoomMovementY(state,
			playerState.bottomY + depthDirection * verticalStep);
		candidateZ = stepTowardsFloat(playerState.z, playerState.targetZ, depthStep);
	}

	if (engine.isPathfindingDebugEnabled()) {
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding step room='%s' current=(%d,%d,z=%.2f) target=(x=%d,y=%d,z=%.2f) progress=(x=%d,z=%d) direction=(x=%d,z=%d) candidate=(%d,%d,z=%.2f)",
			state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
			playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ,
			playerState.moveTargetXReached, playerState.moveTargetZReached,
			horizontalDirection, depthDirection,
			candidateCenterX, candidateBottomY, (double)candidateZ);
	}

	const Entity *blocker = nullptr;
	bool moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
		playerState, candidateCenterX, candidateBottomY, candidateZ,
		nullptr, &blocker);
	if (!moved) {
		if (engine.isPathfindingDebugEnabled()) {
			const Common::Rect blockerRect = blocker ? blocker->getScreenRect() : Common::Rect();
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding blocked room='%s' candidate=(%d,%d,z=%.2f) blocker='%s' class=0x%x rect=(%d,%d)-(%d,%d) z=%.2f z_extent=%.2f",
				state.roomName.c_str(), candidateCenterX, candidateBottomY, (double)candidateZ,
				blocker ? blocker->getName().c_str() : "",
				blocker ? blocker->getClassId() : -1,
				blockerRect.left, blockerRect.top, blockerRect.right, blockerRect.bottom,
				blocker ? (double)blocker->getZ() : 0.0,
				blocker ? (double)blocker->getZExtent() : 0.0);
		}
		setPlayerCurrentBlocker(playerState, blocker);
		if (blocker && recordNativePrimaryWalkBlockerHistory(state, playerState, *blocker,
				horizontalDirection, depthDirection)) {
			return setIdleAnimation(playerState,
				resolveFacingFromMoveDirection(horizontalDirection, depthDirection,
					playerState.facing));
		}

		playerState.hasMoveTarget = false;
		int idleFacing = playerState.facing;
		if (idleFacing < 0) {
			if (horizontalDirection != 0)
				idleFacing = horizontalDirection < 0 ? 1 : 2;
			else
				idleFacing = depthDirection < 0 ? 3 : 0;
		}
		return setIdleAnimation(playerState, idleFacing);
	}

	const int actualFacing = resolveFacingFromRoomMovement(
		previousCenterX, previousBottomY, playerState.centerX, playerState.bottomY);
	(void)setPlayerWalkAnimation(playerState, actualFacing);
	refreshPlayerCurrentBlocker(engine, sceneObjects, sceneAnimations, playerState);
	if (engine.isPathfindingDebugEnabled()) {
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding applied room='%s' previous=(%d,%d) current=(%d,%d,z=%.2f) facing=%d",
			state.roomName.c_str(), previousCenterX, previousBottomY,
			playerState.centerX, playerState.bottomY, (double)playerState.z,
			actualFacing);
	}
	updatePlayerMoveTargetProgress(state, playerState);
	if (hasPlayerReachedMoveTarget(playerState)) {
		playerState.hasMoveTarget = false;
		(void)setIdleAnimation(playerState, actualFacing);
	}
	debugC(playerState.hasMoveTarget ? 2 : 1, kDebugPlayer,
		"Harvester: player move step room='%s' pos=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f) facing=%d frame=%d active=%d moved=%d",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ,
		playerState.facing, playerState.entity->getCurrentFrame(),
		playerState.hasMoveTarget, moved);
	return true;
}

bool Player::stepKeyboardMovement(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<AnimRecord> &sceneAnimations,
		RoomPlayerState &playerState, bool moveLeft, bool moveRight, bool moveUp, bool moveDown) {
	if (!playerState.entity || !supportsMovementBand(state))
		return false;

	const int horizontalInput = (moveRight ? 1 : 0) - (moveLeft ? 1 : 0);
	const int verticalInput = (moveDown ? 1 : 0) - (moveUp ? 1 : 0);
	if (horizontalInput == 0 && verticalInput == 0)
		return false;
	if (playerState.turnActive || playerState.hitActive)
		return false;

	const int previousCenterX = playerState.centerX;
	const int previousBottomY = playerState.bottomY;
	const float previousZ = playerState.z;
	const int horizontalStep = computeRoomPlayerHorizontalStep(state, playerState.z);
	const int verticalStep = computeRoomPlayerVerticalScreenStep();
	const float depthStep = computeRoomPlayerDepthStep(state);
	int candidateCenterX = clampPlayerCenterXToNativeBounds(
		playerState, playerState.centerX + horizontalInput * horizontalStep);
	int candidateBottomY = playerState.bottomY;
	float candidateZ = playerState.z;
	if (verticalInput != 0) {
		candidateBottomY = clampRoomMovementY(state,
			playerState.bottomY + verticalInput * verticalStep);
		// Native room-combat movement advances screen Y and room depth independently:
		// down-walk states apply -z_velocity_step while up-walk states apply +z_velocity_step.
		candidateZ = clampRoomDepth(state, playerState.z - verticalInput * depthStep);
	}
	if (candidateCenterX == playerState.centerX &&
			candidateBottomY == playerState.bottomY &&
			fabsf(candidateZ - playerState.z) <= kRoomDepthCompareEpsilon) {
		return false;
	}

	const int desiredFacing = resolveFacingFromRoomMovement(
		playerState.centerX, playerState.bottomY, candidateCenterX, candidateBottomY);
	if (desiredFacing != playerState.facing && startTurnAnimation(playerState, desiredFacing))
		return true;

	if (!consumePlayerMovementTick(playerState))
		return false;

	playerState.hasMoveTarget = false;
	bool moved = tryApplyPlayerMovementWithFallbacks(engine, state, sceneObjects, sceneAnimations,
		playerState, previousCenterX, previousBottomY, previousZ,
		candidateCenterX, candidateBottomY, candidateZ);
	if (!moved)
		return false;

	const int actualFacing = resolveFacingFromRoomMovement(
		previousCenterX, previousBottomY, playerState.centerX, playerState.bottomY);
	(void)setPlayerWalkAnimation(playerState, actualFacing);
	debugC(2, kDebugPlayer,
		"Harvester: player keyboard move room='%s' input=(%d,%d) pos=(%d,%d,z=%.2f) facing=%d frame=%d moved=%d",
		state.roomName.c_str(), horizontalInput, verticalInput,
		playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.facing, playerState.entity->getCurrentFrame(), moved);
	return true;
}

bool Player::requestIdleAnimationExit(const RoomSetupState &state,
		RoomPlayerState &playerState, RoomIdleAnimationState &idleState) {
	if (!idleState.active || idleState.exiting || !idleState.entity)
		return false;

	const int lastFrame = idleState.entity->getLastFrame();
	const int exitStartFrame = MIN(lastFrame, kRoomPlayerIdleLoopLastFrame);
	const int exitLastFrame = MIN(lastFrame, kRoomPlayerIdleExitLastFrame);
	if (exitLastFrame <= exitStartFrame)
		return finishPlayerIdleAnimation(state, playerState, idleState);

	positionPlayerIdleAnimationEntity(state, playerState, *idleState.entity);
	idleState.exiting = true;
	idleState.entity->setAnimationFrameRange(exitStartFrame, exitLastFrame, false);
	idleState.entity->setCurrentFrame(exitStartFrame);
	idleState.entity->setAnimationRate(kRoomPlayerIdleAnimationRate);
	debugC(1, kDebugPlayer,
		"Harvester: player idle animation exit room='%s' frames=%d..%d rate=%d",
		state.roomName.c_str(), exitStartFrame, exitLastFrame, kRoomPlayerIdleAnimationRate);
	return true;
}

bool Player::startIdleAnimation(HarvesterEngine &engine, const RoomSetupState &state,
		RoomPlayerState &playerState, RoomIdleAnimationState &idleState) {
	if (!playerState.entity || idleState.active || isIdleAnimationExcludedRoom(state.roomName))
		return false;

	Entity *idleEntity = ensurePlayerIdleAnimationEntity(engine, state, playerState, idleState);
	if (!idleEntity)
		return false;

	const int lastFrame = idleEntity->getLastFrame();
	const int loopLastFrame = MIN(lastFrame, kRoomPlayerIdleLoopLastFrame);
	positionPlayerIdleAnimationEntity(state, playerState, *idleEntity);
	idleState.restoreFacing = playerState.facing;
	idleState.active = true;
	idleState.loopStarted = false;
	idleState.exiting = false;
	idleEntity->setVisible(true);
	idleEntity->setAnimationFrameRange(0, loopLastFrame, false);
	idleEntity->setCurrentFrame(0);
	idleEntity->setAnimationRate(kRoomPlayerIdleAnimationRate);
	playerState.hasMoveTarget = false;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	playerState.entity->setVisible(false);
	debugC(1, kDebugPlayer,
		"Harvester: player idle animation start room='%s' facing=%d frames=%d..%d rate=%d trigger_tick=%u",
		state.roomName.c_str(), idleState.restoreFacing, 0, loopLastFrame,
		kRoomPlayerIdleAnimationRate, idleState.triggerTick);
	return true;
}

bool Player::updateIdleAnimation(const RoomSetupState &state,
		RoomPlayerState &playerState, RoomIdleAnimationState &idleState) {
	if (!idleState.active || !idleState.entity)
		return false;

	const int lastFrame = idleState.entity->getLastFrame();
	const int loopLastFrame = MIN(lastFrame, kRoomPlayerIdleLoopLastFrame);
	if (!idleState.exiting && !idleState.loopStarted &&
			idleState.entity->getCurrentFrame() >= loopLastFrame) {
		const int loopStartFrame = MIN(lastFrame, kRoomPlayerIdleLoopStartFrame);
		idleState.loopStarted = true;
		if (loopStartFrame < loopLastFrame) {
			idleState.entity->setAnimationFrameRange(loopStartFrame, loopLastFrame, true);
			idleState.entity->setCurrentFrame(loopStartFrame);
			idleState.entity->setAnimationRate(kRoomPlayerIdleAnimationRate);
		}
		debugC(1, kDebugPlayer,
			"Harvester: player idle animation loop room='%s' frames=%d..%d rate=%d",
			state.roomName.c_str(), loopStartFrame, loopLastFrame, kRoomPlayerIdleAnimationRate);
		return true;
	}

	if (!idleState.exiting)
		return false;

	const int exitLastFrame = MIN(lastFrame, kRoomPlayerIdleExitLastFrame);
	if (idleState.entity->getCurrentFrame() < exitLastFrame)
		return false;

	return finishPlayerIdleAnimation(state, playerState, idleState);
}

} // End of namespace Harvester
