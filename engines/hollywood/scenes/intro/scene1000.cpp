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

#include "hollywood/scenes/intro/scene1000.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/paletteman.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kTitleFrontEndArchiveName = "RESOURCE.A00";
const char *const kChapter1MusicArchiveName = "RESOURCE.M01";
const uint16 kTitleFrontEndMusicCueId = 0x000b;
const uint16 kTitleFrontEndNextState = 0x03f4;
const uint32 kTitleFrontEndDurationMillis = 11250;
const uint32 kTitlePatchTimeMillis = 3000;
const uint32 kSecondaryStartTimeMillis = 5000;
const uint32 kBlinkFrameMillis = 75;
const uint32 kSecondaryFrameMillis = 75;

const byte kTitleBlinkDescriptorFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 5,
	4, 3, 2, 1, 0, 1, 2, 3,
	4, 5, 6, 7, 8, 9, 10, 11,
	10, 9, 8, 7, 6, 5, 4, 0
};

Scene1000::Scene1000(HollywoodEngine *vm) :
		_vm(vm),
		_music(),
		_random("hollywood_scene1000"),
		_resourceArenaCursor(0),
		_lastBlinkMillis(0),
		_blinkPatternMode(0),
		_blinkFrameIndex(0),
		_secondaryFrameIndex(0),
		_blinkDirty(true),
		_secondaryDirty(false),
		_secondaryVisible(false),
		_titlePatchApplied(false),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_frameDecodeBuffer.resize(kFrameBufferSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
}

bool Scene1000::play() {
	GameplayState &state = _vm->gameState();
	state.initializeRonItemResourcePages();
	state.initializeRonInventoryItems();
	state.currentInventoryOwnerIndex = 0;
	state.activeAudioChapterIndex = 1;
	state.currentAmbientMusicCueId = kTitleFrontEndMusicCueId;

	if (!load())
		return false;

	_music.setArchive(Common::Path(kChapter1MusicArchiveName));
	_music.playMusicCue(kTitleFrontEndMusicCueId, 100);

	renderOverlayFrame(true);
	fadeInPalette();

	if (!_skipRequested && !Engine::shouldQuit())
		runTitleFrontEndSequence();

	fadeOutPalette();
	_music.stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kTitleFrontEndNextState;

	return true;
}

bool Scene1000::load() {
	if (!_vm->resources()->readChunkTable(Common::Path(kTitleFrontEndArchiveName), _chunkTable)) {
		warning("Failed to read %s header", kTitleFrontEndArchiveName);
		return false;
	}

	for (uint i = 0; i <= 4; ++i) {
		if (!_chunkTable.isValidChunk(i)) {
			warning("%s is missing title front-end chunk %u", kTitleFrontEndArchiveName, i);
			return false;
		}
	}

	if (!loadChunk(0, _frameDecodeBuffer, kFrameBufferSize) ||
			!loadChunk(1, _paletteResource, kPaletteSize))
		return false;

	uint32 resourceArenaSize = 0;
	for (uint i = 2; i <= 4; ++i)
		resourceArenaSize += _chunkTable.sizes[i];

	_resourceArena.resize(resourceArenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	for (uint i = 2; i <= 4; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	return true;
}

bool Scene1000::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kTitleFrontEndArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kTitleFrontEndArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed destination", kTitleFrontEndArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kTitleFrontEndArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u",
		kTitleFrontEndArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene1000::loadArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kTitleFrontEndArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kTitleFrontEndArchiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the title front-end resource arena",
			kTitleFrontEndArchiveName, index);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kTitleFrontEndArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		kTitleFrontEndArchiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

void Scene1000::runTitleFrontEndSequence() {
	uint32 elapsed = 0;
	uint32 secondaryElapsed = 0;
	uint32 lastMillis = g_system->getMillis();

	while (elapsed < kTitleFrontEndDurationMillis && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		elapsed += delta;

		if (!_titlePatchApplied && elapsed >= kTitlePatchTimeMillis)
			applyTitlePatch();

		if (!_secondaryVisible && elapsed >= kSecondaryStartTimeMillis) {
			_secondaryVisible = true;
			_secondaryDirty = true;
		}

		if (_secondaryVisible && _secondaryFrameIndex < kSecondaryDescriptorCount - 1) {
			secondaryElapsed += delta;
			while (secondaryElapsed >= kSecondaryFrameMillis && _secondaryFrameIndex < kSecondaryDescriptorCount - 1) {
				secondaryElapsed -= kSecondaryFrameMillis;
				++_secondaryFrameIndex;
				_secondaryDirty = true;
			}
		}

		if (now - _lastBlinkMillis >= kBlinkFrameMillis) {
			_lastBlinkMillis = now;
			advanceBlinkFrame();
		}

		renderOverlayFrame(false);
		g_system->delayMillis(10);
	}
}

void Scene1000::applyTitlePatch() {
	drawResourceBlockListToSceneFramebuffer(_resourceChunkOffsets[2]);
	_titlePatchApplied = true;
	renderOverlayFrame(true);
}

void Scene1000::advanceBlinkFrame() {
	if (_blinkPatternMode != 0) {
		if (_blinkPatternMode == 1 && _blinkFrameIndex > 5) {
			_blinkPatternMode = 0;
		} else if (_blinkPatternMode == 2 && _blinkFrameIndex > 0x1e) {
			_blinkPatternMode = 0;
		} else {
			++_blinkFrameIndex;
			_blinkDirty = true;
			return;
		}
	}

	if (_blinkFrameIndex != 0) {
		_blinkFrameIndex = 0;
		_blinkDirty = true;
		return;
	}

	if (_random.getRandomNumber(14) == 0) {
		_blinkFrameIndex = 1;
		_blinkDirty = true;
		return;
	}

	if (_random.getRandomNumber(19) == 0) {
		_blinkFrameIndex = _random.getRandomBit() ? 7 : 2;
		_blinkPatternMode = _blinkFrameIndex == 7 ? 2 : 1;
	}

	_blinkDirty = true;
}

void Scene1000::renderOverlayFrame(bool forceDirty) {
	if (!forceDirty && !_blinkDirty && !_secondaryDirty)
		return;

	if (forceDirty) {
		_blinkDirty = true;
		if (_secondaryVisible)
			_secondaryDirty = true;
	}

	if (_blinkDirty) {
		const uint blinkMapIndex = MIN<uint>(_blinkFrameIndex, ARRAYSIZE(kTitleBlinkDescriptorFrameMap) - 1);
		restoreSpriteBackground(_resourceChunkOffsets[3], kBlinkDescriptorCount,
			kTitleBlinkDescriptorFrameMap[blinkMapIndex]);
	}
	if (_secondaryDirty && _secondaryVisible)
		restoreSpriteBackground(_resourceChunkOffsets[4], kSecondaryDescriptorCount, _secondaryFrameIndex);

	const uint blinkMapIndex = MIN<uint>(_blinkFrameIndex, ARRAYSIZE(kTitleBlinkDescriptorFrameMap) - 1);
	drawStripSpriteFrame(_resourceChunkOffsets[3], kBlinkDescriptorCount,
		kTitleBlinkDescriptorFrameMap[blinkMapIndex]);
	if (_secondaryVisible)
		drawStripSpriteFrame(_resourceChunkOffsets[4], kSecondaryDescriptorCount, _secondaryFrameIndex);

	_blinkDirty = false;
	_secondaryDirty = false;
	presentFrame();
}

void Scene1000::restoreSpriteBackground(uint32 baseOffset, uint16 descriptorCount, uint16 descriptorIndex) {
	if (descriptorIndex >= descriptorCount)
		return;

	const uint entryOffset = baseOffset + kFrameDescriptorSize * descriptorIndex;
	if (entryOffset + kFrameDescriptorSize > _resourceArena.size())
		return;

	const uint32 packedWidth = readUint32(_resourceArena, entryOffset + 4);
	const uint32 packedRows = readUint32(_resourceArena, entryOffset + 8);
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

void Scene1000::drawStripSpriteFrame(uint32 baseOffset, uint16 descriptorCount, uint16 descriptorIndex) {
	if (descriptorIndex >= descriptorCount)
		return;

	const uint entryOffset = baseOffset + kFrameDescriptorSize * descriptorIndex;
	if (entryOffset + kFrameDescriptorSize > _resourceArena.size())
		return;

	const uint16 spanCount = readUint16(_resourceArena, entryOffset + 12);
	uint cursor = baseOffset + kFrameDescriptorSize * descriptorCount + readUint32(_resourceArena, entryOffset);
	if (cursor > _resourceArena.size())
		return;

	for (uint spanIndex = 0; spanIndex < spanCount; ++spanIndex) {
		if (cursor + 5 > _resourceArena.size())
			return;

		const uint32 packedDestination = readUint32(_resourceArena, cursor);
		const uint dataLength = _resourceArena[cursor + 4];
		cursor += 5;

		const uint x = packedDestination & 0xffff;
		const uint y = (packedDestination >> 16) & 0xffff;
		const uint destinationOffset = x + y * HollywoodEngine::kSceneBufferWidth;
		if (cursor + dataLength > _resourceArena.size() ||
				destinationOffset + dataLength > _sceneFramebuffer.size())
			return;

		memcpy(_sceneFramebuffer.data() + destinationOffset, _resourceArena.data() + cursor, dataLength);
		cursor += dataLength;
	}
}

void Scene1000::drawResourceBlockListToSceneFramebuffer(uint32 baseOffset) {
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
		const uint destinationOffset = x + y * HollywoodEngine::kSceneBufferWidth;
		if (cursor + size > _resourceArena.size() ||
				destinationOffset + size > _sceneFramebuffer.size())
			return;

		memcpy(_sceneFramebuffer.data() + destinationOffset, _resourceArena.data() + cursor, size);
		cursor += size;
	}
}

void Scene1000::fadeInPalette() {
	byte threshold = 0x3f;
	while (!_skipRequested && !Engine::shouldQuit()) {
		for (uint i = 0; i < _paletteResource.size(); ++i) {
			if (_paletteResource[i] >= threshold)
				_paletteCurrent[i] = MIN<byte>(_paletteResource[i], _paletteCurrent[i] + 3);
		}
		presentFrame();
		if (threshold == 0)
			return;
		threshold = threshold > 3 ? threshold - 3 : 0;
		if (delay(20))
			return;
	}
}

void Scene1000::fadeOutPalette() {
	byte threshold = 0;
	while (!_skipRequested && !Engine::shouldQuit()) {
		for (uint i = 0; i < _paletteResource.size(); ++i) {
			if (_paletteResource[i] >= threshold)
				_paletteCurrent[i] = _paletteCurrent[i] >= 3 ? _paletteCurrent[i] - 3 : 0;
		}
		presentFrame();
		if (threshold >= 0x3f)
			return;
		threshold = MIN<byte>(0x3f, threshold + 3);
		if (delay(20))
			return;
	}
}

void Scene1000::presentFrame() {
	byte palette[kPaletteSize];
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

bool Scene1000::pollEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			_music.stop();
			return true;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
					event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_SPACE) {
				_skipRequested = true;
				return true;
			}
			break;
		default:
			break;
		}
	}

	return false;
}

bool Scene1000::delay(uint32 millis) {
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

uint16 Scene1000::readUint16(const Common::Array<byte> &source, uint offset) const {
	if (offset + 2 > source.size())
		return 0;

	return source[offset] | (source[offset + 1] << 8);
}

uint32 Scene1000::readUint32(const Common::Array<byte> &source, uint offset) const {
	if (offset + 4 > source.size())
		return 0;

	return source[offset] |
		(source[offset + 1] << 8) |
		(source[offset + 2] << 16) |
		(source[offset + 3] << 24);
}

} // End of namespace Hollywood
