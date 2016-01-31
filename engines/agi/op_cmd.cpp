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
 */

#include "base/version.h"

#include "agi/agi.h"
#include "agi/inv.h"
#include "agi/sprite.h"
#include "agi/text.h"
#include "agi/graphics.h"
#include "agi/opcodes.h"
#include "agi/menu.h"
#include "agi/systemui.h"
#include "agi/words.h"

#include "common/random.h"
#include "common/textconsole.h"

namespace Agi {

#define getGameID() state->_vm->getGameID()
#define getFeatures() state->_vm->getFeatures()
#define getVersion() state->_vm->getVersion()
#define getLanguage() state->_vm->getLanguage()

void cmdIncrement(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	if (getVersion() < 0x2000) {
		if (state->vars[varNr] < 0xf0)
			++state->vars[varNr];
	} else {
		if (state->vars[varNr] != 0xff)
			++state->vars[varNr];
	}
}

void cmdDecrement(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	if (state->vars[varNr] != 0)
		--state->vars[varNr];
}

void cmdAssignN(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];

	state->vars[varNr] = value;

	// WORKAROUND for a bug in fan game "Get outta SQ"
	// Total number of points is stored in variable 7, which
	// is then incorrectly assigned to 0. Thus, when the game
	// is restarted, "Points 0 of 0" is shown. We set the
	// variable to the correct value here
	// Fixes bug #1942476 - "AGI: Fan(Get Outta SQ) - Score
	// is lost on restart"
	if (getGameID() == GID_GETOUTTASQ && varNr == 7)
		state->vars[varNr] = 8;
}

void cmdAddN(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];

	state->vars[varNr] += value;
}

void cmdSubN(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];

	state->vars[varNr] -= value;
}

void cmdAssignV(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->vars[varNr1] = state->vars[varNr2];
}

void cmdAddV(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->vars[varNr1] += state->vars[varNr2];
}

void cmdSubV(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->vars[varNr1] -= state->vars[varNr2];
}

void cmdMulN(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];

	state->vars[varNr] *= value;
}

void cmdMulV(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->vars[varNr1] *= state->vars[varNr2];
}

void cmdDivN(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];

	state->vars[varNr] /= value;
}

void cmdDivV(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->vars[varNr1] /= state->vars[varNr2];
}

void cmdRandomV1(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	state->vars[varNr] = state->_vm->_rnd->getRandomNumber(250);
}

void cmdRandom(AgiGame *state, uint8 *parameter) {
	uint16 valueMin = parameter[0];
	uint16 valueMax = parameter[1];
	uint16 varNr = parameter[2];

	state->vars[varNr] = state->_vm->_rnd->getRandomNumber(valueMax - valueMin) + valueMin;
}

void cmdLindirectN(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];

	state->vars[state->vars[varNr]] = value;
}

void cmdLindirectV(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->vars[state->vars[varNr1]] = state->vars[varNr2];
}

void cmdRindirect(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->vars[varNr1] = state->vars[state->vars[varNr2]];
}

void cmdSet(AgiGame *state, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	state->_vm->setflag(flagNr, true);
}

void cmdReset(AgiGame *state, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	state->_vm->setflag(flagNr, false);
}

void cmdToggle(AgiGame *state, uint8 *parameter) {
	AgiEngine *vm = state->_vm;
	uint16 flagNr = parameter[0];
	bool curFlagState = vm->getflag(flagNr);

	vm->setflag(flagNr, !curFlagState);
}

void cmdSetV(AgiGame *state, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	if (getVersion() < 0x2000) {
		state->vars[flagNr] = 1;
	} else {
		flagNr = state->vars[flagNr];
		
		state->_vm->setflag(flagNr, true);
	}
}

void cmdResetV(AgiGame *state, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	if (getVersion() < 0x2000) {
		state->vars[flagNr] = 0;
	} else {
		flagNr = state->vars[flagNr];

		state->_vm->setflag(flagNr, false);
	}
}

void cmdToggleV(AgiGame *state, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	if (getVersion() < 0x2000) {
		state->vars[flagNr] ^= 1;
	} else {
		AgiEngine *vm = state->_vm;
		flagNr = state->vars[flagNr];
		bool curFlagState = vm->getflag(flagNr);

		vm->setflag(flagNr, !curFlagState);
	}
}

void cmdNewRoom(AgiGame *state, uint8 *parameter) {
	uint16 newRoomNr = parameter[0];

	state->_vm->newRoom(newRoomNr);

	// WORKAROUND: Works around intro skipping bug (#1737343) in Gold Rush.
	// Intro was skipped because the enter-keypress finalizing the entering
	// of the copy protection string (Copy protection is in logic.128) was
	// left over to the intro scene (Starts with room 73 i.e. logic.073).
	// The intro scene checks for any keys pressed and if it finds any it
	// jumps to the game's start (Room 1 i.e. logic.001). We clear the
	// keyboard buffer when the intro sequence's first room (Room 73) is
	// loaded so that no keys from the copy protection scene can be left
	// over to cause the intro to skip to the game's start.
	if (getGameID() == GID_GOLDRUSH && newRoomNr == 73)
		state->keypress = 0;
}

void cmdNewRoomF(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	state->_vm->newRoom(state->vars[varNr]);
}

void cmdLoadView(AgiGame *state, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	state->_vm->agiLoadResource(RESOURCETYPE_VIEW, resourceNr);
}

void cmdLoadLogic(AgiGame *state, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	state->_vm->agiLoadResource(RESOURCETYPE_LOGIC, resourceNr);
}

void cmdLoadSound(AgiGame *state, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	state->_vm->agiLoadResource(RESOURCETYPE_SOUND, resourceNr);
}

void cmdLoadViewF(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	state->_vm->agiLoadResource(RESOURCETYPE_VIEW, state->vars[varNr]);
}

void cmdLoadLogicF(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	state->_vm->agiLoadResource(RESOURCETYPE_LOGIC, state->vars[varNr]);
}

void cmdDiscardView(AgiGame *state, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	state->_vm->agiUnloadResource(RESOURCETYPE_VIEW, resourceNr);
}

void cmdObjectOnAnything(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~(fOnWater | fOnLand);
}

void cmdObjectOnLand(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fOnLand;
}

void cmdObjectOnWater(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fOnWater;
}

void cmdObserveHorizon(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fIgnoreHorizon;
}

void cmdIgnoreHorizon(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fIgnoreHorizon;
}

void cmdObserveObjs(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fIgnoreObjects;
}

void cmdIgnoreObjs(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fIgnoreObjects;
}

void cmdObserveBlocks(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fIgnoreBlocks;
}

void cmdIgnoreBlocks(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fIgnoreBlocks;
}

void cmdSetHorizon(AgiGame *state, uint8 *parameter) {
	uint16 horizonValue = parameter[0];

	state->horizon = horizonValue;
}

void cmdGetPriority(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->vars[varNr] = screenObj->priority;
}

void cmdSetPriority(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 priority = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fFixedPriority;
	screenObj->priority = priority;
}

void cmdSetPriorityF(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fFixedPriority;
	screenObj->priority = state->vars[varNr];
}

void cmdReleasePriority(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fFixedPriority;
}

void cmdSetUpperLeft(AgiGame *state, uint8 *parameter) {				// do nothing (AGI 2.917)
}

void cmdStartUpdate(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->_vm->startUpdate(screenObj);
}

void cmdStopUpdate(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->_vm->stopUpdate(screenObj);
}

void cmdCurrentView(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->vars[varNr] = screenObj->currentViewNr;
}

void cmdCurrentCel(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->vars[varNr] = screenObj->currentCelNr;
	debugC(4, kDebugLevelScripts, "v%d=%d", varNr, state->vars[varNr]);
}

void cmdCurrentLoop(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->vars[varNr] = screenObj->currentLoopNr;
}

void cmdLastCel(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->vars[varNr] = screenObj->loopData->celCount - 1;
}

void cmdSetCel(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 celNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->_vm->setCel(screenObj, celNr);
	if (getVersion() >= 0x2000) {
		screenObj->flags &= ~fDontupdate;
	}
}

void cmdSetCelF(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->_vm->setCel(screenObj, state->vars[varNr]);
	screenObj->flags &= ~fDontupdate;
}

void cmdSetView(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 viewNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->_vm->setView(screenObj, viewNr);
}

void cmdSetViewF(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->_vm->setView(screenObj, state->vars[varNr]);
}

void cmdSetLoop(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->_vm->setLoop(screenObj, loopNr);
}

void cmdSetLoopF(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->_vm->setLoop(screenObj, state->vars[varNr]);
}

void cmdNumberOfLoops(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->vars[varNr] = screenObj->loopCount;
}

void cmdFixLoop(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fFixLoop;
}

void cmdReleaseLoop(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fFixLoop;
}

void cmdStepSize(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->stepSize = state->vars[varNr];
}

void cmdStepTime(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->stepTime = screenObj->stepTimeCount = state->vars[varNr];
}

void cmdCycleTime(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->cycleTime = screenObj->cycleTimeCount = state->vars[varNr];
}

void cmdStopCycling(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fCycling;
}

void cmdStartCycling(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fCycling;
}

void cmdNormalCycle(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->cycle = kCycleNormal;
	screenObj->flags |= fCycling;
}

void cmdReverseCycle(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->cycle = kCycleReverse;
	screenObj->flags |= fCycling;
}

void cmdSetDir(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->direction = state->vars[varNr];
}

void cmdGetDir(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->vars[varNr] = screenObj->direction;
}

void cmdGetRoomF(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->vars[varNr2] = state->_vm->objectGetLocation(state->vars[varNr1]);
}

void cmdPut(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];

	state->_vm->objectSetLocation(objectNr, state->vars[varNr]);
}

void cmdPutF(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];

	state->_vm->objectSetLocation(state->vars[varNr1], state->vars[varNr2]);
}

void cmdDrop(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];

	state->_vm->objectSetLocation(objectNr, 0);
}

void cmdGet(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];

	state->_vm->objectSetLocation(objectNr, EGO_OWNED);
}

void cmdGetV1(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];

	state->_vm->objectSetLocation(objectNr, EGO_OWNED_V1);
}

void cmdGetF(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	state->_vm->objectSetLocation(state->vars[varNr], EGO_OWNED);
}

void cmdWordToString(AgiGame *state, uint8 *parameter) {
	uint16 stringNr = parameter[0];
	uint16 wordNr = parameter[1];

	strcpy(state->strings[stringNr], state->_vm->_words->getEgoWord(wordNr));
}

void cmdOpenDialogue(AgiGame *state, uint8 *parameter) {
	state->_vm->_text->dialogueOpen();
}

void cmdCloseDialogue(AgiGame *state, uint8 *parameter) {
	state->_vm->_text->dialogueClose();
}

void cmdCloseWindow(AgiGame *state, uint8 *parameter) {
	state->_vm->_text->closeWindow();
}

void cmdStatusLineOn(AgiGame *state, uint8 *parameter) {
	TextMgr *text = state->_vm->_text;

	text->statusEnable();
	text->statusDraw();
}

void cmdStatusLineOff(AgiGame *state, uint8 *parameter) {
	TextMgr *text = state->_vm->_text;

	text->statusDisable();
	state->_vm->_text->statusClear();
}

void cmdShowObj(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];

	state->_vm->_sprites->showObject(objectNr);
}

void cmdShowObjV(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	state->_vm->_sprites->showObject(state->vars[varNr]);
}

void cmdSound(AgiGame *state, uint8 *parameter) {
	uint16 resourceNr = parameter[0];
	uint16 flagNr = parameter[1];

	state->_vm->_sound->startSound(resourceNr, flagNr);
}

void cmdStopSound(AgiGame *state, uint8 *parameter) {
	state->_vm->_sound->stopSound();
}

void cmdMenuInput(AgiGame *state, uint8 *parameter) {
	AgiEngine *vm = state->_vm;

	if (vm->getflag(VM_FLAG_MENUS_WORK)) {
		vm->_menu->delayedExecute();
	}
}

void cmdEnableItem(AgiGame *state, uint8 *parameter) {
	uint16 controlCode = parameter[0];

	state->_vm->_menu->itemEnable(controlCode);
}

void cmdDisableItem(AgiGame *state, uint8 *parameter) {
	uint16 controlCode = parameter[0];

	state->_vm->_menu->itemDisable(controlCode);
}

void cmdSubmitMenu(AgiGame *state, uint8 *parameter) {
	state->_vm->_menu->submit();
}

void cmdSetScanStart(AgiGame *state, uint8 *parameter) {
	state->_curLogic->sIP = state->_curLogic->cIP;
}

void cmdResetScanStart(AgiGame *state, uint8 *parameter) {
	state->_curLogic->sIP = 2;
}

void cmdSaveGame(AgiGame *state, uint8 *parameter) {
	if (state->automaticSave) {
		if (state->_vm->saveGameAutomatic()) {
			// automatic save succeded
			return;
		}
		// fall back to regular dialog otherwise
	}
	state->_vm->saveGameDialog();
}

void cmdLoadGame(AgiGame *state, uint8 *parameter) {
	if (state->automaticSave) {
		if (state->_vm->loadGameAutomatic()) {
			// automatic restore succeded
			return;
		}
		// fall back to regular dialog otherwise
	}
	state->_vm->loadGameDialog();
}

void cmdInitDisk(AgiGame *state, uint8 *parameter) {				// do nothing
}

void cmdLog(AgiGame *state, uint8 *parameter) {				// do nothing
}

void cmdTraceOn(AgiGame *state, uint8 *parameter) {				// do nothing
}

void cmdTraceInfo(AgiGame *state, uint8 *parameter) {				// do nothing
}

void cmdShowMem(AgiGame *state, uint8 *parameter) {
	state->_vm->_text->messageBox("Enough memory");
}

void cmdInitJoy(AgiGame *state, uint8 *parameter) { // do nothing
}

void cmdScriptSize(AgiGame *state, uint8 *parameter) {
	debug(0, "script.size(%d)", parameter[0]);
}

// This implementation is based on observations of Amiga's Gold Rush.
// You can try this out (in the original and in ScummVM) by writing "bird man"
// to enter Gold Rush's debug mode and then writing "show position" or "sp".
// TODO: Make the cycle and motion status lines more like in Amiga's Gold Rush.
// TODO: Add control status line (After stepsize, before cycle status).
// Don't know what the control status means yet, possibly flags?
// Examples of the control-value (Taken in the first screen i.e. room 1):
// 4051 (When ego is stationary),
// 471 (When walking on the first screen's bridge),
// 71 (When walking around, using the mouse or the keyboard).
void cmdObjStatusF(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[state->vars[varNr]];

	const char *cycleDesc;  // Object's cycle description line
	const char *motionDesc; // Object's motion description line
	char msg[256];          // The whole object status message

	// Generate cycle description line
	switch (screenObj->cycle) {
	case kCycleNormal:
		cycleDesc = "normal cycle";
		break;
	case kCycleEndOfLoop:
		cycleDesc = "end of loop";
		break;
	case kCycleRevLoop:
		cycleDesc = "reverse loop";
		break;
	case kCycleReverse:
		cycleDesc = "reverse cycle";
		break;
	default:
		cycleDesc = "unknown cycle type";
		break;
	}

	// Generate motion description line
	switch (screenObj->motionType) {
	case kMotionNormal:
		motionDesc = "normal motion";
		break;
	case kMotionWander:
		motionDesc = "wandering";
		break;
	case kMotionFollowEgo:
		motionDesc = "following ego";
		break;
	case kMotionMoveObj:
		// Amiga's Gold Rush! most probably uses "move to (x, y)"
		// here with real values for x and y. The same output
		// is used when moving the ego around using the mouse.
		motionDesc = "moving to a point";
		break;
	default:
		motionDesc = "unknown motion type";
		break;
	}

	sprintf(msg,
		"Object %d:\n" \
		"x: %d  xsize: %d\n" \
		"y: %d  ysize: %d\n" \
		"pri: %d\n" \
		"stepsize: %d\n" \
		"%s\n" \
		"%s",
		state->vars[varNr],
		screenObj->xPos, screenObj->xSize,
		screenObj->yPos, screenObj->ySize,
		screenObj->priority,
		screenObj->stepSize,
		cycleDesc,
		motionDesc);
	state->_vm->_text->messageBox(msg);
}

// unknown commands:
// unk_170: Force savegame name -- j5
// unk_171: script save -- j5
// unk_172: script restore -- j5
// unk_173: Activate keypressed control (ego only moves while key is pressed)
// unk_174: Change priority table (used in KQ4) -- j5
// unk_177: Disable menus completely -- j5
// unk_181: Deactivate keypressed control (default control of ego)
void cmdSetSimple(AgiGame *state, uint8 *parameter) {
	if (!(getFeatures() & (GF_AGI256 | GF_AGI256_2))) {
		int16 stringNr = parameter[0];
		const char *textPtr = nullptr;

		state->automaticSave = false;

		// Try to get description for automatic saves
		textPtr = state->strings[stringNr];
		strncpy(state->automaticSaveDescription, textPtr, sizeof(state->automaticSaveDescription));
		if (state->automaticSaveDescription[0]) {
			// We got it and it's set, so enable automatic saving
			state->automaticSave = true;
		}

	} else { // AGI256 and AGI256-2 use this unknown170 command to load 256 color pictures.
		// Load the picture. Similar to void cmdLoad_pic(AgiGame *state, uint8 *p).
		SpritesMgr *spritesMgr = state->_vm->_sprites;
		uint16 varNr = parameter[0];
		uint16 resourceNr = state->vars[varNr];

		spritesMgr->eraseSprites();
		state->_vm->agiLoadResource(RESOURCETYPE_PICTURE, resourceNr);

		// Draw the picture. Similar to void cmdDraw_pic(AgiGame *state, uint8 *p).
		state->_vm->_picture->decodePicture(resourceNr, false, true);
		spritesMgr->drawAllSpriteLists();
		state->pictureShown = false;

		// Show the picture. Similar to void cmdShow_pic(AgiGame *state, uint8 *p).
		state->_vm->setflag(VM_FLAG_OUTPUT_MODE, false);
		state->_vm->_text->closeWindow();
		state->_vm->_picture->showPic();
		state->pictureShown = true;

		// Loading trigger
		state->_vm->loadingTrigger_DrawPicture();
	}
}

void cmdPopScript(AgiGame *state, uint8 *parameter) {
	if (getVersion() >= 0x2915) {
		debug(0, "pop.script");
	}
}

void cmdHoldKey(AgiGame *state, uint8 *parameter) {
	if (getVersion() >= 0x3098) {
		state->_vm->_egoHoldKey = true;
	}
}

void cmdDiscardSound(AgiGame *state, uint8 *parameter) {
	if (getVersion() >= 0x2936) {
		debug(0, "discard.sound");
	}
}

void cmdHideMouse(AgiGame *state, uint8 *parameter) {
	// WORKAROUND: Turns off current movement that's being caused with the mouse.
	// This fixes problems with too many popup boxes appearing in the Amiga
	// Gold Rush's copy protection failure scene (i.e. the hanging scene, logic.192).
	// Previously multiple popup boxes appeared one after another if you tried
	// to walk somewhere else than to the right using the mouse.
	// FIXME: Write a proper implementation using disassembly and
	//        apply it to other games as well if applicable.
	//state->screenObjTable[SCREENOBJECTS_EGO_ENTRY].flags &= ~fAdjEgoXY;
	state->_vm->_game.mouseHidden = true;

	g_system->showMouse(false);
}

void cmdAllowMenu(AgiGame *state, uint8 *parameter) {
	uint16 allowed = parameter[0];

	if (getVersion() >= 0x3098) {
		if (allowed) {
			state->_vm->_menu->accessAllow();
		} else {
			state->_vm->_menu->accessDeny();
		}
	}
}

void cmdShowMouse(AgiGame *state, uint8 *parameter) {
	state->_vm->_game.mouseHidden = false;

	g_system->showMouse(true);
}

void cmdFenceMouse(AgiGame *state, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	uint16 varNr3 = parameter[2];
	uint16 varNr4 = parameter[3];

	state->mouseFence.moveTo(varNr1, varNr2);
	state->mouseFence.setWidth(varNr3 - varNr1);
	state->mouseFence.setHeight(varNr4 - varNr1);
}

void cmdReleaseKey(AgiGame *state, uint8 *parameter) {
	if (getVersion() >= 0x3098) {
		state->_vm->_egoHoldKey = false;
	}
}

void cmdAdjEgoMoveToXY(AgiGame *state, uint8 *parameter) {
	int8 x, y;

	switch (logicNamesCmd[182].argumentsLength()) {
	// The 2 arguments version is used at least in Amiga Gold Rush!
	// (v2.05 1989-03-09, Amiga AGI 2.316) in logics 130 and 150
	// (Using arguments (0, 0), (0, 7), (0, 8), (9, 9) and (-9, 9)).
	case 2:
		// Both arguments are signed 8-bit (i.e. in range -128 to +127).
		x = (int8) parameter[0];
		y = (int8) parameter[1];

		// Turn off ego's current movement caused with the mouse if
		// adj.ego.move.to.x.y is called with other arguments than previously.
		// Fixes weird looping behavior when walking to a ladder in the mines
		// (Rooms 147-162) in Gold Rush using the mouse. Sometimes the ego didn't
		// stop when walking to a ladder using the mouse but kept moving on the
		// ladder in a horizontally looping manner i.e. from right to left, from
		// right to left etc. In the Amiga Gold Rush the ego stopped when getting
		// onto the ladder so this is more like it (Although that may be caused
		// by something else because this command doesn't do any flag manipulations
		// in the Amiga version - checked it with disassembly).
		if (x != state->adjMouseX || y != state->adjMouseY)
			state->screenObjTable[SCREENOBJECTS_EGO_ENTRY].flags &= ~fAdjEgoXY;

		state->adjMouseX = x;
		state->adjMouseY = y;

		debugC(4, kDebugLevelScripts, "adj.ego.move.to.x.y(%d, %d)", x, y);
		break;
	// TODO: Check where (if anywhere) the 0 arguments version is used
	case 0:
	default:
		state->screenObjTable[SCREENOBJECTS_EGO_ENTRY].flags |= fAdjEgoXY;
		break;
	}
}

void cmdParse(AgiGame *state, uint8 *parameter) {
	TextMgr *text = state->_vm->_text;
	uint16 stringNr = parameter[0];

	state->vars[VM_VAR_WORD_NOT_FOUND] = 0;
	state->_vm->setflag(VM_FLAG_ENTERED_CLI, false);
	state->_vm->setflag(VM_FLAG_SAID_ACCEPTED_INPUT, false);

	state->_vm->_words->parseUsingDictionary(text->stringPrintf(state->strings[stringNr]));
}

void cmdCall(AgiGame *state, uint8 *parameter) {
	uint16 logicNr = parameter[0];
	int oldCIP;
	int oldLognum;

	// CM: we don't save sIP because set.scan.start can be
	//     used in a called script (fixes xmas demo)
	oldCIP = state->_curLogic->cIP;
	oldLognum = state->lognum;

	state->_vm->runLogic(logicNr);

	state->lognum = oldLognum;
	state->_curLogic = &state->logics[state->lognum];
	state->_curLogic->cIP = oldCIP;
}

void cmdCallF(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];

	cmdCall(state, &state->vars[varNr]);
}

void cmdDrawPicV1(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 resourceNr = state->vars[varNr];

	debugC(6, kDebugLevelScripts, "=== draw pic V1 %d ===", resourceNr);
	state->_vm->_picture->decodePicture(resourceNr, true);

	// TODO: check, if this was really done
	state->_vm->_text->promptClear();

	// Loading trigger
	state->_vm->loadingTrigger_DrawPicture();
}

void cmdDrawPic(AgiGame *state, uint8 *parameter) {
	SpritesMgr *spritesMgr = state->_vm->_sprites;
	uint16 varNr = parameter[0];
	uint16 resourceNr = state->vars[varNr];

	debugC(6, kDebugLevelScripts, "=== draw pic %d ===", resourceNr);

	spritesMgr->eraseSprites(); // === CHANGED ===
	state->_vm->_picture->decodePicture(resourceNr, true);
	spritesMgr->buildAllSpriteLists();
	spritesMgr->drawAllSpriteLists(); // === CHANGED ===
	//state->_vm->_sprites->blitBoth();
	//state->_vm->_sprites->commitBoth();
	state->pictureShown = false;
	debugC(6, kDebugLevelScripts, "--- end of draw pic %d ---", resourceNr);

	// WORKAROUND for a script bug which exists in SQ1, logic scripts
	// 20 and 110. Flag 103 is not reset correctly, which leads to erroneous
	// behavior from view 46 (the spider droid). View 46 is supposed to
	// follow ego and explode when it comes in contact with him. However, as
	// flag 103 is not reset correctly, when the player goes down the path
	// and back up, the spider is always at the base of the path (since it
	// can't go up) and kills the player when he goes down at ground level
	// (although the spider droid sprite itself seems to be correctly positioned).
	// With this workaround, when the player goes back to picture 20 (1 screen
	// above the ground), flag 103 is reset, thereby fixing this issue. Note
	// that this is a script bug and occurs in the original interpreter as well.
	// Fixes bug #3056: AGI: SQ1 (2.2 DOS ENG) bizzare exploding roger
	if (getGameID() == GID_SQ1 && resourceNr == 20)
		state->_vm->setflag(103, false);

	// Loading trigger
	state->_vm->loadingTrigger_DrawPicture();
}

void cmdShowPic(AgiGame *state, uint8 *parameter) {
	debugC(6, kDebugLevelScripts, "=== show pic ===");

	state->_vm->setflag(VM_FLAG_OUTPUT_MODE, false);
	state->_vm->_text->closeWindow();
	state->_vm->_picture->showPicWithTransition();
	state->pictureShown = true;

	debugC(6, kDebugLevelScripts, "--- end of show pic ---");
}

void cmdLoadPic(AgiGame *state, uint8 *parameter) {
	SpritesMgr *spritesMgr = state->_vm->_sprites;
	uint16 varNr = parameter[0];
	uint16 resourceNr = state->vars[varNr];

	spritesMgr->eraseSprites();
	state->_vm->agiLoadResource(RESOURCETYPE_PICTURE, resourceNr);
	spritesMgr->buildAllSpriteLists();
	spritesMgr->drawAllSpriteLists();
}

void cmdLoadPicV1(AgiGame *state, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 resourceNr = state->vars[varNr];

	state->_vm->agiLoadResource(RESOURCETYPE_PICTURE, resourceNr);
}

void cmdDiscardPic(AgiGame *state, uint8 *parameter) {
	debugC(6, kDebugLevelScripts, "--- discard pic ---");
	// do nothing
}

void cmdOverlayPic(AgiGame *state, uint8 *parameter) {
	SpritesMgr *spritesMgr = state->_vm->_sprites;
	uint16 varNr = parameter[0];
	uint16 resourceNr = state->vars[varNr];

	debugC(6, kDebugLevelScripts, "--- overlay pic ---");

	spritesMgr->eraseSprites();
	state->_vm->_picture->decodePicture(resourceNr, false);
	spritesMgr->buildAllSpriteLists();
	spritesMgr->drawAllSpriteLists();
	spritesMgr->showAllSpriteLists();
	state->pictureShown = false;

	// Loading trigger
	state->_vm->loadingTrigger_DrawPicture();
}

void cmdShowPriScreen(AgiGame *state, uint8 *parameter) {
	GfxMgr *gfx = state->_vm->_gfx;

	gfx->debugShowMap(1); // switch to priority map

	state->_vm->waitKey();

	gfx->debugShowMap(0); // switch back to visual map
}

void cmdAnimateObj(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	if (getVersion() < 0x2000) {
		if (screenObj->flags & fDidntMove)
			return;
	} else {
		if (screenObj->flags & fAnimated)
			return;
	}

	debugC(4, kDebugLevelScripts, "animate vt entry #%d", objectNr);
	screenObj->flags = fAnimated | fUpdate | fCycling;

	if (getVersion() < 0x2000) {
		screenObj->flags |= fDidntMove;
	}

	screenObj->motionType = kMotionNormal;
	screenObj->cycle = kCycleNormal;
	screenObj->direction = 0;
}

void cmdUnanimateAll(AgiGame *state, uint8 *parameter) {
	int i;

	state->_vm->_sprites->eraseSprites();

	for (i = 0; i < SCREENOBJECTS_MAX; i++)
		state->screenObjTable[i].flags &= ~(fAnimated | fDrawn);
}

void cmdDraw(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	if (screenObj->flags & fDrawn)
		return;

//	if (vt.ySize <= 0 || vt.xSize <= 0)
//		return;

	debugC(4, kDebugLevelScripts, "draw entry %d", screenObj->objectNr);

	screenObj->flags |= fUpdate;
	if (getVersion() >= 0x3000) {
		state->_vm->setLoop(screenObj, screenObj->currentLoopNr);
		state->_vm->setCel(screenObj, screenObj->currentCelNr);
	}

	SpritesMgr *sprites = state->_vm->_sprites;

	state->_vm->fixPosition(objectNr);
	screenObj->xPos_prev = screenObj->xPos;
	screenObj->yPos_prev = screenObj->yPos;
	screenObj->xSize_prev = screenObj->xSize;
	screenObj->ySize_prev = screenObj->ySize;
	//screenObj->celData2 = screenObj->celData;
	sprites->eraseRegularSprites();
	screenObj->flags |= fDrawn;
	sprites->buildRegularSpriteList();
	sprites->drawRegularSpriteList();
	sprites->showSprite(screenObj);
	screenObj->flags &= ~fDontupdate;

	debugC(4, kDebugLevelScripts, "vt entry #%d flags = %02x", objectNr, screenObj->flags);
}

void cmdErase(AgiGame *state, uint8 *parameter) {
	SpritesMgr *sprites = state->_vm->_sprites;
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	bool noUpdateFlag = false;

	if (!(screenObj->flags & fDrawn))
		return;

	sprites->eraseRegularSprites();
	if ((screenObj->flags & fUpdate) == 0) {
		noUpdateFlag = true;
		sprites->eraseStaticSprites();
	}

	screenObj->flags &= ~fDrawn;

	if (noUpdateFlag) {
		sprites->buildStaticSpriteList();
		sprites->drawStaticSpriteList();
	}
	sprites->buildRegularSpriteList();
	sprites->drawRegularSpriteList();
	sprites->showSprite(screenObj);
}

void cmdPosition(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 xPos = parameter[1];
	uint16 yPos = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = screenObj->xPos_prev = xPos;
	screenObj->yPos = screenObj->yPos_prev = yPos;
}

void cmdPositionV1(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 xPos = parameter[1];
	uint16 yPos = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = xPos;
	screenObj->yPos = yPos;
}

void cmdPositionF(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = screenObj->xPos_prev = state->vars[varNr1];
	screenObj->yPos = screenObj->yPos_prev = state->vars[varNr2];
}

void cmdPositionFV1(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = state->vars[varNr1];
	screenObj->yPos = state->vars[varNr2];
}

void cmdGetPosn(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	state->vars[varNr1] = (unsigned char)screenObj->xPos;
	state->vars[varNr2] = (unsigned char)screenObj->yPos;
}

void cmdReposition(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	int16 dx = (int8) state->vars[varNr1];
	int16 dy = (int8) state->vars[varNr2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "dx=%d, dy=%d", dx, dy);
	screenObj->flags |= fUpdatePos;

	if (dx < 0 && screenObj->xPos < -dx)
		screenObj->xPos = 0;
	else
		screenObj->xPos += dx;

	if (dy < 0 && screenObj->yPos < -dy)
		screenObj->yPos = 0;
	else
		screenObj->yPos += dy;

	state->_vm->fixPosition(objectNr);
}

void cmdRepositionV1(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 xPosPlus = parameter[1];
	uint16 yPosPlus = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos_prev = screenObj->xPos;
	screenObj->yPos_prev = screenObj->yPos;
	screenObj->flags |= fUpdatePos;

	screenObj->xPos = (screenObj->xPos + xPosPlus) & 0xff;
	screenObj->yPos = (screenObj->yPos + yPosPlus) & 0xff;
}

void cmdRepositionTo(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 xPos = parameter[1];
	uint16 yPos = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = xPos;
	screenObj->yPos = yPos;
	screenObj->flags |= fUpdatePos;
	state->_vm->fixPosition(objectNr);
}

void cmdRepositionToF(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = state->vars[varNr1];
	screenObj->yPos = state->vars[varNr2];
	screenObj->flags |= fUpdatePos;
	state->_vm->fixPosition(objectNr);
}

void cmdAddToPic(AgiGame *state, uint8 *parameter) {
	uint16 viewNr = parameter[0];
	uint16 loopNr = parameter[1];
	uint16 celNr = parameter[2];
	uint16 xPos = parameter[3];
	uint16 yPos = parameter[4];
	uint16 priority = parameter[5];
	uint16 border = parameter[6];

	state->_vm->_sprites->addToPic(viewNr, loopNr, celNr, xPos, yPos, priority, border);
}

void cmdAddToPicV1(AgiGame *state, uint8 *parameter) {
	uint16 viewNr = parameter[0];
	uint16 loopNr = parameter[1];
	uint16 celNr = parameter[2];
	uint16 xPos = parameter[3];
	uint16 yPos = parameter[4];
	uint16 priority = parameter[5];

	state->_vm->_sprites->addToPic(viewNr, loopNr, celNr, xPos, yPos, priority, -1);
}

void cmdAddToPicF(AgiGame *state, uint8 *parameter) {
	uint16 viewNr = state->vars[parameter[0]];
	uint16 loopNr = state->vars[parameter[1]];
	uint16 celNr = state->vars[parameter[2]];
	uint16 xPos = state->vars[parameter[3]];
	uint16 yPos = state->vars[parameter[4]];
	uint16 priority = state->vars[parameter[5]];
	uint16 border = state->vars[parameter[6]];

	state->_vm->_sprites->addToPic(viewNr, loopNr, celNr, xPos, yPos, priority, border);
}

void cmdForceUpdate(AgiGame *state, uint8 *parameter) {
	SpritesMgr *spritesMgr = state->_vm->_sprites;

	spritesMgr->eraseSprites();
	spritesMgr->buildAllSpriteLists();
	spritesMgr->drawAllSpriteLists();
	spritesMgr->showAllSpriteLists();
}

void cmdReverseLoop(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopFlag = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "o%d, f%d", objectNr, loopFlag);
	screenObj->cycle = kCycleRevLoop;
	screenObj->flags |= (fDontupdate | fUpdate | fCycling);
	screenObj->loop_flag = loopFlag;
	state->_vm->setflag(screenObj->loop_flag, false);
}

void cmdReverseLoopV1(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopFlag = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "o%d, f%d", objectNr, loopFlag);
	screenObj->cycle = kCycleRevLoop;
	state->_vm->setCel(screenObj, 0);
	screenObj->flags |= (fDontupdate | fUpdate | fCycling);
	screenObj->loop_flag = loopFlag;
	//screenObj->parm3 = 0;
}

void cmdEndOfLoop(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopFlag = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "o%d, f%d", objectNr, loopFlag);
	screenObj->cycle = kCycleEndOfLoop;
	screenObj->flags |= (fDontupdate | fUpdate | fCycling);
	screenObj->loop_flag = loopFlag;
	state->_vm->setflag(screenObj->loop_flag, false);
}

void cmdEndOfLoopV1(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopFlag = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "o%d, f%d", objectNr, loopFlag);
	screenObj->cycle = kCycleEndOfLoop;
	state->_vm->setCel(screenObj, 0);
	screenObj->flags |= (fDontupdate | fUpdate | fCycling);
	screenObj->loop_flag = loopFlag;
	//screenObj->parm3 = 0;
}

void cmdBlock(AgiGame *state, uint8 *parameter) {
	uint16 x1 = parameter[0];
	uint16 y1 = parameter[1];
	uint16 x2 = parameter[2];
	uint16 y2 = parameter[3];

	debugC(4, kDebugLevelScripts, "x1=%d, y1=%d, x2=%d, y2=%d", x1, y1, x2, y2);
	state->block.active = true;
	state->block.x1 = x1;
	state->block.y1 = y1;
	state->block.x2 = x2;
	state->block.y2 = y2;
}

void cmdUnblock(AgiGame *state, uint8 *parameter) {
	state->block.active = false;
}

void cmdNormalMotion(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->motionType = kMotionNormal;
}

void cmdStopMotion(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->direction = 0;
	screenObj->motionType = kMotionNormal;
	if (objectNr == 0) {		// ego only
		state->_vm->setVar(VM_VAR_EGO_DIRECTION, 0);
		state->playerControl = false;
	}
}

void cmdStopMotionV1(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fAnimated;
}

void cmdStartMotion(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->motionType = kMotionNormal;
	if (objectNr == 0) {		// ego only
		state->_vm->setVar(VM_VAR_EGO_DIRECTION, 0);
		state->playerControl = true;
	}
}

void cmdStartMotionV1(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fAnimated;
}

void cmdPlayerControl(AgiGame *state, uint8 *parameter) {
	ScreenObjEntry *screenObjEgo = &state->screenObjTable[SCREENOBJECTS_EGO_ENTRY];

	state->playerControl = true;

	if (screenObjEgo->motionType != kMotionEgo)
		screenObjEgo->motionType = kMotionNormal;
}

void cmdProgramControl(AgiGame *state, uint8 *parameter) {
	state->playerControl = false;
}

void cmdFollowEgo(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 followStepSize = parameter[1];
	uint16 followFlag = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->motionType = kMotionFollowEgo;
	if (followStepSize <= screenObj->stepSize) {
		screenObj->follow_stepSize = screenObj->stepSize;
	} else {
		screenObj->follow_stepSize = followStepSize;
	}
	screenObj->follow_flag = followFlag;
	screenObj->follow_count = 255;

	if (getVersion() < 0x2000) {
		state->vars[screenObj->follow_flag] = 0;
		screenObj->flags |= fUpdate | fAnimated;
	} else {
		state->_vm->setflag(screenObj->follow_flag, false);
		screenObj->flags |= fUpdate;
	}
}

void cmdMoveObj(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 moveX = parameter[1];
	uint16 moveY = parameter[2];
	uint16 stepSize = parameter[3];
	uint16 moveFlag = parameter[4];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];
	// _D (_D_WARN "o=%d, x=%d, y=%d, s=%d, f=%d", p0, p1, p2, p3, p4);

	screenObj->motionType = kMotionMoveObj;
	screenObj->move_x = moveX;
	screenObj->move_y = moveY;
	screenObj->move_stepSize = screenObj->stepSize;
	screenObj->move_flag = moveFlag;

	if (stepSize != 0)
		screenObj->stepSize = stepSize;

	if (getVersion() < 0x2000) {
		state->vars[moveFlag] = 0;
		screenObj->flags |= fUpdate | fAnimated;
	} else {
		state->_vm->setflag(screenObj->move_flag, false);
		screenObj->flags |= fUpdate;
	}

	if (objectNr == 0)
		state->playerControl = false;

	// AGI 2.272 (ddp, xmas) doesn't call move_obj!
	if (getVersion() > 0x2272)
		state->_vm->moveObj(screenObj);
}

void cmdMoveObjF(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 moveX = state->vars[parameter[1]];
	uint16 moveY = state->vars[parameter[2]];
	uint16 stepSize = state->vars[parameter[3]];
	uint16 moveFlag = parameter[4];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->motionType = kMotionMoveObj;
	screenObj->move_x = moveX;
	screenObj->move_y = moveY;
	screenObj->move_stepSize = screenObj->stepSize;
	screenObj->move_flag = moveFlag;

	if (stepSize != 0)
		screenObj->stepSize = stepSize;

	state->_vm->setflag(screenObj->move_flag, false);
	screenObj->flags |= fUpdate;

	if (objectNr == 0)
		state->playerControl = false;

	// AGI 2.272 (ddp, xmas) doesn't call move_obj!
	if (getVersion() > 0x2272)
		state->_vm->moveObj(screenObj);
}

void cmdWander(AgiGame *state, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	if (objectNr == 0)
		state->playerControl = false;

	screenObj->motionType = kMotionWander;
	if (getVersion() < 0x2000) {
		screenObj->flags |= fUpdate | fAnimated;
	} else {
		screenObj->flags |= fUpdate;
	}
}

void cmdSetGameID(AgiGame *state, uint8 *parameter) {
	uint16 textNr = parameter[0];

	if (state->_curLogic->texts && (textNr - 1) <= state->_curLogic->numTexts)
		Common::strlcpy(state->id, state->_curLogic->texts[textNr - 1], 8);
	else
		state->id[0] = 0;

	debug(0, "Game ID: \"%s\"", state->id);
}

void cmdPause(AgiGame *state, uint8 *parameter) {
	AgiEngine *vm = state->_vm;
	int originalClockState = state->clockEnabled;
	bool skipPause = false;

	state->clockEnabled = false;

	// We check in here, if a special key was specified to trigger menus.
	// If that's the case, normally triggering the menu should be handled inside handleController()
	// For the rare cases, where this approach doesn't work because the trigger is not mapped to a controller,
	//  we trigger the menu in here.
	//
	// for further study read the comments for handleController()
	//
	// This is needed for at least Mixed Up Mother Goose for Apple IIgs.
	if (state->specialMenuTriggerKey) {
		if (vm->_menu->isAvailable()) {
			// Pulldown-menu is actually available (was submitted)
			skipPause = true;

			// Check, if special trigger key is currently NOT mapped.
			if (vm->getSpecialMenuControllerSlot() < 0) {
				// menu trigger is not mapped, trigger menu
				vm->_menu->delayedExecute();
			} else {
				// menu trigger is mapped, do not replace "pause"
				skipPause = false;
			}
		} else {
			warning("menu is not available, doing regular pause game instead");
		}
	}

	if (!skipPause) {
		// Show pause message box
		state->_vm->_systemUI->pauseDialog();
	}

	state->clockEnabled = originalClockState;
}

void cmdSetMenu(AgiGame *state, uint8 *parameter) {
	uint16 textNr = parameter[0];

	debugC(4, kDebugLevelScripts, "text %02x of %02x", textNr, state->_curLogic->numTexts);

	if (state->_curLogic->texts != NULL && (textNr - 1) <= state->_curLogic->numTexts) {
		const char *menuText = state->_curLogic->texts[textNr - 1];

		state->_vm->_menu->addMenu(menuText);
	}
}

void cmdSetMenuItem(AgiGame *state, uint8 *parameter) {
	uint16 textNr = parameter[0] - 1;
	uint16 controllerSlot = parameter[1];

	debugC(4, kDebugLevelScripts, "text %02x of %02x", textNr, state->_curLogic->numTexts);

	if (state->_curLogic->texts != NULL && textNr <= state->_curLogic->numTexts) {
		const char *menuItemText = state->_curLogic->texts[textNr];

		state->_vm->_menu->addMenuItem(menuItemText, controllerSlot);
	}
}

void cmdVersion(AgiGame *state, uint8 *parameter) {
	char ver2Msg[] =
	    "\n"
	    "                               \n\n"
	    "  ScummVM Sierra AGI v%x.%03x";
	char ver3Msg[] =
	    "\n"
	    "                             \n\n"
	    "ScummVM Sierra AGI v%x.002.%03x";

	Common::String verMsg = TITLE " v%s";

	int ver = getVersion();
	int maj = (ver >> 12) & 0xf;
	int min = ver & 0xfff;

	verMsg += (maj == 2 ? ver2Msg : ver3Msg);
	verMsg = Common::String::format(verMsg.c_str(), gScummVMVersion, maj, min);

	state->_vm->_text->messageBox(verMsg.c_str());
}

void cmdConfigureScreen(AgiGame *state, uint8 *parameter) {
	TextMgr *textMgr = state->_vm->_text;
	uint16 lineMinPrint = parameter[0];
	uint16 promptRow = parameter[1];
	uint16 statusRow = parameter[2];

	state->_vm->_text->configureScreen(lineMinPrint);

	textMgr->statusRow_Set(statusRow);
	textMgr->promptRow_Set(promptRow);
}

void cmdTextScreen(AgiGame *state, uint8 *parameter) {
	GfxMgr  *gfxMgr = state->_vm->_gfx;
	TextMgr *textMgr = state->_vm->_text;

	debugC(4, kDebugLevelScripts, "switching to text mode");

	state->gfxMode = false;
	gfxMgr->setPalette(false); // set text-mode palette
	textMgr->charAttrib_Set(textMgr->_textAttrib.foreground, textMgr->_textAttrib.background);
	gfxMgr->clearDisplay(0);
	textMgr->clearLines(0, 24, textMgr->_textAttrib.combinedBackground);
}

void cmdGraphics(AgiGame *state, uint8 *parameter) {
	debugC(4, kDebugLevelScripts, "switching to graphics mode");

	state->_vm->redrawScreen();
}

void cmdSetTextAttribute(AgiGame *state, uint8 *parameter) {
	int16 foreground = parameter[0];
	int16 background = parameter[1];
	state->_vm->_text->charAttrib_Set(foreground, background);
}

void cmdStatus(AgiGame *state, uint8 *parameter) {
	TextMgr *textMgr = state->_vm->_text;
	InventoryMgr *inventoryMgr = state->_vm->_inventory;

	textMgr->inputEditOn();
	textMgr->charAttrib_Push();
	textMgr->charAttrib_Set(0, 15);

	cmdTextScreen(state, parameter);

	inventoryMgr->show();

	//invent_state = 0;
	textMgr->charAttrib_Pop();
	state->_vm->redrawScreen();
}

void cmdQuit(AgiGame *state, uint8 *parameter) {
	uint16 withoutPrompt = parameter[0];
//	const char *buttons[] = { "Quit", "Continue", NULL };

	state->_vm->_sound->stopSound();
	if (withoutPrompt) {
		state->_vm->quitGame();
	} else {
		if (state->_vm->_systemUI->quitDialog()) {
			state->_vm->quitGame();
		}
	}
}

void cmdQuitV1(AgiGame *state, uint8 *parameter) {
	state->_vm->_sound->stopSound();
	state->_vm->quitGame();
}

void cmdRestartGame(AgiGame *state, uint8 *parameter) {
	bool doRestart = false;

	state->_vm->_sound->stopSound();

	if (state->_vm->getflag(VM_FLAG_AUTO_RESTART)) {
		doRestart = true;
	} else {
		doRestart = state->_vm->_systemUI->restartDialog();
	}

	if (doRestart) {
		state->_vm->_restartGame = true;
		state->_vm->setflag(VM_FLAG_RESTART_GAME, true);
		state->_vm->_menu->itemEnableAll();
	}
}

void cmdDistance(AgiGame *state, uint8 *parameter) {
	uint16 objectNr1 = parameter[0];
	uint16 objectNr2 = parameter[1];
	uint16 destVarNr = parameter[2];
	int16 x1, y1, x2, y2, d;
	ScreenObjEntry *screenObj1 = &state->screenObjTable[objectNr1];
	ScreenObjEntry *screenObj2 = &state->screenObjTable[objectNr2];

	if (screenObj1->flags & fDrawn && screenObj2->flags & fDrawn) {
		x1 = screenObj1->xPos + screenObj1->xSize / 2;
		y1 = screenObj1->yPos;
		x2 = screenObj2->xPos + screenObj2->xSize / 2;
		y2 = screenObj2->yPos;
		d = ABS(x1 - x2) + ABS(y1 - y2);
		if (d > 0xfe)
			d = 0xfe;
	} else {
		d = 0xff;
	}

	// WORKAROUND: Fixes King's Quest IV's script bug #3067 (KQ4: Zombie bug).
	// This bug also happens in the original interpreter.
	// In the graveyard (Rooms 16 and 18) at night if you had the Obsidian Scarab (Item 4)
	// and you were very close to a spot where a zombie was going to rise up from the
	// ground you could reproduce the bug. Just standing there and letting the zombie
	// try to rise up the Obsidian Scarab would repel the zombie immediately and that
	// would make the script bug so that the zombie would still come up but it just
	// wouldn't chase Rosella around anymore. If it had worked correctly the zombie
	// wouldn't have come up at all or it would have come up and gone back down
	// immediately. The latter approach is the one implemented here.
	if (getGameID() == GID_KQ4 && (state->vars[VM_VAR_CURRENT_ROOM] == 16 || state->vars[VM_VAR_CURRENT_ROOM] == 18) && destVarNr >= 221 && destVarNr <= 223) {
		// Rooms 16 and 18 are graveyards where three zombies come up at night. They use logics 16 and 18.
		// Variables 221-223 are used to save the distance between each zombie and Rosella.
		// Variables 155, 156 and 162 are used to save the state of each zombie in room 16.
		// Variables 155, 156 and 158 are used to save the state of each zombie in room 18.
		// Rosella gets turned to a zombie only if any of the zombies is under 10 units away
		// from her and she doesn't have the Obsidian Scarab (Item 4). Likewise Rosella makes
		// a zombie go back into the ground if the zombie comes under 15 units away from her
		// and she has the Obsidian Scarab. To ensure a zombie always first rises up before
		// checking for either of the aforementioned conditions (Rosella getting turned to
		// a zombie or the zombie getting turned away by the scarab) we make it appear the
		// zombie is far away from Rosella if the zombie is not already up and chasing her.
		enum zombieStates {ZOMBIE_SET_TO_RISE_UP, ZOMBIE_RISING_UP, ZOMBIE_CHASING_EGO};
		uint8 zombieStateVarNumList[] = {155, 156, (uint8)((state->vars[VM_VAR_CURRENT_ROOM] == 16) ? 162 : 158)};
		uint8 zombieNum         = destVarNr - 221;                         // Zombie's number (In range 0-2)
		uint8 zombieStateVarNum = zombieStateVarNumList[zombieNum]; // Number of the variable containing zombie's state
		uint8 zombieState       = state->vars[zombieStateVarNum];   // Zombie's state
		// If zombie is not chasing Rosella then set its distance from Rosella to the maximum
		if (zombieState != ZOMBIE_CHASING_EGO)
			d = 0xff;
	}

	state->vars[destVarNr] = (unsigned char)d;
}

void cmdAcceptInput(AgiGame *state, uint8 *parameter) {
	TextMgr *textMgr = state->_vm->_text;

	debugC(4, kDebugLevelScripts | kDebugLevelInput, "input normal");

	state->_vm->newInputMode(INPUTMODE_NORMAL);

	textMgr->promptEnable();
	textMgr->promptRedraw();
}

void cmdPreventInput(AgiGame *state, uint8 *parameter) {
	TextMgr *textMgr = state->_vm->_text;

	debugC(4, kDebugLevelScripts | kDebugLevelInput, "no input");

	state->_vm->newInputMode(INPUTMODE_NONE);

	textMgr->promptDisable();

	textMgr->inputEditOn();
	textMgr->clearLine(textMgr->promptRow_Get(), 0);
}

void cmdCancelLine(AgiGame *state, uint8 *parameter) {
	state->_vm->_text->promptCancelLine();
}

void cmdEchoLine(AgiGame *state, uint8 *parameter) {
	TextMgr *textMgr = state->_vm->_text;

	if (textMgr->promptIsEnabled()) {
		textMgr->promptEchoLine();
	}
}

void cmdGetString(AgiGame *state, uint8 *parameter) {
	TextMgr *textMgr = state->_vm->_text;
	int16 stringDestNr = parameter[0];
	int16 leadInTextNr = parameter[1] - 1;
	int16 stringRow = parameter[2];
	int16 stringColumn = parameter[3];
	int16 stringMaxLen = parameter[4];
	bool previousEditState = false;
	const char *leadInTextPtr = nullptr;

	if (stringMaxLen > TEXT_STRING_MAX_SIZE)
		stringMaxLen = TEXT_STRING_MAX_SIZE;

	debugC(4, kDebugLevelScripts, "%d %d %d %d %d", stringDestNr, leadInTextNr, stringRow, stringColumn, stringMaxLen);

	previousEditState = textMgr->inputGetEditStatus();

	textMgr->charPos_Push();
	textMgr->inputEditOn();

	// Workaround for SQLC bug.
	// See Sarien bug #792125 for details
//	if (promptRow > 24)
//		promptRow = 24;
//	if (promptColumn > 39)
//		promptColumn = 39;

	if (stringRow < 25) {
		textMgr->charPos_Set(stringRow, stringColumn);
	}

	if (state->_curLogic->texts && state->_curLogic->numTexts >= leadInTextNr) {
		leadInTextPtr = state->_curLogic->texts[leadInTextNr];

		leadInTextPtr = textMgr->stringPrintf(leadInTextPtr);
		leadInTextPtr = textMgr->stringWordWrap(leadInTextPtr, 40); // ?? not absolutely sure

		textMgr->displayText(leadInTextPtr);
	}

	state->_vm->cycleInnerLoopActive(CYCLE_INNERLOOP_GETSTRING);

	textMgr->stringSet("");
	textMgr->stringEdit(stringMaxLen);

	// copy string to destination
	// TODO: not sure if set all the time or only when ENTER is pressed
	strcpy(&state->_vm->_game.strings[stringDestNr][0], (char *)textMgr->_inputString);

	textMgr->charPos_Pop();

	if (!previousEditState) {
		textMgr->inputEditOff();
	}
}

void cmdGetNum(AgiGame *state, uint8 *parameter) {
	TextMgr *textMgr = state->_vm->_text;
	int16 leadInTextNr = parameter[0] - 1;
	int16 numberDestVarNr = parameter[1];
	const char *leadInTextPtr = nullptr;

	debugC(4, kDebugLevelScripts, "%d %d", leadInTextNr, numberDestVarNr);

	textMgr->inputEditOn();
	textMgr->charPos_Set(textMgr->promptRow_Get(), 0);

	if (state->_curLogic->texts && state->_curLogic->numTexts >= leadInTextNr) {
		leadInTextPtr = state->_curLogic->texts[leadInTextNr];

		leadInTextPtr = textMgr->stringPrintf(leadInTextPtr);
		leadInTextPtr = textMgr->stringWordWrap(leadInTextPtr, 40); // ?? not absolutely sure

		textMgr->displayText(leadInTextPtr);
	}

	textMgr->inputEditOff();

	state->_vm->cycleInnerLoopActive(CYCLE_INNERLOOP_GETNUMBER);

	textMgr->stringSet("");
	textMgr->stringEdit(3);

	textMgr->promptRedraw();

	state->vars[numberDestVarNr] = atoi((char *)textMgr->_inputString);

	debugC(4, kDebugLevelScripts, "[%s] -> %d", state->strings[MAX_STRINGS], state->vars[numberDestVarNr]);
}

void cmdSetCursorChar(AgiGame *state, uint8 *parameter) {
	TextMgr *textMgr = state->_vm->_text;
	uint16 textNr = parameter[0] - 1;

	if (state->_curLogic->texts != NULL && textNr <= state->_curLogic->numTexts) {
		textMgr->inputSetCursorChar(*state->_curLogic->texts[textNr]);
	} else {
		// default
		textMgr->inputSetCursorChar('_');
	}
}

void cmdSetKey(AgiGame *state, uint8 *parameter) {
	uint16 key = parameter[0] + (parameter[1] << 8);
	uint16 controllerSlot = parameter[2];
	int16 keyMappingSlot = -1;

	for (int i = 0; i < MAX_CONTROLLER_KEYMAPPINGS; i++) {
		if (keyMappingSlot == -1 && !state->controllerKeyMapping[i].keycode)
			keyMappingSlot = i;

		if (state->controllerKeyMapping[i].keycode == key && state->controllerKeyMapping[i].controllerSlot == controllerSlot)
			return;
	}

	if (keyMappingSlot == -1) {
		warning("Number of set.keys exceeded %d", MAX_CONTROLLER_KEYMAPPINGS);
		return;
	}

	debugC(4, kDebugLevelScripts, "cmdSetKey: %d %d %d", parameter[0], parameter[1], controllerSlot);
	state->controllerKeyMapping[keyMappingSlot].keycode = key;
	state->controllerKeyMapping[keyMappingSlot].controllerSlot = controllerSlot;

	state->controllerOccured[controllerSlot] = false;
}

void cmdSetString(AgiGame *state, uint8 *parameter) {
	uint16 stringNr = parameter[0];
	uint16 textNr = parameter[1] - 1;
	// CM: to avoid crash in Groza (str = 150)
	if (stringNr > MAX_STRINGS)
		return;
	strcpy(state->strings[stringNr], state->_curLogic->texts[textNr]);
}

void cmdDisplay(AgiGame *state, uint8 *parameter) {
	// V1 has 4 args
	int16 textNr = (getVersion() >= 0x2000 ? parameter[2] : parameter[3]);
	int16 textRow = parameter[0];
	int16 textColumn = parameter[1];

	state->_vm->_text->display(textNr, textRow, textColumn);
}

void cmdDisplayF(AgiGame *state, uint8 *parameter) {
	int16 textRow = state->vars[parameter[0]];
	int16 textColumn = state->vars[parameter[1]];
	int16 textNr = state->vars[parameter[2]];

	state->_vm->_text->display(textNr, textRow, textColumn);
}

void cmdClearTextRect(AgiGame *state, uint8 *parameter) {
	int16 textUpperRow = parameter[0];
	int16 textUpperColumn = parameter[1];
	int16 textLowerRow = parameter[2];
	int16 textLowerColumn = parameter[3];
	int16 color = state->_vm->_text->calculateTextBackground(parameter[4]);

	state->_vm->_text->clearBlock(textUpperRow, textUpperColumn, textLowerRow, textLowerColumn, color);
}

void cmdToggleMonitor(AgiGame *state, uint8 *parameter) {
	debug(0, "toggle.monitor");
}

void cmdClearLines(AgiGame *state, uint8 *parameter) {
	int16 textRowUpper = parameter[0];
	int16 textRowLower = parameter[1];
	int16 color = state->_vm->_text->calculateTextBackground(parameter[2]);

	// Residence 44 calls clear.lines(24,0,0), see Sarien bug #558423
	// Agent06 incorrectly calls clear.lines(1,150,0), see ScummVM bugs
	// #1935838 and #1935842
	if (textRowUpper > textRowLower) {
		warning("cmdClearLines: RowUpper higher than RowLower");
		textRowLower = textRowUpper;
	}
	state->_vm->_text->clearLines(textRowUpper, textRowLower, color);
}

void cmdPrint(AgiGame *state, uint8 *parameter) {
	int16 textNr = parameter[0];

	state->_vm->_text->print(textNr);
}

void cmdPrintF(AgiGame *state, uint8 *parameter) {
	int16 textNr = state->vars[parameter[0]];

	state->_vm->_text->print(textNr);
}

void cmdPrintAt(AgiGame *state, uint8 *parameter) {
	int16 textNr = parameter[0];
	int16 textRow = parameter[1];
	int16 textColumn = parameter[2];
	int16 textWidth = parameter[3];

	debugC(4, kDebugLevelScripts, "%d %d %d %d", textNr, textRow, textColumn, textWidth);

	state->_vm->_text->printAt(textNr, textRow, textColumn, textWidth);
}

void cmdPrintAtV(AgiGame *state, uint8 *parameter) {
	int16 textNr = state->vars[parameter[0]];
	int16 textRow = parameter[1];
	int16 textColumn = parameter[2];
	int16 textWidth = parameter[3];

	debugC(4, kDebugLevelScripts, "%d %d %d %d", textNr, textRow, textColumn, textWidth);

	state->_vm->_text->printAt(textNr, textRow, textColumn, textWidth);
}

void cmdPushScript(AgiGame *state, uint8 *parameter) {
	// We run AGIMOUSE always as a side effect
	//if (getFeatures() & GF_AGIMOUSE || true) {
		state->vars[27] = state->_vm->_mouse.button;
		state->vars[28] = state->_vm->_mouse.x / 2;
		state->vars[29] = state->_vm->_mouse.y;
	/*} else {
		if (getVersion() >= 0x2915) {
			debug(0, "push.script");
		}
	}*/
}

void cmdSetPriBase(AgiGame *state, uint8 *parameter) {
	uint16 priorityBase = parameter[0];

	debug(0, "Priority base set to %d", priorityBase);

	state->_vm->_gfx->setPriorityTable(priorityBase);
}

void cmdMousePosn(AgiGame *state, uint8 *parameter) {
	uint16 destVarNr1 = parameter[0];
	uint16 destVarNr2 = parameter[1];
	int16 mouseX = state->_vm->_mouse.x;
	int16 mouseY = state->_vm->_mouse.y;

	state->_vm->adjustPosToGameScreen(mouseX, mouseY);

	state->vars[destVarNr1] = mouseX;
	state->vars[destVarNr2] = mouseY;
}

void cmdShakeScreen(AgiGame *state, uint8 *parameter) {
	uint16 shakeCount = parameter[0];

	// AGIPAL uses shake.screen values between 100 and 109 to set the palette
	// (Checked the original AGIPAL-hack's shake.screen-routine's disassembly).
	if (shakeCount >= 100 && shakeCount < 110) {
		if (getFeatures() & GF_AGIPAL) {
			state->_vm->_gfx->setAGIPal(shakeCount);
			return;
		} else {
			warning("It looks like GF_AGIPAL flag is missing");
		}
	}

	state->_vm->_gfx->shakeScreen(shakeCount);
}

void cmdSetSpeed(AgiGame *state, uint8 *parameter) {
	// V1 command
	(void)state;
	(void)parameter;
	// speed = _v[p0];
}

void cmdSetItemView(AgiGame *state, uint8 *parameter) {
	// V1 command
	(void)state;
	(void)parameter;
}

void cmdCallV1(AgiGame *state, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	state->_vm->agiLoadResource(RESOURCETYPE_LOGIC, resourceNr);
	// FIXME: The following instruction looks incomplete.
	// Maybe something is meant to be assigned to, or read from,
	// the logic_list entry?
//	state->logic_list[++state->max_logics];
	// For now, just do the increment, to silence a clang warning
	++state->max_logics;
	state->vars[13] = 1;
}

void cmdNewRoomV1(AgiGame *state, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	warning("cmdNewRoomV1()");
	state->_vm->agiLoadResource(RESOURCETYPE_LOGIC, resourceNr);
	state->max_logics = 1;
	state->logic_list[1] = resourceNr;
	state->vars[13] = 1;
}

void cmdNewRoomVV1(AgiGame *state, uint8 *parameter) {
	uint16 resourceNr = state->vars[parameter[0]];

	warning("cmdNewRoomVV1()");
	state->_vm->agiLoadResource(RESOURCETYPE_LOGIC, resourceNr);
	state->max_logics = 1;
	state->logic_list[1] = resourceNr;
	state->vars[13] = 1;
}

void cmdUnknown(AgiGame *state, uint8 *parameter) {
	warning("Skipping unknown opcode %2X", *(state->_curLogic->data + state->_curLogic->cIP - 1));
}

/**
 * Execute a logic script
 * @param n  Number of the logic resource to execute
 */
int AgiEngine::runLogic(int n) {
	AgiGame *state = &_game;
	uint8 op = 0;
	uint8 p[CMD_BSIZE] = { 0 };
	int num = 0;
	ScriptPos sp;
	//int logic_index = 0;

	state->logic_list[0] = 0;
	state->max_logics = 0;

	debugC(2, kDebugLevelScripts, "=================");
	debugC(2, kDebugLevelScripts, "runLogic(%d)", n);

	sp.script = n;
	sp.curIP = 0;
	_game.execStack.push_back(sp);

	// If logic not loaded, load it
	if (~_game.dirLogic[n].flags & RES_LOADED) {
		debugC(4, kDebugLevelScripts, "logic %d not loaded!", n);
		agiLoadResource(RESOURCETYPE_LOGIC, n);
	}

	_game.lognum = n;
	_game._curLogic = &_game.logics[_game.lognum];

	_game._curLogic->cIP = _game._curLogic->sIP;

	_timerHack = 0;
	while (state->_curLogic->cIP < _game.logics[n].size && !(shouldQuit() || _restartGame)) {
		// TODO: old code, needs to be adjusted
#if 0
		if (_debug.enabled) {
			if (_debug.steps > 0) {
				if (_debug.logic0 || n) {
					debugConsole(n, lCOMMAND_MODE, NULL);
					_debug.steps--;
				}
			} else {
				_sprites->blitBoth();
				_sprites->commitBoth();
				do {
					mainCycle();
				} while (!_debug.steps && _debug.enabled);
				_sprites->eraseAllSprites();
			}
		}
#endif

		_game.execStack.back().curIP = state->_curLogic->cIP;

		char st[101];
		int sz = MIN(_game.execStack.size(), 100u);
		memset(st, '.', sz);
		st[sz] = 0;

		switch (op = *(state->_curLogic->data + state->_curLogic->cIP++)) {
		case 0xff:	// if (open/close)
			testIfCode(n);
			break;
		case 0xfe:	// goto
			// +2 covers goto size
			state->_curLogic->cIP += 2 + ((int16)READ_LE_UINT16(state->_curLogic->data + state->_curLogic->cIP));

			// timer must keep running even in goto loops,
			// but AGI engine can't do that :(
			if (_timerHack > 20) {
				pollTimer();
				updateTimer();
				_timerHack = 0;
			}
			break;
		case 0x00:	// return
			debugC(2, kDebugLevelScripts, "%sreturn() // Logic %d", st, n);
			debugC(2, kDebugLevelScripts, "=================");

//			if (getVersion() < 0x2000) {
//				if (logic_index < state->max_logics) {
//					n = state->logic_list[++logic_index];
//					state->_curLogic = &state->logics[n];
//					state->lognum = n;
//					ip = 2;
//					warning("running logic %d\n", n);
//					break;
//				}
//				_v[13]=0;
//			}

			_game.execStack.pop_back();
			return 1;
		default:
			num = logicNamesCmd[op].argumentsLength();
			memmove(p, state->_curLogic->data + state->_curLogic->cIP, num);
			memset(p + num, 0, CMD_BSIZE - num);

			debugC(2, kDebugLevelScripts, "%s%s(%d %d %d)", st, logicNamesCmd[op].name, p[0], p[1], p[2]);

			_agiCommands[op](&_game, p);
			state->_curLogic->cIP += num;
		}

//		if ((op == 0x0B || op == 0x3F || op == 0x40) && logic_index < state->max_logics) {
//			n = state->logic_list[++logic_index];
//			state->_curLogic = &state->logics[n];
//			state->lognum = n;
//			state->_curLogic_cIP = 2;
//			warning("running logic %d\n", n);
//		}

		if (_game.exitAllLogics)
			break;
	}

	_game.execStack.pop_back();

	return 0;		// after executing new.room()
}

void AgiEngine::executeAgiCommand(uint8 op, uint8 *p) {
	debugC(2, kDebugLevelScripts, "%s(%d %d %d)", logicNamesCmd[op].name, p[0], p[1], p[2]);

	_agiCommands[op](&_game, p);
}

} // End of namespace Agi
