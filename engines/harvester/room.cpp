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

#include "common/algorithm.h"
#include "common/events.h"
#include "common/ptr.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/cheats.h"
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
#include "harvester/room_session.h"
#include "harvester/room_support.h"

namespace Harvester {

static const uint32 kRoomExitFastClickWindowTicks = 20;
static const char *const kPlayerInventoryLabel = "your inventory";
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

static Common::String resolveStartupNpcLabel(const NpcRecord &npc) {
	Common::String label = !npc.entityInitArg.empty() ? npc.entityInitArg : npc.npcName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

static bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap = IndexedBitmap();
	return decodeBitmap(data, bitmap);
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

static void setScaledRoomPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

RoomSystem::RoomSystem(HarvesterEngine &engine, Common::Point &mousePos,
		InventorySystem &inventory, CheatSystem &cheats)
	: _engine(engine), _mousePos(mousePos), _inventory(inventory), _cheats(cheats) {
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
		RoomSession session(_engine, flow, scene, entityManager, _inventory);
		debugC(1, kDebugRoom, "Harvester: created room session room='%s'", scene.state.roomName.c_str());
		RoomPlayerState &playerState = session.getPlayerState();
		playerState.entity = entityManager ? entityManager->findSceneEntityByName("PLAYER") : nullptr;
		playerState.centerX = scene.state.playerSpawnX;
		playerState.bottomY = scene.state.playerSpawnY;
		playerState.z = (float)scene.state.playerSpawnZ;
		playerState.facing = scene.state.playerFacing;
		playerState.combatLoadout =
			_engine.getScript() ? _engine.getScript()->getPlayerCombatLoadout() : 0;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		RoomCombat &combat = session.getCombat();
		combat.initializeCombatStates(scene.state);
		RoomSceneRuntime &sceneRuntime = session.getSceneRuntime();
		RoomCombatEffects &combatEffects = session.getCombatEffects();
		ResolvedText inspectText;
		Common::String currentInteractionPromptText;
		bool showingInspectText = false;
		bool inspectCanDismiss = false;
		auto beginInspectText = [&](const ObjectRecord &object, const ResolvedText &text) {
			inspectText = text;
			showingInspectText = true;
			inspectCanDismiss = false;
			debugC(2, kDebugRoom,
				"Harvester: showing IDENT object='%s' prompt='%s' box='%s' text='%s'",
				object.objectName.c_str(), currentInteractionPromptText.c_str(),
				inspectText.boxName.c_str(), inspectText.value.c_str());
		};
		ResolvedText combatLoadoutStatusText;
		uint32 combatLoadoutStatusHideTick = 0;
		bool closeInventoryAfterCombatLoadoutStatus = false;
		bool &moveLeft = session.getMovementInput().moveLeft;
		bool &moveRight = session.getMovementInput().moveRight;
		bool &moveUp = session.getMovementInput().moveUp;
		bool &moveDown = session.getMovementInput().moveDown;
		bool attackModifierHeld = false;
		Common::String &pendingRegionName = session.getPendingRegionName();
		Common::String pendingRoomChange;
		bool pendingRoomChangeIsRoomName = false;
		bool pendingRoomChangeUsesSavedRoomState = false;
		SaveRoomState pendingRoomChangeSavedRoomState;
		const bool canExitCloseupToParent = flow._roomLoopDepth > 1;
		uint32 lastLeftButtonReleaseTick = 0;
		uint32 nextKeyboardAttackAllowedTick = 0;
		RoomIdleAnimationState &idleState = session.getIdleState();
		bool needsRedraw = true;
		RoomItemInteractions roomItems(_engine, scene, sceneRuntime, _inventory, pendingRoomChange);

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

		session.resetIdleState();

		auto syncAnimatedRoomActorPlacement = [&]() {
			Player::syncAnimationPlacement(playerState);
			if (!entityManager)
				return;

			// Class-4 NPCs retain their spawn base through ambient and death animation banks.
			for (const MonsterRecord &monster : scene.state.roomMonsters) {
				Entity *entity = entityManager->findSceneEntityByName(monster.monsterName);
				if (!entity)
					continue;
				(void)applyRoomActorPlacement(scene.state, *entity,
					monster.posX, monster.posY, (float)monster.posZ);
			}
		};

		auto openInventoryOverlay = [&]() {
			moveLeft = false;
			moveRight = false;
			moveUp = false;
			moveDown = false;
			pendingRegionName.clear();
			(void)session.cancelPlayerAttackAnimation();
			playerState.hasMoveTarget = false;
			playerState.turnActive = false;
			playerState.turnTargetFacing = -1;
			if (!roomItems.stowCarriedRoomItemToInventory())
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
					sceneRuntime.syncGlobalTimerEntities(exitInteraction.previousTimerRecords, true);

				if (!exitInteraction.cutscenePath.empty()) {
					FstPlayer fstPlayer(_engine);
					if (!fstPlayer.play(exitInteraction.cutscenePath))
						return Common::kReadingFailed;
				}

				if (!exitInteraction.dialogueNpcName.empty() ||
						!exitInteraction.dialogueContinuationTag.empty() ||
						!exitInteraction.modalText.value.empty() ||
						exitInteraction.lightingCommand != kStartupLightingCommandNone ||
						exitInteraction.requestPlayerGotoXZ ||
						exitInteraction.requestPlayerGotoZ) {
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

				exitInteraction = Common::move(continuationInteraction);
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
			session.resetIdleState();
			session.captureCurrentSaveState();
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
		typedef decltype(captureDialogueBackdrop) CaptureDialogueBackdropFn;
		typedef decltype(showCdChangePrompt) ShowCdChangePromptFn;
		typedef decltype(runRoomExitCommands) RunRoomExitCommandsFn;
		typedef decltype(applyLightingCommand) ApplyLightingCommandFn;
		typedef decltype(applyPlayerGotoXZ) ApplyPlayerGotoXZFn;
		typedef decltype(runModalShowText) RunModalShowTextFn;
		typedef decltype(startPlayerDefeatSequence) StartPlayerDefeatSequenceFn;

		struct RoomInteractionCallbacksImpl : RoomInteractionCallbacks {
			RoomInteractionCallbacksImpl(
					RoomSession &session_,
					CaptureDialogueBackdropFn &captureDialogueBackdropFn_,
					ShowCdChangePromptFn &showCdChangePromptFn_,
					RunRoomExitCommandsFn &runRoomExitCommandsFn_,
					ApplyLightingCommandFn &applyLightingCommandFn_,
					ApplyPlayerGotoXZFn &applyPlayerGotoXZFn_,
					RunModalShowTextFn &runModalShowTextFn_,
					StartPlayerDefeatSequenceFn &startPlayerDefeatSequenceFn_)
				: session(session_),
				  captureDialogueBackdropFn(captureDialogueBackdropFn_),
				  showCdChangePromptFn(showCdChangePromptFn_),
				  runRoomExitCommandsFn(runRoomExitCommandsFn_),
				  applyLightingCommandFn(applyLightingCommandFn_),
				  applyPlayerGotoXZFn(applyPlayerGotoXZFn_),
				  runModalShowTextFn(runModalShowTextFn_),
				  startPlayerDefeatSequenceFn(startPlayerDefeatSequenceFn_) {
			}

			bool refreshCurrentScene(bool preservePlayerPlacement) override {
				return session.refreshCurrentScene(preservePlayerPlacement);
			}

			bool applyCurrentRoomRuntimeMutationsInPlace(bool preservePaletteState) override {
				return session.applyCurrentRoomRuntimeMutationsInPlace(preservePaletteState);
			}

			void syncGlobalTimerEntities(const Common::Array<TimerRecord> &previousTimerRecords) override {
				session.getSceneRuntime().syncGlobalTimerEntities(previousTimerRecords);
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
				session.resetIdleState();
			}

			void notePlayerActivity() override {
				session.notePlayerActivity();
			}

			void stopPlayerRegionInteraction() override {
				session.stopPlayerRegionInteraction();
			}

			void startPlayerDefeatSequence(const char *reason, const Common::String &sourceName,
					int damageType) override {
				startPlayerDefeatSequenceFn(reason, sourceName, damageType);
			}

			RoomSession &session;
			CaptureDialogueBackdropFn &captureDialogueBackdropFn;
			ShowCdChangePromptFn &showCdChangePromptFn;
			RunRoomExitCommandsFn &runRoomExitCommandsFn;
			ApplyLightingCommandFn &applyLightingCommandFn;
			ApplyPlayerGotoXZFn &applyPlayerGotoXZFn;
			RunModalShowTextFn &runModalShowTextFn;
			StartPlayerDefeatSequenceFn &startPlayerDefeatSequenceFn;
		};
		RoomInteractionCallbacksImpl interactionCallbacks(
			session,
			captureDialogueBackdrop, showCdChangePrompt,
			runRoomExitCommands, applyLightingCommand, applyPlayerGotoXZ, runModalShowText,
			startPlayerDefeatSequence);
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
				sceneRuntime.syncGlobalTimerEntities(roomEntryInteraction.previousTimerRecords);
				if (!session.applyCurrentRoomRuntimeMutationsInPlace(false) &&
						!session.refreshCurrentScene(true)) {
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
			session.resetIdleState();
		}
		ScopedDeferredLiveNpcDeathTransitions deferredLiveNpcDeathTransitions(
			_engine.getScript());

	auto capturePlayerAttackTarget = [&]() {
		playerState.attackTargetName.clear();
		playerState.attackTargetClassId = -1;

		const RoomHoverState hoverState = resolveRoomHoverState(
			_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
			_mousePos, flow._menuTextConfig, &flow._dialogue);
		combat.capturePlayerAttackTarget(scene.state, playerState, entityManager, _mousePos, hoverState.npc);
	};

	if (!_inventory.refresh())
		return Common::kReadingFailed;
	Graphics::FrameLimiter limiter(g_system, 60);
	session.captureCurrentSaveState();

	if (shouldRunStartupRoomProbe())
		logStartupRoomProbe(_engine, scene, currentRoomTarget, _mousePos, flow._menuTextConfig);

	while (!_engine.shouldQuit()) {
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (flow.takePendingNewGameRestart()) {
			flow.prepareForNewGame();
			pendingRoomChange = "START";
			pendingRoomChangeIsRoomName = false;
			pendingRoomChangeUsesSavedRoomState = false;
			pendingRoomChangeSavedRoomState.clear();
			if (!roomItems.stowCarriedRoomItemToInventory())
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
			if (!roomItems.stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (flow.hasPendingDebugRoomChange()) {
			if (!roomItems.stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			return Common::kNoError;
		}
		if (!pendingRoomChange.empty()) {
			if (!roomItems.stowCarriedRoomItemToInventory())
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
				if (!roomItems.stowCarriedRoomItemToInventory())
					return Common::kReadingFailed;
				break;
			}
			needsRedraw = true;
		}
		session.captureCurrentSaveState();
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
			const bool roomCarryActive = roomItems.hasCarriedRoomItem();
			const bool activeCarry = inventorySelectionActive || roomCarryActive;
			const bool inventoryPanelContainsMouse = _inventory.isOpen() && inventoryPanelBounds.contains(_mousePos);
			const bool suppressHover = showingInspectText || idleState.active || idleState.exiting ||
				session.isPlayerCombatLocked() ||
				(_inventory.isOpen() && (inventoryPanelContainsMouse || !inventorySelectionActive));
			RoomHoverState hoverState = suppressHover
				? RoomHoverState()
				: resolveRoomHoverState(_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, _mousePos, flow._menuTextConfig, &flow._dialogue);
			if (!suppressHover && inventorySelectionActive && !hoverState.npc) {
				if (ObjectRecord *selectedTarget = roomItems.findSelectedInventoryRoomTarget(_mousePos))
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
					promptText = _inventory.buildSelectedPrompt(targetLabel, flow._menuTextConfig);
					_inventory.setPromptText(promptText);
				} else {
					promptText = buildUseItemPrompt(
						flow._menuTextConfig, roomItems.getCarriedRoomItemLabel(), targetLabel);
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
			if (!showingInspectText)
				currentInteractionPromptText = promptText;
			if (Entity *cursor = entityManager ? entityManager->getCursorEntity() : nullptr) {
				cursor->setAnimationSequence(
					(showingInspectText || idleState.active || idleState.exiting ||
						session.isPlayerCombatLocked() ||
						_inventory.isOpen() || activeCarry)
						? 7
						: hoverState.cursorSequence);
			}

			drawRoomScene(_engine, *activeScreen, scene, scene.targetPaletteBrightness);
			combatEffects.drawCombatDamagePopups(*activeScreen, _engine.isCombatDebugEnabled());
			if (_engine.getScript() && playerState.entity) {
				drawCombatLoadoutResourceIcons(*activeScreen, *art, *_engine.getScript(),
					_engine.getScript()->getPlayerCombatLoadout());
			}
			if (_inventory.isOpen()) {
				_inventory.drawOverlay(*activeScreen);
				drawInventoryWeekday(*activeScreen, *inventoryTooltipFont,
					_inventory.resolveWeekdayLabel(flow._menuTextConfig));
			}
			if (inventorySelectionActive) {
				_inventory.drawSelectedDragItem(*activeScreen, _mousePos);
			} else if (roomCarryActive && roomItems.getCarriedRoomItemBitmap().isValid()) {
				const int drawX = _mousePos.x - (int)roomItems.getCarriedRoomItemBitmap().width / 2;
				const int drawY = _mousePos.y - (int)roomItems.getCarriedRoomItemBitmap().height / 2;
				blitBitmap(*activeScreen, roomItems.getCarriedRoomItemBitmap(), drawX, drawY);
			}

			if (showingInspectText) {
				drawRoomInspectText(*activeScreen, *art, *inspectFont, inspectText, useNativeInspectFont);
				if (!currentInteractionPromptText.empty())
					drawRoomPrompt(*activeScreen, *promptFont,
						currentInteractionPromptText, useNativePromptFont);
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
				session.notePlayerActivity();
				if (idleState.active || idleState.exiting) {
					if (Player::requestIdleAnimationExit(scene.state, playerState, idleState))
						needsRedraw = true;
					break;
				}
				if (session.isPlayerCombatLocked())
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
								session.captureCurrentSaveState();
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
				if (roomItems.hasCarriedRoomItem()) {
					if (!roomItems.stowCarriedRoomItemToInventory())
						return Common::kReadingFailed;
					needsRedraw = true;
					break;
				}
				Script *script = _engine.getScript();
				if (!showingInspectText && !session.isPlayerCombatLocked() && script && playerState.entity &&
						script->getPlayerCurrentHitPoints() > 0 && !script->isPlayerControlPaused()) {
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
				session.notePlayerActivity();
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
				if (session.isPlayerCombatLocked())
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
							_inventory.setPromptText(_inventory.buildSelectedPrompt(
								Common::String(), flow._menuTextConfig));
							needsRedraw = true;
							break;
						}

						if (!inventoryHover->object.objectName.equalsIgnoreCase(_inventory.getSelectedItemName())) {
							debugC(1, kDebugInventory,
								"Harvester: inventory left click using selected='%s' on target='%s'",
								_inventory.getSelectedItemName().c_str(),
								inventoryHover->object.objectName.c_str());
							Common::Error interactionError =
								roomItems.handleInventoryTargetInteraction(inventoryHover->object, interactionProcessor, needsRedraw);
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
					_mousePos, flow._menuTextConfig, &flow._dialogue);
				ObjectRecord *selectedRoomTarget = nullptr;
				if (_inventory.hasSelection() && !hoverState.npc)
					selectedRoomTarget = roomItems.findSelectedInventoryRoomTarget(_mousePos);
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
					if (roomItems.hasCarriedRoomItem()) {
					if (clickHoverState.playerEntity && playerState.entity &&
							clickHoverState.playerEntity == playerState.entity) {
						if (!roomItems.stowCarriedRoomItemToInventory())
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
							roomItems.handleInventoryTargetInteraction(*roomTarget, interactionProcessor, needsRedraw);
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
				// select one target in reverse render order; a selected object prevents exit-region selection.
				if (hoverState.region && !hoverState.object && playerState.entity) {
					if (isFastExitClick &&
						hoverState.region->startEnabled &&
						roomAllowsImmediateExitClick(scene.state.roomName)) {
						session.stopPlayerRegionInteraction();
						Common::Error interactionError =
							interactionProcessor.runRegionInteraction(*hoverState.region, _inventory, needsRedraw);
						if (interactionError.getCode() != Common::kNoError)
							return interactionError;
						if (flow.hasPendingMainMenuReturn())
							return Common::kNoError;
						needsRedraw = true;
						break;
					}
					interactionProcessor.queueRegionInteraction(*hoverState.region, _mousePos);
					Common::Error interactionError =
						interactionProcessor.tryActivatePendingRegion(_inventory, needsRedraw);
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
					// Native closeup exits return through room_setup so the parent on-enter command runs.
					if (canExitCloseupToParent)
						flow.requestCloseupParentRestart();
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
						beginInspectText(*clickedObject, resolvedInspectText);
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
					Common::Error pickupError = roomItems.handleRoomItemPickup(*clickedObject, playerState.entity != nullptr,
						interactionProcessor, interactionCallbacks, needsRedraw);
					if (pickupError.getCode() != Common::kNoError)
						return pickupError;
					break;
				}

				InteractionResult interaction;
				if (!_engine.getScript()->resolveObjectInteraction(
						*clickedObject, interaction, scene.state.roomName)) {
					if (canShowInspectText) {
						beginInspectText(*clickedObject, resolvedInspectText);
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

				session.notePlayerActivity();
				if (event.kbd.hasFlags(Common::KBD_CTRL))
					attackModifierHeld = true;
				if (idleState.active || idleState.exiting) {
					if (Player::requestIdleAnimationExit(scene.state, playerState, idleState))
						needsRedraw = true;
					break;
				}

				if (Script *script = _engine.getScript()) {
					InteractionResult cheatInteraction;
					CheatActivation cheatActivation;
					const CheatInputResult cheatInputResult = _cheats.processKey(
						event.kbd, *script, scene.state.roomName,
						cheatInteraction, cheatActivation);
					if (cheatInputResult != kCheatInputIgnored) {
						if (cheatInputResult == kCheatInputComplete && cheatActivation.activated) {
							if (cheatActivation.clearInteractionState && _inventory.clearSelection())
								needsRedraw = true;
							if (cheatActivation.inventoryChanged && _inventory.isOpen() &&
									!_inventory.refresh()) {
								return Common::kReadingFailed;
							}
							if (cheatActivation.hasInteraction) {
								if (_inventory.isOpen())
									(void)_inventory.close();
								bool didTransition = false;
								Common::Error cheatError = interactionProcessor.handleInteractionResult(
									cheatInteraction, didTransition, Common::String());
								if (cheatError.getCode() != Common::kNoError)
									return cheatError;
								if (flow.hasPendingMainMenuReturn())
									return Common::kNoError;
							}
							needsRedraw = true;
						}
						break;
					}
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
				if (session.isPlayerCombatLocked())
					break;

				if (event.kbd.keycode == Common::KEYCODE_LCTRL ||
						event.kbd.keycode == Common::KEYCODE_RCTRL) {
					attackModifierHeld = true;
				} else if (event.kbd.keycode == Common::KEYCODE_k && _engine.isCombatDebugEnabled()) {
					Common::Error debugKillError = combat.killDebugActiveMonster(_engine, scene.state, playerState, entityManager,
						_mousePos, combatEffects, interactionProcessor, needsRedraw);
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
					if (session.cancelPlayerAttackAnimation())
						needsRedraw = true;
					if (!roomItems.stowCarriedRoomItemToInventory())
						return Common::kReadingFailed;
					moveLeft = false;
					moveRight = false;
					moveUp = false;
					moveDown = false;
					pendingRegionName.clear();
					playerState.hasMoveTarget = false;
					playerState.turnActive = false;
					playerState.turnTargetFacing = -1;
					session.captureCurrentSaveState();
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
					session.resetIdleState();
					needsRedraw = true;
					break;
				}

				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					if (!roomItems.stowCarriedRoomItemToInventory())
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
			if (!roomItems.stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}

		Script *script = _engine.getScript();
		const bool playerAlive = script && script->getPlayerCurrentHitPoints() > 0;
		const bool playerControlPaused = script && script->isPlayerControlPaused();
		const bool playerCanAct = playerAlive && !playerControlPaused;
		if (!playerAlive && (moveLeft || moveRight || moveUp || moveDown ||
				playerState.hasMoveTarget || playerState.turnActive ||
				playerState.attackActive || playerState.hitActive)) {
			attackModifierHeld = false;
			session.stopPlayerRegionInteraction();
		}

		// Native run_harvester_main_loop checks player/region overlap before
		// update_actor_runtime_state. This is significant when a wide walk frame
		// reaches a narrow exit: the actor update may otherwise satisfy the X
		// target and replace that frame with a narrower idle frame first.
		Common::Error pendingRegionError =
			session.isPlayerCombatLocked()
				? Common::kNoError
				: interactionProcessor.tryActivatePendingRegion(_inventory, needsRedraw);
		if (pendingRegionError.getCode() != Common::kNoError)
			return pendingRegionError;
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!roomItems.stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (!session.isPlayerCombatLocked()) {
			pendingRegionError = interactionProcessor.tryActivatePassiveRegion(_inventory, needsRedraw);
			if (pendingRegionError.getCode() != Common::kNoError)
				return pendingRegionError;
			if (flow.hasPendingMainMenuReturn())
				return Common::kNoError;
			if (!pendingRoomChange.empty()) {
				if (!roomItems.stowCarriedRoomItemToInventory())
					return Common::kReadingFailed;
				break;
			}
		}

		// At 60 Hz, consume animation advances before the next screen update so
		// a walk frame and its movement step are presented together.
		if (flow.tickRuntimeEntities())
			needsRedraw = true;
		syncAnimatedRoomActorPlacement();

		Common::Error combatError = Common::kNoError;
		if (!playerControlPaused) {
			combatError = combat.resolvePlayerAttackContact(_engine, scene.state, playerState, entityManager,
					sceneRuntime, combatEffects, interactionProcessor, needsRedraw);
			if (combatError.getCode() != Common::kNoError)
				return combatError;
			if (flow.hasPendingMainMenuReturn())
				return Common::kNoError;
			if (!pendingRoomChange.empty()) {
				if (!roomItems.stowCarriedRoomItemToInventory())
					return Common::kReadingFailed;
				break;
			}
			if (Player::updateAttackAnimationState(_engine, playerState))
				needsRedraw = true;
			if (Player::updateHitAnimationState(
					_engine, scene.state, scene.sceneObjects, scene.sceneAnimations, playerState)) {
				needsRedraw = true;
			}
		}
		const bool keyboardAttackRequested =
			attackModifierHeld && (moveLeft || moveRight || moveUp || moveDown);
		if (playerCanAct &&
				!playerState.attackActive && !playerState.hitActive && keyboardAttackRequested &&
				!idleState.active && !idleState.exiting) {
			const uint32 now = Player::getRuntimeClockTicks();
			if (nextKeyboardAttackAllowedTick == 0 || (int32)(now - nextKeyboardAttackAllowedTick) >= 0) {
				combat.capturePlayerAttackTargetForKeyboard(scene.state, playerState, entityManager, moveLeft, moveRight, moveUp, moveDown);
				if (Player::startKeyboardAttackAnimation(
						scene.state, playerState, moveLeft, moveRight, moveUp, moveDown)) {
					nextKeyboardAttackAllowedTick = now + kNativeKeyboardAttackRepeatTicks;
					session.notePlayerActivity();
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
		if (playerCanAct && !playerState.attackActive && !playerState.hitActive &&
				Player::updateTurnAnimationState(playerState)) {
			needsRedraw = true;
		}

		if (playerCanAct &&
				!playerState.attackActive && !playerState.hitActive &&
				!keyboardAttackRequested && !idleState.active && !idleState.exiting) {
			const int regionFacing = interactionProcessor.getPendingRegionFacing();
			if (Player::stepKeyboardMovement(_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState, moveLeft, moveRight, moveUp, moveDown)) {
				session.notePlayerActivity();
				needsRedraw = true;
			} else if (Player::stepMoveTarget(
					_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState, regionFacing)) {
				session.notePlayerActivity();
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
		combatError = combat.updateRoomMonsterCombat(_engine, scene.state, playerState, idleState,
			entityManager, sceneRuntime, combatEffects, _cheats, interactionCallbacks, interactionProcessor, needsRedraw);
		if (combatError.getCode() != Common::kNoError)
			return combatError;
		if (combatEffects.syncCombatHitEffects())
			needsRedraw = true;
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!roomItems.stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		if (Player::updateDeathAnimationState(playerState)) {
			requestPlayerGameOver("combat_player_death_complete", Common::String());
			needsRedraw = true;
		}
		if (flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (combatEffects.pruneCombatHitEffects())
			needsRedraw = true;
		if (combatEffects.pruneCombatDamagePopups())
			needsRedraw = true;
		if (entityManager) {
			Common::Array<Common::String> expiredTimerNames;
			if (entityManager->takeExpiredTimerNames(expiredTimerNames)) {
				sceneRuntime.syncCurrentRoomRuntimeState(scene.sceneRegions);
				for (const Common::String &timerName : expiredTimerNames) {
					InteractionResult timerInteraction;
					if (!_engine.getScript()->executeTimerAction(
							timerName, timerInteraction, true, scene.state.roomName))
						continue;

					bool didTransition = false;
					Common::Error interactionError =
						interactionProcessor.handleInteractionResult(
							timerInteraction, didTransition, Common::String(), false);
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
		if (_engine.isCombatDebugEnabled() && combatEffects.hasDamagePopups())
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
