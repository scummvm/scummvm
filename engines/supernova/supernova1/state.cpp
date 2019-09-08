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

#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "gui/message.h"

#include "supernova/screen.h"
#include "supernova/supernova.h"
#include "supernova/supernova1/state.h"
#include "supernova/supernova1/stringid.h"

namespace Supernova {

bool GameManager1::serialize(Common::WriteStream *out) {
	if (out->err())
		return false;

	// GameState
	out->writeSint32LE(_time);
	out->writeSint32LE(_state._timeSleep);
	out->writeSint32LE(_state._timeAlarm);
	out->writeSint32LE(_state._eventTime);
	out->writeSint32LE(_state._eventCallback);
	out->writeSint32LE(_state._arrivalDaysLeft);
	out->writeSint32LE(_state._shipEnergyDaysLeft);
	out->writeSint32LE(_state._landingModuleEnergyDaysLeft);
	out->writeUint16LE(_state._greatFlag);
	out->writeSint16LE(_state._timeRobot);
	out->writeSint16LE(_state._money);
	out->writeByte(_state._coins);
	out->writeByte(_state._shoes);
	out->writeByte(_state._origin);
	out->writeByte(_state._destination);
	out->writeByte(_state._language);
	out->writeByte(_state._corridorSearch);
	out->writeByte(_state._alarmOn);
	out->writeByte(_state._terminalStripConnected);
	out->writeByte(_state._terminalStripWire);
	out->writeByte(_state._cableConnected);
	out->writeByte(_state._powerOff);
	out->writeByte(_state._dream);
	for (int i = 0; i < 4; i++)
		out->writeByte(_state._nameSeen[i]);
	out->writeByte(_state._playerHidden);

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
	out->writeByte(_currentRoom->getId());
	for (int i = 0; i < NUMROOMS1; ++i) {
		_rooms[i]->serialize(out);
	}

	return !out->err();
}

bool GameManager1::deserialize(Common::ReadStream *in, int version) {
	if (in->err())
		return false;

	// GameState
	_time = in->readSint32LE();
	_state._timeSleep = in->readSint32LE();
	_state._timeAlarm = in->readSint32LE();
	_state._eventTime = in->readSint32LE();
	if (version >= 4)
		_state._eventCallback = (EventFunction)in->readSint32LE();
	else
		_state._eventCallback = kNoFn;
	_state._arrivalDaysLeft = in->readSint32LE();
	_state._shipEnergyDaysLeft = in->readSint32LE();
	_state._landingModuleEnergyDaysLeft = in->readSint32LE();
	_state._greatFlag = in->readUint16LE();
	_state._timeRobot = in->readSint16LE();
	_state._money = in->readSint16LE();
	_vm->setGameString(kStringInventoryMoney, Common::String::format("%d Xa", _state._money));
	_state._coins = in->readByte();
	_state._shoes = in->readByte();
	if (version >= 6)
		_state._origin = in->readByte();
	else
		_state._origin = 0;
	_state._destination = in->readByte();
	_state._language = in->readByte();
	_state._corridorSearch = in->readByte();
	_state._alarmOn = in->readByte();
	_state._terminalStripConnected = in->readByte();
	_state._terminalStripWire = in->readByte();
	_state._cableConnected = in->readByte();
	_state._powerOff = in->readByte();
	_state._dream = in->readByte();

	for (int i = 0; i < 4; i++) {
		if (version >= 7)
			_state._nameSeen[i] = in->readByte();
		else
			_state._nameSeen[i] = false;
	}

	if (version >= 8)
		_state._playerHidden = in->readByte();
	else
		_state._playerHidden = false;

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
	RoomId curRoomId = static_cast<RoomId>(in->readByte());
	for (int i = 0; i < NUMROOMS1; ++i) {
		_rooms[i]->deserialize(in, version);
	}
	changeRoom(curRoomId);

	// Some additional variables
	_guiEnabled = true;
	_animationEnabled = true;

	return !in->err();
}

// Used by Look Watch (when it's fixed). Do not remove.
static Common::String timeToString(int msec) {
	char s[9] = " 0:00:00";
	msec /= 1000;
	s[7] = msec % 10 + '0';
	msec /= 10;
	s[6] = msec %  6 + '0';
	msec /=  6;
	s[4] = msec % 10 + '0';
	msec /= 10;
	s[3] = msec %  6 + '0';
	msec /=  6;
	s[1] = msec % 10 + '0';
	msec /= 10;
	if (msec)
		s[0] = msec + '0';

	return Common::String(s);
}

GameManager1::GameManager1(SupernovaEngine *vm, Sound *sound)
	: GameManager(vm, sound) {
	initRooms();
	changeRoom(INTRO1);
	initState();
}

GameManager1::~GameManager1() {
	destroyRooms();
}

void GameManager1::destroyRooms() {
	delete _rooms[INTRO1];
	delete _rooms[CORRIDOR_ROOM];
	delete _rooms[HALL];
	delete _rooms[SLEEP];
	delete _rooms[COCKPIT];
	delete _rooms[AIRLOCK];
	delete _rooms[HOLD];
	delete _rooms[LANDINGMODULE];
	delete _rooms[GENERATOR];
	delete _rooms[OUTSIDE];
	delete _rooms[CABIN_R1];
	delete _rooms[CABIN_R2];
	delete _rooms[CABIN_R3];
	delete _rooms[CABIN_L1];
	delete _rooms[CABIN_L2];
	delete _rooms[CABIN_L3];
	delete _rooms[BATHROOM];

	delete _rooms[ROCKS];
	delete _rooms[CAVE];
	delete _rooms[MEETUP];
	delete _rooms[ENTRANCE];
	delete _rooms[REST];
	delete _rooms[ROGER];
	delete _rooms[GLIDER];
	delete _rooms[MEETUP2];
	delete _rooms[MEETUP3];

	delete _rooms[CELL];
	delete _rooms[CORRIDOR1];
	delete _rooms[CORRIDOR2];
	delete _rooms[CORRIDOR3];
	delete _rooms[CORRIDOR4];
	delete _rooms[CORRIDOR5];
	delete _rooms[CORRIDOR6];
	delete _rooms[CORRIDOR7];
	delete _rooms[CORRIDOR8];
	delete _rooms[CORRIDOR9];
	delete _rooms[BCORRIDOR];
	delete _rooms[GUARD];
	delete _rooms[GUARD3];
	delete _rooms[OFFICE_L1];
	delete _rooms[OFFICE_L2];
	delete _rooms[OFFICE_R1];
	delete _rooms[OFFICE_R2];
	delete _rooms[OFFICE_L];
	delete _rooms[ELEVATOR];
	delete _rooms[STATION];
	delete _rooms[SIGN_ROOM];
	delete _rooms[OUTRO];
	delete _rooms;
}

void GameManager1::initState() {
	GameManager::initState();
	_time = ticksToMsec(916364); // 2 pm
	_state._timeSleep = 0;
	_state._timeAlarm = ticksToMsec(458182);    // 7 am
	_state._eventTime = kMaxTimerValue;
	_state._eventCallback = kNoFn;
	_state._arrivalDaysLeft = 2840;
	_state._shipEnergyDaysLeft = 2135;
	_state._landingModuleEnergyDaysLeft = 923;
	_state._greatFlag = 0;
	_state._timeRobot = 0;
	_state._money = 0;
	_state._coins = 0;
	_state._shoes = 0;
	_state._origin = 0;
	_state._destination = 255;
	_state._language = 0;
	_state._corridorSearch = false;
	_state._alarmOn = false;
	_state._terminalStripConnected = false;
	_state._terminalStripWire = false;
	_state._cableConnected = false;
	_state._powerOff = false;
	_state._dream = false;
}

void GameManager1::initRooms() {
	_rooms = new Room *[NUMROOMS1];
	_rooms[INTRO1] = new Intro(_vm, this);
	_rooms[CORRIDOR_ROOM] = new ShipCorridor(_vm, this);
	_rooms[HALL] = new ShipHall(_vm, this);
	_rooms[SLEEP] = new ShipSleepCabin(_vm, this);
	_rooms[COCKPIT] = new ShipCockpit(_vm, this);
	_rooms[AIRLOCK] = new ShipAirlock(_vm, this);
	_rooms[HOLD] = new ShipHold(_vm, this);
	_rooms[LANDINGMODULE] = new ShipLandingModule(_vm, this);
	_rooms[GENERATOR] = new ShipGenerator(_vm, this);
	_rooms[OUTSIDE] = new ShipOuterSpace(_vm, this);
	_rooms[CABIN_R1] = new ShipCabinR1(_vm, this);
	_rooms[CABIN_R2] = new ShipCabinR2(_vm, this);
	_rooms[CABIN_R3] = new ShipCabinR3(_vm, this);
	_rooms[CABIN_L1] = new ShipCabinL1(_vm, this);
	_rooms[CABIN_L2] = new ShipCabinL2(_vm, this);
	_rooms[CABIN_L3] = new ShipCabinL3(_vm, this);
	_rooms[BATHROOM] = new ShipCabinBathroom(_vm, this);

	_rooms[ROCKS] = new ArsanoRocks(_vm, this);
	_rooms[CAVE] = new ArsanoCave(_vm, this);
	_rooms[MEETUP] = new ArsanoMeetup(_vm, this);
	_rooms[ENTRANCE] = new ArsanoEntrance(_vm, this);
	_rooms[REST] = new ArsanoRemaining(_vm, this);
	_rooms[ROGER] = new ArsanoRoger(_vm, this);
	_rooms[GLIDER] = new ArsanoGlider(_vm, this);
	_rooms[MEETUP2] = new ArsanoMeetup2(_vm, this);
	_rooms[MEETUP3] = new ArsanoMeetup3(_vm, this);

	_rooms[CELL] = new AxacussCell(_vm, this);
	_rooms[CORRIDOR1] = new AxacussCorridor1(_vm, this);
	_rooms[CORRIDOR2] = new AxacussCorridor2(_vm, this);
	_rooms[CORRIDOR3] = new AxacussCorridor3(_vm, this);
	_rooms[CORRIDOR4] = new AxacussCorridor4(_vm, this);
	_rooms[CORRIDOR5] = new AxacussCorridor5(_vm, this);
	_rooms[CORRIDOR6] = new AxacussCorridor6(_vm, this);
	_rooms[CORRIDOR7] = new AxacussCorridor7(_vm, this);
	_rooms[CORRIDOR8] = new AxacussCorridor8(_vm, this);
	_rooms[CORRIDOR9] = new AxacussCorridor9(_vm, this);
	_rooms[BCORRIDOR] = new AxacussBcorridor(_vm, this);
	_rooms[GUARD] = new AxacussIntersection(_vm, this);
	_rooms[GUARD3] = new AxacussExit(_vm, this);
	_rooms[OFFICE_L1] = new AxacussOffice1(_vm, this);
	_rooms[OFFICE_L2] = new AxacussOffice2(_vm, this);
	_rooms[OFFICE_R1] = new AxacussOffice3(_vm, this);
	_rooms[OFFICE_R2] = new AxacussOffice4(_vm, this);
	_rooms[OFFICE_L] = new AxacussOffice5(_vm, this);
	_rooms[ELEVATOR] = new AxacussElevator(_vm, this);
	_rooms[STATION] = new AxacussStation(_vm, this);
	_rooms[SIGN_ROOM] = new AxacussSign(_vm, this);
	_rooms[OUTRO] = new Outro(_vm, this);
}

bool GameManager1::canSaveGameStateCurrently() {
	return _animationEnabled && _guiEnabled;
}

void GameManager1::updateEvents() {
	handleTime();
	if (_animationEnabled && !_vm->_screen->isMessageShown() && _animationTimer == 0)
		_currentRoom->animation();

	if (_state._eventCallback != kNoFn && _time >= _state._eventTime) {
		_vm->_allowLoadGame = false;
		_vm->_allowSaveGame = false;
		_state._eventTime = kMaxTimerValue;
		EventFunction fn = _state._eventCallback;
		_state._eventCallback = kNoFn;
		switch (fn) {
		case kNoFn:
			break;
		case kSupernovaFn:
			supernovaEvent();
			break;
		case kGuardReturnedFn:
			guardReturnedEvent();
			break;
		case kGuardWalkFn:
			guardWalkEvent();
			break;
		case kTaxiFn:
			taxiEvent();
			break;
		case kSearchStartFn:
			searchStartEvent();
			break;
		default:
			break; //shouldn't happen
		}
		_vm->_allowLoadGame = true;
		_vm->_allowSaveGame = true;
		return;
	}

	if (_state._alarmOn && _state._timeAlarm <= _time) {
		_state._alarmOn = false;
		alarm();
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
			if (_currentRoom->getId() != INTRO1 && _sound->isPlaying())
				return;
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

void GameManager1::corridorOnEntrance() {
	if (_state._corridorSearch)
		busted(0);
}

void GameManager1::telomat(int nr) {
	static Common::String name[8] = {
		"DR. ALAB HANSI",
		"ALAB HANSI",
		"SAVAL LUN",
		"x",
		"PROF. DR. UGNUL TSCHABB",
		"UGNUL TSCHABB",
		"ALGA HURZ LI",
		"x"
	};

	static Common::String name2[4] = {
		"Alab Hansi",
		"Saval Lun",
		"Ugnul Tschabb",
		"Alga Hurz Li"
	};

	int dial1[4];
	dial1[0] = kStringTelomat1;
	dial1[1] = kNoString;
	dial1[2] = kStringTelomat3;
	dial1[3] = kStringDialogSeparator;

	static byte rows1[3] = {1, 2, 1};

	int dial2[4];
	dial2[0] = kStringTelomat4;
	dial2[1] = kStringTelomat5;
	dial2[2] = kStringTelomat6;
	dial2[3] = kStringDialogSeparator;

	static byte rows2[4] = {1, 1, 1, 1};

	_guiEnabled = false;
	_vm->renderBox(0, 0, 320, 200, kColorBlack);
	_vm->renderText(kStringTelomat7, 100, 70, kColorGreen);
	_vm->renderText(kStringTelomat8, 100, 81, kColorGreen);
	_vm->renderText(kStringTelomat9, 100, 92, kColorGreen);
	_vm->renderText(kStringTelomat10, 100, 103, kColorGreen);
	_vm->renderText(kStringTelomat11, 100, 120, kColorDarkGreen);
	Common::String input;
	do {
		getInput();

		switch (_key.keycode) {
		case Common::KEYCODE_2: {
			_vm->renderBox(0, 0, 320, 200, kColorDarkBlue);
			_vm->renderText(kStringTelomat12, 50, 80, kColorGreen);
			_vm->renderText(kStringTelomat13, 50, 91, kColorGreen);
			do {
				edit(input, 50, 105, 30);
			} while ((_key.keycode != Common::KEYCODE_RETURN) && (_key.keycode != Common::KEYCODE_ESCAPE));

			if (_key.keycode == Common::KEYCODE_ESCAPE) {
				_vm->renderBox(0, 0, 320, 200, kColorBlack);
				_vm->renderRoom(*_currentRoom);
				_vm->paletteBrightness();
				_guiEnabled = true;
				drawMapExits();
				return;
			}

			input.toUppercase();

			int i = 0;
			while ((i < 8) && (input != name[i]))
				i++;
			i >>= 1;
			if (i == 4) {
				_vm->renderText(kStringTelomat14, 50, 120, kColorGreen);
				wait(10);
				_vm->renderBox(0, 0, 320, 200, kColorBlack);
				_vm->renderRoom(*_currentRoom);
				_vm->paletteBrightness();
				_guiEnabled = true;
				drawMapExits();
				return;
			}

			if ((i == nr) || _rooms[BCORRIDOR]->getObject(4 + i)->hasProperty(CAUGHT)) {
				_vm->renderText(kStringTelomat15, 50, 120, kColorGreen);
				wait(10);
				_vm->renderBox(0, 0, 320, 200, kColorBlack);
				_vm->renderRoom(*_currentRoom);
				_vm->paletteBrightness();
				_guiEnabled = true;
				drawMapExits();
				return;
			}

			_vm->renderText(kStringTelomat16, 50, 120, kColorGreen);
			wait(10);
			_vm->renderBox(0, 0, 320, 200, kColorBlack);
			_vm->renderRoom(*_currentRoom);
			_vm->paletteBrightness();
			_vm->renderMessage(kStringTelomat17, kMessageTop, name2[i]);
			wait(_messageDuration, true);
			_vm->removeMessage();
			if (_state._nameSeen[nr]) {
				Common::String string = _vm->getGameString(kStringTelomat2);
				_vm->setGameString(kStringPlaceholder1, Common::String::format(string.c_str(), name2[nr].c_str()));
				dial1[1] = kStringPlaceholder1;
				_currentRoom->addSentence(1, 1);
			} else
				_currentRoom->removeSentence(1, 1);

			switch (dialog(3, rows1, dial1, 1)) {
			case 1: _vm->renderMessage(kStringTelomat18, kMessageTop);
				wait(_messageDuration, true);
				_vm->removeMessage();
				if ((_state._destination == 255) && !_rooms[BCORRIDOR]->isSectionVisible(7)) {
					_state._eventTime = _time + ticksToMsec(150);
					_state._eventCallback = kGuardWalkFn;
					_state._origin = i;
					_state._destination = nr;
				}
				break;
			case 0: _vm->renderMessage(kStringTelomat19, kMessageTop);
				wait(_messageDuration, true);
				_vm->removeMessage();
				if (dialog(4, rows2, dial2, 0) != 3) {
					wait(10);
					say(kStringTelomat20);
				}
				_rooms[BCORRIDOR]->setSectionVisible(7, true);
				_rooms[BCORRIDOR]->setSectionVisible(i + 1, true);
				_state._eventTime = kMaxTimerValue;
				_currentRoom->addSentence(0, 1);
			}
			_guiEnabled = true;
			drawMapExits();
			return;
			}
		case Common::KEYCODE_1:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
			_vm->renderBox(0, 0, 320, 200, kColorDarkBlue);
			_vm->renderText(kStringTelomat21, 100, 90, kColorGreen);
			input = "";
			do {
				edit(input, 100, 105, 30);
			} while ((_key.keycode != Common::KEYCODE_RETURN) && (_key.keycode != Common::KEYCODE_ESCAPE));

			if (_key.keycode == Common::KEYCODE_RETURN) {
				_vm->renderText(kStringShipSleepCabin9, 100, 120, kColorGreen);
				wait(10);
			}
			// fallthrough
		case Common::KEYCODE_ESCAPE:
			_vm->renderBox(0, 0, 320, 200, kColorBlack);
			_vm->renderRoom(*_currentRoom);
			_vm->paletteBrightness();
			_guiEnabled = true;
			drawMapExits();
			return;
		default:
			break;
		}
	} while (true);
}

void GameManager1::startSearch() {
	if ((_currentRoom->getId() >= CORRIDOR1) && (_currentRoom->getId() <= BCORRIDOR))
		busted(0);

	_state._corridorSearch = true;
}

void GameManager1::search(int time) {
	_state._eventTime = _time + ticksToMsec(time);
	_state._eventCallback = kSearchStartFn;
}

void GameManager1::guardNoticed() {
	_vm->paletteFadeOut();
	Room *r = _currentRoom;
	_currentRoom = _rooms[GUARD];
	_vm->setCurrentImage(40);
	_vm->renderBox(0, 0, 320, 200, 0);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_vm->renderImage(2);
	reply(kStringGuardNoticed1, 2, 5);
	wait(2);
	reply(kStringGuardNoticed2, 2, 5);
	_vm->paletteFadeOut();
	_currentRoom->setSectionVisible(2, false);
	_currentRoom->setSectionVisible(5, false);
	_currentRoom = r;
	_guiEnabled = true;
	drawMapExits();
}

void GameManager1::busted(int i) {
	if (i > 0)
		_vm->renderImage(i);
	if (i == 0) {
		if ((_currentRoom->getId() >= OFFICE_L1) && (_currentRoom->getId() <= OFFICE_R2)) {
			if (_currentRoom->getId() < OFFICE_R1)
				i = 10;
			else
				i = 5;
			if (!_currentRoom->getObject(0)->hasProperty(OPENED)) {
				_vm->renderImage(i - 1);
				_sound->play(kAudioDoorOpen);
				wait(2);
			}
			_vm->renderImage(i);
			wait(3);
			_vm->renderImage(i + 3);
			_sound->play(kAudioVoiceHalt);
			_vm->renderImage(i);
			wait(5);
			if (_currentRoom->getId() == OFFICE_L2)
				i = 13;
			_vm->renderImage(i + 1);
			wait(3);
			_vm->renderImage(i + 2);
			shot(0, 0);
			return;
		} else if (_currentRoom->getId() == BCORRIDOR)
			_vm->renderImage(21);
		else if (_currentRoom->isSectionVisible(4))
			_vm->renderImage(32); // below
		else if (_currentRoom->isSectionVisible(2))
			_vm->renderImage(30); // right
		else if (_currentRoom->isSectionVisible(1))
			_vm->renderImage(31); // left
		else
			_vm->renderImage(33); // above
	}
	_sound->play(kAudioVoiceHalt);
	wait(3);
	shot(0, 0);
}

void GameManager1::novaScroll() {
	static byte planet_f[6] = {0xeb,0xec,0xf0,0xed,0xf1,0xf2};
	static byte nova_f[13] = {0xea,0xe9,0xf5,0xf3,0xf7,0xf4,0xf6,
		0xf9,0xfb,0xfc,0xfd,0xfe,0xfa};
	static byte rgb[65][3] = {
		{ 5, 0, 0},{10, 0, 0},{15, 0, 0},{20, 0, 0},{25, 0, 0},
		{30, 0, 0},{35, 0, 0},{40, 0, 0},{45, 0, 0},{50, 0, 0},
		{55, 0, 0},{60, 0, 0},{63,10, 5},{63,20,10},{63,30,15},
		{63,40,20},{63,50,25},{63,60,30},{63,63,33},{63,63,30},
		{63,63,25},{63,63,20},{63,63,15},{63,63,10},{60,60,15},
		{57,57,20},{53,53,25},{50,50,30},{47,47,35},{43,43,40},
		{40,40,45},{37,37,50},{33,33,53},{30,30,56},{27,27,59},
		{23,23,61},{20,20,63},{21,25,63},{22,30,63},{25,35,63},
		{30,40,63},{35,45,63},{40,50,63},{45,55,63},{50,60,63},
		{55,63,63},{59,63,63},{63,63,63},{63,60,63},{60,50,60},
		{55,40,55},{50,30,50},{45,20,45},{40,10,40},{42,15,42},
		{45,20,45},{47,25,47},{50,30,50},{52,35,52},{55,40,55},
		{57,45,57},{60,50,60},{62,55,62},{63,60,63},{63,63,63}};

	byte palette[768];
	_vm->_system->getPaletteManager()->grabPalette(palette, 0, 255);

	for (int t = 0; t < 65; ++t) {
		for (int i = 0; i < 6; ++i) {
			int idx = 3 * (planet_f[i] - 1);
			for (int c = 0 ; c < 3 ; ++c) {
				if (palette[idx+c] < rgb[t][c])
					palette[idx+c] = rgb[t][c];
			}
		}
		for (int cycle = 0; cycle < t && cycle < 13; ++cycle) {
			int idx = 3 * (nova_f[cycle] - 1);
			for (int c = 0 ; c < 3 ; ++c)
				palette[idx + c] = rgb[t - cycle - 1][c];
		}

		_vm->_system->getPaletteManager()->setPalette(palette, 0, 255);
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(_vm->_delay);
	}
}

void GameManager1::supernovaEvent() {
	_vm->removeMessage();
	CursorMan.showMouse(false);
	if (_currentRoom->getId() <= CAVE) {
		_vm->renderMessage(kStringSupernova1);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->paletteFadeOut();
		changeRoom(MEETUP);
		_rooms[AIRLOCK]->getObject(0)->disableProperty(OPENED);
		_rooms[AIRLOCK]->setSectionVisible(3, true);
		_rooms[AIRLOCK]->getObject(1)->setProperty(OPENED);
		_rooms[AIRLOCK]->setSectionVisible(17, true);
		_rooms[AIRLOCK]->setSectionVisible(6, false);
		_vm->renderRoom(*_currentRoom);
		_vm->paletteFadeIn();
	}
	_vm->renderMessage(kStringSupernova2);
	wait(_messageDuration, true);
	_vm->removeMessage();
	_vm->setCurrentImage(26);
	_vm->renderImage(0);
	_vm->paletteBrightness();
	novaScroll();
	_vm->paletteFadeOut();
	_vm->renderBox(0, 0, 320, 200, kColorBlack);
	_vm->_screen->setGuiBrightness(255);
	_vm->paletteBrightness();

	if (_currentRoom->getId() == GLIDER) {
		_vm->renderMessage(kStringSupernova3);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->_screen->setGuiBrightness(0);
		_vm->paletteBrightness();
		_vm->renderRoom(*_currentRoom);
		_vm->paletteFadeIn();
		_vm->renderMessage(kStringSupernova4, kMessageTop);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->renderMessage(kStringSupernova5, kMessageTop);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->renderMessage(kStringSupernova6, kMessageTop);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->renderMessage(kStringSupernova7, kMessageTop);
		wait(_messageDuration, true);
		_vm->removeMessage();
		changeRoom(MEETUP2);
		_rooms[MEETUP2]->setSectionVisible(1, true);
		_rooms[MEETUP2]->removeSentence(0, 1);
		_inventory.remove(*(_rooms[ROGER]->getObject(3)));
		_inventory.remove(*(_rooms[ROGER]->getObject(7)));
		_inventory.remove(*(_rooms[ROGER]->getObject(8)));
	} else {
		_vm->renderMessage(kStringSupernova8);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->_screen->setGuiBrightness(0);
		_vm->paletteBrightness();
		changeRoom(MEETUP2);
		if (_rooms[ROGER]->getObject(3)->hasProperty(CARRIED) && !_rooms[GLIDER]->isSectionVisible(5)) {
			_rooms[MEETUP2]->setSectionVisible(1, true);
			_rooms[MEETUP2]->setSectionVisible(12, true);
			_rooms[MEETUP2]->getObject(1)->_click = 0;
			_rooms[MEETUP2]->getObject(0)->_click = 1;
			_rooms[MEETUP2]->removeSentence(0, 1);
		}
		_rooms[MEETUP2]->removeSentence(1, 1);
	}
	_rooms[AIRLOCK]->getObject(4)->setProperty(WORN);
	_rooms[AIRLOCK]->getObject(5)->setProperty(WORN);
	_rooms[AIRLOCK]->getObject(6)->setProperty(WORN);
	_rooms[CAVE]->getObject(1)->_exitRoom = MEETUP2;
	_guiEnabled = true;
	CursorMan.showMouse(true);
}

void GameManager1::guardReturnedEvent() {
	if (_currentRoom->getId() == GUARD)
		busted(-1);
	else if ((_currentRoom->getId() == CORRIDOR9) && (_currentRoom->isSectionVisible(27)))
		busted(0);

	_rooms[GUARD]->setSectionVisible(1, false);
	_rooms[GUARD]->getObject(3)->_click = 0;
	_rooms[GUARD]->setSectionVisible(6, false);
	_rooms[GUARD]->getObject(2)->disableProperty(OPENED);
	_rooms[GUARD]->setSectionVisible(7, false);
	_rooms[GUARD]->getObject(5)->_click = 255;
	_rooms[CORRIDOR9]->setSectionVisible(27, false);
	_rooms[CORRIDOR9]->setSectionVisible(28, true);
	_rooms[CORRIDOR9]->getObject(1)->disableProperty(OPENED);
}

void GameManager1::walk(int imgId) {
	if (_prevImgId)
		_vm->renderImage(_prevImgId + kSectionInvert);
	_vm->renderImage(imgId);
	_prevImgId = imgId;
	wait(3);
}

void GameManager1::guardWalkEvent() {
	_prevImgId = 0;
	bool behind = (!_rooms[BCORRIDOR]->getObject(_state._origin + 4)->hasProperty(OCCUPIED) ||
				   _rooms[BCORRIDOR]->getObject(_state._origin + 4)->hasProperty(OPENED));
	_rooms[BCORRIDOR]->getObject(_state._origin + 4)->disableProperty(OCCUPIED);
	if (_currentRoom == _rooms[BCORRIDOR]) {
		if (_vm->_screen->isMessageShown())
			_vm->removeMessage();

		if (!behind) {
			_vm->renderImage(_state._origin + 1);
			_prevImgId = _state._origin + 1;
			_sound->play(kAudioDoorOpen);
			wait(3);
		}

		int imgId;
		switch (_state._origin) {
		case 0:
			imgId = 11;
			break;
		case 1:
			imgId = 16;
			break;
		case 2:
			imgId = 15;
			break;
		case 3:
		default:
			imgId = 20;
			break;
		}
		_vm->renderImage(imgId);
		if (!behind) {
			wait(3);
			_vm->renderImage(_prevImgId + kSectionInvert);
			_sound->play(kAudioDoorClose);
		}

		_prevImgId = imgId;
		wait(3);
		switch (_state._origin) {
		case 0:
			walk(12);
			walk(13);
			break;
		case 1:
			walk(17);
			walk(18);
			break;
		case 2:
			walk(14);
			walk(13);
			break;
		case 3:
			walk(19);
			walk(18);
		}

		if (!_state._playerHidden) {
			if (_state._origin & 1)
				walk(10);
			else
				walk(5);
			busted(-1);
		}

		if ((_state._origin & 1) && !(_state._destination & 1)) {
			for (int i = 10; i >= 5; i--)
				walk(i);
			walk(13);
		} else if (!(_state._origin & 1) && (_state._destination & 1)) {
			for (int i = 5; i <= 10; i++)
				walk(i);
			walk(18);
		}

		switch (_state._destination) {
		case 0:
			for (int i = 13; i >= 11; i--)
				walk(i);
			break;
		case 1:
			for (int i = 18; i >= 16; i--)
				walk(i);
			break;
		case 2:
			for (int i = 13; i <= 15; i++)
				walk(i);
			break;
		case 3:
			for (int i = 18; i <= 20; i++)
				walk(i);
		}

		if (behind) {
			_vm->renderImage(_state._destination + 1);
			_sound->play(kAudioDoorOpen);
			wait(3);
			_vm->renderImage(_prevImgId + kSectionInvert);
			wait(3);
			_vm->renderImage(_state._destination + 1 + kSectionInvert);
			_sound->play(kAudioDoorClose);
			_rooms[BCORRIDOR]->getObject(_state._destination + 4)->setProperty(OCCUPIED);
			_state._destination = 255;
		} else if (_rooms[BCORRIDOR]->isSectionVisible(_state._destination + 1)) {
			_vm->renderImage(_prevImgId + kSectionInvert);
			_rooms[BCORRIDOR]->getObject(_state._destination + 4)->setProperty(OCCUPIED);
			SWAP(_state._origin, _state._destination);
			_state._eventTime = _time + ticksToMsec(60);
			_state._eventCallback = kGuardWalkFn;
		} else {
			wait(18);
			SWAP(_state._origin, _state._destination);
			_state._eventCallback = kGuardWalkFn;
		}
	} else if (behind) {
		_rooms[BCORRIDOR]->getObject(_state._destination + 4)->setProperty(OCCUPIED);
		if (_currentRoom == _rooms[OFFICE_L1 + _state._destination])
			busted(0);
		_state._destination = 255;
	} else if (_rooms[BCORRIDOR]->isSectionVisible(_state._destination + 1) && _rooms[OFFICE_L1 + _state._destination]->getObject(0)->hasProperty(OPENED)) {
		_rooms[BCORRIDOR]->getObject(_state._destination + 4)->setProperty(OCCUPIED);
		if (_currentRoom == _rooms[OFFICE_L1 + _state._destination])
			busted(0);
		SWAP(_state._origin, _state._destination);
		_state._eventTime = _time + ticksToMsec(60);
		_state._eventCallback = kGuardWalkFn;
	} else {
		SWAP(_state._origin, _state._destination);
		_state._eventCallback = kGuardWalkFn;
	}
}

void GameManager1::taxiEvent() {
	if (_currentRoom->getId() == SIGN_ROOM) {
		changeRoom(STATION);
		_vm->renderRoom(*_currentRoom);
	}

	_vm->renderImage(1);
	_vm->renderImage(2);
	_sound->play(kAudioRocks);
	screenShake();
	_vm->renderImage(9);
	_currentRoom->getObject(1)->setProperty(OPENED);
	_vm->renderImage(1);
	_currentRoom->setSectionVisible(2, false);
	_vm->renderImage(3);
	for (int i = 4; i <= 8; i++) {
		wait(2);
		_vm->renderImage(invertSection(i - 1));
		_vm->renderImage(i);
	}
	_rooms[SIGN_ROOM]->setSectionVisible(2, false);
	_rooms[SIGN_ROOM]->setSectionVisible(3, true);
}

void GameManager1::searchStartEvent() {
	if ((_currentRoom >= _rooms[CORRIDOR1]) && (_currentRoom <= _rooms[BCORRIDOR]))
		busted(0);
	_state._corridorSearch = true;
}

void GameManager1::great(uint number) {
	if (number && (_state._greatFlag & (1 << number)))
		return;

	_sound->play(kAudioSuccess);
	_state._greatFlag |= 1 << number;
}

bool GameManager1::airless() {
	return (_currentRoom->getId() == HOLD ||
			_currentRoom->getId() == LANDINGMODULE ||
			_currentRoom->getId() == GENERATOR ||
			_currentRoom->getId() == OUTSIDE ||
			_currentRoom->getId() == ROCKS ||
			_currentRoom->getId() == CAVE ||
			_currentRoom->getId() == MEETUP ||
			_currentRoom->getId() == MEETUP2 ||
			_currentRoom->getId() == MEETUP3 ||
			(_currentRoom->getId() == AIRLOCK && _rooms[AIRLOCK]->getObject(1)->hasProperty(OPENED)));
}

void GameManager1::turnOff() {
	if (_state._powerOff)
		return;

	_state._powerOff = true;
	roomBrightness();
}

void GameManager1::turnOn() {
	if (!_state._powerOff)
		return;

	_state._powerOff = false;
	_vm->_screen->setViewportBrightness(255);
	_rooms[SLEEP]->setSectionVisible(1, false);
	_rooms[SLEEP]->setSectionVisible(2, false);
	_rooms[COCKPIT]->setSectionVisible(22, false);
}

void GameManager1::roomBrightness() {
	_roomBrightness = 255;
	if ((_currentRoom->getId() != OUTSIDE) && (_currentRoom->getId() < ROCKS) && _state._powerOff)
		_roomBrightness = 153;
	else if (_currentRoom->getId() == CAVE)
		_roomBrightness = 0;
	else if ((_currentRoom->getId() == GUARD3) && _state._powerOff)
		_roomBrightness = 0;

	if (_vm->_screen->getViewportBrightness() != 0)
		_vm->_screen->setViewportBrightness(_roomBrightness);

	_vm->paletteBrightness();
}

void GameManager1::handleTime() {
	if (_timerPaused)
		return;
	int32 newTime = g_system->getMillis();
	int32 delta = newTime - _oldTime;
	_time += delta;
	if (_time > 86400000) {
		_time -= 86400000; // 24h wrap around
		_state._alarmOn = (_state._timeAlarm > _time);
	}
	if (_animationTimer > delta)
		_animationTimer -= delta;
	else
		_animationTimer = 0;

	_oldTime = newTime;
}

void GameManager1::loadTime() {
	pauseTimer(false);
}

void GameManager1::saveTime() {
	pauseTimer(true);
}

void GameManager1::shock() {
	_sound->play(kAudioShock);
	dead(kStringShock);
}

void GameManager1::drawMapExits() {
	_vm->renderBox(281, 161, 39, 39, kColorWhite25);

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

void GameManager1::shot(int a, int b) {
	if (a)
		_vm->renderImage(a);
	_sound->play(kAudioGunShot);
	wait(2);
	if (b)
		_vm->renderImage(b);
	wait(2);
	if (a)
		_vm->renderImage(a);
	_sound->play(kAudioGunShot);
	wait(2);
	if (b)
		_vm->renderImage(b);

	dead(kStringShot);
}

void GameManager1::takeMoney(int amount) {
	Object *moneyObject = _rooms[INTRO1]->getObject(4);
	_state._money += amount;
	_vm->setGameString(kStringInventoryMoney, Common::String::format("%d Xa", _state._money));

	if (_state._money > 0) {
		takeObject(*moneyObject);
		if (amount > 0)
			great(0);
	} else {
		_inventory.remove(*moneyObject);
	}
}

void GameManager1::openLocker(const Room *room, Object *obj, Object *lock, int section) {
	_vm->renderImage(section);
	obj->setProperty(OPENED);
	lock->_click = 255;
	SWAP(obj->_click, obj->_click2);
}

void GameManager1::closeLocker(const Room *room, Object *obj, Object *lock, int section) {
	if (!obj->hasProperty(OPENED))
		_vm->renderMessage(kStringCloseLocker_1);
	else {
		_vm->renderImage(invertSection(section));
		obj->disableProperty(OPENED);
		lock->_click = lock->_click2;
		SWAP(obj->_click, obj->_click2);
	}
}

bool GameManager1::isHelmetOff() {
	Object *helmet = _inventory.get(HELMET);
	if (helmet && helmet->hasProperty(WORN)) {
		_vm->renderMessage(kStringIsHelmetOff_1);
		return false;
	}

	return true;
}

bool GameManager1::genericInteract(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && (obj1._id == SCHNUCK)) {
		if (isHelmetOff()) {
			takeObject(obj1);
			_vm->renderMessage(kStringGenericInteract_1);
			_inventory.remove(obj1);
		}
	} else if ((verb == ACTION_USE) && (obj1._id == EGG)) {
		if (isHelmetOff()) {
			takeObject(obj1);
			if (obj1.hasProperty(OPENED))
				_vm->renderMessage(kStringGenericInteract_1);
			else
				_vm->renderMessage(kStringGenericInteract_2);

			_inventory.remove(obj1);
		}
	} else if ((verb == ACTION_OPEN) && (obj1._id == EGG)) {
		takeObject(obj1);
		if (obj1.hasProperty(OPENED))
			_vm->renderMessage(kStringGenericInteract_3);
		else {
			takeObject(*_rooms[ENTRANCE]->getObject(8));
			_vm->renderMessage(kStringGenericInteract_4);
			obj1.setProperty(OPENED);
		}
	} else if ((verb == ACTION_USE) && (obj1._id == PILL)) {
		if (isHelmetOff()) {
			_vm->renderMessage(kStringGenericInteract_5);
			great(0);
			_inventory.remove(obj1);
			_state._language = 2;
			takeObject(*_rooms[ENTRANCE]->getObject(17));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == PILL_HULL) &&
			   (_state._language == 2)) {
		_vm->renderMessage(kStringGenericInteract_6);
		_state._language = 1;
	} else if ((verb == ACTION_OPEN) && (obj1._id == WALLET)) {
		if (!_rooms[ROGER]->getObject(3)->hasProperty(CARRIED))
			_vm->renderMessage(kStringGenericInteract_7);
		else if (_rooms[ROGER]->getObject(7)->hasProperty(CARRIED))
			_vm->renderMessage(kStringGenericInteract_8);
		else {
			_vm->renderMessage(kStringGenericInteract_9);
			takeObject(*_rooms[ROGER]->getObject(7));
			takeObject(*_rooms[ROGER]->getObject(8));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == NEWSPAPER)) {
		animationOff();
		saveTime();

		_vm->renderMessage(kStringGenericInteract_10);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->renderMessage(kStringGenericInteract_11);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->setCurrentImage(2);
		_vm->renderImage(0);
		_vm->setColor63(40);
		getInput();
		_vm->renderRoom(*_currentRoom);
		roomBrightness();
		_vm->renderMessage(kStringGenericInteract_12);

		loadTime();
		animationOn();
	} else if ((verb == ACTION_LOOK) && (obj1._id == KEYCARD2)) {
		_vm->renderMessage(obj1._description);
		obj1._description = kStringKeycard2Description2;
	} else if ((verb == ACTION_LOOK) && (obj1._id == WATCH))
		_vm->renderMessage(kStringGenericInteract_13, kMessageNormal, timeToString(_time), timeToString(_state._timeAlarm));
	else if ((verb == ACTION_PRESS) && (obj1._id == WATCH)) {
		bool validInput = true;
		int hours = 0;
		int minutes = 0;

		animationOff();
		_vm->saveScreen(88, 87, 144, 24);
		_vm->renderBox(88, 87, 144, 24, kColorWhite35);
		_vm->renderText(kStringGenericInteract_14, 91, 90, kColorWhite99);
		Common::String input;
		do {
			validInput = true;
			input.clear();
			_vm->renderBox(91, 99, 138, 9, kColorDarkBlue);
			edit(input, 91, 100, 5);

			int seperator = -1;
			for (uint i = 0; i < input.size(); ++i) {
				if (input[i] == ':') {
					seperator = i;
					break;
				}
			}
			if ((seperator == -1) || (seperator > 2)) {
				validInput = false;
				continue;
			}

			int decimalPlace = 1;
			for (int i = 0; i < seperator; ++i) {
				if (Common::isDigit(input[i])) {
					hours = hours * decimalPlace + (input[i] - '0');
					decimalPlace *= 10;
				} else {
					validInput = false;
					break;
				}
			}
			decimalPlace = 1;
			for (uint i = seperator + 1; i < input.size(); ++i) {
				if (Common::isDigit(input[i])) {
					minutes = minutes * decimalPlace + (input[i] - '0');
					decimalPlace *= 10;
				} else {
					validInput = false;
					break;
				}
			}
			if ((hours > 23) || (minutes > 59))
				validInput = false;

			animationOn();
		} while (!validInput && (_key.keycode != Common::KEYCODE_ESCAPE));

		_vm->restoreScreen();
		if (_key.keycode != Common::KEYCODE_ESCAPE) {
			_state._timeAlarm = (hours * 60 + minutes) * 60 * 1000;
			_state._alarmOn = (_state._timeAlarm > _time);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, WIRE)) {
		Room *r = _rooms[CABIN_L3];
		if (!r->getObject(8)->hasProperty(CARRIED)) {
			if (r->isSectionVisible(26))
				_vm->renderMessage(kStringTakeMessage);
			else
				return false;
		} else {
			r->getObject(8)->_name = kStringWireAndClip;
			r = _rooms[HOLD];
			_inventory.remove(*r->getObject(2));
			_state._terminalStripConnected = true;
			_state._terminalStripWire = true;
			_vm->renderMessage(kStringOk);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, SPOOL)) {
		Room *r = _rooms[CABIN_L2];
		takeObject(*r->getObject(9));
		r->getObject(9)->_name = kSringSpoolAndClip;
		r = _rooms[HOLD];
		_inventory.remove(*r->getObject(2));
		_state._terminalStripConnected = true;
		_vm->renderMessage(kStringOk);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SPOOL)) {
		Room *r = _rooms[CABIN_L3];
		if (!_state._terminalStripConnected) {
			if (r->isSectionVisible(26))
				_vm->renderMessage(kStringCable3);
			else
				return false;
		} else {
			if (!r->getObject(8)->hasProperty(CARRIED))
				_vm->renderMessage(kStringTakeMessage);
			else {
				r = _rooms[CABIN_L2];
				takeObject(*r->getObject(9));
				r = _rooms[CABIN_L3];
				r->getObject(8)->_name = kStringGeneratorWire;
				r = _rooms[CABIN_L2];
				_inventory.remove(*r->getObject(9));
				_state._cableConnected = true;
				_vm->renderMessage(kStringOk);
			}
		}
	} else if ((verb == ACTION_USE) && (obj1._id == SUIT)) {
		takeObject(obj1);
		if ((_currentRoom->getId() >= ENTRANCE) && (_currentRoom->getId() <= ROGER)) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage(kStringGenericInteract_15);
				_rooms[AIRLOCK]->getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(6)->disableProperty(WORN);
			} else
				_vm->renderMessage(kStringGenericInteract_16);
		} else {
			if (obj1.hasProperty(WORN)) {
				Room *r = _rooms[AIRLOCK];
				if (r->getObject(4)->hasProperty(WORN))
					_vm->renderMessage(kStringGenericInteract_17);
				else if (r->getObject(6)->hasProperty(WORN))
					_vm->renderMessage(kStringGenericInteract_18);
				else {
					obj1.disableProperty(WORN);
					_vm->renderMessage(kStringGenericInteract_19);
				}
			} else {
				obj1.setProperty(WORN);
				_vm->renderMessage(kStringGenericInteract_20);
			}
		}
	} else if ((verb == ACTION_USE) && (obj1._id == HELMET)) {
		takeObject(obj1);
		if ((_currentRoom->getId() >= ENTRANCE) && (_currentRoom->getId() <= ROGER)) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage(kStringGenericInteract_21);
				_rooms[AIRLOCK]->getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(6)->disableProperty(WORN);
			} else
				_vm->renderMessage(kStringGenericInteract_22);
		} else if (obj1.hasProperty(WORN)) {
			obj1.disableProperty(WORN);
			_vm->renderMessage(kStringGenericInteract_24);
			getInput();
			if (airless())
				dead(kStringGenericInteract_23);
		} else {
			Room *r = _rooms[AIRLOCK];
			if (r->getObject(5)->hasProperty(WORN)) {
				obj1.setProperty(WORN);
				_vm->renderMessage(kStringGenericInteract_25);
			} else
				_vm->renderMessage(kStringGenericInteract_26);
		}
	} else if ((verb == ACTION_USE) && (obj1._id == LIFESUPPORT)) {
		takeObject(obj1);
		if ((_currentRoom->getId() >= ENTRANCE) && (_currentRoom->getId() <= ROGER)) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage(kStringGenericInteract_21);
				_rooms[AIRLOCK]->getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(6)->disableProperty(WORN);
			} else
				_vm->renderMessage(kStringGenericInteract_22);
		} else if (obj1.hasProperty(WORN)) {
			obj1.disableProperty(WORN);
			_vm->renderMessage(kStringGenericInteract_28);
			getInput();
			if (airless())
				dead(kStringGenericInteract_27);
		} else {
			Room *r = _rooms[AIRLOCK];
			if (r->getObject(5)->hasProperty(WORN)) {
				obj1.setProperty(WORN);
				_vm->renderMessage(kStringGenericInteract_29);
			} else
				_vm->renderMessage(kStringGenericInteract_26);
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == BATHROOM_DOOR)) {
		_rooms[BATHROOM]->getObject(2)->_exitRoom = _currentRoom->getId();
		return false;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SOCKET))
		_vm->renderMessage(kStringGenericInteract_30);
	else if ((verb == ACTION_LOOK) && (obj1._id == BOOK2)) {
		_vm->renderMessage(kStringGenericInteract_31);
		wait(_messageDuration, true);
		_vm->removeMessage();
		_vm->renderMessage(kStringGenericInteract_32);
	} else
		return false;

	return true;
}

void GameManager1::handleInput() {
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
				_vm->renderMessage(kStringGenericInteract_33);
			} else if (!_inputObject[0]->hasProperty(EXIT)) {
				// You're already there.
				_vm->renderMessage(kStringGenericInteract_34);
			} else if (_inputObject[0]->hasProperty(OPENABLE) && !_inputObject[0]->hasProperty(OPENED)) {
				// This is closed
				_vm->renderMessage(kStringShipHold9);
			} else
				changeRoom(_inputObject[0]->_exitRoom);

			break;

		case ACTION_TAKE:
			if (_inputObject[0]->hasProperty(OPENED)) {
				// You already have that
				_vm->renderMessage(kStringGenericInteract_35);
			} else if (_inputObject[0]->hasProperty(UNNECESSARY)) {
				// You do not need that.
				_vm->renderMessage(kStringGenericInteract_36);
			} else if (!_inputObject[0]->hasProperty(TAKE)) {
				// You can't take that.
				_vm->renderMessage(kStringGenericInteract_37);
			} else
				takeObject(*_inputObject[0]);

			break;

		case ACTION_OPEN:
			if (!_inputObject[0]->hasProperty(OPENABLE)) {
				// This can't be opened
				_vm->renderMessage(kStringGenericInteract_38);
			} else if (_inputObject[0]->hasProperty(OPENED)) {
				// This is already opened.
				_vm->renderMessage(kStringGenericInteract_39);
			} else if (_inputObject[0]->hasProperty(CLOSED)) {
				// This is locked.
				_vm->renderMessage(kStringGenericInteract_40);
			} else {
				_vm->renderImage(_inputObject[0]->_section);
				_inputObject[0]->setProperty(OPENED);
				byte i = _inputObject[0]->_click;
				_inputObject[0]->_click  = _inputObject[0]->_click2;
				_inputObject[0]->_click2 = i;
				_sound->play(kAudioDoorOpen);
			}
			break;

		case ACTION_CLOSE:
			if (!_inputObject[0]->hasProperty(OPENABLE) ||
				(_inputObject[0]->hasProperty(CLOSED) &&
				 _inputObject[0]->hasProperty(OPENED))) {
				// This can't be closed.
				_vm->renderMessage(kStringGenericInteract_41);
			} else if (!_inputObject[0]->hasProperty(OPENED)) {
				// This is already closed.
				_vm->renderMessage(kStringCloseLocker_1);
			} else {
				_vm->renderImage(invertSection(_inputObject[0]->_section));
				_inputObject[0]->disableProperty(OPENED);
				byte i = _inputObject[0]->_click;
				_inputObject[0]->_click  = _inputObject[0]->_click2;
				_inputObject[0]->_click2 = i;
				_sound->play(kAudioDoorClose);
			}
			break;

		case ACTION_GIVE:
			if (_inputObject[0]->hasProperty(CARRIED)) {
				// Better keep it!
				_vm->renderMessage(kStringGenericInteract_42);
			}
			break;

		default:
			// This is not possible.
			_vm->renderMessage(kStringGenericInteract_43);
		}
	}
}

void GameManager1::executeRoom() {
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
		drawMapExits();
		drawInventory();
		drawStatus();
		drawCommandBox();
	}

	roomBrightness();
	if (_vm->_screen->getViewportBrightness() == 0)
		_vm->paletteFadeIn();

	if (!_currentRoom->hasSeen() && _newRoom) {
		_newRoom = false;
		_currentRoom->onEntrance();
	}
}

void GameManager1::guardShot() {
	_vm->renderImage(2);
	_vm->renderImage(5);
	wait(3);
	_vm->renderImage(2);

	_sound->play(kAudioVoiceHalt);
	while (_sound->isPlaying())
		wait(1);

	_vm->renderImage(5);
	wait(5);
	_vm->renderImage(3);
	wait(3);

	shot(4, 3);
}

void GameManager1::guard3Shot() {
	_vm->renderImage(1);
	wait(3);
	_sound->play(kAudioVoiceHalt); // 46/0
	while (_sound->isPlaying())
		wait(1);

	wait(5);
	_vm->renderImage(2);
	wait(3);
	shot(3,2);
}

void GameManager1::alarm() {
	if (_rooms[INTRO1]->getObject(2)->hasProperty(CARRIED)) {
		alarmSound();
		if (_currentRoom->getId() == GUARD)
			guardShot();
		else if (_currentRoom->getId() == CORRIDOR4 || _currentRoom->getId() == CORRIDOR7) {
			guardNoticed();
			_state._corridorSearch = true;
		} else if (_currentRoom->getId() == GUARD3)
			guard3Shot();
		else if (_currentRoom->getId() == CORRIDOR1)
			busted(33);
	} else {
		if (_currentRoom->getId() == CORRIDOR2 || _currentRoom->getId() == CORRIDOR4 ||
			_currentRoom->getId() == GUARD     || _currentRoom->getId() == CORRIDOR7 ||
			_currentRoom->getId() == CELL)
		{
			alarmSound();
			if (_currentRoom->getId() == GUARD)
				guardShot();
			guardNoticed();
			if (_currentRoom->getId() == CORRIDOR4)
				_state._corridorSearch = true;
		}
		_rooms[GUARD]->setSectionVisible(1, true);
		_rooms[GUARD]->getObject(3)->_click = 255;
		if (!_rooms[GUARD]->getObject(5)->hasProperty(CARRIED)) {
			_rooms[GUARD]->setSectionVisible(7, true);
			_rooms[GUARD]->getObject(5)->_click = 4;
		}
		_state._eventTime = _time + ticksToMsec(180);
		_state._eventCallback = kGuardReturnedFn;
	}
}

void GameManager1::alarmSound() {
	animationOff();
	_vm->removeMessage();
	_vm->renderMessage(kStringAlarm);

	int32 end = _time + ticksToMsec(_messageDuration);
	do {
		_sound->play(kAudioAlarm);
		while (_sound->isPlaying()) {
			g_system->delayMillis(_vm->_delay);
			updateEvents();
			g_system->updateScreen();
		}
	} while (_time < end && !_vm->shouldQuit());

	_vm->removeMessage();
	animationOn();
}

}
