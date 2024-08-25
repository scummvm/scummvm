/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"

namespace ICB {

mcodeFunctionReturnCodes fx_narrow_screen(int32 &result, int32 *params) { return (MS->fx_narrow_screen(result, params)); }

mcodeFunctionReturnCodes fx_generic_fade(int32 &result, int32 *params) { return (MS->fx_generic_fade(result, params)); }

mcodeFunctionReturnCodes _game_session::fx_narrow_screen(int32 &, int32 *params) {
	/* Parameters */
	int32 mode = params[0];    // border mode
	int32 red = params[1];     // red component of cover
	int32 green = params[2];   // green component of cover
	int32 blue = params[3];    // blue component of cover
	int32 percent = params[4]; // 0 - 100 percentage screen to cover
	int32 cycles = params[5];  // number of cycles to shirnk over

	/* Function Storage */
	static int32 heightStep = 0;
	static int32 alphaStep = 0;
	static int32 cycleCount = 0;

	// Get access to the border rectangle
	LRECT &border = surface_manager->BorderRect();

	// Check if there is a set loaded.  If not we want to ignore the cycles param
	if (!MSS.OK())
		cycles = 1;

	// Calculate the fxtarget screen coverage
	int32 fxtarget = (480 * percent) / 200;

	if (cycleCount == 0) {
		// First time through

		// Set the border colour
		surface_manager->BorderRed() = (uint8)red;
		surface_manager->BorderGreen() = (uint8)green;
		surface_manager->BorderBlue() = (uint8)blue;

		// Semi-non-persistence(ish)... If percentage is not 0 make sure we start from 0
		if (percent) {
			border.top = 0;
			border.bottom = SCREEN_DEPTH;
			surface_manager->BorderAlpha() = 0;
		} else
			surface_manager->BorderAlpha() = 255;

		// Calculate the per cycle height step
		heightStep = (cycles) ? (fxtarget - border.top) / cycles : fxtarget;

		// Calculate alpha step
		alphaStep = (cycles) ? 255 / cycles : 255;

		// Check if we are fading out
		if (percent == 0)
			alphaStep = 0 - alphaStep;

		// Mode 0 has no fade so set alpha to solid
		if (mode == 0) { // Solid Colour
			alphaStep = 0;
			surface_manager->BorderAlpha() = 255;
		}

		if (mode == 2) { // Just Fade no shrink so set height step to 0
			heightStep = 0;

			if (percent) {
				border.top = fxtarget;
				border.bottom = SCREEN_DEPTH - fxtarget;
			}
		}

		// Set the mode
		surface_manager->BorderMode() = mode;

		// Check we actually need to move the borders or fade the screen
		if (heightStep == 0 && alphaStep == 0)
			return (IR_CONT);
	}

	// Check if we have reached the specified border size
	if (cycleCount == cycles) {
		cycleCount = 0;

		if (percent) {
			// We are leaving the screen with borders of one sort or another
			// So stop blending and switch to solid borders
			surface_manager->BorderMode() = 0;
		} else {
			// We were removing the borders, so stop drawing them now
			border.top = 0;
			border.bottom = SCREEN_DEPTH;
		}
		return (IR_CONT);
	}

	// Check we aren't going to over step the borders
	if (abs(fxtarget - border.top) <= abs(heightStep)) {
		border.bottom -= fxtarget - border.top;
		border.top = fxtarget;
	} else {
		// Move the borders
		border.top += heightStep;
		border.bottom -= heightStep;
	}

	// Check the alpha isn't going to go too far
	if ((alphaStep + surface_manager->BorderAlpha()) > 255) {
		surface_manager->BorderAlpha() = (uint8)255;
	} else if ((alphaStep + surface_manager->BorderAlpha()) < 0) {
		surface_manager->BorderAlpha() = (uint8)0;
	} else {
		surface_manager->BorderAlpha() = (uint8)(alphaStep + surface_manager->BorderAlpha());
	}

	cycleCount++;

	return (IR_REPEAT);
}

// the full monty effect, fx_generic_fade(mode (0,1,2), on/off, r, g, b, cycles)
// where mode is 0-brighten, 1-darken, 2-fade
// where on/off is equiverlant to to/from

mcodeFunctionReturnCodes _game_session::fx_generic_fade(int32 &, int32 *params) {
	int32 mode = params[0];
	int32 onOff = params[1];
	int32 fromRed = params[2];
	int32 fromGreen = params[3];
	int32 fromBlue = params[4];
	int32 toRed = params[5];
	int32 toGreen = params[6];
	int32 toBlue = params[7];
	int32 cycles = params[8];

	/* Function Storage */
	static int32 alphaStep = 0;

	// Is this the first time through ?
	if (alphaStep == 0) {
		// First time through
		alphaStep = (cycles) ? 255 / cycles : 255;

		// Set the colour components
		surface_manager->FadeFromRed() = (uint8)fromRed;
		surface_manager->FadeFromGreen() = (uint8)fromGreen;
		surface_manager->FadeFromBlue() = (uint8)fromBlue;
		surface_manager->FadeToRed() = (uint8)toRed;
		surface_manager->FadeToGreen() = (uint8)toGreen;
		surface_manager->FadeToBlue() = (uint8)toBlue;

		// Set the initial alpha value
		if (onOff)
			surface_manager->FadeAlpha() = 0; // Fading in
		else
			surface_manager->FadeAlpha() = 255; // Fading out

		// Set the fade mode
		surface_manager->FadeMode() = mode + 1;
	}

	// Increment / Decrement the alpha value
	int32 newAlpha = surface_manager->FadeAlpha();
	if (onOff) {
		// Fading in
		newAlpha += alphaStep;
	} else {
		// Fading out
		newAlpha -= alphaStep;
	}

	// Check the limits
	if (newAlpha <= 0) {
		// Finished fade out
		surface_manager->FadeMode() = 0;
		surface_manager->FadeAlpha() = 0;
		alphaStep = 0;
		return (IR_CONT);
	}

	if (newAlpha >= 255) {
		// Finished fade in
		surface_manager->FadeAlpha() = 255;
		alphaStep = 0;
		return (IR_CONT);
	}

	surface_manager->FadeAlpha() = (uint8)newAlpha;

	return (IR_REPEAT);
}

} // End of namespace ICB
