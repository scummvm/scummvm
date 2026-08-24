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

#include "hollywood/scenes/playable/scene3000.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene3000ArchiveName = "RESOURCE.C00";
const char *const kScene3000MusicArchiveName = "RESOURCE.M03";
const uint16 kScene3000MusicCueId = 0x000b;
const uint16 kScene3000NextState = 0x0bc2;
const uint16 kScene3000ViewportXOffset = 0x00c0;
const uint32 kScene3000LargePaletteCycleMillis = 300;
const uint32 kScene3000SmallPaletteCycleMillis = 350;
const uint32 kScene3000SpriteFrameMillis = 125;
const uint32 kScene3000ClipFrameMillis = 60;
const uint kScene3000LargeDescriptorCount = 0x1e;
const uint kScene3000SmallDescriptorCount = 8;
const uint kScene3000ClipDescriptorCount = 0x6e;
const byte kScene3000InitialLargeFrame = 4;
const byte kScene3000ClipFinalFrame = 0x6d;
const uint kScene3000PatchTick = 100;
const uint kScene3000ClipDelayTicks = 200;
const uint kScene3000BackgroundRefreshFrame = 100;
const uint kScene3000BackgroundRefreshBytes = 0x10000;
const int kScene3000BeforeFirstClipFrame = -1;

Scene3000::Scene3000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "scene 3000"),
		_largeFrame(kScene3000InitialLargeFrame),
		_smallFrame(0),
		_clipFrame(0) {
}

const char *Scene3000::resourceArchiveName() const {
	return kScene3000ArchiveName;
}

const char *Scene3000::musicArchiveName() const {
	return kScene3000MusicArchiveName;
}

uint16 Scene3000::musicCueId() const {
	return kScene3000MusicCueId;
}

uint16 Scene3000::nextState() const {
	return kScene3000NextState;
}

byte Scene3000::activeAudioChapterIndex() const {
	return 3;
}

uint Scene3000::sceneArenaFirstChunk() const {
	return 2;
}

uint Scene3000::sceneArenaLastChunk() const {
	return 6;
}

uint16 Scene3000::sceneViewportXOffset() const {
	return kScene3000ViewportXOffset;
}

void Scene3000::drawInitialFrame() {
	drawPresentationFrame(true, true, kScene3000BeforeFirstClipFrame);
}

void Scene3000::runPresentation() {
	uint delayedClipTick = 0;
	uint32 largePaletteAccumulator = 0;
	uint32 smallPaletteAccumulator = 0;
	uint32 largeAccumulator = 0;
	uint32 smallAccumulator = 0;
	uint32 clipAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	bool largeDirty = false;
	bool smallDirty = false;

	while (!_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		largePaletteAccumulator += delta;
		smallPaletteAccumulator += delta;
		largeAccumulator += delta;
		smallAccumulator += delta;
		clipAccumulator += delta;

		while (largePaletteAccumulator >= kScene3000LargePaletteCycleMillis) {
			largePaletteAccumulator -= kScene3000LargePaletteCycleMillis;
			rotatePaletteRange(0xf0, 0xfa);
		}

		while (smallPaletteAccumulator >= kScene3000SmallPaletteCycleMillis) {
			smallPaletteAccumulator -= kScene3000SmallPaletteCycleMillis;
			rotatePaletteRange(0xfb, 0xfd);
		}

		while (largeAccumulator >= kScene3000SpriteFrameMillis) {
			largeAccumulator -= kScene3000SpriteFrameMillis;
			if (_vm->gameState().windmillBladesMoving) {
				_largeFrame = _largeFrame < 0x1d ? _largeFrame + 1 : 0;
				largeDirty = true;
			}
		}

		while (smallAccumulator >= kScene3000SpriteFrameMillis) {
			smallAccumulator -= kScene3000SpriteFrameMillis;
			_smallFrame = _smallFrame < 7 ? _smallFrame + 1 : 0;
			smallDirty = true;
		}

		const byte previousClipFrame = _clipFrame;
		while (clipAccumulator >= kScene3000ClipFrameMillis) {
			clipAccumulator -= kScene3000ClipFrameMillis;
			if (delayedClipTick < kScene3000ClipDelayTicks) {
				++delayedClipTick;
				if (delayedClipTick == kScene3000PatchTick) {
					drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2], _sceneFramebuffer.managedSurface());
				}
				continue;
			}
			if (_clipFrame < kScene3000ClipFinalFrame) {
				++_clipFrame;
				if (_clipFrame == kScene3000BackgroundRefreshFrame)
					memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(),
						kScene3000BackgroundRefreshBytes);
			}
		}

		if (largeDirty || smallDirty || previousClipFrame != _clipFrame) {
			drawPresentationFrame(largeDirty, smallDirty, previousClipFrame);
			largeDirty = false;
			smallDirty = false;
		}

		if (_clipFrame >= kScene3000ClipFinalFrame)
			return;

		g_system->delayMillis(10);
	}
}

void Scene3000::drawPresentationFrame(bool largeDirty, bool smallDirty, int previousClipFrame) {
	if (largeDirty) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[3], 0,
			kScene3000LargeDescriptorCount, _largeFrame,
			_baseFramebuffer.surface(), _sceneFramebuffer.surface());
	}
	const uint smallChunkIndex = _vm->gameState().scene3080ChimneySmokeAnimationChanged ? 5 : 4;
	if (smallDirty) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[smallChunkIndex], 0,
			kScene3000SmallDescriptorCount, _smallFrame,
			_baseFramebuffer.surface(), _sceneFramebuffer.surface());
	}
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[3], 0,
		kScene3000LargeDescriptorCount, _largeFrame, _sceneFramebuffer.managedSurface());
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[smallChunkIndex], 0,
		kScene3000SmallDescriptorCount, _smallFrame, _sceneFramebuffer.managedSurface());
	for (int clipFrame = previousClipFrame + 1; clipFrame <= _clipFrame; ++clipFrame)
		drawClipFrameDelta(6, kScene3000ClipDescriptorCount, clipFrame);

	presentFrame();
}

} // End of namespace Hollywood
