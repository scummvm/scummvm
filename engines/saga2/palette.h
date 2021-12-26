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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_PALETTE_H
#define SAGA2_PALETTE_H

#include "saga2/vpal.h"

namespace Saga2 {

/* ===================================================================== *
   Function prototypes
 * ===================================================================== */

//  Initialize the state of the current palette and fade up/down.
void initPaletteState();
//  Save the current state of the current palette and fade up/down in
//  a save file.
void savePaletteState(Common::OutSaveFile *outS);
//  Load and set the current state of the current palette and fade
//  up/down from a save file.
void loadPaletteState(Common::InSaveFile *in);
//  Cleanup the palette
inline void cleanupPaletteState() { /* do nothing */ }

} // end of namespace Saga2

#endif
