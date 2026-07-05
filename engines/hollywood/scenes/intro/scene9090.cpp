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

#include "hollywood/scenes/intro/scene9090.h"

#include "common/debug.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene9090ArchiveName = "RESOURCE.I09";
const char *const kScene9090TextArchiveName = "RESOURCE.003";
const char *const kScene9090MusicArchiveName = "RESOURCE.M09";
const uint kScene9090StageIndex = 910;
const uint16 kScene9090MusicCueId = 0x000f;
const uint16 kScene9090NextState = 0x23aa;
const uint kScene9090SpeechRow = 4;
const byte kScene9090SpeechColor = 0xfb;
const int kScene9090SpeechLineHeight = 20;

struct Scene9090SpeechStep {
	byte frameIndex;
	uint16 centerX;
	uint16 topY;
	byte red;
	byte green;
	byte blue;
};

const Scene9090SpeechStep kScene9090SpeechSteps[] = {
	{ 0, 0x78, 0xaa, 0x00, 0x26, 0x3f },
	{ 1, 0x276, 0x10c, 0x3f, 0x3f, 0x3f },
	{ 2, 0xc0, 0xc8, 0x3f, 0x3f, 0x3f },
	{ 3, 0x276, 0x10c, 0x3f, 0x3f, 0x3f },
	{ 4, 0x78, 0xaa, 0x00, 0x26, 0x3f },
	{ 5, 0x276, 0x10c, 0x3f, 0x3f, 0x3f },
	{ 6, 0xc0, 0xc8, 0x3f, 0x3f, 0x3f },
	{ 7, 0x78, 0xaa, 0x00, 0x26, 0x3f }
};

Scene9090::Scene9090(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9090"),
		_resources(),
		_music(),
		_speech(),
		_text(),
		_paletteResource(),
		_sceneFillRuns(),
		_subtitle() {
	_paletteResource.resize(kPaletteSize);
	_subtitle.visible = false;
	_subtitle.colorIndex = kScene9090SpeechColor;
	_subtitle.centerX = 0;
	_subtitle.topY = 0;
}

bool Scene9090::play() {
	if (!load())
		return false;

	memcpy(_sceneFramebuffer.data(), _savedFramebuffer.data(), _sceneFramebuffer.size());
	drawOfficePatch();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	_music.setArchive(Common::Path(kScene9090MusicArchiveName));
	_music.playMusicCue(kScene9090MusicCueId, 30);
	revealSavedFramebufferWithCurtain();

	if (!_skipRequested && !Engine::shouldQuit())
		runDialogueSequence();

	clearSubtitle();
	clearSceneFramebufferWithCurtain();
	fadeOutPalette();
	_speech.stop();
	_music.stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene9090NextState;

	return true;
}

bool Scene9090::load() {
	if (!_resources.loadChunkTable(kScene9090ArchiveName))
		return false;

	for (uint i = 0; i <= 12; ++i) {
		if (!_resources.validateChunk(kScene9090ArchiveName, _debugName, i))
			return false;
	}

	if (!loadChunk(0, _savedFramebuffer, kFrameBufferSize) ||
			!loadChunk(1, _paletteResource, kPaletteSize) ||
			!loadVariableChunk(2, _sceneFillRuns))
		return false;

	uint32 arenaSize = 0;
	for (uint i = 5; i <= 12; ++i)
		arenaSize += _resources.chunkTable.sizes[i];
	_resources.allocateArena(arenaSize);
	for (uint i = 5; i <= 12; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	if (!_text.loadStage(kScene9090TextArchiveName, _debugName, kScene9090StageIndex))
		return false;

	return true;
}

bool Scene9090::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9090::loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9090::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	return _resources.loadVariableChunk(index, destination);
}

bool Scene9090::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index, index);
}

void Scene9090::expandFillRunsToSavedFramebuffer() {
	uint destinationOffset = 0;
	uint sourceOffset = 0;
	while (destinationOffset < _savedFramebuffer.size() && sourceOffset + 3 <= _sceneFillRuns.size()) {
		const byte fill = _sceneFillRuns[sourceOffset];
		const uint16 runLength = readUint16LE(_sceneFillRuns, sourceOffset + 1);
		sourceOffset += 3;
		if (runLength == 0)
			break;

		const uint clampedRunLength = MIN<uint>(runLength, _savedFramebuffer.size() - destinationOffset);
		memset(_savedFramebuffer.data() + destinationOffset, fill, clampedRunLength);
		destinationOffset += clampedRunLength;
	}
}

void Scene9090::drawOfficePatch() {
	if (_resources.chunkOffsets[6] != 0)
		drawResourceBlockList(_resources.arena, _resources.chunkOffsets[6], _sceneFramebuffer.surface());
}

void Scene9090::runDialogueSequence() {
	for (uint i = 0; i < ARRAYSIZE(kScene9090SpeechSteps) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		const Scene9090SpeechStep &step = kScene9090SpeechSteps[i];
		runSpeechLine(step.frameIndex, step.centerX, step.topY, step.red, step.green, step.blue);
	}
}

void Scene9090::runSpeechLine(byte frameIndex, uint16 centerX, uint16 topY, byte red, byte green, byte blue) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!_text.getStageCue(kScene9090SpeechRow, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	_paletteCurrent[kScene9090SpeechColor * 3] = red;
	_paletteCurrent[kScene9090SpeechColor * 3 + 1] = green;
	_paletteCurrent[kScene9090SpeechColor * 3 + 2] = blue;
	runSpeechCue(textRecordId, continuationCount, voiceSampleId, centerX, topY, kScene9090SpeechColor);
}

void Scene9090::runSpeechCue(uint16 textRecordId, byte continuationCount, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, byte colorIndex) {
	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !_skipRequested && !Engine::shouldQuit(); ++part) {
		const Common::String text = _text.largeTextRecord(textRecordId + part);
		if (!text.empty()) {
			_subtitle.visible = true;
			_subtitle.colorIndex = colorIndex;
			_subtitle.centerX = centerX;
			_subtitle.topY = topY;
			wrapSubtitleText(text, centerX, _subtitle.lines);
		}

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _subtitle.lines.size() * 1100);
		uint32 elapsed = 0;
		while (elapsed < duration && !_skipRequested && !Engine::shouldQuit()) {
			if (pollEvents())
				return;
			presentFrame();
			const uint32 slice = MIN<uint32>(duration - elapsed, 10);
			g_system->delayMillis(slice);
			elapsed += slice;
		}
		clearSubtitle();
	}
}

void Scene9090::clearSubtitle() {
	_subtitle.visible = false;
	_subtitle.lines.clear();
}

void Scene9090::drawFrameOverlays() {
	if (!_subtitle.visible || !_vm->font() || !_vm->font()->isLoaded())
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);

	for (uint lineIndex = 0; lineIndex < _subtitle.lines.size(); ++lineIndex) {
		const Common::String &line = _subtitle.lines[lineIndex];
		const int lineWidth = subtitleTextWidth(line);
		int x = (int)_subtitle.centerX - (lineWidth >> 1);
		if (x < 0)
			x = 0;
		if (x + lineWidth > HollywoodEngine::kScreenWidth)
			x = MAX<int>(0, HollywoodEngine::kScreenWidth - lineWidth);
		const int y = (int)_subtitle.topY + lineIndex * kScene9090SpeechLineHeight;
		font->drawString(_screen.surfacePtr(), line, x, y, lineWidth, _subtitle.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene9090::wrapSubtitleText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = anchorSceneX < 0xa0 || HollywoodEngine::kScreenWidth - anchorSceneX < 0xa0 ? 0x24 : 0x32;
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
		maxChars = maxChars > 2 ? maxChars - 2 : 1;
	}
}

uint Scene9090::subtitleTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene9090::fadeOutPalette() {
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
