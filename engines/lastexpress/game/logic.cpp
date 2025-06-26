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

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/game/beetle.h"
#include "lastexpress/game/savegame.h"

#include "lastexpress/menu/menu.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

LogicManager::LogicManager(LastExpressEngine *engine) {
	_engine = engine;
}

LogicManager::~LogicManager() {
}

void LogicManager::send(int sender, int receiver, int actionId, ConsCallParam param) {
	_engine->getMessageManager()->addMessage(receiver, actionId, sender, param);
}

void LogicManager::sendAll(int character, int action, ConsCallParam param) {
	for (int i = 1; i < 40; ++i) {
		if (i != character)
			_engine->getMessageManager()->addMessage(i, action, character, param);
	}
}

void LogicManager::fedEx(int sender, int receiver, int action, ConsCallParam param) {
	Message msg;

	if (!_engine->isDemo() || _engine->_navigationEngineIsRunning) {
		msg.sender = sender;
		msg.receiver = receiver;
		msg.action = action;
		msg.param = param;
		_engine->getMessageManager()->forceMessage(&msg);
	}
}

void LogicManager::forceJump(int character, void (LogicManager::*functionPointer)(CONS_PARAMS)) {
	if (!_engine->isDemo() || _engine->_navigationEngineIsRunning) {
		getCharacter(character).currentCall = 0;
		getCharacter(character).inventoryItem = 0;

		if (whoRunningDialog(character))
			endDialog(character);

		endGraphics(character);
		releaseEverything(character);

		(this->*functionPointer)(0, 0, 0, 0);
	}
}

void LogicManager::autoMessage(int character, int action, ConsCallParam param) {

	int chosenIdx = 0;
	for (Message *i = _engine->getMessageManager()->_autoMessages; i->receiver; ++i) {
		chosenIdx++;
		if (chosenIdx >= 128)
			return;
	}

	_engine->getMessageManager()->_autoMessages[chosenIdx].receiver = character;
	_engine->getMessageManager()->_autoMessages[chosenIdx].action = action;
	_engine->getMessageManager()->_autoMessages[chosenIdx].param = param;
}

bool LogicManager::doAutoMessage(Message *msg) {
	for (int i = 0; i < 128 && _engine->getMessageManager()->_autoMessages[i].receiver; ++i) {
		if (msg->receiver == _engine->getMessageManager()->_autoMessages[i].receiver &&
			msg->action == _engine->getMessageManager()->_autoMessages[i].action) {

			getCharacterParams(_engine->getMessageManager()->_autoMessages[i].receiver, 8)[_engine->getMessageManager()->_autoMessages[i].param.intParam] = 1;
			return true;
		}
	}

	return false;
}

void LogicManager::save(int character, int type, int event) {
	if (!_engine->isDemo() || _engine->_navigationEngineIsRunning)
		_engine->getVCR()->writeSavePoint(type, character, event);
}

void LogicManager::endGame(int type, int value, int sceneIndex, bool showScene) {
	if (_engine->isDemo() && !_engine->_navigationEngineIsRunning)
		return;

	_engine->getSoundManager()->endAmbient();
	_engine->getOtisManager()->wipeAllGSysInfo();

	_engine->_navigationEngineIsRunning = false;
	_engine->getMessageManager()->clearMessageQueue();
	_engine->_stopUpdatingCharacters = false;

	if (showScene) {
		fadeToWhite();

		if (!sceneIndex) {
			while (!_engine->mouseHasRightClicked()) {
				Slot *i;

				for (i = _engine->getSoundManager()->_soundCache; i; i = i->getNext()) {
					if (i->hasTag(kSoundTagLink))
						break;
				}

				if (!i && !whoRunningDialog(kCharacterClerk))
					break;

				_engine->handleEvents();

				_engine->getSoundManager()->soundThread();
			}
		}

		if (sceneIndex) {
			if (!_engine->mouseHasRightClicked()) {
				bumpCathNode(sceneIndex);

				while (whoRunningDialog(kCharacterTableE)) {
					if (_engine->mouseHasRightClicked())
						break;

					_engine->handleEvents();

					_engine->getSoundManager()->soundThread();
				}
			}
		}
	}

	_engine->getMenu()->doEgg(false, type, value);
}

void LogicManager::winGame() {
	if (_engine->isDemo() && _engine->_navigationEngineIsRunning) {
		_engine->getSoundManager()->endAmbient();
		_engine->getOtisManager()->wipeAllGSysInfo();
		_engine->getMessageManager()->clearMessageQueue();
		_engine->_stopUpdatingCharacters = true;
		_engine->demoEnding(true);
		_engine->getMenu()->doEgg(false, 0, 0);
	} else {
		_engine->getSoundManager()->endAmbient();
		playNIS(kEventFinalSequence);
		_engine->doCredits();
		_engine->getOtisManager()->wipeAllGSysInfo();
		_engine->getMessageManager()->clearMessageQueue();
		_engine->_stopUpdatingCharacters = true;
		_engine->getMenu()->doEgg(false, 0, 0);
	}
}

void LogicManager::killGracePeriod() {
	_engine->_gracePeriodTimer = 0;
}

void LogicManager::fadeToBlack() {
	_engine->getGraphicsManager()->setMouseDrawable(false);
	_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_frontBuffer, 0, 0, 640, 480);
	_engine->getGraphicsManager()->_renderBox1.x = 0;
	_engine->getGraphicsManager()->_renderBox1.y = 0;
	_engine->getGraphicsManager()->_renderBox1.width = 640;
	_engine->getGraphicsManager()->_renderBox1.height = 480;
	_engine->getGraphicsManager()->stepDissolve(&_engine->getGraphicsManager()->_renderBox1);
}

void LogicManager::fadeToWhite() {
	PixMap *backgroundBuffer;
	PixMap white;

	_engine->getGraphicsManager()->setMouseDrawable(false);

	white = 0x7FFF;
	_engine->getGraphicsManager()->modifyPalette(&white, 1);

	backgroundBuffer = _engine->getGraphicsManager()->_frontBuffer;

	for (int i = 480; i > 0; i--) {
		for (int j = 640; j > 0; j--) {
			*backgroundBuffer++ = white;
		}
	}

	_engine->getGraphicsManager()->_renderBox1.x = 0;
	_engine->getGraphicsManager()->_renderBox1.y = 0;
	_engine->getGraphicsManager()->_renderBox1.width = 640;
	_engine->getGraphicsManager()->_renderBox1.height = 480;
	_engine->getGraphicsManager()->stepDissolve(&_engine->getGraphicsManager()->_renderBox1);
}

void LogicManager::restoreIcons() {
	_engine->getGraphicsManager()->drawItemDim(_globals[kGlobalCathIcon], 0, 0, 1);
	_engine->getGraphicsManager()->drawItemDim(_engine->_currentGameFileColorId + 39, 608, 448, 1);

	if (_activeItem) {
		_engine->getGraphicsManager()->drawItem(_items[_activeItem].mnum, 44, 0);
		_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
	}

	_engine->getGraphicsManager()->burstBox(0, 0, 32, 32);
	_engine->getGraphicsManager()->burstBox(608, 448, 32, 32);
}

void LogicManager::dropItem(int item, int outLocation) {
	if (item < 32) {
		if (_items[item].floating != outLocation) {
			_items[item].floating = outLocation;
			if (nodeHasItem(item)) {
				if (!_doubleClickFlag)
					cleanNIS();
			}
		}
	}
}

void LogicManager::takeItem(int item) {
	int cathDir;

	if (item < 32) {
		if (_items[item].floating) {
			_items[item].floating = 0;

			if (item == kItem3) {
				if (checkLoc(kCharacterCath, kCarRestaurant)) {
					cathDir = checkCathDir();
					if (_closeUp)
						cathDir = _trainData[_nodeReturn].cathDir;

					if (cathDir == 56) {
						if (!_closeUp) {
							bumpCath(kCarRestaurant, 56, 255);
							return;
						}

						_nodeReturn = getBumpNode(kCarRestaurant, 56, 255);
						return;
					}
				}
			} else if (item == 5) {
				if (checkLoc(kCharacterCath, kCarRedSleeping) && !_doubleClickFlag) {
					cathDir = checkCathDir();
					if (_closeUp)
						cathDir = _trainData[_nodeReturn].cathDir;
					if (cathDir >= 23 && cathDir <= 32) {
						if (!_closeUp) {
							bumpCath(kCarRedSleeping, cathDir, 255);
							return;
						}

						_nodeReturn = getBumpNode(kCarRedSleeping, cathDir, 255);
						return;
					}
				}
			} else if (item == 7 && checkLoc(kCharacterCath, kCarGreenSleeping) && !_doubleClickFlag) {
				cathDir = checkCathDir();
				if (_closeUp)
					cathDir = _trainData[_nodeReturn].cathDir;

				if (cathDir == 1 || (cathDir >= 22 && cathDir <= 33)) {
					if (_closeUp) {
						_nodeReturn = getBumpNode(kCarGreenSleeping, cathDir, 255);
						return;
					}

					bumpCath(kCarGreenSleeping, cathDir, 255);
				}
			}
		}
	}
}

void LogicManager::giveCathItem(int item) {
	if (item < 32) {
		_items[item].floating = 0;
		_items[item].haveIt = 1;

		if (_items[item].mnum) {
			if (!_items[item].inPocket) {
				_activeItem = item;
				_engine->getGraphicsManager()->drawItem(_items[item].mnum, 44, 0);
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}
		}
	}
}

void LogicManager::takeCathItem(int item) {
	if (item < 32) {
		_items[item].floating = 0;
		_items[item].haveIt = 0;

		if (item == _activeItem) {
			_activeItem = 0;
			if (_engine->getGraphicsManager()->acquireSurface()) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
				_engine->getGraphicsManager()->unlockSurface();
			}

			_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
		}
	}
}

bool LogicManager::cathHasItem(int item) {
	if (item && item < 32)
		return _items[item].haveIt == 1;
	else
		return false;
}

int LogicManager::findLargeItem() {
	Item *inventory = _items;

	for (int i = kItemMatchBox; i < 32; i++) {
		Item *item = &inventory[i];

		if (item->haveIt && inventory[i - 1].haveIt && !item->inPocket) {
			return i;
		}
	}

	return 0;
}

int LogicManager::checkCathDir() {
	return _trainData[_activeNode].cathDir;
}

bool LogicManager::checkCathDir(int car, int position) {
	return getCharacter(kCharacterCath).characterPosition.car == car &&
		   _trainData[_activeNode].cathDir == position;
}

bool LogicManager::isFemale(int character) {
	return character == kCharacterAnna
		|| character == kCharacterTatiana
		|| character == kCharacterVesna
		|| character == kCharacterKahina
		|| character == kCharacterMadame
		|| character == kCharacterRebecca
		|| character == kCharacterSophie
		|| character == kCharacterYasmin
		|| character == kCharacterHadija
		|| character == kCharacterAlouan;
}

bool LogicManager::isSingleFemale(int character) {
	return character == kCharacterTatiana
		|| character == kCharacterRebecca
		|| character == kCharacterSophie;
}

bool LogicManager::isNight() {
	int chapter = _globals[kGlobalChapter];
	return chapter == 1 || chapter == 4 || (chapter == 5 && !_globals[kGlobalIsDayTime]);
}

bool LogicManager::whoOutside(int character) {
	return getCharacter(character).characterPosition.location == 2;
}

bool LogicManager::whoOnScreen(int character) {
	return getCharacter(character).frame1
		&& getCharacter(character).frame1->compType != 3;
}

bool LogicManager::checkLoc(int character, int car) {
	return getCharacter(character).characterPosition.car == car
		&& getCharacter(character).characterPosition.location < 2;
}

bool LogicManager::emptyComp(int car, int position) {
	for (int i = 1; i < 40; i++) {
		if (inComp(i, car, position))
			return false;
	}

	return true;
}

bool LogicManager::inComp(int character, int car, int position) {
	return getCharacter(character).characterPosition.car == car
		&& getCharacter(character).characterPosition.position == position
		&& getCharacter(character).characterPosition.location == 1;
}

bool LogicManager::inComp(int character) {
	uint16 car = getCharacter(character).characterPosition.car;

	return (car == kCarGreenSleeping || car == kCarRedSleeping)
		&& getCharacter(character).characterPosition.location == 1;
}

bool LogicManager::cathOutHisWindow() {
	if (getCharacter(kCharacterCath).characterPosition.location == 2 &&
		getCharacter(kCharacterCath).characterPosition.car == kCarGreenSleeping) {

		uint16 position = getCharacter(kCharacterCath).characterPosition.position;
		if (position == 8200 || position == 7500)
			return true;
	}

	return false;
}

bool LogicManager::cathOutRebeccaWindow() {
	if (getCharacter(kCharacterCath).characterPosition.location == 2 &&
		getCharacter(kCharacterCath).characterPosition.car == kCarRedSleeping) {

		uint16 position = getCharacter(kCharacterCath).characterPosition.position;
		if (position == 4840 || position == 4070)
			return true;
	}

	return false;
}

bool LogicManager::inSuite(int character, int car, int position) {
	if (getCharacter(character).characterPosition.car != car ||
		getCharacter(character).characterPosition.location != 1)
		return false;

	uint16 pos = getCharacter(character).characterPosition.position;

	if (position == 4455) {
		return pos == 4840 || pos == 4070 || pos == 4455;
	}

	if (position == 6130) {
		return pos == 6470 || pos == 5790 || pos == 6130;
	}

	if (position == 7850) {
		return pos == 8200 || pos == 7500 || pos == 7850;
	}

	return false;
}

bool LogicManager::inOffice(int character) {
	if (checkLoc(character, kCarBaggage)) {
		uint16 position = getCharacter(character).characterPosition.position;
		if (position >= 4500 && position <= 5500)
			return true;
	}

	return false;
}

bool LogicManager::inLowBaggage(int character) {
	return checkLoc(character, kCarBaggage)
		&& getCharacter(character).characterPosition.position < 4500;
}

bool LogicManager::inSalon(int character) {
	if (checkLoc(character, kCarRestaurant)) {
		uint16 position = getCharacter(character).characterPosition.position;
		if (position >= 1540 && position <= 3650)
			return true;
	}

	return false;
}

bool LogicManager::inDiningRoom(int character) {
	if (checkLoc(character, kCarRestaurant)) {
		uint16 position = getCharacter(character).characterPosition.position;
		if (position >= 3650 && position <= 5800)
			return true;
	}

	return false;
}

bool LogicManager::inKitchen(int character) {
	return checkLoc(character, kCarRestaurant)
		&& getCharacter(character).characterPosition.position > 5800;
}

bool LogicManager::inOuterSanctum(int character) {
	if (checkLoc(character, kCarKronos)) {
		uint16 position = getCharacter(character).characterPosition.position;
		if (position >= 5500 && position <= 7500)
			return true;
	}

	return false;
}

bool LogicManager::inInnerSanctum(int character) {
	if (checkLoc(character, kCarKronos)) {
		uint16 position = getCharacter(character).characterPosition.position;
		if (position >= 3500 && position <= 5500)
			return true;
	}

	return false;
}

bool LogicManager::onLowPlatform(int character) {
	return checkLoc(character, kCarGreenSleeping)
		&& getCharacter(character).characterPosition.position < 850;
}

bool LogicManager::onKronPlatform(int character) {
	return checkLoc(character, kCarKronos)
		&& getCharacter(character).characterPosition.position < 7900;
}

bool LogicManager::cathInCorridor(int car) {
	return checkLoc(kCharacterCath, car)
		&& !getCharacter(kCharacterCath).characterPosition.location
		&& !onLowPlatform(kCharacterCath);
}

bool LogicManager::cathFacingUp() {
	return _engine->getOtisManager()->fDirection(0);
}

bool LogicManager::rcClear() {
	for (int i = 1; i < 31; i++) {
		if (!getCharacter(i).characterPosition.location && (inSalon(i) || inDiningRoom(i))) {
			return false;
		}
	}

	return true;
}

int32 LogicManager::absPosition(int character1, int character2) {
	return ABS<int>((int)(getCharacter(character1).characterPosition.position
						- getCharacter(character2).characterPosition.position));
}

bool LogicManager::nearChar(int character1, int character2, int maxDist) {
	return getCharacter(character2).characterPosition.car == getCharacter(character1).characterPosition.car
		&& absPosition(character1, character2) <= maxDist
		&& (getCharacter(character1).characterPosition.location != 2
		 || getCharacter(character2).characterPosition.location != 2);
}

bool LogicManager::nearX(int character, int x, int maxDist) {
	return maxDist >= ABS<int>((int)getCharacter(character).characterPosition.position - x);
}

bool LogicManager::whoFacingCath(int character) {
	if (getCharacter(character).direction == 1 && _engine->getOtisManager()->rDirection(0))
		return true;

	return getCharacter(character).direction == 2 && _engine->getOtisManager()->fDirection(0);
}

bool LogicManager::whoWalking(int character) {
	int8 direction = getCharacter(character).direction;

	return direction == 1 || direction == 2;
}

int LogicManager::checkDoor(int door) {
	if (door < 128) {
		return _doors[door].status;
	} else {
		return 0;
	}
}

bool LogicManager::preventEnterComp(int door) {
	int car;
	int position;

	if (door && door <= 8) {
		car = kCarGreenSleeping;
		position = _doorPositions[door - 1];

		if (inComp(kCharacterCath, car, position))
			return false;

	} else if (door < 32 || door > 39) {
		if (door < 17 || door > 22) {
			if (door < 48 || door > 53)
				return false;

			car = kCarRedSleeping;
			position = _doorPositions[door - 48];
		} else {
			car = kCarGreenSleeping;
			position = _doorPositions[door - 17];
		}

	} else {
		car = kCarRedSleeping;
		position = _doorPositions[door - 32];
		if (inComp(kCharacterCath, car, position))
			return false;
	}

	for (int i = 1; i < 40; i++) {
		if (inComp(i, car, position) && i != kCharacterVassili)
			return true;
	}

	return false;	
}

void LogicManager::setDoor(int door, int character, int status, int windowCursor, int handleCursor) {
	if (door < 128) {
		int oldStatus = _doors[door].status;
		_doors[door].who = character;
		_doors[door].status = status;

		if (windowCursor != 255 || handleCursor != 255) {
			if (windowCursor != 255)
				_doors[door].windowCursor = windowCursor;

			if (handleCursor != 255)
				_doors[door].handleCursor = handleCursor;

			mouseStatus();
		}

		_engine->getSoundManager()->_scanAnySoundLoopingSection = true;

		if (oldStatus != status && (oldStatus == 2 || status == 2) && ((door && door <= 8) || (door >= 32 && door <= 39)))
			_engine->getOtisManager()->drawLooseSprites();
	}
}

void LogicManager::setModel(int door, int8 model) {
	if (door < 128)
		_doors[door].model = model;
}

int LogicManager::getModel(int door) {
	if (door < 128) {
		return _doors[door].model;
	} else {
		return 0;
	}
}

void LogicManager::blockView(int character, int car, int position) {
	if (character == kCharacterMitchell)
		character = kCharacterCath;

	if (character <= 31) {
		_blockedViews[100 * car + position] |= 1 << character;
		if (checkCathDir(car, position) || (car == kCarRestaurant && position == 57 && checkCathDir(kCarRestaurant, 50))) {
			playChrExcuseMe(character, kCharacterCath, 0);
			smartBumpCath();
			playDialog(kCharacterCath, "CAT1127A", -1, 0);
		} else {
			mouseStatus();
		}
	}
}

void LogicManager::releaseView(int character, int car, int position) {
	if (character == kCharacterMitchell)
		character = kCharacterCath;

	if (character <= 31) {
		_blockedViews[100 * car + position] &= ~(1 << character);
		mouseStatus();
	}
}

void LogicManager::releaseEverything(int character) {
	if (character == kCharacterMitchell)
		character = kCharacterCath;

	if (character <= 31) {
		for (int i = 0; i < 1000; i++) {
			_blockedViews[i] &= ~(1 << character);
		}

		for (int i = 0; i < 16; _softBlockedX[i - 1] &= ~(1 << character)) {
			_blockedX[i++] &= ~(1 << character);
		}

		mouseStatus();
	}
}

void LogicManager::release2Views(int character, int car, int pos1, int pos2) {
	if (character == kCharacterMitchell)
		character = kCharacterCath;

	if (character <= 31) {
		_blockedViews[100 * car + pos1] &= ~(1 << character);
		_blockedViews[100 * car + pos2] &= ~(1 << character);
		mouseStatus();
	}
}

void LogicManager::block2ViewsBump4(int character, int car, int pos1, int pos2, int pos3, int pos4) {
	if (character == kCharacterMitchell)
		character = kCharacterCath;

	if (character <= 31) {
		_blockedViews[100 * car + pos1] |= 1 << character;
		_blockedViews[100 * car + pos2] |= 1 << character;

		if ((checkCathDir(car, pos1) || checkCathDir(car, pos2) || checkCathDir(car, pos3) || checkCathDir(car, pos4))) {
			playChrExcuseMe(character, kCharacterCath, 0);
			smartBumpCath();
			playDialog(kCharacterCath, "CAT1127A", -1, 0);
		} else {
			mouseStatus();
		}
	}
}

void LogicManager::blockAtDoor(int character, int status) {
	if (character <= 31 && (status - 1) <= 38) {
		switch (status) {
		case 1:
			block2ViewsBump4(character, kCarGreenSleeping, 41, 51, 17, 38);
			_blockedX[0] |= 1 << character;
			break;
		case 2:
			block2ViewsBump4(character, kCarGreenSleeping, 42, 52, 15, 36);
			_blockedX[1] |= 1 << character;
			break;
		case 3:
			block2ViewsBump4(character, kCarGreenSleeping, 43, 53, 13, 34);
			_blockedX[2] |= 1 << character;
			break;
		case 4:
			block2ViewsBump4(character, kCarGreenSleeping, 44, 54, 11, 32);
			_blockedX[3] |= 1 << character;
			break;
		case 5:
			block2ViewsBump4(character, kCarGreenSleeping, 45, 55, 9, 30);
			_blockedX[4] |= 1 << character;
			break;
		case 6:
			block2ViewsBump4(character, kCarGreenSleeping, 46, 56, 7, 28);
			_blockedX[5] |= 1 << character;
			break;
		case 7:
			block2ViewsBump4(character, kCarGreenSleeping, 47, 57, 5, 26);
			_blockedX[6] |= 1 << character;
			break;
		case 8:
			block2ViewsBump4(character, kCarGreenSleeping, 48, 58, 3, 25);
			_blockedX[7] |= 1 << character;
			break;
		case 32:
			block2ViewsBump4(character, kCarRedSleeping, 41, 51, 17, 38);
			_blockedX[8] |= 1 << character;
			break;
		case 33:
			block2ViewsBump4(character, kCarRedSleeping, 42, 52, 15, 36);
			_blockedX[9] |= 1 << character;
			break;
		case 34:
			block2ViewsBump4(character, kCarRedSleeping, 43, 53, 13, 34);
			_blockedX[10] |= 1 << character;
			break;
		case 35:
			block2ViewsBump4(character, kCarRedSleeping, 44, 54, 11, 32);
			_blockedX[11] |= 1 << character;
			break;
		case 36:
			block2ViewsBump4(character, kCarRedSleeping, 45, 55, 9, 30);
			_blockedX[12] |= 1 << character;
			break;
		case 37:
			block2ViewsBump4(character, kCarRedSleeping, 46, 56, 7, 28);
			_blockedX[13] |= 1 << character;
			break;
		case 38:
			block2ViewsBump4(character, kCarRedSleeping, 47, 57, 5, 26);
			_blockedX[14] |= 1 << character;
			break;
		case 39:
			block2ViewsBump4(character, kCarRedSleeping, 48, 58, 3, 25);
			_blockedX[15] |= 1 << character;
			break;
		default:
			return;
		}
	}
}

void LogicManager::releaseAtDoor(int character, int status) {
	if (character <= 31 && (status - 1) <= 38) {
		switch (status) {
		case 1:
			release2Views(character, kCarGreenSleeping, 41, 51);
			_blockedX[0] &= ~(1 << character);
			break;
		case 2:
			release2Views(character, kCarGreenSleeping, 42, 52);
			_blockedX[1] &= ~(1 << character);
			break;
		case 3:
			release2Views(character, kCarGreenSleeping, 43, 53);
			_blockedX[2] &= ~(1 << character);
			break;
		case 4:
			release2Views(character, kCarGreenSleeping, 44, 54);
			_blockedX[3] &= ~(1 << character);
			break;
		case 5:
			release2Views(character, kCarGreenSleeping, 45, 55);
			_blockedX[4] &= ~(1 << character);
			break;
		case 6:
			release2Views(character, kCarGreenSleeping, 46, 56);
			_blockedX[5] &= ~(1 << character);
			break;
		case 7:
			release2Views(character, kCarGreenSleeping, 47, 57);
			_blockedX[6] &= ~(1 << character);
			break;
		case 8:
			release2Views(character, kCarGreenSleeping, 48, 58);
			_blockedX[7] &= ~(1 << character);
			break;
		case 32:
			release2Views(character, kCarRedSleeping, 41, 51);
			_blockedX[8] &= ~(1 << character);
			break;
		case 33:
			release2Views(character, kCarRedSleeping, 42, 52);
			_blockedX[9] &= ~(1 << character);
			break;
		case 34:
			release2Views(character, kCarRedSleeping, 43, 53);
			_blockedX[10] &= ~(1 << character);
			break;
		case 35:
			release2Views(character, kCarRedSleeping, 44, 54);
			_blockedX[11] &= ~(1 << character);
			break;
		case 36:
			release2Views(character, kCarRedSleeping, 45, 55);
			_blockedX[12] &= ~(1 << character);
			break;
		case 37:
			release2Views(character, kCarRedSleeping, 46, 56);
			_blockedX[13] &= ~(1 << character);
			break;
		case 38:
			release2Views(character, kCarRedSleeping, 47, 57);
			_blockedX[14] &= ~(1 << character);
			break;
		case 39:
			release2Views(character, kCarRedSleeping, 48, 58);
			_blockedX[15] &= ~(1 << character);
			break;
		default:
			return;
		}
	}
}

void LogicManager::softBlockAtDoor(int character, int status) {
	if (character <= 31 && (status - 1) <= 38) {
		switch (status) {
		case 1:
			block2ViewsBump4(character, kCarGreenSleeping, 41, 51, 17, 38);
			_softBlockedX[0] |= 1 << character;
			break;
		case 2:
			block2ViewsBump4(character, kCarGreenSleeping, 42, 52, 15, 36);
			_softBlockedX[1] |= 1 << character;
			break;
		case 3:
			block2ViewsBump4(character, kCarGreenSleeping, 43, 53, 13, 34);
			_softBlockedX[2] |= 1 << character;
			break;
		case 4:
			block2ViewsBump4(character, kCarGreenSleeping, 44, 54, 11, 32);
			_softBlockedX[3] |= 1 << character;
			break;
		case 5:
			block2ViewsBump4(character, kCarGreenSleeping, 45, 55, 9, 30);
			_softBlockedX[4] |= 1 << character;
			break;
		case 6:
			block2ViewsBump4(character, kCarGreenSleeping, 46, 56, 7, 28);
			_softBlockedX[5] |= 1 << character;
			break;
		case 7:
			block2ViewsBump4(character, kCarGreenSleeping, 47, 57, 5, 26);
			_softBlockedX[6] |= 1 << character;
			break;
		case 8:
			block2ViewsBump4(character, kCarGreenSleeping, 48, 58, 3, 25);
			_softBlockedX[7] |= 1 << character;
			break;
		case 32:
			block2ViewsBump4(character, kCarRedSleeping, 41, 51, 17, 38);
			_softBlockedX[8] |= 1 << character;
			break;
		case 33:
			block2ViewsBump4(character, kCarRedSleeping, 42, 52, 15, 36);
			_softBlockedX[9] |= 1 << character;
			break;
		case 34:
			block2ViewsBump4(character, kCarRedSleeping, 43, 53, 13, 34);
			_softBlockedX[10] |= 1 << character;
			break;
		case 35:
			block2ViewsBump4(character, kCarRedSleeping, 44, 54, 11, 32);
			_softBlockedX[11] |= 1 << character;
			break;
		case 36:
			block2ViewsBump4(character, kCarRedSleeping, 45, 55, 9, 30);
			_softBlockedX[12] |= 1 << character;
			break;
		case 37:
			block2ViewsBump4(character, kCarRedSleeping, 46, 56, 7, 28);
			_softBlockedX[13] |= 1 << character;
			break;
		case 38:
			block2ViewsBump4(character, kCarRedSleeping, 47, 57, 5, 26);
			_softBlockedX[14] |= 1 << character;
			break;
		case 39:
			block2ViewsBump4(character, kCarRedSleeping, 48, 58, 3, 25);
			_softBlockedX[15] |= 1 << character;
			break;
		default:
			return;
		}
	}
}

void LogicManager::softReleaseAtDoor(int character, int status) {
	if (character <= 31 && (status - 1) <= 38) {
		switch (status) {
		case 1:
			release2Views(character, kCarGreenSleeping, 41, 51);
			_softBlockedX[0] &= ~(1 << character);
			break;
		case 2:
			release2Views(character, kCarGreenSleeping, 42, 52);
			_softBlockedX[1] &= ~(1 << character);
			break;
		case 3:
			release2Views(character, kCarGreenSleeping, 43, 53);
			_softBlockedX[2] &= ~(1 << character);
			break;
		case 4:
			release2Views(character, kCarGreenSleeping, 44, 54);
			_softBlockedX[3] &= ~(1 << character);
			break;
		case 5:
			release2Views(character, kCarGreenSleeping, 45, 55);
			_softBlockedX[4] &= ~(1 << character);
			break;
		case 6:
			release2Views(character, kCarGreenSleeping, 46, 56);
			_softBlockedX[5] &= ~(1 << character);
			break;
		case 7:
			release2Views(character, kCarGreenSleeping, 47, 57);
			_softBlockedX[6] &= ~(1 << character);
			break;
		case 8:
			release2Views(character, kCarGreenSleeping, 48, 58);
			_softBlockedX[7] &= ~(1 << character);
			break;
		case 32:
			release2Views(character, kCarRedSleeping, 41, 51);
			_softBlockedX[8] &= ~(1 << character);
			break;
		case 33:
			release2Views(character, kCarRedSleeping, 42, 52);
			_softBlockedX[9] &= ~(1 << character);
			break;
		case 34:
			release2Views(character, kCarRedSleeping, 43, 53);
			_softBlockedX[10] &= ~(1 << character);
			break;
		case 35:
			release2Views(character, kCarRedSleeping, 44, 54);
			_softBlockedX[11] &= ~(1 << character);
			break;
		case 36:
			release2Views(character, kCarRedSleeping, 45, 55);
			_softBlockedX[12] &= ~(1 << character);
			break;
		case 37:
			release2Views(character, kCarRedSleeping, 46, 56);
			_softBlockedX[13] &= ~(1 << character);
			break;
		case 38:
			release2Views(character, kCarRedSleeping, 47, 57);
			_softBlockedX[14] &= ~(1 << character);
			break;
		case 39:
			release2Views(character, kCarRedSleeping, 48, 58);
			_softBlockedX[15] &= ~(1 << character);
			break;
		default:
			return;
		}
	}
}

int LogicManager::getBumpNode(int car, int position, int param) {
	int i;

	if (_numberOfScenes <= 1)
		return _activeNode;

	for (i = 1; i < _numberOfScenes; i++) {
		Node *curNode = &_trainData[i];

		if (curNode->nodePosition.car != car || curNode->cathDir != position)
			continue;

		if ((param == 0xFF && curNode->parameter3 == 0) ||
			curNode->parameter3 == param ||
			curNode->property == kNodeHas3Items)
			break;
	}

	if (i >= _numberOfScenes)
		return _activeNode;


	if (_blockedViews[100 * _trainData[i].nodePosition.car + _trainData[i].cathDir]) {
		return getSmartBumpNode(i);
	} else {
		return i;
	}
}

void LogicManager::bumpCath(int car, int position, int param) {
	if (!_engine->isDemo() || _engine->_navigationEngineIsRunning) {
		bumpCathNode(getBumpNode(car, position, param));
	}
}

bool LogicManager::obstacleBetween(int character1, int character2) {
	int offset;
	uint32 char1Pos;
	uint32 char2Pos;
	int i, j;

	if (getCharacter(character1).characterPosition.car != getCharacter(character2).characterPosition.car ||
		getCharacter(character1).characterPosition.car < kCarGreenSleeping ||
		getCharacter(character1).characterPosition.car > kCarRedSleeping) {
		return false;
	}

	if (getCharacter(character1).characterPosition.car == 3) {
		offset = 0;
	} else {
		offset = 8;
	}

	if (getCharacter(character1).characterPosition.position >= getCharacter(character2).characterPosition.position) {
		char1Pos = getCharacter(character2).characterPosition.position;
		char2Pos = getCharacter(character1).characterPosition.position;
	} else {
		char1Pos = getCharacter(character1).characterPosition.position;
		char2Pos = getCharacter(character2).characterPosition.position;
	}

	for (i = 7; i > -1 && _doorPositions[i] < (int)char1Pos; i--);

	for (j = 0; j < 8 && _doorPositions[j] > (int)char2Pos; j++);

	if (i > -1 && j < 8) {
		while (j <= i) {
			if (_blockedX[j + offset] || _softBlockedX[j + offset])
				return true;

			j++;
		}
	}

	// The original code went for "k <= 40" here, but that would trigger a bad memory access...
	for (int k = 1; k < 40; ++k) {
		if (k != character1 && k != character2 && whoWalking(k) &&
			getCharacter(k).characterPosition.car == getCharacter(character1).characterPosition.car &&
			getCharacter(k).characterPosition.position > char1Pos && getCharacter(k).characterPosition.position < char2Pos) {
			return true;
		}
	}

	return false;
}

bool LogicManager::bumpCathTowardsCond(int door, bool playSound, bool loadScene) {
	if (getCharacter(kCharacterCath).characterPosition.location || ((door < 2 || door > 8) && (door < 32 || door > 39))) {
		return false;
	}

	if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(kCharacterPolice).characterPosition.car &&
		!getCharacter(kCharacterPolice).characterPosition.location && !obstacleBetween(kCharacterCath, kCharacterPolice)) {
		if (playSound) {
			if (_doors[door].status == 1 || _doors[door].status == 3 || preventEnterComp(door)) {
				queueSFX(kCharacterCath, 13, 0);
			} else {
				queueSFX(kCharacterCath, 14, 0);
				queueSFX(kCharacterCath, 15, 3);
			}
		}

		if (loadScene)
			bumpCathRDoor(door);

		return true;
	}

	if (getCharacter(kCharacterCath).characterPosition.car == kCarGreenSleeping &&
		getCharacter(kCharacterCond1).characterPosition.car == kCarGreenSleeping &&
		!getCharacter(kCharacterCond1).characterPosition.location &&
		!getCharacterParams(kCharacterCond1, 8)[0]) {

		if (!obstacleBetween(kCharacterCath, kCharacterCond1)) {
			if (getCharacter(kCharacterCond1).characterPosition.position < 2740 &&
				getCharacter(kCharacterCond1).characterPosition.position > 850 &&
				(getCharacter(kCharacterCond2).characterPosition.car != kCarGreenSleeping || getCharacter(kCharacterCond2).characterPosition.position > 2740) &&
				(getCharacter(kCharacterTrainM).characterPosition.car != kCarGreenSleeping || getCharacter(kCharacterTrainM).characterPosition.position > 2740)) {

				if (playSound) {
					if (_doors[door].status == 1 || _doors[door].status == 3 || preventEnterComp(door)) {
						queueSFX(kCharacterCath, 13, 0);
					} else {
						queueSFX(kCharacterCath, 14, 0);
						queueSFX(kCharacterCath, 15, 3);
					}
				}

				if (!whoRunningDialog(kCharacterCond1))
					playCondYelling(kCharacterCond1, door);

				send(kCharacterCath, kCharacterCond1, 305159806, 0);

				if (loadScene)
					bumpCathRDoor(door);

				return true;
			}
		}

		if (!obstacleBetween(kCharacterCath, kCharacterCond1) &&
			getCharacter(kCharacterCond1).direction == 1 &&
			getCharacter(kCharacterCond1).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position) {

			if (playSound) {
				if (_doors[door].status == 1 || _doors[door].status == 3 || preventEnterComp(door)) {
					queueSFX(kCharacterCath, 13, 0);
				} else {
					queueSFX(kCharacterCath, 14, 0);
					queueSFX(kCharacterCath, 15, 3);
				}
			}

			if (!whoRunningDialog(kCharacterCond1)) {
				if (rnd(2) == 0) {
					playDialog(kCharacterCond1, "CON1000A", -1, 0);
				} else {
					playDialog(kCharacterCond1, "CON1000", -1, 0);
				}
			}

			if (loadScene)
				bumpCathRDoor(door);

			return true;
		}

		if (!obstacleBetween(kCharacterCath, kCharacterCond1) &&
			getCharacter(kCharacterCond1).direction == 2 &&
			getCharacter(kCharacterCond1).characterPosition.position > getCharacter(kCharacterCath).characterPosition.position) {

			if (playSound) {
				if (_doors[door].status == 1 || _doors[door].status == 3 || preventEnterComp(door)) {
					queueSFX(kCharacterCath, 13, 0);
				} else {
					queueSFX(kCharacterCath, 14, 0);
					queueSFX(kCharacterCath, 15, 3);
				}
			}

			if (!whoRunningDialog(kCharacterCond1)) {
				if (rnd(2) == 0) {
					playDialog(kCharacterCond1, "CON1000A", -1, 0);
				} else {
					playDialog(kCharacterCond1, "CON1000", -1, 0);
				}
			}

			if (loadScene)
				bumpCathFDoor(door);

			return true;
		}
	}

	if (getCharacter(kCharacterCath).characterPosition.car != kCarRedSleeping ||
		getCharacter(kCharacterCond2).characterPosition.car == kCarNone ||
		getCharacter(kCharacterCond2).characterPosition.location ||
		getCharacterParams(kCharacterCond2, 8)[0]) {
		return false;
	}

	if (!obstacleBetween(kCharacterCath, kCharacterCond2)) {
		if (getCharacter(kCharacterCond2).characterPosition.position < 2740 &&
			getCharacter(kCharacterCond2).characterPosition.position > 850 &&
			(getCharacter(kCharacterCond1).characterPosition.car != kCarRedSleeping || getCharacter(kCharacterCond1).characterPosition.position > 2740) &&
			(getCharacter(kCharacterTrainM).characterPosition.car != kCarRedSleeping || getCharacter(kCharacterTrainM).characterPosition.position > 2740) &&
			(getCharacter(kCharacterMadame).characterPosition.car != kCarRedSleeping || getCharacter(kCharacterMadame).characterPosition.position > 2740)) {

			if (playSound) {
				if (_doors[door].status == 1 || _doors[door].status == 3 || preventEnterComp(door)) {
					queueSFX(kCharacterCath, 13, 0);
				} else {
					queueSFX(kCharacterCath, 14, 0);
					queueSFX(kCharacterCath, 15, 3);
				}
			}

			if (!whoRunningDialog(kCharacterCond2))
				playCondYelling(kCharacterCond2, door);

			send(kCharacterCath, kCharacterCond2, 305159806, 0);

			if (loadScene)
				bumpCathRDoor(door);

			return true;
		}
	}

	if (!obstacleBetween(kCharacterCath, kCharacterCond2) && getCharacter(kCharacterCond2).direction == 1 && getCharacter(kCharacterCath).characterPosition.position > getCharacter(kCharacterCond2).characterPosition.position) {
		if (playSound) {
			if (_doors[door].status == 1 || _doors[door].status == 3 || preventEnterComp(door)) {
				queueSFX(kCharacterCath, 13, 0);
			} else {
				queueSFX(kCharacterCath, 14, 0);
				queueSFX(kCharacterCath, 15, 3);
			}
		}

		if (!whoRunningDialog(kCharacterCond2)) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCond2, "JAC1000A", -1, 0);
			} else {
				playDialog(kCharacterCond2, "JAC1000", -1, 0);
			}
		}

		if (loadScene)
			bumpCathRDoor(door);

		return true;
	}

	if (obstacleBetween(kCharacterCath, kCharacterCond2) || getCharacter(kCharacterCond2).direction != 2 || getCharacter(kCharacterCath).characterPosition.position >= getCharacter(kCharacterCond2).characterPosition.position) {
		return false;
	}

	if (playSound) {
		if (_doors[door].status == 1 || _doors[door].status == 3 || preventEnterComp(door)) {
			queueSFX(kCharacterCath, 13, 0);
		} else {
			queueSFX(kCharacterCath, 14, 0);
			queueSFX(kCharacterCath, 15, 3);
		}
	}

	if (!whoRunningDialog(kCharacterCond2)) {
		if (rnd(2) == 0) {
			playDialog(kCharacterCond2, "JAC1000A", -1, 0);
		} else {
			playDialog(kCharacterCond2, "JAC1000", -1, 0);
		}
	}

	if (loadScene)
		bumpCathFDoor(door);

	return true;
}

void LogicManager::bumpCathFx(int car, int position) {
	int i;

	for (i = 1; i <= 18; i++) {
		if (_engine->_fudgePosition[i] > position)
			break;
	}

	if (i <= 18) {
		if (i == 1) {
			bumpCath(car, 1, 255);
		} else if (i == 5) {
			bumpCath(car, 3, 255);
		} else {
			bumpCath(car, i - 1, 255);
		}
	} else {
		bumpCath(car, 18, 255);
	}
}

void LogicManager::bumpCathRx(int car, int position) {
	int i;

	for (i = 40; i >= 22; i--) {
		if (_engine->_fudgePosition[i] < position && i != 24)
			break;
	}

	if (i >= 22) {
		if (i == 40) {
			bumpCath(car, 40, 255);
		} else if (i == 23) {
			bumpCath(car, 25, 255);
		} else {
			bumpCath(car, i + 1, 255);
		}
	} else {
		bumpCath(car, 22, 255);
	}
}

void LogicManager::bumpCathFDoor(int door) {
	if ((door - 1) <= 38) {
		switch (door) {
		case 1:
			bumpCath(kCarGreenSleeping, 17, 255);
			break;
		case 2:
			bumpCath(kCarGreenSleeping, 15, 255);
			break;
		case 3:
			bumpCath(kCarGreenSleeping, 13, 255);
			break;
		case 4:
			bumpCath(kCarGreenSleeping, 11, 255);
			break;
		case 5:
			bumpCath(kCarGreenSleeping, 9, 255);
			break;
		case 6:
			bumpCath(kCarGreenSleeping, 7, 255);
			break;
		case 7:
			bumpCath(kCarGreenSleeping, 5, 255);
			break;
		case 8:
			bumpCath(kCarGreenSleeping, 3, 255);
			break;
		case 32:
			bumpCath(kCarRedSleeping, 17, 255);
			break;
		case 33:
			bumpCath(kCarRedSleeping, 15, 255);
			break;
		case 34:
			bumpCath(kCarRedSleeping, 13, 255);
			break;
		case 35:
			bumpCath(kCarRedSleeping, 11, 255);
			break;
		case 36:
			bumpCath(kCarRedSleeping, 9, 255);
			break;
		case 37:
			bumpCath(kCarRedSleeping, 7, 255);
			break;
		case 38:
			bumpCath(kCarRedSleeping, 5, 255);
			break;
		case 39:
			bumpCath(kCarRedSleeping, 3, 255);
			break;
		default:
			return;
		}
	}
}

void LogicManager::bumpCathRDoor(int door) {
	if ((door - 1) <= 38) {
		switch (door) {
		case 1:
			bumpCath(kCarGreenSleeping, 38, 255);
			break;
		case 2:
			bumpCath(kCarGreenSleeping, 36, 255);
			break;
		case 3:
			bumpCath(kCarGreenSleeping, 34, 255);
			break;
		case 4:
			bumpCath(kCarGreenSleeping, 32, 255);
			break;
		case 5:
			bumpCath(kCarGreenSleeping, 30, 255);
			break;
		case 6:
			bumpCath(kCarGreenSleeping, 28, 255);
			break;
		case 7:
			bumpCath(kCarGreenSleeping, 26, 255);
			break;
		case 8:
			bumpCath(kCarGreenSleeping, 25, 255);
			break;
		case 32:
			bumpCath(kCarRedSleeping, 38, 255);
			break;
		case 33:
			bumpCath(kCarRedSleeping, 36, 255);
			break;
		case 34:
			bumpCath(kCarRedSleeping, 34, 255);
			break;
		case 35:
			bumpCath(kCarRedSleeping, 32, 255);
			break;
		case 36:
			bumpCath(kCarRedSleeping, 30, 255);
			break;
		case 37:
			bumpCath(kCarRedSleeping, 28, 255);
			break;
		case 38:
			bumpCath(kCarRedSleeping, 26, 255);
			break;
		case 39:
			bumpCath(kCarRedSleeping, 25, 255);
			break;
		default:
			return;
		}
	}
}

void LogicManager::bumpCathTylerComp() {
	bumpCathNode(41);
}

int LogicManager::getSmartBumpNode(int node) {
	uint8 effDir;
	uint16 car = _trainData[node].nodePosition.car;

	if (car <= kCarKronos)
		return node;

	if (car <= kCarRedSleeping) {
		if (_engine->getOtisManager()->corrRender(0)) {
			if (!_engine->getOtisManager()->fDirection(0)) {
				effDir = _trainData[node].cathDir + 1;
			} else {
				effDir = _trainData[node].cathDir - 1;
			}

			if (effDir == 4)
				effDir = 3;

			if (effDir == 24)
				effDir = 25;

			if (_blockedViews[100 * car + effDir]) {
				return node;
			} else {
				return getBumpNode(car, effDir, 255);
			}
		} else {
			switch (_trainData[node].cathDir) {
			case 41:
			case 51:
				if (!_blockedViews[100 * car + 39])
					return getBumpNode(car, 39, 255);

				// Fallthrough
			case 42:
			case 52:
				if (!_blockedViews[100 * car + 14])
					return getBumpNode(car, 14, 255);

				// Fallthrough
			case 43:
			case 53:
				if (!_blockedViews[100 * car + 35])
					return getBumpNode(car, 35, 255);

				// Fallthrough
			case 44:
			case 54:
				if (!_blockedViews[100 * car + 10])
					return getBumpNode(car, 10, 255);

				// Fallthrough
			case 45:
			case 55:
				if (!_blockedViews[100 * car + 32])
					return getBumpNode(car, 32, 255);

				// Fallthrough
			case 46:
			case 56:
				if (!_blockedViews[100 * car + 7])
					return getBumpNode(car, 7, 255);

				// Fallthrough
			case 47:
			case 57:
				if (!_blockedViews[100 * car + 27])
					return getBumpNode(car, 27, 255);

				// Fallthrough
			case 48:
			case 58:
				if (!_blockedViews[100 * car + 2])
					return getBumpNode(car, 2, 255);

				// Fallthrough
			default:
				return node;
			}
		}
	} else {
		if (car != kCarRestaurant)
			return node;

		switch (_trainData[node].cathDir) {
		case 50:
		case 56:
		case 57:
		case 58:
			if (!_blockedViews[555])
				return getBumpNode(kCarRestaurant, 55, 255);

			if (!_blockedViews[560])
				return getBumpNode(kCarRestaurant, 60, 255);

			if (!_blockedViews[559])
				return getBumpNode(kCarRestaurant, 59, 255);

			if (!_blockedViews[561])
				return getBumpNode(kCarRestaurant, 61, 255);

			if (!_blockedViews[565])
				return getBumpNode(kCarRestaurant, 65, 255);

			if (!_blockedViews[570])
				return getBumpNode(kCarRestaurant, 70, 255);

			return node;
		case 52:
		case 53:
		case 54:
			if (!_blockedViews[551])
				return getBumpNode(kCarRestaurant, 51, 255);

			if (!_blockedViews[555])
				return getBumpNode(kCarRestaurant, 55, 255);

			if (!_blockedViews[560])
				return getBumpNode(kCarRestaurant, 60, 255);

			if (!_blockedViews[559])
				return getBumpNode(kCarRestaurant, 59, 255);

			if (!_blockedViews[561])
				return getBumpNode(kCarRestaurant, 61, 255);

			if (!_blockedViews[565])
				return getBumpNode(kCarRestaurant, 65, 255);

			if (!_blockedViews[570])
				return getBumpNode(kCarRestaurant, 70, 255);

			return node;
		case 59:
			if (!_blockedViews[560])
				return getBumpNode(kCarRestaurant, 60, 255);

			if (!_blockedViews[559])
				return getBumpNode(kCarRestaurant, 59, 255);

			if (!_blockedViews[561])
				return getBumpNode(kCarRestaurant, 61, 255);

			if (!_blockedViews[565])
				return getBumpNode(kCarRestaurant, 65, 255);

			if (!_blockedViews[570])
				return getBumpNode(kCarRestaurant, 70, 255);

			return node;
		case 60:
			if (!_blockedViews[559])
				return getBumpNode(kCarRestaurant, 59, 255);

			if (!_blockedViews[561])
				return getBumpNode(kCarRestaurant, 61, 255);

			if (!_blockedViews[565])
				return getBumpNode(kCarRestaurant, 65, 255);

			if (!_blockedViews[570])
				return getBumpNode(kCarRestaurant, 70, 255);

			return node;
		case 62:
		case 63:
		case 64:
			if (!_blockedViews[561])
				return getBumpNode(kCarRestaurant, 61, 255);

			if (!_blockedViews[565])
				return getBumpNode(kCarRestaurant, 65, 255);

			if (!_blockedViews[570])
				return getBumpNode(kCarRestaurant, 70, 255);

			return node;
		case 66:
		case 67:
		case 68:
			if (!_blockedViews[565])
				return getBumpNode(kCarRestaurant, 65, 255);

			if (!_blockedViews[570])
				return getBumpNode(kCarRestaurant, 70, 255);

			return node;
		case 69:
		case 71:
			if (!_blockedViews[570])
				return getBumpNode(kCarRestaurant, 70, 255);

			return node;
		default:
			return node;
		}
	}

	return node;
}

void LogicManager::smartBumpCath() {
	if (!_engine->isDemo() || _engine->_navigationEngineIsRunning) {
		bumpCathNode(getSmartBumpNode(_activeNode));
	}
}

void LogicManager::bumpCathCloseUp(int item) {
	if (item < 32) {
		if (_items[item].closeUp) {
			if (!_closeUp) {
				_closeUp = 1;
				_nodeReturn = _activeNode;
			}

			bumpCathNode(_items[item].closeUp);
		}
	}
}

int LogicManager::playFight(int fightId) {
	if (!_engine->isDemo() || _engine->_navigationEngineIsRunning) {
		return _engine->doFight(fightId);
	} else {
		return 1;
	}
}

void LogicManager::playNIS(int nisId) {
	if (_engine->isDemo() && !_engine->_navigationEngineIsRunning)
		return;

	char filename[16];

	bool videoWithoutFade = false;

	if (nisId >= (int)kEventCorpseDropFloorOriginal ||
		nisId == kEventCathWakingUp ||
		nisId == kEventConcertCough ||
		nisId == kEventConcertSit ||
		nisId == kEventConcertLeaveWithBriefcase) {
		videoWithoutFade = true;
	}

	Common::sprintf_s(filename, "%s.NIS", _nisNames[nisId]);

	_engine->getSoundManager()->_scanAnySoundLoopingSection = true;

	restoreIcons();
	displayWaitIcon();

	if (!_engine->mouseHasRightClicked()) {
		if (_engine->_gracePeriodTimer && dialogRunning("TIMER")) {
			fadeDialog("TIMER");
			_engine->_gracePeriodTimer = 105;
		}

		_engine->getGraphicsManager()->_renderBox1.x = 80;
		_engine->getGraphicsManager()->_renderBox1.width = 480;

		if (_engine->isGoldEdition()) {
			warning("NIS support for Gold Edition is currently missing. Skipping cutscene");
		} else {
			if (videoWithoutFade) {
				_engine->getNISManager()->doNIS(filename, kNisFlagBaseFlag);
			} else {
				_engine->getNISManager()->doNIS(filename, kNisFlagBaseFlag | kNisFlagSoundFade);
			}
		}

		if (dialogRunning("TIMER"))
			endDialog("TIMER");
	}

	_engine->getGraphicsManager()->_renderBox1.width = 480;
	_engine->getGraphicsManager()->_renderBox1.height = 480;
	_engine->getGraphicsManager()->_renderBox1.x = 80;
	_engine->getGraphicsManager()->_renderBox1.y = 0;

	_doneNIS[nisId] = 1;

	_realTime += _nisTicks[nisId];
	_gameTime += _nisTicks[nisId] * _timeSpeed;
}

void LogicManager::cleanNIS() {
	if (_engine->isDemo() && !_engine->_navigationEngineIsRunning)
		return;

	if (_closeUp) {
		int largeItem = findLargeItem();
		_closeUp = 0;
		if (largeItem && _activeItem != largeItem) {
			_activeItem = largeItem;
			_engine->getGraphicsManager()->drawItem(_items[largeItem].mnum, 44, 0);
			_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
		}

		if (_blockedViews[100 * _trainData[_nodeReturn].nodePosition.car + _trainData[_nodeReturn].cathDir]) {
			bumpCathNode(getSmartBumpNode(_nodeReturn));
		} else {
			bumpCathNode(_nodeReturn);
		}
	} else {
		bumpCathNode(_activeNode);
	}
}

int LogicManager::getVolume(int character) {
	int chosenVolume;
	int relativePos;
	int positionVolumes[32];

	if (!character)
		return 16;

	positionVolumes[0] = 16;
	positionVolumes[1] = 15;
	positionVolumes[2] = 14;
	positionVolumes[3] = 13;
	positionVolumes[4] = 12;
	positionVolumes[5] = 11;
	positionVolumes[6] = 11;
	positionVolumes[7] = 10;
	positionVolumes[8] = 10;
	positionVolumes[9] = 9;
	positionVolumes[10] = 9;
	positionVolumes[11] = 8;
	positionVolumes[12] = 8;
	positionVolumes[13] = 7;
	positionVolumes[14] = 7;
	positionVolumes[15] = 7;
	positionVolumes[16] = 6;
	positionVolumes[17] = 6;
	positionVolumes[18] = 6;
	positionVolumes[19] = 5;
	positionVolumes[20] = 5;
	positionVolumes[21] = 5;
	positionVolumes[22] = 5;
	positionVolumes[23] = 4;
	positionVolumes[24] = 4;
	positionVolumes[25] = 4;
	positionVolumes[26] = 4;
	positionVolumes[27] = 3;
	positionVolumes[28] = 3;
	positionVolumes[29] = 3;
	positionVolumes[30] = 3;
	positionVolumes[31] = 3;

	if (getCharacter(character).characterPosition.car != getCharacter(kCharacterCath).characterPosition.car)
		return 0;

	chosenVolume = 2;
	relativePos = ABS<int16>((int16)getCharacter(character).characterPosition.position - (int16)getCharacter(kCharacterCath).characterPosition.position) / 230;
	if (relativePos < 32)
		chosenVolume = positionVolumes[relativePos];

	if (getCharacter(kCharacterCath).characterPosition.location != 2) {
		switch (getCharacter(kCharacterCath).characterPosition.car) {
		case kCarKronos:
			if (inOuterSanctum(character) != inOuterSanctum(kCharacterCath))
				chosenVolume >>= 1;
			break;
		case kCarGreenSleeping:
		case kCarRedSleeping:
			if (onLowPlatform(kCharacterCath) && !onLowPlatform(character))
				chosenVolume >>= 1;
			if (getCharacter(kCharacterCath).characterPosition.location == 1 && (getCharacter(character).characterPosition.location != 1 || !nearChar(kCharacterCath, character, 400))) {
				chosenVolume >>= 1;
			}
			break;
		case kCarRestaurant:
			if (inSalon(character) == inSalon(kCharacterCath)) {
				if (inDiningRoom(character) != inDiningRoom(kCharacterCath))
					chosenVolume >>= 1;
			} else {
				chosenVolume >>= 2;
			}
			break;
		default:
			return chosenVolume;
		}

		return chosenVolume;
	}

	if (getCharacter(character).characterPosition.car != kCarKronos && !cathOutHisWindow() && !cathOutRebeccaWindow())
		return 0;

	chosenVolume /= 6;
	return chosenVolume;
}

void LogicManager::queueSFX(int character, uint8 action, uint8 delay) {
	uint8 sndNum;
	int volume;
	int ambientLevel;

	byte sndArray[5];
	char filename[16];

	memset(sndArray, 0, sizeof(sndArray));

	if (getCharacter(character).characterPosition.car == getCharacter(kCharacterCath).characterPosition.car) {
		if (inSalon(character) == inSalon(kCharacterCath)) {
			sndNum = action;
			volume = getVolume(character);

			switch (action) {
			case 36:
				ambientLevel = 16;
				if (volume <= 9)
					ambientLevel = volume + 7;
				_engine->getSoundManager()->raiseAmbient(ambientLevel, delay);
				break;
			case 37:
				_engine->getSoundManager()->levelAmbient(delay);
				break;
			case 150:
				sndArray[0] = 151;
				sndArray[1] = 152;
				sndArray[2] = 153;
				sndArray[3] = 154;
				sndArray[4] = 155;
				sndNum = sndArray[rnd(5)];
				break;
			case 156:
				sndArray[0] = 157;
				sndArray[1] = 158;
				sndArray[2] = 159;
				sndArray[3] = 160;
				sndArray[4] = 161;
				sndNum = sndArray[rnd(5)];
				break;
			case 162:
				sndArray[0] = 163;
				sndArray[1] = 164;
				sndArray[2] = 165;
				sndArray[3] = 166;
				sndArray[4] = 167;
				sndNum = sndArray[rnd(5)];
				break;
			case 168:
				sndArray[0] = 169;
				sndArray[1] = 170;
				sndArray[2] = 171;
				sndArray[3] = 172;
				sndArray[4] = 173;
				sndNum = sndArray[rnd(5)];
				break;
			case 174:
				sndArray[0] = 175;
				sndArray[1] = 176;
				sndArray[2] = 177;
				sndNum = sndArray[rnd(3)];
				break;
			case 180:
				sndArray[0] = 181;
				sndArray[1] = 182;
				sndArray[2] = 183;
				sndArray[3] = 184;
				sndNum = sndArray[rnd(4)];
				break;
			case 184:
				sndArray[0] = 185;
				sndArray[1] = 186;
				sndArray[2] = 187;
				sndNum = sndArray[rnd(3)];
				break;
			case 188:
				sndArray[0] = 189;
				sndArray[1] = 190;
				sndArray[2] = 191;
				sndArray[3] = 192;
				sndArray[4] = 193;
				sndNum = sndArray[rnd(5)];
				break;
			case 194:
				sndArray[0] = 195;
				sndArray[1] = 196;
				sndArray[2] = 197;
				sndNum = sndArray[rnd(3)];
				break;
			case 198:
				sndArray[0] = 198;
				sndArray[1] = 200;
				sndArray[2] = 201;
				sndArray[3] = 202;
				sndArray[4] = 203;
				sndNum = sndArray[rnd(5)];
				break;
			case 246:
				sndArray[0] = 0;
				memcpy(&sndArray[1], "hint", 4);
				sndNum = sndArray[rnd(5)];
				break;
			case 247:
				sndArray[0] = 11;
				memcpy(&sndArray[1], "{|", 2);
				sndNum = sndArray[rnd(3)];
				break;
			case 248:
				sndArray[0] = 0;
				memcpy(&sndArray[1], "glm", 3);
				sndNum = sndArray[rnd(4)];
				break;
			case 249:
				sndArray[0] = 0;
				memcpy(&sndArray[1], "8pq", 3);
				sndNum = sndArray[rnd(4)];
				break;
			case 250:
				sndArray[0] = 0;
				memcpy(&sndArray[1], "ksu", 3);
				sndNum = sndArray[rnd(4)];
				break;
			case 251:
				sndArray[0] = 0;
				sndArray[1] = 11;
				memcpy(&sndArray[2], "8q", 2);
				sndNum = sndArray[rnd(4)];
				break;
			case 252:
				sndArray[0] = 0;
				sndArray[1] = 6;
				memcpy(&sndArray[2], "my", 2);
				sndNum = sndArray[rnd(4)];
				break;
			case 254:
				sndArray[0] = 0;
				memcpy(&sndArray[1], "hxy", 3);
				sndNum = sndArray[rnd(4)];
				break;
			case 255:
				sndArray[0] = 0;
				memcpy(&sndArray[1], "js", 2);
				sndNum = sndArray[rnd(3)];
				break;
			default:
				break;
			}

			if (sndNum) {
				Common::sprintf_s(filename, "LIB%03d.SND", sndNum);
				if (volume)
					_engine->getSoundManager()->playSoundFile(filename, volume, 0, delay);
			}
		}
	}
}

void LogicManager::playChrExcuseMe(int character, int receivingCharacter, int volume) {
	if (whoRunningDialog(character) &&
		character != kCharacterCath && character != kCharacterClerk &&
		(character != kCharacterMaster || receivingCharacter == kCharacterFrancois || receivingCharacter == kCharacterMax || character == kCharacterFrancois) &&
		getCharacter(kCharacterFrancois).walkStepSize != 30) {
		return;
	}

	if ((whoRunningDialog(character) &&
		character != kCharacterCath &&
		character != kCharacterClerk &&
		character != kCharacterMaster) ||
		receivingCharacter == kCharacterFrancois ||
		receivingCharacter == kCharacterMax ||
		(character == kCharacterFrancois && getCharacter(kCharacterFrancois).walkStepSize != 30)) {
		return;
	}

	if (!volume)
		volume = getVolume(character);

	switch (character) {
	case kCharacterAnna:
		playDialog(0, "ANN1107A", volume, 0);
		return;
	case kCharacterAugust:
		switch (rnd(4)) {
		case 0:
			playDialog(0, "AUG1100A", volume, 0);
			break;
		case 1:
			playDialog(0, "AUG1100B", volume, 0);
			break;
		case 2:
			playDialog(0, "AUG1100C", volume, 0);
			break;
		case 3:
		default:
			playDialog(0, "AUG1100D", volume, 0);
		}

		return;
	case kCharacterCond1:
		if (isFemale(receivingCharacter)) {
			switch (rnd(2)) {
			case 0:
				playDialog(0, "CON1111A", volume, 0);
				break;
			case 1:
			default:
				playDialog(0, "CON1111", volume, 0);
			}

			return;
		}

		if (receivingCharacter == kCharacterCath && _globals[kGlobalJacket] == 2 && rnd(2) != 0) {
			if (isNight()) {	
				if (_globals[kGlobalPhaseOfTheNight] != 2) {
					playDialog(0, "CON1110E", volume, 0);
				} else {
					playDialog(0, "CON1110F", volume, 0);
				}
			} else {
				playDialog(0, "CON1110D", volume, 0);
			}
		} else {
			switch (rnd(3)) {
			case 0:
				playDialog(0, "CON1110", volume, 0);
				break;
			case 1:
				playDialog(0, "CON1110A", volume, 0);
				break;
			case 2:
			default:
				playDialog(0, "CON1110C", volume, 0);
			}
		}

		return;
	case kCharacterCond2:
		if (isFemale(receivingCharacter)) {
			playDialog(0, "JAC1111D", volume, 0);
		} else if (!receivingCharacter && _globals[kGlobalJacket] == 2 && rnd(2)) {
			playDialog(0, "JAC1113B", volume, 0);
		} else {
			switch (rnd(4)) {
			case 0:
				playDialog(0, "JAC1111", volume, 0);
				break;
			case 1:
				playDialog(0, "JAC1111A", volume, 0);
				break;
			case 2:
				playDialog(0, "JAC1111B", volume, 0);
				break;
			case 3:
			default:
				playDialog(0, "JAC1111C", volume, 0);
			}
		}

		return;
	case kCharacterHeadWait:
		switch (rnd(2)) {
		case 0:
			playDialog(0, "HED1002", volume, 0);
			break;
		case 1:
		default:
			playDialog(0, "HED1002A", volume, 0);
		}

		return;
	case kCharacterWaiter1:
		switch (rnd(3)) {
		case 0:
			playDialog(0, "WAT1002", volume, 0);
			break;
		case 1:
			playDialog(0, "WAT1002A", volume, 0);
			break;
		case 2:
		default:
			playDialog(0, "WAT1002B", volume, 0);
		}

		return;
	case kCharacterWaiter2:
		switch (rnd(3)) {
		case 0:
			playDialog(0, "WAT1003", volume, 0);
			break;
		case 1:
			playDialog(0, "WAT1003A", volume, 0);
			break;
		case 2:
		default:
			playDialog(0, "WAT1003B", volume, 0);
		}

		return;
	case kCharacterTrainM:
		if (isFemale(receivingCharacter)) {
			switch (rnd(2)) {
			case 0:
				playDialog(0, "TRA1113A", volume, 0);
				break;
			case 1:
			default:
				playDialog(0, "TRA1113B", volume, 0);
			}
		} else {
			playDialog(0, "TRA1112", volume, 0);
		}

		return;
	case kCharacterTatiana:
		switch (rnd(2)) {
		case 0:
			playDialog(0, "TAT1102A", volume, 0);
			break;
		case 1:
		default:
			playDialog(0, "TAT1102B", volume, 0);
		}

		return;
	case kCharacterAlexei:
		switch (rnd(2)) {
		case 0:
			playDialog(0, "ALX1099C", volume, 0);
			break;
		case 1:
		default:
			playDialog(0, "ALX1099D", volume, 0);
		}

		return;
	case kCharacterAbbot:
		if (isFemale(receivingCharacter)) {
			playDialog(0, "ABB3002C", volume, 0);
		} else {
			switch (rnd(3)) {
			case 0:
				playDialog(0, "ABB3002", volume, 0);
				break;
			case 1:
				playDialog(0, "ABB3002A", volume, 0);
				break;
			case 2:
			default:
				playDialog(0, "ABB3002B", volume, 0);
			}
		}

		return;
	case kCharacterVesna:
		switch (rnd(3)) {
		case 0:
			playDialog(0, "VES1109A", volume, 0);
			break;
		case 1:
			playDialog(0, "VES1109B", volume, 0);
			break;
		case 2:
		default:
			playDialog(0, "VES1109C", volume, 0);
		}

		return;
	case kCharacterKahina:
		switch (rnd(2)) {
		case 0:
			playDialog(0, "KAH1001A", volume, 0);
			break;
		case 1:
		default:
			playDialog(0, "KAH1001", volume, 0);
		}

		return;
	case kCharacterFrancois:
		switch (rnd(4)) {
		case 0:
			playDialog(0, "FRA1001", volume, 0);
			break;
		case 1:
			playDialog(0, "FRA1001A", volume, 0);
			break;
		case 2:
			playDialog(0, "FRA1001B", volume, 0);
			break;
		case 3:
		default:
			playDialog(0, "FRA1001C", volume, 0);
		}

		return;
	case kCharacterMadame:
		switch (rnd(4)) {
		case 0:
			playDialog(0, "MME1103A", volume, 0);
			break;
		case 1:
			playDialog(0, "MME1103B", volume, 0);
			break;
		case 2:
			playDialog(0, "MME1103C", volume, 0);
			break;
		case 3:
		default:
			playDialog(0, "MME1103d", volume, 0);
		}

		return;
	case kCharacterMonsieur:
		playDialog(0, "MRB1104", volume, 0);

		if (volume > 2)
			_globals[kGlobalMetMonsieur] = 1;

		return;
	case kCharacterRebecca:
		switch (rnd(2)) {
		case 0:
			playDialog(0, "Reb1106A", volume, 0);
			break;
		case 1:
		default:
			playDialog(0, "REB1106", volume, 0);
		}

		return;
	case kCharacterSophie:
		if (isFemale(receivingCharacter)) {
			switch (rnd(3)) {
			case 0:
				playDialog(0, "SOP1105", volume, 0);
				break;
			case 1:
				playDialog(0, "SOP1105C", volume, 0);
				break;
			case 2:
			default:
				playDialog(0, "SOP1105D", volume, 0);
			}

			return;
		} else {
			switch (rnd(3)) {
			case 0:
				playDialog(0, "SOP1105", volume, 0);
				return;
			case 1:
				playDialog(0, "SOP1105A", volume, 0);
				break;
			case 2:
			default:
				playDialog(0, "SOP1105B", volume, 0);
				break;
			}
		}

		return;
	case kCharacterMahmud:
		playDialog(0, "MAH1101", volume, 0);
		return;
	case kCharacterYasmin:
		playDialog(0, "HAR1002", volume, 0);

		if (volume > 2)
			_globals[kGlobalMetYasmin] = 1;

		return;
	case kCharacterHadija:
		switch (rnd(2)) {
		case 0:
			playDialog(0, "HAR1001A", volume, 0);
			break;
		case 1:
		default:
			playDialog(0, "HAR1001", volume, 0);
		}

		if (volume > 2)
			_globals[kGlobalMetHadija] = 1;

		return;
	case kCharacterAlouan:
		playDialog(0, "HAR1004", volume, 0);
		return;
	default:
		return;
	}
}

void LogicManager::playCathExcuseMe() {
	switch (rnd(3)) {
	case 0:
		playDialog(kCharacterCath, "CAT1126B", -1, 0);
		break;
	case 1:
		playDialog(kCharacterCath, "CAT1126C", -1, 0);
		break;
	case 2:
	default:
		playDialog(kCharacterCath, "CAT1126D", -1, 0);
	}
}

const char *LogicManager::getCathJustChecking() {
	switch (rnd(4)) {
	case 0:
		return "CAT5001";
	case 1:
		return "CAT5001A";
	case 2:
		return "CAT5001B";
	case 3:
		return "CAT5001C";
	default:
		return "CAT5001";
	}
}

const char *LogicManager::getCathSorryDialog() {
	switch (rnd(5)) {
	case 0:
		return "CAT1125";
	case 1:
		return "CAT1125A";
	case 2:
		return "CAT1125B";
	case 3:
		return "CAT1125C";
	case 4:
		return "CAT1125D";
	default:
		return "CAT1125";
	}
}

const char *LogicManager::getCathWCDialog() {
	switch (rnd(3)) {
	case 0:
		return "CAT1520";
	case 1:
		return "CAT1521";
	case 2:
		return "CAT1125";
	default:
		return "CAT1520";
	}
}

void LogicManager::playCondYelling(int character, int situation) {
	if (character == kCharacterCond1) {
		switch (situation) {
		case 2:
			if (_lastTickCondYellingComp2 + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					switch (rnd(2)) {
					case 0:
						playDialog(kCharacterCond1, "Con1500C", 16, 0);
						break;
					case 1:
					default:
						playDialog(kCharacterCond1, "Con1500B", 16, 0);
						break;
					}
					break;
				case 1:
				default:
					playDialog(kCharacterCond1, "Con1502A", 16, 0);
					break;
				}
			} else {
				switch (rnd(2)) {
				case 0:
					switch (rnd(2)) {
					case 0:
						playDialog(kCharacterCond1, "Con1500A", 16, 0);
						break;
					case 1:
					default:
						playDialog(kCharacterCond1, "Con1500", 16, 0);
						break;
					}
					break;
				case 1:
				default:
					playDialog(kCharacterCond1, "Con1502", 16, 0);
					break;
				}
			}

			_lastTickCondYellingComp2 = _realTime;
			break;
		case 3:
			if (_lastTickCondYellingComp3 + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					switch (rnd(2)) {
					case 0:
						playDialog(kCharacterCond1, "Con1500C", 16, 0);
						break;
					case 1:
						playDialog(kCharacterCond1, "Con1500B", 16, 0);
						break;
					}
					break;
				case 1:
					playDialog(kCharacterCond1, "Con1501A", 16, 0);
					break;
				}
			} else {
				switch (rnd(2)) {
				case 0:
					switch (rnd(2)) {
					case 0:
						playDialog(kCharacterCond1, "Con1500A", 16, 0);
						break;
					case 1:
						playDialog(kCharacterCond1, "Con1500", 16, 0);
						break;
					}
					break;
				case 1:
					playDialog(kCharacterCond1, "Con1501", 16, 0);
					break;
				}
			}
			
			_lastTickCondYellingComp3 = _realTime;
			break;
		case 4:
			if (_lastTickCondYellingComp4 + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					switch (rnd(2)) {
					case 0:
						playDialog(kCharacterCond1, "Con1500C", 16, 0);
						break;
					case 1:
					default:
						playDialog(kCharacterCond1, "Con1500B", 16, 0);
						break;
					}
					break;
				case 1:
				default:
					playDialog(kCharacterCond1, "Con1503", 16, 0);
					break;
				}
			} else {
				switch (rnd(2)) {
				case 0:
					switch (rnd(2)) {
					case 0:
						playDialog(kCharacterCond1, "Con1500A", 16, 0);
						break;
					case 1:
					default:
						playDialog(kCharacterCond1, "Con1500", 16, 0);
						break;
					}
					break;
				case 1:
				default:
					playDialog(kCharacterCond1, "Con1503", 16, 0);
					break;
				}
			}
			
			_lastTickCondYellingComp4 = _realTime;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			_lastTickCondYellingCompHarem++;
			switch (_lastTickCondYellingCompHarem) {
			case 1:
				playDialog(kCharacterCond1, "Con1503C", 16, 0);
				break;
			case 2:
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond1, "Con1503A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond1, "Con1503E", 16, 0);
					break;
				}
				break;
			case 3:
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond1, "Con1503D", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond1, "Con1503B", 16, 0);
					break;
				}
				_lastTickCondYellingCompHarem = 0;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		return;
	}

	if (character == kCharacterCond2) {
		switch (situation) {
		case 32:
			if (_lastTickCondYellingCompA + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					break;
				}
			} else {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1508A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1508", 16, 0);
					break;
				}
			}

			_lastTickCondYellingCompA = _realTime;
			return;

		case 33:
			if (_lastTickCondYellingCompB + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					break;
				}
			} else if (_globals[kGlobalConcertIsHappening] || (_gameTime > 2101500 && _gameTime < 2133000)) {
				playDialog(kCharacterCond2, "Jac1507A", 16, 0);
			} else {
				playDialog(kCharacterCond2, "Jac1507", 16, 0);
			}

			_lastTickCondYellingCompB = _realTime;
			return;

		case 34:
			if (_lastTickCondYellingCompC + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					break;
				}
			} else {
				if (_globals[kGlobalChapter] < 3) {
					playDialog(kCharacterCond2, "Jac1506", 16, 0);
					_lastTickCondYellingCompC = _realTime;
					return;
				}

				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1506B", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1506A", 16, 0);
					break;
				}
			}

			_lastTickCondYellingCompC = _realTime;
			return;

		case 35:
			if (_lastTickCondYellingCompD + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					break;
				}
			} else {
				playDialog(kCharacterCond2, "Jac1505", 16, 0);
			}

			_lastTickCondYellingCompD = _realTime;
			return;

		case 36:
			if (_lastTickCondYellingCompE + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					_lastTickCondYellingCompE = _realTime;
					return;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					_lastTickCondYellingCompE = _realTime;
					return;
				}
			}

			if (_globals[kGlobalConcertIsHappening] || (_gameTime > 2115000 && _gameTime < 2133000)) {
				playDialog(kCharacterCond2, "Jac1504B", 16, 0);
				_lastTickCondYellingCompE = _realTime;
				return;
			}

			if (inComp(kCharacterRebecca, kCarRedSleeping, 4840)) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					_lastTickCondYellingCompE = _realTime;
					return;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					_lastTickCondYellingCompE = _realTime;
					return;
				}
			}

			switch (rnd(2)) {
			case 0:
				playDialog(kCharacterCond2, "Jac1504A", 16, 0);
				break;
			case 1:
			default:
				playDialog(kCharacterCond2, "Jac1504", 16, 0);
				break;
			}

			_lastTickCondYellingCompE = _realTime;
			return;

		case 37:
			if (_lastTickCondYellingCompF + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					break;
				}
			} else {
				if (_globals[kGlobalConcertIsHappening] || (_gameTime > 2083500 && _gameTime < 2133000)) {
					playDialog(kCharacterCond2, "Jac1503B", 16, 0);
					_lastTickCondYellingCompF = _realTime;
					return;
				}

				if (rnd(2) == 0 || inComp(kCharacterAnna, kCarRedSleeping, 4070)) {
					playDialog(kCharacterCond2, "Jac1503A", 16, 0);
				} else {
					playDialog(kCharacterCond2, "Jac1503", 16, 0);
				}
			}

			_lastTickCondYellingCompF = _realTime;
			return;

		case 38:
			if (_lastTickCondYellingCompG + 450 >= _realTime) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					break;
				}
			} else {
				if (rnd(2) == 1 || !inComp(kCharacterMilos, kCarRedSleeping, 3050)) {
					playDialog(kCharacterCond2, "Jac1502", 16, 0);
				} else {
					playDialog(kCharacterCond2, "Jac1502A", 16, 0);
				}
			}

			_lastTickCondYellingCompG = _realTime;
			return;

		case 39:
			if (_lastTickCondYellingCompH + 450 >= _realTime || inComp(kCharacterIvo, kCarRedSleeping, 2740)) {
				switch (rnd(2)) {
				case 0:
					playDialog(kCharacterCond2, "Jac1500A", 16, 0);
					break;
				case 1:
				default:
					playDialog(kCharacterCond2, "Jac1500", 16, 0);
					break;
				}
			} else {
				playDialog(kCharacterCond2, "Jac1501", 16, 0);
			}

			_lastTickCondYellingCompH = _realTime;
			return;

		default:
			return;
		}
	}
}

void LogicManager::playDialog(int character, const char *filename, int volume, int delay) {
	int soundFlags;
	char tmp[16];

	if (whoRunningDialog(character) && character && character < 32)
		endDialog(character);

	if (volume == -1) {
		soundFlags = _engine->getLogicManager()->getVolume(character);
	} else {
		soundFlags = volume;
	}

	if (strchr(filename, '.')) {
		Common::strcpy_s(tmp, filename);
	} else {
		Common::sprintf_s(tmp, "%s.SND", filename);
	}

	if (volume != -1)
		soundFlags |= kSoundFlagFixedVolume;

	if (!_engine->getSoundManager()->playSoundFile(tmp, soundFlags, character, delay)) {
		if (character)
			_engine->getLogicManager()->send(kCharacterCath, character, 2, 0);
	}
}

bool LogicManager::whoRunningDialog(int character) {
	return _engine->getSoundManager()->findSlotWho(character) != nullptr;
}

bool LogicManager::dialogRunning(const char *name) {
	char tmp[32];

	if (strchr(name, '.')) {
		Common::strcpy_s(tmp, name);
	} else {
		Common::sprintf_s(tmp, "%s.SND", name);
	}

	return _engine->getSoundManager()->findSlotName(tmp) != nullptr;
}

bool LogicManager::cathRunningDialog(const char *name) {
	char tmp[16];

	if (strchr(name, '.')) {
		Common::strcpy_s(tmp, name);
	} else {
		Common::sprintf_s(tmp, "%s.SND", name);
	}

	Slot *slot = _engine->getSoundManager()->findSlotName(tmp);
	return slot && slot->getAssociatedCharacter() == kCharacterCath;
}

void LogicManager::fadeDialog(uint32 character) {
	Slot *slot = _engine->getSoundManager()->findSlotWho(character);

	if (slot) {
		slot->setFade(0);
		slot->setAssociatedCharacter(0);
	}
}

void LogicManager::fadeDialog(const char *name) {
	char tmp[16];

	if (strchr(name, '.')) {
		Common::strcpy_s(tmp, name);
	} else {
		Common::sprintf_s(tmp, "%s.SND", name);
	}

	Slot *slot = _engine->getSoundManager()->findSlotName(tmp);

	if (slot) {
		slot->setFade(0);
		slot->setAssociatedCharacter(0);
	}
}

void LogicManager::endDialog(uint32 character) {
	Slot *slot = _engine->getSoundManager()->findSlotWho(character);

	if (slot) {
		slot->addStatusFlag(kSoundFlagCloseRequested);
		slot->setAssociatedCharacter(0);

		slot->closeArchive();
	}
}

void LogicManager::endDialog(const char *name) {
	char tmp[16];

	if (strchr(name, '.')) {
		Common::strcpy_s(tmp, name);
	} else {
		Common::sprintf_s(tmp, "%s.SND", name);
	}

	Slot *slot = _engine->getSoundManager()->findSlotName(tmp);

	if (slot) {
		slot->addStatusFlag(kSoundFlagCloseRequested);
		slot->setAssociatedCharacter(0);

		slot->closeArchive();
	}
}

void LogicManager::claimNISLink(int character) {
	Slot *slot = _engine->getSoundManager()->_soundCache;

	if (slot) {
		do {
			if (slot->hasTag(kSoundTagLink))
				break;

			slot = slot->getNext();
		} while (slot);

		if (slot)
			slot->setAssociatedCharacter(character);
	}
}

int32 LogicManager::getSoundTicks(int character) {
	Slot *slot = _engine->getSoundManager()->findSlotWho(character);

	if (slot) {
		return slot->getTime() >> 1;
	} else {
		return 0;
	}
}

int LogicManager::whoseBit(int character) {
	int result = 0;

	if (character && (character & 1) == 0) {
		do {
			character >>= 1;
			result++;
		} while ((character & 1) == 0);
	}

	return result;
}

bool LogicManager::cathWillSeeOtis(int position) {
	int param1;
	int param2;

	switch (position) {
	case 1500:
		param1 = 1;
		param2 = 23;
		break;
	case 2740:
		param1 = 3;
		param2 = 25;
		break;
	case 3050:
		param1 = 5;
		param2 = 26;
		break;
	case 4070:
		param1 = 7;
		param2 = 28;
		break;
	case 4840:
		param1 = 9;
		param2 = 30;
		break;
	case 5790:
		param1 = 11;
		param2 = 32;
		break;
	case 6470:
		param1 = 13;
		param2 = 34;
		break;
	case 7500:
		param1 = 15;
		param2 = 36;
		break;
	case 8200:
		param1 = 17;
		param2 = 38;
		break;
	default:
		return true;
	}

	return (_engine->getOtisManager()->fDirection(0) && _engine->_fudgePosition[param1] >= getCharacter(kCharacterCath).characterPosition.position) ||
		   (_engine->getOtisManager()->rDirection(0) && _engine->_fudgePosition[param2] <= getCharacter(kCharacterCath).characterPosition.position);
}

bool LogicManager::walk(int character, int car, int position) {
	int seqDirection;
	int excuseMeCharacter;
	uint32 characterPosition;
	bool canTeleportToTarget = false;
	bool moveCharacterByOffset = false;
	bool walkCounterUpdated = false;
	int offset = 0;
	
	uint32 cathPos = getCharacter(kCharacterCath).characterPosition.position;

	if (position == 2000 && cathFacingUp() && !checkCathDir(kCarGreenSleeping, 1) && !checkCathDir(kCarRedSleeping, 2))
		position = 1500;

	if (getCharacter(character).direction != 1 && getCharacter(character).direction != 2)
		getCharacter(character).walkCounter = 0;

	if (getCharacter(character).walkCounter) {
		getCharacter(character).walkCounter--;

		if (getCharacter(character).walkCounter == 0x80)
			getCharacter(character).walkCounter = 0;

		if ((getCharacter(character).walkCounter & 0x7F) != 8) {
			getCharacter(character).waitedTicksUntilCycleRestart = 0;
			return false;
		}

		walkCounterUpdated = true;

		if ((getCharacter(character).walkCounter & 0x80) != 0)
			moveCharacterByOffset = true;
	}

	if (car == getCharacter(character).characterPosition.car) {
		if (ABS<int>(getCharacter(character).characterPosition.position - position) >= 100) {
			if (position > 850 && position < 9270 && ABS<int>(getCharacter(character).characterPosition.position - position) < 300)
				canTeleportToTarget = true;
		} else {
			canTeleportToTarget = true;
		}

		if (!canTeleportToTarget &&
			((_engine->getOtisManager()->fDirection(0) && getCharacter(character).direction == 1) ||
				(_engine->getOtisManager()->rDirection(0) && getCharacter(character).direction == 2)) &&
			!cathWillSeeOtis(position) && nearChar(character, kCharacterCath, 250)) {
			canTeleportToTarget = true;
		}

		if (canTeleportToTarget && whoOnScreen(character) && whoFacingCath(character) && !cathWillSeeOtis(position)) {
			canTeleportToTarget = false;
			if (getCharacter(character).direction == 1)
				position = getCharacter(kCharacterCath).characterPosition.position + 250;
			else
				position = getCharacter(kCharacterCath).characterPosition.position - 250;
		}

		if (canTeleportToTarget) {
			getCharacter(character).characterPosition.position = position;

			if (getCharacter(character).direction == 1 || getCharacter(character).direction == 2)
				getCharacter(character).direction = 0;

			getCharacter(character).attachedConductor = 0;

			return true;
		}
	}

	if (car <= getCharacter(character).characterPosition.car) {
		if (car >= getCharacter(character).characterPosition.car) {
			if (position <= getCharacter(character).characterPosition.position)
				seqDirection = 2;
			else
				seqDirection = 1;
		} else {
			seqDirection = 2;
		}
	} else {
		seqDirection = 1;
	}

	if (seqDirection == getCharacter(character).direction && !walkCounterUpdated) {
		if (!nearX(character, 1500, 750) || character == 20) {
			if (getCharacter(character).attachedConductor) {
				send(character, getCharacter(character).attachedConductor, 16, 0);
				getCharacter(character).attachedConductor = 0;
			}
		} else if (!getCharacter(character).attachedConductor) {
			if (getCharacter(character).direction != 1 || (position <= 2000 && car == getCharacter(character).characterPosition.car)) {
				if (getCharacter(character).direction == 2 &&
					(position < 1500 || car != getCharacter(character).characterPosition.car) &&
					getCharacter(character).characterPosition.position > 1500) {
					if (getCharacter(character).characterPosition.car == kCarGreenSleeping) {
						send(character, kCharacterCond1, 11, 0);
						getCharacter(character).attachedConductor = kCharacterCond1;
					} else if (getCharacter(character).characterPosition.car == kCarRedSleeping) {
						send(character, kCharacterCond2, 11, 0);
						getCharacter(character).attachedConductor = kCharacterCond2;
					}
				}
			} else if (getCharacter(character).characterPosition.position < 1500) {
				if (getCharacter(character).characterPosition.car == kCarGreenSleeping) {
					send(character, kCharacterCond1, 11, 1);
					getCharacter(character).attachedConductor = kCharacterCond1;
				} else if (getCharacter(character).characterPosition.car == kCarRedSleeping) {
					send(character, kCharacterCond2, 11, 1);
					getCharacter(character).attachedConductor = kCharacterCond2;
				}
			}
		}

		if (whoOnScreen(character)) {
			if (getCharacter(character).needsPosFudge) {

				if (getCharacter(character).characterPosition.car == kCarGreenSleeping) {
					offset = 0;
				} else if (getCharacter(character).characterPosition.car == kCarRedSleeping) {
					offset = 8;
				}

				for (int i = 0; i < 8; i++) {
					if ((_blockedX[offset + i] || _softBlockedX[offset + i]) && nearX(character, _doorPositions[i], 750) &&
						cathWillSeeOtis(_doorPositions[i]) && ((getCharacter(character).direction == 1 && getCharacter(character).characterPosition.position < _doorPositions[i] &&
							(car != getCharacter(character).characterPosition.car || position > _doorPositions[i])) ||
							(getCharacter(character).direction == 2 && (getCharacter(character).characterPosition.position > _doorPositions[i]) &&
								(car != getCharacter(character).characterPosition.car || position < _doorPositions[i])))) {
						if (_blockedX[offset + i]) {
							excuseMeCharacter = whoseBit(_blockedX[offset + i]);
						} else {
							excuseMeCharacter = whoseBit(_softBlockedX[offset + i]);
						}

						playChrExcuseMe(character, excuseMeCharacter, 0);
						getCharacter(character).walkCounter = 0x80 | 0x10;

						break;
					}
				}

				for (int j = 1; j < 40; j++) {
					if (_engine->getMessageManager()->getMessageHandle(j) && whoOnScreen(j) &&
						j != character && nearChar(character, j, 750) && whoWalking(j) &&
						(character != kCharacterRebecca || j != kCharacterSophie) && (character != kCharacterSophie || j != kCharacterRebecca) &&
						(character != kCharacterIvo     || j != kCharacterSalko)  && (character != kCharacterSalko  || j != kCharacterIvo) &&
						(character != kCharacterMilos   || j != kCharacterVesna)  && (character != kCharacterVesna  || j != kCharacterMilos)) {
						if (getCharacter(character).direction == getCharacter(j).direction) {
							if (ABS<int>(getCharacter(character).characterPosition.position - getCharacter(kCharacterCath).characterPosition.position) > ABS<int>(getCharacter(j).characterPosition.position - getCharacter(kCharacterCath).characterPosition.position)) {
								if (!whoFacingCath(character)) {
									if (seqDirection == 1) {
										if (getCharacter(character).characterPosition.position < 9500)
											getCharacter(character).characterPosition.position += 500;
									} else if (getCharacter(character).characterPosition.position > 500) {
										getCharacter(character).characterPosition.position -= 500;
									}

									_engine->getOtisManager()->startSeq(character, seqDirection, 1);
									return false;
								}

								getCharacter(character).waitedTicksUntilCycleRestart = 0;
								break;
							}
						} else if ((getCharacter(character).direction == 1 && getCharacter(character).characterPosition.position < getCharacter(j).characterPosition.position) ||
								   (getCharacter(character).direction == 2 && (getCharacter(character).characterPosition.position > getCharacter(j).characterPosition.position))) {
							getCharacter(character).waitedTicksUntilCycleRestart = 0;
							getCharacter(j).waitedTicksUntilCycleRestart = 0;
							getCharacter(character).walkCounter = 0x10;
							getCharacter(j).walkCounter = 0x10;
							playChrExcuseMe(character, j, 0);
							playChrExcuseMe(j, character, 0);

							if (j > character)
								getCharacter(j).walkCounter++;

							break;
						}
					}
				}
			}

			return false;
		}

		if (getCharacter(character).direction == 1) {
			if (getCharacter(character).characterPosition.position < 10000 - (uint16)getCharacter(character).walkStepSize)
				getCharacter(character).characterPosition.position += getCharacter(character).walkStepSize;
		} else if (getCharacter(character).characterPosition.position > (uint16)getCharacter(character).walkStepSize) {
			getCharacter(character).characterPosition.position -= getCharacter(character).walkStepSize;
		}

		characterPosition = getCharacter(character).characterPosition.position;
		if (characterPosition > 9270 && getCharacter(character).direction == 1) {
			if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car) {
				queueSFX(character, 36, 0);
				queueSFX(character, 37, 30);
			}

			getCharacter(character).characterPosition.car++;
			getCharacter(character).characterPosition.position = 851;

			if (getCharacter(character).characterPosition.car == kCarGreenSleeping && onLowPlatform(kCharacterCath)) {
				queueSFX(kCharacterCath, 14, 0);
				playChrExcuseMe(character, kCarGreenSleeping, 16);
				bumpCath(kCarGreenSleeping, 1, 255);
				playDialog(kCharacterCath, "CAT1127A" , -1, 0);
				queueSFX(kCharacterCath, 15, 0);
			}

			if (getCharacter(character).characterPosition.car > car ||
				(getCharacter(character).characterPosition.car == car && getCharacter(character).characterPosition.position >= position)) {
				getCharacter(character).characterPosition.car = car;
				getCharacter(character).characterPosition.position = position;
				getCharacter(character).direction = 0;
				getCharacter(character).attachedConductor = 0;
				return true;
			}

			if (getCharacter(character).characterPosition.car == kCarGreenSleeping && onKronPlatform(0)) {
				queueSFX(kCharacterCath, 14, 0);
				playChrExcuseMe(character, 0, 16);
				bumpCath(kCarGreenSleeping, 62, 255);
				playDialog(kCharacterCath, "CAT1127A" , -1, 0);
				queueSFX(kCharacterCath, 15, 0);
			}

			if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car) {
				queueSFX(character, 36, 0);
				queueSFX(character, 37, 30);
			}

		} else if (characterPosition < 850 && getCharacter(character).direction == 2) {
			if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car) {
				queueSFX(character, 36, 0);
				queueSFX(character, 37, 30);
			}

			getCharacter(character).characterPosition.car--;
			getCharacter(character).characterPosition.position = 9269;

			if (getCharacter(character).characterPosition.car == kCarKronos && onLowPlatform(kCharacterCath)) {
				queueSFX(kCharacterCath, 14, 0);
				playChrExcuseMe(character, kCharacterCath, 16);
				bumpCath(kCarGreenSleeping, 1, 255);
				playDialog(kCharacterCath, "CAT1127A", -1, 0);
				queueSFX(kCharacterCath, 15, 0);
			}

			if (getCharacter(character).characterPosition.car < car ||
				(getCharacter(character).characterPosition.car == car && getCharacter(character).characterPosition.position <= position)) {
				getCharacter(character).characterPosition.car = car;
				getCharacter(character).characterPosition.position = position;
				getCharacter(character).direction = 0;
				getCharacter(character).attachedConductor = 0;
				return true;
			}

			if (getCharacter(character).characterPosition.car == kCarKronos && onKronPlatform(kCharacterCath)) {
				queueSFX(kCharacterCath, 14, 0);
				playChrExcuseMe(character, 0, 16);
				bumpCath(kCarGreenSleeping, 62, 255);
				playDialog(kCharacterCath, "CAT1127A", -1, 0);
				queueSFX(kCharacterCath, 15, 0);
			}

			if (getCharacter(kCharacterCath).characterPosition.car == getCharacter(character).characterPosition.car) {
				queueSFX(character, 36, 0);
				queueSFX(character, 37, 30);
			}
		}

		if (getCharacter(kCharacterCath).characterPosition.car != getCharacter(character).characterPosition.car || getCharacter(kCharacterCath).characterPosition.location)
			return false;

		if (getCharacter(character).direction == 1) {
			if (cathPos <= characterPosition || cathPos - characterPosition < 500 || cathPos - characterPosition >= (unsigned int)(uint16)getCharacter(character).walkStepSize + 500) {
				return false;
			}

			if (_engine->getOtisManager()->fDirection(0) || _engine->getOtisManager()->doorView()) {
				send(kCharacterCath, character, 6, 0);

				if (_engine->getOtisManager()->doorView())
					bumpCathFDoor(_trainData[_activeNode].parameter1);

				return false;
			}

			if (!_engine->getOtisManager()->rDirection(0))
				return false;
		} else {
			if (cathPos >= characterPosition || characterPosition - cathPos < 500 || (int32)(characterPosition - cathPos) >= getCharacter(character).walkStepSize + 500) {
				return false;
			}

			if (!_engine->getOtisManager()->fDirection(0)) {
				if (_engine->getOtisManager()->rDirection(0) || _engine->getOtisManager()->doorView()) {
					send(kCharacterCath, character, 6, 0);

					if (_engine->getOtisManager()->doorView())
						bumpCathRDoor(_trainData[_activeNode].parameter1);
				}

				return false;
			}
		}

		send(kCharacterCath, character, 5, 0);
		return false;
	}

	if (walkCounterUpdated) {
		if (seqDirection == 1) {
			if (moveCharacterByOffset) {
				if (getCharacter(character).characterPosition.position < 8800)
					getCharacter(character).characterPosition.position += 1200;
			} else if (getCharacter(character).characterPosition.position < 9250) {
				getCharacter(character).characterPosition.position += 750;
			}

			if (car == getCharacter(character).characterPosition.car && getCharacter(character).characterPosition.position >= position) {
				getCharacter(character).characterPosition.position = position;
				getCharacter(character).direction = 0;
				getCharacter(character).attachedConductor = 0;
				return true;
			}
		} else {
			if (moveCharacterByOffset) {
				if (getCharacter(character).characterPosition.position > 1200)
					getCharacter(character).characterPosition.position -= 1200;
			} else if (getCharacter(character).characterPosition.position > 750) {
				getCharacter(character).characterPosition.position -= 750;
			}

			if (car == getCharacter(character).characterPosition.car && getCharacter(character).characterPosition.position <= position) {
				getCharacter(character).characterPosition.position = position;
				getCharacter(character).direction = 0;
				getCharacter(character).attachedConductor = 0;
				return true;
			}
		}
	}

	_engine->getOtisManager()->startSeq(character, seqDirection, 1);
	return false;
}

void LogicManager::startCycOtis(int character, const char *filename) {
	Common::String filenameStr(filename);
	filenameStr.toUppercase();

	Common::strcpy_s(getCharacter(character).sequenceNamePrefix, filenameStr.c_str());
	Common::strcat_s(getCharacter(character).sequenceNamePrefix, "-");

	getCharacter(character).elapsedFrames = 0;
	getCharacter(character).waitedTicksUntilCycleRestart = 0;
	getCharacter(character).currentFrameSeq1 = 0;

	_engine->getOtisManager()->startSeq(character, 3, true);
}

void LogicManager::startSeqOtis(int character, const char *filename) {
	Common::String filenameStr(filename);
	filenameStr.toUppercase();

	Common::strcpy_s(getCharacter(character).sequenceNamePrefix, filenameStr.c_str());
	Common::strcat_s(getCharacter(character).sequenceNamePrefix, "-");

	getCharacter(character).elapsedFrames = 0;
	getCharacter(character).waitedTicksUntilCycleRestart = 0;
	getCharacter(character).currentFrameSeq1 = 0;

	_engine->getOtisManager()->startSeq(character, 4, true);
}

void LogicManager::advanceFrame(int character) {
	Seq *sequence;
	int16 *frameCountPtr;
	int16 elapsedFrames;

	bool found = false;

	if (getCharacter(character).direction == 5) {
		sequence = getCharacter(character).sequence2;
		frameCountPtr = &(getCharacter(character).currentFrameSeq2);
	} else {
		sequence = getCharacter(character).sequence1;
		frameCountPtr = &(getCharacter(character).currentFrameSeq1);
	}

	if (sequence) {
		int16 oldFrameCount = *frameCountPtr;
		if (sequence->numFrames > *frameCountPtr) {
			elapsedFrames = getCharacter(character).elapsedFrames;
			do {
				if ((sequence->numFrames - 1) <= *frameCountPtr)
					break;

				if ((sequence->sprites[*frameCountPtr].flags & 8) != 0) {
					found = true;
				} else {
					if (sequence->sprites[*frameCountPtr].soundAction == 35)
						found = true;

					getCharacter(character).elapsedFrames += sequence->sprites[*frameCountPtr].ticksToWaitUntilCycleRestart;
					(*frameCountPtr)++;
				}
			} while (!found);

			if (!found) {
				*frameCountPtr = oldFrameCount;
				getCharacter(character).elapsedFrames = elapsedFrames;
			}
		}
	}
}

void LogicManager::endGraphics(int character) {
	_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame1, false);
	_engine->getSpriteManager()->destroySprite(&getCharacter(character).frame2, false);

	if (getCharacter(character).sequence2) {
		_engine->getMemoryManager()->freeMem(getCharacter(character).sequence2->rawSeqData);
		delete getCharacter(character).sequence2;
		getCharacter(character).sequence2 = nullptr;
		Common::strcpy_s(getCharacter(character).sequenceName2, "");
		getCharacter(character).needsSecondaryPosFudge = 0;
		getCharacter(character).directionSwitch = 0;
	}

	if (getCharacter(character).sequence1) {
		_engine->getMemoryManager()->freeMem(getCharacter(character).sequence1->rawSeqData);
		delete getCharacter(character).sequence1;
		getCharacter(character).sequence1 = nullptr;
		Common::strcpy_s(getCharacter(character).sequenceName, "");
		getCharacter(character).needsPosFudge = 0;
		getCharacter(character).currentFrameSeq1 = -1;
	}

	Common::strcpy_s(getCharacter(character).sequenceNamePrefix, "");
	getCharacter(character).direction = 0;
	getCharacter(character).doProcessEntity = 1;
}

void LogicManager::makeAllJump(int chapter) {
	if (chapter) {
		for (int i = 0; i < 40; i++) {
			getCharacter(i).currentCall = 0;
			getCharacter(i).inventoryItem = 0;
			endGraphics(i);
		}

		for (int i = 0; i < 1000; i++) {
			_blockedViews[i] = 0;
		}

		for (int i = 0; i < 16; i++) {
			_blockedX[i] = 0;
			_softBlockedX[i] = 0;
		}

		// Originally the first edition of the game checked only for kSoundTagMenu;
		// I added kSoundTagLink since otherwise the music coda playing after
		// the very first cutscene is immediately killed by this function, which
		// is also happening in the original. But the latter gets the news of the
		// sound getting killed a bit later, so we manage to hear it anyway; not
		// here though...
		//
		// Incidentally this is what the demo and subsequent releases do.
		if (chapter == 1 || chapter == 3) {
			_engine->getSoundManager()->killAllExcept(kSoundTagMenu, kSoundTagLink, 0, 0, 0, 0, 0);
		} else {
			_engine->getSoundManager()->killAllExcept(kSoundTagMenu, 0, 0, 0, 0, 0, 0);
		}
	}

	if (_engine->isDemo()) {
		CONS_DemoAnna(chapter);
		CONS_DemoAugust(chapter);
		CONS_DemoCond2(chapter);
		CONS_DemoWaiter1(chapter);
		CONS_DemoTatiana(chapter);
		CONS_DemoAbbot(chapter);
		CONS_DemoVesna(chapter);
		CONS_DemoIvo(chapter);
		CONS_DemoRebecca(chapter);
		CONS_DemoFrancois(chapter);
		CONS_DemoMadame(chapter);
		CONS_DemoMonsieur(chapter);

		if (chapter < 2)
			CONS_DemoMaster(chapter);

		CONS_DemoTableA(chapter);
		CONS_DemoTableB(chapter);
		CONS_DemoTableC(chapter);
		CONS_DemoTableD(chapter);
		CONS_DemoTableE(chapter);
		CONS_DemoTableF(chapter);
	} else {
		CONS_Anna(chapter);
		CONS_August(chapter);
		CONS_Cond1(chapter);
		CONS_Cond2(chapter);
		CONS_HeadWait(chapter);
		CONS_Waiter1(chapter);
		CONS_Waiter2(chapter);
		CONS_Cook(chapter);
		CONS_TrainM(chapter);
		CONS_Tatiana(chapter);
		CONS_Vassili(chapter);
		CONS_Alexei(chapter);
		CONS_Abbot(chapter);
		CONS_Milos(chapter);
		CONS_Vesna(chapter);
		CONS_Ivo(chapter);
		CONS_Salko(chapter);
		CONS_Rebecca(chapter);
		CONS_Sophie(chapter);
		CONS_Francois(chapter);
		CONS_Madame(chapter);
		CONS_Monsieur(chapter);
		CONS_Kronos(chapter);
		CONS_Kahina(chapter);
		CONS_Mahmud(chapter);
		CONS_Yasmin(chapter);
		CONS_Hadija(chapter);
		CONS_Alouan(chapter);
		CONS_Police(chapter);
		CONS_Max(chapter);

		if (chapter < 2)
			CONS_Master(chapter);

		CONS_Clerk(chapter);
		CONS_TableA(chapter);
		CONS_TableB(chapter);
		CONS_TableC(chapter);
		CONS_TableD(chapter);
		CONS_TableE(chapter);
		CONS_TableF(chapter);
		CONS_Mitchell(chapter);
	}
}

void LogicManager::CONS_All(bool isFirstChapter, int character) {
	makeAllJump(isFirstChapter ? 1 : 0);

	_flagBumpCathNode = false;

	if (!isFirstChapter && character) {
		fedEx(kCharacterCath, character, 0, 0);
	}

	if (!_flagBumpCathNode)
		bumpCathNode(_activeNode);

	// Uncomment this to activate "debug walks" functions
	// CONS_Master_WalkTest(0, 0, 0, 0);
}

int32 LogicManager::getGameTime() {
	return _gameTime;
}

int32 LogicManager::getRealTime() {
	return _realTime;
}

int32 LogicManager::getTimeSpeed() {
	return _timeSpeed;
}

} // End of namespace LastExpress
