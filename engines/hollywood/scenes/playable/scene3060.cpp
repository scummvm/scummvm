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

const char *const kScene3060ArchiveName = "RESOURCE.C06";
const char *const kScene3060MusicArchiveName = "RESOURCE.M03";
const char *const kScene3060SoundArchiveName = "RESOURCE.S03";
const uint kScene3060InitialRequiredChunkCount = 10;
const uint kScene3060ArenaFirstChunk = 5;
const uint kScene3060ArenaLastChunk = 9;
const uint kScene3060StageIndex = 306;
const uint16 kScene3060EntryFromScene3050State = 0x0bf4;
const uint16 kScene3060EntryFromSecretPassageState = 0x0bf5;
const uint16 kScene3050EntryFromScene3060State = 0x0beb;
const uint16 kScene3070State = 0x0bfe;
const uint kScene3060ActorBankTableEntry = 0x0000;
const uint kScene3060ActorPaletteTableEntry = 0x00cc;
const uint kScene3060Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3060SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3060FrontFrameMillis = 100;
const uint32 kScene3060GlobeFrameMillis = 125;
const uint32 kScene3060ButtonFrameMillis = 75;
const uint32 kScene3060RedButtonFrameMillis = 30;
const uint kScene3060FrontDescriptorCount = 0x13;
const uint kScene3060GlobeDescriptorCount = 0x1e;
const uint kScene3060ButtonDescriptorCount = 5;
const uint kScene3060SecretDoorDescriptorCount = 0x0e;

const byte kScene3060ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

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
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene3060ArchiveName;
	config.initialRequiredChunkCount = kScene3060InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene3060ArenaFirstChunk;
	config.arenaLastChunk = kScene3060ArenaLastChunk;
	config.stageIndex = kScene3060StageIndex;
	config.debugName = "Scene 3060";
	config.viewportXOffset = 0;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 3;
	config.actorBankTableEntry = kScene3060ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene3060ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene3060Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene3060SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene3060ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene3060ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene3060MusicArchiveName;
	config.soundBank0ArchiveName = kScene3060SoundArchiveName;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene3060EntryFromScene3050State;
	config.mainFlowLastState = kScene3060EntryFromSecretPassageState;
	return config;
}

Scene3060::Scene3060(HollywoodEngine *vm) :
		PlayableScene(vm, scene3060Config(), "scene3060", 0x22d, 0x156, 4, 0xfd, 0xfb),
		_frontChannel(),
		_globeChannel(),
		_frontLayer(),
		_globeLayer(),
		_buttonLayer(),
		_frontLayerMode(0),
		_frontLayerPauseCounter(0),
		_globeSpinDelta(0) {
	_frontLayer.configure(5, kScene3060FrontDescriptorCount,
		kScene3060FrontFrameMap, ARRAYSIZE(kScene3060FrontFrameMap));
	_globeLayer.configure(6, kScene3060GlobeDescriptorCount,
		kScene3060GlobeFrameMap, ARRAYSIZE(kScene3060GlobeFrameMap));
	_buttonLayer.configure(7, kScene3060ButtonDescriptorCount,
		kScene3060LeftButtonFrameMap, ARRAYSIZE(kScene3060LeftButtonFrameMap));
}

bool Scene3060::hasCustomPreviewState() const {
	return true;
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

bool Scene3060::hasCustomComposite() const {
	return true;
}

void Scene3060::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();

	if (actorDrawOrderMode < 4)
		drawResourceSpriteLayer(_globeLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (actorDrawOrderMode >= 4)
		drawResourceSpriteLayer(_globeLayer);

	drawActionOverlayLayer();
	drawResourceSpriteLayer(_buttonLayer);
	drawResourceSpriteLayer(_frontLayer);
}

bool Scene3060::hasCustomEntrySequence() const {
	return true;
}

void Scene3060::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene3060EntryFromSecretPassageState)
		runEntryFromSecretPassage();
	else
		runEntryFromScene3050();
}

bool Scene3060::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene3060::advanceCustomGameplayLoop(uint32 delta) {
	advanceFrontLayer(delta);
	advanceGlobeLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3060::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Ir a recibidor (go to hall): return to the library/living room.
		state.mainFlowStateId = kScene3050EntryFromScene3060State;
		return true;
	case 302: // Mirar puerta secreta / recibidor hidden exit (look at secret door / hall exit).
		beginSecondarySpeechLine(0, 0);
		return true;
	case 303: // Ir a puerta secreta / cuadro central (go to secret passage).
		if (state.scene3060SecretDoorRevealState != 0)
			runEntryPath(_activeActorWorldX, _activeActorWorldY, _activeActorFacing, 0x1aa, 0x161);
		state.mainFlowStateId = kScene3070State;
		return true;
	case 304: // Mirar puerta secreta / cuadro central (look at secret door / painting).
		beginSecondarySpeechLine(1, state.scene3060SecretDoorRevealState != 0 ? 0 : 1);
		return true;
	case 305: // Coger cuadro/libros/título (take painting/books/title): fixed refusal.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 306: // Mirar cuadro superior izquierdo (look at upper-left painting/certificate).
		beginSecondarySpeechLine(4, 0);
		state.scene3060InspectedTitleFlags |= 1;
		applySceneStateToHotspotsAndPatches(1);
		return true;
	case 307: // Mirar cuadro superior derecho (look at upper-right painting/certificate).
		beginSecondarySpeechLine(5, 0);
		state.scene3060InspectedTitleFlags |= 2;
		applySceneStateToHotspotsAndPatches(1);
		return true;
	case 308: // Mirar cuadro inferior izquierdo (look at lower-left painting/certificate).
		beginSecondarySpeechLine(6, 0);
		state.scene3060InspectedTitleFlags |= 4;
		applySceneStateToHotspotsAndPatches(1);
		return true;
	case 309: // Mirar libros (look at books).
		beginSecondarySpeechLine(7, 0);
		state.scene3060InspectedTitleFlags |= 8;
		applySceneStateToHotspotsAndPatches(1);
		return true;
	case 310: // Mirar bola del mundo (look at globe).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Coger/abrir bola del mundo (take/open globe).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Usar bola del mundo (use globe): enables the button controls.
		beginSecondarySpeechLine(10, 0);
		state.scene3060GlobeButtonsDiscovered = true;
		applySceneStateToHotspotsAndPatches(5);
		return true;
	case 313: // Mirar botón izquierdo / estante detail (look at left-button area).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314: // Mirar restos/adorno de la fiesta (look at party remains/decor).
		beginSecondarySpeechLine(12, 0);
		return true;
	case 315: // Mirar título (look at title/certificate).
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
	if (selector == 0xff || selector == 0 || selector == 1 || selector == 5) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		promoteSecretDoorHotspots();
		updateTitleCaptionRows();
		updateGlobeButtonDefaultStrips();

		if (_vm->gameState().scene3060SecretDoorRevealState != 0 &&
				_sceneChunkTable.isValidChunk(9))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

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

void Scene3060::handleActionOverlayFrameHook(byte hookId, uint frame) {
	if (hookId != 1 || _globeSpinDelta == 0)
		return;

	GameplayState &state = _vm->gameState();
	if ((frame % 2) == 0) {
		int nextFrame = (int)state.scene3060GlobeFrame + (int)_globeSpinDelta;
		while (nextFrame < 0)
			nextFrame += kScene3060GlobeDescriptorCount;
		state.scene3060GlobeFrame = (byte)(nextFrame % kScene3060GlobeDescriptorCount);
		_globeLayer.setFrame(state.scene3060GlobeFrame);
	}
}

void Scene3060::resetAnimationLayers() {
	_frontChannel.reset(0, kScene3060FrontFrameMillis);
	_globeChannel.reset(_vm->gameState().scene3060GlobeFrame, kScene3060GlobeFrameMillis);
	_frontLayer.visible = true;
	_globeLayer.visible = true;
	_buttonLayer.visible = false;
	_frontLayer.reset(0);
	_globeLayer.reset(_vm->gameState().scene3060GlobeFrame);
	_buttonLayer.reset(0);
	_frontLayerMode = 0;
	_frontLayerPauseCounter = 0;
	_globeSpinDelta = 0;
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

void Scene3060::updateTitleCaptionRows() {
	const byte flags = _vm->gameState().scene3060InspectedTitleFlags;
	for (byte index = 0; index < 4; ++index) {
		if ((flags & (1 << index)) != 0)
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

void Scene3060::advanceFrontLayer(uint32 delta) {
	const uint frameCount = _frontChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_frontLayerMode == 0) {
			if (_frontLayerPauseCounter > 0) {
				--_frontLayerPauseCounter;
			} else if (_random.getRandomNumber(24) == 0) {
				_frontLayerMode = 1;
				_frontChannel.frameIndex = 10;
			} else if (_random.getRandomNumber(14) == 0) {
				_frontLayerMode = 2;
				_frontChannel.frameIndex = 1;
			}
		} else if (_frontLayerMode == 1) {
			if (_frontChannel.frameIndex < 18) {
				++_frontChannel.frameIndex;
			} else {
				_frontLayerMode = 0;
				_frontLayerPauseCounter = (byte)(_random.getRandomNumber(7) + 2);
				_frontChannel.frameIndex = 0;
			}
		} else if (_frontLayerMode == 2) {
			if (_frontChannel.frameIndex < 9) {
				++_frontChannel.frameIndex;
			} else {
				_frontLayerMode = 0;
				_frontChannel.frameIndex = 0;
			}
		}
		_frontLayer.setFrame(_frontChannel.frameIndex);
	}
}

void Scene3060::advanceGlobeLayer(uint32 delta) {
	const uint frameCount = _globeChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		_globeLayer.setFrame(_vm->gameState().scene3060GlobeFrame);
}

void Scene3060::runEntryFromScene3050() {
	runEntryPath(0x2e3, 0x128, 4, 0x22d, 0x156);
}

void Scene3060::runEntryFromSecretPassage() {
	runEntryPath(0x08a, 0x184, 2, 0x1aa, 0x161);
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
	for (uint frame = 0; frame < frameMapSize && !Engine::shouldQuit(); ++frame) {
		_buttonLayer.setFrame(frame);
		if (frame == 3) {
			int nextFrame = (int)state.scene3060GlobeFrame + globeDelta;
			while (nextFrame < 0)
				nextFrame += kScene3060GlobeDescriptorCount;
			state.scene3060GlobeFrame = (byte)(nextFrame % kScene3060GlobeDescriptorCount);
			_globeLayer.setFrame(state.scene3060GlobeFrame);
		}
		if (waitSceneMillis(kScene3060ButtonFrameMillis))
			break;
	}
	_buttonLayer.visible = false;
	recordGlobeButton(button);
	drawPlayableComposite();
	presentFrame();
}

void Scene3060::runRedButtonSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene3060SecretDoorRevealState != 0) {
		beginSecondarySpeechLine(14, 1);
		return;
	}

	_globeSpinDelta = -1;
	runActionOverlay(ActionOverlaySpec(7, kScene3060ButtonDescriptorCount,
		kScene3060RedButtonFrameMap, ARRAYSIZE(kScene3060RedButtonFrameMap),
		kScene3060RedButtonFrameMillis)
		.showActor()
		.hookEveryFrame(1));
	_globeSpinDelta = 0;

	if (matchesGlobePuzzle()) {
		runSecretDoorReveal();
	} else {
		resetGlobePuzzleHistory();
	}
}

void Scene3060::runSecretDoorReveal() {
	_soundBank0.playSample(0x10, 100);
	runActionOverlay(ActionOverlaySpec(8, kScene3060SecretDoorDescriptorCount,
		kScene3060SecretDoorRevealFrameMap, ARRAYSIZE(kScene3060SecretDoorRevealFrameMap), kScene3060ButtonFrameMillis)
		.showActor()
		.patchAt(7, 0));
	_vm->gameState().scene3060SecretDoorRevealState = 1;
	resetGlobePuzzleHistory();
	applySceneStateToHotspotsAndPatches(0);
	beginSecondarySpeechLine(14, 0);
}

} // End of namespace Hollywood
