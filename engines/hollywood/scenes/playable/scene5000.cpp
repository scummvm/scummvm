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

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene5000ArchiveName = "RESOURCE.E00";
const char *const kScene5000MusicArchiveName = "RESOURCE.M05";
const uint16 kScene5000MusicCueId = 0x000b;
const uint16 kScene5000NextState = 0x1392;
const uint16 kScene5000ViewportXOffset = 0x00c0;
const uint32 kScene5000FrameMillis = 75;
const uint kScene5000EndTick = 0x32;
const uint kScene5000PatchTick = 3;
const uint kScene5000BackgroundRefreshTick = 8;
const uint kScene5000SpriteDescriptorCount = 10;
const uint kScene5000ClipDescriptorCount = 0xa8;
const byte kScene5000ClipFinalFrame = 0xa7;

const byte kScene5000SpriteFrameMap[] = {
	0, 1, 1, 2, 3, 4, 5, 4, 3, 2,
	1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

Scene5000::Scene5000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "scene 5000"),
		_presentationBackground(),
		_random("hollywood_scene5000"),
		_spriteFrame(0),
		_spriteMode(0),
		_spriteRepeatCount(0),
		_clipFrame(0) {
	_presentationBackground.resize(kFrameBufferSize);
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
	resetPresentationBackground();
	drawPresentationFrame(true);
}

void Scene5000::runPresentation() {
	uint tick = 0;
	uint32 frameAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	bool frameDirty = true;
	bool patchVisible = false;

	while (tick < kScene5000EndTick && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;

		while (frameAccumulator >= kScene5000FrameMillis && tick < kScene5000EndTick) {
			frameAccumulator -= kScene5000FrameMillis;
			++tick;

			if (tick == kScene5000PatchTick && !patchVisible) {
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2], _presentationBackground.managedSurface());
				patchVisible = true;
				frameDirty = true;
			}
			if (tick == kScene5000BackgroundRefreshTick) {
				memcpy(_presentationBackground.data(), _baseFramebuffer.data(), 0x10000);
				if (patchVisible)
					drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2], _presentationBackground.managedSurface());
				frameDirty = true;
			}
			if (_clipFrame < kScene5000ClipFinalFrame) {
				++_clipFrame;
				drawClipFrameDeltaToBackground(_clipFrame);
				frameDirty = true;
			}

			advanceSpriteFrame();
			frameDirty = true;
		}

		if (frameDirty) {
			drawPresentationFrame(true);
			frameDirty = false;
		}
		if (_clipFrame >= kScene5000ClipFinalFrame)
			return;

		g_system->delayMillis(10);
	}
}

void Scene5000::resetPresentationBackground() {
	memcpy(_presentationBackground.data(), _baseFramebuffer.data(), _presentationBackground.size());
}

void Scene5000::drawPresentationFrame(bool drawSprite) {
	memcpy(_sceneFramebuffer.data(), _presentationBackground.data(), _sceneFramebuffer.size());
	if (drawSprite) {
		const uint mapIndex = MIN<uint>(_spriteFrame, ARRAYSIZE(kScene5000SpriteFrameMap) - 1);
		const byte descriptor = kScene5000SpriteFrameMap[mapIndex];
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[3], 0,
			kScene5000SpriteDescriptorCount, descriptor, _sceneFramebuffer.managedSurface());
	}
	presentFrame();
}

void Scene5000::drawClipFrameDeltaToBackground(byte frameIndex) {
	const uint32 frameTableOffset = _resourceChunkOffsets[4];
	const uint32 tableEntryOffset = frameTableOffset + ((uint32)frameIndex * 4);
	if (tableEntryOffset + 4 > _resourceArena.size())
		return;

	const uint32 frameOffset = frameTableOffset + ((uint32)kScene5000ClipDescriptorCount * 4) +
		readUint32LE(_resourceArena, tableEntryOffset);
	if (frameOffset + 4 > _resourceArena.size())
		return;

	const uint16 firstRow = readUint16LE(_resourceArena, frameOffset);
	const uint16 lastRow = readUint16LE(_resourceArena, frameOffset + 2);
	uint cursor = frameOffset + 4;
	byte *pixels = _presentationBackground.data();

	for (uint row = firstRow; row <= lastRow && row < HollywoodEngine::kSceneBufferHeight; ++row) {
		if (cursor >= _resourceArena.size())
			return;

		byte runCount = _resourceArena[cursor++];
		for (; runCount != 0; --runCount) {
			if (cursor + 3 > _resourceArena.size())
				return;

			const uint x = readUint16LE(_resourceArena, cursor);
			const byte literalLength = _resourceArena[cursor + 2];
			const uint destinationOffset = row * HollywoodEngine::kSceneBufferWidth + x;
			if (destinationOffset >= _presentationBackground.size())
				return;

			if (literalLength == 0) {
				if (cursor + 5 > _resourceArena.size())
					return;

				const byte fillValue = _resourceArena[cursor + 3];
				const uint fillLength = _resourceArena[cursor + 4];
				cursor += 5;
				if (destinationOffset + fillLength <= _presentationBackground.size())
					memset(pixels + destinationOffset, fillValue, fillLength);
			} else {
				const uint literalOffset = cursor + 3;
				if (literalOffset + literalLength > _resourceArena.size())
					return;

				if (destinationOffset + literalLength <= _presentationBackground.size())
					memcpy(pixels + destinationOffset, _resourceArena.data() + literalOffset, literalLength);
				cursor = literalOffset + literalLength;
			}
		}
	}
}

void Scene5000::advanceSpriteFrame() {
	if (_spriteMode == 1) {
		if (_spriteFrame < 0x1d) {
			++_spriteFrame;
			return;
		}
		if (_spriteRepeatCount != 0) {
			--_spriteRepeatCount;
			_spriteFrame = 0x0c;
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
		if (_spriteRepeatCount != 0) {
			--_spriteRepeatCount;
			_spriteFrame = 2;
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

	_spriteRepeatCount = (byte)(_random.getRandomNumber(1) + 1);
	if (_random.getRandomBit()) {
		_spriteMode = 1;
		_spriteFrame = 0x0c;
	} else {
		_spriteMode = 2;
		_spriteFrame = 2;
	}
}

} // End of namespace Hollywood
