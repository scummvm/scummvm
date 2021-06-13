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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	GameEnded               = 1 << 4,
#ifdef _WIN32
	WindowInactive        = 1 << 8,
	WindowResizing        = 1 << 9,
	TimerSuspended        = 1 << 10,
	WindowReresing        = 1 << 11,
#endif
};

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

// ------------------------------------------------------------------------
// Game suspend / resume / terminate

void suspendGame(void);
bool gameSuspended(void);
void resumeGame(void);
void endGame(void);

// ------------------------------------------------------------------------
// Display initialization

void niceScreenStartup(void);
void initBackPanel(void);

// ------------------------------------------------------------------------
// Display disable flags

void displayEnable(DisplayDisabledBecause reason, bool onOff = true);
bool displayEnabled(uint32 mask = 0xFFFFFFFF);
bool displayOkay(void);
void mainEnable(void);
void mainDisable(void);

inline void displayDisable(DisplayDisabledBecause reason, bool onOff = false) {
	displayEnable(reason, onOff);
}

// ------------------------------------------------------------------------
// palette changes can be disabled

void enablePaletteChanges(void);
void disablePaletteChanges(void);
bool paletteChangesEnabled(void);

// ------------------------------------------------------------------------
// Screen refreshes
void delayedDisplayEnable(void);
void externalPaletteIntrusion(void);
void reDrawScreen(void);
void blackOut(void);
void showLoadMessage(void);

// ------------------------------------------------------------------------
// Video mode save and restore for videos

void pushVidState(void);
void popVidState(void);

// ------------------------------------------------------------------------
// Calls to suspend audio

void suspendAudio(void);
void resumeAudio(void);

// ------------------------------------------------------------------------
// Calls to suspend the game timer

void pauseTimer(void);
void resumeTimer(void);

// ------------------------------------------------------------------------
// The display may be disabled for several reasons these track them

#ifdef _WIN32
void localCursorOn(void);
void localCursorOff(void);
#endif
void blackOut(void);
void assertCurrentPalette(void);

} // end of namespace Saga2

#endif  //DISPLAY_H
