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

#ifndef SCOOBY_MAIN_MENU_VBLANK_HANDLER_H
#define SCOOBY_MAIN_MENU_VBLANK_HANDLER_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/input/controller_input.h"
#include "scooby/main_menu/main_menu_presentation.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/runtime/runtime_state.h"

// Owns the callback installed by Ghidra RunMainMenu at 0x00008276 for each menu retrace.
//
// Lightning replaces palette line two from 0x0000A558, then selects one of three plane-B patches:
// 0x0000A59C at (22,7) with size 4x10, 0x0000A5EC at (27,7) with size 4x5, or 0x0000A614 at (5,6)
// with size 4x5. Four three-retrace palette frames end at normal menu palette source 0x00016CF8,
// after which only the selected rectangle is restored from the immutable map.

namespace Scooby {

class MainMenuVBlankHandler {
public:
	// Binds the installed callback to host input, logical menu presentation, RNG, and shared state.
	// input: recovered controller poll performed once per callback.
	// presentation: logical owner of the callback's animation and lightning publications.
	// random: shared original random-value boundary.
	// state: shared flags and signed countdowns updated by the callback tail.
	MainMenuVBlankHandler(ControllerInput &input, MainMenuPresentation &presentation,
						  DeterministicRandom &random, RuntimeState &state)
		: _input(input), _presentation(presentation), _random(random), _state(state) {
	}

	// Restores the two lightning words initialized by Ghidra RunMainMenu.
	void reset() {
		_lightningDelay = 0;
		_lightningFrameCountdown = 0;
	}

	// Advances lightning, menu animation, input, shared flags, and both countdowns once per retrace.
	//
	// Ghidra: handleMainMenuVBlank (0x0000A38C).
	void handleMainMenuVBlank();

	// Runs the shared callback entry installed by the episode opening without advancing menu-only
	// effects.
	//
	// Raw callback target: 0x0000A4A4 within Ghidra HandleMainMenuVBlank.
	void handleInputVBlankTail();

private:
	static const uint8 kMenuAnimationEnabledMask = 0x10;
	static const uint8 kMenuLightningEnabledMask = 0x80;
	static const int kLightningInitialPaletteOffset = 0xA558;
	static const int16 kLightningFrameCadence = 2;
	static const int16 kLightningInitialFrameByteOffset = 0x0C;
	static const uint16 kLightningDelayScale = 0xC8;
	static const Common::Array<int> kLightningPaletteOffsets;

	void advanceLightning();
	void activateLightningRegion(int sourceOffset, int column, int row, int width, int height);

	ControllerInput &_input;
	MainMenuPresentation &_presentation;
	DeterministicRandom &_random;
	RuntimeState &_state;

	// Replaces Ghidra g_swSharedLightningDelayOrSpriteScratch0016 at 0xFF0016. RunMainMenu
	// initializes it to zero, so the first enabled callback starts a strike; each completed strike
	// replaces it with a value below 200.
	int16 _lightningDelay{};

	// Replaces Ghidra g_swSharedLightningCountdownOrSpriteScratch001A at 0xFF001A. RunMainMenu
	// initializes it to zero, and each authored palette is then retained for three callback
	// invocations through the inclusive countdown.
	int16 _lightningFrameCountdown{};

	// Replaces Ghidra g_swSharedLightningFrameOffsetOrSpriteScratch0018 at 0xFF0018. No image-backed
	// initial value is consumed: strike start writes byte offset 0x0C before the callback first
	// reads the frame-pointer table.
	int16 _lightningFrameByteOffset{};

	// Replaces original word 0xFF0810 with the selected plane-B column. The callback writes the
	// complete region before the terminal frame first restores it.
	int _lightningColumn{};

	// Typed row component of original plane-B byte offset word 0xFF0810.
	int _lightningRow{};

	// Replaces original DMA width word 0xFF0812; written before terminal restore.
	int _lightningWidth{};

	// Replaces original DBF height counter 0xFF0814 as its logical row count; written before restore.
	int _lightningHeight{};
};
} // namespace Scooby

#endif // SCOOBY_MAIN_MENU_VBLANK_HANDLER_H
