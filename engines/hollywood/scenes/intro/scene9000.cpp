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

#include "hollywood/scenes/intro/scene9000.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kIntroArchiveName = "RESOURCE.I00";

const byte kIntroAnimationDescriptorSequence[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20
};

Scene9000::Scene9000(HollywoodEngine *vm) :
		_vm(vm),
		_music(),
		_skipRequested(false) {
	_paletteSource.resize(0x300);
	_paletteCurrent.resize(0x300);
	_frameDecodeBuffer.resize(HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight);
	_sceneFramebuffer.resize(HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight);
	_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
}

bool Scene9000::play() {
	if (!load())
		return false;

	_music.playIntroMusic();

	for (uint chunkIndex = 0; chunkIndex < kIntroChunkCount && !_skipRequested && !Engine::shouldQuit(); ++chunkIndex) {
		if (!loadChunk(chunkIndex)) {
			_music.stop();
			return false;
		}
		if (!runChunk()) {
			_music.stop();
			return false;
		}
	}

	while (_music.isPlaying() && !_skipRequested && !Engine::shouldQuit()) {
		if (delay(10))
			break;
	}

	_music.stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	return true;
}

bool Scene9000::load() {
	if (!_vm->resources()->readChunkTable(Common::Path(kIntroArchiveName), _chunkTable)) {
		warning("Failed to read %s header", kIntroArchiveName);
		return false;
	}

	for (uint i = 0; i < kIntroChunkCount * 2; ++i) {
		if (!_chunkTable.isValidChunk(i)) {
			warning("%s is missing intro chunk %u", kIntroArchiveName, i);
			return false;
		}
	}

	return true;
}

bool Scene9000::loadChunk(uint chunkIndex) {
	const uint paletteChunk = chunkIndex * 2;
	const uint spriteChunk = paletteChunk + 1;

	Common::ScopedPtr<Common::SeekableReadStream> paletteStream(_vm->resources()->createChunkReadStream(Common::Path(kIntroArchiveName), paletteChunk));
	if (!paletteStream || paletteStream->size() > _paletteSource.size()) {
		warning("Failed to open %s palette chunk %u", kIntroArchiveName, paletteChunk);
		return false;
	}

	memset(_paletteSource.data(), 0, _paletteSource.size());
	if (paletteStream->read(_paletteSource.data(), paletteStream->size()) != (uint32)paletteStream->size()) {
		warning("Failed to read %s palette chunk %u", kIntroArchiveName, paletteChunk);
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> spriteStream(_vm->resources()->createChunkReadStream(Common::Path(kIntroArchiveName), spriteChunk));
	if (!spriteStream) {
		warning("Failed to open %s sprite chunk %u", kIntroArchiveName, spriteChunk);
		return false;
	}

	_resourceArena.resize(spriteStream->size());
	if (spriteStream->read(_resourceArena.data(), _resourceArena.size()) != _resourceArena.size()) {
		warning("Failed to read %s sprite chunk %u", kIntroArchiveName, spriteChunk);
		return false;
	}

	debugC(1, kDebugResources, "Loaded intro chunk %u: palette=%u bytes sprite=%u bytes",
		chunkIndex, (uint)paletteStream->size(), (uint)_resourceArena.size());
	resetChunkState();
	return true;
}

bool Scene9000::runChunk() {
	byte frameIndex = 0;
	byte fadeThreshold = 63;
	bool fadeInComplete = false;
	bool fadeOutComplete = false;

	presentFrame();

	while ((frameIndex < 0x15 || !fadeInComplete) && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;

		if (frameIndex < 0x15) {
			frameIndex++;
			const byte descriptorIndex = kIntroAnimationDescriptorSequence[frameIndex];
			restoreSpriteBackground(descriptorIndex);
			drawStripSpriteFrame(descriptorIndex);
		}

		if (!fadeInComplete) {
			for (uint i = 0; i < kAnimatedPaletteByteCount; ++i) {
				if (_paletteSource[i] >= fadeThreshold)
					_paletteCurrent[i] = MIN<byte>(_paletteSource[i], _paletteCurrent[i] + 3);
			}

			if (fadeThreshold == 0)
				fadeInComplete = true;
			else
				fadeThreshold = fadeThreshold > 3 ? fadeThreshold - 3 : 0;
		}

		presentFrame();
		if (delay(kFrameStepMillis))
			return true;
	}

	for (uint holdStep = 0; holdStep < kHoldStepCount && !_skipRequested && !Engine::shouldQuit(); ++holdStep) {
		if (delay(kHoldStepMillis))
			return true;
	}

	fadeThreshold = 0;
	while ((frameIndex > 1 || !fadeOutComplete) && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;

		if (frameIndex > 1) {
			frameIndex--;
			const byte descriptorIndex = kIntroAnimationDescriptorSequence[frameIndex];
			restoreSpriteBackground(descriptorIndex);
			drawStripSpriteFrame(descriptorIndex);
		}

		if (!fadeOutComplete) {
			for (uint i = 0; i < kAnimatedPaletteByteCount; ++i) {
				if (_paletteSource[i] >= fadeThreshold)
					_paletteCurrent[i] = _paletteCurrent[i] >= 3 ? _paletteCurrent[i] - 3 : 0;
			}

			if (fadeThreshold == 63)
				fadeOutComplete = true;
			else
				fadeThreshold = MIN<byte>(63, fadeThreshold + 3);
		}

		presentFrame();
		if (delay(kPaletteStepMillis))
			return true;
	}

	return true;
}

bool Scene9000::pollEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
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

bool Scene9000::delay(uint32 millis) {
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

void Scene9000::resetChunkState() {
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memset(_frameDecodeBuffer.data(), 0, _frameDecodeBuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
}

void Scene9000::presentFrame() {
	presentIndexedFrame(_sceneFramebuffer.surface(), _paletteCurrent, _screen, _displayPalette);
}

void Scene9000::restoreSpriteBackground(uint16 descriptorIndex) {
	if (descriptorIndex >= kIntroFrameDescriptorCount)
		return;

	const uint entryOffset = kIntroFrameDescriptorSize * descriptorIndex;
	if (entryOffset + kIntroFrameDescriptorSize > _resourceArena.size())
		return;

	const uint32 packedWidth = readUint32(entryOffset + 4);
	const uint32 packedRows = readUint32(entryOffset + 8);
	const uint copyWidth = (packedWidth >> 16) & 0xffff;
	const uint x = packedWidth & 0xffff;
	const uint firstRow = packedRows & 0xffff;
	const uint lastRow = (packedRows >> 16) & 0xffff;

	if (firstRow > lastRow || copyWidth == 0)
		return;

	for (uint row = firstRow; row <= lastRow && row < HollywoodEngine::kSceneBufferHeight; ++row) {
		const uint destinationOffset = x + row * HollywoodEngine::kSceneBufferWidth;
		if (destinationOffset + copyWidth > _sceneFramebuffer.size() ||
				destinationOffset + copyWidth > _frameDecodeBuffer.size())
			break;

		memcpy(&_sceneFramebuffer[destinationOffset], &_frameDecodeBuffer[destinationOffset], copyWidth);
	}
}

void Scene9000::drawStripSpriteFrame(uint16 descriptorIndex) {
	if (descriptorIndex >= kIntroFrameDescriptorCount)
		return;

	const uint entryOffset = kIntroFrameDescriptorSize * descriptorIndex;
	if (entryOffset + kIntroFrameDescriptorSize > _resourceArena.size())
		return;

	const uint16 spanCount = readUint16(entryOffset + 12);
	uint cursor = kIntroFrameDescriptorSize * kIntroFrameDescriptorCount + readUint32(entryOffset);
	if (cursor > _resourceArena.size())
		return;

	for (uint spanIndex = 0; spanIndex < spanCount; ++spanIndex) {
		if (cursor + 5 > _resourceArena.size())
			return;

		const uint32 destination = readUint32(cursor);
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

uint16 Scene9000::readUint16(uint offset) const {
	if (offset + 2 > _resourceArena.size())
		return 0;

	return _resourceArena[offset] | (_resourceArena[offset + 1] << 8);
}

uint32 Scene9000::readUint32(uint offset) const {
	if (offset + 4 > _resourceArena.size())
		return 0;

	return _resourceArena[offset] |
		(_resourceArena[offset + 1] << 8) |
		(_resourceArena[offset + 2] << 16) |
		(_resourceArena[offset + 3] << 24);
}

} // End of namespace Hollywood
