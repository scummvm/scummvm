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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_DISPLAY_H
#define SAGA2_DISPLAY_H

namespace Saga2 {

/* ===================================================================== *
   Types
 * ===================================================================== */

enum DisplayDisabledBecause {
	GameNotInitialized      = 1 << 0,
	PlayingVideo            = 1 << 1,
	GraphicsInit            = 1 << 2,
	GameSuspended           = 1 << 3,
	GameEnded               = 1 << 4
};

/* ===================================================================== *
   Prototypes
 * ===================================================================== */
void endGame();

// ------------------------------------------------------------------------
// Display initialization

void niceScreenStartup();
void initBackPanel();

// ------------------------------------------------------------------------
// Display disable flags

void displayEnable(DisplayDisabledBecause reason, bool onOff = true);
bool displayEnabled(uint32 mask = 0xFFFFFFFF);
bool displayOkay();
void mainEnable();
void mainDisable();

inline void displayDisable(DisplayDisabledBecause reason, bool onOff = false) {
	displayEnable(reason, onOff);
}

// ------------------------------------------------------------------------
// palette changes can be disabled

void enablePaletteChanges();
void disablePaletteChanges();
bool paletteChangesEnabled();

// ------------------------------------------------------------------------
// Screen refreshes
void delayedDisplayEnable();
void externalPaletteIntrusion();
void reDrawScreen();
void showLoadMessage();

// ------------------------------------------------------------------------
// Video mode save and restore for videos

void pushVidState();
void popVidState();

// ------------------------------------------------------------------------
// The display may be disabled for several reasons these track them
void blackOut();

} // end of namespace Saga2

#endif  //DISPLAY_H
