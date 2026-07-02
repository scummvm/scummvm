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

#include "hollywood/scenes/playable/scene8000.h"

#include "common/system.h"

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene8000ArchiveName = "RESOURCE.H00";
const char *const kScene8000MusicArchiveName = "RESOURCE.M08";
const char *const kScene8000SoundArchiveName = "RESOURCE.S08";
const uint16 kScene8000MusicCueId = 0x000b;
const uint16 kScene8000NextState = 0x1f4a;
const uint32 kScene8000FrameMillis = 75;
const uint32 kScene8000MainSpriteMillis = 60;
const uint32 kScene8000SecondarySpriteMillis = 1000;
const uint kScene8000EndTick = 0x31;
const uint kScene8000PatchTick = 3;
const uint kScene8000BackgroundRefreshTick = 8;
const uint kScene8000MainDescriptorCount = 0x18;
const uint kScene8000SecondaryDescriptorCount = 0x2a;

const byte kScene8000MainFrameMap[] = {
	0, 23, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 10, 9, 8
};

const byte kScene8000SecondaryFrameMap[] = {
	41, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40
};

Scene8000::Scene8000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "scene 8000"),
		_soundBank0(),
		_random("scene8000"),
		_mainFrame(0),
		_mainState(0),
		_mainRepeatCount(0),
		_secondaryFrame(0),
		_tick(0),
		_patchVisible(false),
		_secondaryVisible(false) {
	_soundBank0.setArchive(Common::Path(kScene8000SoundArchiveName));
}

Scene8000::~Scene8000() {
	_soundBank0.stop();
}

const char *Scene8000::resourceArchiveName() const {
	return kScene8000ArchiveName;
}

const char *Scene8000::musicArchiveName() const {
	return kScene8000MusicArchiveName;
}

uint16 Scene8000::musicCueId() const {
	return kScene8000MusicCueId;
}

uint16 Scene8000::nextState() const {
	return kScene8000NextState;
}

byte Scene8000::activeAudioChapterIndex() const {
	return 8;
}

uint Scene8000::sceneArenaFirstChunk() const {
	return 2;
}

uint Scene8000::sceneArenaLastChunk() const {
	return 4;
}

void Scene8000::drawInitialFrame() {
	drawPresentationFrame();
}

void Scene8000::runPresentation() {
	_vm->gameState().currentAmbientMusicCueId = kScene8000MusicCueId;
	_soundBank0.playSample(0x1e, 100, true);

	uint32 frameAccumulator = 0;
	uint32 mainAccumulator = 0;
	uint32 secondaryAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	bool frameDirty = true;

	while (_tick < kScene8000EndTick && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			break;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		mainAccumulator += delta;
		secondaryAccumulator += delta;

		while (frameAccumulator >= kScene8000FrameMillis && _tick < kScene8000EndTick) {
			frameAccumulator -= kScene8000FrameMillis;
			++_tick;
			if (_tick == kScene8000PatchTick) {
				_patchVisible = true;
				_secondaryVisible = true;
				_soundBank0.stop();
				_soundBank0.playSample(0x1d, 50);
			}
			if (_tick == kScene8000BackgroundRefreshTick) {
				memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
				frameDirty = true;
			}
			frameDirty = true;
		}

		while (mainAccumulator >= kScene8000MainSpriteMillis) {
			mainAccumulator -= kScene8000MainSpriteMillis;
			advanceMainSprite();
			frameDirty = true;
		}

		while (_secondaryVisible && secondaryAccumulator >= kScene8000SecondarySpriteMillis) {
			secondaryAccumulator -= kScene8000SecondarySpriteMillis;
			if (_secondaryFrame + 1 < ARRAYSIZE(kScene8000SecondaryFrameMap))
				++_secondaryFrame;
			frameDirty = true;
		}

		if (frameDirty) {
			drawPresentationFrame();
			frameDirty = false;
		}

		g_system->delayMillis(10);
	}

	_soundBank0.stop();
}

void Scene8000::advanceMainSprite() {
	if (_mainState == 0) {
		if (_random.getRandomNumber(49) == 0) {
			_mainFrame = 9;
			_mainState = 3;
			return;
		}
		if (_random.getRandomNumber(49) == 0) {
			_mainFrame = 2;
			_mainState = 1;
			_mainRepeatCount = (byte)_random.getRandomNumber(5);
			return;
		}
		if (_random.getRandomNumber(49) == 0) {
			_mainFrame = 8;
			_mainState = 2;
			_mainRepeatCount = (byte)_random.getRandomNumber(5);
			return;
		}
		return;
	}

	if (_mainState == 1) {
		if (_mainFrame == 8) {
			if (_mainRepeatCount == 0) {
				_mainFrame = 0;
				_mainState = 0;
			} else {
				_mainFrame = 2;
				--_mainRepeatCount;
			}
			return;
		}
		++_mainFrame;
		return;
	}

	if (_mainState == 2) {
		if (_mainFrame == 2) {
			if (_mainRepeatCount == 0) {
				_mainFrame = 0;
				_mainState = 0;
			} else {
				_mainFrame = 8;
				--_mainRepeatCount;
			}
			return;
		}
		--_mainFrame;
		return;
	}

	if (_mainFrame == 0x0f) {
		_mainFrame = 0;
		_mainState = 0;
	} else {
		++_mainFrame;
	}
}

void Scene8000::drawPresentationFrame() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	if (_patchVisible)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2], _sceneFramebuffer.managedSurface());

	const byte mainFrame = kScene8000MainFrameMap[MIN<uint>(_mainFrame, ARRAYSIZE(kScene8000MainFrameMap) - 1)];
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[3], 0,
		kScene8000MainDescriptorCount, mainFrame, _sceneFramebuffer.managedSurface());
	drawSecondarySpriteIfVisible();
	presentFrame();
}

void Scene8000::drawSecondarySpriteIfVisible() {
	if (!_secondaryVisible)
		return;

	const byte frame = kScene8000SecondaryFrameMap[MIN<uint>(_secondaryFrame, ARRAYSIZE(kScene8000SecondaryFrameMap) - 1)];
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[4], 0,
		kScene8000SecondaryDescriptorCount, frame, _sceneFramebuffer.managedSurface());
}

} // End of namespace Hollywood
