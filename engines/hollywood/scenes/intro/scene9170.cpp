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

#include "hollywood/scenes/intro/scene9170.h"

#include "common/debug.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene9170ArchiveName = "RESOURCE.I17";
const char *const kScene9170TextArchiveName = "RESOURCE.003";
const char *const kScene9170MusicArchiveName = "RESOURCE.M09";
const uint kScene9170StageIndex = 921;
const uint16 kScene9170NextState = 0x23c8;
const uint16 kScene9170MusicCueId = 0x000c;
const uint kScene9170TallFramebufferSize = 0x100000;
const byte kScene9170BlueSpeechColor = 0xfb;
const uint kScene9170SpeechFrameMillis = 125;
const uint kScene9170ScrollFrameMillis = 50;
const uint kScene9170EffectFrameMillis = 60;
const int kScene9170SpeechLineHeight = 20;

const byte kScene9170UpperFrameMaps[3][5] = {
	{ 0, 1, 2, 3, 4 },
	{ 5, 6, 7, 8, 9 },
	{ 10, 11, 12, 13, 14 }
};

const byte kScene9170LowerFrameMap[] = {
	0, 1, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8
};

const byte kScene9170EffectFrameMap[] = {
	0, 1, 2, 3, 4, 5, 4, 3, 2, 1
};

// The callback increments past idle state 0 before selecting descriptors 0-3.
const byte kScene9170EventFrameMap[] = {
	0, 0, 1, 2, 3
};

const byte kScene9170ScrollDownA[] = {
	4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 40, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4
};

const byte kScene9170ScrollUpA[] = {
	4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 40, 40, 40, 40, 40, 60,
	60, 40, 40, 40, 40, 40, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4
};

const byte kScene9170ScrollDownB[] = {
	4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 40, 40, 24, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4
};

const byte kScene9170ScrollUpB[] = {
	4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 24, 40, 40, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4
};

Scene9170::Scene9170(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9170", kScene9170TallFramebufferSize, kFrameBufferSize),
		_resources(),
		_music(vm->introMusic()),
		_speech(),
		_ambientSpeech(),
		_sound(),
		_ambientSound(),
		_text(),
		_random("hollywood_scene9170"),
		_paletteResource(),
		_baseFramebuffer(),
		_staticFramebuffer(),
		_subtitle(),
		_rowOffset(0),
		_upperActorsEnabled(false),
		_lowerActorsEnabled(false),
		_lowerDirty(false),
		_effectDirty(false),
		_eventDirty(false),
		_lowerFrame(0),
		_effectFrame(0),
		_eventFrame(0),
		_lastTalkingFrame(0xff),
		_channelCanvasOffset(0),
		_ambientEffectsEnabled(false),
		_ambientSpeechSampleId(0),
		_shakeActive(false),
		_shakeRowOffset(0) {
	_paletteResource.resize(kPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_staticFramebuffer.resize(kScene9170TallFramebufferSize);
	_subtitle.visible = false;
	_subtitle.colorIndex = kScene9170BlueSpeechColor;
	_subtitle.centerX = 0;
	_subtitle.topY = 0;
	_upperFrames[0] = _upperFrames[1] = _upperFrames[2] = 0;
	_upperDirty[0] = _upperDirty[1] = _upperDirty[2] = false;
}

bool Scene9170::play() {
	if (!load())
		return false;

	runSequence();

	_speech.stop();
	_sound.stop();
	stopLowerRoomAmbience();
	clearSubtitle();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	_rowOffset = 0;
	composeFrame();
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene9170NextState;

	return true;
}

bool Scene9170::load() {
	if (!_resources.loadChunkTable(kScene9170ArchiveName))
		return false;

	if (!_resources.validateChunkRange(kScene9170ArchiveName, _debugName, 0, 8))
		return false;

	if (!loadChunk(0, _paletteResource, kPaletteSize))
		return false;

	_resources.allocateArena(_resources.totalChunkSize(1, 8));
	for (uint i = 1; i <= 8; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	if (!_text.loadStage(kScene9170TextArchiveName, _debugName, kScene9170StageIndex))
		return false;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	return true;
}

bool Scene9170::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9170::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index, index);
}

void Scene9170::runSequence() {
	buildInitialStaticFrame();
	_music->setArchive(Common::Path(kScene9170MusicArchiveName));
	_music->playMusicCue(kScene9170MusicCueId, 50);

	_rowOffset = 0;
	addBlockListToCanvas(2, 0);
	fadeInPalette();
	if (delay(7000))
		return;

	scrollByTable(kScene9170ScrollDownA, ARRAYSIZE(kScene9170ScrollDownA), true);
	if (_skipRequested || Engine::shouldQuit())
		return;

	for (byte frameIndex = 0; frameIndex < 6 && !_skipRequested && !Engine::shouldQuit(); ++frameIndex) {
		runSpeechLine(1, frameIndex, 0x0e8, 0x0cc, 0x20, 0x30, 0x3f, 0);
		runSpeechLine(2, frameIndex, 0x145, 0x09a, 0x20, 0x30, 0x3f, 1);
		if (frameIndex < 5)
			runSpeechLine(3, frameIndex, 0x194, 0x0ce, 0x20, 0x30, 0x3f, 2);
		else
			runSpeechLine(3, frameIndex, 0x140, 0x08c, 0x20, 0x30, 0x3f, 5);
	}
	runSpeechLine(3, 5, 0x140, 0x08c, 0x20, 0x30, 0x3f, 5);
	if (_skipRequested || Engine::shouldQuit())
		return;

	for (uint i = 0; i < ARRAYSIZE(kScene9170ScrollUpA) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		if (pollEvents())
			return;
		_rowOffset = _rowOffset > kScene9170ScrollUpA[i] ? _rowOffset - kScene9170ScrollUpA[i] : 0;
		presentFrame();
		if (i == 0x0f)
			switchToLowerRoomFrame();
		if (i + 1 < ARRAYSIZE(kScene9170ScrollUpA) && delay(kScene9170ScrollFrameMillis))
			return;
	}
	if (_skipRequested || Engine::shouldQuit())
		return;

	startLowerRoomAmbience();
	waitWithAnimations(2000, 3, true);
	runSpeechLine(4, 0, 0x128, 0x0c2, 0x3f, 0x20, 0x3f, 3);
	_lowerFrame = 0;
	_lowerDirty = true;
	composeFrame();
	presentFrame();
	waitWithAnimations(2000, 3, true);
	runEventOverlayFrames();
	if (_skipRequested || Engine::shouldQuit())
		return;

	clearCanvasRows(0x1e0, HollywoodEngine::kScreenHeight);
	addBlockListToCanvas(4, 0x220);
	runShake();
	runSpeechLine(4, 1, 0x128, 0x0c2, 0x3f, 0x20, 0x3f, 3);
	_lowerFrame = 0;
	_lowerDirty = true;
	composeFrame();
	presentFrame();
	waitWithAnimations(1000, 3, false);
	scrollByTable(kScene9170ScrollDownB, ARRAYSIZE(kScene9170ScrollDownB), true);
	if (delay(3000))
		return;
	scrollByTable(kScene9170ScrollUpB, ARRAYSIZE(kScene9170ScrollUpB), false);
	runSpeechLine(4, 2, 0x128, 0x0c2, 0x3f, 0x20, 0x3f, 3);
	_lowerFrame = 0;
	_lowerDirty = true;
	composeFrame();
	presentFrame();
	waitWithAnimations(2000, 3, false);
	scrollTo(0x140, 4);
	if (delay(3000))
		return;

	_lowerFrame = 7;
	_lowerDirty = true;
	composeFrame();
	scrollTo(0x0a0, -4);
	waitWithAnimations(1000, 4, false, false);
	runSpeechLine(4, 3, 0x128, 0x0c2, 0x3f, 0x20, 0x3f, 4);
	waitWithAnimations(2000, 4, false);
	scrollTo(0x140, 4);
}

void Scene9170::buildInitialStaticFrame() {
	_staticFramebuffer.clear(0);
	_sceneFramebuffer.clear(0);
	_baseFramebuffer.clear(0);
	drawResourceBlockList(_resources.arena, _resources.chunkOffsets[3], _baseFramebuffer.surface());
	copyBaseToCanvasAtYOffset(0x1e0);

	_upperActorsEnabled = true;
	_lowerActorsEnabled = false;
	_upperFrames[0] = _upperFrames[1] = _upperFrames[2] = 0;
	_upperDirty[0] = _upperDirty[1] = _upperDirty[2] = true;
	_lowerFrame = 0;
	_effectFrame = 0;
	_eventFrame = 0;
	_lowerDirty = false;
	_effectDirty = false;
	_eventDirty = false;
	_channelCanvasOffset = 0x1e0;
	composeFrame();
}

void Scene9170::switchToLowerRoomFrame() {
	_baseFramebuffer.clear(0);
	drawResourceBlockList(_resources.arena, _resources.chunkOffsets[1], _baseFramebuffer.surface());
	copyBaseToCanvasAtYOffset(0);
	_upperActorsEnabled = false;
	_lowerActorsEnabled = true;
	_upperDirty[0] = _upperDirty[1] = _upperDirty[2] = false;
	_lowerFrame = 0;
	_effectFrame = 0;
	_lowerDirty = true;
	_effectDirty = true;
	_eventDirty = false;
	_channelCanvasOffset = 0;
	composeFrame();

	clearCanvasRows(0x1e0, HollywoodEngine::kScreenHeight);
	_rowOffset = 0x1e0;
	addBlockListToCanvas(2, 0x1e0);
}

void Scene9170::addBlockListToCanvas(uint chunkIndex, int yOffset) {
	if (chunkIndex >= IntroResourceSet::kResourceChunkCount)
		return;
	drawResourceBlockList(_resources.arena, _resources.chunkOffsets[chunkIndex],
		_staticFramebuffer.surface(), yOffset);
	drawResourceBlockList(_resources.arena, _resources.chunkOffsets[chunkIndex],
		_sceneFramebuffer.surface(), yOffset);
}

void Scene9170::copyBaseToCanvasAtYOffset(int yOffset) {
	for (int row = 0; row < HollywoodEngine::kScreenHeight; ++row) {
		const int destinationRow = row + yOffset;
		if (destinationRow < 0 || destinationRow >= _staticFramebuffer.surface().h)
			continue;
		memcpy(_staticFramebuffer.data() + destinationRow * HollywoodEngine::kSceneBufferWidth,
			_baseFramebuffer.data() + row * HollywoodEngine::kSceneBufferWidth,
			HollywoodEngine::kSceneBufferWidth);
		memcpy(_sceneFramebuffer.data() + destinationRow * HollywoodEngine::kSceneBufferWidth,
			_baseFramebuffer.data() + row * HollywoodEngine::kSceneBufferWidth,
			HollywoodEngine::kSceneBufferWidth);
	}
}

void Scene9170::clearCanvasRows(int yOffset, int rowCount) {
	for (int row = 0; row < rowCount; ++row) {
		const int destinationRow = yOffset + row;
		if (destinationRow < 0 || destinationRow >= _staticFramebuffer.surface().h)
			continue;
		memset(_staticFramebuffer.data() + destinationRow * HollywoodEngine::kSceneBufferWidth,
			0, HollywoodEngine::kSceneBufferWidth);
		memset(_sceneFramebuffer.data() + destinationRow * HollywoodEngine::kSceneBufferWidth,
			0, HollywoodEngine::kSceneBufferWidth);
	}
}

void Scene9170::composeFrame() {
	const bool drawEvent = _lowerActorsEnabled && _eventDirty;
	if (!_upperDirty[0] && !_upperDirty[1] && !_upperDirty[2] &&
			!_lowerDirty && !_effectDirty && !_eventDirty)
		return;

	if (_upperActorsEnabled) {
		for (uint i = 0; i < 3; ++i) {
			if (_upperDirty[i])
				restoreSpriteChannel(8, 0x0f, kScene9170UpperFrameMaps[i],
					ARRAYSIZE(kScene9170UpperFrameMaps[i]), _upperFrames[i]);
		}
	}

	if (_lowerActorsEnabled) {
		if (_lowerDirty)
			restoreSpriteChannel(6, 9, kScene9170LowerFrameMap, ARRAYSIZE(kScene9170LowerFrameMap), _lowerFrame);
		if (_effectDirty)
			restoreSpriteChannel(5, 6, kScene9170EffectFrameMap, ARRAYSIZE(kScene9170EffectFrameMap), _effectFrame);
		if (_eventDirty)
			restoreSpriteChannel(7, 4, kScene9170EventFrameMap, ARRAYSIZE(kScene9170EventFrameMap), _eventFrame);
	}

	if (_upperActorsEnabled) {
		for (uint i = 0; i < 3; ++i)
			drawSpriteChannel(8, 0x0f, kScene9170UpperFrameMaps[i],
				ARRAYSIZE(kScene9170UpperFrameMaps[i]), _upperFrames[i]);
	}

	if (_lowerActorsEnabled) {
		drawSpriteChannel(6, 9, kScene9170LowerFrameMap, ARRAYSIZE(kScene9170LowerFrameMap), _lowerFrame);
		drawSpriteChannel(5, 6, kScene9170EffectFrameMap, ARRAYSIZE(kScene9170EffectFrameMap), _effectFrame);
		if (drawEvent)
			drawSpriteChannel(7, 4, kScene9170EventFrameMap, ARRAYSIZE(kScene9170EventFrameMap), _eventFrame);
	}

	_upperDirty[0] = _upperDirty[1] = _upperDirty[2] = false;
	_lowerDirty = false;
	_effectDirty = false;
	_eventDirty = false;
}

void Scene9170::restoreSpriteChannel(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= IntroResourceSet::kResourceChunkCount || frameMapSize == 0)
		return;

	const byte mappedFrame = frameMap[MIN<uint>(frameIndex, frameMapSize - 1)];
	restoreSpriteBackground(_resources.arena, _resources.chunkOffsets[chunkIndex], 0,
		descriptorCount, mappedFrame, _staticFramebuffer.surface(), _sceneFramebuffer.surface(),
		_channelCanvasOffset);
}

void Scene9170::drawSpriteChannel(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= IntroResourceSet::kResourceChunkCount || frameMapSize == 0)
		return;

	const byte mappedFrame = frameMap[MIN<uint>(frameIndex, frameMapSize - 1)];
	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[chunkIndex], 0,
		descriptorCount, mappedFrame, _sceneFramebuffer.surface(), _channelCanvasOffset);
}

void Scene9170::scrollByTable(const byte *table, uint tableSize, bool add) {
	for (uint i = 0; i < tableSize && !_skipRequested && !Engine::shouldQuit(); ++i) {
		if (pollEvents())
			return;
		if (add)
			_rowOffset += table[i];
		else
			_rowOffset = _rowOffset > table[i] ? _rowOffset - table[i] : 0;
		presentFrame();
		if (i + 1 < tableSize && delay(kScene9170ScrollFrameMillis))
			return;
	}
}

void Scene9170::scrollTo(uint targetRowOffset, int step) {
	if (step == 0)
		return;

	while (_rowOffset != targetRowOffset && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;
		if (step > 0)
			_rowOffset = MIN<uint>(targetRowOffset, _rowOffset + step);
		else
			_rowOffset = _rowOffset > (uint)-step ? _rowOffset - (uint)-step : 0;
		presentFrame();
		if (_rowOffset != targetRowOffset && delay(kScene9170ScrollFrameMillis))
			return;
	}
}

void Scene9170::waitWithAnimations(uint32 millis, byte speakerGroup, bool animateAmbient,
		bool presentChanges) {
	uint32 elapsed = 0;
	uint32 speakerElapsed = 0;
	uint32 effectElapsed = 0;
	while (elapsed < millis && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		bool dirty = false;
		if (speakerElapsed >= kScene9170SpeechFrameMillis) {
			speakerElapsed %= kScene9170SpeechFrameMillis;
			advanceSpeakerIdleAnimation(speakerGroup);
			advanceUpperIdleAnimations(speakerGroup);
			dirty = true;
		}
		if (animateAmbient && effectElapsed >= kScene9170EffectFrameMillis) {
			effectElapsed %= kScene9170EffectFrameMillis;
			advanceLowerRoomAmbience();
			dirty = true;
		}
		if (dirty) {
			composeFrame();
			if (presentChanges)
				presentFrame();
		}

		const uint32 slice = MIN<uint32>(millis - elapsed, 10);
		g_system->delayMillis(slice);
		elapsed += slice;
		speakerElapsed += slice;
		effectElapsed += slice;
	}
}

void Scene9170::fadeInPalette() {
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

void Scene9170::startLowerRoomAmbience() {
	_ambientEffectsEnabled = true;
	_ambientSpeechSampleId = 0;
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	_text.getStageCue(3, 5, textRecordId, continuationCount, _ambientSpeechSampleId);
}

void Scene9170::stopLowerRoomAmbience() {
	_ambientEffectsEnabled = false;
	_ambientSpeechSampleId = 0;
	_ambientSpeech.stop();
	_ambientSound.stop();
}

void Scene9170::advanceLowerRoomAmbience() {
	if (!_ambientEffectsEnabled)
		return;

	_effectFrame = (_effectFrame + 1) % ARRAYSIZE(kScene9170EffectFrameMap);
	_effectDirty = true;
	if (_ambientSpeechSampleId != 0 && !_ambientSpeech.isPlaying())
		_ambientSpeech.playSample(_ambientSpeechSampleId, 50);
	if (!_ambientSound.isPlaying())
		_ambientSound.playSample(0x1b, 100);
}

void Scene9170::runSpeechLine(byte rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue, byte speakerGroup) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!_text.getStageCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	_paletteCurrent[kScene9170BlueSpeechColor * 3] = red;
	_paletteCurrent[kScene9170BlueSpeechColor * 3 + 1] = green;
	_paletteCurrent[kScene9170BlueSpeechColor * 3 + 2] = blue;
	runSpeechCue(textRecordId, continuationCount, voiceSampleId, centerX, topY, speakerGroup);
}

void Scene9170::runSpeechCue(uint16 textRecordId, byte continuationCount, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, byte speakerGroup) {
	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !_skipRequested && !Engine::shouldQuit(); ++part) {
		const Common::String text = _text.largeTextRecord(textRecordId + part);
		if (!text.empty()) {
			_subtitle.visible = true;
			_subtitle.colorIndex = kScene9170BlueSpeechColor;
			wrapSubtitleText(text, centerX, _subtitle.lines);
			calculateSubtitleBounds(centerX, topY);
		}

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _subtitle.lines.size() * 1100);
		uint32 elapsed = 0;
		uint32 speechElapsed = 0;
		uint32 effectElapsed = 0;
		composeFrame();
		presentFrame();
		while (elapsed < duration && !_skipRequested && !Engine::shouldQuit()) {
			if (pollEvents())
				return;

			bool dirty = false;
			if (speechElapsed >= kScene9170SpeechFrameMillis) {
				speechElapsed %= kScene9170SpeechFrameMillis;
				advanceSpeechAnimation(speakerGroup);
				advanceUpperIdleAnimations(speakerGroup);
				dirty = true;
			}
			if (_ambientEffectsEnabled && effectElapsed >= kScene9170EffectFrameMillis) {
				effectElapsed %= kScene9170EffectFrameMillis;
				advanceLowerRoomAmbience();
				dirty = true;
			}
			if (dirty) {
				composeFrame();
				presentFrame();
			}

			const uint32 slice = MIN<uint32>(duration - elapsed, 10);
			g_system->delayMillis(slice);
			elapsed += slice;
			speechElapsed += slice;
			effectElapsed += slice;
		}

		_speech.stop();
		resetSpeakerFrame(speakerGroup);
		clearSubtitle();
		composeFrame();
		presentFrame();
	}
}

void Scene9170::advanceSpeechAnimation(byte speakerGroup) {
	const byte frame = nextTalkingFrame();
	switch (speakerGroup) {
	case 0:
		_upperFrames[0] = frame;
		_upperDirty[0] = true;
		break;
	case 1:
		_upperFrames[1] = frame;
		_upperDirty[1] = true;
		break;
	case 2:
		_upperFrames[2] = frame;
		_upperDirty[2] = true;
		break;
	case 3:
		_lowerFrame = 2 + frame;
		_lowerDirty = true;
		break;
	case 4:
		_lowerFrame = 7 + frame;
		_lowerDirty = true;
		break;
	case 5:
		_upperFrames[0] = _upperFrames[1] = _upperFrames[2] = frame;
		_upperDirty[0] = _upperDirty[1] = _upperDirty[2] = true;
		break;
	default:
		break;
	}
}

void Scene9170::advanceSpeakerIdleAnimation(byte speakerGroup) {
	switch (speakerGroup) {
	case 0:
	case 1:
	case 2:
		if (_upperFrames[speakerGroup] == 4)
			_upperFrames[speakerGroup] = 0;
		else
			_upperFrames[speakerGroup] = _random.getRandomNumber(14) == 0 ? 4 : 0;
		_upperDirty[speakerGroup] = true;
		break;
	case 3:
		if (_lowerFrame == 1)
			_lowerFrame = 0;
		else
			_lowerFrame = _random.getRandomNumber(14) == 0 ? 1 : 0;
		_lowerDirty = true;
		break;
	case 4:
		if (_lowerFrame == 11)
			_lowerFrame = 7;
		else
			_lowerFrame = _random.getRandomNumber(14) == 0 ? 11 : 7;
		_lowerDirty = true;
		break;
	default:
		break;
	}
}

void Scene9170::advanceUpperIdleAnimations(byte speakerGroup) {
	if (!_upperActorsEnabled || speakerGroup > 2)
		return;

	for (byte actor = 0; actor < 3; ++actor) {
		if (actor == speakerGroup)
			continue;
		if (_upperFrames[actor] == 4)
			_upperFrames[actor] = 0;
		else
			_upperFrames[actor] = _random.getRandomNumber(14) == 0 ? 4 : 0;
		_upperDirty[actor] = true;
	}
}

void Scene9170::resetSpeakerFrame(byte speakerGroup) {
	switch (speakerGroup) {
	case 0:
	case 1:
	case 2:
		_upperFrames[speakerGroup] = 0;
		_upperDirty[speakerGroup] = true;
		break;
	case 3:
		_lowerFrame = 2;
		_lowerDirty = true;
		break;
	case 4:
		_lowerFrame = 7;
		_lowerDirty = true;
		break;
	case 5:
		_upperFrames[0] = _upperFrames[1] = _upperFrames[2] = 0;
		_upperDirty[0] = _upperDirty[1] = _upperDirty[2] = true;
		break;
	default:
		break;
	}
}

byte Scene9170::nextTalkingFrame() {
	byte frame = 0;
	do {
		frame = (byte)_random.getRandomNumber(4);
	} while (frame == _lastTalkingFrame);
	_lastTalkingFrame = frame;
	return frame;
}

void Scene9170::runEventOverlayFrames() {
	for (byte frame = 1; frame < ARRAYSIZE(kScene9170EventFrameMap) && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		_eventFrame = frame;
		_eventDirty = true;
		composeFrame();
		presentFrame();
		if (frame + 1 < ARRAYSIZE(kScene9170EventFrameMap) && delay(kScene9170EffectFrameMillis))
			return;
	}
}

void Scene9170::runShake() {
	stopLowerRoomAmbience();
	_sound.playSample(0x1e, 100);
	_shakeActive = true;
	for (uint i = 0; i < 0x32 && !_skipRequested && !Engine::shouldQuit(); ++i) {
		if (pollEvents())
			break;
		_shakeRowOffset = _random.getRandomNumber(2) * 4;
		presentFrame();
	}
	_shakeActive = false;
	_shakeRowOffset = 0;
}

uint Scene9170::presentRowOffset() const {
	return _rowOffset + (_shakeActive ? _shakeRowOffset : 0);
}

void Scene9170::stopAudio() {
	_speech.stop();
	_ambientSpeech.stop();
	_sound.stop();
	_ambientSound.stop();
	_music->stop();
}

void Scene9170::clearSubtitle() {
	_subtitle.visible = false;
	_subtitle.lines.clear();
}

void Scene9170::drawFrameOverlays() {
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
		int y = (int)_subtitle.topY + lineIndex * kScene9170SpeechLineHeight;
		if (_rowOffset == 0x0a0 || _rowOffset == 0x140)
			y -= (int)_rowOffset;
		font->drawString(_screen.surfacePtr(), line, x, y, lineWidth, _subtitle.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene9170::wrapSubtitleText(const Common::String &text, uint16 anchorSceneX,
		Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	const int anchorX = CLIP<int>(anchorSceneX, 10, HollywoodEngine::kScreenWidth - 10);
	const int edgeDistance = MIN<int>(anchorX, HollywoodEngine::kScreenWidth - anchorX);
	uint maxChars = edgeDistance < 0xa0 ? edgeDistance * 0x32 / 0xa0 : 0x32;
	maxChars = MAX<uint>(maxChars, 0x18);
	const uint lineWidthReduction = maxChars < 0x2a ? (maxChars > 0x20 ? 2 : 1) : 3;
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
		maxChars = maxChars > lineWidthReduction ? maxChars - lineWidthReduction : 1;
	}
}

void Scene9170::calculateSubtitleBounds(uint16 anchorCenterX, uint16 anchorTopY) {
	uint maxWidth = 0;
	for (uint lineIndex = 0; lineIndex < _subtitle.lines.size(); ++lineIndex)
		maxWidth = MAX<uint>(maxWidth, subtitleTextWidth(_subtitle.lines[lineIndex]));

	const int width = (int)maxWidth;
	const int halfWidth = width >> 1;
	int centerX = anchorCenterX;
	if (centerX - halfWidth - 1 + width > 0x27e) {
		centerX = 0x27d - halfWidth;
		if ((maxWidth & 1) == 0)
			centerX = 0x27e - halfWidth;
	}
	if (centerX - halfWidth < 1)
		centerX = halfWidth + 1;

	_subtitle.centerX = centerX;
	_subtitle.topY = MAX<int>(1, (int)anchorTopY - _subtitle.lines.size() * kScene9170SpeechLineHeight);
}

uint Scene9170::subtitleTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

} // End of namespace Hollywood
