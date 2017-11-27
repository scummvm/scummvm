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
#include "graphics/palette.h"
#include "gui/message.h"
#include "supernova/supernova.h"
#include "supernova/state.h"
#include "graphics/cursorman.h"

namespace Supernova {

bool GameManager::serialize(Common::WriteStream *out) {
	if (out->err())
		return false;

	// GameState
	out->writeSint32LE(_state._time);
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
	for (int i = 0; i < NUMROOMS; ++i) {
		_rooms[i]->serialize(out);
	}

	return !out->err();
}


bool GameManager::deserialize(Common::ReadStream *in, int version) {
	if (in->err())
		return false;

	// GameState
	_state._time = in->readSint32LE();
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

	_oldTime = g_system->getMillis();

	// Inventory
	int inventorySize = in->readSint32LE();
	_inventoryScroll = in->readSint32LE();
	_inventory.clear();
	for (int i = 0; i < inventorySize; ++i) {
		RoomID objectRoom = static_cast<RoomID>(in->readSint32LE());
		int objectIndex = in->readSint32LE();
		_inventory.add(*_rooms[objectRoom]->getObject(objectIndex));
	}

	// Rooms
	RoomID curRoomId = static_cast<RoomID>(in->readByte());
	for (int i = 0; i < NUMROOMS; ++i) {
		_rooms[i]->deserialize(in, version);
	}
	changeRoom(curRoomId);

	// Some additional variables
	_guiEnabled = true;
	_animationEnabled = true;

	return !in->err();
}

Inventory::Inventory() : _numObjects(0) {
}

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
	: _isHighlighted(false)
	, _bgColorNormal(kColorWhite25)
	, _bgColorHighlighted(kColorWhite44)
	, _bgColor(kColorWhite25)
	, _textColorNormal(kColorGreen)
	, _textColorHighlighted(kColorLightGreen)
	, _textColor(kColorGreen)
{
	_text[0] = '\0';
}

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

GameManager::GameManager(SupernovaEngine *vm) {
	_vm = vm;

	initRooms();
	changeRoom(INTRO);
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
	delete _rooms[OUTRO];
}


void GameManager::initState() {
	Object::setObjectNull(_currentInputObject);
	Object::setObjectNull(_inputObject[0]);
	Object::setObjectNull(_inputObject[1]);
	_inputVerb = ACTION_WALK;
	_processInput = false;
	_guiEnabled = true;
	_animationEnabled = true;
	_roomBrightness = 255;
	_mouseClicked = false;
	_keyPressed = false;
	_mouseX = -1;
	_mouseY = -1;
	_mouseField = -1;
	_inventoryScroll = 0;
	_oldTime = g_system->getMillis();
	_timerPaused = 0;
	_timePaused = false;
	_timer1 = 0;
	_animationTimer = 0;

	_currentSentence = -1;
	for (int i = 0 ; i < 6 ; ++i) {
		_sentenceNumber[i] = -1;
		_texts[i] = kNoString;
		_rows[i] = 0;
		_rowsStart[i] = 0;
	}

	_state._time = ticksToMsec(916364); // 2 pm
	_state._timeSleep = 0;
	_state._timeAlarm = ticksToMsec(458182);    // 7 am
	_state._eventTime = 0xffffffff;
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

	_prevImgId = 0;
}

void GameManager::initRooms() {
	_rooms[INTRO] = new Intro(_vm, this);
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
	_rooms[SIGN] = new AxacussSign(_vm, this);
	_rooms[OUTRO] = new Outro(_vm, this);
}

void GameManager::initGui() {
	int commandButtonX = 0;
	for (int i = 0; i < ARRAYSIZE(_guiCommandButton); ++i) {
		const Common::String &text = _vm->getGameString(guiCommands[i]);
		int width;
		if (i < 9)
			width = _vm->textWidth(text) + 2;
		else
			width = 320 - commandButtonX;

		_guiCommandButton[i].setSize(commandButtonX, 150, commandButtonX + width, 159);
		_guiCommandButton[i].setText(text.c_str());
		_guiCommandButton[i].setColor(kColorWhite25, kColorDarkGreen, kColorWhite44, kColorGreen);
		commandButtonX += width + 2;
	}

	for (int i = 0; i < ARRAYSIZE(_guiInventory); ++i) {
		int inventoryX = 136 * (i % 2);
		int inventoryY = 161 + 10 * (i / 2);

		_guiInventory[i].setSize(inventoryX, inventoryY, inventoryX + 135, inventoryY + 9);
		_guiInventory[i].setColor(kColorWhite25, kColorDarkRed, kColorWhite35, kColorRed);
	}
	_guiInventoryArrow[0].setSize(272, 161, 279, 180);
	_guiInventoryArrow[0].setColor(kColorWhite25, kColorDarkRed, kColorWhite35, kColorRed);
	_guiInventoryArrow[0].setText("\x82");
	_guiInventoryArrow[0].setTextPosition(273, 166);
	_guiInventoryArrow[1].setSize(272, 181, 279, 200);
	_guiInventoryArrow[1].setColor(kColorWhite25, kColorDarkRed, kColorWhite35, kColorRed);
	_guiInventoryArrow[1].setText("\x83");
	_guiInventoryArrow[1].setTextPosition(273, 186);
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
		_vm->setTextSpeed();
		break;
	case Common::KEYCODE_F5:
		// load/save
		break;
	case Common::KEYCODE_x:
		if (state.flags & Common::KBD_ALT) {
			// quit game
			GUI::MessageDialog *dialog = new GUI::MessageDialog("Quit Game?", "Quit", "Cancel");
			if (dialog->runModal() == GUI::kMessageOK)
				_vm->quitGame();
			delete dialog;

			// TODO: Add original quit game message prompt
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
		if (_vm->_messageDisplayed) {
			// Hide the message and consume the event
			_vm->removeMessage();
			return;
		}

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
			if (_inventoryScroll < _inventory.getSize() - ARRAYSIZE(_guiInventory))
				_inventoryScroll += 2;
		}
	} else if (_mouseClickType == Common::EVENT_RBUTTONUP) {
		if (_vm->_messageDisplayed) {
			// Hide the message and consume the event
			_vm->removeMessage();
			return;
		}
		if (Object::isNullObject(_currentInputObject))
			return;

		if (((_mouseField >= 0) && (_mouseField < 256)) ||
		    ((_mouseField >= 512) && (_mouseField < 768))) {
			_inputObject[0] = _currentInputObject;
			ObjectTypes type = _inputObject[0]->_type;
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
			for (int i = 0; (_currentRoom->getObject(i)->_id != INVALIDOBJECT) &&
			                (field == -1) && i < kMaxObject; i++) {
				click = _currentRoom->getObject(i)->_click;
				if (click != 255 && _vm->_currentImage) {
					MSNImageDecoder::ClickField *clickField = _vm->_currentImage->_clickField;
					do {
						if ((_mouseX >= clickField[click].x1) && (_mouseX <= clickField[click].x2) &&
						    (_mouseY >= clickField[click].y1) && (_mouseY <= clickField[click].y2))
							field = i;

						click = clickField[click].next;
					} while ((click != 0) && (field == -1));
				}
			}
		}

		if (_mouseField != field) {
			if (_mouseField >= 768) {
				_guiInventoryArrow[_mouseField - 768].setHighlight(false);
			} else if (_mouseField >= 512) {
				_guiInventory[_mouseField - 512].setHighlight(false);
			} else if (_mouseField >= 256) {
				_guiCommandButton[_mouseField - 256].setHighlight(false);
			} else if (_mouseField !=  -1) {
			}
			Object::setObjectNull(_currentInputObject);

			_mouseField = field;
			if (_mouseField >= 768) {
				_guiInventoryArrow[_mouseField - 768].setHighlight(true);
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

void GameManager::corridorOnEntrance() {
	if (_state._corridorSearch)
		busted(0);
}

void GameManager::telomat(int nr) {
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

	Common::String dial1[4];
	dial1[0] = "Guten Tag, hier ist Horst Hummel.";
	dial1[1] = "";
	dial1[2] = "Es ist sehr wichtig.";
	dial1[3] = "|";

	static byte rows1[3] = {1, 2, 1};

	Common::String dial2[4];
	dial2[0] = "Vom Mars.";
	dial2[1] = "Vom Klo.";
	dial2[2] = "Das werde ich kaum erzählen.";
	dial2[3] = "|";

	static byte rows2[4] = {1, 1, 1, 1};

	_vm->renderBox(0, 0, 320, 200, kColorBlack);
	_vm->renderText("1 Büromanager", 100, 70, kColorGreen);
	_vm->renderText("2 Telomat", 100, 81, kColorGreen);
	_vm->renderText("3 ProText", 100, 92, kColorGreen);
	_vm->renderText("4 Calculata", 100, 103, kColorGreen);
	_vm->renderText("Bitte wählen", 100, 120, kColorDarkGreen);
	Common::String input;
	do {
		getInput();

		switch (_key.keycode) {
		case Common::KEYCODE_2: {
			_vm->renderBox(0, 0, 320, 200, kColorDarkBlue);
			_vm->renderText("Geben Sie den gewünschten Namen ein:", 50, 80, kColorGreen);
			_vm->renderText("(Vor- und Nachname)", 50, 91, kColorGreen);
			do
				edit(input, 50, 105, 30);
			while ((_key.keycode != Common::KEYCODE_RETURN) && (_key.keycode != Common::KEYCODE_ESCAPE));
	
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
			while ((input != name[i]) && (i < 8))
				i++;
			i >>= 1;
			if (i == 4) {
				_vm->renderText("Name unbekannt", 50, 120, kColorGreen);
				wait2(10);
				_vm->renderBox(0, 0, 320, 200, kColorBlack);
				_vm->renderRoom(*_currentRoom);
				_vm->paletteBrightness();
				_guiEnabled = true;
				drawMapExits();
				return;
			}

			if ((i == nr) || _rooms[BCORRIDOR]->getObject(4 + i)->hasProperty(CAUGHT)) {
				_vm->renderText("Verbindung unmöglich", 50, 120, kColorGreen);
				wait2(10);
				_vm->renderBox(0, 0, 320, 200, kColorBlack);
				_vm->renderRoom(*_currentRoom);
				_vm->paletteBrightness();
				_guiEnabled = true;
				drawMapExits();
				return;
			}

			_vm->renderText("Verbindung wird hergestellt", 50, 120, kColorGreen);
			wait2(10);
			_vm->renderBox(0, 0, 320, 200, kColorBlack);
			_vm->renderRoom(*_currentRoom);
			_vm->paletteBrightness();
			input = Common::String::format("%s am Apparat.", name2[i]);
			_vm->renderMessage(input, kMessageTop);
			waitOnInput(_timer1);
			_vm->removeMessage();
			if (_state._nameSeen[nr]) {
				input = Common::String::format("Hier ist %s. Können Sie mal gerade kommen?", name2[nr]);
				dial1[1] = input;
				_currentRoom->addSentence(1, 1);
			} else
				_currentRoom->removeSentence(1, 1);

			switch (dialog(3, rows1, dial1, 1)) {
			case 1: _vm->renderMessage("Huch, Sie hören sich aber|nicht gut an. Ich komme sofort.", kMessageTop);
				waitOnInput(_timer1);
				_vm->removeMessage();
				if ((_state._destination == 255) && !_rooms[BCORRIDOR]->isSectionVisible(7)) {
					_state._eventTime = _state._time + ticksToMsec(150);
					_state._eventCallback = kGuardWalkFn;
					_state._origin = i;
					_state._destination = nr;
				}
				break;
			case 0: _vm->renderMessage("Horst Hummel! Von wo rufen Sie an?", kMessageTop);
				waitOnInput(_timer1);
				_vm->removeMessage();
				if (dialog(4, rows2, dial2, 0) != 3) {
					wait2(10);
					say("Hmm, keine Antwort.");
				}
				_rooms[BCORRIDOR]->setSectionVisible(7, true);
				_rooms[BCORRIDOR]->setSectionVisible(i + 1, true);
				_state._eventTime = 0xffffffff;
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
			_vm->renderText("Passwort:", 100, 90, kColorGreen);
			input = "";
			do
				edit(input, 100, 105, 30);
			while ((_key.keycode != Common::KEYCODE_RETURN) && (_key.keycode != Common::KEYCODE_ESCAPE));

			if (_key.keycode == Common::KEYCODE_RETURN) {
				_vm->renderText("Falsches Passwort", 100, 120, kColorGreen);
				wait2(10);
			}
		case Common::KEYCODE_ESCAPE:
			_vm->renderBox(0, 0, 320, 200, kColorBlack);
			_vm->renderRoom(*_currentRoom);
			_vm->paletteBrightness();
			_guiEnabled = true;
			drawMapExits();
			return;
		}
	} while (true);
}

void GameManager::startSearch() {
	if ((_currentRoom->getId() >= CORRIDOR1) && (_currentRoom->getId() <= BCORRIDOR))
		busted(0);

	_state._corridorSearch = true;
}

void GameManager::search(int time) {
	_state._eventTime = _state._time + ticksToMsec(time);
	_state._eventCallback = kSearchStartFn;
}

void GameManager::guardNoticed() {
	_vm->paletteFadeOut();
	Room *r = _currentRoom;
	_currentRoom = _rooms[GUARD];
	_vm->setCurrentImage(40);
	_vm->renderBox(0, 0, 320, 200, 0);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_vm->renderImage(2);
	reply(kStringGuardNoticed1, 2, 5);
	wait2(2);
	reply(kStringGuardNoticed2, 2, 5);
	_vm->paletteFadeOut();
	r->setSectionVisible(2, false);
	r->setSectionVisible(5, false);
	_currentRoom = r;
	_guiEnabled = true;
	drawMapExits();
	// new_room = true;
	warning("SKIPPED: new_room");
}

void GameManager::busted(int i) {
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
				_vm->playSound(kAudioDoorOpen);
				wait2(2);
			}
			_vm->renderImage(i);
			wait2(3);
			_vm->renderImage(i + 3);
			_vm->playSound(kAudioVoiceHalt);
			_vm->renderImage(i);
			wait2(5);
			if (_currentRoom->getId() == OFFICE_L2)
				i = 13;
			_vm->renderImage(i + 1);
			wait2(3);
			_vm->renderImage(i + 2);
			shot(0, 0);
		} else if (_currentRoom->getId() == BCORRIDOR) {
			_vm->renderImage(21);
		} else {
			if (_currentRoom->isSectionVisible(4))
				_vm->renderImage(32); // below
			else if (_currentRoom->isSectionVisible(2))
				_vm->renderImage(30); // right
			else if (_currentRoom->isSectionVisible(1))
				_vm->renderImage(31); // left
			else
				_vm->renderImage(33); // above
		}
	}
	_vm->playSound(kAudioVoiceHalt);
	wait2(3);
	shot(0, 0);
}

void GameManager::novaScroll() {
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
		for (int kreis = 0; kreis < t && kreis < 13; ++kreis) {
			int idx = 3 * (nova_f[kreis] - 1);
			for (int c = 0 ; c < 3 ; ++c)
				palette[idx+c] = rgb[t-kreis-1][c];
		}

		_vm->_system->getPaletteManager()->setPalette(palette, 0, 255);
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(_vm->_delay);
	}
}

void GameManager::supernovaEvent() {
	_vm->removeMessage();
	CursorMan.showMouse(false);
	if (_currentRoom->getId() <= CAVE) {
		_vm->renderMessage(kStringSupernova1);
		waitOnInput(_timer1);
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
	waitOnInput(_timer1);
	_vm->removeMessage();
	_vm->setCurrentImage(26);
	_vm->renderImage(0);
	_vm->paletteBrightness();
	novaScroll();
	_vm->paletteFadeOut();
	_vm->renderBox(0, 0, 320, 200, kColorBlack);
	_vm->_menuBrightness = 255;
	_vm->paletteBrightness();

	if (_currentRoom->getId() == GLIDER) {
		_vm->renderMessage(kStringSupernova3);
		waitOnInput(_timer1);
		_vm->removeMessage();
		_vm->_menuBrightness = 0;
		_vm->paletteBrightness();
		_vm->renderRoom(*_currentRoom);
		_vm->paletteFadeIn();
		_vm->renderMessage(kStringSupernova4, kMessageTop);
		waitOnInput(_timer1);
		_vm->removeMessage();
		_vm->renderMessage(kStringSupernova5, kMessageTop);
		waitOnInput(_timer1);
		_vm->removeMessage();
		_vm->renderMessage(kStringSupernova6, kMessageTop);
		waitOnInput(_timer1);
		_vm->removeMessage();
		_vm->renderMessage(kStringSupernova7, kMessageTop);
		waitOnInput(_timer1);
		_vm->removeMessage();
		changeRoom(MEETUP2);
		_rooms[MEETUP2]->setSectionVisible(1, true);
		_rooms[MEETUP2]->removeSentence(0, 1);
		_inventory.remove(*(_rooms[ROGER]->getObject(3)));
		_inventory.remove(*(_rooms[ROGER]->getObject(7)));
		_inventory.remove(*(_rooms[ROGER]->getObject(8)));
	} else {
		_vm->renderMessage(kStringSupernova8);
		waitOnInput(_timer1);
		_vm->removeMessage();
		_vm->_menuBrightness = 0;
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

void GameManager::guardReturnedEvent() {
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

void GameManager::walk(int imgId) {
	if (_prevImgId)
		_vm->renderImage(_prevImgId + 128);
	_vm->renderImage(imgId);
	_prevImgId = imgId;
	wait2(3);
}

void GameManager::guardWalkEvent() {
	_prevImgId = 0;
	bool behind = (!_rooms[BCORRIDOR]->getObject(_state._origin + 4)->hasProperty(OCCUPIED) || _rooms[BCORRIDOR]->getObject(_state._origin + 4)->hasProperty(OPENED));
	_rooms[BCORRIDOR]->getObject(_state._origin + 4)->resetProperty(OCCUPIED);
	if (_currentRoom == _rooms[BCORRIDOR]) {
		if (_vm->_messageDisplayed)
			_vm->removeMessage();

		if (!behind) {
			_vm->renderImage(_state._origin + 1);
			_prevImgId = _state._origin + 1;
			_vm->playSound(kAudioDoorOpen);
			wait2(3);
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
			wait2(3);
			_vm->renderImage(_prevImgId + 128);
			_vm->playSound(kAudioDoorClose);
		}

		_prevImgId = imgId;
		wait2(3);
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
	
		if (!_currentRoom->isSectionVisible(kMaxSection - 1)) {
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
			_vm->playSound(kAudioDoorOpen);
			wait2(3);
			_vm->renderImage(_prevImgId + 128);
			wait2(3);
			_vm->renderImage(_state._destination + 1 + 128);
			_vm->playSound(kAudioDoorClose);
			_rooms[BCORRIDOR]->getObject(_state._destination + 4)->setProperty(OCCUPIED);
			_state._destination = 255;
		} else if (_rooms[BCORRIDOR]->isSectionVisible(_state._destination + 1)) {
			_vm->renderImage(_prevImgId + 128);
			_rooms[BCORRIDOR]->getObject(_state._destination + 4)->setProperty(OCCUPIED);
			SWAP(_state._origin, _state._destination);
			_state._eventTime = _state._time + ticksToMsec(60);
			_state._eventCallback = kGuardWalkFn;
		} else {
			wait2(18);
			SWAP(_state._origin, _state._destination);
			_state._eventCallback = kGuardWalkFn;
		}
	} else {
		if (behind) {
			_rooms[BCORRIDOR]->getObject(_state._destination + 4)->setProperty(OCCUPIED);
			if (_currentRoom == _rooms[OFFICE_L1 + _state._destination])
				busted(0);
			_state._destination = 255;
		} else if (_rooms[BCORRIDOR]->isSectionVisible(_state._destination + 1) && _rooms[OFFICE_L1 + _state._destination]->getObject(0)->hasProperty(OPENED)) {
			_rooms[BCORRIDOR]->getObject(_state._destination + 4)->setProperty(OCCUPIED);
			if (_currentRoom == _rooms[OFFICE_L1 + _state._destination])
				busted(0);
			SWAP(_state._origin, _state._destination);
			_state._eventTime = _state._time + ticksToMsec(60);
			_state._eventCallback = kGuardWalkFn;
		} else {
			SWAP(_state._origin, _state._destination);
			_state._eventCallback = kGuardWalkFn;
		}
	}
}

void GameManager::taxiEvent() {
	if (_currentRoom->getId() == SIGN) {
		changeRoom(STATION);
		_vm->renderRoom(*_currentRoom);
	}

	_vm->renderImage(1);
	_vm->renderImage(2);
	_vm->playSound(kAudioRocks);
	screenShake();
	_vm->renderImage(9);
	_currentRoom->getObject(1)->setProperty(OPENED);
	_vm->renderImage(1);
	_currentRoom->setSectionVisible(2, false);
	_vm->renderImage(3);
	for (int i = 4; i <= 8; i++) {
		wait2(2);
		_vm->renderImage(invertSection(i - 1));
		_vm->renderImage(i);
	}
	_rooms[SIGN]->setSectionVisible(2, false);
	_rooms[SIGN]->setSectionVisible(3, true);
}

void GameManager::searchStartEvent() {
	if ((_currentRoom >= _rooms[CORRIDOR1]) && (_currentRoom <= _rooms[BCORRIDOR]))
		busted(0);
	_state._corridorSearch = true;
}

void GameManager::outro() {
//	title = 2;
	_vm->playSoundMod(49);
//	title = 0;
	_vm->paletteFadeOut();
	_vm->setCurrentImage(55);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	getInput();
	_vm->paletteFadeOut();
	// TODO: render info file
	// Exit
}

void GameManager::great(uint number) {
	if (number && (_state._greatFlag & (1 << number)))
		return;

	_vm->playSound(kAudioUndef7);
	_state._greatFlag |= 1 << number;
}

bool GameManager::airless() {
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

void GameManager::sentence(int number, bool brightness) {
	if (number < 0)
		return;
	_vm->renderBox(0, 141 + _rowsStart[number] * 10, 320, _rows[number] * 10 - 1, brightness ? kColorWhite44 : kColorWhite25);
	if (_texts[_rowsStart[number]] == kStringDialogSeparator)
		_vm->renderText(kStringConversationEnd, 1, 142 + _rowsStart[number] * 10, brightness ? kColorRed : kColorDarkRed);
	else {
		for (int r = _rowsStart[number]; r < _rowsStart[number] + _rows[number]; ++r)
			_vm->renderText(_texts[r], 1, 142 + r * 10, brightness ? kColorGreen : kColorDarkGreen);
	}
}

void GameManager::say(StringID textId) {
	Common::String str = _vm->getGameString(textId);
	if (!str.empty())
		say(str.c_str());
}

void GameManager::say(const char *text) {
	Common::String t(text);
	char *row[6];
	Common::String::iterator p = t.begin();
	uint numRows = 0;
	while (*p) {
		row[numRows++] = p;
		while ((*p != '\0') && (*p != '|')) {
			++p;
		}
		if (*p == '|') {
			*p = 0;
			++p;
		}
	}

	_vm->renderBox(0, 138, 320, 62, kColorBlack);
	_vm->renderBox(0, 141, 320, numRows * 10 - 1, kColorWhite25);
	for (uint r = 0; r < numRows; ++r)
		_vm->renderText(row[r], 1, 142 + r * 10, kColorDarkGreen);
	waitOnInput((t.size() + 20) * _vm->_textSpeed / 10);
	_vm->renderBox(0, 138, 320, 62, kColorBlack);
}

void GameManager::reply(StringID textId, int aus1, int aus2) {
	Common::String str = _vm->getGameString(textId);
	if (!str.empty())
		reply(str.c_str(), aus1, aus2);
}

void GameManager::reply(const char *text, int aus1, int aus2) {
	if (*text != '|')
		_vm->renderMessage(text, kMessageTop);

	for (int z = (strlen(text) + 20) * _vm->_textSpeed / 40; z > 0; --z) {
		_vm->renderImage(aus1);
		waitOnInput(2);
		if (_keyPressed || _mouseClicked)
			z = 1;
		_vm->renderImage(aus2);
		waitOnInput(2);
		if (_keyPressed || _mouseClicked)
			z = 1;
	}
	if (*text != '|')
		_vm->removeMessage();
}

int GameManager::dialog(int num, byte rowLength[6], Common::String text[6], int number) {
	warning("STUB dialog");
	return 0;
}

int GameManager::dialog(int num, byte rowLength[6], StringID text[6], int number) {
	_vm->_allowLoadGame = false;
	_guiEnabled = false;

	bool remove[6];
	for (int i = 0; i < 5; ++i)
		remove[i] = _currentRoom->sentenceRemoved(i, number);
	// The original does not initialize remove[5]!!!
	// Set it to false/0. But maybe the loop above should use 6 instead of 5?
	remove[5] = false;

	_vm->renderBox(0, 138, 320, 62, kColorBlack);

	for (int i = 0; i < 6 ; ++i)
		_sentenceNumber[i] = -1;

	int r = 0, rq = 0;
	for (int i = 0; i < num; ++i) {
		if (!remove[i]) {
			_rowsStart[i] = r;
			_rows[i] = rowLength[i];
			for (int j = 0; j < _rows[i]; ++j, ++r, ++rq) {
				_texts[r] = text[rq];
				_sentenceNumber[r] = i;
			}
			sentence(i, false);
		} else
			rq += rowLength[i];
	}

	_currentSentence = -1;
	do {
		mouseInput3();
	} while (_currentSentence == -1 && !_vm->shouldQuit());

	_vm->renderBox(0, 138, 320, 62, kColorBlack);

	if (number && _texts[_rowsStart[_currentSentence]] != kStringDialogSeparator)
		_currentRoom->removeSentence(_currentSentence, number);

	_guiEnabled = true;
	_vm->_allowLoadGame = true;

	return _currentSentence;
}

void GameManager::mousePosDialog(int x, int y) {
	int a = y < 141 ? -1 : _sentenceNumber[(y - 141) / 10];
	if (a != _currentSentence) {
		sentence(_currentSentence, false);
		_currentSentence = a;
		sentence(_currentSentence, true);
	}
}

void GameManager::turnOff() {
	if (_state._powerOff)
		return;

	_state._powerOff = true;
	roomBrightness();
	_vm->paletteBrightness();
}

void GameManager::turnOn() {
	if (!_state._powerOff)
		return;

	_state._powerOff = false;
	_vm->_brightness = 255;
//	_vm->paletteBrightness();
	_rooms[SLEEP]->setSectionVisible(1, false);
	_rooms[SLEEP]->setSectionVisible(2, false);
	_rooms[COCKPIT]->setSectionVisible(22, false);
}

void GameManager::takeObject(Object &obj) {
	if (obj.hasProperty(CARRIED))
		return;

	if (obj._section != 0)
		_vm->renderImage(obj._section);
	obj.setProperty(CARRIED);
	obj._click = obj._click2 = 255;
	_inventory.add(obj);
	if (_inventory.getSize() > _inventoryScroll + 8) {
		_inventoryScroll = _inventory.getSize() - 8;
		_inventoryScroll += _inventoryScroll % 2;
	}
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

void GameManager::drawInventory() {
	for (int i = 0; i < ARRAYSIZE(_guiInventory); ++i) {
		_vm->renderBox(_guiInventory[i].left,
		               _guiInventory[i].top,
		               _guiInventory[i].width(),
		               _guiInventory[i].height(),
		               _guiInventory[i]._bgColor);
		_vm->renderText(_inventory.get(i + _inventoryScroll)->_name,
		                _guiInventory[i]._textPosition.x,
		                _guiInventory[i]._textPosition.y,
		                _guiInventory[i]._textColor);
	}

	_vm->renderBox(_guiInventoryArrow[0].left,
	               _guiInventoryArrow[0].top,
	               _guiInventoryArrow[0].width(),
	               _guiInventoryArrow[0].height(),
	               _guiInventoryArrow[0]._bgColor);
	_vm->renderBox(_guiInventoryArrow[1].left,
	               _guiInventoryArrow[1].top,
	               _guiInventoryArrow[1].width(),
	               _guiInventoryArrow[1].height(),
	               _guiInventoryArrow[1]._bgColor);
	if (_inventory.getSize() > ARRAYSIZE(_guiInventory)) {
		if (_inventoryScroll != 0) {
			_vm->renderText(_guiInventoryArrow[0]._text,
			                _guiInventoryArrow[0]._textPosition.x,
			                _guiInventoryArrow[0]._textPosition.y,
			                _guiInventoryArrow[0]._textColor);
		}
		if (_inventoryScroll + ARRAYSIZE(_guiInventory) < _inventory.getSize()) {
			_vm->renderText(_guiInventoryArrow[1]._text,
			                _guiInventoryArrow[1]._textPosition.x,
			                _guiInventoryArrow[1]._textPosition.y,
			                _guiInventoryArrow[1]._textColor);
		}
	}
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
	while (!_vm->shouldQuit()) {
		_vm->updateEvents();
		if (_mouseClicked)
			return _mouseClickType;
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
	return Common::EVENT_INVALID;
}

void GameManager::getInput() {
	while (!_vm->shouldQuit()) {
		_vm->updateEvents();
		if (_mouseClicked || _keyPressed)
			break;
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
}

void GameManager::mouseInput3() {
	do {
		_vm->updateEvents();
		mousePosDialog(_mouseX, _mouseY);
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	} while (!_mouseClicked && !_vm->shouldQuit());
}

void GameManager::roomBrightness() {
	_roomBrightness = 255;
	if ((_currentRoom->getId() != OUTSIDE) && (_currentRoom->getId() < ROCKS) ) {
		if (_state._powerOff)
			_roomBrightness = 153;
	} else if (_currentRoom->getId() == CAVE) {
		_roomBrightness = 0;
	} else if (_currentRoom->getId() == GUARD3) {
		if (_state._powerOff)
			_roomBrightness = 0;
	}

	if (_vm->_brightness != 0)
		_vm->_brightness = _roomBrightness;
}

void GameManager::changeRoom(RoomID id) {
	_currentRoom = _rooms[id];
}

void GameManager::wait2(int ticks) {
	int32 end = _state._time + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		_vm->updateEvents();
		g_system->updateScreen();
	} while (_state._time < end && !_vm->shouldQuit());
}

void GameManager::waitOnInput(int ticks) {
	int32 end = _state._time + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		_vm->updateEvents();
		g_system->updateScreen();
	} while (_state._time < end && !_vm->shouldQuit() && !_keyPressed && !_mouseClicked);
}

bool GameManager::waitOnInput(int ticks, Common::KeyCode &keycode) {
	keycode = Common::KEYCODE_INVALID;
	int32 end = _state._time + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		_vm->updateEvents();
		g_system->updateScreen();
		if (_keyPressed) {
			keycode = _key.keycode;
			_key.reset();
			return true;
		} else if (_mouseClicked)
			return true;
	} while (_state._time < end  && !_vm->shouldQuit());
	return false;
}

void GameManager::setAnimationTimer(int ticks) {
	_animationTimer = ticksToMsec(ticks);
}

void GameManager::handleTime() {
	if (_timerPaused)
		return;
	int32 newTime = g_system->getMillis();
	int32 delta = newTime - _oldTime;
	_state._time += delta;
	if (_state._time > 86400000)
		_state._time -= 86400000; // 24h wrap around
	if (_animationTimer > delta)
		_animationTimer -= delta;
	else
		_animationTimer = 0;

	_oldTime = newTime;
}

void GameManager::pauseTimer(bool pause) {
	if (pause == _timerPaused)
		return;

	if (pause) {
		_timerPaused = true;
		int32 delta = g_system->getMillis() - _oldTime;
		_timePaused = _state._time + delta;
	} else {
		_state._time = _timePaused;
		_oldTime = g_system->getMillis();
		_timerPaused = false;
	}
}

void GameManager::loadTime() {
	pauseTimer(false);
}

void GameManager::saveTime() {
	pauseTimer(true);
}

void GameManager::screenShake() {
	for (int i = 0; i < 12; ++i) {
		_vm->_system->setShakePos(8);
		wait2(1);
		_vm->_system->setShakePos(0);
		wait2(1);
	}
}

void GameManager::shock() {
	_vm->playSound(kAudioShock);
	dead(kStringShock);
}

void GameManager::showMenu() {
	_vm->renderBox(0, 138, 320, 62, 0);
	_vm->renderBox(0, 140, 320, 9, kColorWhite25);
	drawCommandBox();
	_vm->renderBox(281, 161, 39, 39, kColorWhite25);
	drawInventory();
}

void GameManager::drawMapExits() {
// TODO: Preload _exitList on room entry instead on every call
	_vm->renderBox(281, 161, 39, 39, kColorWhite25);

	for (int i = 0; i < 25; i++)
		_exitList[i] = -1;
	for (int i = 0; i < kMaxObject; i++) {
		if (_currentRoom->getObject(i)->hasProperty(EXIT)) {
			byte r = _currentRoom->getObject(i)->_direction;
			_exitList[r] = i;
			int x = 284 + 7 * (r % 5);
			int y = 164 + 7 * (r / 5);
			_vm->renderBox(x, y, 5, 5, kColorDarkRed);
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
	int overdrawWidth = ((int)((length + 1) * (kFontWidth + 2)) > (kScreenWidth - x)) ?
	                    kScreenWidth - x : (length + 1) * (kFontWidth + 2);

	while (isEditing) {
		_vm->_textCursorX = x;
		_vm->_textCursorY = y;
		_vm->_textColor = kColorWhite99;
		_vm->renderBox(x, y - 1, overdrawWidth, 9, kColorDarkBlue);
		for (uint i = 0; i < input.size(); ++i) {
			// Draw char highlight depending on cursor position
			if (i == cursorIndex) {
				_vm->renderBox(_vm->_textCursorX, y - 1, _vm->textWidth(input[i]), 9, kColorWhite99);
				_vm->_textColor = kColorDarkBlue;
				_vm->renderText(input[i]);
				_vm->_textColor = kColorWhite99;
			} else {
				_vm->renderText(input[i]);
			}
		}
		if (cursorIndex == input.size()) {
			_vm->renderBox(_vm->_textCursorX + 1, y - 1, 6, 9, kColorDarkBlue);
			_vm->renderBox(_vm->_textCursorX    , y - 1, 1, 9, kColorWhite99);
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

void GameManager::shot(int a, int b) {
	if (a)
		_vm->renderImage(a);
	_vm->playSound(kAudioGunShot);
	wait2(2);
	if (b)
		_vm->renderImage(b);
	wait2(2);
	if (a)
		_vm->renderImage(a);
	_vm->playSound(kAudioGunShot);
	wait2(2);
	if (b)
		_vm->renderImage(b);

	dead(kStringShot);
}

void GameManager::takeMoney(int amount) {
	_state._money += amount;
	if (amount > 0)
		great(0);
	// TODO: kmaxobject - 1?
//	_rooms[OFFICE_R1]->getObject(5)->_name = _rooms[OFFICE_R1]->getObject(kMaxObject - 1);
//	_rooms[OFFICE_R1]->object[5].name = &(_rooms[OFFICE_R1]->object[MAX_OBJECT-1]);
//	strcpy(_rooms[OFFICE_R1]->object[5].name,ltoa((long)_state.money));
//	strcat(_rooms[OFFICE_R1]->object[5].name," Xa");

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
	_vm->renderText(_vm->getGameString(guiStatusCommands[index]), 1, 141, kColorDarkGreen);

	if (Object::isNullObject(_inputObject[0])) {
		_vm->renderText(_currentInputObject->_name);
	} else {
		_vm->renderText(_inputObject[0]->_name);
		if (_inputVerb == ACTION_GIVE)
			_vm->renderText(kPhrasalVerbParticleGiveTo);
		else if (_inputVerb == ACTION_USE)
			_vm->renderText(kPhrasalVerbParticleUseWith);

		_vm->renderText(_currentInputObject->_name);
	}
}

void GameManager::openLocker(const Room *room, Object *obj, Object *lock, int section) {
	_vm->renderImage(section);
	obj->setProperty(OPENED);
	lock->_click = 255;
	int i = obj->_click;
	obj->_click = obj->_click2;
	obj->_click2 = i;
}

void GameManager::closeLocker(const Room *room, Object *obj, Object *lock, int section) {
	if (!obj->hasProperty(OPENED)) {
		_vm->renderMessage(kStringCloseLocker_1);
	} else {
		_vm->renderImage(invertSection(section));
		obj->disableProperty(OPENED);
		lock->_click = lock->_click2;
		int i = obj->_click;
		obj->_click = obj->_click2;
		obj->_click2 = i;
	}
}

void GameManager::dead(StringID messageId) {
	_vm->paletteFadeOut();
	_guiEnabled = false;
	_vm->setCurrentImage(11);
	_vm->renderImage(0);
	_vm->renderMessage(messageId);
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
	changeRoom(CABIN_R3);
	g_system->fillScreen(kColorBlack);
	_vm->paletteFadeIn();

	_guiEnabled = true;
}

// TODO: Remove this function when all the texts are properly extracted
void GameManager::dead(const char *message) {
	_vm->paletteFadeOut();
	_guiEnabled = false;
	_vm->setCurrentImage(11);
	_vm->renderImage(0);
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
	changeRoom(CABIN_R3);
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
		_vm->renderMessage(kStringIsHelmetOff_1);
		return false;
	}

	return true;
}

bool GameManager::genericInteract(Action verb, Object &obj1, Object &obj2) {
	Room *r;
	Common::String input;

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
		if (obj1.hasProperty(OPENED)) {
			_vm->renderMessage(kStringGenericInteract_3);
		} else {
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
		if (!_rooms[ROGER]->getObject(3)->hasProperty(CARRIED)) {
			_vm->renderMessage(kStringGenericInteract_7);
		} else if (_rooms[ROGER]->getObject(7)->hasProperty(CARRIED)) {
			_vm->renderMessage(kStringGenericInteract_8);
		} else {
			_vm->renderMessage(kStringGenericInteract_9);
			takeObject(*_rooms[ROGER]->getObject(7));
			takeObject(*_rooms[ROGER]->getObject(8));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == NEWSPAPER)) {
		_vm->renderMessage(kStringGenericInteract_10);
		waitOnInput(_timer1);
		_vm->removeMessage();
		_vm->renderMessage(kStringGenericInteract_11);
		waitOnInput(_timer1);
		_vm->removeMessage();
		_vm->setCurrentImage(2);
		_vm->renderImage(0);
		_vm->setColor63(40);
		getInput();
		_vm->renderRoom(*_currentRoom);
		roomBrightness();
		_vm->paletteBrightness();
		_vm->renderMessage(kStringGenericInteract_12);
	} else if ((verb == ACTION_LOOK) && (obj1._id == KEYCARD2)) {
		_vm->renderMessage(obj1._description);
		obj1._description = kStringKeycard2Description2;
	} else if ((verb == ACTION_LOOK) && (obj1._id == WATCH)) {
		// FIXME: kStringGenericInteract_13 requires 2 string parameters in order to format the string properly
		_vm->renderMessage(kStringGenericInteract_13, kMessageNormal);
		//    timeToString(_state._time).c_str(),
		//    timeToString(_state._timeAlarm).c_str());
	} else if ((verb == ACTION_PRESS) && (obj1._id == WATCH)) {
		bool validInput = true;
		int hours = 0;
		int minutes = 0;

		animationOff();
		_vm->saveScreen(88, 87, 144, 24);
		_vm->renderBox(88, 87, 144, 24, kColorWhite35);
		_vm->renderText(kStringGenericInteract_14, 91, 90, kColorWhite99);
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
			_state._alarmOn = (_state._timeAlarm > _state._time);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, WIRE)) {
		r = _rooms[CABIN_L3];
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
		r = _rooms[CABIN_L2];
		takeObject(*r->getObject(9));
		r->getObject(9)->_name = kSringSpoolAndClip;
		r = _rooms[HOLD];
		_inventory.remove(*r->getObject(2));
		_state._terminalStripConnected = true;
		_vm->renderMessage(kStringOk);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SPOOL)) {
		r = _rooms[CABIN_L3];
		if (!_state._terminalStripConnected) {
			if (r->isSectionVisible(26))
				_vm->renderMessage(kStringCable3);
			else
				return false;
		} else {
			if (!r->getObject(8)->hasProperty(CARRIED)) {
				_vm->renderMessage(kStringTakeMessage);
			} else {
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
				r = _rooms[AIRLOCK];
				if (r->getObject(4)->hasProperty(WORN)) {
					_vm->renderMessage(kStringGenericInteract_17);
				} else if (r->getObject(6)->hasProperty(WORN)) {
					_vm->renderMessage(kStringGenericInteract_18);
				} else {
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
			} else {
				_vm->renderMessage(kStringGenericInteract_22);
			}
		} else {
			if (obj1.hasProperty(WORN)) {
				if (airless()) {
					dead(kStringGenericInteract_23);
				}
				obj1.disableProperty(WORN);
				_vm->renderMessage(kStringGenericInteract_24);
			} else {
				r = _rooms[AIRLOCK];
				if (r->getObject(5)->hasProperty(WORN)) {
					obj1.setProperty(WORN);
					_vm->renderMessage(kStringGenericInteract_25);
				} else {
					_vm->renderMessage(kStringGenericInteract_26);
				}
			}
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
		} else {
			if (obj1.hasProperty(WORN)) {
				if (airless())
					dead(kStringGenericInteract_27);

				obj1.disableProperty(WORN);
				_vm->renderMessage(kStringGenericInteract_28);
			} else {
				r = _rooms[AIRLOCK];
				if (r->getObject(5)->hasProperty(WORN)) {
					obj1.setProperty(WORN);
					_vm->renderMessage(kStringGenericInteract_29);
				} else
					_vm->renderMessage(kStringGenericInteract_26);
			}
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == BATHROOM_DOOR)) {
		_rooms[BATHROOM]->getObject(2)->_exitRoom = _currentRoom->getId();
		return false;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SOCKET))
		_vm->renderMessage(kStringGenericInteract_30);
	else if ((verb == ACTION_LOOK) && (obj1._id == BOOK2)) {
		_vm->renderMessage(kStringGenericInteract_31);
		waitOnInput(_timer1);
		_vm->removeMessage();
		_vm->renderMessage(kStringGenericInteract_32);
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
			} else {
				changeRoom(_inputObject[0]->_exitRoom);
			}
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
			} else {
				takeObject(*_inputObject[0]);
			}
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
				_vm->playSound(kAudioDoorOpen);
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
				_vm->playSound(kAudioDoorClose);
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

void GameManager::guardShot() {
	_vm->renderImage(2);
	_vm->renderImage(5);
	wait2(3);
	_vm->renderImage(2);

	_vm->playSound(kAudioVoiceHalt);
	while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle))
		wait2(1);

	_vm->renderImage(5);
	wait2(5);
	_vm->renderImage(3);
	wait2(3);

	shot(4, 3);
}

void GameManager::guard3Shot() {
	_vm->renderImage(1);
	wait2(3);
	_vm->playSound(kAudioVoiceHalt); // 46/0
	while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle))
		wait2(1);

	wait2(5);
	_vm->renderImage(2);
	wait2(3);
	shot(3,2);
}

}
