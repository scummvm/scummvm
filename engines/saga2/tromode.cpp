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

#include "common/config-manager.h"

#include "saga2/saga2.h"
#include "saga2/annoy.h"
#include "saga2/detection.h"
#include "saga2/fta.h"
#include "saga2/player.h"
#include "saga2/display.h"
#include "saga2/panel.h"
#include "saga2/vpal.h"

namespace Saga2 {

#define VIDEO_EXTENSION ".SMK"

#define INTRO_VID1  "TRIMARK" VIDEO_EXTENSION
#define INTRO_VID2  "INTRO" VIDEO_EXTENSION

#define INTRO_VID_DINO "TESTVID" VIDEO_EXTENSION

#define WIN_VID_1   "END_1" VIDEO_EXTENSION
#define WIN_VID_2   "END_2" VIDEO_EXTENSION
#define WIN_VID_3   "END_3A" VIDEO_EXTENSION
#define WIN_VID_4   "END_3B" VIDEO_EXTENSION

#define LOSE_VID    "END_4" VIDEO_EXTENSION


#define ERASE_BETWEEN true
#define VIDEO_X 0
#define VIDEO_Y 0

extern bool allPlayerActorsDead;

int16 OptionsDialog(bool disableSaveResume = false);
void SystemEventLoop();
void freeAllTileBanks();
void resetInputDevices();
void cursorFullHide(bool onOff);

static void doIntro();
static void doWintro(int16 whichOne);
static void doLostro();

static void waitForVideo();
void waitForInput();

static void TroModeSetup();
static void TroModeCleanup();

static bool abortFlag = false;
//DO_OUTRO_IN_CLEANUP
static int whichOutro = -1;

// ------------------------------------------------------------------------
// Play intro video

void setIntroMode() {
	blackOut();
	if (!abortFlag) {
		TroModeSetup();
		doIntro();
		TroModeCleanup();
	}
	showLoadMessage();
	resetInputDevices();
}

// ------------------------------------------------------------------------
// Play outro video

void setOutroMode() {
}

// ------------------------------------------------------------------------
// Winning videos
extern GameWorld            *currentWorld;          // pointer to the current world

void setWintroMode(int16 whichOne) {
	whichOutro = whichOne;
	allPlayerActorsDead = true;
}

// ------------------------------------------------------------------------
// Losing video

void fadeDown();
void fadeUp();
void dumpGBASE(char *msg);

void setLostroMode() {
	abortFlag = false;
	allPlayerActorsDead = false;
	if (GameMode::newmodeFlag)
		GameMode::update();

	if (!abortFlag) {
		freeAllTileBanks();
		TroModeSetup();
		if (whichOutro >= 0)
			doWintro(whichOutro);
		else
			doLostro();
		whichOutro = -1;
		TroModeCleanup();
	}
	OptionsDialog(true);
	reDrawScreen();
}

/* ===================================================================== *
   Entry & termination
 * ===================================================================== */

void TroModeExternEvent() {
	abortFlag = true;
}

// ------------------------------------------------------------------------
// Entry code

static void TroModeSetup() {
	suspendAudio();
	g_vm->_pointer->hide();
	g_vm->_pal->quickSavePalette();
	blackOut();
	displayDisable(PlayingVideo);
	pushVidState();
	resetInputDevices();
	abortFlag = false;
}

// ------------------------------------------------------------------------
// Exit

static void TroModeCleanup() {
	g_vm->endVideo();
	popVidState();
	displayEnable(PlayingVideo);
	blackOut();
	g_vm->_pal->quickRestorePalette();
	resumeAudio();
	g_vm->_pointer->show();
//	g_vm->_pointer->manditoryShow();                     //  hide mouse pointer
	resetInputDevices();
}

/* ===================================================================== *
   Wait for Event type routines
 * ===================================================================== */

// ------------------------------------------------------------------------
// Wait till a video completes

static void waitForVideo() {
	while (g_vm->checkVideo()) {
		SystemEventLoop();
		if (abortFlag)
			return;

		g_system->delayMillis(10);
	}
}

// ------------------------------------------------------------------------
// Wait till the user hits a key or clicks or screams or whatever

void waitForInput() {
	abortFlag = false;
	while (!abortFlag) {
		SystemEventLoop();
		if (abortFlag)
			return;

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

/* ===================================================================== *
   Video playback
 * ===================================================================== */

static void playAVideo(const char *fileName, int x, int y) { //, int16 from, int16 to )
	g_vm->startVideo(fileName, x, y);
	if (!g_vm->checkVideo()) {
		g_vm->endVideo();
		abortFlag = true;
		return;
	}
	waitForVideo();
}


/* ===================================================================== *
   These are the actual video routines
 * ===================================================================== */

// ------------------------------------------------------------------------
// intro video(s)

static void doIntro() {
	if (g_vm->getGameId() == GID_FTA2) {
		playAVideo(INTRO_VID1, 0, 0);
		abortFlag = false;
		playAVideo(INTRO_VID2, 0, 0);
	} else {
		playAVideo(INTRO_VID_DINO, 0, 0);
	}
}

// ------------------------------------------------------------------------
// one of several endings

static void doWintro(int16 whichOne) {
	switch (whichOne) {
	case 0:
		playAVideo(WIN_VID_1, 0, 0);
		return;
	case 1:
		playAVideo(WIN_VID_2, 0, 0);
		return;
	case 2:
		playAVideo(WIN_VID_3, 0, 0);
		return;
	case 3:
		playAVideo(WIN_VID_4, 0, 0);
		return;
	}
}

// ------------------------------------------------------------------------
// lost

static void doLostro() {
	playAVideo(LOSE_VID, 0, 0);
}

} // end of namespace Saga2
