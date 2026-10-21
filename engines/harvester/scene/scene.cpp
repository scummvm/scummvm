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

#include "harvester/scene/scene.h"

#include "backends/keymapper/keymapper.h"
#include "common/algorithm.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/media/media_manager.h"
#include "harvester/player/player.h"
#include "harvester/combat/combat.h"
#include "harvester/entity.h"

#include "common/ptr.h"
#include "graphics/blit.h"
#include "graphics/fontman.h"
#include "harvester/cft_font.h"
#include "harvester/cheats.h"
#include "harvester/resources.h"
#include "harvester/monster.h"
#include "harvester/scene/interaction.h"
#include "harvester/scene/input.h"
#include "harvester/scene/room_presentation.h"
#include "harvester/scene/room_support.h"
#include "harvester/scene/scene_entities.h"

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
		blitTransparentBitmap(screen, icon, kNativeCombatResourceIconsX + i * kNativeCombatResourceIconSpacing,
			kNativeCombatResourceIconsY);
}

Scene::Scene(HarvesterEngine &engine, Flow &flow, SceneState &scene, EntityManager *entityManager,
		InventorySystem &inventory, CheatSystem &cheats, const RoomViewResources &view, bool canExitCloseupToParent)
	: _engine(engine), _flow(flow), _scene(scene), _entityManager(entityManager), _inventory(inventory),
	  _sceneRuntime(scene.state, engine.getScript(), engine.getResources(), entityManager), _combatEffects(entityManager),
	  _cheats(cheats), _view(view), _canExitCloseupToParent(canExitCloseupToParent),
	  _roomItems(engine, scene, _sceneRuntime, inventory, _pendingTransition.roomName),
	  _presentation(engine, scene, view.art, view.inspectFont, view.useNativeInspectFont),
	  _interactionProcessor(engine, flow, scene, _player, _pendingRegionName, _pendingTransition.roomName,
		_pendingTransition.isRoomName, _pendingTransition.usesSavedRoomState, _pendingTransition.savedRoomState,
		canExitCloseupToParent, *this),
	  _combat(engine, scene, _player, entityManager, _sceneRuntime, _combatEffects,
		cheats, *this, _interactionProcessor) {
}

void Scene::resetIdleState() {
	_player.resetIdleState();
}

void Scene::notePlayerActivity() {
	_player.noteActivity();
}

bool Scene::isPlayerCombatLocked() const {
	return _player.isCombatLocked();
}

bool Scene::cancelPlayerAttackAnimation() {
	return _player.cancelAttackAnimation();
}

void Scene::stopPlayerRegionInteraction() {
	const RoomPlayerState &playerState = _player.getState();
	_inputState.moveLeft = false;
	_inputState.moveRight = false;
	_inputState.moveUp = false;
	_inputState.moveDown = false;
	_pendingRegionName.clear();
	(void)cancelPlayerAttackAnimation();
	_player.cancelMovement();
	if (playerState.entity && playerState.facing >= 0)
		(void)_player.setIdleAnimation(playerState.facing);
	syncPlayerPlacement();
}

void Scene::syncPlayerPlacement() {
	_player.syncAnimationPlacement();
}

void Scene::captureCurrentSaveState() {
	const RoomPlayerState &playerState = _player.getState();
	syncPlayerPlacement();
	_sceneRuntime.syncCurrentRoomRuntimeState(_scene.sceneRegions);
	const int facing = playerState.facing >= 0 ? playerState.facing : _scene.state.playerFacing;
	_engine.captureCurrentSaveRoomState(_scene.state.entranceName, _scene.state.roomName,
		playerState.centerX, playerState.bottomY, (int)playerState.z, facing,
		_scene.state.discNumber, _engine.getMusicPath());
}

bool Scene::applyCurrentRoomRuntimeMutationsInPlace(bool preservePaletteState) {
	const RoomPlayerState &playerState = _player.getState();
	Script *script = _engine.getScript();
	ResourceManager *resources = _engine.getResources();
	if (!script || !resources)
		return false;

	byte previousPalette[256 * 3];
	memcpy(previousPalette, _scene.palette, sizeof(previousPalette));
	const float previousPaletteBrightness = _scene.targetPaletteBrightness;
	Common::Array<AudioCommand> entryAudioCommands = _scene.state.audioCommands;
	debugC(2, kDebugRoom, "Harvester: reconciling room runtime mutations room='%s' preserve_palette=%d",
		_scene.state.roomName.c_str(), preservePaletteState);
	RoomSetupState updatedState;
	if (!script->materializeRoomState(
			_scene.state.entranceName, _scene.state.roomName, updatedState, *resources)) {
		return false;
	}
	updatedState.audioCommands = Common::move(entryAudioCommands);

	SceneState updatedScene;
	if (!_sceneRuntime.loadSceneResources(updatedState, updatedScene))
		return false;
	Common::Array<MonsterCombatState> updatedMonsterCombatStates =
		_combat.prepareMonsterCombatStatesForSceneChange(_scene.state.roomMonsters, updatedState.roomMonsters);

	if (!_sceneRuntime.reconcileSceneEntitiesInPlace(_scene, updatedState, updatedScene))
		return false;

	_scene.state = updatedState;
	_combat.commitMonsterCombatStatesForSceneChange(Common::move(updatedMonsterCombatStates));
	_scene.sceneObjects = updatedScene.sceneObjects;
	_scene.sceneAnimations = updatedScene.sceneAnimations;
	_scene.sceneRegions = updatedScene.sceneRegions;
	_roomItems.hideCarriedRoomItem();
	const int playerCombatLoadout = script->getPlayerCombatLoadout();
	if (playerState.entity && playerState.combatLoadout != playerCombatLoadout &&
			!_player.syncCombatLoadoutVisual(_engine, updatedState, playerCombatLoadout)) {
		return false;
	}
	if (preservePaletteState) {
		memcpy(_scene.palette, previousPalette, sizeof(_scene.palette));
		_scene.targetPaletteBrightness = previousPaletteBrightness;
	} else {
		memcpy(_scene.palette, updatedScene.palette, sizeof(_scene.palette));
		_scene.targetPaletteBrightness = updatedScene.targetPaletteBrightness;
	}
	captureCurrentSaveState();
	return _inventory.refresh();
}

bool Scene::refreshCurrentScene(bool preservePlayerPlacement) {
	debugC(2, kDebugRoom, "Harvester: refreshing room scene room='%s' preserve_player_placement=%d",
		_scene.state.roomName.c_str(), preservePlayerPlacement);
	Common::Array<AudioCommand> entryAudioCommands = _scene.state.audioCommands;
	RoomSetupState refreshedState;
	if (!_engine.getScript()->materializeRoomState(
			_scene.state.entranceName, _scene.state.roomName, refreshedState, *_engine.getResources())) {
		return false;
	}

	refreshedState.audioCommands = Common::move(entryAudioCommands);
	if (!_sceneRuntime.loadSceneResources(refreshedState, _scene))
		return false;
	_combatEffects.clearHitEffects();
	if (!_flow.populateRoomSceneEntities(_scene.state, _scene.sceneObjects, _scene.sceneAnimations))
		return false;
	_sceneRuntime.restoreRoomMonsterAnimations();
	_roomItems.hideCarriedRoomItem();
	_player.rebindAfterSceneRefresh(_engine, _scene.state, _entityManager, preservePlayerPlacement);
	_combat.resetForSceneRefresh();
	_pendingRegionName.clear();
	resetIdleState();
	_flow.resetCursorAnimationSequence();
	captureCurrentSaveState();
	return _inventory.refresh();
}

void Scene::initializePlayerState() {
	_player.initialize(_scene.state,
		_entityManager ? _entityManager->findSceneEntityByName("PLAYER") : nullptr,
		_engine.getScript() ? _engine.getScript()->getPlayerCombatLoadout() : 0);
	_combat.initializeCombatStates();
}

void Scene::syncAnimatedRoomActorPlacement() {
	_player.syncAnimationPlacement();
	if (!_entityManager)
		return;

	// Class-4 NPCs retain their spawn base through ambient and death animation banks.
	for (const MonsterRecord &monster : _scene.state.roomMonsters) {
		Entity *entity = _entityManager->findSceneEntityByName(monster.monsterName);
		if (!entity)
			continue;
		(void)applyRoomActorPlacement(_scene.state, *entity,
			monster.posX, monster.posY, (float)monster.posZ);
	}
}

Common::Error Scene::runRoomExitCommands() {
	InteractionResult exitInteraction;
	if (!_engine.getScript()->executeRoomExitCommands(_scene.state.roomName, exitInteraction))
		return Common::kReadingFailed;

	for (uint exitStep = 0; exitStep < 128; ++exitStep) {
		if (exitInteraction.cdChangeDisc > 0) {
			Common::Error cdPromptError = _flow.showCdChangePrompt(exitInteraction.cdChangeDisc);
			if (cdPromptError.getCode() != Common::kNoError)
				return cdPromptError;
		}
		if (!exitInteraction.musicPath.empty())
			(void)_engine.playMusic(exitInteraction.musicPath);
		_flow.executeStartupAudioCommands(exitInteraction.audioCommands);
		if (exitInteraction.mutatedRuntimeState)
			_sceneRuntime.syncGlobalTimerEntities(exitInteraction.previousTimerRecords, true);

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
				_scene.state.roomName.c_str());
		}

		if (exitInteraction.continuationTag.empty())
			return Common::kNoError;

		InteractionResult continuationInteraction;
		if (!_engine.getScript()->executeActionTag(
				exitInteraction.continuationTag, continuationInteraction, false,
				_scene.executionContext())) {
			return Common::kNoError;
		}

		exitInteraction = Common::move(continuationInteraction);
	}

	warning("Harvester: room exit command chain for '%s' exceeded continuation safety limit",
		_scene.state.roomName.c_str());
	return Common::kNoError;
}

void Scene::applyPlayerGotoXZ(int x, int z) {
	_player.applyGotoXZ(_scene.state, _entityManager, x, z);
	resetIdleState();
	captureCurrentSaveState();
}

void Scene::requestPlayerGameOver(const char *reason, const Common::String &sourceName) {
	if (_flow.hasPendingMainMenuReturn())
		return;

	debugC(1, kDebugCombat,
		"Harvester: player game over queued reason='%s' source='%s' hp=%d",
		reason ? reason : "", sourceName.c_str(),
		_engine.getScript()
			? _engine.getScript()->getPlayerCurrentHitPoints()
			: 0);
	_flow.requestGameOverReturn();
}

void Scene::startPlayerDefeatSequence(const char *reason, const Common::String &sourceName, int damageType) {
	const RoomPlayerState &playerState = _player.getState();
	if (_flow.hasPendingMainMenuReturn() || playerState.deathActive)
		return;

	_player.cancelIdleAnimation();

	if (!_player.startDeathAnimation(damageType, _engine.isGoreEnabled())) {
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
}

InputResult Scene::runRoomEntryInteraction(const InteractionResult &roomEntryInteraction) {
	bool didEntryTransition = false;
	Common::Error entryInteractionError = _interactionProcessor.handleInteractionResult(
		roomEntryInteraction, didEntryTransition, Common::String());
	if (entryInteractionError.getCode() != Common::kNoError)
		return InputResult::leaveRoom(entryInteractionError);
	if (_flow.hasPendingMainMenuReturn() || didEntryTransition)
		return InputResult::leaveRoom(Common::kNoError);
	return InputResult();
}

void Scene::updateRoomInterface(bool &needsRedraw) {
	captureCurrentSaveState();
	if (_inventory.isOpen() && _inventory.refreshIfRuntimeStateChanged())
		needsRedraw = true;
	if (!_presentationState.combatLoadoutStatusText.value.empty() &&
			_presentationState.combatLoadoutStatusHideTick != 0 &&
			(int32)(Player::getRuntimeClockTicks() - _presentationState.combatLoadoutStatusHideTick) >= 0) {
		_presentationState.combatLoadoutStatusText = ResolvedText();
		_presentationState.combatLoadoutStatusHideTick = 0;
		if (_presentationState.closeInventoryAfterCombatLoadoutStatus) {
			_presentationState.closeInventoryAfterCombatLoadoutStatus = false;
			if (_inventory.close())
				needsRedraw = true;
		}
		needsRedraw = true;
	}
}

InputResult Scene::drawRoomFrame(bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	const RoomIdleAnimationState &idleState = _player.getIdleState();
	const bool fadeIn = _engine.getMedia()->isRoomPaletteFadePending();
	if (!needsRedraw && !fadeIn)
		return InputResult();

	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen)
		return InputResult::leaveRoom(Common::kNoError);

	const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
	const bool inventorySelectionActive = _inventory.hasSelection();
	const bool roomCarryActive = _roomItems.hasCarriedRoomItem();
	const bool activeCarry = inventorySelectionActive || roomCarryActive;
	const bool inventoryPanelContainsMouse = _inventory.isOpen() && inventoryPanelBounds.contains(_flow.getMousePos());
	const bool suppressHover = _presentationState.showingInspectText || idleState.active || idleState.exiting ||
		isPlayerCombatLocked() ||
		(_inventory.isOpen() && (inventoryPanelContainsMouse || !inventorySelectionActive));
	RoomHoverState hoverState = suppressHover
		? RoomHoverState()
		: _flow.getRoomHoverState(_scene);
	Common::String promptText;
	Common::String inventoryTooltipText;
	auto resolveCarryTargetLabel = [&]() {
		const Entity *target = suppressHover ? nullptr : _roomItems.findSelectedItemTarget(_flow.getMousePos(), false);
		if (!target)
			return Common::String();
		if (target == playerState.entity)
			return Common::String(kPlayerInventoryLabel);
		if (const ObjectRecord *object = SceneEntities::findSceneObjectByName(_scene.sceneObjects, target->getName()))
			return _engine.getScript()->resolveObjectLabel(*object);
		for (const NpcRecord &npc : _scene.state.roomNpcs) {
			if (npc.npcName.equalsIgnoreCase(target->getName()))
				return resolveStartupNpcLabel(npc);
		}
		return Common::String();
	};
	if (activeCarry) {
		Common::String targetLabel;
		const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow.getMousePos());
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
			promptText = _inventory.buildSelectedPrompt(targetLabel, _flow.getMenuTextConfig());
			_inventory.setPromptText(promptText);
		} else {
			promptText = buildUseItemPrompt(
				_flow.getMenuTextConfig(), _roomItems.getCarriedRoomItemLabel(), targetLabel);
		}
		hoverState.cursorSequence = 7;
	} else if (_inventory.isOpen()) {
		const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow.getMousePos());
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
	if (!_presentationState.showingInspectText)
		_presentationState.currentInteractionPromptText = promptText;
	if (Entity *cursor = _entityManager ? _entityManager->getCursorEntity() : nullptr) {
		cursor->setAnimationSequence(
			(_presentationState.showingInspectText || idleState.active || idleState.exiting ||
				isPlayerCombatLocked() ||
				_inventory.isOpen() || activeCarry)
				? 7
				: hoverState.cursorSequence);
	}

	RoomPresentation::drawRoomScene(_engine, *activeScreen, _scene, fadeIn ? 0.0f : _scene.targetPaletteBrightness);
	_combatEffects.drawCombatDamagePopups(*activeScreen, _engine.isCombatDebugEnabled());
	if (_engine.getScript() && playerState.entity) {
		drawCombatLoadoutResourceIcons(*activeScreen, *_view.art, *_engine.getScript(),
			_engine.getScript()->getPlayerCombatLoadout());
	}
	if (_inventory.isOpen()) {
		_inventory.drawOverlay(*activeScreen);
		drawInventoryWeekday(*activeScreen, *_view.inventoryTooltipFont,
			_inventory.resolveWeekdayLabel(_flow.getMenuTextConfig()));
	}
	if (inventorySelectionActive) {
		_inventory.drawSelectedDragItem(*activeScreen, _flow.getMousePos());
	} else if (roomCarryActive && _roomItems.getCarriedRoomItemBitmap().isValid()) {
		const int drawX = _flow.getMousePos().x - (int)_roomItems.getCarriedRoomItemBitmap().width / 2;
		const int drawY = _flow.getMousePos().y - (int)_roomItems.getCarriedRoomItemBitmap().height / 2;
		blitTransparentBitmap(*activeScreen, _roomItems.getCarriedRoomItemBitmap(), drawX, drawY);
	}

	if (_presentationState.showingInspectText) {
		RoomPresentation::drawRoomInspectText(*activeScreen, *_view.art, *_view.inspectFont, _presentationState.inspectText, _view.useNativeInspectFont);
		if (!_presentationState.currentInteractionPromptText.empty())
			drawRoomPrompt(*activeScreen, *_view.promptFont,
				_presentationState.currentInteractionPromptText, _view.useNativePromptFont);
	} else if (!_presentationState.combatLoadoutStatusText.value.empty()) {
		RoomPresentation::drawRoomInspectText(*activeScreen, *_view.art, *_view.inspectFont, _presentationState.combatLoadoutStatusText,
			_view.useNativeInspectFont);
	} else if (!inventoryTooltipText.empty()) {
		drawInventoryTooltip(*activeScreen, *_view.inventoryTooltipFont, inventoryTooltipText);
	} else if (!promptText.empty()) {
		drawRoomPrompt(*activeScreen, *_view.promptFont, promptText, _view.useNativePromptFont);
	}

	if (_entityManager)
		_entityManager->drawCursor(*activeScreen);
	activeScreen->makeAllDirty();
	activeScreen->update();
	needsRedraw = false;

	if (fadeIn) {
		_engine.getMedia()->clearRoomPaletteFade();
		debugC(2, kDebugRoom, "Harvester: fading room '%s' in after movie", _scene.state.roomName.c_str());
		Common::Error fadeError = _flow.fadeInRoomScene(_scene.palette, _scene.targetPaletteBrightness);
		if (fadeError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(fadeError);
	}
	return InputResult();
}

InputResult Scene::pollRoomEvents(Input &input, bool &needsRedraw) {
	Common::Keymap *gameplay = g_system->getEventManager()->getKeymapper()->getKeymap("harvester-gameplay");
	Common::Keymap *defaults = g_system->getEventManager()->getKeymapper()->getKeymap("engine-default");
	const bool defaultsEnabled = defaults && defaults->isEnabled();
	Common::Event event;
	while (true) {
		// Nested menus and dialogue retain their keyboard input while the room
		// handler runs. Only the room event poll uses the gameplay actions.
		if (gameplay)
			gameplay->setEnabled(true);
		if (gameplay && defaults)
			defaults->setEnabled(false);
		const bool hasEvent = g_system->getEventManager()->pollEvent(event);
		if (gameplay)
			gameplay->setEnabled(false);
		if (gameplay && defaults)
			defaults->setEnabled(defaultsEnabled);
		if (!hasEvent)
			break;
		const InputResult inputResult = input.handleEvent(event, needsRedraw);
		if (inputResult.returnFromRoom)
			return inputResult;
	}
	return InputResult();
}

Scene::PlayerControlState Scene::preparePlayerControlState() {
	const RoomPlayerState &playerState = _player.getState();
	Script *script = _engine.getScript();
	const bool playerAlive = script && script->getPlayerCurrentHitPoints() > 0;
	const bool playerControlPaused = script && script->isPlayerControlPaused();
	const bool playerCanAct = playerAlive && !playerControlPaused;
	(void)_player.updateAnimationRate(!playerControlPaused && _inputState.speedUpActive);
	if (!playerAlive && (_inputState.moveLeft || _inputState.moveRight || _inputState.moveUp || _inputState.moveDown ||
			playerState.hasMoveTarget || playerState.turnActive ||
			playerState.attackActive || playerState.hitActive)) {
		_inputState.attackModifierHeld = false;
		stopPlayerRegionInteraction();
	}

	PlayerControlState control;
	control.alive = playerAlive;
	control.paused = playerControlPaused;
	control.canAct = playerCanAct;
	return control;
}

void Scene::advanceRuntimeEntities(bool &needsRedraw) {
	if (_flow.tickRuntimeEntities())
		needsRedraw = true;
	syncAnimatedRoomActorPlacement();
}

void Scene::updatePlayerCombatAnimations(bool &needsRedraw) {
	if (_player.updateAttackAnimationState(_engine))
		needsRedraw = true;
	if (_player.updateHitAnimationState(_engine, _scene.state, _scene.sceneObjects, _scene.sceneAnimations)) {
		needsRedraw = true;
	}
	if (_player.updateEvasiveAnimation(_engine))
		needsRedraw = true;
}

void Scene::updatePlayerControls(const PlayerControlState &control, bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	const RoomIdleAnimationState &idleState = _player.getIdleState();
	const bool attackLeft = (_inputState.attackModifierHeld && _inputState.moveLeft) ||
		(_inputState.mediumAttackHeld && playerState.facing == 1);
	const bool attackRight = (_inputState.attackModifierHeld && _inputState.moveRight) ||
		(_inputState.mediumAttackHeld && playerState.facing == 2);
	const bool attackUp = _inputState.highAttackHeld || (_inputState.attackModifierHeld && _inputState.moveUp);
	const bool attackDown = _inputState.lowAttackHeld || (_inputState.attackModifierHeld && _inputState.moveDown);
	const bool keyboardAttackRequested =
		attackLeft || attackRight || attackUp || attackDown || _inputState.mediumAttackHeld;
	const bool directEvasiveRequested = _inputState.duckHeld || _inputState.jumpHeld;
	if (control.canAct &&
			!playerState.attackActive && !playerState.hitActive && playerState.evasiveState < 0 && keyboardAttackRequested &&
			!idleState.active && !idleState.exiting) {
		const uint32 now = Player::getRuntimeClockTicks();
		if (_inputState.nextKeyboardAttackAllowedTick == 0 || (int32)(now - _inputState.nextKeyboardAttackAllowedTick) >= 0) {
			_combat.capturePlayerAttackTargetForKeyboard(
				attackLeft, attackRight, attackUp, attackDown);
			if (_player.startKeyboardAttackAnimation(_scene.state,
					attackLeft, attackRight, attackUp, attackDown)) {
				_inputState.nextKeyboardAttackAllowedTick = now + kNativeKeyboardAttackRepeatTicks;
				notePlayerActivity();
				debugC(1, kDebugCombat,
					"Harvester: _combat player keyboard attack start loadout=%d weapon='%s' damage=%d damage_type='%s' target_class='%s' target='%s' input=(L=%d R=%d U=%d D=%d) facing=%d",
					playerState.combatLoadout,
					Player::describeCombatLoadout(playerState.combatLoadout),
					Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout),
					Player::describeCombatDamageType(Player::resolveCombatLoadoutDamageType(playerState.combatLoadout)),
					describeCombatTargetClass(playerState.attackTargetClassId),
					playerState.attackTargetName.c_str(),
					attackLeft, attackRight, attackUp, attackDown, playerState.facing);
				needsRedraw = true;
			} else {
				_player.clearAttackTarget();
			}
		}
	}
	if (control.canAct && !playerState.attackActive && !playerState.hitActive &&
			_player.updateTurnAnimationState()) {
		needsRedraw = true;
	}

	if (control.canAct &&
			!playerState.attackActive && !playerState.hitActive && playerState.evasiveState < 0 &&
			!keyboardAttackRequested && !idleState.active && !idleState.exiting) {
		const int regionFacing = _interactionProcessor.getPendingRegionFacing();
		if (_inputState.evasiveModifierHeld || directEvasiveRequested) {
			if (_player.startEvasiveAnimation(
					!directEvasiveRequested && _inputState.moveLeft,
					!directEvasiveRequested && _inputState.moveRight,
					_inputState.jumpHeld || (!directEvasiveRequested && _inputState.moveUp),
					_inputState.duckHeld || (!directEvasiveRequested && _inputState.moveDown)) ||
					_player.stepKeyboardMovement(_engine, _scene.state, _scene.sceneObjects, _scene.sceneAnimations,
						false, false, false, false)) {
				notePlayerActivity();
				needsRedraw = true;
			}
		} else if (_player.stepKeyboardMovement(_engine, _scene.state, _scene.sceneObjects, _scene.sceneAnimations,
				_inputState.moveLeft, _inputState.moveRight, _inputState.moveUp, _inputState.moveDown)) {
			notePlayerActivity();
			needsRedraw = true;
		} else if (_player.stepMoveTarget(_engine, _scene.state, _scene.sceneObjects, _scene.sceneAnimations, regionFacing)) {
			notePlayerActivity();
			needsRedraw = true;
		}

		if (!_presentationState.showingInspectText && !keyboardAttackRequested && !directEvasiveRequested &&
				!_inputState.moveLeft && !_inputState.moveRight && !_inputState.moveUp && !_inputState.moveDown &&
				!playerState.hasMoveTarget && !playerState.turnActive && !playerState.hitActive && playerState.evasiveState < 0 &&
				playerState.entity && playerState.facing >= 0 &&
				!Player::isIdleAnimationExcludedRoom(_scene.state.roomName) &&
				Player::getRuntimeClockTicks() > idleState.triggerTick &&
				_player.startIdleAnimation(_engine, _scene.state)) {
			needsRedraw = true;
		}
	}
}

void Scene::updatePlayerDeath(bool &needsRedraw) {
	if (_player.updateDeathAnimationState()) {
		requestPlayerGameOver("combat_player_death_complete", Common::String());
		needsRedraw = true;
	}
}

void Scene::pruneCombatEffects(bool &needsRedraw) {
	if (_combatEffects.pruneCombatHitEffects())
		needsRedraw = true;
	if (_combatEffects.pruneCombatDamagePopups())
		needsRedraw = true;
}

InputResult Scene::dispatchExpiredTimers(bool &needsRedraw) {
	if (_entityManager) {
		Common::Array<Common::String> expiredTimerNames;
		if (_entityManager->takeExpiredTimerNames(expiredTimerNames)) {
			_sceneRuntime.syncCurrentRoomRuntimeState(_scene.sceneRegions);
			for (const Common::String &timerName : expiredTimerNames) {
				InteractionResult timerInteraction;
				if (!_engine.getScript()->executeTimerAction(
						timerName, timerInteraction, true, _scene.executionContext()))
					continue;

				bool didTransition = false;
				Common::Error interactionError =
					_interactionProcessor.handleInteractionResult(
						timerInteraction, didTransition, Common::String(), false);
				if (interactionError.getCode() != Common::kNoError)
					return InputResult::leaveRoom(interactionError);
				if (_flow.hasPendingMainMenuReturn())
					return InputResult::leaveRoom(Common::kNoError);
				needsRedraw = true;
				if (!_pendingTransition.roomName.empty())
					break;
			}
		}
	}
	return InputResult();
}

InputResult Scene::runRoomFrames(const Common::String &currentRoomTarget) {
	Input input(_engine, _flow, *this, _scene, _inventory, _cheats,
		_roomItems, _interactionProcessor, _presentationState, *_view.art, _canExitCloseupToParent);
	bool needsRedraw = true;

	if (!_inventory.refresh())
		return InputResult::leaveRoom(Common::kReadingFailed);
	Graphics::FrameLimiter limiter(g_system, 60);
	captureCurrentSaveState();

	if (shouldRunStartupRoomProbe())
		_flow.logStartupRoomProbe(_scene, currentRoomTarget);

	while (!_engine.shouldQuit()) {
		if (_flow.hasPendingMainMenuReturn())
			return InputResult::leaveRoom(Common::kNoError);
		if (_flow.takePendingNewGameRestart()) {
			_flow.prepareForNewGame();
			_pendingTransition.roomName = "START";
			_pendingTransition.isRoomName = false;
			_pendingTransition.usesSavedRoomState = false;
			_pendingTransition.savedRoomState.clear();
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return InputResult::leaveRoom(Common::kReadingFailed);
			break;
		}
		if (_engine.hasPendingLoadedSaveRoomState()) {
			_pendingTransition.roomName = _engine.getPendingLoadedSaveRoomState().entranceName;
			_pendingTransition.isRoomName = false;
			_pendingTransition.usesSavedRoomState = false;
			_pendingTransition.savedRoomState.clear();
			if (_pendingTransition.roomName.empty()) {
				_pendingTransition.roomName = _engine.getPendingLoadedSaveRoomState().roomName;
				_pendingTransition.isRoomName = true;
			}
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return InputResult::leaveRoom(Common::kReadingFailed);
			break;
		}
		if (_flow.hasPendingDebugRoomChange()) {
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return InputResult::leaveRoom(Common::kReadingFailed);
			return InputResult::leaveRoom(Common::kNoError);
		}
		if (!_pendingTransition.roomName.empty()) {
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return InputResult::leaveRoom(Common::kReadingFailed);
			break;
		}
		InteractionResult queuedDebugInteraction;
		if (_flow.takeQueuedDebugInteraction(queuedDebugInteraction)) {
			// Run console-injected commands through the normal room interaction handler so
			// CHANGE_ROOM, modal text, dialogue, and chained tags behave exactly like script-driven actions.
			bool didDebugTransition = false;
			Common::Error debugInteractionError = _interactionProcessor.handleInteractionResult(
				queuedDebugInteraction, didDebugTransition, Common::String());
			if (debugInteractionError.getCode() != Common::kNoError)
				return InputResult::leaveRoom(debugInteractionError);
			if (_flow.hasPendingMainMenuReturn())
				return InputResult::leaveRoom(Common::kNoError);
			if (!_pendingTransition.roomName.empty()) {
				if (!_roomItems.stowCarriedRoomItemToInventory())
					return InputResult::leaveRoom(Common::kReadingFailed);
				break;
			}
			needsRedraw = true;
		}
		updateRoomInterface(needsRedraw);
		const InputResult drawResult = drawRoomFrame(needsRedraw);
		if (drawResult.returnFromRoom)
			return drawResult;

		const InputResult inputResult = pollRoomEvents(input, needsRedraw);
		if (inputResult.returnFromRoom)
			return inputResult;
		if (!_pendingTransition.roomName.empty()) {
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return InputResult::leaveRoom(Common::kReadingFailed);
			break;
		}

		const PlayerControlState control = preparePlayerControlState();
		advanceRuntimeEntities(needsRedraw);
		Common::Error pendingRegionError =
			tryActivatePendingRegion(needsRedraw);
		if (pendingRegionError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(pendingRegionError);
		if (_flow.hasPendingMainMenuReturn())
			return InputResult::leaveRoom(Common::kNoError);
		if (!_pendingTransition.roomName.empty()) {
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return InputResult::leaveRoom(Common::kReadingFailed);
			break;
		}
		if (!isPlayerCombatLocked()) {
			pendingRegionError = tryActivatePassiveRegion(needsRedraw);
			if (pendingRegionError.getCode() != Common::kNoError)
				return InputResult::leaveRoom(pendingRegionError);
			if (_flow.hasPendingMainMenuReturn())
				return InputResult::leaveRoom(Common::kNoError);
			if (!_pendingTransition.roomName.empty()) {
				if (!_roomItems.stowCarriedRoomItemToInventory())
					return InputResult::leaveRoom(Common::kReadingFailed);
				break;
			}
		}

		Common::Error combatError = Common::kNoError;
		if (!control.paused) {
			combatError = resolvePlayerAttackContact(needsRedraw);
			if (combatError.getCode() != Common::kNoError)
				return InputResult::leaveRoom(combatError);
			if (_flow.hasPendingMainMenuReturn())
				return InputResult::leaveRoom(Common::kNoError);
			if (!_pendingTransition.roomName.empty()) {
				if (!_roomItems.stowCarriedRoomItemToInventory())
					return InputResult::leaveRoom(Common::kReadingFailed);
				break;
			}
			updatePlayerCombatAnimations(needsRedraw);
		}
		updatePlayerControls(control, needsRedraw);
		combatError = updateRoomMonsterCombat(needsRedraw);
		if (combatError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(combatError);
		syncCombatHitEffects(needsRedraw);
		if (_flow.hasPendingMainMenuReturn())
			return InputResult::leaveRoom(Common::kNoError);
		if (!_pendingTransition.roomName.empty()) {
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return InputResult::leaveRoom(Common::kReadingFailed);
			break;
		}
		updatePlayerDeath(needsRedraw);
		if (_flow.hasPendingMainMenuReturn())
			return InputResult::leaveRoom(Common::kNoError);
		pruneCombatEffects(needsRedraw);
		const InputResult timerResult = dispatchExpiredTimers(needsRedraw);
		if (timerResult.returnFromRoom)
			return timerResult;
		updateIdleAnimation(needsRedraw);
		if (_engine.isCombatDebugEnabled() && _combatEffects.hasDamagePopups())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}
	if (_engine.shouldQuit())
		return InputResult::leaveRoom(Common::kNoError);
	if (_flow.hasPendingMainMenuReturn())
		return InputResult::leaveRoom(Common::kNoError);
	if (_pendingTransition.roomName.empty())
		return InputResult::leaveRoom(Common::kNoError);
	if (_pendingTransition.roomName.equals(kExitCloseupPendingRoomChange)) {
		_flow.requestCloseupParentRestart();
		return InputResult::leaveRoom(Common::kNoError);
	}

	return InputResult();
}

void Scene::syncGlobalTimerEntities(const Common::Array<TimerRecord> &previousTimerRecords) {
	_sceneRuntime.syncGlobalTimerEntities(previousTimerRecords);
}

bool Scene::captureDialogueBackdrop(IndexedBitmap &backdrop) {
	if (_engine.getMedia()->isRoomPaletteFadePending()) {
		bool needsRedraw = true;
		if (drawRoomFrame(needsRedraw).returnFromRoom)
			return false;
	}
	return _presentation.captureDialogueBackdrop(backdrop);
}

Common::Error Scene::showCdChangePrompt(int discNumber) {
	return _flow.showCdChangePrompt(discNumber);
}

Common::Error Scene::applyLightingCommand(StartupLightingCommand command) {
	return _presentation.applyLightingCommand(command);
}

Common::Error Scene::runModalShowText(const ResolvedText &text) {
	return _presentation.runModalShowText(text, _flow);
}

Common::Error Scene::tryActivatePendingRegion(bool &needsRedraw) {
	return isPlayerCombatLocked() ? Common::kNoError : _interactionProcessor.tryActivatePendingRegion(_inventory, needsRedraw);
}

Common::Error Scene::tryActivatePassiveRegion(bool &needsRedraw) {
	return _interactionProcessor.tryActivatePassiveRegion(_inventory, needsRedraw);
}

Common::Error Scene::resolvePlayerAttackContact(bool &needsRedraw) {
	return _combat.resolvePlayerAttackContact(needsRedraw);
}

Common::Error Scene::updateRoomMonsterCombat(bool &needsRedraw) {
	return _combat.updateRoomMonsterCombat(needsRedraw);
}

void Scene::syncCombatHitEffects(bool &needsRedraw) {
	if (_combatEffects.syncCombatHitEffects())
		needsRedraw = true;
}

void Scene::updateIdleAnimation(bool &needsRedraw) {
	if (_player.updateIdleAnimation(_scene.state))
		needsRedraw = true;
}

void Scene::capturePlayerAttackTarget(const Common::Point &mousePos, const NpcRecord *npc) {
	_combat.capturePlayerAttackTarget(mousePos, npc);
}

Common::Error Scene::killDebugActiveMonster(const Common::Point &mousePos, bool &needsRedraw) {
	return _combat.killDebugActiveMonster(mousePos, needsRedraw);
}

SceneRunner::SceneRunner(HarvesterEngine &engine,
		InventorySystem &inventory, CheatSystem &cheats)
	: _engine(engine), _inventory(inventory), _cheats(cheats) {
}

Common::Error SceneRunner::runRoomLoop(Flow &flow, const Common::String &targetName,
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

		_engine.getMedia()->resetMoviePaletteState();
		SceneState scene;
		if (!SceneEntities::loadRoomSceneResources(state, *_engine.getResources(), scene))
			return Common::kReadingFailed;

		Graphics::Screen *screen = _engine.getScreen();
		const Art *art = _engine.getArt();
		const Graphics::Font *bodyFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		const Text *text = _engine.getText();
		const CftFontResource *promptFontResource = text ? text->findFont("MEDFONT1") : nullptr;
		const CftFontResource *inventoryTooltipFontResource = text ? text->findFont("TEXTFONT") : nullptr;
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
		const RoomViewResources view = { art, promptFont, inspectFont, inventoryTooltipFont,
			useNativePromptFont, useNativeInspectFont };
		Scene session(_engine, flow, scene, entityManager, _inventory, _cheats, view,
			flow.isNestedRoomLoop());
		debugC(1, kDebugRoom, "Harvester: created room session room='%s'", scene.state.roomName.c_str());
		session.initializePlayerState();

		session.resetIdleState();

		InteractionResult roomEntryInteraction;
		bool hasPendingRoomEntryInteraction = false;
		if (shouldRunRoomEntryCommands) {
			if (!_engine.getScript()->executeRoomEnterCommands(
					scene.state.roomName, roomEntryInteraction)) {
				return Common::kReadingFailed;
			}

			// Native room_setup applies entry-time state changes before the first visible room frame.
			if (roomEntryInteraction.mutatedRuntimeState) {
				session.syncGlobalTimerEntities(roomEntryInteraction.previousTimerRecords);
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

		logScenePaletteSummary("room setup initial palette", scene, 0.0f);
		RoomPresentation::drawRoomScene(_engine, *screen, scene, 0.0f);
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
				const InputResult entryResult = session.runRoomEntryInteraction(roomEntryInteraction);
				if (entryResult.returnFromRoom)
					return entryResult.error;
			}

			flow.resetCursorAnimationSequence();
			session.resetIdleState();
		}
		ScopedDeferredLiveNpcDeathTransitions deferredLiveNpcDeathTransitions(
			_engine.getScript());

		const InputResult frameResult = session.runRoomFrames(currentRoomTarget);
		if (frameResult.returnFromRoom)
			return frameResult.error;
		const RoomPendingTransition &pendingTransition = session.getPendingTransition();

		currentRoomTarget = pendingTransition.roomName;
		currentTargetIsRoomName = pendingTransition.isRoomName;
		currentTargetUsesSavedRoomState = pendingTransition.usesSavedRoomState;
		currentTargetSavedRoomState = pendingTransition.savedRoomState;
	}

	return Common::kNoError;
}

} // End of namespace Harvester
