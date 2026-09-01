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

#include "hollywood/scenes/intro/scene9110.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kI11ArchiveName = "RESOURCE.I11";
const char *const kStage003ArchiveName = "RESOURCE.003";
const uint16 kScene9110MusicCueId = 0x000c;
const byte kPrimarySpeechTextColor = 0xfb;
const uint kScene9110MouthInterval = 125;
const uint kScene9110IdleInterval = 100;
const uint kScene9110CycleInterval = 90;
const uint kScene9110Chunk3Interval = 75;
const uint kScene9110MusicFadeInterval = 50;

Scene9110::Scene9110(HollywoodEngine *vm) :
		PresentationScene(vm, "Scene 9110"),
		_music(vm->introMusic()),
		_speech(vm->getLanguage()),
		_text(),
		_random("hollywood_scene9110"),
		_mouthAccumulator(0),
		_chunk3Accumulator(0),
		_idleAccumulator(0),
		_cycleAccumulator(0),
		_musicFadeAccumulator(0),
		_chunk2MouthFrame(2),
		_chunk2IdleFrame(0),
		_chunk2CycleFrame(6),
		_chunk3Frame(0),
		_chunk2CycleDirection(0),
		_lastMouthVariant(0xff) {
	_paletteResource.resize(kPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
}

bool Scene9110::play() {
	if (!load())
		return false;

	initializeCompositeState();
	drawCompositeToFramebuffer();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	if (!_music->isPlaying())
		_music->playMusicCue(kScene9110MusicCueId, 100);

	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		if (pollEvents())
			break;
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
	}

	if (!_skipRequested && !Engine::shouldQuit())
		runSpeechSequence();

	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 0x14) {
		if (pollEvents())
			break;
		clearSceneFramebufferBand(sweepOffset, 0x14);
		presentFrame();
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	stopAudio();
	return true;
}

bool Scene9110::load() {
	return loadResourceI11Assets() &&
		_text.loadStage(kStage003ArchiveName, _debugName, kStage911Index);
}

bool Scene9110::loadResourceI11Assets() {
	if (!_resources.loadChunkTable(kI11ArchiveName))
		return false;

	for (uint i = 0; i < kI11RequiredChunkCount; ++i) {
		if (!_resources.validateChunk(kI11ArchiveName, _debugName, i))
			return false;
	}

	if (!loadFixedChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(1, _paletteResource, kPaletteSize))
		return false;

	uint32 resourceArenaSize = 0;
	for (uint i = 2; i < kI11RequiredChunkCount; ++i)
		resourceArenaSize += _resources.chunkTable.sizes[i];

	_resources.allocateArena(resourceArenaSize);

	for (uint i = 2; i < kI11RequiredChunkCount; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memset(_savedFramebuffer.data(), 0, _savedFramebuffer.size());
	return true;
}

void Scene9110::initializeCompositeState() {
	_mouthAccumulator = kScene9110MouthInterval;
	_chunk3Accumulator = kScene9110Chunk3Interval;
	_idleAccumulator = kScene9110IdleInterval;
	_cycleAccumulator = kScene9110CycleInterval;
	_musicFadeAccumulator = kScene9110MusicFadeInterval;
	_chunk2MouthFrame = 2;
	_chunk2IdleFrame = 0;
	_chunk2CycleFrame = 6;
	_chunk3Frame = 0;
	_chunk2CycleDirection = 0;
	_lastMouthVariant = 0xff;
	clearSubtitle();
}

void Scene9110::runSpeechSequence() {
	const SpeechTextStyle rightSpeech = { 0x193, 0x35, kPrimarySpeechTextColor, 0x3f, 0x28, 0x32, true };
	const SpeechTextStyle topSpeech = { 0x140, 0x14, kPrimarySpeechTextColor, 0x0c, 0x20, 0x32, true };
	const SpeechStep steps[] = {
		{ 2, 0, rightSpeech, kWaitChunk2MouthMotion },
		{ 1, 0, topSpeech, kWaitChunk3Animation },
		{ 2, 1, rightSpeech, kWaitChunk2MouthMotion },
		{ 1, 1, topSpeech, kWaitChunk3Animation },
		{ 2, 2, rightSpeech, kWaitChunk2MouthMotion }
	};

	for (uint i = 0; i < ARRAYSIZE(steps) && !_skipRequested && !Engine::shouldQuit(); ++i)
		runSpeechStep(steps[i]);
}

void Scene9110::runSpeechStep(const SpeechStep &step) {
	const SceneSpeechCue popup = _text.stageCue(step.rowIndex, step.frameIndex);
	const uint16 sampleId = popup.voiceSampleId;

	beginSubtitle(popup, step.speechTextStyle);
	presentComposite();

	const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
	const uint32 fallbackMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
	waitForSpeechOrDelay(fallbackMillis, step.waitMode);
	_speech.stop();

	if (step.waitMode == kWaitChunk2MouthMotion)
		_chunk2MouthFrame = 2;

	clearSubtitle();
	if (!_skipRequested && !Engine::shouldQuit())
		presentComposite();
}

void Scene9110::waitForSpeechOrDelay(uint32 fallbackMillis, SpeechWaitMode waitMode) {
	uint32 elapsedTotal = 0;
	uint32 lastFrameMillis = g_system->getMillis();
	while (!_skipRequested && !Engine::shouldQuit()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsedTotal >= fallbackMillis)
			break;

		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		elapsedTotal += elapsed;

		if (advanceAnimationTimers(elapsed, waitMode))
			presentComposite();

		g_system->delayMillis(1);
	}
}

bool Scene9110::advanceAnimationTimers(uint32 millis, SpeechWaitMode waitMode) {
	bool dirty = false;

	_musicFadeAccumulator += millis;
	if (_musicFadeAccumulator >= kScene9110MusicFadeInterval)
		_musicFadeAccumulator %= kScene9110MusicFadeInterval;

	_chunk3Accumulator += millis;
	if (_chunk3Accumulator >= kScene9110Chunk3Interval) {
		_chunk3Accumulator %= kScene9110Chunk3Interval;
		_chunk3Frame = (byte)_random.getRandomNumber(7);
		dirty = true;
	}

	_idleAccumulator += millis;
	if (_idleAccumulator >= kScene9110IdleInterval) {
		_idleAccumulator %= kScene9110IdleInterval;
		if (_chunk2IdleFrame == 1) {
			_chunk2IdleFrame = 0;
			dirty = true;
		} else if (_random.getRandomNumber(14) == 0) {
			_chunk2IdleFrame = 1;
			dirty = true;
		}
	}

	_cycleAccumulator += millis;
	if (_cycleAccumulator >= kScene9110CycleInterval) {
		_cycleAccumulator %= kScene9110CycleInterval;
		if (_chunk2CycleDirection == 0) {
			if (_chunk2CycleFrame < 14)
				_chunk2CycleFrame++;
			else
				_chunk2CycleFrame = 6;
		} else if (_chunk2CycleFrame < 7) {
			_chunk2CycleFrame = 14;
		} else {
			_chunk2CycleFrame--;
		}
		_chunk2CycleDirection = (byte)_random.getRandomNumber(1);
		dirty = true;
	}

	if (waitMode == kWaitChunk2MouthMotion) {
		_mouthAccumulator += millis;
		if (_mouthAccumulator >= kScene9110MouthInterval) {
			_mouthAccumulator %= kScene9110MouthInterval;
			_chunk2MouthFrame = (byte)(nextMouthFrameVariant() + 2);
			dirty = true;
		}
	}

	return dirty;
}

void Scene9110::drawCompositeToFramebuffer() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	drawDescriptorFrame(3, kI11Chunk3DescriptorCount, _chunk3Frame);
	drawDescriptorFrame(2, kI11Chunk2DescriptorCount, _chunk2MouthFrame);
	drawDescriptorFrame(2, kI11Chunk2DescriptorCount, _chunk2IdleFrame);
	drawDescriptorFrame(2, kI11Chunk2DescriptorCount, _chunk2CycleFrame);
}

void Scene9110::presentComposite() {
	drawCompositeToFramebuffer();
	presentFrame();
}

void Scene9110::drawDescriptorFrame(byte localChunkIndex, byte descriptorCount, byte descriptorIndex) {
	if (localChunkIndex >= SceneResources::kResourceChunkCount)
		return;

	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[localChunkIndex], 0,
		descriptorCount, descriptorIndex, _sceneFramebuffer.surface());
}

byte Scene9110::nextMouthFrameVariant() {
	byte nextFrame = 0;
	do {
		nextFrame = (byte)_random.getRandomNumber(3);
	} while (nextFrame == _lastMouthVariant);

	_lastMouthVariant = nextFrame;
	return nextFrame;
}

void Scene9110::beginSubtitle(const SceneSpeechCue &popup, const SpeechTextStyle &speechTextStyle) {
	clearSubtitle();

	const Common::String text = _text.largeTextRecord(popup.textRecordId);
	if (text.empty()) {
		debugC(2, kDebugScene, "Skipping empty subtitle text record %u",
			popup.textRecordId);
		return;
	}

	if (!showAnchoredSubtitle(text, speechTextStyle.colorIndex,
			speechTextStyle.centerX, speechTextStyle.topY))
		return;

	if (speechTextStyle.updatePalette) {
		_paletteCurrent[speechTextStyle.colorIndex * 3] = speechTextStyle.red;
		_paletteCurrent[speechTextStyle.colorIndex * 3 + 1] = speechTextStyle.green;
		_paletteCurrent[speechTextStyle.colorIndex * 3 + 2] = speechTextStyle.blue;
	}

}

void Scene9110::stopAudio() {
	clearSubtitle();
	_speech.stop();
	_music->stop();
}

} // End of namespace Hollywood
