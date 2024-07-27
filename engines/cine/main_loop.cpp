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


#include "common/scummsys.h"
#include "common/events.h"
#include "common/system.h"

#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/various.h"
#include "cine/bg_list.h"
#include "cine/sound.h"

#include "backends/audiocd/audiocd.h"

namespace Cine {

struct MouseStatusStruct {
	int left;
	int right;
};

MouseStatusStruct mouseData;

uint16 mouseRight = 0;
uint16 mouseLeft = 0;

uint16 mouseUpdateStatus;
uint16 dummyU16;

static void processEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		mouseLeft = 1;
		break;
	case Common::EVENT_RBUTTONDOWN:
		mouseRight = 1;
		break;
	case Common::EVENT_MBUTTONDOWN:
		mouseLeft = mouseRight = 1;
		break;
	case Common::EVENT_LBUTTONUP:
		mouseLeft = 0;
		break;
	case Common::EVENT_RBUTTONUP:
		mouseRight = 0;
		break;
	case Common::EVENT_MBUTTONUP:
		mouseLeft = mouseRight = 0;
		break;
	case Common::EVENT_MOUSEMOVE:
		break;
	case Common::EVENT_WHEELUP:
		g_cine->_actionList.push_back(Common::CustomEventType(kActionMenuOptionUp));
		break;
	case Common::EVENT_WHEELDOWN:
		g_cine->_actionList.push_back(Common::CustomEventType(kActionMenuOptionDown));
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		g_cine->_actionList.push_back(event.customType);
		switch (event.customType) {
		case kActionMouseLeft:
			if (allowPlayerInput) {
				mouseLeft = 1;
			}
			break;
		case kActionMouseRight:
			if (allowPlayerInput) {
				mouseRight = 1;
			}
			break;
		case kActionExamine:
			if (allowPlayerInput) {
				playerCommand = 0; // EXAMINE
				makeCommandLine();
			}
			break;
		case kActionTake:
			if (allowPlayerInput) {
				playerCommand = 1; // TAKE
				makeCommandLine();
			}
			break;
		case kActionInventory:
			if (allowPlayerInput && !inMenu) {
				playerCommand = 2; // INVENTORY
				makeCommandLine();
			}
			break;
		case kActionUse:
			if (allowPlayerInput && !inMenu) {
				playerCommand = 3; // USE
				makeCommandLine();
			}
			break;
		case kActionActivate:
			if (allowPlayerInput) {
				playerCommand = 4; // ACTIVATE
				makeCommandLine();
			}
			break;
		case kActionSpeak:
			if (allowPlayerInput) {
				playerCommand = 5; // SPEAK
				makeCommandLine();
			}
			break;
		case kActionActionMenu:
			if (allowPlayerInput && !inMenu) {
				makeActionMenu();
				makeCommandLine();
			}
			break;
		case kActionSystemMenu:
			if (!inMenu) {
				g_cine->makeSystemMenu();
			}
			break;
		case kActionCollisionPage:
			renderer->showCollisionPage(true);
			break;
		case kActionGameSpeedDefault:
			g_cine->setDefaultGameSpeed();
			break;
		case kActionGameSpeedSlower:
			g_cine->modifyGameSpeed(-1); // Slower
			break;
		case kActionGameSpeedFaster:
			g_cine->modifyGameSpeed(+1); // Faster
			break;
		case kActionMoveLeft:
			moveUsingKeyboard(-1, 0); // Left
			break;
		case kActionMoveRight:
			moveUsingKeyboard(+1, 0); // Right
			break;
		case kActionMoveUp:
			moveUsingKeyboard(0, +1); // Up
			break;
		case kActionMoveDown:
			moveUsingKeyboard(0, -1); // Down
			break;
		case kActionMoveUpRight:
			moveUsingKeyboard(+1, +1); // Up & Right
			break;
		case kActionMoveUpLeft:
			moveUsingKeyboard(-1, +1); // Up & Left
			break;
		case kActionMoveDownLeft:
			moveUsingKeyboard(-1, -1); // Down & Left
			break;
		case kActionMoveDownRight:
			moveUsingKeyboard(+1, -1); // Down & Right
			break;
		default:
			break;
		};
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		switch (event.customType) {
		case kActionMouseLeft:
			if (allowPlayerInput) {
				mouseLeft = 0;
			}
			break;
		case kActionMouseRight:
			if (allowPlayerInput) {
				mouseRight = 0;
			}
			break;
		case kActionMoveUp:
		case kActionMoveDown:
		case kActionMoveLeft:
		case kActionMoveRight:
		case kActionMoveUpLeft:
		case kActionMoveUpRight:
		case kActionMoveDownLeft:
		case kActionMoveDownRight:
			// Stop ego movement made with keyboard when releasing a known key
			moveUsingKeyboard(0, 0);
			break;
		case kActionCollisionPage:
			renderer->showCollisionPage(false);
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_KEYDOWN:
		g_cine->_keyInputList.push_back(event.kbd);
		break;
	default:
		break;
	}
}

void manageEvents(CallSource callSource, EventTarget eventTarget, bool useMaxMouseButtonState, Common::Array<Common::Rect> rects) {
	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Point mousePos;
	uint keysPressed = g_cine->_keyInputList.size();
	bool foundTarget = false;
	int eventsChecked = 0;
	uint16 maxMouseLeft = mouseLeft;
	uint16 maxMouseRight = mouseRight;
	uint32 waitStart = g_system->getMillis();
	uint32 waitEnd = waitStart + g_cine->getTimerDelay();
	uint32 frameEnd = waitStart + 20;
	bool frameEnded = false;
	bool waitEnded = false;
	bool checkWaitEnd = (eventTarget == UNTIL_WAIT_ENDED);
	bool updateScreen = false;
	bool updateAudio = false;

	do {
		Common::Event event = Common::Event();
		int eventsCheckedBeforePolling = eventsChecked;
		while (!foundTarget && !frameEnded && (eventMan->pollEvent(event) || eventsChecked == 0)) {
			processEvent(event);
			eventsChecked++;
			maxMouseLeft = MAX<uint16>(mouseLeft, maxMouseLeft);
			maxMouseRight = MAX<uint16>(mouseRight, maxMouseRight);

			bool mouseButtonDown = (mouseLeft != 0 || mouseRight != 0);
			bool mouseButtonUp = !mouseButtonDown;

			switch (eventTarget) {
			case UNTIL_MOUSE_BUTTON_UP_DOWN_UP:
				// fall through
			case UNTIL_MOUSE_BUTTON_UP_DOWN:
				// fall through
			case UNTIL_MOUSE_BUTTON_UP:
				// fall through
			case UNTIL_MOUSE_BUTTON_UP_AND_WAIT_ENDED:
				foundTarget = mouseButtonUp;
				break;
			case UNTIL_MOUSE_BUTTON_DOWN_UP:
				// fall through
			case UNTIL_MOUSE_BUTTON_DOWN:
				foundTarget = mouseButtonDown;
				break;
			case UNTIL_MOUSE_BUTTON_DOWN_OR_KEY_UP_OR_DOWN_OR_IN_RECTS:
				foundTarget = mouseButtonDown;
				if (!g_cine->_actionList.empty()) {
					Common::CustomEventType customType = g_cine->_actionList.back();
					if (customType == kActionMenuOptionUp || customType == kActionMenuOptionDown) {
						foundTarget = true;
					}
				}
				mousePos = g_system->getEventManager()->getMousePos();
				for (Common::Array<Common::Rect>::iterator it = rects.begin(); it != rects.end(); ++it) {
					if (it->contains(mousePos)) {
						foundTarget = true;
						break;
					}
				}
				break;
			case UNTIL_MOUSE_BUTTON_DOWN_OR_KEY_INPUT:
				foundTarget = mouseButtonDown || keysPressed < g_cine->_keyInputList.size();
				break;
			default:
				break;
			}

			uint32 now = g_system->getMillis();
			frameEnded = (now >= frameEnd);
			waitEnded = (now >= waitEnd);

			if (foundTarget) {
				switch (eventTarget) {
				case UNTIL_MOUSE_BUTTON_UP_DOWN_UP:
					eventTarget = UNTIL_MOUSE_BUTTON_DOWN_UP;
					foundTarget = false;
					break;
				case UNTIL_MOUSE_BUTTON_UP_DOWN:
					eventTarget = UNTIL_MOUSE_BUTTON_DOWN;
					foundTarget = false;
					break;
				case UNTIL_MOUSE_BUTTON_DOWN_UP:
					eventTarget = UNTIL_MOUSE_BUTTON_UP;
					foundTarget = false;
					break;
				case UNTIL_MOUSE_BUTTON_UP_AND_WAIT_ENDED:
					eventTarget = UNTIL_WAIT_ENDED;
					checkWaitEnd = true;
					foundTarget = false;
					break;
				default:
					break;
				}
			}

			foundTarget |= (checkWaitEnd && waitEnded);
		}
		int eventsCheckedAfterPolling = eventsChecked;

		bool eventQueueEmpty = (eventsCheckedBeforePolling == eventsCheckedAfterPolling);

		if (eventQueueEmpty) {
			uint32 now = g_system->getMillis();
			frameEnded = (now >= frameEnd);
			waitEnded = (now >= waitEnd);
		}

		if (eventTarget == UNTIL_WAIT_ENDED) {
			foundTarget = waitEnded;
		}

		if (eventTarget == EMPTY_EVENT_QUEUE) {
			foundTarget = eventQueueEmpty;
		}

		foundTarget |= (checkWaitEnd && waitEnded);
		updateScreen = updateAudio = (foundTarget || frameEnded);

		if (updateScreen) {
			if (callSource != EXECUTE_PLAYER_INPUT) {
				g_system->updateScreen();
			} else {
				// Make the command line (e.g. "EXAMINE DOOR" -> "EXAMINE BUTTON")
				// responsive by updating it here.
				if (allowPlayerInput && playerCommand != -1 && !mouseLeft && !mouseRight) {
					// A player command is given, left and right mouse buttons are up
					mousePos = eventMan->getMousePos();
					playerCommandMouseLeftRightUp(mousePos.x, mousePos.y);
					renderer->drawCommand();
				}

				renderer->blit();
			}
		}

		if (updateAudio) {
			g_system->getAudioCDManager()->update(); // For Future Wars CD version
		}

		if (frameEnded) {
			frameEnd += 20;
		}

		g_system->delayMillis(10);
	} while (!foundTarget && !g_cine->shouldQuit());

	if (useMaxMouseButtonState) {
		mouseData.left = maxMouseLeft;
		mouseData.right = maxMouseRight;
	} else {
		mouseData.left = mouseLeft;
		mouseData.right = mouseRight;
	}
}

void getMouseData(uint16 param, uint16 *pButton, uint16 *pX, uint16 *pY) {
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	*pX = mouse.x;
	*pY = mouse.y;

	*pButton = 0;

	if (mouseData.right) {
		(*pButton) |= 2;
	}

	if (mouseData.left) {
		(*pButton) |= 1;
	}
}

/** Removes elements from seqList that have their member variable var4 set to value -1. */
void purgeSeqList() {
	Common::List<SeqListElement>::iterator it = g_cine->_seqList.begin();
	while (it != g_cine->_seqList.end()) {
		if (it->var4 == -1) {
			// Erase the element and jump to the next element
			it = g_cine->_seqList.erase(it);
		} else {
			// Let the element be and jump to the next element
			++it;
		}
	}
}

void CineEngine::mainLoop(int bootScriptIdx) {
	byte di;

	if (_preLoad == false) {
		resetBgIncrustList();

		setTextWindow(0, 0, 20, 200);

		errorVar = 0;

		addScriptToGlobalScripts(bootScriptIdx);

		menuVar = 0;

//		gfxRedrawPage(page0c, page0, page0c, page0, -1);
//		gfxWaitVBL();
//		gfxRedrawMouseCursor();

		inMenu = false;
		allowPlayerInput = 0;
		checkForPendingDataLoadSwitch = 0;

		reloadBgPalOnNextFlip = 0;
		forbidBgPalReload = 0;
		gfxFadeOutCompleted = 0;
		gfxFadeInRequested = 0;
		safeControlsLastAccessedMs = 0;
		lastSafeControlObjIdx = -1;
		isDrawCommandEnabled = 0;
		waitForPlayerClick = 0;
		menuCommandLen = 0;

		playerCommand = -1;
		g_cine->_commandBuffer = "";

		g_cine->_globalVars[VAR_MOUSE_X_POS] = 0;
		g_cine->_globalVars[VAR_MOUSE_Y_POS] = 0;
		if (g_cine->getGameType() == Cine::GType_OS) {
			g_cine->_globalVars[VAR_MOUSE_X_POS_2ND] = 0;
			g_cine->_globalVars[VAR_MOUSE_Y_POS_2ND] = 0;
			g_cine->_globalVars[VAR_BYPASS_PROTECTION] = 0; // set to 1 to bypass the copy protection
			g_cine->_globalVars[VAR_LOW_MEMORY] = 0; // set to 1 to disable some animations, sounds etc.
		}

		renderer->setBlackPalette(true); // Sets _changePal = true

		newPrcName[0] = '\0';
		newRelName[0] = '\0';
		newObjectName[0] = '\0';
		newMsgName[0] = '\0';
		currentCtName[0] = '\0';
		currentPartName[0] = '\0';

		g_sound->stopMusic();
	}

	do {
		// HACK: Force amount of oxygen left to maximum during Operation Stealth's first arcade sequence.
		//       This makes it possible to pass the arcade sequence for now.
		// FIXME: Remove the hack and make the first arcade sequence normally playable.
		/*
		if (g_cine->getGameType() == Cine::GType_OS) {
			Common::String bgName(renderer->getBgName());
			// Check if the background is one of the three backgrounds
			// that are only used during the first arcade sequence.
			if (bgName == "28.PI1" || bgName == "29.PI1" || bgName == "30.PI1") {
				static const uint oxygenObjNum = 202, maxOxygen = 264;
				// Force the amount of oxygen left to the maximum.
				g_cine->_objectTable[oxygenObjNum].x = maxOxygen;
			}
		}*/

		// HACK: In Operation Stealth after the first arcade sequence jump player's position to avoid getting stuck.
		// After the first arcade sequence the player comes up stairs from
		// the water in Santa Paragua's downtown in front of the flower shop.
		// Previously he was completely stuck after getting up the stairs.
		// If the background is the one used in the flower shop scene ("21.PI1")
		// and the player is at the exact location after getting up the stairs
		// then we just nudge him a tiny bit away from the stairs and voila, he's free!
		// Maybe the real problem behind all this is collision data related as it looks
		// like there's some boundary right there near position (204, 110) which we can
		// jump over by moving the character to (204, 109). The script handling the
		// flower shop scene is AIRPORT.PRC's 13th script.
		// FIXME: Remove the hack and solve what's really causing the problem in the first place.
		if (hacksEnabled && g_cine->getGameType() == Cine::GType_OS) {
			if (scumm_stricmp(renderer->getBgName(), "21.PI1") == 0 && g_cine->_objectTable[1].x == 204 && g_cine->_objectTable[1].y == 110) {
				g_cine->_objectTable[1].y--; // Move the player character upward on-screen by one pixel
			}
		}

		stopMusicAfterFadeOut();
		di = executePlayerInput();

		// Clear the zoneQuery table (Operation Stealth specific)
		if (g_cine->getGameType() == Cine::GType_OS) {
			Common::fill(g_cine->_zoneQuery.begin(), g_cine->_zoneQuery.end(), 0);
		}

		if (g_cine->getGameType() == Cine::GType_OS) {
			processSeqList();
		}
		executeObjectScripts();
		executeGlobalScripts();

		purgeObjectScripts();
		purgeGlobalScripts();
		if (g_cine->getGameType() == Cine::GType_OS) {
			purgeSeqList();
		}

		if (playerCommand == -1) {
			setMouseCursor(MOUSE_CURSOR_NORMAL);
		} else {
			setMouseCursor(MOUSE_CURSOR_CROSS);
		}

		if (gfxFadeInRequested) {
			gfxFadeOutCompleted = 0;
		}

		if (renderer->ready()) {
			renderer->drawFrame(true);
		}

		// NOTE: In the original Future Wars and Operation Stealth messages
		// were removed when running the drawOverlays function which is
		// currently called from the renderer's drawFrame function.
		removeMessages();

		if (waitForPlayerClick) {
			_messageLen <<= 3;
			if (_messageLen < 800)
				_messageLen = 800;

			manageEvents(MAIN_LOOP_WAIT_FOR_PLAYER_CLICK, UNTIL_MOUSE_BUTTON_UP_DOWN_UP);
			waitForPlayerClick = 0;
		}

		if (checkForPendingDataLoadSwitch) {
			checkForPendingDataLoad();

			checkForPendingDataLoadSwitch = 0;
		}

		if (di) {
			if ("quit"[menuCommandLen] == (char)di) {
				++menuCommandLen;
				if (menuCommandLen == 4) {
					quitGame();
				}
			} else {
				menuCommandLen = 0;
			}
		}
	} while (!shouldQuit() && !_restartRequested);

	hideMouse();
	g_sound->stopMusic();
	//if (g_cine->getGameType() == Cine::GType_OS) {
	//	freeUnkList();
	//}
	closePart();
}

} // End of namespace Cine
