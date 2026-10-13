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

#include "common/debug.h"
#include "common/endian.h"
#include "common/util.h"

#include "hollywood/debug.h"
#include "hollywood/resource.h"
#include "hollywood/runaway/intro.h"

namespace Hollywood {
namespace {

// The native 10 ms timer (0x409140) advances the first logo track every
// fourth tick and the remaining tracks every second tick. The frame and
// lettering tracks retain the completed tracks underneath (0x44a940).
const uint kLogoFrameCounts[] = { 175, 48, 48 };
const uint kLogoFrameDelays[] = { 40, 20, 20 };
const uint kLogoTrackStarts[] = {
	0,
	(kLogoFrameCounts[0] - 1) * kLogoFrameDelays[0],
	(kLogoFrameCounts[0] - 1) * kLogoFrameDelays[0] + kLogoFrameCounts[1] * kLogoFrameDelays[1]
};
const uint kLogoAnimationDuration = kLogoTrackStarts[2] + (kLogoFrameCounts[2] - 1) * kLogoFrameDelays[2];
const uint kLogoHoldDuration = 2000;
const uint kLogoFadeDuration = 1000;
const uint kStoryFadeDuration = 500;
const uint kStorySkipFadeDuration = 250;
const uint kStoryHoldDuration = 40000;

} // End of anonymous namespace

bool RunawayIntro::load() {
	ChunkArchive archive;
	if (!archive.open(Common::Path("RESOURCE.A02")))
		return false;
	for (uint i = 0; i < ARRAYSIZE(_logo); ++i) {
		if (!_logo[i].load(archive, i, kLogoFrameCounts[i]))
			return false;
	}

	// 0x444390 displays X01 chunk 0 before entering scene 5030. The card
	// already contains the localized story text and the Runaway title.
	Common::Array<byte> pixels;
	if (!archive.open(Common::Path("RESOURCE.X01")) ||
			archive.chunkSize(0) != kRunawayScreenWidth * kRunawayViewportHeight * 2 ||
			!archive.readVariableChunk(0, pixels))
		return false;
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_story.create(kRunawayScreenWidth, kRunawayViewportHeight, format);
	_screen.create(kRunawayScreenWidth, kRunawayScreenHeight, format);
	for (int y = 0; y < _story.h; ++y) {
		uint16 *out = (uint16 *)_story.getBasePtr(0, y);
		for (int x = 0; x < _story.w; ++x)
			out[x] = READ_LE_UINT16(pixels.data() + (y * _story.w + x) * 2);
	}
	debugC(1, kDebugScene, "Runaway demo: Pendulo logo and story card loaded");
	return true;
}

void RunawayIntro::start(uint32 now) {
	_phase = kLogo;
	_phaseStarted = now;
	_audio.playIntroMusic();
	update(now);
}

void RunawayIntro::startStory(uint32 now) {
	_audio.stopMusic();
	_phase = kStoryFadeIn;
	_phaseStarted = now;
	_brightness = 0;
}

void RunawayIntro::skip(uint32 now) {
	if (_phase == kLogo || _phase == kLogoAudio) {
		startStory(now);
	} else if (_phase == kStory) {
		_phase = kStoryFadeOut;
		_phaseStarted = now;
		_storyFadeDuration = kStorySkipFadeDuration;
	}
	// The original clears pending input after the card's fade-in. Ignoring
	// input during its fades also keeps one press from skipping both stages.
}

void RunawayIntro::update(uint32 now) {
	// Carry elapsed time across timed phases so delayed frames do not extend
	// the sequence. The engine clock stops while ScummVM dialogs pause audio.
	for (;;) {
		const uint32 elapsed = now - _phaseStarted;
		switch (_phase) {
		case kLogo: {
			for (uint i = 0; i < ARRAYSIZE(_logo); ++i) {
				_logoFrames[i] = elapsed < kLogoTrackStarts[i] ? -1 :
					(int)MIN<uint32>((elapsed - kLogoTrackStarts[i]) / kLogoFrameDelays[i], kLogoFrameCounts[i] - 1);
			}
			const uint fadeStart = kLogoAnimationDuration + kLogoHoldDuration;
			if (elapsed >= fadeStart + kLogoFadeDuration) {
				_phase = kLogoAudio;
				continue;
			}
			_brightness = elapsed <= fadeStart ? 255 :
				255 * (kLogoFadeDuration - (elapsed - fadeStart)) / kLogoFadeDuration;
			return;
		}
		case kLogoAudio:
			_brightness = 0;
			if (_audio.musicPlaying())
				return;
			startStory(now);
			return;
		case kStoryFadeIn:
			if (elapsed >= kStoryFadeDuration) {
				_phase = kStory;
				_phaseStarted += kStoryFadeDuration;
				continue;
			}
			_brightness = 255 * elapsed / kStoryFadeDuration;
			return;
		case kStory:
			if (elapsed >= kStoryHoldDuration) {
				_phase = kStoryFadeOut;
				_phaseStarted += kStoryHoldDuration;
				_storyFadeDuration = kStoryFadeDuration;
				continue;
			}
			_brightness = 255;
			return;
		case kStoryFadeOut:
			if (elapsed >= _storyFadeDuration) {
				_phase = kFinished;
				continue;
			}
			_brightness = 255 * (_storyFadeDuration - elapsed) / _storyFadeDuration;
			return;
		case kFinished:
			_brightness = 0;
			return;
		}
	}
}

void RunawayIntro::draw() {
	_screen.fillRect(Common::Rect(_screen.w, _screen.h), 0);
	if (!_brightness)
		return;
	if (_phase == kLogo) {
		for (uint i = 0; i < ARRAYSIZE(_logo); ++i) {
			if (_logoFrames[i] >= 0)
				_logo[i].draw(_screen, _logoFrames[i], 0, kRunawayViewportTop);
		}
	} else if (_phase == kStoryFadeIn || _phase == kStory || _phase == kStoryFadeOut) {
		_screen.blitFrom(_story, Common::Point(0, kRunawayViewportTop));
	}
	if (_brightness == 255)
		return;
	// Apply the original gamma fades to the RGB565 viewport without changing
	// backend gamma or affecting ScummVM's own dialogs.
	for (int y = kRunawayViewportTop; y < kRunawayViewportTop + kRunawayViewportHeight; ++y) {
		uint16 *out = (uint16 *)_screen.getBasePtr(0, y);
		for (int x = 0; x < _screen.w; ++x) {
			const uint16 color = out[x];
			out[x] = (((color >> 11) * _brightness / 255) << 11) |
				((((color >> 5) & 63) * _brightness / 255) << 5) | ((color & 31) * _brightness / 255);
		}
	}
}

} // End of namespace Hollywood
