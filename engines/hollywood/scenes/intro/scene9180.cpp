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

#include "hollywood/scenes/intro/scene9180.h"

#include "common/debug.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene9180ArchiveName = "RESOURCE.I18";
const char *const kScene9180TextArchiveName = "RESOURCE.003";
const uint kScene9180StageIndex = 920;
const uint16 kScene9180ExitState = 0x00ff;
const uint kScene9180DescriptorCount = 0x1d;
const uint kScene9180AnimationFrameMillis = 75;
const uint kScene9180SpeechFrameMillis = 125;
const uint kScene9180FlickerIntervalMillis = 75;
const byte kScene9180SpeechColor = 0xfb;
const uint16 kScene9180SpeechCenterX = 0x159;
const uint16 kScene9180SpeechTopY = 0x128;
const int kScene9180SpeechLineHeight = 20;

const byte kScene9180FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 23, 22, 21, 20, 19, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	19, 20, 21, 26, 25, 21, 27, 28, 21, 20,
	19, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 19, 20, 21, 22, 23, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 23, 22, 21
};

Scene9180::Scene9180(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9180"),
		_resources(),
		_speech(),
		_loopSound(),
		_effectSound(),
		_text(),
		_random("hollywood_scene9180"),
		_paletteResource(),
		_normalPalette(),
		_brightPalette(),
		_baseFramebuffer(),
		_subtitle(),
		_frameMapIndex(0),
		_flickerModulus(10),
		_brightPaletteActive(false) {
	_paletteResource.resize(kPaletteSize);
	_normalPalette.resize(kPaletteSize);
	_brightPalette.resize(kPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_subtitle.visible = false;
	_subtitle.colorIndex = kScene9180SpeechColor;
	_subtitle.centerX = 0;
	_subtitle.topY = 0;
}

bool Scene9180::play() {
	if (!load())
		return false;

	runSequence();

	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene9180ExitState;

	return true;
}

bool Scene9180::load() {
	if (!_resources.loadChunkTable(kScene9180ArchiveName))
		return false;

	if (!_resources.validateChunkRange(kScene9180ArchiveName, _debugName, 0, 2))
		return false;

	if (!loadChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadChunk(1, _paletteResource, kPaletteSize))
		return false;

	_resources.allocateArena(_resources.totalChunkSize(2, 2));
	if (!loadArenaChunk(2))
		return false;

	if (!_text.loadStage(kScene9180TextArchiveName, _debugName, kScene9180StageIndex))
		return false;

	memcpy(_normalPalette.data(), _paletteResource.data(), _normalPalette.size());
	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	buildBrightPalette();
	return true;
}

bool Scene9180::loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9180::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9180::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index, index);
}

void Scene9180::runSequence() {
	_frameMapIndex = 0;
	drawComposite();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _savedFramebuffer.size());
	_sceneFramebuffer.clear(0);
	presentFrame();
	_loopSound.playSample(0x1d, 50, true);
	revealSavedFramebufferWithCurtain();
	memcpy(_sceneFramebuffer.data(), _savedFramebuffer.data(), _sceneFramebuffer.size());
	presentFrame();

	waitWithEffects(3000);
	_flickerModulus = 3;
	animateFrameRange(0, 0x34, 1);
	runSpeechLine(0);
	animateFrameRange(0x58, 0x3a, -1);
	_frameMapIndex = 0x35;
	runSpeechLine(1);
	animateFrameRange(0x3a, 0x58, 1);
	_frameMapIndex = 0x35;
	runSpeechLine(2);

	_frameMapIndex = 0x15;
	drawComposite();
	presentFrame();
	waitWithEffects(3000);
	fillBlackPixelsForMemoryFlash();
	presentFrame();
	brightenToWhite();
	_loopSound.stop();
	convertPaletteToGrayscale();
	fadeFromWhiteToGrayscale();
	waitForFinalInput();
}

void Scene9180::drawComposite() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	drawFrameIndex(_frameMapIndex);
}

void Scene9180::drawFrameIndex(byte frameMapIndex) {
	if (frameMapIndex >= ARRAYSIZE(kScene9180FrameMap))
		return;

	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[2], 0,
		kScene9180DescriptorCount, kScene9180FrameMap[frameMapIndex],
		_sceneFramebuffer.surface());
}

void Scene9180::animateFrameRange(byte firstFrameMapIndex, byte lastFrameMapIndex, int step) {
	if (step == 0)
		return;

	int frame = firstFrameMapIndex;
	while (!_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;
		updateFlickerPalette();
		_frameMapIndex = (byte)frame;
		drawComposite();
		presentFrame();
		if (delay(kScene9180AnimationFrameMillis))
			return;
		if (frame == lastFrameMapIndex)
			break;
		frame += step;
	}
}

void Scene9180::waitWithEffects(uint32 millis) {
	uint32 elapsed = 0;
	uint32 flickerElapsed = 0;
	while (elapsed < millis && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;
		if (flickerElapsed >= kScene9180FlickerIntervalMillis) {
			flickerElapsed %= kScene9180FlickerIntervalMillis;
			updateFlickerPalette();
		}
		drawComposite();
		presentFrame();
		const uint32 slice = MIN<uint32>(millis - elapsed, 10);
		g_system->delayMillis(slice);
		elapsed += slice;
		flickerElapsed += slice;
	}
}

void Scene9180::runSpeechLine(byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!_text.getStageCue(1, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	_paletteCurrent[kScene9180SpeechColor * 3] = 6;
	_paletteCurrent[kScene9180SpeechColor * 3 + 1] = 0x3f;
	_paletteCurrent[kScene9180SpeechColor * 3 + 2] = 0x2d;
	runSpeechCue(textRecordId, continuationCount, voiceSampleId);
}

void Scene9180::runSpeechCue(uint16 textRecordId, byte continuationCount, uint16 voiceSampleId) {
	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !_skipRequested && !Engine::shouldQuit(); ++part) {
		const Common::String text = _text.largeTextRecord(textRecordId + part);
		if (!text.empty()) {
			_subtitle.visible = true;
			_subtitle.colorIndex = kScene9180SpeechColor;
			_subtitle.centerX = kScene9180SpeechCenterX;
			_subtitle.topY = kScene9180SpeechTopY;
			wrapSubtitleText(text, kScene9180SpeechCenterX, _subtitle.lines);
		}

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _subtitle.lines.size() * 1100);
		uint32 elapsed = 0;
		uint32 speechElapsed = 0;
		uint32 flickerElapsed = 0;
		byte talkFrame = 0;
		while (elapsed < duration && !_skipRequested && !Engine::shouldQuit()) {
			if (pollEvents())
				return;

			if (speechElapsed >= kScene9180SpeechFrameMillis) {
				speechElapsed %= kScene9180SpeechFrameMillis;
				_frameMapIndex = 0x35 + (talkFrame % 5);
				++talkFrame;
			}
			if (flickerElapsed >= kScene9180FlickerIntervalMillis) {
				flickerElapsed %= kScene9180FlickerIntervalMillis;
				updateFlickerPalette();
			}

			drawComposite();
			presentFrame();
			const uint32 slice = MIN<uint32>(duration - elapsed, 10);
			g_system->delayMillis(slice);
			elapsed += slice;
			speechElapsed += slice;
			flickerElapsed += slice;
		}
		_frameMapIndex = 0x35;
		clearSubtitle();
		drawComposite();
		presentFrame();
	}
}

void Scene9180::updateFlickerPalette() {
	if (!_brightPaletteActive &&
			_random.getRandomNumber(MAX<byte>(1, _flickerModulus) - 1) != 0)
		return;

	const uint speechColorOffset = kScene9180SpeechColor * 3;
	const byte speechRed = _paletteCurrent[speechColorOffset];
	const byte speechGreen = _paletteCurrent[speechColorOffset + 1];
	const byte speechBlue = _paletteCurrent[speechColorOffset + 2];
	if (!_brightPaletteActive) {
		memcpy(_paletteCurrent.data(), _brightPalette.data(), _paletteCurrent.size());
		_effectSound.playSample(0x1c, 100);
		_brightPaletteActive = true;
	} else {
		memcpy(_paletteCurrent.data(), _normalPalette.data(), _paletteCurrent.size());
		_brightPaletteActive = false;
	}
	_paletteCurrent[speechColorOffset] = speechRed;
	_paletteCurrent[speechColorOffset + 1] = speechGreen;
	_paletteCurrent[speechColorOffset + 2] = speechBlue;
}

void Scene9180::buildBrightPalette() {
	memcpy(_brightPalette.data(), _normalPalette.data(), _brightPalette.size());
	for (uint color = 1; color < 0xfb; ++color) {
		uint add = 0x10;
		for (uint component = 0; component < 3; ++component)
			add = MIN<uint>(add, 0x3f - _normalPalette[color * 3 + component]);
		for (uint component = 0; component < 3; ++component)
			_brightPalette[color * 3 + component] = _normalPalette[color * 3 + component] + add;
	}
}

void Scene9180::brightenToWhite() {
	memcpy(_paletteCurrent.data(), _normalPalette.data(), _paletteCurrent.size());
	for (byte threshold = 0; threshold < 0x40 && !_skipRequested && !Engine::shouldQuit(); threshold += 3) {
		for (uint i = 0; i < _paletteCurrent.size(); ++i) {
			if (_paletteResource[i] <= threshold)
				_paletteCurrent[i] = MIN<byte>(0x3f, _paletteCurrent[i] + 3);
		}
		presentFrame();
		if (delay(20))
			return;
	}
}

void Scene9180::convertPaletteToGrayscale() {
	for (uint color = 0; color < 256; ++color) {
		const uint offset = color * 3;
		const byte gray = (byte)((_paletteResource[offset] +
			_paletteResource[offset + 1] + _paletteResource[offset + 2]) / 3);
		_paletteResource[offset] = gray;
		_paletteResource[offset + 1] = gray;
		_paletteResource[offset + 2] = gray;
	}
}

void Scene9180::fadeFromWhiteToGrayscale() {
	for (int threshold = 0x3f; threshold >= 0 && !_skipRequested && !Engine::shouldQuit(); threshold -= 3) {
		for (uint color = 0; color < 256; ++color) {
			for (uint component = 0; component < 3; ++component) {
				const uint offset = color * 3 + component;
				const byte target = _paletteResource[offset];
				if (target <= threshold) {
					if (_paletteCurrent[offset] < target + 3)
						_paletteCurrent[offset] = target;
					else
						_paletteCurrent[offset] -= 3;
				}
			}
		}
		presentFrame();
		if (delay(20))
			return;
	}
	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	presentFrame();
}

void Scene9180::fillBlackPixelsForMemoryFlash() {
	byte *pixels = _sceneFramebuffer.data();
	for (uint i = 0; i < _sceneFramebuffer.size(); ++i) {
		if (pixels[i] == 0)
			pixels[i] = 1;
	}
	_baseFramebuffer.managedSurface().copyFrom(_sceneFramebuffer.managedSurface());
}

void Scene9180::waitForFinalInput() {
	while (!_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;
		presentFrame();
		g_system->delayMillis(10);
	}
}

void Scene9180::stopAudio() {
	_speech.stop();
	_loopSound.stop();
	_effectSound.stop();
}

void Scene9180::clearSubtitle() {
	_subtitle.visible = false;
	_subtitle.lines.clear();
}

void Scene9180::drawFrameOverlays() {
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
		const int y = (int)_subtitle.topY + lineIndex * kScene9180SpeechLineHeight;
		font->drawString(_screen.surfacePtr(), line, x, y, lineWidth, _subtitle.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene9180::wrapSubtitleText(const Common::String &text, uint16 anchorSceneX,
		Common::Array<Common::String> &lines) const {
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

uint Scene9180::subtitleTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

} // End of namespace Hollywood
