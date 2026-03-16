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

#include "harvester/startup_flow.h"

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
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/startup_art.h"
#include "harvester/startup_script.h"
#include "harvester/startup_text.h"

namespace Harvester {

namespace {

static const char *const kQuickTipsPath = "ADJHEAD.RCS";
static const char *const kMenuPath = "MENU.INI";
static const char *const kMenuSectionName = "menu";

static const int kInventoryX = 64;
static const int kInventoryY = 48;
static const int kInventoryItemStartX = 73;
static const int kInventoryItemStartY = 115;
static const int kInventoryItemMaxRight = 564;
static const int kInventoryItemSpacing = 5;
static const int kLogoX = 160;
static const int kLogoY = 0;
static const int kQuickTipsOverlayX = 167;
static const int kQuickTipsOverlayY = 200;

static const int kQuickTipTextX = 180;
static const int kQuickTipTextY = 228;
static const int kQuickTipTextWidth = 280;

static const int kMenuStartY = 100;
static const int kMenuLineSpacing = 28;
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

static const byte kIdentTextColor = 0xd3;
static const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;
static const byte kTextColorDim = 248;
static const byte kShadowColor = 0;
static const byte kPanelFillColor = 1;
static const byte kQuickTipActionColor = 0xc3;
static const byte kRoomPromptColor = 0xce;
static const byte kTransparentPaletteIndex = 0;

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

struct StartupRoomSceneResources {
	StartupRoomSetupState state;
	byte palette[256 * 3] = { 0 };
	Common::Array<StartupObjectRecord> sceneObjects;
	Common::Array<StartupAnimRecord> sceneAnimations;
	Common::Array<StartupRegionRecord> sceneRegions;
	float targetPaletteBrightness = 1.0f;
};

struct StartupRoomPlayerState {
	RuntimeEntity *entity = nullptr;
	int centerX = 0;
	int bottomY = 0;
	float z = 0.0f;
	int facing = -1;
	bool hasMoveTarget = false;
	int targetX = 0;
	float targetZ = 0.0f;
	bool turnActive = false;
	int turnTargetFacing = -1;
	int turnFirstFrame = -1;
	int turnLastFrame = -1;
	int turnEndFrame = -1;
	bool turnPlayBackwards = false;
};

struct StartupRoomHoverState {
	const StartupObjectRecord *object = nullptr;
	const StartupRegionRecord *region = nullptr;
	const RuntimeEntity *playerEntity = nullptr;
	Common::String promptText;
	int cursorSequence = kCursorSequenceNeutral;
};

struct StartupInventoryVisual {
	StartupObjectRecord object;
	IndexedBitmap bitmap;
	Common::Rect bounds;
	bool hasBitmap = false;
};

struct StartupInventoryOverlayState {
	Common::Array<StartupInventoryVisual> items;
	bool open = false;
	Common::String selectedItemName;
	Common::String promptText;
};

struct StartupRoomIdleAnimationState {
	RuntimeEntity *entity = nullptr;
	int restoreFacing = -1;
	uint32 activityTick = 0;
	uint32 resetTick = 0;
	uint32 triggerTick = 0;
	bool active = false;
	bool loopStarted = false;
	bool exiting = false;
};

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

static int roundToInt(float value) {
	return value >= 0.0f ? (int)floorf(value + 0.5f) : (int)ceilf(value - 0.5f);
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

static bool applyRoomActorPlacement(const StartupRoomSetupState &state, RuntimeEntity &entity,
		int centerX, int bottomY, float z, const Common::String *entranceName = nullptr) {
	int width = 0;
	int height = 0;
	int xOffset = 0;
	int yOffset = 0;
	if (!entity.getCurrentFrameMetrics(width, height, xOffset, yOffset))
		return false;

	entity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
	setRoomActorScreenPosition(entity, centerX, bottomY, z, width, height, xOffset, yOffset);

	const float depthScale = computeActorDepthScale(state, z);
	entity.setDepthScale(depthScale);
	if (!entity.getCurrentFrameMetrics(width, height, xOffset, yOffset))
		return false;
	setRoomActorScreenPosition(entity, centerX, bottomY, z, width, height, xOffset, yOffset);

	if (entranceName) {
		debugC(1, kDebugGeneral,
			"Harvester: startup player placement entrance='%s' spawn=(%d,%d,%d) screen_base=(%d,%d) size=%dx%d offsets=(%d,%d) scale=%.3f",
			entranceName->c_str(), centerX, bottomY, roundToInt(z),
			entity.getX(), entity.getY(), width, height, xOffset, yOffset, (double)depthScale);
	}
	return true;
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

static void setPlayerMoveTarget(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
		int targetX, float targetZ) {
	playerState.hasMoveTarget = true;
	playerState.targetX = CLIP<int>(targetX, 0, 639);
	playerState.targetZ = clampRoomDepth(state, targetZ);
	debugC(1, kDebugScene,
		"Harvester: player move target room='%s' current=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f)",
		state.roomName.c_str(), playerState.centerX, playerState.bottomY, (double)playerState.z,
		playerState.targetX, mapRoomDepthToScreenY(state, playerState.targetZ), (double)playerState.targetZ);
}

static void setPlayerMoveTargetFromScreenPoint(const StartupRoomSetupState &state,
		StartupRoomPlayerState &playerState, int targetX, int targetBottomY) {
	setPlayerMoveTarget(state, playerState, targetX,
		mapRoomScreenYToDepth(state, clampRoomMovementY(state, targetBottomY)));
}

static int resolveRegionTargetX(const StartupRegionRecord &region, const StartupRoomPlayerState &playerState) {
	const Common::Rect bounds(region.left, region.top, region.right + 1, region.bottom + 1);
	int targetX = bounds.left + bounds.width() / 2;
	if (playerState.entity && targetX + kRoomRegionTargetXBias < playerState.entity->getScreenRect().right)
		targetX -= kRoomRegionTargetXBias;
	return CLIP<int>(targetX, 0, 639);
}

static float resolveRegionTargetZ(const StartupRegionRecord &region) {
	if (region.desiredFacing == 0)
		return (float)region.maxZ;
	if (region.desiredFacing == 3)
		return (float)region.minZ;

	return (float)(region.minZ + (region.maxZ - region.minZ) / 2);
}

static uint32 getRuntimeClockTicks() {
	return g_system ? (g_system->getMillis() / kRuntimeClockDivisorMs) : 0;
}

static bool isIdleAnimationExcludedRoom(const Common::String &roomName) {
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

static void updatePlayerIdleTrigger(StartupRoomIdleAnimationState &idleState) {
	idleState.triggerTick = MAX(idleState.activityTick, idleState.resetTick) + kRoomPlayerIdleDelayTicks;
}

static void notePlayerActivity(StartupRoomIdleAnimationState &idleState) {
	idleState.activityTick = getRuntimeClockTicks();
	updatePlayerIdleTrigger(idleState);
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

static void logScenePaletteSummary(const char *label, const StartupRoomSceneResources &scene, float brightness) {
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

static void blitTransparentBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	int srcX = 0;
	int srcY = 0;
	int destX = x;
	int destY = y;
	int width = (int)bitmap.width;
	int height = (int)bitmap.height;

	if (destX < 0) {
		srcX = -destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0) {
		srcY = -destY;
		height += destY;
		destY = 0;
	}
	if (destX >= screen.w || destY >= screen.h || width <= 0 || height <= 0)
		return;

	width = MIN<int>(width, screen.w - destX);
	height = MIN<int>(height, screen.h - destY);
	if (width <= 0 || height <= 0)
		return;

	const byte *src = bitmap.pixels.data() + srcY * bitmap.width + srcX;
	byte *dst = (byte *)screen.getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, screen.pitch, bitmap.width, width, height,
		screen.format.bytesPerPixel, kTransparentPaletteIndex);
}

static bool captureScreenBackdrop(const Graphics::Screen &screen, IndexedBitmap &bitmap) {
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

static void buildScaledPalette(const byte *source, float brightness, byte *dest) {
	memset(dest, 0, 256 * 3);
	if (!source)
		return;

	for (uint color = 1; color < 256; ++color) {
		for (uint channel = 0; channel < 3; ++channel) {
			const uint index = color * 3 + channel;
			const int scaled = (int)(source[index] * brightness + 0.5f);
			dest[index] = (byte)MIN<int>(scaled, 255);
		}
	}
}

static void setScaledPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	buildScaledPalette(palette, brightness, scaledPalette);
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

static Common::Rect getHotspotBounds(const StartupObjectRecord &object) {
	if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
		return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);

	return Common::Rect();
}

static Common::Rect getRegionBounds(const StartupRegionRecord &region) {
	if (region.right > region.left && region.bottom > region.top)
		return Common::Rect(region.left, region.top, region.right + 1, region.bottom + 1);

	return Common::Rect();
}

static Common::String resolveSceneObjectSpritePath(const StartupObjectRecord &object) {
	const bool atInitialPlacement = object.currentX == object.initialX &&
		object.currentY == object.initialY &&
		object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
	if (!object.altSpritePath.empty() &&
		(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase("INVENTORY")))
		return object.altSpritePath;

	return object.spritePath;
}

static bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
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

static bool isInventoryExitObject(const StartupObjectRecord &object) {
	return object.objectName.equalsIgnoreCase("INV_EXIT");
}

static bool isInventoryStatusObject(const StartupObjectRecord &object) {
	return object.objectName.hasPrefixIgnoreCase("INV_STAT");
}

static bool buildInventoryVisuals(StartupScript &startupScript, ResourceManager &resources,
		Common::Array<StartupInventoryVisual> &items) {
	items.clear();

	Common::Array<StartupObjectRecord> inventoryObjects;
	startupScript.getVisibleInventoryObjects(inventoryObjects);
	int nextX = kInventoryItemStartX;
	int nextY = kInventoryItemStartY;
	int rowHeight = 0;

	for (const StartupObjectRecord &inventoryObject : inventoryObjects) {
		StartupInventoryVisual visual;
		visual.object = inventoryObject;

		if (isInventoryExitObject(inventoryObject)) {
			visual.bounds = getHotspotBounds(inventoryObject);
			items.push_back(visual);
			continue;
		}
		if (isInventoryStatusObject(inventoryObject))
			continue;

		const Common::String spritePath = resolveSceneObjectSpritePath(inventoryObject);
		if (!spritePath.empty() && loadBitmapResource(resources, spritePath, visual.bitmap)) {
			visual.hasBitmap = true;
			if (nextX + (int)visual.bitmap.width > kInventoryItemMaxRight) {
				nextX = kInventoryItemStartX;
				nextY += rowHeight + kInventoryItemSpacing;
				rowHeight = 0;
			}

			visual.object.currentX = nextX;
			visual.object.currentY = nextY;
			visual.bounds = Common::Rect(nextX, nextY, nextX + visual.bitmap.width, nextY + visual.bitmap.height);
			nextX += visual.bitmap.width + kInventoryItemSpacing;
			rowHeight = MAX<int>(rowHeight, visual.bitmap.height);
		} else {
			visual.bounds = getHotspotBounds(inventoryObject);
		}

		items.push_back(visual);
	}

	return true;
}

static const StartupInventoryVisual *findInventoryVisualAtPoint(
		const Common::Array<StartupInventoryVisual> &items, const Common::Point &point) {
	for (int i = (int)items.size() - 1; i >= 0; --i) {
		if (items[i].bounds.contains(point))
			return &items[i];
	}

	return nullptr;
}

static Common::String buildUseItemPrompt(const Common::String &itemLabel, const Common::String &targetLabel) {
	if (itemLabel.empty())
		return Common::String();
	if (targetLabel.empty())
		return Common::String::format("Use %s on ...", itemLabel.c_str());

	return Common::String::format("Use %s on %s", itemLabel.c_str(), targetLabel.c_str());
}

static Common::Rect getInventoryPanelBounds(const StartupArt &art) {
	const IndexedBitmap &bitmap = art.getInventoryBitmap();
	return Common::Rect(kInventoryX, kInventoryY,
		kInventoryX + (int)bitmap.width, kInventoryY + (int)bitmap.height);
}

static const CftFontResource *findStartupFontByName(const HarvesterEngine &engine, const char *fontName) {
	const StartupText *startupText = engine.getStartupText();
	if (!startupText || !fontName)
		return nullptr;

	for (const CftFontResource &font : startupText->getFonts()) {
		if (font.name.equalsIgnoreCase(fontName))
			return &font;
	}

	return nullptr;
}

static int getNativeRoomMenuLineHeight(const Graphics::Font &selectedFont) {
	return selectedFont.getFontHeight() + 2;
}

static int getNativeRoomMenuSelectionFromMouse(const Graphics::Font &selectedFont, uint itemCount,
		const Common::Point &mousePos) {
	if (itemCount == 0)
		return -1;

	int selection = mousePos.y - kMenuStartY;
	if (selection < 1)
		selection = 1;

	selection /= getNativeRoomMenuLineHeight(selectedFont);
	return MIN<int>(selection, (int)itemCount - 1);
}

static void drawInventoryOverlay(Graphics::Screen &screen, const StartupArt &art, StartupScript &startupScript,
		const Graphics::Font &font, const Common::Array<StartupInventoryVisual> &items,
		const Common::String &selectedItemName, const Common::String &promptText) {
	blitBitmap(screen, art.getInventoryBitmap(), kInventoryX, kInventoryY);

	for (const StartupInventoryVisual &item : items) {
		if (item.hasBitmap && item.bitmap.isValid())
			blitBitmap(screen, item.bitmap, item.object.currentX, item.object.currentY);
	}

	Common::String overlayPrompt = promptText;
	if (overlayPrompt.empty() && !selectedItemName.empty()) {
		for (const StartupInventoryVisual &item : items) {
			if (item.object.objectName.equalsIgnoreCase(selectedItemName)) {
				overlayPrompt = buildUseItemPrompt(startupScript.resolveObjectLabel(item.object), Common::String());
				break;
			}
		}
	}

	if (!overlayPrompt.empty())
		drawShadowedString(screen, font, overlayPrompt, 0, 462, 640, kRoomPromptColor, Graphics::kTextAlignCenter);
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

static bool isInteractiveSceneHotspot(const StartupObjectRecord &object, StartupScript *startupScript) {
	if (object.operatable || !object.actionTag.empty())
		return true;
	if (!startupScript)
		return false;

	StartupResolvedText inspectText;
	return startupScript->resolveObjectInspectText(object, inspectText);
}

static int resolveSceneObjectClass(const StartupObjectRecord &object, const RuntimeEntity *entity,
		StartupScript *startupScript) {
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

static bool loadRoomSceneResources(const StartupRoomSetupState &state, ResourceManager &resources, StartupRoomSceneResources &scene) {
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
		"Harvester: loadRoomSceneResources room='%s' palette='%s' background='%s' brightness=%.2f objects=%u activeObjects=%u sceneObjects=%u anims=%u visibleAnims=%u npcs=%u regions=%u",
		state.roomName.c_str(), state.palettePath.c_str(), state.backgroundPath.c_str(), (double)state.paletteBrightness,
		(uint)state.roomObjects.size(), (uint)state.activeObjects.size(), (uint)scene.sceneObjects.size(),
		(uint)state.roomAnimations.size(), (uint)scene.sceneAnimations.size(),
		(uint)state.roomNpcs.size(), (uint)scene.sceneRegions.size());

	return true;
}

static bool shouldRunStartupRoomProbe() {
	return ConfMan.hasKey("harvester_debug_probe_startup_room") &&
		ConfMan.getBool("harvester_debug_probe_startup_room");
}

static void drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen, const StartupRoomSceneResources &scene,
		float brightness) {
	setScaledPalette(screen, scene.palette, brightness);
	screen.fillRect(screen.getBounds(), 0);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawSceneEntities(screen);
}

static const StartupObjectRecord *findSceneObjectByName(const Common::Array<StartupObjectRecord> &objects,
		const Common::String &objectName) {
	for (const StartupObjectRecord &object : objects) {
		if (object.objectName.equalsIgnoreCase(objectName))
			return &object;
	}

	return nullptr;
}

static const StartupRegionRecord *findSceneRegionByName(const Common::Array<StartupRegionRecord> &regions,
		const Common::String &regionName) {
	for (const StartupRegionRecord &region : regions) {
		if (region.regionName.equalsIgnoreCase(regionName))
			return &region;
	}

	return nullptr;
}

static StartupObjectRecord *findSceneObjectByName(Common::Array<StartupObjectRecord> &objects,
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

static const IndexedBitmap *resolveInspectTextboxBitmap(const StartupArt &art, const StartupResolvedText &text) {
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

static void drawRoomInspectText(Graphics::Screen &screen, const StartupArt &art, const Graphics::Font &font,
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

static bool unlocksRoomObjectInteractionAfterInitialExamine(const StartupObjectRecord &object,
		StartupScript &startupScript) {
	return object.operatable || startupScript.isPickupObject(object);
}

static int resolveRoomObjectCursorSequence(const StartupObjectRecord &object, StartupScript &startupScript) {
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

static Common::String buildRoomObjectPrompt(const StartupObjectRecord &object, StartupScript &startupScript,
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

static bool doesPlayerFacingMatchRegion(int playerFacing, const StartupRegionRecord &region) {
	return region.desiredFacing < 0 || playerFacing == region.desiredFacing;
}

static bool doesPlayerOverlapRegion(const RuntimeEntity &playerEntity, const StartupRegionRecord &region) {
	const Common::Rect regionBounds = getRegionBounds(region);
	if (regionBounds.isEmpty())
		return false;
	if (!playerEntity.getScreenRect().intersects(regionBounds))
		return false;

	const float playerMaxZ = playerEntity.getZ() + playerEntity.getZExtent();
	return playerMaxZ >= (float)region.minZ && (float)region.maxZ >= playerEntity.getZ();
}

static StartupRoomHoverState resolveRoomHoverState(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupRegionRecord> &sceneRegions, const Common::Point &mousePos) {
	StartupRoomHoverState hoverState;
	if (const RuntimeEntity *playerEntity = findRoomPlayerAtPoint(engine, mousePos)) {
		hoverState.playerEntity = playerEntity;
		hoverState.cursorSequence = kCursorSequenceExamine;
		return hoverState;
	}

	StartupScript *startupScript = engine.getStartupScript();
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

static bool setPlayerIdleAnimation(StartupRoomPlayerState &playerState, int facing) {
	if (!playerState.entity)
		return false;

	const PlayerAnimationRange range = resolvePlayerAnimationRange(facing);
	const bool changed = playerState.facing != facing ||
		playerState.entity->getCurrentFrame() != range.idleFrame ||
		playerState.entity->getAnimationRate() != 0;
	playerState.facing = facing;
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

static bool startPlayerTurnAnimation(StartupRoomPlayerState &playerState, int targetFacing) {
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

static bool updatePlayerTurnAnimationState(StartupRoomPlayerState &playerState) {
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

static bool stepPlayerMoveTarget(HarvesterEngine &engine, const StartupRoomSetupState &state,
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

static bool stepPlayerKeyboardMovement(HarvesterEngine &engine, const StartupRoomSetupState &state,
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

	const Common::Rect playerRect = playerState.entity->getScreenRect();
	idleEntity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
	idleEntity.setDepthScale(computeActorDepthScale(state, playerState.z));
	idleEntity.setPosition(playerRect.left, playerRect.top + kRoomPlayerIdleYOffset, playerState.z);
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
		Common::Point(playerState.entity->getScreenRect().left,
			playerState.entity->getScreenRect().top + kRoomPlayerIdleYOffset),
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

static bool requestPlayerIdleAnimationExit(const StartupRoomSetupState &state,
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

static bool startPlayerIdleAnimation(HarvesterEngine &engine, const StartupRoomSetupState &state,
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

static bool updatePlayerIdleAnimation(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
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

static void logStartupRoomProbe(HarvesterEngine &engine, const StartupRoomSceneResources &scene,
		const Common::String &entranceName, Common::Point &mousePos) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	StartupScript *startupScript = engine.getStartupScript();
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
			engine, scene.state, scene.sceneObjects, scene.sceneRegions, probePoint);
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
						engine, scene.state, scene.sceneObjects, scene.sceneRegions, Common::Point(x, y));
					if (candidateHover.cursorSequence == kCursorSequenceWalk) {
						floorProbe = Common::Point(x, y);
						foundFloorProbe = true;
						break;
					}
				}
			}

			const StartupRoomHoverState floorHover = foundFloorProbe
				? resolveRoomHoverState(engine, scene.state, scene.sceneObjects, scene.sceneRegions, floorProbe)
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
	const StartupArt *art = engine.getStartupArt();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	StartupScript *startupScript = engine.getStartupScript();
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

} // End of anonymous namespace

StartupFlow::StartupFlow(HarvesterEngine &engine) : _engine(engine), _mousePos(320, 200) {
}

bool StartupFlow::load() {
	return loadQuickTips() && loadMenuItems();
}

Common::Error StartupFlow::run() {
	if (!ensureCursorEntity())
		return Common::kReadingFailed;

	_engine.getStartupScript()->resetRuntimeState();
	Common::Error error = runQuickTips();
	if (error.getCode() != Common::kNoError)
		return error;

	_engine.getStartupScript()->resetRuntimeState();
	return runRoomLoop("START");
}

bool StartupFlow::loadQuickTips() {
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

bool StartupFlow::loadMenuItems() {
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

Common::Error StartupFlow::runQuickTips() {
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

Common::Error StartupFlow::runMainMenuStub() {
	Graphics::FrameLimiter limiter(g_system, 60);
	Common::String statusMessage = "Stub handoff after room_setup(\"QUICK_TIPS\") and before the main menu loop.";
	int selectedItem = _menuItems.empty() ? -1 : 0;
	bool needsRedraw = true;
	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->clearSceneEntities();
	resetCursorAnimationSequence();

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderMainMenuStub(selectedItem, statusMessage);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredItem = getMenuItemAt(_mousePos);
				if (hoveredItem != -1 && hoveredItem != selectedItem) {
					selectedItem = hoveredItem;
					needsRedraw = true;
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;

				if (_menuItems.empty())
					break;

				if (event.kbd.keycode == Common::KEYCODE_UP) {
					selectedItem = (selectedItem + _menuItems.size() - 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					selectedItem = (selectedItem + 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					if (_menuItems[selectedItem].equalsIgnoreCase("QUIT GAME"))
						return Common::kNoError;

					if (_menuItems[selectedItem].equalsIgnoreCase("NEW GAME")) {
						_engine.getStartupScript()->resetRuntimeState();
						Common::Error roomError = runRoomLoop("START");
						if (roomError.getCode() == Common::kReadingFailed) {
							statusMessage = "Unable to resolve START room setup from HARVEST.SCR.";
							needsRedraw = true;
						} else if (roomError.getCode() != Common::kNoError) {
							return roomError;
						} else {
							statusMessage = "Resolved START room handoff from HARVEST.SCR.";
							needsRedraw = true;
						}
					} else {
						statusMessage = Common::String::format("%s is stubbed; room setup and gameplay loop are next.",
							_menuItems[selectedItem].c_str());
						needsRedraw = true;
					}
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (_menuItems.empty())
					break;

				selectedItem = getMenuItemAt(_mousePos);
				if (selectedItem == -1)
					break;

				if (_menuItems[selectedItem].equalsIgnoreCase("QUIT GAME"))
					return Common::kNoError;

				if (_menuItems[selectedItem].equalsIgnoreCase("NEW GAME")) {
					_engine.getStartupScript()->resetRuntimeState();
					Common::Error roomError = runRoomLoop("START");
					_engine.stopStartupMusic();
					_engine.stopStartupSound();
					if (roomError.getCode() == Common::kReadingFailed) {
						statusMessage = "Unable to resolve START room setup from HARVEST.SCR.";
						needsRedraw = true;
					} else if (roomError.getCode() != Common::kNoError) {
						return roomError;
					} else {
						statusMessage = "Resolved START room handoff from HARVEST.SCR.";
						needsRedraw = true;
					}
				} else {
					statusMessage = Common::String::format("%s is stubbed; room setup and gameplay loop are next.",
						_menuItems[selectedItem].c_str());
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

Common::Error StartupFlow::runRoomMenuStub(const IndexedBitmap &backdrop) {
	Graphics::FrameLimiter limiter(g_system, 60);
	int selectedItem = _menuItems.empty() ? -1 : 0;
	bool needsRedraw = true;
	resetCursorAnimationSequence();

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderRoomMenuStub(backdrop, selectedItem);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredItem = getRoomMenuItemAt(_mousePos);
				if (hoveredItem != -1 && hoveredItem != selectedItem) {
					selectedItem = hoveredItem;
					needsRedraw = true;
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;

				if (_menuItems.empty())
					break;

				if (event.kbd.keycode == Common::KEYCODE_UP) {
					selectedItem = (selectedItem + _menuItems.size() - 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					selectedItem = (selectedItem + 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					debug(1, "Harvester: room menu item '%s' selected but not implemented",
						selectedItem >= 0 ? _menuItems[selectedItem].c_str() : "");
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (_menuItems.empty())
					break;

				selectedItem = getRoomMenuItemAt(_mousePos);
				if (selectedItem == -1)
					break;

				debug(1, "Harvester: room menu item '%s' clicked but not implemented",
					_menuItems[selectedItem].c_str());
				break;
			default:
				break;
			}
		}

		RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
		if (runtimeEntities && runtimeEntities->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error StartupFlow::runRoomLoop(const Common::String &entranceName) {
	StartupRoomSetupState state;
	if (!_engine.getStartupScript()->resolveRoomSetupState(entranceName, state, *_engine.getResources()))
		return Common::kReadingFailed;

	Common::Error transitionError = beginRoomSetupTransition();
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	StartupRoomSceneResources scene;
	if (!loadRoomSceneResources(state, *_engine.getResources(), scene))
		return Common::kReadingFailed;

	Graphics::Screen *screen = _engine.getScreen();
	const StartupArt *art = _engine.getStartupArt();
	const Graphics::Font *bodyFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!screen || !art || !bodyFont)
		return Common::kNoError;

	if (!populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
		return Common::kReadingFailed;

	logScenePaletteSummary("room setup stub palette", scene, kPaletteBrightnessBlack);
	drawRoomScene(_engine, *screen, scene, kPaletteBrightnessBlack);
	screen->makeAllDirty();
	screen->update();

	logScenePaletteSummary("room setup fade target", scene, scene.targetPaletteBrightness);
	transitionError = fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	resetCursorAnimationSequence();
	executeStartupAudioCommands(scene.state.audioCommands);
	if (!scene.state.musicPath.empty())
		(void)_engine.playStartupMusic(scene.state.musicPath);
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	StartupRoomPlayerState playerState;
	playerState.entity = runtimeEntities ? runtimeEntities->findSceneEntityByName(kPlayerActorEntityName) : nullptr;
	playerState.centerX = state.playerSpawnX;
	playerState.bottomY = state.playerSpawnY;
	playerState.z = (float)state.playerSpawnZ;
	playerState.facing = state.playerFacing;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	StartupResolvedText inspectText;
	bool showingInspectText = false;
	bool inspectCanDismiss = false;
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
	Common::String pendingRegionName;
	StartupInventoryOverlayState inventoryState;
	StartupRoomIdleAnimationState idleState;
	bool needsRedraw = true;
	auto resetIdleState = [&]() {
		idleState = StartupRoomIdleAnimationState();
		idleState.activityTick = getRuntimeClockTicks();
		idleState.resetTick = idleState.activityTick;
		updatePlayerIdleTrigger(idleState);
	};
	resetIdleState();
	auto refreshInventoryState = [&]() {
		if (!buildInventoryVisuals(*_engine.getStartupScript(), *_engine.getResources(), inventoryState.items))
			return false;

		if (inventoryState.selectedItemName.empty())
			return true;

		for (const StartupInventoryVisual &item : inventoryState.items) {
			if (item.object.objectName.equalsIgnoreCase(inventoryState.selectedItemName))
				return true;
		}

		inventoryState.selectedItemName.clear();
		inventoryState.promptText.clear();
		return true;
	};
	auto resolveSelectedInventoryLabel = [&]() {
		if (inventoryState.selectedItemName.empty())
			return Common::String();

		for (const StartupInventoryVisual &item : inventoryState.items) {
			if (item.object.objectName.equalsIgnoreCase(inventoryState.selectedItemName))
				return _engine.getStartupScript()->resolveObjectLabel(item.object);
		}

		Common::Array<StartupObjectRecord> inventoryObjects;
		_engine.getStartupScript()->getVisibleInventoryObjects(inventoryObjects);
		for (const StartupObjectRecord &item : inventoryObjects) {
			if (item.objectName.equalsIgnoreCase(inventoryState.selectedItemName))
				return _engine.getStartupScript()->resolveObjectLabel(item);
		}

		return normalizeHarvesterResourcePath(inventoryState.selectedItemName);
	};
	auto clearInventorySelection = [&]() {
		if (inventoryState.selectedItemName.empty() && inventoryState.promptText.empty())
			return false;

		inventoryState.selectedItemName.clear();
		inventoryState.promptText.clear();
		return true;
	};
	auto closeInventoryOverlay = [&]() {
		bool changed = clearInventorySelection();
		if (inventoryState.open)
			changed = true;
		inventoryState.open = false;
		return changed;
	};
	auto openInventoryOverlay = [&]() {
		moveLeft = false;
		moveRight = false;
		moveUp = false;
		moveDown = false;
		pendingRegionName.clear();
		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		inventoryState.open = true;
		inventoryState.promptText.clear();
		return refreshInventoryState();
	};
	auto refreshCurrentScene = [&](bool preservePlayerPlacement) {
		const Common::Array<StartupAudioCommand> entryAudioCommands = scene.state.audioCommands;
		StartupRoomSetupState refreshedState;
		if (!_engine.getStartupScript()->materializeRoomState(
				scene.state.entranceName, scene.state.roomName, refreshedState)) {
			return false;
		}

		refreshedState.audioCommands = entryAudioCommands;
		if (!loadRoomSceneResources(refreshedState, *_engine.getResources(), scene))
			return false;
		if (!populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return false;

		playerState.entity = runtimeEntities ? runtimeEntities->findSceneEntityByName(kPlayerActorEntityName) : nullptr;
		if (playerState.entity) {
			if (!preservePlayerPlacement || playerState.facing < 0) {
				playerState.centerX = scene.state.playerSpawnX;
				playerState.bottomY = scene.state.playerSpawnY;
				playerState.z = (float)scene.state.playerSpawnZ;
				playerState.facing = scene.state.playerFacing;
			}

			const int facing = playerState.facing >= 0 ? playerState.facing : scene.state.playerFacing;
			(void)setPlayerIdleAnimation(playerState, facing);
			(void)applyRoomActorPlacement(scene.state, *playerState.entity,
				playerState.centerX, playerState.bottomY, playerState.z);
		}

		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		pendingRegionName.clear();
		resetIdleState();
		resetCursorAnimationSequence();
		return refreshInventoryState();
	};
	auto runRoomExitCommands = [&]() {
		Common::Array<StartupAudioCommand> exitAudioCommands;
		if (!_engine.getStartupScript()->executeRoomExitCommands(scene.state.roomName, exitAudioCommands))
			return false;
		executeStartupAudioCommands(exitAudioCommands);
		return true;
	};
	auto handleInteractionResult = [&](const StartupInteractionResult &interaction) -> Common::Error {
		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		pendingRegionName.clear();

		Common::String restoreMusicPath = _engine.getStartupMusicPath();
		if (!interaction.musicPath.empty()) {
			(void)_engine.playStartupMusic(interaction.musicPath);
			restoreMusicPath = _engine.getStartupMusicPath();
		}
		executeStartupAudioCommands(interaction.audioCommands);

		if (!interaction.nextRoomName.empty()) {
			if (!runRoomExitCommands())
				return Common::kReadingFailed;

			Common::Error roomError = runRoomLoop(interaction.nextRoomName);
			if (roomError.getCode() != Common::kReadingFailed &&
				roomError.getCode() != Common::kNoError) {
				return roomError;
			}

			if (!refreshCurrentScene(true))
				return Common::kReadingFailed;

			executeStartupAudioCommands(scene.state.audioCommands);
			if (!restoreMusicPath.empty())
				(void)_engine.playStartupMusic(restoreMusicPath);
			else
				_engine.stopStartupMusic();
		} else if (interaction.mutatedRuntimeState) {
			if (!refreshCurrentScene(true))
				return Common::kReadingFailed;
		}

		return Common::kNoError;
	};
	auto handleInventoryTargetInteraction = [&](const StartupObjectRecord &target, bool preferPickup) -> Common::Error {
		if (inventoryState.selectedItemName.empty())
			return Common::kNoError;

		StartupInteractionResult interaction;
		bool handled = false;
		if (preferPickup && _engine.getStartupScript()->isPickupObject(target))
			handled = _engine.getStartupScript()->resolveObjectInteraction(target, interaction);
		else
			handled = _engine.getStartupScript()->resolveUseItemInteraction(
				inventoryState.selectedItemName, target, interaction);
		if (!handled)
			return Common::kNoError;

		clearInventorySelection();
		Common::Error interactionError = handleInteractionResult(interaction);
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		if (!refreshInventoryState())
			return Common::kReadingFailed;

		needsRedraw = true;
		return Common::kNoError;
	};
	auto queueRegionInteraction = [&](const StartupRegionRecord &region) {
		pendingRegionName = region.regionName;
		if (!playerState.entity)
			return;

		if (doesPlayerOverlapRegion(*playerState.entity, region))
			return;

		setPlayerMoveTarget(scene.state, playerState,
			resolveRegionTargetX(region, playerState), resolveRegionTargetZ(region));
	};
	auto runRegionInteraction = [&](const StartupRegionRecord &region) -> Common::Error {
		StartupInteractionResult interaction;
		if (!_engine.getStartupScript()->resolveRegionInteraction(region, interaction))
			return Common::kNoError;

		Common::Error interactionError = handleInteractionResult(interaction);
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;

		if (!refreshInventoryState())
			return Common::kReadingFailed;
		needsRedraw = true;
		return Common::kNoError;
	};
	auto tryActivatePendingRegion = [&]() -> Common::Error {
		if (pendingRegionName.empty() || !playerState.entity)
			return Common::kNoError;

		const StartupRegionRecord *region = findSceneRegionByName(scene.sceneRegions, pendingRegionName);
		if (!region || !region->startEnabled) {
			pendingRegionName.clear();
			return Common::kNoError;
		}
		if (!doesPlayerOverlapRegion(*playerState.entity, *region))
			return Common::kNoError;
		if (!doesPlayerFacingMatchRegion(playerState.facing, *region)) {
			if (!playerState.hasMoveTarget && !playerState.turnActive && region->desiredFacing >= 0)
				(void)startPlayerTurnAnimation(playerState, region->desiredFacing);
			return Common::kNoError;
		}

		pendingRegionName.clear();
		return runRegionInteraction(*region);
	};
	if (!refreshInventoryState())
		return Common::kReadingFailed;
	Graphics::FrameLimiter limiter(g_system, 60);

	if (shouldRunStartupRoomProbe())
		logStartupRoomProbe(_engine, scene, entranceName, _mousePos);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			const Common::Rect inventoryPanelBounds = getInventoryPanelBounds(*art);
			const bool inventoryPanelContainsMouse = inventoryState.open && inventoryPanelBounds.contains(_mousePos);
			const bool suppressHover = showingInspectText || idleState.active || idleState.exiting ||
				(inventoryState.open && (inventoryPanelContainsMouse || inventoryState.selectedItemName.empty()));
			StartupRoomHoverState hoverState = suppressHover
				? StartupRoomHoverState()
				: resolveRoomHoverState(_engine, scene.state, scene.sceneObjects, scene.sceneRegions, _mousePos);
			Common::String promptText;
			if (inventoryState.open) {
				const StartupInventoryVisual *inventoryHover = findInventoryVisualAtPoint(inventoryState.items, _mousePos);
				if (!inventoryState.selectedItemName.empty()) {
					Common::String targetLabel;
					if (inventoryHover && !isInventoryExitObject(inventoryHover->object) &&
						!isInventoryStatusObject(inventoryHover->object)) {
						targetLabel = _engine.getStartupScript()->resolveObjectLabel(inventoryHover->object);
					} else if (!inventoryPanelContainsMouse && hoverState.object) {
						targetLabel = _engine.getStartupScript()->resolveObjectLabel(*hoverState.object);
					}
					promptText = buildUseItemPrompt(resolveSelectedInventoryLabel(), targetLabel);
				} else if (inventoryHover) {
					promptText = _engine.getStartupScript()->resolveObjectLabel(inventoryHover->object);
				}
				inventoryState.promptText = promptText;
				hoverState.cursorSequence = kCursorSequenceNeutral;
			} else {
				promptText = hoverState.promptText;
			}
			if (RuntimeEntity *cursor = runtimeEntities ? runtimeEntities->getCursorEntity() : nullptr) {
				cursor->setAnimationSequence(
					(showingInspectText || idleState.active || idleState.exiting || inventoryState.open)
						? kCursorSequenceNeutral
						: hoverState.cursorSequence);
			}

			drawRoomScene(_engine, *screen, scene, scene.targetPaletteBrightness);
			if (inventoryState.open)
				drawInventoryOverlay(*screen, *art, *_engine.getStartupScript(), *bodyFont,
					inventoryState.items, inventoryState.selectedItemName, inventoryState.promptText);

			if (showingInspectText) {
				drawRoomInspectText(*screen, *art, *bodyFont, inspectText);
			} else if (!promptText.empty()) {
				drawShadowedString(*screen, *bodyFont, promptText,
					0, 462, 640, kRoomPromptColor, Graphics::kTextAlignCenter);
			}

			if (runtimeEntities)
				runtimeEntities->drawCursor(*screen);
			screen->makeAllDirty();
			screen->update();
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
			case Common::EVENT_RBUTTONDOWN:
				notePlayerActivity(idleState);
				if (idleState.active || idleState.exiting) {
					if (requestPlayerIdleAnimationExit(scene.state, playerState, idleState))
						needsRedraw = true;
					break;
				}
				pendingRegionName.clear();
				if (inventoryState.open) {
					if (clearInventorySelection() || closeInventoryOverlay())
						needsRedraw = true;
					break;
				}
				break;
			case Common::EVENT_LBUTTONUP:
				if (showingInspectText)
					inspectCanDismiss = true;
				break;
			case Common::EVENT_LBUTTONDOWN: {
				notePlayerActivity(idleState);
				if (idleState.active || idleState.exiting) {
					if (requestPlayerIdleAnimationExit(scene.state, playerState, idleState))
						needsRedraw = true;
					break;
				}

				if (showingInspectText) {
					if (inspectCanDismiss) {
						showingInspectText = false;
						inspectCanDismiss = false;
						inspectText = StartupResolvedText();
						needsRedraw = true;
					}
					break;
				}

				const Common::Rect inventoryPanelBounds = getInventoryPanelBounds(*art);
				if (inventoryState.open) {
					const StartupInventoryVisual *inventoryHover =
						findInventoryVisualAtPoint(inventoryState.items, _mousePos);
					if (inventoryHover) {
						if (isInventoryExitObject(inventoryHover->object)) {
							if (closeInventoryOverlay())
								needsRedraw = true;
							break;
						}

						if (inventoryState.selectedItemName.empty()) {
							inventoryState.selectedItemName = inventoryHover->object.objectName;
							inventoryState.promptText = buildUseItemPrompt(
								resolveSelectedInventoryLabel(), Common::String());
							needsRedraw = true;
							break;
						}

						if (!inventoryHover->object.objectName.equalsIgnoreCase(inventoryState.selectedItemName)) {
							Common::Error interactionError =
								handleInventoryTargetInteraction(inventoryHover->object, false);
							if (interactionError.getCode() != Common::kNoError)
								return interactionError;
						}
						needsRedraw = true;
						break;
					}

					if (!inventoryState.selectedItemName.empty()) {
						const StartupRoomHoverState useHoverState = resolveRoomHoverState(
							_engine, scene.state, scene.sceneObjects, scene.sceneRegions, _mousePos);
						const StartupObjectRecord *roomTarget = useHoverState.object
							? findSceneObjectByName(scene.sceneObjects, useHoverState.object->objectName)
							: nullptr;
						if (roomTarget) {
							Common::Error interactionError =
								handleInventoryTargetInteraction(*roomTarget, true);
							if (interactionError.getCode() != Common::kNoError)
								return interactionError;
							needsRedraw = true;
							break;
						}
					}

					if (!inventoryPanelBounds.contains(_mousePos) && !inventoryState.selectedItemName.empty()) {
						needsRedraw = true;
						break;
					}
					if (!inventoryPanelBounds.contains(_mousePos) && closeInventoryOverlay())
						needsRedraw = true;
					break;
				}

				const StartupRoomHoverState hoverState = resolveRoomHoverState(
					_engine, scene.state, scene.sceneObjects, scene.sceneRegions, _mousePos);
				debugC(1, kDebugScene,
					"Harvester: room click room='%s' mouse=(%d,%d) object='%s' region='%s' cursor_sequence=%d prompt='%s'",
					scene.state.roomName.c_str(), _mousePos.x, _mousePos.y,
					hoverState.object ? hoverState.object->objectName.c_str() : "",
					hoverState.region ? hoverState.region->regionName.c_str() : "",
					hoverState.cursorSequence, hoverState.promptText.c_str());
				if (hoverState.playerEntity) {
					if (!playerState.entity || hoverState.playerEntity != playerState.entity ||
						idleState.active || idleState.exiting ||
						playerState.hasMoveTarget || playerState.turnActive ||
						playerState.entity->getAnimationRate() != 0) {
						break;
					}
					if (!openInventoryOverlay())
						return Common::kReadingFailed;
					needsRedraw = true;
					break;
				}
				if (hoverState.region && playerState.entity) {
					queueRegionInteraction(*hoverState.region);
					Common::Error interactionError = tryActivatePendingRegion();
					if (interactionError.getCode() != Common::kNoError)
						return interactionError;
					needsRedraw = true;
					break;
				}
				StartupObjectRecord *clickedObject = hoverState.object
					? findSceneObjectByName(scene.sceneObjects, hoverState.object->objectName)
					: nullptr;
				if (!clickedObject) {
					pendingRegionName.clear();
					if (hoverState.cursorSequence == kCursorSequenceWalk && playerState.entity) {
						setPlayerMoveTargetFromScreenPoint(scene.state, playerState, _mousePos.x, _mousePos.y);
						needsRedraw = true;
					}
					break;
				}
				pendingRegionName.clear();
				if (clickedObject->objectName.equalsIgnoreCase("EXIT_BM") ||
					clickedObject->objectName.equalsIgnoreCase("EXIT_HS")) {
					if (!runRoomExitCommands())
						return Common::kReadingFailed;
					return Common::kNoError;
				}

				StartupResolvedText resolvedInspectText;
				const bool hasInspectText =
					_engine.getStartupScript()->resolveObjectInspectText(*clickedObject, resolvedInspectText);
				const bool unlocksAfterInitialExamine =
					unlocksRoomObjectInteractionAfterInitialExamine(*clickedObject, *_engine.getStartupScript());
				const bool canShowInspectText = hasInspectText &&
					resolveInspectTextboxBitmap(*art, resolvedInspectText);
				if (!clickedObject->identShown && unlocksAfterInitialExamine) {
					clickedObject->identShown = true;
					_engine.getStartupScript()->markObjectIdentShown(*clickedObject);
					if (canShowInspectText) {
						inspectText = resolvedInspectText;
						showingInspectText = true;
						inspectCanDismiss = false;
					} else if (hasInspectText) {
						debug(1, "Harvester: unsupported IDENT textbox '%s' for object '%s'",
							resolvedInspectText.boxName.c_str(), clickedObject->objectName.c_str());
					}
					playerState.hasMoveTarget = false;
					playerState.turnActive = false;
					playerState.turnTargetFacing = -1;
					needsRedraw = true;
					break;
				}

				StartupInteractionResult interaction;
				if (!_engine.getStartupScript()->resolveObjectInteraction(*clickedObject, interaction)) {
					if (canShowInspectText) {
						inspectText = resolvedInspectText;
						showingInspectText = true;
						inspectCanDismiss = false;
					} else if (hasInspectText) {
						debug(1, "Harvester: unsupported IDENT textbox '%s' for object '%s'",
							resolvedInspectText.boxName.c_str(), clickedObject->objectName.c_str());
					}
					needsRedraw = true;
					break;
				}

				Common::Error interactionError = handleInteractionResult(interaction);
				if (interactionError.getCode() != Common::kNoError)
					return interactionError;
				needsRedraw = true;
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (showingInspectText) {
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						showingInspectText = false;
						inspectCanDismiss = false;
						inspectText = StartupResolvedText();
						needsRedraw = true;
					}
					break;
				}

				notePlayerActivity(idleState);
				if (idleState.active || idleState.exiting) {
					if (requestPlayerIdleAnimationExit(scene.state, playerState, idleState))
						needsRedraw = true;
					break;
				}

				if (inventoryState.open) {
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						if (clearInventorySelection() || closeInventoryOverlay())
							needsRedraw = true;
					} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
							event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
							event.kbd.keycode == Common::KEYCODE_i) {
						if (closeInventoryOverlay())
							needsRedraw = true;
					}
					break;
				}

				if (event.kbd.keycode == Common::KEYCODE_LEFT)
					moveLeft = true;
				else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
					moveRight = true;
				else if (event.kbd.keycode == Common::KEYCODE_UP)
					moveUp = true;
				else if (event.kbd.keycode == Common::KEYCODE_DOWN)
					moveDown = true;
				else if (event.kbd.keycode == Common::KEYCODE_i) {
					if (!openInventoryOverlay())
						return Common::kReadingFailed;
					needsRedraw = true;
					break;
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					moveLeft = false;
					moveRight = false;
					moveUp = false;
					moveDown = false;
					pendingRegionName.clear();
					playerState.hasMoveTarget = false;
					playerState.turnActive = false;
					playerState.turnTargetFacing = -1;
					IndexedBitmap roomMenuBackdrop;
					drawRoomScene(_engine, *screen, scene, scene.targetPaletteBrightness);
					if (!captureScreenBackdrop(*screen, roomMenuBackdrop))
						return Common::kReadingFailed;
					Common::Error menuError = runRoomMenuStub(roomMenuBackdrop);
					if (menuError.getCode() != Common::kNoError)
						return menuError;
					resetCursorAnimationSequence();
					resetIdleState();
					needsRedraw = true;
					break;
				}

				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					if (!runRoomExitCommands())
						return Common::kReadingFailed;
					return Common::kNoError;
				}
				break;
			case Common::EVENT_KEYUP:
				if (event.kbd.keycode == Common::KEYCODE_LEFT)
					moveLeft = false;
				else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
					moveRight = false;
				else if (event.kbd.keycode == Common::KEYCODE_UP)
					moveUp = false;
				else if (event.kbd.keycode == Common::KEYCODE_DOWN)
					moveDown = false;
				break;
			default:
				break;
			}
		}

		if (updatePlayerTurnAnimationState(playerState))
			needsRedraw = true;

		if (!idleState.active && !idleState.exiting) {
			if (stepPlayerKeyboardMovement(_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState, moveLeft, moveRight, moveUp, moveDown)) {
				notePlayerActivity(idleState);
				needsRedraw = true;
			} else if (stepPlayerMoveTarget(_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState)) {
				notePlayerActivity(idleState);
				needsRedraw = true;
			} else if (!moveLeft && !moveRight && !moveUp && !moveDown && !playerState.hasMoveTarget &&
					!playerState.turnActive &&
					playerState.entity && playerState.facing >= 0 &&
					setPlayerIdleAnimation(playerState, playerState.facing)) {
				needsRedraw = true;
			}

			if (!showingInspectText && !moveLeft && !moveRight && !moveUp && !moveDown &&
					!playerState.hasMoveTarget && !playerState.turnActive &&
					playerState.entity && playerState.facing >= 0 &&
					!isIdleAnimationExcludedRoom(scene.state.roomName) &&
					getRuntimeClockTicks() > idleState.triggerTick &&
					startPlayerIdleAnimation(_engine, scene.state, playerState, idleState)) {
				needsRedraw = true;
			}
		}
		Common::Error pendingRegionError = tryActivatePendingRegion();
		if (pendingRegionError.getCode() != Common::kNoError)
			return pendingRegionError;

		if (tickRuntimeEntities())
			needsRedraw = true;
		if (updatePlayerIdleAnimation(scene.state, playerState, idleState))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

bool StartupFlow::ensureCursorEntity() {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;
	if (runtimeEntities->getCursorEntity())
		return true;

	return runtimeEntities->spawnCursorEntity(_mousePos) != nullptr;
}

bool StartupFlow::populateRoomSceneEntities(const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &drawableObjects,
		const Common::Array<StartupAnimRecord> &drawableAnimations) {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;

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
		entity->setHitTestMode(kRuntimeEntityHitTestNone);
		entity->setLooping(true);
		if (npc.frameDelay > 0)
			entity->setAnimationRate(npc.frameDelay);
		if (!applyRoomActorPlacement(state, *entity, npc.posX, npc.posY, (float)npc.posZ)) {
			debug(1, "Harvester: unable to apply room npc placement for '%s'",
				npc.npcName.c_str());
		}
		debugC(1, kDebugScene,
			"Harvester: scene npc spawned room='%s' npc='%s' class=0x%x pos=(%d,%d,z=%.2f) frame_delay=%d model='%s' active=%d visible=%d",
			state.roomName.c_str(), npc.npcName.c_str(), entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(),
			npc.frameDelay, npc.modelPath.c_str(), npc.active, npc.visible);
	}
	if (state.hasEntrance) {
		const int playerFrame = resolvePlayerFacingFrame(state.playerFacing);
		RuntimeEntity *player = runtimeEntities->spawnSceneActorEntity(kPlayerActorEntityName,
			kPlayerActorResourcePath, Common::Point(state.playerSpawnX, state.playerSpawnY),
			(float)state.playerSpawnZ, playerFrame);
		if (!player) {
			debug(1, "Harvester: unable to spawn startup player actor from '%s'", kPlayerActorResourcePath);
		} else {
			player->setClassId(kRuntimeEntityClassPlayer);
			player->setHitTestMode(kRuntimeEntityHitTestOpaquePixels);
			if (!applyStartupActorPlacement(state, *player)) {
				debug(1, "Harvester: unable to apply startup player placement for entrance '%s'",
					state.entranceName.c_str());
			}
			debugC(1, kDebugGeneral,
				"Harvester: spawned startup player actor '%s' entrance='%s' pos=(%d,%d,%d) facing=%d frame=%d",
				kPlayerActorResourcePath, state.entranceName.c_str(),
				state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ,
				state.playerFacing, playerFrame);
		}
	}

	return true;
}

Common::Error StartupFlow::beginRoomSetupTransition() {
	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->hideCursor();

	const StartupArt *art = _engine.getStartupArt();
	if (art)
		art->drawWaitFrame();

	Common::Error result = Common::kNoError;
	if (pumpTransitionEvents(result))
		return result;

	return Common::kNoError;
}

Common::Error StartupFlow::fadeInRoomScene(const byte *palette, float targetBrightness) {
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

bool StartupFlow::pumpTransitionEvents(Common::Error &result) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (handleSystemEvent(event, result))
			return true;
	}

	return false;
}

void StartupFlow::executeStartupAudioCommands(const Common::Array<StartupAudioCommand> &commands) {
	for (const StartupAudioCommand &command : commands)
		(void)_engine.executeStartupAudioCommand(command);
}

void StartupFlow::resetCursorAnimationSequence() {
	if (!ensureCursorEntity())
		return;

	RuntimeEntity *cursor = _engine.getRuntimeEntities()->getCursorEntity();
	if (!cursor)
		return;

	cursor->setVisible(true);
	cursor->setAnimationSequence(kCursorSequence7);
	(void)_engine.getRuntimeEntities()->syncCursorEntityPosition(_mousePos);
}

bool StartupFlow::tickRuntimeEntities() {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;
	return runtimeEntities->tickSceneEntities() || runtimeEntities->syncCursorEntityPosition(_mousePos);
}

void StartupFlow::renderMainMenuStub(int selectedItem, const Common::String &statusMessage) const {
	Graphics::Screen *screen = _engine.getScreen();
	const StartupArt *art = _engine.getStartupArt();
	const Graphics::Font *titleFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	const Graphics::Font *bodyFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!screen || !art || !titleFont || !bodyFont)
		return;

	screen->setPalette(art->getWaitPalette());
	screen->fillRect(screen->getBounds(), 0);
	blitBitmap(*screen, art->getInventoryBitmap(), kInventoryX, kInventoryY);
	blitBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);

	const Common::Rect panel(96, 96, 544, 432);
	screen->fillRect(panel, kPanelFillColor);

	drawShadowedString(*screen, *titleFont, "Startup Main Menu Stub", panel.left, 110, panel.width(),
		kTextColorNormal, Graphics::kTextAlignCenter);

	for (uint i = 0; i < _menuItems.size(); ++i) {
		const int itemY = kMenuStartY + (int)i * kMenuLineSpacing;
		const byte color = ((int)i == selectedItem) ? kTextColorHover : kTextColorNormal;
		drawShadowedString(*screen, *bodyFont, _menuItems[i], panel.left, itemY, panel.width(), color,
			Graphics::kTextAlignCenter);
	}

	drawShadowedString(*screen, *bodyFont, "This is the next startup handoff after quick tips.", panel.left, 330,
		panel.width(), kTextColorDim, Graphics::kTextAlignCenter);
	drawWrappedShadowedText(*screen, *bodyFont, statusMessage, panel.left + 24, 360, panel.width() - 48,
		kTextColorNormal);
	drawShadowedString(*screen, *bodyFont, "Use mouse or arrow keys. Enter activates. Esc returns to launcher.",
		panel.left, 404, panel.width(), kTextColorDim, Graphics::kTextAlignCenter);

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

void StartupFlow::renderRoomMenuStub(const IndexedBitmap &backdrop, int selectedItem) const {
	Graphics::Screen *screen = _engine.getScreen();
	const StartupArt *art = _engine.getStartupArt();
	const CftFontResource *selectedFontResource = findStartupFontByName(_engine, "HARVFONT");
	const CftFontResource *unselectedFontResource = findStartupFontByName(_engine, "HARVFNT2");
	if (!screen || !art || !selectedFontResource || !unselectedFontResource || !backdrop.isValid())
		return;

	HarvesterCftFont selectedFont(*selectedFontResource);
	HarvesterCftFont unselectedFont(*unselectedFontResource);
	if (!selectedFont.isValid() || !unselectedFont.isValid())
		return;

	blitBitmap(*screen, backdrop, 0, 0);
	blitTransparentBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);

	const int lineSpacing = getNativeRoomMenuLineHeight(selectedFont);
	for (uint i = 0; i < _menuItems.size(); ++i) {
		const Graphics::Font *font = ((int)i == selectedItem)
			? static_cast<const Graphics::Font *>(&selectedFont)
			: static_cast<const Graphics::Font *>(&unselectedFont);
		const int width = font->getStringWidth(_menuItems[i]);
		const int x = (screen->w - width) / 2;
		const int y = kMenuStartY + (int)i * lineSpacing;
		font->drawString(screen, _menuItems[i], x, y, width, 0);
	}

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

bool StartupFlow::handleSystemEvent(const Common::Event &event, Common::Error &result) {
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

int StartupFlow::getMenuItemAt(const Common::Point &mousePos) const {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		return -1;

	for (uint i = 0; i < _menuItems.size(); ++i) {
		const int width = font->getStringWidth(_menuItems[i]);
		const int x = (640 - width) / 2;
		const int y = kMenuStartY + (int)i * kMenuLineSpacing;
		const Common::Rect bounds(x - 12, y - 2, x + width + 12, y + font->getFontHeight() + 2);
		if (bounds.contains(mousePos))
			return (int)i;
	}

	return -1;
}

int StartupFlow::getRoomMenuItemAt(const Common::Point &mousePos) const {
	const CftFontResource *selectedFontResource = findStartupFontByName(_engine, "HARVFONT");
	if (!selectedFontResource)
		return getMenuItemAt(mousePos);

	HarvesterCftFont selectedFont(*selectedFontResource);
	if (!selectedFont.isValid())
		return getMenuItemAt(mousePos);

	return getNativeRoomMenuSelectionFromMouse(selectedFont, _menuItems.size(), mousePos);
}

} // End of namespace Harvester
