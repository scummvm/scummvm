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

#include "hollywood/scenes/intro/scene9110.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kI11ArchiveName = "RESOURCE.I11";
const char *const kStage003ArchiveName = "RESOURCE.003";
const uint16 kScene9110MusicCueId = 0x000c;
const byte kPrimarySpeechTextColor = 0xfb;
const uint kScene9110MouthInterval = 125;
const uint kScene9110IdleInterval = 100;
const uint kScene9110CycleInterval = 90;
const uint kScene9110Chunk3Interval = 75;
const uint kScene9110MusicFadeInterval = 50;

Scene9110::Scene9110(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9110"),
		_music(vm->introMusic()),
		_speech(),
		_random("hollywood_scene9110"),
		_resources(),
		_mouthAccumulator(0),
		_chunk3Accumulator(0),
		_idleAccumulator(0),
		_cycleAccumulator(0),
		_musicFadeAccumulator(0),
		_chunk2MouthFrame(2),
		_chunk2IdleFrame(0),
		_chunk2CycleFrame(6),
		_chunk3Frame(0),
		_chunk2CycleDirection(0),
		_lastMouthVariant(0xff) {
	_paletteResource.resize(kPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_stage003DecodeKey.resize(kStage003DecodeKeySize);
	_stage003Descriptors.resize(kStage003DescriptorTableSize);
	_subtitle.visible = false;
	_subtitle.colorIndex = kPrimarySpeechTextColor;
	_subtitle.centerX = 0;
	_subtitle.topY = 0;
}

bool Scene9110::play() {
	if (!load())
		return false;

	initializeCompositeState();
	drawCompositeToFramebuffer();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	if (!_music->isPlaying())
		_music->playMusicCue(kScene9110MusicCueId, 100);

	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		if (pollEvents())
			break;
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
	}

	if (!_skipRequested && !Engine::shouldQuit())
		runSpeechSequence();

	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 0x14) {
		if (pollEvents())
			break;
		clearSceneFramebufferBand(sweepOffset, 0x14);
		presentFrame();
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	stopAudio();
	return true;
}

bool Scene9110::load() {
	return loadResourceI11Assets() && loadStage003Descriptors();
}

bool Scene9110::loadResourceI11Assets() {
	if (!_resources.loadChunkTable(kI11ArchiveName))
		return false;

	for (uint i = 0; i < kI11RequiredChunkCount; ++i) {
		if (!_resources.validateChunk(kI11ArchiveName, _debugName, i))
			return false;
	}

	if (!loadResourceI11Chunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadResourceI11Chunk(1, _paletteResource, kPaletteSize))
		return false;

	uint32 resourceArenaSize = 0;
	for (uint i = 2; i < kI11RequiredChunkCount; ++i)
		resourceArenaSize += _resources.chunkTable.sizes[i];

	_resources.allocateArena(resourceArenaSize);

	for (uint i = 2; i < kI11RequiredChunkCount; ++i) {
		if (!loadResourceI11ArenaChunk(i))
			return false;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memset(_savedFramebuffer.data(), 0, _savedFramebuffer.size());
	return true;
}

bool Scene9110::loadResourceI11Chunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9110::loadResourceI11Chunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9110::loadResourceI11ArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index, index);
}

bool Scene9110::loadStage003Descriptors() {
	Common::File file;
	if (!file.open(Common::Path(kStage003ArchiveName))) {
		warning("Failed to open %s", kStage003ArchiveName);
		return false;
	}

	if (file.read(_stage003DecodeKey.data(), _stage003DecodeKey.size()) != _stage003DecodeKey.size()) {
		warning("Failed to read %s row decode key", kStage003ArchiveName);
		return false;
	}

	const uint32 stageOffsetEntry = kStage003DecodeKeySize + (kStage911Index * 4);
	if (stageOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s has no stage 911 offset entry", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffsetEntry);
	const uint32 stageOffset = file.readUint32LE();
	if (stageOffset + kStage003DescriptorTableSize > (uint32)file.size()) {
		warning("%s stage 911 descriptor table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffset);
	if (file.read(_stage003Descriptors.data(), _stage003Descriptors.size()) != _stage003Descriptors.size()) {
		warning("Failed to read %s stage 911 descriptor table", kStage003ArchiveName);
		return false;
	}

	if (file.pos() + 3 > file.size()) {
		warning("%s stage 911 text-row header is out of range", kStage003ArchiveName);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kStage003SmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kStage003LargeRowSize;
	if (file.pos() + smallRowBytes + largeRowBytes > file.size()) {
		warning("%s stage 911 text rows are out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(file.pos() + smallRowBytes);
	_stage003LargeRows.resize(largeRowBytes);
	if (file.read(_stage003LargeRows.data(), _stage003LargeRows.size()) != _stage003LargeRows.size()) {
		warning("Failed to read %s stage 911 large text rows", kStage003ArchiveName);
		return false;
	}

	for (uint row = 0; row < largeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_stage003LargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	debugC(1, kDebugResources, "Loaded %s stage 911 descriptors/text at offset=%u largeRowBase=%u largeRows=%u",
		kStage003ArchiveName, stageOffset, kStage003LargeRowBaseIndex, largeRowCount);
	return true;
}

void Scene9110::initializeCompositeState() {
	_mouthAccumulator = kScene9110MouthInterval;
	_chunk3Accumulator = kScene9110Chunk3Interval;
	_idleAccumulator = kScene9110IdleInterval;
	_cycleAccumulator = kScene9110CycleInterval;
	_musicFadeAccumulator = kScene9110MusicFadeInterval;
	_chunk2MouthFrame = 2;
	_chunk2IdleFrame = 0;
	_chunk2CycleFrame = 6;
	_chunk3Frame = 0;
	_chunk2CycleDirection = 0;
	_lastMouthVariant = 0xff;
	clearSubtitle();
}

void Scene9110::runSpeechSequence() {
	const SpeechTextStyle rightSpeech = { 0x193, 0x35, kPrimarySpeechTextColor, 0x3f, 0x28, 0x32, true };
	const SpeechTextStyle topSpeech = { 0x140, 0x14, kPrimarySpeechTextColor, 0x0c, 0x20, 0x32, true };
	const SpeechStep steps[] = {
		{ 2, 0, rightSpeech, kWaitChunk2MouthMotion },
		{ 1, 0, topSpeech, kWaitChunk3Animation },
		{ 2, 1, rightSpeech, kWaitChunk2MouthMotion },
		{ 1, 1, topSpeech, kWaitChunk3Animation },
		{ 2, 2, rightSpeech, kWaitChunk2MouthMotion }
	};

	for (uint i = 0; i < ARRAYSIZE(steps) && !_skipRequested && !Engine::shouldQuit(); ++i)
		runSpeechStep(steps[i]);
}

void Scene9110::runSpeechStep(const SpeechStep &step) {
	const PopupDescriptor popup = getStage003PopupDescriptor(step.rowIndex, step.frameIndex);
	const uint16 sampleId = popup.voiceSampleId;

	beginSubtitle(popup, step.speechTextStyle);
	presentComposite();

	const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
	const uint32 fallbackMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
	waitForSpeechOrDelay(fallbackMillis, step.waitMode);
	_speech.stop();

	if (step.waitMode == kWaitChunk2MouthMotion)
		_chunk2MouthFrame = 2;

	clearSubtitle();
	if (!_skipRequested && !Engine::shouldQuit())
		presentComposite();
}

void Scene9110::waitForSpeechOrDelay(uint32 fallbackMillis, SpeechWaitMode waitMode) {
	uint32 elapsedTotal = 0;
	uint32 lastFrameMillis = g_system->getMillis();
	while (!_skipRequested && !Engine::shouldQuit()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsedTotal >= fallbackMillis)
			break;

		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		elapsedTotal += elapsed;

		if (advanceAnimationTimers(elapsed, waitMode))
			presentComposite();

		g_system->delayMillis(1);
	}
}

bool Scene9110::advanceAnimationTimers(uint32 millis, SpeechWaitMode waitMode) {
	bool dirty = false;

	_musicFadeAccumulator += millis;
	if (_musicFadeAccumulator >= kScene9110MusicFadeInterval)
		_musicFadeAccumulator %= kScene9110MusicFadeInterval;

	_chunk3Accumulator += millis;
	if (_chunk3Accumulator >= kScene9110Chunk3Interval) {
		_chunk3Accumulator %= kScene9110Chunk3Interval;
		_chunk3Frame = (byte)_random.getRandomNumber(7);
		dirty = true;
	}

	_idleAccumulator += millis;
	if (_idleAccumulator >= kScene9110IdleInterval) {
		_idleAccumulator %= kScene9110IdleInterval;
		if (_chunk2IdleFrame == 1) {
			_chunk2IdleFrame = 0;
			dirty = true;
		} else if (_random.getRandomNumber(14) == 0) {
			_chunk2IdleFrame = 1;
			dirty = true;
		}
	}

	_cycleAccumulator += millis;
	if (_cycleAccumulator >= kScene9110CycleInterval) {
		_cycleAccumulator %= kScene9110CycleInterval;
		if (_chunk2CycleDirection == 0) {
			if (_chunk2CycleFrame < 14)
				_chunk2CycleFrame++;
			else
				_chunk2CycleFrame = 6;
		} else if (_chunk2CycleFrame < 7) {
			_chunk2CycleFrame = 14;
		} else {
			_chunk2CycleFrame--;
		}
		_chunk2CycleDirection = (byte)_random.getRandomNumber(1);
		dirty = true;
	}

	if (waitMode == kWaitChunk2MouthMotion) {
		_mouthAccumulator += millis;
		if (_mouthAccumulator >= kScene9110MouthInterval) {
			_mouthAccumulator %= kScene9110MouthInterval;
			_chunk2MouthFrame = (byte)(nextMouthFrameVariant() + 2);
			dirty = true;
		}
	}

	return dirty;
}

void Scene9110::drawCompositeToFramebuffer() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	drawDescriptorFrame(3, kI11Chunk3DescriptorCount, _chunk3Frame);
	drawDescriptorFrame(2, kI11Chunk2DescriptorCount, _chunk2MouthFrame);
	drawDescriptorFrame(2, kI11Chunk2DescriptorCount, _chunk2IdleFrame);
	drawDescriptorFrame(2, kI11Chunk2DescriptorCount, _chunk2CycleFrame);
}

void Scene9110::presentComposite() {
	drawCompositeToFramebuffer();
	presentFrame();
}

void Scene9110::drawDescriptorFrame(byte localChunkIndex, byte descriptorCount, byte descriptorIndex) {
	if (localChunkIndex >= IntroResourceSet::kResourceChunkCount)
		return;

	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[localChunkIndex], 0,
		descriptorCount, descriptorIndex, _sceneFramebuffer.surface());
}

byte Scene9110::nextMouthFrameVariant() {
	byte nextFrame = 0;
	do {
		nextFrame = (byte)_random.getRandomNumber(3);
	} while (nextFrame == _lastMouthVariant);

	_lastMouthVariant = nextFrame;
	return nextFrame;
}

void Scene9110::beginSubtitle(const PopupDescriptor &popup, const SpeechTextStyle &speechTextStyle) {
	clearSubtitle();
	if (!_vm->subtitlesEnabled())
		return;
	if (!_vm->font() || !_vm->font()->isLoaded()) {
		debugC(1, kDebugScene, "Skipping subtitle for text record %u: Hollywood font is not loaded",
			popup.textRecordId);
		return;
	}

	const Common::String text = getStage003LargeTextRecord(popup.textRecordId);
	if (text.empty()) {
		debugC(2, kDebugScene, "Skipping empty subtitle text record %u",
			popup.textRecordId);
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

void Scene9110::clearSubtitle() {
	_subtitle.visible = false;
	_subtitle.lines.clear();
}

void Scene9110::drawSubtitleOverlay() {
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

void Scene9110::wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
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

Common::String Scene9110::getStage003LargeTextRecord(uint16 recordId) const {
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

uint Scene9110::actorSpeechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene9110::calculatePrimarySubtitleBounds(const Common::Array<Common::String> &lines,
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

void Scene9110::drawFrameOverlays() {
	drawSubtitleOverlay();
}

void Scene9110::stopAudio() {
	clearSubtitle();
	_speech.stop();
	_music->stop();
}

Scene9110::PopupDescriptor Scene9110::getStage003PopupDescriptor(uint16 rowIndex, byte frameIndex) const {
	const uint recordOffset = ((uint)rowIndex * 100 + frameIndex) * 5;
	if (recordOffset + 5 > _stage003Descriptors.size())
		return PopupDescriptor{0, 0, 0};

	return PopupDescriptor{
		readUint16LE(_stage003Descriptors, recordOffset),
		_stage003Descriptors[recordOffset + 2],
		readUint16LE(_stage003Descriptors, recordOffset + 3)
	};
}

} // End of namespace Hollywood
