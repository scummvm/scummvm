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

#include "common/debug.h"

#include "hollywood/hollywood.h"
#include "hollywood/debug.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/intro/scene9050.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const char *const kI05ArchiveName = "RESOURCE.I05";
const char *const kI06ArchiveName = "RESOURCE.I06";
const char *const kI07ArchiveName = "RESOURCE.I07";
const char *const kI08ArchiveName = "RESOURCE.I08";
const char *const kStage9050SoundArchiveName = "RESOURCE.S09";
const uint16 kStage9050MusicCuePrelude = 0x000b;
const uint16 kStage9050MusicCueMain = 0x000c;
const byte kNoMusicCue = 0xff;
const byte kNoSoundCue = 0xff;
const byte kStage9050ClipSoundCue = 0x12;
const byte kStage9050I06SoundCue = 0x13;
const byte kStage9050FinalStartSoundCue = 0x17;
const byte kStage9050FinalEndSoundCue = 0x18;
const uint16 kScene9120MainFlowState = 0x23a0;
const byte kStage9050FirstClipSegmentId = 1;
const byte kStage9050FirstClipLastFrameIndex = 0x90;
const byte kStage9050SecondClipSegmentId = 2;
const byte kStage9050SecondClipLastFrameIndex = 0x22;
const byte kStage9050ThirdClipSegmentId = 3;
const byte kStage9050ThirdClipLastFrameIndex = 0x3f;
const byte kStage9050FourthClipSegmentId = 4;
const byte kStage9050FourthClipLastFrameIndex = 0x81;
const byte kStage9050FifthClipSegmentId = 5;
const byte kStage9050FifthClipLastFrameIndex = 0x0f;
const byte kStage9050SixthClipSegmentId = 6;
const byte kStage9050SixthClipLastFrameIndex = 0x29;
const byte kStage9050SeventhClipSegmentId = 7;
const byte kStage9050SeventhClipLastFrameIndex = 0x4b;
const byte kI05CombinedRevealEntriesPerSegment = 5;
const byte kI05LayeredRevealEntriesPerSegment = 6;
const uint kI05LayeredRevealLayoutMarkerChunk = 35;

struct Scene9050I06TimingProfile {
	uint16 sequenceDoneFrame;
	uint16 interludeStartFrame;
	uint16 interludeDoneFrame;
	uint16 frameCounterWrap;
	uint16 overlayEnableFrame;
	uint16 overlayDisableFrame;
	uint16 scriptedSequenceFrame;
};

const Scene9050I06TimingProfile kScene9050LaterEditionI06TimingProfile = {
	0x17f, 0x80, 0x1ff, 0x27f, 0x10, 0xa0, 0xc0
};

// RESOURCE.I06 is shared, but the first executable uses its first-half timing tables.
const Scene9050I06TimingProfile kScene9050FirstEditionI06TimingProfile = {
	0xbf, 0x40, 0xff, 0x13f, 8, 0x50, 0x60
};

const Scene9050I06TimingProfile &scene9050I06TimingProfile(const HollywoodEngine *vm) {
	return vm->isFirstEdition() ?
		kScene9050FirstEditionI06TimingProfile : kScene9050LaterEditionI06TimingProfile;
}

const byte kStage9050InterClipRevealFrames[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
const byte kStage9050InterClipReverseFrames[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0 };
const byte kStage9050ResourceI07FinalFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 6, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
};

const byte kStage9050ResourceI06SecondarySpriteFrameByHalfFrame[] = {
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	22, 21, 20, 19, 18, 17, 16, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
	30, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

const byte kStage9050ResourceI06PalettePulseAddends[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kStage9050ResourceI06PrimaryForwardFrames[] = { 0, 0x17, 0x18, 0x19, 0x1a };
const byte kStage9050ResourceI06PrimaryReturnFrames[] = { 0x19, 0x18, 0x17, 0 };
const byte kStage9050ResourceI06InterludeForwardFrames[] = { 0, 0x0f, 0x10, 0x11, 0x12 };
const byte kStage9050ResourceI06InterludeReturnFrames[] = { 0x11, 0x10, 0x0f, 0 };

int getStage9050ResourceI06ScrollDelta(uint frameIndex) {
	if (frameIndex >= 0xa0 && frameIndex < 0xc0)
		return -4;
	if (frameIndex >= 0x100 && frameIndex < 0x140)
		return 4;
	if (frameIndex >= 0x180 && frameIndex < 0x1c0)
		return -4;
	if (frameIndex >= 0x220 && frameIndex < 0x240)
		return 4;

	return 0;
}

Scene9050::Scene9050(HollywoodEngine *vm) :
		PresentationScene(vm, "Stage 9050"),
		_music(),
		_continuousSound(),
		_effectSound(),
		_random("hollywood_scene9050"),
		_i05ClipChunkSize(0),
		_i05ClipFrameCount(0),
		_i06ScrollAccumulator(0),
		_i06PrimarySpriteAccumulator(0),
		_i06SecondarySpriteAccumulator(0),
		_i06VerticalBobAccumulator(0),
		_i06PalettePulseAccumulator(0),
		_i06FrameCounter(0),
		_i06BaseImageScrollOffset(kI06InitialBaseScrollOffset),
		_i06PreviousBaseImageScrollOffset(kI06InitialBaseScrollOffset),
		_i06PrimarySpriteFrame(0),
		_i06PrimarySpriteSequenceState(0),
		_i06PrimarySpriteSequenceStep(0),
		_i06SecondarySpriteFrame(0x0f),
		_i06PreviousSecondarySpriteFrame(0x0f),
		_i06RandomBasePhase(0),
		_i06PreviousRandomBasePhase(0),
		_i06VerticalBobOffset(0),
		_i06VerticalBobTicksRemaining(0),
		_i06VerticalBobDelta(1),
		_i06PalettePulseStepIndex(0x18),
		_currentMusicCue(kNoMusicCue),
		_continuousSoundCue(kNoSoundCue),
		_i05EntriesPerSegment(0),
		_blockingAnimationMode(kNoBlockingAnimation),
		_blockingAnimationFrame(0),
		_blockingAnimationChunk(0),
		_i06OptionalOverlayChunk5Enabled(false),
		_i06BaseFrameDirty(false),
		_i06PrimarySpriteDirty(false),
		_i06SecondarySpriteDirty(false),
		_i06CompositeForceDirty(false),
		_i06PaletteDirty(false),
		_i06SequenceFinished(false) {
	_paletteResource.resize(kPaletteSize);
	_clipBaseFramebuffer.resize(kFrameBufferSize);
	_continuousSound.setArchive(Common::Path(kStage9050SoundArchiveName));
	_effectSound.setArchive(Common::Path(kStage9050SoundArchiveName));
}

bool Scene9050::play() {
	if (!loadResourceI06Assets())
		return false;

	bool result = true;
	runResourceI06AnimatedPresentation();

	if (result && !_skipRequested && !Engine::shouldQuit())
		result = playResourceI05ClipSegment(kStage9050FirstClipSegmentId, kStage9050FirstClipLastFrameIndex, false);
	if (result && !_skipRequested && !Engine::shouldQuit())
		runStage9050InterClipSpriteReveal();
	if (result && !_skipRequested && !Engine::shouldQuit())
		advanceStage9050Cutscene();
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = runResourceI08BlinkSequence();
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = playResourceI05ClipSegment(kStage9050SecondClipSegmentId, kStage9050SecondClipLastFrameIndex, false);
	if (result && !_skipRequested && !Engine::shouldQuit())
		runStage9050InterClipSpriteReveal();
	if (result && !_skipRequested && !Engine::shouldQuit())
		advanceStage9050Cutscene();
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = playResourceI05ClipSegment(kStage9050ThirdClipSegmentId, kStage9050ThirdClipLastFrameIndex, true);
	if (result && !_skipRequested && !Engine::shouldQuit())
		runStage9050InterClipSpriteReveal();
	if (result && !_skipRequested && !Engine::shouldQuit())
		advanceStage9050Cutscene();

	if (result && !_skipRequested && !Engine::shouldQuit()) {
		result = loadResourceI06Assets();
		if (result)
			result = runResourceI06ScrollInterlude();
	}
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = playResourceI05ClipSegment(kStage9050FourthClipSegmentId, kStage9050FourthClipLastFrameIndex, false);
	if (result && !_skipRequested && !Engine::shouldQuit())
		runStage9050InterClipSpriteReveal();
	if (result && !_skipRequested && !Engine::shouldQuit())
		advanceStage9050Cutscene();
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = runResourceI08BlinkSequence();
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = playResourceI05ClipSegment(kStage9050FifthClipSegmentId, kStage9050FifthClipLastFrameIndex, false);
	if (result && !_skipRequested && !Engine::shouldQuit())
		runStage9050InterClipSpriteReveal();
	if (result && !_skipRequested && !Engine::shouldQuit())
		advanceStage9050Cutscene();
	if (result && !_skipRequested && !Engine::shouldQuit()) {
		result = loadResourceI06Assets();
		if (result)
			result = runResourceI06SpriteInterlude();
	}
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = playResourceI05ClipSegment(kStage9050SixthClipSegmentId, kStage9050SixthClipLastFrameIndex, false);
	if (result && !_skipRequested && !Engine::shouldQuit())
		runStage9050InterClipSpriteReveal();
	if (result && !_skipRequested && !Engine::shouldQuit())
		advanceStage9050Cutscene();
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = playResourceI05ClipSegment(kStage9050SeventhClipSegmentId, kStage9050SeventhClipLastFrameIndex, true);
	if (result && !_skipRequested && !Engine::shouldQuit())
		result = runResourceI07FinalAnimation();
	if (result && !_skipRequested && !Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene9120MainFlowState;

	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	return result;
}

bool Scene9050::loadResourceI06Assets() {
	if (!_resources.loadChunkTable(kI06ArchiveName))
		return false;

	for (uint i = 0; i < kI06RequiredChunkCount; ++i) {
		if (!_resources.validateChunk(kI06ArchiveName, _debugName, i))
			return false;
	}

	_resources.allocateArena(_resources.totalChunkSize(1, kI06RequiredChunkCount - 1));

	if (!loadFixedChunk(0, _paletteResource, kPaletteSize))
		return false;

	for (uint i = 1; i < kI06RequiredChunkCount; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	return true;
}

bool Scene9050::loadResourceI05ClipSegment(byte segmentId) {
	if (segmentId == 0) {
		warning("Invalid %s Stage 9050 segment 0", kI05ArchiveName);
		return false;
	}

	if (!_resources.loadChunkTable(kI05ArchiveName))
		return false;

	// Italian archives combine the two Spanish reveal layers into one chunk.
	_i05EntriesPerSegment = _resources._chunkTable.isValidChunk(kI05LayeredRevealLayoutMarkerChunk) ?
		kI05LayeredRevealEntriesPerSegment : kI05CombinedRevealEntriesPerSegment;
	const uint baseIndex = ((uint)segmentId - 1) * _i05EntriesPerSegment;
	const bool finalLayeredSegment = _i05EntriesPerSegment == kI05LayeredRevealEntriesPerSegment &&
		segmentId == kStage9050SeventhClipSegmentId;
	const uint lastLocalChunkIndex = finalLayeredSegment ? 3 : _i05EntriesPerSegment - 1;
	if (baseIndex + lastLocalChunkIndex >= kResourceChunkCount) {
		warning("%s Stage 9050 segment %u exceeds the archive chunk table", kI05ArchiveName, segmentId);
		return false;
	}

	for (uint i = 0; i <= lastLocalChunkIndex; ++i) {
		if (!_resources.validateChunk(kI05ArchiveName, _debugName, baseIndex + i))
			return false;
	}

	uint32 resourceArenaSize = 0;
	for (uint i = 2; i <= lastLocalChunkIndex; ++i)
		resourceArenaSize += _resources._chunkTable.sizes[baseIndex + i];
	_resources.allocateArena(resourceArenaSize);

	if (!loadFixedChunk(baseIndex, _clipBaseFramebuffer, kFrameBufferSize))
		return false;
	if (!loadFixedChunk(baseIndex + 1, _paletteResource, kPaletteSize))
		return false;
	_i05ClipChunkSize = _resources._chunkTable.sizes[baseIndex + 3];

	for (uint i = 2; i <= lastLocalChunkIndex; ++i) {
		if (!loadArenaChunk(baseIndex + i, i))
			return false;
	}

	return true;
}

bool Scene9050::loadResourceI08BlinkAssets() {
	if (!_resources.loadChunkTable(kI08ArchiveName))
		return false;

	for (uint i = 0; i < kI08RequiredChunkCount; ++i) {
		if (!_resources.validateChunk(kI08ArchiveName, _debugName, i))
			return false;
	}

	_resources.allocateArena(_resources._chunkTable.sizes[2]);

	if (!loadFixedChunk(0, _clipBaseFramebuffer, kFrameBufferSize))
		return false;
	if (!loadFixedChunk(1, _paletteResource, kPaletteSize))
		return false;
	if (!loadArenaChunk(2))
		return false;

	return true;
}

bool Scene9050::loadResourceI07FinalAssets() {
	if (!_resources.loadChunkTable(kI07ArchiveName))
		return false;

	for (uint i = 0; i < kI07RequiredChunkCount; ++i) {
		if (!_resources.validateChunk(kI07ArchiveName, _debugName, i))
			return false;
	}

	_resources.allocateArena(_resources._chunkTable.sizes[2]);

	if (!loadFixedChunk(0, _clipBaseFramebuffer, kFrameBufferSize))
		return false;
	if (!loadFixedChunk(1, _paletteResource, kPaletteSize))
		return false;
	if (!loadArenaChunk(2))
		return false;

	return true;
}

void Scene9050::runResourceI06AnimatedPresentation() {
	initializeResourceI06AnimatedPresentation();

	if (revealSavedFramebufferWithCurtain())
		return;

	if (runResourceI06AnimationLoop(false, false))
		return;

	clearSceneFramebufferWithCurtain();
}

void Scene9050::initializeResourceI06AnimatedPresentation() {
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memset(_savedFramebuffer.data(), 0, _savedFramebuffer.size());
	presentFrame();

	_i06ScrollAccumulator = 0;
	_i06PrimarySpriteAccumulator = 0;
	_i06SecondarySpriteAccumulator = 0;
	_i06VerticalBobAccumulator = 0;
	_i06PalettePulseAccumulator = 0;
	_i06FrameCounter = 0;
	_i06BaseImageScrollOffset = kI06InitialBaseScrollOffset;
	_i06PreviousBaseImageScrollOffset = kI06InitialBaseScrollOffset;
	_i06PrimarySpriteFrame = 0;
	_i06PrimarySpriteSequenceState = 0;
	_i06PrimarySpriteSequenceStep = 0;
	_i06SecondarySpriteFrame = 0x0f;
	_i06PreviousSecondarySpriteFrame = 0x0f;
	_i06RandomBasePhase = 0;
	_i06PreviousRandomBasePhase = 0;
	_i06VerticalBobOffset = 0;
	_i06VerticalBobTicksRemaining = 0;
	_i06VerticalBobDelta = 1;
	_i06PalettePulseStepIndex = 0x18;
	_i06OptionalOverlayChunk5Enabled = false;
	_i06SequenceFinished = false;
	_currentMusicCue = kNoMusicCue;
	markResourceI06CompositeDirty();

	presentResourceI06AnimatedFrame();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _sceneFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	presentFrame();
}

bool Scene9050::runResourceI06ScrollInterlude() {
	return runResourceI06Interlude(false);
}

bool Scene9050::runResourceI06SpriteInterlude() {
	return runResourceI06Interlude(true);
}

bool Scene9050::runResourceI06Interlude(bool runScriptedSpriteSequence) {
	stopContinuousSound();

	if (clearSceneFramebufferWithCurtain())
		return true;

	if (_skipRequested || Engine::shouldQuit())
		return true;

	initializeResourceI06Interlude();
	if (revealSavedFramebufferWithCurtain())
		return true;

	if (runResourceI06AnimationLoop(true, runScriptedSpriteSequence))
		return true;

	clearSceneFramebufferWithCurtain();
	return true;
}

void Scene9050::initializeResourceI06Interlude() {
	const Scene9050I06TimingProfile &timing = scene9050I06TimingProfile(_vm);

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	_i06ScrollAccumulator = 0;
	_i06PrimarySpriteAccumulator = 0;
	_i06SecondarySpriteAccumulator = 0;
	_i06VerticalBobAccumulator = 0;
	_i06PalettePulseAccumulator = 0;
	_i06FrameCounter = timing.interludeStartFrame;
	_i06BaseImageScrollOffset = kI06InitialBaseScrollOffset;
	_i06PreviousBaseImageScrollOffset = kI06InitialBaseScrollOffset;
	_i06PrimarySpriteFrame = 0;
	_i06PrimarySpriteSequenceState = 0;
	_i06PrimarySpriteSequenceStep = 0;
	_i06SecondarySpriteFrame = 0x0f;
	_i06PreviousSecondarySpriteFrame = 0x0f;
	_i06RandomBasePhase = 0;
	_i06PreviousRandomBasePhase = 0;
	_i06VerticalBobOffset = 0;
	_i06VerticalBobTicksRemaining = 0;
	_i06VerticalBobDelta = 1;
	_i06PalettePulseStepIndex = 0x18;
	_i06OptionalOverlayChunk5Enabled = false;
	_i06SequenceFinished = false;
	markResourceI06CompositeDirty();

	presentResourceI06AnimatedFrame();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _sceneFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	presentFrame();
}

void Scene9050::copyResourceI06ScrolledBaseFrame() {
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());

	const uint32 chunkOffset = _resources._chunkOffsets[1];
	const uint32 chunkSize = _resources._chunkTable.sizes[1];
	if (_i06BaseImageScrollOffset >= chunkSize || chunkOffset + chunkSize > _resources._arena.size())
		return;

	const uint32 copySize = MIN<uint32>(chunkSize - _i06BaseImageScrollOffset, _sceneFramebuffer.size());
	memcpy(_sceneFramebuffer.data(), _resources._arena.data() + chunkOffset + _i06BaseImageScrollOffset, copySize);
}

void Scene9050::presentResourceI06AnimatedFrame() {
	const bool redrawFrame = _i06BaseFrameDirty || _i06PrimarySpriteDirty ||
		_i06SecondarySpriteDirty || _i06CompositeForceDirty;

	if (redrawFrame) {
		copyResourceI06ScrolledBaseFrame();
		drawResourceBlockList(_resources._arena, _resources._chunkOffsets[4], _sceneFramebuffer.surface(), _i06RandomBasePhase);
		drawResourceI06AnimatedFrame(2, (byte)(_i06PrimarySpriteFrame + _i06VerticalBobOffset));
		drawResourceI06AnimatedFrame(3, _i06SecondarySpriteFrame);
		if (_i06OptionalOverlayChunk5Enabled)
			drawResourceBlockList(_resources._arena, _resources._chunkOffsets[5], _sceneFramebuffer.surface());

		_i06BaseFrameDirty = false;
		_i06PrimarySpriteDirty = false;
		_i06SecondarySpriteDirty = false;
		_i06CompositeForceDirty = false;
		_i06PaletteDirty = false;
		presentFrame();
	} else if (_i06PaletteDirty) {
		_i06PaletteDirty = false;
		presentFrame();
	}
}

void Scene9050::drawResourceI06AnimatedFrame(byte chunkIndex, byte frameIndex) {
	if (chunkIndex >= kResourceChunkCount)
		return;

	drawStripSpriteFrame(_resources._arena, _resources._chunkOffsets[chunkIndex], 0,
		kI06AnimatedFrameDescriptorCount, frameIndex, _sceneFramebuffer.surface(), _i06RandomBasePhase);
}

void Scene9050::advanceResourceI06Timers(uint32 millis) {
	_i06ScrollAccumulator += millis;
	if (_i06ScrollAccumulator >= 60) {
		_i06ScrollAccumulator %= 60;
		advanceResourceI06Scroll();
	}

	_i06SecondarySpriteAccumulator += millis;
	if (_i06SecondarySpriteAccumulator >= 75) {
		_i06SecondarySpriteAccumulator %= 75;
		advanceResourceI06SecondarySprite();
	}

	_i06PrimarySpriteAccumulator += millis;
	if (_i06PrimarySpriteAccumulator >= 75) {
		_i06PrimarySpriteAccumulator %= 75;
		advanceResourceI06PrimarySprite();
	}

	_i06VerticalBobAccumulator += millis;
	if (_i06VerticalBobAccumulator >= 100) {
		_i06VerticalBobAccumulator %= 100;
		advanceResourceI06VerticalBob();
	}

	_i06PalettePulseAccumulator += millis;
	if (_i06PalettePulseAccumulator >= 50) {
		_i06PalettePulseAccumulator %= 50;
		advanceResourceI06PalettePulse();
	}
}

void Scene9050::advanceResourceI06InterludeTimers(uint32 millis, bool runScriptedSpriteSequence) {
	_i06ScrollAccumulator += millis;
	if (_i06ScrollAccumulator >= 60) {
		_i06ScrollAccumulator %= 60;
		advanceResourceI06InterludeScroll(runScriptedSpriteSequence);
	}

	_i06SecondarySpriteAccumulator += millis;
	if (_i06SecondarySpriteAccumulator >= 75) {
		_i06SecondarySpriteAccumulator %= 75;
		advanceResourceI06SecondarySprite();
	}

	_i06PrimarySpriteAccumulator += millis;
	if (_i06PrimarySpriteAccumulator >= 75) {
		_i06PrimarySpriteAccumulator %= 75;
		advanceResourceI06InterludePrimarySprite(runScriptedSpriteSequence);
	}

	_i06VerticalBobAccumulator += millis;
	if (_i06VerticalBobAccumulator >= 100) {
		_i06VerticalBobAccumulator %= 100;
		advanceResourceI06VerticalBob();
	}

	_i06PalettePulseAccumulator += millis;
	if (_i06PalettePulseAccumulator >= 50) {
		_i06PalettePulseAccumulator %= 50;
		advanceResourceI06PalettePulse();
	}
}

void Scene9050::advanceResourceI06Scroll() {
	const Scene9050I06TimingProfile &timing = scene9050I06TimingProfile(_vm);

	if (_i06FrameCounter == timing.frameCounterWrap) {
		_i06FrameCounter = 0;
		_i06SequenceFinished = true;
	} else if (_i06PrimarySpriteSequenceState < 2) {
		_i06FrameCounter++;
	}

	if (_i06FrameCounter == timing.sequenceDoneFrame)
		_i06SequenceFinished = true;

	if (_i06FrameCounter == timing.overlayEnableFrame) {
		_i06OptionalOverlayChunk5Enabled = true;
		_i06BaseFrameDirty = true;
	}
	if (_i06FrameCounter == timing.overlayDisableFrame) {
		_i06OptionalOverlayChunk5Enabled = false;
		_i06BaseFrameDirty = true;
	}
	if (_i06FrameCounter == timing.scriptedSequenceFrame && _i06PrimarySpriteSequenceState < 2) {
		_i06PrimarySpriteSequenceStep = 0;
		_i06PrimarySpriteSequenceState = 2;
	}

	const int scrollOffset = (int)_i06BaseImageScrollOffset + getStage9050ResourceI06ScrollDelta(_i06FrameCounter);
	_i06BaseImageScrollOffset = (uint16)MAX<int>(0, MIN<int>(scrollOffset, _resources._chunkTable.sizes[1] - 1));
	if (_i06PreviousBaseImageScrollOffset != _i06BaseImageScrollOffset) {
		_i06BaseFrameDirty = true;
		_i06PreviousBaseImageScrollOffset = _i06BaseImageScrollOffset;
	}

	if ((_i06FrameCounter & 1) == 0) {
		_i06PreviousRandomBasePhase = _i06RandomBasePhase;
		_i06RandomBasePhase = (byte)_random.getRandomNumber(1);
		if (_i06RandomBasePhase != _i06PreviousRandomBasePhase)
			markResourceI06CompositeDirty();
	}
}

void Scene9050::advanceResourceI06InterludeScroll(bool runScriptedSpriteSequence) {
	const Scene9050I06TimingProfile &timing = scene9050I06TimingProfile(_vm);

	if (_i06FrameCounter == timing.frameCounterWrap) {
		_i06FrameCounter = 0;
		_i06SequenceFinished = true;
	} else {
		_i06FrameCounter++;
	}

	if (_i06FrameCounter == timing.interludeDoneFrame)
		_i06SequenceFinished = true;

	if (runScriptedSpriteSequence && _i06FrameCounter == timing.scriptedSequenceFrame) {
		_i06PrimarySpriteSequenceStep = 0;
		_i06PrimarySpriteSequenceState = 2;
	}

	const int scrollOffset = (int)_i06BaseImageScrollOffset + getStage9050ResourceI06ScrollDelta(_i06FrameCounter);
	_i06BaseImageScrollOffset = (uint16)MAX<int>(0, MIN<int>(scrollOffset, _resources._chunkTable.sizes[1] - 1));
	if (_i06PreviousBaseImageScrollOffset != _i06BaseImageScrollOffset) {
		_i06BaseFrameDirty = true;
		_i06PreviousBaseImageScrollOffset = _i06BaseImageScrollOffset;
	}

	if ((_i06FrameCounter & 1) == 0) {
		_i06PreviousRandomBasePhase = _i06RandomBasePhase;
		_i06RandomBasePhase = (byte)_random.getRandomNumber(1);
		if (_i06RandomBasePhase != _i06PreviousRandomBasePhase)
			markResourceI06CompositeDirty();
	}
}

void Scene9050::advanceResourceI06SecondarySprite() {
	const uint tableIndex = _i06FrameCounter >> 1;
	if (tableIndex >= ARRAYSIZE(kStage9050ResourceI06SecondarySpriteFrameByHalfFrame))
		return;

	_i06SecondarySpriteFrame = kStage9050ResourceI06SecondarySpriteFrameByHalfFrame[tableIndex];
	if (_i06PreviousSecondarySpriteFrame != _i06SecondarySpriteFrame) {
		_i06SecondarySpriteDirty = true;
		_i06PreviousSecondarySpriteFrame = _i06SecondarySpriteFrame;
	}
}

void Scene9050::advanceResourceI06PrimarySprite() {
	if (_i06PrimarySpriteSequenceState == 4) {
		if (_i06PrimarySpriteSequenceStep == 4) {
			_i06PrimarySpriteSequenceState = 0;
		} else {
			_i06PrimarySpriteFrame = kStage9050ResourceI06PrimaryReturnFrames[_i06PrimarySpriteSequenceStep];
			_i06PrimarySpriteSequenceStep++;
			_i06PrimarySpriteDirty = true;
		}
		return;
	}

	if (_i06PrimarySpriteSequenceState == 3) {
		if (!_music.isPlaying()) {
			_currentMusicCue = kStage9050MusicCueMain;
			_music.playMusicCue(_currentMusicCue, 100);
		} else if (_currentMusicCue == kStage9050MusicCueMain) {
			if (_i06PrimarySpriteSequenceStep == 0x18) {
				_i06PrimarySpriteSequenceStep = 0;
				_i06PrimarySpriteSequenceState = 4;
			} else {
				_i06PrimarySpriteSequenceStep++;
			}
		}
		return;
	}

	if (_i06PrimarySpriteSequenceState == 2) {
		if (_i06PrimarySpriteSequenceStep == 4) {
			_i06PrimarySpriteSequenceState = 3;
			_currentMusicCue = kStage9050MusicCuePrelude;
			_music.playMusicCue(_currentMusicCue, 100);
		} else {
			_i06PrimarySpriteSequenceStep++;
			_i06PrimarySpriteFrame = kStage9050ResourceI06PrimaryForwardFrames[_i06PrimarySpriteSequenceStep];
			_i06PrimarySpriteDirty = true;
		}
		return;
	}

	if (_i06PrimarySpriteSequenceState == 1) {
		_i06PrimarySpriteFrame = 0;
		_i06PrimarySpriteSequenceState = 0;
		_i06PrimarySpriteDirty = true;
		return;
	}

	if (_random.getRandomNumber(0x18) == 0) {
		_i06PrimarySpriteFrame = 10;
		_i06PrimarySpriteSequenceState = 1;
		_i06PrimarySpriteDirty = true;
	}
}

void Scene9050::advanceResourceI06InterludePrimarySprite(bool runScriptedSpriteSequence) {
	if (runScriptedSpriteSequence) {
		if (_i06PrimarySpriteSequenceState == 4) {
			if (_i06PrimarySpriteSequenceStep == ARRAYSIZE(kStage9050ResourceI06InterludeReturnFrames)) {
				_i06PrimarySpriteSequenceState = 0;
			} else {
				_i06PrimarySpriteFrame = kStage9050ResourceI06InterludeReturnFrames[_i06PrimarySpriteSequenceStep];
				_i06PrimarySpriteSequenceStep++;
				_i06PrimarySpriteDirty = true;
			}
			return;
		}

		if (_i06PrimarySpriteSequenceState == 3) {
			if (_i06PrimarySpriteSequenceStep == 0x10) {
				_i06PrimarySpriteSequenceStep = 0;
				_i06PrimarySpriteSequenceState = 4;
			} else {
				_i06PrimarySpriteSequenceStep++;
			}
			return;
		}

		if (_i06PrimarySpriteSequenceState == 2) {
			if (_i06PrimarySpriteSequenceStep == 4) {
				_i06PrimarySpriteSequenceState = 3;
			} else {
				_i06PrimarySpriteSequenceStep++;
				_i06PrimarySpriteFrame = kStage9050ResourceI06InterludeForwardFrames[_i06PrimarySpriteSequenceStep];
				_i06PrimarySpriteDirty = true;
			}
			return;
		}
	}

	if (_i06PrimarySpriteSequenceState == 1) {
		_i06PrimarySpriteFrame = 0;
		_i06PrimarySpriteSequenceState = 0;
		_i06PrimarySpriteDirty = true;
		return;
	}

	if (_random.getRandomNumber(0x18) == 0) {
		_i06PrimarySpriteFrame = 10;
		_i06PrimarySpriteSequenceState = 1;
		_i06PrimarySpriteDirty = true;
	}
}

void Scene9050::advanceResourceI06VerticalBob() {
	if (_i06PrimarySpriteSequenceState == 2 || _i06PrimarySpriteSequenceState == 4) {
		_i06VerticalBobOffset = 0;
		return;
	}

	if (_i06VerticalBobTicksRemaining == 0) {
		_i06VerticalBobTicksRemaining = (byte)(_random.getRandomNumber(3) + 2);
		_i06VerticalBobDelta = _random.getRandomNumber(1) == 0 ? -1 : 1;
		return;
	}

	const int newOffset = (int)_i06VerticalBobOffset + _i06VerticalBobDelta;
	if (newOffset == 5)
		_i06VerticalBobOffset = 0;
	else if (newOffset < 0)
		_i06VerticalBobOffset = 4;
	else
		_i06VerticalBobOffset = (byte)newOffset;

	_i06PrimarySpriteDirty = true;
	_i06VerticalBobTicksRemaining--;
}

void Scene9050::advanceResourceI06PalettePulse() {
	if (_i06PalettePulseStepIndex == 0x77)
		_i06PalettePulseStepIndex = 0;
	else
		_i06PalettePulseStepIndex++;

	if (_i06PalettePulseStepIndex >= ARRAYSIZE(kStage9050ResourceI06PalettePulseAddends))
		return;

	const byte addend = kStage9050ResourceI06PalettePulseAddends[_i06PalettePulseStepIndex];
	for (uint paletteIndex = 1; paletteIndex < 0x94; ++paletteIndex) {
		for (uint channel = 0; channel < 3; ++channel) {
			const uint offset = paletteIndex * 3 + channel;
			_paletteCurrent[offset] = MIN<byte>(0x3f, _paletteResource[offset] + addend);
		}
	}
	_i06PaletteDirty = true;
}

void Scene9050::markResourceI06CompositeDirty() {
	_i06BaseFrameDirty = true;
	_i06PrimarySpriteDirty = true;
	_i06SecondarySpriteDirty = true;
	_i06CompositeForceDirty = true;
}

bool Scene9050::runResourceI06AnimationLoop(bool interlude,
		bool runScriptedSpriteSequence) {
	if (pollEvents()) {
		if (_skipRequested || Engine::shouldQuit())
			return true;
		consumeStepAdvanceRequest();
		finishResourceI06AnimationLoop(interlude);
		return false;
	}

	_i06ScrollAccumulator = 60;
	_i06SecondarySpriteAccumulator = 75;
	_i06PrimarySpriteAccumulator = 75;
	_i06VerticalBobAccumulator = 100;
	_i06PalettePulseAccumulator = 50;

	uint32 elapsed = 0;
	while (!_i06SequenceFinished && !_skipRequested && !Engine::shouldQuit()) {
		ensureContinuousSound(kStage9050I06SoundCue, 10);
		if (interlude)
			advanceResourceI06InterludeTimers(elapsed, runScriptedSpriteSequence);
		else
			advanceResourceI06Timers(elapsed);
		presentResourceI06AnimatedFrame();
		if (_i06SequenceFinished)
			break;
		if (delay(10)) {
			if (_skipRequested || Engine::shouldQuit())
				return true;
			finishResourceI06AnimationLoop(interlude);
			return false;
		}
		elapsed = 10;
	}

	return _skipRequested || Engine::shouldQuit();
}

void Scene9050::finishResourceI06AnimationLoop(bool interlude) {
	const Scene9050I06TimingProfile &timing = scene9050I06TimingProfile(_vm);
	const uint firstFrame = interlude ? timing.interludeStartFrame + 1 : 1;
	const uint finalFrame = interlude ? timing.interludeDoneFrame : timing.sequenceDoneFrame;

	int scrollOffset = kI06InitialBaseScrollOffset;
	for (uint frame = firstFrame; frame <= finalFrame; ++frame)
		scrollOffset += getStage9050ResourceI06ScrollDelta(frame);

	const uint32 baseImageSize = _resources._chunkTable.sizes[1];
	const int maximumScrollOffset = baseImageSize == 0 ? 0 : (int)baseImageSize - 1;
	_i06FrameCounter = finalFrame;
	_i06BaseImageScrollOffset = (uint16)CLIP<int>(scrollOffset, 0, maximumScrollOffset);
	_i06PreviousBaseImageScrollOffset = _i06BaseImageScrollOffset;
	_i06PrimarySpriteFrame = 0;
	_i06PrimarySpriteSequenceState = 0;
	_i06PrimarySpriteSequenceStep = 0;
	_i06VerticalBobOffset = 0;
	_i06VerticalBobTicksRemaining = 0;
	_i06OptionalOverlayChunk5Enabled = false;

	const uint secondaryFrameIndex = finalFrame >> 1;
	if (secondaryFrameIndex < ARRAYSIZE(kStage9050ResourceI06SecondarySpriteFrameByHalfFrame))
		_i06SecondarySpriteFrame = kStage9050ResourceI06SecondarySpriteFrameByHalfFrame[secondaryFrameIndex];
	_i06PreviousSecondarySpriteFrame = _i06SecondarySpriteFrame;
	_i06SequenceFinished = true;

	if (!interlude && (_currentMusicCue != kStage9050MusicCueMain || !_music.isPlaying())) {
		_currentMusicCue = kStage9050MusicCueMain;
		_music.playMusicCue(_currentMusicCue, 100);
	}

	markResourceI06CompositeDirty();
	presentResourceI06AnimatedFrame();
}

void Scene9050::ensureContinuousSound(byte cueId, byte volumePercent) {
	if (_continuousSoundCue != cueId) {
		_continuousSound.stop();
		_continuousSoundCue = cueId;
	}
	if (!_continuousSound.isPlaying())
		_continuousSound.playSample(cueId, volumePercent);
}

void Scene9050::stopContinuousSound() {
	_continuousSound.stop();
	_continuousSoundCue = kNoSoundCue;
}

void Scene9050::runResourceI05Clip(byte segmentId, byte lastFrameIndex, bool fadeInBeforePlayback) {
	debugC(1, kDebugScene, "Playing Stage 9050 %s segment %u to frame %u", kI05ArchiveName, segmentId, lastFrameIndex);

	if (fadeInBeforePlayback && clearSceneFramebufferWithCurtain())
		return;

	if (_skipRequested || Engine::shouldQuit())
		return;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	memcpy(_savedFramebuffer.data(), _clipBaseFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	if (revealSavedFramebufferWithCurtain())
		return;

	if (waitResourceI05ClipHold())
		return;

	if (lastFrameIndex == 0)
		return;
	_i05ClipFrameCount = lastFrameIndex;
	playBlockingAnimation(kResourceI05ClipAnimation, 0,
		lastFrameIndex - 1, 50);
}

void Scene9050::drawResourceI05ClipFrameDelta(byte lastFrameIndex, byte frameIndex) {
	drawResourceDeltaClipFrame(_resources._arena, _resources._chunkOffsets[3],
		_i05ClipChunkSize, lastFrameIndex, frameIndex, _sceneFramebuffer.data(),
		_sceneFramebuffer.size());
}

bool Scene9050::waitResourceI05ClipHold() {
	if (_skipRequested || Engine::shouldQuit())
		return true;

	return waitSceneCounterPast(2);
}

bool Scene9050::playResourceI05ClipSegment(byte segmentId, byte lastFrameIndex, bool fadeInBeforePlayback) {
	if (_continuousSoundCue == kStage9050I06SoundCue)
		stopContinuousSound();

	if (!loadResourceI05ClipSegment(segmentId))
		return false;

	runResourceI05Clip(segmentId, lastFrameIndex, fadeInBeforePlayback);
	return !_skipRequested && !Engine::shouldQuit();
}

void Scene9050::runStage9050InterClipSpriteReveal() {
	const uint32 paletteOffset = _resources._chunkOffsets[2];
	if (paletteOffset + kPaletteSize <= _resources._arena.size()) {
		memcpy(_paletteCurrent.data(), _resources._arena.data() + paletteOffset, kPaletteSize);
		memcpy(_paletteResource.data(), _paletteCurrent.data(), kPaletteSize);
		presentFrame();
	}

	runResourceI05InterClipRevealPhase(4);
	if (_i05EntriesPerSegment < kI05LayeredRevealEntriesPerSegment)
		return;

	if (waitSceneCounterPast(1))
		return;

	runResourceI05InterClipRevealPhase(5);
}

void Scene9050::advanceStage9050Cutscene() {
	if (waitSceneCounterPast(4))
		return;

	runResourceI05InterClipReversePhase();
}

void Scene9050::runResourceI05InterClipRevealPhase(byte localChunkIndex) {
	playBlockingAnimation(kInterClipRevealAnimation, 0,
		ARRAYSIZE(kStage9050InterClipRevealFrames) - 1, 60, localChunkIndex);
}

void Scene9050::runResourceI05InterClipReversePhase() {
	playBlockingAnimation(kInterClipReverseAnimation, 0,
		ARRAYSIZE(kStage9050InterClipReverseFrames) - 1, 60);
}

void Scene9050::restoreAndDrawResourceDescriptorFrame(byte localChunkIndex, byte descriptorCount, byte descriptorIndex,
		bool drawFrame) {
	if (localChunkIndex >= kResourceChunkCount)
		return;

	const uint32 baseOffset = _resources._chunkOffsets[localChunkIndex];
	restoreSpriteBackground(_resources._arena, baseOffset, 0, descriptorCount, descriptorIndex,
		_clipBaseFramebuffer.surface(), _sceneFramebuffer.surface());
	if (drawFrame)
		drawStripSpriteFrame(_resources._arena, baseOffset, 0, descriptorCount, descriptorIndex, _sceneFramebuffer.surface());
}

bool Scene9050::runResourceI08BlinkSequence() {
	stopContinuousSound();

	if (!loadResourceI08BlinkAssets())
		return false;

	if (clearSceneFramebufferWithCurtain())
		return true;

	if (_skipRequested || Engine::shouldQuit())
		return true;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	memcpy(_savedFramebuffer.data(), _clipBaseFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	if (revealSavedFramebufferWithCurtain())
		return true;

	if (waitResourceI08BlinkLoop(6000))
		return true;

	clearSceneFramebufferWithCurtain();

	return true;
}

bool Scene9050::waitResourceI08BlinkLoop(uint32 millis) {
	const uint32 frameCount = (millis + 49) / 50;
	if (frameCount == 0)
		return false;
	if (frameCount > 0x100)
		return true;

	return !playBlockingAnimation(kResourceI08BlinkAnimation, 0,
		(byte)(frameCount - 1), 50, 0, true);
}

bool Scene9050::runResourceI07FinalAnimation() {
	if (!loadResourceI07FinalAssets())
		return false;

	if (clearSceneFramebufferWithCurtain())
		return true;

	if (_skipRequested || Engine::shouldQuit())
		return true;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	memcpy(_savedFramebuffer.data(), _clipBaseFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	presentFrame();
	if (revealSavedFramebufferWithCurtain())
		return true;

	playBlockingAnimation(kResourceI07FinalAnimation, 1,
		ARRAYSIZE(kStage9050ResourceI07FinalFrameMap) - 1, 75);

	return true;
}

bool Scene9050::playBlockingAnimation(BlockingAnimationMode mode,
		byte firstFrame, byte lastFrame, uint32 frameMillis, byte chunkIndex,
		bool waitAfterFinalFrame) {
	_blockingAnimationMode = mode;
	_blockingAnimationFrame = firstFrame;
	_blockingAnimationChunk = chunkIndex;

	AnimationFrameRange range(firstFrame, lastFrame, frameMillis);
	if (!waitAfterFinalFrame)
		range.noFinalFrameDelay();
	bool completed = _animationPlayer.playAndPresent(_blockingAnimationFrame, range);
	if (!completed && !_skipRequested && !Engine::shouldQuit()) {
		if (mode == kResourceI05ClipAnimation) {
			while (_blockingAnimationFrame < lastFrame) {
				_blockingAnimationFrame++;
				drawResourceI05ClipFrameDelta(_i05ClipFrameCount, _blockingAnimationFrame);
			}
			presentFrame();
		} else {
			_blockingAnimationFrame = lastFrame;
			presentAnimationFrame();
		}
		completed = true;
	}
	_blockingAnimationMode = kNoBlockingAnimation;
	return completed;
}

void Scene9050::presentAnimationFrame() {
	const bool continuousSound = _blockingAnimationMode == kResourceI05ClipAnimation ||
		_blockingAnimationMode == kInterClipRevealAnimation ||
		_blockingAnimationMode == kInterClipReverseAnimation ||
		_blockingAnimationMode == kResourceI07FinalAnimation;
	if (continuousSound)
		ensureContinuousSound(kStage9050ClipSoundCue, 100);

	switch (_blockingAnimationMode) {
	case kResourceI05ClipAnimation:
		drawResourceI05ClipFrameDelta(_i05ClipFrameCount, _blockingAnimationFrame);
		break;
	case kInterClipRevealAnimation:
		restoreAndDrawResourceDescriptorFrame(_blockingAnimationChunk,
			kI05InterClipFrameDescriptorCount,
			kStage9050InterClipRevealFrames[_blockingAnimationFrame], true);
		break;
	case kInterClipReverseAnimation: {
		const bool drawFrame = _blockingAnimationFrame + 1 <
			ARRAYSIZE(kStage9050InterClipReverseFrames);
		const byte frame = kStage9050InterClipReverseFrames[_blockingAnimationFrame];
		restoreAndDrawResourceDescriptorFrame(4, kI05InterClipFrameDescriptorCount,
			frame, drawFrame);
		if (_i05EntriesPerSegment >= kI05LayeredRevealEntriesPerSegment) {
			restoreAndDrawResourceDescriptorFrame(5, kI05InterClipFrameDescriptorCount,
				frame, drawFrame);
		}
		break;
	}
	case kResourceI08BlinkAnimation: {
		const byte frame = (_blockingAnimationFrame & 1) == 0 ? 1 : 0;
		restoreAndDrawResourceDescriptorFrame(2, kI08BlinkFrameDescriptorCount,
			frame, true);
		break;
	}
	case kResourceI07FinalAnimation:
		if (_blockingAnimationFrame == 1)
			_effectSound.playSample(kStage9050FinalStartSoundCue, 100);
		else if (_blockingAnimationFrame == ARRAYSIZE(kStage9050ResourceI07FinalFrameMap) - 1)
			_effectSound.playSample(kStage9050FinalEndSoundCue, 100);
		restoreAndDrawResourceDescriptorFrame(2, kI07FinalFrameDescriptorCount,
			kStage9050ResourceI07FinalFrameMap[_blockingAnimationFrame], true);
		break;
	case kNoBlockingAnimation:
		break;
	}

	presentFrame();
}

bool Scene9050::waitForAnimationFrame(uint32 millis, bool allowSkip) {
	const bool continuousSound = _blockingAnimationMode == kResourceI05ClipAnimation ||
		_blockingAnimationMode == kInterClipRevealAnimation ||
		_blockingAnimationMode == kInterClipReverseAnimation ||
		_blockingAnimationMode == kResourceI07FinalAnimation;
	if (!continuousSound)
		return PresentationScene::waitForAnimationFrame(millis, allowSkip);

	uint32 remaining = millis;
	while (remaining != 0 && !animationPlaybackShouldStop()) {
		ensureContinuousSound(kStage9050ClipSoundCue, 100);
		const uint32 slice = MIN<uint32>(remaining, 10);
		if (delay(slice, allowSkip))
			return true;
		remaining -= slice;
	}

	return animationPlaybackShouldStop();
}

bool Scene9050::waitSceneCounterPast(uint threshold) {
	TimedPresentationLoop loop(*this, (threshold + 1) * 1000);
	while (loop.beginFrame()) {
		ensureContinuousSound(kStage9050ClipSoundCue, 100);
		loop.finishFrame();
	}

	consumeStepAdvanceRequest();
	return _skipRequested || Engine::shouldQuit();
}

void Scene9050::stopAudio() {
	_music.stop();
	stopContinuousSound();
	_effectSound.stop();
}

} // End of namespace Hollywood
