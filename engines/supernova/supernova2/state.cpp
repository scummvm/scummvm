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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/text-to-speech.h"
#include "graphics/cursorman.h"
#include "gui/message.h"

#include "supernova/screen.h"
#include "supernova/supernova.h"
#include "supernova/supernova2/state.h"
#include "supernova/supernova2/stringid.h"

namespace Supernova {

bool GameManager2::serialize(Common::WriteStream *out) {
	if (out->err())
		return false;

	// GameState
	out->writeSint16LE(_state._money);
	out->writeSint32LE(_state._startTime - g_system->getMillis());
	out->writeByte(_state._addressKnown);
	out->writeByte(_state._poleMagnet);
	out->writeByte(_state._admission);
	out->writeByte(_state._tipsy);
	out->writeByte(_state._dark);
	out->writeByte(_state._elevatorE);
	out->writeByte(_state._elevatorNumber);
	out->writeByte(_state._toMuseum);
	out->writeSint16LE(_state._pyraE);
	out->writeByte(_state._pyraS);
	out->writeByte(_state._pyraZ);
	out->writeByte(_state._alarmOn);
	out->writeByte(_state._alarmCracked);
	out->writeByte(_state._haste);
	out->writeByte(_state._pressureCounter);
	out->writeByte(_state._sirenOn);
	out->writeSint16LE(_state._pyraDirection);
	out->writeUint32LE(_state._eventTime - g_system->getMillis());
	out->writeSint32LE(_state._eventCallback);
	out->writeByte(_state._taxiPossibility);
	for (int i = 0; i < 15; i++) {
		out->writeSint16LE(_state._puzzleTab[i]);
	}

	// Inventory
	out->writeSint32LE(_inventory.getSize());
	out->writeSint32LE(_inventoryScroll);
	for (int i = 0; i < _inventory.getSize(); ++i) {
		Object *objectStateBegin = _rooms[_inventory.get(i)->_roomId]->getObject(0);
		byte objectIndex = _inventory.get(i) - objectStateBegin;
		out->writeSint32LE(_inventory.get(i)->_roomId);
		out->writeSint32LE(objectIndex);
	}

	// Rooms
	out->writeByte(_lastRoom->getId());
	out->writeByte(_currentRoom->getId());
	for (int i = 0; i < NUMROOMS2; ++i) {
		_rooms[i]->serialize(out);
	}

	return !out->err();
}


bool GameManager2::deserialize(Common::ReadStream *in, int version) {
	if (in->err())
		return false;

	// GameState
	_state._money = in->readSint16LE();
	_state._startTime = in->readSint32LE() + g_system->getMillis();
	_state._addressKnown = in->readByte();
	_state._poleMagnet = in->readByte();
	_state._admission = in->readByte();
	_state._tipsy = in->readByte();
	_state._dark = in->readByte();
	_state._elevatorE = in->readByte();
	_state._elevatorNumber = in->readByte();
	_state._toMuseum = in->readByte();
	_state._pyraE = in->readSint16LE();
	_state._pyraS = in->readByte();
	_state._pyraZ = in->readByte();
	_state._alarmOn = in->readByte();
	_state._alarmCracked = in->readByte();
	_state._haste = in->readByte();
	_state._pressureCounter = in->readByte();
	_state._sirenOn = in->readByte();
	_state._pyraDirection = in->readSint16LE();
	_state._eventTime = in->readUint32LE() + g_system->getMillis();
	_state._eventCallback = (EventFunction)in->readSint32LE();
	_state._taxiPossibility = in->readByte();
	for (int i = 0; i < 15; i++)
		_state._puzzleTab[i] = in->readSint16LE();
	_vm->setGameString(kStringMoney, Common::String::format("%d Xa", _state._money));

	_oldTime = g_system->getMillis();

	// Inventory
	int inventorySize = in->readSint32LE();
	_inventoryScroll = in->readSint32LE();
	_inventory.clear();
	for (int i = 0; i < inventorySize; ++i) {
		RoomId objectRoom = static_cast<RoomId>(in->readSint32LE());
		int objectIndex = in->readSint32LE();
		_inventory.add(*_rooms[objectRoom]->getObject(objectIndex));
	}

	// Rooms
	RoomId lastRoomId = static_cast<RoomId>(in->readByte());
	RoomId curRoomId = static_cast<RoomId>(in->readByte());
	for (int i = 0; i < NUMROOMS2; ++i) {
		_rooms[i]->deserialize(in, version);
	}
	_lastRoom = _rooms[lastRoomId];
	changeRoom(curRoomId);

	// Some additional variables
	_state._previousRoom = _rooms[INTRO2];
	_guiEnabled = true;
	_animationEnabled = true;

	return !in->err();
}

GameManager2::GameManager2(SupernovaEngine *vm, Sound *sound)
	: GameManager(vm, sound) {
	initRooms();
	changeRoom(INTRO2);
	initState();
}

GameManager2::~GameManager2() {
	destroyRooms();
}

void GameManager2::destroyRooms() {
	delete _rooms[INTRO2];
	delete _rooms[AIRPORT];
	delete _rooms[TAXISTAND];
	delete _rooms[STREET];
	delete _rooms[GAMES];
	delete _rooms[CABIN2];
	delete _rooms[KIOSK];
	delete _rooms[CULTURE_PALACE];
	delete _rooms[CHECKOUT];
	delete _rooms[CITY1];
	delete _rooms[CITY2];
	delete _rooms[ELEVATOR2];
	delete _rooms[APARTMENT];
	delete _rooms[SHIP];
	delete _rooms[PYRAMID];
	delete _rooms[PYR_ENTRANCE];
	delete _rooms[UPSTAIRS1];
	delete _rooms[DOWNSTAIRS1];
	delete _rooms[BOTTOM_RIGHT_DOOR];
	delete _rooms[BOTTOM_LEFT_DOOR];
	delete _rooms[UPSTAIRS2];
	delete _rooms[DOWNSTAIRS2];
	delete _rooms[UPPER_DOOR];
	delete _rooms[PUZZLE_FRONT];
	delete _rooms[PUZZLE_BEHIND];
	delete _rooms[FORMULA1_F];
	delete _rooms[FORMULA1_N];
	delete _rooms[FORMULA2_F];
	delete _rooms[FORMULA2_N];
	delete _rooms[TOMATO_F];
	delete _rooms[TOMATO_N];
	delete _rooms[MONSTER_F];
	delete _rooms[MONSTER1_N];
	delete _rooms[MONSTER2_N];
	delete _rooms[UPSTAIRS3];
	delete _rooms[DOWNSTAIRS3];
	delete _rooms[LCORRIDOR1];
	delete _rooms[LCORRIDOR2];
	delete _rooms[HOLE_ROOM];
	delete _rooms[IN_HOLE];
	delete _rooms[FLOORDOOR];
	delete _rooms[FLOORDOOR_U];
	delete _rooms[BST_DOOR];
	delete _rooms[HALL2];
	delete _rooms[COFFIN_ROOM];
	delete _rooms[MASK];
	delete _rooms[MUSEUM];
	delete _rooms[MUS_ENTRANCE];
	delete _rooms[MUS1];
	delete _rooms[MUS2];
	delete _rooms[MUS3];
	delete _rooms[MUS4];
	delete _rooms[MUS5];
	delete _rooms[MUS6];
	delete _rooms[MUS7];
	delete _rooms[MUS8];
	delete _rooms[MUS9];
	delete _rooms[MUS10];
	delete _rooms[MUS11];
	delete _rooms[MUS_ROUND];
	delete _rooms[MUS12];
	delete _rooms[MUS13];
	delete _rooms[MUS14];
	delete _rooms[MUS15];
	delete _rooms[MUS16];
	delete _rooms[MUS17];
	delete _rooms[MUS18];
	delete _rooms[MUS19];
	delete _rooms[MUS20];
	delete _rooms[MUS21];
	delete _rooms[MUS22];
}

void GameManager2::initState() {
	GameManager::initState();
	_mapOn = false;
	_steps = false;
	_cracking = false;
	_alarmBefore = false;
	RoomId startSecurityTab[10] = {MUS6, MUS7, MUS11, MUS10, MUS3, MUS2, MUS1, MUS8, MUS9, MUS5};
	for (int i = 0; i < 10; i++)
		_securityTab[i] = startSecurityTab[i];

	_state._money = 20;
	_state._startTime = 0;
	_state._addressKnown = false;
	_state._previousRoom = _currentRoom;
	_lastRoom = _rooms[INTRO2];
	_state._poleMagnet = false;
	_state._admission = 0;
	_state._tipsy = false;
	_state._dark = false;
	_state._elevatorE = 0;
	_state._elevatorNumber = 0;
	_state._toMuseum = false;
	_state._pyraE = 0;
	_state._pyraS = 4;
	_state._pyraZ = 10;
	_state._alarmOn = false;
	_state._alarmCracked = false;
	_state._haste = false;
	_state._pressureCounter = 0;
	_state._sirenOn = false;
	_state._pyraDirection = 0;
	_state._eventTime = kMaxTimerValue;
	_state._eventCallback = kNoFn;
	_state._taxiPossibility = 4;
	int16 startPuzzleTab[15] = {12, 3, 14, 1, 11, 0, 2, 13, 9, 5, 4, 10, 7, 6, 8};
	for (int i = 0; i < 15; i++)
		_state._puzzleTab[i] = startPuzzleTab[i];

	for (int i = 0 ; i < 6 ; ++i) {
		_dials[i] = 1;
	}
}

void GameManager2::initRooms() {
	_rooms = new Room *[NUMROOMS2];
	_rooms[INTRO2] = new Intro2(_vm, this);
	_rooms[AIRPORT] = new Airport(_vm, this);
	_rooms[TAXISTAND] = new TaxiStand(_vm, this);
	_rooms[STREET] = new Street(_vm, this);
	_rooms[GAMES] = new Games(_vm, this);
	_rooms[CABIN2] = new Cabin2(_vm, this);
	_rooms[KIOSK] = new Kiosk(_vm, this);
	_rooms[CULTURE_PALACE] = new CulturePalace(_vm, this);
	_rooms[CHECKOUT] = new Checkout(_vm, this);
	_rooms[CITY1] = new City1(_vm, this);
	_rooms[CITY2] = new City2(_vm, this);
	_rooms[ELEVATOR2] = new Elevator2(_vm, this);
	_rooms[APARTMENT] = new Apartment(_vm, this);
	_rooms[SHIP] = new Ship(_vm, this);
	_rooms[PYRAMID] = new Pyramid(_vm, this);
	_rooms[PYR_ENTRANCE] = new PyrEntrance(_vm, this);
	_rooms[UPSTAIRS1] = new Upstairs1(_vm, this);
	_rooms[DOWNSTAIRS1] = new Downstairs1(_vm, this);
	_rooms[BOTTOM_RIGHT_DOOR] = new BottomRightDoor(_vm, this);
	_rooms[BOTTOM_LEFT_DOOR] = new BottomLeftDoor(_vm, this);
	_rooms[UPSTAIRS2] = new Upstairs2(_vm, this);
	_rooms[DOWNSTAIRS2] = new Downstairs2(_vm, this);
	_rooms[UPPER_DOOR] = new UpperDoor(_vm, this);
	_rooms[PUZZLE_FRONT] = new PuzzleFront(_vm, this);
	_rooms[PUZZLE_BEHIND] = new PuzzleBehind(_vm, this);
	_rooms[FORMULA1_F] = new Formula1F(_vm, this);
	_rooms[FORMULA1_N] = new Formula1N(_vm, this);
	_rooms[FORMULA2_F] = new Formula2F(_vm, this);
	_rooms[FORMULA2_N] = new Formula2N(_vm, this);
	_rooms[TOMATO_F] = new TomatoF(_vm, this);
	_rooms[TOMATO_N] = new TomatoN(_vm, this);
	_rooms[MONSTER_F] = new MonsterF(_vm, this);
	_rooms[MONSTER1_N] = new Monster1N(_vm, this);
	_rooms[MONSTER2_N] = new Monster2N(_vm, this);
	_rooms[UPSTAIRS3] = new Upstairs3(_vm, this);
	_rooms[DOWNSTAIRS3] = new Downstairs3(_vm, this);
	_rooms[LCORRIDOR1] = new LCorridor1(_vm, this);
	_rooms[LCORRIDOR2] = new LCorridor2(_vm, this);
	_rooms[HOLE_ROOM] = new HoleRoom(_vm, this);
	_rooms[IN_HOLE] = new InHole(_vm, this);
	_rooms[FLOORDOOR] = new Floordoor(_vm, this);
	_rooms[FLOORDOOR_U] = new FloordoorU(_vm, this);
	_rooms[BST_DOOR] = new BstDoor(_vm, this);
	_rooms[HALL2] = new Hall2(_vm, this);
	_rooms[COFFIN_ROOM] = new CoffinRoom(_vm, this);
	_rooms[MASK] = new Mask(_vm, this);
	_rooms[MUSEUM] = new Museum(_vm, this);
	_rooms[MUS_ENTRANCE] = new MusEntrance(_vm, this);
	_rooms[MUS1] = new Mus1(_vm, this);
	_rooms[MUS2] = new Mus2(_vm, this);
	_rooms[MUS3] = new Mus3(_vm, this);
	_rooms[MUS4] = new Mus4(_vm, this);
	_rooms[MUS5] = new Mus5(_vm, this);
	_rooms[MUS6] = new Mus6(_vm, this);
	_rooms[MUS7] = new Mus7(_vm, this);
	_rooms[MUS8] = new Mus8(_vm, this);
	_rooms[MUS9] = new Mus9(_vm, this);
	_rooms[MUS10] = new Mus10(_vm, this);
	_rooms[MUS11] = new Mus11(_vm, this);
	_rooms[MUS_ROUND] = new MusRound(_vm, this);
	_rooms[MUS12] = new Mus12(_vm, this);
	_rooms[MUS13] = new Mus13(_vm, this);
	_rooms[MUS14] = new Mus14(_vm, this);
	_rooms[MUS15] = new Mus15(_vm, this);
	_rooms[MUS16] = new Mus16(_vm, this);
	_rooms[MUS17] = new Mus17(_vm, this);
	_rooms[MUS18] = new Mus18(_vm, this);
	_rooms[MUS19] = new Mus19(_vm, this);
	_rooms[MUS20] = new Mus20(_vm, this);
	_rooms[MUS21] = new Mus21(_vm, this);
	_rooms[MUS22] = new Mus22(_vm, this);
}

bool GameManager2::canSaveGameStateCurrently() {
	return _animationEnabled && _guiEnabled && !_state._haste;
}

void GameManager2::updateEvents() {
	handleTime();
	if (_animationEnabled && !_vm->_screen->isMessageShown() && _animationTimer == 0)
		_currentRoom->animation();

	if (_state._eventCallback != kNoFn && g_system->getMillis() >= _state._eventTime) {
		_vm->_allowLoadGame = false;
		_vm->_allowSaveGame = false;
		_state._eventTime = kMaxTimerValue;
		EventFunction fn = _state._eventCallback;
		_state._eventCallback = kNoFn;
		switch (fn) {
		case kNoFn:
			break;
		case kSoberFn:
			sober();
			break;
		case kPyramidEndFn:
			pyramidEnd();
			break;
		case kCaughtFn:
			caught();
			break;
		default: //shouldn't happen
			break;
		}
		_vm->_allowLoadGame = true;
		_vm->_allowSaveGame = true;
		return;
	}

	_mouseClicked = false;
	_keyPressed = false;
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyPressed = true;
			processInput(event.kbd);
			break;
		case Common::EVENT_LBUTTONUP:
			// fallthrough
		case Common::EVENT_RBUTTONUP:
			_mouseClicked = true;
			// fallthrough
		case Common::EVENT_MOUSEMOVE:
			_mouseClickType = event.type;
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			if (_guiEnabled)
				processInput();
			break;
		default:
			break;
		}
	}
}

void GameManager2::handleTime() {
	if (_timerPaused)
		return;
	int32 newTime = g_system->getMillis();
	int32 delta = newTime - _oldTime;
	_time += delta;

	if (_animationTimer > delta)
		_animationTimer -= delta;
	else
		_animationTimer = 0;

	_oldTime = newTime;
}

void GameManager2::drawMapExits() {
	_vm->renderBox(281, 161, 39, 39, kColorWhite25);

	if ((_currentRoom->getId() >= PYR_ENTRANCE && _currentRoom->getId() <= HOLE_ROOM) ||
		(_currentRoom->getId() >= FLOORDOOR && _currentRoom->getId() <= BST_DOOR))
		compass();
	else {
		for (int i = 0; i < 25; i++) {
			int idx;
			if ((idx = _exitList[i]) != -1) {
				byte r = _currentRoom->getObject(idx)->_direction;
				int x = 284 + 7 * (r % 5);
				int y = 164 + 7 * (r / 5);
				_vm->renderBox(x, y, 5, 5, kColorDarkRed);
			}
		}
	}
}

void GameManager2::takeMoney(int amount) {
	_state._money += amount;
	_vm->setGameString(kStringMoney, Common::String::format("%d Xa", _state._money));
}

bool GameManager2::genericInteract(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_OPEN && obj1._id == WALLET) {
		if (_rooms[TAXISTAND]->getObject(4)->_type & CARRIED)
			_vm->renderMessage(kStringEmpty);
		else {
			_vm->renderMessage(kStringWalletOpen);
			takeObject(*_rooms[TAXISTAND]->getObject(4));
			takeObject(*_rooms[TAXISTAND]->getObject(5));
			takeMoney(1);
			_vm->playSound(kAudioSuccess2);
		}
	} else if (verb == ACTION_PRESS && obj1._id == TRANSMITTER) {
		if (_currentRoom == _rooms[TAXISTAND]) {
			if (_currentRoom->getObject(0)->_type != EXIT) {
				_vm->renderImage(5);
				wait(3);
				_vm->renderImage(6);
				_vm->playSound(kAudioTaxiOpen);
				_currentRoom->getObject(0)->_type = EXIT;
				drawMapExits();
			}
		} else if (_currentRoom == _rooms[STREET]	      ||
				   _currentRoom == _rooms[CULTURE_PALACE] ||
				   _currentRoom == _rooms[CITY1]          ||
				   _currentRoom == _rooms[CITY2]) {
			Common::String t = _vm->getGameString(kStringTaxiArrives);
			_vm->renderMessage(t);
			wait((t.size() + 20) * _vm->_textSpeed / 10, true);
			_vm->removeMessage();
			taxi();
		} else
			_vm->renderMessage(kStringNothingHappens);
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, ROD, MAGNET)) {
		Object *o1, *o2;
		if (obj2._id == ROD) {
			o1 = &obj2;
			o2 = &obj1;
		} else {
			o1 = &obj1;
			o2 = &obj2;
		}
		if (!(o1->_type & CARRIED))
			return false;

		if (!(o2->_type & CARRIED))
			takeObject(*o2);

		_vm->renderMessage(kStringAttachMagnet);
		o1->_name = kStringPoleMagnet;
		o1->_description = kStringCunning;
		_inventory.remove(*o2);
		_rooms[APARTMENT]->setSectionVisible(kMaxSection - 1, kShownTrue);
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, CHIP, PLAYER)) {
		Object *o1, *o2;
		if (obj2._id == CHIP) {
			o1 = &obj2;
			o2 = &obj1;
		} else {
			o1 = &obj1;
			o2 = &obj2;
		}
		if (!(o2->_type & CARRIED))
			_vm->renderMessage(kStringMustBuyFirst);
		else {
			if (!(o1->_type & CARRIED))
			{
				_vm->renderImage(1);
				_vm->renderImage(2 + kSectionInvert);
				_currentRoom->getObject(0)->_click = 255;
			} else
				_inventory.remove(*o1);

			_vm->renderMessage(kStringInsertChip);
			if (_state._admission)
				_state._admission = 2;
			else
				_state._admission = 1;
		}
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, DISCMAN, PLAYER)) {
		switch (_state._admission) {
		case 1:
			// fall through
		case 2:
			_vm->renderMessage(kStringTransferCD);
			_state._admission = 2;
			_vm->playSound(kAudioSuccess2);
			break;
		default:
			_vm->renderMessage(kStringCDNotInserted);
		}
	} else if (verb == ACTION_OPEN && obj1._id == PLAYER) {
		switch (_state._admission) {
		case 1:
			_state._admission = 0;
			playerTakeOut();
			break;
		case 2:
			_state._admission = 3;
			playerTakeOut();
			break;
		default:
			_vm->renderMessage(kStringChipNotInserted);
		}
	} else if (verb == ACTION_OPEN && obj1._id == DISCMAN) {
		_vm->renderMessage(kStringWhatFor);
	} else if (verb == ACTION_PRESS && obj1._id == DISCMAN) {
		_vm->renderMessage(kStringMMCD);
		playCD();
	} else if (verb == ACTION_PRESS && obj1._id == PLAYER) {
		switch (_state._admission) {
		case 1:
			_vm->renderMessage(kStringChipEmpty);
			break;
		case 2:
			_vm->renderMessage(kStringListeningToCD);
			playCD();
			break;
		default:
			_vm->renderMessage(kStringNoChip);
		}
	} else if ((verb == ACTION_OPEN || verb == ACTION_USE) &&
			   obj1._id == BOTTLE && (obj1._type & CARRIED)) {
		_vm->renderMessage(kStringTipsy);
		_state._tipsy = true;
		_state._eventTime = g_system->getMillis() + 60000;
		_state._eventCallback = kSoberFn;
	} else if (verb == ACTION_LOOK && obj1._id == MUSCARD) {
		_vm->setCurrentImage(30);
		_vm->renderImage(0);
		//karte_an = true
		wait(100000, true);
		//karte_an = false
		_vm->removeMessage();
		_vm->renderRoom(*_currentRoom);
		drawGUI();
	} else
		return false;
	return true;
}

void GameManager2::playCD() {
	CursorMan.showMouse(false);
	_vm->playSound(kMusicMadMonkeys);
	Common::KeyCode k = Common::KEYCODE_INVALID;
	while(_vm->_sound->isPlaying())
		if (waitOnInput(1, k))
			break;
	_vm->_sound->stop();
	_vm->removeMessage();
	CursorMan.showMouse(true);
}

void GameManager2::handleInput() {
	bool validCommand = genericInteract(_inputVerb, *_inputObject[0], *_inputObject[1]);
	if (!validCommand)
		validCommand = _currentRoom->interact(_inputVerb, *_inputObject[0], *_inputObject[1]);
	if (!validCommand) {
		switch (_inputVerb) {
		case ACTION_LOOK:
			_vm->renderMessage(_inputObject[0]->_description);
			break;

		case ACTION_WALK:
			if (_inputObject[0]->hasProperty(CARRIED)) {
				// You already carry this.
				_vm->renderMessage(kStringGenericInteract1);
			} else if (!_inputObject[0]->hasProperty(EXIT)) {
				// You're already there.
				_vm->renderMessage(kStringGenericInteract2);
			} else if (_inputObject[0]->hasProperty(OPENABLE) && !_inputObject[0]->hasProperty(OPENED)) {
				// This is closed
				_vm->renderMessage(kStringGenericInteract3);
			} else {
				_lastRoom = _currentRoom;
				changeRoom(_inputObject[0]->_exitRoom);
			}

			break;

		case ACTION_TAKE:
			if (_inputObject[0]->hasProperty(OPENED)) {
				// You already have that
				_vm->renderMessage(kStringGenericInteract4);
			} else if (_inputObject[0]->hasProperty(UNNECESSARY)) {
				// You do not need that.
				_vm->renderMessage(kStringGenericInteract5);
			} else if (!_inputObject[0]->hasProperty(TAKE)) {
				// You can't take that.
				_vm->renderMessage(kStringGenericInteract6);
			} else
				takeObject(*_inputObject[0]);

			break;

		case ACTION_OPEN:
			if (!_inputObject[0]->hasProperty(OPENABLE)) {
				// This can't be opened
				_vm->renderMessage(kStringGenericInteract7);
			} else if (_inputObject[0]->hasProperty(OPENED)) {
				// This is already opened.
				_vm->renderMessage(kStringGenericInteract8);
			} else if (_inputObject[0]->hasProperty(CLOSED)) {
				// This is locked.
				_vm->renderMessage(kStringGenericInteract9);
			} else {
				_vm->renderImage(_inputObject[0]->_section);
				_inputObject[0]->setProperty(OPENED);
				byte i = _inputObject[0]->_click;
				_inputObject[0]->_click  = _inputObject[0]->_click2;
				_inputObject[0]->_click2 = i;
				_vm->_sound->play(kAudioTaxiOpen);
			}
			break;

		case ACTION_CLOSE:
			if (!_inputObject[0]->hasProperty(OPENABLE) ||
				(_inputObject[0]->hasProperty(CLOSED) &&
				 _inputObject[0]->hasProperty(OPENED))) {
				// This can't be closed.
				_vm->renderMessage(kStringGenericInteract10);
			} else if (!_inputObject[0]->hasProperty(OPENED)) {
				// This is already closed.
				_vm->renderMessage(kStringGenericInteract11);
			} else {
				_vm->renderImage(invertSection(_inputObject[0]->_section));
				_inputObject[0]->disableProperty(OPENED);
				byte i = _inputObject[0]->_click;
				_inputObject[0]->_click  = _inputObject[0]->_click2;
				_inputObject[0]->_click2 = i;
				_vm->_sound->play(kAudioElevator1);
			}
			break;

		case ACTION_GIVE:
			if (_inputObject[0]->hasProperty(CARRIED)) {
				// Better keep it!
				_vm->renderMessage(kStringGenericInteract12);
			}
			break;

		default:
			// This is not possible.
			_vm->renderMessage(kStringGenericInteract13);
		}
	}
}

void GameManager2::executeRoom() {
	if (_currentRoom == _rooms[PUZZLE_FRONT])
		puzzleConstruction();
	if (_state._sirenOn && !_vm->_sound->isPlaying())
		_vm->_sound->playSiren();
	if (_processInput && !_vm->_screen->isMessageShown() && _guiEnabled) {
		handleInput();
		if (_dead) {
			_dead = false;
			return;
		}
		if (_mouseClicked) {
			Common::Event event;
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse = Common::Point(0, 0);
			_vm->getEventManager()->pushEvent(event);
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse = Common::Point(_mouseX, _mouseY);
			_vm->getEventManager()->pushEvent(event);
		}

		resetInputState();
	}

	if (_guiEnabled) {
		if (!_vm->_screen->isMessageShown()) {
			g_system->fillScreen(kColorBlack);
			_vm->renderRoom(*_currentRoom);
		}
		if (_currentRoom->getId() < MUSEUM)
			drawMapExits();
		else
			drawClock();
		if (_dead) {
			_dead = false;
			return;
		}
		drawInventory();
		drawStatus();
		drawCommandBox();
	}

	if (_vm->_screen->getViewportBrightness() == 0)
		_vm->paletteFadeIn();

	if (!_currentRoom->hasSeen() && _newRoom) {
		_newRoom = false;
		_currentRoom->onEntrance();
	}
}

void GameManager2::leaveTaxi() {
	_currentRoom = _state._previousRoom;
	_vm->renderRoom(*_currentRoom);
	_guiEnabled = true;
}

void GameManager2::taxiUnknownDestination() {
	_vm->renderImage(invertSection(2));
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->renderImage(4);
	wait(_vm->_textSpeed * 3, true);
	_vm->renderImage(invertSection(4));
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->renderImage(2);
}

void GameManager2::taxiPayment(int price, int destination) {
	static int answers[] = {
		kStringPay,
		kStringLeaveTaxi
	};
	if (dialog(2, _dials, answers, 0)) {
		leaveTaxi();
	} else if (_state._money < price) {
		Common::String t = _vm->getGameString(kStringNotEnoughMoney);
		_vm->renderMessage(t);
		wait((t.size() + 20) * _vm->_textSpeed / 10, true);
		_vm->removeMessage();
		leaveTaxi();
	} else {
		takeMoney(-price);
		_vm->renderImage(invertSection(5));
		_vm->renderImage(invertSection(6));
		_vm->renderImage(0);
		_vm->renderImage(1);
		_vm->renderImage(3);

		Common::String t = _vm->getGameString(kStringTaxiAccelerating);
		_vm->renderMessage(t);
		_vm->playSound(kAudioTaxiLeaving);
		while(_vm->_sound->isPlaying())
			wait(1);
		wait((t.size() + 20) * _vm->_textSpeed / 10, true);
		_vm->removeMessage();

		_vm->paletteFadeOut();
		_vm->_system->fillScreen(kColorBlack);
		_vm->_screen->setViewportBrightness(255);
		_vm->_screen->setGuiBrightness(255);
		_vm->paletteBrightness();

		Common::String t2 = _vm->getGameString(kString5MinutesLater);
		_vm->renderMessage(t2);
		_vm->playSound(kAudioTaxiArriving);
		while(_vm->_sound->isPlaying())
			wait(1);
		wait((t2.size() + 20) * _vm->_textSpeed / 10, true);
		_vm->removeMessage();
		_vm->paletteFadeOut();

		switch (destination) {
		case 0:
			changeRoom(TAXISTAND);
			break;
		case 1:
			changeRoom(STREET);
			break;
		case 2:
			changeRoom(CULTURE_PALACE);
			break;
		case 10:
			changeRoom(CITY1);
			break;
		case 11:
			changeRoom(CITY2);
			break;
		default:
			break;
		}
	}
}

void GameManager2::taxi() {
	_vm->_allowSaveGame = false;
	static int dest[] = {
		kStringAirport,
		kStringDowntown,
		kStringCulturePalace,
		kStringEarth,
		kStringPrivateApartment,
		kStringLeaveTaxi
	};
	Common::String input;
	int possibility = _state._taxiPossibility;

	_state._previousRoom = _currentRoom;
	_currentRoom = _rooms[INTRO2];
	_vm->setCurrentImage(4);
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->renderImage(2);

	if (_state._previousRoom == _rooms[TAXISTAND]) possibility += 1;
	else if (_state._previousRoom == _rooms[STREET]) possibility += 2;
	else if (_state._previousRoom == _rooms[CULTURE_PALACE]) possibility += 4;
	int answer;
	do {
		_currentRoom->removeSentenceByMask(possibility, 1);
		switch (answer = dialog(6, _dials, dest, 1)) {
		case 3:
			_state._taxiPossibility += 8;
			possibility += 8;
			taxiUnknownDestination();
			break;
		case 5:
			leaveTaxi();
			break;
		case 4:
			_vm->renderMessage(kStringAddress);
			do {
				edit(input, 101, 70, 18);
			} while ((_key.keycode != Common::KEYCODE_RETURN) && (_key.keycode != Common::KEYCODE_ESCAPE) && !_vm->shouldQuit());

			_vm->removeMessage();
			if (_key.keycode == Common::KEYCODE_ESCAPE) {
				leaveTaxi();
				break;
			}
			input.toUppercase();
			if (input == "115AY2,96A" || input == "115AY2,96B")
				answer = 10;
			else if (input == "341,105A" || input == "341,105B") {
				if (_state._addressKnown)
					answer = 11;
				else {
					Common::String t = _vm->getGameString(kStringCheater);
					_vm->renderMessage(t);
					wait((t.size() + 20) * _vm->_textSpeed / 10, true);
					_vm->removeMessage();

					leaveTaxi();
					break;
				}
			} else {
				answer = 3;
				input = "";
				taxiUnknownDestination();
				break;
			}
			_vm->renderImage(invertSection(2));
			_vm->renderImage(0);
			_vm->renderImage(1);
			_vm->renderImage(6);
			_vm->playSound(kAudioSuccess2);
			taxiPayment(14, answer);
			break;
		default:
			_vm->renderImage(invertSection(2));
			_vm->renderImage(0);
			_vm->renderImage(1);
			_vm->renderImage(5);
			taxiPayment(8, answer);
			break;
		}
	_rooms[INTRO2]->addAllSentences(1);
	} while(answer == 3 && !_vm->shouldQuit());
	_vm->_allowSaveGame = true;

}

void GameManager2::playerTakeOut() {
	_vm->renderMessage(kStringRemoveChip);
	Object *o = _rooms[APARTMENT]->getObject(0);
	o->_section = 0;
	takeObject(*o);
}

void GameManager2::sober() {
	_state._tipsy = false;
}

bool GameManager2::talk(int mod1, int mod2, int rest, MessagePosition pos, int id) {
	Common::KeyCode key = Common::KEYCODE_INVALID;
	const Common::String& text = _vm->getGameString(id);

	Common::TextToSpeechManager *ttsMan = nullptr;
	if (ConfMan.getBool("tts_enabled"))
		ttsMan = g_system->getTextToSpeechManager();

	// Wait for the end of the current speech
	if (ttsMan && ttsMan->isSpeaking())
		wait(0, true, ttsMan);

	_vm->renderMessage(text, pos);
	int animation_count = (text.size() + 20) * (10 - rest) * _vm->_textSpeed / 400;
	_restTime =  (text.size() + 20) * rest * _vm->_textSpeed / 400;

	// We only wait for TTS below if there is no rest time
	if (_restTime)
		ttsMan = nullptr;

	while (animation_count || (ttsMan && ttsMan->isSpeaking())) {
		if (mod1)
			_vm->renderImage(mod1);

		if (waitOnInput(2, key)) {
			_vm->removeMessage();
			if (ttsMan)
				ttsMan->stop();
			return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
		}
		if (mod2)
			_vm->renderImage(mod2);

		if (waitOnInput(2, key)) {
			_vm->removeMessage();
			if (ttsMan)
				ttsMan->stop();
			return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
		}
		if (animation_count)
			animation_count--;
	}
	if (_restTime == 0)
		_vm->removeMessage();

	return true;
}

bool GameManager2::talkRest(int mod1, int mod2, int rest) {
	Common::TextToSpeechManager *ttsMan = nullptr;
	if (ConfMan.getBool("tts_enabled"))
		ttsMan = g_system->getTextToSpeechManager();

	Common::KeyCode key = Common::KEYCODE_INVALID;
	while (rest || (ttsMan && ttsMan->isSpeaking())) {
		_vm->renderImage(mod1);
		if (waitOnInput(2, key)) {
			_vm->removeMessage();
			if (ttsMan)
				ttsMan->stop();
			return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
		}
		_vm->renderImage(mod2);
		if (waitOnInput(2, key)) {
			_vm->removeMessage();
			if (ttsMan)
				ttsMan->stop();
			return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
		}
		if (rest)
			rest--;
	}
	return true;
}

void GameManager2::pyramidEnd() {
	_vm->renderMessage(kStringPyramid0);
	wait(_messageDuration, true);
	_vm->removeMessage();
	_vm->paletteFadeOut();
	_vm->loadGame(kSleepAutosaveSlot);
	changeRoom(CABIN2);
	drawGUI();
	_rooms[CABIN2]->setSectionVisible(kMaxSection - 1, kShownFalse);
	_rooms[CABIN2]->setSectionVisible(kMaxSection - 2, kShownTrue);
	_rooms[CABIN2]->setSectionVisible(1, kShownFalse);
}

void GameManager2::passageConstruction() {
	static ConstructionEntry constructionTab[9] = {
		{0, 4, 10, 2, 13},
		{0, 4, 9,  2, 14},
		{0, 4, 8,  3,  2},
		{1, 4, 7,  3,  1},
		{1, 5, 7,  3,  3},
		{1, 6, 7,  3,  5},
		{1, 4, 7,  1,  2},
		{1, 2, 5,  1,  1},
		{0, 4, 9,  2, 20}
	};

	changeRoom(PYR_ENTRANCE);
	_rooms[PYR_ENTRANCE]->setSectionVisible(1,
			!wall(_state._pyraS, _state._pyraZ, _state._pyraDirection, 0, -1));
	_rooms[PYR_ENTRANCE]->setSectionVisible(2,
			!wall(_state._pyraS, _state._pyraZ, _state._pyraDirection, 0,  1));
	_rooms[PYR_ENTRANCE]->setSectionVisible(7,
			wall(_state._pyraS, _state._pyraZ, _state._pyraDirection, 1,  0));

	if (!_rooms[PYR_ENTRANCE]->isSectionVisible(7)) {
		_rooms[PYR_ENTRANCE]->getObject(3)->_type = EXIT;
		_rooms[PYR_ENTRANCE]->getObject(3)->_click = 0;
		_rooms[PYR_ENTRANCE]->setSectionVisible(3,
				!wall(_state._pyraS, _state._pyraZ, _state._pyraDirection, 1, -1));
		_rooms[PYR_ENTRANCE]->setSectionVisible(4,
				!wall(_state._pyraS, _state._pyraZ, _state._pyraDirection, 1,  1));
		_rooms[PYR_ENTRANCE]->setSectionVisible(8,
				wall(_state._pyraS, _state._pyraZ, _state._pyraDirection, 2,  0));
		if (!_rooms[PYR_ENTRANCE]->isSectionVisible(8)) {
			_rooms[PYR_ENTRANCE]->setSectionVisible(5,
				   !wall(_state._pyraS, _state._pyraZ, _state._pyraDirection, 2, -1));
			_rooms[PYR_ENTRANCE]->setSectionVisible(6,
				   !wall(_state._pyraS, _state._pyraZ, _state._pyraDirection, 2,  1));
		} else {
			_rooms[PYR_ENTRANCE]->setSectionVisible(5, kShownFalse);
			_rooms[PYR_ENTRANCE]->setSectionVisible(6, kShownFalse);
		}
	} else {
		_rooms[PYR_ENTRANCE]->getObject(3)->_type = NULLTYPE;
		_rooms[PYR_ENTRANCE]->getObject(3)->_click = 255;
		_rooms[PYR_ENTRANCE]->setSectionVisible(3, kShownFalse);
		_rooms[PYR_ENTRANCE]->setSectionVisible(4, kShownFalse);
		_rooms[PYR_ENTRANCE]->setSectionVisible(8, kShownFalse);
	}
	for (int i = 0; i < 9; i++) {
		bool b = (_state._pyraE == constructionTab[i]._e &&
			 _state._pyraS == constructionTab[i]._s &&
			 _state._pyraZ == constructionTab[i]._z &&
			 _state._pyraDirection == constructionTab[i]._r);
		if (constructionTab[i]._a > 12)
			_rooms[PYR_ENTRANCE]->setSectionVisible(constructionTab[i]._a, b);
		else if (b)
			_rooms[PYR_ENTRANCE]->setSectionVisible(constructionTab[i]._a, kShownTrue);
	}

	_rooms[PYR_ENTRANCE]->setSectionVisible(18, kShownFalse);
	_rooms[PYR_ENTRANCE]->setSectionVisible(19, kShownFalse);
	_rooms[PYR_ENTRANCE]->setSectionVisible(21, kShownFalse);
	_rooms[PYR_ENTRANCE]->getObject(0)->_click = 255;

	if (_state._pyraE == 0 && _state._pyraS == 4 && _state._pyraZ == 10) {
		switch (_state._pyraDirection) {
		case 0:
			_rooms[PYR_ENTRANCE]->setSectionVisible(19, kShownTrue);
			_rooms[PYR_ENTRANCE]->getObject(0)->_click = 8;
			break;
		case 2:
			_rooms[PYR_ENTRANCE]->setSectionVisible(18, kShownTrue);
			_rooms[PYR_ENTRANCE]->getObject(0)->_click = 7;
			break;
		case 1:
			_rooms[PYR_ENTRANCE]->setSectionVisible(21, kShownTrue);
			_rooms[PYR_ENTRANCE]->getObject(0)->_click = 9;
			break;
		default:
			break;
		}
	}
	_rooms[PYR_ENTRANCE]->setSectionVisible(9,
			 _rooms[PYR_ENTRANCE]->isSectionVisible(7) &&
			!_rooms[PYR_ENTRANCE]->isSectionVisible(1));
	_rooms[PYR_ENTRANCE]->setSectionVisible(10,
			 _rooms[PYR_ENTRANCE]->isSectionVisible(7) &&
			!_rooms[PYR_ENTRANCE]->isSectionVisible(2));
	_rooms[PYR_ENTRANCE]->setSectionVisible(11,
			 _rooms[PYR_ENTRANCE]->isSectionVisible(8) &&
			!_rooms[PYR_ENTRANCE]->isSectionVisible(3));
	_rooms[PYR_ENTRANCE]->setSectionVisible(12,
			 _rooms[PYR_ENTRANCE]->isSectionVisible(8) &&
			!_rooms[PYR_ENTRANCE]->isSectionVisible(4));
}

byte GameManager2::wall(int s, int z, int direction, int stepsForward, int stepsRight) {
	static char vertical[2][12][11] = {
		{
			{0,0,0,0,0,0,0,0,0,0,0},
			{0,1,0,0,0,0,0,0,0,0,0},
			{1,0,1,0,0,0,0,0,0,0,0},
			{1,0,1,0,1,0,0,0,0,0,0},
			{0,1,0,0,1,0,0,1,0,0,0},
			{0,0,1,0,0,0,1,0,1,0,0},
			{0,0,0,1,1,0,1,0,0,0,0},
			{0,0,0,1,1,0,0,1,0,0,0},
			{0,0,0,0,0,1,0,1,0,0,0},
			{0,0,0,0,0,1,1,0,1,0,0},
			{0,0,0,0,1,0,0,0,1,0,0},
			{0,0,0,0,1,0,0,0,0,0,0}
		},
		{
			{0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,1,0,0,0,0,0,0},
			{0,0,0,0,1,0,0,0,0,0,0},
			{0,1,0,1,0,0,0,0,0,0,0},
			{0,1,0,1,1,0,0,0,0,0,0},
			{1,0,0,0,1,0,0,0,0,0,0},
			{0,0,0,0,0,1,0,0,1,0,0},
			{0,0,0,0,0,0,1,1,0,0,1},
			{0,0,0,0,0,1,0,1,0,0,1},
			{0,0,0,0,1,0,1,0,1,1,0},
			{0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0}
		}
	};

	static char horizontal[2][11][12] = {
		{
			{0,1,1,0,0,0,0,0,0,0,0,0},
			{0,0,1,0,0,0,0,0,0,0,0,0},
			{0,1,0,1,1,0,0,0,0,0,0,0},
			{0,1,1,0,0,0,0,0,0,0,0,0},
			{0,0,1,0,0,1,1,0,1,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,1,0,0,0,0},
			{0,0,0,0,1,1,0,0,0,0,0,0},
			{0,0,0,0,0,1,0,1,1,0,0,0},
			{0,0,0,0,0,1,1,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0}
		},
		{
			{0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0},
			{0,1,0,0,1,0,0,0,0,0,0,0},
			{0,0,0,0,1,0,0,0,0,0,0,0},
			{0,0,1,1,0,0,0,0,0,0,0,0},
			{0,1,1,0,0,1,0,0,0,0,0,0},
			{0,0,0,0,0,0,1,0,1,0,0,0},
			{0,0,0,0,0,1,1,0,0,0,0,0},
			{0,0,0,0,0,1,0,1,1,0,1,0},
			{0,0,0,0,0,1,1,0,0,1,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0}
		}
	};
	if (stepsRight) {
		int newR;
		if (stepsRight > 0)
			newR = (direction + 1) & 3;
		else {
			newR = (direction - 1) & 3;
			stepsRight = -stepsRight;
		}
		switch (direction) {
		case 0:
			return wall(s, z - stepsForward, newR, stepsRight, 0);
		case 2:
			return wall(s, z + stepsForward, newR, stepsRight, 0);
		case 1:
			return wall(s + stepsForward, z, newR, stepsRight, 0);
		case 3:
			return wall(s - stepsForward, z, newR, stepsRight, 0);
		default:
			break;
		}
	}
	switch (direction) {
	case 0:
		return vertical  [_state._pyraE][z + 1 - stepsForward][s] == 0;
	case 2:
		return vertical  [_state._pyraE][z     + stepsForward][s] == 0;
	case 1:
		return horizontal[_state._pyraE][z][s     + stepsForward] == 0;
	case 3:
		return horizontal[_state._pyraE][z][s + 1 - stepsForward] == 0;
	default:
		break;
	}
	return 0;
}

bool GameManager2::move(Action verb, Object &obj) {
	if (verb == ACTION_WALK && obj._id == CORRIDOR) {
		switch (_state._pyraDirection) {
			case 0:
				_state._pyraZ--;
				break;
			case 1:
				_state._pyraS++;
				break;
			case 2:
				_state._pyraZ++;
				break;
			case 3:
				_state._pyraS--;
				break;
			default:
				break;
		}
	} else if (verb == ACTION_WALK && obj._id == G_RIGHT) {
		_state._pyraDirection++;
		_state._pyraDirection &= 3;
	} else if (verb == ACTION_WALK && obj._id == G_LEFT) {
		_state._pyraDirection--;
		_state._pyraDirection &= 3;
	} else
		return false;
	return true;
}

void GameManager2::compass() {
	static int dirs[7] = {
		kStringDirection1,
		kStringDirection2,
		kStringDirection3,
		kStringDirection4,
		kStringDirection1,
		kStringDirection2,
		kStringDirection3
	};
	_vm->renderBox(281, 161, 39, 39, kColorWhite63);
	_vm->renderBox(295, 180, 13,  3, kColorWhite44);
	_vm->renderBox(300, 175,  3, 13, kColorWhite44);
	_vm->renderText(dirs[_state._pyraDirection    ], 299, 163, kColorBlack);
	_vm->renderText(dirs[_state._pyraDirection + 1], 312, 179, kColorBlack);
	_vm->renderText(dirs[_state._pyraDirection + 2], 299, 191, kColorBlack);
	_vm->renderText(dirs[_state._pyraDirection + 3], 283, 179, kColorBlack);
}

void GameManager2::puzzleConstruction() {
	_vm->setCurrentImage(12);
	MSNImage *image = _vm->_screen->getCurrentImage();
	for (int i = 0; i < 16; i ++) {
		_puzzleField[i] = 255;
	}
	for (int i = 0; i < 15; i++) {
		image->_section[i + 1].x1 = 95 + (_state._puzzleTab[i] % 4) * 33;
		image->_section[i + 1].x2 = image->_section[i + 1].x1 + 31;
		image->_section[i + 1].y1 = 24 + (_state._puzzleTab[i] / 4) * 25;
		image->_section[i + 1].y2 = image->_section[i + 1].y1 + 23;

		_puzzleField[_state._puzzleTab[i]] = i;
	}
}

void GameManager2::alarm() {
	_vm->_sound->playSiren();
	_state._sirenOn = true;
	if (_vm->_screen->isMessageShown())
		_vm->removeMessage();
	_vm->renderMessage(kStringMuseum7);
	_state._eventTime = g_system->getMillis() + 16200;
	_state._eventCallback = kCaughtFn;
	_state._alarmOn = true;
}

void GameManager2::caught() {
	if (_vm->_screen->isMessageShown())
		_vm->removeMessage();
	if        (_currentRoom->getId() <  MUS1) {
	} else if (_currentRoom->getId() <= MUS2) {
		_vm->renderImage( 8);
		_vm->renderImage(18);
	} else if (_currentRoom->getId() == MUS3) {
		_vm->renderImage(12);
		_vm->renderImage(30);
	} else if (_currentRoom->getId() == MUS4) {
		_vm->renderImage( 8);
		_vm->renderImage(18);
	} else if (_currentRoom->getId() == MUS5) {
		_vm->renderImage( 9);
		_vm->renderImage(29);
	} else if (_currentRoom->getId() <= MUS7) {
		_vm->renderImage( 7);
		_vm->renderImage(17);
	} else if (_currentRoom->getId() <= MUS9) {
		_vm->renderImage( 1);
		_vm->renderImage( 7);
	} else if (_currentRoom->getId() <= MUS11) {
		_vm->renderImage( 2);
		_vm->renderImage( 8);
	}
	caught2();
}

void GameManager2::caught2() {
	_vm->renderMessage(kStringMuseum8);
	_vm->playSound(kAudioCaught);
	wait(_messageDuration, true);
	_vm->removeMessage();
	_state._sirenOn = false;
	_mapOn = false;
	_state._haste = false;
	dead(kStringMuseum9);
}

void GameManager2::drawClock() {
	int time = (g_system->getMillis() - _state._startTime) / 600;
	int second = time % 100;
	Room *r;
	if (!_mapOn) {
		_vm->renderBox(281, 161, 39, 39, kColorWhite25);
		char s[9] = "00";
		s[1] = time % 10 + 48;
		time /= 10;
		s[0] = time % 10 + 48;
		time /= 10;
		_vm->renderText(s, 293, 180, kColorWhite99);
		Common::strlcpy(s, " 0:00", sizeof(s));
		s[4] = time % 10 + 48;
		time /= 10;
		s[3] = time % 10 + 48;
		time /= 10;
		s[1] = time % 10 + 48;
		time /= 10;
		if (time)
			s[0] = time % 10 + 48;
		_vm->renderText(s, 285, 170, kColorWhite99);
	}
	if ((r = _rooms[_securityTab[second / 10]]) == _currentRoom) {
		//arrow();
		_state._alarmCracked = false;
		caught();
		return;
	}
	for (int i = 0; i < 3; i++) {
		Object *o = r->getObject(i);
		if ((o->_id == DOOR || o->_id == ENCRYPTED_DOOR || o->_id == SMALL_DOOR) &&
				(o->_type & OPENED) && ! _state._alarmOn)
			alarm();
	}
	if (!_state._alarmOn && _currentRoom == _rooms[MUS4] &&
			second >= 21 && second <= 40)
		alarm();
	if (_currentRoom == _rooms[MUS_ENTRANCE] && second >= 22 && second <= 29) {
		if (!_steps && !_state._alarmCracked) {
			_steps = true;
			_vm->renderMessage(kStringMuseum6);
		}
	} else _steps = false;
}

void GameManager2::crack(int time) {
	_alarmBefore = _state._alarmOn;
	_cracking = true;
	_vm->_screen->changeCursor(ResourceManager::kCursorWait);
	int t = 0;
	int zv = 0;
	do {
		int z;
		do {
			wait(1);
		} while ((z = (g_system->getMillis() - _state._startTime) / 600) == zv);
		zv = z;
		drawClock();
		t++;
	} while (t < time && _state._alarmOn == _alarmBefore && !_dead);
	_cracking = false;
	_vm->_screen->changeCursor(ResourceManager::kCursorNormal);
	if (_state._alarmOn == _alarmBefore)
		_vm->removeMessage();
}

bool GameManager2::crackDoor(int time) {
	_vm->renderMessage(kStringMuseum15);
	crack(time);
	if (_dead)
		return false;
	if (_state._alarmOn != _alarmBefore) {
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->renderMessage(kStringMuseum16);
	}
	return !_state._alarmOn;
}

void GameManager2::museumDoorInteract(Action verb, Object &obj1, Object &obj2) {
	static struct {
		int _r1;
		int _o1;
		int _r2;
		int _o2;
	} doorTab[11] = {
		{MUS1, 0, MUS2, 0},
		{MUS2, 1, MUS3, 0},
		{MUS3, 1, MUS10, 0},
		{MUS10, 1, MUS11, 0},
		{MUS11, 1, MUS7, 1},
		{MUS7, 0, MUS6, 1},
		{MUS6, 0, MUS5, 1},
		{MUS5, 0, MUS4, 0},
		{MUS5, 2, MUS9, 1},
		{MUS9, 0, MUS8, 1},
		{MUS8, 0, MUS1, 1}
	};
	Room *r;
	if (verb == ACTION_OPEN && obj1._id == DOOR) {
		for (int i = 0; i < 11; i++) {
			if ((_currentRoom == _rooms[doorTab[i]._r1]) &&
				 &obj1 == _currentRoom->getObject(doorTab[i]._o1)) {
				r = _rooms[doorTab[i]._r2];
				r->getObject(doorTab[i]._o2)->_type |= OPENED;
				r->setSectionVisible(r->getObject(doorTab[i]._o2)->_section, kShownTrue);
			} else if ((_currentRoom == _rooms[doorTab[i]._r2]) &&
				 &obj1 == _currentRoom->getObject(doorTab[i]._o2)) {
				r = _rooms[doorTab[i]._r1];
				r->getObject(doorTab[i]._o1)->_type |= OPENED;
				r->setSectionVisible(r->getObject(doorTab[i]._o1)->_section, kShownTrue);
			}
		}
	} else if (verb == ACTION_CLOSE && obj1._id == DOOR) {
		for (int i = 0; i < 11; i++) {
			if ((_currentRoom == _rooms[doorTab[i]._r1]) &&
				 &obj1 == _currentRoom->getObject(doorTab[i]._o1)) {
				r = _rooms[doorTab[i]._r2];
				r->getObject(doorTab[i]._o2)->_type &= ~OPENED;
				r->setSectionVisible(r->getObject(doorTab[i]._o2)->_section, kShownFalse);
			} else if ((_currentRoom == _rooms[doorTab[i]._r2]) &&
				 &obj1 == _currentRoom->getObject(doorTab[i]._o2)) {
				r = _rooms[doorTab[i]._r1];
				r->getObject(doorTab[i]._o1)->_type &= ~OPENED;
				r->setSectionVisible(r->getObject(doorTab[i]._o1)->_section, kShownFalse);
			}
		}
	}
}

void GameManager2::securityEntrance() {
	static struct {
		RoomId _r;
		int _a1;
		int _a2;
	} securityList[11] = {
		{MUS1 ,11,31},
		{MUS2 , 7,17},
		{MUS3 , 7,17},
		{MUS10, 1, 7},
		{MUS11, 1, 7},
		{MUS7 ,10,32},
		{MUS6 , 8,18},
		{MUS5 , 8,18},
		{MUS9 , 2, 8},
		{MUS8 , 2, 8},
		{MUS1 , 0, 0}
	};

	int time = (g_system->getMillis() - _state._startTime) / 600;
	int second = time % 100;

	if (_rooms[_securityTab[second / 10]] == _currentRoom) {
		int i;
		for (i = 0; _currentRoom != _rooms[securityList[i]._r]; i++) {}
		if (_lastRoom == _rooms[securityList[i + 1]._r]) {
			_vm->renderImage(securityList[i]._a1);
			_vm->renderImage(securityList[i]._a2);
			caught2();
		} else
			caught();
	}
}

void GameManager2::pressureAlarmEntrance() {
	_state._pressureCounter = 0;
	securityEntrance();
}

void GameManager2::pressureAlarmCount() {
	if (!(_state._alarmOn ||
			(_currentRoom == _rooms[MUS22] && _currentRoom->isSectionVisible(6)))) {
		_state._pressureCounter++;
		if ((_currentRoom->getId() >= MUS12 && _state._pressureCounter > 8) ||
				_state._pressureCounter > 16)
			alarm();
	}
	setAnimationTimer(11);
}

}

