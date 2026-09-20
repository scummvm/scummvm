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

#ifndef SCOOBY_INTRO_SEQUENCE_H
#define SCOOBY_INTRO_SEQUENCE_H

#include "common/array.h"
#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/palette_color.h"
#include "scooby/graphics/tile_scene.h"

// Reimplements Ghidra RunStartupSequence at 0x000008CE, the four ROM-backed
// presentations executed before runtime entry.
//
// Presentation one is H32 and uses map 0x00023226, ring-LZ tiles 0x00023286,
// and palette 0x000236A0, followed by 50 frames, eight three-frame palette
// rotations, and 100 frames. Presentation two is H40 and uses map
// 0x000278E0, 0x20E0 tile words at 0x00023720, and palette 0x000281A0 for
// 300 frames. Presentation three is H32 and uses maps 0x000219C0/0x00021B40,
// ring-LZ tiles 0x00021E40, and palette 0x0002241A; plane B moves from -256
// through 48 in 76 four-pixel steps before a 200-frame hold. Presentation
// four is H32 and uses map 0x0002249A, ring-LZ tiles 0x0002266E, and
// palette 0x000231A6 for 300 frames. Every presentation has an eight-step
// fade in and fade out.

namespace Scooby {

class IntroSequence {
public:
	// Loads the first startup screen into the high-level scene and starts
	// its fade-in boundary.
	IntroSequence(const ScoobyDooRom &rom, TileScene &scene);

	// Gets whether execution is ready to enter Ghidra RunAdventure at 0x00000BF4.
	bool isComplete() const { return _state == State::Complete; }

	// Applies exactly one recovered V-blank's state changes and authored delay accounting.
	void advanceFrame();

	// Skips the active user-visible startup screen and initializes its successor at black.
	void skipCurrentStep();

private:
	enum class State {
		FirstFadeIn,
		FirstLeadIn,
		FirstPaletteCycle,
		FirstHold,
		FirstFadeOut,
		SecondFadeIn,
		SecondHold,
		SecondFadeOut,
		ThirdFadeIn,
		ThirdScroll,
		ThirdHold,
		ThirdFadeOut,
		FourthFadeIn,
		FourthHold,
		FourthFadeOut,
		Complete
	};

	static const int kFadeStepCount = 8;

	void advanceFirstPaletteCycle();
	void advanceThirdScreenScroll();
	void begin(State state, int frameCount);
	void beginFirstFadeOut() { begin(State::FirstFadeOut, kFadeStepCount); }
	void beginSecondFadeOut() { begin(State::SecondFadeOut, kFadeStepCount); }
	void beginThirdFadeOut() { begin(State::ThirdFadeOut, kFadeStepCount); }
	void beginFourthFadeOut() { begin(State::FourthFadeOut, kFadeStepCount); }
	void fadeIn(State nextState, int nextFrameCount);
	void fadeOut(const Common::Functor0<void> &loadNextScreen, State nextState);
	void loadFirstScreen();
	void loadFourthScreen();
	void loadSecondScreen();
	void loadThirdScreen();
	void readTargetPalette(int romOffset);
	void startFirstPaletteCycle();
	void waitThen(const Common::Functor0<void> &continuation);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	int _cycleDelay;
	int _cyclesRemaining;
	int _remainingFrames;
	State _state;
	Common::Array<PaletteColor> _targetPalette;
};
} // namespace Scooby

#endif // SCOOBY_INTRO_SEQUENCE_H
