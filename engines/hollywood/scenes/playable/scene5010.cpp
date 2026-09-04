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

#include "common/events.h"
#include "common/system.h"

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/playable/scene5010.h"

namespace Hollywood {

const uint16 kScene5010FirstState = 0x1392;
const uint16 kScene5010ViewportXOffset = 0x00a8;
const uint16 kScene5010ViewportMinXOffset = 0x0068;
const uint16 kScene5010ViewportMaxXOffset = 0x00a8;
const uint kScene5010ActorBankTableEntry = 0x0038;
const uint kScene5010ActorPaletteTableEntry = 0x00cc;
const uint kScene5010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene5010SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5010FrameMillis = 75;
const uint32 kScene5010SwitchFrameMillis = 20;
const uint kScene5010SwitchDescriptorCount = 9;
const uint kScene5010TransportPrepDescriptorCount = 8;
const uint kScene5010TransportArrivalDescriptorCount = 0x34;
const uint kScene5010TransportDepartDescriptorCount = 0x3f;
const uint kScene5010SwitchAnimationDescriptorCount = 0x42;
const uint kScene5010ReturnShakeFrameCount = 0x96;
const int kScene5010ReturnShakeOffset = 4;
const byte kScene5010FirstAmbientSoundCue = 0x25;
const byte kScene5010AmbientSoundCueCount = 3;
const byte kScene5010FirstAmbientMusicCue = 0x0b;
const byte kScene5010AmbientMusicCueCount = 5;
const byte kScene5010SwitchLeadFrameCounts[] = { 0x23, 0x13, 0x05 };
const byte kScene5010SwitchCenterFrames[] = { 0x23, 0x32, 0x41 };
const byte kScene5010SwitchTailFrames[] = { 0x00, 0x0f, 0x1e };

enum Scene5010LayerId {
	kScene5010SwitchLayer,
	kScene5010SwitchPanelAnimationLayer
};

const SceneLayerSpec kScene5010LayerSpecs[] = {
	{kSceneAnimationScenePlaced, 6, kScene5010SwitchDescriptorCount,
		nullptr, 0, true, 0},
	{kSceneAnimationScenePlaced, 0, 0, nullptr, 0, false, 0}
};

enum {
	kScene5010NoMovingSelector,
	kScene5010MovingRow,
	kScene5010MovingColumn
};

PlayableSceneConfig scene5010Config() {
	PlayableSceneConfig config(5010,
		SceneResourceLayout(5, 5, 27),
		SceneViewport(kScene5010ViewportXOffset, kScene5010ViewportMinXOffset, kScene5010ViewportMaxXOffset),
		SceneActorPose(0x263, 0x172, 4));
	config.setActorResources(kScene5010ActorBankTableEntry, kScene5010ActorPaletteTableEntry);
	config.setTextResources(kScene5010Resource003RowsOffsetIndex, kScene5010SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 8;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene5010::Scene5010(HollywoodEngine *vm) :
		PlayableScene(vm, scene5010Config()),
		_blinkPatchVisible(false),
		_switchPanelActive(false),
		_switchPanelMovingSelector(kScene5010NoMovingSelector),
		_switchPanelMovingSelectorVisible(true),
		_switchPanelDisplayedRow(0),
		_switchPanelDisplayedColumn(0) {
	_sceneLayers.configure(kScene5010LayerSpecs);
}

void Scene5010::initializeCustomPreviewState() {
	GameplayState &state = _vm->gameState();
	const uint16 stateId = state.mainFlowStateId;
	// A fresh return begins before the cart arrives; a resumed hub keeps its settled state.
	if (stateId != kScene5010FirstState && !hasSavedActiveActorPoseForCurrentState())
		state.scene5010MineCartDeparted = true;

	initializeDefaultPreviewState();
	initializeSwitchLayer();

	if (stateId == kScene5010FirstState) {
		_activeActorWorldX = 0x263;
		_activeActorWorldY = 0x172;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x229;
		_activeActorWorldY = 0x171;
		_activeActorFacing = 4;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene5010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	if (_actionOverlayPlayer.replacesActor()) {
		drawActionOverlayLayer();
		drawSceneLayer(kScene5010SwitchLayer);
		return;
	}

	drawSceneLayer(kScene5010SwitchLayer);

	if (activeWorldY < 0x16b) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		if (activeWorldY < 0x14f && _sceneChunkTable.isValidChunk(7))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
		drawSceneLayer(kScene5010SwitchLayer);
	} else {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	if (actorDrawOrderMode > 5 && actorDrawOrderMode < 9 && _sceneChunkTable.isValidChunk(13))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _sceneFramebuffer);
	if (activeWorldY < 0x138 && _sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	drawActionOverlayLayer();
	if (_blinkPatchVisible && _sceneChunkTable.isValidChunk(27))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[27], _sceneFramebuffer);
	if (_switchPanelActive)
		drawSwitchPanelOverlay();
}

void Scene5010::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene5010FirstState)
		runFirstEntrySequence();
	else
		runReturnEntrySequence();
}

void Scene5010::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

bool Scene5010::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar vagoneta (look at mine cart).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar vagoneta (use mine cart): enter mine transport.
		enterMineTransport();
		return true;
	case 303: // Mirar cartel "Diamond Creek" (look at Diamond Creek sign).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Ir/usar tunel (go/use tunnel): prepare locked mine transport branch.
		prepareMineTransport(true);
		return true;
	case 305: // Mirar tunel oscuro (look at dark tunnel).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 306: // Usar/entrar tunel oscuro (use/enter dark tunnel): short transport branch.
		prepareMineTransport(false);
		return true;
	case 307: // Mirar tunel oscuro (look at dark tunnel), duplicate hotspot.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Mirar sistema de agujas (look at switch system).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Usar sistema de agujas (use switch system): choose mine destination.
		runSwitchPanel();
		return true;
	case 310: // Mirar sistema de agujas (look at switch system), duplicate hotspot.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 311: // Usar sistema de agujas (use switch system), duplicate hotspot.
		runSwitchPanel();
		return true;
	case 312: // Ir a camino (go to road): open Ron's destination selector.
		requestTravelScreenSelection(5);
		return true;
	default:
		return false;
	}
}

bool Scene5010::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 3 && nextRegion == 4) {
		requestedFacing = 5;
		return true;
	}
	if ((currentRegion == 4 && nextRegion == 5) ||
			(currentRegion == 5 && nextRegion == 6) ||
			(currentRegion == 6 && nextRegion == 7)) {
		copySlopeStepDeltasFromSet5A(0x3c);
		requestedFacing = 5;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 7 && nextRegion == 8) {
		requestedFacing = 5;
		return true;
	}
	if (currentRegion == 8 && nextRegion == 7) {
		requestedFacing = 4;
		return true;
	}
	if (currentRegion == 7 && nextRegion == 6) {
		requestedFacing = 2;
		return true;
	}
	if ((currentRegion == 6 && nextRegion == 5) ||
			(currentRegion == 5 && nextRegion == 4) ||
			(currentRegion == 4 && nextRegion == 3)) {
		copySlopeStepDeltasFromSet5A(0x18);
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 3 && nextRegion == 2) {
		requestedFacing = 2;
		return true;
	}

	return false;
}

bool Scene5010::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;

	if (currentRegion == 8) {
		requestedFacing = 1;
		return true;
	}

	return restoredStepDeltas;
}

bool Scene5010::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	const uint firstPatchChunk = state.scene5010MineCartDeparted ? 9 : 10;
	const uint secondPatchChunk = state.scene5010MineCartDeparted ? 12 : 11;
	if (_sceneChunkTable.isValidChunk(firstPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[firstPatchChunk], _baseFramebuffer);
	if (_sceneChunkTable.isValidChunk(secondPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[secondPatchChunk], _baseFramebuffer);
	if (state.scene5010SwitchPanelSeen &&
			_metadata.size() > kSceneItemDefaultStrip + 5)
		_metadata[kSceneItemDefaultStrip + 5] = 5;

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (!state.scene5010MineTransportReady) {
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x0d, 0);
	} else {
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x0d, 1);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x19, 0);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x21, 0);
	}

	initializeSwitchLayer();
	return true;
}

AmbientAudioProfile Scene5010::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene5010FirstAmbientSoundCue,
		kScene5010AmbientSoundCueCount, 20, 1,
		kScene5010FirstAmbientMusicCue, kScene5010AmbientMusicCueCount, 100, 20);
}

void Scene5010::initializeSwitchLayer() {
	_sceneLayers.resetLayer(kScene5010SwitchLayer, switchDescriptorIndex());
	_sceneLayers.setLayerVisible(kScene5010SwitchLayer, true);
}

byte Scene5010::switchDescriptorIndex() const {
	const GameplayState &state = _vm->gameState();
	return (byte)(CLIP<byte>(state.scene5010SwitchRow, 0, 2) * 3 +
		CLIP<byte>(state.scene5010SwitchColumn, 0, 2));
}

void Scene5010::copySlopeStepDeltasFromSet5A(uint targetFirstOffset) {
	const uint sourceFirstOffset = 0x0c;
	const uint count = 0x0c;
	for (uint i = 0; i < count &&
			targetFirstOffset + i < _actorPathStepDeltas.size() &&
			sourceFirstOffset + i < ARRAYSIZE(kActorPathStepDeltaTableSet5A); ++i)
		_actorPathStepDeltas[targetFirstOffset + i] = kActorPathStepDeltaTableSet5A[sourceFirstOffset + i];
}

void Scene5010::runFirstEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (!state.seenScene5010EntryLine) {
		state.scene5010SwitchRow = (byte)_random.getRandomNumber(2);
		state.scene5010SwitchColumn = (byte)_random.getRandomNumber(2);
		initializeSwitchLayer();
		runEntryPathWithFinalFacing(0x27d, 0x104, 2, 0x263, 0x172, 4);
		if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
			return;
		beginSecondarySpeechLine(0, 0);
		state.seenScene5010EntryLine = true;
		return;
	}

	initializeSwitchLayer();
	runEntryPathWithFinalFacing(0x27d, 0x104, 2, 0x263, 0x172, 4);
}

void Scene5010::runReturnEntrySequence() {
	GameplayState &state = _vm->gameState();
	initializeSwitchLayer();
	runMineCartArrival();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	const byte finalFacing = state.scene5010MineTransportState == 1 ||
		state.scene5010MineTransportState == 3 ||
		state.scene5010MineTransportState == 4 ? 3 : 2;
	if (state.scene5010MineTransportState == 4)
		runReturnShake();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
	walkActiveActorTo(0x229, 0x171, finalFacing, 0, false);

	if (state.scene5010MineTransportState == 1) {
		beginSecondarySpeechLine(2, 1);
		state.scene5010MineTransportState = 2;
	} else if (state.scene5010MineTransportState == 3) {
		beginSecondarySpeechLine(2, 2);
		state.scene5010MineTransportState = 2;
	} else if (state.scene5010MineTransportState == 4) {
		beginSecondarySpeechLine(8, 0);
		state.scene5010MineTransportState = 2;
	}
}

void Scene5010::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing, int targetX, int targetY,
		byte finalFacing) {
	setActiveActorPose(startX, startY, startFacing);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;

	walkActiveActorTo(targetX, targetY, finalFacing, 0, false);
	_activeActorFacing = finalFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene5010::runMineCartArrival() {
	setActiveActorPose(0x21c, 0x14c, 2);

	runActorReplacement(ActionOverlaySpec(15, kScene5010TransportArrivalDescriptorCount,
		kScene5010FrameMillis)
		.soundAt(0x1e, 0x16)
		.fadeFromBlackAt(0)
		.noFinalFrameDelay()
		.noRedrawAtEnd());
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_vm->gameState().scene5010MineCartDeparted = false;
	applySceneStateToHotspotsAndPatches(6);
	runActorReplacement(ActionOverlaySpec(8, kScene5010TransportPrepDescriptorCount,
		kScene5010FrameMillis).reverse()
		.noFinalFrameDelay());
}

void Scene5010::runReturnShake() {
	_soundBank0.playSample(0x17, 100);
	const uint16 baseOffset = _viewportXOffset;
	for (uint frame = 0; frame < kScene5010ReturnShakeFrameCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		if (pollEvents(false))
			break;
		_random.getRandomNumber(2);
		const int offset = ((int)_random.getRandomNumber(2) - 1) * kScene5010ReturnShakeOffset;
		_viewportXOffset = (uint16)CLIP<int>((int)baseOffset + offset, 0,
			HollywoodEngine::kSceneBufferWidth - HollywoodEngine::kScreenWidth);
		presentFrame();
	}
	_viewportXOffset = baseOffset;
	presentFrame();
}

void Scene5010::prepareMineTransport(bool showBlinkPatch) {
	GameplayState &state = _vm->gameState();
	if (state.scene5010MineTransportReady) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	if (showBlinkPatch) {
		if (waitSceneMillis(1000, false))
			return;
		_blinkPatchVisible = _sceneChunkTable.isValidChunk(27);
		drawPlayableComposite();
		presentFrame();
		const bool stopped = waitSceneMillis(5000, false);
		_blinkPatchVisible = false;
		drawPlayableComposite();
		presentFrame();
		if (stopped || waitSceneMillis(1000, false))
			return;
	} else if (waitSceneMillis(3000, false)) {
		return;
	}

	walkActiveActorTo(0x11e, 0x16b, 2, 0, false);
	beginSecondarySpeechLine(4, 1);
	state.scene5010MineTransportReady = true;
	applySceneStateToHotspotsAndPatches(2);
}

void Scene5010::enterMineTransport() {
	GameplayState &state = _vm->gameState();
	if (!state.scene5010MineTransportReady) {
		beginSecondarySpeechLine(2, 3);
		return;
	}

	if (!state.scene5010DestinationTableInitialized) {
		beginSecondarySpeechLine(2, 0);
		state.scene5010MineTransportState = 1;
		ensureMineDestinationTable();
	}

	runActorReplacement(ActionOverlaySpec(8, kScene5010TransportPrepDescriptorCount, kScene5010FrameMillis)
		.noRedrawAtEnd());

	state.scene5010MineCartDeparted = true;
	applySceneStateToHotspotsAndPatches(6);

	runActorReplacement(ActionOverlaySpec(16, kScene5010TransportDepartDescriptorCount,
		kScene5010FrameMillis)
		.soundAt(0x1e, 0x15)
		.noRedrawAtEnd());

	state.mainFlowStateId = mineDestinationForCurrentSwitch();
}

void Scene5010::ensureMineDestinationTable() {
	GameplayState &state = _vm->gameState();
	if (state.scene5010DestinationTableInitialized)
		return;

	bool usedValues[11];
	memset(usedValues, 0, sizeof(usedValues));
	const byte selectedSlot = switchDescriptorIndex();
	for (uint slot = 0; slot < ARRAYSIZE(state.scene5010DestinationStateBySwitchSlot); ++slot)
		state.scene5010DestinationStateBySwitchSlot[slot] = 0;

	state.scene5010DestinationStateBySwitchSlot[selectedSlot] = 5060;
	usedValues[6] = true;

	for (uint slot = 0; slot < ARRAYSIZE(state.scene5010DestinationStateBySwitchSlot); ++slot) {
		if (slot == selectedSlot)
			continue;

		byte value = 0;
		do {
			value = (byte)(_random.getRandomNumber(8) + 2);
		} while (usedValues[value]);

		// Value 5 maps to state 0x13ba/5050. In the original this is not the
		// playable trophy room entry: scene 5050 immediately runs the cart
		// bounce-back transition. The playable entry is 0x13bb from Karl's hole.
		usedValues[value] = true;
		state.scene5010DestinationStateBySwitchSlot[slot] = (uint16)(5000 + value * 10);
	}

	state.scene5010DestinationTableInitialized = true;
}

uint16 Scene5010::mineDestinationForCurrentSwitch() const {
	const GameplayState &state = _vm->gameState();
	const byte slot = switchDescriptorIndex();
	if (slot < ARRAYSIZE(state.scene5010DestinationStateBySwitchSlot) &&
			state.scene5010DestinationStateBySwitchSlot[slot] != 0)
		return state.scene5010DestinationStateBySwitchSlot[slot];
	return 5060;
}

void Scene5010::runSwitchPanel() {
	GameplayState &state = _vm->gameState();
	_switchPanelActive = true;
	_switchPanelMovingSelector = kScene5010NoMovingSelector;
	_switchPanelMovingSelectorVisible = true;
	_switchPanelDisplayedRow = CLIP<byte>(state.scene5010SwitchRow, 0, 2);
	_switchPanelDisplayedColumn = CLIP<byte>(state.scene5010SwitchColumn, 0, 2);
	clearSceneLayer(kScene5010SwitchPanelAnimationLayer);
	drawSwitchPanelFrame();
	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	g_system->updateScreen();
	bool done = false;
	while (!done && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollSwitchPanelEvent(done)) {
			_vm->cursor()->leaveInteractiveMode();
			_switchPanelActive = false;
			clearSceneLayer(kScene5010SwitchPanelAnimationLayer);
			return;
		}
		_vm->cursor()->advance(10);
		g_system->updateScreen();
		updateAmbientAudioAndMusicCues(10);
		g_system->delayMillis(10);
	}

	_vm->cursor()->leaveInteractiveMode();
	_switchPanelActive = false;
	clearSceneLayer(kScene5010SwitchPanelAnimationLayer);
	state.scene5010SwitchPanelSeen = true;
	applySceneStateToHotspotsAndPatches(3);
	drawPlayableComposite();
	presentFrame();
}

void Scene5010::drawSwitchPanelFrame() {
	drawPlayableComposite();
	presentFrame();
}

void Scene5010::drawSwitchPanelOverlay() {
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint rowOffset = y * HollywoodEngine::kSceneBufferWidth;
		for (uint x = _viewportXOffset; x < _viewportXOffset + HollywoodEngine::kScreenWidth; ++x) {
			const uint offset = rowOffset + x;
			pixels[offset] = _presentationPaletteRemapTable[pixels[offset]];
		}
	}

	if (_sceneChunkTable.isValidChunk(20))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[20], _sceneFramebuffer);

	const uint rowChunk = 21 + _switchPanelDisplayedRow;
	if ((_switchPanelMovingSelector != kScene5010MovingRow || _switchPanelMovingSelectorVisible) &&
			_sceneChunkTable.isValidChunk(rowChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[rowChunk], _sceneFramebuffer);

	const uint columnChunk = 24 + _switchPanelDisplayedColumn;
	if ((_switchPanelMovingSelector != kScene5010MovingColumn || _switchPanelMovingSelectorVisible) &&
			_sceneChunkTable.isValidChunk(columnChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[columnChunk], _sceneFramebuffer);
	drawSceneLayer(kScene5010SwitchPanelAnimationLayer);
}

byte Scene5010::switchPanelMaskPixelAt(uint16 screenX, uint16 screenY) const {
	if (!_sceneChunkTable.isValidChunk(17))
		return 0;

	const uint sceneX = screenX + viewportXOffset();
	const uint sceneY = screenY + viewportYOffset();
	if (sceneX >= HollywoodEngine::kSceneBufferWidth ||
			sceneY >= HollywoodEngine::kSceneBufferHeight)
		return 0;

	const uint offset = sceneY * HollywoodEngine::kSceneBufferWidth + sceneX;
	if (offset >= _sceneChunkTable.sizes[17] ||
			_resourceChunkOffsets[17] + offset >= _resourceArena.size())
		return 0;

	const uint chunkOffset = _resourceChunkOffsets[17] + offset;
	return _resourceArena[chunkOffset];
}

void Scene5010::handleSwitchPanelChoice(byte choice) {
	if (choice == 0 || choice > 6)
		return;

	GameplayState &state = _vm->gameState();
	byte currentValue;
	byte targetValue;
	uint animationChunk;
	if (choice < 4) {
		currentValue = CLIP<byte>(state.scene5010SwitchRow, 0, 2);
		targetValue = choice - 1;
		if (currentValue == targetValue)
			return;
		_switchPanelMovingSelector = kScene5010MovingRow;
		animationChunk = 18;
	} else {
		currentValue = CLIP<byte>(state.scene5010SwitchColumn, 0, 2);
		targetValue = choice - 4;
		if (currentValue == targetValue)
			return;
		_switchPanelMovingSelector = kScene5010MovingColumn;
		animationChunk = 19;
	}

	_switchPanelMovingSelectorVisible = true;
	uint hideStaticFrame = 0;
	uint showStaticFrame = 0;
	Common::Array<byte> frameMap = buildSwitchPanelAnimation(currentValue, targetValue,
		hideStaticFrame, showStaticFrame);
	AnimationFrameRange range(1, frameMap.size() - 1, kScene5010SwitchFrameMillis);
	range.soundAt(hideStaticFrame, 0x24, 15)
		.commitAt(hideStaticFrame, _switchPanelMovingSelectorVisible, false)
		.soundAt(showStaticFrame, 0x24, 15);
	if (_switchPanelMovingSelector == kScene5010MovingRow)
		range.commitAt(showStaticFrame, _switchPanelDisplayedRow, targetValue);
	else
		range.commitAt(showStaticFrame, _switchPanelDisplayedColumn, targetValue);
	range.commitAt(showStaticFrame, _switchPanelMovingSelectorVisible, true)
		.unskippable();
	BlockingSequence(*this).resourceLayerFrames(kScene5010SwitchPanelAnimationLayer, animationChunk,
		kScene5010SwitchAnimationDescriptorCount, frameMap.data(), frameMap.size(),
		range);

	if (_switchPanelMovingSelector == kScene5010MovingRow)
		state.scene5010SwitchRow = targetValue;
	else
		state.scene5010SwitchColumn = targetValue;
	_switchPanelMovingSelector = kScene5010NoMovingSelector;
	_switchPanelMovingSelectorVisible = true;
	_switchPanelDisplayedRow = CLIP<byte>(state.scene5010SwitchRow, 0, 2);
	_switchPanelDisplayedColumn = CLIP<byte>(state.scene5010SwitchColumn, 0, 2);
	initializeSwitchLayer();
	drawSwitchPanelFrame();
}

Common::Array<byte> Scene5010::buildSwitchPanelAnimation(byte currentValue, byte targetValue,
		uint &hideStaticFrame, uint &showStaticFrame) const {
	// Selector positions occupy separate frame bands; the two holds are where
	// the original swaps the old and new static needle patches.
	Common::Array<byte> frameMap;

	frameMap.push_back(0x22);
	for (uint i = 1; i <= kScene5010SwitchLeadFrameCounts[currentValue]; ++i)
		frameMap.push_back((byte)(0x23 - i));

	hideStaticFrame = frameMap.size();
	for (uint i = 0; i < 7; ++i)
		frameMap.push_back(kScene5010SwitchCenterFrames[currentValue]);

	const int direction = targetValue > currentValue ? 1 : -1;
	for (int i = 1; i <= 5; ++i)
		frameMap.push_back((byte)(kScene5010SwitchCenterFrames[currentValue] + direction * i));
	for (int i = 4; i >= 0; --i)
		frameMap.push_back((byte)(kScene5010SwitchCenterFrames[currentValue] + direction * i));
	for (int frame = kScene5010SwitchCenterFrames[currentValue] + direction;; frame += direction) {
		frameMap.push_back((byte)frame);
		if (frame == kScene5010SwitchCenterFrames[targetValue])
			break;
	}
	for (int i = 1; i <= 5; ++i)
		frameMap.push_back((byte)(kScene5010SwitchCenterFrames[targetValue] - direction * i));
	for (int i = 4; i >= 0; --i)
		frameMap.push_back((byte)(kScene5010SwitchCenterFrames[targetValue] - direction * i));
	for (uint i = 0; i < 6; ++i)
		frameMap.push_back(kScene5010SwitchCenterFrames[targetValue]);

	showStaticFrame = frameMap.size();
	for (uint frame = kScene5010SwitchTailFrames[targetValue]; frame <= 0x22; ++frame)
		frameMap.push_back((byte)frame);
	return frameMap;
}

void Scene5010::activateSwitchPanelAtCursor(bool &done) {
	const byte maskPixel = switchPanelMaskPixelAt(
		_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY());
	if (maskPixel == 7) {
		done = true;
		return;
	}
	if (maskPixel == 0 || maskPixel > 6)
		return;

	_vm->cursor()->leaveInteractiveMode();
	handleSwitchPanelChoice(maskPixel);
	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
		g_system->updateScreen();
	}
}

bool Scene5010::pollSwitchPanelEvent(bool &done) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			return true;
		case Common::EVENT_MAINMENU:
			_vm->openMainMenuDialog();
			if (_vm->isSceneRestartRequested())
				return true;
			_displayPalette.markAllDirty();
			drawSwitchPanelFrame();
			_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				done = true;
			} else if (!event.kbdRepeat &&
					(event.kbd.keycode == Common::KEYCODE_RETURN ||
					 event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
					 event.kbd.keycode == Common::KEYCODE_SPACE)) {
				activateSwitchPanelAtCursor(done);
			}
			break;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			break;
		case Common::EVENT_LBUTTONDOWN:
			_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
			activateSwitchPanelAtCursor(done);
			break;
		case Common::EVENT_RBUTTONDOWN:
			done = true;
			break;
		default:
			break;
		}
	}

	return false;
}

} // End of namespace Hollywood
