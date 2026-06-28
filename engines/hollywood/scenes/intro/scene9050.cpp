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

#include "hollywood/scenes/intro/scene9050.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kI05ArchiveName = "RESOURCE.I05";
const char *const kI06ArchiveName = "RESOURCE.I06";
const char *const kI08ArchiveName = "RESOURCE.I08";
const uint16 kStage9050MusicCuePrelude = 0x000b;
const uint16 kStage9050MusicCueMain = 0x000c;
const byte kNoMusicCue = 0xff;
const byte kStage9050FirstClipSegmentId = 1;
const byte kStage9050FirstClipLastFrameIndex = 0x90;
const byte kStage9050SecondClipSegmentId = 2;
const byte kStage9050SecondClipLastFrameIndex = 0x22;
const byte kStage9050ThirdClipSegmentId = 3;
const byte kStage9050ThirdClipLastFrameIndex = 0x3f;

const byte kStage9050InterClipRevealFrames[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
const byte kStage9050InterClipReverseFrames[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0 };

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
		_vm(vm),
		_music(),
		_random("hollywood_scene9050"),
		_resourceArenaCursor(0),
		_i05ClipFrameAccumulator(0),
		_i05InterClipAccumulator(0),
		_i08BlinkAccumulator(0),
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
		_i06OptionalOverlayChunk5Enabled(false),
		_i06BaseFrameDirty(false),
		_i06PrimarySpriteDirty(false),
		_i06SecondarySpriteDirty(false),
		_i06CompositeForceDirty(false),
		_i06PaletteDirty(false),
		_i06SequenceFinished(false),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_savedFramebuffer.resize(kFrameBufferSize);
	_clipBaseFramebuffer.resize(kFrameBufferSize);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
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

	// Remaining Stage 9050 beats are lower-priority credits; continue with Scene 9120 work.
	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	return result;
}

bool Scene9050::loadResourceChunk(const char *archiveName, uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed destination", archiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool Scene9050::loadResourceArenaChunk(const char *archiveName, uint archiveIndex, uint localChunkIndex) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(archiveName), archiveIndex));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, archiveIndex);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the Stage 9050 arena", archiveName, archiveIndex);
		return false;
	}

	_resourceChunkOffsets[localChunkIndex] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, archiveIndex);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u as local chunk %u: offset=%u size=%u",
		archiveName, archiveIndex, localChunkIndex, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

bool Scene9050::loadResourceI06Assets() {
	if (!_vm->resources()->readChunkTable(Common::Path(kI06ArchiveName), _i06ChunkTable)) {
		warning("Failed to read %s header", kI06ArchiveName);
		return false;
	}

	for (uint i = 0; i < kI06RequiredChunkCount; ++i) {
		if (!_i06ChunkTable.isValidChunk(i)) {
			warning("%s is missing required Stage 9050 chunk %u", kI06ArchiveName, i);
			return false;
		}
	}

	uint32 resourceArenaSize = 0;
	for (uint i = 1; i < kI06RequiredChunkCount; ++i)
		resourceArenaSize += _i06ChunkTable.sizes[i];

	_resourceArena.resize(resourceArenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));

	if (!loadResourceI06Chunk(0, _paletteResource, kPaletteSize))
		return false;

	for (uint i = 1; i < kI06RequiredChunkCount; ++i) {
		if (!loadResourceI06ArenaChunk(i))
			return false;
	}

	return true;
}

bool Scene9050::loadResourceI06Chunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return loadResourceChunk(kI06ArchiveName, index, destination, fixedSize);
}

bool Scene9050::loadResourceI06ArenaChunk(uint index) {
	return loadResourceArenaChunk(kI06ArchiveName, index, index);
}

bool Scene9050::loadResourceI05ClipSegment(byte segmentId) {
	if (segmentId == 0) {
		warning("Invalid %s Stage 9050 segment 0", kI05ArchiveName);
		return false;
	}

	if (!_vm->resources()->readChunkTable(Common::Path(kI05ArchiveName), _i05ChunkTable)) {
		warning("Failed to read %s header", kI05ArchiveName);
		return false;
	}

	const uint baseIndex = ((uint)segmentId - 1) * kI05EntriesPerSegment;
	const uint lastLocalChunkIndex = segmentId < 7 ? 5 : 3;
	if (baseIndex + lastLocalChunkIndex >= kResourceChunkCount) {
		warning("%s Stage 9050 segment %u exceeds the archive chunk table", kI05ArchiveName, segmentId);
		return false;
	}

	for (uint i = 0; i <= lastLocalChunkIndex; ++i) {
		if (!_i05ChunkTable.isValidChunk(baseIndex + i)) {
			warning("%s is missing Stage 9050 segment %u chunk %u", kI05ArchiveName, segmentId, i);
			return false;
		}
	}

	uint32 resourceArenaSize = 0;
	for (uint i = 2; i <= lastLocalChunkIndex; ++i)
		resourceArenaSize += _i05ChunkTable.sizes[baseIndex + i];

	_resourceArena.resize(resourceArenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));

	if (!loadResourceI05Chunk(baseIndex, _clipBaseFramebuffer, kFrameBufferSize))
		return false;
	if (!loadResourceI05Chunk(baseIndex + 1, _paletteResource, kPaletteSize))
		return false;

	for (uint i = 2; i <= lastLocalChunkIndex; ++i) {
		if (!loadResourceI05ArenaChunk(baseIndex + i, i))
			return false;
	}

	return true;
}

bool Scene9050::loadResourceI05Chunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return loadResourceChunk(kI05ArchiveName, index, destination, fixedSize);
}

bool Scene9050::loadResourceI05ArenaChunk(uint archiveIndex, uint localChunkIndex) {
	return loadResourceArenaChunk(kI05ArchiveName, archiveIndex, localChunkIndex);
}

bool Scene9050::loadResourceI08BlinkAssets() {
	if (!_vm->resources()->readChunkTable(Common::Path(kI08ArchiveName), _i08ChunkTable)) {
		warning("Failed to read %s header", kI08ArchiveName);
		return false;
	}

	for (uint i = 0; i < kI08RequiredChunkCount; ++i) {
		if (!_i08ChunkTable.isValidChunk(i)) {
			warning("%s is missing required Stage 9050 chunk %u", kI08ArchiveName, i);
			return false;
		}
	}

	_resourceArena.resize(_i08ChunkTable.sizes[2]);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));

	if (!loadResourceChunk(kI08ArchiveName, 0, _clipBaseFramebuffer, kFrameBufferSize))
		return false;
	if (!loadResourceChunk(kI08ArchiveName, 1, _paletteResource, kPaletteSize))
		return false;
	if (!loadResourceArenaChunk(kI08ArchiveName, 2, 2))
		return false;

	return true;
}

void Scene9050::runResourceI06AnimatedPresentation() {
	initializeResourceI06AnimatedPresentation();

	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		if (pollEvents())
			return;
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
	}

	_i06ScrollAccumulator = 60;
	_i06SecondarySpriteAccumulator = 75;
	_i06PrimarySpriteAccumulator = 75;
	_i06VerticalBobAccumulator = 100;
	_i06PalettePulseAccumulator = 50;
	uint32 lastFrameMillis = g_system->getMillis();
	while (!_i06SequenceFinished && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		advanceResourceI06Timers(elapsed);
		presentResourceI06AnimatedFrame();
		g_system->delayMillis(1);
	}

	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 0x14) {
		if (pollEvents())
			return;
		clearSceneFramebufferBand(sweepOffset, 0x14);
		presentFrame();
	}
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

void Scene9050::copyResourceI06ScrolledBaseFrame() {
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());

	const uint32 chunkOffset = _resourceChunkOffsets[1];
	const uint32 chunkSize = _i06ChunkTable.sizes[1];
	if (_i06BaseImageScrollOffset >= chunkSize || chunkOffset + chunkSize > _resourceArena.size())
		return;

	const uint32 copySize = MIN<uint32>(chunkSize - _i06BaseImageScrollOffset, _sceneFramebuffer.size());
	memcpy(_sceneFramebuffer.data(), _resourceArena.data() + chunkOffset + _i06BaseImageScrollOffset, copySize);
}

void Scene9050::presentResourceI06AnimatedFrame() {
	const bool redrawFrame = _i06BaseFrameDirty || _i06PrimarySpriteDirty ||
		_i06SecondarySpriteDirty || _i06CompositeForceDirty;

	if (redrawFrame) {
		copyResourceI06ScrolledBaseFrame();
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[4], _sceneFramebuffer, _i06RandomBasePhase);
		drawResourceI06AnimatedFrame(2, (byte)(_i06PrimarySpriteFrame + _i06VerticalBobOffset));
		drawResourceI06AnimatedFrame(3, _i06SecondarySpriteFrame);
		if (_i06OptionalOverlayChunk5Enabled)
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);

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

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[chunkIndex], 0,
		kI06AnimatedFrameDescriptorCount, frameIndex, _sceneFramebuffer, _i06RandomBasePhase);
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

void Scene9050::advanceResourceI06Scroll() {
	if (_i06FrameCounter == kI06FrameCounterWrap) {
		_i06FrameCounter = 0;
		_i06SequenceFinished = true;
	} else if (_i06PrimarySpriteSequenceState < 2) {
		_i06FrameCounter++;
	}

	if (_i06FrameCounter == kI06SequenceDoneFrame)
		_i06SequenceFinished = true;

	if (_i06FrameCounter == 0x10) {
		_i06OptionalOverlayChunk5Enabled = true;
		_i06BaseFrameDirty = true;
	}
	if (_i06FrameCounter == 0xa0) {
		_i06OptionalOverlayChunk5Enabled = false;
		_i06BaseFrameDirty = true;
	}
	if (_i06FrameCounter == 0xc0 && _i06PrimarySpriteSequenceState < 2) {
		_i06PrimarySpriteSequenceStep = 0;
		_i06PrimarySpriteSequenceState = 2;
	}

	const int scrollOffset = (int)_i06BaseImageScrollOffset + getStage9050ResourceI06ScrollDelta(_i06FrameCounter);
	_i06BaseImageScrollOffset = (uint16)MAX<int>(0, MIN<int>(scrollOffset, _i06ChunkTable.sizes[1] - 1));
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

void Scene9050::runResourceI05Clip(byte segmentId, byte lastFrameIndex, bool fadeInBeforePlayback) {
	debugC(1, kDebugScene, "Playing Stage 9050 %s segment %u to frame %u", kI05ArchiveName, segmentId, lastFrameIndex);

	if (fadeInBeforePlayback) {
		for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 0x14) {
			if (pollEvents())
				return;
			clearSceneFramebufferBand(sweepOffset, 0x14);
			presentFrame();
		}
	}

	if (_skipRequested || Engine::shouldQuit())
		return;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	memcpy(_savedFramebuffer.data(), _clipBaseFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		if (pollEvents())
			return;
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
	}

	if (waitResourceI05ClipHold())
		return;

	byte frameIndex = 0;
	_i05ClipFrameAccumulator = 50;
	uint32 lastFrameMillis = g_system->getMillis();
	while (frameIndex < lastFrameIndex && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		_i05ClipFrameAccumulator += elapsed;

		if (_i05ClipFrameAccumulator >= 50) {
			_i05ClipFrameAccumulator %= 50;
			drawResourceI05ClipFrameDelta(lastFrameIndex, frameIndex);
			frameIndex++;
			presentFrame();
		}

		g_system->delayMillis(1);
	}
}

void Scene9050::drawResourceI05ClipFrameDelta(byte lastFrameIndex, byte frameIndex) {
	const uint32 frameTableOffset = _resourceChunkOffsets[3];
	const uint32 tableEntryOffset = frameTableOffset + ((uint32)frameIndex * 4);
	if (tableEntryOffset + 4 > _resourceArena.size())
		return;

	const uint32 frameOffset = frameTableOffset + ((uint32)lastFrameIndex * 4) +
		readUint32LE(_resourceArena, tableEntryOffset);
	if (frameOffset + 4 > _resourceArena.size())
		return;

	const uint16 firstRow = readUint16LE(_resourceArena, frameOffset);
	const uint16 lastRow = readUint16LE(_resourceArena, frameOffset + 2);
	uint cursor = frameOffset + 4;

	for (uint row = firstRow; row <= lastRow && row < HollywoodEngine::kSceneBufferHeight; ++row) {
		if (cursor >= _resourceArena.size())
			return;

		byte runCount = _resourceArena[cursor++];
		for (; runCount != 0; --runCount) {
			if (cursor + 3 > _resourceArena.size())
				return;

			const uint x = readUint16LE(_resourceArena, cursor);
			const byte literalLength = _resourceArena[cursor + 2];
			const uint destinationOffset = row * HollywoodEngine::kSceneBufferWidth + x;
			if (destinationOffset >= _sceneFramebuffer.size())
				return;

			if (literalLength == 0) {
				if (cursor + 5 > _resourceArena.size())
					return;

				const byte fillValue = _resourceArena[cursor + 3];
				const uint fillLength = _resourceArena[cursor + 4];
				cursor += 5;
				if (destinationOffset + fillLength <= _sceneFramebuffer.size())
					memset(_sceneFramebuffer.data() + destinationOffset, fillValue, fillLength);
			} else {
				const uint literalOffset = cursor + 3;
				if (literalOffset + literalLength > _resourceArena.size())
					return;

				if (destinationOffset + literalLength <= _sceneFramebuffer.size())
					memcpy(_sceneFramebuffer.data() + destinationOffset,
						_resourceArena.data() + literalOffset, literalLength);
				cursor = literalOffset + literalLength;
			}
		}
	}
}

bool Scene9050::waitResourceI05ClipHold() {
	if (_skipRequested || Engine::shouldQuit())
		return true;

	return waitSceneCounterPast(2);
}

bool Scene9050::playResourceI05ClipSegment(byte segmentId, byte lastFrameIndex, bool fadeInBeforePlayback) {
	if (!loadResourceI05ClipSegment(segmentId))
		return false;

	runResourceI05Clip(segmentId, lastFrameIndex, fadeInBeforePlayback);
	return !_skipRequested && !Engine::shouldQuit();
}

void Scene9050::runStage9050InterClipSpriteReveal() {
	const uint32 paletteOffset = _resourceChunkOffsets[2];
	if (paletteOffset + kPaletteSize <= _resourceArena.size()) {
		memcpy(_paletteCurrent.data(), _resourceArena.data() + paletteOffset, kPaletteSize);
		memcpy(_paletteResource.data(), _paletteCurrent.data(), kPaletteSize);
		presentFrame();
	}

	runResourceI05InterClipRevealPhase(4);
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
	_i05InterClipAccumulator = 60;
	uint frameListIndex = 0;
	uint32 lastFrameMillis = g_system->getMillis();
	while (frameListIndex < ARRAYSIZE(kStage9050InterClipRevealFrames) && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		_i05InterClipAccumulator += elapsed;

		if (_i05InterClipAccumulator >= 60) {
			_i05InterClipAccumulator %= 60;
			restoreAndDrawResourceDescriptorFrame(localChunkIndex, kI05InterClipFrameDescriptorCount,
				kStage9050InterClipRevealFrames[frameListIndex], true);
			presentFrame();
			frameListIndex++;
		}

		g_system->delayMillis(1);
	}
}

void Scene9050::runResourceI05InterClipReversePhase() {
	_i05InterClipAccumulator = 60;
	uint frameListIndex = 0;
	uint32 lastFrameMillis = g_system->getMillis();
	while (frameListIndex < ARRAYSIZE(kStage9050InterClipReverseFrames) && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		_i05InterClipAccumulator += elapsed;

		if (_i05InterClipAccumulator >= 60) {
			_i05InterClipAccumulator %= 60;
			const bool drawFrame = frameListIndex + 1 < ARRAYSIZE(kStage9050InterClipReverseFrames);
			const byte frameIndex = kStage9050InterClipReverseFrames[frameListIndex];
			restoreAndDrawResourceDescriptorFrame(4, kI05InterClipFrameDescriptorCount, frameIndex, drawFrame);
			restoreAndDrawResourceDescriptorFrame(5, kI05InterClipFrameDescriptorCount, frameIndex, drawFrame);
			presentFrame();
			frameListIndex++;
		}

		g_system->delayMillis(1);
	}
}

void Scene9050::restoreAndDrawResourceDescriptorFrame(byte localChunkIndex, byte descriptorCount, byte descriptorIndex,
		bool drawFrame) {
	if (localChunkIndex >= kResourceChunkCount)
		return;

	const uint32 baseOffset = _resourceChunkOffsets[localChunkIndex];
	restoreSpriteBackground(_resourceArena, baseOffset, 0, descriptorCount, descriptorIndex,
		_clipBaseFramebuffer, _sceneFramebuffer);
	if (drawFrame)
		drawStripSpriteFrame(_resourceArena, baseOffset, 0, descriptorCount, descriptorIndex, _sceneFramebuffer);
}

bool Scene9050::runResourceI08BlinkSequence() {
	if (!loadResourceI08BlinkAssets())
		return false;

	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 0x14) {
		if (pollEvents())
			return true;
		clearSceneFramebufferBand(sweepOffset, 0x14);
		presentFrame();
	}

	if (_skipRequested || Engine::shouldQuit())
		return true;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	memcpy(_savedFramebuffer.data(), _clipBaseFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		if (pollEvents())
			return true;
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
	}

	if (waitResourceI08BlinkLoop(6000))
		return true;

	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 0x14) {
		if (pollEvents())
			return true;
		clearSceneFramebufferBand(sweepOffset, 0x14);
		presentFrame();
	}

	return true;
}

bool Scene9050::waitResourceI08BlinkLoop(uint32 millis) {
	byte blinkFrame = 0;
	uint32 elapsedTotal = 0;
	_i08BlinkAccumulator = 50;
	uint32 lastFrameMillis = g_system->getMillis();
	while (elapsedTotal < millis && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		elapsedTotal += elapsed;
		_i08BlinkAccumulator += elapsed;

		if (_i08BlinkAccumulator >= 50) {
			_i08BlinkAccumulator %= 50;
			blinkFrame = blinkFrame == 0 ? 1 : 0;
			restoreAndDrawResourceDescriptorFrame(2, kI08BlinkFrameDescriptorCount, blinkFrame, true);
			presentFrame();
		}

		g_system->delayMillis(1);
	}

	return _skipRequested || Engine::shouldQuit();
}

bool Scene9050::waitSceneCounterPast(uint threshold) {
	const uint32 millis = (threshold + 1) * 1000;
	return delay(millis);
}

void Scene9050::revealSavedFramebufferBand(uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		copyFramebufferRun(_savedFramebuffer, _sceneFramebuffer, sweepOffset + row, leftInset, innerWidth);
		copyFramebufferRun(_savedFramebuffer, _sceneFramebuffer,
			(HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row, leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row;
			copyFramebufferRun(_savedFramebuffer, _sceneFramebuffer, y, leftInset, bandWidth);
			copyFramebufferRun(_savedFramebuffer, _sceneFramebuffer, y, middleRightX, bandWidth);
		}
	}
}

void Scene9050::clearSceneFramebufferBand(uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		clearFramebufferRun(_sceneFramebuffer, sweepOffset + row, leftInset, innerWidth);
		clearFramebufferRun(_sceneFramebuffer,
			(HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row, leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row;
			clearFramebufferRun(_sceneFramebuffer, y, leftInset, bandWidth);
			clearFramebufferRun(_sceneFramebuffer, y, middleRightX, bandWidth);
		}
	}
}

void Scene9050::presentFrame() {
	presentIndexedFrame(_sceneFramebuffer, _paletteCurrent, _screen);
}

bool Scene9050::pollEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			stopAudio();
			return true;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
					event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_SPACE) {
				_skipRequested = true;
				stopAudio();
				return true;
			}
			break;
		default:
			break;
		}
	}

	return false;
}

bool Scene9050::delay(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;
	}

	return _skipRequested || Engine::shouldQuit();
}

void Scene9050::stopAudio() {
	_music.stop();
}

} // End of namespace Hollywood
