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

#include "common/config-manager.h"

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/inv.h"
#include "agi/text.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/systemui.h"
#include "agi/appleIIgs_timedelay_overwrite.h"

namespace Agi {

/**
 * Set up new room.
 * This function is called when ego enters a new room.
 * @param n room number
 */
void AgiEngine::newRoom(int16 newRoomNr) {
	ScreenObjEntry *screenObj;
	ScreenObjEntry *screenObjEgo = &_game.screenObjTable[SCREENOBJECTS_EGO_ENTRY];
	int i;

	// Loading trigger
	artificialDelayTrigger_NewRoom(newRoomNr);

	debugC(4, kDebugLevelMain, "*** room %d ***", newRoomNr);
	_sound->stopSound();

	i = 0;
	for (screenObj = _game.screenObjTable; screenObj < &_game.screenObjTable[SCREENOBJECTS_MAX]; screenObj++) {
		screenObj->objectNr = i++;
		screenObj->flags &= ~(fAnimated | fDrawn);
		screenObj->flags |= fUpdate;
		screenObj->stepTime = 1;
		screenObj->stepTimeCount = 1;
		screenObj->cycleTime = 1;
		screenObj->cycleTimeCount = 1;
		screenObj->stepSize = 1;
	}
	agiUnloadResources();

	_game.playerControl = true;
	_game.block.active = false;
	_game.horizon = 36;
	setVar(VM_VAR_PREVIOUS_ROOM, getVar(VM_VAR_CURRENT_ROOM));
	setVar(VM_VAR_CURRENT_ROOM, newRoomNr);
	setVar(VM_VAR_BORDER_TOUCH_OBJECT, 0);
	setVar(VM_VAR_BORDER_CODE, 0);
	setVar(VM_VAR_EGO_VIEW_RESOURCE, screenObjEgo->currentViewNr);

	agiLoadResource(RESOURCETYPE_LOGIC, newRoomNr);

	// Reposition ego in the new room
	switch (getVar(VM_VAR_BORDER_TOUCH_EGO)) {
	case 1:
		screenObjEgo->yPos = SCRIPT_HEIGHT - 1;
		break;
	case 2:
		screenObjEgo->xPos = 0;
		break;
	case 3:
		screenObjEgo->yPos = _game.horizon + 1;
		break;
	case 4:
		screenObjEgo->xPos = SCRIPT_WIDTH - screenObjEgo->xSize;
		break;
	default:
		break;
	}

	uint16 agiVersion = getVersion();

	if (agiVersion < 0x2000) {
		warning("STUB: NewRoom(%d)", newRoomNr);

		screenObjEgo->flags &= ~fDidntMove;
		// animateObject(0);
		agiLoadResource(RESOURCETYPE_VIEW, screenObjEgo->currentViewNr);
		setView(screenObjEgo, screenObjEgo->currentViewNr);

	} else {
		if (agiVersion >= 0x3000) {
			// this was only done in AGI3
			if (screenObjEgo->motionType == kMotionEgo) {
				screenObjEgo->motionType = kMotionNormal;
				setVar(VM_VAR_EGO_DIRECTION, 0);
			}
		}

		setVar(VM_VAR_BORDER_TOUCH_EGO, 0);
		setFlag(VM_FLAG_NEW_ROOM_EXEC, true);

		_game.exitAllLogics = true;

		_game._vm->_text->statusDraw();
		_game._vm->_text->promptRedraw();
	}
}

void AgiEngine::resetControllers() {
	int i;

	for (i = 0; i < MAX_CONTROLLERS; i++) {
		_game.controllerOccured[i] = false;
	}
}

void AgiEngine::interpretCycle() {
	ScreenObjEntry *screenObjEgo = &_game.screenObjTable[SCREENOBJECTS_EGO_ENTRY];
	bool oldSound;
	byte oldScore;

	if (!_game.playerControl)
		setVar(VM_VAR_EGO_DIRECTION, screenObjEgo->direction);
	else
		screenObjEgo->direction = getVar(VM_VAR_EGO_DIRECTION);

	checkAllMotions();

	oldScore = getVar(VM_VAR_SCORE);
	oldSound = getFlag(VM_FLAG_SOUND_ON);

	// Reset script heuristic here
	resetGetVarSecondsHeuristic();

	_game.exitAllLogics = false;
	while (runLogic(0) == 0 && !(shouldQuit() || _restartGame)) {
		setVar(VM_VAR_WORD_NOT_FOUND, 0);
		setVar(VM_VAR_BORDER_TOUCH_OBJECT, 0);
		setVar(VM_VAR_BORDER_CODE, 0);
		oldScore = getVar(VM_VAR_SCORE);
		setFlag(VM_FLAG_ENTERED_CLI, false);
		_game.exitAllLogics = false;
		_veryFirstInitialCycle = false;
		artificialDelay_CycleDone();
		resetControllers();
	}
	_veryFirstInitialCycle = false;
	artificialDelay_CycleDone();
	resetControllers();

	screenObjEgo->direction = getVar(VM_VAR_EGO_DIRECTION);

	if (getVar(VM_VAR_SCORE) != oldScore || getFlag(VM_FLAG_SOUND_ON) != oldSound)
		_game._vm->_text->statusDraw();

	setVar(VM_VAR_BORDER_TOUCH_OBJECT, 0);
	setVar(VM_VAR_BORDER_CODE, 0);
	setFlag(VM_FLAG_NEW_ROOM_EXEC, false);
	setFlag(VM_FLAG_RESTART_GAME, false);
	setFlag(VM_FLAG_RESTORE_JUST_RAN, false);

	if (_game.gfxMode) {
		updateScreenObjTable();
	}
	_gfx->updateScreen();
	//_gfx->doUpdate();
}

// We return the current key, or 0 if no key was pressed
uint16 AgiEngine::processAGIEvents() {
	uint16 key;
	ScreenObjEntry *screenObjEgo = &_game.screenObjTable[SCREENOBJECTS_EGO_ENTRY];

	wait(10);
	key = doPollKeyboard();

	// In AGI Mouse emulation mode we must update the mouse-related
	// vars in every interpreter cycle.
	//
	// We run AGIMOUSE always as a side effect
	setVar(VM_VAR_MOUSE_X, _mouse.pos.x / 2);
	setVar(VM_VAR_MOUSE_Y, _mouse.pos.y);

	if (!cycleInnerLoopIsActive()) {
		// Click-to-walk mouse interface
		if (_game.playerControl && (screenObjEgo->flags & fAdjEgoXY)) {
			int toX = screenObjEgo->move_x;
			int toY = screenObjEgo->move_y;

			// AGI Mouse games use ego's sprite's bottom left corner for mouse walking target.
			// Amiga games use ego's sprite's bottom center for mouse walking target.
			// Atari ST and Apple II GS seem to use the bottom left
			if (getPlatform() == Common::kPlatformAmiga)
				toX -= (screenObjEgo->xSize / 2); // Center ego's sprite horizontally

			// Adjust ego's sprite's mouse walking target position (These parameters are
			// controlled with the adj.ego.move.to.x.y-command). Note that these values rely
			// on the horizontal centering of the ego's sprite at least on the Amiga platform.
			toX += _game.adjMouseX;
			toY += _game.adjMouseY;

			screenObjEgo->direction = getDirection(screenObjEgo->xPos, screenObjEgo->yPos, toX, toY, screenObjEgo->stepSize);

			if (screenObjEgo->direction == 0)
				inDestination(screenObjEgo);
		}
	}

	handleMouseClicks(key);

	if (!cycleInnerLoopIsActive()) {
		// no inner loop active at the moment, regular processing

		if (key) {
			if (!handleController(key)) {
				if (key) {
					// Only set VAR_KEY, when no controller/direction was detected
					setVar(VM_VAR_KEY, key & 0xFF);
					if (_text->promptIsEnabled()) {
						_text->promptKeyPress(key);
					}
				}
			}
		}

		if (_menu->delayedExecuteActive()) {
			_menu->execute();
		}

	} else {
		// inner loop active
		// call specific workers
		switch (_game.cycleInnerLoopType) {
		case CYCLE_INNERLOOP_GETSTRING: // loop called from TextMgr::stringEdit()
		case CYCLE_INNERLOOP_GETNUMBER:
			if (key) {
				_text->stringKeyPress(key);
			}
			break;

		case CYCLE_INNERLOOP_INVENTORY: // loop called from InventoryMgr::show()
			if (key) {
				_inventory->keyPress(key);
			}
			break;

		case CYCLE_INNERLOOP_MENU_VIA_KEYBOARD:
			if (key) {
				_menu->keyPress(key);
			}
			break;

		case CYCLE_INNERLOOP_MENU_VIA_MOUSE:
			_menu->mouseEvent(key);
			break;

		case CYCLE_INNERLOOP_SYSTEMUI_SELECTSAVEDGAMESLOT:
			if (key) {
				_systemUI->savedGameSlot_KeyPress(key);
			}
			break;

		case CYCLE_INNERLOOP_SYSTEMUI_VERIFICATION:
			_systemUI->askForVerificationKeyPress(key);
			break;

		case CYCLE_INNERLOOP_MESSAGEBOX:
			if (key) {
				_text->messageBox_KeyPress(key);
			}
			break;

		default:
			break;
		}
	}

	// WORKAROUND: For Apple II gs we added a Speed menu; here the user choose some speed setting from the menu
	if (getPlatform() == Common::kPlatformApple2GS && _game.appleIIgsSpeedControllerSlot != 0xffff)
		for (int i = 0; i < 4; i++)
			if (_game.controllerOccured[_game.appleIIgsSpeedControllerSlot + i]) {
				_game.controllerOccured[_game.appleIIgsSpeedControllerSlot + i] = false;
				_game.setAppleIIgsSpeedLevel(i);
			}

	_gfx->updateScreen();

	return key;
}

int AgiEngine::playGame() {
	int ec = errOK;
	const AgiAppleIIgsDelayOverwriteGameEntry *appleIIgsDelayOverwrite = nullptr;
	const AgiAppleIIgsDelayOverwriteRoomEntry *appleIIgsDelayRoomOverwrite = nullptr;

	debugC(2, kDebugLevelMain, "initializing...");
	debugC(2, kDebugLevelMain, "game version = 0x%x", getVersion());

	_sound->stopSound();

	// We need to do this accurately and reset the AGI priorityscreen to 4
	// otherwise at least the fan game Nick's Quest will go into an endless
	// loop, because the game draws views before it draws the first background picture.
	// For further study see bug #3451122
	_gfx->clear(0, 4);

	_game.horizon = 36;
	_game.playerControl = false;

	setFlag(VM_FLAG_LOGIC_ZERO_FIRST_TIME, true); // not in 2.917
	setFlag(VM_FLAG_NEW_ROOM_EXEC, true);         // needed for MUMG and SQ2!
	setFlag(VM_FLAG_SOUND_ON, true);              // enable sound
	// do not set VM_VAR_TIME_DELAY, original AGI did not do it (in the data segment it was simply set to 0)

	_game.gfxMode = true;
	_text->promptRow_Set(22);

	debug(0, "Running AGI script.\n");

	setFlag(VM_FLAG_ENTERED_CLI, false);
	setFlag(VM_FLAG_SAID_ACCEPTED_INPUT, false);
	setVar(VM_VAR_WORD_NOT_FOUND, 0);
	setVar(VM_VAR_KEY, 0);

	debugC(2, kDebugLevelMain, "Entering main loop");
	bool firstLoop = !getFlag(VM_FLAG_RESTART_GAME); // Do not restore on game restart

	if (firstLoop) {
		if (ConfMan.hasKey("save_slot")) {
			// quick restore enabled
			_game.automaticRestoreGame = true;
		}
	}

	artificialDelay_Reset();

	if (getPlatform() == Common::kPlatformApple2GS) {
		// Look up, if there is a time delay overwrite table for the current game
		appleIIgsDelayOverwrite = appleIIgsDelayOverwriteGameTable;
		while (appleIIgsDelayOverwrite->gameId != GID_AGIDEMO) {
			if (appleIIgsDelayOverwrite->gameId == getGameID())
				break; // game found
			appleIIgsDelayOverwrite++;
		}
	}

	do {
		processAGIEvents();

		inGameTimerUpdate();

		uint8 timeDelay = getVar(VM_VAR_TIME_DELAY);

		if (getPlatform() == Common::kPlatformApple2GS) {
			timeDelay++;
			// It seems that either Apple IIgs ran very slowly or that the delay in its interpreter was not working as everywhere else
			// Most games on that platform set the delay to 0, which means no delay in DOS
			// Gold Rush! even "optimizes" itself when larger sprites are on the screen it sets TIME_DELAY to 0.
			// Normally that game runs at TIME_DELAY 1.
			// Maybe a script patch for this game would make sense.
			// TODO: needs further investigation

			int16 timeDelayOverwrite = -99;

			// Now check, if we got a time delay overwrite entry for current room
			if (appleIIgsDelayOverwrite->roomTable) {
				byte curRoom = getVar(VM_VAR_CURRENT_ROOM);
				int16 curPictureNr = _picture->getResourceNr();

				appleIIgsDelayRoomOverwrite = appleIIgsDelayOverwrite->roomTable;
				while (appleIIgsDelayRoomOverwrite->fromRoom >= 0) {
					if ((appleIIgsDelayRoomOverwrite->fromRoom <= curRoom) && (appleIIgsDelayRoomOverwrite->toRoom >= curRoom)) {
						if ((appleIIgsDelayRoomOverwrite->activePictureNr == curPictureNr) || (appleIIgsDelayRoomOverwrite->activePictureNr == -1)) {
							if (appleIIgsDelayRoomOverwrite->onlyWhenPlayerNotInControl) {
								if (_game.playerControl) {
									// Player is actually currently in control? -> then skip this entry
									appleIIgsDelayRoomOverwrite++;
									continue;
								}
							}
							timeDelayOverwrite = appleIIgsDelayRoomOverwrite->timeDelayOverwrite;
							break;
						}
					}
					appleIIgsDelayRoomOverwrite++;
				}
			}

			if (timeDelayOverwrite == -99) {
				// use default time delay in case no room specific one was found ...
				if (_game.appleIIgsSpeedLevel == 2)
					// ... and the user set the speed to "Normal" ...
					timeDelayOverwrite = appleIIgsDelayOverwrite->defaultTimeDelayOverwrite;
				else
					// ... otherwise, use the speed the user requested (either from menu, or from text parser)
					timeDelayOverwrite = _game.appleIIgsSpeedLevel;
			}


			if (timeDelayOverwrite >= 0) {
				if (timeDelayOverwrite != timeDelay) {
					// delayOverwrite is not the same as the delay taken from the scripts? overwrite it
					//warning("AppleIIgs: time delay overwrite from %d to %d", timeDelay, timeDelayOverwrite);

					setVar(VM_VAR_TIME_DELAY, timeDelayOverwrite - 1); // adjust for Apple IIgs
					timeDelay = timeDelayOverwrite;
				}
			}
		}

		// Increment the delay value by one, so that we wait for at least 1 cycle
		// In Original AGI 1 cycle was 50 milliseconds, so 20 frames per second
		// So TIME_DELAY 1 resulted in around 20 frames per second
		//               2 resulted in around 10 frames per second
		//               0 however resulted in no limits at all, so the game ran as fast as possible
		// We obviously do not want the game to run as fast as possible, so we will use 40 frames per second instead.
		timeDelay = timeDelay * 2;
		if (!timeDelay)
			timeDelay = 1;

		// Our cycle counter runs at 25 milliseconds.
		// So time delay has to be 1 for the originally unlimited speed - for our 40 fps
		//                         2 for 20 frames per second
		//                         4 for 10 frames per second
		//                         and so on.

		if (_passedPlayTimeCycles >= timeDelay) {
			// code to check for executed cycles
			// TimeDate time;
			// g_system->getTimeAndDate(time);
			// warning("cycle %d", time.tm_sec);
			inGameTimerResetPassedCycles();

			interpretCycle();

			// Check if the user has asked to load a game from the command line
			// or the launcher
			if (_game.automaticRestoreGame) {
				_game.automaticRestoreGame = false;
				checkQuickLoad();
			}

			setFlag(VM_FLAG_ENTERED_CLI, false);
			setFlag(VM_FLAG_SAID_ACCEPTED_INPUT, false);
			setVar(VM_VAR_WORD_NOT_FOUND, 0);
			setVar(VM_VAR_KEY, 0);
		}

	} while (!(shouldQuit() || _restartGame));

	_sound->stopSound();

	return ec;
}

int AgiEngine::runGame() {
	int ec = errOK;

	// Execute the game
	do {
		debugC(2, kDebugLevelMain, "game loop");
		debugC(2, kDebugLevelMain, "game version = 0x%x", getVersion());

		if (agiInit() != errOK)
			break;

		if (_restartGame) {
			setFlag(VM_FLAG_RESTART_GAME, true);
			// do not set VM_VAR_TIME_DELAY, original AGI did not do it

			// Reset in-game timer
			inGameTimerReset();

			_restartGame = false;
		}

		// Set computer type (v20 i.e. vComputer) and sound type
		switch (getPlatform()) {
		case Common::kPlatformAtariST:
			setVar(VM_VAR_COMPUTER, kAgiComputerAtariST);
			setVar(VM_VAR_SOUNDGENERATOR, kAgiSoundPC);
			break;
		case Common::kPlatformAmiga:
			if (getFeatures() & GF_OLDAMIGAV20)
				setVar(VM_VAR_COMPUTER, kAgiComputerAmigaOld);
			else
				setVar(VM_VAR_COMPUTER, kAgiComputerAmiga);
			setVar(VM_VAR_SOUNDGENERATOR, kAgiSoundTandy);
			break;
		case Common::kPlatformApple2GS:
			setVar(VM_VAR_COMPUTER, kAgiComputerApple2GS);
			if (getFeatures() & GF_2GSOLDSOUND)
				setVar(VM_VAR_SOUNDGENERATOR, kAgiSound2GSOld);
			else
				setVar(VM_VAR_SOUNDGENERATOR, kAgiSoundTandy);
			break;
		case Common::kPlatformDOS:
		default:
			setVar(VM_VAR_COMPUTER, kAgiComputerPC);
			setVar(VM_VAR_SOUNDGENERATOR, kAgiSoundPC);
			break;
		}

		// Set monitor type (v26 i.e. vMonitor)
		switch (_renderMode) {
		case Common::kRenderCGA:
			setVar(VM_VAR_MONITOR, kAgiMonitorCga);
			break;
		case Common::kRenderHercA:
		case Common::kRenderHercG:
			// Set EGA for now. Some games place text differently, when this is set to kAgiMonitorHercules.
			// Text placement was different for Hercules rendering (16x12 instead of 16x16). There also was
			// not enough space left for the prompt at the bottom. This was caused by the Hercules resolution.
			// We don't have this restriction and we also support the regular prompt for Hercules mode.
			// In theory Sierra could have had special Hercules code inside their games.
			// TODO: check this.
			setVar(VM_VAR_MONITOR, kAgiMonitorEga);
			break;
		// Don't know if Amiga AGI games use a different value than kAgiMonitorEga
		// for vMonitor so I just use kAgiMonitorEga for them (As was done before too).
		case Common::kRenderAmiga:
		case Common::kRenderApple2GS:
		case Common::kRenderAtariST:
		case Common::kRenderEGA:
		case Common::kRenderVGA:
		default:
			setVar(VM_VAR_MONITOR, kAgiMonitorEga);
			break;
		}

		setVar(VM_VAR_FREE_PAGES, 180); // Set amount of free memory to realistic value
		setVar(VM_VAR_MAX_INPUT_CHARACTERS, 38);
		_text->promptDisable();

		ec = playGame();
		agiDeinit();
	} while (_restartGame);

	delete _menu;
	_menu = NULL;

	releaseImageStack();

	return ec;
}

} // End of namespace Agi
