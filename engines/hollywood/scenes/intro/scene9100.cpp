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

#include "hollywood/scenes/intro/scene9100.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/paletteman.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

static const char *const kI10ArchiveName = "RESOURCE.I10";
static const char *const kStage003ArchiveName = "RESOURCE.003";
static const uint16 kScene9100MusicCueId = 0x000f;
static const uint kStage003DecodeKeySize = 0x141;
static const uint kStage003StageOffsetTableSize = 0xff4;
static const uint kStage910Index = 910;

static const byte kI10ForegroundFrameRemap[] = {
	0, 31, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13,
	32, 33, 34, 35, 14, 15, 16, 16, 17, 18, 19, 29, 20, 21, 22, 23,
	23, 24, 25, 26, 30, 22, 21, 20, 16, 0, 0, 0, 0, 1, 2, 3,
	0, 4, 5, 5, 6, 7, 8, 5, 4, 0, 0, 0, 0, 1, 2, 3,
	2, 1, 0, 0
};

Scene9100::Scene9100(HollywoodEngine *vm) :
		_vm(vm),
		_music(),
		_speech(),
		_resourceArenaCursor(0),
		_lastClockFrameMillis(0),
		_lastTalkingFrameMillis(0),
		_foregroundActorFrame(0),
		_clockFrame(32),
		_talkingFrame(0),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteDefault.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_frameDecodeBuffer.resize(kFrameDecodeBufferSize);
	_sceneFramebuffer.resize(kFrameDecodeBufferSize);
	_savedFramebuffer.resize(kFrameDecodeBufferSize);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
	_stage003Descriptors.resize(kStage003DescriptorTableSize);
}

bool Scene9100::play() {
	if (!load())
		return false;

	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _frameDecodeBuffer.size());
	drawInitialForegroundFrame();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _sceneFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());

	_paletteCurrent[0x2f7] = 0x3f;
	_paletteCurrent[0x2f8] = 0x3f;
	_paletteCurrent[0x2f9] = 0x3f;
	presentFrame();

	_music.playMusicCue(kScene9100MusicCueId, 30);

	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
		if (delay(50))
			break;
	}

	if (!_skipRequested && !Engine::shouldQuit()) {
		expandFillRunsToSavedFramebuffer();
		restoreResourceBlocksToFrameAndScene(_resourceChunkOffsets[16]);
		presentFrame();
		runOpeningPrelude();
	}

	if (!_skipRequested && !Engine::shouldQuit())
		runCinematicSequence();

	if (!_skipRequested && !Engine::shouldQuit()) {
		runEndingWipe();
	}

	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	return true;
}

bool Scene9100::load() {
	if (!_vm->resources()->readChunkTable(Common::Path(kI10ArchiveName), _i10ChunkTable)) {
		warning("Failed to read %s header", kI10ArchiveName);
		return false;
	}

	for (uint i = 0; i <= 16; ++i) {
		if (!_i10ChunkTable.isValidChunk(i)) {
			warning("%s is missing scene 9100 chunk %u", kI10ArchiveName, i);
			return false;
		}
	}
	if (!_i10ChunkTable.isValidChunk(20) || !_i10ChunkTable.isValidChunk(21) || !_i10ChunkTable.isValidChunk(23)) {
		warning("%s is missing required scene 9100 scratch chunks", kI10ArchiveName);
		return false;
	}

	if (!loadChunk(0, _frameDecodeBuffer, kFrameDecodeBufferSize) ||
			!loadChunk(1, _paletteDefault, kPaletteSize) ||
			!loadVariableChunk(2, _sceneFillRuns) ||
			!loadStage003Descriptors())
		return false;
	memcpy(_paletteCurrent.data(), _paletteDefault.data(), _paletteCurrent.size());

	uint32 resourceArenaSize = 0;
	for (uint i = 5; i <= 16; ++i)
		resourceArenaSize += _i10ChunkTable.sizes[i];

	_resourceArena.resize(resourceArenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());

	_resourceArenaCursor = 0;
	for (uint i = 5; i <= 16; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	const uint32 scratchSize = MAX<uint32>(
		kScratchPrimaryPayloadBase + _i10ChunkTable.sizes[23],
		MAX<uint32>(_i10ChunkTable.sizes[20], kScratchChunk21Base + _i10ChunkTable.sizes[21]));
	_resourceScratchArena.resize(scratchSize);
	memset(_resourceScratchArena.data(), 0, _resourceScratchArena.size());

	if (!loadScratchChunk(20, 0) ||
			!loadScratchChunk(21, kScratchChunk21Base) ||
			!loadScratchChunk(23, kScratchPrimaryPayloadBase))
		return false;

	return true;
}

bool Scene9100::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI10ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI10ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed destination", kI10ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI10ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u", kI10ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene9100::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI10ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI10ArchiveName, index);
		return false;
	}

	destination.resize(stream->size());
	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI10ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s variable chunk %u: size=%u", kI10ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene9100::loadArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI10ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI10ArchiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the scene 9100 resource arena", kI10ArchiveName, index);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI10ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		kI10ArchiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

bool Scene9100::loadScratchChunk(uint index, uint32 destinationOffset) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI10ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s scratch chunk %u", kI10ArchiveName, index);
		return false;
	}

	if (destinationOffset + stream->size() > _resourceScratchArena.size()) {
		warning("%s scratch chunk %u does not fit the scene 9100 scratch arena", kI10ArchiveName, index);
		return false;
	}

	if (stream->read(_resourceScratchArena.data() + destinationOffset, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s scratch chunk %u", kI10ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s scratch chunk %u: offset=%u size=%u",
		kI10ArchiveName, index, destinationOffset, (uint)stream->size());
	return true;
}

bool Scene9100::loadStage003Descriptors() {
	Common::File file;
	if (!file.open(Common::Path(kStage003ArchiveName))) {
		warning("Failed to open %s", kStage003ArchiveName);
		return false;
	}

	const uint32 stageOffsetEntry = kStage003DecodeKeySize + (kStage910Index * 4);
	if (stageOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s has no stage 910 offset entry", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffsetEntry);
	const uint32 stageOffset = file.readUint32LE();
	if (stageOffset + kStage003DescriptorTableSize > (uint32)file.size()) {
		warning("%s stage 910 descriptor table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffset);
	if (file.read(_stage003Descriptors.data(), _stage003Descriptors.size()) != _stage003Descriptors.size()) {
		warning("Failed to read %s stage 910 descriptor table", kStage003ArchiveName);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s stage 910 descriptors at offset=%u", kStage003ArchiveName, stageOffset);
	return true;
}

void Scene9100::runOpeningPrelude() {
	for (byte frame = 11; frame <= 14 && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		drawForegroundActorFrame(frame);
		presentFrame();
		if (delayFrame(100, kTalkingOverlayNone, 0, false))
			return;
	}

	runConversationStep(0, 0, kTalkingOverlayNone, 0, true);

	for (byte frame = 20; frame <= 22 && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		drawForegroundActorFrame(frame);
		presentFrame();
		if (delayFrame(100, kTalkingOverlayNone, 0, false))
			return;
	}

	for (uint pulse = 0; pulse < 12 && !_skipRequested && !Engine::shouldQuit(); ++pulse) {
		if (delayFrame(100, kTalkingOverlayNone, 0, false))
			return;
	}
}

void Scene9100::runCinematicSequence() {
	static const CinematicStep kSteps[] = {
		{ 2, 1, 3, kTalkingOverlayBase320000, 0, true, false },
		{ 1, 1, 0, kTalkingOverlayBase0, 1, false, false },
		{ 2, 1, 1, kTalkingOverlayBase320000, 0, false, false },
		{ 4, 1, 2, kTalkingOverlayNone, 0, false, false },
		{ 2, 1, 3, kTalkingOverlayBase320000, 0, true, false },
		{ 1, 1, 4, kTalkingOverlayBase0, 1, false, false },
		{ 2, 1, 5, kTalkingOverlayBase320000, 0, false, false },
		{ 3, 1, 16, kTalkingOverlayBase640000, 0, true, false },
		{ 2, 1, 8, kTalkingOverlayBase320000, 1, false, false },
		{ 3, 1, 17, kTalkingOverlayBase640000, 1, false, false },
		{ 0, 1, 9, kTalkingOverlayBase640000, 1, false, false },
		{ 1, 1, 10, kTalkingOverlayBase0, 0, false, false },
		{ 2, 1, 11, kTalkingOverlayBase320000, 0, false, false },
		{ 3, 1, 12, kTalkingOverlayBase640000, 1, false, false },
		{ 2, 1, 13, kTalkingOverlayBase320000, 1, false, false },
		{ 4, 1, 14, kTalkingOverlayNone, 0, false, false },
		{ 3, 1, 15, kTalkingOverlayBase640000, 0, true, false }
	};

	for (uint i = 0; i < ARRAYSIZE(kSteps) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		applyBackgroundMode(kSteps[i]);
		runConversationStep(kSteps[i].textBankIndex, kSteps[i].descriptorIndex,
			kSteps[i].talkingOverlayBase, kSteps[i].talkingOverlayVariant, kSteps[i].animateForegroundActor);
	}
}

void Scene9100::runEndingWipe() {
	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 20) {
		clearSceneFramebufferBand(sweepOffset, 20);
		presentFrame();
		if (delay(50))
			return;
	}
}

void Scene9100::runConversationStep(uint16 textBankIndex, byte descriptorIndex, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor) {
	_talkingFrame = 0;
	_lastTalkingFrameMillis = g_system->getMillis();
	const uint16 sampleId = getStage003VoiceSample(textBankIndex, descriptorIndex);
	const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
	const uint32 fallbackMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
	waitForSpeechOrDelay(fallbackMillis, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor);
}

void Scene9100::waitForSpeechOrDelay(uint32 fallbackMillis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor) {
	uint32 elapsed = 0;
	while (!_skipRequested && !Engine::shouldQuit()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsed >= fallbackMillis)
			break;

		if (delayFrame(50, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor))
			return;
		elapsed += 50;
	}
}

void Scene9100::drawInitialForegroundFrame() {
	drawForegroundActorFrame(0);
}

void Scene9100::drawForegroundActorFrame(byte frameIndex) {
	if (frameIndex >= ARRAYSIZE(kI10ForegroundFrameRemap))
		return;

	_foregroundActorFrame = frameIndex;
	const uint16 descriptorIndex = kI10ForegroundFrameRemap[frameIndex];
	restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[5], 0, kI10ForegroundDescriptorCount, descriptorIndex);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[5], 0, kI10ForegroundDescriptorCount, descriptorIndex);
}

void Scene9100::drawClockFrame(byte frameIndex) {
	_clockFrame = frameIndex % kI10ClockDescriptorCount;
	restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[9], 0, kI10ClockDescriptorCount, _clockFrame);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[9], 0, kI10ClockDescriptorCount, _clockFrame);
}

void Scene9100::drawTalkingOverlay(TalkingOverlayBase talkingOverlayBase, byte frameIndex, byte talkingOverlayVariant) {
	if (talkingOverlayBase == kTalkingOverlayNone)
		return;

	_talkingFrame = frameIndex % 5;
	const uint16 descriptorIndex = _talkingFrame + (5 * (talkingOverlayVariant != 0 ? 1 : 0));
	const uint32 baseOffset = (uint32)talkingOverlayBase;
	restoreSpriteBackground(_resourceScratchArena, baseOffset, 0, kI10TalkingOverlayDescriptorCount, descriptorIndex);
	drawStripSpriteFrame(_resourceScratchArena, baseOffset, 0, kI10TalkingOverlayDescriptorCount, descriptorIndex);
}

void Scene9100::drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset, uint16 descriptorCount, uint16 descriptorIndex) {
	if (descriptorIndex >= descriptorCount)
		return;

	const uint entryOffset = baseOffset + descriptorTableOffset + (kFrameDescriptorSize * descriptorIndex);
	if (entryOffset + kFrameDescriptorSize > resource.size())
		return;

	const uint16 spanCount = readUint16(resource, entryOffset + 12);
	uint cursor = baseOffset + descriptorTableOffset + (kFrameDescriptorSize * descriptorCount) + readUint32(resource, entryOffset);
	if (cursor > resource.size())
		return;

	for (uint spanIndex = 0; spanIndex < spanCount; ++spanIndex) {
		if (cursor + 5 > resource.size())
			return;

		const uint32 destination = readUint32(resource, cursor);
		const uint dataLength = resource[cursor + 4];
		cursor += 5;

		const uint x = destination & 0xffff;
		const uint y = (destination >> 16) & 0xffff;
		const uint destinationOffset = x + y * HollywoodEngine::kSceneBufferWidth;
		if (cursor + dataLength > resource.size() ||
				destinationOffset + dataLength > _sceneFramebuffer.size())
			return;

		memcpy(_sceneFramebuffer.data() + destinationOffset, resource.data() + cursor, dataLength);
		cursor += dataLength;
	}
}

void Scene9100::restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset, uint16 descriptorCount, uint16 descriptorIndex) {
	if (descriptorIndex >= descriptorCount)
		return;

	const uint entryOffset = baseOffset + descriptorTableOffset + (kFrameDescriptorSize * descriptorIndex);
	if (entryOffset + kFrameDescriptorSize > resource.size())
		return;

	const uint32 packedWidth = readUint32(resource, entryOffset + 4);
	const uint32 packedRows = readUint32(resource, entryOffset + 8);
	const uint copyWidth = (packedWidth >> 16) & 0xffff;
	const uint x = packedWidth & 0xffff;
	const uint firstRow = packedRows & 0xffff;
	const uint lastRow = (packedRows >> 16) & 0xffff;
	if (copyWidth == 0 || firstRow > lastRow)
		return;

	for (uint row = firstRow; row <= lastRow; ++row) {
		const uint destinationOffset = x + row * HollywoodEngine::kSceneBufferWidth;
		if (destinationOffset + copyWidth > _frameDecodeBuffer.size() ||
				destinationOffset + copyWidth > _sceneFramebuffer.size())
			return;

		memcpy(_sceneFramebuffer.data() + destinationOffset, _frameDecodeBuffer.data() + destinationOffset, copyWidth);
	}
}

void Scene9100::drawResourceBlockListToBuffer(uint32 baseOffset, Common::Array<byte> &destination) {
	if (baseOffset + 2 > _resourceArena.size())
		return;

	const uint16 blockCount = readUint16(_resourceArena, baseOffset);
	uint cursor = baseOffset + 2;
	for (uint blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
		if (cursor + 6 > _resourceArena.size())
			return;

		const uint32 packedDestination = readUint32(_resourceArena, cursor);
		const uint16 size = readUint16(_resourceArena, cursor + 4);
		cursor += 6;

		const uint x = packedDestination & 0xffff;
		const uint y = (packedDestination >> 16) & 0xffff;
		const uint targetOffset = x + y * HollywoodEngine::kSceneBufferWidth;
		if (cursor + size > _resourceArena.size() || targetOffset + size > destination.size())
			return;

		memcpy(destination.data() + targetOffset, _resourceArena.data() + cursor, size);
		cursor += size;
	}
}

void Scene9100::restoreResourceBlocksToFrameAndScene(uint32 baseOffset) {
	drawResourceBlockListToBuffer(baseOffset, _frameDecodeBuffer);
	drawResourceBlockListToBuffer(baseOffset, _sceneFramebuffer);
}

void Scene9100::expandFillRunsToSavedFramebuffer() {
	uint destinationOffset = 0;
	uint sourceOffset = 0;
	while (destinationOffset < _savedFramebuffer.size() && sourceOffset + 3 <= _sceneFillRuns.size()) {
		const byte fill = _sceneFillRuns[sourceOffset];
		const uint16 runLength = readUint16(_sceneFillRuns, sourceOffset + 1);
		sourceOffset += 3;
		if (runLength == 0)
			break;

		const uint clampedRunLength = MIN<uint>(runLength, _savedFramebuffer.size() - destinationOffset);
		memset(_savedFramebuffer.data() + destinationOffset, fill, clampedRunLength);
		destinationOffset += clampedRunLength;
	}
}

void Scene9100::applyBackgroundMode(const CinematicStep &step) {
	if (step.copyFrameToSavedBefore)
		memcpy(_savedFramebuffer.data(), _frameDecodeBuffer.data(), _savedFramebuffer.size());

	switch (step.backgroundMode) {
	case 1:
		drawResourceBlockListToBuffer(getSegmentOffset(8), _frameDecodeBuffer);
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		copyPaletteSegment(5);
		break;
	case 2:
		drawResourceBlockListToBuffer(getSegmentOffset(9), _frameDecodeBuffer);
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		copyPaletteSegment(6);
		break;
	case 3:
		drawResourceBlockListToBuffer(getSegmentOffset(10), _frameDecodeBuffer);
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		copyPaletteSegment(7);
		break;
	case 4:
		memcpy(_frameDecodeBuffer.data(), _savedFramebuffer.data(), _frameDecodeBuffer.size());
		expandFillRunsToSavedFramebuffer();
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		drawForegroundActorFrame(_foregroundActorFrame);
		copyDefaultPalette();
		break;
	default:
		break;
	}

	if (step.talkingOverlayBase != kTalkingOverlayNone && step.talkingOverlayVariant != 0)
		drawTalkingOverlay(step.talkingOverlayBase, 0, step.talkingOverlayVariant);

	if (step.backgroundMode != 0)
		presentFrame();
}

void Scene9100::copyPaletteSegment(byte segmentIndex) {
	const uint32 sourceOffset = getSegmentOffset(segmentIndex);
	if (sourceOffset + kPaletteSize > _resourceArena.size())
		return;

	memcpy(_paletteCurrent.data(), _resourceArena.data() + sourceOffset, kPaletteSize);
}

void Scene9100::copyDefaultPalette() {
	memcpy(_paletteCurrent.data(), _paletteDefault.data(), _paletteCurrent.size());
}

void Scene9100::revealSavedFramebufferBand(uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		copySavedFramebufferRun(sweepOffset + row, leftInset, innerWidth);
		copySavedFramebufferRun((HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row, leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleLeftX = leftInset;
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row;
			copySavedFramebufferRun(y, middleLeftX, bandWidth);
			copySavedFramebufferRun(y, middleRightX, bandWidth);
		}
	}
}

void Scene9100::clearSceneFramebufferBand(uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		clearSceneFramebufferRun(sweepOffset + row, leftInset, innerWidth);
		clearSceneFramebufferRun((HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row, leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleLeftX = leftInset;
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row;
			clearSceneFramebufferRun(y, middleLeftX, bandWidth);
			clearSceneFramebufferRun(y, middleRightX, bandWidth);
		}
	}
}

void Scene9100::copySavedFramebufferRun(int y, int x, int width) {
	if (width <= 0 || y < 0 || x < 0)
		return;

	const uint offset = x + y * HollywoodEngine::kSceneBufferWidth;
	if (offset + width > _sceneFramebuffer.size() || offset + width > _savedFramebuffer.size())
		return;

	memcpy(_sceneFramebuffer.data() + offset, _savedFramebuffer.data() + offset, width);
}

void Scene9100::clearSceneFramebufferRun(int y, int x, int width) {
	if (width <= 0 || y < 0 || x < 0)
		return;

	const uint offset = x + y * HollywoodEngine::kSceneBufferWidth;
	if (offset + width > _sceneFramebuffer.size())
		return;

	memset(_sceneFramebuffer.data() + offset, 0, width);
}

void Scene9100::presentFrame() {
	byte palette[0x300];
	for (uint i = 0; i < ARRAYSIZE(palette); ++i)
		palette[i] = MIN<byte>(255, _paletteCurrent[i] * 4);

	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint sourceOffset = y * HollywoodEngine::kSceneBufferWidth;
		memcpy(_screen.data() + y * HollywoodEngine::kScreenWidth,
			_sceneFramebuffer.data() + sourceOffset,
			HollywoodEngine::kScreenWidth);
	}

	g_system->copyRectToScreen(_screen.data(), HollywoodEngine::kScreenWidth, 0, 0,
		HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight);
	g_system->updateScreen();
}

bool Scene9100::pollEvents() {
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

bool Scene9100::delay(uint32 millis) {
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

bool Scene9100::delayFrame(uint32 millis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor) {
	uint32 remaining = millis;
	while (remaining != 0 && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;

		const uint32 now = g_system->getMillis();
		bool dirty = false;
		if (now - _lastClockFrameMillis >= 1000) {
			_lastClockFrameMillis = now;
			drawClockFrame((byte)((_clockFrame + 1) % kI10ClockDescriptorCount));
			dirty = true;
		}
		if (talkingOverlayBase != kTalkingOverlayNone && now - _lastTalkingFrameMillis >= 125) {
			_lastTalkingFrameMillis = now;
			drawTalkingOverlay(talkingOverlayBase, (byte)((_talkingFrame + 1) % 5), talkingOverlayVariant);
			dirty = true;
		}
		if (animateForegroundActor && talkingOverlayBase == kTalkingOverlayNone && now - _lastTalkingFrameMillis >= 125) {
			_lastTalkingFrameMillis = now;
			_talkingFrame = (_talkingFrame + 1) % 4;
			drawForegroundActorFrame((byte)(15 + _talkingFrame));
			dirty = true;
		}
		if (dirty)
			presentFrame();
	}

	return _skipRequested || Engine::shouldQuit();
}

void Scene9100::stopAudio() {
	_music.stop();
	_speech.stop();
}

uint32 Scene9100::getSegmentOffset(byte segmentIndex) const {
	const uint chunkIndex = 5 + segmentIndex;
	if (chunkIndex >= ARRAYSIZE(_resourceChunkOffsets))
		return 0;

	return _resourceChunkOffsets[chunkIndex];
}

uint16 Scene9100::getStage003VoiceSample(uint16 textBankIndex, byte descriptorIndex) const {
	const uint recordOffset = (textBankIndex * 500) + (descriptorIndex * 5);
	if (recordOffset + 5 > _stage003Descriptors.size())
		return 0;

	return readUint16(_stage003Descriptors, recordOffset + 3);
}

uint16 Scene9100::readUint16(const Common::Array<byte> &source, uint offset) const {
	if (offset + 2 > source.size())
		return 0;

	return source[offset] | (source[offset + 1] << 8);
}

uint32 Scene9100::readUint32(const Common::Array<byte> &source, uint offset) const {
	if (offset + 4 > source.size())
		return 0;

	return source[offset] |
		(source[offset + 1] << 8) |
		(source[offset + 2] << 16) |
		(source[offset + 3] << 24);
}

} // End of namespace Hollywood
