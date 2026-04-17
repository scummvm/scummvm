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
#include "harvester/room_combat.h"
#include "harvester/room_interaction.h"
#include "harvester/room_support.h"

namespace Harvester {

static const uint32 kRoomExitFastClickWindowTicks = 20;
static const char *const kPlayerInventoryLabel = "your inventory";
static const char *const kInventoryOwnerName = "INVENTORY";
static const byte kTransparentPaletteIndex = 0;
static const float kNormalPaletteBrightness = 1.0f;
static const float kDimPaletteBrightness = 0.6f;
static const int kNativeInventoryDragCloseLeft = 0x45;
static const int kNativeInventoryDragCloseTop = 0x4b;
static const int kNativeInventoryDragCloseRight = 0x239;
static const int kNativeInventoryDragCloseBottom = 0x1b4;
static const int kNativeInventoryTooltipX = 0xbc;
static const int kNativeInventoryTooltipY = 0x19e;
static const int kNativeInventoryWeekdayX = 0x1e0;
static const int kNativeInventoryWeekdayY = 0x18c;
static const byte kNativeInventoryTooltipColor = 0xf4;
static const uint32 kCombatLoadoutStatusDisplayTicks = 75;
static const int kNativeCombatResourceIconsX = 10;
static const int kNativeCombatResourceIconsY = 30;
static const int kNativeCombatResourceIconSpacing = 15;
static const char *const kCdChangePromptPalettePath = "1:/GRAPHIC/PAL/CD1.PAL";

static bool roomAllowsImmediateExitClick(const Common::String &roomName) {
	return !roomName.equalsIgnoreCase("LAVAPIT") &&
		!roomName.equalsIgnoreCase("BOWLSNTRY1");
}

static bool isOutsideNativeInventoryDragCloseBounds(const Common::Point &point) {
	return point.x < kNativeInventoryDragCloseLeft ||
		point.x >= kNativeInventoryDragCloseRight ||
		point.y < kNativeInventoryDragCloseTop ||
		point.y >= kNativeInventoryDragCloseBottom;
}

static void debugLogInventoryClick(const char *buttonLabel, const Common::Point &point,
		const InventoryVisual *inventoryHover) {
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
	const Text *text = engine.getText();
	if (!text || !fontName)
		return nullptr;

	for (const CftFontResource &font : text->getFonts()) {
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

static Common::String resolveStartupNpcLabel(const NpcRecord &npc) {
	Common::String label = !npc.entityInitArg.empty() ? npc.entityInitArg : npc.npcName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

static Common::String resolveCarriedObjectSpritePath(const ObjectRecord &object) {
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

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *palette) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(palette, data.data(), 256 * 3);
	return true;
}

static void applyRoomPalette(Graphics::Screen &screen, const HarvesterEngine &engine,
		const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	buildHarvesterDisplayPalette(palette, brightness * engine.getGammaBrightnessScale(), scaledPalette);
	screen.setPalette(scaledPalette);
}

static void renderCdChangePromptScreen(HarvesterEngine &engine, const IndexedBitmap &bitmap,
		const byte *palette) {
	Graphics::Screen *screen = engine.getScreen();
	if (!screen)
		return;

	applyRoomPalette(*screen, engine, palette, 1.0f);
	screen->fillRect(screen->getBounds(), 0);
	if (bitmap.isValid())
		screen->copyRectToSurface(bitmap.pixels.data(), bitmap.width, 0, 0, bitmap.width, bitmap.height);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

static bool shouldDispatchPickupActionOnCarryStart(const ObjectRecord &object) {
	if (object.actionTag.empty())
		return false;

	return !object.objectName.equalsIgnoreCase("SANDWICH") &&
		!object.objectName.equalsIgnoreCase("SANDWICH2") &&
		!object.objectName.equalsIgnoreCase("SYRINGE");
}

static bool shouldDispatchPickupActionOnDirectInventoryTransfer(const ObjectRecord &object) {
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

static void drawCombatLoadoutResourceIcons(Graphics::Screen &screen, const Art &art,
		const Script &script, int loadout) {
	CombatLoadoutHudInfo info;
	if (!resolveCombatLoadoutHudInfo(loadout, info))
		return;

	const Common::Array<IndexedBitmap> &ammoIcons = art.getAmmoIcons();
	if (info.iconIndex >= ammoIcons.size())
		return;

	const IndexedBitmap &icon = ammoIcons[info.iconIndex];
	if (!icon.isValid())
		return;

	const int iconCount = CLIP<int>(script.getPlayerCombatResourceCount(loadout), 0, info.maxVisibleCount);
	for (int i = 0; i < iconCount; ++i)
		blitBitmap(screen, icon, kNativeCombatResourceIconsX + i * kNativeCombatResourceIconSpacing,
			kNativeCombatResourceIconsY);
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

static void setScaledRoomPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

RoomSystem::RoomSystem(HarvesterEngine &engine, Common::Point &mousePos,
		InventorySystem &inventory)
	: _engine(engine), _mousePos(mousePos), _inventory(inventory) {
}

Common::Error RoomSystem::runRoomLoop(Flow &flow, const Common::String &targetName,
		bool targetIsRoomName) {
	if (flow.hasPendingMainMenuReturn())
		return Common::kNoError;

	Common::String currentRoomTarget = targetName;
	bool currentTargetIsRoomName = targetIsRoomName;
	bool currentTargetUsesSavedRoomState = false;
	SaveRoomState currentTargetSavedRoomState;
	while (!currentRoomTarget.empty()) {
		RoomSetupState state;
		bool shouldRunRoomEntryCommands = false;
		if (_engine.hasPendingLoadedSaveRoomState()) {
			// Native load destroys live render entities and frees preserved global timers
			// before room_setup rebuilds the loaded room from restored TIMER records.
			if (EntityManager *entityManager = _engine.getRuntimeEntities())
				entityManager->clearSceneEntities(false);
			flow.resetRoomNpcDialogueState();
			if (_engine.hasPendingLoadedDialogueStateBlob()) {
				if (!flow.loadDialogueSaveStateBlob(
						_engine.getPendingLoadedDialogueStateBlob(),
						_engine.getPendingLoadedDialogueStateBlobVersion()))
					return Common::kReadingFailed;
				_engine.clearPendingLoadedDialogueStateBlob();
			}
			if (_engine.getPendingLoadedDisc() > 0 &&
					!_engine.activateDisc(_engine.getPendingLoadedDisc())) {
				return Common::kReadingFailed;
			}
			const SaveRoomState &loadedState = _engine.getPendingLoadedSaveRoomState();
			debugC(1, kDebugRoom,
				"Harvester: applying pending loaded room state entrance='%s' room='%s' spawn=(%d,%d,%d) facing=%d music='%s'",
				loadedState.entranceName.c_str(), loadedState.roomName.c_str(),
				loadedState.playerX, loadedState.playerY, loadedState.playerZ,
				loadedState.playerFacing, loadedState.musicPath.c_str());
			if (!_engine.getScript()->materializeRoomState(
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
			_engine.clearPendingLoadedSaveRoomState();
			currentTargetIsRoomName = false;
			shouldRunRoomEntryCommands = false;
		} else if ((currentTargetIsRoomName
				? !_engine.getScript()->resolveRoomSetupStateByRoomName(
					currentRoomTarget, state, *_engine.getResources())
				: !_engine.getScript()->resolveRoomSetupState(
					currentRoomTarget, state, *_engine.getResources()))) {
			return Common::kReadingFailed;
		} else {
			shouldRunRoomEntryCommands = true;
		}
		if (shouldRunRoomEntryCommands && currentTargetUsesSavedRoomState &&
				currentTargetSavedRoomState.valid) {
			const SaveRoomState &savedState = currentTargetSavedRoomState;
			state.entranceName = savedState.entranceName;
			state.playerSpawnX = savedState.playerX;
			state.playerSpawnY = savedState.playerY;
			state.playerSpawnZ = savedState.playerZ;
			state.playerFacing = savedState.playerFacing;
			if (!savedState.musicPath.empty())
				state.musicPath = savedState.musicPath;
			debugC(1, kDebugRoom,
				"Harvester: applying saved room restart state entrance='%s' room='%s' spawn=(%d,%d,%d) facing=%d music='%s'",
				savedState.entranceName.c_str(), savedState.roomName.c_str(),
				savedState.playerX, savedState.playerY, savedState.playerZ,
				savedState.playerFacing, savedState.musicPath.c_str());
		}

		Common::Error transitionError = flow.beginRoomSetupTransition();
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;

		RoomSceneResources scene;
		if (!loadRoomSceneResources(state, *_engine.getResources(), scene))
			return Common::kReadingFailed;

		auto getActiveScreen = [&]() -> Graphics::Screen * {
			return _engine.getScreen();
		};
		Graphics::Screen *screen = getActiveScreen();
		const Art *art = _engine.getArt();
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
		EntityManager *entityManager = _engine.getRuntimeEntities();

		if (!flow.populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return Common::kReadingFailed;
		if (entityManager) {
			for (const MonsterRecord &monster : scene.state.roomMonsters) {
				Entity *entity = entityManager->findSceneEntityByName(monster.monsterName);
				if (entity)
					Monster::applyAnimation(*entity, monster);
			}
			entityManager->pauseTimerCountdowns();
		}

		flow.resetCursorAnimationSequence();
		flow.executeStartupAudioCommands(scene.state.audioCommands);
		if (!scene.state.musicPath.empty())
			(void)_engine.playMusic(scene.state.musicPath);
		RoomPlayerState playerState;
		playerState.entity = entityManager ? entityManager->findSceneEntityByName("PLAYER") : nullptr;
		playerState.centerX = scene.state.playerSpawnX;
		playerState.bottomY = scene.state.playerSpawnY;
		playerState.z = (float)scene.state.playerSpawnZ;
		playerState.facing = scene.state.playerFacing;
		playerState.combatLoadout =
			_engine.getScript() ? _engine.getScript()->getPlayerCombatLoadout() : 0;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		Common::Array<RoomMonsterCombatState> monsterCombatStates;
		monsterCombatStates.resize(scene.state.roomMonsters.size());
		Common::Array<RoomNpcCombatState> npcCombatStates;
		npcCombatStates.resize(scene.state.roomNpcs.size());
		Common::Array<RoomHitEffectState> hitEffectStates;
		Common::Array<RoomCombatDamagePopupState> damagePopupStates;
		uint nextCombatEffectId = 0;
		ResolvedText inspectText;
		bool showingInspectText = false;
		bool inspectCanDismiss = false;
		ResolvedText combatLoadoutStatusText;
		uint32 combatLoadoutStatusHideTick = 0;
		bool closeInventoryAfterCombatLoadoutStatus = false;
		bool moveLeft = false;
		bool moveRight = false;
		bool moveUp = false;
		bool moveDown = false;
		bool attackModifierHeld = false;
		Common::String pendingRegionName;
		Common::String pendingRoomChange;
		bool pendingRoomChangeIsRoomName = false;
		bool pendingRoomChangeUsesSavedRoomState = false;
		SaveRoomState pendingRoomChangeSavedRoomState;
		const bool canExitCloseupToParent = flow._roomLoopDepth > 1;
		uint32 lastLeftButtonReleaseTick = 0;
		uint32 nextKeyboardAttackAllowedTick = 0;
		RoomIdleAnimationState idleState;
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
		auto queueCombatLoadoutStatusMessage = [&](const Common::String &message) {
			if (message.empty()) {
				const bool hadMessage = !combatLoadoutStatusText.value.empty();
				combatLoadoutStatusText = ResolvedText();
				combatLoadoutStatusHideTick = 0;
				if (hadMessage)
					needsRedraw = true;
				return;
			}

			combatLoadoutStatusText.boxName = "BOX1";
			combatLoadoutStatusText.value = message;
			combatLoadoutStatusHideTick =
				Player::getRuntimeClockTicks() + kCombatLoadoutStatusDisplayTicks;
			needsRedraw = true;
		};
		auto hideSceneObject = [&](const Common::String &objectName, const Common::String *ownerOrRoom) {
			ObjectRecord *sceneObject = findSceneObjectByName(scene.sceneObjects, objectName);
			if (!sceneObject)
				return;

			sceneObject->visible = false;
			sceneObject->runtimeVisible = false;
			if (ownerOrRoom)
				sceneObject->currentOwnerOrRoom = *ownerOrRoom;
		};
		auto removeSceneEntityByName = [&](const Common::String &entityName) {
			if (!entityManager)
				return;

			Entity *entity = entityManager->detachSceneEntityByName(entityName);
			delete entity;
		};
		auto resetIdleState = [&]() {
			idleState = RoomIdleAnimationState();
			idleState.activityTick = Player::getRuntimeClockTicks();
			idleState.resetTick = idleState.activityTick;
			Player::updateIdleTrigger(idleState);
		};
		auto notePlayerActivity = [&]() {
			idleState.activityTick = Player::getRuntimeClockTicks();
			Player::updateIdleTrigger(idleState);
		};
		auto isPlayerCombatLocked = [&]() {
			return playerState.attackActive || playerState.hitActive || playerState.deathActive;
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
			playerState.attackSoundPlaybackFrame = -1;
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
			Script *script = _engine.getScript();
			if (!script)
				return;

			// Region enable state is a persistent script mutation; keep the runtime record in lockstep
			// with the currently materialized scene so one-shot regions do not resurrect on revisit.
			for (const RegionRecord &baseRegion : script->getRegions()) {
				if (!baseRegion.roomName.equalsIgnoreCase(scene.state.roomName))
					continue;

				const bool enabled =
					findSceneRegionByName(scene.sceneRegions, baseRegion.regionName) != nullptr;
				(void)script->setRuntimeRegionEnabled(baseRegion.regionName, enabled);
			}

			if (entityManager) {
				for (AnimRecord &anim : scene.state.roomAnimations) {
					Entity *entity = entityManager->findSceneEntityByName(anim.animName);
					if (!entity)
						continue;
					(void)script->syncRuntimeAnimState(anim.animName,
						entity->isAnimationEnabled(), anim.visible, entity->getCurrentFrame());
				}
			}
			for (const MonsterRecord &monster : scene.state.roomMonsters)
				(void)script->syncRuntimeMonsterRecord(monster);
			if (!entityManager)
				return;

			for (const TimerRecord &baseTimer : script->getTimers()) {
				Entity *entity = entityManager->findSceneEntityByName(baseTimer.timerName);
				if (!entity || entity->getClassId() != kRuntimeEntityClassTimer)
					continue;

				const TimerRecord *runtimeTimer = script->findRuntimeTimerRecord(baseTimer.timerName);
				TimerRecord timer = runtimeTimer ? *runtimeTimer : baseTimer;
				timer.currentValue = entity->getTimerCurrentValue();
				timer.enabled = entity->isTimerEnabled();
				timer.looping = entity->isTimerLooping();
				timer.global = entity->isTimerGlobal();
				(void)script->syncRuntimeTimerRecord(timer);
			}
		};
		auto captureCurrentSaveState = [&]() {
			syncCurrentRoomRuntimeState();
			const int facing = playerState.facing >= 0 ? playerState.facing : scene.state.playerFacing;
			_engine.captureCurrentSaveRoomState(scene.state.entranceName, scene.state.roomName,
				playerState.centerX, playerState.bottomY, (int)playerState.z, facing,
				scene.state.discNumber, _engine.getMusicPath());
		};
		auto getSceneObjectBounds = [&](const ObjectRecord &object) {
			if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
				return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);
			return Common::Rect();
		};
		auto resolveSceneObjectSpritePathLocal = [&](const ObjectRecord &object) {
			const bool atInitialPlacement = object.currentX == object.initialX &&
				object.currentY == object.initialY &&
				object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
			if (!object.altSpritePath.empty() &&
					(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)))
				return object.altSpritePath;
			return object.spritePath;
		};
		auto isInteractiveSceneHotspot = [&](const ObjectRecord &object) {
			if (object.operatable || !object.actionTag.empty())
				return true;

			Script *script = _engine.getScript();
			if (!script)
				return false;
			if (script->isPickupObject(object))
				return true;

			ResolvedText inspectText;
			return script->resolveObjectInspectText(object, inspectText);
		};
		auto resolveSceneObjectClassLocal = [&](const ObjectRecord &object,
				const Entity *entity) {
			if (entity) {
				return ((!scene.state.backgroundObjectName.empty() &&
						object.objectName.equalsIgnoreCase(scene.state.backgroundObjectName)) ||
					(object.initialX == 0 && object.initialY == 0 &&
						entity->getBoundsWidth() == 640 && entity->getBoundsHeight() == 480))
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
		auto spawnSceneObjectEntityFromRecord = [&](const ObjectRecord &object) {
			if (!entityManager)
				return;

			const Common::String spritePath = resolveSceneObjectSpritePathLocal(object);
			const Common::Rect hotspotBounds = getSceneObjectBounds(object);
			Entity *entity = nullptr;
			if (!spritePath.empty() && spritePath.hasSuffixIgnoreCase(".BM")) {
				entity = entityManager->spawnSceneBitmapEntity(object.objectName, spritePath,
					Common::Point(object.currentX, object.currentY), (float)object.currentZ);
			} else if (!hotspotBounds.isEmpty()) {
				entity = entityManager->spawnSceneHotspotEntity(object.objectName, hotspotBounds,
					(float)object.currentZ);
			}
			if (!entity)
				return;

			entity->setClassId(resolveSceneObjectClassLocal(
				object, entity->hasFrames() ? entity : nullptr));
			entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
			entity->setZExtent((float)object.zExtent);
		};
		auto getSceneRegionBounds = [](const RegionRecord &region) {
			if (region.right > region.left && region.bottom > region.top)
				return Common::Rect(region.left, region.top, region.right + 1, region.bottom + 1);
			return Common::Rect();
		};
		auto spawnSceneRegionEntityFromRecord = [&](const RegionRecord &region) {
			if (!entityManager)
				return;

			const Common::Rect bounds = getSceneRegionBounds(region);
			if (bounds.isEmpty())
				return;

			Entity *entity = entityManager->spawnSceneHotspotEntity(
				region.regionName, bounds, (float)region.minZ);
			if (!entity)
				return;

			entity->setClassId(kRuntimeEntityClassRectHotspot19);
			entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
			entity->setZExtent((float)MAX(0, region.maxZ - region.minZ));
			if (!region.cursorEnabled)
				entity->setHitTestMode(kRuntimeEntityHitTestNone);
			debugC(1, kDebugRoom,
				"Harvester: scene region spawned room='%s' region='%s' class=0x%x bounds=(%d,%d)-(%d,%d) z=[%d,%d] facing=%d cursor=%d action='%s'",
				region.roomName.c_str(), region.regionName.c_str(), entity->getClassId(),
				bounds.left, bounds.top, bounds.right, bounds.bottom,
				region.minZ, region.maxZ, region.desiredFacing, region.cursorEnabled, region.actionTag.c_str());
		};
		auto findRoomAnimByNameConst = [](const Common::Array<AnimRecord> &anims,
				const Common::String &animName) -> const AnimRecord * {
			for (const AnimRecord &anim : anims) {
				if (anim.animName.equalsIgnoreCase(animName))
					return &anim;
			}
			return nullptr;
		};
		auto findRoomNpcByNameConst = [](const Common::Array<NpcRecord> &npcs,
				const Common::String &npcName) -> const NpcRecord * {
			for (const NpcRecord &npc : npcs) {
				if (npc.npcName.equalsIgnoreCase(npcName))
					return &npc;
			}
			return nullptr;
		};
		auto findRoomMonsterByNameConst = [](const Common::Array<MonsterRecord> &monsters,
				const Common::String &monsterName) -> const MonsterRecord * {
			for (const MonsterRecord &monster : monsters) {
				if (monster.monsterName.equalsIgnoreCase(monsterName))
					return &monster;
			}
			return nullptr;
		};
		auto shouldSpawnRoomNpcEntity = [](const NpcRecord &npc) {
			return npc.visible && !npc.deathOrMonsterfyFlag;
		};
		auto shouldSpawnRoomMonsterEntity = [](const MonsterRecord &monster) {
			return monster.visible;
		};
		auto sameNpcEntityState = [&](const NpcRecord &lhs, const NpcRecord &rhs) {
			return lhs.posX == rhs.posX &&
				lhs.posY == rhs.posY &&
				lhs.posZ == rhs.posZ &&
				lhs.frameDelay == rhs.frameDelay &&
				lhs.active == rhs.active &&
				lhs.visible == rhs.visible &&
				lhs.runtimeSpawned == rhs.runtimeSpawned &&
				lhs.runtimeState == rhs.runtimeState &&
				lhs.deathDamageType == rhs.deathDamageType &&
				lhs.deathOrMonsterfyFlag == rhs.deathOrMonsterfyFlag &&
				lhs.modelPath.equalsIgnoreCase(rhs.modelPath);
		};
		auto isQueuedLiveNpcDeathMutation = [&](const NpcRecord &lhs, const NpcRecord &rhs) {
			return lhs.posX == rhs.posX &&
				lhs.posY == rhs.posY &&
				lhs.posZ == rhs.posZ &&
				lhs.frameDelay == rhs.frameDelay &&
				lhs.active &&
				rhs.active &&
				lhs.visible &&
				rhs.visible &&
				!lhs.runtimeSpawned &&
				!rhs.runtimeSpawned &&
				lhs.runtimeState < 0 &&
				rhs.runtimeState < 0 &&
				!lhs.deathOrMonsterfyFlag &&
				!rhs.deathOrMonsterfyFlag &&
				lhs.deathDamageType == 0 &&
				rhs.deathDamageType != 0 &&
				lhs.modelPath.equalsIgnoreCase(rhs.modelPath);
		};
		auto sameMonsterEntityState = [&](const MonsterRecord &lhs, const MonsterRecord &rhs) {
			return lhs.posX == rhs.posX &&
				lhs.posY == rhs.posY &&
				lhs.posZ == rhs.posZ &&
				lhs.facing == rhs.facing &&
				lhs.active == rhs.active &&
				lhs.visible == rhs.visible &&
				lhs.runtimeSpawned == rhs.runtimeSpawned &&
				lhs.runtimeState == rhs.runtimeState &&
				lhs.modelPath.equalsIgnoreCase(rhs.modelPath);
		};
		auto sameSceneObjectState = [&](const ObjectRecord &lhs, const ObjectRecord &rhs) {
			return lhs.currentX == rhs.currentX &&
				lhs.currentY == rhs.currentY &&
				lhs.currentZ == rhs.currentZ &&
				lhs.zExtent == rhs.zExtent &&
				lhs.operatable == rhs.operatable &&
				lhs.visible == rhs.visible &&
				lhs.runtimeVisible == rhs.runtimeVisible &&
				lhs.actionTag.equalsIgnoreCase(rhs.actionTag) &&
				lhs.identTextKey.equalsIgnoreCase(rhs.identTextKey) &&
				lhs.interactionLabel.equalsIgnoreCase(rhs.interactionLabel) &&
				lhs.currentOwnerOrRoom.equalsIgnoreCase(rhs.currentOwnerOrRoom) &&
				resolveSceneObjectSpritePathLocal(lhs).equalsIgnoreCase(resolveSceneObjectSpritePathLocal(rhs)) &&
				sameRect(getSceneObjectBounds(lhs), getSceneObjectBounds(rhs));
		};
		auto sameSceneRegionState = [&](const RegionRecord &lhs, const RegionRecord &rhs) {
			return lhs.left == rhs.left &&
				lhs.top == rhs.top &&
				lhs.right == rhs.right &&
				lhs.bottom == rhs.bottom &&
				lhs.minZ == rhs.minZ &&
				lhs.maxZ == rhs.maxZ &&
				lhs.startEnabled == rhs.startEnabled &&
				lhs.cursorEnabled == rhs.cursorEnabled &&
				lhs.desiredFacing == rhs.desiredFacing &&
				lhs.actionTag.equalsIgnoreCase(rhs.actionTag);
		};
		auto applyRoomNpcPlacement = [&](Entity &entity, const NpcRecord &npc) {
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
		auto spawnSceneNpcEntityFromRecord = [&](const NpcRecord &npc) -> Entity * {
			const bool shouldSpawnNpc = shouldSpawnRoomNpcEntity(npc);
			const bool preservedCorpse = !npc.active &&
				npc.runtimeSpawned &&
				npc.deathDamageType != 0 &&
				npc.runtimeState >= 0;
			if (!entityManager || !shouldSpawnNpc)
				return nullptr;

			const int initialFrame = preservedCorpse ? npc.runtimeState : 0;
			Entity *entity = entityManager->spawnSceneActorEntity(
				npc.npcName, npc.modelPath, Common::Point(npc.posX, npc.posY), (float)npc.posZ, initialFrame);
			if (!entity)
				return nullptr;

			entity->setClassId(kRuntimeEntityClassNpc);
			entity->setZExtent(kNativeNpcMonsterZExtent);
			entity->setHitTestMode(preservedCorpse ? kRuntimeEntityHitTestNone : kRuntimeEntityHitTestOpaquePixels);
			if (preservedCorpse) {
				const int corpseFrame = MIN(entity->getLastFrame(), npc.runtimeState);
				entity->setAnimationFrameRange(corpseFrame, corpseFrame, false);
				entity->setAnimationRate(0);
				entity->setCurrentFrame(corpseFrame);
				entity->setAnimationEnabled(false);
			} else {
				entity->setAnimationFrameRange(0, MIN(entity->getLastFrame(), kRoomNpcAmbientLastFrame), true);
				entity->setAnimationRate(npc.frameDelay > 0 ? npc.frameDelay : 0);
			}
			entity->setVisible(true);
			if (!applyRoomNpcPlacement(*entity, npc)) {
				removeSceneEntityByName(npc.npcName);
				return nullptr;
			}
			entityManager->reinsertSceneEntity(entity);
			return entity;
		};
		auto spawnSceneMonsterEntityFromRecord = [&](const RoomSetupState &roomState,
				const MonsterRecord &monster) -> Entity * {
			if (!entityManager || !shouldSpawnRoomMonsterEntity(monster))
				return nullptr;

			Entity *entity = entityManager->spawnSceneActorEntity(
				monster.monsterName, monster.modelPath,
				Common::Point(monster.posX, monster.posY), (float)monster.posZ,
				Monster::resolveFacingFrame(monster.facing));
			if (!entity)
				return nullptr;

			entity->setClassId(kRuntimeEntityClassMonster);
			entity->setZExtent(kNativeNpcMonsterZExtent);
			entity->setHitTestMode(kRuntimeEntityHitTestNone);
			entity->setVisible(monster.visible);
			Monster::applyAnimation(*entity, monster);
			if (!applyRoomActorPlacement(roomState, *entity, monster.posX, monster.posY, (float)monster.posZ)) {
				removeSceneEntityByName(monster.monsterName);
				return nullptr;
			}
			entityManager->reinsertSceneEntity(entity);
			return entity;
		};
		auto resolveHitEffectAnchor = [&](const Entity &targetEntity,
				Common::Point &anchorPoint, float &renderZ) {
			const Common::Rect targetRect = targetEntity.getScreenRect();
			anchorPoint.x = targetRect.left + targetRect.width() / 2;
			anchorPoint.y = targetRect.top + targetRect.height() / 3;
			renderZ = targetEntity.getZ() - kNativeHitEffectRenderZBias;
		};
		auto resolveCombatDamagePopupAnchor = [&](const Entity &targetEntity,
				Common::Point &anchorPoint) {
			const Common::Rect targetRect = targetEntity.getScreenRect();
			anchorPoint.x = targetRect.left + targetRect.width() / 2;
			anchorPoint.y = targetRect.top - kCombatDebugDamagePopupVerticalGap;
		};
		auto spawnCombatHitEffect = [&](Entity &sourceEntity,
				const Common::String &followTargetName) {
			if (!entityManager)
				return false;

			RoomHitEffectState effectState;
			effectState.entityName = Common::String::format("__combat_blood_%u", nextCombatEffectId++);
			effectState.followTargetName = followTargetName;
			resolveHitEffectAnchor(sourceEntity, effectState.anchorPoint, effectState.renderZ);

			Entity *effectEntity = entityManager->spawnSceneAnimationEntity(
				effectState.entityName, kNativeHitEffectResourcePath, effectState.anchorPoint,
				effectState.renderZ, kNativeHitEffectAnimationRate, true, true, false,
				false, false, 0);
			if (!effectEntity)
				return false;

			hitEffectStates.push_back(effectState);
			return true;
		};
		auto spawnCombatDamagePopup = [&](Entity &targetEntity,
				const Common::String &followTargetName, int damageAmount) {
			if (damageAmount <= 0)
				return;

			RoomCombatDamagePopupState popupState;
			popupState.followTargetName = followTargetName;
			popupState.startTick = Player::getRuntimeClockTicks();
			popupState.damageAmount = damageAmount;
			resolveCombatDamagePopupAnchor(targetEntity, popupState.anchorPoint);
			damagePopupStates.push_back(popupState);
		};
		auto syncCombatHitEffects = [&]() {
			if (!entityManager)
				return false;

			bool changed = false;
			for (RoomHitEffectState &effectState : hitEffectStates) {
				Entity *effectEntity = entityManager->findSceneEntityByName(effectState.entityName);
				if (!effectEntity)
					continue;

				if (!effectState.followTargetName.empty()) {
					Entity *targetEntity =
						entityManager->findSceneEntityByName(effectState.followTargetName);
					if (targetEntity && targetEntity->isVisible()) {
						resolveHitEffectAnchor(*targetEntity, effectState.anchorPoint, effectState.renderZ);
					}
				}

				if (effectEntity->getX() != effectState.anchorPoint.x ||
						effectEntity->getY() != effectState.anchorPoint.y ||
						fabsf(effectEntity->getZ() - effectState.renderZ) > 0.0001f) {
					effectEntity->setPosition(
						effectState.anchorPoint.x, effectState.anchorPoint.y, effectState.renderZ);
					entityManager->reinsertSceneEntity(effectEntity);
					changed = true;
				}
			}

			return changed;
		};
		auto drawCombatDamagePopups = [&](Graphics::Screen &screen) {
			if (!_engine.isCombatDebugEnabled() || damagePopupStates.empty())
				return;

			const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
			if (!font)
				return;

			byte displayPalette[256 * 3];
			screen.getPalette(displayPalette);
			const byte white = findNearestPaletteColor(displayPalette, 0xff, 0xff, 0xff);
			const byte black = findNearestPaletteColor(displayPalette, 0x00, 0x00, 0x00);
			const uint32 now = Player::getRuntimeClockTicks();
			for (RoomCombatDamagePopupState &popupState : damagePopupStates) {
				if (entityManager && !popupState.followTargetName.empty()) {
					Entity *targetEntity =
						entityManager->findSceneEntityByName(popupState.followTargetName);
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
		};
		auto pruneCombatHitEffects = [&]() {
			bool removedAny = false;
			for (uint i = 0; i < hitEffectStates.size();) {
				const Common::String effectName = hitEffectStates[i].entityName;
				Entity *effectEntity = entityManager
					? entityManager->findSceneEntityByName(effectName)
					: nullptr;
				if (!effectEntity) {
					hitEffectStates.remove_at(i);
					continue;
				}

				const bool finished =
					!effectEntity->isAnimationEnabled() &&
					effectEntity->getCurrentFrame() == effectEntity->getLastFrame();
				if (finished && hitEffectStates[i].finished) {
					removeSceneEntityByName(effectName);
					hitEffectStates.remove_at(i);
					removedAny = true;
					continue;
				}

				hitEffectStates[i].finished = finished;
				++i;
			}

			return removedAny;
		};
		auto pruneCombatDamagePopups = [&]() {
			const uint32 now = Player::getRuntimeClockTicks();
			bool removedAny = false;
			for (uint i = 0; i < damagePopupStates.size();) {
				if (now - damagePopupStates[i].startTick < kCombatDebugDamagePopupDurationMs) {
					++i;
					continue;
				}

				damagePopupStates.remove_at(i);
				removedAny = true;
			}

			return removedAny;
		};
		auto isTimerMaterializedInCurrentRoom = [&](const Common::String &timerName) {
			for (const TimerRecord &timer : scene.state.roomTimers) {
				if (timer.timerName.equalsIgnoreCase(timerName))
					return true;
			}

			return false;
		};
		auto syncGlobalTimerEntities =
				[&](const Common::Array<TimerRecord> &previousTimerRecords,
						bool includeCurrentRoomTimers = false) {
			Script *script = _engine.getScript();
			if (!script || !entityManager || previousTimerRecords.empty())
				return;

			for (const TimerRecord &previousTimer : previousTimerRecords) {
				const TimerRecord *runtimeTimer =
					script->findRuntimeTimerRecord(previousTimer.timerName);
				if (!runtimeTimer || !runtimeTimer->global ||
						(!includeCurrentRoomTimers &&
							isTimerMaterializedInCurrentRoom(runtimeTimer->timerName))) {
					continue;
				}

				Entity *entity = entityManager->findSceneEntityByName(runtimeTimer->timerName);
				if (entity && entity->getClassId() != kRuntimeEntityClassTimer)
					continue;

				if (!entity) {
					(void)script->syncRuntimeTimerRecord(previousTimer);
					debugC(1, kDebugRoom,
						"Harvester: global timer mutation ignored current_room='%s' timer='%s' owner_room='%s' reason='missing live timer entity'",
						scene.state.roomName.c_str(), previousTimer.timerName.c_str(),
						previousTimer.arg1.c_str());
					continue;
				}

				if (entity->getTimerInitialValue() == runtimeTimer->initialValue &&
						entity->isTimerEnabled() == runtimeTimer->enabled &&
						entity->isTimerLooping() == runtimeTimer->looping &&
						entity->isTimerGlobal() == runtimeTimer->global) {
					continue;
				}

				entityManager->configureSceneTimerEntity(*entity,
					runtimeTimer->initialValue, runtimeTimer->currentValue,
					runtimeTimer->enabled, runtimeTimer->looping, runtimeTimer->global);
				debugC(1, kDebugRoom,
					"Harvester: global timer synced current_room='%s' timer='%s' owner_room='%s' current=%d initial=%d enabled=%d loop=%d",
					scene.state.roomName.c_str(), runtimeTimer->timerName.c_str(),
					runtimeTimer->arg1.c_str(), runtimeTimer->currentValue,
					runtimeTimer->initialValue, runtimeTimer->enabled,
					runtimeTimer->looping);
			}
		};
		auto applyCurrentRoomRuntimeMutationsInPlace = [&](bool preservePaletteState) {
			Script *script = _engine.getScript();
			ResourceManager *resources = _engine.getResources();
			if (!script || !resources)
				return false;

			byte previousPalette[256 * 3];
			memcpy(previousPalette, scene.palette, sizeof(previousPalette));
			const float previousPaletteBrightness = scene.targetPaletteBrightness;
			const Common::Array<AudioCommand> entryAudioCommands = scene.state.audioCommands;
			RoomSetupState updatedState;
			if (!script->materializeRoomState(
					scene.state.entranceName, scene.state.roomName, updatedState, *resources)) {
				return false;
			}
			updatedState.audioCommands = entryAudioCommands;

			RoomSceneResources updatedScene;
			if (!loadRoomSceneResources(updatedState, *resources, updatedScene))
				return false;
			Common::Array<RoomMonsterCombatState> updatedMonsterCombatStates;
			updatedMonsterCombatStates.resize(updatedState.roomMonsters.size());
			for (uint i = 0; i < updatedState.roomMonsters.size(); ++i) {
				const MonsterRecord &updatedMonster = updatedState.roomMonsters[i];
				for (uint j = 0; j < scene.state.roomMonsters.size() &&
						j < monsterCombatStates.size(); ++j) {
					const MonsterRecord &previousMonster = scene.state.roomMonsters[j];
					if (!previousMonster.monsterName.equalsIgnoreCase(updatedMonster.monsterName))
						continue;
					if (sameMonsterEntityState(previousMonster, updatedMonster))
						updatedMonsterCombatStates[i] = monsterCombatStates[j];
					break;
				}
			}

			for (const ObjectRecord &object : scene.sceneObjects) {
				if (!findSceneObjectByName(updatedScene.sceneObjects, object.objectName))
					removeSceneEntityByName(object.objectName);
			}
			for (const ObjectRecord &object : updatedScene.sceneObjects) {
				const ObjectRecord *previous = findSceneObjectByName(scene.sceneObjects, object.objectName);
				const bool changed = !previous || !sameSceneObjectState(*previous, object);
				if (!changed)
					continue;

				removeSceneEntityByName(object.objectName);
				spawnSceneObjectEntityFromRecord(object);
			}

			for (const RegionRecord &region : scene.sceneRegions) {
				if (!findSceneRegionByName(updatedScene.sceneRegions, region.regionName))
					removeSceneEntityByName(region.regionName);
			}
			for (const RegionRecord &region : updatedScene.sceneRegions) {
				const RegionRecord *previous =
					findSceneRegionByName(scene.sceneRegions, region.regionName);
				const bool changed = !previous || !sameSceneRegionState(*previous, region);
				if (!changed)
					continue;

				removeSceneEntityByName(region.regionName);
				spawnSceneRegionEntityFromRecord(region);
			}

			for (const TimerRecord &timer : scene.state.roomTimers) {
				bool found = false;
				for (const TimerRecord &updatedTimer : updatedState.roomTimers) {
					if (updatedTimer.timerName.equalsIgnoreCase(timer.timerName)) {
						found = true;
						break;
					}
				}
				if (!found)
					removeSceneEntityByName(timer.timerName);
			}
			for (const TimerRecord &timer : updatedState.roomTimers) {
				const TimerRecord *previous = nullptr;
				for (const TimerRecord &candidate : scene.state.roomTimers) {
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
				if (entityManager) {
					(void)entityManager->spawnSceneTimerEntity(
						timer.timerName, timer.initialValue, timer.currentValue,
						timer.enabled, timer.looping, timer.global);
				}
			}

			for (AnimRecord &anim : updatedState.roomAnimations) {
				const AnimRecord *previous =
					findRoomAnimByNameConst(scene.state.roomAnimations, anim.animName);
				const bool wasDisplayed = previous && shouldDisplaySceneAnimation(*previous);
				const bool isDisplayed = shouldDisplaySceneAnimation(anim);
				const bool becameVisible = !wasDisplayed && isDisplayed;
				if (becameVisible)
					anim.runtimeState = 0;

				Entity *entity = entityManager
					? entityManager->findSceneEntityByName(anim.animName)
					: nullptr;
				if (!isDisplayed) {
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
					if (entityManager) {
						entity = entityManager->spawnSceneAnimationEntity(
							anim.animName, anim.resourcePath, Common::Point(anim.x, anim.y),
							(float)anim.z, anim.frameDelay, anim.active, isDisplayed, anim.looping,
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
					entity->setVisible(isDisplayed);
					if (anim.active && anim.runtimeState >= 0 && !entity->isAnimationEnabled())
						entity->setCurrentFrame(anim.runtimeState);
					entity->setAnimationEnabled(anim.active);
					if (zChanged)
						entityManager->reinsertSceneEntity(entity);
				}
			}

			for (const AnimRecord &anim : scene.state.roomAnimations) {
				if (!findRoomAnimByNameConst(updatedState.roomAnimations, anim.animName))
					removeSceneEntityByName(anim.animName);
			}

			for (const NpcRecord &npc : scene.state.roomNpcs) {
				const NpcRecord *updatedNpc =
					findRoomNpcByNameConst(updatedState.roomNpcs, npc.npcName);
				const bool keepCurrentRoomDeathEntity =
					!updatedNpc &&
					isRetainedCurrentRoomNpcDeathRecord(npc) &&
					entityManager &&
					entityManager->findSceneEntityByName(npc.npcName);
				const bool keepEntity =
					updatedNpc &&
					shouldSpawnRoomNpcEntity(*updatedNpc) &&
					(sameNpcEntityState(npc, *updatedNpc) ||
						isQueuedLiveNpcDeathMutation(npc, *updatedNpc));
				if (keepCurrentRoomDeathEntity) {
					debugC(1, kDebugCombat,
						"Harvester: retained current-room npc death entity npc='%s' damage_type=%d frame=%d",
						npc.npcName.c_str(), npc.deathDamageType, npc.runtimeState);
					continue;
				}
				if (!keepEntity)
					removeSceneEntityByName(npc.npcName);
			}
			for (const NpcRecord &npc : updatedState.roomNpcs) {
				if (!shouldSpawnRoomNpcEntity(npc))
					continue;

				Entity *entity = entityManager
					? entityManager->findSceneEntityByName(npc.npcName)
					: nullptr;
				const NpcRecord *previousNpc =
					findRoomNpcByNameConst(scene.state.roomNpcs, npc.npcName);
				const bool keepQueuedDeathEntity =
					entity &&
					previousNpc &&
					isQueuedLiveNpcDeathMutation(*previousNpc, npc);
				if (keepQueuedDeathEntity)
					continue;
				const bool needsRespawn =
					!entity || !previousNpc || !sameNpcEntityState(*previousNpc, npc);
				if (!needsRespawn)
					continue;

				removeSceneEntityByName(npc.npcName);
				if (!spawnSceneNpcEntityFromRecord(npc))
					return false;
			}

			for (const MonsterRecord &monster : scene.state.roomMonsters) {
				const MonsterRecord *updatedMonster =
					findRoomMonsterByNameConst(updatedState.roomMonsters, monster.monsterName);
				const bool keepEntity =
					updatedMonster &&
					shouldSpawnRoomMonsterEntity(*updatedMonster) &&
					sameMonsterEntityState(monster, *updatedMonster);
				if (!keepEntity)
					removeSceneEntityByName(monster.monsterName);
			}
			for (const MonsterRecord &monster : updatedState.roomMonsters) {
				if (!shouldSpawnRoomMonsterEntity(monster))
					continue;

				Entity *entity = entityManager
					? entityManager->findSceneEntityByName(monster.monsterName)
					: nullptr;
				const MonsterRecord *previousMonster =
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
			if (preservePaletteState) {
				memcpy(scene.palette, previousPalette, sizeof(scene.palette));
				scene.targetPaletteBrightness = previousPaletteBrightness;
			} else {
				memcpy(scene.palette, updatedScene.palette, sizeof(scene.palette));
				scene.targetPaletteBrightness = updatedScene.targetPaletteBrightness;
			}
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
			const Common::Array<AudioCommand> entryAudioCommands = scene.state.audioCommands;
			RoomSetupState refreshedState;
			if (!_engine.getScript()->materializeRoomState(
					scene.state.entranceName, scene.state.roomName, refreshedState, *_engine.getResources())) {
				return false;
			}

			refreshedState.audioCommands = entryAudioCommands;
			if (!loadRoomSceneResources(refreshedState, *_engine.getResources(), scene))
				return false;
			hitEffectStates.clear();
			nextCombatEffectId = 0;
			if (!flow.populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
				return false;
			if (entityManager) {
				for (const MonsterRecord &monster : scene.state.roomMonsters) {
					Entity *entity = entityManager->findSceneEntityByName(monster.monsterName);
					if (entity)
						Monster::applyAnimation(*entity, monster);
				}
			}

			playerState.entity = entityManager ? entityManager->findSceneEntityByName("PLAYER") : nullptr;
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
			playerState.combatLoadout =
				_engine.getScript() ? _engine.getScript()->getPlayerCombatLoadout() : 0;
			monsterCombatStates.clear();
			monsterCombatStates.resize(scene.state.roomMonsters.size());
			pendingRegionName.clear();
			resetIdleState();
			flow.resetCursorAnimationSequence();
			captureCurrentSaveState();
			return _inventory.refresh();
		};
		auto syncAnimatedRoomActorPlacement = [&]() {
			if (playerState.entity) {
				(void)applyRoomActorPlacement(scene.state, *playerState.entity,
					playerState.centerX, playerState.bottomY, playerState.z);
			}
			if (!entityManager)
				return;

			for (const NpcRecord &npc : scene.state.roomNpcs) {
				Entity *entity = entityManager->findSceneEntityByName(npc.npcName);
				if (!entity)
					continue;
				(void)applyRoomNpcPlacement(*entity, npc);
			}
			for (const MonsterRecord &monster : scene.state.roomMonsters) {
				Entity *entity = entityManager->findSceneEntityByName(monster.monsterName);
				if (!entity)
					continue;
				(void)applyRoomActorPlacement(scene.state, *entity,
					monster.posX, monster.posY, (float)monster.posZ);
			}
		};
		auto stowCarriedRoomItemToInventory = [&]() {
			if (!hasCarriedRoomItem())
				return true;

			_engine.getScript()->addRuntimeObjectToInventory(carriedRoomItemName);
			clearCarriedRoomItem();
			return _inventory.refresh();
		};
		auto shouldCancelDeniedPickup = [&](const InteractionResult &interaction,
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
		auto restoreDeniedPickup = [&](Script &script,
				const ObjectRecord &savedObject, bool clearCarryState) -> Common::Error {
			if (!script.syncRuntimeObjectRecord(savedObject))
				return Common::kReadingFailed;
			if (clearCarryState)
				clearCarriedRoomItem();

			if (!applyCurrentRoomRuntimeMutationsInPlace(true) && !refreshCurrentScene(true))
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
		auto showCdChangePrompt = [&](int discNumber) -> Common::Error {
			if (discNumber <= 0)
				return Common::kNoError;

			ResourceManager *resources = _engine.getResources();
			if (!resources)
				return Common::kReadingFailed;
			const int previousDisc = resources->getCurrentDisc();

			if (_engine.shouldShowCdChangePrompts()) {
				Graphics::Screen *screen = _engine.getScreen();
				if (!screen)
					return Common::kReadingFailed;

				const Common::String bitmapPath = Common::String::format("1:/GRAPHIC/OTHER/CD%d.BM", discNumber);
				IndexedBitmap promptBitmap;
				byte promptPalette[256 * 3];
				if (!loadBitmapResource(*resources, bitmapPath, promptBitmap) ||
						!loadPaletteResource(*resources, kCdChangePromptPalettePath, promptPalette)) {
					warning("Harvester: unable to load CD change prompt assets for disc %d", discNumber);
					return Common::kReadingFailed;
				}

				debugC(1, kDebugScene, "Harvester: showing CD change prompt for disc %d", discNumber);
				_engine.stopMusic();

				Common::Event event;
				while (g_system->getEventManager()->pollEvent(event)) {
					Common::Error result = Common::kNoError;
					if (flow.handleSystemEvent(event, result))
						return result;
					if (event.type == Common::EVENT_MOUSEMOVE)
						_mousePos = event.mouse;
				}

				bool waitingForRelease = false;
				bool needsRedraw = true;
				bool activateRequestedDisc = false;
				Graphics::FrameLimiter limiter(g_system, 60);

				while (!_engine.shouldQuit() && !activateRequestedDisc) {
					if (needsRedraw) {
						renderCdChangePromptScreen(_engine, promptBitmap, promptPalette);
						needsRedraw = false;
					}

					while (g_system->getEventManager()->pollEvent(event)) {
						Common::Error result = Common::kNoError;
						if (flow.handleSystemEvent(event, result))
							return result;

						switch (event.type) {
						case Common::EVENT_MOUSEMOVE:
							_mousePos = event.mouse;
							needsRedraw = true;
							break;
						case Common::EVENT_LBUTTONDOWN:
							waitingForRelease = true;
							break;
						case Common::EVENT_LBUTTONUP:
							if (waitingForRelease)
								activateRequestedDisc = true;
							break;
						default:
							break;
						}
					}

					EntityManager *entityManager = _engine.getRuntimeEntities();
					if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
						needsRedraw = true;

					limiter.delayBeforeSwap();
					limiter.startFrame();
				}
			}

			if (!_engine.activateDisc(discNumber)) {
				warning("Harvester: unable to activate disc %d resources", discNumber);
				return Common::kReadingFailed;
			}
			if ((discNumber == 1 || discNumber == 3) &&
					previousDisc > 0 && previousDisc != resources->getCurrentDisc()) {
				Script *script = _engine.getScript();
				if (!script || !script->reloadTownWorld(*resources)) {
					warning("Harvester: unable to reload town script after disc prompt %d -> %d",
						previousDisc, discNumber);
					return Common::kReadingFailed;
				}
			}

			return Common::kNoError;
		};
		auto runRoomExitCommands = [&]() -> Common::Error {
			InteractionResult exitInteraction;
			if (!_engine.getScript()->executeRoomExitCommands(scene.state.roomName, exitInteraction))
				return Common::kReadingFailed;

			for (uint exitStep = 0; exitStep < 128; ++exitStep) {
				if (exitInteraction.cdChangeDisc > 0) {
					Common::Error cdPromptError = showCdChangePrompt(exitInteraction.cdChangeDisc);
					if (cdPromptError.getCode() != Common::kNoError)
						return cdPromptError;
				}
				if (!exitInteraction.musicPath.empty())
					(void)_engine.playMusic(exitInteraction.musicPath);
				flow.executeStartupAudioCommands(exitInteraction.audioCommands);
				if (exitInteraction.mutatedRuntimeState)
					syncGlobalTimerEntities(exitInteraction.previousTimerRecords, true);

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
					debugC(1, kDebugRoom,
						"Harvester: room exit command for '%s' produced unsupported deferred output; preserving accumulated state",
						scene.state.roomName.c_str());
				}

				if (exitInteraction.continuationTag.empty())
					return Common::kNoError;

				InteractionResult continuationInteraction;
				if (!_engine.getScript()->executeActionTag(
						exitInteraction.continuationTag, continuationInteraction, false,
						scene.state.roomName)) {
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
				scene.targetPaletteBrightness = kDimPaletteBrightness;
				drawRoomScene(_engine, *activeScreen, scene, scene.targetPaletteBrightness);
				setScaledRoomPalette(*activeScreen, scene.palette, scene.targetPaletteBrightness);
				activeScreen->makeAllDirty();
				activeScreen->update();
				return Common::kNoError;
			case kStartupLightingCommandNormal:
				if (ResourceManager *resources = _engine.getResources()) {
					if (!scene.state.palettePath.empty() &&
							!loadPaletteResource(*resources, scene.state.palettePath, scene.palette)) {
						warning("Harvester: unable to reload room palette '%s' for CHANGE_LIGHTING NORMAL",
							scene.state.palettePath.c_str());
					}
				}
				scene.targetPaletteBrightness = kNormalPaletteBrightness;
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
				debugC(1, kDebugRoom,
					"Harvester: CHANGE_LIGHTING FADE_IN has no direct room-side equivalent yet; preserving control flow");
				return Common::kNoError;
			}

			return Common::kNoError;
		};
		auto applyPlayerGotoXZ = [&](int x, int z) {
			playerState.hasMoveTarget = false;
			playerState.turnActive = false;
			playerState.turnTargetFacing = -1;
			playerState.centerX = x;
			playerState.z = (float)z;
			playerState.bottomY = mapRoomDepthToScreenYForCombat(
				scene.state, playerState.z, playerState.bottomY);
			Common::Rect screenRect;
			bool hasScreenRect = false;
			float depthScale = 0.0f;
			if (playerState.entity) {
				(void)applyRoomActorPlacement(scene.state, *playerState.entity,
					playerState.centerX, playerState.bottomY, playerState.z);
				if (entityManager)
					entityManager->reinsertSceneEntity(playerState.entity);
				screenRect = playerState.entity->getScreenRect();
				hasScreenRect = true;
				depthScale = playerState.entity->getDepthScale();
			}
			debugC(1, kDebugPlayer,
				"Harvester: PC_GOTO_XZ applied x=%d z=%d pos=(%d,%d,z=%.2f) screen_rect=(%d,%d)-(%d,%d) depth_scale=%.3f",
				x, z, playerState.centerX, playerState.bottomY, (double)playerState.z,
				hasScreenRect ? screenRect.left : 0, hasScreenRect ? screenRect.top : 0,
				hasScreenRect ? screenRect.right : 0, hasScreenRect ? screenRect.bottom : 0,
				(double)depthScale);
			resetIdleState();
			captureCurrentSaveState();
		};
		auto runModalShowText = [&](const ResolvedText &modalText) -> Common::Error {
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
					if (flow.handleSystemEvent(event, result))
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
		auto requestPlayerGameOver = [&](const char *reason, const Common::String &sourceName) {
			if (flow.hasPendingMainMenuReturn())
				return;

			debugC(1, kDebugCombat,
				"Harvester: player game over queued reason='%s' source='%s' hp=%d",
				reason ? reason : "", sourceName.c_str(),
				_engine.getScript()
					? _engine.getScript()->getPlayerCurrentHitPoints()
					: 0);
			flow.requestGameOverReturn();
		};
		auto startPlayerDefeatSequence = [&](const char *reason, const Common::String &sourceName,
				int damageType) {
			if (flow.hasPendingMainMenuReturn() || playerState.deathActive)
				return;

			if (idleState.entity) {
				idleState.entity->setVisible(false);
				idleState.entity->setAnimationRate(0);
			}
			idleState.active = false;
			idleState.loopStarted = false;
			idleState.exiting = false;
			idleState.restoreFacing = -1;
			if (playerState.entity)
				playerState.entity->setVisible(true);

			if (!Player::startDeathAnimation(playerState, damageType, _engine.isGoreEnabled())) {
				debugC(1, kDebugCombat,
					"Harvester: player defeat fallback reason='%s' source='%s' damage_type=%d hp=%d",
					reason ? reason : "", sourceName.c_str(), damageType,
					_engine.getScript()
						? _engine.getScript()->getPlayerCurrentHitPoints()
						: 0);
				requestPlayerGameOver(reason, sourceName);
				return;
			}

			debugC(1, kDebugCombat,
				"Harvester: player defeat started reason='%s' source='%s' damage_type=%d hp=%d",
				reason ? reason : "", sourceName.c_str(), damageType,
				_engine.getScript()
					? _engine.getScript()->getPlayerCurrentHitPoints()
					: 0);
		};
		typedef decltype(refreshCurrentScene) RefreshCurrentSceneFn;
		typedef decltype(applyCurrentRoomRuntimeMutationsInPlace) ApplyCurrentRoomRuntimeMutationsInPlaceFn;
		typedef decltype(syncGlobalTimerEntities) SyncGlobalTimerEntitiesFn;
		typedef decltype(captureDialogueBackdrop) CaptureDialogueBackdropFn;
		typedef decltype(showCdChangePrompt) ShowCdChangePromptFn;
		typedef decltype(runRoomExitCommands) RunRoomExitCommandsFn;
		typedef decltype(applyLightingCommand) ApplyLightingCommandFn;
		typedef decltype(applyPlayerGotoXZ) ApplyPlayerGotoXZFn;
		typedef decltype(runModalShowText) RunModalShowTextFn;
		typedef decltype(resetIdleState) ResetIdleStateFn;
		typedef decltype(stopPlayerRegionInteraction) StopPlayerRegionInteractionFn;
		typedef decltype(startPlayerDefeatSequence) StartPlayerDefeatSequenceFn;

		struct RoomInteractionCallbacksImpl : RoomInteractionCallbacks {
			RoomInteractionCallbacksImpl(
					RefreshCurrentSceneFn &refreshCurrentSceneFn_,
					ApplyCurrentRoomRuntimeMutationsInPlaceFn &applyCurrentRoomRuntimeMutationsInPlaceFn_,
					SyncGlobalTimerEntitiesFn &syncGlobalTimerEntitiesFn_,
					CaptureDialogueBackdropFn &captureDialogueBackdropFn_,
					ShowCdChangePromptFn &showCdChangePromptFn_,
					RunRoomExitCommandsFn &runRoomExitCommandsFn_,
					ApplyLightingCommandFn &applyLightingCommandFn_,
					ApplyPlayerGotoXZFn &applyPlayerGotoXZFn_,
					RunModalShowTextFn &runModalShowTextFn_,
					ResetIdleStateFn &resetIdleStateFn_,
					StopPlayerRegionInteractionFn &stopPlayerRegionInteractionFn_,
					StartPlayerDefeatSequenceFn &startPlayerDefeatSequenceFn_)
				: refreshCurrentSceneFn(refreshCurrentSceneFn_),
				  applyCurrentRoomRuntimeMutationsInPlaceFn(applyCurrentRoomRuntimeMutationsInPlaceFn_),
				  syncGlobalTimerEntitiesFn(syncGlobalTimerEntitiesFn_),
				  captureDialogueBackdropFn(captureDialogueBackdropFn_),
				  showCdChangePromptFn(showCdChangePromptFn_),
				  runRoomExitCommandsFn(runRoomExitCommandsFn_),
				  applyLightingCommandFn(applyLightingCommandFn_),
				  applyPlayerGotoXZFn(applyPlayerGotoXZFn_),
				  runModalShowTextFn(runModalShowTextFn_),
				  resetIdleStateFn(resetIdleStateFn_),
				  stopPlayerRegionInteractionFn(stopPlayerRegionInteractionFn_),
				  startPlayerDefeatSequenceFn(startPlayerDefeatSequenceFn_) {
			}

			bool refreshCurrentScene(bool preservePlayerPlacement) override {
				return refreshCurrentSceneFn(preservePlayerPlacement);
			}

			bool applyCurrentRoomRuntimeMutationsInPlace(bool preservePaletteState) override {
				return applyCurrentRoomRuntimeMutationsInPlaceFn(preservePaletteState);
			}

			void syncGlobalTimerEntities(const Common::Array<TimerRecord> &previousTimerRecords) override {
				syncGlobalTimerEntitiesFn(previousTimerRecords);
			}

			bool captureDialogueBackdrop(IndexedBitmap &dialogueBackdrop) override {
				return captureDialogueBackdropFn(dialogueBackdrop);
			}

			Common::Error showCdChangePrompt(int discNumber) override {
				return showCdChangePromptFn(discNumber);
			}

			Common::Error runRoomExitCommands() override {
				return runRoomExitCommandsFn();
			}

			Common::Error applyLightingCommand(StartupLightingCommand lightingCommand) override {
				return applyLightingCommandFn(lightingCommand);
			}

			void applyPlayerGotoXZ(int x, int z) override {
				applyPlayerGotoXZFn(x, z);
			}

			Common::Error runModalShowText(const ResolvedText &modalText) override {
				return runModalShowTextFn(modalText);
			}

			void resetIdleState() override {
				resetIdleStateFn();
			}

			void stopPlayerRegionInteraction() override {
				stopPlayerRegionInteractionFn();
			}

			void startPlayerDefeatSequence(const char *reason, const Common::String &sourceName,
					int damageType) override {
				startPlayerDefeatSequenceFn(reason, sourceName, damageType);
			}

			RefreshCurrentSceneFn &refreshCurrentSceneFn;
			ApplyCurrentRoomRuntimeMutationsInPlaceFn &applyCurrentRoomRuntimeMutationsInPlaceFn;
			SyncGlobalTimerEntitiesFn &syncGlobalTimerEntitiesFn;
			CaptureDialogueBackdropFn &captureDialogueBackdropFn;
			ShowCdChangePromptFn &showCdChangePromptFn;
			RunRoomExitCommandsFn &runRoomExitCommandsFn;
			ApplyLightingCommandFn &applyLightingCommandFn;
			ApplyPlayerGotoXZFn &applyPlayerGotoXZFn;
			RunModalShowTextFn &runModalShowTextFn;
			ResetIdleStateFn &resetIdleStateFn;
			StopPlayerRegionInteractionFn &stopPlayerRegionInteractionFn;
			StartPlayerDefeatSequenceFn &startPlayerDefeatSequenceFn;
		};
		RoomInteractionCallbacksImpl interactionCallbacks(
			refreshCurrentScene, applyCurrentRoomRuntimeMutationsInPlace,
			syncGlobalTimerEntities, captureDialogueBackdrop, showCdChangePrompt,
			runRoomExitCommands, applyLightingCommand, applyPlayerGotoXZ, runModalShowText,
			resetIdleState, stopPlayerRegionInteraction, startPlayerDefeatSequence);
		RoomInteractionProcessor interactionProcessor(
			_engine, flow, scene, playerState, pendingRegionName, pendingRoomChange,
			pendingRoomChangeIsRoomName, pendingRoomChangeUsesSavedRoomState,
			pendingRoomChangeSavedRoomState, canExitCloseupToParent, interactionCallbacks);
		InteractionResult roomEntryInteraction;
		bool hasPendingRoomEntryInteraction = false;
		if (shouldRunRoomEntryCommands) {
			if (!_engine.getScript()->executeRoomEnterCommands(
					scene.state.roomName, roomEntryInteraction)) {
				return Common::kReadingFailed;
			}

			// Native room_setup applies entry-time state changes before the first visible room frame.
			if (roomEntryInteraction.mutatedRuntimeState) {
				syncGlobalTimerEntities(roomEntryInteraction.previousTimerRecords);
				if (!applyCurrentRoomRuntimeMutationsInPlace(false) &&
						!refreshCurrentScene(true)) {
					return Common::kReadingFailed;
				}
				roomEntryInteraction.mutatedRuntimeState = false;
				roomEntryInteraction.visualRuntimeStateChanged = false;
			}
			hasPendingRoomEntryInteraction = hasRoomEntryInteraction(roomEntryInteraction);
		}

		transitionError = flow.waitForRoomSetupTransitionHold();
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;

		logScenePaletteSummary("room setup stub palette", scene, 0.0f);
		drawRoomScene(_engine, *screen, scene, 0.0f);
		screen->makeAllDirty();
		screen->update();

		logScenePaletteSummary("room setup fade target", scene, scene.targetPaletteBrightness);
		transitionError = flow.fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;
		if (entityManager)
			entityManager->resumeTimerCountdowns();

		if (shouldRunRoomEntryCommands) {
			if (hasPendingRoomEntryInteraction) {
				bool didEntryTransition = false;
				Common::Error entryInteractionError = interactionProcessor.handleInteractionResult(
					roomEntryInteraction, didEntryTransition, Common::String());
				if (entryInteractionError.getCode() != Common::kNoError)
					return entryInteractionError;
				if (flow.hasPendingMainMenuReturn() || didEntryTransition)
					return Common::kNoError;
			}

			flow.resetCursorAnimationSequence();
			resetIdleState();
		}
		ScopedDeferredLiveNpcDeathTransitions deferredLiveNpcDeathTransitions(
			_engine.getScript());
		auto findRoomNpcRecordByName = [&](const Common::String &npcName) -> NpcRecord * {
			for (NpcRecord &npc : scene.state.roomNpcs) {
				if (npc.npcName.equalsIgnoreCase(npcName))
					return &npc;
			}

			return nullptr;
		};
		auto findRoomNpcIndexByName = [&](const Common::String &npcName) -> int {
			for (uint i = 0; i < scene.state.roomNpcs.size(); ++i) {
				if (scene.state.roomNpcs[i].npcName.equalsIgnoreCase(npcName))
					return (int)i;
			}

			return -1;
		};
		auto findRoomMonsterRecordByName = [&](const Common::String &monsterName) -> MonsterRecord * {
			for (MonsterRecord &monster : scene.state.roomMonsters) {
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
		auto findSceneRuntimeEntity = [&](const Common::String &entityName) -> Entity * {
			return entityManager ? entityManager->findSceneEntityByName(entityName) : nullptr;
		};
		auto queueInnerSanctumHerrillDeathAfterMuck = [&](const MonsterRecord &monster) {
			if (!monster.roomName.equalsIgnoreCase(kInnerSanctumRoomName) ||
					!monster.monsterName.equalsIgnoreCase(kMuckeyMonsterName) ||
					!monster.onDeathActionTag.equalsIgnoreCase(kActivateInnerSanctumDoorActionTag)) {
				return false;
			}

			Script *script = _engine.getScript();
			NpcRecord *herrill = findRoomNpcRecordByName(kHerrillLogNpcName);
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
		};
		auto findMonsterTargetAtPoint = [&](const Common::Point &point) -> MonsterRecord * {
			MonsterRecord *bestMonster = nullptr;
			int bestZ = -0x7fffffff;
			int bestTop = -0x7fffffff;
			for (MonsterRecord &monster : scene.state.roomMonsters) {
				if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
					continue;

				Entity *entity = findSceneRuntimeEntity(monster.monsterName);
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
		auto findOverlappingMonsterTarget = [&]() -> MonsterRecord * {
			if (!playerState.entity)
				return nullptr;

			for (MonsterRecord &monster : scene.state.roomMonsters) {
				if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
					continue;

				Entity *entity = findSceneRuntimeEntity(monster.monsterName);
				if (entity && areCombatantsWithinRoomCombatReach(scene.state,
						*playerState.entity, playerState.z, *entity, (float)monster.posZ, monster.engageDistance))
					return &monster;
			}

			return nullptr;
		};
		auto findOverlappingNpcTarget = [&]() -> NpcRecord * {
			if (!playerState.entity)
				return nullptr;

			for (NpcRecord &npc : scene.state.roomNpcs) {
				if (!npc.visible || npc.deathOrMonsterfyFlag || npc.deathDamageType != 0)
					continue;

				Entity *entity = findSceneRuntimeEntity(npc.npcName);
				if (entity && areCombatantsWithinRoomCombatReach(scene.state,
						*playerState.entity, playerState.z, *entity, entity->getZ(), 0))
					return &npc;
			}

			return nullptr;
		};
		auto playerAttackCanReachTarget = [&](Entity *targetEntity, int engageDistance = 0) {
			if (!playerState.entity || !targetEntity)
				return false;
			if (Player::isProjectileCombatLoadout(playerState.combatLoadout))
				return true;

			return areCombatantsWithinRoomCombatReach(scene.state,
				*playerState.entity, playerState.z, *targetEntity, targetEntity->getZ(), engageDistance);
		};
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
		auto resolveNativePlayerAttackGeometry = [](int attackFirstFrame,
				NativeAttackSide &attackSide, NativeAttackBand &attackBand) {
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
		};
		auto findProjectileContactMonsterTarget = [&]() -> MonsterRecord * {
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

			for (MonsterRecord &monster : scene.state.roomMonsters) {
				if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
					continue;

				Entity *entity = findSceneRuntimeEntity(monster.monsterName);
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
	auto handleCombatInteraction = [&](InteractionResult interaction) -> Common::Error {
		bool didTransition = false;
		Common::Error interactionError =
			interactionProcessor.handleInteractionResult(interaction, didTransition, Common::String());
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		if (!flow.hasPendingMainMenuReturn() &&
				_engine.getScript() &&
				_engine.getScript()->getPlayerCurrentHitPoints() <= 0) {
			stopPlayerRegionInteraction();
			startPlayerDefeatSequence("combat_interaction", Common::String(), 1);
		}

		needsRedraw = true;
		return Common::kNoError;
	};
	auto beginNpcDeathTransition = [&](NpcRecord &npc, Entity &npcEntity,
			RoomNpcCombatState &combatState, int deathDamageType) {
		RoomDeathAnimationRange deathRange;
		if (!resolveNpcDeathAnimationRange(npcEntity, !npc.monsterfyTargetName.empty(),
				deathDamageType, _engine.isGoreEnabled(), deathRange))
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
			(void)_engine.playSound(npc.audioPath);
		debugC(1, kDebugCombat,
			"Harvester: combat npc death start target='%s' damage_type=%d frames=%d..%d monsterfy='%s' on_death='%s'",
			npc.npcName.c_str(), deathDamageType, deathRange.firstFrame, deathRange.lastFrame,
			npc.monsterfyTargetName.c_str(), npc.onDeathActionTag.c_str());
		needsRedraw = true;
		return true;
	};
	auto finalizeNpcDeathTransition = [&](NpcRecord &npc,
			RoomNpcCombatState &combatState) -> Common::Error {
		Script *script = _engine.getScript();
		Entity *npcEntity = findSceneRuntimeEntity(npc.npcName);
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
			npcEntity->setHitTestMode(kRuntimeEntityHitTestNone);
			npcEntity->setVisible(true);
		} else {
			removeSceneEntityByName(npc.npcName);
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
				interaction = deathInteraction;
				interaction.mutatedRuntimeState = true;
				interaction.visualRuntimeStateChanged = true;
			}
		}

		return handleCombatInteraction(interaction);
	};
	auto killDebugActiveMonster = [&]() -> Common::Error {
		if (!_engine.isCombatDebugEnabled())
			return Common::kNoError;

		auto isKillableMonster = [&](const MonsterRecord *monster) {
			return monster && monster->active && monster->visible && monster->currentHitPoints > 0;
		};
		auto selectMonsterEntity = [&](MonsterRecord *monster) -> Entity * {
			Entity *entity = monster ? findSceneRuntimeEntity(monster->monsterName) : nullptr;
			return (entity && entity->isVisible()) ? entity : nullptr;
		};
		auto resolveKillTarget = [&]() -> MonsterRecord * {
			if (playerState.attackTargetClassId == kRuntimeEntityClassMonster) {
				MonsterRecord *monster = findRoomMonsterRecordByName(playerState.attackTargetName);
				if (isKillableMonster(monster) && selectMonsterEntity(monster))
					return monster;
			}

			if (MonsterRecord *monster = findMonsterTargetAtPoint(_mousePos)) {
				if (isKillableMonster(monster) && selectMonsterEntity(monster))
					return monster;
			}

			if (playerState.entity) {
				for (uint i = 0; i < scene.state.roomMonsters.size() && i < monsterCombatStates.size(); ++i) {
					MonsterRecord &monster = scene.state.roomMonsters[i];
					const RoomMonsterCombatState &combatState = monsterCombatStates[i];
					if (!combatState.attackActive || !isKillableMonster(&monster) || !selectMonsterEntity(&monster))
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
			for (MonsterRecord &monster : scene.state.roomMonsters) {
				if (!isKillableMonster(&monster))
					continue;

				Entity *entity = selectMonsterEntity(&monster);
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
		};

		MonsterRecord *monster = resolveKillTarget();
		Entity *monsterEntity = selectMonsterEntity(monster);
		if (!monster || !monsterEntity) {
			debugC(1, kDebugCombat,
				"Harvester: debug combat kill skipped reason='no active monster' cursor=(%d,%d)",
				_mousePos.x, _mousePos.y);
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
			spawnCombatDamagePopup(*monsterEntity, monster->monsterName, damageLanded);

		if (playerState.attackTargetClassId == kRuntimeEntityClassMonster &&
				playerState.attackTargetName.equalsIgnoreCase(monster->monsterName)) {
			playerState.attackTargetName.clear();
			playerState.attackTargetClassId = -1;
		}

		debugC(1, kDebugCombat,
			"Harvester: debug combat kill target='%s' damage=%d damage_type=%d cursor=(%d,%d)",
			monster->monsterName.c_str(), damageLanded, debugDamageType, _mousePos.x, _mousePos.y);

		const int monsterIndex = findRoomMonsterIndexByName(monster->monsterName);
		RoomMonsterCombatState *combatState =
			monsterIndex >= 0 && monsterIndex < (int)monsterCombatStates.size()
				? &monsterCombatStates[(uint)monsterIndex]
				: nullptr;
		if (combatState)
			clearRoomMonsterCombatState(*combatState);
		(void)script->syncRuntimeMonsterRecord(*monster);

		RoomDeathAnimationRange deathRange;
		if (combatState && resolveMonsterDeathAnimationRange(*monsterEntity, monster->facing,
				debugDamageType, _engine.isGoreEnabled(), deathRange)) {
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
				(void)_engine.playSound(monster->deathSound);
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
				interaction = deathInteraction;
				interaction.mutatedRuntimeState = true;
				interaction.visualRuntimeStateChanged = true;
			}
		}

		return handleCombatInteraction(interaction);
	};
	auto capturePlayerAttackTarget = [&]() {
			playerState.attackTargetName.clear();
			playerState.attackTargetClassId = -1;

			const RoomHoverState hoverState = resolveRoomHoverState(
				_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
				_mousePos, &flow._dialogue);
			if (hoverState.npc) {
				playerState.attackTargetName = hoverState.npc->npcName;
				playerState.attackTargetClassId = kRuntimeEntityClassNpc;
				debugC(1, kDebugCombat,
					"Harvester: combat player target capture class='npc' name='%s' cursor=(%d,%d)",
					playerState.attackTargetName.c_str(), _mousePos.x, _mousePos.y);
				return;
			}

		if (MonsterRecord *monster = findMonsterTargetAtPoint(_mousePos)) {
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

		for (NpcRecord &npc : scene.state.roomNpcs) {
			if (!npc.visible || npc.deathOrMonsterfyFlag || npc.deathDamageType != 0)
				continue;
			considerTarget(findSceneRuntimeEntity(npc.npcName), npc.npcName, kRuntimeEntityClassNpc);
		}
		for (MonsterRecord &monster : scene.state.roomMonsters) {
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
		auto startMonsterHitReaction = [&](MonsterRecord &monster, Entity &monsterEntity,
				RoomMonsterCombatState *combatState, int attackerAttackFirstFrame) {
			if (spawnCombatHitEffect(monsterEntity, monster.monsterName))
				needsRedraw = true;

			if (!combatState) {
				(void)playRandomRoomAttackSound(_engine,
					monster.hitSound1, monster.hitSound2, monster.hitSound3);
				return;
			}

			RoomHitAnimationRange range;
			if (!resolveRoomMonsterHitAnimationRange(monsterEntity, attackerAttackFirstFrame, range)) {
				(void)playRandomRoomAttackSound(_engine,
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
				(void)playRandomRoomAttackSound(_engine,
					monster.hitSound1, monster.hitSound2, monster.hitSound3);
				combatState->hitSoundPlayed = true;
			}
			debugC(1, kDebugCombat,
				"Harvester: combat monster hit reaction target='%s' attack_frame=%d frames=%d..%d resume_facing=%d knockback=%d rate=%d",
				monster.monsterName.c_str(), attackerAttackFirstFrame, range.firstFrame, range.lastFrame,
				range.resumeFacing, range.knockbackX, kNativeMonsterHitAnimationRate);
			needsRedraw = true;
		};
		auto resolvePlayerAttackContact = [&]() -> Common::Error {
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
		Script *script = _engine.getScript();
		if (!script)
			return Common::kNoError;

		if (playerState.attackTargetClassId < 0 && !Player::isProjectileCombatLoadout(playerState.combatLoadout)) {
			if (NpcRecord *fallbackNpc = findOverlappingNpcTarget()) {
				playerState.attackTargetName = fallbackNpc->npcName;
				playerState.attackTargetClassId = kRuntimeEntityClassNpc;
			}
		}
		if (playerState.attackTargetClassId == kRuntimeEntityClassNpc) {
			NpcRecord *npc = findRoomNpcRecordByName(playerState.attackTargetName);
			Entity *npcEntity = npc ? findSceneRuntimeEntity(npc->npcName) : nullptr;
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
				script->queueRuntimeNpcDeathOrMonsterfy(npc->npcName, damageType);
			const bool jimmyFlagChanged = npc->npcName.equalsIgnoreCase("JIMMY")
				? script->setRuntimeFlagValue("JIMMY_ATTACKED", true)
				: false;
			const int npcIndex = findRoomNpcIndexByName(npc->npcName);
			RoomNpcCombatState *combatState =
				npcIndex >= 0 && npcIndex < (int)npcCombatStates.size()
					? &npcCombatStates[(uint)npcIndex]
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
				if (spawnCombatHitEffect(*npcEntity, followTargetName))
					needsRedraw = true;
			}
			if (runtimeChanged && combatState && npcEntity &&
					beginNpcDeathTransition(*npc, *npcEntity, *combatState, damageType))
				return Common::kNoError;
			if (runtimeChanged) {
				debugC(1, kDebugCombat,
					"Harvester: combat npc defeated target='%s' damage_type=%d on_death='%s' fallback='no death bank'",
					npc->npcName.c_str(), damageType, npc->onDeathActionTag.c_str());
				RoomNpcCombatState fallbackState;
				fallbackState.deathDamageType = damageType;
				return finalizeNpcDeathTransition(*npc, combatState ? *combatState : fallbackState);
			}

			return Common::kNoError;
		}

		MonsterRecord *monster = nullptr;
		if (playerState.attackTargetClassId == kRuntimeEntityClassMonster)
			monster = findRoomMonsterRecordByName(playerState.attackTargetName);
		Entity *monsterEntity = monster ? findSceneRuntimeEntity(monster->monsterName) : nullptr;
		if (!monster || !monsterEntity || !playerAttackCanReachTarget(monsterEntity, monster ? monster->engageDistance : 0)) {
			if (!Player::isProjectileCombatLoadout(playerState.combatLoadout)) {
				monster = findOverlappingMonsterTarget();
			} else if (playerState.attackTargetClassId < 0) {
				monster = findProjectileContactMonsterTarget();
				if (monster) {
					playerState.attackTargetName = monster->monsterName;
					playerState.attackTargetClassId = kRuntimeEntityClassMonster;
				}
			} else {
				monster = nullptr;
			}
			monsterEntity = monster ? findSceneRuntimeEntity(monster->monsterName) : nullptr;
		}
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
			const int monsterIndex = findRoomMonsterIndexByName(monster->monsterName);
			RoomMonsterCombatState *combatState =
				monsterIndex >= 0 && monsterIndex < (int)monsterCombatStates.size()
					? &monsterCombatStates[(uint)monsterIndex]
					: nullptr;
			debugC(1, kDebugCombat,
				"Harvester: combat player monster hit target='%s' loadout=%d weapon='%s' damage=%d damage_type='%s' hp=%d->%d reachable=1",
				monster->monsterName.c_str(), playerState.combatLoadout,
				Player::describeCombatLoadout(playerState.combatLoadout), damageAmount,
				Player::describeCombatDamageType(damageType), hitPointsBefore, monster->currentHitPoints);
			if (damageLanded > 0) {
				spawnCombatDamagePopup(*monsterEntity, monster->monsterName, damageLanded);
				startMonsterHitReaction(*monster, *monsterEntity, combatState, playerState.attackFirstFrame);
			}
			if (monster->currentHitPoints > 0) {
				(void)script->syncRuntimeMonsterRecord(*monster);
				needsRedraw = true;
				return Common::kNoError;
			}
			if (combatState)
				clearRoomMonsterCombatState(*combatState);
		(void)script->syncRuntimeMonsterRecord(*monster);

		RoomDeathAnimationRange deathRange;
		if (combatState && resolveMonsterDeathAnimationRange(*monsterEntity, monster->facing,
				damageType, _engine.isGoreEnabled(), deathRange)) {
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
				(void)_engine.playSound(monster->deathSound);
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
					monster->onDeathActionTag, deathInteraction, true, monster->roomName)) {
				interaction = deathInteraction;
				interaction.mutatedRuntimeState = true;
				interaction.visualRuntimeStateChanged = true;
			}
		}
		if (herrillDeathQueued) {
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}

		return handleCombatInteraction(interaction);
	};
	auto updateRoomMonsterCombat = [&]() -> Common::Error {
		if (!entityManager)
			return Common::kNoError;

		Script *script = _engine.getScript();
		const uint32 now = Player::getRuntimeClockTicks();
		const uint32 moveInterval = MAX<uint32>(1, 100U / (uint32)kRoomMonsterAnimationRate);
		const int horizontalStepBase = 8;
		const int depthStep = MAX<int>(1, roundRoomCombatFloat(
			scene.state.roomZVelocityStep > 0.0f ? scene.state.roomZVelocityStep : 1.0f));
		bool playerAlive = script && script->getPlayerCurrentHitPoints() > 0;

		if (npcCombatStates.size() != scene.state.roomNpcs.size())
			npcCombatStates.resize(scene.state.roomNpcs.size());
		if (monsterCombatStates.size() != scene.state.roomMonsters.size())
			monsterCombatStates.resize(scene.state.roomMonsters.size());

		for (uint i = 0; i < scene.state.roomNpcs.size(); ++i) {
			NpcRecord &npc = scene.state.roomNpcs[i];
			RoomNpcCombatState &combatState = npcCombatStates[i];
			Entity *entity = findSceneRuntimeEntity(npc.npcName);
			if (!entity || !npc.visible || npc.deathOrMonsterfyFlag) {
				clearRoomNpcCombatState(combatState);
				continue;
			}
			if (!combatState.deathActive && npc.active && npc.deathDamageType != 0) {
				if (beginNpcDeathTransition(npc, *entity, combatState, npc.deathDamageType))
					continue;

				debugC(1, kDebugCombat,
					"Harvester: room npc death fallback target='%s' damage_type=%d on_death='%s' reason='no death bank'",
					npc.npcName.c_str(), npc.deathDamageType, npc.onDeathActionTag.c_str());
				combatState.deathDamageType = npc.deathDamageType;
				combatState.deathLastFrame = entity->getCurrentFrame();
				return finalizeNpcDeathTransition(npc, combatState);
			}
			if (!combatState.deathActive)
				continue;
			if (entity->getCurrentFrame() != combatState.deathLastFrame)
				continue;

			entity->setAnimationRate(0);
			entity->setAnimationFrameRange(combatState.deathLastFrame, combatState.deathLastFrame, false);
			entity->setCurrentFrame(combatState.deathLastFrame);
			entity->setAnimationEnabled(false);
			return finalizeNpcDeathTransition(npc, combatState);
		}

		for (uint i = 0; i < scene.state.roomMonsters.size(); ++i) {
			MonsterRecord &monster = scene.state.roomMonsters[i];
			RoomMonsterCombatState &combatState = monsterCombatStates[i];
			Entity *entity = findSceneRuntimeEntity(monster.monsterName);
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
					(void)script->syncRuntimeMonsterRecord(monster);
					debugC(1, kDebugCombat,
						"Harvester: combat monster death complete target='%s' damage_type=%d corpse_frame=%d on_death='%s'",
						monster.monsterName.c_str(), combatState.deathDamageType,
						monster.runtimeState,
						monster.onDeathActionTag.c_str());
					clearRoomMonsterCombatState(combatState);
					const bool herrillDeathQueued = queueInnerSanctumHerrillDeathAfterMuck(monster);

					InteractionResult interaction;
					interaction.mutatedRuntimeState = true;
					interaction.visualRuntimeStateChanged = true;
					if (script && !monster.onDeathActionTag.empty()) {
						InteractionResult deathInteraction;
						if (script->executeActionTag(
								monster.onDeathActionTag, deathInteraction, true, monster.roomName)) {
							interaction = deathInteraction;
							interaction.mutatedRuntimeState = true;
							interaction.visualRuntimeStateChanged = true;
						}
					}
					if (herrillDeathQueued) {
						interaction.mutatedRuntimeState = true;
						interaction.visualRuntimeStateChanged = true;
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

			if (combatState.hitActive) {
				const int currentFrame = entity->getCurrentFrame();
				if (!combatState.hitSoundPlayed &&
						monster.hitSoundTriggerFrame >= 0 &&
						currentFrame >= combatState.hitFirstFrame + monster.hitSoundTriggerFrame) {
					(void)playRandomRoomAttackSound(_engine,
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
							scene.state, *entity, monster.posX, monster.posY, (float)monster.posZ);
						(void)script->syncRuntimeMonsterRecord(monster);
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
					if (!script) {
						debugC(1, kDebugCombat,
							"Harvester: combat monster attack contact monster='%s' frame=%d skipped reason='no startup script'",
							monster.monsterName.c_str(), currentFrame);
					} else if (!playerAlive || !hasTrackedPlayerTarget) {
						debugC(1, kDebugCombat,
							"Harvester: combat monster attack miss monster='%s' frame=%d player_hp=%d reason='no live target'",
							monster.monsterName.c_str(), currentFrame, script->getPlayerCurrentHitPoints());
					} else {
						const int playerHitPointsBefore = script->getPlayerCurrentHitPoints();
						const bool changed = script->adjustPlayerCurrentHitPoints(-monster.damageAmount);
						const int playerHitPointsAfter = script->getPlayerCurrentHitPoints();
						const int damageLanded = playerHitPointsBefore - playerHitPointsAfter;
						debugC(1, kDebugCombat,
							"Harvester: combat monster attack hit monster='%s' frame=%d damage=%d damage_type='%s' player_hp=%d->%d changed=%d",
							monster.monsterName.c_str(), currentFrame, monster.damageAmount,
							Player::describeCombatDamageType(monster.damageType),
							playerHitPointsBefore, playerHitPointsAfter, changed);
						if (damageLanded > 0 && playerState.entity) {
							spawnCombatDamagePopup(*playerState.entity, playerState.entity->getName(), damageLanded);
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
									Player::startHitAnimation(_engine, playerState, combatState.attackFirstFrame)) {
								notePlayerActivity();
							}
						}
						if (changed && playerHitPointsAfter <= 0) {
							playerAlive = false;
							stopPlayerRegionInteraction();
							startPlayerDefeatSequence(
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
				// Native room-entry monster spawns seed a left/right pursue state immediately
				// from the player's relative X position. Our room-combat shim does not model
				// those actor-state bytes directly, so allow one immediate pursue evaluation
				// before the first animation-driven movement tick lands.
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
				// Native class-6 pursuit stores a target frame-left X, not a center X.
				const int desiredFrameLeftX = monsterCenterX < playerCenterX
					? playerRect.right + engageDistance - monsterRect.width() / 2
					: playerRect.left - engageDistance + monsterRect.width() / 2;
				if (ABS(monsterRect.left - desiredFrameLeftX) > liveWaypointTolerance) {
					attemptedMoveX = true;
					const int step = MAX<int>(1, roundRoomCombatFloat(
						Player::computeDepthScale(scene.state, (float)monster.posZ) * (float)horizontalStepBase));
					const int monsterFrameLeftOffset = monsterRect.left - monster.posX;
					const int desiredLogicalCenterX = desiredFrameLeftX - monsterFrameLeftOffset;
					monster.posX = stepTowardsRoomCombatInt(monster.posX, desiredLogicalCenterX, step);
					monster.posX = CLIP<int>(monster.posX, monster.screenMinXBound, monster.screenMaxXBound);
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
				if (monster.posZ != previousZ)
					entityManager->reinsertSceneEntity(entity);
				if (script)
					(void)script->syncRuntimeMonsterRecord(monster);
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
			const bool closeEnoughForAttack = isWithinNativeMonsterAttackEntryRange(
				*entity, (float)monster.posZ, *playerState.entity, playerState.z, engageDistance);
			// Native class-6 monsters store `now + random(99)` when they arm an attack, then
			// apply the fixed 0x32-tick gate when deciding whether a later strike may start.
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
			combatState.attackCooldownSeedTick = now + _engine.getRandomNumber(99);
			monster.facing = range.resumeFacing;
			entity->setAnimationFrameRange(range.firstFrame, range.lastFrame, false);
			entity->setAnimationRate(kNativeMonsterAttackAnimationRate);
			entity->setAnimationEnabled(true);
			entity->setCurrentFrame(range.firstFrame);
			debugC(1, kDebugCombat,
				"Harvester: combat monster attack start monster='%s' target='%s' frames=%d..%d contact=%d resume_facing=%d live_center_dx=%d z_delta=%.2f engage=%d rate=%d attack_seed_tick=%u close_bypass=%d",
				monster.monsterName.c_str(), combatState.attackTargetName.c_str(), range.firstFrame, range.lastFrame,
				combatState.attackContactFrame, range.resumeFacing, liveCenterDx, (double)zDelta,
				engageDistance, kNativeMonsterAttackAnimationRate, combatState.attackCooldownSeedTick,
				closeRangeBypassDistance);
			needsRedraw = true;
		}

		return Common::kNoError;
	};
	auto moveRoomItemDirectlyToInventory = [&](const ObjectRecord &object) -> Common::Error {
		Script *script = _engine.getScript();
		if (!script)
			return Common::kReadingFailed;

		const ObjectRecord savedObject = object;

		script->addRuntimeObjectToInventory(object.objectName);
		const Common::String inventoryOwner(kInventoryOwnerName);
		hideSceneObject(object.objectName, &inventoryOwner);
		removeSceneEntityByName(object.objectName);

		bool didTransition = false;
		if (shouldDispatchPickupActionOnDirectInventoryTransfer(object)) {
			InteractionResult pickupInteraction;
			if (script->executeActionTag(
					object.actionTag, pickupInteraction, true, scene.state.roomName)) {
				Common::Error interactionError =
					interactionProcessor.handleInteractionResult(
						pickupInteraction, didTransition, Common::String());
				if (interactionError.getCode() != Common::kNoError)
					return interactionError;
				if (flow.hasPendingMainMenuReturn())
					return Common::kNoError;
				if (shouldCancelDeniedPickup(pickupInteraction, didTransition)) {
					debugC(1, kDebugRoom,
						"Harvester: cancelling denied direct pickup object='%s' action='%s' text='%s'",
						savedObject.objectName.c_str(), savedObject.actionTag.c_str(),
						pickupInteraction.modalText.value.c_str());
					return restoreDeniedPickup(*script, savedObject, false);
				}
			}
		}

		if (!_inventory.refresh())
			return Common::kReadingFailed;
		needsRedraw = true;
		return Common::kNoError;
	};
	auto beginRoomItemCarry = [&](const ObjectRecord &object) -> Common::Error {
		Script *script = _engine.getScript();
		ResourceManager *resources = _engine.getResources();
		if (!script || !resources)
			return Common::kReadingFailed;

		const ObjectRecord savedObject = object;

		clearCarriedRoomItem();
		carriedRoomItemName = object.objectName;
		carriedRoomItemLabel = script->resolveObjectLabel(object);
		const Common::String spritePath = resolveCarriedObjectSpritePath(object);
		if (!spritePath.empty())
			loadBitmapResource(*resources, spritePath, carriedRoomItemBitmap);

		script->setRuntimeObjectVisible(object.currentOwnerOrRoom, object.objectName, false);
		hideSceneObject(object.objectName, nullptr);
		removeSceneEntityByName(object.objectName);

		bool didTransition = false;
		if (shouldDispatchPickupActionOnCarryStart(object)) {
			InteractionResult pickupInteraction;
			if (script->executeActionTag(
					object.actionTag, pickupInteraction, true, scene.state.roomName)) {
				Common::Error interactionError =
					interactionProcessor.handleInteractionResult(
						pickupInteraction, didTransition, Common::String());
				if (interactionError.getCode() != Common::kNoError)
					return interactionError;
				if (flow.hasPendingMainMenuReturn())
					return Common::kNoError;
				if (shouldCancelDeniedPickup(pickupInteraction, didTransition)) {
					debugC(1, kDebugRoom,
						"Harvester: cancelling denied carried pickup object='%s' action='%s' text='%s'",
						savedObject.objectName.c_str(), savedObject.actionTag.c_str(),
						pickupInteraction.modalText.value.c_str());
					return restoreDeniedPickup(*script, savedObject, true);
				}
				if (!didTransition &&
						!pickupInteraction.dialogueNpcName.empty() &&
						!script->isObjectInInventory(savedObject.objectName)) {
					debugC(1, kDebugRoom,
						"Harvester: restoring carried pickup after dialogue object='%s' action='%s' npc='%s'",
						savedObject.objectName.c_str(), savedObject.actionTag.c_str(),
						pickupInteraction.dialogueNpcName.c_str());
					return restoreDeniedPickup(*script, savedObject, true);
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
	auto handleInventoryTargetInteraction = [&](const ObjectRecord &target) -> Common::Error {
		if (!_inventory.hasSelection())
			return Common::kNoError;

		const Common::String selectedItemName = _inventory.getSelectedItemName();
		InteractionResult interaction;
		const bool handled = _engine.getScript()->resolveUseItemInteraction(
			selectedItemName, target, interaction, scene.state.roomName);
		if (!handled) {
			debugC(1, kDebugInventory,
				"Harvester: inventory target miss selected='%s' target='%s' owner='%s' action='%s'",
				selectedItemName.c_str(), target.objectName.c_str(),
				target.currentOwnerOrRoom.c_str(), target.actionTag.c_str());
			return Common::kNoError;
		}

		StartupRoomTransitionKind roomTransition = interaction.roomTransition;
		if (roomTransition == kStartupRoomTransitionNone && !interaction.nextRoomName.empty())
			roomTransition = kStartupRoomTransitionCloseup;

		// Flow shares one inventory system across nested room loops. Clear the held item
		// before entering a closeup so it is not still carried while the child room loop
		// is active.
		if (!interaction.nextRoomName.empty() &&
				roomTransition != kStartupRoomTransitionChangeRoom)
			_inventory.clearSelection();

		bool didTransition = false;
		Common::Error interactionError =
			interactionProcessor.handleInteractionResult(interaction, didTransition, selectedItemName);
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		_inventory.clearSelection();
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!_inventory.refresh())
			return Common::kReadingFailed;

		needsRedraw = true;
		return Common::kNoError;
	};
	auto findSelectedInventoryRoomTarget = [&](const Common::Point &point) -> ObjectRecord * {
		if (!_inventory.hasSelection())
			return nullptr;

		EntityManager *entityManager = _engine.getRuntimeEntities();
		Script *script = _engine.getScript();
		if (!entityManager || !script)
			return nullptr;

		const Common::String selectedItemName = _inventory.getSelectedItemName();
		ObjectRecord *bestTarget = nullptr;
		int bestDrawIndex = -1;
		for (ObjectRecord &candidate : scene.sceneObjects) {
			if (candidate.objectName.empty() ||
					!script->hasUseItemInteraction(selectedItemName, candidate)) {
				continue;
			}

			const Entity *entity = entityManager->findSceneEntityByName(candidate.objectName);
			if (!entity || !entity->hitTest(point))
				continue;
			if (entity->getClassId() == kRuntimeEntityClassBackground ||
					entity->getClassId() == kRuntimeEntityClassPlayer ||
					entity->getClassId() == kRuntimeEntityClassRectHotspot19) {
				continue;
			}

			const int drawIndex = entityManager->findSceneEntityDrawIndexByName(entity->getName());
			if (drawIndex < 0)
				continue;
			if (!bestTarget || drawIndex > bestDrawIndex) {
				bestTarget = &candidate;
				bestDrawIndex = drawIndex;
			}
		}

		return bestTarget;
	};
	auto queueRegionInteraction = [&](const RegionRecord &region) {
		pendingRegionName = region.regionName;
		if (!playerState.entity)
			return;

		const bool playerOverlapsRegion = doesPlayerOverlapRegion(*playerState.entity, region);
		if (_engine.isPathfindingDebugEnabled()) {
			const Common::Rect playerRect = playerState.entity->getScreenRect();
			const Common::Rect regionBounds(region.left, region.top, region.right + 1, region.bottom + 1);
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding region click room='%s' mouse=(%d,%d) region='%s' bounds=(%d,%d)-(%d,%d) z=[%d,%d] start=%d cursor=%d desired_facing=%d player=(%d,%d,z=%.2f) player_rect=(%d,%d)-(%d,%d) overlaps=%d action='%s'",
				scene.state.roomName.c_str(), _mousePos.x, _mousePos.y,
				region.regionName.c_str(),
				regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
				region.minZ, region.maxZ, region.startEnabled, region.cursorEnabled,
				region.desiredFacing,
				playerState.centerX, playerState.bottomY, (double)playerState.z,
				playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
				playerOverlapsRegion, region.actionTag.c_str());
		}
		if (playerOverlapsRegion)
			return;

		Player::setRegionMoveTarget(scene.state, playerState,
			Player::resolveRegionTargetX(scene.state, region, playerState),
			Player::resolveRegionTargetZ(region));
		if (_engine.isPathfindingDebugEnabled()) {
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding region target room='%s' region='%s' target=(%d,%d,z=%.2f)",
				scene.state.roomName.c_str(), region.regionName.c_str(),
				playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ);
		}
	};
	auto runRegionInteraction = [&](const RegionRecord &region) -> Common::Error {
		InteractionResult interaction;
		if (!_engine.getScript()->resolveRegionInteraction(region, interaction, scene.state.roomName))
			return Common::kNoError;

		bool didTransition = false;
		Common::Error interactionError =
			interactionProcessor.handleInteractionResult(
				interaction, didTransition, Common::String());
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;

		if (!_inventory.refresh())
			return Common::kReadingFailed;
		needsRedraw = true;
		return Common::kNoError;
	};
	auto tryActivatePendingRegion = [&]() -> Common::Error {
		if (pendingRegionName.empty() || !playerState.entity)
			return Common::kNoError;

		const RegionRecord *region = findSceneRegionByName(scene.sceneRegions, pendingRegionName);
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
	auto tryActivateHoveredRegion = [&]() -> Common::Error {
		if (!playerState.entity)
			return Common::kNoError;

		const RoomHoverState hoverState = resolveRoomHoverState(
			_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
			scene.sceneRegions, _mousePos, &flow._dialogue);
		if (!hoverState.region || !hoverState.region->startEnabled)
			return Common::kNoError;
		if (!doesPlayerOverlapRegion(*playerState.entity, *hoverState.region))
			return Common::kNoError;
		if (!doesPlayerFacingMatchRegion(playerState.facing, *hoverState.region))
			return Common::kNoError;

		stopPlayerRegionInteraction();
		return runRegionInteraction(*hoverState.region);
	};
	auto tryActivatePassiveRegion = [&]() -> Common::Error {
		if (!playerState.entity)
			return Common::kNoError;

		for (const RegionRecord &region : scene.sceneRegions) {
			if (!region.startEnabled || region.cursorEnabled)
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
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (flow.takePendingNewGameRestart()) {
			flow.prepareForNewGame();
			pendingRoomChange = "START";
			pendingRoomChangeIsRoomName = false;
			pendingRoomChangeUsesSavedRoomState = false;
			pendingRoomChangeSavedRoomState.clear();
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (_engine.hasPendingLoadedSaveRoomState()) {
			pendingRoomChange = _engine.getPendingLoadedSaveRoomState().entranceName;
			pendingRoomChangeIsRoomName = false;
			pendingRoomChangeUsesSavedRoomState = false;
			pendingRoomChangeSavedRoomState.clear();
			if (pendingRoomChange.empty()) {
				pendingRoomChange = _engine.getPendingLoadedSaveRoomState().roomName;
				pendingRoomChangeIsRoomName = true;
			}
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (flow.hasPendingDebugRoomChange()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			return Common::kNoError;
		}
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		InteractionResult queuedDebugInteraction;
		if (flow.takeQueuedDebugInteraction(queuedDebugInteraction)) {
			// Run console-injected commands through the normal room interaction handler so
			// CHANGE_ROOM, modal text, dialogue, and chained tags behave exactly like script-driven actions.
			bool didDebugTransition = false;
			Common::Error debugInteractionError = interactionProcessor.handleInteractionResult(
				queuedDebugInteraction, didDebugTransition, Common::String());
			if (debugInteractionError.getCode() != Common::kNoError)
				return debugInteractionError;
			if (flow.hasPendingMainMenuReturn())
				return Common::kNoError;
			if (!pendingRoomChange.empty()) {
				if (!stowCarriedRoomItemToInventory())
					return Common::kReadingFailed;
				break;
			}
			needsRedraw = true;
		}
		captureCurrentSaveState();
		if (_inventory.isOpen() && _inventory.refreshIfRuntimeStateChanged())
			needsRedraw = true;
		if (!combatLoadoutStatusText.value.empty() &&
				combatLoadoutStatusHideTick != 0 &&
				(int32)(Player::getRuntimeClockTicks() - combatLoadoutStatusHideTick) >= 0) {
			combatLoadoutStatusText = ResolvedText();
			combatLoadoutStatusHideTick = 0;
			if (closeInventoryAfterCombatLoadoutStatus) {
				closeInventoryAfterCombatLoadoutStatus = false;
				if (_inventory.close())
					needsRedraw = true;
			}
			needsRedraw = true;
		}

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
				isPlayerCombatLocked() ||
				(_inventory.isOpen() && (inventoryPanelContainsMouse || !inventorySelectionActive));
			RoomHoverState hoverState = suppressHover
				? RoomHoverState()
				: resolveRoomHoverState(_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, _mousePos, &flow._dialogue);
			if (!suppressHover && inventorySelectionActive && !hoverState.npc) {
				if (ObjectRecord *selectedTarget = findSelectedInventoryRoomTarget(_mousePos))
					hoverState.object = selectedTarget;
			}
			Common::String promptText;
			Common::String inventoryTooltipText;
			auto resolveCarryTargetLabel = [&]() {
				if (hoverState.playerEntity && playerState.entity && hoverState.playerEntity == playerState.entity)
					return Common::String(kPlayerInventoryLabel);
				if (hoverState.object)
					return _engine.getScript()->resolveObjectLabel(*hoverState.object);
				if (hoverState.npc)
					return resolveStartupNpcLabel(*hoverState.npc);
				return Common::String();
			};
			if (activeCarry) {
				Common::String targetLabel;
				const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
				if (_inventory.isOpen()) {
					if (inventoryHover && !InventorySystem::isExitObject(inventoryHover->object) &&
							!InventorySystem::isStatusObject(inventoryHover->object) &&
							!inventoryHover->object.objectName.equalsIgnoreCase(_inventory.getSelectedItemName())) {
						targetLabel = _engine.getScript()->resolveObjectLabel(inventoryHover->object);
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
				const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
				if (inventoryHover &&
						!InventorySystem::isExitObject(inventoryHover->object)) {
					inventoryTooltipText =
						_engine.getScript()->resolveInventoryTooltipText(inventoryHover->object);
				}
				_inventory.setPromptText(Common::String());
				hoverState.cursorSequence = 7;
			} else {
				promptText = hoverState.promptText;
			}
			if (Entity *cursor = entityManager ? entityManager->getCursorEntity() : nullptr) {
				cursor->setAnimationSequence(
					(showingInspectText || idleState.active || idleState.exiting ||
						isPlayerCombatLocked() ||
						_inventory.isOpen() || activeCarry)
						? 7
						: hoverState.cursorSequence);
			}

			drawRoomScene(_engine, *activeScreen, scene, scene.targetPaletteBrightness);
			drawCombatDamagePopups(*activeScreen);
			if (_engine.getScript() && playerState.entity) {
				drawCombatLoadoutResourceIcons(*activeScreen, *art, *_engine.getScript(),
					_engine.getScript()->getPlayerCombatLoadout());
			}
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
			} else if (!combatLoadoutStatusText.value.empty()) {
				drawRoomInspectText(*activeScreen, *art, *inspectFont, combatLoadoutStatusText,
					useNativeInspectFont);
			} else if (!inventoryTooltipText.empty()) {
				drawInventoryTooltip(*activeScreen, *inventoryTooltipFont, inventoryTooltipText);
			} else if (!promptText.empty()) {
				drawRoomPrompt(*activeScreen, *promptFont, promptText, useNativePromptFont);
			}

			if (entityManager)
				entityManager->drawCursor(*activeScreen);
			activeScreen->makeAllDirty();
			activeScreen->update();
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				if (_inventory.isOpen() && _inventory.hasSelection()) {
					const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
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
				if (isPlayerCombatLocked())
					break;
				pendingRegionName.clear();
				if (_inventory.isOpen()) {
					const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
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
						Script *script = _engine.getScript();
						const int previousLoadout = playerState.combatLoadout >= 0
							? playerState.combatLoadout
							: (script ? script->getPlayerCombatLoadout() : 0);
						bool loadoutChanged = false;
						if (_inventory.toggleCombatLoadout(inventoryHover->object,
								playerState.combatLoadout, loadoutChanged)) {
							debugC(1, kDebugInventory,
								"Harvester: inventory right click handled as combat toggle object='%s' changed=%d",
								inventoryHover->object.objectName.c_str(), loadoutChanged);
							if (loadoutChanged) {
								if (script) {
									const Common::String statusMessage = buildCombatLoadoutStatusMessage(
										*script, inventoryHover->object, previousLoadout,
										script->getPlayerCombatLoadout());
									queueCombatLoadoutStatusMessage(statusMessage);
									closeInventoryAfterCombatLoadoutStatus = true;
								}
								captureCurrentSaveState();
								if (playerState.entity) {
									(void)Player::syncCombatLoadoutVisual(_engine, scene.state, playerState,
										_engine.getScript()->getPlayerCombatLoadout());
								} else if (_engine.getScript()) {
									playerState.combatLoadout =
										_engine.getScript()->getPlayerCombatLoadout();
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

							InteractionResult interaction;
							bool didTransition = false;
							const bool executedActionTag = _engine.getScript()->executeActionTag(
									secondaryAction.actionTag, interaction, true, scene.state.roomName);
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
									didTransition, flow.hasPendingMainMenuReturn());
								if (interactionError.getCode() != Common::kNoError)
									return interactionError;
								if (flow.hasPendingMainMenuReturn())
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
				Script *script = _engine.getScript();
				if (!showingInspectText && !isPlayerCombatLocked() && script && playerState.entity &&
						script->getPlayerCurrentHitPoints() > 0) {
					playerState.hasMoveTarget = false;
					playerState.turnActive = false;
					playerState.turnTargetFacing = -1;
					playerState.turnFirstFrame = -1;
					playerState.turnLastFrame = -1;
					playerState.turnEndFrame = -1;
					playerState.turnPlayBackwards = false;
					(void)Player::syncCombatLoadoutVisual(_engine, scene.state, playerState,
						script->getPlayerCombatLoadout());
					capturePlayerAttackTarget();
					if (Player::startAttackAnimation(scene.state, playerState, _mousePos)) {
						debugC(1, kDebugCombat,
							"Harvester: combat player attack start loadout=%d weapon='%s' damage=%d damage_type='%s' target_class='%s' target='%s' cursor=(%d,%d)",
							playerState.combatLoadout,
							Player::describeCombatLoadout(playerState.combatLoadout),
							Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout),
							Player::describeCombatDamageType(Player::resolveCombatLoadoutDamageType(playerState.combatLoadout)),
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
						inspectText = ResolvedText();
						needsRedraw = true;
					}
					break;
				}
				if (isPlayerCombatLocked())
					break;

				const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
				if (_inventory.isOpen()) {
					const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
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

				const RoomHoverState hoverState = resolveRoomHoverState(
					_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
					_mousePos, &flow._dialogue);
				ObjectRecord *selectedRoomTarget = nullptr;
				if (_inventory.hasSelection() && !hoverState.npc)
					selectedRoomTarget = findSelectedInventoryRoomTarget(_mousePos);
				if (selectedRoomTarget &&
						(!hoverState.object ||
						 !selectedRoomTarget->objectName.equalsIgnoreCase(hoverState.object->objectName))) {
					debugC(1, kDebugInventory,
						"Harvester: inventory target remap selected='%s' hovered='%s' remapped='%s'",
						_inventory.getSelectedItemName().c_str(),
						hoverState.object ? hoverState.object->objectName.c_str() : "",
						selectedRoomTarget->objectName.c_str());
				}
				RoomHoverState clickHoverState = hoverState;
				if (selectedRoomTarget)
					clickHoverState.object = selectedRoomTarget;
				debugC(1, kDebugRoom,
					"Harvester: room click room='%s' mouse=(%d,%d) object='%s' npc='%s' region='%s' cursor_sequence=%d prompt='%s'",
					scene.state.roomName.c_str(), _mousePos.x, _mousePos.y,
					clickHoverState.object ? clickHoverState.object->objectName.c_str() : "",
					clickHoverState.npc ? clickHoverState.npc->npcName.c_str() : "",
					clickHoverState.region ? clickHoverState.region->regionName.c_str() : "",
					clickHoverState.cursorSequence, clickHoverState.promptText.c_str());
					if (hasCarriedRoomItem()) {
					if (clickHoverState.playerEntity && playerState.entity &&
							clickHoverState.playerEntity == playerState.entity) {
						if (!stowCarriedRoomItemToInventory())
							return Common::kReadingFailed;
						needsRedraw = true;
					}
					break;
				}
				if (_inventory.hasSelection()) {
					const Common::String selectedItemName = _inventory.getSelectedItemName();
					if (clickHoverState.npc) {
						bool didTransition = false;
						Common::Error dialogueError = interactionProcessor.runScriptedDialogue(
							clickHoverState.npc->npcName, selectedItemName, Common::String(), didTransition);
						if (dialogueError.getCode() != Common::kNoError)
							return dialogueError;
						if (flow.hasPendingMainMenuReturn())
							return Common::kNoError;
						if (_inventory.clearSelection())
							needsRedraw = true;
						break;
					}

					ObjectRecord *roomTarget = clickHoverState.object
						? findSceneObjectByName(scene.sceneObjects, clickHoverState.object->objectName)
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
					if (flow.hasPendingMainMenuReturn())
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
						if (flow.hasPendingMainMenuReturn())
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
				ObjectRecord *clickedObject = hoverState.object
					? findSceneObjectByName(scene.sceneObjects, hoverState.object->objectName)
					: nullptr;
				if (!clickedObject) {
					pendingRegionName.clear();
					if (hoverState.cursorSequence == 0 && playerState.entity) {
						if (_engine.isPathfindingDebugEnabled()) {
							const Common::Rect playerRect = playerState.entity->getScreenRect();
							debugC(1, kDebugPathfinding,
								"Harvester: pathfinding click accepted room='%s' mouse=(%d,%d) movement_band=(y=%d..%d,z=%d..%d) player=(%d,%d,z=%.2f) player_rect=(%d,%d)-(%d,%d)",
								scene.state.roomName.c_str(), _mousePos.x, _mousePos.y,
								scene.state.roomMaxZScreenY, scene.state.roomMinZScreenY,
								scene.state.roomMinZ, scene.state.roomMaxZ,
								playerState.centerX, playerState.bottomY, (double)playerState.z,
								playerRect.left, playerRect.top, playerRect.right, playerRect.bottom);
						}
						Player::setMoveTargetFromScreenPoint(scene.state, playerState, _mousePos.x, _mousePos.y);
						if (_engine.isPathfindingDebugEnabled()) {
							debugC(1, kDebugPathfinding,
								"Harvester: pathfinding click target room='%s' mouse=(%d,%d) target=(%d,%d,z=%.2f)",
								scene.state.roomName.c_str(), _mousePos.x, _mousePos.y,
								playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ);
						}
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

				ResolvedText resolvedInspectText;
				const bool hasInspectText =
					_engine.getScript()->resolveObjectInspectText(*clickedObject, resolvedInspectText);
				const bool unlocksAfterInitialExamine =
					unlocksRoomObjectInteractionAfterInitialExamine(*clickedObject, *_engine.getScript());
				const bool canShowInspectText =
					hasInspectText && resolveInspectTextboxBitmap(*art, resolvedInspectText);
				if (!clickedObject->identShown && unlocksAfterInitialExamine) {
					clickedObject->identShown = true;
					_engine.getScript()->markObjectIdentShown(*clickedObject);
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
				if (_engine.getScript()->isPickupObject(*clickedObject)) {
					InteractionResult blockedPickupInteraction;
					if (_engine.getScript()->isPickupBlockedByAction(
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

				InteractionResult interaction;
				if (!_engine.getScript()->resolveObjectInteraction(
						*clickedObject, interaction, scene.state.roomName)) {
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
				if (flow.hasPendingMainMenuReturn())
					return Common::kNoError;
				needsRedraw = true;
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (showingInspectText) {
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						showingInspectText = false;
						inspectCanDismiss = false;
						inspectText = ResolvedText();
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
				if (isPlayerCombatLocked())
					break;

				if (event.kbd.keycode == Common::KEYCODE_LCTRL ||
						event.kbd.keycode == Common::KEYCODE_RCTRL) {
					attackModifierHeld = true;
				} else if (event.kbd.keycode == Common::KEYCODE_k && _engine.isCombatDebugEnabled()) {
					Common::Error debugKillError = killDebugActiveMonster();
					if (debugKillError.getCode() != Common::kNoError)
						return debugKillError;
					break;
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
					Common::Error menuError = flow.runRoomMenuStub(
						roomMenuBackdrop, scene.palette, scene.targetPaletteBrightness,
						scene.state.hasEntrance);
					if (menuError.getCode() != Common::kNoError)
						return menuError;
					flow.resetCursorAnimationSequence();
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

		const bool playerCanAct =
			_engine.getScript() &&
			_engine.getScript()->getPlayerCurrentHitPoints() > 0;
		if (!playerCanAct && (moveLeft || moveRight || moveUp || moveDown ||
				playerState.hasMoveTarget || playerState.turnActive ||
				playerState.attackActive || playerState.hitActive)) {
			attackModifierHeld = false;
			stopPlayerRegionInteraction();
		}

		bool playerAdvancedThisFrame = false;
		Common::Error combatError = resolvePlayerAttackContact();
		if (combatError.getCode() != Common::kNoError)
			return combatError;
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (Player::updateAttackAnimationState(_engine, playerState)) {
			needsRedraw = true;
		}
		if (Player::updateHitAnimationState(
				_engine, scene.state, scene.sceneObjects, scene.sceneAnimations, playerState)) {
			playerAdvancedThisFrame = true;
			needsRedraw = true;
		}
		const bool keyboardAttackRequested =
			attackModifierHeld && (moveLeft || moveRight || moveUp || moveDown);
		if (playerCanAct &&
				!playerState.attackActive && !playerState.hitActive && keyboardAttackRequested &&
				!idleState.active && !idleState.exiting) {
			const uint32 now = Player::getRuntimeClockTicks();
			if (nextKeyboardAttackAllowedTick == 0 || (int32)(now - nextKeyboardAttackAllowedTick) >= 0) {
				capturePlayerAttackTargetForKeyboard(moveLeft, moveRight, moveUp, moveDown);
				if (Player::startKeyboardAttackAnimation(
						scene.state, playerState, moveLeft, moveRight, moveUp, moveDown)) {
					nextKeyboardAttackAllowedTick = now + kNativeKeyboardAttackRepeatTicks;
					notePlayerActivity();
					debugC(1, kDebugCombat,
						"Harvester: combat player keyboard attack start loadout=%d weapon='%s' damage=%d damage_type='%s' target_class='%s' target='%s' input=(L=%d R=%d U=%d D=%d) facing=%d",
						playerState.combatLoadout,
						Player::describeCombatLoadout(playerState.combatLoadout),
						Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout),
						Player::describeCombatDamageType(Player::resolveCombatLoadoutDamageType(playerState.combatLoadout)),
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
		if (!playerState.attackActive && !playerState.hitActive &&
				Player::updateTurnAnimationState(playerState)) {
			playerAdvancedThisFrame = true;
			needsRedraw = true;
		}

		if (playerCanAct &&
				!playerState.attackActive && !playerState.hitActive &&
				!keyboardAttackRequested && !idleState.active && !idleState.exiting) {
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
					!playerState.turnActive && !playerState.hitActive &&
					playerState.entity && playerState.facing >= 0 &&
					Player::setIdleAnimation(playerState, playerState.facing)) {
				needsRedraw = true;
			}

			if (!showingInspectText && !keyboardAttackRequested &&
					!moveLeft && !moveRight && !moveUp && !moveDown &&
					!playerState.hasMoveTarget && !playerState.turnActive && !playerState.hitActive &&
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
		if (syncCombatHitEffects())
			needsRedraw = true;
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		Common::Error pendingRegionError =
			isPlayerCombatLocked()
				? Common::kNoError
				: tryActivatePendingRegion();
		if (pendingRegionError.getCode() != Common::kNoError)
			return pendingRegionError;
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (playerAdvancedThisFrame && !isPlayerCombatLocked()) {
			pendingRegionError = tryActivatePassiveRegion();
			if (pendingRegionError.getCode() != Common::kNoError)
				return pendingRegionError;
			if (flow.hasPendingMainMenuReturn())
				return Common::kNoError;
			if (!pendingRoomChange.empty()) {
				if (!stowCarriedRoomItemToInventory())
					return Common::kReadingFailed;
				break;
			}

			pendingRegionError = tryActivateHoveredRegion();
			if (pendingRegionError.getCode() != Common::kNoError)
				return pendingRegionError;
			if (flow.hasPendingMainMenuReturn())
				return Common::kNoError;
			if (!pendingRoomChange.empty()) {
				if (!stowCarriedRoomItemToInventory())
					return Common::kReadingFailed;
				break;
			}
		}

		if (flow.tickRuntimeEntities())
			needsRedraw = true;
		syncAnimatedRoomActorPlacement();
		if (Player::updateDeathAnimationState(playerState)) {
			requestPlayerGameOver("combat_player_death_complete", Common::String());
			needsRedraw = true;
		}
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (pruneCombatHitEffects())
			needsRedraw = true;
		if (pruneCombatDamagePopups())
			needsRedraw = true;
		if (entityManager) {
			Common::Array<Common::String> expiredTimerNames;
			if (entityManager->takeExpiredTimerNames(expiredTimerNames)) {
				syncCurrentRoomRuntimeState();
				for (const Common::String &timerName : expiredTimerNames) {
					InteractionResult timerInteraction;
					if (!_engine.getScript()->executeTimerAction(
							timerName, timerInteraction, true, scene.state.roomName))
						continue;

					bool didTransition = false;
					Common::Error interactionError =
						interactionProcessor.handleInteractionResult(
							timerInteraction, didTransition, Common::String());
					if (interactionError.getCode() != Common::kNoError)
						return interactionError;
					if (flow.hasPendingMainMenuReturn())
						return Common::kNoError;
					needsRedraw = true;
					if (!pendingRoomChange.empty())
						break;
				}
			}
		}
		if (Player::updateIdleAnimation(scene.state, playerState, idleState))
			needsRedraw = true;
		if (_engine.isCombatDebugEnabled() && !damagePopupStates.empty())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}
	if (_engine.shouldQuit())
		return Common::kNoError;
	if (flow.hasPendingMainMenuReturn())
		return Common::kNoError;
	if (pendingRoomChange.empty())
		return Common::kNoError;
	if (pendingRoomChange.equals(kExitCloseupPendingRoomChange)) {
		flow.requestCloseupParentRestart();
		return Common::kNoError;
	}

	currentRoomTarget = pendingRoomChange;
	currentTargetIsRoomName = pendingRoomChangeIsRoomName;
	currentTargetUsesSavedRoomState = pendingRoomChangeUsesSavedRoomState;
	currentTargetSavedRoomState = pendingRoomChangeSavedRoomState;
	}

	return Common::kNoError;
}

} // End of namespace Harvester
