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


static const char *timeToString(int t) {
	// TODO: Does ScummVM emulate PIT timings for DOS?

	static char s[9];
	strcpy(s," 0:00:00");
	s[7] = t % 10 + '0';
	t /= 10;
	s[6] = t %  6 + '0';
	t /=  6;
	s[4] = t % 10 + '0';
	t /= 10;
	s[3] = t %  6 + '0';
	t /=  6;
	s[1] = t % 10 + '0';
	t /= 10;
	if (t)
		s[0] = t+48;

	return(s);
}

GameManager::GameManager(SupernovaEngine *vm) {
	_rooms[INTRO] = new StartingItems(vm, this);
	_rooms[CORRIDOR] = new ShipCorridor(vm, this);
	_rooms[HALL] = new ShipHall(vm, this);
	_rooms[SLEEP] = new ShipSleepCabin(vm, this);
	_rooms[COCKPIT] = new ShipCockpit(vm, this);
	_rooms[AIRLOCK] = new ShipAirlock(vm, this);
	_rooms[HOLD] = new ShipHold(vm, this);
	_rooms[LANDINGMODULE] = new ShipLandingModule(vm, this);
	_rooms[GENERATOR] = new ShipGenerator(vm, this);
	_rooms[OUTSIDE] = new ShipOuterSpace(vm, this);
	_rooms[CABIN_R1] = new ShipCabinR1(vm, this);
	_rooms[CABIN_R2] = new ShipCabinR2(vm, this);
	_rooms[CABIN_R3] = new ShipCabinR3(vm, this);
	_rooms[CABIN_L1] = new ShipCabinL1(vm, this);
	_rooms[CABIN_L2] = new ShipCabinL2(vm, this);
	_rooms[CABIN_L3] = new ShipCabinL3(vm, this);
	_rooms[BATHROOM] = new ShipCabinBathroom(vm, this);

//	_rooms[ROCKS]
//	_rooms[CAVE]
//	_rooms[MEETUP]
//	_rooms[ENTRANCE]
//	_rooms[REST]
//	_rooms[ROGER]
//	_rooms[GLIDER]
//	_rooms[MEETUP2]
//	_rooms[MEETUP3]

//	_rooms[CELL]
//	_rooms[CORRIDOR1]
//	_rooms[CORRIDOR2]
//	_rooms[CORRIDOR3]
//	_rooms[CORRIDOR4]
//	_rooms[CORRIDOR5]
//	_rooms[CORRIDOR6]
//	_rooms[CORRIDOR7]
//	_rooms[CORRIDOR8]
//	_rooms[CORRIDOR9]
//	_rooms[BCORRIDOR]
//	_rooms[GUARD]
//	_rooms[GUARD3]
//	_rooms[OFFICE_L1]
//	_rooms[OFFICE_L2]
//	_rooms[OFFICE_R1]
//	_rooms[OFFICE_R2]
//	_rooms[OFFICE_L]
//	_rooms[ELEVATOR]
//	_rooms[STATION]
//	_rooms[SIGN]

	_currentRoom = _rooms[INTRO];
	_vm = vm;
	Object::setObjectNull(_currentInputObject);
	Object::setObjectNull(_inputObject[0]);
	Object::setObjectNull(_inputObject[1]);
	_inputVerb = ACTION_WALK;
	_processInput = false;
	_mouseField = -1;
	_inventoryScroll = 0;

	initGui();
}

void GameManager::initGui() {
	int x = 0;
	for (int i = 0; i < ARRAYSIZE(_guiCommandButton); ++i) {
		int width;
		if (i < 9)
			width = _vm->textWidth(guiCommand_DE[i]) + 2;
		else
			width = 320 - x;

		_guiCommandButton[i].setSize(x, 150, x + width, 159);
		_guiCommandButton[i].setText(guiCommand_DE[i]);
		_guiCommandButton[i].setColor(kColorWhite25, kColorDarkGreen, kColorWhite44, kColorGreen);
		x += width + 2;
	}

	// Inventory + Inventory Arrows
	for (int i = 0; i < ARRAYSIZE(_guiInventory); ++i) {
		int x = 136 * (i % 2);
		int y = 161 + 10 * (i / 2);

		_guiInventory[i].setSize(x, y, x + 135, y + 9);
		_guiInventory[i].setColor(kColorWhite25, kColorDarkRed, kColorWhite35, kColorRed);
	}

	// Minimap

	// Status (?)
}

void GameManager::processInput(Common::KeyState &state) {
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
		} else {
			_key = state.ascii;
		}
		break;
	default:
		_key = state.ascii;
	}
}

void GameManager::resetInputState() {
	Object::setObjectNull(_inputObject[0]);
	Object::setObjectNull(_inputObject[1]);
	_inputVerb = ACTION_WALK;
	_processInput = false;

	processInput(Common::EVENT_MOUSEMOVE, _mouseX, _mouseY);
}

void GameManager::processInput(Common::EventType eventType, int x, int y) {
	_mouseClickType = eventType;
	_mouseX = x;
	_mouseY = y;

	if (_mouseClickType == Common::EVENT_LBUTTONUP) {
		_vm->removeMessage();
		if (Object::isNullObject(_currentInputObject))
			return;

		if (_mouseField >= 256 && _mouseField < 512) {
			resetInputState();
			_inputVerb = static_cast<Action>(_mouseField - 256);
			drawStatus();
		}
		if (_inputVerb == ACTION_GIVE || _inputVerb == ACTION_USE) {
			if (Object::isNullObject(_inputObject[0]))
				_inputObject[0] = _currentInputObject;
			else
				_inputObject[1] = _currentInputObject;
		} else {
			_inputObject[0] = _currentInputObject;
			_processInput = true;
		}

	} else if (_mouseClickType == Common::EVENT_RBUTTONUP) {
		_vm->removeMessage();
		if (Object::isNullObject(_currentInputObject))
			return;

		ObjectType type;
		if (((_mouseField >= 0) && (_mouseField < 256)) ||
		    ((_mouseField >= 512) && (_mouseField < 768))) {
			_inputObject[0] = _currentInputObject;
			type = _inputObject[0]->_type;
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
		if ((y >= _guiCommandButton[0].top) && (y <= _guiCommandButton[0].bottom)) {
			field = 9;
			while (x < _guiCommandButton[field].left - 1)
				field--;
			field += 256;
		}
		/* exit box? */
		else if ((x >= 283) && (x <= 317) && (y >= 163) && (y <= 197)) {
			field = _exitList[(x - 283) / 7 + 5 * ((y - 163) / 7)];
		}
		/* inventory box */
		else if ((y >= 161) && (x <= 270)) {
			field = (x + 1) / 136 + ((y - 161) / 10) * 2;
			if (field + _inventoryScroll < _inventory.getSize())
				field += 512;
			else
				field = -1;
		}
		/* inventory arrows */
		else if ((y >= 161) && (x >= 271) && (x < 279)) {
			if (y > 180)
				field = 769;
			else
				field = 768;
		}
		/* normal item */
		else {
			for (int i = 0; (_currentRoom->getObject(i)->_name[0] != '\0') && (field == -1); i++) {
				click = _currentRoom->getObject(i)->_click;
				if (click != 255) {
					MSNImageDecoder::ClickField *clickField = _vm->_currentImage->_clickField;
					do {
						if ((x >= clickField[click].x1) && (x <= clickField[click].x2) &&
						    (y >= clickField[click].y1) && (y <= clickField[click].y2))
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

		drawStatus();
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

bool GameManager::isHelmetOff() {
	Object *helmet = _inventory.get(HELMET);
	if (helmet && helmet->hasProperty(WORN)) {
		_vm->renderMessage("Irgendwie ist ein Raumhelm|beim Essen unpraktisch.");
		return false;
	}

	return true;
}

void GameManager::great(uint number) {
	if (number && (_state.greatF & (1 << number)))
		return;

	_vm->playSound(kAudioUndef7);
	_state.greatF |= 1 << number;
}

bool GameManager::airless() {
	return (
	((_currentRoom >  _rooms[AIRLOCK]) && (_currentRoom < _rooms[CABIN_R1])) ||
	((_currentRoom >  _rooms[BATHROOM])&& (_currentRoom < _rooms[ENTRANCE])) ||
	((_currentRoom == _rooms[AIRLOCK]) && (_currentRoom->getObject(1)->hasProperty(OPENED))) ||
	(_currentRoom  >= _rooms[MEETUP2])
	);
}

void GameManager::turnOff() {
	if (_state.powerOff)
		return;

	_state.powerOff = true;
	roomBrightness();

}
void GameManager::turnOn() {
	// STUB
}

void GameManager::takeObject(Object &obj) {
	if (obj.hasProperty(CARRIED))
		return;

	if (obj._section != 0)
		_vm->renderImage(_currentRoom->getFileNumber(), obj._section);
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
		_vm->renderText(_inventory.get(i + _inventoryScroll)->_name,
		                _guiInventory[i]._textPosition.x,
		                _guiInventory[i]._textPosition.y,
		                _guiInventory[i]._textColor);
	}

	_vm->renderBox(272, 161, 7, 19, HGR_INV);
	_vm->renderBox(272, 181, 7, 19, HGR_INV);
}

void GameManager::mouseInput() {
	// STUB
}

void GameManager::mouseInput2() {
	// STUB
}

void GameManager::mouseInput3() {
	// STUB
}

void GameManager::mouseWait(int delay) {
	// STUB
}

void GameManager::roomBrightness() {
	// STUB
}

void GameManager::loadTime() {
	// STUB
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

void GameManager::wait2(int delay) {
	// STUB
}

void GameManager::screenShake() {
	// STUB
}

void GameManager::palette() {
	// STUB
	// Adjust palette to brightness parameters and make it current
}

void GameManager::shock() {
	// STUB
	_vm->playSound(kAudioShock);
	// die
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
	// STUB
}

void GameManager::animationOn() {
	// STUB
}

void GameManager::edit(char *text, int x, int y, int length) {
	// STUB
}

void GameManager::loadOverlayStart() {
	// STUB
}

void GameManager::drawStatus() {
	int index = static_cast<int>(_inputVerb);
	_vm->renderBox(0, 140, 320, 9, HGR_BEF_ANZ);
	_vm->renderText(guiStatusCommand_DE[index], 1, 141, COL_BEF_ANZ);

	if (Object::isNullObject(_inputObject[0])) {
		_vm->renderText(_currentInputObject->_name);
	} else {
		_vm->renderText(_inputObject[0]->_name);
		if (_inputVerb == ACTION_GIVE) {
			// to
			_vm->renderText(" an ");
		} else if (_inputVerb == ACTION_USE) {
			// with
			_vm->renderText(" mit ");
		}

		_vm->renderText(_currentInputObject->_name);
	}
}

void GameManager::openLocker(const Room *room, Object *obj, Object *lock, int section) {
	_vm->renderImage(room->getFileNumber(), section);
	obj->setProperty(OPENED);
	lock->_click = 255;
	int i = obj->_click;
	obj->_click = obj->_click2;
	obj->_click2 = i;
}

void GameManager::closeLocker(const Room *room, Object *obj, Object *lock, int section) {
	// STUB
}

int GameManager::invertSection(int section) {
	if (section < 128)
		section += 128;
	else
		section -= 128;

	return section;
}


bool GameManager::genericInteract(Action verb, Object &obj1, Object &obj2) {
	Room *r;
	char t[150];

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
			_vm->renderMessage("Du hast es doch schon geffnet.");
		} else {
			takeObject(*_rooms[ENTRANCE]->getObject(8));
			_vm->renderMessage("In dem Ei ist eine Tablette|in einer Plastikhlle.");
			obj1.setProperty(OPENED);
		}
	} else if ((verb == ACTION_USE) && (obj1._id == PILL)) {
		if (isHelmetOff()) {
			_vm->renderMessage("Du iát die Tablette und merkst,|daá sich irgendetwas verndert hat.");
			great(0);
			_inventory.remove(obj1);
			_state.language = 2;
			takeObject(*_rooms[ENTRANCE]->getObject(17));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == PILL_HULL) &&
	           (_state.language == 2)) {
		_vm->renderMessage("Komisch! Auf einmal kannst du die Schrift lesen!|Darauf steht:\"Wenn Sie diese Schrift jetzt|lesen knnen, hat die Tablette gewirkt.\"");
		_state.language = 1;
	} else if ((verb == ACTION_OPEN) && (obj1._id == WALLET)) {
		if (!_rooms[ROGER]->getObject(3)->hasProperty(CARRIED)) {
			_vm->renderMessage("Das muát du erst nehmen.");
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
		// TODO: Implement a 'newspaper room' to eliminate handling
		//       those cases seperately
		_vm->renderImage(2, 0);
		_vm->setColor63(40);
		mouseInput2();
		_vm->renderRoom(*_currentRoom);
		roomBrightness();
		palette();
		showMenu();
		drawMapExits();
		_vm->renderMessage("Hmm, irgendwie komme|ich mir verarscht vor.");
	} else if ((verb == ACTION_LOOK) && (obj1._id == KEYCARD2)) {
		_vm->renderMessage(obj1._description);
		obj1._description = "Es ist die Keycard des Commanders.";
	} else if ((verb == ACTION_LOOK) && (obj1._id == WATCH)) {
		_vm->renderMessage(Common::String::format(
		    "Es ist eine Uhr mit extra|lautem Wecker. "
		    "Sie hat einen|Knopf zum Verstellen der Alarmzeit.|"
		    "Uhrzeit: %s   Alarmzeit: %s",
		    timeToString(_vm->getDOSTicks() - _state.timeStarting),
		    timeToString(_state.timeAlarm)).c_str());
	} else if ((verb == ACTION_PRESS) && (obj1._id == WATCH)) {
		char *min;
		int hours, minutes;
		bool f;
		animationOff();
		_vm->saveScreen(88, 87, 144, 24);
		_vm->renderBox(88, 87, 144, 24, kColorWhite35);
		_vm->renderText("Neue Alarmzeit (hh:mm) :", 91, 90, kColorWhite99);
		do {
			t[0] = 0;
			_vm->renderBox(91, 99, 138, 9, kColorDarkBlue);
			do {
				edit(t, 91, 100, 5);
			} while ((_key != Common::ASCII_RETURN) && (_key != Common::ASCII_ESCAPE));
			f = false;
			if (t[0] == ':') {
				t[0] = 0;
				min = &(t[1]);
			} else if (t[1] == ':') {
				t[1] = 0;
				min = &(t[2]);
			} else if (t[2] == ':') {
				t[2] = 0;
				min = &(t[3]);
			} else {
				f = true;
			}

			for (uint i = 0; i < strlen(t); i++)
				if ((t[i] < '0') || (t[i] > '9')) f = true;
			for (uint i = 0; i < strlen(min); i++)
				if ((min[i] < '0') || (min[i] > '9')) f = true;
			hours = atoi(t);
			minutes = atoi(min);
			if ((hours > 23) || (minutes > 59)) f = true;
			animationOn();
		} while (f && (_key != Common::ASCII_ESCAPE));
		_vm->restoreScreen();
		if (_key != Common::ASCII_ESCAPE) {
			_state.timeAlarm = (hours * 60 + minutes) * 1092.3888 + 8;
			_state.timeAlarmSystem = _state.timeAlarm + _state.timeStarting;
			_state.alarmOn = (_state.timeAlarmSystem > _vm->getDOSTicks());
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, WIRE)) {
		r = _rooms[CABIN_L3];
		if (!r->getObject(8)->hasProperty(CARRIED)) {
			if (r->isSectionVisible(26))
				_vm->renderMessage(Object::takeMessage);
			else
				return false;
		} else {
			r->getObject(8)->_name = "Leitung mit Lsterklemme";
			r = _rooms[HOLD];
			_inventory.remove(*r->getObject(2));
			_state.terminalStripConnected = true;
			_state.terminalStripWire = true;
			_vm->renderMessage("Ok.");
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, SPOOL)) {
		r = _rooms[CABIN_L2];
		takeObject(*r->getObject(9));
		r->getObject(9)->_name = "Kabelrolle mit Lsterklemme";
		r = _rooms[HOLD];
		_inventory.remove(*r->getObject(2));
		_state.terminalStripConnected = true;
		_vm->renderMessage("Ok.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SPOOL)) {
		r = _rooms[CABIN_L3];
		if (!_state.terminalStripConnected) {
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
				_state.cableConnected = true;
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
					_vm->renderMessage("Du muát erst den Helm abnehmen.");
				} else if (r->getObject(6)->hasProperty(WORN)) {
					_vm->renderMessage("Du muát erst den Versorgungsteil abnehmen.");
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
					//TODO: Death screen
//					longjmp(dead, "Den Helm httest du|besser angelassen!");
				}
				obj1.disableProperty(WORN);
				_vm->renderMessage("Du ziehst den Helm ab.");
			} else {
				r = _rooms[AIRLOCK];
				if (r->getObject(5)->hasProperty(WORN)) {
					obj1.setProperty(WORN);
					_vm->renderMessage("Du ziehst den Helm auf.");
				} else {
					_vm->renderMessage("Du muát erst den Anzug anziehen.");
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
					//TODO: Death screen
//					longjmp(dead, "Den Versorungsteil httest du|besser nicht abgenommen!");
				}
				obj1.disableProperty(WORN);
				_vm->renderMessage("Du nimmst den Versorgungsteil ab.");
			} else {
				r = _rooms[AIRLOCK];
				if (r->getObject(5)->hasProperty(WORN)) {
					obj1.setProperty(WORN);
					_vm->renderMessage("Du ziehst den Versorgungsteil an.");
				} else {
					_vm->renderMessage("Du muát erst den Anzug anziehen.");
				}
			}
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == BATHROOM_DOOR)) {
//		*bathroom = current_room;
		return false;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SOCKET))
		_vm->renderMessage("Die Leitung ist hier unntz.");
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

void GameManager::executeRoom() {
	if (!_vm->_messageDisplayed)
		_vm->renderRoom(*_currentRoom);
	drawMapExits();
	drawInventory();
	drawStatus();
	drawCommandBox();

	animationOn();
	roomBrightness();
	if (_vm->_brightness == 0)
		_vm->paletteFadeIn();
	else
		_vm->paletteBrightness();
	if (!_currentRoom->hasSeen())
		_currentRoom->onEntrance();

	if (_vm->_messageDisplayed || !_processInput)
		return;

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
				_vm->renderMessage("Das trgst du doch bei dir.");
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
				_vm->renderMessage("Das lát sich nicht ffnen.");
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
				_vm->renderMessage("Das lát sich nicht schlieáen.");
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

		if (_newOverlay) {
			loadOverlayStart();
			_newOverlay = false;
		}
		if (_newRoom) {
			_newRoom = false;
		}
	}

	resetInputState();
}

}
