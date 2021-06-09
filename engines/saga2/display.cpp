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

#include "saga2/std.h"
#include "saga2/display.h"
#include "saga2/intrface.h"
#include "saga2/loadmsg.h"
#include "saga2/grabinfo.h"

namespace Saga2 {

extern bool             gameRunning;
extern bool             delayReDraw;
extern gDisplayPort     mainPort;               // default rendering port
extern BackWindow       *mainWindow;            // main window...
#ifdef _WIN32
extern CFTWindow        *pWindow;
#endif

/* ===================================================================== *
   Globals
 * ===================================================================== */

bool                        paletteMayHaveChanged = FALSE;


/* ===================================================================== *
   Locals
 * ===================================================================== */

static uint32 displayStatus = GraphicsInit;
static bool gameSuspendFlag = FALSE;
static bool paletteSuspendFlag = FALSE;
#ifndef _WIN32
static bool VideoSaved = FALSE;
static uint8 VideoSaveMode;
#endif


/* ===================================================================== *
   Prototypes
 * ===================================================================== */

void reDrawScreen(void);
void quickSavePalette(void);
void quickRestorePalette(void);
void localCursorOn(void);
void localCursorOff(void);
void setPaletteToBlack(void);
void lightsOut(void);
void loadingScreen(void);
void resetInputDevices(void);
APPFUNC(cmdWindowFunc);                      // main window event handler
#ifdef _WIN32
void suspendProcessResources(void);
void resumeProcessResources(void);
#endif
static void switchOn(void);
static void switchOff(void);


/* ===================================================================== *
   Game suspend / resume / terminate
 * ===================================================================== */

// ------------------------------------------------------------------------
// Suspend all game activity

void suspendGame(void) {
	//dispMM("Suspending game");
	if (!gameSuspendFlag) {
		//localCursorOff();
		//quickSavePalette();
		displayDisable(GameSuspended);
		suspendAudio();
		pauseTimer();
#ifdef _WIN32
		suspendProcessResources();
#endif
		gameSuspendFlag = TRUE;
	}
}

// ------------------------------------------------------------------------
// check for suspended game

bool gameSuspended(void) {
	return gameSuspendFlag;
}

// ------------------------------------------------------------------------
// resume suspended game

void resumeGame(void) {
	if (gameSuspendFlag) {
		gameSuspendFlag = FALSE;
#ifdef _WIN32
		resumeProcessResources();
#endif
		resumeTimer();
		resumeAudio();
		displayEnable(GameSuspended);
#if _WIN32
		if (pWindow)
			pWindow->RestoreMinimizedDisplay();
#endif
	}
}


// ------------------------------------------------------------------------
// end game (normally)

void endGame(void) {
	blackOut();
	displayDisable(GameEnded);
	gameRunning = FALSE;
}


/* ===================================================================== *
   Display initialization
 * ===================================================================== */
void dayNightUpdate(void);
void fadeUp(void);
void displayUpdate(void);
void drawMainDisplay(void);

void niceScreenStartup(void) {
	blackOut();
	disablePaletteChanges();
	mainEnable();
	closeLoadMode();
#ifdef _WIN32
	localCursorOn();
#endif
	pointer.move(Point16(320, 240));
	//pointer.hide();
	enablePaletteChanges();
	displayUpdate();
	dayNightUpdate();
	fadeUp();
	pointer.manditoryShow();                        //  hide mouse pointer
	reDrawScreen();
	//pointer.show();
	updateAllUserControls();
	reDrawScreen();
	mouseInfo.replaceObject();
	mouseInfo.clearGauge();
	mouseInfo.setText(NULL);
	mouseInfo.setIntent(GrabInfo::WalkTo);
	resetInputDevices();
}

// ------------------------------------------------------------------------
// backbuffer startup

void initBackPanel(void) {
	if (mainWindow)
		return;

	mainWindow = new BackWindow(
	                 Rect16(0, 0, screenWidth, screenHeight),
	                 0,
	                 cmdWindowFunc);
	if (mainWindow == nullptr)
		error("Error initializing the back panel");
}

/* ===================================================================== *
   Display disable flags
 * ===================================================================== */

// ------------------------------------------------------------------------
// enable / disable blitting

void displayEnable(DisplayDisabledBecause reason, bool onOff) {
	bool prev = displayEnabled();
	if (!onOff)
		displayStatus |= reason;
	else
		displayStatus &= (~reason);
	if (prev != displayEnabled()) {
		if (displayEnabled())
			switchOn();
		else
			switchOff();
	}
}

// ------------------------------------------------------------------------
// This is a check to see if blitting is enabled

bool displayEnabled(uint32 mask) {
	return true;
}

bool displayOkay(void) {
	return displayEnabled();
}


// ------------------------------------------------------------------------
// Main on/off swiotch for display

void mainEnable(void) {
	displayEnable(GameNotInitialized);
}

// ------------------------------------------------------------------------
// This is a check to see if blitting is enabled

void mainDisable(void) {
	displayDisable(GameNotInitialized);
}

// ------------------------------------------------------------------------
// On/Off hooks

static void switchOn(void) {
	enableUserControls();
}

static void switchOff(void) {
	disableUserControls();
}

/* ===================================================================== *
   Palette disable hooks
 * ===================================================================== */

void enablePaletteChanges(void) {
	paletteSuspendFlag = FALSE;
	paletteMayHaveChanged = TRUE;
}

void disablePaletteChanges(void) {
	paletteSuspendFlag = TRUE;
}

bool paletteChangesEnabled(void) {
	return !paletteSuspendFlag;
}


/* ===================================================================== *
   Refresh
 * ===================================================================== */

// ------------------------------------------------------------------------
// notice that screen may be dirty

void delayedDisplayEnable(void) {
	delayReDraw = FALSE;
}

// ------------------------------------------------------------------------
// notice that palette may be dirty

void externalPaletteIntrusion(void) {
	paletteMayHaveChanged = TRUE;
}

// ------------------------------------------------------------------------
// force a full screen redraw

void reDrawScreen(void) {
	//dispMM("refresh");
	Rect16 r = Rect16(0, 0, 640, 480);
	if (mainWindow && displayEnabled()) {
		//updateAllUserControls();
		drawMainDisplay();
		mainWindow->invalidate(r);
		delayReDraw = FALSE;
		if (paletteMayHaveChanged) {
			paletteMayHaveChanged = FALSE;
			assertCurrentPalette();
			paletteMayHaveChanged = FALSE;
		}
	} else
		delayReDraw = TRUE;
	//mainWindow->invalidate(r);
}


/* ===================================================================== *
   Clear screen
 * ===================================================================== */

void blackOut(void) {
	bool dispEnable = displayEnabled();
#ifdef _WIN32
	if (!dispEnable)
		resumeDDGraphics();
#endif
	mainPort.drawMode = drawModeReplace;
	mainPort.setColor(0);            //  fill screen with color
	mainPort.fillRect(Rect16(0, 0, 640, 480));
#ifdef _WIN32
	if (pWindow) {
		pWindow->SetEraseColor(0);
		pWindow->Erase();
		pWindow->FillBackBuffer();
		pWindow->Flip();
		pWindow->Erase();
	}
#endif
	lightsOut();
#ifdef _WIN32
	if (!dispEnable)
		suspendDDGraphics();
#endif
}


/* ===================================================================== *
   Loading screen
 * ===================================================================== */

// ------------------------------------------------------------------------
// enable / disable blitting

void showLoadMessage(void) {
	uint32 saved = displayStatus;
	displayStatus = 0;
	loadingScreen();
	displayStatus = saved;
}


/* ===================================================================== *
   Video mode save and restore for videos
 * ===================================================================== */

void pushVidState(void) {
}

void popVidState(void) {
}

} // end of namespace Saga2
