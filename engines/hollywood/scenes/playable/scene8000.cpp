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
const uint32 kScene8000SecondMillis = 1000;
const uint32 kScene8000PaletteCycleMillis = 300;
const uint32 kScene8000MainSpriteMillis = 75;
const uint32 kScene8000SecondarySpriteMillis = 60;
const uint kScene8000EndTick = 0x31;
const uint kScene8000PatchTick = 3;
const uint kScene8000BackgroundRefreshTick = 8;
const byte kScene8000SecondaryCompleteFrame = 0x2a;
const uint kScene8000NearEndTickAfterSecondaryComplete = 0x2f;
const uint kScene8000MainDescriptorCount = 0x18;
const uint kScene8000SecondaryDescriptorCount = 0x2a;
const uint kScene8000BackgroundRefreshBytes = 0x10000;

const byte kScene8000MainFrameMap[] = {
	0, 23, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 10, 9, 8
};

const byte kScene8000SecondaryFrameMap[] = {
	41, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41
};

Scene8000::Scene8000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "scene 8000"),
		_backgroundSound(),
		_secondarySound(),
		_random("scene8000"),
		_mainFrame(0),
		_mainState(0),
		_mainRepeatCount(0),
		_secondaryFrame(0),
		_tick(0),
		_secondaryVisible(false) {
	_backgroundSound.setArchive(Common::Path(kScene8000SoundArchiveName));
	_secondarySound.setArchive(Common::Path(kScene8000SoundArchiveName));
}

Scene8000::~Scene8000() {
	_backgroundSound.stop();
	_secondarySound.stop();
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
	drawPresentationFrame(true, false);
	_backgroundSound.playSample(0x1e, 100);
}

void Scene8000::runPresentation() {
	_vm->gameState().currentAmbientMusicCueId = kScene8000MusicCueId;

	uint32 secondAccumulator = 0;
	uint32 paletteAccumulator = 0;
	uint32 mainAccumulator = 0;
	uint32 secondaryAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	bool mainDirty = false;
	bool secondaryDirty = false;

	while (_tick < kScene8000EndTick && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			break;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		secondAccumulator += delta;
		paletteAccumulator += delta;
		mainAccumulator += delta;
		if (_secondaryVisible)
			secondaryAccumulator += delta;

		while (paletteAccumulator >= kScene8000PaletteCycleMillis) {
			paletteAccumulator -= kScene8000PaletteCycleMillis;
			rotatePaletteRange(0xd0, 0xe7);
		}

		while (mainAccumulator >= kScene8000MainSpriteMillis) {
			mainAccumulator -= kScene8000MainSpriteMillis;
			mainDirty |= advanceMainSprite();
		}

		while (_secondaryVisible && secondaryAccumulator >= kScene8000SecondarySpriteMillis) {
			secondaryAccumulator -= kScene8000SecondarySpriteMillis;
			if (_secondaryFrame == 0)
				_secondarySound.playSample(0x1d, 50);

			if (_secondaryFrame < kScene8000SecondaryCompleteFrame) {
				++_secondaryFrame;
				if (_secondaryFrame == kScene8000SecondaryCompleteFrame)
					_tick = kScene8000NearEndTickAfterSecondaryComplete;
				secondaryDirty = true;
			}
		}

		while (secondAccumulator >= kScene8000SecondMillis && _tick < kScene8000EndTick) {
			secondAccumulator -= kScene8000SecondMillis;
			++_tick;
			if (_tick == kScene8000PatchTick) {
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2],
					_sceneFramebuffer.managedSurface());
				_secondaryVisible = true;
				secondaryAccumulator = kScene8000SecondarySpriteMillis;
			}
			if (_tick == kScene8000BackgroundRefreshTick)
				memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(),
					kScene8000BackgroundRefreshBytes);
		}

		if (mainDirty || secondaryDirty) {
			drawPresentationFrame(mainDirty, secondaryDirty);
			mainDirty = false;
			secondaryDirty = false;
		}

		g_system->delayMillis(10);
	}

	_backgroundSound.stop();
	_secondarySound.stop();
}

bool Scene8000::advanceMainSprite() {
	if (_mainState == 0) {
		if (_random.getRandomNumber(49) == 0) {
			_mainFrame = 9;
			_mainState = 3;
			return true;
		}
		if (_random.getRandomNumber(49) == 0) {
			_mainFrame = 2;
			_mainState = 1;
			_mainRepeatCount = (byte)_random.getRandomNumber(5);
			return true;
		}
		if (_random.getRandomNumber(49) == 0) {
			_mainFrame = 8;
			_mainState = 2;
			_mainRepeatCount = (byte)_random.getRandomNumber(5);
			return true;
		}
		return false;
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
			return true;
		}
		++_mainFrame;
		return true;
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
			return true;
		}
		--_mainFrame;
		return true;
	}

	if (_mainFrame == 0x0f) {
		_mainFrame = 0;
		_mainState = 0;
	} else {
		++_mainFrame;
	}
	return true;
}

void Scene8000::drawPresentationFrame(bool mainDirty, bool secondaryDirty) {
	const byte mainFrame = kScene8000MainFrameMap[MIN<uint>(_mainFrame, ARRAYSIZE(kScene8000MainFrameMap) - 1)];
	if (mainDirty) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[3], 0,
			kScene8000MainDescriptorCount, mainFrame, _baseFramebuffer.surface(),
			_sceneFramebuffer.surface());
	}
	if (secondaryDirty) {
		const byte secondaryFrame = kScene8000SecondaryFrameMap[MIN<uint>(_secondaryFrame,
			ARRAYSIZE(kScene8000SecondaryFrameMap) - 1)];
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[4], 0,
			kScene8000SecondaryDescriptorCount, secondaryFrame, _baseFramebuffer.surface(),
			_sceneFramebuffer.surface());
	}
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
