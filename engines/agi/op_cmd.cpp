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
#include "common/system.h"
#include "common/textconsole.h"

namespace Agi {

void cmdIncrement(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	byte   varVal = vm->getVar(varNr);

	if (vm->getVersion() < 0x2000) {
		if (varVal < 0xf0) {
			varVal++;
			vm->setVar(varNr, varVal);
		}
	} else {
		if (varVal != 0xff) {
			varVal++;
			vm->setVar(varNr, varVal);
		}
	}
}

void cmdDecrement(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	byte   varVal = vm->getVar(varNr);

	if (varVal != 0) {
		varVal--;
		vm->setVar(varNr, varVal);
	}
}

void cmdAssignN(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];

	vm->setVar(varNr, value);

	// WORKAROUND for a bug in fan game "Get outta SQ"
	// Total number of points is stored in variable 7, which
	// is then incorrectly assigned to 0. Thus, when the game
	// is restarted, "Points 0 of 0" is shown. We set the
	// variable to the correct value here
	// Fixes bug #3696 - "AGI: Fan(Get Outta SQ) - Score
	// is lost on restart"
	if (vm->getGameID() == GID_GETOUTTASQ && varNr == 7)
		vm->setVar(varNr, 8);
}

void cmdAddN(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];
	byte   varVal = vm->getVar(varNr);

	vm->setVar(varNr, varVal + value);
}

void cmdSubN(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];
	byte   varVal = vm->getVar(varNr);

	vm->setVar(varNr, varVal - value);
}

void cmdAssignV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1  = parameter[0];
	uint16 varNr2  = parameter[1];
	byte   varVal2 = vm->getVar(varNr2);

	vm->setVar(varNr1, varVal2);
}

void cmdAddV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	byte   varVal1 = vm->getVar(varNr1);
	byte   varVal2 = vm->getVar(varNr2);

	vm->setVar(varNr1, varVal1 + varVal2);
}

void cmdSubV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	byte   varVal1 = vm->getVar(varNr1);
	byte   varVal2 = vm->getVar(varNr2);

	vm->setVar(varNr1, varVal1 - varVal2);
}

void cmdMulN(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];
	byte   varVal = vm->getVar(varNr);

	vm->setVar(varNr, varVal * value);
}

void cmdMulV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	byte   varVal1 = vm->getVar(varNr1);
	byte   varVal2 = vm->getVar(varNr2);

	vm->setVar(varNr1, varVal1 * varVal2);
}

void cmdDivN(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];
	byte   varVal = vm->getVar(varNr);

	vm->setVar(varNr, varVal / value);
}

void cmdDivV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	byte   varVal1 = vm->getVar(varNr1);
	byte   varVal2 = vm->getVar(varNr2);

	vm->setVar(varNr1, varVal1 / varVal2);
}

void cmdRandomV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];

	vm->setVar(varNr, vm->_rnd->getRandomNumber(250));
}

void cmdRandom(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 valueMin = parameter[0];
	uint16 valueMax = parameter[1];
	uint16 varNr = parameter[2];

	vm->setVar(varNr, vm->_rnd->getRandomNumber(valueMax - valueMin) + valueMin);
}

void cmdLindirectN(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 value = parameter[1];
	byte   varVal = vm->getVar(varNr);

	vm->setVar(varVal, value);
}

void cmdLindirectV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	byte   varVal1 = vm->getVar(varNr1);
	byte   varVal2 = vm->getVar(varNr2);

	vm->setVar(varVal1, varVal2);
}

void cmdRindirect(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	byte   varVal2 = vm->getVar(varNr2);
	byte   value   = vm->getVar(varVal2);

	vm->setVar(varNr1, value);
}

void cmdSet(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	vm->setFlag(flagNr, true);
}

void cmdReset(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	vm->setFlag(flagNr, false);
}

void cmdToggle(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 flagNr = parameter[0];
	bool curFlagState = vm->getFlag(flagNr);

	vm->setFlag(flagNr, !curFlagState);
}

void cmdSetV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	if (vm->getVersion() < 0x2000) {
		vm->setVar(flagNr, 1);
	} else {
		flagNr = vm->getVar(flagNr);

		vm->setFlag(flagNr, true);
	}
}

void cmdResetV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	if (vm->getVersion() < 0x2000) {
		vm->setVar(flagNr, 0);
	} else {
		flagNr = vm->getVar(flagNr);

		vm->setFlag(flagNr, false);
	}
}

void cmdToggleV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 flagNr = parameter[0];

	if (vm->getVersion() < 0x2000) {
		byte value = vm->getVar(flagNr);
		vm->setVar(flagNr, value ^ 1);
	} else {
		flagNr = vm->getVar(flagNr);
		bool curFlagState = vm->getFlag(flagNr);

		vm->setFlag(flagNr, !curFlagState);
	}
}

void cmdNewRoom(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 newRoomNr = parameter[0];

	vm->newRoom(newRoomNr);
}

void cmdNewRoomF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	byte   value = vm->getVar(varNr);

	vm->newRoom(value);
}

void cmdLoadView(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	vm->loadResource(RESOURCETYPE_VIEW, resourceNr);
}

void cmdLoadLogic(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	vm->loadResource(RESOURCETYPE_LOGIC, resourceNr);
}

void cmdLoadSound(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	vm->loadResource(RESOURCETYPE_SOUND, resourceNr);
}

void cmdLoadViewF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	byte   value = vm->getVar(varNr);

	vm->loadResource(RESOURCETYPE_VIEW, value);
}

void cmdLoadLogicF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	byte   value = vm->getVar(varNr);

	vm->loadResource(RESOURCETYPE_LOGIC, value);
}

void cmdDiscardView(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	vm->unloadResource(RESOURCETYPE_VIEW, resourceNr);
}

void cmdObjectOnAnything(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~(fOnWater | fOnLand);
}

void cmdObjectOnLand(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fOnLand;
}

void cmdObjectOnWater(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fOnWater;
}

void cmdObserveHorizon(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fIgnoreHorizon;
}

void cmdIgnoreHorizon(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fIgnoreHorizon;
}

void cmdObserveObjs(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fIgnoreObjects;
}

void cmdIgnoreObjs(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fIgnoreObjects;
}

void cmdObserveBlocks(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fIgnoreBlocks;
}

void cmdIgnoreBlocks(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fIgnoreBlocks;
}

void cmdSetHorizon(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 horizonValue = parameter[0];

	state->horizon = horizonValue;
}

void cmdGetPriority(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setVar(varNr, screenObj->priority);
}

void cmdSetPriority(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 priority = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fFixedPriority;
	screenObj->priority = priority;
}

void cmdSetPriorityF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fFixedPriority;
	screenObj->priority = vm->getVar(varNr);
}

void cmdReleasePriority(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fFixedPriority;
}

void cmdSetUpperLeft(AgiGame *state, AgiEngine *vm, uint8 *parameter) {             // do nothing (AGI 2.917)
}

void cmdStartUpdate(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->startUpdate(screenObj);
}

void cmdStopUpdate(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->stopUpdate(screenObj);
}

void cmdCurrentView(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setVar(varNr, screenObj->currentViewNr);
}

void cmdCurrentCel(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setVar(varNr, screenObj->currentCelNr);
	debugC(4, kDebugLevelScripts, "v%d=%d", varNr, screenObj->currentCelNr);
}

void cmdCurrentLoop(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setVar(varNr, screenObj->currentLoopNr);
}

void cmdLastCel(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setVar(varNr, screenObj->loopData->celCount - 1);
}

void cmdSetCel(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 celNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setCel(screenObj, celNr);
	if (vm->getVersion() >= 0x2000) {
		screenObj->flags &= ~fDontUpdate;
	}
}

void cmdSetCelF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];
	byte   value = vm->getVar(varNr);

	vm->setCel(screenObj, value);
	screenObj->flags &= ~fDontUpdate;
}

void cmdSetView(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 viewNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setView(screenObj, viewNr);
}

void cmdSetViewF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];
	byte   value = vm->getVar(varNr);

	vm->setView(screenObj, value);
}

void cmdSetLoop(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setLoop(screenObj, loopNr);
}

void cmdSetLoopF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];
	byte   value = vm->getVar(varNr);

	vm->setLoop(screenObj, value);
}

void cmdNumberOfLoops(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setVar(varNr, screenObj->loopCount);
}

void cmdFixLoop(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fFixLoop;
}

void cmdReleaseLoop(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fFixLoop;
}

void cmdStepSize(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->stepSize = vm->getVar(varNr);
}

void cmdStepTime(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->stepTime = screenObj->stepTimeCount = vm->getVar(varNr);
}

void cmdCycleTime(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->cycleTime = screenObj->cycleTimeCount = vm->getVar(varNr);
}

void cmdStopCycling(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fCycling;
}

void cmdStartCycling(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fCycling;
}

void cmdNormalCycle(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->cycle = kCycleNormal;
	screenObj->flags |= fCycling;
}

void cmdReverseCycle(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->cycle = kCycleReverse;
	screenObj->flags |= fCycling;
}

void cmdSetDir(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->direction = vm->getVar(varNr);
}

void cmdGetDir(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setVar(varNr, screenObj->direction);
}

void cmdGetRoomF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	byte   varVal1 = vm->getVar(varNr1);

	vm->setVar(varNr2, vm->objectGetLocation(varVal1));
}

void cmdPut(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr = parameter[1];
	byte   varVal = vm->getVar(varNr);

	vm->objectSetLocation(objectNr, varVal);
}

void cmdPutF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	byte   varVal1 = vm->getVar(varNr1);
	byte   varVal2 = vm->getVar(varNr2);

	vm->objectSetLocation(varVal1, varVal2);
}

void cmdDrop(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];

	vm->objectSetLocation(objectNr, 0);
}

void cmdGet(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];

	vm->objectSetLocation(objectNr, EGO_OWNED);
}

void cmdGetV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];

	vm->objectSetLocation(objectNr, EGO_OWNED_V1);
}

void cmdGetF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	byte   varVal = vm->getVar(varNr);

	vm->objectSetLocation(varVal, EGO_OWNED);
}

void cmdWordToString(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 stringNr = parameter[0];
	uint16 wordNr = parameter[1];

	state->setString(stringNr, vm->_words->getEgoWord(wordNr));
}

void cmdOpenDialogue(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_text->dialogueOpen();
}

void cmdCloseDialogue(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_text->dialogueClose();
}

void cmdCloseWindow(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_text->closeWindow();
}

void cmdStatusLineOn(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *text = vm->_text;

	text->statusEnable();
	text->statusDraw();
}

void cmdStatusLineOff(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *text = vm->_text;

	text->statusDisable();
	text->statusClear();
}

void cmdShowObj(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];

	vm->_sprites->showObject(objectNr);
}

void cmdShowObjV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	byte   varVal = vm->getVar(varNr);

	vm->_sprites->showObject(varVal);
}

void cmdSound(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 resourceNr = parameter[0];
	uint16 flagNr = parameter[1];

	if (vm->getPlatform() == Common::kPlatformApple2 ||
		vm->getPlatform() == Common::kPlatformCoCo3) {
		// Play the sound until it finishes or until a key is pressed.
		// Sound playback is a blocking operation on these platforms.
		// If sound is off then playback is not started.
		if (vm->getFlag(VM_FLAG_SOUND_ON)) {
			vm->_sound->startSound(resourceNr, flagNr);
			vm->waitAnyKeyOrFinishedSound();
			vm->_sound->stopSound();
		}
		vm->setFlagOrVar(flagNr, true);
	} else {
		vm->_sound->startSound(resourceNr, flagNr);
	}
}

void cmdStopSound(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_sound->stopSound();
}

void cmdMenuInput(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if (vm->getFlag(VM_FLAG_MENUS_ACCESSIBLE)) {
		vm->_menu->delayedExecuteViaKeyboard();
	}
}

void cmdEnableItem(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 controlCode = parameter[0];

	vm->_menu->itemEnable(controlCode);
}

void cmdDisableItem(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 controlCode = parameter[0];

	vm->_menu->itemDisable(controlCode);
}

void cmdSubmitMenu(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_menu->submit();
}

void cmdSetScanStart(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	state->_curLogic->sIP = state->_curLogic->cIP;
}

void cmdResetScanStart(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	state->_curLogic->sIP = 2;
}

void cmdSaveGame(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if (vm->getVersion() >= 0x2272) {
		// this was only done since 2.272
		vm->_sound->stopSound();
	}

	PauseToken pt = vm->pauseEngine();

	if (state->automaticSave) {
		if (vm->saveGameAutomatic()) {
			// automatic save succeeded
			return;
		}
		// fall back to regular dialog otherwise
	}

	vm->saveGameDialog();
}

void cmdLoadGame(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if (vm->getVersion() >= 0x2272) {
		// this was only done since 2.272
		vm->_sound->stopSound();
	}

	PauseToken pt = vm->pauseEngine();

	if (state->automaticSave) {
		if (vm->loadGameAutomatic()) {
			// automatic restore succeeded
			return;
		}
		// fall back to regular dialog otherwise
	}

	vm->loadGameDialog();
}

void cmdInitDisk(AgiGame *state, AgiEngine *vm, uint8 *parameter) {             // do nothing
}

// The log command logs adds an entry to the game's log file and the console.
// Neither are enabled by default.
//
// Room <#>
// Input line: <text>
// <message>
//
// To see the logs in your console, use the following arguments to scummvm:
//   --debugflags=Scripts -d 1
// To see the logs in a file, create the "dumps" directory.
//
void cmdLog(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 textNr = parameter[0];
	if (state->_curLogic->texts != nullptr && (textNr - 1) <= state->_curLogic->numTexts) {
		byte currentRoom = vm->getVar(VM_VAR_CURRENT_ROOM);
		const char *inputLine = (char *)vm->_text->_promptPrevious;
		const char *message = state->_curLogic->texts[textNr - 1];

		Common::String logMessage = Common::String::format("Room %hhu\nInput line: %s\n%s\n",
														   currentRoom,
														   inputLine,
														   vm->_text->stringPrintf(message));

		debugCN(1, kDebugLevelScripts, "%s", logMessage.c_str());

		Common::DumpFile *&dumpFile = vm->_logFile;
		if (!dumpFile) {
			// Creates a unique log file based on game and system time.
			// For example: dumps/agi.kq.20221013214511.log
			TimeDate date;
			vm->_system->getTimeAndDate(date, true);
			Common::Path logFileName(Common::String::format("dumps/agi.%s.%d%02d%02d%02d%02d%02d.log",
				vm->getTargetName().c_str(),
				date.tm_year + 1900,
				date.tm_mon + 1,
				date.tm_mday,
				date.tm_hour,
				date.tm_min,
				date.tm_sec), '/');

			dumpFile = new Common::DumpFile();
			dumpFile->open(logFileName);
		}
		// The logs will only be written if the "dumps" folder has been created by
		// the user.
		if (dumpFile->isOpen()) {
			dumpFile->writeString(logMessage);
			dumpFile->flush();
		}
	}
}

void cmdTraceOn(AgiGame *state, AgiEngine *vm, uint8 *parameter) {              // do nothing
}

void cmdTraceInfo(AgiGame *state, AgiEngine *vm, uint8 *parameter) {                // do nothing
}

void cmdShowMem(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_text->messageBox("Enough memory");
}

void cmdInitJoy(AgiGame *state, AgiEngine *vm, uint8 *parameter) { // do nothing
}

void cmdScriptSize(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
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
void cmdObjStatusF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[vm->getVar(varNr)];

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

	Common::sprintf_s(msg,
	        "Object %d:\n" \
	        "x: %d  xsize: %d\n" \
	        "y: %d  ysize: %d\n" \
	        "pri: %d\n" \
	        "stepsize: %d\n" \
	        "%s\n" \
	        "%s",
	        vm->getVar(varNr),
	        screenObj->xPos, screenObj->xSize,
	        screenObj->yPos, screenObj->ySize,
	        screenObj->priority,
	        screenObj->stepSize,
	        cycleDesc,
	        motionDesc);
	vm->_text->messageBox(msg);
}

// only known to used by MMMG for setting the player's name as the save description
void cmdSetSimple(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if ((vm->getVersion() < 0x2425) || (vm->getVersion() == 0x2440)) {
		// was not available before 2.2425, but also not available in 2.440
		warning("set.simple called, although not available for current AGI version");
		return;
	}

	int16 stringNr = parameter[0];

	// Try to get description for automatic saves
	const char *textPtr = state->getString(stringNr);
	strncpy(state->automaticSaveDescription, textPtr, sizeof(state->automaticSaveDescription));
	state->automaticSaveDescription[sizeof(state->automaticSaveDescription) - 1] = '\0';

	// enable automatic saving if description isn't empty
	state->automaticSave = (state->automaticSaveDescription[0] != '\0');
}

// pop.script was not available until 2.425, and also not available in 2.440
void cmdPopScript(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if ((vm->getVersion() < 0x2425) || (vm->getVersion() == 0x2440)) {
		// was not available before 2.2425, but also not available in 2.440
		warning("pop.script called, although not available for current AGI version");
		return;
	}

	debug(0, "pop.script");
}

// discard.sound only existed on Apple IIgs. Apple IIgs sound resources were
// relatively large, so scripts would unload them from memory when finished.
void cmdDiscardSound(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	debug(0, "discard.sound");
}

void cmdShowMouse(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if (state->mouseEnabled) {
		state->mouseHidden = false;

		vm->_system->showMouse(true);
	}
}

void cmdHideMouse(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if ((vm->getGameID() == GID_MH1) && (vm->getPlatform() == Common::kPlatformApple2GS)) {
		// Called right after beating arcade sequence on day 4 in the hospital Parameter is "1".
		// Right before cutscene. show.mouse isn't called. Probably different function.
		warning("hide.mouse called, disabled for MH1 Apple IIgs");
		return;
	}

	// WORKAROUND: Turns off current movement that's being caused with the mouse.
	// This fixes problems with too many popup boxes appearing in the Amiga
	// Gold Rush's copy protection failure scene (i.e. the hanging scene, logic.192).
	// Previously multiple popup boxes appeared one after another if you tried
	// to walk somewhere else than to the right using the mouse.
	// FIXME: Write a proper implementation using disassembly and
	//        apply it to other games as well if applicable.
	//state->screenObjTable[SCREENOBJECTS_EGO_ENTRY].flags &= ~fAdjEgoXY;
	if (state->mouseEnabled) {
		state->mouseHidden = true;

		vm->_system->showMouse(false);
	}
}

void cmdAllowMenu(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if (vm->getVersion() < 0x3098) {
		// was not available before 3.098
		warning("allow.menu called, although not available for current AGI version");
		return;
	}

	uint16 allowed = parameter[0];

	if (allowed) {
		vm->_menu->accessAllow();
	} else {
		vm->_menu->accessDeny();
	}
}

void cmdFenceMouse(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr1 = parameter[0];
	uint16 varNr2 = parameter[1];
	uint16 varNr3 = parameter[2];
	uint16 varNr4 = parameter[3];

	state->mouseFence.moveTo(varNr1, varNr2);
	state->mouseFence.setWidth(varNr3 - varNr1);
	state->mouseFence.setHeight(varNr4 - varNr1);
}

// HoldKey was added in 2.425
// There was no way to disable this mode until 3.098 though
void cmdHoldKey(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if ((vm->getVersion() < 0x2425) || (vm->getVersion() == 0x2440)) {
		// was not available before 2.425, but also not available in 2.440
		warning("hold.key called, although not available for current AGI version");
		return;
	}

	vm->_keyHoldMode = true;
}

void cmdReleaseKey(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if (vm->getVersion() < 0x3098) {
		// was not available before 3.098
		warning("release.key called, although not available for current AGI version");
		return;
	}

	vm->_keyHoldMode = false;
}

void cmdAdjEgoMoveToXY(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	const AgiOpCodeEntry *opCodeTable = vm->getOpCodesTable();
	int8 x, y;

	// There are apparently two versions of this opcode: one that takes
	// two parameters and one that takes none. The only games that call
	// this opcode use the two parameter version.
	switch (opCodeTable[182].parameterSize) {
	// The 2 parameter version is used in:
	// Amiga/Atari ST Gold Rush!   - Logic 130, 150
	// Amiga/Atari ST Manhunter 1  - Logic 0
	// Amiga Manhunter 2           - Logic 0
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

	// No games call this; all games call the 2 parameter version
	default:
		state->screenObjTable[SCREENOBJECTS_EGO_ENTRY].flags |= fAdjEgoXY;
		break;
	}
}

void cmdParse(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 stringNr = parameter[0];

	vm->setVar(VM_VAR_WORD_NOT_FOUND, 0);
	vm->setFlag(VM_FLAG_ENTERED_CLI, false);
	vm->setFlag(VM_FLAG_SAID_ACCEPTED_INPUT, false);

	vm->_words->parseUsingDictionary(vm->_text->stringPrintf(state->getString(stringNr)));
}

void cmdCall(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 logicNr = parameter[0];

	// CM: we don't save sIP because set.scan.start can be
	//     used in a called script (fixes xmas demo)
	int oldCIP = state->_curLogic->cIP;
	int oldLognum = state->curLogicNr;

	vm->runLogic(logicNr);

	state->curLogicNr = oldLognum;
	state->_curLogic = &state->logics[state->curLogicNr];
	state->_curLogic->cIP = oldCIP;
}

void cmdCallF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	byte   logicNr = vm->getVar(varNr);

	cmdCall(state, vm, &logicNr);
}

void cmdDrawPicV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 resourceNr = vm->getVar(varNr);

	debugC(6, kDebugLevelScripts, "=== draw pic V1 %d ===", resourceNr);
	vm->_picture->decodePicture(resourceNr, true);

	// TODO: check, if this was really done
	vm->_text->promptClear();

	// Loading trigger
	vm->artificialDelayTrigger_DrawPicture(resourceNr);
}

void cmdDrawPic(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	SpritesMgr *spritesMgr = vm->_sprites;
	uint16 varNr = parameter[0];
	uint16 resourceNr = vm->getVar(varNr);

	debugC(6, kDebugLevelScripts, "=== draw pic %d ===", resourceNr);

	spritesMgr->eraseSprites();
	vm->_picture->decodePicture(resourceNr, true);

	spritesMgr->buildAllSpriteLists();
	spritesMgr->drawAllSpriteLists();
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
	// Fixes bug #3056: AGI: SQ1 (2.2 DOS ENG) bizarre exploding roger
	if (vm->getGameID() == GID_SQ1 && resourceNr == 20)
		vm->setFlag(103, false);

	// Loading trigger
	vm->artificialDelayTrigger_DrawPicture(resourceNr);
}

void cmdShowPic(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	debugC(6, kDebugLevelScripts, "=== show pic ===");

	vm->setFlag(VM_FLAG_OUTPUT_MODE, false);
	vm->_text->closeWindow();
	vm->_picture->showPicWithTransition();
	state->pictureShown = true;

	debugC(6, kDebugLevelScripts, "--- end of show pic ---");
}

void cmdLoadPic(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	SpritesMgr *spritesMgr = vm->_sprites;
	uint16 varNr = parameter[0];
	uint16 resourceNr = vm->getVar(varNr);

	spritesMgr->eraseSprites();
	vm->loadResource(RESOURCETYPE_PICTURE, resourceNr);
	spritesMgr->buildAllSpriteLists();
	spritesMgr->drawAllSpriteLists();
}

void cmdLoadPicV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 varNr = parameter[0];
	uint16 resourceNr = vm->getVar(varNr);

	vm->loadResource(RESOURCETYPE_PICTURE, resourceNr);
}

void cmdDiscardPic(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	debugC(6, kDebugLevelScripts, "--- discard pic ---");
	// do nothing
}

void cmdOverlayPic(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	SpritesMgr *spritesMgr = vm->_sprites;
	uint16 varNr = parameter[0];
	uint16 resourceNr = vm->getVar(varNr);

	debugC(6, kDebugLevelScripts, "--- overlay pic ---");

	spritesMgr->eraseSprites();
	vm->_picture->decodePicture(resourceNr, false);
	spritesMgr->buildAllSpriteLists();
	spritesMgr->drawAllSpriteLists();
	spritesMgr->showAllSpriteLists();
	state->pictureShown = false;

	// Loading trigger
	vm->artificialDelayTrigger_DrawPicture(resourceNr);
}

void cmdShowPriScreen(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_gfx->debugShowMap(1); // switch to priority map

	state->_vm->waitKey();

	vm->_gfx->debugShowMap(0); // switch back to visual map
}

void cmdAnimateObj(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	if (vm->getVersion() < 0x2000) {
		if (screenObj->flags & fDidntMove)
			return;
	} else {
		if (screenObj->flags & fAnimated)
			return;
	}

	debugC(4, kDebugLevelScripts, "animate vt entry #%d", objectNr);
	screenObj->flags = fAnimated | fUpdate | fCycling;

	if (vm->getVersion() < 0x2000) {
		screenObj->flags |= fDidntMove;
	}

	screenObj->motionType = kMotionNormal;
	screenObj->cycle = kCycleNormal;
	screenObj->direction = 0;
}

void cmdUnanimateAll(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_sprites->eraseSprites();

	for (int i = 0; i < SCREENOBJECTS_MAX; i++)
		state->screenObjTable[i].flags &= ~(fAnimated | fDrawn);
}

void cmdDraw(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	if (screenObj->flags & fDrawn)
		return;

//	if (vt.ySize <= 0 || vt.xSize <= 0)
//		return;

	debugC(4, kDebugLevelScripts, "draw entry %d", screenObj->objectNr);

	screenObj->flags |= fUpdate;
	if (vm->getVersion() >= 0x3000) {
		vm->setLoop(screenObj, screenObj->currentLoopNr);
		vm->setCel(screenObj, screenObj->currentCelNr);
	}

	SpritesMgr *spritesMgr = vm->_sprites;

	vm->fixPosition(objectNr);
	screenObj->xPos_prev = screenObj->xPos;
	screenObj->yPos_prev = screenObj->yPos;
	screenObj->xSize_prev = screenObj->xSize;
	screenObj->ySize_prev = screenObj->ySize;
	//screenObj->celData2 = screenObj->celData;
	spritesMgr->eraseRegularSprites();
	screenObj->flags |= fDrawn;
	spritesMgr->buildRegularSpriteList();
	spritesMgr->drawRegularSpriteList();
	spritesMgr->showSprite(screenObj);
	screenObj->flags &= ~fDontUpdate;

	debugC(4, kDebugLevelScripts, "vt entry #%d flags = %02x", objectNr, screenObj->flags);
}

void cmdErase(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	SpritesMgr *spritesMgr = vm->_sprites;
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	bool noUpdateFlag = false;

	if (!(screenObj->flags & fDrawn))
		return;

	spritesMgr->eraseRegularSprites();
	if ((screenObj->flags & fUpdate) == 0) {
		noUpdateFlag = true;
		spritesMgr->eraseStaticSprites();
	}

	screenObj->flags &= ~fDrawn;

	if (noUpdateFlag) {
		spritesMgr->buildStaticSpriteList();
		spritesMgr->drawStaticSpriteList();
	}
	spritesMgr->buildRegularSpriteList();
	spritesMgr->drawRegularSpriteList();
	spritesMgr->showSprite(screenObj);
}

void cmdPosition(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 xPos = parameter[1];
	uint16 yPos = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = screenObj->xPos_prev = xPos;
	screenObj->yPos = screenObj->yPos_prev = yPos;
}

void cmdPositionV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 xPos = parameter[1];
	uint16 yPos = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = xPos;
	screenObj->yPos = yPos;
}

void cmdPositionF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = screenObj->xPos_prev = vm->getVar(varNr1);
	screenObj->yPos = screenObj->yPos_prev = vm->getVar(varNr2);

	// WORKAROUND: KQ3 Rosella Staircase bug; fixed by Sierra in Amiga version 2.15.
	// When changing rooms while Rosella follows, if ego is facing left or right
	// but not diagonal, then Rosella is placed at the same position as ego.
	// This can trap the player on the stairs. Sierra fixed this by placing Rosella
	// one pixel above ego, just like when facing down or diagonal-down. Bug #11996
	if (vm->getGameID() == GID_KQ3 && state->curLogicNr == 119 && objectNr == 15 &&
		screenObj->yPos == state->screenObjTable[SCREENOBJECTS_EGO_ENTRY].yPos) {
		screenObj->yPos--;
		screenObj->yPos_prev--;
	}
}

void cmdPositionFV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = vm->getVar(varNr1);
	screenObj->yPos = vm->getVar(varNr2);
}

void cmdGetPosn(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	vm->setVar(varNr1, (unsigned char)screenObj->xPos);
	vm->setVar(varNr2, (unsigned char)screenObj->yPos);
}

void cmdReposition(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	int16 dx = (int8) vm->getVar(varNr1);
	int16 dy = (int8) vm->getVar(varNr2);
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

	vm->fixPosition(objectNr);
}

void cmdRepositionV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
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

void cmdRepositionTo(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 xPos = parameter[1];
	uint16 yPos = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = xPos;
	screenObj->yPos = yPos;
	screenObj->flags |= fUpdatePos;
	vm->fixPosition(objectNr);
}

void cmdRepositionToF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 varNr1 = parameter[1];
	uint16 varNr2 = parameter[2];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->xPos = vm->getVar(varNr1);
	screenObj->yPos = vm->getVar(varNr2);
	screenObj->flags |= fUpdatePos;
	vm->fixPosition(objectNr);
}

void cmdAddToPic(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 viewNr = parameter[0];
	uint16 loopNr = parameter[1];
	uint16 celNr = parameter[2];
	uint16 xPos = parameter[3];
	uint16 yPos = parameter[4];
	uint16 priority = parameter[5];
	uint16 border = parameter[6];

	vm->_sprites->addToPic(viewNr, loopNr, celNr, xPos, yPos, priority, border);
}

void cmdAddToPicV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 viewNr = parameter[0];
	uint16 loopNr = parameter[1];
	uint16 celNr = parameter[2];
	uint16 xPos = parameter[3];
	uint16 yPos = parameter[4];
	uint16 priority = parameter[5];

	vm->_sprites->addToPic(viewNr, loopNr, celNr, xPos, yPos, priority, -1);
}

void cmdAddToPicF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 viewNr = vm->getVar(parameter[0]);
	uint16 loopNr = vm->getVar(parameter[1]);
	uint16 celNr = vm->getVar(parameter[2]);
	uint16 xPos = vm->getVar(parameter[3]);
	uint16 yPos = vm->getVar(parameter[4]);
	uint16 priority = vm->getVar(parameter[5]);
	uint16 border = vm->getVar(parameter[6]);

	vm->_sprites->addToPic(viewNr, loopNr, celNr, xPos, yPos, priority, border);
}

void cmdForceUpdate(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	SpritesMgr *spritesMgr = vm->_sprites;

	spritesMgr->eraseSprites();
	spritesMgr->buildAllSpriteLists();
	spritesMgr->drawAllSpriteLists();
	spritesMgr->showAllSpriteLists();
}

void cmdReverseLoop(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopFlag = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "o%d, f%d", objectNr, loopFlag);
	screenObj->cycle = kCycleRevLoop;
	screenObj->flags |= (fDontUpdate | fUpdate | fCycling);
	screenObj->setLoopFlag(loopFlag);
	vm->setFlag(screenObj->loop_flag, false);

	vm->cyclerActivated(screenObj);
}

void cmdReverseLoopV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopFlag = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "o%d, f%d", objectNr, loopFlag);
	screenObj->cycle = kCycleRevLoop;
	vm->setCel(screenObj, 0);
	screenObj->flags |= (fDontUpdate | fUpdate | fCycling);
	screenObj->setLoopFlag(loopFlag);
	//screenObj->parm3 = 0;
}

void cmdEndOfLoop(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopFlag = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "o%d, f%d", objectNr, loopFlag);
	screenObj->cycle = kCycleEndOfLoop;
	screenObj->flags |= (fDontUpdate | fUpdate | fCycling);
	screenObj->setLoopFlag(loopFlag);
	vm->setFlag(screenObj->loop_flag, false);

	vm->cyclerActivated(screenObj);
}

void cmdEndOfLoopV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 loopFlag = parameter[1];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	debugC(4, kDebugLevelScripts, "o%d, f%d", objectNr, loopFlag);
	screenObj->cycle = kCycleEndOfLoop;
	vm->setCel(screenObj, 0);
	screenObj->flags |= (fDontUpdate | fUpdate | fCycling);
	screenObj->setLoopFlag(loopFlag);
	//screenObj->parm3 = 0;
}

void cmdBlock(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
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

void cmdUnblock(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	state->block.active = false;
}

void cmdNormalMotion(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->motionType = kMotionNormal;
}

void cmdStopMotion(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->direction = 0;
	screenObj->motionType = kMotionNormal;
	if (objectNr == 0) {        // ego only
		vm->setVar(VM_VAR_EGO_DIRECTION, 0);
		state->playerControl = false;
	}
}

void cmdStopMotionV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags &= ~fAnimated;
}

void cmdStartMotion(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->motionType = kMotionNormal;
	if (objectNr == 0) {        // ego only
		vm->setVar(VM_VAR_EGO_DIRECTION, 0);
		state->playerControl = true;
	}
}

void cmdStartMotionV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->flags |= fAnimated;
}

void cmdPlayerControl(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	ScreenObjEntry *screenObjEgo = &state->screenObjTable[SCREENOBJECTS_EGO_ENTRY];

	state->playerControl = true;

	if (screenObjEgo->motionType != kMotionEgo)
		screenObjEgo->motionType = kMotionNormal;
}

void cmdProgramControl(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	state->playerControl = false;
}

void cmdFollowEgo(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
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

	vm->setFlagOrVar(screenObj->follow_flag, false);
	if (vm->getVersion() < 0x2000) {
		screenObj->flags |= fUpdate | fAnimated;
	} else {
		screenObj->flags |= fUpdate;
	}

	vm->motionActivated(screenObj);
}

void cmdMoveObj(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 moveX = parameter[1];
	uint16 moveY = parameter[2];
	uint16 stepSize = parameter[3];
	uint16 moveFlag = parameter[4];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->motionType = kMotionMoveObj;
	screenObj->move_x = moveX;
	screenObj->move_y = moveY;
	screenObj->move_stepSize = screenObj->stepSize;
	screenObj->move_flag = moveFlag;

	if (stepSize != 0)
		screenObj->stepSize = stepSize;

	vm->setFlagOrVar(screenObj->move_flag, false);
	if (vm->getVersion() < 0x2000) {
		screenObj->flags |= fUpdate | fAnimated;
	} else {
		screenObj->flags |= fUpdate;
	}

	vm->motionActivated(screenObj);

	if (objectNr == 0)
		state->playerControl = false;

	// AGI 2.272 (ddp, xmas) doesn't call moveObj
	if (vm->getVersion() > 0x2272)
		vm->moveObj(screenObj);
}

void cmdMoveObjF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	uint16 moveX = vm->getVar(parameter[1]);
	uint16 moveY = vm->getVar(parameter[2]);
	uint16 stepSize = vm->getVar(parameter[3]);
	uint16 moveFlag = parameter[4];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	screenObj->motionType = kMotionMoveObj;
	screenObj->move_x = moveX;
	screenObj->move_y = moveY;
	screenObj->move_stepSize = screenObj->stepSize;
	screenObj->move_flag = moveFlag;

	if (stepSize != 0)
		screenObj->stepSize = stepSize;

	vm->setFlag(screenObj->move_flag, false);
	screenObj->flags |= fUpdate;

	vm->motionActivated(screenObj);

	if (objectNr == 0)
		state->playerControl = false;

	// AGI 2.272 (ddp, xmas) doesn't call moveObj
	if (vm->getVersion() > 0x2272)
		vm->moveObj(screenObj);
}

void cmdWander(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr = parameter[0];
	ScreenObjEntry *screenObj = &state->screenObjTable[objectNr];

	if (objectNr == 0)
		state->playerControl = false;

	screenObj->motionType = kMotionWander;
	if (vm->getVersion() < 0x2000) {
		screenObj->flags |= fUpdate | fAnimated;
	} else {
		screenObj->flags |= fUpdate;
	}

	vm->motionActivated(screenObj);
}

void cmdSetGameID(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 textNr = parameter[0];

	if (state->_curLogic->texts && (textNr - 1) <= state->_curLogic->numTexts)
		Common::strlcpy(state->id, state->_curLogic->texts[textNr - 1], 8);
	else
		state->id[0] = 0;

	debug(0, "Game ID: \"%s\"", state->id);
}

void cmdPause(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	// Show pause message box
	PauseToken pt = vm->pauseEngine();

	vm->_systemUI->pauseDialog();
}

void cmdSetMenu(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 textNr = parameter[0];

	debugC(4, kDebugLevelScripts, "text %02x of %02x", textNr, state->_curLogic->numTexts);

	if (state->_curLogic->texts != nullptr && (textNr - 1) <= state->_curLogic->numTexts) {
		const char *menuText = state->_curLogic->texts[textNr - 1];

		vm->_menu->addMenu(menuText);
	}
}

void cmdSetMenuItem(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 textNr = parameter[0] - 1;
	uint16 controllerSlot = parameter[1];

	debugC(4, kDebugLevelScripts, "text %02x of %02x", textNr, state->_curLogic->numTexts);

	if (state->_curLogic->texts != nullptr && textNr <= state->_curLogic->numTexts) {
		const char *menuItemText = state->_curLogic->texts[textNr];

		vm->_menu->addMenuItem(menuItemText, controllerSlot);
	}
}

void cmdVersion(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	char ver2Msg[] =
	    "\n"
	    "                               \n\n"
	    "  ScummVM Sierra AGI v%x.%03x";
	char ver3Msg[] =
	    "\n"
	    "                             \n\n"
	    "ScummVM Sierra AGI v%x.002.%03x";

	Common::String verMsg = TITLE " v%s";

	int ver = vm->getVersion();
	int maj = (ver >> 12) & 0xf;
	int min = ver & 0xfff;

	verMsg += (maj == 2 ? ver2Msg : ver3Msg);
	verMsg = Common::String::format(verMsg.c_str(), gScummVMVersion, maj, min);

	vm->_text->messageBox(verMsg.c_str());
}

void cmdConfigureScreen(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *textMgr = vm->_text;
	uint16 gameRow = parameter[0];
	uint16 promptRow = parameter[1];
	uint16 statusRow = parameter[2];

	textMgr->configureScreen(gameRow);
	textMgr->statusRow_Set(statusRow);
	textMgr->promptRow_Set(promptRow);
}

void cmdTextScreen(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	GfxMgr  *gfxMgr = vm->_gfx;
	TextMgr *textMgr = vm->_text;

	debugC(4, kDebugLevelScripts, "switching to text mode");

	state->gfxMode = false;
	gfxMgr->setPalette(false); // set text-mode palette
	textMgr->charAttrib_Set(textMgr->_textAttrib.foreground, textMgr->_textAttrib.background);
	gfxMgr->clearDisplay(0);
	textMgr->clearLines(0, 24, textMgr->_textAttrib.combinedBackground);
}

void cmdGraphics(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	debugC(4, kDebugLevelScripts, "switching to graphics mode");

	vm->redrawScreen();
}

void cmdSetTextAttribute(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	int16 foreground = parameter[0];
	int16 background = parameter[1];
	vm->_text->charAttrib_Set(foreground, background);
}

void cmdStatus(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *textMgr = vm->_text;

	textMgr->inputEditOn();
	textMgr->charAttrib_Push();
	textMgr->charAttrib_Set(0, 15);

	cmdTextScreen(state, vm, parameter);

	vm->_inventory->show();

	//invent_state = 0;
	textMgr->charAttrib_Pop();
	vm->redrawScreen();
}

void cmdQuit(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 withoutPrompt = parameter[0];

	vm->_sound->stopSound();
	if (withoutPrompt) {
		vm->quitGame();
	} else {
		if (vm->_systemUI->quitDialog()) {
			vm->quitGame();
		}
	}
}

void cmdQuitV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_sound->stopSound();
	vm->quitGame();
}

void cmdRestartGame(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	bool doRestart = false;

	vm->_sound->stopSound();

	if (vm->getFlag(VM_FLAG_AUTO_RESTART)) {
		doRestart = true;
	} else {
		doRestart = vm->_systemUI->restartDialog();
	}

	if (doRestart) {
		vm->_restartGame = true;
		vm->setFlag(VM_FLAG_RESTART_GAME, true);
		vm->_menu->itemEnableAll();
	}
}

void cmdDistance(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 objectNr1 = parameter[0];
	uint16 objectNr2 = parameter[1];
	uint16 destVarNr = parameter[2];
	ScreenObjEntry *screenObj1 = &state->screenObjTable[objectNr1];
	ScreenObjEntry *screenObj2 = &state->screenObjTable[objectNr2];

	int d;
	if (screenObj1->flags & fDrawn && screenObj2->flags & fDrawn) {
		int16 x1 = screenObj1->xPos + screenObj1->xSize / 2;
		int16 y1 = screenObj1->yPos;
		int16 x2 = screenObj2->xPos + screenObj2->xSize / 2;
		int16 y2 = screenObj2->yPos;
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
	if (vm->getGameID() == GID_KQ4 && (vm->getVar(VM_VAR_CURRENT_ROOM) == 16 || vm->getVar(VM_VAR_CURRENT_ROOM) == 18) && destVarNr >= 221 && destVarNr <= 223) {
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
		uint8 zombieStateVarNumList[] = {155, 156, (uint8)((vm->getVar(VM_VAR_CURRENT_ROOM) == 16) ? 162 : 158)};
		uint8 zombieNum         = destVarNr - 221;                         // Zombie's number (In range 0-2)
		uint8 zombieStateVarNum = zombieStateVarNumList[zombieNum]; // Number of the variable containing zombie's state
		uint8 zombieState       = vm->getVar(zombieStateVarNum);   // Zombie's state
		// If zombie is not chasing Rosella then set its distance from Rosella to the maximum
		if (zombieState != ZOMBIE_CHASING_EGO)
			d = 0xff;
	}

	vm->setVar(destVarNr, (unsigned char)d);
}

void cmdAcceptInput(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *textMgr = vm->_text;

	debugC(4, kDebugLevelInput, "input normal");

	textMgr->promptEnable();
	textMgr->promptRedraw();
}

void cmdPreventInput(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *textMgr = vm->_text;

	debugC(4, kDebugLevelInput, "no input");

	textMgr->promptDisable();

	textMgr->inputEditOn();
	textMgr->clearLine(textMgr->promptRow_Get(), 0);
}

void cmdCancelLine(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->_text->promptCancelLine();
}

void cmdEchoLine(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *textMgr = vm->_text;

	if (textMgr->promptIsEnabled()) {
		textMgr->promptEchoLine();
	}
}

void cmdGetString(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *textMgr = vm->_text;
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

	vm->cycleInnerLoopActive(CYCLE_INNERLOOP_GETSTRING);

	textMgr->stringSet("");
	textMgr->stringEdit(stringMaxLen);

	// copy string to destination
	// TODO: not sure if set all the time or only when ENTER is pressed
	vm->_game.setString(stringDestNr, (char *)textMgr->_inputString);

	textMgr->charPos_Pop();

	if (!previousEditState) {
		textMgr->inputEditOff();
	}
}

void cmdGetNum(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *textMgr = vm->_text;
	int16 leadInTextNr = parameter[0] - 1;
	int16 numberDestVarNr = parameter[1];
	const char *leadInTextPtr = nullptr;
	byte  number = 0;

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

	vm->cycleInnerLoopActive(CYCLE_INNERLOOP_GETNUMBER);

	textMgr->stringSet("");
	textMgr->stringEdit(3);

	textMgr->promptRedraw();

	number = atoi((char *)textMgr->_inputString);
	vm->setVar(numberDestVarNr, number);

	debugC(4, kDebugLevelScripts, "[%s] -> %d", state->getString(MAX_STRINGS), number);
}

void cmdSetCursorChar(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	TextMgr *textMgr = vm->_text;
	uint16 textNr = parameter[0] - 1;

	if (state->_curLogic->texts != nullptr && textNr <= state->_curLogic->numTexts) {
		textMgr->inputSetCursorChar(*state->_curLogic->texts[textNr]);
	} else {
		// default
		textMgr->inputSetCursorChar('_');
	}
}

void cmdSetKey(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
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

	state->controllerOccurred[controllerSlot] = false;
}

void cmdSetString(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 stringNr = parameter[0];
	uint16 textNr = parameter[1] - 1;

	state->setString(stringNr, state->_curLogic->texts[textNr]);
}

void cmdDisplay(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	// V1 has 4 args
	int16 textNr = (vm->getVersion() >= 0x2000 ? parameter[2] : parameter[3]);
	int16 textRow = parameter[0];
	int16 textColumn = parameter[1];

	vm->_text->display(textNr, textRow, textColumn);
}

void cmdDisplayF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	int16 textRow = vm->getVar(parameter[0]);
	int16 textColumn = vm->getVar(parameter[1]);
	int16 textNr = vm->getVar(parameter[2]);

	vm->_text->display(textNr, textRow, textColumn);
}

void cmdClearTextRect(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	int16 textUpperRow = parameter[0];
	int16 textUpperColumn = parameter[1];
	int16 textLowerRow = parameter[2];
	int16 textLowerColumn = parameter[3];
	int16 color = vm->_text->calculateTextBackground(parameter[4]);

	vm->_text->clearBlock(textUpperRow, textUpperColumn, textLowerRow, textLowerColumn, color);
}

void cmdToggleMonitor(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	debug(0, "toggle.monitor");
}

void cmdClearLines(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	int16 textRowUpper = parameter[0];
	int16 textRowLower = parameter[1];
	int16 color = vm->_text->calculateTextBackground(parameter[2]);

	// Residence 44 calls clear.lines(24,0,0), see Sarien bug #558423
	// Agent06 incorrectly calls clear.lines(1,150,0), see ScummVM bugs
	// #3679 and #3680
	if (textRowUpper > textRowLower) {
		warning("cmdClearLines: RowUpper higher than RowLower");
		textRowLower = textRowUpper;
	}
	vm->_text->clearLines(textRowUpper, textRowLower, color);
}

void cmdPrint(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	int16 textNr = parameter[0];

	vm->_text->print(textNr);
}

void cmdPrintF(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	int16 textNr = vm->getVar(parameter[0]);

	vm->_text->print(textNr);
}

void cmdPrintAt(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	int16 textNr = parameter[0];
	int16 textRow = parameter[1];
	int16 textColumn = parameter[2];
	int16 textWidth = parameter[3];

	debugC(4, kDebugLevelScripts, "%d %d %d %d", textNr, textRow, textColumn, textWidth);

	vm->_text->printAt(textNr, textRow, textColumn, textWidth);
}

void cmdPrintAtV(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	int16 textNr = vm->getVar(parameter[0]);
	int16 textRow = parameter[1];
	int16 textColumn = parameter[2];
	int16 textWidth = parameter[3];

	debugC(4, kDebugLevelScripts, "%d %d %d %d", textNr, textRow, textColumn, textWidth);

	vm->_text->printAt(textNr, textRow, textColumn, textWidth);
}

// push.script was not available until 2.425, and also not available in 2.440
void cmdPushScript(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if ((vm->getVersion() < 0x2425) || (vm->getVersion() == 0x2440)) {
		// was not available before 2.2425, but also not available in 2.440
		warning("push.script called, although not available for current AGI version");
		return;
	}

	debug(0, "push.script");
}

void cmdSetPriBase(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	if ((vm->getVersion() != 0x2425) && (vm->getVersion() < 0x2936)) {
		// was only available in the 2.425 interpreter and from 2.936 (last AGI2 version) onwards
		warning("set.pri.base called, although not available for current AGI version");
		return;
	}

	uint16 priorityBase = parameter[0];

	debug(0, "Priority base set to %d", priorityBase);

	vm->_gfx->setPriorityTable(priorityBase);
}

void cmdGetMousePosn(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 destVarNr1 = parameter[0];
	uint16 destVarNr2 = parameter[1];
	int16 mouseX = vm->_mouse.pos.x;
	int16 mouseY = vm->_mouse.pos.y;

	vm->_gfx->translateDisplayPosToGameScreen(mouseX, mouseY);

	vm->setVar(destVarNr1, mouseX);
	vm->setVar(destVarNr2, mouseY);
}

void cmdShakeScreen(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 shakeCount = parameter[0];

	// AGIPAL uses shake.screen values between 100 and 109 to set the palette
	// (Checked the original AGIPAL-hack's shake.screen-routine's disassembly).
	if (shakeCount >= 100 && shakeCount < 110) {
		vm->_gfx->setAGIPal(shakeCount);
		return;
	}

	vm->_gfx->shakeScreen(shakeCount);
}

void cmdSetSpeed(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	byte varNr = parameter[0];
	byte speed = vm->getVar(varNr);

	vm->_game.speedLevel = speed;
}

void cmdSetItemView(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	// V1 command
	(void)state;
	(void)parameter;
}

void cmdCallV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	vm->loadResource(RESOURCETYPE_LOGIC, resourceNr);
	// FIXME: The following instruction looks incomplete.
	// Maybe something is meant to be assigned to, or read from,
	// the logic_list entry?
//	state->logic_list[++state->max_logics];
	// For now, just do the increment, to silence a clang warning
	++state->max_logics;
	vm->setVar(13, 1); // ???? maybe create another enum vor VM Vars
}

void cmdNewRoomV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 resourceNr = parameter[0];

	warning("cmdNewRoomV1()");
	vm->loadResource(RESOURCETYPE_LOGIC, resourceNr);
	state->max_logics = 1;
	state->logic_list[1] = resourceNr;
	vm->setVar(13, 1);
}

void cmdNewRoomVV1(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	uint16 resourceNr = vm->getVar(parameter[0]);

	warning("cmdNewRoomVV1()");
	vm->loadResource(RESOURCETYPE_LOGIC, resourceNr);
	state->max_logics = 1;
	state->logic_list[1] = resourceNr;
	vm->setVar(13, 1);
}

// The AGI256 interpreter modified opcode 170 to load 256 color pictures
void cmdAgi256LoadPic(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	// Load the picture. Similar to void cmdLoadPic.
	SpritesMgr *spritesMgr = vm->_sprites;
	uint16 varNr = parameter[0];
	uint16 resourceNr = vm->getVar(varNr);

	spritesMgr->eraseSprites();
	vm->loadResource(RESOURCETYPE_PICTURE, resourceNr);

	// Draw the picture. Similar to void cmdDrawPic.
	vm->_picture->decodePicture(resourceNr, false, true);
	spritesMgr->drawAllSpriteLists();
	state->pictureShown = false;

	// Loading trigger
	vm->artificialDelayTrigger_DrawPicture(resourceNr);
}

// The AGIMOUSE interpreter modified opcode 171 to set variables 27-29 to mouse state
void cmdAgiMouseGetMouseState(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	vm->setVar(VM_VAR_MOUSE_BUTTONSTATE, vm->_mouse.button);
	vm->setVar(VM_VAR_MOUSE_X, vm->_mouse.pos.x / 2);
	vm->setVar(VM_VAR_MOUSE_Y, vm->_mouse.pos.y);
}

void cmdUnknown(AgiGame *state, AgiEngine *vm, uint8 *parameter) {
	byte opcode = *(state->_curLogic->data + state->_curLogic->cIP - 1);
	Common::String parameterString;
	for (int i = 0; i < vm->getOpCodesTable()[opcode].parameterSize; i++) {
		if (i > 0) {
			parameterString += ",";
		}
		parameterString += Common::String::format(" %2X (%d)", parameter[i], parameter[i]);
	}
	warning("Unknown opcode: %2X (%d), parameters:%s", opcode, opcode, parameterString.c_str());
}

/**
 * Execute a logic script
 * @param n  Number of the logic resource to execute
 */
int AgiEngine::runLogic(int16 logicNr) {
	AgiGame *state = &_game;
	//int logic_index = 0;

	state->logic_list[0] = 0;
	state->max_logics = 0;

	debugC(2, kDebugLevelScripts, "=================");
	debugC(2, kDebugLevelScripts, "runLogic(%d)", logicNr);

	ScriptPos sp;
	sp.script = logicNr;
	sp.curIP = 0;
	_game.execStack.push_back(sp);

	// If logic not loaded, load it
	if (~_game.dirLogic[logicNr].flags & RES_LOADED) {
		debugC(4, kDebugLevelScripts, "logic %d not loaded!", logicNr);
		loadResource(RESOURCETYPE_LOGIC, logicNr);
	}

	_game.curLogicNr = logicNr;
	_game._curLogic = &_game.logics[_game.curLogicNr];

	_game._curLogic->cIP = _game._curLogic->sIP;

	while (state->_curLogic->cIP < _game.logics[logicNr].size && !(shouldQuit() || _restartGame)) {
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

		// Just a counter for every instruction, that got executed
		_instructionCounter++;

		_game.execStack.back().curIP = state->_curLogic->cIP;

		char st[101];
		int sz = MIN(_game.execStack.size(), 100u);
		memset(st, '.', sz);
		st[sz] = 0;

		uint8 op = *(state->_curLogic->data + state->_curLogic->cIP++);
		switch (op) {
		case 0xff:  // if (open/close)
			testIfCode(logicNr);
			break;
		case 0xfe:  // goto
			// +2 covers goto size
			state->_curLogic->cIP += 2 + ((int16)READ_LE_UINT16(state->_curLogic->data + state->_curLogic->cIP));
			break;
		case 0x00:  // return
			debugC(2, kDebugLevelScripts, "%sreturn() // Logic %d", st, logicNr);
			debugC(2, kDebugLevelScripts, "=================");

//			if (vm->getVersion() < 0x2000) {
//				if (logic_index < state->max_logics) {
//					n = state->logic_list[++logic_index];
//					state->_curLogic = &state->logics[n];
//					state->lognum = n;
//					ip = 2;
//					warning("running logic %d", n);
//					break;
//				}
//				_v[13]=0;
//			}

			_game.execStack.pop_back();
			return 1;
		default: {
			if (!_opCodes[op].functionPtr) {
				error("Illegal opcode %2X (%d) in logic %d, ip %d", op, op, state->curLogicNr, state->_curLogic->cIP);
			}

			uint8 p[CMD_BSIZE];
			int curParameterSize = _opCodes[op].parameterSize;
			memmove(p, state->_curLogic->data + state->_curLogic->cIP, curParameterSize);
			memset(p + curParameterSize, 0, CMD_BSIZE - curParameterSize);

			debugC(2, kDebugLevelScripts, "%s%s(%d %d %d)", st, _opCodes[op].name, p[0], p[1], p[2]);

			_opCodes[op].functionPtr(&_game, this, p);
			state->_curLogic->cIP += curParameterSize;
		}
		}

//		if ((op == 0x0B || op == 0x3F || op == 0x40) && logic_index < state->max_logics) {
//			n = state->logic_list[++logic_index];
//			state->_curLogic = &state->logics[n];
//			state->lognum = n;
//			state->_curLogic_cIP = 2;
//			warning("running logic %d", n);
//		}

		if (_game.exitAllLogics)
			break;
	}

	_game.execStack.pop_back();

	return 0;       // after executing new.room()
}

void AgiEngine::executeAgiCommand(uint8 op, uint8 *p) {
	debugC(2, kDebugLevelScripts, "%s(%d %d %d)", _opCodes[op].name, p[0], p[1], p[2]);

	_opCodes[op].functionPtr(&_game, this, p);
}

} // End of namespace Agi
