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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/peggle/game.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

#define SYSCOLORS   19

bool    bHasPalette = true;             // whether the display is palette driven
int     nColorDepth = 8;                // display's bits per pixel
int     nScreenWidth = 640,             // screen size
        nScreenHeight = 480;

// Windows color parameters that can be modified
int syscolorelements[SYSCOLORS] = {
	COLOR_BTNFACE,
	COLOR_BTNHIGHLIGHT,
	COLOR_BTNSHADOW,
	COLOR_BTNTEXT
};


// Place to save Windows colors being altered
uint32 oldsyscolorvalues[SYSCOLORS];


// 8-Bit 256 color parameter values for Windows
uint32 syscolorvalues_8bit[SYSCOLORS] = {
	RGB(207, 103, 123),
	RGB(187, 147, 107),
	RGB(147, 99, 79),
	RGB(123, 75, 67),
};

// Non-Palette oriented color parameter values for Windows
uint32 syscolorvalues_moby[SYSCOLORS] = {
	RGB(207, 103, 123),
	RGB(187, 147, 107),
	RGB(147, 99, 79),
	RGB(123, 75, 67),
};

// Set the default colors used by Windows controls
void WINAPI WindowsInit() {
	int     i;
	HDC     hDC;

	hDC = GetDC(nullptr);                      // get screen's device context

	nScreenWidth = GetDeviceCaps(hDC, HORZRES);
	nScreenHeight = GetDeviceCaps(hDC, VERTRES);

	bHasPalette = ((GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE) ? true : false);
	nColorDepth = GetDeviceCaps(hDC, PLANES) * GetDeviceCaps(hDC, BITSPIXEL);

	ReleaseDC(nullptr, hDC);

	for (i = 0; i < SYSCOLORS; i++) {
		oldsyscolorvalues[i] = GetSysColor(syscolorelements[i]);
	}
}

// Reset the default Windows control colors
void WINAPI WindowsCleanup() {
}

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
