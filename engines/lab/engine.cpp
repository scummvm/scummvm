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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/image.h"
#include "lab/intro.h"
#include "lab/labsets.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/tilepuzzle.h"
#include "lab/utils.h"

namespace Lab {

// LAB: Labyrinth specific code for the special puzzles
#define SPECIALLOCK         100
#define SPECIALBRICK        101
#define SPECIALBRICKNOMOUSE 102

enum Items {
	kItemHelmet = 1,
	kItemBelt = 3,
	kItemPithHelmet = 7,
	kItemJournal = 9,
	kItemNotes = 12,
	kItemWestPaper = 18,
	kItemWhiskey = 25,
	kItemLamp = 27,
	kItemMap = 28,
	kItemQuarter = 30
};

#define kCondLampOn         151
#define kCondBeltGlowing     70
#define kCondUsedHelmet     184

enum Monitors {
	kMonitorMuseum = 71,
	kMonitorGramophone = 72,
	kMonitorUnicycle = 73,
	kMonitorStatue = 74,
	kMonitorTalisman = 75,
	kMonitorLute = 76,
	kMonitorClock = 77,
	kMonitorWindow = 78,
	//kMonitorBelt = 79,
	kMonitorLibrary = 80,
	kMonitorTerminal = 81
	//kMonitorLevers = 82
};

static char initcolors[] = { '\x00', '\x00', '\x00', '\x30',
							 '\x30', '\x30', '\x10', '\x10',
							 '\x10', '\x14', '\x14', '\x14',
							 '\x20', '\x20', '\x20', '\x24',
							 '\x24', '\x24', '\x2c', '\x2c',
							 '\x2c', '\x08', '\x08', '\x08' };

uint16 LabEngine::getQuarters() {
	return _inventory[kItemQuarter]._many;
}

void LabEngine::setQuarters(uint16 quarters) {
	_inventory[kItemQuarter]._many = quarters;
}

/**
 * Draws the message for the room.
 */
void LabEngine::drawRoomMessage(uint16 curInv, CloseDataPtr closePtr) {
	if (_lastTooLong) {
		_lastTooLong = false;
		return;
	}

	if (_alternate) {
		if ((curInv <= _numInv) && _conditions->in(curInv) && _inventory[curInv]._bitmapName) {
			if ((curInv == kItemLamp) && _conditions->in(kCondLampOn))
				// LAB: Labyrinth specific
				drawStaticMessage(kTextkLampOn);
			else if (_inventory[curInv]._many > 1) {
				Common::String roomMessage = Common::String(_inventory[curInv]._name) + "  (" + Common::String::format("%d", _inventory[curInv]._many) + ")";
				_graphics->drawMessage(roomMessage.c_str());
			} else
				_graphics->drawMessage(_inventory[curInv]._name);
		}
	} else
		drawDirection(closePtr);

	_lastTooLong = _graphics->_lastMessageLong;
}

void LabEngine::freeScreens() {
	for (uint16 i = 0; i < 20; i++) {
		delete _moveImages[i];
		_moveImages[i] = nullptr;
	}

	for (uint16 imgIdx = 0; imgIdx < 10; imgIdx++) {
		delete _invImages[imgIdx];
		_invImages[imgIdx] = nullptr;
	}
}

/**
 * Permanently flips the imagery of a gadget.
 */
void LabEngine::perFlipGadget(uint16 gadgetId) {
	for (GadgetList::iterator gadget = _moveGadgetList.begin(); gadget != _moveGadgetList.end(); ++gadget) {
		Gadget *topGadget = *gadget;
		if (topGadget->_gadgetID == gadgetId) {
			Image *tmpImage = topGadget->_image;
			topGadget->_image = topGadget->_altImage;
			topGadget->_altImage = tmpImage;

			if (!_alternate) {
				_event->mouseHide();
				topGadget->_image->drawImage(topGadget->x, topGadget->y);
				_event->mouseShow();
			}

			break;
		}
	}
}

/**
 * Eats all the available messages.
 */
void LabEngine::eatMessages() {
	IntuiMessage *msg;

	do {
		msg = _event->getMsg();
	} while (msg);
}

/**
 * Checks whether the close up is one of the special case closeups.
 */
bool LabEngine::doCloseUp(CloseDataPtr closePtr) {
	if (!closePtr)
		return false;

	int monltmargin, monrtmargin, montopmargin, lutertmargin;

	if (getPlatform() != Common::kPlatformWindows) {
		monltmargin = 0;
		monrtmargin = 319;
		montopmargin = 0;
		lutertmargin = 124;
	} else {
		monltmargin = 2;
		monrtmargin = 317;
		montopmargin = 2;
		lutertmargin = 128;
	}

	switch (closePtr->_closeUpType) {
	case kMonitorMuseum:
	case kMonitorLibrary:
	case kMonitorWindow:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, monrtmargin, 165);
		break;
	case kMonitorGramophone:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, 171, 165);
		break;
	case kMonitorUnicycle:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, 100, montopmargin, monrtmargin, 165);
		break;
	case kMonitorStatue:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, 117, montopmargin, monrtmargin, 165);
		break;
	case kMonitorTalisman:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, 184, 165);
		break;
	case kMonitorLute:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, lutertmargin, 165);
		break;
	case kMonitorClock:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, 206, 165);
		break;
	case kMonitorTerminal:
		doMonitor(closePtr->_graphicName, closePtr->_message, true, monltmargin, montopmargin, monrtmargin, 165);
		break;
	default:
		return false;
	}

	_curFileName = " ";
	_graphics->drawPanel();

	return true;
}

/**
 * Gets the current inventory name.
 */
const char *LabEngine::getInvName(uint16 curInv) {
	if (_mainDisplay)
		return _inventory[curInv]._bitmapName;

	if ((curInv == kItemLamp) && _conditions->in(kCondLampOn))
		return "P:Mines/120";

	if ((curInv == kItemBelt) && _conditions->in(kCondBeltGlowing))
		return "P:Future/kCondBeltGlowing";

	if (curInv == kItemWestPaper) {
		_curFileName = _inventory[curInv]._bitmapName;
		_anim->_noPalChange = true;
		_graphics->readPict(_curFileName, false);
		_anim->_noPalChange = false;
		doWestPaper();
	} else if (curInv == kItemNotes) {
		_curFileName = _inventory[curInv]._bitmapName;
		_anim->_noPalChange = true;
		_graphics->readPict(_curFileName, false);
		_anim->_noPalChange = false;
		doNotes();
	}

	return _inventory[curInv]._bitmapName;
}

/**
 * Turns the interface off.
 */
void LabEngine::interfaceOff() {
	if (!_interfaceOff) {
		_event->attachGadgetList(nullptr);
		_event->mouseHide();
		_interfaceOff = true;
	}
}

/**
 * Turns the interface on.
 */
void LabEngine::interfaceOn() {
	if (_interfaceOff) {
		_interfaceOff = false;
		_event->mouseShow();
	}

	if (_graphics->_longWinInFront)
		_event->attachGadgetList(nullptr);
	else if (_alternate)
		_event->attachGadgetList(&_invGadgetList);
	else
		_event->attachGadgetList(&_moveGadgetList);
}

/**
 * If the user hits the "Use" gadget; things that can get used on themselves.
 */
bool LabEngine::doUse(uint16 curInv) {
	switch (curInv) {
	case kItemMap:
		drawStaticMessage(kTextUseMap);
		interfaceOff();
		_anim->stopDiff();
		_curFileName = " ";
		_closeDataPtr = nullptr;
		doMap(_roomNum);
		_graphics->setPalette(initcolors, 8);
		_graphics->drawMessage(nullptr);
		_graphics->drawPanel();
		return true;
	case kItemJournal:
		drawStaticMessage(kTextUseJournal);
		interfaceOff();
		_anim->stopDiff();
		_curFileName = " ";
		_closeDataPtr = nullptr;
		doJournal();
		_graphics->drawPanel();
		_graphics->drawMessage(nullptr);
		return true;
	case kItemLamp:
		interfaceOff();

		if (_conditions->in(kCondLampOn)) {
			drawStaticMessage(kTextTurnLampOff);
			_conditions->exclElement(kCondLampOn);
		}
		else {
			drawStaticMessage(kTextTurnkLampOn);
			_conditions->inclElement(kCondLampOn);
		}

		_anim->_doBlack = false;
		_anim->_waitForEffect = true;
		_graphics->readPict("Music:Click", true);
		_anim->_waitForEffect = false;

		_anim->_doBlack = false;
		_nextFileName = getInvName(curInv);
		return true;
	case kItemBelt:
		if (!_conditions->in(kCondBeltGlowing))
			_conditions->inclElement(kCondBeltGlowing);

		_anim->_doBlack = false;
		_nextFileName = getInvName(curInv);
		return true;
	case kItemWhiskey:
		_conditions->inclElement(kCondUsedHelmet);
		drawStaticMessage(kTextUseWhiskey);
		return true;
	case kItemPithHelmet:
		_conditions->inclElement(kCondUsedHelmet);
		drawStaticMessage(kTextUsePith);
		return true;
	case kItemHelmet:
		_conditions->inclElement(kCondUsedHelmet);
		drawStaticMessage(kTextUseHelmet);
		return true;
	default:
		return false;
	}
}

/**
 * Decrements the current inventory number.
 */
void LabEngine::decIncInv(uint16 *curInv, bool decreaseFl) {
	interfaceOff();

	if (decreaseFl)
		(*curInv)--;
	else
		(*curInv)++;

	while (*curInv && (*curInv <= _numInv)) {
		if (_conditions->in(*curInv) && _inventory[*curInv]._bitmapName) {
			_nextFileName = getInvName(*curInv);
			break;
		}

		if (decreaseFl)
			(*curInv)--;
		else
			(*curInv)++;
	}

	if ((*curInv == 0) || (*curInv > _numInv)) {
		if (decreaseFl)
			*curInv = _numInv;
		else
			*curInv = 1;

		while (*curInv && (*curInv <= _numInv)) {
			if (_conditions->in(*curInv) && _inventory[*curInv]._bitmapName) {
				_nextFileName = getInvName(*curInv);
				break;
			}

			if (decreaseFl)
				(*curInv)--;
			else
				(*curInv)++;
		}
	}
}

/**
 * The main game loop.
 */
void LabEngine::mainGameLoop() {
	uint16 actionMode = 4;
	uint16 curInv = kItemMap;

	bool forceDraw = false;
	bool gotMessage = true;

	_graphics->setPalette(initcolors, 8);

	_closeDataPtr = nullptr;
	_roomNum = 1;
	_direction = NORTH;

	_resource->readRoomData("LAB:Doors");
	if (!(_inventory = _resource->readInventory("LAB:Inventor")))
		return;

	if (!(_conditions = new LargeSet(_highestCondition + 1, this)))
		return;

	if (!(_roomsFound = new LargeSet(_manyRooms + 1, this)))
		return;

	_conditions->readInitialConditions("LAB:Conditio");

	_graphics->_longWinInFront = false;
	_graphics->drawPanel();

	perFlipGadget(actionMode);

	// Set up initial picture.
	while (1) {
		_event->processInput(true);

		if (gotMessage) {
			if (_quitLab || g_engine->shouldQuit()) {
				_anim->stopDiff();
				break;
			}

			_music->resumeBackMusic();

			// Sees what kind of close up we're in and does the appropriate stuff, if any.
			if (doCloseUp(_closeDataPtr)) {
				_closeDataPtr = nullptr;
				mayShowCrumbIndicator();
				_graphics->screenUpdate();
			}

			// Sets the current picture properly on the screen
			if (_mainDisplay)
				_nextFileName = getPictName(&_closeDataPtr);

			if (_noUpdateDiff) {
				// Potentially entered another room
				_roomsFound->inclElement(_roomNum);
				forceDraw |= (strcmp(_nextFileName, _curFileName) != 0);

				_noUpdateDiff = false;
				_curFileName = _nextFileName;
			} else if (strcmp(_nextFileName, _curFileName) != 0) {
				interfaceOff();
				// Potentially entered another room
				_roomsFound->inclElement(_roomNum);
				_curFileName = _nextFileName;

				if (_closeDataPtr) {
					switch (_closeDataPtr->_closeUpType) {
					case SPECIALLOCK:
						if (_mainDisplay)
							_tilePuzzle->showCombination(_curFileName);
						break;
					case SPECIALBRICK:
					case SPECIALBRICKNOMOUSE:
						if (_mainDisplay)
							_tilePuzzle->showTile(_curFileName, (_closeDataPtr->_closeUpType == SPECIALBRICKNOMOUSE));
						break;
					default:
						_graphics->readPict(_curFileName, false);
						break;
					}
				} else
					_graphics->readPict(_curFileName, false);

				drawRoomMessage(curInv, _closeDataPtr);
				forceDraw = false;

				mayShowCrumbIndicator();
				_graphics->screenUpdate();

				if (!_followingCrumbs)
					eatMessages();
			}

			if (forceDraw) {
				drawRoomMessage(curInv, _closeDataPtr);
				forceDraw = false;
				_graphics->screenUpdate();
			}
		}

		// Make sure we check the music at least after every message
		_music->updateMusic();
		interfaceOn();
		IntuiMessage *curMsg = _event->getMsg();

		if (!curMsg) {
			// Does music load and next animation frame when you've run out of messages
			gotMessage = false;
			_music->checkRoomMusic();
			_music->updateMusic();
			_anim->diffNextFrame();

			if (_followingCrumbs) {
				int result = followCrumbs();

				if (result != 0) {
					uint16 code = 0;
					switch (result) {
					case VKEY_UPARROW:
						code = 7;
						break;
					case VKEY_LTARROW:
						code = 6;
						break;
					case VKEY_RTARROW:
						code = 8;
						break;
					default:
						break;
					}

					gotMessage = true;
					mayShowCrumbIndicator();
					_graphics->screenUpdate();
					if (!fromCrumbs(GADGETUP, code, 0, _event->updateAndGetMousePos(), curInv, curMsg, forceDraw, code, actionMode))
						break;
				}
			}

			mayShowCrumbIndicator();
			_graphics->screenUpdate();
		} else {
			gotMessage = true;

			Common::Point curPos;
			curPos.x  = curMsg->_mouseX;
			curPos.y  = curMsg->_mouseY;

			_followingCrumbs = false;
			if (!fromCrumbs(curMsg->_msgClass, curMsg->_code, curMsg->_qualifier, curPos, curInv, curMsg, forceDraw, curMsg->_gadgetID, actionMode))
				break;
		}
	}

	delete _conditions;
	delete _roomsFound;

	if (_rooms) {
		delete[] _rooms;
		_rooms = nullptr;
	}

	if (_inventory) {
		for (int i = 1; i <= _numInv; i++) {
			if (_inventory[i]._name)
				delete _inventory[i]._name;

			if (_inventory[i]._bitmapName)
				delete _inventory[i]._bitmapName;
		}

		delete[] _inventory;
	}
}

void LabEngine::showLab2Teaser() {
	_graphics->blackAllScreen();
	_graphics->readPict("P:End/L2In.1", true);

	for (uint16 i = 0; i < 120; i++) {
		_music->updateMusic();
		waitTOF();
	}

	_graphics->readPict("P:End/L2In.9", true);
	_graphics->readPict("P:End/Lost", true);

	warning("STUB: waitForPress");
	while (!1) { // 1 means ignore SDL_ProcessInput calls
		_music->updateMusic();
		_anim->diffNextFrame();
		waitTOF();
	}
}

bool LabEngine::fromCrumbs(uint32 tmpClass, uint16 code, uint16 qualifier, Common::Point tmpPos,
			uint16 &curInv, IntuiMessage *curMsg, bool &forceDraw, uint16 gadgetId, uint16 &actionMode) {
	uint32 msgClass = tmpClass;
	Common::Point curPos = tmpPos;

	uint16 oldDirection = 0;
	uint16 lastInv = kItemMap;
	CloseDataPtr wrkClosePtr = nullptr;
	bool doit;

	_anim->_doBlack = false;

	if ((msgClass == RAWKEY) && (!_graphics->_longWinInFront)) {
		byte codeLower = tolower(code);

		if (code == 13) {
			// The return key
			msgClass = MOUSEBUTTONS;
			qualifier = IEQUALIFIER_LEFTBUTTON;
			curPos = _event->getMousePos();
		} else if (getPlatform() == Common::kPlatformWindows && codeLower == 'b') {
			// Start bread crumbs
			_breadCrumbs[0]._roomNum = 0;
			_numCrumbs = 0;
			_droppingCrumbs = true;
			mayShowCrumbIndicator();
			_graphics->screenUpdate();
		} else if (codeLower == 'f' || codeLower == 'r') {
			// Follow bread crumbs
			if (_droppingCrumbs) {
				if (_numCrumbs > 0) {
					_followingCrumbs = true;
					_followCrumbsFast = (codeLower == 'r');
					_isCrumbTurning = false;
					_isCrumbWaiting = false;
					uint32 t = g_system->getMillis();
					_crumbSecs = t / 1000;
					_crumbMicros = t % 1000;

					if (_alternate) {
						eatMessages();
						_alternate = false;
						_anim->_doBlack = true;
						_graphics->_doNotDrawMessage = false;

						_mainDisplay = true;
						// Sets the correct gadget list
						interfaceOn();
						_graphics->drawPanel();
						drawRoomMessage(curInv, _closeDataPtr);
						_graphics->screenUpdate();
					}
				} else {
					_breadCrumbs[0]._roomNum = 0;
					_droppingCrumbs = false;

					// Need to hide indicator!!!!
					mayShowCrumbIndicatorOff();
					_graphics->screenUpdate();
				}
			}
		} else if (code == 315 || codeLower == 'x' || codeLower == 'q') {
			// Quit?
			_graphics->_doNotDrawMessage = false;
			_graphics->drawMessage("Do you want to quit? (Y/N)");
			eatMessages();
			interfaceOff();

			while (1) {
				// Make sure we check the music at least after every message
				_music->updateMusic();
				curMsg = _event->getMsg();

				if (!curMsg) {
					// Does music load and next animation frame when you've run out of messages
					_music->updateMusic();
					_anim->diffNextFrame();
				} else {
					if (curMsg->_msgClass == RAWKEY) {
						codeLower = tolower(curMsg->_code);
						if (codeLower == 'y' || codeLower == 'q') {
							_anim->stopDiff();
							return false;
						} else if (curMsg->_code < 128) {
							break;
						}
					} else if (curMsg->_msgClass == MOUSEBUTTONS) {
						break;
					}
				}
			}

			forceDraw = true;
			interfaceOn();
		} else if (code == 9) {
			// TAB key
			msgClass = DELTAMOVE;
		} else if (code == 27) {
			// ESC key
			_closeDataPtr = nullptr;
		}

		eatMessages();
	}

	if (_graphics->_longWinInFront) {
		if ((msgClass == RAWKEY) || ((msgClass == MOUSEBUTTONS) &&
			  ((IEQUALIFIER_LEFTBUTTON & qualifier) ||
				(IEQUALIFIER_RIGHTBUTTON & qualifier)))) {
			_graphics->_longWinInFront = false;
			_graphics->_doNotDrawMessage = false;
			_graphics->drawPanel();
			drawRoomMessage(curInv, _closeDataPtr);
			_graphics->screenUpdate();
		}
	} else if ((msgClass == GADGETUP) && !_alternate) {
		if (gadgetId <= 5) {
			if ((actionMode == 4) && (gadgetId == 4) && _closeDataPtr) {
				doMainView(&_closeDataPtr);

				_anim->_doBlack = true;
				wrkClosePtr = nullptr;
				_closeDataPtr = nullptr;
				mayShowCrumbIndicator();
				_graphics->screenUpdate();
			} else if (gadgetId == 5) {
				eatMessages();

				_alternate = true;
				_anim->_doBlack = true;
				_graphics->_doNotDrawMessage = false;
				// Sets the correct gadget list
				interfaceOn();
				_mainDisplay = false;

				if (lastInv && _conditions->in(lastInv)) {
					curInv = lastInv;
					_nextFileName = getInvName(curInv);
				} else
					decIncInv(&curInv, false);

				_graphics->drawPanel();
				drawRoomMessage(curInv, _closeDataPtr);

				mayShowCrumbIndicator();
				_graphics->screenUpdate();
			} else {
				uint16 oldActionMode = actionMode;
				actionMode = gadgetId;

				if (oldActionMode < 5)
					perFlipGadget(oldActionMode);

				perFlipGadget(actionMode);

				if (gadgetId <= 4)
					drawStaticMessage(kTextTakeWhat + gadgetId);
				_graphics->screenUpdate();
			}
		} else if (gadgetId == 9) {
			doUse(kItemMap);

			mayShowCrumbIndicator();
			_graphics->screenUpdate();
		} else if (gadgetId >= 6) {
			// Arrow Gadgets
			_closeDataPtr = nullptr;
			wrkClosePtr = nullptr;

			if ((gadgetId == 6) || (gadgetId == 8)) {
				if (gadgetId == 6)
					drawStaticMessage(kTextTurnLeft);
				else
					drawStaticMessage(kTextTurnRight);

				_curFileName = " ";

				oldDirection = _direction;

				uint16 newDir = processArrow(_direction, gadgetId - 6);
				doTurn(_direction, newDir, &_closeDataPtr);
				_anim->_doBlack = true;
				_direction = newDir;
				forceDraw = true;

				mayShowCrumbIndicator();
				_graphics->screenUpdate();
			} else if (gadgetId == 7) {
				uint16 oldRoomNum = _roomNum;

				if (doGoForward(&_closeDataPtr)) {
					if (oldRoomNum == _roomNum)
						_anim->_doBlack = true;
				} else {
					_anim->_doBlack = true;
					_direction = processArrow(_direction, gadgetId - 6);

					if (oldRoomNum != _roomNum) {
						drawStaticMessage(kTextGoForward);
						// Potentially entered a new room
						_roomsFound->inclElement(_roomNum);
						_curFileName = " ";
						forceDraw = true;
					} else {
						_anim->_doBlack = true;
						drawStaticMessage(kTextNoPath);
					}
				}

				if (_followingCrumbs) {
					if (_isCrumbTurning) {
						if (_direction == oldDirection) {
							_followingCrumbs = false;
						}
					} else {
						if (_roomNum == oldRoomNum) { // didn't get there?
							_followingCrumbs = false;
						}
					}
				} else if (_droppingCrumbs && oldRoomNum != _roomNum) {
					// If in surreal maze, turn off DroppingCrumbs.
					if (_roomNum >= 245 && _roomNum <= 280) {
						_followingCrumbs = false;
						_droppingCrumbs = false;
						_numCrumbs = 0;
						_breadCrumbs[0]._roomNum = 0;
					} else {
						bool intersect = false;
						for (int idx = 0; idx < _numCrumbs; idx++) {
							if (_breadCrumbs[idx]._roomNum == _roomNum) {
								_numCrumbs = idx + 1;
								_breadCrumbs[_numCrumbs]._roomNum = 0;
								intersect = true;
							}
						}

						if (!intersect) {
							if (_numCrumbs == MAX_CRUMBS) {
								_numCrumbs = MAX_CRUMBS - 1;
								memcpy(&_breadCrumbs[0], &_breadCrumbs[1], _numCrumbs * sizeof _breadCrumbs[0]);
							}

							_breadCrumbs[_numCrumbs]._roomNum = _roomNum;
							_breadCrumbs[_numCrumbs++]._direction = _direction;
						}
					}
				}

				mayShowCrumbIndicator();
				_graphics->screenUpdate();
			}
		}
	} else if ((msgClass == GADGETUP) && _alternate) {
		_anim->_doBlack = true;

		if (gadgetId == 0) {
			eatMessages();
			_alternate = false;
			_anim->_doBlack = true;
			_graphics->_doNotDrawMessage = false;

			_mainDisplay = true;
			// Sets the correct gadget list
			interfaceOn();
			_graphics->drawPanel();
			drawRoomMessage(curInv, _closeDataPtr);

			_graphics->screenUpdate();
		}

		gadgetId--;

		if (gadgetId == 0) {
			interfaceOff();
			_anim->stopDiff();
			_curFileName = " ";

			doit = !saveRestoreGame();
			_closeDataPtr = nullptr;
			_mainDisplay = true;

			curInv = kItemMap;
			lastInv = kItemMap;

			_nextFileName = getInvName(curInv);

			_graphics->drawPanel();

			if (doit) {
				_graphics->drawMessage("Disk operation failed.");
				_graphics->setPalette(initcolors, 8);
				g_system->delayMillis(1000);
			}
			_graphics->screenUpdate();
		} else if (gadgetId == 1) {
			if (!doUse(curInv)) {
				uint16 oldActionMode = actionMode;
				// Use button
				actionMode = 5;

				if (oldActionMode < 5)
					perFlipGadget(oldActionMode);

				drawStaticMessage(kTextUseOnWhat);
				_mainDisplay = true;

				_graphics->screenUpdate();
			}
		} else if (gadgetId == 2) {
			_mainDisplay = !_mainDisplay;

			if ((curInv == 0) || (curInv > _numInv)) {
				curInv = 1;

				while ((curInv <= _numInv) && (!_conditions->in(curInv)))
					curInv++;
			}

			if ((curInv <= _numInv) && _conditions->in(curInv) && _inventory[curInv]._bitmapName)
				_nextFileName = getInvName(curInv);

			_graphics->screenUpdate();
		} else if (gadgetId == 3) {
			// Left gadget
			decIncInv(&curInv, true);
			lastInv = curInv;
			_graphics->_doNotDrawMessage = false;
			drawRoomMessage(curInv, _closeDataPtr);

			_graphics->screenUpdate();
		} else if (gadgetId == 4) {
			// Right gadget
			decIncInv(&curInv, false);
			lastInv = curInv;
			_graphics->_doNotDrawMessage = false;
			drawRoomMessage(curInv, _closeDataPtr);

			_graphics->screenUpdate();
		} else if (gadgetId == 5) {
			// bread crumbs
			_breadCrumbs[0]._roomNum = 0;
			_numCrumbs = 0;
			_droppingCrumbs = true;
			mayShowCrumbIndicator();
			_graphics->screenUpdate();
		} else if (gadgetId == 6) {
			// follow crumbs
			if (_droppingCrumbs) {
				if (_numCrumbs > 0) {
					_followingCrumbs = true;
					_followCrumbsFast = false;
					_isCrumbTurning = false;
					_isCrumbWaiting = false;
					uint32 t = g_system->getMillis();
					_crumbSecs = t / 1000;
					_crumbMicros = t % 1000;

					eatMessages();
					_alternate = false;
					_anim->_doBlack = true;
					_graphics->_doNotDrawMessage = false;

					_mainDisplay = true;
					// Sets the correct gadget list
					interfaceOn();
					_graphics->drawPanel();
					drawRoomMessage(curInv, _closeDataPtr);
					_graphics->screenUpdate();
				} else {
					_breadCrumbs[0]._roomNum = 0;
					_droppingCrumbs = false;

					// Need to hide indicator!!!!
					mayShowCrumbIndicatorOff();
					_graphics->screenUpdate();
				}
			}
		}
	} else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & qualifier) && _mainDisplay) {
		interfaceOff();
		_mainDisplay = true;

		doit = false;

		if (_closeDataPtr) {
			switch (_closeDataPtr->_closeUpType) {
			case SPECIALLOCK:
				if (_mainDisplay)
					_tilePuzzle->mouseCombination(curPos);
				break;
			case SPECIALBRICK:
				if (_mainDisplay)
					_tilePuzzle->mouseTile(curPos);
				break;
			default:
				doit = true;
				break;
			}
		} else
			doit = true;


		if (doit) {
			wrkClosePtr = nullptr;
			eatMessages();

			if (actionMode == 0) {
				// Take something.
				if (doActionRule(Common::Point(curPos.x, curPos.y), actionMode, _roomNum, &_closeDataPtr))
					_curFileName = _newFileName;
				else if (takeItem(curPos.x, curPos.y, &_closeDataPtr))
					drawStaticMessage(kTextTakeItem);
				else if (doActionRule(curPos, TAKEDEF - 1, _roomNum, &_closeDataPtr))
					_curFileName = _newFileName;
				else if (doActionRule(curPos, TAKE - 1, 0, &_closeDataPtr))
					_curFileName = _newFileName;
				else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
					drawStaticMessage(kTextNothing);
			} else if ((actionMode == 1) || (actionMode == 2) || (actionMode == 3)) {
				// Manipulate an object, Open up a "door" or Close a "door"
				if (doActionRule(curPos, actionMode, _roomNum, &_closeDataPtr))
					_curFileName = _newFileName;
				else if (!doActionRule(curPos, actionMode, 0, &_closeDataPtr)) {
					if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
						drawStaticMessage(kTextNothing);
				}
			} else if (actionMode == 4) {
				// Look at closeups
				CloseDataPtr tmpClosePtr = _closeDataPtr;
				setCurrentClose(curPos, &tmpClosePtr, true);

				if (_closeDataPtr == tmpClosePtr) {
					if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
						drawStaticMessage(kTextNothing);
				} else if (tmpClosePtr->_graphicName) {
					if (*(tmpClosePtr->_graphicName)) {
						_anim->_doBlack = true;
						_closeDataPtr = tmpClosePtr;
					} else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
						drawStaticMessage(kTextNothing);
				} else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
					drawStaticMessage(kTextNothing);
			} else if ((actionMode == 5)  && _conditions->in(curInv)) {
				// Use an item on something else
				if (doOperateRule(curPos, curInv, &_closeDataPtr)) {
					_curFileName = _newFileName;

					if (!_conditions->in(curInv))
						decIncInv(&curInv, false);
				} else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
					drawStaticMessage(kTextNothing);
			}
		}

		mayShowCrumbIndicator();
		_graphics->screenUpdate();
	} else if (msgClass == DELTAMOVE) {
		ViewData *vptr = getViewData(_roomNum, _direction);
		CloseDataPtr oldClosePtr = vptr->_closeUps;

		if (!wrkClosePtr) {
			CloseDataPtr tmpClosePtr = _closeDataPtr;
			setCurrentClose(curPos, &tmpClosePtr, true);

			if (!tmpClosePtr || (tmpClosePtr == _closeDataPtr)) {
				if (!_closeDataPtr)
					wrkClosePtr = oldClosePtr;
				else
					wrkClosePtr = _closeDataPtr->_subCloseUps;
			} else
				wrkClosePtr = tmpClosePtr->_nextCloseUp;
		} else
			wrkClosePtr = wrkClosePtr->_nextCloseUp;


		if (!wrkClosePtr) {
			if (!_closeDataPtr)
				wrkClosePtr = oldClosePtr;
			else
				wrkClosePtr = _closeDataPtr->_subCloseUps;
		}

		if (wrkClosePtr)
			_event->setMousePos(Common::Point(_utils->scaleX((wrkClosePtr->_x1 + wrkClosePtr->_x2) / 2), _utils->scaleY((wrkClosePtr->_y1 + wrkClosePtr->_y2) / 2)));
	} else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RIGHTBUTTON & qualifier)) {
		eatMessages();
		_alternate = !_alternate;
		_anim->_doBlack = true;
		_graphics->_doNotDrawMessage = false;
		_mainDisplay = true;
		// Sets the correct gadget list
		interfaceOn();

		if (_alternate) {
			if (lastInv && _conditions->in(lastInv))
				curInv = lastInv;
			else
				decIncInv(&curInv, false);
		}

		_graphics->drawPanel();
		drawRoomMessage(curInv, _closeDataPtr);

		mayShowCrumbIndicator();
		_graphics->screenUpdate();
	}
	return true;
}

void LabEngine::go() {
	_isHiRes = ((getFeatures() & GF_LOWRES) == 0);
	_graphics->setUpScreens();

	_event->initMouse();
	_msgFont = _resource->getFont("P:AvanteG.12");
	_event->mouseHide();

	Intro *intro = new Intro(this);
	intro->introSequence();
	delete intro;

	_event->mouseShow();
	mainGameLoop();

	_graphics->closeFont(_msgFont);
	_graphics->freePict();

	freeScreens();

	_music->freeMusic();
}

/**
 * New code to allow quick(er) return navigation in game.
 */
int LabEngine::followCrumbs() {
	// NORTH, SOUTH, EAST, WEST
	int movement[4][4] = {
		{ VKEY_UPARROW, VKEY_RTARROW, VKEY_RTARROW, VKEY_LTARROW },
		{ VKEY_RTARROW, VKEY_UPARROW, VKEY_LTARROW, VKEY_RTARROW },
		{ VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW, VKEY_RTARROW },
		{ VKEY_RTARROW, VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW }
	};

	if (_isCrumbWaiting) {
		if (g_system->getMillis() <= _crumbSecs * 1000 + _crumbMicros)
			return 0;

		_isCrumbWaiting = false;
	}

	if (!_isCrumbTurning)
		_breadCrumbs[_numCrumbs--]._roomNum = 0;

	// Is the current crumb this room? If not, logic error.
	if (_roomNum != _breadCrumbs[_numCrumbs]._roomNum) {
		_numCrumbs = 0;
		_breadCrumbs[0]._roomNum = 0;
		_droppingCrumbs = false;
		_followingCrumbs = false;
		return 0;
	}

	int exitDir;

	// which direction is last crumb
	if (_breadCrumbs[_numCrumbs]._direction == EAST)
		exitDir = WEST;
	else if (_breadCrumbs[_numCrumbs]._direction == WEST)
		exitDir = EAST;
	else if (_breadCrumbs[_numCrumbs]._direction == NORTH)
		exitDir = SOUTH;
	else
		exitDir = NORTH;

	int moveDir = movement[_direction][exitDir];

	if (_numCrumbs == 0) {
		_isCrumbTurning = false;
		_breadCrumbs[0]._roomNum = 0;
		_droppingCrumbs = false;
		_followingCrumbs = false;
	} else {
		int theDelay = (_followCrumbsFast ? ONESECOND / 4 : ONESECOND);

		_isCrumbTurning = (moveDir != VKEY_UPARROW);
		_isCrumbWaiting = true;

		_crumbSecs   = (theDelay + g_system->getMillis()) / 1000;
		_crumbMicros = (theDelay + g_system->getMillis()) % 1000;
	}

	return moveDir;
}

byte dropCrumbs[] = { 0x00 };
Image dropCrumbsImage(24, 24, dropCrumbs);

void LabEngine::mayShowCrumbIndicator() {
	if (getPlatform() != Common::kPlatformWindows)
		return;

	if (_droppingCrumbs && _mainDisplay) {
		_event->mouseHide();
		dropCrumbsImage.drawMaskImage(612, 4);
		_event->mouseShow();
	}
}

byte dropCrumbsOff[] = { 0x00 };
Image dropCrumbsOffImage(24, 24, dropCrumbsOff);

void LabEngine::mayShowCrumbIndicatorOff() {
	if (getPlatform() != Common::kPlatformWindows)
		return;

	if (_mainDisplay) {
		_event->mouseHide();
		dropCrumbsOffImage.drawMaskImage(612, 4);
		_event->mouseShow();
	}
}

} // End of namespace Lab
