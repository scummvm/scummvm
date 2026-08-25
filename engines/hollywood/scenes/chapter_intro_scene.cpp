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

#include "hollywood/scenes/chapter_intro_scene.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

ChapterIntroScene::ChapterIntroScene(HollywoodEngine *vm, const char *debugName) :
		_vm(vm),
		_music(),
		_resourceArenaCursor(0),
		_debugName(debugName),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
}

bool ChapterIntroScene::play() {
	initializeChapterState();

	if (!load())
		return false;

	_music.setArchive(Common::Path(musicArchiveName()));
	_music.playMusicCue(musicCueId(), 100);
	drawInitialFrame();
	fadeInPalette();

	if (!_skipRequested && !Engine::shouldQuit())
		runPresentation();

	// Fade the palette currently on screen, including cycled entries.
	memcpy(_paletteResource.data(), _paletteCurrent.data(), _paletteResource.size());
	fadeOutPalette();
	_music.stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = nextState();

	return true;
}

uint16 ChapterIntroScene::sceneViewportXOffset() const {
	return 0;
}

void ChapterIntroScene::initializeChapterState() {
	GameplayState &state = _vm->gameState();
	state.initializeRonItemResourcePages();
	if (!state.ronInventoryInitialized)
		state.initializeRonInventoryItems();
	state.currentInventoryOwnerIndex = 0;
	state.activeAudioChapterIndex = activeAudioChapterIndex();
	state.currentAmbientMusicCueId = musicCueId();
}

void ChapterIntroScene::adjustPaletteAfterLoad() {
}

void ChapterIntroScene::drawInitialFrame() {
}

bool ChapterIntroScene::load() {
	if (!_vm->resources()->readChunkTable(Common::Path(resourceArchiveName()), _chunkTable)) {
		warning("Failed to read %s header", resourceArchiveName());
		return false;
	}

	for (uint i = 0; i <= sceneArenaLastChunk(); ++i) {
		if (!_chunkTable.isValidChunk(i)) {
			warning("%s is missing %s chunk %u", resourceArchiveName(), _debugName, i);
			return false;
		}
	}

	if (!loadChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadChunk(1, _paletteResource, kPaletteSize))
		return false;

	uint32 resourceArenaSize = 0;
	for (uint i = sceneArenaFirstChunk(); i <= sceneArenaLastChunk(); ++i)
		resourceArenaSize += _chunkTable.sizes[i];

	_resourceArena.resize(resourceArenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	for (uint i = sceneArenaFirstChunk(); i <= sceneArenaLastChunk(); ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	adjustPaletteAfterLoad();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	return true;
}

bool ChapterIntroScene::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_vm->resources()->createChunkReadStream(Common::Path(resourceArchiveName()), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", resourceArchiveName(), index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit %s destination", resourceArchiveName(), index, _debugName);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", resourceArchiveName(), index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u",
		resourceArchiveName(), index, (uint)stream->size());
	return true;
}

bool ChapterIntroScene::loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_vm->resources()->createChunkReadStream(Common::Path(resourceArchiveName()), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", resourceArchiveName(), index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit %s destination", resourceArchiveName(), index, _debugName);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", resourceArchiveName(), index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u",
		resourceArchiveName(), index, (uint)stream->size());
	return true;
}

bool ChapterIntroScene::loadArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_vm->resources()->createChunkReadStream(Common::Path(resourceArchiveName()), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", resourceArchiveName(), index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the %s resource arena", resourceArchiveName(), index, _debugName);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", resourceArchiveName(), index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		resourceArchiveName(), index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

void ChapterIntroScene::fadeInPalette() {
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

void ChapterIntroScene::fadeOutPalette() {
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

void ChapterIntroScene::presentFrame() {
	presentIndexedFrame(_sceneFramebuffer.surface(), _paletteCurrent, _screen, _displayPalette, 0, sceneViewportXOffset());
}

void ChapterIntroScene::rotatePaletteRange(uint firstIndex, uint lastIndex) {
	if (firstIndex >= lastIndex || lastIndex >= _paletteCurrent.size() / 3)
		return;

	byte lastColor[3];
	memcpy(lastColor, _paletteCurrent.data() + lastIndex * 3, sizeof(lastColor));
	for (uint index = lastIndex; index > firstIndex; --index) {
		memcpy(_paletteCurrent.data() + index * 3,
			_paletteCurrent.data() + (index - 1) * 3, sizeof(lastColor));
	}
	memcpy(_paletteCurrent.data() + firstIndex * 3, lastColor, sizeof(lastColor));
	_displayPalette.uploadFrom6Bit(_paletteCurrent);
}

bool ChapterIntroScene::delay(uint32 millis) {
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

bool ChapterIntroScene::pollEvents() {
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

void ChapterIntroScene::drawClipFrameDelta(uint chunkIndex, uint tableEntryCount, byte frameIndex) {
	if (!_chunkTable.isValidChunk(chunkIndex))
		return;

	ResourceDeltaClipPlayer::drawFrame(_resourceArena, _resourceChunkOffsets[chunkIndex],
		_chunkTable.sizes[chunkIndex], tableEntryCount, frameIndex, _sceneFramebuffer.data(),
		_sceneFramebuffer.size());
}

} // End of namespace Hollywood
