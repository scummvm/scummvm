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

#include "hollywood/scenes/intro/scene9140.h"

#include "common/debug.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene9140ArchiveName = "RESOURCE.I14";
const char *const kScene9140TextArchiveName = "RESOURCE.003";
const uint kScene9140StageIndex = 913;
const byte kScene9140SpeechColor = 0xf8;
const uint kScene9140SpeechFrameMillis = 125;
const uint kScene9140PoseFrameMillis = 125;
const int kScene9140SpeechLineHeight = 20;
const uint kScene9140RightBodyDescriptorCount = 0x14;
const uint kScene9140MouthDescriptorCount = 4;

const byte kScene9140RightBodyFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 7, 6, 5, 0, 13, 14, 15,
	16, 17, 18, 19, 0
};

const byte kScene9140MouthFrameMap[] = {
	0, 1, 2, 3
};

const Scene9140::SequenceStep kScene9140Variant0[] = {
	{ Scene9140::kSpeechLeft, 0, 0 },
	{ Scene9140::kOpenRightPose, 0, 0 },
	{ Scene9140::kSpeechRightHigh, 1, 0 },
	{ Scene9140::kCloseRightPose, 0, 0 },
	{ Scene9140::kEnableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechLeft, 1, 1 },
	{ Scene9140::kDisableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechRightLow, 1, 2 }
};

const Scene9140::SequenceStep kScene9140Variant1[] = {
	{ Scene9140::kSpeechLeft, 2, 0 },
	{ Scene9140::kSpeechRightLow, 2, 1 },
	{ Scene9140::kEnableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechLeft, 2, 2 },
	{ Scene9140::kDisableLeftLoop, 0, 0 },
	{ Scene9140::kOpenRightPose, 0, 0 },
	{ Scene9140::kSpeechRightHigh, 2, 3 },
	{ Scene9140::kCloseRightPose, 0, 0 },
	{ Scene9140::kSpeechLeft, 2, 4 }
};

const Scene9140::SequenceStep kScene9140Variant2[] = {
	{ Scene9140::kSpeechLeft, 3, 0 },
	{ Scene9140::kOpenRightPose, 0, 0 },
	{ Scene9140::kSpeechRightHigh, 3, 1 },
	{ Scene9140::kCloseRightPose, 0, 0 },
	{ Scene9140::kEnableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechLeft, 3, 2 },
	{ Scene9140::kDisableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechRightLow, 3, 3 },
	{ Scene9140::kEnableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechLeft, 3, 4 }
};

const Scene9140::SequenceStep kScene9140Variant3[] = {
	{ Scene9140::kEnableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechLeft, 4, 0 },
	{ Scene9140::kDisableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechRightLow, 4, 1 },
	{ Scene9140::kEnableLeftLoop, 0, 0 },
	{ Scene9140::kSpeechLeft, 4, 2 },
	{ Scene9140::kDisableLeftLoop, 0, 0 },
	{ Scene9140::kOpenRightPose, 0, 0 },
	{ Scene9140::kSpeechRightHigh, 4, 3 },
	{ Scene9140::kCloseRightPose, 0, 0 }
};

const Scene9140::SequenceStep *const kScene9140Variants[] = {
	kScene9140Variant0,
	kScene9140Variant1,
	kScene9140Variant2,
	kScene9140Variant3
};

const uint kScene9140VariantStepCounts[] = {
	ARRAYSIZE(kScene9140Variant0),
	ARRAYSIZE(kScene9140Variant1),
	ARRAYSIZE(kScene9140Variant2),
	ARRAYSIZE(kScene9140Variant3)
};

Scene9140::Scene9140(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9140"),
		_resources(),
		_speech(),
		_text(),
		_paletteResource(),
		_baseFramebuffer(),
		_subtitle(),
		_rightBodyFrame(0),
		_mouthFrame(0),
		_leftLoopEnabled(false),
		_speechAnimationStep(0) {
	_paletteResource.resize(kPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_subtitle.visible = false;
	_subtitle.colorIndex = kScene9140SpeechColor;
	_subtitle.centerX = 0;
	_subtitle.topY = 0;
}

bool Scene9140::play() {
	if (!load())
		return false;

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	drawComposite();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _savedFramebuffer.size());
	_sceneFramebuffer.clear(0);
	presentFrame();
	revealSavedFramebufferWithCurtain();
	memcpy(_sceneFramebuffer.data(), _savedFramebuffer.data(), _sceneFramebuffer.size());
	presentFrame();

	GameplayState &state = _vm->gameState();
	const byte variantIndex = state.scene9140VariantIndex;
	if (variantIndex < ARRAYSIZE(kScene9140Variants))
		state.scene9140VariantIndex = variantIndex + 1;

	if (!_skipRequested && !Engine::shouldQuit())
		runVariantSequence(variantIndex);

	clearSubtitle();
	clearSceneFramebufferWithCurtain();
	fadeOutPalette();
	_speech.stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		state.mainFlowStateId = state.scene9140ReturnStateId != 0 ? state.scene9140ReturnStateId : 1000;

	return true;
}

bool Scene9140::load() {
	if (!_resources.loadChunkTable(kScene9140ArchiveName))
		return false;

	if (!_resources.validateChunkRange(kScene9140ArchiveName, _debugName, 0, 4))
		return false;

	if (!loadChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadChunk(1, _paletteResource, kPaletteSize))
		return false;

	_resources.allocateArena(_resources.totalChunkSize(2, 4));
	for (uint i = 2; i <= 4; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	if (!_text.loadStage(kScene9140TextArchiveName, _debugName, kScene9140StageIndex))
		return false;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	return true;
}

bool Scene9140::loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9140::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9140::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index, index);
}

void Scene9140::runVariantSequence(byte variantIndex) {
	if (variantIndex >= ARRAYSIZE(kScene9140Variants))
		return;

	const Scene9140::SequenceStep *steps = kScene9140Variants[variantIndex];
	const uint stepCount = kScene9140VariantStepCounts[variantIndex];
	for (uint i = 0; i < stepCount && !_skipRequested && !Engine::shouldQuit(); ++i)
		runStep(steps[i]);
}

void Scene9140::runStep(const SequenceStep &step) {
	switch (step.type) {
	case kSpeechLeft:
		runSpeechLine(step.rowIndex, step.frameIndex, 0x4c, 0xea, 0x28, 0x16, 0x0b, true);
		break;
	case kSpeechRightLow:
		runSpeechLine(step.rowIndex, step.frameIndex, 0xb1, 0xef, 0x20, 0x32, 0x00, false);
		break;
	case kSpeechRightHigh:
		runSpeechLine(step.rowIndex, step.frameIndex, 0xa7, 0x101, 0x20, 0x32, 0x00, false);
		break;
	case kOpenRightPose:
		animateRightPose(4, 8);
		break;
	case kCloseRightPose:
		animateRightPose(12, 16);
		_rightBodyFrame = 0;
		drawComposite();
		presentFrame();
		break;
	case kEnableLeftLoop:
		_leftLoopEnabled = true;
		_rightBodyFrame = 0x11;
		break;
	case kDisableLeftLoop:
		_leftLoopEnabled = false;
		_rightBodyFrame = 0;
		break;
	}
}

void Scene9140::runSpeechLine(byte rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue, bool leftSpeaker) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!_text.getStageCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	_paletteCurrent[kScene9140SpeechColor * 3] = red;
	_paletteCurrent[kScene9140SpeechColor * 3 + 1] = green;
	_paletteCurrent[kScene9140SpeechColor * 3 + 2] = blue;
	runSpeechCue(textRecordId, continuationCount, voiceSampleId, centerX, topY, leftSpeaker);
}

void Scene9140::runSpeechCue(uint16 textRecordId, byte continuationCount, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, bool leftSpeaker) {
	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !_skipRequested && !Engine::shouldQuit(); ++part) {
		const Common::String text = _text.largeTextRecord(textRecordId + part);
		if (!text.empty()) {
			_subtitle.visible = true;
			_subtitle.colorIndex = kScene9140SpeechColor;
			_subtitle.centerX = centerX;
			_subtitle.topY = topY;
			wrapSubtitleText(text, centerX, _subtitle.lines);
		}

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _subtitle.lines.size() * 1100);
		uint32 elapsed = 0;
		uint32 animationElapsed = 0;
		_speechAnimationStep = 0;
		while (elapsed < duration && !_skipRequested && !Engine::shouldQuit()) {
			if (pollEvents())
				return;

			if (animationElapsed >= kScene9140SpeechFrameMillis) {
				animationElapsed %= kScene9140SpeechFrameMillis;
				++_speechAnimationStep;
				if (leftSpeaker) {
					_mouthFrame = _speechAnimationStep % ARRAYSIZE(kScene9140MouthFrameMap);
					if (_leftLoopEnabled)
						_rightBodyFrame = 0x11 + (_speechAnimationStep % 8);
				} else {
					const byte baseFrame = _leftLoopEnabled ? 8 : 0;
					_rightBodyFrame = baseFrame + (_speechAnimationStep % 5);
					_mouthFrame = 0;
				}
			}

			drawComposite();
			presentFrame();
			const uint32 slice = MIN<uint32>(duration - elapsed, 10);
			g_system->delayMillis(slice);
			elapsed += slice;
			animationElapsed += slice;
		}

		_mouthFrame = 0;
		if (leftSpeaker && _leftLoopEnabled)
			_rightBodyFrame = 0x11;
		else if (!leftSpeaker)
			_rightBodyFrame = _leftLoopEnabled ? 8 : 0;
		clearSubtitle();
		drawComposite();
		presentFrame();
	}
}

void Scene9140::animateRightPose(byte firstFrame, byte lastFrame) {
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		if (pollEvents())
			return;
		_rightBodyFrame = frame;
		drawComposite();
		presentFrame();
		if (delay(kScene9140PoseFrameMillis))
			return;
	}
}

void Scene9140::drawComposite() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	const byte bodyFrame = _rightBodyFrame < ARRAYSIZE(kScene9140RightBodyFrameMap) ?
		kScene9140RightBodyFrameMap[_rightBodyFrame] : 0;
	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[4], 0,
		kScene9140RightBodyDescriptorCount, bodyFrame, _sceneFramebuffer.surface());

	const byte mouthFrame = _mouthFrame < ARRAYSIZE(kScene9140MouthFrameMap) ?
		kScene9140MouthFrameMap[_mouthFrame] : 0;
	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[3], 0,
		kScene9140MouthDescriptorCount, mouthFrame, _sceneFramebuffer.surface());
}

void Scene9140::clearSubtitle() {
	_subtitle.visible = false;
	_subtitle.lines.clear();
}

void Scene9140::drawFrameOverlays() {
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
		const int y = (int)_subtitle.topY + lineIndex * kScene9140SpeechLineHeight;
		font->drawString(_screen.surfacePtr(), line, x, y, lineWidth, _subtitle.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene9140::wrapSubtitleText(const Common::String &text, uint16 anchorSceneX,
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

uint Scene9140::subtitleTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene9140::fadeOutPalette() {
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
