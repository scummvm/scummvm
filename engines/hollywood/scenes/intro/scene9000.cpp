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

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kIntroArchiveName = "RESOURCE.I00";

const byte kIntroAnimationDescriptorSequence[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20
};

Scene9000::Scene9000(HollywoodEngine *vm) :
		PresentationScene(vm, "intro scene 9000", kFrameBufferSize, 0),
		_music() {
	_paletteSource.resize(0x300);
	_frameDecodeBuffer.resize(HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight);
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
			drawAnimationFrame(descriptorIndex);
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
			drawAnimationFrame(descriptorIndex);
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

void Scene9000::stopAudio() {
	_music.stop();
}

void Scene9000::resetChunkState() {
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memset(_frameDecodeBuffer.data(), 0, _frameDecodeBuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
}

void Scene9000::drawAnimationFrame(uint16 descriptorIndex) {
	restoreSpriteBackground(_resourceArena, 0, 0, kIntroFrameDescriptorCount,
		descriptorIndex, _frameDecodeBuffer.surface(), _sceneFramebuffer.surface());
	drawStripSpriteFrame(_resourceArena, 0, 0, kIntroFrameDescriptorCount,
		descriptorIndex, _sceneFramebuffer.surface());
}

} // End of namespace Hollywood
