/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_PALETTE_H
#define SAGA2_PALETTE_H

namespace Saga2 {

/* ===================================================================== *
   Typedefs
 * ===================================================================== */

typedef gPalette        *gPalettePtr;

/* ===================================================================== *
   Exports
 * ===================================================================== */

//  Global palette resource handles
extern gPalettePtr   midnightPalette,
       noonPalette,
       darkPalette;

/* ===================================================================== *
   Function prototypes
 * ===================================================================== */

//  Initialize global palette resources
void loadPalettes(void);
//  Dump global palette resources
void cleanupPalettes(void);

//  Begin fade up/down
void beginFade(gPalettePtr newPalette, int32 fadeDuration);
//  Update state of palette fade up/down
bool updatePalette(void);

//  Linearly interpolate between two specified palettes
void createPalette(
    gPalettePtr newP,
    gPalettePtr srcP,
    gPalettePtr dstP,
    int32       elapsedTime,
    int32       totalTime);
//  Set the current palette
void setCurrentPalette(gPalettePtr newPal);
//  Return the current palette
void getCurrentPalette(gPalettePtr pal);

void setPaletteToBlack(void);

//  Initialize the state of the current palette and fade up/down.
void initPaletteState(void);
//  Save the current state of the current palette and fade up/down in
//  a save file.
void savePaletteState(SaveFileConstructor &saveGame);
//  Load and set the current state of the current palette and fade
//  up/down from a save file.
void loadPaletteState(SaveFileReader &saveGame);
//  Cleanup the palette
inline void cleanupPaletteState(void) { /* do nothing */ }

} // end of namespace Saga2

#endif
