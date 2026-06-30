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

#include "hollywood/scenes/playable/scene6000.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene6000ArchiveName = "RESOURCE.F00";
const char *const kScene6000MusicArchiveName = "RESOURCE.M06";
const uint16 kScene6000MusicCueId = 0x000b;
const uint16 kScene6000NextState = 0x177a;
const uint32 kScene6000FrameTickMillis = 75;
const uint32 kScene6000SpriteTickMillis = 1000;
const uint kScene6000SpriteDescriptorCount = 0x2b;
const uint kScene6000EndTick = 0x32;

const byte kScene6000SpriteFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42
};

Scene6000::Scene6000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "scene 6000"),
		_spriteFrameIndex(0),
		_previousSpriteDescriptor(0),
		_hasPreviousSpriteDescriptor(false) {
}

const char *Scene6000::resourceArchiveName() const {
	return kScene6000ArchiveName;
}

const char *Scene6000::musicArchiveName() const {
	return kScene6000MusicArchiveName;
}

uint16 Scene6000::musicCueId() const {
	return kScene6000MusicCueId;
}

uint16 Scene6000::nextState() const {
	return kScene6000NextState;
}

byte Scene6000::activeAudioChapterIndex() const {
	return 6;
}

uint Scene6000::sceneArenaFirstChunk() const {
	return 2;
}

uint Scene6000::sceneArenaLastChunk() const {
	return 3;
}

void Scene6000::adjustPaletteAfterLoad() {
	if (_paletteResource.size() >= 0x300) {
		_paletteResource[0x2fd] = 0x3f;
		_paletteResource[0x2fe] = 0x3f;
		_paletteResource[0x2ff] = 0x3f;
	}
}

void Scene6000::runPresentation() {
	uint tick = 0;
	uint32 frameAccumulator = 0;
	uint32 spriteAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	bool spriteVisible = false;
	bool spriteDirty = true;

	while (tick < kScene6000EndTick && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		spriteAccumulator += delta;

		while (frameAccumulator >= kScene6000FrameTickMillis && tick < kScene6000EndTick) {
			frameAccumulator -= kScene6000FrameTickMillis;
			++tick;
			if (tick == 3)
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2], _sceneFramebuffer.managedSurface());
			if (tick == 5) {
				spriteVisible = true;
				spriteDirty = true;
			}
		}

		while (spriteVisible && spriteAccumulator >= kScene6000SpriteTickMillis) {
			spriteAccumulator -= kScene6000SpriteTickMillis;
			if (_spriteFrameIndex + 1 < ARRAYSIZE(kScene6000SpriteFrameMap)) {
				++_spriteFrameIndex;
				spriteDirty = true;
			}
		}

		if (spriteDirty) {
			drawAnimatedSpriteFrame(spriteVisible);
			spriteDirty = false;
		}
		g_system->delayMillis(10);
	}
}

void Scene6000::drawAnimatedSpriteFrame(bool drawSprite) {
	if (_hasPreviousSpriteDescriptor) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[3], 0,
			kScene6000SpriteDescriptorCount, _previousSpriteDescriptor,
			_baseFramebuffer.surface(), _sceneFramebuffer.surface());
	}

	const uint mapIndex = MIN<uint>(_spriteFrameIndex, ARRAYSIZE(kScene6000SpriteFrameMap) - 1);
	const uint16 descriptor = kScene6000SpriteFrameMap[mapIndex];
	restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[3], 0,
		kScene6000SpriteDescriptorCount, descriptor,
		_baseFramebuffer.surface(), _sceneFramebuffer.surface());

	if (drawSprite)
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[3], 0,
			kScene6000SpriteDescriptorCount, descriptor, _sceneFramebuffer.surface());

	_previousSpriteDescriptor = descriptor;
	_hasPreviousSpriteDescriptor = true;
	presentFrame();
}

} // End of namespace Hollywood
