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

#include "saga2/saga2.h"
#include "saga2/display.h"
#include "saga2/intrface.h"
#include "saga2/loadmsg.h"
#include "saga2/grabinfo.h"

namespace Saga2 {

extern bool             gameRunning;
extern bool             delayReDraw;
extern BackWindow       *mainWindow;            // main window...

/* ===================================================================== *
   Globals
 * ===================================================================== */

bool                        paletteMayHaveChanged = false;


/* ===================================================================== *
   Locals
 * ===================================================================== */

static uint32 displayStatus = GraphicsInit;
static bool paletteSuspendFlag = false;


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
static void switchOn(void);
static void switchOff(void);

// ------------------------------------------------------------------------
// end game (normally)

void endGame(void) {
	blackOut();
	displayDisable(GameEnded);
	gameRunning = false;
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
	g_vm->_mouseInfo->replaceObject();
	g_vm->_mouseInfo->clearGauge();
	g_vm->_mouseInfo->setText(NULL);
	g_vm->_mouseInfo->setIntent(GrabInfo::WalkTo);
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
	paletteSuspendFlag = false;
	paletteMayHaveChanged = true;
}

void disablePaletteChanges(void) {
	paletteSuspendFlag = true;
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
	delayReDraw = false;
}

// ------------------------------------------------------------------------
// notice that palette may be dirty

void externalPaletteIntrusion(void) {
	paletteMayHaveChanged = true;
}

// ------------------------------------------------------------------------
// force a full screen redraw

void reDrawScreen(void) {
	//dispMM("refresh");
	Rect16 r = Rect16(0, 0, 640, 480);
	if (mainWindow && displayEnabled()) {
		//updateAllUserControls();
		drawMainDisplay();
		mainWindow->invalidate(&r);
		delayReDraw = false;
		if (paletteMayHaveChanged) {
			paletteMayHaveChanged = false;
			assertCurrentPalette();
			paletteMayHaveChanged = false;
		}
	} else
		delayReDraw = true;
	//mainWindow->invalidate(r);
}


/* ===================================================================== *
   Clear screen
 * ===================================================================== */

void blackOut(void) {
	g_vm->_mainPort.drawMode = drawModeReplace;
	g_vm->_mainPort.setColor(0);            //  fill screen with color
	g_vm->_mainPort.fillRect(Rect16(0, 0, 640, 480));
	lightsOut();
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
