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

#include "hollywood/scenes/playable/scene2090.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2090ArchiveName = "RESOURCE.B09";
const char *const kScene2090MusicArchiveName = "RESOURCE.M02";
const char *const kScene2090SoundArchiveName = "RESOURCE.S02";
const uint kScene2090InitialRequiredChunkCount = 8;
const uint kScene2090ArenaFirstChunk = 5;
const uint kScene2090ArenaLastChunk = 7;
const uint kScene2090StageIndex = 209;
const uint16 kScene2090FirstState = 0x082a;
const uint16 kScene2090SpecialEntryState = 0x082b;
const uint16 kScene2080ReturnState = 0x0821;
const uint16 kScene2020ReturnState = 0x07e5;
const uint16 kScene2090ViewportXOffset = 0x0020;
const uint kScene2090ActorBankTableEntry = 0x0070;
const uint kScene2090ActorPaletteTableEntry = 0x00cc;
const uint kScene2090Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2090SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2090FrameMillis = 75;
const uint32 kScene2090SlowFrameMillis = 125;
const byte kScene2090InvalidFacing = 0xff;
const byte kScene2090EntryClipChunk = 5;
const byte kScene2090EntryClipDescriptorCount = 10;
const byte kScene2090BackClipChunk = 6;
const byte kScene2090BackClipDescriptorCount = 9;
const byte kScene2090ForegroundChunk = 7;
const byte kScene2090ForegroundDescriptorCount = 0x3b;
const byte kScene2090SpecialEntryStartForegroundFrame = 0x35;
const byte kScene2090SpecialEntryMidForegroundFrame = 0x47;
const byte kScene2090SpecialEntryFinalForegroundFrame = 0x4c;
const byte kScene2090FinaleFirstForegroundStopFrame = 0x24;
const byte kScene2090FinaleSpeechTriggerFrame = 0x26;
const byte kScene2090FinaleLastForegroundFrame = 0x35;
const byte kScene2090RequiredItem2A = 0x2a;
const byte kScene2090RequiredItem2C = 0x2c;
const byte kScene2090RequiredItem2E = 0x2e;

const byte kScene2090ActorPathStepDeltaTableSet5A[] = {
	4, 1, 1, 2, 2, 2, 5, 1, 0, 0, 3, 2,
	2, 1, 6, 4, 4, 5, 4, 3, 7, 2, 1, 6,
	6, 6, 5, 4, 7, 4, 7, 7, 3, 4, 3, 7,
	3, 2, 2, 3, 0, 3, 3, 1, 0, 3, 1, 4,
	4, 7, 7, 3, 4, 3, 7, 6, 6, 5, 4, 7,
	4, 3, 7, 2, 1, 6, 2, 1, 6, 4, 4, 5
};

const byte kScene2090ForegroundFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, 52, 51, 50, 49, 48, 47, 46,
	45, 44, 43, 42, 41, 40, 39, 38, 37, 36,
	35, 53, 54, 55, 56, 57, 58
};

static_assert(ARRAYSIZE(kScene2090ActorPathStepDeltaTableSet5A) == 72,
	"Scene 2090 actor path table size changed");
static_assert(ARRAYSIZE(kScene2090ForegroundFrameMap) == 0x4d,
	"Scene 2090 foreground frame map size changed");
static_assert(kScene2090ForegroundDescriptorCount > 58,
	"Scene 2090 foreground descriptor count is too small");

static PlayableSceneConfig scene2090Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene2090ArchiveName;
	config.initialRequiredChunkCount = kScene2090InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene2090ArenaFirstChunk;
	config.arenaLastChunk = kScene2090ArenaLastChunk;
	config.stageIndex = kScene2090StageIndex;
	config.debugName = "Scene 2090";
	config.viewportXOffset = kScene2090ViewportXOffset;
	config.viewportMinXOffset = kScene2090ViewportXOffset;
	config.viewportMaxXOffset = kScene2090ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 2;
	config.actorBankTableEntry = kScene2090ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene2090ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene2090Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene2090SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene2090ActorPathStepDeltaTableSet5A;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene2090ActorPathStepDeltaTableSet5A);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene2090MusicArchiveName;
	config.soundBank0ArchiveName = kScene2090SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	return config;
}

Scene2090::Scene2090(HollywoodEngine *vm) :
		PlayableScene(vm, scene2090Config(), "scene2090", 0x116, 0x1ab, 4, 0xfd, 0xfb),
		_foregroundLayer() {
	_foregroundLayer.configure(kScene2090ForegroundChunk, kScene2090ForegroundDescriptorCount,
		kScene2090ForegroundFrameMap, ARRAYSIZE(kScene2090ForegroundFrameMap));
}

void Scene2090::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);

	if (_vm->gameState().mainFlowStateId == kScene2090SpecialEntryState) {
		_activeActorWorldX = 0x151;
		_activeActorWorldY = 0x0df;
		_activeActorFacing = 1;
		resetForegroundLayer(true, kScene2090SpecialEntryStartForegroundFrame);
	} else {
		_activeActorWorldX = 0x116;
		_activeActorWorldY = 0x1ab;
		_activeActorFacing = 4;
		resetForegroundLayer(false, 0);
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene2090::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawResourceSpriteLayer(_foregroundLayer);
}

void Scene2090::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2090SpecialEntryState) {
		runEntryFromScene2020();
		_vm->gameState().mainFlowStateId = kScene2090FirstState;
	} else {
		runEntryFromScene2080();
	}
}

bool Scene2090::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2090::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a escalera (go to stairs): approach the altar steps.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Mirar escalera (look at stairs): marble and gold description.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Ir atras hacia B08 (go back to B08): animated return to the sarcophagus chamber.
		runBackTransitionToScene2080();
		return true;
	case 304: // Ir a entrada (go to entrance): describes the connection to the sarcophagus chamber.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar entrada/altar ritual (look at entrance): checks princess hair and Nile pollen.
		runGuardOrCurtainInteraction();
		return true;
	case 306: // Usar pergamino too early (use scroll early): wait for the right moment.
		beginSecondarySpeechLine(5, 0);
		return true;
	default:
		return false;
	}
}

bool Scene2090::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x39, 0x254);

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene2090::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	(void)restoredStepDeltas;

	if (currentRegion == 4 && nextRegion == 5) {
		requestedFacing = 2;
		return true;
	}
	if ((currentRegion == 4 && nextRegion == 3) ||
			(currentRegion == 5 && nextRegion == 4)) {
		requestedFacing = 5;
		return true;
	}

	return false;
}

bool Scene2090::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)targetY;
	(void)restoredStepDeltas;

	if (currentRegion == 5 && targetX < state.x) {
		requestedFacing = 5;
		return true;
	}

	return false;
}

bool Scene2090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	if (selector != 0 && selector != 0xff)
		return true;

	restoreBaseFramebufferFromOriginal();

	const uint paletteMaskBytes = MIN<uint>(_paletteMask.size(), _paletteMaskOriginal.size());
	const uint fullPaletteMaskBytes = MIN<uint>(_fullPaletteRegionMask.size(), _paletteMaskOriginal.size());
	if (paletteMaskBytes != 0)
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), paletteMaskBytes);
	if (_paletteMask.size() > paletteMaskBytes)
		memset(_paletteMask.data() + paletteMaskBytes, 0, _paletteMask.size() - paletteMaskBytes);
	if (fullPaletteMaskBytes != 0)
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), fullPaletteMaskBytes);
	if (_fullPaletteRegionMask.size() > fullPaletteMaskBytes)
		memset(_fullPaletteRegionMask.data() + fullPaletteMaskBytes, 0,
			_fullPaletteRegionMask.size() - fullPaletteMaskBytes);

	const uint walkableMaskBytes = MIN<uint>(_walkablePaletteMask.size(), _fullPaletteRegionMask.size());
	if (walkableMaskBytes != 0)
		memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), walkableMaskBytes);
	if (_walkablePaletteMask.size() > walkableMaskBytes)
		memset(_walkablePaletteMask.data() + walkableMaskBytes, 0,
			_walkablePaletteMask.size() - walkableMaskBytes);
	for (uint i = 0; i < walkableMaskBytes; ++i) {
		if (_walkablePaletteMask[i] == 3 || _walkablePaletteMask[i] == 6)
			_walkablePaletteMask[i] = 0;
	}

	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene2090::ambientAudioProfile() const {
	return createLoopingAmbientAudioProfile(100);
}

void Scene2090::resetForegroundLayer(bool visible, byte frameIndex) {
	_foregroundLayer.visible = visible;
	_foregroundLayer.reset(frameIndex);
}

void Scene2090::setForegroundFrame(byte frameIndex) {
	_foregroundLayer.visible = true;
	_foregroundLayer.setFrame(frameIndex);
}

void Scene2090::runEntryFromScene2080() {
	resetForegroundLayer(false, 0);
	setActiveActorPose(0x152, 0x194, 4);

	copyBaseFramebufferToSceneFramebuffer();
	drawClipFrameDelta(kScene2090EntryClipChunk, kScene2090EntryClipDescriptorCount, 0);
	presentFrame();
	playDeltaClip(kScene2090EntryClipChunk, kScene2090EntryClipDescriptorCount,
		kScene2090EntryClipDescriptorCount, kScene2090FrameMillis, 1);
	_soundBank0.stop();

	runEntryPathWithFinalFacing(0x152, 0x194, 4, 0x116, 0x1ab, kScene2090InvalidFacing, 0);

	GameplayState &state = _vm->gameState();
	if (!state.scene2090EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene2090EntryLineSeen = true;
	}
}

void Scene2090::runEntryFromScene2020() {
	setActiveActorPose(0x151, 0x0df, 1);

	resetForegroundLayer(true, kScene2090SpecialEntryStartForegroundFrame);
	drawPlayableComposite();
	presentFrame();
	_soundBank0.playSample(0x0f, 100);

	runForegroundAnimationTo(kScene2090SpecialEntryMidForegroundFrame, kScene2090SlowFrameMillis);
	runForegroundAnimationTo(kScene2090SpecialEntryFinalForegroundFrame, kScene2090SlowFrameMillis);

	if (hasInventoryItem(kScene2090RequiredItem2A))
		removeInventoryItem(kScene2090RequiredItem2A);
	if (hasInventoryItem(kScene2090RequiredItem2C))
		removeInventoryItem(kScene2090RequiredItem2C);
	if (hasInventoryItem(kScene2090RequiredItem2E))
		removeInventoryItem(kScene2090RequiredItem2E);

	beginSecondarySpeechLine(4, 9);
}

void Scene2090::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel) {
	setActiveActorPose(startX, startY, startFacing);

	drawPlayableComposite();
	presentFrame();
	walkActiveActorTo(targetX, targetY, finalFacing, finalCel, false);
	if (finalFacing != kScene2090InvalidFacing)
		_activeActorFacing = finalFacing;
	_activeActorCel = finalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene2090::runBackTransitionToScene2080() {
	drawPlayableComposite();
	presentFrame();
	_soundBank0.playSample(0x1e, 100);
	playDeltaClip(kScene2090BackClipChunk, kScene2090BackClipDescriptorCount,
		kScene2090BackClipDescriptorCount, kScene2090FrameMillis);
	_vm->gameState().mainFlowStateId = kScene2080ReturnState;
}

void Scene2090::runGuardOrCurtainInteraction() {
	const bool hasItem2E = hasInventoryItem(kScene2090RequiredItem2E);
	const bool hasItem2C = hasInventoryItem(kScene2090RequiredItem2C);

	if (!hasItem2E) {
		beginSecondarySpeechLine(4, hasItem2C ? 0 : 2);
		return;
	}
	if (!hasItem2C) {
		beginSecondarySpeechLine(4, 1);
		return;
	}

	walkActiveActorTo(0x151, 0x0df, 1, 0, false);
	beginSecondarySpeechLine(4, 3);
	beginSecondarySpeechLine(4, 4);
	beginSecondarySpeechLine(4, 5);
	beginSecondarySpeechLine(4, 6);

	resetForegroundLayer(true, 0);
	runForegroundAnimationTo(kScene2090FinaleFirstForegroundStopFrame, kScene2090FrameMillis);

	beginSecondarySpeechLine(4, 7);
	_soundBank0.playSample(0x0e, 100);
	runForegroundAnimationTo(kScene2090FinaleLastForegroundFrame, kScene2090SlowFrameMillis,
		kScene2090FinaleSpeechTriggerFrame, 4, 8);
	waitForStartedSpeechAndClear(1200);

	runCurtainClearToBlack();
	_soundBank0.stop();
	_vm->gameState().mainFlowStateId = kScene2020ReturnState;
}

void Scene2090::runForegroundAnimationTo(byte targetFrame, uint32 frameMillis,
		int speechTriggerFrame, uint16 speechRow, byte speechFrame) {
	while (_foregroundLayer.frameIndex < targetFrame && !_vm->isSceneRestartRequested()) {
		setForegroundFrame(_foregroundLayer.frameIndex + 1);
		if ((int)_foregroundLayer.frameIndex == speechTriggerFrame)
			startSecondarySpeechLine(speechRow, speechFrame);
		if (waitSceneMillis(frameMillis))
			break;
	}
}

void Scene2090::waitForStartedSpeechAndClear(uint32 fallbackMillis) {
	uint32 elapsed = 0;
	while (!_vm->isSceneRestartRequested() &&
			(_speech.isPlaying() || elapsed < fallbackMillis)) {
		const uint32 slice = _speech.isPlaying() ? 50 : MIN<uint32>(50, fallbackMillis - elapsed);
		if (waitSceneMillis(slice))
			break;
		elapsed += slice;
	}

	_speech.stop();
	clearSpeechOverlay();
	drawPlayableComposite();
	presentFrame();
}

void Scene2090::runCurtainClearToBlack() {
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	for (uint y = 0; y < HollywoodEngine::kScreenHeight && !_vm->isSceneRestartRequested(); y += 0x14) {
		const uint height = MIN<uint>(0x14, HollywoodEngine::kScreenHeight - y);
		for (uint row = 0; row < height; ++row) {
			memset(pixels + (y + row) * HollywoodEngine::kSceneBufferWidth,
				0, HollywoodEngine::kSceneBufferWidth);
		}
		presentFrame();
		if (pollEvents(true))
			break;
		g_system->delayMillis(10);
	}
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
}

} // End of namespace Hollywood
