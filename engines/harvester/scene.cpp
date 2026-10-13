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

#include "harvester/scene.h"

#include "common/algorithm.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/flow.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/player.h"
#include "harvester/combat.h"
#include "harvester/entity.h"

#include "common/ptr.h"
#include "graphics/blit.h"
#include "graphics/fontman.h"
#include "harvester/cft_font.h"
#include "harvester/cheats.h"
#include "harvester/resources.h"
#include "harvester/monster.h"
#include "harvester/interaction.h"
#include "harvester/input.h"
#include "harvester/room_presentation.h"
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

Scene::Scene(HarvesterEngine &engine, Flow &flow, SceneState &scene, EntityManager *entityManager,
		InventorySystem &inventory, CheatSystem &cheats, const RoomViewResources &view, bool canExitCloseupToParent)
	: _engine(engine), _flow(flow), _scene(scene), _entityManager(entityManager), _inventory(inventory),
	  _sceneRuntime(scene.state, engine.getScript(), engine.getResources(), entityManager), _combatEffects(entityManager),
	  _cheats(cheats), _view(view), _canExitCloseupToParent(canExitCloseupToParent),
	  _roomItems(engine, scene, _sceneRuntime, inventory, _pendingTransition.roomName),
	  _presentation(engine, scene, view.art, view.inspectFont, view.useNativeInspectFont),
	  _interactionProcessor(engine, flow, scene, _playerState, _pendingRegionName, _pendingTransition.roomName,
		_pendingTransition.isRoomName, _pendingTransition.usesSavedRoomState, _pendingTransition.savedRoomState,
		canExitCloseupToParent, *this) {
}

void Scene::resetIdleState() {
	_idleState = RoomIdleAnimationState();
	_idleState.activityTick = Player::getRuntimeClockTicks();
	_idleState.resetTick = _idleState.activityTick;
	Player::updateIdleTrigger(_idleState);
}

void Scene::notePlayerActivity() {
	_idleState.activityTick = Player::getRuntimeClockTicks();
	Player::updateIdleTrigger(_idleState);
}

bool Scene::isPlayerCombatLocked() const {
	return _playerState.attackActive || _playerState.hitActive || _playerState.deathActive;
}

bool Scene::cancelPlayerAttackAnimation() {
	if (!_playerState.attackActive)
		return false;

	_playerState.attackActive = false;
	_playerState.attackFirstFrame = -1;
	_playerState.attackLastFrame = -1;
	_playerState.attackContactFrame = -1;
	if (!_playerState.entity)
		return false;
	_playerState.attackSoundPlayed = false;
	_playerState.attackSoundPlaybackFrame = -1;
	_playerState.attackContactResolved = false;
	_playerState.attackTargetName.clear();
	_playerState.attackTargetClassId = -1;
	const int resumeFacing = _playerState.attackResumeFacing >= 0
		? _playerState.attackResumeFacing
		: (_playerState.facing >= 0 ? _playerState.facing : 0);
	_playerState.attackResumeFacing = -1;
	return Player::setIdleAnimation(_playerState, resumeFacing);
}

void Scene::stopPlayerRegionInteraction() {
	_inputState.moveLeft = false;
	_inputState.moveRight = false;
	_inputState.moveUp = false;
	_inputState.moveDown = false;
	_pendingRegionName.clear();
	(void)cancelPlayerAttackAnimation();
	_playerState.hasMoveTarget = false;
	_playerState.turnActive = false;
	_playerState.turnTargetFacing = -1;
	if (_playerState.entity && _playerState.facing >= 0)
		(void)Player::setIdleAnimation(_playerState, _playerState.facing);
	syncPlayerPlacement();
}

void Scene::syncPlayerPlacement() {
	Player::syncAnimationPlacement(_playerState);
}

void Scene::captureCurrentSaveState() {
	syncPlayerPlacement();
	_sceneRuntime.syncCurrentRoomRuntimeState(_scene.sceneRegions);
	const int facing = _playerState.facing >= 0 ? _playerState.facing : _scene.state.playerFacing;
	_engine.captureCurrentSaveRoomState(_scene.state.entranceName, _scene.state.roomName,
		_playerState.centerX, _playerState.bottomY, (int)_playerState.z, facing,
		_scene.state.discNumber, _engine.getMusicPath());
}

bool Scene::applyCurrentRoomRuntimeMutationsInPlace(bool preservePaletteState) {
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
	const int playerCombatLoadout = script->getPlayerCombatLoadout();
	if (_playerState.entity && _playerState.combatLoadout != playerCombatLoadout &&
			!Player::syncCombatLoadoutVisual(
				_engine, updatedState, _playerState, playerCombatLoadout)) {
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
	Player::rebindAfterSceneRefresh(_scene.state, _playerState, _entityManager, preservePlayerPlacement);
	_playerState.combatLoadout =
		_engine.getScript() ? _engine.getScript()->getPlayerCombatLoadout() : 0;
	_combat.resetForSceneRefresh(_scene.state);
	_pendingRegionName.clear();
	resetIdleState();
	_flow.resetCursorAnimationSequence();
	captureCurrentSaveState();
	return _inventory.refresh();
}

void Scene::initializePlayerState() {
	_playerState.entity = _entityManager ? _entityManager->findSceneEntityByName("PLAYER") : nullptr;
	_playerState.centerX = _scene.state.playerSpawnX;
	_playerState.bottomY = _scene.state.playerSpawnY;
	_playerState.z = (float)_scene.state.playerSpawnZ;
	_playerState.facing = _scene.state.playerFacing;
	_playerState.combatLoadout =
		_engine.getScript() ? _engine.getScript()->getPlayerCombatLoadout() : 0;
	_playerState.turnActive = false;
	_playerState.turnTargetFacing = -1;
	_combat.initializeCombatStates(_scene.state);
}

void Scene::syncAnimatedRoomActorPlacement() {
	Player::syncAnimationPlacement(_playerState);
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
				_scene.state.roomName)) {
			return Common::kNoError;
		}

		exitInteraction = Common::move(continuationInteraction);
	}

	warning("Harvester: room exit command chain for '%s' exceeded continuation safety limit",
		_scene.state.roomName.c_str());
	return Common::kNoError;
}

void Scene::applyPlayerGotoXZ(int x, int z) {
	_playerState.hasMoveTarget = false;
	_playerState.turnActive = false;
	_playerState.turnTargetFacing = -1;
	_playerState.centerX = x;
	_playerState.z = (float)z;
	_playerState.bottomY = mapRoomDepthToScreenYForCombat(
		_scene.state, _playerState.z, _playerState.bottomY);
	Common::Rect screenRect;
	bool hasScreenRect = false;
	float depthScale = 0.0f;
	if (_playerState.entity) {
		(void)applyRoomActorPlacement(_scene.state, *_playerState.entity,
			_playerState.centerX, _playerState.bottomY, _playerState.z);
		if (_entityManager)
			_entityManager->reinsertSceneEntity(_playerState.entity);
		screenRect = _playerState.entity->getScreenRect();
		hasScreenRect = true;
		depthScale = _playerState.entity->getDepthScale();
	}
	debugC(1, kDebugPlayer,
		"Harvester: PC_GOTO_XZ applied x=%d z=%d pos=(%d,%d,z=%.2f) screen_rect=(%d,%d)-(%d,%d) depth_scale=%.3f",
		x, z, _playerState.centerX, _playerState.bottomY, (double)_playerState.z,
		hasScreenRect ? screenRect.left : 0, hasScreenRect ? screenRect.top : 0,
		hasScreenRect ? screenRect.right : 0, hasScreenRect ? screenRect.bottom : 0,
		(double)depthScale);
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
	if (_flow.hasPendingMainMenuReturn() || _playerState.deathActive)
		return;

	if (_idleState.entity) {
		_idleState.entity->setVisible(false);
		_idleState.entity->setAnimationRate(0);
	}
	_idleState.active = false;
	_idleState.loopStarted = false;
	_idleState.exiting = false;
	_idleState.restoreFacing = -1;
	if (_playerState.entity)
		_playerState.entity->setVisible(true);

	if (!Player::startDeathAnimation(_playerState, damageType, _engine.isGoreEnabled())) {
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

bool Scene::drawRoomFrame(bool &needsRedraw) {
	if (!needsRedraw)
		return true;

	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen)
		return false;

	const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
	const bool inventorySelectionActive = _inventory.hasSelection();
	const bool roomCarryActive = _roomItems.hasCarriedRoomItem();
	const bool activeCarry = inventorySelectionActive || roomCarryActive;
	const bool inventoryPanelContainsMouse = _inventory.isOpen() && inventoryPanelBounds.contains(_flow._mousePos);
	const bool suppressHover = _presentationState.showingInspectText || _idleState.active || _idleState.exiting ||
		isPlayerCombatLocked() ||
		(_inventory.isOpen() && (inventoryPanelContainsMouse || !inventorySelectionActive));
	RoomHoverState hoverState = suppressHover
		? RoomHoverState()
		: resolveRoomHoverState(_engine, _scene.state, _scene.sceneObjects, _scene.state.roomNpcs,
			_scene.sceneRegions, _flow._mousePos, _flow._menuTextConfig, &_flow._dialogue);
	if (!suppressHover && inventorySelectionActive && !hoverState.npc) {
		if (ObjectRecord *selectedTarget = _roomItems.findSelectedInventoryRoomTarget(_flow._mousePos))
			hoverState.object = selectedTarget;
	}
	Common::String promptText;
	Common::String inventoryTooltipText;
	auto resolveCarryTargetLabel = [&]() {
		if (hoverState.playerEntity && _playerState.entity && hoverState.playerEntity == _playerState.entity)
			return Common::String(kPlayerInventoryLabel);
		if (hoverState.object)
			return _engine.getScript()->resolveObjectLabel(*hoverState.object);
		if (hoverState.npc)
			return resolveStartupNpcLabel(*hoverState.npc);
		return Common::String();
	};
	if (activeCarry) {
		Common::String targetLabel;
		const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow._mousePos);
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
			promptText = _inventory.buildSelectedPrompt(targetLabel, _flow._menuTextConfig);
			_inventory.setPromptText(promptText);
		} else {
			promptText = buildUseItemPrompt(
				_flow._menuTextConfig, _roomItems.getCarriedRoomItemLabel(), targetLabel);
		}
		hoverState.cursorSequence = 7;
	} else if (_inventory.isOpen()) {
		const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow._mousePos);
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
			(_presentationState.showingInspectText || _idleState.active || _idleState.exiting ||
				isPlayerCombatLocked() ||
				_inventory.isOpen() || activeCarry)
				? 7
				: hoverState.cursorSequence);
	}

	drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
	_combatEffects.drawCombatDamagePopups(*activeScreen, _engine.isCombatDebugEnabled());
	if (_engine.getScript() && _playerState.entity) {
		drawCombatLoadoutResourceIcons(*activeScreen, *_view.art, *_engine.getScript(),
			_engine.getScript()->getPlayerCombatLoadout());
	}
	if (_inventory.isOpen()) {
		_inventory.drawOverlay(*activeScreen);
		drawInventoryWeekday(*activeScreen, *_view.inventoryTooltipFont,
			_inventory.resolveWeekdayLabel(_flow._menuTextConfig));
	}
	if (inventorySelectionActive) {
		_inventory.drawSelectedDragItem(*activeScreen, _flow._mousePos);
	} else if (roomCarryActive && _roomItems.getCarriedRoomItemBitmap().isValid()) {
		const int drawX = _flow._mousePos.x - (int)_roomItems.getCarriedRoomItemBitmap().width / 2;
		const int drawY = _flow._mousePos.y - (int)_roomItems.getCarriedRoomItemBitmap().height / 2;
		RoomPresentation::blitBitmap(*activeScreen, _roomItems.getCarriedRoomItemBitmap(), drawX, drawY);
	}

	if (_presentationState.showingInspectText) {
		drawRoomInspectText(*activeScreen, *_view.art, *_view.inspectFont, _presentationState.inspectText, _view.useNativeInspectFont);
		if (!_presentationState.currentInteractionPromptText.empty())
			drawRoomPrompt(*activeScreen, *_view.promptFont,
				_presentationState.currentInteractionPromptText, _view.useNativePromptFont);
	} else if (!_presentationState.combatLoadoutStatusText.value.empty()) {
		drawRoomInspectText(*activeScreen, *_view.art, *_view.inspectFont, _presentationState.combatLoadoutStatusText,
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

	return true;
}

InputResult Scene::pollRoomEvents(Input &input, bool &needsRedraw) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		const InputResult inputResult = input.handleEvent(event, needsRedraw);
		if (inputResult.returnFromRoom)
			return inputResult;
	}
	return InputResult();
}

Scene::PlayerControlState Scene::preparePlayerControlState() {
	Script *script = _engine.getScript();
	const bool playerAlive = script && script->getPlayerCurrentHitPoints() > 0;
	const bool playerControlPaused = script && script->isPlayerControlPaused();
	const bool playerCanAct = playerAlive && !playerControlPaused;
	if (!playerAlive && (_inputState.moveLeft || _inputState.moveRight || _inputState.moveUp || _inputState.moveDown ||
			_playerState.hasMoveTarget || _playerState.turnActive ||
			_playerState.attackActive || _playerState.hitActive)) {
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
	if (Player::updateAttackAnimationState(_engine, _playerState))
		needsRedraw = true;
	if (Player::updateHitAnimationState(
			_engine, _scene.state, _scene.sceneObjects, _scene.sceneAnimations, _playerState)) {
		needsRedraw = true;
	}
}

void Scene::updatePlayerControls(const PlayerControlState &control, bool &needsRedraw) {
	const bool keyboardAttackRequested =
		_inputState.attackModifierHeld && (_inputState.moveLeft || _inputState.moveRight || _inputState.moveUp || _inputState.moveDown);
	if (control.canAct &&
			!_playerState.attackActive && !_playerState.hitActive && keyboardAttackRequested &&
			!_idleState.active && !_idleState.exiting) {
		const uint32 now = Player::getRuntimeClockTicks();
		if (_inputState.nextKeyboardAttackAllowedTick == 0 || (int32)(now - _inputState.nextKeyboardAttackAllowedTick) >= 0) {
			_combat.capturePlayerAttackTargetForKeyboard(_scene.state, _playerState, _entityManager, _inputState.moveLeft, _inputState.moveRight, _inputState.moveUp, _inputState.moveDown);
			if (Player::startKeyboardAttackAnimation(
					_scene.state, _playerState, _inputState.moveLeft, _inputState.moveRight, _inputState.moveUp, _inputState.moveDown)) {
				_inputState.nextKeyboardAttackAllowedTick = now + kNativeKeyboardAttackRepeatTicks;
				notePlayerActivity();
				debugC(1, kDebugCombat,
					"Harvester: _combat player keyboard attack start loadout=%d weapon='%s' damage=%d damage_type='%s' target_class='%s' target='%s' input=(L=%d R=%d U=%d D=%d) facing=%d",
					_playerState.combatLoadout,
					Player::describeCombatLoadout(_playerState.combatLoadout),
					Player::resolveCombatLoadoutDamageAmount(_playerState.combatLoadout),
					Player::describeCombatDamageType(Player::resolveCombatLoadoutDamageType(_playerState.combatLoadout)),
					describeCombatTargetClass(_playerState.attackTargetClassId),
					_playerState.attackTargetName.c_str(),
					_inputState.moveLeft, _inputState.moveRight, _inputState.moveUp, _inputState.moveDown, _playerState.facing);
				needsRedraw = true;
			} else {
				_playerState.attackTargetName.clear();
				_playerState.attackTargetClassId = -1;
			}
		}
	}
	if (control.canAct && !_playerState.attackActive && !_playerState.hitActive &&
			Player::updateTurnAnimationState(_playerState)) {
		needsRedraw = true;
	}

	if (control.canAct &&
			!_playerState.attackActive && !_playerState.hitActive &&
			!keyboardAttackRequested && !_idleState.active && !_idleState.exiting) {
		const int regionFacing = _interactionProcessor.getPendingRegionFacing();
		if (Player::stepKeyboardMovement(_engine, _scene.state, _scene.sceneObjects, _scene.sceneAnimations,
				_playerState, _inputState.moveLeft, _inputState.moveRight, _inputState.moveUp, _inputState.moveDown)) {
			notePlayerActivity();
			needsRedraw = true;
		} else if (Player::stepMoveTarget(
				_engine, _scene.state, _scene.sceneObjects, _scene.sceneAnimations,
				_playerState, regionFacing)) {
			notePlayerActivity();
			needsRedraw = true;
		}

		if (!_presentationState.showingInspectText && !keyboardAttackRequested &&
				!_inputState.moveLeft && !_inputState.moveRight && !_inputState.moveUp && !_inputState.moveDown &&
				!_playerState.hasMoveTarget && !_playerState.turnActive && !_playerState.hitActive &&
				_playerState.entity && _playerState.facing >= 0 &&
				!Player::isIdleAnimationExcludedRoom(_scene.state.roomName) &&
				Player::getRuntimeClockTicks() > _idleState.triggerTick &&
				Player::startIdleAnimation(_engine, _scene.state, _playerState, _idleState)) {
			needsRedraw = true;
		}
	}
}

void Scene::updatePlayerDeath(bool &needsRedraw) {
	if (Player::updateDeathAnimationState(_playerState)) {
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
						timerName, timerInteraction, true, _scene.state.roomName))
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
		logStartupRoomProbe(_engine, _scene, currentRoomTarget, _flow._mousePos, _flow._menuTextConfig);

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
		if (!drawRoomFrame(needsRedraw))
			return InputResult::leaveRoom(Common::kNoError);

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
	return _combat.resolvePlayerAttackContact(_engine, _scene.state, _playerState, _entityManager,
		_sceneRuntime, _combatEffects, _interactionProcessor, needsRedraw);
}

Common::Error Scene::updateRoomMonsterCombat(bool &needsRedraw) {
	return _combat.updateRoomMonsterCombat(_engine, _scene.state, _playerState, _idleState,
		_entityManager, _sceneRuntime, _combatEffects, _cheats, *this, _interactionProcessor, needsRedraw);
}

void Scene::syncCombatHitEffects(bool &needsRedraw) {
	if (_combatEffects.syncCombatHitEffects())
		needsRedraw = true;
}

void Scene::updateIdleAnimation(bool &needsRedraw) {
	if (Player::updateIdleAnimation(_scene.state, _playerState, _idleState))
		needsRedraw = true;
}

void Scene::capturePlayerAttackTarget(const Common::Point &mousePos, const NpcRecord *npc) {
	_combat.capturePlayerAttackTarget(_scene.state, _playerState, _entityManager, mousePos, npc);
}

Common::Error Scene::killDebugActiveMonster(const Common::Point &mousePos, bool &needsRedraw) {
	return _combat.killDebugActiveMonster(_engine, _scene.state, _playerState, _entityManager,
		mousePos, _combatEffects, _interactionProcessor, needsRedraw);
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

		SceneState scene;
		if (!loadRoomSceneResources(state, *_engine.getResources(), scene))
			return Common::kReadingFailed;

		Graphics::Screen *screen = _engine.getScreen();
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
		const RoomViewResources view = { art, promptFont, inspectFont, inventoryTooltipFont,
			useNativePromptFont, useNativeInspectFont };
		Scene session(_engine, flow, scene, entityManager, _inventory, _cheats, view,
			flow._roomLoopDepth > 1);
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
