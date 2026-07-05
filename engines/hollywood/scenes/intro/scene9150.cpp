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

#include "hollywood/scenes/intro/scene9150.h"

#include "common/debug.h"
#include "common/path.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const char *const kScene9150ArchiveName = "RESOURCE.I15";
const char *const kScene9150TextArchiveName = "RESOURCE.003";
const uint16 kScene9150NextState = 0x2382;
const uint kScene9150DescriptorCount = 0x1f;
const uint kScene9150FrameIntervalMillis = 30;
const uint kScene9150StaticSpeechTableOffset = 0x5f58;

struct Scene9150ClipStep {
	byte chunkIndex;
	byte staticSpeechRowIndex;
};

const Scene9150ClipStep kScene9150ClipSteps[] = {
	{ 3, 0xe0 },
	{ 1, 0xe1 },
	{ 4, 0xdf },
	{ 2, 0xe2 }
};

Scene9150::Scene9150(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9150"),
		_speech(),
		_text(),
		_paletteResource(),
		_clipResource() {
	_paletteResource.resize(kPaletteSize);
}

bool Scene9150::play() {
	if (!load())
		return false;

	for (uint i = 0; i < ARRAYSIZE(kScene9150ClipSteps) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		const Scene9150ClipStep &step = kScene9150ClipSteps[i];
		if (!loadClipChunk(step.chunkIndex))
			return false;
		runClip(step.staticSpeechRowIndex);
	}

	_speech.stop();
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene9150NextState;

	return true;
}

bool Scene9150::load() {
	return loadPalette() &&
		_text.loadStaticSpeechCues(kScene9150TextArchiveName, _debugName, kScene9150StaticSpeechTableOffset);
}

bool Scene9150::loadPalette() {
	ChunkArchive archive;
	if (!archive.open(Common::Path(kScene9150ArchiveName))) {
		warning("Failed to read %s header", kScene9150ArchiveName);
		return false;
	}

	if (!archive.readFixedChunk(0, _paletteResource, kPaletteSize, _debugName))
		return false;

	return true;
}

bool Scene9150::loadClipChunk(uint chunkIndex) {
	ChunkArchive archive;
	if (!archive.open(Common::Path(kScene9150ArchiveName))) {
		warning("Failed to read %s header", kScene9150ArchiveName);
		return false;
	}

	if (!archive.readVariableChunk(chunkIndex, _clipResource)) {
		warning("Failed to read %s clip chunk %u", kScene9150ArchiveName, chunkIndex);
		return false;
	}

	return true;
}

void Scene9150::runClip(byte staticSpeechRowIndex) {
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());

	for (byte frame = 0; frame < kScene9150DescriptorCount && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		if (pollEvents())
			return;

		drawClipFrame(frame);
		presentFrame();
		if (delay(kScene9150FrameIntervalMillis))
			return;
	}

	playStaticSpeechPair(staticSpeechRowIndex);
	fadeOutPalette();
}

void Scene9150::drawClipFrame(byte frameIndex) {
	restoreSpriteBackground(_clipResource, 0, 0, kScene9150DescriptorCount, frameIndex,
		_savedFramebuffer.surface(), _sceneFramebuffer.surface());
	drawStripSpriteFrame(_clipResource, 0, 0, kScene9150DescriptorCount, frameIndex,
		_sceneFramebuffer.surface());
}

void Scene9150::playStaticSpeechPair(byte rowIndex) {
	for (byte frameIndex = 0; frameIndex < 2 && !_skipRequested && !Engine::shouldQuit(); ++frameIndex) {
		uint16 textRecordId = 0;
		byte continuationCount = 0;
		uint16 voiceSampleId = 0;
		if (!_text.getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
			continue;
		(void)textRecordId;
		(void)continuationCount;

		const bool started = voiceSampleId != 0 && _speech.playSample(voiceSampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
		uint32 elapsed = 0;
		while (elapsed < duration && !_skipRequested && !Engine::shouldQuit()) {
			if (pollEvents())
				return;
			const uint32 slice = MIN<uint32>(duration - elapsed, 10);
			g_system->delayMillis(slice);
			elapsed += slice;
		}
	}
}

void Scene9150::fadeOutPalette() {
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

} // End of namespace Hollywood
