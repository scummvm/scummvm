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
#include "supernova/supernova.h"
#include "supernova/state.h"

namespace Supernova {

Inventory::Inventory()
    : _numObjects(0)
{}

// TODO: Update Inventory surface for scrolling
void Inventory::add(Object &obj) {
	if (_numObjects < kMaxCarry)
		_inventory[_numObjects++] = &obj;

//	if (inventory_amount>8) inventory_scroll = ((inventory_amount+1)/2)*2-8;
//	show_inventory();
}

// TODO: Update Inventory surface for scrolling
void Inventory::remove(Object &obj) {
	for (int i = 0; i < _numObjects; ++i) {
		if (_inventory[i] == &obj) {
			--_numObjects;
			while (i < _numObjects) {
				_inventory[i] = _inventory[i + 1];
				++i;
			}
			obj.disableProperty(CARRIED);
		}
	}
}

void Inventory::clear() {
	_numObjects = 0;
}

Object *Inventory::get(int index) const {
	if (index < _numObjects)
		return _inventory[index];

	return const_cast<Object *>(&Object::nullObject);
}

Object *Inventory::get(ObjectID id) const {
	for (int i = 0; i < _numObjects; ++i) {
		if (_inventory[i]->_id == id)
			return _inventory[i];
	}

	return const_cast<Object *>(&Object::nullObject);
}


GuiElement::GuiElement()
    : _text("")
    , _isHighlighted(false)
    , _bgColorNormal(kColorWhite25)
    , _bgColorHighlighted(kColorWhite44)
    , _bgColor(kColorWhite25)
    , _textColorNormal(kColorGreen)
    , _textColorHighlighted(kColorLightGreen)
    , _textColor(kColorGreen)
{}

void GuiElement::setText(const char *text) {
	strncpy(_text, text, sizeof(_text));
}

void GuiElement::setTextPosition(int x, int y) {
	_textPosition.x = x;
	_textPosition.y = y;
}
void GuiElement::setSize(int x1, int y1, int x2, int y2) {
	this->left = x1;
	this->top = y1;
	this->right = x2;
	this->bottom = y2;

	_textPosition.x = x1 + 1;
	_textPosition.y = y1 + 1;
}

void GuiElement::setColor(int bgColor, int textColor, int bgColorHighlighted, int textColorHightlighted) {
	_bgColor = bgColor;
	_textColor = textColor;
	_bgColorNormal = bgColor;
	_textColorNormal = textColor;
	_bgColorHighlighted = bgColorHighlighted;
	_textColorHighlighted = textColorHightlighted;
}

void GuiElement::setHighlight(bool isHighlighted) {
	if (isHighlighted) {
		_bgColor = _bgColorHighlighted;
		_textColor = _textColorHighlighted;
	} else {
		_bgColor = _bgColorNormal;
		_textColor = _textColorNormal;
	}
}


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

GameManager::GameManager(SupernovaEngine *vm) {
	_vm = vm;

	initRooms();
	_currentRoom = _rooms[SLEEP];
	initState();
	initGui();
}

GameManager::~GameManager() {
	destroyRooms();
}

void GameManager::destroyRooms() {
	delete _rooms[INTRO];
	delete _rooms[CORRIDOR];
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
	delete _rooms[SIGN];
}


void GameManager::initState() {
	Object::setObjectNull(_currentInputObject);
	Object::setObjectNull(_inputObject[0]);
	Object::setObjectNull(_inputObject[1]);
	_inputVerb = ACTION_WALK;
	_processInput = false;
	_guiEnabled = true;
	_animationEnabled = true;
	_mouseField = -1;
	_inventoryScroll = 0;
	_timer1 = 0;
	_animationTimer = 0;

	_state._time = 0;
	_state._timeSleep = 0;
	_state._timeStarting = ticksToMsec(916364); // 2 pm
	_state._timeAlarm = ticksToMsec(458182);    // 7 am
	_state._timeAlarmSystem = _state._timeAlarm + _state._timeStarting;
	_state._eventTime = 0xffffffff;
	_state._arrivalDaysLeft = 2840;
	_state._shipEnergyDaysLeft = 2135;
	_state._landingModuleEnergyDaysLeft = 923;
	_state._greatFlag = 0;
	_state._timeRobot = 0;
	_state._money = 0;
	_state._coins = 0;
	_state._shoes = 0;
	_state._nameSeen = 0;
	_state._destination = 255;
	_state._benOverlay = 0;
	_state._language = 0;
	_state._corridorSearch = false;
	_state._alarmOn = false;
	_state._terminalStripConnected = false;
	_state._terminalStripWire = false;
	_state._cableConnected = false;
	_state._powerOff = false;
	_state._cockpitSeen = false;
	_state._airlockSeen = false;
	_state._holdSeen = false;
	_state._dream = false;
}

void GameManager::initRooms() {
	_rooms[INTRO] = new StartingItems(_vm, this);
	_rooms[CORRIDOR] = new ShipCorridor(_vm, this);
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
	_rooms[CORRIDOR4] = new AxacussCorridor3(_vm, this);
	_rooms[CORRIDOR5] = new AxacussCorridor4(_vm, this);
	_rooms[CORRIDOR6] = new AxacussCorridor5(_vm, this);
	_rooms[CORRIDOR7] = new AxacussCorridor6(_vm, this);
	_rooms[CORRIDOR8] = new AxacussCorridor7(_vm, this);
	_rooms[CORRIDOR9] = new AxacussCorridor8(_vm, this);
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
	_rooms[SIGN] = new AxacussSign(_vm, this);
}

void GameManager::initGui() {
	int commandButtonX = 0;
	for (int i = 0; i < ARRAYSIZE(_guiCommandButton); ++i) {
		int width;
		if (i < 9)
			width = _vm->textWidth(guiCommand_DE[i]) + 2;
		else
			width = 320 - commandButtonX;

		_guiCommandButton[i].setSize(commandButtonX, 150, commandButtonX + width, 159);
		_guiCommandButton[i].setText(guiCommand_DE[i]);
		_guiCommandButton[i].setColor(kColorWhite25, kColorDarkGreen, kColorWhite44, kColorGreen);
		commandButtonX += width + 2;
	}

	for (int i = 0; i < ARRAYSIZE(_guiInventory); ++i) {
		int inventoryX = 136 * (i % 2);
		int inventoryY = 161 + 10 * (i / 2);

		_guiInventory[i].setSize(inventoryX, inventoryY, inventoryX + 135, inventoryY + 9);
		_guiInventory[i].setColor(kColorWhite25, kColorDarkRed, kColorWhite35, kColorRed);
	}
}


void GameManager::processInput(Common::KeyState &state) {
	_key = state;

	switch (state.keycode) {
	case Common::KEYCODE_F1:
		// help
		break;
	case Common::KEYCODE_F2:
		// show game doc
		break;
	case Common::KEYCODE_F3:
		// show game info
		break;
	case Common::KEYCODE_F4:
		// set text speed
		break;
	case Common::KEYCODE_F5:
		// load/save
		break;
	case Common::KEYCODE_x:
		if (state.flags & Common::KBD_ALT) {
			// quit game
			_vm->_gameRunning = false;
		}
		break;
	default:
		break;
	}
}

void GameManager::resetInputState() {
	Object::setObjectNull(_inputObject[0]);
	Object::setObjectNull(_inputObject[1]);
	_inputVerb = ACTION_WALK;
	_processInput = false;
	_mouseClicked = false;
	_keyPressed = false;
	_key.reset();
	_mouseClickType = Common::EVENT_MOUSEMOVE;

	processInput();
}

void GameManager::processInput() {
	if (_mouseClickType == Common::EVENT_LBUTTONUP) {
		_vm->removeMessage();

		if (((_mouseField >= 0) && (_mouseField < 256)) ||
		    ((_mouseField >= 512) && (_mouseField < 768))) {
			if (_inputVerb == ACTION_GIVE || _inputVerb == ACTION_USE) {
				if (Object::isNullObject(_inputObject[0])) {
					_inputObject[0] = _currentInputObject;
					if (!_inputObject[0]->hasProperty(COMBINABLE))
						_processInput = true;
				} else {
					_inputObject[1] = _currentInputObject;
					_processInput = true;
				}
			} else {
				_inputObject[0] = _currentInputObject;
				if (!Object::isNullObject(_currentInputObject))
					_processInput = true;
			}
		} else if (_mouseField >= 256 && _mouseField < 512) {
			resetInputState();
			_inputVerb = static_cast<Action>(_mouseField - 256);
		} else if (_mouseField == 768) {
			if (_inventoryScroll >= 2)
				_inventoryScroll -= 2;
		} else if (_mouseField == 769) {
			_inventoryScroll += 2;
		}
	} else if (_mouseClickType == Common::EVENT_RBUTTONUP) {
		_vm->removeMessage();
		if (Object::isNullObject(_currentInputObject))
			return;

		if (((_mouseField >= 0) && (_mouseField < 256)) ||
		    ((_mouseField >= 512) && (_mouseField < 768))) {
			_inputObject[0] = _currentInputObject;
			ObjectType type = _inputObject[0]->_type;
			if (type & OPENABLE) {
				if (type & OPENED)
					_inputVerb = ACTION_CLOSE;
				else
					_inputVerb = ACTION_OPEN;
			} else if (type & PRESS) {
				_inputVerb = ACTION_PRESS;
			} else if (type & TALK) {
				_inputVerb = ACTION_TALK;
			} else {
				_inputVerb = ACTION_LOOK;
			}

			_processInput = true;
		}

	} else if (_mouseClickType == Common::EVENT_MOUSEMOVE) {
		int field = -1;
		int click = -1;

		/* command row? */
		if ((_mouseY >= _guiCommandButton[0].top) && (_mouseY <= _guiCommandButton[0].bottom)) {
			field = 9;
			while (_mouseX < _guiCommandButton[field].left - 1)
				field--;
			field += 256;
		}
		/* exit box? */
		else if ((_mouseX >= 283) && (_mouseX <= 317) && (_mouseY >= 163) && (_mouseY <= 197)) {
			field = _exitList[(_mouseX - 283) / 7 + 5 * ((_mouseY - 163) / 7)];
		}
		/* inventory box */
		else if ((_mouseY >= 161) && (_mouseX <= 270)) {
			field = (_mouseX + 1) / 136 + ((_mouseY - 161) / 10) * 2;
			if (field + _inventoryScroll < _inventory.getSize())
				field += 512;
			else
				field = -1;
		}
		/* inventory arrows */
		else if ((_mouseY >= 161) && (_mouseX >= 271) && (_mouseX < 279)) {
			if (_mouseY > 180)
				field = 769;
			else
				field = 768;
		}
		/* normal item */
		else {
			for (int i = 0; (_currentRoom->getObject(i)->_name.c_str()[0] != '\0') && (field == -1) && i < kMaxObject; i++) {
				click = _currentRoom->getObject(i)->_click;
				if (click != 255) {
					MSNImageDecoder::ClickField *clickField = _vm->_currentImage->_clickField;
					do {
						if ((_mouseX >= clickField[click].x1) && (_mouseX <= clickField[click].x2) &&
						    (_mouseY >= clickField[click].y1) && (_mouseY <= clickField[click].y2))
							field = i;

						click = clickField[click].next;
					} while ((click != 0) && (field == -1));
				}
			}
//			if ((_objectNumber == 1) && (_currentRoom->getObject(field) == _currentInputObject))
//				field = -1;
		}

		if (_mouseField != field) {
			if (_mouseField >= 768) {
				inventory_arrow(_mouseField - 768, false);
			} else if (_mouseField >= 512) {
				_guiInventory[_mouseField - 512].setHighlight(false);
			} else if (_mouseField >= 256) {
				_guiCommandButton[_mouseField - 256].setHighlight(false);
			} else if (_mouseField !=  -1) {
			}
			Object::setObjectNull(_currentInputObject);

			_mouseField = field;
			if (_mouseField >= 768) {
				inventory_arrow(_mouseField - 768, true);
			} else if (_mouseField >= 512) {
				_guiInventory[_mouseField - 512].setHighlight(true);
				_currentInputObject = _inventory.get(_mouseField - 512 + _inventoryScroll);
			} else if (_mouseField >= 256) {
				_guiCommandButton[_mouseField - 256].setHighlight(true);
			} else if (_mouseField !=  -1) {
				_currentInputObject = _currentRoom->getObject(_mouseField);
			}
		}
	}
}

void GameManager::drawImage(int section) {
	bool sectionVisible = true;

	if (section > 128) {
		sectionVisible = false;
		section -= 128;
	}

	do {
		_currentRoom->setSectionVisible(section, sectionVisible);
		if (sectionVisible)
			_vm->renderImage(_currentRoom->getFileNumber(), section);
		else
			_vm->renderImage(_currentRoom->getFileNumber(), section + 128);
		section = _vm->_currentImage->_section[section].next;
	} while (section != 0);
}

void GameManager::corridorOnEntrance() {
	if (_state._corridorSearch)
		busted(0);
}

void busted(int i) {
	// STUB
}

void GameManager::telomat(int number) {
	// STUB
}

void GameManager::startSearch() {
	if ((_currentRoom >= _rooms[CORRIDOR1]) && (_currentRoom <= _rooms[BCORRIDOR]))
		busted(0);

	_state._corridorSearch = true;
}

void GameManager::search(int time) {
	_state._eventTime = _state._time + time;
//	*event = &search_start;
}

void GameManager::guardNoticed() {
	// STUB
}

void GameManager::busted(int i) {
	if (i > 0)
		drawImage(i);
	if (i == 0) {
		if ((_currentRoom >= _rooms[OFFICE_L1]) && (_currentRoom <= _rooms[OFFICE_R2])) {
			if (_currentRoom < _rooms[OFFICE_R1])
				i = 10;
			else
				i = 5;
			if (!_currentRoom->getObject(0)->hasProperty(OPENED)) {
				drawImage(i - 1);
				_vm->playSound(kAudioDoorOpen);
				wait2(2);
			}
			drawImage(i);
			wait2(3);
			drawImage(i + 3);
			_vm->playSound(kAudioVoiceHalt);
			drawImage(i);
			wait2(5);
			if (_currentRoom == _rooms[OFFICE_L2])
				i = 13;
			drawImage(i + 1);
			wait2(3);
			drawImage(i + 2);
			shot(0, 0);
		} else if (_currentRoom == _rooms[BCORRIDOR]) {
			drawImage(21);
		} else {
			if (_currentRoom->isSectionVisible(4))
				drawImage(32); // below
			else if (_currentRoom->isSectionVisible(2))
				drawImage(30); // right
			else if (_currentRoom->isSectionVisible(1))
				drawImage(31); // left
			else
				drawImage(33); // above
		}
	}
	_vm->playSound(kAudioVoiceHalt);
	wait2(3);
	shot(0, 0);
}

void GameManager::guardReturned() {
	if (_currentRoom == _rooms[GUARD])
		busted(-1);
	else if ((_currentRoom == _rooms[CORRIDOR9]) && (_currentRoom->isSectionVisible(27)))
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

void GameManager::taxi() {
	if (_currentRoom == _rooms[SIGN]) {
		changeRoom(STATION);
	}

	drawImage(1);
	drawImage(2);
	_vm->playSound(kAudioRocks);
	screenShake();
	drawImage(9);
	_currentRoom->getObject(1)->setProperty(OPENED);
	drawImage(1);
	_currentRoom->setSectionVisible(2, false);
	drawImage(3);
	for (int i = 4; i <= 8; i++) {
		wait2(2);
		drawImage(invertSection(i - 1));
		drawImage(i);
	}
	_rooms[SIGN]->setSectionVisible(2, false);
	_rooms[SIGN]->setSectionVisible(3, true);
}

void GameManager::outro() {
	_state._benOverlay = 3;
//	load_overlay();
//	title = 2;
	_vm->playSoundMod(49);
//	title = 0;
	_state._benOverlay = 0;
	_vm->paletteFadeOut();
	_vm->renderImage(55, 0);
	_vm->paletteFadeIn();
	getInput();
	_vm->paletteFadeOut();
	// TODO: render info file
//	longjmp(termination,1);
}

void GameManager::great(uint number) {
	if (number && (_state._greatFlag & (1 << number)))
		return;

	_vm->playSound(kAudioUndef7);
	_state._greatFlag |= 1 << number;
}

bool GameManager::airless() {
	return (
	((_currentRoom >  _rooms[AIRLOCK]) && (_currentRoom < _rooms[CABIN_R1])) ||
	((_currentRoom >  _rooms[BATHROOM])&& (_currentRoom < _rooms[ENTRANCE])) ||
	((_currentRoom == _rooms[AIRLOCK]) && (_currentRoom->getObject(1)->hasProperty(OPENED))) ||
	(_currentRoom  >= _rooms[MEETUP2])
	);
}

void GameManager::shipStart() {
	// STUB
}

void GameManager::removeSentence(int sentence, int number) {
	// STUB
}

void GameManager::addSentence(int sentence, int number) {
	// STUB
}

void GameManager::say(const char *text) {
	// STUB
}

void GameManager::reply(const char *text, int aus1, int aus2) {
	// STUB
}

int GameManager::dialog(int num, byte *rowLength[], const char **text[6], int number) {
	// STUB
	return 0;
}

void GameManager::turnOff() {
	if (_state._powerOff)
		return;

	_state._powerOff = true;
	roomBrightness();

}
void GameManager::turnOn() {
	if (!_state._powerOff)
		return;

	_state._powerOff = false;
	_vm->paletteBrightness();
	Room *room = _rooms[SLEEP];
	room->setSectionVisible(1, false);
	room->setSectionVisible(2, false);
	room = _rooms[COCKPIT];
	room->setSectionVisible(22, false);
}

void GameManager::takeObject(Object &obj) {
	if (obj.hasProperty(CARRIED))
		return;

	if (obj._section != 0)
		drawImage(obj._section);
	obj.setProperty(CARRIED);
	obj._click = obj._click2 = 255;
	_inventory.add(obj);
}

void GameManager::drawCommandBox() {
	for (int i = 0; i < ARRAYSIZE(_guiCommandButton); ++i) {
		_vm->renderBox(_guiCommandButton[i].left,
		               _guiCommandButton[i].top,
		               _guiCommandButton[i].width(),
		               _guiCommandButton[i].height(),
		               _guiCommandButton[i]._bgColor);
		_vm->renderText(_guiCommandButton[i]._text,
		                _guiCommandButton[i]._textPosition.x,
		                _guiCommandButton[i]._textPosition.y,
		                _guiCommandButton[i]._textColor);
	}
}

void GameManager::inventory_arrow(int num, bool brightness) {
	// STUB
}

void GameManager::drawInventory() {
	for (int i = 0; i < ARRAYSIZE(_guiInventory); ++i) {
		_vm->renderBox(_guiInventory[i].left,
		               _guiInventory[i].top,
		               _guiInventory[i].width(),
		               _guiInventory[i].height(),
		               _guiInventory[i]._bgColor);
		_vm->renderText(_inventory.get(i + _inventoryScroll)->_name.c_str(),
		                _guiInventory[i]._textPosition.x,
		                _guiInventory[i]._textPosition.y,
		                _guiInventory[i]._textColor);
	}

	_vm->renderBox(272, 161, 7, 19, HGR_INV);
	_vm->renderBox(272, 181, 7, 19, HGR_INV);
}

uint16 GameManager::getKeyInput(bool blockForPrintChar) {
	while (true) {
		_vm->updateEvents();
		if (_keyPressed) {
			if (blockForPrintChar) {
				if (Common::isPrint(_key.keycode) ||
				    _key.keycode == Common::KEYCODE_BACKSPACE ||
				    _key.keycode == Common::KEYCODE_DELETE ||
				    _key.keycode == Common::KEYCODE_RETURN ||
				    _key.keycode == Common::KEYCODE_SPACE ||
				    _key.keycode == Common::KEYCODE_ESCAPE ||
				    _key.keycode == Common::KEYCODE_UP ||
				    _key.keycode == Common::KEYCODE_DOWN ||
				    _key.keycode == Common::KEYCODE_LEFT ||
				    _key.keycode == Common::KEYCODE_RIGHT) {
					if (_key.flags & Common::KBD_SHIFT)
						return toupper(_key.ascii);
					else
						return tolower(_key.ascii);
				}
			} else {
				return _key.ascii;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
}

Common::EventType GameManager::getMouseInput() {
	while (true) {
		_vm->updateEvents();
		if (_mouseClicked) {
			return _mouseClickType;
		}
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
}

void GameManager::getInput() {
	while (true) {
		_vm->updateEvents();
		if (_mouseClicked || _keyPressed) {
			break;
		}
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
}

// TODO: Unify mouseInput3 and mouseWait with getMouseInput
void GameManager::mouseInput3() {
	// STUB
}
void GameManager::mouseWait(int delay) {
	// STUB
}

static void dimColor(SupernovaEngine *vm, int color) {
	color -= 16;
	color *= 3;
	// TODO: alters palette image data permanently (get system palette instead?)
	vm->_currentImage->_palette[color + 0] = vm->_currentImage->_palette[color + 0] * 3 / 5;
	vm->_currentImage->_palette[color + 1] = vm->_currentImage->_palette[color + 1] * 3 / 5;
	vm->_currentImage->_palette[color + 2] = vm->_currentImage->_palette[color + 2] * 3 / 5;
}

void GameManager::roomBrightness() {
	const byte specialColors[2][18] = {
	    {0x42, 0x49, 0x55, 0x68, 0x50, 0x5d, 0x3c, 1},
	    {0x85, 0x91, 0x99, 0x92, 0x9b, 0x96, 0x9a, 0xa6,
	     0xb0, 0xb4, 0xb5, 0xc2, 0xd1, 0xbe, 0xb6, 0xa8,
	     0x6b, 1}
	};
	char sf;
	int i;
	if ((_currentRoom == _rooms[HOLD]) && (_state._benOverlay == 1)) {
		if (_state._powerOff) {
			for (int f = 16; f < 255; f++) {
				i = 0;
				do {
					sf = specialColors[0][i] - 1;
					i++;
				} while (sf && (sf != f));
				if (!sf)
					dimColor(_vm, f);
			}
		}
		if (!(_state._landingModuleEnergyDaysLeft && _rooms[LANDINGMODULE]->isSectionVisible(7))) {
			i = 0;
			while (sf = specialColors[0][i] - 1) {
				dimColor(_vm, sf);
				i++;
			};
		}
	} else if ((_currentRoom == _rooms[LANDINGMODULE]) && (_state._benOverlay == 1)) {
		if (!(_state._landingModuleEnergyDaysLeft && _rooms[LANDINGMODULE]->isSectionVisible(7))) {
			for (int f = 16; f < 255; f++) {
				i=0;
				do {
					sf = specialColors[1][i] - 1;
					i++;
				} while (sf && (sf != f));
				if (!sf)
					dimColor(_vm, f);
			}
		}
		if (_state._powerOff) {
			i=0;
			while (sf = specialColors[1][i] - 1) {
				dimColor(_vm, sf);
				i++;
			};
		}
	} else if ((_currentRoom == _rooms[CAVE]) && (_state._benOverlay == 1)) {
		_vm->_brightness = 0;
	} else if ((_currentRoom != _rooms[OUTSIDE]) &&
	           (_currentRoom <  _rooms[ROCKS]) && (_state._benOverlay == 1)) {
		if (_state._powerOff)
			for (int f = 16; f < 255; f++)
				dimColor(_vm, f);
	} else if ((_currentRoom == _rooms[GUARD3]) && (_state._benOverlay == 2)) {
		if (_state._powerOff)
			_vm->_brightness = 0;
	}

	_vm->paletteBrightness();
}

void GameManager::loadTime() {
	_state._timeStarting += _state._time;
	if (_state._eventTime != 1)
		_state._eventTime += _state._time;
	_state._timeAlarmSystem = _state._timeAlarm + _state._timeStarting;
}

void GameManager::saveTime() {
	// STUB
}

bool GameManager::saveGame(int number) {
	// STUB
	return false;
}

void GameManager::changeRoom(RoomID id) {
	_currentRoom = _rooms[id];
}

void GameManager::errorTemp() {
	// STUB
}

void GameManager::wait2(int ticks) {
	uint end = g_system->getMillis() + ticksToMsec(ticks);
	while (g_system->getMillis() < end) {
		_vm->updateEvents();
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
}

void GameManager::setAnimationTimer(int ticks) {
	_animationTimer = g_system->getMillis() + ticksToMsec(ticks);
}

void GameManager::handleTime() {
	_state._time = g_system->getMillis();
	if (_animationTimer <= _state._time)
		_animationTimer = 0;
}

void GameManager::screenShake() {
	// STUB
}

void GameManager::palette() {
	// STUB
	// Adjust palette to brightness parameters and make it current
}

void GameManager::shock() {
	_vm->playSound(kAudioShock);
	death("Du h\204ttest besser vorher|den Stecker rausgezogen.");
}

void GameManager::showMenu() {
	_vm->renderBox(0, 138, 320, 62, 0);
	_vm->renderBox(0, 140, 320, 9, HGR_BEF_ANZ);
	drawCommandBox();
	_vm->renderBox(281, 161, 39, 39, HGR_AUSG);
	drawInventory();
}

void GameManager::drawMapExits() {
// TODO: Preload _exitList on room entry instead on every call
	_vm->renderBox(281, 161, 39, 39, HGR_AUSG);

	for (int i = 0; i < 25; i++)
		_exitList[i] = -1;
	for (int i = 0; i < kMaxObject; i++) {
		if (_currentRoom->getObject(i)->hasProperty(EXIT)) {
			byte r = _currentRoom->getObject(i)->_direction;
			_exitList[r] = i;
			int x = 284 + 7 * (r % 5);
			int y = 164 + 7 * (r / 5);
			_vm->renderBox(x, y, 5, 5, COL_AUSG);
		}
	}
}

void GameManager::animationOff() {
	_animationEnabled = false;
}

void GameManager::animationOn() {
	_animationEnabled = true;
}

void GameManager::edit(Common::String &input, int x, int y, uint length) {
	bool isEditing = true;
	uint cursorIndex = input.size();
	// NOTE: Pixels for char needed = kFontWidth + 2px left and right side bearing
	int overdrawWidth = ((length + 1) * (kFontWidth + 2) > kScreenWidth - x) ?
	                    kScreenWidth - x : (length + 1) * (kFontWidth + 2);

	while (isEditing) {
		_vm->_textCursorX = x;
		_vm->_textCursorY = y;
		_vm->_textColor = COL_EDIT;
		_vm->renderBox(x, y - 1, overdrawWidth, 9, HGR_EDIT);
		for (uint i = 0; i < input.size(); ++i) {
			// Draw char highlight depending on cursor position
			if (i == cursorIndex) {
				_vm->renderBox(_vm->_textCursorX, y - 1, _vm->textWidth(input[i]), 9, COL_EDIT);
				_vm->_textColor = HGR_EDIT;
				_vm->renderText(input[i]);
				_vm->_textColor = COL_EDIT;
			} else {
				_vm->renderText(input[i]);
			}
		}
		if (cursorIndex == input.size()) {
			_vm->renderBox(_vm->_textCursorX + 1, y - 1, 6, 9, HGR_EDIT);
			_vm->renderBox(_vm->_textCursorX    , y - 1, 1, 9, COL_EDIT);
		}

		getKeyInput(true);
		switch (_key.keycode) {
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_ESCAPE:
			isEditing = false;
			break;
		case Common::KEYCODE_UP:
		case Common::KEYCODE_DOWN:
			cursorIndex = input.size();
			break;
		case Common::KEYCODE_LEFT:
			if (cursorIndex != 0) {
				--cursorIndex;
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (cursorIndex != input.size()) {
				++cursorIndex;
			}
			break;
		case Common::KEYCODE_DELETE:
			if (cursorIndex != input.size()) {
				input.deleteChar(cursorIndex);
			}
			break;
		case Common::KEYCODE_BACKSPACE:
			if (cursorIndex != 0) {
				--cursorIndex;
				input.deleteChar(cursorIndex);
			}
			break;
		default:
			if (Common::isPrint(_key.ascii) && input.size() < length) {
				input.insertChar(_key.ascii, cursorIndex);
				++cursorIndex;
			}
			break;
		}
	}
}

void GameManager::loadOverlayStart() {
	// STUB
}

void GameManager::shot(int a, int b) {
	if (a)
		drawImage(a);
	_vm->playSound(kAudioGunShot);
	wait2(2);
	if (b)
		drawImage(b);
	wait2(2);
	if (a)
		drawImage(a);
	_vm->playSound(kAudioGunShot);
	wait2(2);
	if (b)
		drawImage(b);

	death("Der Axacussaner hat dich erwischt.");
}

void GameManager::takeMoney(int amount) {
	_state._money += amount;
	if (amount > 0)
		great(0);
	// TODO: kmaxobject - 1?
//	_rooms[OFFICE_R1]->getObject(5)->_name = _rooms[OFFICE_R1]->getObject(kMaxObject - 1);
//	raumz[OFFICE_R1]->object[5].name = &(raumz[OFFICE_R1]->object[MAX_OBJECT-1]);
//	strcpy(raumz[OFFICE_R1]->object[5].name,ltoa((long)_state.money));
//	strcat(raumz[OFFICE_R1]->object[5].name," Xa");

	if (_state._money) {
		if (!_rooms[OFFICE_R1]->getObject(5)->hasProperty(CARRIED))
			takeObject(*_rooms[OFFICE_R1]->getObject(5));
	} else {
		_inventory.remove(*_rooms[OFFICE_R1]->getObject(5));
	}
}

void GameManager::drawStatus() {
	int index = static_cast<int>(_inputVerb);
	_vm->renderBox(0, 140, 320, 9, kColorWhite25);
	_vm->renderText(guiStatusCommand_DE[index], 1, 141, kColorDarkGreen);

	if (Object::isNullObject(_inputObject[0])) {
		_vm->renderText(_currentInputObject->_name.c_str());
	} else {
		_vm->renderText(_inputObject[0]->_name.c_str());
		if (_inputVerb == ACTION_GIVE) {
			_vm->renderText(" an ");
		} else if (_inputVerb == ACTION_USE) {
			_vm->renderText(" mit ");
		}

		_vm->renderText(_currentInputObject->_name.c_str());
	}
}

void GameManager::openLocker(const Room *room, Object *obj, Object *lock, int section) {
	drawImage(section);
	obj->setProperty(OPENED);
	lock->_click = 255;
	int i = obj->_click;
	obj->_click = obj->_click2;
	obj->_click2 = i;
}

void GameManager::closeLocker(const Room *room, Object *obj, Object *lock, int section) {
	if (!obj->hasProperty(OPENED)) {
		_vm->renderMessage("Das ist schon geschlossen.");
	} else {
		drawImage(invertSection(section));
		obj->disableProperty(OPENED);
		lock->_click = lock->_click2;
		int i = obj->_click;
		obj->_click = obj->_click2;
		obj->_click2 = i;
	}
}

void GameManager::death(const char *message) {
	_vm->paletteFadeOut();
	_guiEnabled = false;
	_vm->renderImage(11, 0);
	_vm->renderMessage(message);
	_vm->playSound(kAudioDeath);
	_vm->paletteFadeIn();
	getInput();
	_vm->paletteFadeOut();
	_vm->removeMessage();

	// TODO: Load screen
	destroyRooms();
	initRooms();
	initState();
	initGui();
	_inventory.clear();
	changeRoom(INTRO);
	g_system->fillScreen(kColorBlack);
	_vm->paletteFadeIn();

	_guiEnabled = true;
}

int GameManager::invertSection(int section) {
	if (section < 128)
		section += 128;
	else
		section -= 128;

	return section;
}

bool GameManager::isHelmetOff() {
	Object *helmet = _inventory.get(HELMET);
	if (helmet && helmet->hasProperty(WORN)) {
		_vm->renderMessage("Irgendwie ist ein Raumhelm|beim Essen unpraktisch.");
		return false;
	}

	return true;
}

bool GameManager::genericInteract(Action verb, Object &obj1, Object &obj2) {
	Room *r;
	char t[150];
	Common::String input;

	if ((verb == ACTION_USE) && (obj1._id == SCHNUCK)) {
		if (isHelmetOff()) {
			takeObject(obj1);
			_vm->renderMessage("Schmeckt ganz gut.");
			_inventory.remove(obj1);
		}
	} else if ((verb == ACTION_USE) && (obj1._id == EGG)) {
		if (isHelmetOff()) {
			takeObject(obj1);
			if (obj1.hasProperty(OPENED))
				_vm->renderMessage("Schmeckt ganz gut.");
			else
				_vm->renderMessage("Da war irgendetwas drin,|aber jetzt hast du es|mit runtergeschluckt.");

			_inventory.remove(obj1);
		}
	} else if ((verb == ACTION_OPEN) && (obj1._id == EGG)) {
		takeObject(obj1);
		if (obj1.hasProperty(OPENED)) {
			_vm->renderMessage("Du hast es doch schon ge\224ffnet.");
		} else {
			takeObject(*_rooms[ENTRANCE]->getObject(8));
			_vm->renderMessage("In dem Ei ist eine Tablette|in einer Plastikh\201lle.");
			obj1.setProperty(OPENED);
		}
	} else if ((verb == ACTION_USE) && (obj1._id == PILL)) {
		if (isHelmetOff()) {
			_vm->renderMessage("Du iát die Tablette und merkst,|da\341 sich irgendetwas ver\216ndert hat.");
			great(0);
			_inventory.remove(obj1);
			_state._language = 2;
			takeObject(*_rooms[ENTRANCE]->getObject(17));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == PILL_HULL) &&
	           (_state._language == 2)) {
		_vm->renderMessage("Komisch! Auf einmal kannst du die Schrift lesen!|Darauf steht:\"Wenn Sie diese Schrift jetzt|lesen k\224nnen, hat die Tablette gewirkt.\"");
		_state._language = 1;
	} else if ((verb == ACTION_OPEN) && (obj1._id == WALLET)) {
		if (!_rooms[ROGER]->getObject(3)->hasProperty(CARRIED)) {
			_vm->renderMessage("Das mu\341t du erst nehmen.");
		} else if (_rooms[ROGER]->getObject(7)->hasProperty(CARRIED)) {
			_vm->renderMessage("Sie ist leer.");
		} else {
			_vm->renderMessage("Du findest 10 Buckazoids und eine Keycard.");
			takeObject(*_rooms[ROGER]->getObject(7));
			takeObject(*_rooms[ROGER]->getObject(8));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == NEWSPAPER)) {
		_vm->renderMessage("Es ist eine Art elektronische Zeitung.");
		mouseWait(_timer1);
		_vm->removeMessage();
		_vm->renderMessage("Halt, hier ist ein interessanter Artikel.");
		mouseWait(_timer1);
		_vm->removeMessage();
		_vm->renderImage(2, 0);
		_vm->setColor63(40);
		getInput();
		_vm->renderRoom(*_currentRoom);
		roomBrightness();
		palette();
		showMenu();
		drawMapExits();
		_vm->renderMessage("Hmm, irgendwie komme|ich mir verarscht vor.");
	} else if ((verb == ACTION_LOOK) && (obj1._id == KEYCARD2)) {
		_vm->renderMessage(obj1._description.c_str());
		obj1._description = "Es ist die Keycard des Commanders.";
	} else if ((verb == ACTION_LOOK) && (obj1._id == WATCH)) {
		_vm->renderMessage(Common::String::format(
		    "Es ist eine Uhr mit extra|lautem Wecker. "
		    "Sie hat einen|Knopf zum Verstellen der Alarmzeit.|"
		    "Uhrzeit: %s   Alarmzeit: %s",
		    timeToString(_state._time + _state._timeStarting).c_str(),
		    timeToString(_state._timeAlarm).c_str()).c_str());
	} else if ((verb == ACTION_PRESS) && (obj1._id == WATCH)) {
		bool validInput = true;
		int hours = 0;
		int minutes = 0;

		animationOff();
		_vm->saveScreen(88, 87, 144, 24);
		_vm->renderBox(88, 87, 144, 24, kColorWhite35);
		_vm->renderText("Neue Alarmzeit (hh:mm) :", 91, 90, kColorWhite99);
		// TODO: Adjust for msec time instead of ticks
		do {
			validInput = true;
			input.clear();
			_vm->renderBox(91, 99, 138, 9, kColorDarkBlue);
			edit(input, 91, 100, 5);

			int seperator = -1;
			for (int i = 0; i < input.size(); ++i) {
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
			for (int i = seperator + 1; i < input.size(); ++i) {
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
			_state._timeAlarmSystem = _state._timeAlarm + _state._timeStarting;
			_state._alarmOn = (_state._timeAlarmSystem > _vm->_system->getMillis());
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, WIRE)) {
		r = _rooms[CABIN_L3];
		if (!r->getObject(8)->hasProperty(CARRIED)) {
			if (r->isSectionVisible(26))
				_vm->renderMessage(Object::takeMessage);
			else
				return false;
		} else {
			r->getObject(8)->_name = "Leitung mit L\201sterklemme";
			r = _rooms[HOLD];
			_inventory.remove(*r->getObject(2));
			_state._terminalStripConnected = true;
			_state._terminalStripWire = true;
			_vm->renderMessage("Ok.");
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, SPOOL)) {
		r = _rooms[CABIN_L2];
		takeObject(*r->getObject(9));
		r->getObject(9)->_name = "Kabelrolle mit L\201sterklemme";
		r = _rooms[HOLD];
		_inventory.remove(*r->getObject(2));
		_state._terminalStripConnected = true;
		_vm->renderMessage("Ok.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SPOOL)) {
		r = _rooms[CABIN_L3];
		if (!_state._terminalStripConnected) {
			if (r->isSectionVisible(26))
				_vm->renderMessage("Womit denn?");
			else
				return false;
		} else {
			if (!r->getObject(8)->hasProperty(CARRIED)) {
				_vm->renderMessage(Object::takeMessage);
			} else {
				r = _rooms[CABIN_L2];
				takeObject(*r->getObject(9));
				r = _rooms[CABIN_L3];
				r->getObject(8)->_name = "langes Kabel mit Stecker";
				r = _rooms[CABIN_L2];
				_inventory.remove(*r->getObject(9));
				_state._cableConnected = true;
				_vm->renderMessage("Ok.");
			}
		}
	} else if ((verb == ACTION_USE) && (obj1._id == SUIT)) {
		takeObject(obj1);
		if ((_currentRoom >= _rooms[ENTRANCE]) && (_currentRoom <= _rooms[ROGER])) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage("Die Luft hier ist atembar,|du ziehst den Anzug aus.");
				_rooms[AIRLOCK]->getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(6)->disableProperty(WORN);
			} else
				_vm->renderMessage("Hier drinnen brauchtst du deinen Anzug nicht.");
		} else {
			if (obj1.hasProperty(WORN)) {
				r = _rooms[AIRLOCK];
				if (r->getObject(4)->hasProperty(WORN)) {
					_vm->renderMessage("Du mu\341t erst den Helm abnehmen.");
				} else if (r->getObject(6)->hasProperty(WORN)) {
					_vm->renderMessage("Du mu\341t erst den Versorgungsteil abnehmen.");
				} else {
					obj1.disableProperty(WORN);
					_vm->renderMessage("Du ziehst den Raumanzug aus.");
				}
			} else {
				obj1.setProperty(WORN);
				_vm->renderMessage("Du ziehst den Raumanzug an.");
			}
		}
	} else if ((verb == ACTION_USE) && (obj1._id == HELMET)) {
		takeObject(obj1);
		if ((_currentRoom >= _rooms[ENTRANCE]) && (_currentRoom <= _rooms[ROGER])) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage("Die Luft hier ist atembar,|du ziehst den Anzug aus.");
				_rooms[AIRLOCK]->getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(6)->disableProperty(WORN);
			} else {
				_vm->renderMessage("Hier drinnen brauchtst du deinen Anzug nicht.");
			}
		} else {
			if (obj1.hasProperty(WORN)) {
				if (airless()) {
					death("Den Helm h\204ttest du|besser angelassen!");
				}
				obj1.disableProperty(WORN);
				_vm->renderMessage("Du ziehst den Helm ab.");
			} else {
				r = _rooms[AIRLOCK];
				if (r->getObject(5)->hasProperty(WORN)) {
					obj1.setProperty(WORN);
					_vm->renderMessage("Du ziehst den Helm auf.");
				} else {
					_vm->renderMessage("Du mu\341t erst den Anzug anziehen.");
				}
			}
		}
	} else if ((verb == ACTION_USE) && (obj1._id == LIFESUPPORT)) {
		takeObject(obj1);
		if ((_currentRoom >= _rooms[ENTRANCE]) && (_currentRoom <= _rooms[ROGER])) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage("Die Luft hier ist atembar,|du ziehst den Anzug aus.");
				_rooms[AIRLOCK]->getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK]->getObject(6)->disableProperty(WORN);
			} else
				_vm->renderMessage("Hier drinnen brauchtst du deinen Anzug nicht.");
		} else {
			if (obj1.hasProperty(WORN)) {
				if (airless()) {
					death("Den Versorgungsteil h\204ttest du|besser nicht abgenommen!");
				}
				obj1.disableProperty(WORN);
				_vm->renderMessage("Du nimmst den Versorgungsteil ab.");
			} else {
				r = _rooms[AIRLOCK];
				if (r->getObject(5)->hasProperty(WORN)) {
					obj1.setProperty(WORN);
					_vm->renderMessage("Du ziehst den Versorgungsteil an.");
				} else {
					_vm->renderMessage("Du mu\341t erst den Anzug anziehen.");
				}
			}
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == BATHROOM_DOOR)) {
//		*bathroom = current_room;
		return false;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SOCKET))
		_vm->renderMessage("Die Leitung ist hier unn\201tz.");
	else if ((verb == ACTION_LOOK) && (obj1._id == BOOK2)) {
		_vm->renderMessage("Stark, das ist ja die Fortsetzung zum \"Anhalter\":|\"Das Restaurant am Ende des Universums\".");
		mouseWait(_timer1);
		_vm->removeMessage();
		_vm->renderMessage("Moment mal, es ist ein Lesezeichen drin,|auf dem \"Zweiundvierzig\" steht.");
	} else {
		return false;
	}

	return true;
}

void GameManager::handleInput() {
	bool validCommand = genericInteract(_inputVerb, *_inputObject[0], *_inputObject[1]);
	if (!validCommand)
		validCommand = _currentRoom->interact(_inputVerb, *_inputObject[0], *_inputObject[1]);
	if (!validCommand) {
		switch (_inputVerb) {
		case ACTION_LOOK:
			_vm->renderMessage(_inputObject[0]->_description.c_str());
			break;

		case ACTION_WALK:
			if (_inputObject[0]->hasProperty(CARRIED)) {
				// You already carry this.
				_vm->renderMessage("Das tr\204gst du doch bei dir.");
			} else if (!_inputObject[0]->hasProperty(EXIT)) {
				// You're already there.
				_vm->renderMessage("Du bist doch schon da.");
			} else if (_inputObject[0]->hasProperty(OPENABLE) && !_inputObject[0]->hasProperty(OPENED)) {
				// This is closed
				_vm->renderMessage("Das ist geschlossen.");
			} else {
				changeRoom(_inputObject[0]->_exitRoom);
			}
			break;

		case ACTION_TAKE:
			if (_inputObject[0]->hasProperty(OPENED)) {
				// You already have that
				_vm->renderMessage("Das hast du doch schon.");
			} else if (_inputObject[0]->hasProperty(UNNECESSARY)) {
				// You do not need that.
				_vm->renderMessage("Das brauchst du nicht.");
			} else if (!_inputObject[0]->hasProperty(TAKE)) {
				// You can't take that.
				_vm->renderMessage("Das kannst du nicht nehmen.");
			} else {
				takeObject(*_inputObject[0]);
			}
			break;

		case ACTION_OPEN:
			if (!_inputObject[0]->hasProperty(OPENABLE)) {
				// This can't be opened
				_vm->renderMessage("Das l\204\341t sich nicht \224ffnen.");
			} else if (_inputObject[0]->hasProperty(OPENED)) {
				// This is already opened.
				_vm->renderMessage("Das ist schon offen.");
			} else if (_inputObject[0]->hasProperty(CLOSED)) {
				// This is locked.
				_vm->renderMessage("Das ist verschlossen.");
			} else {
				drawImage(_inputObject[0]->_section);
				_inputObject[0]->setProperty(OPENED);
				byte i = _inputObject[0]->_click;
				_inputObject[0]->_click  = _inputObject[0]->_click2;
				_inputObject[0]->_click2 = i;
				_vm->playSound(kAudioDoorOpen);
			}
			break;

		case ACTION_CLOSE:
			if (!_inputObject[0]->hasProperty(OPENABLE) ||
			    (_inputObject[0]->hasProperty(CLOSED) &&
			     _inputObject[0]->hasProperty(OPENED))) {
				// This can't be closed.
				_vm->renderMessage("Das l\204\341t sich nicht schlie\341en.");
			} else if (!_inputObject[0]->hasProperty(OPENED)) {
				// This is already closed.
				_vm->renderMessage("Das ist schon geschlossen.");
			} else {
				drawImage(invertSection(_inputObject[0]->_section));
				_inputObject[0]->disableProperty(OPENED);
				byte i = _inputObject[0]->_click;
				_inputObject[0]->_click  = _inputObject[0]->_click2;
				_inputObject[0]->_click2 = i;
				_vm->playSound(kAudioDoorClose);
			}
			break;

		case ACTION_GIVE:
			if (_inputObject[0]->hasProperty(CARRIED)) {
				// Better keep it!
				_vm->renderMessage("Behalt es lieber!");
			}
			break;

		default:
			// This is not possible.
			_vm->renderMessage("Das geht nicht.");
		}
	}
}

void GameManager::executeRoom() {
	if (_processInput && !_vm->_messageDisplayed && _guiEnabled) {
		handleInput();
		resetInputState();
	}

	if (_guiEnabled) {
		if (!_vm->_messageDisplayed) {
			g_system->fillScreen(kColorBlack);
			_vm->renderRoom(*_currentRoom);
		}
		drawMapExits();
		drawInventory();
		drawStatus();
		drawCommandBox();
	}
	roomBrightness();
	if (_vm->_brightness == 0)
		_vm->paletteFadeIn();
	else
		_vm->paletteBrightness();
	if (!_currentRoom->hasSeen())
		_currentRoom->onEntrance();
}

}
