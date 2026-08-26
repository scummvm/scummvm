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

#include "hollywood/scenes/playable/scene4050.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene4050D09ReturnTransitionState = 0x0fd3;
const uint16 kScene4040ReturnState = 0x0fc9;
const uint16 kScene4060FirstState = 0x0fdc;
const uint16 kScene4090ReturnState = 0x0ffb;
const int kScene4050RonWorldX = 0x0173;
const int kScene4050RonWorldY = 0x013a;
const byte kScene4050RonFacing = 3;
const uint kScene4050ActorBankTableEntry = 0x0000;
const uint kScene4050ActorPaletteTableEntry = 0x00cc;
const uint kScene4050Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4050SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4050RonFrameMillis = 75;
const uint32 kScene4050RonSpeechFrameMillis = 150;
const uint32 kScene4050FlagPaletteCycleMillis = 300;
const uint kScene4050BackgroundChunk = 6;
const uint kScene4050BackgroundDescriptorCount = 8;
const uint kScene4050RonChunk = 7;
const uint kScene4050RonDescriptorCount = 0x1d;
const uint kScene4050PatchState1Chunk = 8;
const uint kScene4050PatchState2Chunk = 9;
const uint kScene4050ExitPatchChunk = 10;
const uint kScene4050D09ReturnTransitionChunk = 11;
const uint kScene4050D09ReturnTransitionDescriptorCount = 0x15;
const uint kScene4050SceneObjectPaletteFirstColor = 0xb0;
const uint kScene4050SceneObjectPaletteLastColor = 0xcf;
const byte kScene4050LongRopeItem = 0x6b;
const byte kScene4050PatchStateBase = 0;
const byte kScene4050PatchStateRopeAttached = 1;
const byte kScene4050PatchStateWindowReached = 2;
const byte kScene4050RonIdleBlinkFrame = 4;
const byte kScene4050RonSpeechFrameCount = 4;
const byte kScene4050TextColor = 0xfd;
const byte kScene4050AttachRopeSoundHook = 1;
const byte kScene4050SwingSoundHook = 2;

const byte kScene4050D09ReturnTransitionFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9,
	8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 1, 0, 1, 0
};

PlayableSceneConfig scene4050Config() {
	PlayableSceneConfig config(4050,
		SceneResourceLayout(5, 5, 11),
		SceneViewport(0),
		SceneActorPose(kScene4050RonWorldX, kScene4050RonWorldY, kScene4050RonFacing));
	config.setActorResources(kScene4050ActorBankTableEntry, kScene4050ActorPaletteTableEntry);
	config.setTextResources(kScene4050Resource003RowsOffsetIndex, kScene4050SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene4050::Scene4050(HollywoodEngine *vm) :
		PlayableScene(vm, scene4050Config()),
		_backgroundChannel(),
		_flagPaletteChannel(),
		_ronSpeechChannel(),
		_ronIdleChannel(),
		_backgroundLayer(),
		_ronLayer(),
		_d09ReturnTransitionLayer(),
		_ronManualSequenceActive(false) {
}

void Scene4050::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	restoreSceneObjectPaletteRange();
	resetAnimationLayers();
	setActiveActorPose(kScene4050RonWorldX, kScene4050RonWorldY, kScene4050RonFacing);
}

void Scene4050::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)drawActiveActor;
	(void)activeFacing;
	(void)activeCel;
	(void)activeWorldX;
	(void)activeWorldY;
	(void)drawSecondaryActor;
	(void)secondaryFacing;
	(void)secondaryFrame;
	(void)secondaryWorldX;
	(void)secondaryWorldY;
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawSceneLayers();
	drawActionOverlayLayer();
}

void Scene4050::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene4050D09ReturnTransitionState) {
		runD09ReturnTransitionSequence();
		return;
	}

	setActiveActorPose(kScene4050RonWorldX, kScene4050RonWorldY, kScene4050RonFacing);
	_backgroundLayer.setFrame(0);
	setRonResourceFrame(0);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	if (state.scene4050EntryLineSeen)
		return;

	beginRonResourceSpeechLine(1, 0);
	state.scene4050EntryLineSeen = true;
}

bool Scene4050::prepareCustomGameplayLoop() {
	restoreSceneObjectPaletteRange();
	resetAnimationLayers();
	return true;
}

bool Scene4050::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackgroundLayer(delta);
	advanceFlagPalette(delta);
	advanceRonLayer(delta);
	return false;
}

bool Scene4050::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Ir a/coger/usar saliente (go to/take/use ledge): cannot reach it.
		beginRonResourceSpeechLine(0, 0);
		return true;
	case 302: // Mirar saliente (look at ledge).
		beginRonResourceSpeechLine(2, 0);
		return true;
	case 303: // Ir a interior (go inside): return to scene 4040.
		state.mainFlowStateId = kScene4040ReturnState;
		return true;
	case 304: // Mirar interior (look inside/from the ledge).
		beginRonResourceSpeechLine(3, 0);
		return true;
	case 305: // Coger cuerda (take rope): leaves it in place.
		beginRonResourceSpeechLine(4, 0);
		return true;
	case 306: // Mirar cuerda (look at rope), state-aware.
		beginRonResourceSpeechLine(5, state.scene4050RopeSwingState == kScene4050PatchStateRopeAttached ? 0 : 1);
		return true;
	case 307: // Usar cuerda (use rope): swing toward the window once attached.
		useSceneRope();
		return true;
	case 308: // Mirar ventana (look at window), state-aware.
		beginRonResourceSpeechLine(7, state.scene4050RopeSwingState < kScene4050PatchStateWindowReached ? 0 : 1);
		return true;
	case 309: // Usar ventana (use window): cannot reach it directly.
		beginRonResourceSpeechLine(6, 2);
		return true;
	case 310: // Usar cuerda larga con saliente (use long rope with ledge): attach rope.
		useLongRopeOnLedge();
		return true;
	case 311: // Usar cuerda corta con saliente (use short rope with ledge): too short.
		beginRonResourceSpeechLine(8, 0);
		return true;
	default:
		return false;
	}
}

bool Scene4050::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	if (_vm->gameState().scene4010AlternateBackgroundState != 0 && _sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);

	const byte patchState = MIN<byte>(_vm->gameState().scene4050RopeSwingState, kScene4050PatchStateWindowReached);
	if (patchState == kScene4050PatchStateRopeAttached && _sceneChunkTable.isValidChunk(kScene4050PatchState1Chunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4050PatchState1Chunk], _baseFramebuffer);
	else if (patchState == kScene4050PatchStateWindowReached && _sceneChunkTable.isValidChunk(kScene4050PatchState2Chunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4050PatchState2Chunk], _baseFramebuffer);

	applyPatchStateColorMap(patchState);
	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	ScenePoint ronPoint;
	ronPoint.x = kScene4050RonWorldX;
	ronPoint.y = kScene4050RonWorldY;
	_hotspots.setActionTarget(1, ronPoint, ronPoint);
	return true;
}

bool Scene4050::shouldDrawSecondaryActorInPlayableComposite() const {
	return false;
}

AmbientAudioProfile Scene4050::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0b, 3, 20, 1, 0x0b, 5, 100, 50);
}

void Scene4050::resetAnimationLayers() {
	_backgroundLayer.configure(kScene4050BackgroundChunk, kScene4050BackgroundDescriptorCount, nullptr, 0);
	_backgroundLayer.visible = true;
	_backgroundLayer.setFrame(0);
	_backgroundChannel.reset(0, kScene4050RonFrameMillis);
	_flagPaletteChannel.reset(0, kScene4050FlagPaletteCycleMillis);

	_ronLayer.configure(kScene4050RonChunk, kScene4050RonDescriptorCount, nullptr, 0);
	_ronLayer.visible = true;
	_ronLayer.setFrame(0);
	_d09ReturnTransitionLayer.configure(kScene4050D09ReturnTransitionChunk,
		kScene4050D09ReturnTransitionDescriptorCount,
		kScene4050D09ReturnTransitionFrameMap,
		ARRAYSIZE(kScene4050D09ReturnTransitionFrameMap));
	_d09ReturnTransitionLayer.visible = false;
	_d09ReturnTransitionLayer.setFrame(0);
	_ronSpeechChannel.reset(0, kScene4050RonSpeechFrameMillis);
	_ronIdleChannel.reset(0, kScene4050RonSpeechFrameMillis);
	_ronManualSequenceActive = false;
}

void Scene4050::restoreSceneObjectPaletteRange() {
	const uint firstOffset = kScene4050SceneObjectPaletteFirstColor * 3;
	const uint byteCount = (kScene4050SceneObjectPaletteLastColor -
		kScene4050SceneObjectPaletteFirstColor + 1) * 3;
	if (_paletteResource.size() < firstOffset + byteCount ||
			_paletteCurrent.size() < firstOffset + byteCount)
		return;

	memcpy(_paletteCurrent.data() + firstOffset, _paletteResource.data() + firstOffset, byteCount);
	_surfaceState.rebuildPresentationPaletteRemapTable();
	invalidatePresentationPalette();
}

void Scene4050::drawSceneLayers() {
	drawResourceSpriteLayer(_backgroundLayer);
	drawResourceSpriteLayer(_d09ReturnTransitionLayer);
	drawResourceSpriteLayer(_ronLayer);
}

void Scene4050::advanceBackgroundLayer(uint32 delta) {
	const uint frameCount = _backgroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte nextFrame = _backgroundLayer.frameIndex == 7 ? 0 : (byte)(_backgroundLayer.frameIndex + 1);
		_backgroundChannel.frameIndex = nextFrame;
		_backgroundLayer.setFrame(nextFrame);
	}
}

void Scene4050::advanceFlagPalette(uint32 delta) {
	const uint frameCount = _flagPaletteChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		rotateFlagPalette();
}

void Scene4050::rotateFlagPalette() {
	const uint firstColor = 0x80;
	const uint lastColor = 0xaf;
	if (_paletteCurrent.size() <= lastColor * 3 + 2)
		return;

	byte saved[3];
	memcpy(saved, &_paletteCurrent[lastColor * 3], sizeof(saved));
	for (uint color = lastColor; color > firstColor; --color)
		memcpy(&_paletteCurrent[color * 3], &_paletteCurrent[(color - 1) * 3], 3);
	memcpy(&_paletteCurrent[firstColor * 3], saved, sizeof(saved));
	invalidatePresentationPalette();
}

void Scene4050::advanceRonLayer(uint32 delta) {
	if (_ronManualSequenceActive)
		return;

	if (_speechOverlay.visible) {
		const uint frameCount = _ronSpeechChannel.consumeFrames(delta);
		for (uint frame = 0; frame < frameCount; ++frame) {
			byte nextFrame = _ronLayer.frameIndex;
			for (uint attempt = 0; attempt < 8 && nextFrame == _ronLayer.frameIndex; ++attempt)
				nextFrame = (byte)_random.getRandomNumber(kScene4050RonSpeechFrameCount - 1);
			if (nextFrame == _ronLayer.frameIndex)
				nextFrame = (byte)((_ronLayer.frameIndex + 1) % kScene4050RonSpeechFrameCount);
			setRonResourceFrame(nextFrame);
		}
		return;
	}

	const uint frameCount = _ronIdleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_ronLayer.frameIndex == kScene4050RonIdleBlinkFrame) {
			setRonResourceFrame(0);
		} else if (_random.getRandomNumber(14) == 0) {
			setRonResourceFrame(kScene4050RonIdleBlinkFrame);
		}
	}
}

void Scene4050::setRonResourceFrame(byte frameIndex) {
	_ronLayer.setFrame(frameIndex);
	_ronSpeechChannel.frameIndex = frameIndex;
	_ronIdleChannel.frameIndex = frameIndex;
}

void Scene4050::beginRonResourceSpeechLine(uint16 rowIndex, byte frameIndex) {
	int left = 0;
	int top = 0;
	int width = 0;
	int bottom = 0;
	uint16 centerX = _activeActorWorldX;
	uint16 topY = _activeActorWorldY;
	if (resourceDescriptorBounds(_ronLayer.chunkIndex, _ronLayer.descriptorCount,
			_ronLayer.descriptorIndex(), left, top, width, bottom)) {
		centerX = (uint16)(left + width / 2);
		topY = (uint16)top;
	}

	runSpeechLine(_speechOverlay, rowIndex, frameIndex, centerX, topY, kScene4050TextColor,
		true, false, false);
	setRonResourceFrame(0);
}

void Scene4050::runD09ReturnTransitionSequence() {
	resetAnimationLayers();
	_backgroundLayer.setFrame(0);
	_ronLayer.visible = false;
	_d09ReturnTransitionLayer.visible = true;
	_d09ReturnTransitionLayer.setFrame(0);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	const byte lastFrame = state.scene4090WideCoffinVariant == 0 ? 0x2e : 0x1a;
	_soundBank0.playSampleLooping(0x30, 100);
	playAnimationFrames(_d09ReturnTransitionLayer,
		AnimationFrameRange(0, lastFrame, kScene4050RonFrameMillis));
	_soundBank0.stop();

	state.mainFlowStateId = kScene4090ReturnState;
	state.activeActorPoseValid = false;
}

void Scene4050::useLongRopeOnLedge() {
	if (!hasInventoryItem(kScene4050LongRopeItem)) {
		beginRonResourceSpeechLine(8, 0);
		return;
	}

	beginRonResourceSpeechLine(8, 1);
	_ronManualSequenceActive = true;
	playAnimationFrames(_ronLayer,
		AnimationFrameRange(4, 0x11, kScene4050RonFrameMillis)
			.hookAt(6, kScene4050AttachRopeSoundHook));
	setRonResourceFrame(_ronLayer.frameIndex);
	_ronManualSequenceActive = false;
	removeInventoryItem(kScene4050LongRopeItem);
	_soundBank0.playSample(1, 100);
	_vm->gameState().scene4050RopeSwingState = kScene4050PatchStateRopeAttached;
	applySceneStateToHotspotsAndPatches(1);
	beginRonResourceSpeechLine(8, 2);
}

void Scene4050::useSceneRope() {
	GameplayState &state = _vm->gameState();
	if (state.scene4050RopeSwingState == kScene4050PatchStateWindowReached) {
		beginRonResourceSpeechLine(6, 2);
		return;
	}

	beginRonResourceSpeechLine(6, 1);
	if (_sceneChunkTable.isValidChunk(kScene4050ExitPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4050ExitPatchChunk], _baseFramebuffer);
	_ronManualSequenceActive = true;
	playAnimationFrames(_ronLayer,
		AnimationFrameRange(0x11, 0x1b, kScene4050RonFrameMillis)
			.hookAt(0x17, kScene4050SwingSoundHook));
	setRonResourceFrame(_ronLayer.frameIndex);
	_ronManualSequenceActive = false;
	state.scene4050RopeSwingState = kScene4050PatchStateWindowReached;
	state.mainFlowStateId = kScene4060FirstState;
}

void Scene4050::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene4050AttachRopeSoundHook && frame == 6)
		_soundBank0.playSample(0x2d, 100);
	else if (hookId == kScene4050SwingSoundHook && frame == 0x17)
		_soundBank0.playSample(0x2e, 100);
}

void Scene4050::applyPatchStateColorMap(byte patchState) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte item = _paletteMaskOriginal[kSceneColorToItemMap + i];
		byte mappedItem = item;
		if (patchState == kScene4050PatchStateBase) {
			if (item == 3 || item == 5)
				mappedItem = 0;
			else if (item == 6)
				mappedItem = 4;
		} else if (patchState == kScene4050PatchStateRopeAttached) {
			if (item == 3)
				mappedItem = 3;
			else if (item == 5)
				mappedItem = 0;
			else if (item == 6)
				mappedItem = 4;
		} else if (patchState == kScene4050PatchStateWindowReached) {
			if (item == 3)
				mappedItem = 0;
			else if (item == 5 || item == 6)
				mappedItem = 3;
		}
		_paletteMask[kSceneColorToItemMap + i] = mappedItem;
	}
}

bool Scene4050::resourceDescriptorBounds(uint chunkIndex, uint16 descriptorCount, uint16 descriptorIndex,
		int &left, int &top, int &width, int &bottom) const {
	if (chunkIndex >= HollywoodEngine::kResourceChunkCount || descriptorIndex >= descriptorCount)
		return false;

	const uint entryOffset = _resourceChunkOffsets[chunkIndex] + kFrameDescriptorSize * descriptorIndex;
	if (entryOffset + kFrameDescriptorSize > _resourceArena.size())
		return false;

	const uint32 packedWidth = readUint32LE(_resourceArena, entryOffset + 4);
	const uint32 packedRows = readUint32LE(_resourceArena, entryOffset + 8);
	left = packedWidth & 0xffff;
	width = (packedWidth >> 16) & 0xffff;
	top = packedRows & 0xffff;
	bottom = (packedRows >> 16) & 0xffff;
	return width > 0 && top <= bottom;
}

} // End of namespace Hollywood
