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

#include "harvester/startup_room.h"

#include "common/events.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/runtime_entity.h"
#include "harvester/startup_art.h"
#include "harvester/startup_flow.h"
#include "harvester/startup_inventory.h"
#include "harvester/startup_room_support.h"

namespace Harvester {

StartupRoomSystem::StartupRoomSystem(HarvesterEngine &engine, Common::Point &mousePos,
		StartupInventorySystem &inventory)
	: _engine(engine), _mousePos(mousePos), _inventory(inventory) {
}

Common::Error StartupRoomSystem::runRoomLoop(StartupFlow &startupFlow, const Common::String &entranceName) {
	if (startupFlow.hasPendingMainMenuReturn())
		return Common::kNoError;

	StartupRoomSetupState state;
	if (!_engine.getStartupScript()->resolveRoomSetupState(entranceName, state, *_engine.getResources()))
		return Common::kReadingFailed;

	Common::Error transitionError = startupFlow.beginRoomSetupTransition();
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

	if (!startupFlow.populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
		return Common::kReadingFailed;

	logScenePaletteSummary("room setup stub palette", scene, 0.0f);
	drawRoomScene(_engine, *screen, scene, 0.0f);
	screen->makeAllDirty();
	screen->update();

	logScenePaletteSummary("room setup fade target", scene, scene.targetPaletteBrightness);
	transitionError = startupFlow.fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	startupFlow.resetCursorAnimationSequence();
	startupFlow.executeStartupAudioCommands(scene.state.audioCommands);
	if (!scene.state.musicPath.empty())
		(void)_engine.playStartupMusic(scene.state.musicPath);
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
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
	StartupRoomIdleAnimationState idleState;
	bool needsRedraw = true;
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
	resetIdleState();

	auto openInventoryOverlay = [&]() {
		moveLeft = false;
		moveRight = false;
		moveUp = false;
		moveDown = false;
		pendingRegionName.clear();
		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		return _inventory.open();
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
		if (!startupFlow.populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return false;

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
		return _inventory.refresh();
	};
	auto captureDialogueBackdrop = [&](IndexedBitmap &dialogueBackdrop) {
		drawRoomScene(_engine, *screen, scene, scene.targetPaletteBrightness);
		return captureScreenBackdrop(*screen, dialogueBackdrop);
	};
	auto runRoomExitCommands = [&]() {
		Common::Array<StartupAudioCommand> exitAudioCommands;
		if (!_engine.getStartupScript()->executeRoomExitCommands(scene.state.roomName, exitAudioCommands))
			return false;
		startupFlow.executeStartupAudioCommands(exitAudioCommands);
		return true;
	};
	struct InteractionProcessor {
		HarvesterEngine &engine;
		StartupFlow &startupFlow;
		StartupRoomSceneResources &scene;
		StartupRoomPlayerState &playerState;
		Common::String &pendingRegionName;
		decltype(refreshCurrentScene) &refreshCurrentSceneFn;
		decltype(captureDialogueBackdrop) &captureDialogueBackdropFn;
		decltype(runRoomExitCommands) &runRoomExitCommandsFn;
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

			if (!interaction.nextRoomName.empty()) {
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

			if (!interaction.dialogueNpcName.empty()) {
				Common::Error dialogueError = runScriptedDialogue(
					interaction.dialogueNpcName, Common::String(), interaction.dialogueContinuationTag,
					didTransition);
				if (dialogueError.getCode() != Common::kNoError)
					return dialogueError;
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
		_engine, startupFlow, scene, playerState, pendingRegionName, refreshCurrentScene,
		captureDialogueBackdrop, runRoomExitCommands, resetIdleState
	};
	auto handleInventoryTargetInteraction = [&](const StartupObjectRecord &target, bool preferPickup) -> Common::Error {
		if (!_inventory.hasSelection())
			return Common::kNoError;

		StartupInteractionResult interaction;
		bool handled = false;
		if (preferPickup && _engine.getStartupScript()->isPickupObject(target))
			handled = _engine.getStartupScript()->resolveObjectInteraction(target, interaction);
		else
			handled = _engine.getStartupScript()->resolveUseItemInteraction(
				_inventory.getSelectedItemName(), target, interaction);
		if (!handled)
			return Common::kNoError;

		_inventory.clearSelection();
		bool didTransition = false;
		Common::Error interactionError = interactionProcessor.handleInteractionResult(interaction, didTransition);
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
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

		pendingRegionName.clear();
		return runRegionInteraction(*region);
	};
	if (!_inventory.refresh())
		return Common::kReadingFailed;
	Graphics::FrameLimiter limiter(g_system, 60);

	if (shouldRunStartupRoomProbe())
		logStartupRoomProbe(_engine, scene, entranceName, _mousePos);

	while (!_engine.shouldQuit()) {
		if (startupFlow.hasPendingMainMenuReturn())
			return Common::kNoError;

		if (needsRedraw) {
			const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
			const bool inventoryPanelContainsMouse = _inventory.isOpen() && inventoryPanelBounds.contains(_mousePos);
			const bool suppressHover = showingInspectText || idleState.active || idleState.exiting ||
				(_inventory.isOpen() && (inventoryPanelContainsMouse || !_inventory.hasSelection()));
			StartupRoomHoverState hoverState = suppressHover
				? StartupRoomHoverState()
				: resolveRoomHoverState(_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, _mousePos);
			Common::String promptText;
			if (_inventory.isOpen()) {
				const StartupInventoryVisual *inventoryHover = _inventory.findItemAtPoint(_mousePos);
				if (_inventory.hasSelection()) {
					Common::String targetLabel;
					if (inventoryHover && !StartupInventorySystem::isExitObject(inventoryHover->object) &&
						!StartupInventorySystem::isStatusObject(inventoryHover->object)) {
						targetLabel = _engine.getStartupScript()->resolveObjectLabel(inventoryHover->object);
					} else if (!inventoryPanelContainsMouse && hoverState.object) {
						targetLabel = _engine.getStartupScript()->resolveObjectLabel(*hoverState.object);
					}
					promptText = _inventory.buildSelectedPrompt(targetLabel);
				} else if (inventoryHover) {
					promptText = _engine.getStartupScript()->resolveObjectLabel(inventoryHover->object);
				}
				_inventory.setPromptText(promptText);
				hoverState.cursorSequence = 7;
			} else {
				promptText = hoverState.promptText;
			}
			if (RuntimeEntity *cursor = runtimeEntities ? runtimeEntities->getCursorEntity() : nullptr) {
				cursor->setAnimationSequence(
					(showingInspectText || idleState.active || idleState.exiting || _inventory.isOpen())
						? 7
						: hoverState.cursorSequence);
			}

			drawRoomScene(_engine, *screen, scene, scene.targetPaletteBrightness);
			if (_inventory.isOpen())
				_inventory.drawOverlay(*screen, *bodyFont);

			if (showingInspectText) {
				drawRoomInspectText(*screen, *art, *bodyFont, inspectText);
			} else if (!promptText.empty()) {
				_engine.getScreen();
				// Shared room prompt rendering remains in the orchestration layer.
				{
					const byte shadowColor = 0;
					bodyFont->drawString(screen, promptText, 1, 463, 640, shadowColor, Graphics::kTextAlignCenter);
					bodyFont->drawString(screen, promptText, 0, 462, 640, 0xce, Graphics::kTextAlignCenter);
				}
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
					if (_inventory.clearSelection() || _inventory.close())
						needsRedraw = true;
					break;
				}
				break;
			case Common::EVENT_LBUTTONUP:
				if (showingInspectText)
					inspectCanDismiss = true;
				break;
			case Common::EVENT_LBUTTONDOWN: {
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
						if (StartupInventorySystem::isExitObject(inventoryHover->object)) {
							if (_inventory.close())
								needsRedraw = true;
							break;
						}

						if (!_inventory.hasSelection()) {
							_inventory.selectItem(inventoryHover->object.objectName);
							_inventory.setPromptText(_inventory.buildSelectedPrompt(Common::String()));
							needsRedraw = true;
							break;
						}

						if (!inventoryHover->object.objectName.equalsIgnoreCase(_inventory.getSelectedItemName())) {
							Common::Error interactionError =
								handleInventoryTargetInteraction(inventoryHover->object, false);
							if (interactionError.getCode() != Common::kNoError)
								return interactionError;
						}
						needsRedraw = true;
						break;
					}

					if (_inventory.hasSelection()) {
						const StartupRoomHoverState useHoverState = resolveRoomHoverState(
							_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
							scene.sceneRegions, _mousePos);
						if (useHoverState.npc) {
							bool didTransition = false;
							Common::Error dialogueError = interactionProcessor.runScriptedDialogue(
								useHoverState.npc->npcName, _inventory.getSelectedItemName(),
								Common::String(), didTransition);
							if (dialogueError.getCode() != Common::kNoError)
								return dialogueError;
							if (startupFlow.hasPendingMainMenuReturn())
								return Common::kNoError;
							if (_inventory.clearSelection() || _inventory.close())
								needsRedraw = true;
							needsRedraw = true;
							break;
						}
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

					if (!inventoryPanelBounds.contains(_mousePos) && _inventory.hasSelection()) {
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
						if (_inventory.clearSelection() || _inventory.close())
							needsRedraw = true;
					} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
							event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
							event.kbd.keycode == Common::KEYCODE_i) {
						if (_inventory.close())
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
					Common::Error menuError = startupFlow.runRoomMenuStub(roomMenuBackdrop);
					if (menuError.getCode() != Common::kNoError)
						return menuError;
					startupFlow.resetCursorAnimationSequence();
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
				notePlayerActivity();
				needsRedraw = true;
			} else if (stepPlayerMoveTarget(_engine, scene.state, scene.sceneObjects, scene.sceneAnimations,
					playerState)) {
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

		if (startupFlow.tickRuntimeEntities())
			needsRedraw = true;
		if (updatePlayerIdleAnimation(scene.state, playerState, idleState))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

} // End of namespace Harvester
