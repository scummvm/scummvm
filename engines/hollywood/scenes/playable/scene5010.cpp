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

#include "hollywood/scenes/playable/scene5010.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

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
const uint kScene5010SwitchDescriptorCount = 9;
const uint kScene5010TransportPrepDescriptorCount = 8;
const uint kScene5010TransportDepartDescriptorCount = 0x3f;
const byte kScene5010FirstAmbientSoundCue = 0x25;
const byte kScene5010AmbientSoundCueCount = 3;
const byte kScene5010FirstAmbientMusicCue = 0x0b;
const byte kScene5010AmbientMusicCueCount = 5;

const byte kScene5010TransportPrepFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7
};

static PlayableSceneConfig scene5010Config() {
	PlayableSceneConfig config(5010,
		SceneResourceLayout(5, 5, 27),
		SceneViewport(kScene5010ViewportXOffset, kScene5010ViewportMinXOffset, kScene5010ViewportMaxXOffset),
		SceneActorPose(0x263, 0x172, 4));
	config.setActorResources(kScene5010ActorBankTableEntry, kScene5010ActorPaletteTableEntry);
	config.setTextResources(kScene5010Resource003RowsOffsetIndex, kScene5010SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 8;
	return config;
}

Scene5010::Scene5010(HollywoodEngine *vm) :
		PlayableScene(vm, scene5010Config()),
		_switchLayer() {
}

void Scene5010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	initializeSwitchLayer();

	const uint16 stateId = _vm->gameState().mainFlowStateId;
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
	drawResourceSpriteLayer(_switchLayer);

	if (activeWorldY < 0x16b) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		if (activeWorldY < 0x14f && _sceneChunkTable.isValidChunk(7))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
		drawResourceSpriteLayer(_switchLayer);
	} else {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	if (actorDrawOrderMode > 5 && actorDrawOrderMode < 9 && _sceneChunkTable.isValidChunk(13))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _sceneFramebuffer);
	if (activeWorldY < 0x138 && _sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	drawActionOverlayLayer();
}

void Scene5010::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene5010FirstState)
		runFirstEntrySequence();
	else
		runReturnEntrySequence();
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
	case 312: // Unused no-op scene action slot after setup.
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
		copyStepDeltasFromSet87(0x3c, 0x47);
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
		copyStepDeltasFromSet87(0x18, 0x23);
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
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (selector == 6 || selector == 0xff) {
		const uint firstPatchChunk = state.scene5010MineCartDeparted ? 9 : 10;
		const uint secondPatchChunk = state.scene5010MineCartDeparted ? 12 : 11;
		if (_sceneChunkTable.isValidChunk(firstPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[firstPatchChunk], _baseFramebuffer);
		if (_sceneChunkTable.isValidChunk(secondPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[secondPatchChunk], _baseFramebuffer);
	}
	if ((selector == 3 || selector == 0xff) && state.scene5010SwitchPanelSeen &&
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
	_switchLayer.configure(6, kScene5010SwitchDescriptorCount, nullptr, 0);
	_switchLayer.visible = true;
	_switchLayer.setFrame(switchDescriptorIndex());
	_switchLayer.hasPreviousDescriptor = false;
}

byte Scene5010::switchDescriptorIndex() const {
	const GameplayState &state = _vm->gameState();
	return (byte)(CLIP<byte>(state.scene5010SwitchRow, 0, 2) * 3 +
		CLIP<byte>(state.scene5010SwitchColumn, 0, 2));
}

void Scene5010::copyStepDeltasFromSet87(uint firstOffset, uint lastOffset) {
	for (uint offset = firstOffset; offset <= lastOffset &&
			offset < _actorPathStepDeltas.size() &&
			offset < ARRAYSIZE(kActorPathStepDeltaTableSet87); ++offset) {
		_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
	}
}

void Scene5010::runFirstEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (!state.seenScene5010EntryLine) {
		state.scene5010SwitchRow = (byte)_random.getRandomNumber(2);
		state.scene5010SwitchColumn = (byte)_random.getRandomNumber(2);
		initializeSwitchLayer();
		runEntryPathWithFinalFacing(0x27d, 0x104, 2, 0x263, 0x172, 4);
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
	runEntryPathWithFinalFacing(0x21c, 0x14c, 4, 0x229, 0x171, 4);

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

void Scene5010::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing, int targetX, int targetY, byte finalFacing) {
	setActiveActorPose(startX, startY, startFacing);
	drawPlayableComposite();
	presentFrame();

	walkActiveActorTo(targetX, targetY, finalFacing, 0, false);
	_activeActorFacing = finalFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene5010::prepareMineTransport(bool showBlinkPatch) {
	GameplayState &state = _vm->gameState();
	if (state.scene5010MineTransportReady) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	if (showBlinkPatch && _sceneChunkTable.isValidChunk(27)) {
		if (waitSceneMillis(1000))
			return;
		drawPlayableComposite();
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[27], _sceneFramebuffer);
		presentFrame();
		if (waitSceneMillis(5000))
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

	runActorReplacement(ActionOverlaySpec(8, kScene5010TransportPrepDescriptorCount,
		kScene5010TransportPrepFrameMap, ARRAYSIZE(kScene5010TransportPrepFrameMap), kScene5010FrameMillis));

	state.scene5010MineCartDeparted = true;
	applySceneStateToHotspotsAndPatches(6);

	Common::Array<byte> frameMap;
	frameMap.resize(kScene5010TransportDepartDescriptorCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;
	runActorReplacement(ActionOverlaySpec(16, kScene5010TransportDepartDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5010FrameMillis)
		.soundAt(0x1e, 0x15));

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
	drawSwitchPanelFrame();
	bool done = false;
	while (!done && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollSwitchPanelEvent(done))
			return;
		updateAmbientAudioAndMusicCues(10);
		g_system->delayMillis(10);
	}

	_vm->gameState().scene5010SwitchPanelSeen = true;
	applySceneStateToHotspotsAndPatches(3);
	drawPlayableComposite();
	presentFrame();
}

void Scene5010::drawSwitchPanelFrame() {
	drawPlayableComposite();
	if (_sceneChunkTable.isValidChunk(20))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[20], _sceneFramebuffer);

	const uint rowChunk = 21 + CLIP<byte>(_vm->gameState().scene5010SwitchRow, 0, 2);
	if (_sceneChunkTable.isValidChunk(rowChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[rowChunk], _sceneFramebuffer);

	const uint columnChunk = 24 + CLIP<byte>(_vm->gameState().scene5010SwitchColumn, 0, 2);
	if (_sceneChunkTable.isValidChunk(columnChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[columnChunk], _sceneFramebuffer);
	presentFrame();
}

byte Scene5010::switchPanelMaskPixelAt(uint16 screenX, uint16 screenY) const {
	if (!_sceneChunkTable.isValidChunk(17))
		return 0;

	const uint sceneX = screenX + viewportXOffset();
	const uint sceneY = screenY + viewportYOffset();
	const uint offset = sceneY * HollywoodEngine::kSceneBufferWidth + sceneX;
	const uint chunkOffset = _resourceChunkOffsets[17] + offset;
	if (sceneX >= HollywoodEngine::kSceneBufferWidth ||
			sceneY >= HollywoodEngine::kSceneBufferHeight ||
			chunkOffset >= _resourceArena.size())
		return 0;

	return _resourceArena[chunkOffset];
}

void Scene5010::handleSwitchPanelChoice(byte choice) {
	if (choice == 0 || choice > 6)
		return;

	GameplayState &state = _vm->gameState();
	if (choice < 4) {
		const byte row = choice - 1;
		if (state.scene5010SwitchRow == row)
			return;
		state.scene5010SwitchRow = row;
	} else {
		const byte column = choice - 4;
		if (state.scene5010SwitchColumn == column)
			return;
		state.scene5010SwitchColumn = column;
	}

	_soundBank0.playSample(0x24, 15);
	initializeSwitchLayer();
	drawSwitchPanelFrame();
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
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
					event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_SPACE)
				done = true;
			break;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			break;
		case Common::EVENT_LBUTTONDOWN: {
			_vm->cursor()->updatePosition(event.mouse);
			const byte maskPixel = switchPanelMaskPixelAt((uint16)event.mouse.x, (uint16)event.mouse.y);
			if (maskPixel == 7)
				done = true;
			else if (maskPixel > 0 && maskPixel < 7)
				handleSwitchPanelChoice(maskPixel);
			break;
		}
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
