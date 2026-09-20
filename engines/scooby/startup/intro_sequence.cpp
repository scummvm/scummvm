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

#include "intro_sequence.h"

#include "common/algorithm.h"

#include "scooby/common/contracts.h"
#include "scooby/compression/ring_lz_decoder.h"
#include "scooby/graphics/genesis_asset_decoder.h"

namespace Scooby {

IntroSequence::IntroSequence(const ScoobyDooRom &rom, TileScene &scene)
	: _rom(rom), _scene(scene), _cycleDelay(0), _cyclesRemaining(0), _remainingFrames(0),
	  _state(State::Complete), _targetPalette(64) {
	Common::fill(_targetPalette.begin(), _targetPalette.end(), PaletteColor());
	loadFirstScreen();
	begin(State::FirstFadeIn, kFadeStepCount);
}

void IntroSequence::advanceFrame() {
	switch (_state) {
	case State::FirstFadeIn:
		fadeIn(State::FirstLeadIn, 50);
		break;
	case State::FirstLeadIn: {
		Common::Functor0Mem<void, IntroSequence> continuation(this, &IntroSequence::startFirstPaletteCycle);
		waitThen(continuation);
		break;
	}
	case State::FirstPaletteCycle:
		advanceFirstPaletteCycle();
		break;
	case State::FirstHold: {
		Common::Functor0Mem<void, IntroSequence> continuation(this, &IntroSequence::beginFirstFadeOut);
		waitThen(continuation);
		break;
	}
	case State::FirstFadeOut: {
		Common::Functor0Mem<void, IntroSequence> loadNextScreen(this, &IntroSequence::loadSecondScreen);
		fadeOut(loadNextScreen, State::SecondFadeIn);
		break;
	}
	case State::SecondFadeIn:
		fadeIn(State::SecondHold, 300);
		break;
	case State::SecondHold: {
		Common::Functor0Mem<void, IntroSequence> continuation(this, &IntroSequence::beginSecondFadeOut);
		waitThen(continuation);
		break;
	}
	case State::SecondFadeOut: {
		Common::Functor0Mem<void, IntroSequence> loadNextScreen(this, &IntroSequence::loadThirdScreen);
		fadeOut(loadNextScreen, State::ThirdFadeIn);
		break;
	}
	case State::ThirdFadeIn:
		fadeIn(State::ThirdScroll, 76);
		break;
	case State::ThirdScroll:
		advanceThirdScreenScroll();
		break;
	case State::ThirdHold: {
		Common::Functor0Mem<void, IntroSequence> continuation(this, &IntroSequence::beginThirdFadeOut);
		waitThen(continuation);
		break;
	}
	case State::ThirdFadeOut: {
		Common::Functor0Mem<void, IntroSequence> loadNextScreen(this, &IntroSequence::loadFourthScreen);
		fadeOut(loadNextScreen, State::FourthFadeIn);
		break;
	}
	case State::FourthFadeIn:
		fadeIn(State::FourthHold, 300);
		break;
	case State::FourthHold: {
		Common::Functor0Mem<void, IntroSequence> continuation(this, &IntroSequence::beginFourthFadeOut);
		waitThen(continuation);
		break;
	}
	case State::FourthFadeOut:
		_scene.advancePaletteTowardBlack();
		if (--_remainingFrames == 0) {
			_state = State::Complete;
		}
		break;
	case State::Complete:
		break;
	default:
		SDM_UNREACHABLE("Unhandled startup state.");
	}
}

void IntroSequence::skipCurrentStep() {
	switch (_state) {
	case State::FirstFadeIn:
	case State::FirstLeadIn:
	case State::FirstPaletteCycle:
	case State::FirstHold:
	case State::FirstFadeOut:
		loadSecondScreen();
		begin(State::SecondFadeIn, kFadeStepCount);
		break;
	case State::SecondFadeIn:
	case State::SecondHold:
	case State::SecondFadeOut:
		loadThirdScreen();
		begin(State::ThirdFadeIn, kFadeStepCount);
		break;
	case State::ThirdFadeIn:
	case State::ThirdScroll:
	case State::ThirdHold:
	case State::ThirdFadeOut:
		loadFourthScreen();
		begin(State::FourthFadeIn, kFadeStepCount);
		break;
	case State::FourthFadeIn:
	case State::FourthHold:
	case State::FourthFadeOut:
		_scene.clearContent();
		_state = State::Complete;
		break;
	case State::Complete:
		break;
	default:
		SDM_UNREACHABLE("Unhandled startup state.");
	}
}

void IntroSequence::advanceFirstPaletteCycle() {
	if (_cycleDelay == 0) {
		_scene.rotateFirstLogoRamp();
		_cyclesRemaining--;
		_cycleDelay = 2;
		return;
	}

	_cycleDelay--;
	if (_cycleDelay == 0 && _cyclesRemaining == 0) {
		begin(State::FirstHold, 100);
	}
}

void IntroSequence::advanceThirdScreenScroll() {
	int completedSteps = 76 - _remainingFrames + 1;
	_scene.setHorizontalOffsets(0, -256 + completedSteps * 4);
	if (--_remainingFrames == 0) {
		begin(State::ThirdHold, 200);
	}
}

void IntroSequence::begin(State state, int frameCount) {
	_state = state;
	_remainingFrames = frameCount;
}

void IntroSequence::fadeIn(State nextState, int nextFrameCount) {
	_scene.advancePaletteToward(MakeSpan(_targetPalette));
	if (--_remainingFrames == 0) {
		begin(nextState, nextFrameCount);
	}
}

void IntroSequence::fadeOut(const Common::Functor0<void> &loadNextScreen, State nextState) {
	_scene.advancePaletteTowardBlack();
	if (--_remainingFrames != 0) {
		return;
	}

	loadNextScreen();
	begin(nextState, kFadeStepCount);
}

void IntroSequence::loadFirstScreen() {
	_scene.reset(HorizontalDisplayMode::H32);
	_scene.loadLayerRows(_rom.readBytes(0x23226, 12 * 4 * static_cast<int>(sizeof(uint16))),
						 TileLayer::Foreground,
						 10, 11, 12, 4);
	_scene.loadTiles(MakeSpan(decompressRingLz(_rom, 0x23286)));
	readTargetPalette(0x236A0);
}

void IntroSequence::loadFourthScreen() {
	_scene.reset(HorizontalDisplayMode::H32);
	_scene.loadLayerRows(_rom.readBytes(0x2249A, 26 * 9 * static_cast<int>(sizeof(uint16))),
						 TileLayer::Foreground,
						 3, 8, 26, 9);
	_scene.loadTiles(MakeSpan(decompressRingLz(_rom, 0x2266E)));
	readTargetPalette(0x231A6);
}

void IntroSequence::loadSecondScreen() {
	_scene.reset(HorizontalDisplayMode::H40);
	_scene.loadLayerRows(_rom.readBytes(0x278E0, 40 * 28 * static_cast<int>(sizeof(uint16))),
						 TileLayer::Foreground,
						 0, 0, 40, 28);
	_scene.loadTiles(_rom.readBytes(0x23720, 0x20E0 * static_cast<int>(sizeof(uint16))));
	readTargetPalette(0x281A0);
}

void IntroSequence::loadThirdScreen() {
	_scene.reset(HorizontalDisplayMode::H32);
	_scene.loadLayerRows(_rom.readBytes(0x219C0, 32 * 6 * static_cast<int>(sizeof(uint16))),
						 TileLayer::Foreground,
						 0, 9, 32, 6);
	_scene.loadLayerRows(_rom.readBytes(0x21B40, 64 * 6 * static_cast<int>(sizeof(uint16))),
						 TileLayer::Background,
						 0, 9, 64, 6);
	_scene.loadTiles(MakeSpan(decompressRingLz(_rom, 0x21E40)));
	_scene.setHorizontalOffsets(0, -256);
	readTargetPalette(0x2241A);
}

void IntroSequence::readTargetPalette(int romOffset) {
	Common::Array<PaletteColor> palette = readPalette(_rom, romOffset);
	Common::copy(palette.begin(), palette.end(), _targetPalette.begin());
}

void IntroSequence::startFirstPaletteCycle() {
	_state = State::FirstPaletteCycle;
	_cyclesRemaining = 8;
	_cycleDelay = 0;
}

void IntroSequence::waitThen(const Common::Functor0<void> &continuation) {
	if (--_remainingFrames == 0) {
		continuation();
	}
}
} // namespace Scooby