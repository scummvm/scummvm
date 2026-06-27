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

const char *const kI06ArchiveName = "RESOURCE.I06";
const uint16 kStage9050MusicCuePrelude = 0x000b;
const uint16 kStage9050MusicCueMain = 0x000c;
const byte kNoMusicCue = 0xff;

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
	_paletteResource.resize(kHollywoodPaletteSize);
	_paletteCurrent.resize(kHollywoodPaletteSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_savedFramebuffer.resize(kFrameBufferSize);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
}

bool Scene9050::play() {
	if (!loadResourceI06Assets())
		return false;

	runResourceI06AnimatedPresentation();

	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
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

	if (!loadResourceI06Chunk(0, _paletteResource, kHollywoodPaletteSize))
		return false;

	for (uint i = 1; i < kI06RequiredChunkCount; ++i) {
		if (!loadResourceI06ArenaChunk(i))
			return false;
	}

	return true;
}

bool Scene9050::loadResourceI06Chunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI06ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI06ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed destination", kI06ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI06ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u", kI06ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene9050::loadResourceI06ArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI06ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI06ArchiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the Stage 9050 arena", kI06ArchiveName, index);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI06ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		kI06ArchiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
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
