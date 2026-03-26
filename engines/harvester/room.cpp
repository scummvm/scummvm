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

#include "harvester/room.h"

#include "common/endian.h"
#include "common/events.h"
#include "common/ptr.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/art.h"
#include "harvester/flow.h"
#include "harvester/inventory.h"
#include "harvester/monster.h"
#include "harvester/player.h"
#include "harvester/room_support.h"

namespace Harvester {

static const uint32 kRoomExitFastClickWindowTicks = 20;
static const char *const kPlayerInventoryLabel = "your inventory";
static const char *const kInventoryOwnerName = "INVENTORY";
static const byte kTransparentPaletteIndex = 0;
static const int kNativeInventoryDragCloseLeft = 0x45;
static const int kNativeInventoryDragCloseTop = 0x4b;
static const int kNativeInventoryDragCloseRight = 0x239;
static const int kNativeInventoryDragCloseBottom = 0x1b4;
static const int kNativeInventoryTooltipX = 0xbc;
static const int kNativeInventoryTooltipY = 0x19e;
static const int kNativeInventoryWeekdayX = 0x1e0;
static const int kNativeInventoryWeekdayY = 0x18c;
static const byte kNativeInventoryTooltipColor = 0xf4;
static const int kRoomMonsterAnimationRate = 17;
static const int kRoomNpcAmbientLastFrame = 0x3b;
static const int kDefaultMonsterAttackContactFrameOffset = 2;
static const int kMuckeyAttackContactFrameOffset = 7;
static const float kNativeMonsterPursuitZTolerance = 2.0f;
static const float kNativeMonsterHorizontalWaypointTolerance = 50.0f;
static const uint32 kNativeKeyboardAttackRepeatTicks = 25;
static const int kNativeKeyboardAttackSideWindow = 30;
static const char *const kNativeCombatDamageEffectResourcePath = "blood.abm";
static const int kNativeCombatDamageEffectAnimationRate = 10;
static const float kNativeCombatDamageEffectRenderZBias = 0.01f;

static int roundRoomCombatFloat(float value);

struct RoomMonsterFacingAnimationRange {
	RoomMonsterFacingAnimationRange() {}
	RoomMonsterFacingAnimationRange(int walkFirstFrame, int walkLastFrame, int idleFrame)
		: walkFirstFrame(walkFirstFrame), walkLastFrame(walkLastFrame), idleFrame(idleFrame) {}

	int walkFirstFrame = 0;
	int walkLastFrame = 0;
	int idleFrame = 0;
};

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

struct RoomMonsterCombatState {
	bool attackActive = false;
	int attackFirstFrame = -1;
	int attackLastFrame = -1;
	int attackContactFrame = -1;
	int attackResumeFacing = -1;
	bool attackSoundPlayed = false;
	bool attackContactResolved = false;
	Common::String attackTargetName;
	bool deathActive = false;
	int deathFirstFrame = -1;
	int deathLastFrame = -1;
	int deathDamageType = 0;
	uint32 nextMovementTick = 0;
	uint32 nextAttackAllowedTick = 0;
};

struct RoomCombatEffectState {
	Common::String entityName;
	Common::String followTargetName;
	Common::Point anchorPoint;
	float renderZ = 0.0f;
	bool finished = false;
};

static bool roomAllowsImmediateExitClick(const Common::String &roomName) {
	return !roomName.equalsIgnoreCase("LAVAPIT") &&
		!roomName.equalsIgnoreCase("RMNBATH") &&
		!roomName.equalsIgnoreCase("BOWLSNTRY1");
}

static bool isOutsideNativeInventoryDragCloseBounds(const Common::Point &point) {
	return point.x < kNativeInventoryDragCloseLeft ||
		point.x >= kNativeInventoryDragCloseRight ||
		point.y < kNativeInventoryDragCloseTop ||
		point.y >= kNativeInventoryDragCloseBottom;
}

static void debugLogInventoryClick(const char *buttonLabel, const Common::Point &point,
		const StartupInventoryVisual *inventoryHover) {
	if (!inventoryHover) {
		debugC(1, kDebugInventory,
			"Harvester: inventory %s click at (%d,%d) hit no item", buttonLabel, point.x, point.y);
		return;
	}

	debugC(1, kDebugInventory,
		"Harvester: inventory %s click at (%d,%d) object='%s' sprite='%s' alt='%s' action='%s' owner='%s' text='%s' bounds=(%d,%d)-(%d,%d) exit=%d status=%d",
		buttonLabel, point.x, point.y, inventoryHover->object.objectName.c_str(),
		inventoryHover->object.spritePath.c_str(), inventoryHover->object.altSpritePath.c_str(),
		inventoryHover->object.actionTag.c_str(), inventoryHover->object.currentOwnerOrRoom.c_str(),
		inventoryHover->object.inventoryTextKey.c_str(), inventoryHover->bounds.left,
		inventoryHover->bounds.top, inventoryHover->bounds.right, inventoryHover->bounds.bottom,
		InventorySystem::isExitObject(inventoryHover->object),
		InventorySystem::isStatusObject(inventoryHover->object));
}

static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	int destX = x;
	int destY = y;
	int srcX = 0;
	int srcY = 0;
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

static const CftFontResource *findStartupFontByName(const HarvesterEngine &engine, const char *fontName) {
	const Text *startupText = engine.getStartupText();
	if (!startupText || !fontName)
		return nullptr;

	for (const CftFontResource &font : startupText->getFonts()) {
		if (font.name.equalsIgnoreCase(fontName))
			return &font;
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

static Common::String resolveStartupNpcLabel(const StartupNpcRecord &npc) {
	Common::String label = !npc.entityInitArg.empty() ? npc.entityInitArg : npc.npcName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

static Common::String resolveCarriedObjectSpritePath(const StartupObjectRecord &object) {
	if (!object.altSpritePath.empty())
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

static bool shouldDispatchPickupActionOnCarryStart(const StartupObjectRecord &object) {
	if (object.actionTag.empty())
		return false;

	return !object.objectName.equalsIgnoreCase("SANDWICH") &&
		!object.objectName.equalsIgnoreCase("SANDWICH2") &&
		!object.objectName.equalsIgnoreCase("SYRINGE");
}

static bool shouldDispatchPickupActionOnDirectInventoryTransfer(const StartupObjectRecord &object) {
	if (object.actionTag.empty())
		return false;

	return !object.objectName.equalsIgnoreCase("ST_ASPRIN") &&
		!object.objectName.equalsIgnoreCase("ST_COUGHM") &&
		!object.objectName.equalsIgnoreCase("ST_VITAMN");
}

static void drawRoomPrompt(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &promptText, bool useNativeFont) {
	if (promptText.empty())
		return;

	if (useNativeFont) {
		font.drawString(&screen, promptText, 0, 462, 640, 0, Graphics::kTextAlignCenter);
		return;
	}

	font.drawString(&screen, promptText, 1, 463, 640, 0, Graphics::kTextAlignCenter);
	font.drawString(&screen, promptText, 0, 462, 640, 0xce, Graphics::kTextAlignCenter);
}

static void drawInventoryTooltip(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &tooltipText) {
	if (tooltipText.empty())
		return;

	font.drawString(&screen, tooltipText, kNativeInventoryTooltipX, kNativeInventoryTooltipY,
		font.getStringWidth(tooltipText), kNativeInventoryTooltipColor);
}

static void drawInventoryWeekday(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &weekdayText) {
	if (weekdayText.empty())
		return;

	font.drawString(&screen, weekdayText, kNativeInventoryWeekdayX, kNativeInventoryWeekdayY,
		font.getStringWidth(weekdayText), kNativeInventoryTooltipColor);
}

static void setScaledRoomPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getStartupGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

static RoomMonsterFacingAnimationRange resolveRoomMonsterFacingAnimationRange(int facing) {
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

static bool setRoomMonsterAnimation(RuntimeEntity &entity, int facing, bool walking) {
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

	const bool changed = entity.getAnimationRate() != 0 ||
		entity.getCurrentFrame() != range.idleFrame;
	entity.setAnimationRate(0);
	entity.setAnimationFrameRange(range.idleFrame, range.idleFrame, false);
	entity.setCurrentFrame(range.idleFrame);
	entity.setAnimationEnabled(false);
	return changed;
}

static bool runtimeEntityHasFrameRange(const RuntimeEntity &entity, int firstFrame, int lastFrame) {
	if (!entity.hasFrames() || firstFrame < 0 || lastFrame < firstFrame)
		return false;

	return (uint)lastFrame < entity.getFrameCount();
}

static bool resolveMonsterAttackAnimationRange(HarvesterEngine &engine,
		const RuntimeEntity &entity, int actorCenterX, int targetCenterX,
		RoomAttackAnimationRange &range) {
	static const RoomAttackAnimationRange kLeftAttackRanges[3] = {
		RoomAttackAnimationRange(0x50, 0x59, 1),
		RoomAttackAnimationRange(0x5a, 0x63, 1),
		RoomAttackAnimationRange(0x64, 0x6d, 1)
	};
	static const RoomAttackAnimationRange kRightAttackRanges[3] = {
		RoomAttackAnimationRange(0x6e, 0x77, 2),
		RoomAttackAnimationRange(0x78, 0x81, 2),
		RoomAttackAnimationRange(0x82, 0x8b, 2)
	};

	const RoomAttackAnimationRange *candidates =
		targetCenterX < actorCenterX ? kLeftAttackRanges : kRightAttackRanges;
	int availableCandidateIndices[3];
	int availableCandidateCount = 0;
	for (int i = 0; i < 3; ++i) {
		if (!runtimeEntityHasFrameRange(entity, candidates[i].firstFrame, candidates[i].lastFrame))
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

static bool resolveMonsterDeathAnimationRange(const RuntimeEntity &entity, int facing,
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
		if (!runtimeEntityHasFrameRange(entity, candidate.firstFrame, candidate.lastFrame))
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

static int stepTowardsRoomCombatInt(int current, int target, int step) {
	if (step <= 0 || current == target)
		return current;
	if (current < target)
		return MIN(current + step, target);
	return MAX(current - step, target);
}

static int computeRectAxisGap(int minA, int maxA, int minB, int maxB) {
	if (maxA <= minB)
		return minB - maxA;
	if (maxB <= minA)
		return minA - maxB;
	return 0;
}

static int computeRuntimeEntityHorizontalGap(const RuntimeEntity &left, const RuntimeEntity &right) {
	const Common::Rect leftRect = left.getScreenRect();
	const Common::Rect rightRect = right.getScreenRect();
	return computeRectAxisGap(leftRect.left, leftRect.right, rightRect.left, rightRect.right);
}

static bool doRuntimeEntityDepthExtentsOverlap(const RuntimeEntity &first, const RuntimeEntity &second) {
	const float firstZMin = first.getZ();
	const float firstZMax = firstZMin + first.getZExtent();
	const float secondZMin = second.getZ();
	const float secondZMax = secondZMin + second.getZExtent();
	return !(firstZMax < secondZMin || secondZMax < firstZMin);
}

static bool areCombatantsWithinNativeCloseRange(const StartupRoomSetupState &state,
		const RuntimeEntity &attacker, float attackerZ,
		const RuntimeEntity &target, float targetZ, int engageDistance) {
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

static bool playRandomRoomAttackSound(HarvesterEngine &engine, const Common::String &sound1,
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
	return engine.playStartupSound(*availableSounds[soundIndex]);
}

static float clampRoomDepthForEvent(const StartupRoomSetupState &state, float z) {
	return CLIP<float>(z,
		(float)MIN(state.roomMinZ, state.roomMaxZ),
		(float)MAX(state.roomMinZ, state.roomMaxZ));
}

static int mapRoomDepthToScreenYForEvent(const StartupRoomSetupState &state, float z, int fallbackY) {
	if (state.roomMaxZScreenY < 0 || state.roomMinZScreenY < state.roomMaxZScreenY)
		return fallbackY;
	if (state.roomMaxZ == state.roomMinZ)
		return CLIP<int>(fallbackY, state.roomMaxZScreenY, state.roomMinZScreenY);

	const float clampedZ = clampRoomDepthForEvent(state, z);
	const float offset = ((float)state.roomMaxZ - clampedZ) *
		(float)(state.roomMinZScreenY - state.roomMaxZScreenY) /
		(float)(state.roomMaxZ - state.roomMinZ);
	return CLIP<int>(state.roomMaxZScreenY + (int)(offset >= 0.0f ? offset + 0.5f : offset - 0.5f),
		state.roomMaxZScreenY, state.roomMinZScreenY);
}

static int roundRoomCombatFloat(float value) {
	return value >= 0.0f ? (int)(value + 0.5f) : (int)(value - 0.5f);
}

static int mapRoomDepthToScreenYForCombat(const StartupRoomSetupState &state, float z, int fallbackY) {
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

static int resolveMonsterAttackContactFrameOffset(const StartupMonsterRecord &monster) {
	return monster.monsterName.equalsIgnoreCase("MUCKEY")
		? kMuckeyAttackContactFrameOffset
		: kDefaultMonsterAttackContactFrameOffset;
}

static void clearRoomMonsterCombatState(RoomMonsterCombatState &combatState) {
	combatState = RoomMonsterCombatState();
}

static void clearRoomMonsterAttackState(RoomMonsterCombatState &combatState) {
	combatState.attackActive = false;
	combatState.attackFirstFrame = -1;
	combatState.attackLastFrame = -1;
	combatState.attackContactFrame = -1;
	combatState.attackResumeFacing = -1;
	combatState.attackSoundPlayed = false;
	combatState.attackContactResolved = false;
	combatState.attackTargetName.clear();
}

RoomSystem::RoomSystem(HarvesterEngine &engine, Common::Point &mousePos,
		InventorySystem &inventory)
	: _engine(engine), _mousePos(mousePos), _inventory(inventory) {
}

Common::Error RoomSystem::runRoomLoop(Flow &startupFlow, const Common::String &entranceName) {
	if (startupFlow.hasPendingMainMenuReturn())
		return Common::kNoError;

	Common::String currentRoomTarget = entranceName;
	while (!currentRoomTarget.empty()) {
		StartupRoomSetupState state;
		if (_engine.hasPendingLoadedStartupSaveRoomState()) {
			startupFlow.resetRoomNpcDialogueState();
			if (_engine.hasPendingLoadedDialogueStateBlob()) {
				if (!startupFlow.loadDialogueSaveStateBlob(
						_engine.getPendingLoadedDialogueStateBlob(),
						_engine.getPendingLoadedDialogueStateBlobVersion()))
					return Common::kReadingFailed;
				_engine.clearPendingLoadedDialogueStateBlob();
			}
			const StartupSaveRoomState &loadedState = _engine.getPendingLoadedStartupSaveRoomState();
			debugC(1, kDebugGeneral,
				"Harvester: applying pending loaded room state entrance='%s' room='%s' spawn=(%d,%d,%d) facing=%d music='%s'",
				loadedState.entranceName.c_str(), loadedState.roomName.c_str(),
				loadedState.playerX, loadedState.playerY, loadedState.playerZ,
				loadedState.playerFacing, loadedState.musicPath.c_str());
			if (!_engine.getStartupScript()->materializeRoomState(
					loadedState.entranceName, loadedState.roomName, state, *_engine.getResources())) {
				return Common::kReadingFailed;
			}
			state.entranceName = loadedState.entranceName;
			state.playerSpawnX = loadedState.playerX;
			state.playerSpawnY = loadedState.playerY;
			state.playerSpawnZ = loadedState.playerZ;
			state.playerFacing = loadedState.playerFacing;
			state.musicPath = loadedState.musicPath;
			currentRoomTarget = !loadedState.entranceName.empty() ? loadedState.entranceName : loadedState.roomName;
			_engine.clearPendingLoadedStartupSaveRoomState();
		} else if (!_engine.getStartupScript()->resolveRoomSetupState(
				currentRoomTarget, state, *_engine.getResources())) {
			return Common::kReadingFailed;
		}

		Common::Error transitionError = startupFlow.beginRoomSetupTransition();
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;

		StartupRoomSceneResources scene;
		if (!loadRoomSceneResources(state, *_engine.getResources(), scene))
			return Common::kReadingFailed;

		auto getActiveScreen = [&]() -> Graphics::Screen * {
			return _engine.getScreen();
		};
		Graphics::Screen *screen = getActiveScreen();
		const Art *art = _engine.getStartupArt();
		const Graphics::Font *bodyFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		const CftFontResource *promptFontResource = findStartupFontByName(_engine, "MEDFONT1");
		const CftFontResource *inventoryTooltipFontResource = findStartupFontByName(_engine, "TEXTFONT");
		Common::ScopedPtr<HarvesterCftFont> promptCftFont;
		Common::ScopedPtr<HarvesterCftFont> inventoryTooltipCftFont;
		const Graphics::Font *promptFont = bodyFont;
		const Graphics::Font *inspectFont = bodyFont;
		const Graphics::Font *inventoryTooltipFont = bodyFont;
		bool useNativePromptFont = false;
		bool useNativeInspectFont = false;
		if (promptFontResource) {
			promptCftFont.reset(new HarvesterCftFont(*promptFontResource));
			if (promptCftFont->isValid()) {
				promptFont = promptCftFont.get();
				useNativePromptFont = true;
			}
		}
		if (inventoryTooltipFontResource) {
			inventoryTooltipCftFont.reset(new HarvesterCftFont(*inventoryTooltipFontResource));
			if (inventoryTooltipCftFont->isValid()) {
				inventoryTooltipFont = inventoryTooltipCftFont.get();
				inspectFont = inventoryTooltipCftFont.get();
				useNativeInspectFont = true;
			}
		}
		if (!screen || !art || !bodyFont || !promptFont || !inventoryTooltipFont || !inspectFont)
			return Common::kNoError;
		RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();

		if (!startupFlow.populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return Common::kReadingFailed;
		if (runtimeEntities) {
			for (const StartupMonsterRecord &monster : scene.state.roomMonsters) {
				RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(monster.monsterName);
				if (entity)
					Monster::applyAnimation(*entity, monster);
			}
			runtimeEntities->pauseTimerCountdowns();
		}

		transitionError = startupFlow.waitForRoomSetupTransitionHold();
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;

		logScenePaletteSummary("room setup stub palette", scene, 0.0f);
		drawRoomScene(_engine, *screen, scene, 0.0f);
		screen->makeAllDirty();
		screen->update();

		logScenePaletteSummary("room setup fade target", scene, scene.targetPaletteBrightness);
		transitionError = startupFlow.fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;
		if (runtimeEntities)
			runtimeEntities->resumeTimerCountdowns();

		startupFlow.resetCursorAnimationSequence();
		startupFlow.executeStartupAudioCommands(scene.state.audioCommands);
		if (!scene.state.musicPath.empty())
			(void)_engine.playStartupMusic(scene.state.musicPath);
		StartupRoomPlayerState playerState;
		playerState.entity = runtimeEntities ? runtimeEntities->findSceneEntityByName("PLAYER") : nullptr;
		playerState.centerX = state.playerSpawnX;
		playerState.bottomY = state.playerSpawnY;
		playerState.z = (float)state.playerSpawnZ;
		playerState.facing = state.playerFacing;
		playerState.combatLoadout =
			_engine.getStartupScript() ? _engine.getStartupScript()->getPlayerCombatLoadout() : 0;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		Common::Array<RoomMonsterCombatState> monsterCombatStates;
		monsterCombatStates.resize(scene.state.roomMonsters.size());
		Common::Array<RoomCombatEffectState> combatEffectStates;
		uint nextCombatEffectId = 0;
		StartupResolvedText inspectText;
		bool showingInspectText = false;
		bool inspectCanDismiss = false;
		bool moveLeft = false;
		bool moveRight = false;
		bool moveUp = false;
		bool moveDown = false;
		bool attackModifierHeld = false;
		Common::String pendingRegionName;
		Common::String pendingRoomChange;
		uint32 lastLeftButtonReleaseTick = 0;
		uint32 nextKeyboardAttackAllowedTick = 0;
		StartupRoomIdleAnimationState idleState;
		bool needsRedraw = true;
		Common::String carriedRoomItemName;
		Common::String carriedRoomItemLabel;
		IndexedBitmap carriedRoomItemBitmap;
		auto hasCarriedRoomItem = [&]() {
			return !carriedRoomItemName.empty();
		};
		auto clearCarriedRoomItem = [&]() {
			carriedRoomItemName.clear();
			carriedRoomItemLabel.clear();
			carriedRoomItemBitmap = IndexedBitmap();
		};
		auto hideSceneObject = [&](const Common::String &objectName, const Common::String *ownerOrRoom) {
			StartupObjectRecord *sceneObject = findSceneObjectByName(scene.sceneObjects, objectName);
			if (!sceneObject)
				return;

			sceneObject->visible = false;
			sceneObject->runtimeVisible = false;
			if (ownerOrRoom)
				sceneObject->currentOwnerOrRoom = *ownerOrRoom;
		};
		auto removeSceneEntityByName = [&](const Common::String &entityName) {
			if (!runtimeEntities)
				return;

			RuntimeEntity *entity = runtimeEntities->detachSceneEntityByName(entityName);
			delete entity;
		};
		auto resetIdleState = [&]() {
			idleState = StartupRoomIdleAnimationState();
			idleState.activityTick = Player::getRuntimeClockTicks();
			idleState.resetTick = idleState.activityTick;
			Player::updateIdleTrigger(idleState);
		};
		auto notePlayerActivity = [&]() {
			idleState.activityTick = Player::getRuntimeClockTicks();
			Player::updateIdleTrigger(idleState);
		};
		auto cancelPlayerAttackAnimation = [&]() {
			if (!playerState.attackActive)
				return false;

			playerState.attackActive = false;
			playerState.attackFirstFrame = -1;
			playerState.attackLastFrame = -1;
			playerState.attackContactFrame = -1;
			if (!playerState.entity)
				return false;
			playerState.attackSoundPlayed = false;
			playerState.attackSoundFrame = -1;
			playerState.attackContactResolved = false;
			playerState.attackTargetName.clear();
			playerState.attackTargetClassId = -1;
			const int resumeFacing = playerState.attackResumeFacing >= 0
				? playerState.attackResumeFacing
				: (playerState.facing >= 0 ? playerState.facing : 0);
			playerState.attackResumeFacing = -1;
			return Player::setIdleAnimation(playerState, resumeFacing);
		};
		auto syncCurrentRoomRuntimeState = [&]() {
			Script *startupScript = _engine.getStartupScript();
			if (!startupScript)
				return;

			if (runtimeEntities) {
				for (StartupAnimRecord &anim : scene.state.roomAnimations) {
					RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(anim.animName);
					if (!entity)
						continue;
					(void)startupScript->syncRuntimeAnimState(anim.animName,
						entity->isAnimationEnabled(), entity->isVisible(), entity->getCurrentFrame());
				}
			}
			for (const StartupMonsterRecord &monster : scene.state.roomMonsters)
				(void)startupScript->syncRuntimeMonsterRecord(monster);
			if (!runtimeEntities)
				return;

			for (StartupTimerRecord &timer : scene.state.roomTimers) {
				RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(timer.timerName);
				if (!entity)
					continue;
				timer.currentValue = entity->getTimerCurrentValue();
				timer.enabled = entity->isTimerEnabled();
				(void)startupScript->syncRuntimeTimerRecord(timer);
			}
		};
		auto captureCurrentSaveState = [&]() {
			syncCurrentRoomRuntimeState();
			const int facing = playerState.facing >= 0 ? playerState.facing : scene.state.playerFacing;
			_engine.captureCurrentStartupSaveRoomState(scene.state.entranceName, scene.state.roomName,
				playerState.centerX, playerState.bottomY, (int)playerState.z, facing,
				_engine.getStartupMusicPath());
		};
		auto getSceneObjectBounds = [&](const StartupObjectRecord &object) {
			if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
				return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);
			return Common::Rect();
		};
		auto resolveSceneObjectSpritePathLocal = [&](const StartupObjectRecord &object) {
			const bool atInitialPlacement = object.currentX == object.initialX &&
				object.currentY == object.initialY &&
				object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
			if (!object.altSpritePath.empty() &&
					(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)))
				return object.altSpritePath;
			return object.spritePath;
		};
		auto isInteractiveSceneHotspot = [&](const StartupObjectRecord &object) {
			if (object.operatable || !object.actionTag.empty())
				return true;

			Script *startupScript = _engine.getStartupScript();
			if (!startupScript)
				return false;
			if (startupScript->isPickupObject(object))
				return true;

			StartupResolvedText inspectText;
			return startupScript->resolveObjectInspectText(object, inspectText);
		};
		auto resolveSceneObjectClassLocal = [&](const StartupObjectRecord &object,
				const RuntimeEntity *entity) {
			if (entity) {
				return object.initialX == 0 && object.initialY == 0 &&
					entity->getBoundsWidth() == 640 && entity->getBoundsHeight() == 480
					? kRuntimeEntityClassBackground
					: kRuntimeEntityClassObject;
			}

			return isInteractiveSceneHotspot(object)
				? kRuntimeEntityClassRectHotspot
				: kRuntimeEntityClassDisabledHotspot;
		};
		auto sameRect = [](const Common::Rect &lhs, const Common::Rect &rhs) {
			return lhs.left == rhs.left && lhs.top == rhs.top &&
				lhs.right == rhs.right && lhs.bottom == rhs.bottom;
		};
		auto spawnSceneObjectEntityFromRecord = [&](const StartupObjectRecord &object) {
			if (!runtimeEntities)
				return;

			const Common::String spritePath = resolveSceneObjectSpritePathLocal(object);
			const Common::Rect hotspotBounds = getSceneObjectBounds(object);
			RuntimeEntity *entity = nullptr;
			if (!spritePath.empty() && spritePath.hasSuffixIgnoreCase(".BM")) {
				entity = runtimeEntities->spawnSceneBitmapEntity(object.objectName, spritePath,
					Common::Point(object.currentX, object.currentY), (float)object.currentZ);
			} else if (!hotspotBounds.isEmpty()) {
				entity = runtimeEntities->spawnSceneHotspotEntity(object.objectName, hotspotBounds,
					(float)object.currentZ);
			}
			if (!entity)
				return;

			entity->setClassId(resolveSceneObjectClassLocal(
				object, entity->hasFrames() ? entity : nullptr));
			entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
			entity->setZExtent((float)object.zExtent);
		};
		auto getSceneRegionBounds = [](const StartupRegionRecord &region) {
			if (region.right > region.left && region.bottom > region.top)
				return Common::Rect(region.left, region.top, region.right + 1, region.bottom + 1);
			return Common::Rect();
		};
		auto spawnSceneRegionEntityFromRecord = [&](const StartupRegionRecord &region) {
			if (!runtimeEntities)
				return;

			const Common::Rect bounds = getSceneRegionBounds(region);
			if (bounds.isEmpty())
				return;

			RuntimeEntity *entity = runtimeEntities->spawnSceneHotspotEntity(
				region.regionName, bounds, (float)region.minZ);
			if (!entity)
				return;

			entity->setClassId(kRuntimeEntityClassRectHotspot19);
			entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
			entity->setZExtent((float)MAX(0, region.maxZ - region.minZ));
			if (!region.cursorEnabled)
				entity->setHitTestMode(kRuntimeEntityHitTestNone);
		};
		auto findRoomAnimByNameConst = [](const Common::Array<StartupAnimRecord> &anims,
				const Common::String &animName) -> const StartupAnimRecord * {
			for (const StartupAnimRecord &anim : anims) {
				if (anim.animName.equalsIgnoreCase(animName))
					return &anim;
			}
			return nullptr;
		};
		auto findRoomNpcByNameConst = [](const Common::Array<StartupNpcRecord> &npcs,
				const Common::String &npcName) -> const StartupNpcRecord * {
			for (const StartupNpcRecord &npc : npcs) {
				if (npc.npcName.equalsIgnoreCase(npcName))
					return &npc;
			}
			return nullptr;
		};
		auto findRoomMonsterByNameConst = [](const Common::Array<StartupMonsterRecord> &monsters,
				const Common::String &monsterName) -> const StartupMonsterRecord * {
			for (const StartupMonsterRecord &monster : monsters) {
				if (monster.monsterName.equalsIgnoreCase(monsterName))
					return &monster;
			}
			return nullptr;
		};
		auto shouldSpawnRoomNpcEntity = [](const StartupNpcRecord &npc) {
			return npc.visible && !npc.deathOrMonsterfyFlag;
		};
		auto shouldSpawnRoomMonsterEntity = [](const StartupMonsterRecord &monster) {
			return monster.visible;
		};
		auto sameNpcEntityState = [&](const StartupNpcRecord &lhs, const StartupNpcRecord &rhs) {
			return lhs.posX == rhs.posX &&
				lhs.posY == rhs.posY &&
				lhs.posZ == rhs.posZ &&
				lhs.frameDelay == rhs.frameDelay &&
				lhs.visible == rhs.visible &&
				lhs.deathOrMonsterfyFlag == rhs.deathOrMonsterfyFlag &&
				lhs.modelPath.equalsIgnoreCase(rhs.modelPath);
		};
		auto sameMonsterEntityState = [&](const StartupMonsterRecord &lhs, const StartupMonsterRecord &rhs) {
			return lhs.posX == rhs.posX &&
				lhs.posY == rhs.posY &&
				lhs.posZ == rhs.posZ &&
				lhs.facing == rhs.facing &&
				lhs.active == rhs.active &&
				lhs.visible == rhs.visible &&
				lhs.modelPath.equalsIgnoreCase(rhs.modelPath);
		};
		auto applyRoomNpcPlacement = [&](RuntimeEntity &entity, const StartupNpcRecord &npc) {
			int width = 0;
			int height = 0;
			int xOffset = 0;
			int yOffset = 0;
			if (!entity.getCurrentFrameMetrics(width, height, xOffset, yOffset))
				return false;

			const float renderZ =
				(float)npc.posZ - floorf(MAX<float>(entity.getZExtent(), 0.0f) * 0.5f);
			entity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
			entity.setPosition(npc.posX - xOffset - width / 2, npc.posY - height - yOffset, renderZ);
			return true;
		};
		auto spawnSceneNpcEntityFromRecord = [&](const StartupNpcRecord &npc) -> RuntimeEntity * {
			if (!runtimeEntities || !shouldSpawnRoomNpcEntity(npc))
				return nullptr;

			RuntimeEntity *entity = runtimeEntities->spawnSceneActorEntity(
				npc.npcName, npc.modelPath, Common::Point(npc.posX, npc.posY), (float)npc.posZ, 0);
			if (!entity)
				return nullptr;

			entity->setClassId(kRuntimeEntityClassNpc);
			entity->setHitTestMode(kRuntimeEntityHitTestOpaquePixels);
			entity->setAnimationFrameRange(0, MIN(entity->getLastFrame(), kRoomNpcAmbientLastFrame), true);
			entity->setAnimationRate(npc.frameDelay > 0 ? npc.frameDelay : 0);
			entity->setVisible(true);
			if (!applyRoomNpcPlacement(*entity, npc)) {
				removeSceneEntityByName(npc.npcName);
				return nullptr;
			}
			runtimeEntities->reinsertSceneEntity(entity);
			return entity;
		};
		auto spawnSceneMonsterEntityFromRecord = [&](const StartupRoomSetupState &roomState,
				const StartupMonsterRecord &monster) -> RuntimeEntity * {
			if (!runtimeEntities || !shouldSpawnRoomMonsterEntity(monster))
				return nullptr;

			RuntimeEntity *entity = runtimeEntities->spawnSceneActorEntity(
				monster.monsterName, monster.modelPath,
				Common::Point(monster.posX, monster.posY), (float)monster.posZ,
				Monster::resolveFacingFrame(monster.facing));
			if (!entity)
				return nullptr;

			entity->setClassId(kRuntimeEntityClassMonster);
			entity->setHitTestMode(kRuntimeEntityHitTestNone);
			entity->setVisible(monster.visible);
			Monster::applyAnimation(*entity, monster);
			if (!applyRoomActorPlacement(roomState, *entity, monster.posX, monster.posY, (float)monster.posZ)) {
				removeSceneEntityByName(monster.monsterName);
				return nullptr;
			}
			runtimeEntities->reinsertSceneEntity(entity);
			return entity;
		};
		auto resolveCombatEffectAnchor = [&](const RuntimeEntity &targetEntity,
				Common::Point &anchorPoint, float &renderZ) {
			const Common::Rect targetRect = targetEntity.getScreenRect();
			anchorPoint.x = targetRect.left + targetRect.width() / 2;
			anchorPoint.y = targetRect.top + targetRect.height() / 3;
			renderZ = targetEntity.getZ() - kNativeCombatDamageEffectRenderZBias;
		};
		auto spawnCombatDamageEffect = [&](RuntimeEntity &sourceEntity,
				const Common::String &followTargetName) {
			if (!runtimeEntities)
				return false;

			RoomCombatEffectState effectState;
			effectState.entityName = Common::String::format("__combat_blood_%u", nextCombatEffectId++);
			effectState.followTargetName = followTargetName;
			resolveCombatEffectAnchor(sourceEntity, effectState.anchorPoint, effectState.renderZ);

			RuntimeEntity *effectEntity = runtimeEntities->spawnSceneAnimationEntity(
				effectState.entityName, kNativeCombatDamageEffectResourcePath, effectState.anchorPoint,
				effectState.renderZ, kNativeCombatDamageEffectAnimationRate, true, true, false,
				false, false, 0);
			if (!effectEntity)
				return false;

			combatEffectStates.push_back(effectState);
			return true;
		};
		auto syncCombatDamageEffects = [&]() {
			if (!runtimeEntities)
				return false;

			bool changed = false;
			for (RoomCombatEffectState &effectState : combatEffectStates) {
				RuntimeEntity *effectEntity = runtimeEntities->findSceneEntityByName(effectState.entityName);
				if (!effectEntity)
					continue;

				if (!effectState.followTargetName.empty()) {
					RuntimeEntity *targetEntity =
						runtimeEntities->findSceneEntityByName(effectState.followTargetName);
					if (targetEntity && targetEntity->isVisible()) {
						resolveCombatEffectAnchor(*targetEntity, effectState.anchorPoint, effectState.renderZ);
					}
				}

				if (effectEntity->getX() != effectState.anchorPoint.x ||
						effectEntity->getY() != effectState.anchorPoint.y ||
						fabsf(effectEntity->getZ() - effectState.renderZ) > 0.0001f) {
					effectEntity->setPosition(
						effectState.anchorPoint.x, effectState.anchorPoint.y, effectState.renderZ);
					runtimeEntities->reinsertSceneEntity(effectEntity);
					changed = true;
				}
			}

			return changed;
		};
		auto pruneCombatDamageEffects = [&]() {
			bool removedAny = false;
			for (uint i = 0; i < combatEffectStates.size();) {
				const Common::String effectName = combatEffectStates[i].entityName;
				RuntimeEntity *effectEntity = runtimeEntities
					? runtimeEntities->findSceneEntityByName(effectName)
					: nullptr;
				if (!effectEntity) {
					combatEffectStates.remove_at(i);
					continue;
				}

				const bool finished =
					!effectEntity->isAnimationEnabled() &&
					effectEntity->getCurrentFrame() == effectEntity->getLastFrame();
				if (finished && combatEffectStates[i].finished) {
					removeSceneEntityByName(effectName);
					combatEffectStates.remove_at(i);
					removedAny = true;
					continue;
				}

				combatEffectStates[i].finished = finished;
				++i;
			}

			return removedAny;
		};
		auto applyCurrentRoomRuntimeMutationsInPlace = [&]() {
			Script *startupScript = _engine.getStartupScript();
			ResourceManager *resources = _engine.getResources();
			if (!startupScript || !resources)
				return false;

			const Common::Array<StartupAudioCommand> entryAudioCommands = scene.state.audioCommands;
			StartupRoomSetupState updatedState;
			if (!startupScript->materializeRoomState(
					scene.state.entranceName, scene.state.roomName, updatedState, *resources)) {
				return false;
			}
			updatedState.audioCommands = entryAudioCommands;

			StartupRoomSceneResources updatedScene;
			if (!loadRoomSceneResources(updatedState, *resources, updatedScene))
				return false;
			Common::Array<RoomMonsterCombatState> updatedMonsterCombatStates;
			updatedMonsterCombatStates.resize(updatedState.roomMonsters.size());
			for (uint i = 0; i < updatedState.roomMonsters.size(); ++i) {
				const StartupMonsterRecord &updatedMonster = updatedState.roomMonsters[i];
				for (uint j = 0; j < scene.state.roomMonsters.size() &&
						j < monsterCombatStates.size(); ++j) {
					const StartupMonsterRecord &previousMonster = scene.state.roomMonsters[j];
					if (!previousMonster.monsterName.equalsIgnoreCase(updatedMonster.monsterName))
						continue;
					if (sameMonsterEntityState(previousMonster, updatedMonster))
						updatedMonsterCombatStates[i] = monsterCombatStates[j];
					break;
				}
			}

			for (const StartupObjectRecord &object : scene.sceneObjects) {
				if (!findSceneObjectByName(updatedScene.sceneObjects, object.objectName))
					removeSceneEntityByName(object.objectName);
			}
			for (const StartupObjectRecord &object : updatedScene.sceneObjects) {
				const StartupObjectRecord *previous = findSceneObjectByName(scene.sceneObjects, object.objectName);
				const Common::String spritePath = resolveSceneObjectSpritePathLocal(object);
				const Common::Rect bounds = getSceneObjectBounds(object);
				const bool changed = !previous ||
					previous->currentX != object.currentX ||
					previous->currentY != object.currentY ||
					previous->currentZ != object.currentZ ||
					previous->zExtent != object.zExtent ||
					previous->operatable != object.operatable ||
					!previous->actionTag.equalsIgnoreCase(object.actionTag) ||
					!previous->identTextKey.equalsIgnoreCase(object.identTextKey) ||
					!resolveSceneObjectSpritePathLocal(*previous).equalsIgnoreCase(spritePath) ||
					!sameRect(getSceneObjectBounds(*previous), bounds);
				if (!changed)
					continue;

				removeSceneEntityByName(object.objectName);
				spawnSceneObjectEntityFromRecord(object);
			}

			for (const StartupRegionRecord &region : scene.sceneRegions) {
				if (!findSceneRegionByName(updatedScene.sceneRegions, region.regionName))
					removeSceneEntityByName(region.regionName);
			}
			for (const StartupRegionRecord &region : updatedScene.sceneRegions) {
				const StartupRegionRecord *previous =
					findSceneRegionByName(scene.sceneRegions, region.regionName);
				const bool changed = !previous ||
					previous->left != region.left ||
					previous->top != region.top ||
					previous->right != region.right ||
					previous->bottom != region.bottom ||
					previous->minZ != region.minZ ||
					previous->maxZ != region.maxZ ||
					previous->cursorEnabled != region.cursorEnabled;
				if (!changed)
					continue;

				removeSceneEntityByName(region.regionName);
				spawnSceneRegionEntityFromRecord(region);
			}

			for (const StartupTimerRecord &timer : scene.state.roomTimers) {
				bool found = false;
				for (const StartupTimerRecord &updatedTimer : updatedState.roomTimers) {
					if (updatedTimer.timerName.equalsIgnoreCase(timer.timerName)) {
						found = true;
						break;
					}
				}
				if (!found)
					removeSceneEntityByName(timer.timerName);
			}
			for (const StartupTimerRecord &timer : updatedState.roomTimers) {
				const StartupTimerRecord *previous = nullptr;
				for (const StartupTimerRecord &candidate : scene.state.roomTimers) {
					if (candidate.timerName.equalsIgnoreCase(timer.timerName)) {
						previous = &candidate;
						break;
					}
				}
				const bool changed = !previous ||
					previous->initialValue != timer.initialValue ||
					previous->currentValue != timer.currentValue ||
					previous->enabled != timer.enabled ||
					previous->looping != timer.looping ||
					previous->global != timer.global;
				if (!changed)
					continue;

				removeSceneEntityByName(timer.timerName);
				if (runtimeEntities) {
					(void)runtimeEntities->spawnSceneTimerEntity(
						timer.timerName, timer.initialValue, timer.currentValue,
						timer.enabled, timer.looping, timer.global);
				}
			}

			for (StartupAnimRecord &anim : updatedState.roomAnimations) {
				const StartupAnimRecord *previous =
					findRoomAnimByNameConst(scene.state.roomAnimations, anim.animName);
				const bool becameVisible = (!previous || !previous->visible) && anim.visible;
				if (becameVisible)
					anim.runtimeState = 0;

				RuntimeEntity *entity = runtimeEntities
					? runtimeEntities->findSceneEntityByName(anim.animName)
					: nullptr;
				if (!anim.visible) {
					removeSceneEntityByName(anim.animName);
					continue;
				}

				const bool needsRespawn = !entity || !previous ||
					!previous->resourcePath.equalsIgnoreCase(anim.resourcePath) ||
					previous->x != anim.x ||
					previous->y != anim.y ||
					previous->z != anim.z ||
					previous->frameDelay != anim.frameDelay ||
					previous->looping != anim.looping ||
					previous->pingPong != anim.pingPong ||
					previous->backward != anim.backward;
				if (needsRespawn) {
					removeSceneEntityByName(anim.animName);
					if (runtimeEntities) {
						entity = runtimeEntities->spawnSceneAnimationEntity(
							anim.animName, anim.resourcePath, Common::Point(anim.x, anim.y),
							(float)anim.z, anim.frameDelay, anim.active, anim.visible, anim.looping,
							anim.backward, anim.pingPong, anim.runtimeState);
					}
				} else if (entity) {
					const float renderZ =
						(float)anim.z - floorf(MAX<float>(entity->getZExtent(), 0.0f) * 0.5f);
					const bool zChanged = entity->getZ() != renderZ;
					entity->setPosition(anim.x, anim.y, renderZ);
					entity->setLooping(anim.looping);
					entity->setPingPong(anim.pingPong);
					entity->setPlayBackwards(anim.backward);
					entity->setAnimationRate(anim.frameDelay);
					entity->setVisible(anim.visible);
					entity->setAnimationEnabled(anim.active);
					if (zChanged)
						runtimeEntities->reinsertSceneEntity(entity);
				}
			}

			for (const StartupAnimRecord &anim : scene.state.roomAnimations) {
				if (!findRoomAnimByNameConst(updatedState.roomAnimations, anim.animName))
					removeSceneEntityByName(anim.animName);
			}

			for (const StartupNpcRecord &npc : scene.state.roomNpcs) {
				const StartupNpcRecord *updatedNpc =
					findRoomNpcByNameConst(updatedState.roomNpcs, npc.npcName);
				const bool keepEntity =
					updatedNpc &&
					shouldSpawnRoomNpcEntity(*updatedNpc) &&
					sameNpcEntityState(npc, *updatedNpc);
				if (!keepEntity)
					removeSceneEntityByName(npc.npcName);
			}
			for (const StartupNpcRecord &npc : updatedState.roomNpcs) {
				if (!shouldSpawnRoomNpcEntity(npc))
					continue;

				RuntimeEntity *entity = runtimeEntities
					? runtimeEntities->findSceneEntityByName(npc.npcName)
					: nullptr;
				const StartupNpcRecord *previousNpc =
					findRoomNpcByNameConst(scene.state.roomNpcs, npc.npcName);
				const bool needsRespawn =
					!entity || !previousNpc || !sameNpcEntityState(*previousNpc, npc);
				if (!needsRespawn)
					continue;

				removeSceneEntityByName(npc.npcName);
				if (!spawnSceneNpcEntityFromRecord(npc))
					return false;
			}

			for (const StartupMonsterRecord &monster : scene.state.roomMonsters) {
				const StartupMonsterRecord *updatedMonster =
					findRoomMonsterByNameConst(updatedState.roomMonsters, monster.monsterName);
				const bool keepEntity =
					updatedMonster &&
					shouldSpawnRoomMonsterEntity(*updatedMonster) &&
					sameMonsterEntityState(monster, *updatedMonster);
				if (!keepEntity)
					removeSceneEntityByName(monster.monsterName);
			}
			for (const StartupMonsterRecord &monster : updatedState.roomMonsters) {
				if (!shouldSpawnRoomMonsterEntity(monster))
					continue;

				RuntimeEntity *entity = runtimeEntities
					? runtimeEntities->findSceneEntityByName(monster.monsterName)
					: nullptr;
				const StartupMonsterRecord *previousMonster =
					findRoomMonsterByNameConst(scene.state.roomMonsters, monster.monsterName);
				const bool needsRespawn =
					!entity || !previousMonster || !sameMonsterEntityState(*previousMonster, monster);
				if (!needsRespawn)
					continue;

				removeSceneEntityByName(monster.monsterName);
				if (!spawnSceneMonsterEntityFromRecord(updatedState, monster))
					return false;
			}

			scene.state = updatedState;
			monsterCombatStates = updatedMonsterCombatStates;
			scene.sceneObjects = updatedScene.sceneObjects;
			scene.sceneAnimations = updatedScene.sceneAnimations;
			scene.sceneRegions = updatedScene.sceneRegions;
			memcpy(scene.palette, updatedScene.palette, sizeof(scene.palette));
			scene.targetPaletteBrightness = updatedScene.targetPaletteBrightness;
			captureCurrentSaveState();
			return _inventory.refresh();
		};
		resetIdleState();
		auto stopPlayerRegionInteraction = [&]() {
			moveLeft = false;
			moveRight = false;
			moveUp = false;
			moveDown = false;
			pendingRegionName.clear();
			(void)cancelPlayerAttackAnimation();
			playerState.hasMoveTarget = false;
			playerState.turnActive = false;
			playerState.turnTargetFacing = -1;
			if (playerState.entity && playerState.facing >= 0)
				(void)Player::setIdleAnimation(playerState, playerState.facing);
		};
		auto refreshCurrentScene = [&](bool preservePlayerPlacement) {
			const Common::Array<StartupAudioCommand> entryAudioCommands = scene.state.audioCommands;
			StartupRoomSetupState refreshedState;
			if (!_engine.getStartupScript()->materializeRoomState(
					scene.state.entranceName, scene.state.roomName, refreshedState, *_engine.getResources())) {
				return false;
			}

			refreshedState.audioCommands = entryAudioCommands;
			if (!loadRoomSceneResources(refreshedState, *_engine.getResources(), scene))
				return false;
			combatEffectStates.clear();
			nextCombatEffectId = 0;
			if (!startupFlow.populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
				return false;
			if (runtimeEntities) {
				for (const StartupMonsterRecord &monster : scene.state.roomMonsters) {
					RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(monster.monsterName);
					if (entity)
						Monster::applyAnimation(*entity, monster);
				}
			}

			playerState.entity = runtimeEntities ? runtimeEntities->findSceneEntityByName("PLAYER") : nullptr;
			if (playerState.entity) {
				if (!preservePlayerPlacement || playerState.facing < 0) {
					playerState.centerX = scene.state.playerSpawnX;
					playerState.bottomY = scene.state.playerSpawnY;
					playerState.z = (float)scene.state.playerSpawnZ;
					playerState.facing = scene.state.playerFacing;
				}

				const int facing = playerState.facing >= 0 ? playerState.facing : scene.state.playerFacing;
				(void)Player::setIdleAnimation(playerState, facing);
				(void)applyRoomActorPlacement(scene.state, *playerState.entity,
					playerState.centerX, playerState.bottomY, playerState.z);
			}

			playerState.hasMoveTarget = false;
			playerState.turnActive = false;
			playerState.turnTargetFacing = -1;
			playerState.attackActive = false;
			playerState.attackFirstFrame = -1;
			playerState.attackLastFrame = -1;
			playerState.attackContactFrame = -1;
			playerState.attackResumeFacing = -1;
			playerState.attackSoundPlayed = false;
			playerState.attackSoundFrame = -1;
			playerState.attackContactResolved = false;
			playerState.attackTargetName.clear();
			playerState.attackTargetClassId = -1;
			playerState.combatLoadout =
				_engine.getStartupScript() ? _engine.getStartupScript()->getPlayerCombatLoadout() : 0;
			monsterCombatStates.clear();
			monsterCombatStates.resize(scene.state.roomMonsters.size());
			pendingRegionName.clear();
			resetIdleState();
			startupFlow.resetCursorAnimationSequence();
			captureCurrentSaveState();
			return _inventory.refresh();
		};
		auto stowCarriedRoomItemToInventory = [&]() {
			if (!hasCarriedRoomItem())
				return true;

			_engine.getStartupScript()->addRuntimeObjectToInventory(carriedRoomItemName);
			clearCarriedRoomItem();
			return _inventory.refresh();
		};
		auto shouldCancelDeniedPickup = [&](const StartupInteractionResult &interaction,
				bool didTransition) {
			return !didTransition &&
				!interaction.mutatedRuntimeState &&
				!interaction.requestPlayerGotoXZ &&
				interaction.lightingCommand == kStartupLightingCommandNone &&
				!interaction.requestMainMenu &&
				interaction.dialogueNpcName.empty() &&
				interaction.dialogueContinuationTag.empty() &&
				interaction.continuationTag.empty() &&
				interaction.nextRoomName.empty() &&
				interaction.cutscenePath.empty() &&
				!interaction.modalText.value.empty();
		};
		auto restoreDeniedPickup = [&](Script &startupScript,
				const StartupObjectRecord &savedRuntimeObject, bool clearCarryState) -> Common::Error {
			if (!startupScript.syncRuntimeObjectRecord(savedRuntimeObject))
				return Common::kReadingFailed;
			if (clearCarryState)
				clearCarriedRoomItem();

			if (!applyCurrentRoomRuntimeMutationsInPlace() && !refreshCurrentScene(true))
				return Common::kReadingFailed;
			if (!_inventory.refresh())
				return Common::kReadingFailed;
			needsRedraw = true;
			return Common::kNoError;
		};
		auto openInventoryOverlay = [&]() {
			moveLeft = false;
			moveRight = false;
			moveUp = false;
			moveDown = false;
			pendingRegionName.clear();
			(void)cancelPlayerAttackAnimation();
			playerState.hasMoveTarget = false;
			playerState.turnActive = false;
			playerState.turnTargetFacing = -1;
			if (!stowCarriedRoomItemToInventory())
				return false;
			return _inventory.open();
		};
		auto captureDialogueBackdrop = [&](IndexedBitmap &dialogueBackdrop) {
			Graphics::Screen *activeScreen = getActiveScreen();
			if (!activeScreen)
				return false;

			drawRoomScene(_engine, *activeScreen, scene, scene.targetPaletteBrightness);
			return captureScreenBackdrop(*activeScreen, dialogueBackdrop);
		};
		auto runRoomExitCommands = [&]() -> Common::Error {
			StartupInteractionResult exitInteraction;
			if (!_engine.getStartupScript()->executeRoomExitCommands(scene.state.roomName, exitInteraction))
				return Common::kReadingFailed;

			for (uint exitStep = 0; exitStep < 128; ++exitStep) {
				if (!exitInteraction.musicPath.empty())
					(void)_engine.playStartupMusic(exitInteraction.musicPath);
				startupFlow.executeStartupAudioCommands(exitInteraction.audioCommands);

				if (!exitInteraction.cutscenePath.empty()) {
					FstPlayer fstPlayer(_engine);
					if (!fstPlayer.play(exitInteraction.cutscenePath))
						return Common::kReadingFailed;
				}

				if (!exitInteraction.dialogueNpcName.empty() ||
						!exitInteraction.dialogueContinuationTag.empty() ||
						!exitInteraction.modalText.value.empty() ||
						exitInteraction.lightingCommand != kStartupLightingCommandNone ||
						exitInteraction.requestPlayerGotoXZ) {
					debugC(1, kDebugScene,
						"Harvester: room exit command for '%s' produced unsupported deferred output; preserving accumulated state",
						scene.state.roomName.c_str());
				}

				if (exitInteraction.continuationTag.empty())
					return Common::kNoError;

				StartupInteractionResult continuationInteraction;
				if (!_engine.getStartupScript()->executeActionTag(
						exitInteraction.continuationTag, continuationInteraction, false)) {
					return Common::kNoError;
				}

				exitInteraction = continuationInteraction;
			}

			warning("Harvester: room exit command chain for '%s' exceeded continuation safety limit",
				scene.state.roomName.c_str());
			return Common::kNoError;
		};
		auto applyLightingCommand = [&](StartupLightingCommand lightingCommand) -> Common::Error {
			Graphics::Screen *activeScreen = getActiveScreen();
			if (!activeScreen)
				return Common::kNoError;

			switch (lightingCommand) {
			case kStartupLightingCommandNone:
				return Common::kNoError;
			case kStartupLightingCommandDim:
				drawRoomScene(_engine, *activeScreen, scene, 0.6f);
				setScaledRoomPalette(*activeScreen, scene.palette, 0.6f);
				activeScreen->makeAllDirty();
				activeScreen->update();
				return Common::kNoError;
			case kStartupLightingCommandNormal:
				drawRoomScene(_engine, *activeScreen, scene, scene.targetPaletteBrightness);
				setScaledRoomPalette(*activeScreen, scene.palette, scene.targetPaletteBrightness);
				activeScreen->makeAllDirty();
				activeScreen->update();
				return Common::kNoError;
			case kStartupLightingCommandBlack: {
				byte blackPalette[256 * 3] = { 0 };
				activeScreen->fillRect(activeScreen->getBounds(), 0);
				activeScreen->setPalette(blackPalette);
				activeScreen->makeAllDirty();
				activeScreen->update();
				return Common::kNoError;
			}
			case kStartupLightingCommandFadeIn:
				debugC(1, kDebugScene,
					"Harvester: CHANGE_LIGHTING FADE_IN has no direct room-side equivalent yet; preserving control flow");
				return Common::kNoError;
			}

			return Common::kNoError;
		};
		auto applyPlayerGotoXZ = [&](int x, int z) {
			(void)cancelPlayerAttackAnimation();
			playerState.hasMoveTarget = false;
			playerState.turnActive = false;
			playerState.turnTargetFacing = -1;
			playerState.centerX = x;
			playerState.z = clampRoomDepthForEvent(scene.state, (float)z);
			playerState.bottomY = mapRoomDepthToScreenYForEvent(scene.state, playerState.z, playerState.bottomY);
			if (playerState.entity) {
				const int facing = playerState.facing >= 0 ? playerState.facing : scene.state.playerFacing;
				if (facing >= 0)
					(void)Player::setIdleAnimation(playerState, facing);
				(void)applyRoomActorPlacement(scene.state, *playerState.entity,
					playerState.centerX, playerState.bottomY, playerState.z);
			}
			resetIdleState();
			captureCurrentSaveState();
		};
		auto runModalShowText = [&](const StartupResolvedText &modalText) -> Common::Error {
			Graphics::Screen *activeScreen = getActiveScreen();
			if (!activeScreen || !art || !inspectFont)
				return Common::kNoError;
			if (!resolveInspectTextboxBitmap(*art, modalText)) {
				debug(1, "Harvester: unsupported SHOW_TEXT textbox '%s' text='%s'",
					modalText.boxName.c_str(), modalText.value.c_str());
				return Common::kNoError;
			}

			IndexedBitmap backdrop;
			if (!captureDialogueBackdrop(backdrop))
				return Common::kReadingFailed;

			Graphics::FrameLimiter limiter(g_system, 60);
			bool initialInputReleased = false;
			bool dismissPressed = false;
			uint frameCount = 0;

			while (!_engine.shouldQuit()) {
				Common::Event event;
				Common::Error result = Common::kNoError;
				while (g_system->getEventManager()->pollEvent(event)) {
					if (startupFlow.handleSystemEvent(event, result))
						return result;

					if (!initialInputReleased) {
						if (event.type == Common::EVENT_LBUTTONUP ||
								event.type == Common::EVENT_RBUTTONUP ||
								event.type == Common::EVENT_KEYUP) {
							initialInputReleased = true;
						}
						continue;
					}

					if (!dismissPressed) {
						if (event.type == Common::EVENT_LBUTTONDOWN ||
								event.type == Common::EVENT_RBUTTONDOWN) {
							dismissPressed = true;
						} else if (event.type == Common::EVENT_KEYDOWN) {
							return Common::kNoError;
						}
						continue;
					}

					if (event.type == Common::EVENT_LBUTTONUP ||
							event.type == Common::EVENT_RBUTTONUP) {
						return Common::kNoError;
					}
				}

				if (!initialInputReleased && frameCount++ > 0)
					initialInputReleased = true;

				blitBitmap(*activeScreen, backdrop, 0, 0);
				setScaledRoomPalette(*activeScreen, scene.palette, scene.targetPaletteBrightness);
				drawRoomInspectText(*activeScreen, *art, *inspectFont, modalText, useNativeInspectFont);
				activeScreen->makeAllDirty();
				activeScreen->update();

				limiter.delayBeforeSwap();
				limiter.startFrame();
			}

			return Common::kNoError;
		};
		struct InteractionProcessor {
			HarvesterEngine &engine;
			Flow &startupFlow;
			StartupRoomSceneResources &scene;
			StartupRoomPlayerState &playerState;
			Common::String &pendingRegionName;
			Common::String &pendingRoomChange;
			decltype(refreshCurrentScene) &refreshCurrentSceneFn;
			decltype(applyCurrentRoomRuntimeMutationsInPlace) &applyCurrentRoomRuntimeMutationsInPlaceFn;
			decltype(captureDialogueBackdrop) &captureDialogueBackdropFn;
			decltype(runRoomExitCommands) &runRoomExitCommandsFn;
			decltype(applyLightingCommand) &applyLightingCommandFn;
			decltype(applyPlayerGotoXZ) &applyPlayerGotoXZFn;
			decltype(runModalShowText) &runModalShowTextFn;
			decltype(resetIdleState) &resetIdleStateFn;

			Common::Error handleInteractionResult(const StartupInteractionResult &interaction,
					bool &didTransition, const Common::String &usedItemName) {
				didTransition = false;

				playerState.hasMoveTarget = false;
				playerState.turnActive = false;
				playerState.turnTargetFacing = -1;
				pendingRegionName.clear();

				if (interaction.requestMainMenu) {
					engine.stopStartupMusic();
					engine.stopStartupSound();
					if (!interaction.deathFlicPath.empty()) {
						FstPlayer fstPlayer(engine);
						if (!fstPlayer.play(interaction.deathFlicPath))
							return Common::kReadingFailed;
					}

					startupFlow.requestMainMenuReturn();
					return Common::kNoError;
				}

				Common::String restoreMusicPath = engine.getStartupMusicPath();
				if (!interaction.musicPath.empty()) {
					(void)engine.playStartupMusic(interaction.musicPath);
					restoreMusicPath = engine.getStartupMusicPath();
				}
				startupFlow.executeStartupAudioCommands(interaction.audioCommands);

				StartupRoomTransitionKind roomTransition = interaction.roomTransition;
				if (roomTransition == kStartupRoomTransitionNone && !interaction.nextRoomName.empty())
					roomTransition = kStartupRoomTransitionCloseup;

				if (!interaction.nextRoomName.empty() &&
						roomTransition == kStartupRoomTransitionChangeRoom) {
					Common::String resolvedTransitionTarget = interaction.nextRoomName;
					Common::Error resolveError = startupFlow.resolveRoomTransitionTarget(
						interaction.nextRoomName, resolvedTransitionTarget);
					if (resolveError.getCode() != Common::kNoError)
						return resolveError;
					if (resolvedTransitionTarget.empty())
						return Common::kNoError;

					Common::Error exitError = runRoomExitCommandsFn();
					if (exitError.getCode() != Common::kNoError)
						return exitError;

					// Native CHANGE_ROOM queues a room handoff for the live loop instead of nesting.
					pendingRoomChange = resolvedTransitionTarget;
					didTransition = true;
				} else if (!interaction.nextRoomName.empty()) {
					Common::Error exitError = runRoomExitCommandsFn();
					if (exitError.getCode() != Common::kNoError)
						return exitError;

					Common::Error roomError = startupFlow.runRoomLoop(interaction.nextRoomName);
					if (startupFlow.hasPendingMainMenuReturn())
						return Common::kNoError;
					if (roomError.getCode() != Common::kReadingFailed &&
						roomError.getCode() != Common::kNoError) {
						return roomError;
					}

					if (!refreshCurrentSceneFn(true))
						return Common::kReadingFailed;

					startupFlow.executeStartupAudioCommands(scene.state.audioCommands);
					if (!restoreMusicPath.empty())
						(void)engine.playStartupMusic(restoreMusicPath);
					else
						engine.stopStartupMusic();
					didTransition = true;
				} else if (interaction.mutatedRuntimeState) {
					if (!applyCurrentRoomRuntimeMutationsInPlaceFn() &&
							!refreshCurrentSceneFn(true)) {
						return Common::kReadingFailed;
					}
				}

				if (didTransition)
					return Common::kNoError;

				if (interaction.requestPlayerGotoXZ)
					applyPlayerGotoXZFn(interaction.playerGotoX, interaction.playerGotoZ);

				if (interaction.lightingCommand != kStartupLightingCommandNone) {
					Common::Error lightingError = applyLightingCommandFn(interaction.lightingCommand);
					if (lightingError.getCode() != Common::kNoError)
						return lightingError;
				}

				if (!interaction.cutscenePath.empty()) {
					FstPlayer fstPlayer(engine);
					if (!fstPlayer.play(interaction.cutscenePath))
						return Common::kReadingFailed;
				}

				if (!interaction.modalText.value.empty()) {
					Common::Error modalError = runModalShowTextFn(interaction.modalText);
					if (modalError.getCode() != Common::kNoError)
						return modalError;
				}

				if (!interaction.dialogueNpcName.empty()) {
					Common::Error dialogueError = runScriptedDialogue(
						interaction.dialogueNpcName, usedItemName, interaction.dialogueContinuationTag,
						didTransition);
					if (dialogueError.getCode() != Common::kNoError)
						return dialogueError;
				}

				if (didTransition)
					return Common::kNoError;

				if (!interaction.continuationTag.empty()) {
					StartupInteractionResult continuationInteraction;
					if (engine.getStartupScript()->executeActionTag(
							interaction.continuationTag, continuationInteraction)) {
						Common::Error interactionError =
							handleInteractionResult(continuationInteraction, didTransition, usedItemName);
						if (interactionError.getCode() != Common::kNoError)
							return interactionError;
					}
				}

				return Common::kNoError;
			}

			Common::Error runScriptedDialogue(const Common::String &npcName, const Common::String &usedItemName,
					const Common::String &continuationTag, bool &didTransition) {
				didTransition = false;

				const StartupNpcRecord *dialogueNpc = engine.getStartupScript()->findRuntimeNpcRecord(npcName);
				if (dialogueNpc) {
					IndexedBitmap dialogueBackdrop;
					if (!captureDialogueBackdropFn(dialogueBackdrop))
						return Common::kReadingFailed;

					Common::Error dialogueError = startupFlow.runRoomNpcDialogue(
						dialogueBackdrop, scene.palette, scene.targetPaletteBrightness, *dialogueNpc,
						usedItemName);
					if (dialogueError.getCode() != Common::kNoError)
						return dialogueError;
				} else {
					warning("Harvester: unresolved startup dialogue npc '%s' while processing room dialogue",
						npcName.c_str());
				}

				StartupInteractionResult dialogueInteraction;
				bool abortRemainingCommandChain = false;
				if (startupFlow.takeQueuedDialogueInteraction(dialogueInteraction)) {
					abortRemainingCommandChain = dialogueInteraction.abortRemainingCommandChain;
					Common::Error interactionError =
						handleInteractionResult(dialogueInteraction, didTransition, usedItemName);
					if (interactionError.getCode() != Common::kNoError)
						return interactionError;
					if (startupFlow.hasPendingMainMenuReturn())
						return Common::kNoError;
				}
				if (!didTransition && !abortRemainingCommandChain && !continuationTag.empty()) {
					StartupInteractionResult continuationInteraction;
					if (engine.getStartupScript()->executeActionTag(continuationTag, continuationInteraction)) {
						Common::Error interactionError =
							handleInteractionResult(continuationInteraction, didTransition, usedItemName);
						if (interactionError.getCode() != Common::kNoError)
							return interactionError;
						if (startupFlow.hasPendingMainMenuReturn())
							return Common::kNoError;
					}
				}

				startupFlow.resetCursorAnimationSequence();
				resetIdleStateFn();
				return Common::kNoError;
			}
		};
		InteractionProcessor interactionProcessor = {
			_engine, startupFlow, scene, playerState, pendingRegionName, pendingRoomChange,
			refreshCurrentScene, applyCurrentRoomRuntimeMutationsInPlace, captureDialogueBackdrop,
			runRoomExitCommands, applyLightingCommand, applyPlayerGotoXZ, runModalShowText,
			resetIdleState
		};
	auto findRoomNpcRecordByName = [&](const Common::String &npcName) -> StartupNpcRecord * {
		for (StartupNpcRecord &npc : scene.state.roomNpcs) {
			if (npc.npcName.equalsIgnoreCase(npcName))
				return &npc;
		}

		return nullptr;
	};
	auto findRoomMonsterRecordByName = [&](const Common::String &monsterName) -> StartupMonsterRecord * {
		for (StartupMonsterRecord &monster : scene.state.roomMonsters) {
			if (monster.monsterName.equalsIgnoreCase(monsterName))
				return &monster;
		}

		return nullptr;
	};
	auto findRoomMonsterIndexByName = [&](const Common::String &monsterName) -> int {
		for (uint i = 0; i < scene.state.roomMonsters.size(); ++i) {
			if (scene.state.roomMonsters[i].monsterName.equalsIgnoreCase(monsterName))
				return (int)i;
		}

		return -1;
	};
	auto findSceneRuntimeEntity = [&](const Common::String &entityName) -> RuntimeEntity * {
		return runtimeEntities ? runtimeEntities->findSceneEntityByName(entityName) : nullptr;
	};
	auto findMonsterTargetAtPoint = [&](const Common::Point &point) -> StartupMonsterRecord * {
		StartupMonsterRecord *bestMonster = nullptr;
		int bestZ = -0x7fffffff;
		int bestTop = -0x7fffffff;
		for (StartupMonsterRecord &monster : scene.state.roomMonsters) {
			if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
				continue;

			RuntimeEntity *entity = findSceneRuntimeEntity(monster.monsterName);
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
	};
	auto findOverlappingMonsterTarget = [&]() -> StartupMonsterRecord * {
		if (!playerState.entity)
			return nullptr;

		for (StartupMonsterRecord &monster : scene.state.roomMonsters) {
			if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
				continue;

			RuntimeEntity *entity = findSceneRuntimeEntity(monster.monsterName);
			if (entity && areCombatantsWithinNativeCloseRange(scene.state,
					*playerState.entity, playerState.z, *entity, (float)monster.posZ, monster.engageDistance))
				return &monster;
		}

		return nullptr;
	};
	auto findOverlappingNpcTarget = [&]() -> StartupNpcRecord * {
		if (!playerState.entity)
			return nullptr;

		for (StartupNpcRecord &npc : scene.state.roomNpcs) {
			if (!npc.visible || npc.deathOrMonsterfyFlag)
				continue;

			RuntimeEntity *entity = findSceneRuntimeEntity(npc.npcName);
			if (entity && areCombatantsWithinNativeCloseRange(scene.state,
					*playerState.entity, playerState.z, *entity, entity->getZ(), 0))
				return &npc;
		}

		return nullptr;
	};
	auto playerAttackCanReachTarget = [&](RuntimeEntity *targetEntity, int engageDistance = 0) {
		if (!playerState.entity || !targetEntity)
			return false;
		if (Player::isProjectileCombatLoadout(playerState.combatLoadout))
			return true;

		return areCombatantsWithinNativeCloseRange(scene.state,
			*playerState.entity, playerState.z, *targetEntity, targetEntity->getZ(), engageDistance);
	};
	auto describeCombatTargetClass = [](int classId) -> const char * {
		switch (classId) {
		case kRuntimeEntityClassNpc:
			return "npc";
		case kRuntimeEntityClassMonster:
			return "monster";
		case kRuntimeEntityClassPlayer:
			return "player";
		default:
			return "none";
		}
	};
	auto handleCombatInteraction = [&](StartupInteractionResult interaction) -> Common::Error {
		bool didTransition = false;
		Common::Error interactionError =
			interactionProcessor.handleInteractionResult(interaction, didTransition, Common::String());
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;

		needsRedraw = true;
		return Common::kNoError;
	};
	auto capturePlayerAttackTarget = [&]() {
		playerState.attackTargetName.clear();
		playerState.attackTargetClassId = -1;

		const StartupRoomHoverState hoverState = resolveRoomHoverState(
			_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions, _mousePos);
		if (hoverState.npc) {
			playerState.attackTargetName = hoverState.npc->npcName;
			playerState.attackTargetClassId = kRuntimeEntityClassNpc;
			debugC(1, kDebugCombat,
				"Harvester: combat player target capture class='npc' name='%s' cursor=(%d,%d)",
				playerState.attackTargetName.c_str(), _mousePos.x, _mousePos.y);
			return;
		}

		if (StartupMonsterRecord *monster = findMonsterTargetAtPoint(_mousePos)) {
			playerState.attackTargetName = monster->monsterName;
			playerState.attackTargetClassId = kRuntimeEntityClassMonster;
			debugC(1, kDebugCombat,
				"Harvester: combat player target capture class='monster' name='%s' cursor=(%d,%d)",
				playerState.attackTargetName.c_str(), _mousePos.x, _mousePos.y);
			return;
		}

		debugC(1, kDebugCombat,
			"Harvester: combat player target capture class='none' cursor=(%d,%d)",
			_mousePos.x, _mousePos.y);
	};
	auto capturePlayerAttackTargetForKeyboard = [&](bool attackLeft, bool attackRight, bool attackUp, bool attackDown) {
		playerState.attackTargetName.clear();
		playerState.attackTargetClassId = -1;
		if (!playerState.entity)
			return;

		enum KeyboardAttackSide {
			kKeyboardAttackSideNone,
			kKeyboardAttackSideLeft,
			kKeyboardAttackSideRight
		};
		enum KeyboardAttackBand {
			kKeyboardAttackBandMid,
			kKeyboardAttackBandUpper,
			kKeyboardAttackBandLower
		};

		KeyboardAttackSide attackSide = kKeyboardAttackSideNone;
		KeyboardAttackBand attackBand = kKeyboardAttackBandMid;
		if (attackRight) {
			attackSide = kKeyboardAttackSideRight;
			attackBand = kKeyboardAttackBandMid;
		} else if (attackLeft) {
			attackSide = kKeyboardAttackSideLeft;
			attackBand = kKeyboardAttackBandMid;
		} else if (attackUp) {
			if (playerState.facing == 2) {
				attackSide = kKeyboardAttackSideRight;
				attackBand = kKeyboardAttackBandUpper;
			} else if (playerState.facing == 1) {
				attackSide = kKeyboardAttackSideLeft;
				attackBand = kKeyboardAttackBandUpper;
			}
		} else if (attackDown) {
			if (playerState.facing == 2) {
				attackSide = kKeyboardAttackSideRight;
				attackBand = kKeyboardAttackBandLower;
			} else if (playerState.facing == 1) {
				attackSide = kKeyboardAttackSideLeft;
				attackBand = kKeyboardAttackBandLower;
			}
		}
		if (attackSide == kKeyboardAttackSideNone) {
			debugC(1, kDebugCombat,
				"Harvester: combat player keyboard target capture class='none' input=(L=%d R=%d U=%d D=%d) facing=%d reason='no native attack family'",
				attackLeft, attackRight, attackUp, attackDown, playerState.facing);
			return;
		}

		const Common::Rect playerRect = playerState.entity->getScreenRect();
		const int playerLeft = playerRect.left;
		const int playerRight = playerRect.right;
		const int playerCenterX = playerState.centerX;
		RuntimeEntity *bestEntity = nullptr;
		Common::String bestName;
		int bestClassId = -1;
		int bestLeadingEdge = attackSide == kKeyboardAttackSideLeft ? -0x7fffffff : 0x7fffffff;

		auto considerTarget = [&](RuntimeEntity *targetEntity, const Common::String &targetName, int classId) {
			if (!targetEntity || !targetEntity->isVisible() ||
					!doRuntimeEntityDepthExtentsOverlap(*playerState.entity, *targetEntity)) {
				return;
			}

			const Common::Rect targetRect = targetEntity->getScreenRect();
			const int targetLeft = targetRect.left;
			const int targetCenterX = targetLeft + (targetRect.width() / 2);
			bool matchesDirection = false;
			if (attackSide == kKeyboardAttackSideLeft) {
				if (attackBand == kKeyboardAttackBandMid)
					matchesDirection = targetLeft < playerCenterX;
				else
					matchesDirection =
						targetLeft < playerCenterX && targetCenterX > playerLeft - kNativeKeyboardAttackSideWindow;
				if (!matchesDirection || targetLeft <= bestLeadingEdge)
					return;
			} else {
				if (attackBand == kKeyboardAttackBandMid)
					matchesDirection = playerCenterX < targetLeft;
				else
					matchesDirection =
						playerCenterX < targetLeft && targetCenterX < playerRight + kNativeKeyboardAttackSideWindow;
				if (!matchesDirection || targetLeft >= bestLeadingEdge)
					return;
			}

			bestEntity = targetEntity;
			bestName = targetName;
			bestClassId = classId;
			bestLeadingEdge = targetLeft;
		};

		for (StartupNpcRecord &npc : scene.state.roomNpcs) {
			if (!npc.visible || npc.deathOrMonsterfyFlag)
				continue;
			considerTarget(findSceneRuntimeEntity(npc.npcName), npc.npcName, kRuntimeEntityClassNpc);
		}
		for (StartupMonsterRecord &monster : scene.state.roomMonsters) {
			if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
				continue;
			considerTarget(findSceneRuntimeEntity(monster.monsterName), monster.monsterName, kRuntimeEntityClassMonster);
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
	};
	auto resolvePlayerAttackContact = [&]() -> Common::Error {
		if (!playerState.attackActive || playerState.attackContactResolved || !playerState.entity ||
				playerState.attackContactFrame < 0 ||
				playerState.entity->getCurrentFrame() < playerState.attackContactFrame) {
			return Common::kNoError;
		}

		playerState.attackContactResolved = true;
		debugC(1, kDebugCombat,
			"Harvester: combat player attack contact frame=%d loadout=%d target_class='%s' target='%s'",
			playerState.entity->getCurrentFrame(), playerState.combatLoadout,
			describeCombatTargetClass(playerState.attackTargetClassId), playerState.attackTargetName.c_str());
		Script *startupScript = _engine.getStartupScript();
		if (!startupScript)
			return Common::kNoError;

		const int damageType = Player::resolveCombatLoadoutDamageType(playerState.combatLoadout);
		if (playerState.attackTargetClassId < 0 && !Player::isProjectileCombatLoadout(playerState.combatLoadout)) {
			if (StartupNpcRecord *fallbackNpc = findOverlappingNpcTarget()) {
				playerState.attackTargetName = fallbackNpc->npcName;
				playerState.attackTargetClassId = kRuntimeEntityClassNpc;
			}
		}
		if (playerState.attackTargetClassId == kRuntimeEntityClassNpc) {
			StartupNpcRecord *npc = findRoomNpcRecordByName(playerState.attackTargetName);
			RuntimeEntity *npcEntity = npc ? findSceneRuntimeEntity(npc->npcName) : nullptr;
			if (!npc || !npcEntity || !playerAttackCanReachTarget(npcEntity))
				npc = Player::isProjectileCombatLoadout(playerState.combatLoadout) ? nullptr : findOverlappingNpcTarget();
			npcEntity = npc ? findSceneRuntimeEntity(npc->npcName) : nullptr;
			if (!npc) {
				debugC(1, kDebugCombat,
					"Harvester: combat player attack missed class='npc' target='%s' reason='no reachable npc'",
					playerState.attackTargetName.c_str());
				return Common::kNoError;
			}

			const bool runtimeChanged =
				startupScript->triggerRuntimeNpcDeathOrMonsterfy(npc->npcName, damageType);
			const bool jimmyFlagChanged = npc->npcName.equalsIgnoreCase("JIMMY")
				? startupScript->setRuntimeFlagValue("JIMMY_ATTACKED", true)
				: false;
			debugC(1, kDebugCombat,
				"Harvester: combat player npc hit target='%s' damage_type=%d runtime_changed=%d jimmy_flag=%d",
				npc->npcName.c_str(), damageType, runtimeChanged, jimmyFlagChanged);
			if (!runtimeChanged && !jimmyFlagChanged)
				return Common::kNoError;
			if (runtimeChanged && npcEntity) {
				const Common::String followTargetName = npc->monsterfyTargetName;
				if (spawnCombatDamageEffect(*npcEntity, followTargetName))
					needsRedraw = true;
			}

			StartupInteractionResult interaction;
			interaction.mutatedRuntimeState = runtimeChanged;
			if (npc->monsterfyTargetName.empty() && !npc->onDeathActionTag.empty()) {
				StartupInteractionResult deathInteraction;
				if (startupScript->executeActionTag(npc->onDeathActionTag, deathInteraction)) {
					interaction = deathInteraction;
					interaction.mutatedRuntimeState = true;
				}
			}

			return handleCombatInteraction(interaction);
		}

		StartupMonsterRecord *monster = nullptr;
		if (playerState.attackTargetClassId == kRuntimeEntityClassMonster)
			monster = findRoomMonsterRecordByName(playerState.attackTargetName);
		RuntimeEntity *monsterEntity = monster ? findSceneRuntimeEntity(monster->monsterName) : nullptr;
		if (!monster || !monsterEntity || !playerAttackCanReachTarget(monsterEntity, monster ? monster->engageDistance : 0)) {
			if (!Player::isProjectileCombatLoadout(playerState.combatLoadout))
				monster = findOverlappingMonsterTarget();
			else
				monster = nullptr;
			monsterEntity = monster ? findSceneRuntimeEntity(monster->monsterName) : nullptr;
		}
		if (!monster || !monsterEntity) {
			debugC(1, kDebugCombat,
				"Harvester: combat player attack missed class='monster' target='%s' reason='no reachable monster'",
				playerState.attackTargetName.c_str());
			return Common::kNoError;
		}

		const int damageAmount = Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout);
		const int hitPointsBefore = monster->currentHitPoints;
		monster->currentHitPoints = MAX(0,
			monster->currentHitPoints - damageAmount);
		debugC(1, kDebugCombat,
			"Harvester: combat player monster hit target='%s' damage=%d hp=%d->%d reachable=1",
			monster->monsterName.c_str(), damageAmount, hitPointsBefore, monster->currentHitPoints);
		if (monster->currentHitPoints > 0) {
			(void)startupScript->syncRuntimeMonsterRecord(*monster);
			needsRedraw = true;
			return Common::kNoError;
		}

		const int monsterIndex = findRoomMonsterIndexByName(monster->monsterName);
		RoomMonsterCombatState *combatState =
			monsterIndex >= 0 && monsterIndex < (int)monsterCombatStates.size()
				? &monsterCombatStates[(uint)monsterIndex]
				: nullptr;
		if (combatState)
			clearRoomMonsterCombatState(*combatState);
		(void)startupScript->syncRuntimeMonsterRecord(*monster);

		RoomDeathAnimationRange deathRange;
		if (combatState && resolveMonsterDeathAnimationRange(*monsterEntity, monster->facing,
				damageType, _engine.isGoreEnabled(), deathRange)) {
			combatState->deathActive = true;
			combatState->deathFirstFrame = deathRange.firstFrame;
			combatState->deathLastFrame = deathRange.lastFrame;
			combatState->deathDamageType = damageType;
			combatState->nextMovementTick = 0;
			combatState->nextAttackAllowedTick = 0;
			monsterEntity->setAnimationFrameRange(deathRange.firstFrame, deathRange.lastFrame, false);
			monsterEntity->setAnimationRate(kRoomMonsterAnimationRate);
			monsterEntity->setAnimationEnabled(true);
			monsterEntity->setCurrentFrame(deathRange.firstFrame);
			if (!monster->deathSound.empty())
				(void)_engine.playStartupSound(monster->deathSound);
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
		debugC(1, kDebugCombat,
			"Harvester: combat monster defeated target='%s' on_death='%s' fallback='no death bank'",
			monster->monsterName.c_str(), monster->onDeathActionTag.c_str());
		StartupInteractionResult interaction;
		interaction.mutatedRuntimeState = true;
		if (!monster->onDeathActionTag.empty()) {
			StartupInteractionResult deathInteraction;
			if (startupScript->executeActionTag(monster->onDeathActionTag, deathInteraction)) {
				interaction = deathInteraction;
				interaction.mutatedRuntimeState = true;
			}
		}

		return handleCombatInteraction(interaction);
	};
	auto updateRoomMonsterCombat = [&]() -> Common::Error {
		if (!runtimeEntities)
			return Common::kNoError;

		Script *startupScript = _engine.getStartupScript();
		const uint32 now = Player::getRuntimeClockTicks();
		const uint32 moveInterval = MAX<uint32>(1, 100U / (uint32)kRoomMonsterAnimationRate);
		const int horizontalStepBase = 8;
		const int depthStep = MAX<int>(1, roundRoomCombatFloat(
			scene.state.roomZVelocityStep > 0.0f ? scene.state.roomZVelocityStep : 1.0f));
		bool playerAlive = startupScript && startupScript->getPlayerCurrentHitPoints() > 0;

		if (monsterCombatStates.size() != scene.state.roomMonsters.size())
			monsterCombatStates.resize(scene.state.roomMonsters.size());

		for (uint i = 0; i < scene.state.roomMonsters.size(); ++i) {
			StartupMonsterRecord &monster = scene.state.roomMonsters[i];
			RoomMonsterCombatState &combatState = monsterCombatStates[i];
			RuntimeEntity *entity = findSceneRuntimeEntity(monster.monsterName);
			if (!entity || !monster.visible) {
				clearRoomMonsterCombatState(combatState);
				continue;
			}
			if (combatState.deathActive) {
				if (entity->getCurrentFrame() == combatState.deathLastFrame) {
					combatState.deathActive = false;
					monster.active = false;
					monster.visible = false;
					(void)startupScript->syncRuntimeMonsterRecord(monster);
					debugC(1, kDebugCombat,
						"Harvester: combat monster death complete target='%s' damage_type=%d on_death='%s'",
						monster.monsterName.c_str(), combatState.deathDamageType,
						monster.onDeathActionTag.c_str());
					clearRoomMonsterCombatState(combatState);

					StartupInteractionResult interaction;
					interaction.mutatedRuntimeState = true;
					if (startupScript && !monster.onDeathActionTag.empty()) {
						StartupInteractionResult deathInteraction;
						if (startupScript->executeActionTag(monster.onDeathActionTag, deathInteraction)) {
							interaction = deathInteraction;
							interaction.mutatedRuntimeState = true;
						}
					}

					return handleCombatInteraction(interaction);
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
					if (!startupScript) {
						debugC(1, kDebugCombat,
							"Harvester: combat monster attack contact monster='%s' frame=%d skipped reason='no startup script'",
							monster.monsterName.c_str(), currentFrame);
					} else if (!playerAlive || !hasTrackedPlayerTarget) {
						debugC(1, kDebugCombat,
							"Harvester: combat monster attack miss monster='%s' frame=%d player_hp=%d reason='no live target'",
							monster.monsterName.c_str(), currentFrame, startupScript->getPlayerCurrentHitPoints());
					} else {
						const int playerHitPointsBefore = startupScript->getPlayerCurrentHitPoints();
						const bool changed = startupScript->adjustPlayerCurrentHitPoints(-monster.damageAmount);
						const int playerHitPointsAfter = startupScript->getPlayerCurrentHitPoints();
						debugC(1, kDebugCombat,
							"Harvester: combat monster attack hit monster='%s' frame=%d damage=%d player_hp=%d->%d changed=%d",
							monster.monsterName.c_str(), currentFrame, monster.damageAmount,
							playerHitPointsBefore, playerHitPointsAfter, changed);
						if (changed && playerHitPointsAfter <= 0) {
							playerAlive = false;
							stopPlayerRegionInteraction();
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

			if (combatState.nextMovementTick != 0 && (int32)(now - combatState.nextMovementTick) < 0)
				continue;
			combatState.nextMovementTick = now + moveInterval;

			const int previousX = monster.posX;
			const int previousY = monster.posY;
			const int previousZ = monster.posZ;
			const Common::Rect playerRect = playerState.entity->getScreenRect();
			const Common::Rect monsterRect = entity->getScreenRect();
			const int playerCenterX = playerRect.left + playerRect.width() / 2;
			const int monsterCenterX = monsterRect.left + monsterRect.width() / 2;
			const int liveCenterDx = playerCenterX - monsterCenterX;
			const int absLiveCenterDx = ABS(liveCenterDx);
			const int monsterLiveCenterOffset = monsterCenterX - monster.posX;
			const int playerTargetZ = roundRoomCombatFloat(playerState.z);
			const float zDelta = playerState.z - (float)monster.posZ;
			const float absZDelta = zDelta >= 0.0f ? zDelta : -zDelta;
			const int engageDistance = MAX(0, monster.engageDistance);
			const int liveWaypointTolerance = roundRoomCombatFloat(
				Player::computeDepthScale(scene.state,
					MIN<float>(playerState.z, (float)monster.posZ)) * kNativeMonsterHorizontalWaypointTolerance);
			bool attemptedMoveX = false;
			bool attemptedMoveZ = false;
			if (absZDelta > kNativeMonsterPursuitZTolerance) {
				attemptedMoveZ = true;
				monster.posZ = stepTowardsRoomCombatInt(monster.posZ, playerTargetZ, depthStep);
				monster.posZ = (int)clampRoomDepthForEvent(scene.state, (float)monster.posZ);
				monster.posY = mapRoomDepthToScreenYForCombat(scene.state, (float)monster.posZ, monster.posY);
			} else if (absLiveCenterDx > engageDistance) {
				const int desiredLiveCenterX = monsterCenterX < playerCenterX
					? playerRect.left - engageDistance + monsterRect.width() / 2
					: playerRect.right + engageDistance - monsterRect.width() / 2;
				if (ABS(monsterCenterX - desiredLiveCenterX) > liveWaypointTolerance) {
					attemptedMoveX = true;
					const int step = MAX<int>(1, roundRoomCombatFloat(
						Player::computeDepthScale(scene.state, (float)monster.posZ) * (float)horizontalStepBase));
					const int desiredLogicalCenterX = desiredLiveCenterX - monsterLiveCenterOffset;
					monster.posX = stepTowardsRoomCombatInt(monster.posX, desiredLogicalCenterX, step);
					monster.posX = CLIP<int>(monster.posX, monster.minXBound, monster.maxXBound);
				}
			}

			const bool moved = monster.posX != previousX || monster.posY != previousY || monster.posZ != previousZ;
			if (moved) {
				const int movedX = monster.posX - previousX;
				const int movedY = monster.posY - previousY;
				if (ABS(movedX) > ABS(movedY))
					monster.facing = movedX < 0 ? 1 : 2;
				else if (movedY != 0)
					monster.facing = movedY < 0 ? 3 : 0;
				(void)applyRoomActorPlacement(scene.state, *entity, monster.posX, monster.posY, (float)monster.posZ);
				if (startupScript)
					(void)startupScript->syncRuntimeMonsterRecord(monster);
				debugC(1, kDebugCombat,
					"Harvester: combat monster chase move monster='%s' live_center_dx=%d z_delta=%.2f engage=%d waypoint_tol=%d from=(%d,%d,z=%d) to=(%d,%d,z=%d) facing=%d",
					monster.monsterName.c_str(), liveCenterDx, (double)zDelta, engageDistance, liveWaypointTolerance,
					previousX, previousY, previousZ, monster.posX, monster.posY, monster.posZ, monster.facing);
				needsRedraw = true;
				needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, true) || needsRedraw;
				continue;
			}

			if (attemptedMoveX || attemptedMoveZ) {
				debugC(1, kDebugCombat,
					"Harvester: combat monster chase stalled monster='%s' live_center_dx=%d z_delta=%.2f engage=%d waypoint_tol=%d pos=(%d,%d,z=%d) attempted_x=%d attempted_z=%d",
					monster.monsterName.c_str(), liveCenterDx, (double)zDelta, engageDistance, liveWaypointTolerance,
					monster.posX, monster.posY, monster.posZ, attemptedMoveX, attemptedMoveZ);
			}

			needsRedraw = setRoomMonsterAnimation(*entity, monster.facing, false) || needsRedraw;
			const bool closeEnoughForAttack =
				absZDelta <= kNativeMonsterPursuitZTolerance &&
				absLiveCenterDx <= engageDistance;
			const bool attackCooldownExpired =
				combatState.nextAttackAllowedTick == 0 || (int32)(now - combatState.nextAttackAllowedTick) >= 0;
			if (!closeEnoughForAttack)
				continue;
			if (!attackCooldownExpired && absLiveCenterDx > MAX(1, engageDistance / 3))
				continue;

			RoomAttackAnimationRange range;
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
			combatState.nextAttackAllowedTick = now + 50 + _engine.getRandomNumber(99);
			monster.facing = range.resumeFacing;
			entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
			entity->setAnimationRate(kRoomMonsterAnimationRate);
			entity->setAnimationEnabled(true);
			entity->setCurrentFrame(range.firstFrame);
			debugC(1, kDebugCombat,
				"Harvester: combat monster attack start monster='%s' target='%s' frames=%d..%d contact=%d resume_facing=%d live_center_dx=%d z_delta=%.2f engage=%d next_attack_tick=%u",
				monster.monsterName.c_str(), combatState.attackTargetName.c_str(), range.firstFrame, range.lastFrame,
				combatState.attackContactFrame, range.resumeFacing, liveCenterDx, (double)zDelta,
				engageDistance, combatState.nextAttackAllowedTick);
			needsRedraw = true;
		}

		return Common::kNoError;
	};
	auto moveRoomItemDirectlyToInventory = [&](const StartupObjectRecord &object) -> Common::Error {
		Script *startupScript = _engine.getStartupScript();
		if (!startupScript)
			return Common::kReadingFailed;

		const StartupObjectRecord savedRuntimeObject = object;

		startupScript->addRuntimeObjectToInventory(object.objectName);
		const Common::String inventoryOwner(kInventoryOwnerName);
		hideSceneObject(object.objectName, &inventoryOwner);
		removeSceneEntityByName(object.objectName);

		bool didTransition = false;
		if (shouldDispatchPickupActionOnDirectInventoryTransfer(object)) {
			StartupInteractionResult pickupInteraction;
			if (startupScript->executeActionTag(object.actionTag, pickupInteraction)) {
				Common::Error interactionError =
					interactionProcessor.handleInteractionResult(
						pickupInteraction, didTransition, Common::String());
				if (interactionError.getCode() != Common::kNoError)
					return interactionError;
				if (startupFlow.hasPendingMainMenuReturn())
					return Common::kNoError;
				if (shouldCancelDeniedPickup(pickupInteraction, didTransition)) {
					debugC(1, kDebugScene,
						"Harvester: cancelling denied direct pickup object='%s' action='%s' text='%s'",
						object.objectName.c_str(), object.actionTag.c_str(),
						pickupInteraction.modalText.value.c_str());
					return restoreDeniedPickup(*startupScript, savedRuntimeObject, false);
				}
			}
		}

		if (!_inventory.refresh())
			return Common::kReadingFailed;
		needsRedraw = true;
		return Common::kNoError;
	};
	auto beginRoomItemCarry = [&](const StartupObjectRecord &object) -> Common::Error {
		Script *startupScript = _engine.getStartupScript();
		ResourceManager *resources = _engine.getResources();
		if (!startupScript || !resources)
			return Common::kReadingFailed;

		const StartupObjectRecord savedRuntimeObject = object;

		clearCarriedRoomItem();
		carriedRoomItemName = object.objectName;
		carriedRoomItemLabel = startupScript->resolveObjectLabel(object);
		const Common::String spritePath = resolveCarriedObjectSpritePath(object);
		if (!spritePath.empty())
			loadBitmapResource(*resources, spritePath, carriedRoomItemBitmap);

		startupScript->setRuntimeObjectVisible(object.currentOwnerOrRoom, object.objectName, false);
		hideSceneObject(object.objectName, nullptr);
		removeSceneEntityByName(object.objectName);

		bool didTransition = false;
		if (shouldDispatchPickupActionOnCarryStart(object)) {
			StartupInteractionResult pickupInteraction;
			if (startupScript->executeActionTag(object.actionTag, pickupInteraction)) {
				Common::Error interactionError =
					interactionProcessor.handleInteractionResult(
						pickupInteraction, didTransition, Common::String());
				if (interactionError.getCode() != Common::kNoError)
					return interactionError;
				if (startupFlow.hasPendingMainMenuReturn())
					return Common::kNoError;
				if (shouldCancelDeniedPickup(pickupInteraction, didTransition)) {
					debugC(1, kDebugScene,
						"Harvester: cancelling denied carried pickup object='%s' action='%s' text='%s'",
						object.objectName.c_str(), object.actionTag.c_str(),
						pickupInteraction.modalText.value.c_str());
					return restoreDeniedPickup(*startupScript, savedRuntimeObject, true);
				}
			}
		}

		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			needsRedraw = true;
			return Common::kNoError;
		}

		needsRedraw = true;
		return Common::kNoError;
	};
	auto handleInventoryTargetInteraction = [&](const StartupObjectRecord &target) -> Common::Error {
		if (!_inventory.hasSelection())
			return Common::kNoError;

		const Common::String selectedItemName = _inventory.getSelectedItemName();
		StartupInteractionResult interaction;
		const bool handled = _engine.getStartupScript()->resolveUseItemInteraction(
			selectedItemName, target, interaction);
		if (!handled)
			return Common::kNoError;

		bool didTransition = false;
		Common::Error interactionError =
			interactionProcessor.handleInteractionResult(interaction, didTransition, selectedItemName);
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		_inventory.clearSelection();
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!_inventory.refresh())
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

		Player::setMoveTarget(scene.state, playerState,
			Player::resolveRegionTargetX(region, playerState),
			Player::resolveRegionTargetZ(region));
	};
	auto runRegionInteraction = [&](const StartupRegionRecord &region) -> Common::Error {
		StartupInteractionResult interaction;
		if (!_engine.getStartupScript()->resolveRegionInteraction(region, interaction))
			return Common::kNoError;

		bool didTransition = false;
		Common::Error interactionError =
			interactionProcessor.handleInteractionResult(
				interaction, didTransition, Common::String());
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;

		if (!_inventory.refresh())
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
				(void)Player::startTurnAnimation(playerState, region->desiredFacing);
			return Common::kNoError;
		}

		stopPlayerRegionInteraction();
		return runRegionInteraction(*region);
	};
		auto tryActivateOverlappedRegion = [&]() -> Common::Error {
			if (!playerState.entity)
				return Common::kNoError;

		for (const StartupRegionRecord &region : scene.sceneRegions) {
			if (!region.startEnabled)
				continue;
			if (!doesPlayerOverlapRegion(*playerState.entity, region))
				continue;
			if (!doesPlayerFacingMatchRegion(playerState.facing, region))
				continue;

			stopPlayerRegionInteraction();
			return runRegionInteraction(region);
		}

			return Common::kNoError;
		};
		if (!_inventory.refresh())
			return Common::kReadingFailed;
		Graphics::FrameLimiter limiter(g_system, 60);
		captureCurrentSaveState();

	if (shouldRunStartupRoomProbe())
		logStartupRoomProbe(_engine, scene, currentRoomTarget, _mousePos);

	while (!_engine.shouldQuit()) {
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (startupFlow.takePendingNewGameRestart()) {
			startupFlow.prepareForNewGame();
			pendingRoomChange = "START";
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (_engine.hasPendingLoadedStartupSaveRoomState()) {
			pendingRoomChange = _engine.getPendingLoadedStartupSaveRoomState().entranceName;
			if (pendingRoomChange.empty())
				pendingRoomChange = _engine.getPendingLoadedStartupSaveRoomState().roomName;
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		captureCurrentSaveState();
		if (_inventory.isOpen() && _inventory.refreshIfRuntimeStateChanged())
			needsRedraw = true;

		if (needsRedraw) {
			Graphics::Screen *activeScreen = getActiveScreen();
			if (!activeScreen)
				return Common::kNoError;

			const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
			const bool inventorySelectionActive = _inventory.hasSelection();
			const bool roomCarryActive = hasCarriedRoomItem();
			const bool activeCarry = inventorySelectionActive || roomCarryActive;
			const bool inventoryPanelContainsMouse = _inventory.isOpen() && inventoryPanelBounds.contains(_mousePos);
			const bool suppressHover = showingInspectText || idleState.active || idleState.exiting ||
				playerState.attackActive ||
				(_inventory.isOpen() && (inventoryPanelContainsMouse || !inventorySelectionActive));
			StartupRoomHoverState hoverState = suppressHover
				? StartupRoomHoverState()
				: resolveRoomHoverState(_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, _mousePos);
			Common::String promptText;
			Common::String inventoryTooltipText;
			auto resolveCarryTargetLabel = [&]() {
				if (hoverState.playerEntity && playerState.entity && hoverState.playerEntity == playerState.entity)
					return Common::String(kPlayerInventoryLabel);
				if (hoverState.object)
					return _engine.getStartupScript()->resolveObjectLabel(*hoverState.object);
				if (hoverState.npc)
					return resolveStartupNpcLabel(*hoverState.npc);
				return Common::String();
			};
			if (activeCarry) {
				Common::String targetLabel;
				const StartupInventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
				if (_inventory.isOpen()) {
					if (inventoryHover && !InventorySystem::isExitObject(inventoryHover->object) &&
							!InventorySystem::isStatusObject(inventoryHover->object) &&
							!inventoryHover->object.objectName.equalsIgnoreCase(_inventory.getSelectedItemName())) {
						targetLabel = _engine.getStartupScript()->resolveObjectLabel(inventoryHover->object);
					} else if (!inventoryPanelContainsMouse) {
						targetLabel = resolveCarryTargetLabel();
					}
				} else {
					targetLabel = resolveCarryTargetLabel();
				}
				if (inventorySelectionActive) {
					promptText = _inventory.buildSelectedPrompt(targetLabel);
					_inventory.setPromptText(promptText);
				} else {
					promptText = buildUseItemPrompt(carriedRoomItemLabel, targetLabel);
				}
				hoverState.cursorSequence = 7;
			} else if (_inventory.isOpen()) {
				const StartupInventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
				if (inventoryHover &&
						!InventorySystem::isExitObject(inventoryHover->object)) {
					inventoryTooltipText =
						_engine.getStartupScript()->resolveInventoryTooltipText(inventoryHover->object);
				}
				_inventory.setPromptText(Common::String());
				hoverState.cursorSequence = 7;
			} else {
				promptText = hoverState.promptText;
			}
			if (RuntimeEntity *cursor = runtimeEntities ? runtimeEntities->getCursorEntity() : nullptr) {
				cursor->setAnimationSequence(
					(showingInspectText || idleState.active || idleState.exiting ||
						playerState.attackActive ||
						_inventory.isOpen() || activeCarry)
						? 7
						: hoverState.cursorSequence);
			}

			drawRoomScene(_engine, *activeScreen, scene, scene.targetPaletteBrightness);
			if (_inventory.isOpen()) {
				_inventory.drawOverlay(*activeScreen);
				drawInventoryWeekday(*activeScreen, *inventoryTooltipFont, _inventory.resolveWeekdayLabel());
			}
			if (inventorySelectionActive) {
				_inventory.drawSelectedDragItem(*activeScreen, _mousePos);
			} else if (roomCarryActive && carriedRoomItemBitmap.isValid()) {
				const int drawX = _mousePos.x - (int)carriedRoomItemBitmap.width / 2;
				const int drawY = _mousePos.y - (int)carriedRoomItemBitmap.height / 2;
				blitBitmap(*activeScreen, carriedRoomItemBitmap, drawX, drawY);
			}

			if (showingInspectText) {
				drawRoomInspectText(*activeScreen, *art, *inspectFont, inspectText, useNativeInspectFont);
			} else if (!inventoryTooltipText.empty()) {
				drawInventoryTooltip(*activeScreen, *inventoryTooltipFont, inventoryTooltipText);
			} else if (!promptText.empty()) {
				drawRoomPrompt(*activeScreen, *promptFont, promptText, useNativePromptFont);
			}

			if (runtimeEntities)
				runtimeEntities->drawCursor(*activeScreen);
			activeScreen->makeAllDirty();
			activeScreen->update();
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (startupFlow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				if (_inventory.isOpen() && _inventory.hasSelection()) {
					const StartupInventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
					// Native inventory handoff is driven by the latched selected item, not by
					// a second click after the selection has already been made.
					if (!inventoryHover && isOutsideNativeInventoryDragCloseBounds(_mousePos)) {
						if (_inventory.close())
							needsRedraw = true;
					}
				}
				needsRedraw = true;
				break;
			case Common::EVENT_RBUTTONDOWN: {
				notePlayerActivity();
				if (idleState.active || idleState.exiting) {
					if (Player::requestIdleAnimationExit(scene.state, playerState, idleState))
						needsRedraw = true;
					break;
				}
				if (playerState.attackActive)
					break;
				pendingRegionName.clear();
				if (_inventory.isOpen()) {
					const StartupInventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
					debugLogInventoryClick("right", _mousePos, inventoryHover);
					if (_inventory.hasSelection()) {
						debugC(1, kDebugInventory,
							"Harvester: inventory right click cleared active selection='%s'",
							_inventory.getSelectedItemName().c_str());
						if (_inventory.clearSelection())
							needsRedraw = true;
						break;
					}
					if (inventoryHover &&
							!InventorySystem::isExitObject(inventoryHover->object) &&
							!InventorySystem::isStatusObject(inventoryHover->object)) {
						bool loadoutChanged = false;
						if (_inventory.toggleCombatLoadout(inventoryHover->object,
								playerState.combatLoadout, loadoutChanged)) {
							debugC(1, kDebugInventory,
								"Harvester: inventory right click handled as combat toggle object='%s' changed=%d",
								inventoryHover->object.objectName.c_str(), loadoutChanged);
							if (loadoutChanged) {
								captureCurrentSaveState();
								if (playerState.entity) {
									(void)Player::syncCombatLoadoutVisual(_engine, scene.state, playerState,
										_engine.getStartupScript()->getPlayerCombatLoadout());
								} else if (_engine.getStartupScript()) {
									playerState.combatLoadout =
										_engine.getStartupScript()->getPlayerCombatLoadout();
								}
								if (!_inventory.refresh())
									return Common::kReadingFailed;
							}
							needsRedraw = true;
							break;
						}

						InventorySecondaryAction secondaryAction;
						if (_inventory.resolveSecondaryAction(inventoryHover->object, secondaryAction)) {
							debugC(1, kDebugInventory,
								"Harvester: inventory right click executing secondary action object='%s' action='%s' closeInventory=%d",
								inventoryHover->object.objectName.c_str(),
								secondaryAction.actionTag.c_str(), secondaryAction.closeInventory);
							if (secondaryAction.closeInventory)
								(void)_inventory.close();

							StartupInteractionResult interaction;
							bool didTransition = false;
							const bool executedActionTag = _engine.getStartupScript()->executeActionTag(
									secondaryAction.actionTag, interaction);
							debugC(1, kDebugInventory,
								"Harvester: inventory right click executeActionTag('%s') -> %d music='%s' cutscene='%s' nextRoom='%s'",
								secondaryAction.actionTag.c_str(), executedActionTag,
								interaction.musicPath.c_str(), interaction.cutscenePath.c_str(),
								interaction.nextRoomName.c_str());
							if (executedActionTag) {
								Common::Error interactionError =
									interactionProcessor.handleInteractionResult(
										interaction, didTransition, Common::String());
								debugC(1, kDebugInventory,
									"Harvester: inventory right click action='%s' interaction result error=%d transitioned=%d pendingMainMenu=%d",
									secondaryAction.actionTag.c_str(), interactionError.getCode(),
									didTransition, startupFlow.hasPendingMainMenuReturn());
								if (interactionError.getCode() != Common::kNoError)
									return interactionError;
								if (startupFlow.hasPendingMainMenuReturn())
									return Common::kNoError;
							}

							if (_inventory.isOpen()) {
								if (!_inventory.refresh())
									return Common::kReadingFailed;
							} else {
								debugC(1, kDebugInventory,
									"Harvester: inventory right click skipped overlay refresh after action='%s' transitioned=%d closeInventory=%d",
									secondaryAction.actionTag.c_str(), didTransition, secondaryAction.closeInventory);
							}
							needsRedraw = true;
							break;
						}
						debugC(1, kDebugInventory,
							"Harvester: inventory right click object='%s' produced no combat toggle or secondary action",
							inventoryHover->object.objectName.c_str());
					}
					debugC(1, kDebugInventory, "Harvester: inventory right click closing overlay");
					if (_inventory.close())
						needsRedraw = true;
					break;
				}
				if (_inventory.hasSelection()) {
					if (_inventory.clearSelection())
						needsRedraw = true;
					break;
				}
				if (hasCarriedRoomItem()) {
					if (!stowCarriedRoomItemToInventory())
						return Common::kReadingFailed;
					needsRedraw = true;
					break;
				}
				Script *startupScript = _engine.getStartupScript();
				if (!showingInspectText && startupScript && playerState.entity &&
						startupScript->getPlayerCurrentHitPoints() > 0) {
					playerState.hasMoveTarget = false;
					playerState.turnActive = false;
					playerState.turnTargetFacing = -1;
					playerState.turnFirstFrame = -1;
					playerState.turnLastFrame = -1;
					playerState.turnEndFrame = -1;
					playerState.turnPlayBackwards = false;
					(void)Player::syncCombatLoadoutVisual(_engine, scene.state, playerState,
						startupScript->getPlayerCombatLoadout());
					capturePlayerAttackTarget();
					if (Player::startAttackAnimation(scene.state, playerState, _mousePos)) {
						debugC(1, kDebugCombat,
							"Harvester: combat player attack start loadout=%d target_class='%s' target='%s' cursor=(%d,%d)",
							playerState.combatLoadout,
							describeCombatTargetClass(playerState.attackTargetClassId),
							playerState.attackTargetName.c_str(), _mousePos.x, _mousePos.y);
						needsRedraw = true;
					} else {
						playerState.attackTargetName.clear();
						playerState.attackTargetClassId = -1;
					}
				}
				break;
			}
			case Common::EVENT_LBUTTONUP:
				lastLeftButtonReleaseTick = Player::getRuntimeClockTicks();
				if (showingInspectText)
					inspectCanDismiss = true;
				break;
			case Common::EVENT_LBUTTONDOWN: {
				const uint32 now = Player::getRuntimeClockTicks();
				const bool isFastExitClick =
					lastLeftButtonReleaseTick != 0 &&
					now - lastLeftButtonReleaseTick < kRoomExitFastClickWindowTicks;
				notePlayerActivity();
				if (idleState.active || idleState.exiting) {
					if (Player::requestIdleAnimationExit(scene.state, playerState, idleState))
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
				if (playerState.attackActive)
					break;

				const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
				if (_inventory.isOpen()) {
					const StartupInventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
					debugLogInventoryClick("left", _mousePos, inventoryHover);
					if (inventoryHover) {
						if (InventorySystem::isExitObject(inventoryHover->object)) {
							debugC(1, kDebugInventory, "Harvester: inventory left click exit button");
							const bool clearedSelection = _inventory.clearSelection();
							const bool closedInventory = _inventory.close();
							if (clearedSelection || closedInventory)
								needsRedraw = true;
							break;
						}
						if (InventorySystem::isStatusObject(inventoryHover->object))
							break;

						if (!_inventory.hasSelection()) {
							debugC(1, kDebugInventory,
								"Harvester: inventory left click selecting object='%s'",
								inventoryHover->object.objectName.c_str());
							_inventory.selectItem(inventoryHover->object.objectName);
							_inventory.setPromptText(_inventory.buildSelectedPrompt(Common::String()));
							needsRedraw = true;
							break;
						}

						if (!inventoryHover->object.objectName.equalsIgnoreCase(_inventory.getSelectedItemName())) {
							debugC(1, kDebugInventory,
								"Harvester: inventory left click using selected='%s' on target='%s'",
								_inventory.getSelectedItemName().c_str(),
								inventoryHover->object.objectName.c_str());
							Common::Error interactionError =
								handleInventoryTargetInteraction(inventoryHover->object);
							if (interactionError.getCode() != Common::kNoError)
								return interactionError;
						}
						needsRedraw = true;
						break;
					}

					if (_inventory.hasSelection()) {
						if (!inventoryPanelBounds.contains(_mousePos) && _inventory.close())
							needsRedraw = true;
						break;
					}
					if (!inventoryPanelBounds.contains(_mousePos) && _inventory.close())
						needsRedraw = true;
					break;
				}

				const StartupRoomHoverState hoverState = resolveRoomHoverState(
					_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions, _mousePos);
				debugC(1, kDebugScene,
					"Harvester: room click room='%s' mouse=(%d,%d) object='%s' npc='%s' region='%s' cursor_sequence=%d prompt='%s'",
					scene.state.roomName.c_str(), _mousePos.x, _mousePos.y,
					hoverState.object ? hoverState.object->objectName.c_str() : "",
					hoverState.npc ? hoverState.npc->npcName.c_str() : "",
					hoverState.region ? hoverState.region->regionName.c_str() : "",
					hoverState.cursorSequence, hoverState.promptText.c_str());
					if (hasCarriedRoomItem()) {
					if (hoverState.playerEntity && playerState.entity &&
							hoverState.playerEntity == playerState.entity) {
						if (!stowCarriedRoomItemToInventory())
							return Common::kReadingFailed;
						needsRedraw = true;
					}
					break;
				}
				if (_inventory.hasSelection()) {
					const Common::String selectedItemName = _inventory.getSelectedItemName();
					if (hoverState.npc) {
						bool didTransition = false;
						Common::Error dialogueError = interactionProcessor.runScriptedDialogue(
							hoverState.npc->npcName, selectedItemName, Common::String(), didTransition);
						if (dialogueError.getCode() != Common::kNoError)
							return dialogueError;
						if (startupFlow.hasPendingMainMenuReturn())
							return Common::kNoError;
						if (_inventory.clearSelection())
							needsRedraw = true;
						break;
					}

					StartupObjectRecord *roomTarget = hoverState.object
						? findSceneObjectByName(scene.sceneObjects, hoverState.object->objectName)
						: nullptr;
					if (roomTarget) {
						Common::Error interactionError =
							handleInventoryTargetInteraction(*roomTarget);
						if (interactionError.getCode() != Common::kNoError)
							return interactionError;
						needsRedraw = true;
					}
					break;
				}
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
				if (hoverState.npc) {
					pendingRegionName.clear();
					moveLeft = false;
					moveRight = false;
					moveUp = false;
					moveDown = false;
					playerState.hasMoveTarget = false;
					playerState.turnActive = false;
					playerState.turnTargetFacing = -1;
					bool didTransition = false;
					Common::Error dialogueError = interactionProcessor.runScriptedDialogue(
						hoverState.npc->npcName, Common::String(), Common::String(), didTransition);
					if (dialogueError.getCode() != Common::kNoError)
						return dialogueError;
					if (startupFlow.hasPendingMainMenuReturn())
						return Common::kNoError;
					needsRedraw = true;
					break;
				}
				if (hoverState.region && playerState.entity) {
					if (isFastExitClick &&
						hoverState.region->startEnabled &&
						roomAllowsImmediateExitClick(scene.state.roomName)) {
						stopPlayerRegionInteraction();
						Common::Error interactionError = runRegionInteraction(*hoverState.region);
						if (interactionError.getCode() != Common::kNoError)
							return interactionError;
						if (startupFlow.hasPendingMainMenuReturn())
							return Common::kNoError;
						needsRedraw = true;
						break;
					}
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
					if (hoverState.cursorSequence == 0 && playerState.entity) {
						Player::setMoveTargetFromScreenPoint(scene.state, playerState, _mousePos.x, _mousePos.y);
						needsRedraw = true;
					}
					break;
				}
				pendingRegionName.clear();
				if (clickedObject->objectName.equalsIgnoreCase("EXIT_BM") ||
					clickedObject->objectName.equalsIgnoreCase("EXIT_HS")) {
					Common::Error exitError = runRoomExitCommands();
					if (exitError.getCode() != Common::kNoError)
						return exitError;
					return Common::kNoError;
				}

				StartupResolvedText resolvedInspectText;
				const bool hasInspectText =
					_engine.getStartupScript()->resolveObjectInspectText(*clickedObject, resolvedInspectText);
				const bool unlocksAfterInitialExamine =
					unlocksRoomObjectInteractionAfterInitialExamine(*clickedObject, *_engine.getStartupScript());
				const bool canShowInspectText =
					hasInspectText && resolveInspectTextboxBitmap(*art, resolvedInspectText);
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
				if (_engine.getStartupScript()->isPickupObject(*clickedObject)) {
					StartupInteractionResult blockedPickupInteraction;
					if (_engine.getStartupScript()->isPickupBlockedByAction(
							*clickedObject, &blockedPickupInteraction)) {
						bool didTransition = false;
						Common::Error interactionError =
							interactionProcessor.handleInteractionResult(
								blockedPickupInteraction, didTransition, Common::String());
						if (interactionError.getCode() != Common::kNoError)
							return interactionError;
						needsRedraw = true;
						break;
					}

					Common::Error pickupError = playerState.entity
						? beginRoomItemCarry(*clickedObject)
						: moveRoomItemDirectlyToInventory(*clickedObject);
					if (pickupError.getCode() != Common::kNoError)
						return pickupError;
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

				bool didTransition = false;
				Common::Error interactionError =
					interactionProcessor.handleInteractionResult(
						interaction, didTransition, Common::String());
				if (interactionError.getCode() != Common::kNoError)
					return interactionError;
				if (startupFlow.hasPendingMainMenuReturn())
					return Common::kNoError;
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

				notePlayerActivity();
				if (event.kbd.hasFlags(Common::KBD_CTRL))
					attackModifierHeld = true;
				if (idleState.active || idleState.exiting) {
					if (Player::requestIdleAnimationExit(scene.state, playerState, idleState))
						needsRedraw = true;
					break;
				}

				if (_inventory.isOpen()) {
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						const bool clearedSelection = _inventory.clearSelection();
						const bool closedInventory = _inventory.close();
						if (clearedSelection || closedInventory)
							needsRedraw = true;
					} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
							event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
							event.kbd.keycode == Common::KEYCODE_i) {
						if (_inventory.close())
							needsRedraw = true;
					}
					break;
				}
				if (_inventory.hasSelection()) {
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						if (_inventory.clearSelection())
							needsRedraw = true;
						break;
					}
					if (event.kbd.keycode == Common::KEYCODE_i) {
						if (_inventory.clearSelection())
							needsRedraw = true;
						if (!openInventoryOverlay())
							return Common::kReadingFailed;
						needsRedraw = true;
						break;
					}
				}

				if (event.kbd.keycode == Common::KEYCODE_LCTRL ||
						event.kbd.keycode == Common::KEYCODE_RCTRL) {
					attackModifierHeld = true;
				} else if (event.kbd.keycode == Common::KEYCODE_LEFT)
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
					if (cancelPlayerAttackAnimation())
						needsRedraw = true;
					if (!stowCarriedRoomItemToInventory())
						return Common::kReadingFailed;
					moveLeft = false;
					moveRight = false;
					moveUp = false;
					moveDown = false;
					pendingRegionName.clear();
					playerState.hasMoveTarget = false;
					playerState.turnActive = false;
					playerState.turnTargetFacing = -1;
					captureCurrentSaveState();
					Graphics::Screen *activeScreen = getActiveScreen();
					if (!activeScreen)
						return Common::kNoError;
					IndexedBitmap roomMenuBackdrop;
					drawRoomScene(_engine, *activeScreen, scene, scene.targetPaletteBrightness);
					if (!captureScreenBackdrop(*activeScreen, roomMenuBackdrop))
						return Common::kReadingFailed;
					Common::Error menuError = startupFlow.runRoomMenuStub(
						roomMenuBackdrop, scene.palette, scene.targetPaletteBrightness);
					if (menuError.getCode() != Common::kNoError)
						return menuError;
					startupFlow.resetCursorAnimationSequence();
					resetIdleState();
					needsRedraw = true;
					break;
				}

				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					if (!stowCarriedRoomItemToInventory())
						return Common::kReadingFailed;
					Common::Error exitError = runRoomExitCommands();
					if (exitError.getCode() != Common::kNoError)
						return exitError;
					return Common::kNoError;
				}
				break;
			case Common::EVENT_KEYUP:
				if (event.kbd.keycode == Common::KEYCODE_LCTRL ||
						event.kbd.keycode == Common::KEYCODE_RCTRL)
					attackModifierHeld = false;
				else if (event.kbd.keycode == Common::KEYCODE_LEFT)
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
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}

		bool playerAdvancedThisFrame = false;
		Common::Error combatError = resolvePlayerAttackContact();
		if (combatError.getCode() != Common::kNoError)
			return combatError;
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (Player::updateAttackAnimationState(_engine, playerState)) {
			needsRedraw = true;
		}
		const bool keyboardAttackRequested =
			attackModifierHeld && (moveLeft || moveRight || moveUp || moveDown);
		if (!playerState.attackActive && keyboardAttackRequested &&
				!idleState.active && !idleState.exiting) {
			const uint32 now = Player::getRuntimeClockTicks();
			if (nextKeyboardAttackAllowedTick == 0 || (int32)(now - nextKeyboardAttackAllowedTick) >= 0) {
				capturePlayerAttackTargetForKeyboard(moveLeft, moveRight, moveUp, moveDown);
				if (Player::startKeyboardAttackAnimation(
						scene.state, playerState, moveLeft, moveRight, moveUp, moveDown)) {
					nextKeyboardAttackAllowedTick = now + kNativeKeyboardAttackRepeatTicks;
					notePlayerActivity();
					debugC(1, kDebugCombat,
						"Harvester: combat player keyboard attack start loadout=%d target_class='%s' target='%s' input=(L=%d R=%d U=%d D=%d) facing=%d",
						playerState.combatLoadout,
						describeCombatTargetClass(playerState.attackTargetClassId),
						playerState.attackTargetName.c_str(),
						moveLeft, moveRight, moveUp, moveDown, playerState.facing);
					needsRedraw = true;
				} else {
					playerState.attackTargetName.clear();
					playerState.attackTargetClassId = -1;
				}
			}
		}
		if (!playerState.attackActive && Player::updateTurnAnimationState(playerState)) {
			playerAdvancedThisFrame = true;
			needsRedraw = true;
		}

		if (!playerState.attackActive && !keyboardAttackRequested && !idleState.active && !idleState.exiting) {
			if (Player::stepKeyboardMovement(_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState, moveLeft, moveRight, moveUp, moveDown)) {
				playerAdvancedThisFrame = true;
				notePlayerActivity();
				needsRedraw = true;
			} else if (Player::stepMoveTarget(
					_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState)) {
				playerAdvancedThisFrame = true;
				notePlayerActivity();
				needsRedraw = true;
			} else if (!moveLeft && !moveRight && !moveUp && !moveDown && !playerState.hasMoveTarget &&
					!playerState.turnActive &&
					playerState.entity && playerState.facing >= 0 &&
					Player::setIdleAnimation(playerState, playerState.facing)) {
				needsRedraw = true;
			}

			if (!showingInspectText && !keyboardAttackRequested &&
					!moveLeft && !moveRight && !moveUp && !moveDown &&
					!playerState.hasMoveTarget && !playerState.turnActive &&
					playerState.entity && playerState.facing >= 0 &&
					!Player::isIdleAnimationExcludedRoom(scene.state.roomName) &&
					Player::getRuntimeClockTicks() > idleState.triggerTick &&
					Player::startIdleAnimation(_engine, scene.state, playerState, idleState)) {
				needsRedraw = true;
			}
		}
		combatError = updateRoomMonsterCombat();
		if (combatError.getCode() != Common::kNoError)
			return combatError;
		if (syncCombatDamageEffects())
			needsRedraw = true;
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		Common::Error pendingRegionError =
			playerState.attackActive ? Common::kNoError : tryActivatePendingRegion();
		if (pendingRegionError.getCode() != Common::kNoError)
			return pendingRegionError;
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		pendingRegionError =
			(playerAdvancedThisFrame && !playerState.attackActive)
				? tryActivateOverlappedRegion()
				: Common::kNoError;
		if (pendingRegionError.getCode() != Common::kNoError)
			return pendingRegionError;
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}

		if (startupFlow.tickRuntimeEntities())
			needsRedraw = true;
		if (pruneCombatDamageEffects())
			needsRedraw = true;
		if (runtimeEntities) {
			Common::Array<Common::String> expiredTimerNames;
			if (runtimeEntities->takeExpiredTimerNames(expiredTimerNames)) {
				syncCurrentRoomRuntimeState();
				for (const Common::String &timerName : expiredTimerNames) {
					StartupInteractionResult timerInteraction;
					if (!_engine.getStartupScript()->executeTimerAction(timerName, timerInteraction))
						continue;

					bool didTransition = false;
					Common::Error interactionError =
						interactionProcessor.handleInteractionResult(
							timerInteraction, didTransition, Common::String());
					if (interactionError.getCode() != Common::kNoError)
						return interactionError;
					if (startupFlow.hasPendingMainMenuReturn())
						return Common::kNoError;
					needsRedraw = true;
					if (!pendingRoomChange.empty())
						break;
				}
			}
		}
		if (Player::updateIdleAnimation(scene.state, playerState, idleState))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}
	if (_engine.shouldQuit())
		return Common::kNoError;
	if (startupFlow.hasPendingMainMenuReturn())
		return Common::kNoError;
	if (pendingRoomChange.empty())
		return Common::kNoError;

	currentRoomTarget = pendingRoomChange;
	}

	return Common::kNoError;
}

} // End of namespace Harvester
