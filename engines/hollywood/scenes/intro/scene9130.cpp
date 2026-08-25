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

#include "hollywood/scenes/intro/scene9130.h"

#include "common/debug.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const char *const kScene9130ArchiveName = "RESOURCE.I13";
const char *const kScene9130TextArchiveName = "RESOURCE.003";
const uint kScene9130StageIndex = 912;
const uint16 kScene9130NextState = 0x23d2;
const uint kScene9130ClipChunk = 5;
const uint kScene9130ClipFrameCount = 0xf3;
const uint kScene9130ClipFrameIntervalMillis = 120;
const uint kScene9130SpeechRow = 5;
const int kScene9130SpeechLineHeight = 20;

struct Scene9130SpeechStyle {
	uint16 centerX;
	uint16 topY;
	byte colorIndex;
	byte red;
	byte green;
	byte blue;
};

const Scene9130SpeechStyle kScene9130SpeechStyles[] = {
	{ 0x108, 0xda, 0xfb, 0x3f, 0x3f, 0x3f },
	{ 0x16d, 0xda, 0xfb, 0x3f, 0x28, 0x32 },
	{ 0x16d, 0xda, 0xfb, 0x3f, 0x28, 0x32 },
	{ 0x108, 0xda, 0xfb, 0x3f, 0x3f, 0x3f },
	{ 0x16d, 0xda, 0xfb, 0x3f, 0x28, 0x32 }
};

Scene9130::Scene9130(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9130"),
		_resources(),
		_music(vm->introMusic()),
		_speech(),
		_text(),
		_paletteResource(),
		_subtitle(),
		_activeTextRecordId(0),
		_activeVoiceSampleId(0),
		_activeContinuationCount(0),
		_activeContinuationIndex(0),
		_activeSpeechStyleIndex(0),
		_activeSpeechCue(false),
		_nextSpeechFrameIndex(0) {
	_paletteResource.resize(kPaletteSize);
	_subtitle.visible = false;
	_subtitle.colorIndex = 0xfb;
	_subtitle.centerX = 0;
	_subtitle.topY = 0;
}

bool Scene9130::play() {
	if (!load())
		return false;

	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memcpy(_sceneFramebuffer.data(), _savedFramebuffer.data(), _sceneFramebuffer.size());
	drawClipFrame(0);
	presentFrame();

	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	_music->setVolume(100);
	revealSavedFramebufferWithCurtain();

	if (!_skipRequested && !Engine::shouldQuit())
		runClipAndDialogue();

	clearSubtitle();
	clearSceneFramebufferWithCurtain();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	_speech.stop();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene9130NextState;

	return true;
}

bool Scene9130::load() {
	if (!_resources.loadChunkTable(kScene9130ArchiveName))
		return false;

	for (uint i = 0; i <= kScene9130ClipChunk; ++i) {
		if (!_resources.validateChunk(kScene9130ArchiveName, _debugName, i))
			return false;
	}

	if (!loadChunk(0, _savedFramebuffer, kFrameBufferSize) ||
			!loadChunk(1, _paletteResource, kPaletteSize))
		return false;

	_resources.allocateArena(_resources.chunkTable.sizes[kScene9130ClipChunk]);
	if (!loadArenaChunk(kScene9130ClipChunk))
		return false;

	if (!_text.loadStage(kScene9130TextArchiveName, _debugName, kScene9130StageIndex))
		return false;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	return true;
}

bool Scene9130::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9130::loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9130::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index, index);
}

void Scene9130::runClipAndDialogue() {
	_nextSpeechFrameIndex = 0;
	uint32 lastFrameMillis = g_system->getMillis();
	byte frameIndex = 1;

	while (frameIndex < kScene9130ClipFrameCount && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		maybeStartNextSpeechLine();

		const uint32 now = g_system->getMillis();
		if (now - lastFrameMillis >= kScene9130ClipFrameIntervalMillis) {
			lastFrameMillis = now;
			if ((frameIndex - 1) % 12 == 0)
				_speech.setVolume(85);
			drawClipFrame(frameIndex);
			presentFrame();
			++frameIndex;
		}

		g_system->delayMillis(5);
	}

	while (_speech.isPlaying() && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;
		presentFrame();
		g_system->delayMillis(10);
	}
}

void Scene9130::drawClipFrame(byte frameIndex) {
	ResourceDeltaClipPlayer::drawFrame(_resources.arena, _resources.chunkOffsets[kScene9130ClipChunk],
		_resources.chunkTable.sizes[kScene9130ClipChunk], kScene9130ClipFrameCount,
		frameIndex, _sceneFramebuffer.data(), _sceneFramebuffer.size());
}

void Scene9130::maybeStartNextSpeechLine() {
	if (_speech.isPlaying())
		return;

	if (_activeSpeechCue) {
		if (_activeContinuationIndex < _activeContinuationCount) {
			startCurrentSpeechSegment();
			return;
		}
		_activeSpeechCue = false;
		clearSubtitle();
	}

	if (_nextSpeechFrameIndex >= ARRAYSIZE(kScene9130SpeechStyles))
		return;

	if (prepareSpeechLine(_nextSpeechFrameIndex)) {
		++_nextSpeechFrameIndex;
		startCurrentSpeechSegment();
	} else {
		_nextSpeechFrameIndex = ARRAYSIZE(kScene9130SpeechStyles);
	}
}

bool Scene9130::prepareSpeechLine(byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!_text.getStageCue(kScene9130SpeechRow, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return false;

	_activeTextRecordId = textRecordId;
	_activeVoiceSampleId = voiceSampleId;
	_activeContinuationCount = MAX<byte>(1, continuationCount);
	_activeContinuationIndex = 0;
	_activeSpeechStyleIndex = frameIndex;
	_activeSpeechCue = true;
	return true;
}

bool Scene9130::startCurrentSpeechSegment() {
	if (!_activeSpeechCue || _activeContinuationIndex >= _activeContinuationCount)
		return false;

	const byte frameIndex = _activeSpeechStyleIndex;
	const Scene9130SpeechStyle &style = kScene9130SpeechStyles[frameIndex];
	_paletteCurrent[style.colorIndex * 3] = style.red;
	_paletteCurrent[style.colorIndex * 3 + 1] = style.green;
	_paletteCurrent[style.colorIndex * 3 + 2] = style.blue;

	const Common::String text = _text.largeTextRecord(_activeTextRecordId + _activeContinuationIndex);
	if (!text.empty()) {
		_subtitle.visible = true;
		_subtitle.colorIndex = style.colorIndex;
		_subtitle.centerX = style.centerX;
		_subtitle.topY = style.topY;
		wrapSubtitleText(text, style.centerX, _subtitle.lines);
	}

	const uint16 sampleId = _activeVoiceSampleId == 0 ? 0 : _activeVoiceSampleId + _activeContinuationIndex;
	++_activeContinuationIndex;
	_speech.stop();
	if (sampleId != 0)
		_speech.playSample(sampleId, 100);
	return true;
}

void Scene9130::clearSubtitle() {
	_subtitle.visible = false;
	_subtitle.lines.clear();
}

void Scene9130::drawFrameOverlays() {
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
		const int y = (int)_subtitle.topY + lineIndex * kScene9130SpeechLineHeight;
		font->drawString(_screen.surfacePtr(), line, x, y, lineWidth, _subtitle.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene9130::wrapSubtitleText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
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

uint Scene9130::subtitleTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene9130::stopAudio() {
	_speech.stop();
	_music->stop();
}

} // End of namespace Hollywood
