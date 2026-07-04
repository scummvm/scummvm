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

#include "hollywood/scenes/intro/scene9010.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kI01ArchiveName = "RESOURCE.I01";
const char *const kI02ArchiveName = "RESOURCE.I02";
const char *const kI03ArchiveName = "RESOURCE.I03";
const char *const kStage003ArchiveName = "RESOURCE.003";
const uint16 kPostIntroMusicCueId = 0x000e;
const uint kStage003DecodeKeySize = 0x141;
const uint kStage003StageOffsetTableSize = 0xff4;
const uint kStage901Index = 901;
const uint16 kScene9010SpeechRowIndex = 1;
const byte kScene9010SpeechTextColor = 0x7a;
const byte kPaletteFadeExcludedIndex = 122;

const byte kCharacterDescriptorSequence[] = {
	0x0f, 0x0d, 0x0e, 0x10, 0x00, 0x01, 0x02, 0x03,
	0x04, 0x05, 0x06, 0x07, 0x08, 0x0b, 0x08, 0x09,
	0x0a, 0x0c, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03,
	0x02, 0x01, 0x00, 0x0f, 0x00, 0x00, 0x00
};

const byte kPopupSequence[] = {
	1, 3, 4, 5, 2, 0, 4, 1, 5, 3, 0, 2
};

Scene9010::Scene9010(HollywoodEngine *vm) :
		_vm(vm),
		_music(),
		_speech(),
		_skipRequested(false),
		_alternatePoseActive(false),
		_characterFrameIndex(0),
		_lastTalkingFrameVariant(0xff),
		_scene9010FadeCountdown(63),
		_scene9010FadeComplete(false),
		_scene9010FadeAccumulator(0) {
	_paletteSource.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_frameDecodeBuffer.resize(kFrameDecodeBufferSize);
	_sceneFramebuffer.resize(kSceneFramebufferSize);
	_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_stage003DecodeKey.resize(kStage003LargeRowSize);
	_stage003Descriptors.resize(kStage003DescriptorTableSize);
	_subtitle.visible = false;
	_subtitle.colorIndex = kScene9010SpeechTextColor;
	_subtitle.centerX = 0;
	_subtitle.topY = 0;
}

bool Scene9010::play() {
	if (!playScene9010())
		return false;

	if (!_skipRequested && !Engine::shouldQuit() && !playScene9030())
		return false;

	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	return true;
}

bool Scene9010::playScene9010() {
	if (!loadScene9010Resources())
		return false;

	_music.playMusicCue(kPostIntroMusicCueId, 100);

	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _frameDecodeBuffer.size());
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	_scene9010FadeCountdown = 63;
	_scene9010FadeComplete = false;
	_scene9010FadeAccumulator = 0;
	_alternatePoseActive = false;
	_characterFrameIndex = 0;
	_lastTalkingFrameVariant = 0xff;

	drawCharacterFrame(_characterFrameIndex);
	presentFrame();

	for (uint exchangeIndex = 0; exchangeIndex < 8 && !_skipRequested && !Engine::shouldQuit(); ++exchangeIndex) {
		const bool targetAlternatePose = exchangeIndex != 7 && (exchangeIndex % 2) == 0;
		if (!runPoseTransition(targetAlternatePose))
			return true;

		if (!playSpeechExchange(kPopupSequence[exchangeIndex]))
			return true;
	}

	if (_skipRequested || Engine::shouldQuit())
		return true;

	if (!playI02Animation())
		return _skipRequested || Engine::shouldQuit();

	return true;
}

bool Scene9010::playScene9030() {
	if (!loadI03Scene())
		return false;

	presentFrame();
	if (delay(5000))
		return true;

	uint32 elapsedRevealMillis = 0;
	uint rowOffset = 0;
	while (elapsedRevealMillis < 6000 && !_skipRequested && !Engine::shouldQuit()) {
		if (rowOffset < 0x118) {
			rowOffset += 4;
			presentFrame(rowOffset);
		}
		if (delay(50))
			return true;
		elapsedRevealMillis += 50;
	}

	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 20) {
		clearFinalSweepBand(0x118, sweepOffset, 0x14);
		presentFrame(0x118);
		if (delay(50))
			return true;
	}

	return true;
}

bool Scene9010::loadScene9010Resources() {
	if (!_vm->resources()->readChunkTable(Common::Path(kI01ArchiveName), _i01ChunkTable)) {
		warning("Failed to read %s header", kI01ArchiveName);
		return false;
	}

	if (!_i01ChunkTable.isValidChunk(0) || !_i01ChunkTable.isValidChunk(1) || !_i01ChunkTable.isValidChunk(2) ||
			!_i01ChunkTable.isValidChunk(3) || !_i01ChunkTable.isValidChunk(4)) {
		warning("%s is missing required post-intro chunks", kI01ArchiveName);
		return false;
	}

	if (!loadI01Chunk(0, _frameDecodeBuffer, kFrameDecodeBufferSize) ||
			!loadI01Chunk(1, _paletteSource, kPaletteSize) ||
			!loadI01Chunk(2, _resourceArena, 0) ||
			!loadI01Chunk(3, _i02PaletteTable, 0) ||
			!loadI01Chunk(4, _i02FramePayload, 0) ||
			!loadStage003Descriptors())
		return false;

	if (_i02PaletteTable.size() < 21 * kPaletteSize || _i02FramePayload.size() != 0x4bb42) {
		warning("%s has invalid I02 animation resources", kI01ArchiveName);
		return false;
	}

	return true;
}

bool Scene9010::loadI01Chunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI01ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI01ArchiveName, index);
		return false;
	}

	if (fixedSize != 0) {
		if (stream->size() > fixedSize || destination.size() < fixedSize) {
			warning("%s chunk %u does not fit its fixed destination", kI01ArchiveName, index);
			return false;
		}
	} else {
		destination.resize(stream->size());
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI01ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u", kI01ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene9010::loadI01Chunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI01ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI01ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed destination", kI01ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI01ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u", kI01ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene9010::loadStage003Descriptors() {
	Common::File file;
	if (!file.open(Common::Path(kStage003ArchiveName))) {
		warning("Failed to open %s", kStage003ArchiveName);
		return false;
	}

	if (file.read(_stage003DecodeKey.data(), _stage003DecodeKey.size()) != _stage003DecodeKey.size()) {
		warning("Failed to read %s row decode key", kStage003ArchiveName);
		return false;
	}

	const uint32 stageOffsetEntry = kStage003DecodeKeySize + (kStage901Index * 4);
	if (stageOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s has no stage 901 offset entry", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffsetEntry);
	const uint32 stageOffset = file.readUint32LE();
	if (stageOffset + kStage003DescriptorTableSize > (uint32)file.size()) {
		warning("%s stage 901 descriptor table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffset);
	if (file.read(_stage003Descriptors.data(), _stage003Descriptors.size()) != _stage003Descriptors.size()) {
		warning("Failed to read %s stage 901 descriptor table", kStage003ArchiveName);
		return false;
	}

	if (file.pos() + 3 > file.size()) {
		warning("%s stage 901 text-row header is out of range", kStage003ArchiveName);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kStage003SmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kStage003LargeRowSize;
	if (file.pos() + smallRowBytes + largeRowBytes > file.size()) {
		warning("%s stage 901 text rows are out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(file.pos() + smallRowBytes);
	_stage003LargeRows.resize(largeRowBytes);
	if (file.read(_stage003LargeRows.data(), _stage003LargeRows.size()) != _stage003LargeRows.size()) {
		warning("Failed to read %s stage 901 large text rows", kStage003ArchiveName);
		return false;
	}

	for (uint row = 0; row < largeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_stage003LargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	debugC(1, kDebugResources, "Loaded %s stage 901 descriptors at offset=%u", kStage003ArchiveName, stageOffset);
	return true;
}

bool Scene9010::loadI03Scene() {
	Common::File file;
	if (!file.open(Common::Path(kI03ArchiveName))) {
		warning("Failed to open %s", kI03ArchiveName);
		return false;
	}

	if (file.size() < 0x140) {
		warning("%s is too small", kI03ArchiveName);
		return false;
	}

	file.seek(0xa0);
	const uint32 sceneByteCount = file.readUint32LE();
	const uint32 paletteByteCount = file.readUint32LE();
	if (sceneByteCount == 0 || sceneByteCount > _sceneFramebuffer.size() ||
			0x140 + sceneByteCount + paletteByteCount > (uint32)file.size()) {
		warning("%s has invalid scene span: scene=%u palette=%u", kI03ArchiveName, sceneByteCount, paletteByteCount);
		return false;
	}

	file.seek(0x140);
	if (kFrameDecodeBufferSize + sceneByteCount > _sceneFramebuffer.size()) {
		warning("%s scene does not fit appended scene buffer", kI03ArchiveName);
		return false;
	}

	if (file.read(_sceneFramebuffer.data() + kFrameDecodeBufferSize, sceneByteCount) != sceneByteCount) {
		warning("Failed to read %s scene", kI03ArchiveName);
		return false;
	}

	if (paletteByteCount != 0) {
		if (paletteByteCount > _paletteSource.size()) {
			warning("%s palette is too large: %u", kI03ArchiveName, paletteByteCount);
			return false;
		}
		if (file.read(_paletteSource.data(), paletteByteCount) != paletteByteCount) {
			warning("Failed to read %s palette", kI03ArchiveName);
			return false;
		}
	}

	debugC(1, kDebugResources, "Loaded %s scene=%u palette=%u", kI03ArchiveName, sceneByteCount, paletteByteCount);
	return true;
}

bool Scene9010::runPoseTransition(bool targetAlternatePose) {
	if (targetAlternatePose == _alternatePoseActive)
		return !delayScene9010(2000);

	if (targetAlternatePose) {
		_characterFrameIndex = 5;
		while (_characterFrameIndex <= 0x0e && !_skipRequested && !Engine::shouldQuit()) {
			drawCharacterFrame(_characterFrameIndex);
			presentFrame();
			if (_characterFrameIndex == 0x0e)
				break;
			if (delayScene9010(75))
				return false;
			_characterFrameIndex++;
		}
		_alternatePoseActive = true;
		return true;
	}

	_characterFrameIndex = 19;
	while (_characterFrameIndex <= 0x1c && !_skipRequested && !Engine::shouldQuit()) {
		drawCharacterFrame(_characterFrameIndex);
		presentFrame();
		if (_characterFrameIndex == 0x1c)
			break;
		if (delayScene9010(75))
			return false;
		_characterFrameIndex++;
	}

	_alternatePoseActive = false;
	_characterFrameIndex = 0;
	drawCharacterFrame(_characterFrameIndex);
	presentFrame();
	return true;
}

bool Scene9010::playSpeechExchange(byte descriptorIndex) {
	const PopupDescriptor popup = getStage003PopupDescriptor(descriptorIndex);
	const uint segmentCount = MAX<uint>(1, popup.continuationCount);
	for (uint segmentIndex = 0; segmentIndex < segmentCount && !_skipRequested && !Engine::shouldQuit(); ++segmentIndex) {
		const uint16 sampleId = popup.voiceSampleId + segmentIndex;
		beginSubtitle(popup, segmentIndex);
		presentFrame();

		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1000;
		uint32 elapsed = 0;

		while (elapsed < duration && !_skipRequested && !Engine::shouldQuit()) {
			const byte frameVariant = nextTalkingFrameVariant();
			_characterFrameIndex = frameVariant + (_alternatePoseActive ? 14 : 0);
			drawCharacterFrame(_characterFrameIndex);
			presentFrame();

			const uint32 step = MIN<uint32>(125, duration - elapsed);
			if (delayScene9010(step))
				break;
			elapsed += step;
		}

		_speech.stop();
		clearSubtitle();
		if (segmentIndex + 1 < segmentCount && !_skipRequested && !Engine::shouldQuit() &&
				delayScene9010(375))
			return false;
	}

	_characterFrameIndex = _alternatePoseActive ? 0x0e : 0;
	drawCharacterFrame(_characterFrameIndex);
	presentFrame();

	if (_skipRequested || Engine::shouldQuit())
		return false;

	return !delayScene9010(250);
}

bool Scene9010::playI02Animation() {
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	drawResourceBlockList(_i02FramePayload);
	setI02PaletteFrame(0);
	presentFrame();

	if (delay(3000))
		return false;

	Common::File file;
	if (!file.open(Common::Path(kI02ArchiveName))) {
		warning("Failed to open %s", kI02ArchiveName);
		return false;
	}
	if (file.size() != (int64)(_i02FramePayload.size() * 20)) {
		warning("%s has unexpected stream size: %lld", kI02ArchiveName, (long long)file.size());
		return false;
	}

	for (uint frameIndex = 1; frameIndex < 21 && !_skipRequested && !Engine::shouldQuit(); ++frameIndex) {
		if (!readI02StreamFrame(file))
			return false;

		setI02PaletteFrame(frameIndex);
		drawResourceBlockList(_i02FramePayload);
		presentFrame();

		if (delay(50))
			return false;
	}

	return true;
}

bool Scene9010::readI02StreamFrame(Common::File &file) {
	if (_i02FramePayload.size() == 0)
		return false;

	if (file.read(_i02FramePayload.data(), _i02FramePayload.size()) != _i02FramePayload.size()) {
		warning("Failed to read %s animation frame", kI02ArchiveName);
		return false;
	}

	return true;
}

void Scene9010::setI02PaletteFrame(uint frameIndex) {
	const uint offset = frameIndex * kPaletteSize;
	if (offset + kPaletteSize > _i02PaletteTable.size())
		return;

	memcpy(_paletteSource.data(), _i02PaletteTable.data() + offset, kPaletteSize);
	memcpy(_paletteCurrent.data(), _paletteSource.data(), kPaletteSize);
}

void Scene9010::drawResourceBlockList(const Common::Array<byte> &blockList) {
	if (blockList.size() < 2)
		return;

	const uint16 blockCount = readUint16(blockList, 0);
	uint cursor = 2;
	for (uint blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
		if (cursor + 6 > blockList.size())
			return;

		const uint32 destination = readUint32(blockList, cursor);
		const uint16 size = readUint16(blockList, cursor + 4);
		cursor += 6;

		const uint x = destination & 0xffff;
		const uint y = (destination >> 16) & 0xffff;
		const uint destinationOffset = x + y * HollywoodEngine::kSceneBufferWidth;
		if (cursor + size > blockList.size() || destinationOffset + size > _sceneFramebuffer.size())
			return;

		memcpy(_sceneFramebuffer.data() + destinationOffset, blockList.data() + cursor, size);
		cursor += size;
	}
}

byte Scene9010::nextTalkingFrameVariant() {
	_lastTalkingFrameVariant = (byte)((_lastTalkingFrameVariant + 1) % 5);
	return _lastTalkingFrameVariant;
}

void Scene9010::drawCharacterFrame(byte frameIndex) {
	if (frameIndex >= ARRAYSIZE(kCharacterDescriptorSequence))
		frameIndex = 0;

	const byte descriptorIndex = kCharacterDescriptorSequence[frameIndex];
	restoreSpriteBackground(descriptorIndex);
	drawStripSpriteFrame(descriptorIndex);
}

void Scene9010::restoreSpriteBackground(uint16 descriptorIndex) {
	if (descriptorIndex >= kCharacterFrameDescriptorCount)
		return;

	const uint entryOffset = kFrameDescriptorSize * descriptorIndex;
	if (entryOffset + kFrameDescriptorSize > _resourceArena.size())
		return;

	const uint32 packedWidth = readUint32(_resourceArena, entryOffset + 4);
	const uint32 packedRows = readUint32(_resourceArena, entryOffset + 8);
	const uint copyWidth = (packedWidth >> 16) & 0xffff;
	const uint x = packedWidth & 0xffff;
	const uint firstRow = packedRows & 0xffff;
	const uint lastRow = (packedRows >> 16) & 0xffff;

	if (firstRow > lastRow || copyWidth == 0)
		return;

	for (uint row = firstRow; row <= lastRow; ++row) {
		const uint destinationOffset = x + row * HollywoodEngine::kSceneBufferWidth;
		if (destinationOffset + copyWidth > _sceneFramebuffer.size() ||
				destinationOffset + copyWidth > _frameDecodeBuffer.size())
			break;

		memcpy(&_sceneFramebuffer[destinationOffset], &_frameDecodeBuffer[destinationOffset], copyWidth);
	}
}

void Scene9010::drawStripSpriteFrame(uint16 descriptorIndex) {
	if (descriptorIndex >= kCharacterFrameDescriptorCount)
		return;

	const uint entryOffset = kFrameDescriptorSize * descriptorIndex;
	if (entryOffset + kFrameDescriptorSize > _resourceArena.size())
		return;

	const uint16 spanCount = readUint16(_resourceArena, entryOffset + 12);
	uint cursor = kFrameDescriptorSize * kCharacterFrameDescriptorCount + readUint32(_resourceArena, entryOffset);
	if (cursor > _resourceArena.size())
		return;

	for (uint spanIndex = 0; spanIndex < spanCount; ++spanIndex) {
		if (cursor + 5 > _resourceArena.size())
			return;

		const uint32 destination = readUint32(_resourceArena, cursor);
		const uint dataLength = _resourceArena[cursor + 4];
		cursor += 5;

		const uint x = destination & 0xffff;
		const uint y = (destination >> 16) & 0xffff;
		const uint destinationOffset = x + y * HollywoodEngine::kSceneBufferWidth;

		if (cursor + dataLength > _resourceArena.size() ||
				destinationOffset + dataLength > _sceneFramebuffer.size())
			return;

		memcpy(&_sceneFramebuffer[destinationOffset], &_resourceArena[cursor], dataLength);
		cursor += dataLength;
	}
}

void Scene9010::updateScene9010PaletteFade() {
	if (_scene9010FadeComplete)
		return;

	for (uint paletteIndex = 0; paletteIndex < 256; ++paletteIndex) {
		if (paletteIndex == kPaletteFadeExcludedIndex)
			continue;

		for (uint channel = 0; channel < 3; ++channel) {
			const uint offset = (paletteIndex * 3) + channel;
			if (_paletteSource[offset] >= _scene9010FadeCountdown && _paletteCurrent[offset] < _paletteSource[offset])
				_paletteCurrent[offset]++;
		}
	}

	if (_scene9010FadeCountdown == 1)
		_scene9010FadeComplete = true;
	else
		_scene9010FadeCountdown--;
}

bool Scene9010::fadeInPalette(uint32 stepMillis) {
	byte fadeThreshold = 63;
	bool fadeInComplete = false;

	while (!fadeInComplete && !_skipRequested && !Engine::shouldQuit()) {
		for (uint i = 0; i < _paletteSource.size(); ++i) {
			if (_paletteSource[i] >= fadeThreshold)
				_paletteCurrent[i] = MIN<byte>(_paletteSource[i], _paletteCurrent[i] + 3);
		}

		if (fadeThreshold == 0)
			fadeInComplete = true;
		else
			fadeThreshold = fadeThreshold > 3 ? fadeThreshold - 3 : 0;

		presentFrame();
		if (delay(stepMillis))
			return true;
	}

	return _skipRequested || Engine::shouldQuit();
}

bool Scene9010::fadeOutPalette(uint32 stepMillis) {
	byte fadeThreshold = 0;
	bool fadeOutComplete = false;

	while (!fadeOutComplete && !_skipRequested && !Engine::shouldQuit()) {
		for (uint i = 0; i < _paletteSource.size(); ++i) {
			if (_paletteSource[i] >= fadeThreshold)
				_paletteCurrent[i] = _paletteCurrent[i] >= 3 ? _paletteCurrent[i] - 3 : 0;
		}

		if (fadeThreshold == 63)
			fadeOutComplete = true;
		else
			fadeThreshold = MIN<byte>(63, fadeThreshold + 3);

		presentFrame();
		if (delay(stepMillis))
			return true;
	}

	return _skipRequested || Engine::shouldQuit();
}

void Scene9010::presentFrame(uint rowOffset, uint xOffset) {
	_displayPalette.uploadFrom6Bit(_paletteCurrent);

	if (_screen.empty())
		_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());

	const Graphics::Surface &framebuffer = _sceneFramebuffer.surface();
	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		byte *destination = (byte *)_screen.getBasePtr(0, y);
		const uint sourceY = y + rowOffset;
		if (sourceY < (uint)framebuffer.h && xOffset + HollywoodEngine::kScreenWidth <= (uint)framebuffer.w) {
			memcpy(destination, framebuffer.getBasePtr(xOffset, sourceY), HollywoodEngine::kScreenWidth);
		} else {
			memset(destination, 0, HollywoodEngine::kScreenWidth);
		}
	}

	drawSubtitleOverlay();
	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0,
		HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight);
	g_system->updateScreen();
}

void Scene9010::beginSubtitle(const PopupDescriptor &popup, uint segmentIndex) {
	clearSubtitle();
	if (!_vm->subtitlesEnabled())
		return;
	if (!_vm->font() || !_vm->font()->isLoaded()) {
		debugC(1, kDebugScene, "Skipping scene 9010 subtitle text record %u: Hollywood font is not loaded",
			popup.textRecordId + segmentIndex);
		return;
	}

	const Common::String text = getStage003LargeTextRecord(popup.textRecordId + segmentIndex);
	if (text.empty()) {
		debugC(2, kDebugScene, "Skipping empty scene 9010 subtitle text record %u",
			popup.textRecordId + segmentIndex);
		return;
	}

	const SpeechTextStyle speechTextStyle = getCurrentSpeechTextStyle();
	wrapActorSpeechText(text, speechTextStyle.centerX, _subtitle.lines);
	if (_subtitle.lines.empty())
		return;

	_paletteSource[speechTextStyle.colorIndex * 3] = speechTextStyle.red;
	_paletteSource[speechTextStyle.colorIndex * 3 + 1] = speechTextStyle.green;
	_paletteSource[speechTextStyle.colorIndex * 3 + 2] = speechTextStyle.blue;
	_paletteCurrent[speechTextStyle.colorIndex * 3] = speechTextStyle.red;
	_paletteCurrent[speechTextStyle.colorIndex * 3 + 1] = speechTextStyle.green;
	_paletteCurrent[speechTextStyle.colorIndex * 3 + 2] = speechTextStyle.blue;

	_subtitle.visible = true;
	_subtitle.colorIndex = speechTextStyle.colorIndex;
	calculatePrimarySubtitleBounds(_subtitle.lines, speechTextStyle, _subtitle.centerX, _subtitle.topY);
}

void Scene9010::clearSubtitle() {
	_subtitle.visible = false;
	_subtitle.lines.clear();
}

void Scene9010::drawSubtitleOverlay() {
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

void Scene9010::wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
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

Common::String Scene9010::getStage003LargeTextRecord(uint16 recordId) const {
	if (recordId < kStage003LargeRowBaseIndex)
		return Common::String();

	const uint localRecordId = recordId - kStage003LargeRowBaseIndex;
	const uint offset = localRecordId * kStage003LargeRowSize;
	if (offset >= _stage003LargeRows.size())
		return Common::String();

	const byte *row = _stage003LargeRows.data() + offset;
	uint length = 0;
	while (length < kStage003LargeRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

uint Scene9010::actorSpeechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene9010::calculatePrimarySubtitleBounds(const Common::Array<Common::String> &lines,
		const SpeechTextStyle &speechTextStyle, uint16 &centerX, uint16 &topY) const {
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

Scene9010::PopupDescriptor Scene9010::getStage003PopupDescriptor(byte descriptorIndex) const {
	const uint recordOffset = ((uint)kScene9010SpeechRowIndex * 100 + descriptorIndex) * 5;
	if (recordOffset + 5 > _stage003Descriptors.size())
		return PopupDescriptor{0, 0, 0};

	return PopupDescriptor{
		readUint16(_stage003Descriptors, recordOffset),
		_stage003Descriptors[recordOffset + 2],
		readUint16(_stage003Descriptors, recordOffset + 3)
	};
}

Scene9010::SpeechTextStyle Scene9010::getCurrentSpeechTextStyle() const {
	if (_alternatePoseActive)
		return SpeechTextStyle{0x0e0, 0x108, kScene9010SpeechTextColor, 0x20, 0x3f, 0x3f};

	return SpeechTextStyle{0x0c8, 0x106, kScene9010SpeechTextColor, 0x20, 0x3f, 0x3f};
}

void Scene9010::clearFinalSweepBand(uint rowOffset, uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		clearSceneFramebufferRun(rowOffset + sweepOffset + row, leftInset, innerWidth);
		clearSceneFramebufferRun(rowOffset + (HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row, leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleLeftX = leftInset;
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = rowOffset + combinedInset + row;
			clearSceneFramebufferRun(y, middleLeftX, bandWidth);
			clearSceneFramebufferRun(y, middleRightX, bandWidth);
		}
	}
}

void Scene9010::clearSceneFramebufferRun(int y, int x, int width) {
	if (width <= 0 || y < 0 || x < 0)
		return;

	const uint offset = x + y * HollywoodEngine::kSceneBufferWidth;
	if (offset + width > _sceneFramebuffer.size())
		return;

	memset(&_sceneFramebuffer[offset], 0, width);
}

bool Scene9010::pollEvents() {
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

bool Scene9010::delay(uint32 millis) {
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

bool Scene9010::delayScene9010(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;

		_scene9010FadeAccumulator += slice;
		while (_scene9010FadeAccumulator >= 150) {
			_scene9010FadeAccumulator -= 150;
			updateScene9010PaletteFade();
			presentFrame();
		}
	}

	return _skipRequested || Engine::shouldQuit();
}

void Scene9010::stopAudio() {
	clearSubtitle();
	_speech.stop();
	_music.stop();
}

uint16 Scene9010::readUint16(const Common::Array<byte> &source, uint offset) const {
	if (offset + 2 > source.size())
		return 0;

	return source[offset] | (source[offset + 1] << 8);
}

uint32 Scene9010::readUint32(const Common::Array<byte> &source, uint offset) const {
	if (offset + 4 > source.size())
		return 0;

	return source[offset] |
		(source[offset + 1] << 8) |
		(source[offset + 2] << 16) |
		(source[offset + 3] << 24);
}

} // End of namespace Hollywood
