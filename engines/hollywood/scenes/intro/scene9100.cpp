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
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kI10ArchiveName = "RESOURCE.I10";
const char *const kStage003ArchiveName = "RESOURCE.003";
const uint16 kScene9100MusicCueId = 0x000f;
const uint16 kScene9101CompletionState = 1000;
const uint kStage003DecodeKeySize = 0x141;
const uint kStage003StageOffsetTableSize = 0xff4;
const uint kStage910Index = 910;
const uint16 kStage910LargeRowBaseIndex = 500;
const byte kPrimarySpeechTextColor = 0xfb;
const byte kSecondarySpeechTextColor = 0xfd;
const int kOriginalSpeechLineHeight = 20;
const uint kActorFacingCount = 6;
const uint kActorCelsPerFacing = 13;
const uint kActorDescriptorCount = kActorFacingCount * kActorCelsPerFacing;
const uint kActorSpriteDescriptorSize = 28;
const uint kActorFacingRunStride = 160000;
const uint kPresentationPaletteRemapTableSize = 256;
const uint kI10RonActorRunChunk = 23;
const uint kI10RonActorDescriptorChunk = 24;
const uint kI10SueActorRunChunk = 25;
const uint kI10SueActorDescriptorChunk = 26;
const byte kI10SceneActorFacing = 5;
const uint kI10SceneActorDescriptorBase = kI10SceneActorFacing * kActorCelsPerFacing;
const uint kActorEntryFrameDelayMillis = 90;
const uint32 kRonEntryPathDurationMillis = 4200;
const uint32 kSueEntryPathDurationMillis = 3600;

const byte kI10ForegroundFrameRemap[] = {
	0, 31, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13,
	32, 33, 34, 35, 14, 15, 16, 16, 17, 18, 19, 29, 20, 21, 22, 23,
	23, 24, 25, 26, 30, 22, 21, 20, 16
};

const Scene9100::SpeechTextStyle kDeskPrimaryBlueSpeech = { 0x78, 0xb9, kPrimarySpeechTextColor, 0x00, 0x26, 0x3f, true };
const Scene9100::SpeechTextStyle kInsetBlueSpeech = { 0x140, 0x50, kPrimarySpeechTextColor, 0x00, 0x26, 0x3f, true };
const Scene9100::SpeechTextStyle kInsetWhiteSpeech = { 0x140, 0x50, kPrimarySpeechTextColor, 0x3f, 0x3f, 0x3f, true };
const Scene9100::SpeechTextStyle kInsetSueSpeech = { 0x140, 0x50, kPrimarySpeechTextColor, 0x3f, 0x28, 0x32, true };
const Scene9100::SpeechTextStyle kRonSecondarySpeech = { 0x276, 0x10c, kSecondarySpeechTextColor, 0x00, 0x00, 0x00, false };
const Scene9100::SpeechTextStyle kSueSecondarySpeech = { 0x276, 0xf7, kSecondarySpeechTextColor, 0x00, 0x00, 0x00, false };

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
		_clockChunk7Frame(0x2c),
		_clockChunk8Frame(0x30),
		_clockChunk9Frame(0x20),
		_clockChunk7CarryGate(4),
		_talkingFrame(0),
		_lastTalkingFrameVariant(0xff),
		_deskPrimaryActorFrame(0),
		_deskSecondaryActorFrame(0),
		_clockVisible(false),
		_deskPrimaryActorVisible(false),
		_deskSecondaryActorVisible(false),
		_dialogueBranch(false),
		_subtitle(),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteDefault.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_frameDecodeBuffer.resize(kFrameDecodeBufferSize);
	_sceneFramebuffer.resize(kFrameDecodeBufferSize);
	_savedFramebuffer.resize(kFrameDecodeBufferSize);
	_cleanOfficeBaseFramebuffer.resize(kFrameDecodeBufferSize);
	_presentationPaletteRemapTable.resize(kPresentationPaletteRemapTableSize);
	for (uint i = 0; i < _presentationPaletteRemapTable.size(); ++i)
		_presentationPaletteRemapTable[i] = 0;
	_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_stage003DecodeKey.resize(kStage003DecodeKeySize);
	_stage003Descriptors.resize(kStage003DescriptorTableSize);
	_stage003LargeRowBaseIndex = kStage910LargeRowBaseIndex;
	_secondaryScratchBuffer.resize(kSecondaryScratchBufferSize);
	_subtitle.visible = false;
	_subtitle.colorIndex = kPrimarySpeechTextColor;
	_subtitle.centerX = 0;
	_subtitle.topY = 0;
}

bool Scene9100::play() {
	if (!load(false))
		return false;

	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _frameDecodeBuffer.size());
	runEntryActorAnimations();
	_clockVisible = true;
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
		drawResourceBlockListToSceneFramebuffer(_resourceChunkOffsets[16]);
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

bool Scene9100::playDialogueBranch() {
	if (!load(true))
		return false;

	initializeDialogueBranchOfficeState();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _sceneFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());

	copyDefaultPalette();
	presentFrame();
	_music.playMusicCue(kScene9100MusicCueId, 30);

	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
		if (delay(50))
			break;
	}

	if (!_skipRequested && !Engine::shouldQuit())
		runDialogueBranchSequence();

	if (!_skipRequested && !Engine::shouldQuit())
		runEndingWipe();

	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit()) {
		GameplayState &state = _vm->gameState();
		state.initializeRonItemResourcePages();
		state.initializeRonInventoryItems();
		state.currentInventoryOwnerIndex = 0;
		state.activeAudioChapterIndex = 9;
		state.inventoryPanelDirty = true;
		state.mainFlowStateId = kScene9101CompletionState;
	}

	return true;
}

bool Scene9100::load(bool dialogueBranch) {
	_dialogueBranch = dialogueBranch;
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
	if (dialogueBranch && !_i10ChunkTable.isValidChunk(17)) {
		warning("%s is missing scene 9101 branch chunk 17", kI10ArchiveName);
		return false;
	}
	if (!_i10ChunkTable.isValidChunk(18) || !_i10ChunkTable.isValidChunk(19) ||
			!_i10ChunkTable.isValidChunk(20) || !_i10ChunkTable.isValidChunk(21) ||
			!_i10ChunkTable.isValidChunk(22) || !_i10ChunkTable.isValidChunk(23) ||
			!_i10ChunkTable.isValidChunk(24) || !_i10ChunkTable.isValidChunk(25) ||
			!_i10ChunkTable.isValidChunk(26)) {
		warning("%s is missing required scene 9100 scratch chunks", kI10ArchiveName);
		return false;
	}

	if (!loadChunk(0, _frameDecodeBuffer, kFrameDecodeBufferSize) ||
			!loadChunk(1, _paletteDefault, kPaletteSize) ||
			!loadVariableChunk(2, _sceneFillRuns) ||
			!loadStage003Descriptors())
		return false;
	memcpy(_cleanOfficeBaseFramebuffer.data(), _frameDecodeBuffer.data(), _cleanOfficeBaseFramebuffer.size());
	memcpy(_paletteCurrent.data(), _paletteDefault.data(), _paletteCurrent.size());
	buildPresentationPaletteRemapTable(_paletteCurrent, _presentationPaletteRemapTable);

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
	if (dialogueBranch && !loadArenaChunkAlias(17, 17, 16))
		return false;

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

bool Scene9100::loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
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

bool Scene9100::loadArenaChunkAlias(uint sourceIndex, uint aliasIndex, uint targetIndex) {
	if (aliasIndex >= kResourceChunkCount || targetIndex >= kResourceChunkCount) {
		warning("%s chunk %u cannot be aliased to invalid chunk slot %u/%u",
			kI10ArchiveName, sourceIndex, aliasIndex, targetIndex);
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI10ArchiveName), sourceIndex));
	if (!stream) {
		warning("Failed to open %s branch chunk %u", kI10ArchiveName, sourceIndex);
		return false;
	}

	const uint32 destinationOffset = _resourceChunkOffsets[targetIndex];
	const uint32 requiredSize = destinationOffset + stream->size();
	if (requiredSize > _resourceArena.size()) {
		const uint oldSize = _resourceArena.size();
		_resourceArena.resize(requiredSize);
		memset(_resourceArena.data() + oldSize, 0, _resourceArena.size() - oldSize);
	}

	if (stream->read(_resourceArena.data() + destinationOffset, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s branch chunk %u", kI10ArchiveName, sourceIndex);
		return false;
	}

	_resourceChunkOffsets[aliasIndex] = destinationOffset;
	debugC(1, kDebugResources, "Loaded %s branch chunk %u as arena chunk %u: offset=%u size=%u",
		kI10ArchiveName, sourceIndex, aliasIndex, destinationOffset, (uint)stream->size());
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

	if (file.read(_stage003DecodeKey.data(), _stage003DecodeKey.size()) != _stage003DecodeKey.size()) {
		warning("Failed to read %s row decode key", kStage003ArchiveName);
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

	if (file.pos() + 3 > file.size()) {
		warning("%s stage 910 text-row header is out of range", kStage003ArchiveName);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kStage003SmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kStage003LargeRowSize;
	if (file.pos() + smallRowBytes + largeRowBytes > file.size()) {
		warning("%s stage 910 text rows are out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(file.pos() + smallRowBytes);
	_stage003LargeRows.resize(largeRowBytes);
	if (file.read(_stage003LargeRows.data(), _stage003LargeRows.size()) != _stage003LargeRows.size()) {
		warning("Failed to read %s stage 910 large text rows", kStage003ArchiveName);
		return false;
	}

	for (uint row = 0; row < largeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_stage003LargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	debugC(1, kDebugResources, "Loaded %s stage 910 descriptors/text at offset=%u largeRowBase=%u largeRows=%u",
		kStage003ArchiveName, stageOffset, _stage003LargeRowBaseIndex, largeRowCount);
	return true;
}

bool Scene9100::loadActorResources() {
	return loadI10ActorBank(kI10RonActorRunChunk, kI10RonActorDescriptorChunk, _actorBankI10Ron) &&
		loadI10ActorBank(kI10SueActorRunChunk, kI10SueActorDescriptorChunk, _actorBankI10Sue);
}

bool Scene9100::loadI10ActorBank(uint runStreamChunkIndex, uint descriptorChunkIndex, ActorSpriteBank &bank) {
	Common::ScopedPtr<Common::SeekableReadStream> runStream(_vm->resources()->createChunkReadStream(Common::Path(kI10ArchiveName), runStreamChunkIndex));
	if (!runStream) {
		warning("Failed to open %s actor run chunk %u", kI10ArchiveName, runStreamChunkIndex);
		return false;
	}

	bank.runStreams.resize(kActorFacingCount * kActorFacingRunStride);
	memset(bank.runStreams.data(), 0, bank.runStreams.size());
	bank.descriptors.resize(kActorDescriptorCount);
	for (uint i = 0; i < bank.descriptors.size(); ++i) {
		bank.descriptors[i].runStreamOffset = 0;
		bank.descriptors[i].opaqueRunCount = 0;
		bank.descriptors[i].paletteRunCount = 0;
		bank.descriptors[i].anchorX = 0;
		bank.descriptors[i].anchorY = 0;
		bank.descriptors[i].width = 0;
		bank.descriptors[i].height = 0;
	}

	if (runStream->size() > kActorFacingRunStride) {
		warning("%s actor run chunk %u is too large", kI10ArchiveName, runStreamChunkIndex);
		return false;
	}

	const uint runBase = kI10SceneActorFacing * kActorFacingRunStride;
	if (runStream->read(bank.runStreams.data() + runBase, runStream->size()) != (uint32)runStream->size()) {
		warning("Failed to read %s actor run chunk %u", kI10ArchiveName, runStreamChunkIndex);
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> descriptorStream(_vm->resources()->createChunkReadStream(Common::Path(kI10ArchiveName), descriptorChunkIndex));
	if (!descriptorStream) {
		warning("Failed to open %s actor descriptor chunk %u", kI10ArchiveName, descriptorChunkIndex);
		return false;
	}

	if (descriptorStream->size() % kActorSpriteDescriptorSize != 0) {
		warning("%s actor descriptor chunk %u has an invalid size", kI10ArchiveName, descriptorChunkIndex);
		return false;
	}

	const uint descriptorCount = descriptorStream->size() / kActorSpriteDescriptorSize;
	if (descriptorCount > kActorCelsPerFacing) {
		warning("%s actor descriptor chunk %u has too many descriptors", kI10ArchiveName, descriptorChunkIndex);
		return false;
	}

	Common::Array<byte> descriptorData;
	descriptorData.resize(descriptorStream->size());
	if (descriptorStream->read(descriptorData.data(), descriptorData.size()) != descriptorData.size()) {
		warning("Failed to read %s actor descriptor chunk %u", kI10ArchiveName, descriptorChunkIndex);
		return false;
	}

	for (uint i = 0; i < descriptorCount; ++i) {
		const uint offset = i * kActorSpriteDescriptorSize;
		ActorSpriteDescriptor &descriptor = bank.descriptors[kI10SceneActorDescriptorBase + i];
		descriptor.runStreamOffset = readUint32(descriptorData, offset);
		descriptor.opaqueRunCount = readUint32(descriptorData, offset + 4);
		descriptor.paletteRunCount = readUint32(descriptorData, offset + 8);
		descriptor.anchorX = readSint16(descriptorData, offset + 12);
		descriptor.anchorY = readSint16(descriptorData, offset + 16);
		descriptor.width = readUint16(descriptorData, offset + 20);
		descriptor.height = readUint16(descriptorData, offset + 24);
	}

	debugC(1, kDebugResources, "Loaded %s actor chunks %u/%u", kI10ArchiveName, runStreamChunkIndex, descriptorChunkIndex);
	return true;
}

void Scene9100::applyActorHighlightColor(byte highlightRed, byte highlightGreen, byte highlightBlue) {
	_paletteCurrent[0x2f7] = highlightRed;
	_paletteCurrent[0x2f8] = highlightGreen;
	_paletteCurrent[0x2f9] = highlightBlue;
}

void Scene9100::runEntryActorAnimations() {
	IndexedSurfaceBuffer baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	applyActorHighlightColor(0x3f, 0x3f, 0x3f);
	playEntryActorAnimation(_actorBankI10Ron, 0x307, 0x1d4, baseFramebuffer);

	memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
}

void Scene9100::showSueEntryActor() {
	IndexedSurfaceBuffer baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	applyActorHighlightColor(0x3f, 0x28, 0x32);
	playEntryActorAnimation(_actorBankI10Sue, 0x130, 0x172, baseFramebuffer);

	memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
}

void Scene9100::playEntryActorAnimation(const ActorSpriteBank &bank, int worldX, int worldY, IndexedSurfaceBuffer &baseFramebuffer) {
	const byte kFacingTurnToCamera = 5;
	const byte kTurnCel = 2;
	const byte kFinalCel = 0;
	const byte kFrames[][2] = {
		{ kFacingTurnToCamera, kTurnCel },
		{ kFacingTurnToCamera, kFinalCel }
	};

	if (bank.descriptors.size() < kActorDescriptorCount || bank.runStreams.size() < kActorFacingCount * kActorFacingRunStride)
		return;

	for (uint i = 0; i < ARRAYSIZE(kFrames) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawActorFrame(bank, kFrames[i][0], kFrames[i][1], worldX, worldY);
		presentFrame();
		if (delay(kActorEntryFrameDelayMillis))
			return;
	}

	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());
}

void Scene9100::runRonEntryConversation() {
	const PopupDescriptor popup = getStage003PopupDescriptor(0, 2);
	const uint segmentCount = MAX<uint>(1, popup.continuationCount);

	IndexedSurfaceBuffer baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	uint32 pathElapsed = 0;
	uint32 lastPathFrameMillis = g_system->getMillis();
	byte foregroundFrame = _foregroundTalkBaseFrame;
	bool pathPresented = false;

	for (uint segmentIndex = 0; segmentIndex < segmentCount && !_skipRequested && !Engine::shouldQuit(); ++segmentIndex) {
		const uint16 sampleId = popup.voiceSampleId + segmentIndex;
		beginSubtitle(popup, segmentIndex, kDeskPrimaryBlueSpeech);
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
					advanceClockFrame();
				drawForegroundActorFrame(foregroundFrame);
				drawRonEntryPathFrame(pathElapsed, kRonEntryPathDurationMillis);
				presentFrame();
			}
		}

		if (segmentIndex + 1 < segmentCount && !_skipRequested && !Engine::shouldQuit())
			delayFrame(375, kTalkingOverlayNone, 0, true, true);
	}
	clearSubtitle();

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
	const uint startDescriptorIndex = kI10SceneActorDescriptorBase + 1;
	if (startDescriptorIndex >= _actorBankI10Ron.descriptors.size())
		return;

	const ActorSpriteDescriptor &startDescriptor = _actorBankI10Ron.descriptors[startDescriptorIndex];
	const int startSpriteLeft = MAX<int>(0, HollywoodEngine::kScreenWidth - (int)startDescriptor.width - 8);
	const int fixedViewportStartX = startSpriteLeft + startDescriptor.anchorX;

	const int visibleStartY = originalStartY +
		((originalStartX - fixedViewportStartX) * (targetY - originalStartY)) / (originalStartX - targetX);
	const uint32 clampedElapsed = MIN<uint32>(pathElapsedMillis, pathDurationMillis);
	const int x = fixedViewportStartX + ((targetX - fixedViewportStartX) * (int)clampedElapsed) / (int)pathDurationMillis;
	const int y = visibleStartY + ((targetY - visibleStartY) * (int)clampedElapsed) / (int)pathDurationMillis;

	const bool finalFrame = clampedElapsed >= pathDurationMillis;
	const byte facing = kI10SceneActorFacing;
	const byte cel = finalFrame ? 0 : (byte)(1 + ((clampedElapsed / 60) % 12));
	drawActorFrame(_actorBankI10Ron, facing, cel, x, y);
}

void Scene9100::runSueEntrySequence() {
	if (_skipRequested || Engine::shouldQuit())
		return;

	restoreOfficeFrameAndPresent();
	applyActorHighlightColor(0x3f, 0x28, 0x32);

	animateForegroundFrames(27, 31);
	_foregroundTalkBaseFrame = 32;
	runConversationStep(1, 6, kTalkingOverlayNone, 0, true, true, kDeskPrimaryBlueSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	animateForegroundFrames(36, 40);
	_foregroundTalkBaseFrame = 23;
	_deskPrimaryActorVisible = true;
	animateDeskPrimaryStaticFrames(0, 2);
	drawOfficeCompositeLayers();
	presentFrame();
	runConversationStep(1, 7, kTalkingOverlayNone, 0, false, true, kSueSecondarySpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	runSueEntryPath();
	_deskSecondaryActorVisible = true;
	animateDeskSecondaryStaticFrames(0, 5);
	drawOfficeCompositeLayers();
	presentFrame();
}

void Scene9100::runSueEntryPath() {
	IndexedSurfaceBuffer baseFramebuffer;
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
	const uint startDescriptorIndex = kI10SceneActorDescriptorBase + 1;
	if (startDescriptorIndex >= _actorBankI10Sue.descriptors.size())
		return;

	const ActorSpriteDescriptor &startDescriptor = _actorBankI10Sue.descriptors[startDescriptorIndex];
	const int startSpriteLeft = MAX<int>(0, HollywoodEngine::kScreenWidth - (int)startDescriptor.width - 8);
	const int fixedViewportStartX = startSpriteLeft + startDescriptor.anchorX;

	const int visibleStartY = originalStartY +
		((originalStartX - fixedViewportStartX) * (targetY - originalStartY)) / (originalStartX - targetX);
	const uint32 clampedElapsed = MIN<uint32>(pathElapsedMillis, pathDurationMillis);
	const int x = fixedViewportStartX + ((targetX - fixedViewportStartX) * (int)clampedElapsed) / (int)pathDurationMillis;
	const int y = visibleStartY + ((targetY - visibleStartY) * (int)clampedElapsed) / (int)pathDurationMillis;

	const bool finalFrame = clampedElapsed >= pathDurationMillis;
	const byte facing = kI10SceneActorFacing;
	const byte cel = finalFrame ? 0 : (byte)(1 + ((clampedElapsed / 60) % 12));
	drawActorFrame(_actorBankI10Sue, facing, cel, x, y);
}

void Scene9100::drawActorFrame(const ActorSpriteBank &bank, byte facing, byte cel, int worldX, int worldY) {
	drawActorSpriteFrame(bank, facing, cel, worldX, worldY, -1,
		_sceneFramebuffer.surface(), _presentationPaletteRemapTable);
}

void Scene9100::runOpeningPrelude() {
	animateForegroundFrames(11, 14);

	_foregroundTalkBaseFrame = 15;
	runConversationStep(0, 0, kTalkingOverlayNone, 0, true, true, kDeskPrimaryBlueSpeech);

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

	runConversationStep(0, 1, kTalkingOverlayNone, 0, false, true, kRonSecondarySpeech);
	_foregroundTalkBaseFrame = 23;
	runRonEntryConversation();
}

void Scene9100::runCinematicSequence() {
	const CinematicStep kSteps[] = {
		{ 2, 0, 3, kTalkingOverlayBase320000, 0, true, false, kInsetWhiteSpeech },
		{ 1, 1, 0, kTalkingOverlayBase0, 1, false, false, kInsetBlueSpeech },
		{ 2, 1, 1, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech },
		{ 4, 1, 2, kTalkingOverlayNone, 0, false, true, kDeskPrimaryBlueSpeech },
		{ 2, 1, 3, kTalkingOverlayBase320000, 0, true, false, kInsetWhiteSpeech },
		{ 1, 1, 4, kTalkingOverlayBase0, 1, false, false, kInsetBlueSpeech },
		{ 2, 1, 5, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech },
		{ 3, 1, 16, kTalkingOverlayBase640000, 0, true, false, kInsetSueSpeech },
		{ 2, 1, 8, kTalkingOverlayBase320000, 1, false, false, kInsetWhiteSpeech },
		{ 3, 1, 17, kTalkingOverlayBase640000, 1, false, false, kInsetSueSpeech },
		{ 0, 1, 9, kTalkingOverlayBase640000, 0, false, false, kInsetSueSpeech },
		{ 1, 1, 10, kTalkingOverlayBase0, 0, false, false, kInsetBlueSpeech },
		{ 2, 1, 11, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech },
		{ 3, 1, 12, kTalkingOverlayBase640000, 1, false, false, kInsetSueSpeech },
		{ 2, 1, 13, kTalkingOverlayBase320000, 1, false, false, kInsetWhiteSpeech },
		{ 4, 1, 14, kTalkingOverlayNone, 0, false, true, kDeskPrimaryBlueSpeech },
		{ 3, 1, 15, kTalkingOverlayBase640000, 0, true, false, kInsetSueSpeech }
	};

	for (uint i = 0; i < ARRAYSIZE(kSteps) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		if (i == 7)
			runSueEntrySequence();
		if (_skipRequested || Engine::shouldQuit())
			return;

		applyBackgroundMode(kSteps[i]);
		const bool animateClock = kSteps[i].backgroundMode == 4;
		if (kSteps[i].textBankIndex == 1 && kSteps[i].descriptorIndex == 2)
			runForegroundPoseToDialogueState();
		else if (kSteps[i].textBankIndex == 1 && kSteps[i].descriptorIndex == 14)
			_foregroundTalkBaseFrame = 0x17;
		runConversationStep(kSteps[i].textBankIndex, kSteps[i].descriptorIndex,
			kSteps[i].talkingOverlayBase, kSteps[i].talkingOverlayVariant, kSteps[i].animateForegroundActor, animateClock,
			kSteps[i].speechTextStyle);
		if (kSteps[i].textBankIndex == 1 && kSteps[i].descriptorIndex == 2)
			runForegroundPoseBackToDeskIdle();
	}
}

void Scene9100::initializeDialogueBranchOfficeState() {
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	_foregroundActorFrame = 0x17;
	_foregroundTalkBaseFrame = 0x17;
	_deskPrimaryActorFrame = 0;
	_deskSecondaryActorFrame = 0;
	_deskPrimaryActorVisible = true;
	_deskSecondaryActorVisible = false;
	_clockChunk7Frame = 0x33;
	_clockChunk8Frame = 0x13;
	_clockChunk9Frame = 8;
	_clockChunk7CarryGate = 1;
	_clockVisible = true;
	copyDefaultPalette();
	drawOfficeCompositeLayers();
}

void Scene9100::runDialogueBranchSequence() {
	prepareDialogueBranchOfficePatch();

	runForegroundPoseToDialogueState();
	runConversationStep(3, 0, kTalkingOverlayNone, 0, true, true, kDeskPrimaryBlueSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	runForegroundPoseBackToDeskIdle();
	memcpy(_savedFramebuffer.data(), _frameDecodeBuffer.data(), _savedFramebuffer.size());

	prepareSceneFadeFrameWithScratchActor(0);
	runConversationStep(3, 1, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	prepareChunkDFrameWithOptionalActorOverlay(true);
	runConversationStep(3, 2, kTalkingOverlayBase0, 1, false, false, kInsetBlueSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	prepareSceneFadeFrameWithScratchActor(0);
	runConversationStep(3, 3, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	prepareChunkDFrameWithOptionalActorOverlay(true);
	runConversationStep(3, 4, kTalkingOverlayBase0, 1, false, false, kInsetBlueSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	prepareSceneFadeFrameWithScratchActor(0);
	runConversationStep(3, 5, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	restoreOfficeFrameAndPresent();
	_foregroundTalkBaseFrame = 0x17;
	drawForegroundActorFrame(_foregroundTalkBaseFrame);
	presentFrame();
	runConversationStep(3, 6, kTalkingOverlayNone, 0, true, true, kDeskPrimaryBlueSpeech);
}

void Scene9100::prepareDialogueBranchOfficePatch() {
	expandFillRunsToSavedFramebuffer();
	drawResourceBlockListToSceneFramebuffer(_resourceChunkOffsets[17]);
	drawOfficeCompositeLayers();
	presentFrame();
}

void Scene9100::runForegroundPoseToDialogueState() {
	animateForegroundFrames(27, 31);
	_foregroundTalkBaseFrame = 32;
}

void Scene9100::runForegroundPoseBackToDeskIdle() {
	animateForegroundFrames(36, 40);
	_foregroundTalkBaseFrame = 0x17;
	drawForegroundActorFrame(_foregroundTalkBaseFrame);
	presentFrame();
}

void Scene9100::prepareSceneFadeFrameWithScratchActor(byte talkingOverlayVariant) {
	applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(9));
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	copyPaletteSegment(6);
	drawTalkingOverlay(kTalkingOverlayBase320000, 0, talkingOverlayVariant);
	presentFrame();
}

void Scene9100::prepareChunkDFrameWithOptionalActorOverlay(bool actorOverlayEnabled) {
	applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(8));
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	copyPaletteSegment(5);
	if (actorOverlayEnabled)
		drawTalkingOverlay(kTalkingOverlayBase0, 0, 1);
	presentFrame();
}

void Scene9100::runEndingWipe() {
	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 20) {
		clearSceneFramebufferBand(sweepOffset, 20);
		presentFrame();
		if (delay(50))
			return;
	}
}

void Scene9100::runConversationStep(uint16 textBankIndex, byte descriptorIndex, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, const SpeechTextStyle &speechTextStyle, bool animateInsetActor, byte insetTalkBaseFrame) {
	_talkingFrame = 0;
	_lastTalkingFrameMillis = g_system->getMillis();
	const PopupDescriptor popup = getStage003PopupDescriptor(textBankIndex, descriptorIndex);
	const uint segmentCount = MAX<uint>(1, popup.continuationCount);
	for (uint segmentIndex = 0; segmentIndex < segmentCount && !_skipRequested && !Engine::shouldQuit(); ++segmentIndex) {
		const uint16 sampleId = popup.voiceSampleId + segmentIndex;
		beginSubtitle(popup, segmentIndex, speechTextStyle);
		presentFrame();
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 fallbackMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
		waitForSpeechOrDelay(fallbackMillis, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor, animateClock, animateInsetActor, insetTalkBaseFrame);
		clearSubtitle();
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

void Scene9100::beginSubtitle(const PopupDescriptor &popup, uint segmentIndex, const SpeechTextStyle &speechTextStyle) {
	clearSubtitle();
	if (!_vm->subtitlesEnabled())
		return;
	if (!_vm->font() || !_vm->font()->isLoaded()) {
		debugC(1, kDebugScene, "Skipping subtitle for text record %u: Hollywood font is not loaded",
			popup.textRecordId + segmentIndex);
		return;
	}

	const Common::String text = getStage003LargeTextRecord(popup.textRecordId + segmentIndex);
	if (text.empty()) {
		debugC(2, kDebugScene, "Skipping empty subtitle text record %u",
			popup.textRecordId + segmentIndex);
		return;
	}

	wrapActorSpeechText(text, speechTextStyle.centerX, _subtitle.lines);
	if (_subtitle.lines.empty())
		return;

	if (speechTextStyle.updatePalette) {
		_paletteCurrent[speechTextStyle.colorIndex * 3] = speechTextStyle.red;
		_paletteCurrent[speechTextStyle.colorIndex * 3 + 1] = speechTextStyle.green;
		_paletteCurrent[speechTextStyle.colorIndex * 3 + 2] = speechTextStyle.blue;
	}

	_subtitle.visible = true;
	_subtitle.colorIndex = speechTextStyle.colorIndex;
	calculatePrimarySubtitleBounds(_subtitle.lines, speechTextStyle, _subtitle.centerX, _subtitle.topY);
}

void Scene9100::clearSubtitle() {
	_subtitle.visible = false;
	_subtitle.lines.clear();
}

void Scene9100::drawSubtitleOverlay() {
	if (!_subtitle.visible || !_vm->font() || !_vm->font()->isLoaded())
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);

	for (uint lineIndex = 0; lineIndex < _subtitle.lines.size(); ++lineIndex) {
		const Common::String &line = _subtitle.lines[lineIndex];
		const int lineWidth = actorSpeechTextWidth(line);
		const int x = (int)_subtitle.centerX - (lineWidth >> 1);
		const int y = (int)_subtitle.topY + lineIndex * kOriginalSpeechLineHeight;
		font->drawString(_screen.surfacePtr(), line, x, y, lineWidth, _subtitle.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene9100::wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = 0x32;
	const int anchorX = anchorSceneX;
	if (anchorX < 0xa0)
		maxChars = (anchorX * 0x32) / 0xa0;
	else if (HollywoodEngine::kScreenWidth - anchorX < 0xa0)
		maxChars = ((HollywoodEngine::kScreenWidth - anchorX) * 0x32) / 0xa0;
	maxChars = MAX<uint>(maxChars, 0x18);

	const uint lineShrink = maxChars < 0x2a ? (maxChars > 0x20 ? 2 : 1) : 3;
	const char *source = text.c_str();
	const uint textLength = text.size();
	uint cursor = 0;
	while (cursor < textLength && lines.size() < 10) {
		uint end = textLength;
		if (cursor + maxChars < textLength) {
			end = cursor + maxChars;
			while (end > cursor && (byte)source[end] != 0x20 && source[end] != 0)
				--end;
			while (end > cursor && (byte)source[end - 1] == 0x20)
				--end;
			if (end == cursor)
				end = MIN<uint>(textLength, cursor + maxChars);
		}

		lines.push_back(Common::String(source + cursor, end - cursor));

		cursor = end;
		while (cursor < textLength && (byte)source[cursor] == 0x20)
			++cursor;

		maxChars = maxChars > lineShrink ? maxChars - lineShrink : 1;
	}
}

Common::String Scene9100::getStage003LargeTextRecord(uint16 recordId) const {
	if (recordId < _stage003LargeRowBaseIndex)
		return Common::String();

	const uint localRecordId = recordId - _stage003LargeRowBaseIndex;
	const uint offset = localRecordId * kStage003LargeRowSize;
	if (offset >= _stage003LargeRows.size())
		return Common::String();

	const byte *row = _stage003LargeRows.data() + offset;
	uint length = 0;
	while (length < kStage003LargeRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

uint Scene9100::actorSpeechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene9100::calculatePrimarySubtitleBounds(const Common::Array<Common::String> &lines, const SpeechTextStyle &speechTextStyle, uint16 &centerX, uint16 &topY) const {
	uint textWidth = 0;
	for (uint i = 0; i < lines.size(); ++i)
		textWidth = MAX<uint>(textWidth, actorSpeechTextWidth(lines[i]));

	int adjustedCenterX = speechTextStyle.centerX;
	if (((adjustedCenterX - (int)(textWidth >> 1)) - 1 + (int)textWidth) > 0x27e) {
		adjustedCenterX = 0x27d - (int)(textWidth >> 1);
		if ((textWidth & 1) == 0)
			adjustedCenterX = 0x27e - (int)(textWidth >> 1);
	}
	if (adjustedCenterX - (int)(textWidth >> 1) < 1)
		adjustedCenterX = (textWidth >> 1) + 1;

	int adjustedTopY = speechTextStyle.topY - (int)lines.size() * kOriginalSpeechLineHeight;
	if (adjustedTopY < 1)
		adjustedTopY = 1;

	centerX = (uint16)MAX<int>(0, MIN<int>(adjustedCenterX, HollywoodEngine::kScreenWidth - 1));
	topY = (uint16)MAX<int>(0, MIN<int>(adjustedTopY, HollywoodEngine::kScreenHeight - 1));
}

void Scene9100::drawInitialForegroundFrame() {
	drawForegroundActorFrame(0);
}

void Scene9100::drawForegroundActorFrame(byte frameIndex) {
	if (frameIndex >= ARRAYSIZE(kI10ForegroundFrameRemap))
		return;

	_foregroundActorFrame = frameIndex;
	restoreForegroundActorLayer();
	drawOfficeCompositeLayers();
}

void Scene9100::restoreForegroundActorLayer() {
	if (_foregroundActorFrame >= ARRAYSIZE(kI10ForegroundFrameRemap))
		return;

	const uint16 descriptorIndex = kI10ForegroundFrameRemap[_foregroundActorFrame];
	restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[5], 0, kI10ForegroundDescriptorCount, descriptorIndex);
}

void Scene9100::drawForegroundActorLayer() {
	if (_foregroundActorFrame >= ARRAYSIZE(kI10ForegroundFrameRemap))
		return;

	const uint16 descriptorIndex = kI10ForegroundFrameRemap[_foregroundActorFrame];
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[5], 0, kI10ForegroundDescriptorCount, descriptorIndex);
}

void Scene9100::drawDeskActorLayer(uint32 baseOffset, uint16 descriptorCount, byte frameIndex, bool restoreBackground) {
	if (frameIndex >= descriptorCount)
		return;

	if (restoreBackground)
		restoreSpriteBackground(_secondaryScratchBuffer, baseOffset, 0, descriptorCount, frameIndex);
	drawStripSpriteFrame(_secondaryScratchBuffer, baseOffset, 0, descriptorCount, frameIndex);
}

void Scene9100::drawDeskPrimaryStaticFrame(byte frameIndex, bool restoreBackground) {
	_deskPrimaryActorFrame = MIN<byte>(frameIndex, kI10DeskPrimaryStaticDescriptorCount - 1);
	if (restoreBackground)
		drawDeskPrimaryStaticLayer(true);
	drawOfficeCompositeLayers();
}

void Scene9100::drawDeskSecondaryStaticFrame(byte frameIndex, bool restoreBackground) {
	_deskSecondaryActorFrame = MIN<byte>(frameIndex, kI10DeskSecondaryStaticDescriptorCount - 1);
	if (restoreBackground)
		drawDeskSecondaryStaticLayer(true);
	drawOfficeCompositeLayers();
}

void Scene9100::drawDeskPrimaryStaticLayer(bool restoreBackground) {
	if (!_deskPrimaryActorVisible)
		return;

	drawDeskActorLayer(kDeskPrimaryStaticBase, kI10DeskPrimaryStaticDescriptorCount,
		_deskPrimaryActorFrame, restoreBackground);
}

void Scene9100::drawDeskSecondaryStaticLayer(bool restoreBackground) {
	if (!_deskSecondaryActorVisible)
		return;

	drawDeskActorLayer(kDeskSecondaryStaticBase, kI10DeskSecondaryStaticDescriptorCount,
		_deskSecondaryActorFrame, restoreBackground);
}

void Scene9100::drawPersistentDeskActors() {
	drawDeskSecondaryStaticLayer(false);
	drawDeskPrimaryStaticLayer(false);
}

void Scene9100::drawOfficeCompositeLayers() {
	drawForegroundActorLayer();
	drawPersistentDeskActors();
	drawClockLayers(false);
}

void Scene9100::syncOfficeRestoreBaseFromSaved() {
	memcpy(_frameDecodeBuffer.data(), _savedFramebuffer.data(), _frameDecodeBuffer.size());
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
		presentFrame();
		if (delayFrame(50, kTalkingOverlayNone, 0, false, true))
			return;
	}
}

void Scene9100::advanceClockFrame() {
	_clockVisible = true;
	_clockChunk9Frame = (_clockChunk9Frame + 1) % kI10ClockDescriptorCount;
	if (_clockChunk9Frame == 0) {
		_clockChunk8Frame = (_clockChunk8Frame + 1) % kI10ClockDescriptorCount;
		if (_clockChunk7CarryGate == 5) {
			_clockChunk7Frame = (_clockChunk7Frame + 1) % kI10ClockDescriptorCount;
			_clockChunk7CarryGate = 0;
		}
	}
	restoreClockAreaBackground();
	drawOfficeCompositeLayers();
}

void Scene9100::restoreClockAreaBackground() {
	for (byte frame = 0; frame < kI10ClockDescriptorCount; ++frame) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[7], 0, kI10ClockDescriptorCount, frame);
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[8], 0, kI10ClockDescriptorCount, frame);
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[9], 0, kI10ClockDescriptorCount, frame);
	}
}

void Scene9100::drawClockLayers(bool restoreBackground) {
	if (!_clockVisible)
		return;

	if (restoreBackground)
		restoreClockAreaBackground();
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[7], 0, kI10ClockDescriptorCount, _clockChunk7Frame);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[8], 0, kI10ClockDescriptorCount, _clockChunk8Frame);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[9], 0, kI10ClockDescriptorCount, _clockChunk9Frame);
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

void Scene9100::applyResourceSpanPatchToFrameDecodeBuffer(uint32 baseOffset) {
	drawResourceBlockListToBuffer(baseOffset, _frameDecodeBuffer);
}

void Scene9100::drawResourceBlockListToSceneFramebuffer(uint32 baseOffset) {
	drawResourceBlockListToBuffer(baseOffset, _sceneFramebuffer);
}

void Scene9100::drawResourceBlockListToBuffer(uint32 baseOffset, IndexedSurfaceBuffer &destination) {
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

void Scene9100::restoreResourceBlockListFromCleanOfficeBase(uint32 baseOffset, IndexedSurfaceBuffer &destination) {
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
		if (cursor + size > _resourceArena.size() ||
				targetOffset + size > destination.size() ||
				targetOffset + size > _cleanOfficeBaseFramebuffer.size())
			return;

		memcpy(destination.data() + targetOffset, _cleanOfficeBaseFramebuffer.data() + targetOffset, size);
		cursor += size;
	}
}

void Scene9100::removeInitialOfficeTitlePatch(IndexedSurfaceBuffer &destination) {
	if (_dialogueBranch)
		return;

	restoreResourceBlockListFromCleanOfficeBase(_resourceChunkOffsets[16], destination);
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
	syncOfficeRestoreBaseFromSaved();
	removeInitialOfficeTitlePatch(_frameDecodeBuffer);
	expandFillRunsToSavedFramebuffer();
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	drawForegroundActorFrame(_foregroundActorFrame);
	copyDefaultPalette();
	presentFrame();
}

void Scene9100::applyBackgroundMode(const CinematicStep &step) {
	if (step.copyFrameToSavedBefore) {
		memcpy(_savedFramebuffer.data(), _frameDecodeBuffer.data(), _savedFramebuffer.size());
		removeInitialOfficeTitlePatch(_savedFramebuffer);
	}

	switch (step.backgroundMode) {
	case 1:
		applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(8));
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		copyPaletteSegment(5);
		break;
	case 2:
		applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(9));
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		copyPaletteSegment(6);
		break;
	case 3:
		applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(10));
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
	buildPresentationPaletteRemapTable(_paletteCurrent, _presentationPaletteRemapTable);
}

void Scene9100::copyDefaultPalette() {
	memcpy(_paletteCurrent.data(), _paletteDefault.data(), _paletteCurrent.size());
	buildPresentationPaletteRemapTable(_paletteCurrent, _presentationPaletteRemapTable);
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
	_displayPalette.uploadFrom6Bit(_paletteCurrent);

	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint sourceOffset = y * HollywoodEngine::kSceneBufferWidth;
		memcpy(_screen.getBasePtr(0, y),
			_sceneFramebuffer.data() + sourceOffset,
			HollywoodEngine::kScreenWidth);
	}

	drawSubtitleOverlay();

	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0,
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
			advanceClockFrame();
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
	clearSubtitle();
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
