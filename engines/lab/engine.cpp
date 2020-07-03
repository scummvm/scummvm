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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"

#include "gui/message.h"

#include "lab/lab.h"
#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/image.h"
#include "lab/interface.h"
#include "lab/intro.h"
#include "lab/labsets.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/speciallocks.h"
#include "lab/utils.h"

namespace Lab {

enum SpecialLock {
	kLockCombination = 100,
	kLockTiles = 101,
	kLockTileSolution = 102
};

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

enum AltButtons {
	kButtonMainDisplay,
	kButtonSaveLoad,
	kButtonUseItem,
	kButtonLookAtItem,
	kButtonPrevItem,
	kButtonNextItem,
	kButtonBreadCrumbs,
	kButtonFollowCrumbs
};

static char initColors[] = { '\x00', '\x00', '\x00', '\x30',
							 '\x30', '\x30', '\x10', '\x10',
							 '\x10', '\x14', '\x14', '\x14',
							 '\x20', '\x20', '\x20', '\x24',
							 '\x24', '\x24', '\x2c', '\x2c',
							 '\x2c', '\x08', '\x08', '\x08' };

void LabEngine::handleTrialWarning() {
	// Check if this is the Wyrmkeep trial
	Common::File roomFile;
	bool knownVersion = true;
	bool roomFileOpened = roomFile.open("rooms/48");

	if (!roomFileOpened)
		knownVersion = false;
	else if (roomFile.size() != 892)
		knownVersion = false;
	else {
		roomFile.seek(352);
		byte checkByte = roomFile.readByte();
		if (checkByte == 0x00) {
			// Full Windows version
		}
		else if (checkByte == 0x80) {
			// Wyrmkeep trial version
			_extraGameFeatures = GF_WINDOWS_TRIAL;

			GUI::MessageDialog trialMessage(_("This is a trial Windows version of the game. To play the full version, you will need to use the original interpreter and purchase a key from Wyrmkeep"));
			trialMessage.runModal();
		}
		else {
			knownVersion = false;
		}

		roomFile.close();
	}

	if (!knownVersion)
		error("Unknown Windows version found, please report this version to the ScummVM team");
}

uint16 LabEngine::getQuarters() {
	return _inventory[kItemQuarter]._quantity;
}

void LabEngine::setQuarters(uint16 quarters) {
	_inventory[kItemQuarter]._quantity = quarters;
}

void LabEngine::drawRoomMessage(uint16 curInv, const CloseData *closePtr) {
	if (_lastTooLong) {
		_lastTooLong = false;
		return;
	}

	if (_alternate) {
		if ((curInv <= _numInv) && _conditions->in(curInv) && !_inventory[curInv]._bitmapName.empty()) {
			if ((curInv == kItemLamp) && _conditions->in(kCondLampOn))
				// LAB: Labyrinth specific
				drawStaticMessage(kTextkLampOn);
			else if (_inventory[curInv]._quantity > 1) {
				Common::String roomMessage = _inventory[curInv]._name + "  (" + Common::String::format("%d", _inventory[curInv]._quantity) + ")";
				_graphics->drawMessage(roomMessage.c_str(), false);
			} else
				_graphics->drawMessage(_inventory[curInv]._name.c_str(), false);
		}
	} else
		drawDirection(closePtr);

	_lastTooLong = _graphics->_lastMessageLong;
}

void LabEngine::freeScreens() {
	for (int i = 0; i < 20; i++) {
		delete _moveImages[i];
		_moveImages[i] = nullptr;
	}

	for (int imgIdx = 0; imgIdx < 10; imgIdx++) {
		delete _invImages[imgIdx];
		_invImages[imgIdx] = nullptr;
	}

	// We can't use freeButtonList() here, because some buttons are shared
	// between the two lists.
	for (ButtonList::iterator buttonIter = _moveButtonList.begin(); buttonIter != _moveButtonList.end(); ++buttonIter) {
		delete *buttonIter;
	}
	_moveButtonList.clear();

	for (ButtonList::iterator buttonIter = _invButtonList.begin(); buttonIter != _invButtonList.end(); ++buttonIter) {
		delete *buttonIter;
	}
	_invButtonList.clear();
}

void LabEngine::perFlipButton(uint16 buttonId) {
	for (ButtonList::iterator button = _moveButtonList.begin(); button != _moveButtonList.end(); ++button) {
		Button *topButton = *button;
		if (topButton->_buttonId == buttonId) {
			SWAP<Image *>(topButton->_image, topButton->_altImage);

			if (!_alternate)
				topButton->_image->drawImage(topButton->_x, topButton->_y);

			break;
		}
	}
}

void LabEngine::eatMessages() {
	IntuiMessage *msg;

	do {
		msg = _event->getMsg();
	} while (msg && !shouldQuit());
}

void LabEngine::handleMonitorCloseup() {
	if (!_closeDataPtr)
		return;

	Common::Rect textRect(2, 2, 317, 165);
	bool isInteractive = false;

	switch (_closeDataPtr->_closeUpType) {
	case kMonitorMuseum:
	case kMonitorLibrary:
	case kMonitorWindow:
		break;
	case kMonitorGramophone:
		textRect.right = 171;
		break;
	case kMonitorUnicycle:
		textRect.left = 100;
		break;
	case kMonitorStatue:
		textRect.left = 117;
		break;
	case kMonitorTalisman:
		textRect.right = 184;
		break;
	case kMonitorLute:
		textRect.right = 128;
		break;
	case kMonitorClock:
		textRect.right = 206;
		break;
	case kMonitorTerminal:
		isInteractive = true;
		break;
	default:
		return;
	}

	doMonitor(_closeDataPtr->_graphicName, _closeDataPtr->_message, isInteractive, textRect);

	_curFileName = " ";
	_graphics->drawPanel();

	_closeDataPtr = nullptr;
	_interface->mayShowCrumbIndicator();
	_graphics->screenUpdate();
}

Common::String LabEngine::getInvName(uint16 curInv) {
	if (_mainDisplay)
		return _inventory[curInv]._bitmapName;

	if ((curInv == kItemLamp) && _conditions->in(kCondLampOn))
		return "P:Mines/120";

	if ((curInv == kItemBelt) && _conditions->in(kCondBeltGlowing))
		return "P:Future/BeltGlow";

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

void LabEngine::interfaceOff() {
	_interface->attachButtonList(nullptr);
	_event->mouseHide();
}

void LabEngine::interfaceOn() {
	if (_graphics->_longWinInFront)
		_interface->attachButtonList(nullptr);
	else if (_alternate)
		_interface->attachButtonList(&_invButtonList);
	else
		_interface->attachButtonList(&_moveButtonList);

	_event->mouseShow();
}

bool LabEngine::doUse(uint16 curInv) {
	switch (curInv) {
	case kItemMap:
		drawStaticMessage(kTextUseMap);
		interfaceOff();
		_anim->stopDiff();
		_curFileName = " ";
		_closeDataPtr = nullptr;
		doMap();
		_graphics->setPalette(initColors, 8);
		_graphics->drawMessage("", false);
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
		_graphics->drawMessage("", false);
		return true;
	case kItemLamp:
		interfaceOff();

		if (_conditions->in(kCondLampOn)) {
			drawStaticMessage(kTextTurnLampOff);
			_conditions->exclElement(kCondLampOn);
		} else {
			drawStaticMessage(kTextTurnkLampOn);
			_conditions->inclElement(kCondLampOn);
		}

		_anim->_doBlack = false;
		_anim->_waitForEffect = true;
		_graphics->readPict("Music:Click");
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

void LabEngine::decIncInv(uint16 *curInv, bool decreaseFl) {
	int8 step = (decreaseFl) ? -1 : 1;
	uint newInv = *curInv + step;

	// Handle wrapping
	if (newInv < 1)
		newInv = _numInv;
	if (newInv > _numInv)
		newInv = 1;

	interfaceOff();

	while (newInv && (newInv <= _numInv)) {
		if (_conditions->in(newInv) && !_inventory[newInv]._bitmapName.empty()) {
			_nextFileName = getInvName(newInv);
			*curInv = newInv;
			break;
		}

		newInv += step;

		// Handle wrapping
		if (newInv < 1)
			newInv = _numInv;
		if (newInv > _numInv)
			newInv = 1;
	}
}

void LabEngine::mainGameLoop() {
	_graphics->setPalette(initColors, 8);

	_closeDataPtr = nullptr;
	_roomNum = 1;
	_direction = kDirectionNorth;

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

	uint16 actionMode = 4;
	perFlipButton(actionMode);

	// Load saved slot from the launcher, if requested
	if (ConfMan.hasKey("save_slot")) {
		loadGame(ConfMan.getInt("save_slot"));

		// Since the intro hasn't been shown, init the background music here
		_music->resetMusic(false);
	}

	uint16 curInv = kItemMap;
	bool forceDraw = false;
	bool gotMessage = true;
	// Set up initial picture.
	while (1) {
		_event->processInput();
		_system->delayMillis(10);

		if (gotMessage) {
			if (_quitLab || shouldQuit()) {
				_anim->stopDiff();
				break;
			}

			handleMonitorCloseup();

			// Sets the current picture properly on the screen
			if (_mainDisplay)
				_nextFileName = getPictName(true);

			if (_noUpdateDiff) {
				// Potentially entered another room
				_roomsFound->inclElement(_roomNum);
				forceDraw |= (_nextFileName != _curFileName);

				_noUpdateDiff = false;
				_curFileName = _nextFileName;
			} else if (_nextFileName != _curFileName) {
				interfaceOff();
				// Potentially entered another room
				_roomsFound->inclElement(_roomNum);
				_curFileName = _nextFileName;

				if (_closeDataPtr && _mainDisplay) {
					switch (_closeDataPtr->_closeUpType) {
					case kLockCombination:
						_specialLocks->showCombinationLock(_curFileName);
						break;
					case kLockTiles:
					case kLockTileSolution:
						_specialLocks->showTileLock(_curFileName, (_closeDataPtr->_closeUpType == kLockTileSolution));
						break;
					default:
						_graphics->readPict(_curFileName, false);
						break;
					}
				} else
					_graphics->readPict(_curFileName, false);

				drawRoomMessage(curInv, _closeDataPtr);
				forceDraw = false;

				_interface->mayShowCrumbIndicator();
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
		updateEvents();
		interfaceOn();
		IntuiMessage *curMsg = _event->getMsg();
		if (shouldQuit()) {
			_quitLab = true;
			return;
		}

		if (!curMsg) {
			// Does music load and next animation frame when you've run out of messages
			gotMessage = false;
			updateEvents();
			_anim->diffNextFrame();

			if (_followingCrumbs) {
				MainButton code = followCrumbs();

				if (code == kButtonForward || code == kButtonLeft || code == kButtonRight) {
					gotMessage = true;
					_interface->mayShowCrumbIndicator();
					_graphics->screenUpdate();
					if (!processEvent(kMessageButtonUp, code, 0, _event->updateAndGetMousePos(), curInv, curMsg, forceDraw, code, actionMode))
						break;
				}
			}

			_interface->mayShowCrumbIndicator();
			_graphics->screenUpdate();
		} else {
			gotMessage = true;
			_followingCrumbs = false;
			if (!processEvent(curMsg->_msgClass, curMsg->_code, curMsg->_qualifier, curMsg->_mouse, curInv, curMsg, forceDraw, curMsg->_code, actionMode))
				break;
		}
	}
}

void LabEngine::showLab2Teaser() {
	_graphics->blackAllScreen();
	_graphics->readPict("P:End/L2In.1");

	for (int i = 0; i < 120; i++) {
		updateEvents();
		waitTOF();
	}

	_graphics->readPict("P:End/L2In.9");
	_graphics->readPict("P:End/Lost");

	while (!_event->getMsg() && !shouldQuit()) {
		updateEvents();
		_anim->diffNextFrame();
		waitTOF();
	}
}

bool LabEngine::processEvent(MessageClass tmpClass, uint16 code, uint16 qualifier, Common::Point tmpPos,
			uint16 &curInv, IntuiMessage *curMsg, bool &forceDraw, uint16 buttonId, uint16 &actionMode) {

	if (shouldQuit())
		return false;

	MessageClass msgClass = tmpClass;
	Common::Point curPos = tmpPos;
	uint16 oldDirection = 0;
	uint16 lastInv = kItemMap;

	if (code == Common::KEYCODE_RETURN)
		msgClass = kMessageLeftClick;

	bool leftButtonClick = (msgClass == kMessageLeftClick);
	bool rightButtonClick = (msgClass == kMessageRightClick);

	_anim->_doBlack = false;

	if (_graphics->_longWinInFront) {
		if (msgClass == kMessageRawKey || leftButtonClick || rightButtonClick) {
			_graphics->_longWinInFront = false;
			_graphics->drawPanel();
			drawRoomMessage(curInv, _closeDataPtr);
			_graphics->screenUpdate();
		}
	} else if (msgClass == kMessageRawKey) {
		return processKey(curMsg, msgClass, qualifier, curPos, curInv, forceDraw, code);
	} else if (msgClass == kMessageButtonUp) {
		if (!_alternate)
			processMainButton(curInv, lastInv, oldDirection, forceDraw, buttonId, actionMode);
		else
			processAltButton(curInv, lastInv, buttonId, actionMode);
	} else if (leftButtonClick && _mainDisplay) {
		interfaceOff();
		_mainDisplay = true;

		if (_closeDataPtr && _closeDataPtr->_closeUpType == kLockCombination)
			_specialLocks->combinationClick(curPos);
		else if (_closeDataPtr && _closeDataPtr->_closeUpType == kLockTiles)
			_specialLocks->tileClick(curPos);
		else
			performAction(actionMode, curPos, curInv);

		_interface->mayShowCrumbIndicator();
		_graphics->screenUpdate();
	} else if (rightButtonClick) {
		eatMessages();
		_alternate = !_alternate;
		_anim->_doBlack = true;
		_mainDisplay = true;
		// Sets the correct button list
		interfaceOn();

		if (_alternate) {
			if (lastInv && _conditions->in(lastInv))
				curInv = lastInv;
			else
				decIncInv(&curInv, false);
		}

		_graphics->drawPanel();
		drawRoomMessage(curInv, _closeDataPtr);

		_interface->mayShowCrumbIndicator();
		_graphics->screenUpdate();
	}

	return true;
}

bool LabEngine::processKey(IntuiMessage *curMsg, uint32 msgClass, uint16 &qualifier, Common::Point &curPos, uint16 &curInv, bool &forceDraw, uint16 code) {
	if ((getPlatform() == Common::kPlatformWindows) && (code == Common::KEYCODE_b)) {
		// Start bread crumbs
		_breadCrumbs[0]._crumbRoomNum = 0;
		_numCrumbs = 0;
		_droppingCrumbs = true;
		_interface->mayShowCrumbIndicator();
		_graphics->screenUpdate();
	} else if (getPlatform() == Common::kPlatformWindows && (code == Common::KEYCODE_f || code == Common::KEYCODE_r)) {
		// Follow bread crumbs
		if (_droppingCrumbs) {
			if (_numCrumbs > 0) {
				_followingCrumbs = true;
				_followCrumbsFast = (code == Common::KEYCODE_r);
				_isCrumbTurning = false;
				_isCrumbWaiting = false;
				_crumbTimestamp = _system->getMillis();

				if (_alternate) {
					eatMessages();
					_alternate = false;
					_anim->_doBlack = true;

					_mainDisplay = true;
					// Sets the correct button list
					interfaceOn();
					_graphics->drawPanel();
					drawRoomMessage(curInv, _closeDataPtr);
					_graphics->screenUpdate();
				}
			} else {
				_breadCrumbs[0]._crumbRoomNum = 0;
				_droppingCrumbs = false;

				_interface->mayShowCrumbIndicatorOff();
				_graphics->screenUpdate();
			}
		}
	} else if ((code == Common::KEYCODE_x) || (code == Common::KEYCODE_q)) {
		// Quit?
		_graphics->drawMessage("Do you want to quit? (Y/N)", false);
		eatMessages();
		interfaceOff();

		while (1) {
			// Make sure we check the music at least after every message
			updateEvents();
			curMsg = _event->getMsg();

			if (shouldQuit())
				return false;

			if (!curMsg) {
				// Does music load and next animation frame when you've run out of messages
				updateEvents();
				_anim->diffNextFrame();
			} else if (curMsg->_msgClass == kMessageRawKey) {
				if ((curMsg->_code == Common::KEYCODE_y) || (curMsg->_code == Common::KEYCODE_q)) {
					_anim->stopDiff();
					return false;
				} else if (curMsg->_code < 128)
					break;
			} else if ((curMsg->_msgClass == kMessageLeftClick) || (curMsg->_msgClass == kMessageRightClick))
				break;
		}

		forceDraw = true;
		interfaceOn();
	} else if (code == Common::KEYCODE_ESCAPE) {
		_closeDataPtr = nullptr;
	} else if (code == Common::KEYCODE_TAB) {
		const CloseData *tmpClosePtr = _closeDataPtr;

		// get next close-up in list after the one pointed to by curPos
		setCurrentClose(curPos, &tmpClosePtr, true, true);

		if (tmpClosePtr != _closeDataPtr)
			_event->setMousePos(Common::Point(_utils->scaleX((tmpClosePtr->_x1 + tmpClosePtr->_x2) / 2), _utils->scaleY((tmpClosePtr->_y1 + tmpClosePtr->_y2) / 2)));
	}

	eatMessages();

	return true;
}

void LabEngine::processMainButton(uint16 &curInv, uint16 &lastInv, uint16 &oldDirection, bool &forceDraw, uint16 buttonId, uint16 &actionMode) {
	switch (buttonId) {
	case kButtonPickup:
	case kButtonUse:
	case kButtonOpen:
	case kButtonClose:
	case kButtonLook:
		if ((actionMode == 4) && (buttonId == kButtonLook) && _closeDataPtr) {
			doMainView();

			_anim->_doBlack = true;
			_closeDataPtr = nullptr;
			_interface->mayShowCrumbIndicator();
		} else {
			uint16 oldActionMode = actionMode;
			actionMode = buttonId;

			if (oldActionMode < 5)
				perFlipButton(oldActionMode);

			perFlipButton(actionMode);
			drawStaticMessage(kTextTakeWhat + buttonId);
		}
		break;

	case kButtonInventory:
		eatMessages();

		_alternate = true;
		_anim->_doBlack = true;
		// Sets the correct button list
		interfaceOn();
		_mainDisplay = false;

		if (lastInv && _conditions->in(lastInv)) {
			curInv = lastInv;
			_nextFileName = getInvName(curInv);
		} else
			decIncInv(&curInv, false);

		_graphics->drawPanel();
		drawRoomMessage(curInv, _closeDataPtr);

		_interface->mayShowCrumbIndicator();
		break;

	case kButtonLeft:
	case kButtonRight: {
		_closeDataPtr = nullptr;
		if (buttonId == kButtonLeft)
			drawStaticMessage(kTextTurnLeft);
		else
			drawStaticMessage(kTextTurnRight);

		_curFileName = " ";
		oldDirection = _direction;

		uint16 newDir = processArrow(_direction, buttonId - 6);
		doTurn(_direction, newDir);
		_anim->_doBlack = true;
		_direction = newDir;
		forceDraw = true;
		_interface->mayShowCrumbIndicator();
		}
		break;

	case kButtonForward: {
		_closeDataPtr = nullptr;
		int oldRoomNum = _roomNum;

		if (doGoForward()) {
			if (oldRoomNum == _roomNum)
				_anim->_doBlack = true;
		} else {
			_anim->_doBlack = true;
			_direction = processArrow(_direction, buttonId - 6);

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
				if (_direction == oldDirection)
					_followingCrumbs = false;
			} else if (_roomNum == oldRoomNum) { // didn't get there?
				_followingCrumbs = false;
			}
		} else if (_droppingCrumbs && (oldRoomNum != _roomNum)) {
			// If in surreal maze, turn off DroppingCrumbs.
			if ((_roomNum >= 245) && (_roomNum <= 280)) {
				_followingCrumbs = false;
				_droppingCrumbs = false;
				_numCrumbs = 0;
				_breadCrumbs[0]._crumbRoomNum = 0;
			} else {
				bool intersect = false;
				for (int idx = 0; idx < _numCrumbs; idx++) {
					if (_breadCrumbs[idx]._crumbRoomNum == _roomNum) {
						_numCrumbs = idx + 1;
						_breadCrumbs[_numCrumbs]._crumbRoomNum = 0;
						intersect = true;
					}
				}

				if (!intersect) {
					if (_numCrumbs == MAX_CRUMBS) {
						_numCrumbs = MAX_CRUMBS - 1;
						memmove(&_breadCrumbs[0], &_breadCrumbs[1], _numCrumbs * sizeof _breadCrumbs[0]);
					}

					_breadCrumbs[_numCrumbs]._crumbRoomNum = _roomNum;
					_breadCrumbs[_numCrumbs++]._crumbDirection = _direction;
				}
			}
		}

		_interface->mayShowCrumbIndicator();
		}
		break;

	case kButtonMap:
		doUse(kItemMap);

		_interface->mayShowCrumbIndicator();
		break;

	default:
		break;
	}

	_graphics->screenUpdate();
}

void LabEngine::processAltButton(uint16 &curInv, uint16 &lastInv, uint16 buttonId, uint16 &actionMode) {
	_anim->_doBlack = true;

	switch (buttonId) {
	case kButtonMainDisplay:
		eatMessages();
		_alternate = false;
		_anim->_doBlack = true;

		_mainDisplay = true;
		// Sets the correct button list
		interfaceOn();
		_graphics->drawPanel();
		drawRoomMessage(curInv, _closeDataPtr);
		break;

	case kButtonSaveLoad: {
		interfaceOff();
		_anim->stopDiff();
		_curFileName = " ";

		bool saveRestoreSuccessful = saveRestoreGame();
		_closeDataPtr = nullptr;
		_mainDisplay = true;

		curInv = lastInv = kItemMap;
		_nextFileName = getInvName(curInv);

		_graphics->drawPanel();

		if (!saveRestoreSuccessful) {
			_graphics->drawMessage("Save/restore aborted", false);
			_graphics->setPalette(initColors, 8);
			_system->delayMillis(1000);
		}
		}
		break;

	case kButtonUseItem:
		if (!doUse(curInv)) {
			uint16 oldActionMode = actionMode;
			// Use button
			actionMode = 5;

			if (oldActionMode < 5)
				perFlipButton(oldActionMode);

			drawStaticMessage(kTextUseOnWhat);
			_mainDisplay = true;
		}
		break;

	case kButtonLookAtItem:
		_mainDisplay = !_mainDisplay;

		if ((curInv == 0) || (curInv > _numInv)) {
			curInv = 1;

			while ((curInv <= _numInv) && !_conditions->in(curInv))
				curInv++;
		}

		if ((curInv <= _numInv) && _conditions->in(curInv) && !_inventory[curInv]._bitmapName.empty())
			_nextFileName = getInvName(curInv);

		break;

	case kButtonPrevItem:
		decIncInv(&curInv, true);
		lastInv = curInv;
		drawRoomMessage(curInv, _closeDataPtr);
		break;

	case kButtonNextItem:
		decIncInv(&curInv, false);
		lastInv = curInv;
		drawRoomMessage(curInv, _closeDataPtr);
		break;

	case kButtonBreadCrumbs:
		_breadCrumbs[0]._crumbRoomNum = 0;
		_numCrumbs = 0;
		_droppingCrumbs = true;
		_interface->mayShowCrumbIndicator();
		break;

	case kButtonFollowCrumbs:
		if (_droppingCrumbs) {
			if (_numCrumbs > 0) {
				_followingCrumbs = true;
				_followCrumbsFast = false;
				_isCrumbTurning = false;
				_isCrumbWaiting = false;
				_crumbTimestamp = _system->getMillis();

				eatMessages();
				_alternate = false;
				_anim->_doBlack = true;

				_mainDisplay = true;
				// Sets the correct button list
				interfaceOn();
				_graphics->drawPanel();
				drawRoomMessage(curInv, _closeDataPtr);
			} else {
				_breadCrumbs[0]._crumbRoomNum = 0;
				_droppingCrumbs = false;

				_interface->mayShowCrumbIndicatorOff();
			}
		}
		break;

	default:
		break;
	}

	_graphics->screenUpdate();
}

void LabEngine::performAction(uint16 actionMode, Common::Point curPos, uint16 &curInv) {
	eatMessages();

	switch (actionMode) {
	case 0:
		// Take something.
		if (doActionRule(curPos, actionMode, _roomNum))
			_curFileName = _newFileName;
		else if (takeItem(curPos))
			drawStaticMessage(kTextTakeItem);
		else if (doActionRule(curPos, kRuleActionTakeDef, _roomNum))
			_curFileName = _newFileName;
		else if (doActionRule(curPos, kRuleActionTake, 0))
			_curFileName = _newFileName;
		else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
			drawStaticMessage(kTextNothing);

		break;

	case 1:
	case 2:
	case 3:
		// Manipulate an object, Open up a "door" or Close a "door"
		if (doActionRule(curPos, actionMode, _roomNum))
			_curFileName = _newFileName;
		else if (!doActionRule(curPos, actionMode, 0)) {
			if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
				drawStaticMessage(kTextNothing);
		}
		break;

	case 4: {
		// Look at closeups
		const CloseData *tmpClosePtr = _closeDataPtr;
		setCurrentClose(curPos, &tmpClosePtr, true);

		if (_closeDataPtr == tmpClosePtr) {
			if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
				drawStaticMessage(kTextNothing);
		} else if (!tmpClosePtr->_graphicName.empty()) {
			_anim->_doBlack = true;
			_closeDataPtr = tmpClosePtr;
		} else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
			drawStaticMessage(kTextNothing);
		}
		break;

	case 5:
		if (_conditions->in(curInv)) {
			// Use an item on something else
			if (doOperateRule(curPos, curInv)) {
				_curFileName = _newFileName;

				if (!_conditions->in(curInv))
					decIncInv(&curInv, false);
			}
			else if (curPos.y < (_utils->vgaScaleY(149) + _utils->svgaCord(2)))
				drawStaticMessage(kTextNothing);
		}
		break;

	default:
		break;
	}
}

void LabEngine::go() {
	if (getPlatform() == Common::kPlatformWindows)
		handleTrialWarning();

	_isHiRes = ((getFeatures() & GF_LOWRES) == 0);
	_graphics->setUpScreens();

	_event->initMouse();
	if (_msgFont)
		_graphics->freeFont(&_msgFont);

	if (getPlatform() != Common::kPlatformAmiga)
		_msgFont = _resource->getFont("F:AvanteG.12");
	else
		_msgFont = _resource->getFont("F:Map.fon");

	// If the user has requested to load a game from the launcher, skip the intro
	if (!ConfMan.hasKey("save_slot")) {
		_event->mouseHide();
		_introPlaying = true;
		Intro *intro = new Intro(this);
		intro->play();
		delete intro;
		_introPlaying = false;
		_event->mouseShow();
	}

	mainGameLoop();

	_graphics->freeFont(&_msgFont);
	_graphics->freePict();

	freeScreens();

	_music->freeMusic();
}

MainButton LabEngine::followCrumbs() {
	// kDirectionNorth, kDirectionSouth, kDirectionEast, kDirectionWest
	MainButton movement[4][4] = {
		{ kButtonForward, kButtonRight, kButtonRight, kButtonLeft },
		{ kButtonRight, kButtonForward, kButtonLeft, kButtonRight },
		{ kButtonLeft, kButtonRight, kButtonForward, kButtonRight },
		{ kButtonRight, kButtonLeft, kButtonRight, kButtonForward }
	};

	if (_isCrumbWaiting) {
		if (_system->getMillis() <= _crumbTimestamp)
			return kButtonNone;

		_isCrumbWaiting = false;
	}

	if (!_isCrumbTurning)
		_breadCrumbs[_numCrumbs--]._crumbRoomNum = 0;

	// Is the current crumb this room? If not, logic error.
	if (_roomNum != _breadCrumbs[_numCrumbs]._crumbRoomNum) {
		_numCrumbs = 0;
		_breadCrumbs[0]._crumbRoomNum = 0;
		_droppingCrumbs = false;
		_followingCrumbs = false;
		return kButtonNone;
	}

	Direction exitDir;
	// which direction is last crumb
	if (_breadCrumbs[_numCrumbs]._crumbDirection == kDirectionEast)
		exitDir = kDirectionWest;
	else if (_breadCrumbs[_numCrumbs]._crumbDirection == kDirectionWest)
		exitDir = kDirectionEast;
	else if (_breadCrumbs[_numCrumbs]._crumbDirection == kDirectionNorth)
		exitDir = kDirectionSouth;
	else
		exitDir = kDirectionNorth;

	MainButton moveDir = movement[_direction][exitDir];

	if (_numCrumbs == 0) {
		_isCrumbTurning = false;
		_breadCrumbs[0]._crumbRoomNum = 0;
		_droppingCrumbs = false;
		_followingCrumbs = false;
	} else {
		_isCrumbTurning = (moveDir != kButtonForward);
		_isCrumbWaiting = true;

		int theDelay = (_followCrumbsFast ? 1000 / 4 : 1000);
		_crumbTimestamp = theDelay + _system->getMillis();
	}

	return moveDir;
}

} // End of namespace Lab
