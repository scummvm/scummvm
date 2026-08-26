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

#include "hollywood/scenes/playable/scene3060.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene3060EntryFromSecretPassageState = 0x0bf5;
const uint16 kScene3050EntryFromScene3060State = 0x0beb;
const uint16 kScene3070State = 0x0bfe;
const uint kScene3060ActorBankTableEntry = 0x0000;
const uint kScene3060ActorPaletteTableEntry = 0x00cc;
const uint kScene3060Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3060SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3060FrontFrameMillis = 75;
const uint32 kScene3060ButtonFrameMillis = 75;
const uint32 kScene3060SecretDoorFrameMillis = 125;
const uint32 kScene3060RedGlobeInitialStepMillis = 30;
const uint32 kScene3060RedGlobeAccelerationMillis = 5;
const byte kScene3060RedGlobeRevolutionCount = 6;
const byte kScene3060RotateGlobeForwardHook = 1;
const byte kScene3060RotateGlobeBackwardHook = 2;
const uint kScene3060FrontDescriptorCount = 0x13;
const uint kScene3060GlobeDescriptorCount = 0x1e;
const uint kScene3060ButtonDescriptorCount = 5;
const uint kScene3060SecretDoorDescriptorCount = 0x0e;

const byte kScene3060FrontFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18
};

const byte kScene3060GlobeFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29
};

const byte kScene3060LeftButtonFrameMap[] = { 0, 1, 2, 2, 1, 0 };
const byte kScene3060RightButtonFrameMap[] = { 0, 1, 4, 4, 1, 0 };
const byte kScene3060RedButtonFrameMap[] = { 0, 1, 3, 3, 1, 0 };
const byte kScene3060SecretDoorRevealFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

static PlayableSceneConfig scene3060Config() {
	PlayableSceneConfig config(3060,
		SceneResourceLayout(10, 5, 9),
		SceneViewport(0),
		SceneActorPose(0x22d, 0x156, 4));
	config.setActorResources(kScene3060ActorBankTableEntry, kScene3060ActorPaletteTableEntry);
	config.setTextResources(kScene3060Resource003RowsOffsetIndex, kScene3060SpeechCueDescriptorTableOffset);
	config.useActorDepthTest = true;
	return config;
}

Scene3060::Scene3060(HollywoodEngine *vm) :
		PlayableScene(vm, scene3060Config()),
		_frontChannel(),
		_frontLayer(),
		_globeLayer(),
		_buttonLayer(),
		_frontLayerMode(0),
		_secretDoorRevealActive(false) {
	_frontLayer.configure(5, kScene3060FrontDescriptorCount,
		kScene3060FrontFrameMap, ARRAYSIZE(kScene3060FrontFrameMap));
	_globeLayer.configure(6, kScene3060GlobeDescriptorCount,
		kScene3060GlobeFrameMap, ARRAYSIZE(kScene3060GlobeFrameMap));
	_buttonLayer.configure(7, kScene3060ButtonDescriptorCount,
		kScene3060LeftButtonFrameMap, ARRAYSIZE(kScene3060LeftButtonFrameMap));
}

void Scene3060::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	if (_vm->gameState().mainFlowStateId == kScene3060EntryFromSecretPassageState) {
		_activeActorWorldX = 0x1aa;
		_activeActorWorldY = 0x161;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x22d;
		_activeActorWorldY = 0x156;
		_activeActorFacing = 4;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene3060::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	updateSceneDepthThresholds(actorDrawOrderMode);

	if (_secretDoorRevealActive) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		drawResourceSpriteLayer(_frontLayer);
		drawActionOverlayLayer();
		drawResourceSpriteLayer(_globeLayer);
		return;
	}

	if (actorDrawOrderMode < 4)
		drawResourceSpriteLayer(_globeLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (actorDrawOrderMode >= 4)
		drawResourceSpriteLayer(_globeLayer);

	drawActionOverlayLayer();
	drawResourceSpriteLayer(_frontLayer);
	drawResourceSpriteLayer(_buttonLayer);
}

void Scene3060::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene3060EntryFromSecretPassageState)
		runEntryFromSecretPassage();
	else
		runEntryFromScene3050();
}

bool Scene3060::prepareCustomGameplayLoop() {
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene3060::advanceCustomGameplayLoop(uint32 delta) {
	if (_secretDoorRevealActive)
		return true;

	updateAmbientAudioAndMusicCues(delta);
	advanceFrontLayer(delta);
	return true;
}

bool Scene3060::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Ir a recibidor (go to the hall): return to Scene 3050.
		state.mainFlowStateId = kScene3050EntryFromScene3060State;
		return true;
	case 302: // Mirar recibidor (look toward the hall).
		beginSecondarySpeechLine(0, 0);
		return true;
	case 303: // Ir a puerta secreta / cuadro central (go to secret passage).
		if (state.scene3060SecretDoorRevealState == 1) {
			if (!walkActiveActorTo(0x0f8, 0x149, 5, 0))
				return true;
			beginSecondarySpeechLine(1, 0);
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				return true;
			if (!walkActiveActorTo(0x08a, 0x184, 0xff, 0))
				return true;
			state.scene3060SecretDoorRevealState = 2;
		}
		state.mainFlowStateId = kScene3070State;
		return true;
	case 304: // Mirar puerta secreta / cuadro central (look at secret door / painting).
		beginSecondarySpeechLine(2, state.scene3060SecretDoorRevealState == 1 ? 0 : 1);
		return true;
	case 305: // Coger cuadro/título (take a painting/degree): fixed refusal.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 306: // Mirar el primer título (look at the first degree).
		beginSecondarySpeechLine(4, 0);
		if ((state.scene3060InspectedTitleFlags & 1) == 0) {
			state.scene3060InspectedTitleFlags |= 1;
			applySceneStateToHotspotsAndPatches(1);
		}
		return true;
	case 307: // Mirar el segundo título (look at the second degree).
		beginSecondarySpeechLine(5, 0);
		if ((state.scene3060InspectedTitleFlags & 2) == 0) {
			state.scene3060InspectedTitleFlags |= 2;
			applySceneStateToHotspotsAndPatches(2);
		}
		return true;
	case 308: // Mirar el tercer título (look at the third degree).
		beginSecondarySpeechLine(6, 0);
		if ((state.scene3060InspectedTitleFlags & 4) == 0) {
			state.scene3060InspectedTitleFlags |= 4;
			applySceneStateToHotspotsAndPatches(3);
		}
		return true;
	case 309: // Mirar el cuarto título (look at the fourth degree).
		beginSecondarySpeechLine(7, 0);
		if ((state.scene3060InspectedTitleFlags & 8) == 0) {
			state.scene3060InspectedTitleFlags |= 8;
			applySceneStateToHotspotsAndPatches(4);
		}
		return true;
	case 310: // Ir a/Mirar libros (go to/look at the books).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Coger/Abrir libros (take/open the books).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Usar libros (use the books).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Mirar bola del mundo (look at the globe).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314: // Usar bola del mundo (use the globe).
		beginSecondarySpeechLine(12, 0);
		return true;
	case 315: // Mirar cualquiera de los botones (look at any button).
		beginSecondarySpeechLine(13, 0);
		return true;
	case 316: // Usar botón izquierdo (use left button): rotate globe forward.
		runGlobeButtonSequence(1, kScene3060LeftButtonFrameMap, ARRAYSIZE(kScene3060LeftButtonFrameMap), 1);
		return true;
	case 317: // Usar botón derecho (use right button): rotate globe backward.
		runGlobeButtonSequence(2, kScene3060RightButtonFrameMap, ARRAYSIZE(kScene3060RightButtonFrameMap), -1);
		return true;
	case 318: // Usar botón rojo (use red button): validate globe-button sequence.
		runRedButtonSequence();
		return true;
	default:
		return false;
	}
}

bool Scene3060::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	bool reloadHotspots = false;
	if (selector == 0xff || selector == 0) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		promoteSecretDoorHotspots();

		if (_vm->gameState().scene3060SecretDoorRevealState != 0 &&
				_sceneChunkTable.isValidChunk(9))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);

		rebuildWalkableMask();
		reloadHotspots = true;
	}

	if (selector == 0xff || (selector >= 1 && selector <= 4)) {
		updateTitleCaptionRows(selector);
		reloadHotspots = true;
	}

	if (selector == 0xff || selector == 5) {
		updateGlobeButtonDefaultStrips();
		reloadHotspots = true;
	}

	if (reloadHotspots) {
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		patchSecretDoorMovementModes();
	}

	return true;
}

bool Scene3060::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX < 0x067)
		targetX = 0x067;

	while (targetY < 0x1df) {
		++targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}

	while (targetY > 0) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		--targetY;
	}

	return true;
}

bool Scene3060::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion != 5 || nextRegion != 4)
		return false;

	for (uint i = 0; i < 0x0c && 0x18 + i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[0x18 + i] = kActorPathStepDeltaTableSetB4[i];
	requestedFacing = 2;
	restoredStepDeltas = true;
	return true;
}

bool Scene3060::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)targetY;

	if (currentRegion != 5)
		return false;

	const uint sourceOffset = targetX < state.x ? 0x24 : 0;
	const uint destinationOffset = targetX < state.x ? 0x3c : 0x18;
	for (uint i = 0; i < 0x0c && sourceOffset + i < ARRAYSIZE(kActorPathStepDeltaTableSetB4) &&
			destinationOffset + i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[destinationOffset + i] = kActorPathStepDeltaTableSetB4[sourceOffset + i];
	requestedFacing = targetX < state.x ? 5 : 2;
	restoredStepDeltas = true;
	return true;
}

AmbientAudioProfile Scene3060::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

bool Scene3060::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene3060::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

void Scene3060::resetAnimationLayers() {
	_frontChannel.reset(0, kScene3060FrontFrameMillis);
	_frontLayer.visible = true;
	_globeLayer.visible = true;
	_buttonLayer.visible = false;
	_frontLayer.reset(0);
	_globeLayer.reset(_vm->gameState().scene3060GlobeFrame);
	_buttonLayer.reset(0);
	_frontLayerMode = 0;
	_secretDoorRevealActive = false;
}

void Scene3060::rebuildWalkableMask() {
	rebuildWalkablePaletteMask();
}

void Scene3060::copySmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene3060::updateTitleCaptionRows(byte selector) {
	const byte flags = _vm->gameState().scene3060InspectedTitleFlags;
	for (byte index = 0; index < 4; ++index) {
		if ((selector == 0xff || selector == index + 1) && (flags & (1 << index)) != 0)
			copySmallRow(12, (byte)(3 + index));
	}
}

void Scene3060::promoteSecretDoorHotspots() {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	const bool revealed = _vm->gameState().scene3060SecretDoorRevealState != 0;
	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
		if (!revealed) {
			if (originalItem == 2 || originalItem == 0x0c)
				_paletteMask[kSceneColorToItemMap + i] = 7;
		} else {
			if (originalItem == 2)
				_paletteMask[kSceneColorToItemMap + i] = 2;
			else if (originalItem == 0x0c)
				_paletteMask[kSceneColorToItemMap + i] = 0;
		}
	}
}

void Scene3060::updateGlobeButtonDefaultStrips() {
	if (!_vm->gameState().scene3060GlobeButtonsDiscovered ||
			_metadata.size() < kSceneItemDefaultStrip + HollywoodEngine::kSceneItemCount)
		return;

	_metadata[kSceneItemDefaultStrip + 9] = 5;
	_metadata[kSceneItemDefaultStrip + 10] = 5;
	_metadata[kSceneItemDefaultStrip + 11] = 5;
}

void Scene3060::patchSecretDoorMovementModes() {
	const byte state = _vm->gameState().scene3060SecretDoorRevealState;
	if (state == 0)
		return;

	_hotspots.setVerbMovementModeByGlobalRecordIndex(0x11, state == 1 ? 0 : 1);
	_hotspots.setVerbMovementModeByGlobalRecordIndex(0x4d, 0);
	_hotspots.setVerbMovementModeByGlobalRecordIndex(0x55, 0);
	_hotspots.setVerbMovementModeByGlobalRecordIndex(0x5d, 0);
}

void Scene3060::updateSceneDepthThresholds(byte actorDrawOrderMode) {
	if (_drawActorDepthYThresholds.size() <= 3)
		return;

	if (actorDrawOrderMode == 5) {
		_drawActorDepthYThresholds[2] = 0x03e7;
		_drawActorDepthYThresholds[3] = 0x03e7;
	} else {
		_drawActorDepthYThresholds[2] = 0x0172;
		_drawActorDepthYThresholds[3] = 0;
	}
}

void Scene3060::advanceFrontLayer(uint32 delta) {
	const uint frameCount = _frontChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		switch (_frontLayerMode) {
		case 0: // Advance to frame 9.
			if (_frontChannel.frameIndex < 9)
				++_frontChannel.frameIndex;
			else
				_frontLayerMode = 1;
			break;
		case 1: // Wait at frame 9.
			if (_random.getRandomNumber(199) == 0) {
				_frontLayerMode = 3;
			} else if (_random.getRandomNumber(39) == 0) {
				_frontChannel.frameIndex = 10;
				_frontLayerMode = 2;
			}
			break;
		case 2: // Wait at frame 10.
			if (_random.getRandomNumber(199) == 0) {
				_frontLayerMode = 3;
			} else if (_random.getRandomNumber(39) == 0) {
				_frontChannel.frameIndex = 9;
				_frontLayerMode = 1;
			}
			break;
		case 3: // Advance to frame 18.
			if (_frontChannel.frameIndex < 18)
				++_frontChannel.frameIndex;
			else
				_frontLayerMode = 4;
			break;
		case 4: // Wait at frame 18.
			if (_random.getRandomNumber(199) == 0) {
				_frontLayerMode = 0;
				_frontChannel.frameIndex = 0;
			}
			break;
		}
		_frontLayer.setFrame(_frontChannel.frameIndex);
	}
}

void Scene3060::runEntryFromScene3050() {
	_frontChannel.frameIndex = 18;
	_frontLayerMode = 4;
	_frontLayer.setFrame(18);
	runEntryPath(0x2e3, 0x128, 4, 0x22d, 0x156);
}

void Scene3060::runEntryFromSecretPassage() {
	_frontChannel.frameIndex = 18;
	_frontLayerMode = 4;
	_frontLayer.setFrame(18);
	runEntryPath(0x08a, 0x184, 2, 0x1aa, 0x161);
}

void Scene3060::rotateGlobe(int delta) {
	GameplayState &state = _vm->gameState();
	int nextFrame = (int)state.scene3060GlobeFrame + delta;
	while (nextFrame < 0)
		nextFrame += kScene3060GlobeDescriptorCount;
	state.scene3060GlobeFrame = (byte)(nextFrame % kScene3060GlobeDescriptorCount);
	_globeLayer.setFrame(state.scene3060GlobeFrame);
}

void Scene3060::markGlobeButtonsDiscovered() {
	GameplayState &state = _vm->gameState();
	if (state.scene3060GlobeButtonsDiscovered)
		return;

	state.scene3060GlobeButtonsDiscovered = true;
	applySceneStateToHotspotsAndPatches(5);
}

void Scene3060::recordGlobeButton(byte button) {
	GameplayState &state = _vm->gameState();
	byte repeatCount = 1;
	if (state.scene3060LastGlobePuzzleButton == button) {
		const uint pairOffset = (uint)state.scene3060GlobePuzzleRunIndex * 2 + 1;
		if (pairOffset < sizeof(state.scene3060GlobePuzzleRuns))
			repeatCount = (byte)(state.scene3060GlobePuzzleRuns[pairOffset] + 1);
	} else {
		state.scene3060GlobePuzzleRunIndex = (byte)((state.scene3060GlobePuzzleRunIndex + 1) & 3);
	}

	const uint pairOffset = (uint)state.scene3060GlobePuzzleRunIndex * 2;
	if (pairOffset + 1 < sizeof(state.scene3060GlobePuzzleRuns)) {
		state.scene3060GlobePuzzleRuns[pairOffset] = button;
		state.scene3060GlobePuzzleRuns[pairOffset + 1] = repeatCount;
	}
	state.scene3060LastGlobePuzzleButton = button;
}

void Scene3060::resetGlobePuzzleHistory() {
	GameplayState &state = _vm->gameState();
	state.scene3060GlobePuzzleRunIndex = 3;
	state.scene3060LastGlobePuzzleButton = 0;
	for (uint i = 0; i < sizeof(state.scene3060GlobePuzzleRuns); ++i)
		state.scene3060GlobePuzzleRuns[i] = 0;
}

bool Scene3060::matchesGlobePuzzle() const {
	const GameplayState &state = _vm->gameState();
	const byte *history = state.scene3060GlobePuzzleRuns;
	return history[0] == 1 && history[1] == 5 &&
		history[2] == 2 && history[3] == 2 &&
		history[4] == 1 && history[5] == 3 &&
		history[6] == 2 && history[7] == 4;
}

void Scene3060::runGlobeButtonSequence(byte button, const byte *frameMap, uint frameMapSize, int globeDelta) {
	GameplayState &state = _vm->gameState();
	if (state.scene3060SecretDoorRevealState != 0) {
		beginSecondarySpeechLine(14, 1);
		return;
	}

	_buttonLayer.configure(7, kScene3060ButtonDescriptorCount, frameMap, frameMapSize);
	_buttonLayer.visible = true;
	if (frameMapSize != 0) {
		const byte hookId = globeDelta > 0 ? kScene3060RotateGlobeForwardHook :
			kScene3060RotateGlobeBackwardHook;
		playAnimationFrames(_buttonLayer,
			AnimationFrameRange(0, frameMapSize - 1, kScene3060ButtonFrameMillis)
				.unskippable().hookAt(3, hookId));
	}
	_buttonLayer.visible = false;
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	recordGlobeButton(button);
	markGlobeButtonsDiscovered();
	drawPlayableComposite();
	presentFrame();
}

void Scene3060::handleAnimationFrameHook(byte hookId, uint frame) {
	if (frame != 3)
		return;
	if (hookId == kScene3060RotateGlobeForwardHook)
		rotateGlobe(1);
	else if (hookId == kScene3060RotateGlobeBackwardHook)
		rotateGlobe(-1);
}

void Scene3060::runRedButtonSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene3060SecretDoorRevealState != 0) {
		beginSecondarySpeechLine(14, 1);
		return;
	}

	_buttonLayer.configure(7, kScene3060ButtonDescriptorCount,
		kScene3060RedButtonFrameMap, ARRAYSIZE(kScene3060RedButtonFrameMap));
	_buttonLayer.visible = true;
	_buttonLayer.setFrame(0);
	drawPlayableComposite();
	presentFrame();

	uint buttonFrame = 0;
	uint32 buttonRemaining = kScene3060ButtonFrameMillis;
	bool spinning = false;
	uint32 spinRemaining = 0;
	uint32 spinDelay = kScene3060RedGlobeInitialStepMillis;
	byte remainingRevolutions = kScene3060RedGlobeRevolutionCount;

	while ((buttonFrame + 1 < ARRAYSIZE(kScene3060RedButtonFrameMap) || spinning) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		const bool buttonAnimating = buttonFrame + 1 < ARRAYSIZE(kScene3060RedButtonFrameMap);
		uint32 waitMillis = buttonAnimating ? buttonRemaining : spinRemaining;
		if (spinning && (!buttonAnimating || spinRemaining < waitMillis))
			waitMillis = spinRemaining;

		if (waitMillis != 0 && waitSceneMillis(waitMillis, false))
			break;
		if (buttonAnimating)
			buttonRemaining -= waitMillis;
		if (spinning)
			spinRemaining -= waitMillis;

		if (buttonAnimating && buttonRemaining == 0) {
			++buttonFrame;
			_buttonLayer.setFrame((byte)buttonFrame);
			buttonRemaining = kScene3060ButtonFrameMillis;
			if (buttonFrame == 3) {
				spinning = true;
				spinRemaining = 0;
			}
		}

		if (spinning && spinRemaining == 0) {
			const bool wrapped = state.scene3060GlobeFrame == 0;
			rotateGlobe(-1);
			if (wrapped) {
				--remainingRevolutions;
				if (remainingRevolutions == 0) {
					spinning = false;
				} else {
					spinDelay -= kScene3060RedGlobeAccelerationMillis;
					spinRemaining = spinDelay;
				}
			} else {
				spinRemaining = spinDelay;
			}
		}
	}

	_buttonLayer.visible = false;
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	if (matchesGlobePuzzle()) {
		runSecretDoorReveal();
	} else {
		resetGlobePuzzleHistory();
	}
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	markGlobeButtonsDiscovered();
	drawPlayableComposite();
	presentFrame();
}

void Scene3060::runSecretDoorReveal() {
	_soundBank0.playSample(0x10, 100);
	_secretDoorRevealActive = true;
	runSceneOverlay(ActionOverlaySpec(8, kScene3060SecretDoorDescriptorCount,
		kScene3060SecretDoorRevealFrameMap, ARRAYSIZE(kScene3060SecretDoorRevealFrameMap), kScene3060SecretDoorFrameMillis)
		.noRedrawAtEnd());
	_secretDoorRevealActive = false;
	_soundBank0.stop();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_vm->gameState().scene3060SecretDoorRevealState = 1;
	applySceneStateToHotspotsAndPatches(0);
	beginSecondarySpeechLine(14, 0);
}

} // End of namespace Hollywood
