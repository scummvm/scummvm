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

#include "harvester/flow.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/art.h"
#include "harvester/script.h"

namespace Harvester {

namespace {

static const char *const kQuickTipsPath = "ADJHEAD.RCS";
static const char *const kMenuPath = "MENU.INI";
static const char *const kMenuSectionName = "menu";
static const char *const kTownMapPalettePath = "1:/GRAPHIC/PAL/HARVMAP.PAL";
static const char *const kTownMapMusicPath = "SOUND/MUSIC/MENACE.CMP";
static const char *const kTownMapBitmapPaths[] = {
	"1:/GRAPHIC/TOWN/HARVMAP1.BM",
	"1:/GRAPHIC/TOWN/HARVMAP2.BM",
	"1:/GRAPHIC/TOWN/HARVMAP3.BM",
	"1:/GRAPHIC/TOWN/HARVMAP4.BM"
};

static const int kQuickTipsOverlayX = 167;
static const int kQuickTipsOverlayY = 200;

static const int kQuickTipTextX = 180;
static const int kQuickTipTextY = 228;
static const int kQuickTipTextWidth = 280;

static const int kCursorSequence7 = 7;
static const int kIdentTextboxX = 85;
static const int kIdentTextboxY = 177;
static const int kIdentTextboxTextInsetX = 10;
static const int kIdentTextboxTextInsetY = 5;
static const char *const kPlayerActorEntityName = "PLAYER";
static const char *const kPlayerActorResourcePath = "1:/GRAPHIC/MONSTERS/PC/PC0.ABM";
static const uint32 kPaletteFadeTickMs = 4;
static const float kPaletteFadeStep = 0.1f;
static const float kPaletteBrightnessBlack = 0.0f;
static const float kRoomPlayerHorizontalMoveBase = 8.0f;
static const int kRoomRegionTargetXBias = 10;
static const float kRoomDepthCompareEpsilon = 0.01f;
static const int kRoomPlayerWalkAnimationRate = 17;
static const int kRoomNpcAmbientLastFrame = 0x3b;
static const int kTownMapEdgeThreshold = 9;
static const int kTownMapCursorHitExtent = 5;

static const byte kIdentTextColor = 0xd3;
static const byte kTextColorNormal = 255;
static const byte kShadowColor = 0;
static const byte kQuickTipActionColor = 0xc3;
static const byte kTownMapLabelColor = 0x28;

static const int kCursorSequenceWalk = 0;
static const int kCursorSequenceExamine = 1;
static const int kCursorSequenceTalk = 2;
static const int kCursorSequenceOperate = 4;
static const int kCursorSequencePickup = 5;
static const int kCursorSequenceTransition = 6;
static const int kCursorSequenceNeutral = 7;
static const char *const kPlayerIdleAnimationEntityName = "IDLE_ANIM";
static const char *const kPlayerIdleAnimationResourcePath = "1:/GRAPHIC/ROOMANIM/PCLOUN02.ABM";
static const uint32 kRuntimeClockDivisorMs = 10;
static const uint32 kRoomPlayerIdleDelayTicks = 3000;
static const int kRoomPlayerIdleAnimationRate = 30;
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

} // End of anonymous namespace

static int roundToInt(float value) {
	return value >= 0.0f ? (int)floorf(value + 0.5f) : (int)ceilf(value - 0.5f);
}

static int clampTownMapPanelIndex(int panelIndex) {
	return CLIP<int>(panelIndex, 0, ARRAYSIZE(kTownMapBitmapPaths) - 1);
}

static int resolveTownMapEdgePanel(int currentPanel, const Common::Point &mousePos, int width, int height) {
	if (mousePos.x <= kTownMapEdgeThreshold) {
		if (currentPanel == 1)
			return 0;
		if (currentPanel == 3)
			return 2;
	} else if (mousePos.x >= width - 1 - kTownMapEdgeThreshold) {
		if (currentPanel == 0)
			return 1;
		if (currentPanel == 2)
			return 3;
	} else if (mousePos.y <= kTownMapEdgeThreshold) {
		if (currentPanel == 2)
			return 0;
		if (currentPanel == 3)
			return 1;
	} else if (mousePos.y >= height - 1 - kTownMapEdgeThreshold) {
		if (currentPanel == 0)
			return 2;
		if (currentPanel == 1)
			return 3;
	}

	return currentPanel;
}

static int resolveTownMapKeyPanel(int currentPanel, Common::KeyCode keycode) {
	switch (keycode) {
	case Common::KEYCODE_LEFT:
		if (currentPanel == 1)
			return 0;
		if (currentPanel == 3)
			return 2;
		break;
	case Common::KEYCODE_RIGHT:
		if (currentPanel == 0)
			return 1;
		if (currentPanel == 2)
			return 3;
		break;
	case Common::KEYCODE_UP:
		if (currentPanel == 2)
			return 0;
		if (currentPanel == 3)
			return 1;
		break;
	case Common::KEYCODE_DOWN:
		if (currentPanel == 0)
			return 2;
		if (currentPanel == 1)
			return 3;
		break;
	default:
		break;
	}

	return currentPanel;
}

static bool isTownMapLocationHovered(const StartupMapLocationRecord &location, int currentPanel,
		const Common::Point &mousePos) {
	if (location.panelIndex != currentPanel)
		return false;

	const int cursorRight = mousePos.x + kTownMapCursorHitExtent;
	const int cursorBottom = mousePos.y + kTownMapCursorHitExtent;
	return mousePos.x <= location.maxX && cursorRight >= location.minX &&
		mousePos.y <= location.maxY && cursorBottom >= location.minY;
}

static const StartupMapLocationRecord *findTownMapLocationAt(
		const Common::Array<StartupMapLocationRecord> &locations, int currentPanel,
		const Common::Point &mousePos) {
	for (const StartupMapLocationRecord &location : locations) {
		if (isTownMapLocationHovered(location, currentPanel, mousePos))
			return &location;
	}

	return nullptr;
}

static int resolvePlayerFacingFrame(int facing) {
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

static int resolveMonsterFacingFrame(int facing) {
	switch (facing) {
	case 1:
		return 0x0e;
	case 2:
		return 0x2c;
	case 3:
		return 0x28;
	default:
		return 0;
	}
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
	// Ghidra confirms stationary startup keyboard turn states between the locomotion banks.
	if (fromFacing == 0 && toFacing == 1) {
		range = PlayerTurnAnimationRange{ 0x0a, 0x0e, true };
		return true;
	}
	if (fromFacing == 1 && toFacing == 0) {
		range = PlayerTurnAnimationRange{ 0x0a, 0x0e, false };
		return true;
	}
	if (fromFacing == 1 && toFacing == 3) {
		range = PlayerTurnAnimationRange{ 0x19, 0x1d, true };
		return true;
	}
	if (fromFacing == 3 && toFacing == 1) {
		range = PlayerTurnAnimationRange{ 0x19, 0x1d, false };
		return true;
	}
	if (fromFacing == 3 && toFacing == 2) {
		range = PlayerTurnAnimationRange{ 0x28, 0x2c, true };
		return true;
	}
	if (fromFacing == 2 && toFacing == 3) {
		range = PlayerTurnAnimationRange{ 0x28, 0x2c, false };
		return true;
	}
	if (fromFacing == 2 && toFacing == 0) {
		range = PlayerTurnAnimationRange{ 0x37, 0x3b, true };
		return true;
	}
	if (fromFacing == 0 && toFacing == 2) {
		range = PlayerTurnAnimationRange{ 0x37, 0x3b, false };
		return true;
	}

	return false;
}

static bool roomSupportsMovementBand(const StartupRoomSetupState &state) {
	return state.roomMaxZScreenY >= 0 &&
		state.roomMinZScreenY >= state.roomMaxZScreenY;
}

static int clampRoomMovementY(const StartupRoomSetupState &state, int screenY) {
	if (!roomSupportsMovementBand(state))
		return screenY;

	return CLIP<int>(screenY, state.roomMaxZScreenY, state.roomMinZScreenY);
}

static float mapRoomScreenYToDepth(const StartupRoomSetupState &state, int screenY) {
	if (!roomSupportsMovementBand(state))
		return (float)state.playerSpawnZ;

	if (screenY <= state.roomMaxZScreenY)
		return (float)state.roomMaxZ;
	if (screenY >= state.roomMinZScreenY)
		return (float)state.roomMinZ;

	return (float)(state.roomMaxZ - ((state.roomMaxZ - state.roomMinZ) *
		(screenY - state.roomMaxZScreenY)) /
		(state.roomMinZScreenY - state.roomMaxZScreenY));
}

static float clampRoomDepth(const StartupRoomSetupState &state, float z) {
	if (!roomSupportsMovementBand(state))
		return z;

	return CLIP<float>(z,
		(float)MIN(state.roomMinZ, state.roomMaxZ),
		(float)MAX(state.roomMinZ, state.roomMaxZ));
}

static int mapRoomDepthToScreenY(const StartupRoomSetupState &state, float z) {
	if (!roomSupportsMovementBand(state))
		return state.playerSpawnY;

	if (state.roomMaxZ == state.roomMinZ)
		return clampRoomMovementY(state, state.playerSpawnY);

	const float clampedZ = clampRoomDepth(state, z);
	const float offset = ((float)state.roomMaxZ - clampedZ) *
		(float)(state.roomMinZScreenY - state.roomMaxZScreenY) /
		(float)(state.roomMaxZ - state.roomMinZ);
	return clampRoomMovementY(state, state.roomMaxZScreenY + roundToInt(offset));
}

static float computeActorDepthScale(const StartupRoomSetupState &state, float z) {
	float scale = 1.0f;
	if (state.roomPerspectiveScale != 0.0f) {
		scale -= (z - (float)state.roomFullScaleZ) * state.roomPerspectiveScale;
		if (scale <= 0.0f)
			scale = 1.0f;
	}

	return scale;
}

static void setRoomActorScreenPosition(RuntimeEntity &entity, int centerX, int bottomY, float z,
		int width, int height, int xOffset, int yOffset) {
	entity.setPosition(centerX - xOffset - width / 2, bottomY - height - yOffset, z);
}

static bool applyRoomActorPlacementInternal(const StartupRoomSetupState &state, RuntimeEntity &entity,
		int centerX, int bottomY, float z, const Common::String *entranceName, bool applyDepthScale) {
	int width = 0;
	int height = 0;
	int xOffset = 0;
	int yOffset = 0;
	if (!entity.getCurrentFrameMetrics(width, height, xOffset, yOffset))
		return false;

	entity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
	setRoomActorScreenPosition(entity, centerX, bottomY, z, width, height, xOffset, yOffset);

	float depthScale = 1.0f;
	if (applyDepthScale) {
		depthScale = computeActorDepthScale(state, z);
		entity.setDepthScale(depthScale);
		if (!entity.getCurrentFrameMetrics(width, height, xOffset, yOffset))
			return false;
		setRoomActorScreenPosition(entity, centerX, bottomY, z, width, height, xOffset, yOffset);
	}

	if (entranceName) {
		debugC(1, kDebugGeneral,
			"Harvester: startup player placement entrance='%s' spawn=(%d,%d,%d) screen_base=(%d,%d) size=%dx%d offsets=(%d,%d) scale=%.3f",
			entranceName->c_str(), centerX, bottomY, roundToInt(z),
			entity.getX(), entity.getY(), width, height, xOffset, yOffset, (double)depthScale);
	}
	return true;
}

bool applyRoomActorPlacement(const StartupRoomSetupState &state, RuntimeEntity &entity,
		int centerX, int bottomY, float z, const Common::String *entranceName) {
	return applyRoomActorPlacementInternal(state, entity, centerX, bottomY, z, entranceName, true);
}

static bool applyStartupActorPlacement(const StartupRoomSetupState &state, RuntimeEntity &entity) {
	return applyRoomActorPlacement(state, entity,
		state.playerSpawnX, state.playerSpawnY, (float)state.playerSpawnZ, &state.entranceName);
}

static int resolveFacingFromMovementDelta(int dx, int dy) {
	if (ABS(dx) > ABS(dy))
		return dx < 0 ? 1 : 2;
	if (dy < 0)
		return 3;
	return 0;
}

static int resolveFacingFromRoomMovement(const StartupRoomSetupState &state,
		int fromCenterX, float fromZ, int toCenterX, float toZ) {
	return resolveFacingFromMovementDelta(
		toCenterX - fromCenterX,
		mapRoomDepthToScreenY(state, toZ) - mapRoomDepthToScreenY(state, fromZ));
}

static int computeRoomPlayerHorizontalStep(const StartupRoomSetupState &state, float z) {
	return MAX<int>(1, roundToInt(computeActorDepthScale(state, z) * kRoomPlayerHorizontalMoveBase));
}

static float computeRoomPlayerDepthStep(const StartupRoomSetupState &state) {
	return state.roomZVelocityStep > 0.0f ? state.roomZVelocityStep : 1.0f;
}

uint32 getRuntimeClockTicks();

static uint32 computeAnimationTickInterval(int rate) {
	return rate > 0 ? MAX<uint32>(1, 100U / (uint32)rate) : 0;
}

static bool consumePlayerMovementTick(StartupRoomPlayerState &playerState) {
	const uint32 interval = computeAnimationTickInterval(kRoomPlayerWalkAnimationRate);
	if (interval == 0) {
		playerState.nextMovementTick = 0;
		return true;
	}

	const uint32 now = getRuntimeClockTicks();
	if (playerState.nextMovementTick != 0 &&
			(int32)(now - playerState.nextMovementTick) < 0) {
		return false;
	}

	playerState.nextMovementTick = now + interval;
	return true;
}

static int stepTowardsInt(int current, int target, int step) {
	if (step <= 0 || current == target)
		return current;
	if (current < target)
		return MIN(current + step, target);
	return MAX(current - step, target);
}

static float stepTowardsFloat(float current, float target, float step) {
	if (step <= 0.0f || fabsf(target - current) <= kRoomDepthCompareEpsilon)
		return target;
	if (current < target)
		return MIN(current + step, target);
	return MAX(current - step, target);
}

static bool doesPlayerOverlapRoomBlocker(HarvesterEngine &engine,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations, const RuntimeEntity &playerEntity) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;

	for (const StartupObjectRecord &object : sceneObjects) {
		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(object.objectName);
		if (entity && playerEntity.overlapsEntity(*entity))
			return true;
	}

	for (const StartupAnimRecord &anim : sceneAnimations) {
		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(anim.animName);
		if (entity && playerEntity.overlapsEntity(*entity))
			return true;
	}

	return false;
}

static bool isPlayerMovementBlocked(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState, int candidateCenterX, float candidateZ) {
	if (!playerState.entity)
		return true;

	const int oldCenterX = playerState.centerX;
	const int oldBottomY = playerState.bottomY;
	const float oldZ = playerState.z;
	const int candidateBottomY = mapRoomDepthToScreenY(state, candidateZ);
	if (!applyRoomActorPlacement(state, *playerState.entity,
			candidateCenterX, candidateBottomY, candidateZ)) {
		(void)applyRoomActorPlacement(state, *playerState.entity, oldCenterX, oldBottomY, oldZ);
		return true;
	}

	const bool blocked = doesPlayerOverlapRoomBlocker(engine, sceneObjects, sceneAnimations, *playerState.entity);
	(void)applyRoomActorPlacement(state, *playerState.entity, oldCenterX, oldBottomY, oldZ);
	return blocked;
}

static bool tryApplyPlayerMovement(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState, int candidateCenterX, float candidateZ) {
	if (!playerState.entity)
		return false;

	candidateCenterX = CLIP<int>(candidateCenterX, 0, 639);
	candidateZ = clampRoomDepth(state, candidateZ);
	if (candidateCenterX == playerState.centerX &&
			fabsf(candidateZ - playerState.z) <= kRoomDepthCompareEpsilon) {
		return false;
	}
	if (isPlayerMovementBlocked(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, candidateZ)) {
		return false;
	}

	playerState.centerX = candidateCenterX;
	playerState.z = candidateZ;
	playerState.bottomY = mapRoomDepthToScreenY(state, candidateZ);
	return applyRoomActorPlacement(state, *playerState.entity,
		playerState.centerX, playerState.bottomY, playerState.z);
}

void setPlayerMoveTarget(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
		int targetX, float targetZ) {
	playerState.hasMoveTarget = true;
	playerState.nextMovementTick = 0;
	playerState.targetX = CLIP<int>(targetX, 0, 639);
	playerState.targetZ = clampRoomDepth(state, targetZ);
	debugC(1, kDebugScene,
		"Harvester: player move target room='%s' current=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f)",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, mapRoomDepthToScreenY(state, playerState.targetZ), (double)playerState.targetZ);
}

void setPlayerMoveTargetFromScreenPoint(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, int targetX, int targetBottomY) {
	setPlayerMoveTarget(state, playerState, targetX,
		mapRoomScreenYToDepth(state, clampRoomMovementY(state, targetBottomY)));
}

int resolveRegionTargetX(const StartupRegionRecord &region, const StartupRoomPlayerState &playerState) {
	const Common::Rect bounds(region.left, region.top, region.right + 1, region.bottom + 1);
	int targetX = bounds.left + bounds.width() / 2;
	if (playerState.entity && targetX + kRoomRegionTargetXBias < playerState.entity->getScreenRect().right)
		targetX -= kRoomRegionTargetXBias;
	return CLIP<int>(targetX, 0, 639);
}

float resolveRegionTargetZ(const StartupRegionRecord &region) {
	if (region.desiredFacing == 0)
		return (float)region.maxZ;
	if (region.desiredFacing == 3)
		return (float)region.minZ;

	return (float)(region.minZ + (region.maxZ - region.minZ) / 2);
}

uint32 getRuntimeClockTicks() {
	return g_system ? (g_system->getMillis() / kRuntimeClockDivisorMs) : 0;
}

bool isIdleAnimationExcludedRoom(const Common::String &roomName) {
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

void updatePlayerIdleTrigger(StartupRoomIdleAnimationState &idleState) {
	idleState.triggerTick = MAX(idleState.activityTick, idleState.resetTick) + kRoomPlayerIdleDelayTicks;
}

static void notePlayerIdleReset(StartupRoomIdleAnimationState &idleState) {
	idleState.resetTick = getRuntimeClockTicks();
	updatePlayerIdleTrigger(idleState);
}

static void getPaletteByteRange(const byte *palette, byte &minValue, byte &maxValue) {
	minValue = 255;
	maxValue = 0;

	for (uint i = 0; i < 256 * 3; ++i) {
		minValue = MIN(minValue, palette[i]);
		maxValue = MAX(maxValue, palette[i]);
	}
}

static uint32 hashPalette(const byte *palette) {
	uint32 hash = 2166136261U;
	for (uint i = 0; i < 256 * 3; ++i) {
		hash ^= palette[i];
		hash *= 16777619U;
	}

	return hash;
}

void logScenePaletteSummary(const char *label, const StartupRoomSceneResources &scene, float brightness) {
	const byte *palette = scene.palette;
	byte minValue = 0;
	byte maxValue = 0;
	getPaletteByteRange(palette, minValue, maxValue);

	debugC(1, kDebugGeneral,
		"Harvester: %s room='%s' palette='%s' bytes=%u range=[%u,%u] brightness=%.2f hash=%08x idx0=(%u,%u,%u) idx1=(%u,%u,%u) idx127=(%u,%u,%u) idx255=(%u,%u,%u)",
		label, scene.state.roomName.c_str(), scene.state.palettePath.c_str(), 256U * 3U, minValue, maxValue,
		(double)brightness, hashPalette(palette),
		palette[0], palette[1], palette[2],
		palette[3], palette[4], palette[5],
		palette[127 * 3], palette[127 * 3 + 1], palette[127 * 3 + 2],
		palette[255 * 3], palette[255 * 3 + 1], palette[255 * 3 + 2]);
}

static Common::Rect quickTipsExitRect() {
	return Common::Rect(180, 280, 238, 291);
}

static Common::Rect quickTipsNextRect() {
	return Common::Rect(420, 280, 492, 291);
}

static Common::Rect quickTipsToggleRect() {
	return Common::Rect(258, 280, 366, 291);
}

static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	screen.copyRectToSurface(bitmap.pixels.data(), bitmap.width, x, y, bitmap.width, bitmap.height);
}

bool captureScreenBackdrop(const Graphics::Screen &screen, IndexedBitmap &bitmap) {
	if (screen.w <= 0 || screen.h <= 0 || screen.format.bytesPerPixel != 1)
		return false;

	bitmap.width = (uint32)screen.w;
	bitmap.height = (uint32)screen.h;
	bitmap.pixels.resize(bitmap.width * bitmap.height);
	for (int y = 0; y < screen.h; ++y)
		memcpy(bitmap.pixels.data() + y * bitmap.width, screen.getBasePtr(0, y), bitmap.width);

	return true;
}

static void drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, Graphics::TextAlign align = Graphics::kTextAlignLeft) {
	font.drawString(&screen, text, x + 1, y + 1, width, kShadowColor, align);
	font.drawString(&screen, text, x, y, width, color, align);
}

static void drawWrappedShadowedText(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color) {
	Common::Array<Common::String> lines;
	font.wordWrapText(text, width, lines);

	const int lineHeight = font.getFontHeight() + 2;
	for (uint i = 0; i < lines.size(); ++i)
		drawShadowedString(screen, font, lines[i], x, y + i * lineHeight, width, color);
}

static void setScaledPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getStartupGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *dest) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(dest, data.data(), 256 * 3);

	byte minValue = 0;
	byte maxValue = 0;
	getPaletteByteRange(dest, minValue, maxValue);
	debugC(1, kDebugGeneral,
		"Harvester: decoded room palette '%s' bytes=%u range=[%u,%u] idx0=(%u,%u,%u) idx255=(%u,%u,%u)",
		path.c_str(), (uint)data.size(), minValue, maxValue,
		dest[0], dest[1], dest[2],
		dest[255 * 3], dest[255 * 3 + 1], dest[255 * 3 + 2]);

	return true;
}

Common::Rect getHotspotBounds(const StartupObjectRecord &object) {
	if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
		return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);

	return Common::Rect();
}

static Common::Rect getRegionBounds(const StartupRegionRecord &region) {
	if (region.right > region.left && region.bottom > region.top)
		return Common::Rect(region.left, region.top, region.right + 1, region.bottom + 1);

	return Common::Rect();
}

Common::String resolveSceneObjectSpritePath(const StartupObjectRecord &object) {
	const bool atInitialPlacement = object.currentX == object.initialX &&
		object.currentY == object.initialY &&
		object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
	if (!object.altSpritePath.empty() &&
		(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase("INVENTORY")))
		return object.altSpritePath;

	return object.spritePath;
}

bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap = IndexedBitmap();
	bitmap.width = READ_LE_UINT32(data.data());
	bitmap.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = bitmap.width * bitmap.height;
	if (bitmap.width == 0 || bitmap.height == 0 || data.size() < 12 + pixelCount)
		return false;

	bitmap.pixels.resize(pixelCount);
	memcpy(bitmap.pixels.data(), data.data() + 12, pixelCount);
	return true;
}

static void logSceneObjectSelection(const char *decision, const char *source, const StartupObjectRecord &object,
		const Common::String &detail = Common::String()) {
	const Common::Rect hotspotBounds = getHotspotBounds(object);
	const Common::String resolvedSpritePath = resolveSceneObjectSpritePath(object);
	debugC(1, kDebugScene,
		"Harvester: scene object %s source='%s' object='%s' owner='%s' visible=%d runtimeVisible=%d sprite='%s' alt='%s' resolved='%s' pos=(%d,%d,%d) bounds=(%d,%d)-(%d,%d) action='%s' detail='%s'",
		decision, source, object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
		object.visible, object.runtimeVisible, object.spritePath.c_str(), object.altSpritePath.c_str(),
		resolvedSpritePath.c_str(),
		object.currentX, object.currentY, object.currentZ,
		hotspotBounds.left, hotspotBounds.top, hotspotBounds.right, hotspotBounds.bottom,
		object.actionTag.c_str(), detail.c_str());
}

static bool isBackgroundSceneObject(const StartupObjectRecord &object, const RuntimeEntity &entity) {
	return object.initialX == 0 && object.initialY == 0 &&
		entity.getBoundsWidth() == 640 && entity.getBoundsHeight() == 480;
}

static bool isInteractiveSceneHotspot(const StartupObjectRecord &object, Script *startupScript) {
	if (object.operatable || !object.actionTag.empty())
		return true;
	if (!startupScript)
		return false;
	if (startupScript->isPickupObject(object))
		return true;

	StartupResolvedText inspectText;
	return startupScript->resolveObjectInspectText(object, inspectText);
}

static int resolveSceneObjectClass(const StartupObjectRecord &object, const RuntimeEntity *entity,
		Script *startupScript) {
	if (entity)
		return isBackgroundSceneObject(object, *entity) ? kRuntimeEntityClassBackground : kRuntimeEntityClassObject;

	return isInteractiveSceneHotspot(object, startupScript)
		? kRuntimeEntityClassRectHotspot
		: kRuntimeEntityClassDisabledHotspot;
}

static void queueVisibleSceneObject(const char *source, const StartupObjectRecord &object,
		Common::Array<StartupObjectRecord> &sceneObjects) {
	if (!object.visible) {
		logSceneObjectSelection("skipped", source, object, "visible=0");
		return;
	}

	for (const StartupObjectRecord &sceneObject : sceneObjects) {
		if (sceneObject.currentOwnerOrRoom.equalsIgnoreCase(object.currentOwnerOrRoom) &&
			sceneObject.objectName.equalsIgnoreCase(object.objectName)) {
			logSceneObjectSelection("skipped", source, object, "duplicate owner/object");
			return;
		}
	}

	sceneObjects.push_back(object);
	logSceneObjectSelection("queued", source, object);
}

bool loadRoomSceneResources(const StartupRoomSetupState &state, ResourceManager &resources, StartupRoomSceneResources &scene) {
	scene = StartupRoomSceneResources();
	scene.state = state;
	scene.targetPaletteBrightness = state.paletteBrightness;
	if (!loadPaletteResource(resources, state.palettePath, scene.palette)) {
		return false;
	}

	Common::Array<StartupObjectRecord> sceneObjects;
	for (const StartupObjectRecord &object : state.roomObjects)
		queueVisibleSceneObject("room", object, sceneObjects);
	for (const StartupObjectRecord &object : state.activeObjects)
		queueVisibleSceneObject("active", object, sceneObjects);

	scene.sceneObjects = sceneObjects;
	for (const StartupAnimRecord &anim : state.roomAnimations) {
		if (anim.active || anim.visible)
			scene.sceneAnimations.push_back(anim);
	}
	scene.sceneRegions = state.roomRegions;

	debugC(1, kDebugGeneral,
		"Harvester: loadRoomSceneResources room='%s' palette='%s' background='%s' brightness=%.2f objects=%u activeObjects=%u sceneObjects=%u anims=%u visibleAnims=%u npcs=%u monsters=%u regions=%u",
		state.roomName.c_str(), state.palettePath.c_str(), state.backgroundPath.c_str(), (double)state.paletteBrightness,
		(uint)state.roomObjects.size(), (uint)state.activeObjects.size(), (uint)scene.sceneObjects.size(),
		(uint)state.roomAnimations.size(), (uint)scene.sceneAnimations.size(),
		(uint)state.roomNpcs.size(), (uint)state.roomMonsters.size(), (uint)scene.sceneRegions.size());

	return true;
}

bool shouldRunStartupRoomProbe() {
	return ConfMan.hasKey("harvester_debug_probe_startup_room") &&
		ConfMan.getBool("harvester_debug_probe_startup_room");
}

void drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen, const StartupRoomSceneResources &scene,
		float brightness) {
	setScaledPalette(screen, scene.palette, brightness);
	screen.fillRect(screen.getBounds(), 0);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawSceneEntities(screen);
}

const StartupObjectRecord *findSceneObjectByName(const Common::Array<StartupObjectRecord> &objects,
		const Common::String &objectName) {
	for (const StartupObjectRecord &object : objects) {
		if (object.objectName.equalsIgnoreCase(objectName))
			return &object;
	}

	return nullptr;
}

const StartupRegionRecord *findSceneRegionByName(const Common::Array<StartupRegionRecord> &regions,
		const Common::String &regionName) {
	for (const StartupRegionRecord &region : regions) {
		if (region.regionName.equalsIgnoreCase(regionName))
			return &region;
	}

	return nullptr;
}

static const StartupNpcRecord *findSceneNpcByName(const Common::Array<StartupNpcRecord> &npcs,
		const Common::String &npcName) {
	for (const StartupNpcRecord &npc : npcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

StartupObjectRecord *findSceneObjectByName(Common::Array<StartupObjectRecord> &objects,
		const Common::String &objectName) {
	for (StartupObjectRecord &object : objects) {
		if (object.objectName.equalsIgnoreCase(objectName))
			return &object;
	}

	return nullptr;
}

static const StartupObjectRecord *findRoomObjectAtPoint(HarvesterEngine &engine,
		const Common::Array<StartupObjectRecord> &sceneObjects, const Common::Point &point) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	const RuntimeEntity *topEntity = nullptr;
	for (const StartupObjectRecord &object : sceneObjects) {
		if (object.objectName.empty())
			continue;

		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(object.objectName);
		if (!entity || !entity->hitTest(point))
			continue;
		if (entity->getClassId() == kRuntimeEntityClassBackground ||
			entity->getClassId() == kRuntimeEntityClassPlayer ||
			entity->getClassId() == kRuntimeEntityClassRectHotspot19) {
			continue;
		}
		if (!topEntity || entity->getZ() <= topEntity->getZ())
			topEntity = entity;
	}
	if (!topEntity)
		return nullptr;

	return findSceneObjectByName(sceneObjects, topEntity->getName());
}

static const StartupRegionRecord *findRoomRegionAtPoint(HarvesterEngine &engine,
		const Common::Array<StartupRegionRecord> &sceneRegions, const Common::Point &point) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	const RuntimeEntity *topEntity = nullptr;
	for (const StartupRegionRecord &region : sceneRegions) {
		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(region.regionName);
		if (!entity || !entity->hitTest(point))
			continue;
		if (entity->getClassId() != kRuntimeEntityClassRectHotspot19)
			continue;
		if (!topEntity || entity->getZ() <= topEntity->getZ())
			topEntity = entity;
	}
	if (!topEntity)
		return nullptr;

	return findSceneRegionByName(sceneRegions, topEntity->getName());
}

static const RuntimeEntity *findRoomPlayerAtPoint(HarvesterEngine &engine, const Common::Point &point) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	const RuntimeEntity *entity = runtimeEntities->findTopSceneEntityAt(point);
	if (!entity || entity->getClassId() != kRuntimeEntityClassPlayer)
		return nullptr;

	return entity;
}

static const StartupNpcRecord *findRoomNpcAtPoint(HarvesterEngine &engine,
		const Common::Array<StartupNpcRecord> &sceneNpcs, const Common::Point &point) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	const RuntimeEntity *entity = runtimeEntities->findTopSceneEntityAt(point);
	if (!entity || entity->getClassId() != kRuntimeEntityClassNpc)
		return nullptr;

	return findSceneNpcByName(sceneNpcs, entity->getName());
}

const IndexedBitmap *resolveInspectTextboxBitmap(const Art &art, const StartupResolvedText &text) {
	if (text.boxName.equalsIgnoreCase("BOX1"))
		return art.getTextboxBitmap(0);
	if (text.boxName.equalsIgnoreCase("BOX2"))
		return art.getTextboxBitmap(1);
	if (text.boxName.equalsIgnoreCase("BOX3"))
		return art.getTextboxBitmap(2);
	if (text.boxName.equalsIgnoreCase("BOX4"))
		return art.getTextboxBitmap(3);

	return nullptr;
}

void drawRoomInspectText(Graphics::Screen &screen, const Art &art, const Graphics::Font &font,
		const StartupResolvedText &inspectText) {
	const IndexedBitmap *textbox = resolveInspectTextboxBitmap(art, inspectText);
	if (!textbox || !textbox->isValid())
		return;

	blitBitmap(screen, *textbox, kIdentTextboxX, kIdentTextboxY);
	drawWrappedShadowedText(screen, font, inspectText.value,
		kIdentTextboxX + kIdentTextboxTextInsetX,
		kIdentTextboxY + kIdentTextboxTextInsetY,
		MAX<int>(0, (int)textbox->width - (kIdentTextboxTextInsetX + 2)),
		kIdentTextColor);
}

bool unlocksRoomObjectInteractionAfterInitialExamine(const StartupObjectRecord &object,
		Script &startupScript) {
	return object.operatable || startupScript.isPickupObject(object);
}

static int resolveRoomObjectCursorSequence(const StartupObjectRecord &object, Script &startupScript) {
	if (object.objectName.equalsIgnoreCase("EXIT_BM") || object.objectName.equalsIgnoreCase("EXIT_HS"))
		return kCursorSequenceTransition;

	StartupResolvedText inspectText;
	const bool pickupObject = startupScript.isPickupObject(object);
	if (!object.identShown && unlocksRoomObjectInteractionAfterInitialExamine(object, startupScript))
		return kCursorSequenceExamine;
	if (pickupObject)
		return kCursorSequencePickup;
	if (object.operatable)
		return kCursorSequenceOperate;
	if (startupScript.resolveObjectInspectText(object, inspectText))
		return kCursorSequenceExamine;

	if (startupScript.hasObjectInteraction(object))
		return kCursorSequenceExamine;

	return kCursorSequenceNeutral;
}

static Common::String buildRoomObjectPrompt(const StartupObjectRecord &object, Script &startupScript,
		int cursorSequence) {
	const Common::String label = startupScript.resolveObjectLabel(object);
	if (label.empty())
		return Common::String();

	if (cursorSequence == kCursorSequenceOperate)
		return Common::String::format("Operate the %s", label.c_str());
	if (cursorSequence == kCursorSequencePickup)
		return Common::String::format("Pick up the %s", label.c_str());
	if (cursorSequence == kCursorSequenceTalk)
		return Common::String::format("Talk to %s", label.c_str());
	if (cursorSequence == kCursorSequenceTransition)
		return Common::String::format("Go to %s", label.c_str());

	return Common::String::format("Examine %s", label.c_str());
}

static Common::String buildRoomNpcPrompt(const StartupNpcRecord &npc) {
	Common::String label = !npc.entityInitArg.empty() ? npc.entityInitArg : npc.npcName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	if (label.empty())
		return Common::String();
	return Common::String::format("Talk to %s", label.c_str());
}

bool doesPlayerFacingMatchRegion(int playerFacing, const StartupRegionRecord &region) {
	return region.desiredFacing < 0 || playerFacing == region.desiredFacing;
}

bool doesPlayerOverlapRegion(const RuntimeEntity &playerEntity, const StartupRegionRecord &region) {
	const Common::Rect regionBounds = getRegionBounds(region);
	if (regionBounds.isEmpty())
		return false;
	if (!playerEntity.getScreenRect().intersects(regionBounds))
		return false;

	const float playerMaxZ = playerEntity.getZ() + playerEntity.getZExtent();
	return playerMaxZ >= (float)region.minZ && (float)region.maxZ >= playerEntity.getZ();
}

StartupRoomHoverState resolveRoomHoverState(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupNpcRecord> &sceneNpcs,
		const Common::Array<StartupRegionRecord> &sceneRegions, const Common::Point &mousePos) {
	StartupRoomHoverState hoverState;
	if (const RuntimeEntity *playerEntity = findRoomPlayerAtPoint(engine, mousePos)) {
		hoverState.playerEntity = playerEntity;
		hoverState.cursorSequence = kCursorSequenceExamine;
		return hoverState;
	}
	if (const StartupNpcRecord *npc = findRoomNpcAtPoint(engine, sceneNpcs, mousePos)) {
		hoverState.npc = npc;
		hoverState.cursorSequence = kCursorSequenceTalk;
		hoverState.promptText = buildRoomNpcPrompt(*npc);
		return hoverState;
	}

	Script *startupScript = engine.getStartupScript();
	if (!startupScript)
		return hoverState;
	hoverState.object = findRoomObjectAtPoint(engine, sceneObjects, mousePos);
	hoverState.region = findRoomRegionAtPoint(engine, sceneRegions, mousePos);
	if (hoverState.object) {
		hoverState.cursorSequence = resolveRoomObjectCursorSequence(*hoverState.object, *startupScript);
		hoverState.promptText = buildRoomObjectPrompt(*hoverState.object, *startupScript, hoverState.cursorSequence);
		if (hoverState.cursorSequence != kCursorSequenceNeutral || !hoverState.promptText.empty())
			return hoverState;
	}
	if (hoverState.region && hoverState.region->cursorEnabled) {
		hoverState.object = nullptr;
		hoverState.cursorSequence = kCursorSequenceTransition;
		hoverState.promptText.clear();
		return hoverState;
	}

	if (roomSupportsMovementBand(state) &&
		mousePos.y >= state.roomMaxZScreenY &&
		mousePos.y <= state.roomMinZScreenY) {
		hoverState.cursorSequence = kCursorSequenceWalk;
	}

	return hoverState;
}

bool setPlayerIdleAnimation(StartupRoomPlayerState &playerState, int facing) {
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
		debugC(1, kDebugScene,
			"Harvester: player idle animation facing=%d frame=%d",
			facing, range.idleFrame);
	}
	return changed;
}

static bool setPlayerWalkAnimation(StartupRoomPlayerState &playerState, int facing) {
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
	debugC(1, kDebugScene,
		"Harvester: player walk animation facing=%d frames=%d..%d rate=%d",
		facing, range.walkFirstFrame, range.walkLastFrame, kRoomPlayerWalkAnimationRate);
	return true;
}

bool startPlayerTurnAnimation(StartupRoomPlayerState &playerState, int targetFacing) {
	if (!playerState.entity || playerState.facing < 0 || playerState.facing == targetFacing)
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
	debugC(1, kDebugScene,
		"Harvester: player turn animation from=%d to=%d frames=%d..%d backwards=%d rate=%d",
		playerState.facing, targetFacing, range.firstFrame, range.lastFrame,
		range.playBackwards, kRoomPlayerWalkAnimationRate);
	return true;
}

bool updatePlayerTurnAnimationState(StartupRoomPlayerState &playerState) {
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
	debugC(1, kDebugScene,
		"Harvester: player turn complete facing=%d frame=%d",
		playerState.facing, playerState.entity->getCurrentFrame());
	return true;
}

bool stepPlayerMoveTarget(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState) {
	if (!playerState.entity || !playerState.hasMoveTarget || playerState.turnActive)
		return false;

	if (playerState.centerX == playerState.targetX &&
			fabsf(playerState.z - playerState.targetZ) <= kRoomDepthCompareEpsilon) {
		playerState.hasMoveTarget = false;
		return setPlayerIdleAnimation(playerState, playerState.facing >= 0 ? playerState.facing : 0);
	}

	const int previousCenterX = playerState.centerX;
	const float previousZ = playerState.z;
	const int targetBottomY = mapRoomDepthToScreenY(state, playerState.targetZ);
	const int moveFacing = resolveFacingFromMovementDelta(
		playerState.targetX - playerState.centerX, targetBottomY - playerState.bottomY);
	if (!consumePlayerMovementTick(playerState))
		return false;
	const int horizontalStep = computeRoomPlayerHorizontalStep(state, playerState.z);
	const float depthStep = computeRoomPlayerDepthStep(state);
	const int candidateCenterX = stepTowardsInt(playerState.centerX, playerState.targetX, horizontalStep);
	const float candidateZ = stepTowardsFloat(playerState.z, playerState.targetZ, depthStep);

	bool moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
		playerState, candidateCenterX, candidateZ);
	if (!moved && candidateCenterX != previousCenterX) {
		moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, previousZ);
	}
	if (!moved && fabsf(candidateZ - previousZ) > kRoomDepthCompareEpsilon) {
		moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, previousCenterX, candidateZ);
	}
	if (!moved) {
		playerState.hasMoveTarget = false;
		return setPlayerIdleAnimation(playerState, playerState.facing >= 0 ? playerState.facing : moveFacing);
	}

	const int actualFacing = resolveFacingFromRoomMovement(
		state, previousCenterX, previousZ, playerState.centerX, playerState.z);
	(void)setPlayerWalkAnimation(playerState, actualFacing);
	if (playerState.centerX == playerState.targetX &&
			fabsf(playerState.z - playerState.targetZ) <= kRoomDepthCompareEpsilon) {
		playerState.bottomY = mapRoomDepthToScreenY(state, playerState.targetZ);
		playerState.z = playerState.targetZ;
		playerState.hasMoveTarget = false;
		(void)setPlayerIdleAnimation(playerState, actualFacing);
	}
	debugC(playerState.hasMoveTarget ? 2 : 1, kDebugScene,
		"Harvester: player move step room='%s' pos=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f) facing=%d frame=%d active=%d moved=%d",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, mapRoomDepthToScreenY(state, playerState.targetZ), (double)playerState.targetZ,
		playerState.facing, playerState.entity->getCurrentFrame(), playerState.hasMoveTarget, moved);
	return true;
}

bool stepPlayerKeyboardMovement(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupAnimRecord> &sceneAnimations,
		StartupRoomPlayerState &playerState, bool moveLeft, bool moveRight, bool moveUp, bool moveDown) {
	if (!playerState.entity || !roomSupportsMovementBand(state))
		return false;

	const int horizontalInput = (moveRight ? 1 : 0) - (moveLeft ? 1 : 0);
	const int verticalInput = (moveDown ? 1 : 0) - (moveUp ? 1 : 0);
	if (horizontalInput == 0 && verticalInput == 0)
		return false;
	if (playerState.turnActive)
		return false;

	const int previousCenterX = playerState.centerX;
	const float previousZ = playerState.z;
	const int horizontalStep = computeRoomPlayerHorizontalStep(state, playerState.z);
	const float depthStep = computeRoomPlayerDepthStep(state);
	int candidateCenterX = CLIP<int>(playerState.centerX + horizontalInput * horizontalStep, 0, 639);
	float candidateZ = playerState.z;
	if (verticalInput != 0) {
		const float positiveZ = clampRoomDepth(state, playerState.z + depthStep);
		const float negativeZ = clampRoomDepth(state, playerState.z - depthStep);
		const int positiveY = mapRoomDepthToScreenY(state, positiveZ);
		const int negativeY = mapRoomDepthToScreenY(state, negativeZ);
		if (verticalInput < 0) {
			if (positiveY < playerState.bottomY)
				candidateZ = positiveZ;
			else if (negativeY < playerState.bottomY)
				candidateZ = negativeZ;
		} else {
			if (positiveY > playerState.bottomY)
				candidateZ = positiveZ;
			else if (negativeY > playerState.bottomY)
				candidateZ = negativeZ;
		}
	}
	if (candidateCenterX == playerState.centerX &&
			fabsf(candidateZ - playerState.z) <= kRoomDepthCompareEpsilon) {
		return false;
	}

	const int desiredFacing = resolveFacingFromRoomMovement(
		state, playerState.centerX, playerState.z, candidateCenterX, candidateZ);
	if (desiredFacing != playerState.facing && startPlayerTurnAnimation(playerState, desiredFacing))
		return true;

	if (!consumePlayerMovementTick(playerState))
		return false;

	playerState.hasMoveTarget = false;
	bool moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
		playerState, candidateCenterX, candidateZ);
	if (!moved && candidateCenterX != previousCenterX) {
		moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, candidateCenterX, previousZ);
	}
	if (!moved && fabsf(candidateZ - previousZ) > kRoomDepthCompareEpsilon) {
		moved = tryApplyPlayerMovement(engine, state, sceneObjects, sceneAnimations,
			playerState, previousCenterX, candidateZ);
	}
	if (!moved)
		return false;

	const int actualFacing = resolveFacingFromRoomMovement(
		state, previousCenterX, previousZ, playerState.centerX, playerState.z);
	(void)setPlayerWalkAnimation(playerState, actualFacing);
	debugC(2, kDebugScene,
		"Harvester: player keyboard move room='%s' input=(%d,%d) pos=(%d,%d,z=%.2f) facing=%d frame=%d moved=%d",
		state.roomName.c_str(), horizontalInput, verticalInput,
		playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.facing, playerState.entity->getCurrentFrame(), moved);
	return true;
}

static void positionPlayerIdleAnimationEntity(const StartupRoomSetupState &state,
		const StartupRoomPlayerState &playerState, RuntimeEntity &idleEntity) {
	if (!playerState.entity)
		return;

	idleEntity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
	idleEntity.setDepthScale(computeActorDepthScale(state, playerState.z));
	idleEntity.setPosition(playerState.entity->getX(),
		playerState.entity->getY() + kRoomPlayerIdleYOffset, playerState.z);
}

static RuntimeEntity *ensurePlayerIdleAnimationEntity(HarvesterEngine &engine,
		const StartupRoomSetupState &state, const StartupRoomPlayerState &playerState,
		StartupRoomIdleAnimationState &idleState) {
	if (idleState.entity || !playerState.entity)
		return idleState.entity;

	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	idleState.entity = runtimeEntities->spawnSceneAnimationEntity(
		kPlayerIdleAnimationEntityName, kPlayerIdleAnimationResourcePath,
		Common::Point(playerState.entity->getX(),
			playerState.entity->getY() + kRoomPlayerIdleYOffset),
		playerState.z, 0, false, false, false, false, false);
	if (!idleState.entity)
		return nullptr;

	positionPlayerIdleAnimationEntity(state, playerState, *idleState.entity);
	idleState.entity->setVisible(false);
	debugC(1, kDebugScene,
		"Harvester: spawned player idle animation '%s' room='%s' frames=0..%d",
		kPlayerIdleAnimationResourcePath, state.roomName.c_str(), idleState.entity->getLastFrame());
	return idleState.entity;
}

static bool finishPlayerIdleAnimation(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
		StartupRoomIdleAnimationState &idleState) {
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
			(void)setPlayerIdleAnimation(playerState, idleState.restoreFacing);
	}

	idleState.active = false;
	idleState.loopStarted = false;
	idleState.exiting = false;
	idleState.restoreFacing = -1;
	notePlayerIdleReset(idleState);
	debugC(1, kDebugScene,
		"Harvester: player idle animation finished room='%s' facing=%d",
		state.roomName.c_str(), playerState.facing);
	return true;
}

bool requestPlayerIdleAnimationExit(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState) {
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
	debugC(1, kDebugScene,
		"Harvester: player idle animation exit room='%s' frames=%d..%d rate=%d",
		state.roomName.c_str(), exitStartFrame, exitLastFrame, kRoomPlayerIdleAnimationRate);
	return true;
}

bool startPlayerIdleAnimation(HarvesterEngine &engine, const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState) {
	if (!playerState.entity || idleState.active || isIdleAnimationExcludedRoom(state.roomName))
		return false;

	RuntimeEntity *idleEntity = ensurePlayerIdleAnimationEntity(engine, state, playerState, idleState);
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
	debugC(1, kDebugScene,
		"Harvester: player idle animation start room='%s' facing=%d frames=%d..%d rate=%d trigger_tick=%u",
		state.roomName.c_str(), idleState.restoreFacing, 0, loopLastFrame,
		kRoomPlayerIdleAnimationRate, idleState.triggerTick);
	return true;
}

bool updatePlayerIdleAnimation(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
		StartupRoomIdleAnimationState &idleState) {
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
		debugC(1, kDebugScene,
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

static bool findRoomObjectProbePoint(HarvesterEngine &engine, const Common::Array<StartupObjectRecord> &sceneObjects,
		const StartupObjectRecord &object, Common::Point &probePoint) {
	const Common::Rect bounds = getHotspotBounds(object);
	if (bounds.isEmpty())
		return false;

	const Common::Point center((bounds.left + bounds.right - 1) / 2, (bounds.top + bounds.bottom - 1) / 2);
	const Common::Point corners[] = {
		center,
		Common::Point(bounds.left, bounds.top),
		Common::Point(bounds.right - 1, bounds.top),
		Common::Point(bounds.left, bounds.bottom - 1),
		Common::Point(bounds.right - 1, bounds.bottom - 1)
	};
	for (const Common::Point &candidate : corners) {
		const StartupObjectRecord *hit = findRoomObjectAtPoint(engine, sceneObjects, candidate);
		if (hit && hit->objectName.equalsIgnoreCase(object.objectName)) {
			probePoint = candidate;
			return true;
		}
	}

	const int xStep = MAX<int>(1, bounds.width() / 6);
	const int yStep = MAX<int>(1, bounds.height() / 6);
	for (int y = bounds.top; y < bounds.bottom; y += yStep) {
		for (int x = bounds.left; x < bounds.right; x += xStep) {
			const Common::Point candidate(x, y);
			const StartupObjectRecord *hit = findRoomObjectAtPoint(engine, sceneObjects, candidate);
			if (hit && hit->objectName.equalsIgnoreCase(object.objectName)) {
				probePoint = candidate;
				return true;
			}
		}
	}

	return false;
}

void logStartupRoomProbe(HarvesterEngine &engine, const StartupRoomSceneResources &scene,
		const Common::String &entranceName, Common::Point &mousePos) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	Script *startupScript = engine.getStartupScript();
	if (!runtimeEntities || !startupScript)
		return;

	if (const RuntimeEntity *cursor = runtimeEntities->getCursorEntity()) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		if (cursor->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels)) {
			debugC(1, kDebugGeneral,
				"Harvester: startup probe cursor sequence=%d frame=%d pixels=%u transparent_pixels=%u preserved_pixels=%u",
				cursor->getAnimationSequence(), cursor->getCurrentFrame(), framePixels, transparentPixels, preservedPixels);
		}
	}

	if (const RuntimeEntity *player = runtimeEntities->findSceneEntityByName(kPlayerActorEntityName)) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		(void)player->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels);
		const Common::Rect playerRect = player->getScreenRect();
		const Common::Rect screenRect(0, 0, 640, 480);
		debugC(1, kDebugGeneral,
			"Harvester: startup probe player room='%s' entrance='%s' frame=%d rect=(%d,%d)-(%d,%d) visible=%d intersects_screen=%d opaque_pixels=%u",
			scene.state.roomName.c_str(), entranceName.c_str(), player->getCurrentFrame(),
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			player->isVisible(), playerRect.intersects(screenRect), framePixels - transparentPixels);
	}

	for (const StartupObjectRecord &object : scene.sceneObjects) {
		if (getHotspotBounds(object).isEmpty())
			continue;

		Common::Point probePoint;
		if (!findRoomObjectProbePoint(engine, scene.sceneObjects, object, probePoint))
			continue;

		const StartupObjectRecord *hoveredObject = findRoomObjectAtPoint(engine, scene.sceneObjects, probePoint);
		if (!hoveredObject)
			continue;

		mousePos = probePoint;
		const Common::String objectLabel = startupScript->resolveObjectLabel(*hoveredObject);
		StartupResolvedText inspectText;
		const StartupRoomHoverState hoverState = resolveRoomHoverState(
			engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions, probePoint);
		const bool hasInteraction = startupScript->hasObjectInteraction(*hoveredObject);
		const bool hasInspectText = startupScript->resolveObjectInspectText(*hoveredObject, inspectText);
		debugC(1, kDebugGeneral,
			"Harvester: startup probe hotspot room='%s' object='%s' point=(%d,%d) label='%s' prompt='%s' cursor_sequence=%d action_tag='%s' interaction=%d next_room='%s' inspect=%d",
			scene.state.roomName.c_str(), hoveredObject->objectName.c_str(), probePoint.x, probePoint.y,
			objectLabel.c_str(), hoverState.promptText.c_str(), hoverState.cursorSequence,
			hoveredObject->actionTag.c_str(), hasInteraction,
			"", hasInspectText);

		if (roomSupportsMovementBand(scene.state)) {
			Common::Point floorProbe;
			bool foundFloorProbe = false;
			for (int y = scene.state.roomMaxZScreenY; y <= scene.state.roomMinZScreenY && !foundFloorProbe; y += 12) {
				for (int x = 48; x < 592; x += 16) {
					const StartupRoomHoverState candidateHover = resolveRoomHoverState(
						engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
						Common::Point(x, y));
					if (candidateHover.cursorSequence == kCursorSequenceWalk) {
						floorProbe = Common::Point(x, y);
						foundFloorProbe = true;
						break;
					}
				}
			}

			const StartupRoomHoverState floorHover = foundFloorProbe
				? resolveRoomHoverState(engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, floorProbe)
				: StartupRoomHoverState();
			debugC(1, kDebugGeneral,
				"Harvester: startup probe floor room='%s' point=(%d,%d) found=%d cursor_sequence=%d prompt='%s'",
				scene.state.roomName.c_str(), floorProbe.x, floorProbe.y, foundFloorProbe,
				floorHover.cursorSequence, floorHover.promptText.c_str());

			RuntimeEntity *player = runtimeEntities->findSceneEntityByName(kPlayerActorEntityName);
			if (player && foundFloorProbe && floorHover.cursorSequence == kCursorSequenceWalk) {
				StartupRoomPlayerState probePlayer;
				probePlayer.entity = player;
				probePlayer.centerX = scene.state.playerSpawnX;
				probePlayer.bottomY = scene.state.playerSpawnY;
				probePlayer.z = (float)scene.state.playerSpawnZ;
				probePlayer.facing = scene.state.playerFacing;
				setPlayerMoveTargetFromScreenPoint(scene.state, probePlayer, floorProbe.x, floorProbe.y);
				for (int i = 0; i < 32 && probePlayer.hasMoveTarget; ++i)
					(void)stepPlayerMoveTarget(engine, scene.state, scene.sceneObjects, scene.sceneAnimations, probePlayer);

				const Common::Rect movedRect = player->getScreenRect();
				debugC(1, kDebugGeneral,
					"Harvester: startup probe movement room='%s' target=(%d,%d) final_rect=(%d,%d)-(%d,%d) z=%.2f",
					scene.state.roomName.c_str(), floorProbe.x, floorProbe.y,
					movedRect.left, movedRect.top, movedRect.right, movedRect.bottom, (double)probePlayer.z);

				player->setCurrentFrame(resolvePlayerFacingFrame(scene.state.playerFacing));
				(void)applyRoomActorPlacement(scene.state, *player,
					scene.state.playerSpawnX, scene.state.playerSpawnY, (float)scene.state.playerSpawnZ);
			}
		}
		return;
	}

	debugC(1, kDebugGeneral, "Harvester: startup probe found no interactive hotspot in room '%s'",
		scene.state.roomName.c_str());
}

static Common::String trimAsciiLine(const Common::String &value) {
	uint start = 0;
	uint end = value.size();

	while (start < end && (value[start] == ' ' || value[start] == '\t'))
		++start;
	while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r'))
		--end;

	return value.substr(start, end - start);
}

static bool loadQuickTipsScene(HarvesterEngine &engine, StartupRoomSceneResources &scene) {
	StartupRoomSetupState state;
	// The original startup path enters the START room first and overlays quick tips on top of it.
	if (!engine.getStartupScript()->resolveRoomSetupState("START", state, *engine.getResources()))
		return false;

	return loadRoomSceneResources(state, *engine.getResources(), scene);
}

static void renderQuickTipsScreen(HarvesterEngine &engine, const StartupRoomSceneResources &scene,
		const Common::Point &mousePos, const Common::String &tipText) {
	Graphics::Screen *screen = engine.getScreen();
	const Art *art = engine.getStartupArt();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	Script *startupScript = engine.getStartupScript();
	if (!screen || !art || !font || !startupScript)
		return;

	drawRoomScene(engine, *screen, scene, scene.targetPaletteBrightness);
	blitBitmap(*screen, art->getTipsBitmap(), kQuickTipsOverlayX, kQuickTipsOverlayY);

	drawWrappedShadowedText(*screen, *font, tipText, kQuickTipTextX, kQuickTipTextY, kQuickTipTextWidth, kTextColorNormal);
	const Common::Rect exitRect = quickTipsExitRect();
	const Common::Rect nextRect = quickTipsNextRect();
	const Common::Rect toggleRect = quickTipsToggleRect();
	const Common::String toggleLabel = startupScript->resolveTextValue(
		startupScript->isQuickTipsEnabled() ? "Show_Tips_ON" : "Show_Tips_OFF");
	drawShadowedString(*screen, *font, "Exit", exitRect.left, exitRect.top, exitRect.width(),
		kQuickTipActionColor);
	drawShadowedString(*screen, *font, "Next", nextRect.left, nextRect.top, nextRect.width(),
		kQuickTipActionColor);
	drawShadowedString(*screen, *font, toggleLabel,
		toggleRect.left, toggleRect.top, toggleRect.width(), kQuickTipActionColor);

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

Flow::Flow(HarvesterEngine &engine)
	: _engine(engine), _mousePos(320, 200), _dialogue(engine, _mousePos), _inventory(engine),
	  _menu(engine, _mousePos, _menuItems), _room(engine, _mousePos, _inventory) {
}

bool Flow::load() {
	return loadQuickTips() && loadMenuItems();
}

Common::Error Flow::run() {
	if (!ensureCursorEntity())
		return Common::kReadingFailed;

	clearPendingMainMenuReturn();
	clearPendingNewGameRestart();
	resetRoomNpcDialogueState();
	Common::Error error = Common::kNoError;
	if (_engine.hasPendingLoadedStartupSaveRoomState()) {
		Common::String initialTarget = _engine.getPendingLoadedStartupSaveRoomState().entranceName;
		if (initialTarget.empty())
			initialTarget = _engine.getPendingLoadedStartupSaveRoomState().roomName;
		error = runRoomLoop(initialTarget);
	} else {
		_engine.clearCurrentStartupSaveRoomState();
		_engine.getStartupScript()->resetRuntimeState();
		error = runQuickTips();
		if (error.getCode() != Common::kNoError)
			return error;

		clearPendingMainMenuReturn();
		clearPendingNewGameRestart();
		error = runMainMenuStub();
		return error;
	}
	if (error.getCode() != Common::kNoError)
		return error;
	if (!takePendingMainMenuReturn())
		return Common::kNoError;

	_engine.stopStartupMusic();
	_engine.stopStartupSound();
	return runMainMenuStub();
}

bool Flow::loadQuickTips() {
	_quickTips.clear();

	Common::Array<byte> data;
	if (!_engine.getResources()->loadFile(kQuickTipsPath, data)) {
		warning("Harvester: unable to load quick tips '%s'", kQuickTipsPath);
		return true;
	}

	Common::String currentLine;
	for (uint i = 0; i < data.size(); ++i) {
		const char ch = (char)data[i];
		if (ch == '\r')
			continue;
		if (ch == '\n') {
			const Common::String trimmed = trimAsciiLine(currentLine);
			if (!trimmed.empty())
				_quickTips.push_back(trimmed);
			currentLine.clear();
			continue;
		}

		currentLine += ch;
	}

	const Common::String trimmed = trimAsciiLine(currentLine);
	if (!trimmed.empty())
		_quickTips.push_back(trimmed);

	debugC(1, kDebugGeneral, "Harvester: loaded %u quick tips from '%s'", (uint)_quickTips.size(), kQuickTipsPath);
	return true;
}

bool Flow::loadMenuItems() {
	_menuItems.clear();

	Common::Array<byte> data;
	if (!_engine.getResources()->loadFile(kMenuPath, data)) {
		warning("Harvester: unable to load startup menu '%s'", kMenuPath);
		return true;
	}

	Common::MemoryReadStream stream(data.data(), data.size());
	Common::INIFile menu;
	menu.setDefaultSectionName(kMenuSectionName);
	menu.requireKeyValueDelimiter();
	menu.suppressValuelessLineWarning();
	if (!menu.loadFromStream(stream)) {
		warning("Harvester: unable to parse startup menu '%s'", kMenuPath);
		return true;
	}

	for (uint i = 1; i <= 6; ++i) {
		Common::String key = Common::String::format("main_menu_%u", i);
		Common::String value;
		if (menu.getKey(key, kMenuSectionName, value) && !value.empty())
			_menuItems.push_back(value);
	}

	debugC(1, kDebugGeneral, "Harvester: loaded %u startup menu items from '%s'", (uint)_menuItems.size(), kMenuPath);
	return true;
}

Common::Error Flow::runQuickTips() {
	if (!_engine.getStartupScript()->isQuickTipsEnabled() || _quickTips.empty())
		return Common::kNoError;

	Common::Error transitionError = beginRoomSetupTransition();
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	StartupRoomSceneResources scene;
	if (!loadQuickTipsScene(_engine, scene))
		return Common::kReadingFailed;

	Graphics::Screen *screen = _engine.getScreen();
	if (screen) {
		if (!populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return Common::kReadingFailed;

		logScenePaletteSummary("quick tips scene palette", scene, kPaletteBrightnessBlack);
		drawRoomScene(_engine, *screen, scene, kPaletteBrightnessBlack);
		screen->makeAllDirty();
		screen->update();

		logScenePaletteSummary("quick tips fade target", scene, scene.targetPaletteBrightness);
		transitionError = fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;
	}

	resetCursorAnimationSequence();

	debugC(1, kDebugGeneral,
		"Harvester: quick tips labels exit='Exit' next='Next' toggle='%s'",
		_engine.getStartupScript()->resolveTextValue(
			_engine.getStartupScript()->isQuickTipsEnabled() ? "Show_Tips_ON" : "Show_Tips_OFF").c_str());

	uint tipIndex = _engine.getRandomNumber(_quickTips.size() - 1);
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderQuickTipsScreen(_engine, scene, _mousePos, _quickTips[tipIndex]);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (quickTipsExitRect().contains(_mousePos))
					return Common::kNoError;

				if (quickTipsNextRect().contains(_mousePos)) {
					tipIndex = (tipIndex + 1) % _quickTips.size();
					needsRedraw = true;
				} else if (quickTipsToggleRect().contains(_mousePos)) {
					_engine.getStartupScript()->setQuickTipsEnabled(!_engine.getStartupScript()->isQuickTipsEnabled());
					needsRedraw = true;
				}
				break;
			default:
				break;
			}
		}

		if (tickRuntimeEntities())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error Flow::runMainMenuStub() {
	return _menu.runMainMenuStub(*this);
}

Common::Error Flow::runRoomMenuStub(const IndexedBitmap &backdrop, const byte *palette, float paletteBrightness) {
	return _menu.runRoomMenuStub(backdrop, palette, paletteBrightness, *this);
}

Common::Error Flow::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const StartupNpcRecord &npc, const Common::String &usedItemName) {
	_queuedDialogueInteraction = StartupInteractionResult();
	_hasQueuedDialogueInteraction = false;
	return _dialogue.runRoomNpcDialogue(backdrop, palette, paletteBrightness, npc, usedItemName, *this);
}

Common::Error Flow::runTownMapSelector(const Common::String &mapEntryName,
		Common::String &destinationEntranceName) {
	destinationEntranceName.clear();

	Script *startupScript = _engine.getStartupScript();
	ResourceManager *resources = _engine.getResources();
	Graphics::Screen *screen = _engine.getScreen();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!startupScript || !resources || !screen || !font)
		return Common::kReadingFailed;

	const StartupMapEntranceRecord *mapEntrance = startupScript->findMapEntranceRecord(mapEntryName);
	if (!mapEntrance) {
		warning("Harvester: unresolved town map entry '%s'", mapEntryName.c_str());
		return Common::kReadingFailed;
	}

	byte palette[256 * 3];
	if (!loadPaletteResource(*resources, kTownMapPalettePath, palette))
		return Common::kReadingFailed;

	Common::Array<IndexedBitmap> panels;
	panels.resize(ARRAYSIZE(kTownMapBitmapPaths));
	for (uint i = 0; i < ARRAYSIZE(kTownMapBitmapPaths); ++i) {
		if (!loadBitmapResource(*resources, kTownMapBitmapPaths[i], panels[i]))
			return Common::kReadingFailed;
	}

	const Common::String previousMusicPath = _engine.getStartupMusicPath();
	(void)_engine.playStartupMusic(kTownMapMusicPath);

	if (!ensureCursorEntity())
		return Common::kReadingFailed;
	resetCursorAnimationSequence();

	if (_mousePos.x < 0 || _mousePos.y < 0 || _mousePos.x >= screen->w || _mousePos.y >= screen->h)
		_mousePos = Common::Point(screen->w / 2, screen->h / 2);
	if (RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities())
		(void)runtimeEntities->syncCursorEntityPosition(_mousePos);

	int currentPanel = clampTownMapPanelIndex(mapEntrance->initialPanelIndex);
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	auto centerCursor = [&]() {
		_mousePos = Common::Point(screen->w / 2, screen->h / 2);
		if (RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities())
			(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
	};
	auto restorePreviousMusic = [&]() {
		if (!destinationEntranceName.empty())
			return;
		if (!previousMusicPath.empty())
			(void)_engine.playStartupMusic(previousMusicPath);
		else
			_engine.stopStartupMusic();
	};

	while (!_engine.shouldQuit()) {
		const int edgePanel = resolveTownMapEdgePanel(currentPanel, _mousePos, screen->w, screen->h);
		if (edgePanel != currentPanel) {
			currentPanel = edgePanel;
			centerCursor();
			needsRedraw = true;
		}

		const StartupMapLocationRecord *hoveredLocation =
			findTownMapLocationAt(startupScript->getMapLocations(), currentPanel, _mousePos);
		if (needsRedraw) {
			setScaledPalette(*screen, palette, 1.0f);
			screen->fillRect(screen->getBounds(), 0);
			blitBitmap(*screen, panels[(uint)currentPanel], 0, 0);
			if (hoveredLocation) {
				drawShadowedString(*screen, *font, hoveredLocation->labelText,
					hoveredLocation->labelX, hoveredLocation->labelY, screen->w, kTownMapLabelColor);
			}
			if (RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities())
				runtimeEntities->drawCursor(*screen);
			screen->makeAllDirty();
			screen->update();
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result)) {
				restorePreviousMusic();
				return result;
			}

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN: {
				const StartupMapLocationRecord *clickedLocation =
					findTownMapLocationAt(startupScript->getMapLocations(), currentPanel, _mousePos);
				if (!clickedLocation)
					break;

				const StartupEntranceRecord *destinationEntrance =
					startupScript->findEntranceRecord(clickedLocation->destinationEntranceName);
				if (!destinationEntrance) {
					warning("Harvester: unresolved town map destination '%s' from '%s'",
						clickedLocation->destinationEntranceName.c_str(), mapEntryName.c_str());
					break;
				}

				destinationEntranceName = destinationEntrance->entranceName;
				debugC(1, kDebugGeneral,
					"Harvester: town map selection entry='%s' panel=%d label='%s' destination='%s'",
					mapEntryName.c_str(), currentPanel, clickedLocation->labelText.c_str(),
					destinationEntranceName.c_str());
				return Common::kNoError;
			}
			case Common::EVENT_KEYDOWN: {
				const int nextPanel = resolveTownMapKeyPanel(currentPanel, event.kbd.keycode);
				if (nextPanel != currentPanel) {
					currentPanel = nextPanel;
					centerCursor();
					needsRedraw = true;
				}
				break;
			}
			default:
				break;
			}
		}

		if (tickRuntimeEntities())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	restorePreviousMusic();
	return Common::kNoError;
}

Common::Error Flow::resolveRoomTransitionTarget(const Common::String &targetName,
		Common::String &resolvedTargetName) {
	resolvedTargetName = targetName;

	Script *startupScript = _engine.getStartupScript();
	if (!startupScript || targetName.empty())
		return Common::kNoError;

	if (startupScript->findMapEntranceRecord(targetName))
		return runTownMapSelector(targetName, resolvedTargetName);

	return Common::kNoError;
}

void Flow::queueDialogueInteraction(const StartupInteractionResult &interaction) {
	_queuedDialogueInteraction = interaction;
	_hasQueuedDialogueInteraction = true;
}

bool Flow::takeQueuedDialogueInteraction(StartupInteractionResult &interaction) {
	if (!_hasQueuedDialogueInteraction)
		return false;

	interaction = _queuedDialogueInteraction;
	_queuedDialogueInteraction = StartupInteractionResult();
	_hasQueuedDialogueInteraction = false;
	return true;
}

void Flow::prepareForNewGame() {
	clearPendingMainMenuReturn();
	clearPendingNewGameRestart();
	_engine.clearPendingLoadedStartupSaveRoomState();
	_engine.clearCurrentStartupSaveRoomState();
	if (_engine.getStartupScript())
		_engine.getStartupScript()->resetRuntimeState();
	resetRoomNpcDialogueState();
}

void Flow::requestNewGameRestart() {
	_pendingNewGameRestart = true;
}

bool Flow::hasPendingNewGameRestart() const {
	return _pendingNewGameRestart;
}

bool Flow::takePendingNewGameRestart() {
	const bool requested = _pendingNewGameRestart;
	_pendingNewGameRestart = false;
	return requested;
}

void Flow::clearPendingNewGameRestart() {
	_pendingNewGameRestart = false;
}

void Flow::requestMainMenuReturn() {
	_pendingMainMenuReturn = true;
	_engine.clearCurrentStartupSaveRoomState();
}

bool Flow::hasPendingMainMenuReturn() const {
	return _pendingMainMenuReturn;
}

bool Flow::takePendingMainMenuReturn() {
	const bool requested = _pendingMainMenuReturn;
	_pendingMainMenuReturn = false;
	return requested;
}

void Flow::clearPendingMainMenuReturn() {
	_pendingMainMenuReturn = false;
}

Common::Error Flow::runRoomLoop(const Common::String &entranceName) {
	return _room.runRoomLoop(*this, entranceName);
}

bool Flow::ensureCursorEntity() {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;
	if (runtimeEntities->getCursorEntity())
		return true;

	return runtimeEntities->spawnCursorEntity(_mousePos) != nullptr;
}

bool Flow::populateRoomSceneEntities(const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &drawableObjects,
		const Common::Array<StartupAnimRecord> &drawableAnimations) {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;

	RuntimeEntity *preservedPlayer = runtimeEntities->detachSceneEntityByName(kPlayerActorEntityName);
	runtimeEntities->clearSceneEntities();
	for (const StartupRegionRecord &region : state.roomRegions) {
		const Common::Rect regionBounds = getRegionBounds(region);
		if (regionBounds.isEmpty())
			continue;

		RuntimeEntity *entity = runtimeEntities->spawnSceneHotspotEntity(
			region.regionName, regionBounds, (float)region.minZ);
		if (!entity) {
			debugC(1, kDebugScene,
				"Harvester: scene region skipped room='%s' region='%s' bounds=(%d,%d)-(%d,%d) z=[%d,%d]",
				state.roomName.c_str(), region.regionName.c_str(),
				regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
				region.minZ, region.maxZ);
			continue;
		}

		entity->setClassId(kRuntimeEntityClassRectHotspot19);
		entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
		entity->setZExtent((float)MAX(0, region.maxZ - region.minZ));
		if (!region.cursorEnabled)
			entity->setHitTestMode(kRuntimeEntityHitTestNone);
		debugC(1, kDebugScene,
			"Harvester: scene region spawned room='%s' region='%s' class=0x%x bounds=(%d,%d)-(%d,%d) z=[%d,%d] facing=%d cursor=%d action='%s'",
			state.roomName.c_str(), region.regionName.c_str(), entity->getClassId(),
			regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
			region.minZ, region.maxZ, region.desiredFacing, region.cursorEnabled, region.actionTag.c_str());
	}
	for (const StartupObjectRecord &object : drawableObjects) {
		RuntimeEntity *entity = nullptr;
		const Common::String spritePath = resolveSceneObjectSpritePath(object);
		const Common::Rect hotspotBounds = getHotspotBounds(object);
		if (!spritePath.empty() && spritePath.hasSuffixIgnoreCase(".BM")) {
			entity = runtimeEntities->spawnSceneBitmapEntity(object.objectName, spritePath,
				Common::Point(object.currentX, object.currentY), (float)object.currentZ);
		} else {
			if (!hotspotBounds.isEmpty())
				entity = runtimeEntities->spawnSceneHotspotEntity(object.objectName, hotspotBounds, (float)object.currentZ);
		}

		if (!entity) {
			debugC(1, kDebugScene,
				"Harvester: scene entity skipped room='%s' object='%s' resolved='%s' bounds=(%d,%d)-(%d,%d) reason='%s'",
				state.roomName.c_str(), object.objectName.c_str(), spritePath.c_str(),
				hotspotBounds.left, hotspotBounds.top, hotspotBounds.right, hotspotBounds.bottom,
				spritePath.empty() || !spritePath.hasSuffixIgnoreCase(".BM")
					? "no_bitmap_and_no_hotspot_bounds"
					: "spawn_failed");
			debug(1, "Harvester: unable to spawn room object entity '%s' from '%s'",
				object.objectName.c_str(), spritePath.c_str());
			continue;
		}

		entity->setClassId(resolveSceneObjectClass(object, entity->hasFrames() ? entity : nullptr, _engine.getStartupScript()));
		entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
		entity->setZExtent((float)object.zExtent);
		const Common::Rect entityRect = entity->getScreenRect();
		debugC(1, kDebugScene,
			"Harvester: scene entity spawned room='%s' object='%s' type='%s' class=0x%x pos=(%d,%d,z=%.2f) rect=(%d,%d)-(%d,%d) sprite='%s' action='%s' operatable=%d ident='%s'",
			state.roomName.c_str(), object.objectName.c_str(),
			entity->hasFrames() ? "bitmap" : "hotspot", entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(),
			entityRect.left, entityRect.top, entityRect.right, entityRect.bottom,
			spritePath.c_str(), object.actionTag.c_str(), object.operatable, object.identTextKey.c_str());
	}
	for (const StartupAnimRecord &anim : drawableAnimations) {
		if (!runtimeEntities->spawnSceneAnimationEntity(anim.animName, anim.resourcePath,
				Common::Point(anim.x, anim.y), (float)anim.z, anim.frameDelay, anim.active, anim.visible,
				anim.looping, anim.backward, anim.pingPong)) {
			debug(1, "Harvester: unable to spawn room anim entity '%s' from '%s'",
				anim.animName.c_str(), anim.resourcePath.c_str());
		}
	}
	for (const StartupNpcRecord &npc : state.roomNpcs) {
		RuntimeEntity *entity = runtimeEntities->spawnSceneActorEntity(npc.npcName,
			npc.modelPath, Common::Point(npc.posX, npc.posY), (float)npc.posZ, 0);
		if (!entity) {
			debug(1, "Harvester: unable to spawn room npc entity '%s' from '%s'",
				npc.npcName.c_str(), npc.modelPath.c_str());
			continue;
		}

		entity->setClassId(kRuntimeEntityClassNpc);
		entity->setHitTestMode(kRuntimeEntityHitTestOpaquePixels);
		// Native spawn_npc_entity_from_record seeds the passive room-NPC loop as frames 0..0x3b.
		entity->setAnimationFrameRange(0, MIN(entity->getLastFrame(), kRoomNpcAmbientLastFrame), true);
		if (npc.frameDelay > 0)
			entity->setAnimationRate(npc.frameDelay);
		// Native room NPCs come from spawn_abm_entity_base, which leaves the depth-scale flag cleared.
		if (!applyRoomActorPlacementInternal(state, *entity,
				npc.posX, npc.posY, (float)npc.posZ, nullptr, false)) {
			debug(1, "Harvester: unable to apply room npc placement for '%s'",
				npc.npcName.c_str());
		}
		debugC(1, kDebugScene,
			"Harvester: scene npc spawned room='%s' npc='%s' class=0x%x pos=(%d,%d,z=%.2f) frame_delay=%d model='%s' active=%d visible=%d",
			state.roomName.c_str(), npc.npcName.c_str(), entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(),
			npc.frameDelay, npc.modelPath.c_str(), npc.active, npc.visible);
	}
	for (const StartupMonsterRecord &monster : state.roomMonsters) {
		RuntimeEntity *entity = runtimeEntities->spawnSceneActorEntity(monster.monsterName,
			monster.modelPath, Common::Point(monster.posX, monster.posY), (float)monster.posZ,
			resolveMonsterFacingFrame(monster.initialFacing));
		if (!entity) {
			debug(1, "Harvester: unable to spawn room monster entity '%s' from '%s'",
				monster.monsterName.c_str(), monster.modelPath.c_str());
			continue;
		}

		entity->setClassId(kRuntimeEntityClassMonster);
		entity->setHitTestMode(kRuntimeEntityHitTestNone);
		entity->setVisible(monster.visible);
		if (!applyRoomActorPlacement(state, *entity, monster.posX, monster.posY, (float)monster.posZ)) {
			debug(1, "Harvester: unable to apply room monster placement for '%s'",
				monster.monsterName.c_str());
		}
		debugC(1, kDebugScene,
			"Harvester: scene monster spawned room='%s' monster='%s' class=0x%x pos=(%d,%d,z=%.2f) facing=%d model='%s' active=%d visible=%d",
			state.roomName.c_str(), monster.monsterName.c_str(), entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(), monster.initialFacing,
			monster.modelPath.c_str(), monster.active, monster.visible);
	}
	if (state.hasEntrance) {
		const int playerFrame = resolvePlayerFacingFrame(state.playerFacing);
		const bool reusedPlayer = preservedPlayer != nullptr;
		RuntimeEntity *player = preservedPlayer;
		if (!reusedPlayer) {
			player = runtimeEntities->spawnSceneActorEntity(kPlayerActorEntityName,
				kPlayerActorResourcePath, Common::Point(state.playerSpawnX, state.playerSpawnY),
				(float)state.playerSpawnZ, playerFrame);
		}
		if (!player) {
			debug(1, "Harvester: unable to spawn startup player actor from '%s'", kPlayerActorResourcePath);
		} else {
			player->setClassId(kRuntimeEntityClassPlayer);
			player->setAnchorMode(kRuntimeEntityAnchorTopLeft);
			player->setHitTestMode(kRuntimeEntityHitTestOpaquePixels);
			player->setVisible(true);
			player->setAnimationRate(0);
			player->setAnimationFrameRange(playerFrame, playerFrame, false);
			player->setCurrentFrame(playerFrame);
			if (!applyStartupActorPlacement(state, *player)) {
				debug(1, "Harvester: unable to apply startup player placement for entrance '%s'",
					state.entranceName.c_str());
			}
			if (reusedPlayer)
				runtimeEntities->adoptSceneEntity(player);
			debugC(1, kDebugGeneral,
				"Harvester: %s startup player actor '%s' entrance='%s' pos=(%d,%d,%d) facing=%d frame=%d",
				reusedPlayer ? "reused" : "spawned", kPlayerActorResourcePath, state.entranceName.c_str(),
				state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ,
				state.playerFacing, playerFrame);
		}
		preservedPlayer = nullptr;
	} else if (preservedPlayer) {
		delete preservedPlayer;
	}

	return true;
}

Common::Error Flow::beginRoomSetupTransition() {
	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->hideCursor();

	const Art *art = _engine.getStartupArt();
	Graphics::Screen *screen = _engine.getScreen();
	if (art && screen)
		art->drawWaitFrame(*screen);

	Common::Error result = Common::kNoError;
	if (pumpTransitionEvents(result))
		return result;

	return Common::kNoError;
}

Common::Error Flow::fadeInRoomScene(const byte *palette, float targetBrightness) {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen || !palette)
		return Common::kNoError;

	for (float brightness = kPaletteFadeStep; brightness < targetBrightness; brightness += kPaletteFadeStep) {
		setScaledPalette(*screen, palette, brightness);
		screen->makeAllDirty();
		screen->update();

		const uint32 nextTick = g_system->getMillis() + kPaletteFadeTickMs;
		while ((int32)(nextTick - g_system->getMillis()) > 0) {
			Common::Error result = Common::kNoError;
			if (pumpTransitionEvents(result))
				return result;
			g_system->delayMillis(1);
		}
	}

	setScaledPalette(*screen, palette, targetBrightness);
	screen->makeAllDirty();
	screen->update();
	return Common::kNoError;
}

bool Flow::pumpTransitionEvents(Common::Error &result) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (handleSystemEvent(event, result))
			return true;
	}

	return false;
}

void Flow::executeStartupAudioCommands(const Common::Array<StartupAudioCommand> &commands) {
	for (const StartupAudioCommand &command : commands)
		(void)_engine.executeStartupAudioCommand(command);
}

void Flow::resetRoomNpcDialogueState() {
	_dialogue.resetRoomNpcDialogueState();
}

void Flow::resetCursorAnimationSequence() {
	if (!ensureCursorEntity())
		return;

	RuntimeEntity *cursor = _engine.getRuntimeEntities()->getCursorEntity();
	if (!cursor)
		return;

	cursor->setVisible(true);
	cursor->setAnimationSequence(kCursorSequence7);
	(void)_engine.getRuntimeEntities()->syncCursorEntityPosition(_mousePos);
}

bool Flow::tickRuntimeEntities() {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;
	return runtimeEntities->tickSceneEntities() || runtimeEntities->syncCursorEntityPosition(_mousePos);
}

bool Flow::handleSystemEvent(const Common::Event &event, Common::Error &result) {
	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		result = Common::kNoError;
		return true;
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		_mousePos = event.mouse;
		break;
	default:
		break;
	}

	return false;
}

} // End of namespace Harvester
