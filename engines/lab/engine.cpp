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
#include "lab/labfun.h"
#include "lab/anim.h"
#include "lab/image.h"
#include "lab/intro.h"
#include "lab/processroom.h"
#include "lab/interface.h"
#include "lab/resource.h"

namespace Lab {

// Global parser data
bool ispal = false;

// LAB: Labyrinth specific code for the special puzzles
#define SPECIALLOCK         100
#define SPECIALBRICK        101
#define SPECIALBRICKNOMOUSE 102

#define MAPNUM               28
#define JOURNALNUM            9
#define WESTPAPERNUM         18
#define NOTESNUM             12
#define WHISKEYNUM           25
#define PITHHELMETNUM         7
#define HELMETNUM             1

#define LAMPNUM              27
#define LAMPON              151

#define BELTNUM               3
#define BELTGLOW             70

#define USEDHELMET          184

#define QUARTERNUM           30


#define MUSEUMMONITOR        71
#define GRAMAPHONEMONITOR    72
#define UNICYCLEMONITOR      73
#define STATUEMONITOR        74
#define TALISMANMONITOR      75
#define LUTEMONITOR          76
#define CLOCKMONITOR         77
#define WINDOWMONITOR        78
#define BELTMONITOR          79
#define LIBRARYMONITOR       80
#define TERMINALMONITOR      81
#define LEVERSMONITOR        82

static char initcolors[] = { '\x00', '\x00', '\x00', '\x30',
							 '\x30', '\x30', '\x10', '\x10',
							 '\x10', '\x14', '\x14', '\x14',
							 '\x20', '\x20', '\x20', '\x24',
							 '\x24', '\x24', '\x2c', '\x2c',
							 '\x2c', '\x08', '\x08', '\x08'};

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
			if ((curInv == LAMPNUM) && _conditions->in(LAMPON))
				// LAB: Labyrinth specific
				drawStaticMessage(kTextLampOn);
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
		delete _numberImages[imgIdx];
		_invImages[imgIdx] = _numberImages[imgIdx] = nullptr;
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
		msg = getMsg();
	} while (msg);
}

/**
 * Checks whether the close up is one of the special case closeups.
 */
bool LabEngine::doCloseUp(CloseDataPtr closePtr) {
	if (closePtr == NULL)
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
	case MUSEUMMONITOR:
	case LIBRARYMONITOR:
	case WINDOWMONITOR:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, monrtmargin, 165);
		break;
	case GRAMAPHONEMONITOR:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, 171, 165);
		break;
	case UNICYCLEMONITOR:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, 100, montopmargin, monrtmargin, 165);
		break;
	case STATUEMONITOR:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, 117, montopmargin, monrtmargin, 165);
		break;
	case TALISMANMONITOR:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, 184, 165);
		break;
	case LUTEMONITOR:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, lutertmargin, 165);
		break;
	case CLOCKMONITOR:
		doMonitor(closePtr->_graphicName, closePtr->_message, false, monltmargin, montopmargin, 206, 165);
		break;
	case TERMINALMONITOR:
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

	if ((curInv == LAMPNUM) && _conditions->in(LAMPON))
		return "P:Mines/120";

	if ((curInv == BELTNUM) && _conditions->in(BELTGLOW))
		return "P:Future/BeltGlow";

	if (curInv == WESTPAPERNUM) {
		_curFileName = _inventory[curInv]._bitmapName;
		_anim->_noPalChange = true;
		_graphics->readPict(_curFileName, false);
		_anim->_noPalChange = false;
		doWestPaper();
	} else if (curInv == NOTESNUM) {
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
		_event->attachGadgetList(NULL);
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
	if (curInv == MAPNUM) {
		// LAB: Labyrinth specific
		drawStaticMessage(kTextUseMap);
		interfaceOff();
		_anim->stopDiff();
		_curFileName = " ";
		_cptr = NULL;
		doMap(_roomNum);
		_graphics->setPalette(initcolors, 8);
		_graphics->drawMessage(NULL);
		_graphics->drawPanel();
	} else if (curInv == JOURNALNUM) {
		// LAB: Labyrinth specific
		drawStaticMessage(kTextUseJournal);
		interfaceOff();
		_anim->stopDiff();
		_curFileName = " ";
		_cptr = NULL;
		doJournal();
		_graphics->drawPanel();
		_graphics->drawMessage(NULL);
	} else if (curInv == LAMPNUM) {
		// LAB: Labyrinth specific
		interfaceOff();

		if (_conditions->in(LAMPON)) {
			drawStaticMessage(kTextTurnLampOff);
			_conditions->exclElement(LAMPON);
		} else {
			drawStaticMessage(kTextTurnLampOn);
			_conditions->inclElement(LAMPON);
		}

		_anim->_doBlack = false;
		_anim->_waitForEffect = true;
		_graphics->readPict("Music:Click", true);
		_anim->_waitForEffect = false;

		_anim->_doBlack = false;
		_nextFileName = getInvName(curInv);
	} else if (curInv == BELTNUM) {
		// LAB: Labyrinth specific
		if (!_conditions->in(BELTGLOW))
			_conditions->inclElement(BELTGLOW);

		_anim->_doBlack = false;
		_nextFileName = getInvName(curInv);
	} else if (curInv == WHISKEYNUM) {
		// LAB: Labyrinth specific
		_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUseWhiskey);
	} else if (curInv == PITHHELMETNUM) {
		// LAB: Labyrinth specific
		_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUsePith);
	} else if (curInv == HELMETNUM) {
		// LAB: Labyrinth specific
		_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUseHelmet);
	} else
		return false;

	return true;
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
	uint16 curInv = MAPNUM;

	bool forceDraw = false;
	bool gotMessage = true;

	_graphics->setPalette(initcolors, 8);

	_cptr    = NULL;
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
			if (doCloseUp(_cptr)) {
				_cptr = NULL;

				mayShowCrumbIndicator();
				_graphics->screenUpdate();
			}

			// Sets the current picture properly on the screen
			if (_mainDisplay)
				_nextFileName = getPictName(&_cptr);

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

				if (_cptr) {
					if ((_cptr->_closeUpType == SPECIALLOCK) && _mainDisplay)
						// LAB: Labyrinth specific code
						showCombination(_curFileName);
					else if (((_cptr->_closeUpType == SPECIALBRICK)  ||
								  (_cptr->_closeUpType == SPECIALBRICKNOMOUSE)) &&
								  _mainDisplay)
						// LAB: Labyrinth specific code
						showTile(_curFileName, (bool)(_cptr->_closeUpType == SPECIALBRICKNOMOUSE));
					else
						_graphics->readPict(_curFileName, false);
				} else
					_graphics->readPict(_curFileName, false);

				drawRoomMessage(curInv, _cptr);
				forceDraw = false;

				mayShowCrumbIndicator();
				_graphics->screenUpdate();

				if (!_followingCrumbs)
					eatMessages();
			}

			if (forceDraw) {
				drawRoomMessage(curInv, _cptr);
				forceDraw = false;
				_graphics->screenUpdate();
			}
		}

		// Make sure we check the music at least after every message
		_music->updateMusic();
		interfaceOn();
		IntuiMessage *curMsg = getMsg();

		if (curMsg == NULL) {
			// Does music load and next animation frame when you've run out of messages
			gotMessage = false;
			_music->checkRoomMusic();
			_music->updateMusic();
			_anim->diffNextFrame();

			if (_followingCrumbs) {
				int result = followCrumbs();

				if (result != 0) {
					uint16 code = 0;
					if (result == VKEY_UPARROW)
						code = 7;
					else if (result == VKEY_LTARROW)
						code = 6;
					else if (result == VKEY_RTARROW)
						code = 8;

					gotMessage = true;
					mayShowCrumbIndicator();
					_graphics->screenUpdate();
					if (!from_crumbs(GADGETUP, code, 0, _event->updateAndGetMousePos(), curInv, curMsg, forceDraw, code, actionMode))
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
			if (!from_crumbs(curMsg->_msgClass, curMsg->_code, curMsg->_qualifier, curPos, curInv, curMsg, forceDraw, curMsg->_gadgetID, actionMode))
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

bool LabEngine::from_crumbs(uint32 tmpClass, uint16 code, uint16 Qualifier, Common::Point tmpPos, uint16 &curInv, IntuiMessage *curMsg, bool &forceDraw, uint16 gadgetId, uint16 &actionMode) {
	uint32 msgClass = tmpClass;
	Common::Point curPos = tmpPos;

	uint16 oldDirection = 0;
	uint16 lastInv = MAPNUM;
	CloseDataPtr oldcptr, tempcptr, hcptr = nullptr;
	ViewData *VPtr;
	bool doit;
	uint16 NewDir;

	_anim->_doBlack = false;

	if ((msgClass == RAWKEY) && (!_graphics->_longWinInFront)) {
		if (code == 13) {
			// The return key
			msgClass = MOUSEBUTTONS;
			Qualifier = IEQUALIFIER_LEFTBUTTON;
			curPos = _event->getMousePos();
		} else if (getPlatform() == Common::kPlatformWindows && (code == 'b' || code == 'B')) {
			// Start bread crumbs
			_breadCrumbs[0]._roomNum = 0;
			_numCrumbs = 0;
			_droppingCrumbs = true;
			mayShowCrumbIndicator();
			_graphics->screenUpdate();
		} else if (code == 'f' || code == 'F' || code == 'r' || code == 'R') {
			// Follow bread crumbs
			if (_droppingCrumbs) {
				if (_numCrumbs > 0) {
					_followingCrumbs = true;
					_followCrumbsFast = (code == 'r' || code == 'R');
					_isCrumbTurning = false;
					_isCrumbWaiting = false;
					getTime(&_crumbSecs, &_crumbMicros);

					if (_alternate) {
						eatMessages();
						_alternate = false;
						_anim->_doBlack = true;
						_graphics->_doNotDrawMessage = false;

						_mainDisplay = true;
						// Sets the correct gadget list
						interfaceOn();
						_graphics->drawPanel();
						drawRoomMessage(curInv, _cptr);
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
		} else if ((code == 315) || (code == 'x') || (code == 'X') || (code == 'q') || (code == 'Q')) {
			// Quit?
			_graphics->_doNotDrawMessage = false;
			_graphics->drawMessage("Do you want to quit? (Y/N)");
			doit = false;
			eatMessages();
			interfaceOff();

			while (1) {
				// Make sure we check the music at least after every message
				_music->updateMusic();
				curMsg = getMsg();

				if (curMsg == NULL) {
					// Does music load and next animation frame when you've run out of messages
					_music->updateMusic();
					_anim->diffNextFrame();
				} else {
					if (curMsg->_msgClass == RAWKEY) {
						if ((curMsg->_code == 'Y') || (curMsg->_code == 'y') || (curMsg->_code == 'Q') || (curMsg->_code == 'q')) {
							doit = true;
							break;
						} else if (curMsg->_code < 128) {
							break;
						}
					} else if (curMsg->_msgClass == MOUSEBUTTONS) {
						break;
					}
				}
			}

			if (doit) {
				_anim->stopDiff();
				return false;
			} else {
				forceDraw = true;
				interfaceOn();
			}
		} else if (code == 9) {
			// TAB key
			msgClass = DELTAMOVE;
		} else if (code == 27) {
			// ESC key
			_cptr = NULL;
		}

		eatMessages();
	}

	if (_graphics->_longWinInFront) {
		if ((msgClass == RAWKEY) || ((msgClass == MOUSEBUTTONS) &&
			  ((IEQUALIFIER_LEFTBUTTON & Qualifier) ||
				(IEQUALIFIER_RBUTTON & Qualifier)))) {
			_graphics->_longWinInFront = false;
			_graphics->_doNotDrawMessage = false;
			_graphics->drawPanel();
			drawRoomMessage(curInv, _cptr);
			_graphics->screenUpdate();
		}
	} else if ((msgClass == GADGETUP) && !_alternate) {
		if (gadgetId <= 5) {
			if ((actionMode == 4) && (gadgetId == 4) && (_cptr != NULL)) {
				doMainView(&_cptr);

				_anim->_doBlack = true;
				hcptr = NULL;
				_cptr = NULL;
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
				drawRoomMessage(curInv, _cptr);

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
			doUse(MAPNUM);

			mayShowCrumbIndicator();
			_graphics->screenUpdate();
		} else if (gadgetId >= 6) {
			// Arrow Gadgets
			_cptr = NULL;
			hcptr = NULL;

			if ((gadgetId == 6) || (gadgetId == 8)) {
				if (gadgetId == 6)
					drawStaticMessage(kTextTurnLeft);
				else
					drawStaticMessage(kTextTurnRight);

				_curFileName = " ";

				oldDirection = _direction;

				NewDir = processArrow(_direction, gadgetId - 6);
				doTurn(_direction, NewDir, &_cptr);
				_anim->_doBlack = true;
				_direction = NewDir;
				forceDraw = true;

				mayShowCrumbIndicator();
				_graphics->screenUpdate();
			} else if (gadgetId == 7) {
				uint16 oldRoomNum = _roomNum;

				if (doGoForward(&_cptr)) {
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
					// Note: These numbers were generated by parsing the
					// "Maps" file, which is why they are hard-coded. Bleh!
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
			drawRoomMessage(curInv, _cptr);

			_graphics->screenUpdate();
		}

		gadgetId--;

		if (gadgetId == 0) {
			interfaceOff();
			_anim->stopDiff();
			_curFileName = " ";

			doit = !saveRestoreGame();
			_cptr = NULL;

			_mainDisplay = true;

			curInv = MAPNUM;
			lastInv = MAPNUM;

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
			drawRoomMessage(curInv, _cptr);

			_graphics->screenUpdate();
		} else if (gadgetId == 4) {
			// Right gadget
			decIncInv(&curInv, false);
			lastInv = curInv;
			_graphics->_doNotDrawMessage = false;
			drawRoomMessage(curInv, _cptr);

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
					getTime(&_crumbSecs, &_crumbMicros);

					eatMessages();
					_alternate = false;
					_anim->_doBlack = true;
					_graphics->_doNotDrawMessage = false;

					_mainDisplay = true;
					// Sets the correct gadget list
					interfaceOn();
					_graphics->drawPanel();
					drawRoomMessage(curInv, _cptr);
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
	} else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier) && _mainDisplay) {
		interfaceOff();
		_mainDisplay = true;

		doit = false;

		if (_cptr) {
			if ((_cptr->_closeUpType == SPECIALLOCK) && _mainDisplay)
				// LAB: Labyrinth specific code
				mouseCombination(curPos);
			else if ((_cptr->_closeUpType == SPECIALBRICK) && _mainDisplay)
				mouseTile(curPos);
			else
				doit = true;
		} else
			doit = true;


		if (doit) {
			hcptr = NULL;
			eatMessages();

			if (actionMode == 0) {
				// Take something.
				if (doActionRule(Common::Point(curPos.x, curPos.y), actionMode, _roomNum, &_cptr))
					_curFileName = _newFileName;
				else if (takeItem(curPos.x, curPos.y, &_cptr))
					drawStaticMessage(kTextTakeItem);
				else if (doActionRule(curPos, TAKEDEF - 1, _roomNum, &_cptr))
					_curFileName = _newFileName;
				else if (doActionRule(curPos, TAKE - 1, 0, &_cptr))
					_curFileName = _newFileName;
				else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
					drawStaticMessage(kTextNothing);
			} else if ((actionMode == 1) || (actionMode == 2) || (actionMode == 3)) {
				// Manipulate an object, Open up a "door" or Close a "door"
				if (doActionRule(curPos, actionMode, _roomNum, &_cptr))
					_curFileName = _newFileName;
				else if (!doActionRule(curPos, actionMode, 0, &_cptr)) {
					if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
						drawStaticMessage(kTextNothing);
				}
			} else if (actionMode == 4) {
				// Look at closeups
				tempcptr = _cptr;
				setCurClose(curPos, &tempcptr);

				if (_cptr == tempcptr) {
					if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
						drawStaticMessage(kTextNothing);
				} else if (tempcptr->_graphicName) {
					if (*(tempcptr->_graphicName)) {
						_anim->_doBlack = true;
						_cptr = tempcptr;
					} else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
						drawStaticMessage(kTextNothing);
				} else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
					drawStaticMessage(kTextNothing);
			} else if ((actionMode == 5)  && _conditions->in(curInv)) {
				// Use an item on something else
				if (doOperateRule(curPos.x, curPos.y, curInv, &_cptr)) {
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
		VPtr = getViewData(_roomNum, _direction);
		oldcptr = VPtr->_closeUps;

		if (hcptr == NULL) {
			tempcptr = _cptr;
			setCurClose(curPos, &tempcptr);

			if ((tempcptr == NULL) || (tempcptr == _cptr)) {
				if (_cptr == NULL)
					hcptr = oldcptr;
				else
					hcptr = _cptr->_subCloseUps;
			} else
				hcptr = tempcptr->_nextCloseUp;
		} else
			hcptr = hcptr->_nextCloseUp;


		if (hcptr == NULL) {
			if (_cptr == NULL)
				hcptr = oldcptr;
			else
				hcptr = _cptr->_subCloseUps;
		}

		if (hcptr)
			_event->setMousePos(Common::Point(_utils->scaleX((hcptr->x1 + hcptr->x2) / 2), _utils->scaleY((hcptr->y1 + hcptr->y2) / 2)));
	} else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) {
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
		drawRoomMessage(curInv, _cptr);

		mayShowCrumbIndicator();
		_graphics->screenUpdate();
	}
	return true;
}

void LabEngine::go() {
	_isHiRes = ((getFeatures() & GF_LOWRES) == 0);

	if (!_graphics->setUpScreens()) {
		_isHiRes = false;
		_graphics->setUpScreens();
	}

	_event->initMouse();
	_msgFont = _resource->getFont("P:AvanteG.12");
	_event->mouseHide();

	Intro *intro = new Intro(this);
	intro->introSequence();
	delete intro;

	_event->mouseShow();
	mainGameLoop();

	if (_quitLab) {
		// Won the game
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
	static int movement[4][4] = {
		{ VKEY_UPARROW, VKEY_RTARROW, VKEY_RTARROW, VKEY_LTARROW },
		{ VKEY_RTARROW, VKEY_UPARROW, VKEY_LTARROW, VKEY_RTARROW },
		{ VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW, VKEY_RTARROW },
		{ VKEY_RTARROW, VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW }
	};

	if (_isCrumbWaiting) {
		uint32 Secs;
		uint32 Micros;

		timeDiff(_crumbSecs, _crumbMicros, &Secs, &Micros);

		if (Secs != 0 || Micros != 0)
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

		addCurTime(theDelay / ONESECOND, theDelay % ONESECOND, &_crumbSecs, &_crumbMicros);
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
