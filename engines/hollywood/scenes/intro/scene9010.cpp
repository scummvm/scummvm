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
#include "common/file.h"

#include "hollywood/hollywood.h"
#include "hollywood/font.h"
#include "hollywood/scenes/intro/scene9010.h"

namespace Hollywood {

const char *const kI01ArchiveName = "RESOURCE.I01";
const char *const kI02ArchiveName = "RESOURCE.I02";
const char *const kStage003ArchiveName = "RESOURCE.003";
const uint16 kPostIntroMusicCueId = 0x000e;
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
		PresentationScene(vm, "intro scene 9010", kSceneFramebufferSize, 0),
		_music(),
		_speech(vm->getLanguage(), vm->hasSpeechData()),
		_text(),
		_i02FramePayloadFormat(kI02FramePayloadUnknown),
		_i02SingleFrameOnly(false),
		_alternatePoseActive(false),
		_characterFrameIndex(0),
		_lastTalkingFrameVariant(0xff),
		_scene9010FadeCountdown(63),
		_scene9010FadeComplete(false),
		_scene9010FadeAccumulator(0) {
	_paletteSource.resize(kPaletteSize);
	_frameDecodeBuffer.resize(kFrameDecodeBufferSize);
}

bool Scene9010::play() {
	return playScene9010();
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

bool Scene9010::loadScene9010Resources() {
	if (!_resources.loadChunkTable(kI01ArchiveName))
		return false;

	if (!_resources.validateChunkRange(kI01ArchiveName, _debugName, 0, 2) ||
			((_resources._chunkTable.isValidChunk(3) ||
			_resources._chunkTable.isValidChunk(4)) &&
			(!_resources._chunkTable.isValidChunk(3) ||
			!_resources._chunkTable.isValidChunk(4)))) {
		warning("%s is missing required post-intro chunks", kI01ArchiveName);
		return false;
	}

	_i02SingleFrameOnly = false;
	if (!loadFixedChunk(0, _frameDecodeBuffer, kFrameDecodeBufferSize) ||
			!loadFixedChunk(1, _paletteSource, kPaletteSize) ||
			!loadVariableChunk(2, _characterSpriteResource))
		return false;

	if (_resources._chunkTable.isValidChunk(3)) {
		if (!loadVariableChunk(3, _i02PaletteTable) ||
				!loadVariableChunk(4, _i02FramePayload))
			return false;
	} else if (!loadI02StillFrameResource()) {
		return false;
	}

	if (!_text.loadStage(kStage003ArchiveName, _debugName, kStage901Index))
		return false;

	if (!validateI02AnimationResources()) {
		warning("%s has invalid I02 animation resources", kI01ArchiveName);
		return false;
	}

	return true;
}

bool Scene9010::validateI02AnimationResources() {
	if (_i02PaletteTable.size() < kPaletteSize || _i02FramePayload.empty())
		return false;

	_i02FramePayloadFormat = detectI02FramePayloadFormat(_i02FramePayload);
	if (_i02FramePayloadFormat == kI02FramePayloadUnknown) {
		warning("%s chunk 4 has unknown I02 frame payload format: size=%u",
			kI01ArchiveName, (uint)_i02FramePayload.size());
		return false;
	}

	debugC(1, kDebugResources, "Detected %s I02 frame payload format %u: paletteFrames=%u payloadSize=%u",
		kI01ArchiveName, (uint)_i02FramePayloadFormat, (uint)(_i02PaletteTable.size() / kPaletteSize),
		(uint)_i02FramePayload.size());
	return true;
}

bool Scene9010::loadI02StillFrameResource() {
	Common::File file;
	if (!file.open(Common::Path(kI02ArchiveName))) {
		warning("Failed to open %s", kI02ArchiveName);
		return false;
	}

	const uint32 fileSize = (uint32)file.size();
	uint32 frameSize = 0;
	if (fileSize == kRawSceneFrameSize + kPaletteSize) {
		frameSize = kRawSceneFrameSize;
		_i02FramePayloadFormat = kI02FramePayloadSceneRows;
	} else if (fileSize == kRawScreenFrameSize + kPaletteSize) {
		frameSize = kRawScreenFrameSize;
		_i02FramePayloadFormat = kI02FramePayloadScreenRows;
	} else {
		warning("%s has unexpected first-edition still frame size: %u", kI02ArchiveName, (uint)fileSize);
		return false;
	}

	_i02FramePayload.resize(frameSize);
	_i02PaletteTable.resize(kPaletteSize);
	if (file.read(_i02FramePayload.data(), _i02FramePayload.size()) != _i02FramePayload.size() ||
			file.read(_i02PaletteTable.data(), _i02PaletteTable.size()) != _i02PaletteTable.size()) {
		warning("Failed to read %s first-edition still frame", kI02ArchiveName);
		return false;
	}

	_i02SingleFrameOnly = true;
	debugC(1, kDebugResources, "Loaded %s first-edition still frame: frame=%u palette=%u",
		kI02ArchiveName, (uint)_i02FramePayload.size(), (uint)_i02PaletteTable.size());
	return true;
}

bool Scene9010::runPoseTransition(bool targetAlternatePose) {
	if (targetAlternatePose == _alternatePoseActive) {
		if (delayScene9010(2000) && (_skipRequested || Engine::shouldQuit()))
			return false;
		return true;
	}

	if (targetAlternatePose) {
		_characterFrameIndex = 5;
		while (_characterFrameIndex <= 0x0e && !_skipRequested && !Engine::shouldQuit()) {
			drawCharacterFrame(_characterFrameIndex);
			presentFrame();
			if (_characterFrameIndex == 0x0e)
				break;
			if (delayScene9010(75)) {
				if (_skipRequested || Engine::shouldQuit())
					return false;
				break;
			}
			_characterFrameIndex++;
		}
		_characterFrameIndex = 0x0e;
		drawCharacterFrame(_characterFrameIndex);
		presentFrame();
		_alternatePoseActive = true;
		return true;
	}

	_characterFrameIndex = 19;
	while (_characterFrameIndex <= 0x1c && !_skipRequested && !Engine::shouldQuit()) {
		drawCharacterFrame(_characterFrameIndex);
		presentFrame();
		if (_characterFrameIndex == 0x1c)
			break;
		if (delayScene9010(75)) {
			if (_skipRequested || Engine::shouldQuit())
				return false;
			break;
		}
		_characterFrameIndex++;
	}

	_alternatePoseActive = false;
	_characterFrameIndex = 0;
	drawCharacterFrame(_characterFrameIndex);
	presentFrame();
	return true;
}

bool Scene9010::playSpeechExchange(byte descriptorIndex) {
	const SceneSpeechCue popup = _text.stageCue(kScene9010SpeechRowIndex, descriptorIndex);
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
				delayScene9010(375) && (_skipRequested || Engine::shouldQuit()))
			return false;
	}

	_characterFrameIndex = _alternatePoseActive ? 0x0e : 0;
	drawCharacterFrame(_characterFrameIndex);
	presentFrame();

	if (_skipRequested || Engine::shouldQuit())
		return false;

	if (delayScene9010(250) && (_skipRequested || Engine::shouldQuit()))
		return false;
	return true;
}

bool Scene9010::playI02Animation() {
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	drawI02FramePayload(_i02FramePayload);
	setI02PaletteFrame(0);
	presentFrame();

	if (delay(3000))
		return !(_skipRequested || Engine::shouldQuit());

	if (_i02SingleFrameOnly)
		return true;

	ResourceChunkTable i02ChunkTable;
	const uint chunkedFrameCount = detectI02ChunkedFrameCount(i02ChunkTable);
	if (chunkedFrameCount != 0) {
		for (uint frameIndex = 0; frameIndex < chunkedFrameCount && !_skipRequested &&
				!Engine::shouldQuit(); ++frameIndex) {
			if (!loadI02ChunkedFrame(frameIndex))
				return false;

			setI02PaletteFrame(frameIndex + 1);
			drawI02FramePayload(_i02FramePayload);
			presentFrame();

			if (delay(50))
				return !(_skipRequested || Engine::shouldQuit());
		}

		return true;
	}

	Common::File file;
	if (!file.open(Common::Path(kI02ArchiveName))) {
		warning("Failed to open %s", kI02ArchiveName);
		return false;
	}
	if (_i02FramePayload.empty() || file.size() <= 0 ||
			(uint32)file.size() % _i02FramePayload.size() != 0) {
		warning("%s has unexpected raw stream size: %lld payloadSize=%u",
			kI02ArchiveName, (long long)file.size(), (uint)_i02FramePayload.size());
		return false;
	}

	const uint rawFrameCount = (uint32)file.size() / _i02FramePayload.size();
	for (uint frameIndex = 0; frameIndex < rawFrameCount && !_skipRequested &&
			!Engine::shouldQuit(); ++frameIndex) {
		if (!readI02StreamFrame(file))
			return false;

		setI02PaletteFrame(frameIndex + 1);
		drawI02FramePayload(_i02FramePayload);
		presentFrame();

		if (delay(50))
			return !(_skipRequested || Engine::shouldQuit());
	}

	return true;
}

uint Scene9010::detectI02ChunkedFrameCount(ResourceChunkTable &chunkTable) const {
	Common::File file;
	if (!file.open(Common::Path(kI02ArchiveName)))
		return 0;

	if (!chunkTable.load(file))
		return 0;

	uint frameCount = 0;
	const uint32 fileSize = (uint32)file.size();
	for (uint index = 0; index < kResourceChunkCount; ++index) {
		const uint32 offset = chunkTable.offsets[index];
		const uint32 size = chunkTable.sizes[index];
		if (size == 0)
			break;
		if (offset > fileSize || size > fileSize - offset)
			return 0;
		++frameCount;
	}

	return frameCount;
}

bool Scene9010::loadI02ChunkedFrame(uint frameIndex) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		createResourceChunkReadStream(Common::Path(kI02ArchiveName), frameIndex));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI02ArchiveName, frameIndex);
		return false;
	}

	_i02FramePayload.resize(stream->size());
	if (stream->read(_i02FramePayload.data(), _i02FramePayload.size()) != _i02FramePayload.size()) {
		warning("Failed to read %s chunk %u", kI02ArchiveName, frameIndex);
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
	const uint paletteFrameCount = _i02PaletteTable.size() / kPaletteSize;
	if (paletteFrameCount == 0)
		return;

	frameIndex = MIN<uint>(frameIndex, paletteFrameCount - 1);
	const uint offset = frameIndex * kPaletteSize;
	memcpy(_paletteSource.data(), _i02PaletteTable.data() + offset, kPaletteSize);
	memcpy(_paletteCurrent.data(), _paletteSource.data(), kPaletteSize);
}

Scene9010::I02FramePayloadFormat Scene9010::detectI02FramePayloadFormat(
		const Common::Array<byte> &payload) const {
	if (isValidI02BlockListFrame(payload))
		return kI02FramePayloadBlockList;
	if (payload.size() == kRawScreenFrameSize)
		return kI02FramePayloadScreenRows;
	if (payload.size() == kRawSceneFrameSize)
		return kI02FramePayloadSceneRows;
	return kI02FramePayloadUnknown;
}

bool Scene9010::isValidI02BlockListFrame(const Common::Array<byte> &payload) const {
	if (payload.size() < 2)
		return false;

	const uint16 blockCount = readUint16(payload, 0);
	uint cursor = 2;
	for (uint blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
		if (cursor + 6 > payload.size())
			return false;

		const uint32 destination = readUint32(payload, cursor);
		const uint16 size = readUint16(payload, cursor + 4);
		cursor += 6;

		const uint x = destination & 0xffff;
		const uint y = (destination >> 16) & 0xffff;
		const uint destinationOffset = x + y * HollywoodEngine::kSceneBufferWidth;
		if (cursor + size > payload.size() || destinationOffset + size > _sceneFramebuffer.size())
			return false;

		cursor += size;
	}

	return cursor <= payload.size();
}

void Scene9010::drawI02FramePayload(const Common::Array<byte> &payload) {
	I02FramePayloadFormat format = _i02FramePayloadFormat;
	if (format == kI02FramePayloadUnknown)
		format = detectI02FramePayloadFormat(payload);
	else if ((format == kI02FramePayloadBlockList && !isValidI02BlockListFrame(payload)) ||
			(format == kI02FramePayloadScreenRows && payload.size() != kRawScreenFrameSize) ||
			(format == kI02FramePayloadSceneRows && payload.size() != kRawSceneFrameSize))
		format = detectI02FramePayloadFormat(payload);

	switch (format) {
	case kI02FramePayloadBlockList:
		drawResourceBlockList(payload, 0, _sceneFramebuffer.surface());
		break;
	case kI02FramePayloadScreenRows:
		drawRawI02ScreenRows(payload);
		break;
	case kI02FramePayloadSceneRows:
		drawRawI02SceneRows(payload);
		break;
	default:
		warning("%s frame has unknown I02 payload format: size=%u", kI02ArchiveName,
			(uint)payload.size());
		break;
	}
}

void Scene9010::drawRawI02ScreenRows(const Common::Array<byte> &payload) {
	if (payload.size() < kRawScreenFrameSize)
		return;

	for (uint row = 0; row < HollywoodEngine::kScreenHeight; ++row) {
		const uint sourceOffset = row * HollywoodEngine::kScreenWidth;
		const uint destinationOffset = row * HollywoodEngine::kSceneBufferWidth;
		memcpy(_sceneFramebuffer.data() + destinationOffset, payload.data() + sourceOffset,
			HollywoodEngine::kScreenWidth);
	}
}

void Scene9010::drawRawI02SceneRows(const Common::Array<byte> &payload) {
	if (payload.size() < kRawSceneFrameSize)
		return;

	memcpy(_sceneFramebuffer.data(), payload.data(), kRawSceneFrameSize);
}

byte Scene9010::nextTalkingFrameVariant() {
	_lastTalkingFrameVariant = (byte)((_lastTalkingFrameVariant + 1) % 5);
	return _lastTalkingFrameVariant;
}

void Scene9010::drawCharacterFrame(byte frameIndex) {
	if (frameIndex >= ARRAYSIZE(kCharacterDescriptorSequence))
		frameIndex = 0;

	const byte descriptorIndex = kCharacterDescriptorSequence[frameIndex];
	restoreSpriteBackground(_characterSpriteResource, 0, 0, kCharacterFrameDescriptorCount,
		descriptorIndex, _frameDecodeBuffer.surface(), _sceneFramebuffer.surface());
	drawStripSpriteFrame(_characterSpriteResource, 0, 0, kCharacterFrameDescriptorCount,
		descriptorIndex, _sceneFramebuffer.surface());
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

void Scene9010::beginSubtitle(const SceneSpeechCue &popup, uint segmentIndex) {
	clearSubtitle();

	const Common::String text = _text.largeTextRecord(popup.textRecordId + segmentIndex);
	if (text.empty()) {
		debugC(2, kDebugScene, "Skipping empty scene 9010 subtitle text record %u",
			popup.textRecordId + segmentIndex);
		return;
	}

	const SpeechTextStyle speechTextStyle = getCurrentSpeechTextStyle();
	if (!showAnchoredSubtitle(text, speechTextStyle.colorIndex,
			speechTextStyle.centerX, speechTextStyle.topY))
		return;

	_paletteSource[speechTextStyle.colorIndex * 3] = speechTextStyle.red;
	_paletteSource[speechTextStyle.colorIndex * 3 + 1] = speechTextStyle.green;
	_paletteSource[speechTextStyle.colorIndex * 3 + 2] = speechTextStyle.blue;
	_paletteCurrent[speechTextStyle.colorIndex * 3] = speechTextStyle.red;
	_paletteCurrent[speechTextStyle.colorIndex * 3 + 1] = speechTextStyle.green;
	_paletteCurrent[speechTextStyle.colorIndex * 3 + 2] = speechTextStyle.blue;

}

Scene9010::SpeechTextStyle Scene9010::getCurrentSpeechTextStyle() const {
	if (_alternatePoseActive)
		return SpeechTextStyle{0x0e0, 0x108, kScene9010SpeechTextColor, 0x20, 0x3f, 0x3f};

	return SpeechTextStyle{0x0c8, 0x106, kScene9010SpeechTextColor, 0x20, 0x3f, 0x3f};
}

bool Scene9010::delayScene9010(uint32 millis) {
	TimedPresentationLoop loop(*this, millis);
	while (loop.beginFrame()) {
		const uint32 slice = loop.finishFrame();
		_scene9010FadeAccumulator += slice;
		while (_scene9010FadeAccumulator >= 150) {
			_scene9010FadeAccumulator -= 150;
			updateScene9010PaletteFade();
			presentFrame();
		}
	}

	return consumeStepAdvanceRequest() || _skipRequested || Engine::shouldQuit();
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
