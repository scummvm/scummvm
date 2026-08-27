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

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

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
const byte kScene2090FinaleSpeechHook = 1;
const byte kScene2090PaletteCycleFirstColor = 0xa0;
const byte kScene2090PaletteCycleLastColor = 0xaf;
const int kScene2090CurtainStartOffset = 0xdc;
const byte kScene2090CurtainBandWidth = 0x14;
const uint kScene2090CurtainEndOffset = 0xf0;

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

static_assert(ARRAYSIZE(kScene2090ForegroundFrameMap) == 0x4d,
	"Scene 2090 foreground frame map size changed");
static_assert(kScene2090ForegroundDescriptorCount > 58,
	"Scene 2090 foreground descriptor count is too small");

static PlayableSceneConfig scene2090Config() {
	PlayableSceneConfig config(2090,
		SceneResourceLayout(8, 5, 7),
		SceneViewport(kScene2090ViewportXOffset, kScene2090ViewportXOffset, kScene2090ViewportXOffset),
		SceneActorPose(0x116, 0x1ab, 4));
	config.setActorResources(kScene2090ActorBankTableEntry, kScene2090ActorPaletteTableEntry);
	config.setTextResources(kScene2090Resource003RowsOffsetIndex, kScene2090SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet5A);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene2090::Scene2090(HollywoodEngine *vm) :
		PlayableScene(vm, scene2090Config()),
		_foregroundLayer(),
		_paletteCycleChannel(),
		_paletteCycleActive(false) {
	_foregroundLayer.configure(kScene2090ForegroundChunk, kScene2090ForegroundDescriptorCount,
		kScene2090ForegroundFrameMap, ARRAYSIZE(kScene2090ForegroundFrameMap));
	_paletteCycleChannel.reset(0, kScene2090FrameMillis);
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
	const bool foregroundReplacesActor = _foregroundLayer.visible;
	drawActiveAndSecondaryActorFrames(drawActiveActor && !foregroundReplacesActor,
		activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor && !foregroundReplacesActor,
		secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (foregroundReplacesActor && (drawActiveActor || drawSecondaryActor))
		updateActorPaletteForWorldPoint(activeWorldX, activeWorldY);
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

bool Scene2090::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

bool Scene2090::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff &&
		stateId != kScene2020ReturnState && !isMainFlowStateInScene(stateId);
}

void Scene2090::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

bool Scene2090::advanceCustomGameplayLoop(uint32 delta) {
	if (_paletteCycleActive) {
		const uint frameCount = _paletteCycleChannel.consumeFrames(delta);
		for (uint frame = 0; frame < frameCount; ++frame)
			rotateRitualPalette();
	}
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2090::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 136: // Usar pergamino (use parchment): start the Karnak ceremony in this room.
	case 306: // Ceremony callback reached by the parchment's scene-local redirect.
		runAltarCeremony();
		return true;
	case 301: // Ir a escalera (go to stairs): movement only.
		return true;
	case 302: // Mirar escalera (look at stairs): they lead up to the altar.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar altar (look at altar): describe its marble and gold.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Ir a entrada (go to entrance): animated return to the sarcophagus chamber.
		runBackTransitionToScene2080();
		return true;
	case 305: // Mirar entrada (look at entrance): describe the connection to the previous chamber.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 307: // Usar flor/pelo con altar (use flower/hair on altar): the parchment must start the ceremony.
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
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2090::resetForegroundLayer(bool visible, byte frameIndex) {
	_foregroundLayer.visible = visible;
	_foregroundLayer.reset(frameIndex);
}

void Scene2090::runEntryFromScene2080() {
	resetForegroundLayer(false, 0);
	setActiveActorPose(0x152, 0x194, 4);

	copyBaseFramebufferToSceneFramebuffer();
	drawClipFrameDelta(kScene2090EntryClipChunk, kScene2090EntryClipDescriptorCount, 0);
	if (fadePaletteFromBlack())
		return;
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
	_soundBank0.playSample(0x0f, 100);
	drawPlayableComposite();
	if (runCurtainRevealFromBlack())
		return;

	setRitualPaletteCycle(true);
	const bool firstPartComplete = playAndPresentAnimationFrames(_foregroundLayer,
		AnimationFrameRange(kScene2090SpecialEntryStartForegroundFrame + 1,
			kScene2090SpecialEntryMidForegroundFrame, kScene2090SlowFrameMillis).unskippable());
	setRitualPaletteCycle(false);
	if (!firstPartComplete)
		return;
	if (!playAndPresentAnimationTransition(_foregroundLayer,
			AnimationTransition(kScene2090SpecialEntryMidForegroundFrame + 1,
				kScene2090SpecialEntryFinalForegroundFrame,
				kScene2090SpecialEntryFinalForegroundFrame,
				kScene2090SlowFrameMillis).unskippable()))
		return;

	if (hasInventoryItem(kScene2090RequiredItem2A))
		removeInventoryItem(kScene2090RequiredItem2A);
	if (hasInventoryItem(kScene2090RequiredItem2C))
		removeInventoryItem(kScene2090RequiredItem2C);
	if (hasInventoryItem(kScene2090RequiredItem2E))
		removeInventoryItem(kScene2090RequiredItem2E);

	resetForegroundLayer(false, 0);
	drawPlayableComposite();
	presentFrame();
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

void Scene2090::runAltarCeremony() {
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
	if (!playAndPresentAnimationTransition(_foregroundLayer,
			AnimationTransition(1, kScene2090FinaleFirstForegroundStopFrame,
				kScene2090FinaleFirstForegroundStopFrame,
				kScene2090FrameMillis).unskippable()))
		return;

	beginSecondarySpeechLine(4, 7);
	_soundBank0.playSample(0x0e, 100);
	setRitualPaletteCycle(true);
	const bool finaleComplete = playAndPresentAnimationFrames(_foregroundLayer,
		AnimationFrameRange(kScene2090FinaleFirstForegroundStopFrame + 1,
			kScene2090FinaleLastForegroundFrame - 1, kScene2090SlowFrameMillis)
			.hookAt(kScene2090FinaleSpeechTriggerFrame, kScene2090FinaleSpeechHook)
			.unskippable());
	if (finaleComplete) {
		_foregroundLayer.setFrame(kScene2090FinaleLastForegroundFrame);
		drawPlayableComposite();
		presentFrame();
	}
	setRitualPaletteCycle(false);
	if (!finaleComplete)
		return;
	waitForStartedSpeechAndClear(1200);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	runCurtainClearToBlack();
	_soundBank0.stop();
	_vm->gameState().mainFlowStateId = kScene2020ReturnState;
}

void Scene2090::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene2090FinaleSpeechHook && frame == kScene2090FinaleSpeechTriggerFrame)
		startSecondarySpeechLine(4, 8);
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

void Scene2090::setRitualPaletteCycle(bool active) {
	_paletteCycleActive = active;
	_paletteCycleChannel.reset(0, kScene2090FrameMillis);
	if (active)
		rotateRitualPalette();
}

void Scene2090::rotateRitualPalette() {
	const uint lastOffset = kScene2090PaletteCycleLastColor * 3;
	if (_paletteCurrent.size() <= lastOffset + 2)
		return;

	byte savedColor[3];
	memcpy(savedColor, _paletteCurrent.data() + lastOffset, sizeof(savedColor));
	for (uint color = kScene2090PaletteCycleLastColor;
			color > kScene2090PaletteCycleFirstColor; --color) {
		memcpy(_paletteCurrent.data() + color * 3,
			_paletteCurrent.data() + (color - 1) * 3, sizeof(savedColor));
	}
	memcpy(_paletteCurrent.data() + kScene2090PaletteCycleFirstColor * 3,
		savedColor, sizeof(savedColor));
	invalidatePresentationPalette();
}

bool Scene2090::runCurtainRevealFromBlack() {
	Graphics::ManagedSurface savedScene;
	savedScene.copyFrom(_sceneFramebuffer);
	byte *destination = framebufferPixels(_sceneFramebuffer);
	if (!destination || savedScene.empty())
		return false;

	memset(destination, 0, framebufferByteCount());
	presentFrame();
	const uint resourcePaletteBytes = MIN<uint>(_sceneChunkTable.sizes[1],
		MIN<uint>(_paletteCurrent.size(), _paletteResource.size()));
	memcpy(_paletteCurrent.data(), _paletteResource.data(), resourcePaletteBytes);
	invalidatePresentationPalette();
	for (int sweep = kScene2090CurtainStartOffset;
			sweep >= 0 && !_vm->isSceneRestartRequested();
			sweep -= kScene2090CurtainBandWidth) {
		applyCurtainBand(&savedScene.rawSurface(), (uint)sweep, kScene2090CurtainBandWidth);
		presentFrame();
		if (pollEvents(false))
			return true;
	}

	_sceneFramebuffer.copyRectToSurface(savedScene.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth,
			HollywoodEngine::kSceneBufferHeight));
	presentFrame();
	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene2090::applyCurtainBand(const Graphics::Surface *source, uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - 2 * (int)sweepOffset;
	if (innerWidth <= 0)
		return;

	const int middleInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - 2 * middleInset;
	const int leftX = _viewportXOffset + sweepOffset;
	const int rightX = leftX + innerWidth - bandWidth;
	Graphics::Surface &destination = *_sceneFramebuffer.surfacePtr();

	for (uint row = 0; row < bandWidth; ++row) {
		const int topY = sweepOffset + row;
		const int bottomY = HollywoodEngine::kScreenHeight - bandWidth - sweepOffset + row;
		if (source) {
			copySurfaceRun(*source, destination, topY, leftX, innerWidth);
			copySurfaceRun(*source, destination, bottomY, leftX, innerWidth);
		} else {
			clearSurfaceRun(destination, topY, leftX, innerWidth);
			clearSurfaceRun(destination, bottomY, leftX, innerWidth);
		}
	}

	for (int row = 0; row < middleHeight; ++row) {
		const int y = middleInset + row;
		if (source) {
			copySurfaceRun(*source, destination, y, leftX, bandWidth);
			copySurfaceRun(*source, destination, y, rightX, bandWidth);
		} else {
			clearSurfaceRun(destination, y, leftX, bandWidth);
			clearSurfaceRun(destination, y, rightX, bandWidth);
		}
	}
}

void Scene2090::runCurtainClearToBlack() {
	if (!framebufferPixels(_sceneFramebuffer))
		return;

	for (uint sweep = 0;
			sweep < kScene2090CurtainEndOffset && !_vm->isSceneRestartRequested();
			sweep += kScene2090CurtainBandWidth) {
		applyCurtainBand(nullptr, sweep, kScene2090CurtainBandWidth);
		presentFrame();
		if (pollEvents(false))
			return;
	}
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	invalidatePresentationPalette();
	presentFrame();
}

} // End of namespace Hollywood
