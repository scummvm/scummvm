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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "tony/globals.h"

namespace Tony {

Globals::Globals() {
	nextLoop = false;
	nextChannel = 0;
	nextSync = 0;
	curChannel = 0;
	flipflop = 0;
	curBackText = NULL;
	bTonyIsSpeaking = false;
	curChangedHotspot = 0;
	Tony = NULL;
	Pointer = NULL;
	Boxes = NULL;
	Loc = NULL;
	Inventory = NULL;
	Input = NULL;
	LoadLocation = NULL;
	UnloadLocation = NULL;
	LinkGraphicTask = NULL;
	Freeze = NULL;
	Unfreeze = NULL;
	WaitFrame = NULL;
	PlayMusic = NULL;
	WaitWipeEnd = NULL;
	CloseWipe = NULL;
	InitWipe = NULL;
	EnableGUI = NULL;
	DisableGUI = NULL;
	SetPalesati = NULL;

	dwTonyNumTexts = 0;
	bTonyInTexts = false;
	bStaticTalk = false;
	bPatIrqFreeze = false;
	bCfgInvLocked = false;
	bCfgInvNoScroll = false;
	bCfgTimerizedText = false;
	bCfgInvUp = false;
	bCfgAnni30 = false;
	bCfgAntiAlias = false;
	bCfgSottotitoli = false;
	bCfgTransparence = false;
	bCfgInterTips = false;
	bCfgDubbing = false;
	bCfgMusic = false;
	bCfgSFX = false;
	nCfgTonySpeed = 0;
	nCfgTextSpeed = 0;
	nCfgDubbingVolume = 0;
	nCfgMusicVolume = 0;
	nCfgSFXVolume = 0;
	bIdleExited = false;
	bSkipSfxNoLoop = false;
	bNoOcchioDiBue = false;
	curDialog = 0;
	curSonoriz = 0;
	bFadeOutStop = false;

//	OSystem::MutexRef cs[10];
//	OSystem::MutexRef vdb;
	Common::fill(&mut[0], &mut[10], (HANDLE)NULL);
	bSkipIdle = false;
	hSkipIdle = 0;
	lastMusic = 0;
	lastTappeto = 0;
	Common::fill(&tappeti[0], &tappeti[200], 0);
	SFM_nLoc = 0;
	vdb = NULL;

	// MPAL global variables
	mpalError = 0;
	lpiifCustom = NULL;
	lplpFunctions = NULL;
	lplpFunctionStrings = NULL;
	nObjs = 0;
	nVars = 0;
	hVars = NULL;
	lpmvVars = NULL;
	nMsgs = 0;
	hMsgs = NULL;
	lpmmMsgs = NULL;
	nDialogs = 0;
	hDialogs = NULL;
	lpmdDialogs = NULL;
	nItems = 0;
	hItems = NULL;
	lpmiItems = NULL;
	nLocations = 0;
	hLocations = NULL;
	lpmlLocations = NULL;
	nScripts = 0;
	hScripts = NULL;
	lpmsScripts = NULL;
	nResources = 0;
	lpResources = NULL;
	bExecutingAction = false;
	bExecutingDialog = false;
	Common::fill(&nPollingLocations[0], &nPollingLocations[MAXPOLLINGLOCATIONS], 0);
	Common::fill(&hEndPollingLocations[0], &hEndPollingLocations[MAXPOLLINGLOCATIONS], 0);
	Common::fill(&PollingThreads[0], &PollingThreads[MAXPOLLINGLOCATIONS], 0);
	hAskChoice = 0;
	hDoneChoice = 0;
	nExecutingAction = 0;
	nExecutingDialog = 0;
	nExecutingChoice = 0;
	nSelectedChoice = 0;
}

} // End of namespace Tony
