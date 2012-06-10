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
	Common::fill(_nextMusic, _nextMusic + MAX_PATH, 0);
	_nextLoop = false;
	_nextChannel = 0;
	_nextSync = 0;
	_curChannel = 0;
	_flipflop = 0;
	_curBackText = NULL;
	_bTonyIsSpeaking = false;
	_curChangedHotspot = 0;
	_tony = NULL;
	_pointer = NULL;
	_boxes = NULL;
	_loc = NULL;
	_inventory = NULL;
	_input = NULL;
	_gfxEngine = NULL;
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

	_dwTonyNumTexts = 0;
	_bTonyInTexts = false;
	_bStaticTalk = false;
	_bPatIrqFreeze = false;
	_bCfgInvLocked = false;
	_bCfgInvNoScroll = false;
	_bCfgTimerizedText = false;
	_bCfgInvUp = false;
	_bCfgAnni30 = false;
	_bCfgAntiAlias = false;
	_bCfgSottotitoli = false;
	_bCfgTransparence = false;
	_bCfgInterTips = false;
	_bCfgDubbing = false;
	_bCfgMusic = false;
	_bCfgSFX = false;
	_bAlwaysDisplay = false;
	_nCfgTonySpeed = 0;
	_nCfgTextSpeed = 0;
	_nCfgDubbingVolume = 0;
	_nCfgMusicVolume = 0;
	_nCfgSFXVolume = 0;
	_bIdleExited = false;
	_bSkipSfxNoLoop = false;
	_bNoOcchioDiBue = false;
	_curDialog = 0;
	_curSonoriz = 0;
	_bFadeOutStop = false;

//	OSystem::MutexRef vdb;
	Common::fill(&_mut[0], &_mut[10], 0);
	_bSkipIdle = false;
	_hSkipIdle = 0;
	_lastMusic = 0;
	_lastTappeto = 0;
	Common::fill(&_tappeti[0], &_tappeti[200], 0);
	SFM_nLoc = 0;

	// MPAL global variables
	_mpalError = 0;
	_lpiifCustom = NULL;
	_lplpFunctions = NULL;
	_lplpFunctionStrings = NULL;
	_nObjs = 0;
	_nVars = 0;
	_hVars = NULL;
	_lpmvVars = NULL;
	_nMsgs = 0;
	_hMsgs = NULL;
	_lpmmMsgs = NULL;
	_nDialogs = 0;
	_hDialogs = NULL;
	_lpmdDialogs = NULL;
	_nItems = 0;
	_hItems = NULL;
	_lpmiItems = NULL;
	_nLocations = 0;
	_hLocations = NULL;
	_lpmlLocations = NULL;
	_nScripts = 0;
	_hScripts = NULL;
	_lpmsScripts = NULL;
	_nResources = 0;
	_lpResources = NULL;
	_bExecutingAction = false;
	_bExecutingDialog = false;
	Common::fill(&_nPollingLocations[0], &_nPollingLocations[MAXPOLLINGLOCATIONS], 0);
	Common::fill(&_hEndPollingLocations[0], &_hEndPollingLocations[MAXPOLLINGLOCATIONS], 0);
	Common::fill(&_pollingThreads[0], &_pollingThreads[MAXPOLLINGLOCATIONS], 0);
	_hAskChoice = 0;
	_hDoneChoice = 0;
	_nExecutingAction = 0;
	_nExecutingDialog = 0;
	_nExecutingChoice = 0;
	_nSelectedChoice = 0;
	_nTonyNextTalkType = RMTony::TALK_NORMAL;
	_saveTonyLoc = 0;

	for (int i = 0; i < 16; ++i)
		Common::fill((byte *)&_character[i], (byte *)&_character[i] + sizeof(CharacterStruct), 0);
	for (int i = 0; i < 10; ++i)
		Common::fill((byte *)&_mCharacter[i], (byte *)&_mCharacter[i] + sizeof(MCharacterStruct), 0);
	for (int i = 0; i < 256; ++i)
		Common::fill((byte *)&_changedHotspot[i], (byte *)&_changedHotspot[i] + sizeof(ChangedHotspotStruct), 0);
}

} // End of namespace Tony
