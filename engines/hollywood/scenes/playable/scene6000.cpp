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

#include "hollywood/scenes/playable/scene6000.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene6000ArchiveName = "RESOURCE.F00";
const char *const kScene6000MusicArchiveName = "RESOURCE.M06";
const uint16 kScene6000MusicCueId = 0x000b;
const uint16 kScene6000NextState = 0x177a;
const uint32 kScene6000FrameTickMillis = 75;
const uint32 kScene6000SpriteTickMillis = 1000;
const uint kScene6000SpriteDescriptorCount = 0x2b;
const uint kScene6000EndTick = 0x32;

const byte kScene6000SpriteFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42
};

Scene6000::Scene6000(HollywoodEngine *vm) :
		_vm(vm),
		_music(),
		_resourceArenaCursor(0),
		_spriteFrameIndex(0),
		_previousSpriteDescriptor(0),
		_hasPreviousSpriteDescriptor(false),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
}

bool Scene6000::play() {
	GameplayState &state = _vm->gameState();
	state.initializeRonItemResourcePages();
	if (state.inventoryItemCountByOwner[0] == 0)
		state.initializeRonInventoryItems();
	state.currentInventoryOwnerIndex = 0;
	state.activeAudioChapterIndex = 6;
	state.currentAmbientMusicCueId = kScene6000MusicCueId;

	if (!load())
		return false;

	_music.setArchive(Common::Path(kScene6000MusicArchiveName));
	_music.playMusicCue(kScene6000MusicCueId, 100);
	fadeInPalette();

	if (!_skipRequested && !Engine::shouldQuit())
		runPresentation();

	fadeOutPalette();
	_music.stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene6000NextState;

	return true;
}

bool Scene6000::load() {
	if (!_vm->resources()->readChunkTable(Common::Path(kScene6000ArchiveName), _chunkTable)) {
		warning("Failed to read %s header", kScene6000ArchiveName);
		return false;
	}

	for (uint i = 0; i <= 3; ++i) {
		if (!_chunkTable.isValidChunk(i)) {
			warning("%s is missing scene 6000 chunk %u", kScene6000ArchiveName, i);
			return false;
		}
	}

	if (!loadChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadChunk(1, _paletteResource, kPaletteSize))
		return false;

	uint32 resourceArenaSize = 0;
	for (uint i = 2; i <= 3; ++i)
		resourceArenaSize += _chunkTable.sizes[i];

	_resourceArena.resize(resourceArenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	for (uint i = 2; i <= 3; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	if (_paletteResource.size() >= 0x300) {
		_paletteResource[0x2fd] = 0x3f;
		_paletteResource[0x2fe] = 0x3f;
		_paletteResource[0x2ff] = 0x3f;
	}
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	return true;
}

bool Scene6000::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_vm->resources()->createChunkReadStream(Common::Path(kScene6000ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kScene6000ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit scene 6000 destination", kScene6000ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kScene6000ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u",
		kScene6000ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene6000::loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_vm->resources()->createChunkReadStream(Common::Path(kScene6000ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kScene6000ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit scene 6000 destination", kScene6000ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kScene6000ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u",
		kScene6000ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene6000::loadArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_vm->resources()->createChunkReadStream(Common::Path(kScene6000ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kScene6000ArchiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the scene 6000 resource arena", kScene6000ArchiveName, index);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kScene6000ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		kScene6000ArchiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

void Scene6000::runPresentation() {
	uint tick = 0;
	uint32 frameAccumulator = 0;
	uint32 spriteAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	bool spriteVisible = false;
	bool spriteDirty = true;

	while (tick < kScene6000EndTick && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		spriteAccumulator += delta;

		while (frameAccumulator >= kScene6000FrameTickMillis && tick < kScene6000EndTick) {
			frameAccumulator -= kScene6000FrameTickMillis;
			++tick;
			if (tick == 3)
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2], _sceneFramebuffer.managedSurface());
			if (tick == 5) {
				spriteVisible = true;
				spriteDirty = true;
			}
		}

		while (spriteVisible && spriteAccumulator >= kScene6000SpriteTickMillis) {
			spriteAccumulator -= kScene6000SpriteTickMillis;
			if (_spriteFrameIndex + 1 < ARRAYSIZE(kScene6000SpriteFrameMap)) {
				++_spriteFrameIndex;
				spriteDirty = true;
			}
		}

		if (spriteDirty) {
			drawAnimatedSpriteFrame(spriteVisible);
			spriteDirty = false;
		}
		g_system->delayMillis(10);
	}
}

void Scene6000::drawAnimatedSpriteFrame(bool drawSprite) {
	if (_hasPreviousSpriteDescriptor) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[3], 0,
			kScene6000SpriteDescriptorCount, _previousSpriteDescriptor,
			_baseFramebuffer.surface(), _sceneFramebuffer.surface());
	}

	const uint mapIndex = MIN<uint>(_spriteFrameIndex, ARRAYSIZE(kScene6000SpriteFrameMap) - 1);
	const uint16 descriptor = kScene6000SpriteFrameMap[mapIndex];
	restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[3], 0,
		kScene6000SpriteDescriptorCount, descriptor,
		_baseFramebuffer.surface(), _sceneFramebuffer.surface());

	if (drawSprite)
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[3], 0,
			kScene6000SpriteDescriptorCount, descriptor, _sceneFramebuffer.surface());

	_previousSpriteDescriptor = descriptor;
	_hasPreviousSpriteDescriptor = true;
	presentFrame();
}

void Scene6000::fadeInPalette() {
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

void Scene6000::fadeOutPalette() {
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

void Scene6000::presentFrame() {
	presentIndexedFrame(_sceneFramebuffer.surface(), _paletteCurrent, _screen, _displayPalette);
}

bool Scene6000::delay(uint32 millis) {
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

bool Scene6000::pollEvents() {
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

} // End of namespace Hollywood
