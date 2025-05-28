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

BOOL    bHasPalette = TRUE;             // whether the display is palette driven
int     nColorDepth = 8;                // display's bits per pixel
int     nScreenWidth = 640,             // screen size
        nScreenHeight = 480;

#ifdef HIDE

myLOGPALETTE myPalette = {0x300, 256, {

		{255, 255, 255, 0},
		{219, 255, 255, 0},
		{182, 255, 255, 0},
		{146, 255, 255, 0},
		{109, 255, 255, 0},
		{73, 255, 255, 0},
		{36, 255, 255, 0},
		{0, 255, 255, 0},
		{255, 255, 170, 0},
		{219, 255, 170, 0},
		{182, 255, 170, 0},
		{146, 255, 170, 0},
		{109, 255, 170, 0},
		{73, 255, 170, 0},
		{36, 255, 170, 0},
		{0, 255, 170, 0},
		{255, 255, 85, 0},
		{219, 255, 85, 0},
		{182, 255, 85, 0},
		{146, 255, 85, 0},
		{109, 255, 85, 0},
		{73, 255, 85, 0},
		{36, 255, 85, 0},
		{0, 255, 85, 0},
		{255, 255, 0, 0},
		{219, 255, 0, 0},
		{182, 255, 0, 0},
		{146, 255, 0, 0},
		{109, 255, 0, 0},
		{73, 255, 0, 0},
		{36, 255, 0, 0},
		{0, 255, 0, 0},
		{255, 170, 255, 0},
		{219, 170, 255, 0},
		{182, 170, 255, 0},
		{146, 170, 255, 0},
		{109, 170, 255, 0},
		{73, 170, 255, 0},
		{36, 170, 255, 0},
		{0, 170, 255, 0},
		{255, 170, 170, 0},
		{219, 170, 170, 0},
		{182, 170, 170, 0},
		{146, 170, 170, 0},
		{109, 170, 170, 0},
		{73, 170, 170, 0},
		{36, 170, 170, 0},
		{0, 170, 170, 0},
		{255, 170, 85, 0},
		{219, 170, 85, 0},
		{182, 170, 85, 0},
		{146, 170, 85, 0},
		{109, 170, 85, 0},
		{73, 170, 85, 0},
		{36, 170, 85, 0},
		{0, 170, 85, 0},
		{255, 170, 0, 0},
		{219, 170, 0, 0},
		{182, 170, 0, 0},
		{146, 170, 0, 0},
		{109, 170, 0, 0},
		{73, 170, 0, 0},
		{36, 170, 0, 0},
		{0, 170, 0, 0},
		{255, 85, 255, 0},
		{219, 85, 255, 0},
		{182, 85, 255, 0},
		{146, 85, 255, 0},
		{109, 85, 255, 0},
		{73, 85, 255, 0},
		{36, 85, 255, 0},
		{0, 85, 255, 0},
		{255, 85, 170, 0},
		{219, 85, 170, 0},
		{182, 85, 170, 0},
		{146, 85, 170, 0},
		{109, 85, 170, 0},
		{73, 85, 170, 0},
		{36, 85, 170, 0},
		{0, 85, 170, 0},
		{255, 85, 85, 0},
		{219, 85, 85, 0},
		{182, 85, 85, 0},
		{146, 85, 85, 0},
		{109, 85, 85, 0},
		{73, 85, 85, 0},
		{36, 85, 85, 0},
		{0, 85, 85, 0},
		{255, 85, 0, 0},
		{219, 85, 0, 0},
		{182, 85, 0, 0},
		{146, 85, 0, 0},
		{109, 85, 0, 0},
		{73, 85, 0, 0},
		{36, 85, 0, 0},
		{0, 85, 0, 0},
		{255, 0, 255, 0},
		{219, 0, 255, 0},
		{182, 0, 255, 0},
		{146, 0, 255, 0},
		{109, 0, 255, 0},
		{73, 0, 255, 0},
		{36, 0, 255, 0},
		{0, 0, 255, 0},
		{255, 0, 170, 0},
		{219, 0, 170, 0},
		{182, 0, 170, 0},
		{146, 0, 170, 0},
		{109, 0, 170, 0},
		{73, 0, 170, 0},
		{36, 0, 170, 0},
		{0, 0, 170, 0},
		{255, 0, 85, 0},
		{219, 0, 85, 0},
		{182, 0, 85, 0},
		{146, 0, 85, 0},
		{109, 0, 85, 0},
		{73, 0, 85, 0},
		{36, 0, 85, 0},
		{0, 0, 85, 0},
		{255, 0, 0, 0},
		{219, 0, 0, 0},
		{182, 0, 0, 0},
		{146, 0, 0, 0},
		{109, 0, 0, 0},
		{73, 0, 0, 0},
		{36, 0, 0, 0},
		{7, 7, 7, 0},
		{15, 15, 15, 0},
		{22, 22, 22, 0},
		{30, 30, 30, 0},
		{37, 37, 37, 0},
		{45, 45, 45, 0},
		{52, 52, 52, 0},
		{60, 60, 60, 0},
		{67, 67, 67, 0},
		{75, 75, 75, 0},
		{82, 82, 82, 0},
		{90, 90, 90, 0},
		{97, 97, 97, 0},
		{105, 105, 105, 0},
		{112, 112, 112, 0},
		{120, 120, 120, 0},
		{127, 127, 127, 0},
		{135, 135, 135, 0},
		{143, 143, 143, 0},
		{150, 150, 150, 0},
		{158, 158, 158, 0},
		{165, 165, 165, 0},
		{173, 173, 173, 0},
		{180, 180, 180, 0},
		{188, 188, 188, 0},
		{195, 195, 195, 0},
		{203, 203, 203, 0},
		{210, 210, 210, 0},
		{218, 218, 218, 0},
		{225, 225, 225, 0},
		{233, 233, 233, 0},
		{240, 240, 240, 0},
		{182, 170, 255, 0},
		{172, 160, 240, 0},
		{162, 151, 225, 0},
		{151, 141, 209, 0},
		{141, 131, 194, 0},
		{131, 121, 178, 0},
		{120, 112, 163, 0},
		{110, 102, 148, 0},
		{100, 92, 132, 0},
		{89, 83, 117, 0},
		{79, 73, 101, 0},
		{69, 63, 86, 0},
		{58, 53, 71, 0},
		{48, 44, 55, 0},
		{37, 34, 40, 0},
		{27, 24, 24, 0},
		{255, 204, 153, 0},
		{204, 153, 102, 0},
		{153, 102, 0, 0},
		{249, 249, 234, 0},
		{153, 102, 204, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
#endif


// Windows color parameters that can be modified
		int     syscolorelements[SYSCOLORS] = {
			COLOR_BTNFACE,
			COLOR_BTNHIGHLIGHT,
			COLOR_BTNSHADOW,
			COLOR_BTNTEXT
		};


// Place to save Windows colors being altered
		DWORD   oldsyscolorvalues[SYSCOLORS];


// 8-Bit 256 color parameter values for Windows
		DWORD   syscolorvalues_8bit[SYSCOLORS] = {
			RGB(207, 103, 123),
			RGB(187, 147, 107),
			RGB(147, 99, 79),
			RGB(123, 75, 67),
		};

// Non-Palette oriented color parameter values for Windows
		DWORD   syscolorvalues_moby[SYSCOLORS] = {
			RGB(207, 103, 123),
			RGB(187, 147, 107),
			RGB(147, 99, 79),
			RGB(123, 75, 67),
		};


		#ifdef HIDE

// Windows color parameters that can be modified
		int     syscolorelements[SYSCOLORS] = {
			COLOR_ACTIVEBORDER,
			COLOR_ACTIVECAPTION,
			COLOR_BTNFACE,
			COLOR_BTNHIGHLIGHT,
			COLOR_BTNSHADOW,
			COLOR_BTNTEXT,
			COLOR_CAPTIONTEXT,
			COLOR_GRAYTEXT,
			COLOR_HIGHLIGHT,
			COLOR_HIGHLIGHTTEXT,
			COLOR_INACTIVEBORDER,
			COLOR_INACTIVECAPTION,
			COLOR_INACTIVECAPTIONTEXT,
			COLOR_MENU,
			COLOR_MENUTEXT,
			COLOR_SCROLLBAR,
			COLOR_WINDOW,
			COLOR_WINDOWFRAME,
			COLOR_WINDOWTEXT
		};


// Place to save Windows colors being altered
		DWORD   oldsyscolorvalues[SYSCOLORS];


// 8-Bit 256 color parameter values for Windows
		DWORD   syscolorvalues_8bit[SYSCOLORS] = {
			PALETTERGB(192, 192, 192),
			PALETTERGB(0, 0, 255),
			#ifdef HIDE
			PALETTERGB(192, 192, 192),
			PALETTERGB(255, 255, 255),
			PALETTERGB(128, 128, 128),
			PALETTERGB(0, 0, 0),
			#endif
			PALETTERGB(207, 103, 123),
			PALETTERGB(187, 147, 107),
			PALETTERGB(147, 99, 79),
			PALETTERGB(123, 75, 67),

			PALETTERGB(255, 255, 255),
			PALETTERGB(192, 192, 192),
			PALETTERGB(192, 192, 192),
			PALETTERGB(0, 0, 0),
			PALETTERGB(192, 192, 192),
			PALETTERGB(164, 200, 240),
			PALETTERGB(0, 0, 0),
			PALETTERGB(255, 255, 255),
			PALETTERGB(0, 0, 0),
			PALETTERGB(192, 192, 192),
			PALETTERGB(255, 255, 255),
			PALETTERGB(0, 0, 0),
			PALETTERGB(0, 0, 0)
		};

// Non-Palette oriented color parameter values for Windows
		DWORD   syscolorvalues_moby[SYSCOLORS] = {
			RGB(192, 192, 192),
			RGB(0, 0, 255),
			RGB(192, 192, 192),
			RGB(255, 255, 255),
			RGB(128, 128, 128),
			RGB(0, 0, 0),
			RGB(255, 255, 255),
			RGB(192, 192, 192),
			RGB(192, 192, 192),
			RGB(0, 0, 0),
			RGB(192, 192, 192),
			RGB(164, 200, 240),
			RGB(0, 0, 0),
			RGB(255, 255, 255),
			RGB(0, 0, 0),
			RGB(192, 192, 192),
			RGB(255, 255, 255),
			RGB(0, 0, 0),
			RGB(0, 0, 0)
		};
		#endif

// Set the default colors used by Windows controls
		void WINAPI WindowsInit(void) {
			int     i;
			HDC     hDC;

			hDC = GetDC(nullptr);                      // get screen's device context

			nScreenWidth = GetDeviceCaps(hDC, HORZRES);
			nScreenHeight = GetDeviceCaps(hDC, VERTRES);

			bHasPalette = ((GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE) ? TRUE : FALSE);
			nColorDepth = GetDeviceCaps(hDC, PLANES) * GetDeviceCaps(hDC, BITSPIXEL);

			ReleaseDC(nullptr, hDC);

			for (i = 0; i < SYSCOLORS; i++) {
				oldsyscolorvalues[i] = GetSysColor(syscolorelements[i]);
			}

			#if 0
			// This uses an unknown function GetSysColors
			// in the original. Is gameinit.cpp not used at all?
			if (bHasPalette && (nColorDepth >= 8))
				SetSysColors(SYSCOLORS, syscolorelements, syscolorvalues_8bit);
			else if (nColorDepth > 8)
				SetSysColors(SYSCOLORS, syscolorelements, syscolorvalues_moby);
			#endif
		}


// Reset the default Windows control colors
		void WINAPI WindowsCleanup(void) {
			#if 0
			SetSysColors(SYSCOLORS, syscolorelements, oldsyscolorvalues);
			#endif
		}

		#ifdef HIDE
		void CreatePalette(void) {
			int i;

			hPalMem = LocalAlloc(LMEM_FIXED, sizeof(LOGPALETTE) + PalSize * sizeof(PALETTEENTRY));
			if (!hPalMem) {
				Warning("Unable to allocate color palette memory");
				return (false);
			}

			pLogPal = (NPLOGPALETTE) LocalLock(hPalMem);
			pLogPal->palVersion = 0x300;
			pLogPal->palNumEntries = PalSize;

			if (PalSize == 256)
				for (i = 0; i < PalSize; i++) {
					pLogPal->palPalEntry[i].peRed = palette_table[i].red;
					pLogPal->palPalEntry[i].peGreen = palette_table[i].green;
					pLogPal->palPalEntry[i].peBlue = palette_table[i].blue;
					pLogPal->palPalEntry[i].peFlags = nullptr;
				} else if (PalSize == 16)
				for (i = 0; i < PalSize; i++) {
					pLogPal->palPalEntry[i].peRed = mini_palette_table[i].red;
					pLogPal->palPalEntry[i].peGreen = mini_palette_table[i].green;
					pLogPal->palPalEntry[i].peBlue = mini_palette_table[i].blue;
					pLogPal->palPalEntry[i].peFlags = nullptr;
				}

			myPalette = CreatePalette((LPLOGPALETTE)pLogPal);
			if (!myPalette) {
				Warning("Unable to create color palette");
				return (false);
			}

			return (true);
		}
		#endif

	} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
