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

#include "hollywood/scenes/intro/scene1000.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kTitleFrontEndArchiveName = "RESOURCE.A00";
const char *const kChapter1MusicArchiveName = "RESOURCE.M01";
const uint16 kTitleFrontEndMusicCueId = 0x000b;
const uint16 kTitleFrontEndNextState = 0x03f4;
const uint32 kTitlePhaseMillis = 1000;
const uint32 kTitlePaletteCycleMillis = 75;
const uint32 kBlinkFrameMillis = 75;
const uint32 kSecondaryFrameMillis = 75;
const uint kTitlePatchPhase = 3;
const uint kSecondaryStartPhase = 5;
const uint kTitleBackgroundRefreshPhase = 7;
const uint kTitleEndPhase = 0x32;
const uint kTitleNearEndPhase = 0x31;
const uint kTitleBackgroundRefreshBytes = 0x10000;

const byte kTitleBlinkDescriptorFrameMap[] = {
	0, 11, 0, 1, 2, 1, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 10, 10, 10, 10, 9, 8,
	7, 6, 5, 4, 3, 2, 1, 0
};

Scene1000::Scene1000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "title front-end"),
		_random("hollywood_scene1000"),
		_blinkPatternMode(0),
		_blinkFrameIndex(0),
		_secondaryFrameIndex(0),
		_blinkDirty(true),
		_secondaryDirty(false),
		_secondaryVisible(false) {
}

const char *Scene1000::resourceArchiveName() const {
	return kTitleFrontEndArchiveName;
}

const char *Scene1000::musicArchiveName() const {
	return kChapter1MusicArchiveName;
}

uint16 Scene1000::musicCueId() const {
	return kTitleFrontEndMusicCueId;
}

uint16 Scene1000::nextState() const {
	return kTitleFrontEndNextState;
}

byte Scene1000::activeAudioChapterIndex() const {
	return 1;
}

uint Scene1000::sceneArenaFirstChunk() const {
	return 2;
}

uint Scene1000::sceneArenaLastChunk() const {
	return 4;
}

void Scene1000::initializeChapterState() {
	GameplayState &state = _vm->gameState();
	state.currentInventoryOwnerIndex = 0;
	state.activeAudioChapterIndex = activeAudioChapterIndex();
	state.currentAmbientMusicCueId = musicCueId();
}

void Scene1000::drawInitialFrame() {
	renderOverlayFrame(true);
}

void Scene1000::runPresentation() {
	uint phase = 0;
	uint32 phaseAccumulator = 0;
	uint32 paletteAccumulator = 0;
	uint32 blinkAccumulator = 0;
	uint32 secondaryAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();

	while (phase < kTitleEndPhase && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		phaseAccumulator += delta;
		paletteAccumulator += delta;
		blinkAccumulator += delta;
		if (_secondaryVisible)
			secondaryAccumulator += delta;

		while (paletteAccumulator >= kTitlePaletteCycleMillis) {
			paletteAccumulator -= kTitlePaletteCycleMillis;
			rotatePaletteRange(0xf0, 0xfb);
		}

		while (blinkAccumulator >= kBlinkFrameMillis) {
			blinkAccumulator -= kBlinkFrameMillis;
			advanceBlinkFrame();
		}

		if (_secondaryVisible && _secondaryFrameIndex < kSecondaryDescriptorCount - 1) {
			while (secondaryAccumulator >= kSecondaryFrameMillis &&
					_secondaryFrameIndex < kSecondaryDescriptorCount - 1) {
				secondaryAccumulator -= kSecondaryFrameMillis;
				++_secondaryFrameIndex;
				_secondaryDirty = true;
				if (_secondaryFrameIndex == kSecondaryDescriptorCount - 1)
					phase = kTitleNearEndPhase;
			}
		}

		while (phaseAccumulator >= kTitlePhaseMillis && phase < kTitleEndPhase) {
			phaseAccumulator -= kTitlePhaseMillis;
			++phase;
			if (phase == kTitlePatchPhase) {
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2],
					_sceneFramebuffer.managedSurface());
				_blinkDirty = true;
			}
			if (phase == kSecondaryStartPhase) {
				_secondaryVisible = true;
				_secondaryDirty = true;
				secondaryAccumulator = kSecondaryFrameMillis;
			}
			if (phase == kTitleBackgroundRefreshPhase) {
				memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(),
					kTitleBackgroundRefreshBytes);
				_blinkDirty = true;
				_secondaryDirty = _secondaryVisible;
			}
		}

		renderOverlayFrame(false);
		g_system->delayMillis(10);
	}
}

void Scene1000::advanceBlinkFrame() {
	if (_blinkPatternMode != 0) {
		if (_blinkPatternMode == 1 && _blinkFrameIndex > 5) {
			_blinkPatternMode = 0;
			_blinkDirty = true;
			return;
		} else if (_blinkPatternMode == 2 && _blinkFrameIndex > 0x1e) {
			_blinkPatternMode = 0;
			_blinkDirty = true;
			return;
		} else {
			++_blinkFrameIndex;
			_blinkDirty = true;
			return;
		}
	}

	if (_blinkFrameIndex != 0) {
		_blinkFrameIndex = 0;
		_blinkDirty = true;
		return;
	}

	if (_random.getRandomNumber(14) == 0) {
		_blinkFrameIndex = 1;
		_blinkDirty = true;
		return;
	}

	if (_random.getRandomNumber(19) == 0) {
		_blinkFrameIndex = _random.getRandomBit() ? 7 : 2;
		_blinkPatternMode = _blinkFrameIndex == 7 ? 2 : 1;
	}

	_blinkDirty = true;
}

void Scene1000::renderOverlayFrame(bool forceDirty) {
	if (!forceDirty && !_blinkDirty && !_secondaryDirty)
		return;

	if (forceDirty) {
		_blinkDirty = true;
		if (_secondaryVisible)
			_secondaryDirty = true;
	}

	const uint blinkMapIndex = MIN<uint>(_blinkFrameIndex, ARRAYSIZE(kTitleBlinkDescriptorFrameMap) - 1);
	const byte blinkDescriptor = kTitleBlinkDescriptorFrameMap[blinkMapIndex];

	if (_blinkDirty) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[3], 0,
			kBlinkDescriptorCount, blinkDescriptor, _baseFramebuffer.surface(),
			_sceneFramebuffer.surface());
	}
	if (_secondaryDirty && _secondaryVisible) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[4], 0,
			kSecondaryDescriptorCount, _secondaryFrameIndex, _baseFramebuffer.surface(),
			_sceneFramebuffer.surface());
	}

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[3], 0,
		kBlinkDescriptorCount, blinkDescriptor, _sceneFramebuffer.surface());
	if (_secondaryVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[4], 0,
			kSecondaryDescriptorCount, _secondaryFrameIndex, _sceneFramebuffer.surface());
	}

	_blinkDirty = false;
	_secondaryDirty = false;
	presentFrame();
}

} // End of namespace Hollywood
