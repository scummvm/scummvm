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

#include "hollywood/scenes/playable/scene4000.h"

#include "common/system.h"

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene4000ArchiveName = "RESOURCE.D00";
const char *const kScene4000MusicArchiveName = "RESOURCE.M04";
const char *const kScene4000SoundArchiveName = "RESOURCE.S04";
const uint16 kScene4000MusicCueId = 0x000b;
const uint16 kScene4000NextState = 0x0faa;
const uint16 kScene4000ViewportXOffset = 0x0068;
const uint32 kScene4000PhaseMillis = 1000;
const uint32 kScene4000ClipFrameMillis = 55;
const uint kScene4000PatchPhase = 3;
const uint kScene4000LateSoundPhase = 5;
const uint kScene4000ClipStartPhase = 6;
const uint kScene4000BackgroundRefreshPhase = 8;
const uint kScene4000EndPhase = 0x32;
const uint kScene4000NearEndPhase = 0x31;
const uint kScene4000ClipDescriptorCount = 0xad;
const uint kScene4000BackgroundRefreshBytes = 0x10000;
const byte kScene4000ClipFinalFrame = 0xc5;

const byte kScene4000ClipFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 60, 60, 61, 62, 63, 64, 65, 66, 67,
	68, 69, 70, 71, 72, 73, 74, 75, 76, 77,
	78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
	88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
	97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
	97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
	97, 97, 97, 98, 99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
	114, 115, 116, 117, 118, 119, 120, 121, 122, 123,
	124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
	134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
	154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
	164, 165, 166, 167, 168, 169, 170, 171, 172
};

Scene4000::Scene4000(HollywoodEngine *vm) :
		ChapterIntroScene(vm, "scene 4000"),
		_random("scene4000"),
		_baseSound(),
		_ambientSound(),
		_lateSound(),
		_lastAmbientCue(0xff),
		_lastLateCue(0xff),
		_clipFrame(0),
		_clipActive(false),
		_lateSoundEnabled(false) {
	_baseSound.setArchive(Common::Path(kScene4000SoundArchiveName));
	_ambientSound.setArchive(Common::Path(kScene4000SoundArchiveName));
	_lateSound.setArchive(Common::Path(kScene4000SoundArchiveName));
}

Scene4000::~Scene4000() {
	stopSoundCues();
}

const char *Scene4000::resourceArchiveName() const {
	return kScene4000ArchiveName;
}

const char *Scene4000::musicArchiveName() const {
	return kScene4000MusicArchiveName;
}

uint16 Scene4000::musicCueId() const {
	return kScene4000MusicCueId;
}

uint16 Scene4000::nextState() const {
	return kScene4000NextState;
}

byte Scene4000::activeAudioChapterIndex() const {
	return 4;
}

uint Scene4000::sceneArenaFirstChunk() const {
	return 2;
}

uint Scene4000::sceneArenaLastChunk() const {
	return 4;
}

uint16 Scene4000::sceneViewportXOffset() const {
	return kScene4000ViewportXOffset;
}

void Scene4000::drawInitialFrame() {
	presentFrame();
}

void Scene4000::runPresentation() {
	uint phase = 0;
	uint32 phaseAccumulator = 0;
	uint32 clipAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	bool frameDirty = false;

	while (phase < kScene4000EndPhase && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			break;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		phaseAccumulator += delta;
		if (_clipActive)
			clipAccumulator += delta;

		updateSoundCues();

		const byte previousClipFrame = _clipFrame;
		while (_clipActive && clipAccumulator >= kScene4000ClipFrameMillis &&
				_clipFrame < kScene4000ClipFinalFrame) {
			clipAccumulator -= kScene4000ClipFrameMillis;
			++_clipFrame;
			if (_clipFrame == kScene4000ClipFinalFrame)
				phase = kScene4000NearEndPhase;
		}

		while (phaseAccumulator >= kScene4000PhaseMillis && phase < kScene4000EndPhase) {
			phaseAccumulator -= kScene4000PhaseMillis;
			++phase;
			if (phase == kScene4000PatchPhase) {
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[2],
					_sceneFramebuffer.managedSurface());
				presentFrame();
			}
			if (phase == kScene4000LateSoundPhase) {
				_lateSound.playSample(0x2a, 100);
				_lateSoundEnabled = true;
			}
			if (phase == kScene4000ClipStartPhase) {
				_clipActive = true;
				clipAccumulator = kScene4000ClipFrameMillis;
			}
			if (phase == kScene4000BackgroundRefreshPhase)
				memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(),
					kScene4000BackgroundRefreshBytes);
		}

		for (uint clipFrame = previousClipFrame + 1; clipFrame <= _clipFrame; ++clipFrame) {
			const byte frame = kScene4000ClipFrameMap[MIN<uint>(clipFrame,
				ARRAYSIZE(kScene4000ClipFrameMap) - 1)];
			drawClipFrameDelta(4, kScene4000ClipDescriptorCount, frame);
			frameDirty = true;
		}

		if (frameDirty) {
			presentFrame();
			frameDirty = false;
		}

		g_system->delayMillis(10);
	}

	stopSoundCues();
}

void Scene4000::updateSoundCues() {
	if (!_baseSound.isPlaying())
		_baseSound.playSample(0x29, 100);

	if (!_ambientSound.isPlaying()) {
		byte cue = _lastAmbientCue;
		while (cue == _lastAmbientCue)
			cue = (byte)(0x0b + _random.getRandomNumber(2));
		_lastAmbientCue = cue;
		_ambientSound.playSample(cue, 25);
	}

	if (!_lateSoundEnabled || _lateSound.isPlaying() || _random.getRandomNumber(99) != 0)
		return;

	if (_random.getRandomNumber(0x7fff) == 0) {
		_lateSound.playSample(0x0e, 100);
		return;
	}

	byte cue = _lastLateCue;
	while (cue == _lastLateCue)
		cue = (byte)(0x0f + _random.getRandomNumber(4));
	_lastLateCue = cue;
	_lateSound.playSample(cue, 25);
}

void Scene4000::stopSoundCues() {
	_baseSound.stop();
	_ambientSound.stop();
	_lateSound.stop();
}

} // End of namespace Hollywood
