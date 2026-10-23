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

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/scenes/chapter_intro_scene.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

ChapterIntroScene::ChapterIntroScene(HollywoodEngine *vm, const char *debugName) :
		PresentationScene(vm, debugName, kSceneBufferByteCount, 0),
		_music(),
		_paletteResource(),
		_baseFramebuffer() {
	_paletteResource.resize(kPaletteSize);
	_baseFramebuffer.resize(kSceneBufferByteCount);
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

uint ChapterIntroScene::presentXOffset() const {
	return sceneViewportXOffset();
}

void ChapterIntroScene::stopAudio() {
	_music.stop();
}

void ChapterIntroScene::initializeChapterState() {
	GameplayState &state = _vm->gameState();
	state.initializeRonItemResourcePages();
	if (!state.ronInventoryInitialized)
		state.initializeRonInventoryItems(_vm->isDemo());
	state.currentInventoryOwnerIndex = 0;
	state.activeAudioChapterIndex = activeAudioChapterIndex();
	state.currentAmbientMusicCueId = musicCueId();
}

void ChapterIntroScene::adjustPaletteAfterLoad() {
}

void ChapterIntroScene::drawInitialFrame() {
}

bool ChapterIntroScene::load() {
	if (!_resources.loadChunkTable(resourceArchiveName()))
		return false;

	if (!_resources.validateChunkRange(resourceArchiveName(), _debugName, 0,
			sceneArenaLastChunk()))
		return false;

	if (!_resources.loadFixedChunk(_debugName, 0, _baseFramebuffer, kSceneBufferByteCount) ||
			!_resources.loadFixedChunk(_debugName, 1, _paletteResource, kPaletteSize))
		return false;

	_resources.allocateArena(_resources.totalChunkSize(sceneArenaFirstChunk(),
		sceneArenaLastChunk()));
	for (uint i = sceneArenaFirstChunk(); i <= sceneArenaLastChunk(); ++i) {
		if (!_resources.loadArenaChunk(_debugName, i))
			return false;
	}

	adjustPaletteAfterLoad();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
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

void ChapterIntroScene::drawClipFrameDelta(uint chunkIndex, uint tableEntryCount, byte frameIndex) {
	if (!_resources._chunkTable.isValidChunk(chunkIndex))
		return;

	drawResourceDeltaClipFrame(_resources._arena, _resources._chunkOffsets[chunkIndex],
		_resources._chunkTable.sizes[chunkIndex], tableEntryCount, frameIndex, _sceneFramebuffer.data(),
		_sceneFramebuffer.size());
}

} // End of namespace Hollywood
