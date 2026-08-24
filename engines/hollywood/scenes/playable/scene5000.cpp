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

#include "hollywood/scenes/playable/scene5000.h"

#include "common/system.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene5000ArchiveName = "RESOURCE.E00";
const char *const kScene5000MusicArchiveName = "RESOURCE.M05";
const char *const kScene5000SoundArchiveName = "RESOURCE.S05";
const uint16 kScene5000MusicCueId = 0x000b;
const uint16 kScene5000NextState = 0x1392;
const uint16 kScene5000ViewportXOffset = 0x00c0;
const uint32 kScene5000PhaseMillis = 1000;
const uint32 kScene5000SpriteFrameMillis = 75;
const uint32 kScene5000ClipFrameMillis = 60;
const uint kScene5000EndTick = 0x32;
const uint kScene5000NearEndTick = 0x31;
const uint kScene5000PatchTick = 3;
const uint kScene5000BackgroundRefreshTick = 8;
const uint kScene5000BackgroundRefreshBytes = 0x10000;
const uint kScene5000SpriteDescriptorCount = 10;
const uint kScene5000ClipDescriptorCount = 0xa8;
const byte kScene5000ClipFinalFrame = 0xa7;
const int kScene5000BeforeFirstClipFrame = -1;

const byte kScene5000SpriteFrameMap[] = {
	0, 1, 1, 2, 3, 4, 5, 4, 3, 2,
	1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

Scene5000::Scene5000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "scene 5000"),
		_ambientSound(),
		_random("hollywood_scene5000"),
		_lastAmbientCue(0xff),
		_spriteFrame(0),
		_spriteMode(0),
		_clipFrame(0) {
	_ambientSound.setArchive(Common::Path(kScene5000SoundArchiveName));
}

Scene5000::~Scene5000() {
	_ambientSound.stop();
}

const char *Scene5000::resourceArchiveName() const {
	return kScene5000ArchiveName;
}

const char *Scene5000::musicArchiveName() const {
	return kScene5000MusicArchiveName;
}

uint16 Scene5000::musicCueId() const {
	return kScene5000MusicCueId;
}

uint16 Scene5000::nextState() const {
	return kScene5000NextState;
}

byte Scene5000::activeAudioChapterIndex() const {
	return 5;
}

uint Scene5000::sceneArenaFirstChunk() const {
	return 2;
}

uint Scene5000::sceneArenaLastChunk() const {
	return 4;
}

uint16 Scene5000::sceneViewportXOffset() const {
	return kScene5000ViewportXOffset;
}

void Scene5000::drawInitialFrame() {
	drawPresentationFrame(true, kScene5000BeforeFirstClipFrame);
}

void Scene5000::runPresentation() {
	uint tick = 0;
	uint32 phaseAccumulator = 0;
	uint32 spriteAccumulator = kScene5000SpriteFrameMillis;
	uint32 clipAccumulator = kScene5000ClipFrameMillis;
	uint32 lastMillis = g_system->getMillis();
	bool spriteDirty = false;

	while (tick < kScene5000EndTick && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		phaseAccumulator += delta;
		spriteAccumulator += delta;
		clipAccumulator += delta;

		updateAmbientSound();

		while (spriteAccumulator >= kScene5000SpriteFrameMillis) {
			spriteAccumulator -= kScene5000SpriteFrameMillis;
			advanceSpriteFrame();
			spriteDirty = true;
		}

		const byte previousClipFrame = _clipFrame;
		while (clipAccumulator >= kScene5000ClipFrameMillis &&
				_clipFrame < kScene5000ClipFinalFrame) {
			clipAccumulator -= kScene5000ClipFrameMillis;
			++_clipFrame;
			if (_clipFrame == kScene5000ClipFinalFrame)
				tick = kScene5000NearEndTick;
		}

		while (phaseAccumulator >= kScene5000PhaseMillis && tick < kScene5000EndTick) {
			phaseAccumulator -= kScene5000PhaseMillis;
			++tick;

			if (tick == kScene5000PatchTick)
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2],
					_sceneFramebuffer.managedSurface());
			if (tick == kScene5000BackgroundRefreshTick)
				memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(),
					kScene5000BackgroundRefreshBytes);
		}

		if (spriteDirty || previousClipFrame != _clipFrame) {
			drawPresentationFrame(spriteDirty, previousClipFrame);
			spriteDirty = false;
		}

		g_system->delayMillis(10);
	}

	_ambientSound.stop();
}

void Scene5000::drawPresentationFrame(bool spriteDirty, int previousClipFrame) {
	const uint mapIndex = MIN<uint>(_spriteFrame, ARRAYSIZE(kScene5000SpriteFrameMap) - 1);
	const byte descriptor = kScene5000SpriteFrameMap[mapIndex];
	if (spriteDirty) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[3], 0,
			kScene5000SpriteDescriptorCount, descriptor, _baseFramebuffer.surface(),
			_sceneFramebuffer.surface());
	}
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[3], 0,
		kScene5000SpriteDescriptorCount, descriptor, _sceneFramebuffer.managedSurface());
	for (int clipFrame = previousClipFrame + 1; clipFrame <= _clipFrame; ++clipFrame)
		drawClipFrameDelta(4, kScene5000ClipDescriptorCount, clipFrame);
	presentFrame();
}

void Scene5000::advanceSpriteFrame() {
	if (_spriteMode == 1) {
		if (_spriteFrame < 0x1d) {
			++_spriteFrame;
			return;
		}
		_spriteMode = 0;
		_spriteFrame = 0;
		return;
	}

	if (_spriteMode == 2) {
		if (_spriteFrame < 0x0b) {
			++_spriteFrame;
			return;
		}
		_spriteMode = 0;
		_spriteFrame = 0;
		return;
	}

	if (_spriteFrame != 0) {
		_spriteFrame = 0;
		return;
	}
	if (_random.getRandomNumber(14) == 0) {
		_spriteFrame = 1;
		return;
	}
	if (_random.getRandomNumber(39) != 0)
		return;

	if (_random.getRandomBit()) {
		_spriteMode = 1;
		_spriteFrame = 0x0c;
	} else {
		_spriteMode = 2;
		_spriteFrame = 2;
	}
}

void Scene5000::updateAmbientSound() {
	if (_ambientSound.isPlaying())
		return;

	byte cue = _lastAmbientCue;
	while (cue == _lastAmbientCue)
		cue = (byte)(0x25 + _random.getRandomNumber(2));
	_lastAmbientCue = cue;
	_ambientSound.playSample(cue, 20);
}

} // End of namespace Hollywood
