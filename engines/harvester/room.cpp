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
#include "harvester/room_support.h"

namespace Harvester {

static const uint32 kRoomExitFastClickWindowTicks = 20;
static const char *const kPlayerInventoryLabel = "your inventory";
static const char *const kInventoryOwnerName = "INVENTORY";
static const byte kTransparentPaletteIndex = 0;
static const int kRoomMonsterAnimationRate = 17;

struct MonsterAnimationRange {
	MonsterAnimationRange() {}
	MonsterAnimationRange(int walkFirstFrame, int walkLastFrame, int idleFrame)
		: walkFirstFrame(walkFirstFrame), walkLastFrame(walkLastFrame), idleFrame(idleFrame) {}

	int walkFirstFrame = 0;
	int walkLastFrame = 0;
	int idleFrame = 0;
};

static bool roomAllowsImmediateExitClick(const Common::String &roomName) {
	return !roomName.equalsIgnoreCase("LAVAPIT") &&
		!roomName.equalsIgnoreCase("RMNBATH") &&
		!roomName.equalsIgnoreCase("BOWLSNTRY1");
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

static void setScaledRoomPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getStartupGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
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

static MonsterAnimationRange resolveRoomMonsterAnimationRange(int facing) {
	switch (facing) {
	case 0:
		return MonsterAnimationRange(0x00, 0x09, 0x3b);
	case 1:
		return MonsterAnimationRange(0x0f, 0x18, 0x0e);
	case 2:
		return MonsterAnimationRange(0x2d, 0x36, 0x2c);
	case 3:
		return MonsterAnimationRange(0x1e, 0x27, 0x28);
	default:
		return MonsterAnimationRange(0x00, 0x09, 0x3b);
	}
}

static void applyRoomMonsterAnimation(RuntimeEntity &entity, const StartupMonsterRecord &monster) {
	entity.setVisible(monster.visible);
	if (!monster.visible)
		return;

	const MonsterAnimationRange range = resolveRoomMonsterAnimationRange(monster.facing);
	if (monster.active) {
		entity.setAnimationRate(kRoomMonsterAnimationRate);
		entity.setAnimationFrameRange(range.walkFirstFrame, range.walkLastFrame, true);
		entity.setAnimationEnabled(true);
	} else {
		entity.setAnimationRate(0);
		entity.setAnimationFrameRange(range.idleFrame, range.idleFrame, false);
		entity.setCurrentFrame(range.idleFrame);
		entity.setAnimationEnabled(false);
	}
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
			const StartupSaveRoomState &loadedState = _engine.getPendingLoadedStartupSaveRoomState();
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
		Common::ScopedPtr<HarvesterCftFont> promptCftFont;
		const Graphics::Font *promptFont = bodyFont;
		bool useNativePromptFont = false;
		if (promptFontResource) {
			promptCftFont.reset(new HarvesterCftFont(*promptFontResource));
			if (promptCftFont->isValid()) {
				promptFont = promptCftFont.get();
				useNativePromptFont = true;
			}
		}
		if (!screen || !art || !bodyFont || !promptFont)
			return Common::kNoError;
		RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();

		if (!startupFlow.populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return Common::kReadingFailed;
		if (runtimeEntities) {
			for (const StartupMonsterRecord &monster : scene.state.roomMonsters) {
				RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(monster.monsterName);
				if (entity)
					applyRoomMonsterAnimation(*entity, monster);
			}
			runtimeEntities->pauseTimerCountdowns();
		}

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
		Common::String pendingRoomChange;
		uint32 lastLeftButtonReleaseTick = 0;
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
		auto removeSceneObjectEntity = [&](const Common::String &objectName) {
			if (!runtimeEntities)
				return;

			RuntimeEntity *entity = runtimeEntities->detachSceneEntityByName(objectName);
			delete entity;
		};
		auto resetIdleState = [&]() {
			idleState = StartupRoomIdleAnimationState();
			idleState.activityTick = getRuntimeClockTicks();
			idleState.resetTick = idleState.activityTick;
			updatePlayerIdleTrigger(idleState);
		};
		auto notePlayerActivity = [&]() {
			idleState.activityTick = getRuntimeClockTicks();
			updatePlayerIdleTrigger(idleState);
		};
		auto syncCurrentRoomRuntimeState = [&]() {
			Script *startupScript = _engine.getStartupScript();
			if (!startupScript)
				return;

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
		resetIdleState();
		auto stopPlayerRegionInteraction = [&]() {
			moveLeft = false;
			moveRight = false;
			moveUp = false;
			moveDown = false;
			pendingRegionName.clear();
			playerState.hasMoveTarget = false;
			playerState.turnActive = false;
			playerState.turnTargetFacing = -1;
			if (playerState.entity && playerState.facing >= 0)
				(void)setPlayerIdleAnimation(playerState, playerState.facing);
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
		if (!startupFlow.populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return false;
		if (runtimeEntities) {
			for (const StartupMonsterRecord &monster : scene.state.roomMonsters) {
				RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(monster.monsterName);
				if (entity)
					applyRoomMonsterAnimation(*entity, monster);
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
			(void)setPlayerIdleAnimation(playerState, facing);
			(void)applyRoomActorPlacement(scene.state, *playerState.entity,
				playerState.centerX, playerState.bottomY, playerState.z);
		}

		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
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
	auto openInventoryOverlay = [&]() {
		moveLeft = false;
		moveRight = false;
		moveUp = false;
		moveDown = false;
		pendingRegionName.clear();
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
		auto runRoomExitCommands = [&]() {
			Common::Array<StartupAudioCommand> exitAudioCommands;
			if (!_engine.getStartupScript()->executeRoomExitCommands(scene.state.roomName, exitAudioCommands))
				return false;
			startupFlow.executeStartupAudioCommands(exitAudioCommands);
			return true;
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
			playerState.hasMoveTarget = false;
			playerState.turnActive = false;
			playerState.turnTargetFacing = -1;
			playerState.centerX = x;
			playerState.z = clampRoomDepthForEvent(scene.state, (float)z);
			playerState.bottomY = mapRoomDepthToScreenYForEvent(scene.state, playerState.z, playerState.bottomY);
			if (playerState.entity) {
				const int facing = playerState.facing >= 0 ? playerState.facing : scene.state.playerFacing;
				if (facing >= 0)
					(void)setPlayerIdleAnimation(playerState, facing);
				(void)applyRoomActorPlacement(scene.state, *playerState.entity,
					playerState.centerX, playerState.bottomY, playerState.z);
			}
			resetIdleState();
			captureCurrentSaveState();
		};
		auto runModalShowText = [&](const StartupResolvedText &modalText) -> Common::Error {
			Graphics::Screen *activeScreen = getActiveScreen();
			if (!activeScreen || !art || !bodyFont)
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
				drawRoomInspectText(*activeScreen, *art, *bodyFont, modalText);
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
			decltype(captureDialogueBackdrop) &captureDialogueBackdropFn;
			decltype(runRoomExitCommands) &runRoomExitCommandsFn;
			decltype(applyLightingCommand) &applyLightingCommandFn;
			decltype(applyPlayerGotoXZ) &applyPlayerGotoXZFn;
			decltype(runModalShowText) &runModalShowTextFn;
			decltype(resetIdleState) &resetIdleStateFn;

			Common::Error handleInteractionResult(const StartupInteractionResult &interaction,
					bool &didTransition) {
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

					if (!runRoomExitCommandsFn())
						return Common::kReadingFailed;

					// Native CHANGE_ROOM queues a room handoff for the live loop instead of nesting.
					pendingRoomChange = resolvedTransitionTarget;
					didTransition = true;
				} else if (!interaction.nextRoomName.empty()) {
					if (!runRoomExitCommandsFn())
						return Common::kReadingFailed;

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
					if (!refreshCurrentSceneFn(true))
						return Common::kReadingFailed;
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
						interaction.dialogueNpcName, Common::String(), interaction.dialogueContinuationTag,
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
							handleInteractionResult(continuationInteraction, didTransition);
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
					Common::Error interactionError = handleInteractionResult(dialogueInteraction, didTransition);
					if (interactionError.getCode() != Common::kNoError)
						return interactionError;
					if (startupFlow.hasPendingMainMenuReturn())
						return Common::kNoError;
				}
				if (!didTransition && !abortRemainingCommandChain && !continuationTag.empty()) {
					StartupInteractionResult continuationInteraction;
					if (engine.getStartupScript()->executeActionTag(continuationTag, continuationInteraction)) {
						Common::Error interactionError =
							handleInteractionResult(continuationInteraction, didTransition);
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
			refreshCurrentScene, captureDialogueBackdrop, runRoomExitCommands,
			applyLightingCommand, applyPlayerGotoXZ, runModalShowText, resetIdleState
		};
	auto moveRoomItemDirectlyToInventory = [&](const StartupObjectRecord &object) -> Common::Error {
		Script *startupScript = _engine.getStartupScript();
		if (!startupScript)
			return Common::kReadingFailed;

		startupScript->addRuntimeObjectToInventory(object.objectName);
		const Common::String inventoryOwner(kInventoryOwnerName);
		hideSceneObject(object.objectName, &inventoryOwner);
		removeSceneObjectEntity(object.objectName);

		bool didTransition = false;
		if (shouldDispatchPickupActionOnDirectInventoryTransfer(object)) {
			StartupInteractionResult pickupInteraction;
			if (startupScript->executeActionTag(object.actionTag, pickupInteraction)) {
				Common::Error interactionError =
					interactionProcessor.handleInteractionResult(pickupInteraction, didTransition);
				if (interactionError.getCode() != Common::kNoError)
					return interactionError;
				if (startupFlow.hasPendingMainMenuReturn())
					return Common::kNoError;
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

		clearCarriedRoomItem();
		carriedRoomItemName = object.objectName;
		carriedRoomItemLabel = startupScript->resolveObjectLabel(object);
		const Common::String spritePath = resolveCarriedObjectSpritePath(object);
		if (!spritePath.empty())
			loadBitmapResource(*resources, spritePath, carriedRoomItemBitmap);

		startupScript->setRuntimeObjectVisible(object.currentOwnerOrRoom, object.objectName, false);
		hideSceneObject(object.objectName, nullptr);
		removeSceneObjectEntity(object.objectName);

		bool didTransition = false;
		if (shouldDispatchPickupActionOnCarryStart(object)) {
			StartupInteractionResult pickupInteraction;
			if (startupScript->executeActionTag(object.actionTag, pickupInteraction)) {
				Common::Error interactionError =
					interactionProcessor.handleInteractionResult(pickupInteraction, didTransition);
				if (interactionError.getCode() != Common::kNoError)
					return interactionError;
				if (startupFlow.hasPendingMainMenuReturn())
					return Common::kNoError;
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

		StartupInteractionResult interaction;
		const bool handled = _engine.getStartupScript()->resolveUseItemInteraction(
			_inventory.getSelectedItemName(), target, interaction);
		if (!handled)
			return Common::kNoError;

		bool didTransition = false;
		Common::Error interactionError = interactionProcessor.handleInteractionResult(interaction, didTransition);
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

		setPlayerMoveTarget(scene.state, playerState,
			resolveRegionTargetX(region, playerState), resolveRegionTargetZ(region));
	};
	auto runRegionInteraction = [&](const StartupRegionRecord &region) -> Common::Error {
		StartupInteractionResult interaction;
		if (!_engine.getStartupScript()->resolveRegionInteraction(region, interaction))
			return Common::kNoError;

		bool didTransition = false;
		Common::Error interactionError =
			interactionProcessor.handleInteractionResult(interaction, didTransition);
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
				(void)startPlayerTurnAnimation(playerState, region->desiredFacing);
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
				(_inventory.isOpen() && (inventoryPanelContainsMouse || !inventorySelectionActive));
			StartupRoomHoverState hoverState = suppressHover
				? StartupRoomHoverState()
				: resolveRoomHoverState(_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, _mousePos);
			Common::String promptText;
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
						!InventorySystem::isStatusObject(inventoryHover->object)) {
					promptText = _engine.getStartupScript()->resolveObjectLabel(inventoryHover->object);
				}
				_inventory.setPromptText(promptText);
				hoverState.cursorSequence = 7;
			} else {
				promptText = hoverState.promptText;
			}
			if (RuntimeEntity *cursor = runtimeEntities ? runtimeEntities->getCursorEntity() : nullptr) {
				cursor->setAnimationSequence(
					(showingInspectText || idleState.active || idleState.exiting ||
						_inventory.isOpen() || activeCarry)
						? 7
						: hoverState.cursorSequence);
			}

			drawRoomScene(_engine, *activeScreen, scene, scene.targetPaletteBrightness);
			if (_inventory.isOpen())
				_inventory.drawOverlay(*activeScreen);
			if (inventorySelectionActive) {
				_inventory.drawSelectedDragItem(*activeScreen, _mousePos);
			} else if (roomCarryActive && carriedRoomItemBitmap.isValid()) {
				const int drawX = _mousePos.x - (int)carriedRoomItemBitmap.width / 2;
				const int drawY = _mousePos.y - (int)carriedRoomItemBitmap.height / 2;
				blitBitmap(*activeScreen, carriedRoomItemBitmap, drawX, drawY);
			}

			if (showingInspectText) {
				drawRoomInspectText(*activeScreen, *art, *bodyFont, inspectText);
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
				needsRedraw = true;
				break;
			case Common::EVENT_RBUTTONDOWN:
				notePlayerActivity();
				if (idleState.active || idleState.exiting) {
					if (requestPlayerIdleAnimationExit(scene.state, playerState, idleState))
						needsRedraw = true;
					break;
				}
				pendingRegionName.clear();
				if (_inventory.isOpen()) {
					const bool clearedSelection = _inventory.clearSelection();
					const bool closedInventory = _inventory.close();
					if (clearedSelection || closedInventory)
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
				}
				break;
			case Common::EVENT_LBUTTONUP:
				lastLeftButtonReleaseTick = getRuntimeClockTicks();
				if (showingInspectText)
					inspectCanDismiss = true;
				break;
			case Common::EVENT_LBUTTONDOWN: {
				const uint32 now = getRuntimeClockTicks();
				const bool isFastExitClick =
					lastLeftButtonReleaseTick != 0 &&
					now - lastLeftButtonReleaseTick < kRoomExitFastClickWindowTicks;
				notePlayerActivity();
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

				const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
				if (_inventory.isOpen()) {
					const StartupInventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
					if (inventoryHover) {
						if (InventorySystem::isExitObject(inventoryHover->object)) {
							const bool clearedSelection = _inventory.clearSelection();
							const bool closedInventory = _inventory.close();
							if (clearedSelection || closedInventory)
								needsRedraw = true;
							break;
						}
						if (InventorySystem::isStatusObject(inventoryHover->object))
							break;

						if (!_inventory.hasSelection()) {
							_inventory.selectItem(inventoryHover->object.objectName);
							_inventory.setPromptText(_inventory.buildSelectedPrompt(Common::String()));
							needsRedraw = true;
							break;
						}

						if (!inventoryHover->object.objectName.equalsIgnoreCase(_inventory.getSelectedItemName())) {
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
					if (hoverState.npc) {
						bool didTransition = false;
						Common::Error dialogueError = interactionProcessor.runScriptedDialogue(
							hoverState.npc->npcName, _inventory.getSelectedItemName(),
							Common::String(), didTransition);
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
					interactionProcessor.handleInteractionResult(interaction, didTransition);
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
				if (idleState.active || idleState.exiting) {
					if (requestPlayerIdleAnimationExit(scene.state, playerState, idleState))
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
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}

		bool playerAdvancedThisFrame = false;
		if (updatePlayerTurnAnimationState(playerState)) {
			playerAdvancedThisFrame = true;
			needsRedraw = true;
		}

		if (!idleState.active && !idleState.exiting) {
			if (stepPlayerKeyboardMovement(_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState, moveLeft, moveRight, moveUp, moveDown)) {
				playerAdvancedThisFrame = true;
				notePlayerActivity();
				needsRedraw = true;
			} else if (stepPlayerMoveTarget(_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState)) {
				playerAdvancedThisFrame = true;
				notePlayerActivity();
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
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (!pendingRoomChange.empty()) {
			if (!stowCarriedRoomItemToInventory())
				return Common::kReadingFailed;
			break;
		}
		pendingRegionError = playerAdvancedThisFrame ? tryActivateOverlappedRegion() : Common::kNoError;
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
		if (runtimeEntities) {
			Common::Array<Common::String> expiredTimerNames;
			if (runtimeEntities->takeExpiredTimerNames(expiredTimerNames)) {
				syncCurrentRoomRuntimeState();
				for (const Common::String &timerName : expiredTimerNames) {
					StartupInteractionResult timerInteraction;
					if (!_engine.getStartupScript()->executeActionTag(timerName, timerInteraction))
						continue;

					bool didTransition = false;
					Common::Error interactionError =
						interactionProcessor.handleInteractionResult(timerInteraction, didTransition);
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
		if (updatePlayerIdleAnimation(scene.state, playerState, idleState))
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
