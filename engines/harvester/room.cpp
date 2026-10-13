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
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/art.h"
#include "harvester/flow.h"
#include "harvester/inventory.h"
#include "harvester/monster.h"
#include "harvester/player.h"
#include "harvester/room_combat.h"
#include "harvester/room_interaction.h"
#include "harvester/room_input.h"
#include "harvester/room_presentation.h"
#include "harvester/room_session.h"
#include "harvester/room_support.h"

namespace Harvester {

static const char *const kPlayerInventoryLabel = "your inventory";

static const int kNativeInventoryTooltipX = 0xbc;
static const int kNativeInventoryTooltipY = 0x19e;
static const int kNativeInventoryWeekdayX = 0x1e0;
static const int kNativeInventoryWeekdayY = 0x18c;
static const byte kNativeInventoryTooltipColor = 0xf4;

static const int kNativeCombatResourceIconsX = 10;
static const int kNativeCombatResourceIconsY = 30;
static const int kNativeCombatResourceIconSpacing = 15;

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
		RoomPresentation::blitBitmap(screen, icon, kNativeCombatResourceIconsX + i * kNativeCombatResourceIconSpacing,
			kNativeCombatResourceIconsY);
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
		RoomPresentationState presentationState;

		RoomInputState &inputState = session.getInputState();

		Common::String &pendingRegionName = session.getPendingRegionName();
		Common::String pendingRoomChange;
		bool pendingRoomChangeIsRoomName = false;
		bool pendingRoomChangeUsesSavedRoomState = false;
		SaveRoomState pendingRoomChangeSavedRoomState;
		const bool canExitCloseupToParent = flow._roomLoopDepth > 1;

		RoomIdleAnimationState &idleState = session.getIdleState();
		bool needsRedraw = true;
		RoomItemInteractions roomItems(_engine, scene, sceneRuntime, _inventory, pendingRoomChange);

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

		RoomPresentation presentation(_engine, scene, art, inspectFont, useNativeInspectFont);

		auto runRoomExitCommands = [&]() -> Common::Error {
			InteractionResult exitInteraction;
			if (!_engine.getScript()->executeRoomExitCommands(scene.state.roomName, exitInteraction))
				return Common::kReadingFailed;

			for (uint exitStep = 0; exitStep < 128; ++exitStep) {
				if (exitInteraction.cdChangeDisc > 0) {
					Common::Error cdPromptError = flow.showCdChangePrompt(exitInteraction.cdChangeDisc);
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
		typedef decltype(runRoomExitCommands) RunRoomExitCommandsFn;
		typedef decltype(applyPlayerGotoXZ) ApplyPlayerGotoXZFn;
		typedef decltype(startPlayerDefeatSequence) StartPlayerDefeatSequenceFn;

		struct RoomInteractionCallbacksImpl : RoomInteractionCallbacks {
			RoomInteractionCallbacksImpl(
					RoomSession &session_, RoomPresentation &presentation_, Flow &flow_,
					RunRoomExitCommandsFn &runRoomExitCommandsFn_,
					ApplyPlayerGotoXZFn &applyPlayerGotoXZFn_,
					StartPlayerDefeatSequenceFn &startPlayerDefeatSequenceFn_)
				: session(session_), presentation(presentation_), flow(flow_),
				  runRoomExitCommandsFn(runRoomExitCommandsFn_),
				  applyPlayerGotoXZFn(applyPlayerGotoXZFn_),
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
				return presentation.captureDialogueBackdrop(dialogueBackdrop);
			}

			Common::Error showCdChangePrompt(int discNumber) override {
				return flow.showCdChangePrompt(discNumber);
			}

			Common::Error runRoomExitCommands() override {
				return runRoomExitCommandsFn();
			}

			Common::Error applyLightingCommand(StartupLightingCommand lightingCommand) override {
				return presentation.applyLightingCommand(lightingCommand);
			}

			void applyPlayerGotoXZ(int x, int z) override {
				applyPlayerGotoXZFn(x, z);
			}

			Common::Error runModalShowText(const ResolvedText &modalText) override {
				return presentation.runModalShowText(modalText, flow);
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
			RoomPresentation &presentation;
			Flow &flow;
			RunRoomExitCommandsFn &runRoomExitCommandsFn;
			ApplyPlayerGotoXZFn &applyPlayerGotoXZFn;
			StartPlayerDefeatSequenceFn &startPlayerDefeatSequenceFn;
		};
		RoomInteractionCallbacksImpl interactionCallbacks(
			session, presentation, flow, runRoomExitCommands, applyPlayerGotoXZ,
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

	RoomInput input(_engine, flow, session, scene, entityManager, _inventory, _cheats,
		roomItems, interactionProcessor, interactionCallbacks, presentationState, *art, canExitCloseupToParent);

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
		if (!presentationState.combatLoadoutStatusText.value.empty() &&
				presentationState.combatLoadoutStatusHideTick != 0 &&
				(int32)(Player::getRuntimeClockTicks() - presentationState.combatLoadoutStatusHideTick) >= 0) {
			presentationState.combatLoadoutStatusText = ResolvedText();
			presentationState.combatLoadoutStatusHideTick = 0;
			if (presentationState.closeInventoryAfterCombatLoadoutStatus) {
				presentationState.closeInventoryAfterCombatLoadoutStatus = false;
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
			const bool suppressHover = presentationState.showingInspectText || idleState.active || idleState.exiting ||
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
			if (!presentationState.showingInspectText)
				presentationState.currentInteractionPromptText = promptText;
			if (Entity *cursor = entityManager ? entityManager->getCursorEntity() : nullptr) {
				cursor->setAnimationSequence(
					(presentationState.showingInspectText || idleState.active || idleState.exiting ||
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
				RoomPresentation::blitBitmap(*activeScreen, roomItems.getCarriedRoomItemBitmap(), drawX, drawY);
			}

			if (presentationState.showingInspectText) {
				drawRoomInspectText(*activeScreen, *art, *inspectFont, presentationState.inspectText, useNativeInspectFont);
				if (!presentationState.currentInteractionPromptText.empty())
					drawRoomPrompt(*activeScreen, *promptFont,
						presentationState.currentInteractionPromptText, useNativePromptFont);
			} else if (!presentationState.combatLoadoutStatusText.value.empty()) {
				drawRoomInspectText(*activeScreen, *art, *inspectFont, presentationState.combatLoadoutStatusText,
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
			const RoomInputResult inputResult = input.handleEvent(event, needsRedraw);
			if (inputResult.returnFromRoom)
				return inputResult.error;
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
		if (!playerAlive && (inputState.moveLeft || inputState.moveRight || inputState.moveUp || inputState.moveDown ||
				playerState.hasMoveTarget || playerState.turnActive ||
				playerState.attackActive || playerState.hitActive)) {
			inputState.attackModifierHeld = false;
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
			inputState.attackModifierHeld && (inputState.moveLeft || inputState.moveRight || inputState.moveUp || inputState.moveDown);
		if (playerCanAct &&
				!playerState.attackActive && !playerState.hitActive && keyboardAttackRequested &&
				!idleState.active && !idleState.exiting) {
			const uint32 now = Player::getRuntimeClockTicks();
			if (inputState.nextKeyboardAttackAllowedTick == 0 || (int32)(now - inputState.nextKeyboardAttackAllowedTick) >= 0) {
				combat.capturePlayerAttackTargetForKeyboard(scene.state, playerState, entityManager, inputState.moveLeft, inputState.moveRight, inputState.moveUp, inputState.moveDown);
				if (Player::startKeyboardAttackAnimation(
						scene.state, playerState, inputState.moveLeft, inputState.moveRight, inputState.moveUp, inputState.moveDown)) {
					inputState.nextKeyboardAttackAllowedTick = now + kNativeKeyboardAttackRepeatTicks;
					session.notePlayerActivity();
					debugC(1, kDebugCombat,
						"Harvester: combat player keyboard attack start loadout=%d weapon='%s' damage=%d damage_type='%s' target_class='%s' target='%s' input=(L=%d R=%d U=%d D=%d) facing=%d",
						playerState.combatLoadout,
						Player::describeCombatLoadout(playerState.combatLoadout),
						Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout),
						Player::describeCombatDamageType(Player::resolveCombatLoadoutDamageType(playerState.combatLoadout)),
						describeCombatTargetClass(playerState.attackTargetClassId),
						playerState.attackTargetName.c_str(),
						inputState.moveLeft, inputState.moveRight, inputState.moveUp, inputState.moveDown, playerState.facing);
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
					playerState, inputState.moveLeft, inputState.moveRight, inputState.moveUp, inputState.moveDown)) {
				session.notePlayerActivity();
				needsRedraw = true;
			} else if (Player::stepMoveTarget(
					_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState, regionFacing)) {
				session.notePlayerActivity();
				needsRedraw = true;
			}

			if (!presentationState.showingInspectText && !keyboardAttackRequested &&
					!inputState.moveLeft && !inputState.moveRight && !inputState.moveUp && !inputState.moveDown &&
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
