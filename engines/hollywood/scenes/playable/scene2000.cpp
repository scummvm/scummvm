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

#include "hollywood/scenes/playable/scene2000.h"

#include "common/system.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2000ArchiveName = "RESOURCE.B00";
const char *const kScene2000MusicArchiveName = "RESOURCE.M02";
const char *const kScene2000SoundArchiveName = "RESOURCE.S02";
const uint16 kScene2000MusicCueId = 0x000b;
const uint16 kScene2000NextState = 0x07ee;
const uint32 kScene2000PhaseMillis = 1000;
const uint32 kScene2000PaletteCycleMillis = 75;
const uint32 kScene2000SpriteFrameMillis = 75;
const uint32 kScene2000ClipFrameMillis = 75;
const uint kScene2000EndTick = 0x32;
const uint kScene2000PatchTick = 3;
const uint kScene2000ClipStartTick = 5;
const uint kScene2000BackgroundRefreshTick = 8;
const uint kScene2000SmallSpriteDescriptorCount = 0x1e;
const uint kScene2000ClipDescriptorCount = 0x81;
const uint kScene2000BackgroundRefreshBytes = 0x10000;
const byte kScene2000ClipFinalMapIndex = 0xa0;

const byte kScene2000ClipFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 29, 29, 29, 29, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
	74, 75, 76, 77, 78, 79, 80, 81, 82, 82,
	82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
	82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
	82, 82, 82, 82, 82, 83, 84, 85, 86, 87,
	88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
	98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
	108, 109, 110, 111, 112, 113, 114, 115, 116, 117,
	118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128
};

Scene2000::Scene2000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "scene 2000"),
		_random("scene2000"),
		_presentationSound(),
		_clipMapIndex(0) {
	memset(_spriteStates, 0, sizeof(_spriteStates));
	memset(_spriteDeltas, 0, sizeof(_spriteDeltas));
	_presentationSound.setArchive(Common::Path(kScene2000SoundArchiveName));
}

Scene2000::~Scene2000() {
	_presentationSound.stop();
}

const char *Scene2000::resourceArchiveName() const {
	return kScene2000ArchiveName;
}

const char *Scene2000::musicArchiveName() const {
	return kScene2000MusicArchiveName;
}

uint16 Scene2000::musicCueId() const {
	return kScene2000MusicCueId;
}

uint16 Scene2000::nextState() const {
	return kScene2000NextState;
}

byte Scene2000::activeAudioChapterIndex() const {
	return 2;
}

uint Scene2000::sceneArenaFirstChunk() const {
	return 2;
}

uint Scene2000::sceneArenaLastChunk() const {
	return 4;
}

void Scene2000::adjustPaletteAfterLoad() {
	if (_paletteResource.size() >= 0x300) {
		_paletteResource[0x2fd] = 0;
		_paletteResource[0x2fe] = 0;
		_paletteResource[0x2ff] = 0;
	}
}

void Scene2000::drawInitialFrame() {
	presentFrame();
}

void Scene2000::runPresentation() {
	uint tick = 0;
	uint32 phaseAccumulator = 0;
	uint32 paletteAccumulator = 0;
	uint32 spriteAccumulator = 0;
	uint32 clipAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	bool spriteDirty = false;

	_presentationSound.playSample(0x29, 30);

	while (tick < kScene2000EndTick && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			break;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		phaseAccumulator += delta;
		paletteAccumulator += delta;
		spriteAccumulator += delta;
		clipAccumulator += delta;

		while (paletteAccumulator >= kScene2000PaletteCycleMillis) {
			paletteAccumulator -= kScene2000PaletteCycleMillis;
			rotatePaletteRange(0xf0, 0xf8);
		}

		while (spriteAccumulator >= kScene2000SpriteFrameMillis) {
			spriteAccumulator -= kScene2000SpriteFrameMillis;
			advanceSmallSprites();
			spriteDirty = true;
		}

		while (phaseAccumulator >= kScene2000PhaseMillis && tick < kScene2000EndTick) {
			phaseAccumulator -= kScene2000PhaseMillis;
			++tick;

			if (tick == kScene2000PatchTick) {
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[3],
					_sceneFramebuffer.managedSurface());
			}

			if (tick == kScene2000BackgroundRefreshTick)
				memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(),
					kScene2000BackgroundRefreshBytes);

		}

		const byte previousClipMapIndex = _clipMapIndex;
		if (tick < kScene2000ClipStartTick) {
			clipAccumulator %= kScene2000ClipFrameMillis;
		} else {
			while (clipAccumulator >= kScene2000ClipFrameMillis &&
					_clipMapIndex < kScene2000ClipFinalMapIndex) {
				clipAccumulator -= kScene2000ClipFrameMillis;
				++_clipMapIndex;
				if (_clipMapIndex == kScene2000ClipFinalMapIndex)
					tick = kScene2000EndTick - 1;
			}
		}

		if (!spriteDirty && previousClipMapIndex != _clipMapIndex) {
			advanceSmallSprites();
			spriteDirty = true;
		}
		if (spriteDirty || previousClipMapIndex != _clipMapIndex) {
			drawPresentationFrame(previousClipMapIndex);
			spriteDirty = false;
		}

		g_system->delayMillis(10);
	}

	_presentationSound.stop();
}

void Scene2000::advanceSmallSprites() {
	for (uint i = 0; i < ARRAYSIZE(_spriteStates); ++i) {
		_spriteStates[i] = (byte)(_spriteStates[i] + _spriteDeltas[i]);
		if (_spriteStates[i] == 0)
			_spriteDeltas[i] = _random.getRandomNumber(14) == 0 ? 1 : 0;
		if (_spriteStates[i] == 4)
			_spriteDeltas[i] = -1;
	}
}

void Scene2000::drawPresentationFrame(byte previousClipMapIndex) {
	restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[2], 0,
		kScene2000SmallSpriteDescriptorCount, 0, _baseFramebuffer.surface(),
		_sceneFramebuffer.surface());

	for (uint i = 0; i < ARRAYSIZE(_spriteStates); ++i) {
		const byte descriptor = (byte)(_spriteStates[i] + i * 5);
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[2], 0,
			kScene2000SmallSpriteDescriptorCount, descriptor, _sceneFramebuffer.managedSurface());
	}
	for (uint clipMapIndex = previousClipMapIndex + 1; clipMapIndex <= _clipMapIndex; ++clipMapIndex) {
		const byte frame = kScene2000ClipFrameMap[MIN<uint>(clipMapIndex,
			ARRAYSIZE(kScene2000ClipFrameMap) - 1)];
		drawClipFrameDelta(4, kScene2000ClipDescriptorCount, frame);
	}

	presentFrame();
}

} // End of namespace Hollywood
