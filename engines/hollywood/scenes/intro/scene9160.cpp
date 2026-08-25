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

#include "hollywood/scenes/intro/scene9160.h"

#include "common/debug.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene9160ArchiveName = "RESOURCE.I16";
const uint16 kScene9160NextState = 0x23dc;
const uint16 kScene9160ExitState = 0x00ff;
const uint kScene9160TallFramebufferSize = 0x100000;
const uint kScene9160PanelWidth = 640;
const uint kScene9160PanelHeight = 0x200;
const uint kScene9160PanelSize = kScene9160PanelWidth * kScene9160PanelHeight;
const uint kScene9160FirstOverlayChunk = 3;
const uint kScene9160LastOverlayChunk = 24;
const uint kScene9160WaitMillis = 8000;
const uint kScene9160ScrollFrameMillis = 50;

const byte kScene9160ScrollDeltaByStep[] = {
	4, 8, 12, 16, 20, 24, 28, 32, 36, 48, 56, 48, 36, 32, 28, 24, 20, 16, 12, 8, 4
};

Scene9160::Scene9160(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9160", kScene9160TallFramebufferSize, kFrameBufferSize),
		_resources(),
		_music(vm->introMusic()),
		_paletteResource(),
		_panelA(),
		_panelB(),
		_rowOffset(0) {
	_paletteResource.resize(kPaletteSize);
}

bool Scene9160::play() {
	if (!load())
		return false;

	_music->setVolume(100);
	buildInitialFrame();
	_rowOffset = 0;
	presentFrame();
	fadeInPalette();

	uint currentChunk = 4;
	while (currentChunk <= kScene9160LastOverlayChunk && !_skipRequested && !Engine::shouldQuit()) {
		if (waitBeforeScroll())
			break;
		scrollCreditsPanel();
		if (_skipRequested || Engine::shouldQuit())
			break;

		prepareNextPair(currentChunk);
		if (currentChunk < kScene9160LastOverlayChunk)
			++currentChunk;
		else
			break;
	}

	if (!_skipRequested && !Engine::shouldQuit())
		waitForMusicEnd();

	fadeOutPalette();
	_music->stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = _skipRequested ? kScene9160ExitState : kScene9160NextState;

	return true;
}

bool Scene9160::load() {
	if (!_resources.loadChunkTable(kScene9160ArchiveName))
		return false;

	if (!_resources.validateChunkRange(kScene9160ArchiveName, _debugName, 0, kScene9160LastOverlayChunk))
		return false;

	if (!loadVariableChunk(0, _panelA) ||
			!loadVariableChunk(1, _panelB) ||
			!loadChunk(2, _paletteResource, kPaletteSize))
		return false;

	sanitizePanel(_panelA);
	sanitizePanel(_panelB);

	_resources.allocateArena(_resources.totalChunkSize(kScene9160FirstOverlayChunk, kScene9160LastOverlayChunk));
	for (uint i = kScene9160FirstOverlayChunk; i <= kScene9160LastOverlayChunk; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	return true;
}

bool Scene9160::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	if (!_resources.loadVariableChunk(index, destination))
		return false;

	if (destination.size() < kScene9160PanelSize) {
		warning("%s chunk %u is too small for a 640x512 panel", _debugName, index);
		return false;
	}

	return true;
}

bool Scene9160::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9160::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index, index);
}

void Scene9160::sanitizePanel(Common::Array<byte> &panel) {
	const uint count = MIN<uint>(panel.size(), kScene9160PanelSize);
	for (uint i = 0; i < count; ++i) {
		if (panel[i] == 0xff)
			panel[i] = 0x74;
	}
}

void Scene9160::copyPanelToFramebuffer(const Common::Array<byte> &panel, int yOffset) {
	for (uint row = 0; row < kScene9160PanelHeight; ++row) {
		const int destinationRow = yOffset + row;
		if (destinationRow < 0 || destinationRow >= _sceneFramebuffer.surface().h)
			continue;

		const uint sourceOffset = row * kScene9160PanelWidth;
		const uint destinationOffset = destinationRow * HollywoodEngine::kSceneBufferWidth;
		memcpy(_sceneFramebuffer.data() + destinationOffset, panel.data() + sourceOffset, kScene9160PanelWidth);
	}
}

void Scene9160::drawOverlayChunk(uint chunkIndex, int yOffset) {
	if (chunkIndex < kScene9160FirstOverlayChunk || chunkIndex > kScene9160LastOverlayChunk)
		return;

	drawResourceBlockList(_resources.arena, _resources.chunkOffsets[chunkIndex],
		_sceneFramebuffer.surface(), yOffset);
}

void Scene9160::buildInitialFrame() {
	_sceneFramebuffer.clear(0);
	copyPanelToFramebuffer(_panelA, 0);
	drawOverlayChunk(3, 0);
	copyPanelToFramebuffer(_panelB, kScene9160PanelHeight);
	drawOverlayChunk(4, kScene9160PanelHeight);
}

void Scene9160::prepareNextPair(uint currentChunk) {
	const bool currentIsEven = (currentChunk & 1) == 0;
	_rowOffset = 0;
	copyPanelToFramebuffer(currentIsEven ? _panelB : _panelA, 0);
	drawOverlayChunk(currentChunk, 0);

	const uint nextChunk = currentChunk + 1;
	if (nextChunk <= kScene9160LastOverlayChunk) {
		copyPanelToFramebuffer(currentIsEven ? _panelA : _panelB, kScene9160PanelHeight);
		drawOverlayChunk(nextChunk, kScene9160PanelHeight);
	}
	presentFrame();
}

void Scene9160::scrollCreditsPanel() {
	for (uint i = 0; i < ARRAYSIZE(kScene9160ScrollDeltaByStep) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		if (pollEvents())
			return;
		_rowOffset += kScene9160ScrollDeltaByStep[i];
		presentFrame();
		if (delay(kScene9160ScrollFrameMillis))
			return;
	}
}

void Scene9160::fadeInPalette() {
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	for (int threshold = 0x3f; threshold > 0 && !_skipRequested && !Engine::shouldQuit(); --threshold) {
		for (uint color = 0; color < 256; ++color) {
			for (uint component = 0; component < 3; ++component) {
				const uint offset = color * 3 + component;
				if (_paletteResource[offset] >= threshold && _paletteCurrent[offset] < _paletteResource[offset])
					++_paletteCurrent[offset];
			}
		}
		presentFrame();
		if (delay(10))
			return;
	}
	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	presentFrame();
}

void Scene9160::fadeOutPalette() {
	for (byte threshold = 1; threshold < 0x40 && !_skipRequested && !Engine::shouldQuit(); ++threshold) {
		for (uint i = 0; i < _paletteResource.size(); ++i) {
			if (_paletteResource[i] >= threshold)
				_paletteCurrent[i] = _paletteCurrent[i] == 0 ? 0 : _paletteCurrent[i] - 1;
		}
		presentFrame();
		if (delay(10))
			return;
	}
}

bool Scene9160::waitBeforeScroll() {
	uint32 elapsed = 0;
	while (elapsed < kScene9160WaitMillis && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;
		presentFrame();
		const uint32 slice = MIN<uint32>(kScene9160WaitMillis - elapsed, 50);
		g_system->delayMillis(slice);
		elapsed += slice;
	}

	return _skipRequested || Engine::shouldQuit();
}

bool Scene9160::waitForMusicEnd() {
	while (_music->isPlaying() && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;
		presentFrame();
		g_system->delayMillis(50);
	}

	return _skipRequested || Engine::shouldQuit();
}

uint Scene9160::presentRowOffset() const {
	return _rowOffset;
}

void Scene9160::stopAudio() {
	_music->stop();
}

} // End of namespace Hollywood
