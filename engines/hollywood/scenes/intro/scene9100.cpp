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
static const char *const kResource000ArchiveName = "RESOURCE.000";
static const uint16 kScene9100MusicCueId = 0x000f;
static const uint kStage003DecodeKeySize = 0x141;
static const uint kStage003StageOffsetTableSize = 0xff4;
static const uint kStage910Index = 910;
static const uint kResource000OffsetCount = 100;
static const uint kActorBankChunkCount = 14;
static const uint kActorFacingCount = 6;
static const uint kActorCelsPerFacing = 13;
static const uint kActorDescriptorCount = kActorFacingCount * kActorCelsPerFacing;
static const uint kActorSpriteDescriptorSize = 28;
static const uint kActorFacingRunStride = 160000;
static const uint kActorPaletteByteCount = 0x90;
static const uint kActorBankB4OffsetIndex = 0;
static const uint kActorOwner0PaletteOffsetIndex = 0x33;
static const uint kActorBank00OffsetIndex = 0x34;
static const uint kActorOwner1PaletteOffsetIndex = 0x42;
static const uint kActorEntryFrameDelayMillis = 90;
static const uint32 kRonEntryPathDurationMillis = 4200;
static const uint32 kSueEntryPathDurationMillis = 3600;

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
		_random("hollywood_scene9100"),
		_resourceArenaCursor(0),
		_lastClockFrameMillis(0),
		_lastTalkingFrameMillis(0),
		_foregroundActorFrame(0),
		_foregroundTalkBaseFrame(15),
		_clockFrame(32),
		_talkingFrame(0),
		_lastTalkingFrameVariant(0xff),
		_deskPrimaryActorFrame(0),
		_deskSecondaryActorFrame(0),
		_deskPrimaryActorVisible(false),
		_deskSecondaryActorVisible(false),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteDefault.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_frameDecodeBuffer.resize(kFrameDecodeBufferSize);
	_sceneFramebuffer.resize(kFrameDecodeBufferSize);
	_savedFramebuffer.resize(kFrameDecodeBufferSize);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
	_stage003Descriptors.resize(kStage003DescriptorTableSize);
	_secondaryScratchBuffer.resize(kSecondaryScratchBufferSize);
	_actorPaletteOwner0.resize(kActorPaletteByteCount);
	_actorPaletteOwner1.resize(kActorPaletteByteCount);
}

bool Scene9100::play() {
	if (!load())
		return false;

	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _frameDecodeBuffer.size());
	runEntryActorAnimations();
	drawInitialForegroundFrame();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _sceneFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());

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
	if (!_i10ChunkTable.isValidChunk(18) || !_i10ChunkTable.isValidChunk(19) ||
			!_i10ChunkTable.isValidChunk(20) || !_i10ChunkTable.isValidChunk(21) ||
			!_i10ChunkTable.isValidChunk(22)) {
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
		kScratchPrimaryPayloadBase + _i10ChunkTable.sizes[22],
		MAX<uint32>(_i10ChunkTable.sizes[20], kScratchChunk21Base + _i10ChunkTable.sizes[21]));
	_resourceScratchArena.resize(scratchSize);
	memset(_resourceScratchArena.data(), 0, _resourceScratchArena.size());
	memset(_secondaryScratchBuffer.data(), 0, _secondaryScratchBuffer.size());

	if (!loadScratchChunk(20, 0) ||
			!loadScratchChunk(21, kScratchChunk21Base) ||
			!loadScratchChunk(22, kScratchPrimaryPayloadBase) ||
			!loadScratchChunkTo(18, _secondaryScratchBuffer, kDeskPrimaryStaticBase) ||
			!loadScratchChunkTo(19, _secondaryScratchBuffer, kDeskSecondaryStaticBase))
		return false;

	return loadActorResources();
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
	return loadScratchChunkTo(index, _resourceScratchArena, destinationOffset);
}

bool Scene9100::loadScratchChunkTo(uint index, Common::Array<byte> &destination, uint32 destinationOffset) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI10ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s scratch chunk %u", kI10ArchiveName, index);
		return false;
	}

	if (destinationOffset + stream->size() > destination.size()) {
		warning("%s scratch chunk %u does not fit the scene 9100 scratch arena", kI10ArchiveName, index);
		return false;
	}

	if (stream->read(destination.data() + destinationOffset, stream->size()) != (uint32)stream->size()) {
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

bool Scene9100::loadActorResources() {
	Common::File file;
	if (!file.open(Common::Path(kResource000ArchiveName))) {
		warning("Failed to open %s", kResource000ArchiveName);
		return false;
	}

	if (file.size() < 1 + (kResource000OffsetCount * 8)) {
		warning("%s header is too small", kResource000ArchiveName);
		return false;
	}

	file.readByte();

	uint32 offsets[kResource000OffsetCount];
	uint32 sizes[kResource000OffsetCount];
	for (uint i = 0; i < kResource000OffsetCount; ++i)
		offsets[i] = file.readUint32LE();
	for (uint i = 0; i < kResource000OffsetCount; ++i)
		sizes[i] = file.readUint32LE();

	if (!loadActorBank(file, offsets, sizes, kActorBankB4OffsetIndex, _actorBankB4) ||
			!loadActorPalette(file, offsets, kActorOwner0PaletteOffsetIndex, _actorPaletteOwner0) ||
			!loadActorBank(file, offsets, sizes, kActorBank00OffsetIndex, _actorBank00) ||
			!loadActorPalette(file, offsets, kActorOwner1PaletteOffsetIndex, _actorPaletteOwner1))
		return false;

	return true;
}

bool Scene9100::loadActorBank(Common::File &file, const uint32 *offsets, const uint32 *sizes, uint offsetTableIndex, ActorBank &bank) {
	if (offsetTableIndex + kActorBankChunkCount > kResource000OffsetCount) {
		warning("%s actor bank offset index %u is out of range", kResource000ArchiveName, offsetTableIndex);
		return false;
	}

	bank.runStreams.resize(kActorFacingCount * kActorFacingRunStride);
	memset(bank.runStreams.data(), 0, bank.runStreams.size());
	bank.descriptors.resize(kActorDescriptorCount);

	if (!file.seek(offsets[offsetTableIndex])) {
		warning("Failed to seek %s actor bank at offset %u", kResource000ArchiveName, offsets[offsetTableIndex]);
		return false;
	}

	for (uint facing = 0; facing < kActorFacingCount; ++facing) {
		const uint32 chunkSize = sizes[offsetTableIndex + facing];
		if (chunkSize > kActorFacingRunStride) {
			warning("%s actor bank %u facing %u is too large", kResource000ArchiveName, offsetTableIndex, facing);
			return false;
		}

		if (file.read(bank.runStreams.data() + (facing * kActorFacingRunStride), chunkSize) != chunkSize) {
			warning("Failed to read %s actor bank %u facing %u", kResource000ArchiveName, offsetTableIndex, facing);
			return false;
		}
	}

	const uint32 descriptorByteCount = sizes[offsetTableIndex + kActorFacingCount];
	if (descriptorByteCount < kActorDescriptorCount * kActorSpriteDescriptorSize) {
		warning("%s actor bank %u descriptor block is too small", kResource000ArchiveName, offsetTableIndex);
		return false;
	}

	Common::Array<byte> descriptorData;
	descriptorData.resize(descriptorByteCount);
	if (file.read(descriptorData.data(), descriptorByteCount) != descriptorByteCount) {
		warning("Failed to read %s actor bank %u descriptors", kResource000ArchiveName, offsetTableIndex);
		return false;
	}

	for (uint i = 0; i < kActorDescriptorCount; ++i) {
		const uint offset = i * kActorSpriteDescriptorSize;
		ActorSpriteDescriptor &descriptor = bank.descriptors[i];
		descriptor.runStreamOffset = readUint32(descriptorData, offset);
		descriptor.opaqueRunCount = readUint32(descriptorData, offset + 4);
		descriptor.paletteRunCount = readUint32(descriptorData, offset + 8);
		descriptor.anchorX = readSint16(descriptorData, offset + 12);
		descriptor.anchorY = readSint16(descriptorData, offset + 16);
		descriptor.width = readUint16(descriptorData, offset + 20);
		descriptor.height = readUint16(descriptorData, offset + 24);
	}

	debugC(1, kDebugResources, "Loaded %s actor bank at offset index %u", kResource000ArchiveName, offsetTableIndex);
	return true;
}

bool Scene9100::loadActorPalette(Common::File &file, const uint32 *offsets, uint offsetTableIndex, Common::Array<byte> &palette) {
	if (offsetTableIndex >= kResource000OffsetCount) {
		warning("%s actor palette offset index %u is out of range", kResource000ArchiveName, offsetTableIndex);
		return false;
	}

	if (!file.seek(offsets[offsetTableIndex])) {
		warning("Failed to seek %s actor palette at offset %u", kResource000ArchiveName, offsets[offsetTableIndex]);
		return false;
	}

	palette.resize(kActorPaletteByteCount);
	if (file.read(palette.data(), kActorPaletteByteCount) != kActorPaletteByteCount) {
		warning("Failed to read %s actor palette at offset index %u", kResource000ArchiveName, offsetTableIndex);
		return false;
	}

	return true;
}

void Scene9100::applyActorHighlightColor(byte highlightRed, byte highlightGreen, byte highlightBlue) {
	_paletteCurrent[0x2f7] = highlightRed;
	_paletteCurrent[0x2f8] = highlightGreen;
	_paletteCurrent[0x2f9] = highlightBlue;
}

void Scene9100::runEntryActorAnimations() {
	Common::Array<byte> baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	applyActorHighlightColor(0x3f, 0x3f, 0x3f);
	playEntryActorAnimation(_actorBankB4, 0x307, 0x1d4, baseFramebuffer);

	memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
}

void Scene9100::showSueEntryActor() {
	Common::Array<byte> baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	applyActorHighlightColor(0x3f, 0x28, 0x32);
	playEntryActorAnimation(_actorBank00, 0x130, 0x172, baseFramebuffer);

	memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
}

void Scene9100::playEntryActorAnimation(const ActorBank &bank, int worldX, int worldY, Common::Array<byte> &baseFramebuffer) {
	static const byte kFacingTurnToCamera = 5;
	static const byte kTurnCel = 2;
	static const byte kFinalCel = 0;
	static const byte kFrames[][2] = {
		{ kFacingTurnToCamera, kTurnCel },
		{ kFacingTurnToCamera, kFinalCel }
	};

	if (bank.descriptors.size() < kActorDescriptorCount || bank.runStreams.size() < kActorFacingCount * kActorFacingRunStride)
		return;

	for (uint i = 0; i < ARRAYSIZE(kFrames) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawActorSpriteFrame(bank, kFrames[i][0], kFrames[i][1], worldX, worldY);
		presentFrame();
		if (delay(kActorEntryFrameDelayMillis))
			return;
	}

	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());
}

void Scene9100::runRonEntryConversation() {
	const PopupDescriptor popup = getStage003PopupDescriptor(0, 2);
	const uint segmentCount = MAX<uint>(1, popup.continuationCount);

	Common::Array<byte> baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	uint32 pathElapsed = 0;
	uint32 lastPathFrameMillis = g_system->getMillis();
	byte foregroundFrame = _foregroundTalkBaseFrame;
	bool pathPresented = false;

	for (uint segmentIndex = 0; segmentIndex < segmentCount && !_skipRequested && !Engine::shouldQuit(); ++segmentIndex) {
		const uint16 sampleId = popup.voiceSampleId + segmentIndex;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 fallbackMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
		uint32 elapsed = 0;

		if (!pathPresented) {
			memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
			drawForegroundActorFrame(foregroundFrame);
			drawRonEntryPathFrame(0, kRonEntryPathDurationMillis);
			presentFrame();
			pathPresented = true;
		}

		while (!_skipRequested && !Engine::shouldQuit()) {
			const bool speechActive = _speech.isPlaying();
			if (!speechActive && elapsed >= fallbackMillis)
				break;

			if (pollEvents())
				return;

			const uint32 slice = 10;
			g_system->delayMillis(slice);
			elapsed += slice;

			const uint32 now = g_system->getMillis();
			bool dirty = false;
			bool clockDirty = false;
			if (now - _lastClockFrameMillis >= 1000) {
				_lastClockFrameMillis = now;
				clockDirty = true;
				dirty = true;
			}
			if (now - _lastTalkingFrameMillis >= 125) {
				_lastTalkingFrameMillis = now;
				foregroundFrame = (byte)(_foregroundTalkBaseFrame + nextTalkingFrameVariant());
				dirty = true;
			}
			if (pathElapsed < kRonEntryPathDurationMillis && now - lastPathFrameMillis >= 60) {
				pathElapsed = MIN<uint32>(kRonEntryPathDurationMillis, pathElapsed + (now - lastPathFrameMillis));
				lastPathFrameMillis = now;
				dirty = true;
			}

			if (dirty) {
				memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
				if (clockDirty)
					drawClockFrame((byte)((_clockFrame + 1) % kI10ClockDescriptorCount));
				drawForegroundActorFrame(foregroundFrame);
				drawRonEntryPathFrame(pathElapsed, kRonEntryPathDurationMillis);
				presentFrame();
			}
		}

		if (segmentIndex + 1 < segmentCount && !_skipRequested && !Engine::shouldQuit())
			delayFrame(375, kTalkingOverlayNone, 0, true, true);
	}

	if (!_skipRequested && !Engine::shouldQuit()) {
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawForegroundActorFrame(_foregroundTalkBaseFrame);
		drawRonEntryPathFrame(kRonEntryPathDurationMillis, kRonEntryPathDurationMillis);
		presentFrame();
		_deskPrimaryActorVisible = true;
		_deskPrimaryActorFrame = 0;
	}
}

void Scene9100::drawRonEntryPathFrame(uint32 pathElapsedMillis, uint32 pathDurationMillis) {
	const int originalStartX = 0x307;
	const int originalStartY = 0x1d4;
	const int targetX = 0xc0;
	const int targetY = 0x191;
	const uint startDescriptorIndex = (4 * kActorCelsPerFacing) + 1;
	if (startDescriptorIndex >= _actorBankB4.descriptors.size())
		return;

	const ActorSpriteDescriptor &startDescriptor = _actorBankB4.descriptors[startDescriptorIndex];
	const int startSpriteLeft = MAX<int>(0, HollywoodEngine::kScreenWidth - (int)startDescriptor.width - 8);
	const int fixedViewportStartX = startSpriteLeft + startDescriptor.anchorX;

	const int visibleStartY = originalStartY +
		((originalStartX - fixedViewportStartX) * (targetY - originalStartY)) / (originalStartX - targetX);
	const uint32 clampedElapsed = MIN<uint32>(pathElapsedMillis, pathDurationMillis);
	const int x = fixedViewportStartX + ((targetX - fixedViewportStartX) * (int)clampedElapsed) / (int)pathDurationMillis;
	const int y = visibleStartY + ((targetY - visibleStartY) * (int)clampedElapsed) / (int)pathDurationMillis;

	const bool finalFrame = clampedElapsed >= pathDurationMillis;
	const byte facing = finalFrame ? 5 : 4;
	const byte cel = finalFrame ? 0 : (byte)(1 + ((clampedElapsed / 60) % 12));
	drawActorSpriteFrame(_actorBankB4, facing, cel, x, y);
}

void Scene9100::runSueEntrySequence() {
	if (_skipRequested || Engine::shouldQuit())
		return;

	restoreOfficeFrameAndPresent();
	applyActorHighlightColor(0x3f, 0x28, 0x32);

	animateForegroundFrames(27, 31);
	_foregroundTalkBaseFrame = 32;
	runConversationStep(1, 6, kTalkingOverlayNone, 0, true, false);
	if (_skipRequested || Engine::shouldQuit())
		return;

	animateForegroundFrames(36, 40);
	_foregroundTalkBaseFrame = 23;
	_deskPrimaryActorVisible = true;
	animateDeskPrimaryStaticFrames(0, 2);
	drawPersistentDeskActors();
	presentFrame();
	runConversationStep(1, 7, kTalkingOverlayNone, 0, false, false);
	if (_skipRequested || Engine::shouldQuit())
		return;

	runSueEntryPath();
	_deskSecondaryActorVisible = true;
	animateDeskSecondaryStaticFrames(0, 5);
	drawPersistentDeskActors();
	presentFrame();
}

void Scene9100::runSueEntryPath() {
	Common::Array<byte> baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	uint32 pathElapsed = 0;
	uint32 lastPathFrameMillis = g_system->getMillis();

	memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
	drawSueEntryPathFrame(0, kSueEntryPathDurationMillis);
	presentFrame();

	while (pathElapsed < kSueEntryPathDurationMillis && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		g_system->delayMillis(10);

		const uint32 now = g_system->getMillis();
		if (now - lastPathFrameMillis < 60)
			continue;

		pathElapsed = MIN<uint32>(kSueEntryPathDurationMillis, pathElapsed + (now - lastPathFrameMillis));
		lastPathFrameMillis = now;
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawSueEntryPathFrame(pathElapsed, kSueEntryPathDurationMillis);
		presentFrame();
	}

	if (!_skipRequested && !Engine::shouldQuit()) {
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawSueEntryPathFrame(kSueEntryPathDurationMillis, kSueEntryPathDurationMillis);
		presentFrame();
	}
}

void Scene9100::drawSueEntryPathFrame(uint32 pathElapsedMillis, uint32 pathDurationMillis) {
	const int originalStartX = 0x308;
	const int originalStartY = 0x1b5;
	const int targetX = 0x11b;
	const int targetY = 0x16e;
	const uint startDescriptorIndex = (4 * kActorCelsPerFacing) + 1;
	if (startDescriptorIndex >= _actorBank00.descriptors.size())
		return;

	const ActorSpriteDescriptor &startDescriptor = _actorBank00.descriptors[startDescriptorIndex];
	const int startSpriteLeft = MAX<int>(0, HollywoodEngine::kScreenWidth - (int)startDescriptor.width - 8);
	const int fixedViewportStartX = startSpriteLeft + startDescriptor.anchorX;

	const int visibleStartY = originalStartY +
		((originalStartX - fixedViewportStartX) * (targetY - originalStartY)) / (originalStartX - targetX);
	const uint32 clampedElapsed = MIN<uint32>(pathElapsedMillis, pathDurationMillis);
	const int x = fixedViewportStartX + ((targetX - fixedViewportStartX) * (int)clampedElapsed) / (int)pathDurationMillis;
	const int y = visibleStartY + ((targetY - visibleStartY) * (int)clampedElapsed) / (int)pathDurationMillis;

	const bool finalFrame = clampedElapsed >= pathDurationMillis;
	const byte facing = finalFrame ? 5 : 4;
	const byte cel = finalFrame ? 0 : (byte)(1 + ((clampedElapsed / 60) % 12));
	drawActorSpriteFrame(_actorBank00, facing, cel, x, y);
}

void Scene9100::drawActorSpriteFrame(const ActorBank &bank, byte facing, byte cel, int worldX, int worldY) {
	if (facing >= kActorFacingCount || cel >= kActorCelsPerFacing)
		return;

	const uint descriptorIndex = facing * kActorCelsPerFacing + cel;
	if (descriptorIndex >= bank.descriptors.size())
		return;

	const ActorSpriteDescriptor &descriptor = bank.descriptors[descriptorIndex];
	uint cursor = facing * kActorFacingRunStride + descriptor.runStreamOffset;
	if (cursor >= bank.runStreams.size())
		return;

	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	for (uint runIndex = 0; runIndex < descriptor.opaqueRunCount; ++runIndex) {
		if (cursor + 3 > bank.runStreams.size())
			return;

		const int xOffset = bank.runStreams[cursor++];
		const int yOffset = bank.runStreams[cursor++];
		const uint pixelCount = bank.runStreams[cursor++];
		if (cursor + pixelCount > bank.runStreams.size())
			return;

		const int dstY = spriteY + yOffset;
		if (dstY >= 0 && dstY < HollywoodEngine::kSceneBufferHeight) {
			int dstX = spriteX + xOffset;
			uint sourceOffset = 0;
			uint copyCount = pixelCount;
			if (dstX < 0) {
				const uint clipped = MIN<uint>(copyCount, (uint)-dstX);
				sourceOffset += clipped;
				copyCount -= clipped;
				dstX = 0;
			}
			if (dstX + (int)copyCount > HollywoodEngine::kSceneBufferWidth)
				copyCount = MAX<int>(0, HollywoodEngine::kSceneBufferWidth - dstX);

			if (copyCount != 0) {
				const uint destinationOffset = dstX + dstY * HollywoodEngine::kSceneBufferWidth;
				if (destinationOffset + copyCount <= _sceneFramebuffer.size())
					memcpy(_sceneFramebuffer.data() + destinationOffset, bank.runStreams.data() + cursor + sourceOffset, copyCount);
			}
		}

		cursor += pixelCount;
	}

	for (uint runIndex = 0; runIndex < descriptor.paletteRunCount; ++runIndex) {
		if (cursor + 3 > bank.runStreams.size())
			return;

		cursor += 3;
	}
}

void Scene9100::runOpeningPrelude() {
	animateForegroundFrames(11, 14);

	_foregroundTalkBaseFrame = 15;
	runConversationStep(0, 0, kTalkingOverlayNone, 0, true, true);

	animateForegroundFrames(20, 22);
	for (uint pulse = 0; pulse < 12 && !_skipRequested && !Engine::shouldQuit(); ++pulse) {
		if (_random.getRandomNumber(14) == 0) {
			drawForegroundActorFrame(27);
			presentFrame();
		}
		if (delayFrame(100, kTalkingOverlayNone, 0, false, true))
			return;
		if (_foregroundActorFrame == 27) {
			drawForegroundActorFrame(23);
			presentFrame();
		}
	}

	runConversationStep(0, 1, kTalkingOverlayNone, 0, false, true);
	runRonEntryConversation();
}

void Scene9100::runCinematicSequence() {
	static const CinematicStep kSteps[] = {
		{ 2, 0, 3, kTalkingOverlayBase320000, 0, true, false },
		{ 1, 1, 0, kTalkingOverlayBase0, 1, false, false },
		{ 2, 1, 1, kTalkingOverlayBase320000, 0, false, false },
		{ 4, 1, 2, kTalkingOverlayNone, 0, false, false },
		{ 2, 1, 3, kTalkingOverlayBase320000, 0, true, false },
		{ 1, 1, 4, kTalkingOverlayBase0, 1, false, false },
		{ 2, 1, 5, kTalkingOverlayBase320000, 0, false, false },
		{ 3, 1, 16, kTalkingOverlayBase640000, 0, true, false },
		{ 2, 1, 8, kTalkingOverlayBase320000, 1, false, false },
		{ 3, 1, 17, kTalkingOverlayBase640000, 1, false, false },
		{ 0, 1, 9, kTalkingOverlayBase640000, 0, false, false },
		{ 1, 1, 10, kTalkingOverlayBase0, 0, false, false },
		{ 2, 1, 11, kTalkingOverlayBase320000, 0, false, false },
		{ 3, 1, 12, kTalkingOverlayBase640000, 1, false, false },
		{ 2, 1, 13, kTalkingOverlayBase320000, 1, false, false },
		{ 4, 1, 14, kTalkingOverlayNone, 0, false, false },
		{ 3, 1, 15, kTalkingOverlayBase640000, 0, true, false }
	};

	for (uint i = 0; i < ARRAYSIZE(kSteps) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		if (i == 7)
			runSueEntrySequence();
		if (_skipRequested || Engine::shouldQuit())
			return;

		applyBackgroundMode(kSteps[i]);
		runConversationStep(kSteps[i].textBankIndex, kSteps[i].descriptorIndex,
			kSteps[i].talkingOverlayBase, kSteps[i].talkingOverlayVariant, kSteps[i].animateForegroundActor, false);
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

void Scene9100::runConversationStep(uint16 textBankIndex, byte descriptorIndex, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor, byte insetTalkBaseFrame) {
	_talkingFrame = 0;
	_lastTalkingFrameMillis = g_system->getMillis();
	const PopupDescriptor popup = getStage003PopupDescriptor(textBankIndex, descriptorIndex);
	const uint segmentCount = MAX<uint>(1, popup.continuationCount);
	for (uint segmentIndex = 0; segmentIndex < segmentCount && !_skipRequested && !Engine::shouldQuit(); ++segmentIndex) {
		const uint16 sampleId = popup.voiceSampleId + segmentIndex;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 fallbackMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
		waitForSpeechOrDelay(fallbackMillis, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor, animateClock, animateInsetActor, insetTalkBaseFrame);
		if (segmentIndex + 1 < segmentCount && !_skipRequested && !Engine::shouldQuit())
			delayFrame(375, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor, animateClock, animateInsetActor, insetTalkBaseFrame);
	}

	if (!_skipRequested && !Engine::shouldQuit()) {
		if (talkingOverlayBase != kTalkingOverlayNone) {
			drawTalkingOverlay(talkingOverlayBase, 0, talkingOverlayVariant);
			presentFrame();
		} else if (animateInsetActor) {
			drawPersistentDeskActors();
			presentFrame();
		} else if (animateForegroundActor) {
			drawForegroundActorFrame(_foregroundTalkBaseFrame);
			presentFrame();
		}
	}
}

void Scene9100::waitForSpeechOrDelay(uint32 fallbackMillis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor, byte insetTalkBaseFrame) {
	uint32 elapsed = 0;
	while (!_skipRequested && !Engine::shouldQuit()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsed >= fallbackMillis)
			break;

		if (delayFrame(50, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor, animateClock, animateInsetActor, insetTalkBaseFrame))
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
	drawPersistentDeskActors();
}

void Scene9100::drawDeskStaticActorFrame(uint32 baseOffset, uint16 descriptorCount, byte frameIndex, bool restoreBackground) {
	if (frameIndex >= descriptorCount)
		return;

	if (restoreBackground)
		restoreSpriteBackground(_secondaryScratchBuffer, baseOffset, 0, descriptorCount, frameIndex);
	drawStripSpriteFrame(_secondaryScratchBuffer, baseOffset, 0, descriptorCount, frameIndex);
}

void Scene9100::drawDeskPrimaryStaticFrame(byte frameIndex, bool restoreBackground) {
	_deskPrimaryActorFrame = MIN<byte>(frameIndex, kI10DeskPrimaryStaticDescriptorCount - 1);
	drawDeskStaticActorFrame(kDeskPrimaryStaticBase, kI10DeskPrimaryStaticDescriptorCount,
		_deskPrimaryActorFrame, restoreBackground);
}

void Scene9100::drawDeskSecondaryStaticFrame(byte frameIndex, bool restoreBackground) {
	_deskSecondaryActorFrame = MIN<byte>(frameIndex, kI10DeskSecondaryStaticDescriptorCount - 1);
	drawDeskStaticActorFrame(kDeskSecondaryStaticBase, kI10DeskSecondaryStaticDescriptorCount,
		_deskSecondaryActorFrame, restoreBackground);
}

void Scene9100::drawPersistentDeskActors() {
	if (_deskSecondaryActorVisible)
		drawDeskSecondaryStaticFrame(_deskSecondaryActorFrame, false);
	if (_deskPrimaryActorVisible)
		drawDeskPrimaryStaticFrame(_deskPrimaryActorFrame, false);
}

void Scene9100::animateForegroundFrames(byte firstFrame, byte lastFrame) {
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		drawForegroundActorFrame(frame);
		presentFrame();
		if (delayFrame(100, kTalkingOverlayNone, 0, false, true))
			return;
	}
}

void Scene9100::animateDeskPrimaryStaticFrames(byte firstFrame, byte lastFrame) {
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		drawDeskPrimaryStaticFrame(frame);
		presentFrame();
		if (delayFrame(100, kTalkingOverlayNone, 0, false, true))
			return;
	}
}

void Scene9100::animateDeskSecondaryStaticFrames(byte firstFrame, byte lastFrame) {
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		drawDeskSecondaryStaticFrame(frame);
		drawPersistentDeskActors();
		presentFrame();
		if (delayFrame(50, kTalkingOverlayNone, 0, false, true))
			return;
	}
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
	_lastTalkingFrameVariant = _talkingFrame;
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

void Scene9100::restoreOfficeFrameAndPresent() {
	memcpy(_frameDecodeBuffer.data(), _savedFramebuffer.data(), _frameDecodeBuffer.size());
	expandFillRunsToSavedFramebuffer();
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	drawForegroundActorFrame(_foregroundActorFrame);
	copyDefaultPalette();
	presentFrame();
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
		restoreOfficeFrameAndPresent();
		break;
	default:
		break;
	}

	const bool drewInitialOverlay = step.talkingOverlayBase != kTalkingOverlayNone;
	if (drewInitialOverlay)
		drawTalkingOverlay(step.talkingOverlayBase, 0, step.talkingOverlayVariant);

	if ((step.backgroundMode != 0 && step.backgroundMode != 4) || drewInitialOverlay)
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

bool Scene9100::delayFrame(uint32 millis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor, byte insetTalkBaseFrame) {
	uint32 remaining = millis;
	while (remaining != 0 && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;

		const uint32 now = g_system->getMillis();
		bool dirty = false;
		if (animateClock && now - _lastClockFrameMillis >= 1000) {
			_lastClockFrameMillis = now;
			drawClockFrame((byte)((_clockFrame + 1) % kI10ClockDescriptorCount));
			dirty = true;
		}
		if (talkingOverlayBase != kTalkingOverlayNone && now - _lastTalkingFrameMillis >= 125) {
			_lastTalkingFrameMillis = now;
			drawTalkingOverlay(talkingOverlayBase, nextTalkingFrameVariant(), talkingOverlayVariant);
			dirty = true;
		}
		if (animateInsetActor && talkingOverlayBase == kTalkingOverlayNone && now - _lastTalkingFrameMillis >= 125) {
			_lastTalkingFrameMillis = now;
			drawPersistentDeskActors();
			dirty = true;
		} else if (animateForegroundActor && talkingOverlayBase == kTalkingOverlayNone && now - _lastTalkingFrameMillis >= 125) {
			_lastTalkingFrameMillis = now;
			drawForegroundActorFrame((byte)(_foregroundTalkBaseFrame + nextTalkingFrameVariant()));
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

byte Scene9100::nextTalkingFrameVariant() {
	byte nextFrame = 0;
	do {
		nextFrame = (byte)_random.getRandomNumber(4);
	} while (nextFrame == _lastTalkingFrameVariant);

	_lastTalkingFrameVariant = nextFrame;
	return nextFrame;
}

uint32 Scene9100::getSegmentOffset(byte segmentIndex) const {
	const uint chunkIndex = 5 + segmentIndex;
	if (chunkIndex >= ARRAYSIZE(_resourceChunkOffsets))
		return 0;

	return _resourceChunkOffsets[chunkIndex];
}

Scene9100::PopupDescriptor Scene9100::getStage003PopupDescriptor(uint16 textBankIndex, byte descriptorIndex) const {
	const uint recordOffset = (textBankIndex * 500) + (descriptorIndex * 5);
	if (recordOffset + 5 > _stage003Descriptors.size())
		return PopupDescriptor{0, 0, 0};

	return PopupDescriptor{
		readUint16(_stage003Descriptors, recordOffset),
		_stage003Descriptors[recordOffset + 2],
		readUint16(_stage003Descriptors, recordOffset + 3)
	};
}

uint16 Scene9100::readUint16(const Common::Array<byte> &source, uint offset) const {
	if (offset + 2 > source.size())
		return 0;

	return source[offset] | (source[offset + 1] << 8);
}

int16 Scene9100::readSint16(const Common::Array<byte> &source, uint offset) const {
	return (int16)readUint16(source, offset);
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
