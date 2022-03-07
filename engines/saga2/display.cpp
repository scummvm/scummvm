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

#include "common/config-manager.h"

#include "saga2/saga2.h"
#include "saga2/display.h"
#include "saga2/intrface.h"
#include "saga2/loadmsg.h"
#include "saga2/grabinfo.h"
#include "saga2/vpal.h"

namespace Saga2 {

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

void localCursorOn();
void localCursorOff();
void resetInputDevices();
APPFUNC(cmdWindowFunc);                      // main window event handler
static void switchOn();
static void switchOff();

// ------------------------------------------------------------------------
// end game (normally)

void endGame() {
	blackOut();
	displayDisable(GameEnded);
	g_vm->_gameRunning = false;
}


/* ===================================================================== *
   Display initialization
 * ===================================================================== */
void dayNightUpdate();
void fadeUp();
void displayUpdate();
void drawMainDisplay();

void niceScreenStartup() {
	if (ConfMan.hasKey("save_slot")) {
		cleanupGameState();
		loadSavedGameState(ConfMan.getInt("save_slot"));

		if (GameMode::newmodeFlag)
			GameMode::update();
		updateActiveRegions();
	}

	blackOut();
	disablePaletteChanges();
	mainEnable();
	closeLoadMode();
	g_vm->_pointer->move(Point16(320, 240));
	//g_vm->_pointer->hide();
	enablePaletteChanges();
	displayUpdate();
	dayNightUpdate();
	fadeUp();
	g_vm->_pointer->manditoryShow();                        //  hide mouse pointer
	reDrawScreen();
	//g_vm->_pointer->show();
	updateAllUserControls();
	reDrawScreen();
	g_vm->_mouseInfo->replaceObject();
	g_vm->_mouseInfo->clearGauge();
	g_vm->_mouseInfo->setText(nullptr);
	g_vm->_mouseInfo->setIntent(GrabInfo::WalkTo);
	resetInputDevices();
}

// ------------------------------------------------------------------------
// backbuffer startup

void initBackPanel() {
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

bool displayOkay() {
	return displayEnabled();
}


// ------------------------------------------------------------------------
// Main on/off swiotch for display

void mainEnable() {
	displayEnable(GameNotInitialized);
}

// ------------------------------------------------------------------------
// This is a check to see if blitting is enabled

void mainDisable() {
	displayDisable(GameNotInitialized);
}

// ------------------------------------------------------------------------
// On/Off hooks

static void switchOn() {
	enableUserControls();
}

static void switchOff() {
	disableUserControls();
}

/* ===================================================================== *
   Palette disable hooks
 * ===================================================================== */

void enablePaletteChanges() {
	paletteSuspendFlag = false;
	paletteMayHaveChanged = true;
}

void disablePaletteChanges() {
	paletteSuspendFlag = true;
}

bool paletteChangesEnabled() {
	return !paletteSuspendFlag;
}


/* ===================================================================== *
   Refresh
 * ===================================================================== */

// ------------------------------------------------------------------------
// notice that screen may be dirty

void delayedDisplayEnable() {
	delayReDraw = false;
}

// ------------------------------------------------------------------------
// notice that palette may be dirty

void externalPaletteIntrusion() {
	paletteMayHaveChanged = true;
}

// ------------------------------------------------------------------------
// force a full screen redraw

void reDrawScreen() {
	//dispMM("refresh");
	Rect16 r = Rect16(0, 0, 640, 480);
	if (mainWindow && displayEnabled()) {
		//updateAllUserControls();
		drawMainDisplay();
		mainWindow->invalidate(&r);
		delayReDraw = false;
		if (paletteMayHaveChanged) {
			paletteMayHaveChanged = false;
			g_vm->_pal->assertCurrentPalette();
			paletteMayHaveChanged = false;
		}
	} else
		delayReDraw = true;
	//mainWindow->invalidate(r);
}


/* ===================================================================== *
   Clear screen
 * ===================================================================== */

void blackOut() {
	g_vm->_mainPort.drawMode = drawModeReplace;
	g_vm->_mainPort.setColor(0);            //  fill screen with color
	g_vm->_mainPort.fillRect(Rect16(0, 0, 640, 480));
	g_vm->_pal->lightsOut();
}


/* ===================================================================== *
   Loading screen
 * ===================================================================== */

// ------------------------------------------------------------------------
// enable / disable blitting

void showLoadMessage() {
	uint32 saved = displayStatus;
	displayStatus = 0;
	loadingScreen();
	displayStatus = saved;
}


/* ===================================================================== *
   Video mode save and restore for videos
 * ===================================================================== */

void pushVidState() {
}

void popVidState() {
}

} // end of namespace Saga2
