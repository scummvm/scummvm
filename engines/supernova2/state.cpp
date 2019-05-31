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

#include "supernova2/screen.h"
#include "supernova2/supernova2.h"
#include "supernova2/state.h"

namespace Supernova2 {


StringId GameManager::guiCommands[] = {
	kStringCommandGo, kStringCommandLook, kStringCommandTake, kStringCommandOpen, kStringCommandClose,
	kStringCommandPress, kStringCommandPull, kStringCommandUse, kStringCommandTalk, kStringCommandGive
};

StringId GameManager::guiStatusCommands[] = {
	kStringStatusCommandGo, kStringStatusCommandLook, kStringStatusCommandTake, kStringStatusCommandOpen, kStringStatusCommandClose,
	kStringStatusCommandPress, kStringStatusCommandPull, kStringStatusCommandUse, kStringStatusCommandTalk, kStringStatusCommandGive
};

void Inventory::add(Object &obj) {
	if (_numObjects < kMaxCarry) {
		_inventory[_numObjects++] = &obj;
		obj.setProperty(CARRIED);
	}

	if (getSize() > _inventoryScroll + 8) {
		_inventoryScroll = getSize() - 8;
		_inventoryScroll += _inventoryScroll % 2;
	}
}

void Inventory::remove(Object &obj) {
	for (int i = 0; i < _numObjects; ++i) {
		if (_inventory[i] == &obj) {
			if (_inventoryScroll >= 2 && getSize() % 2)
				_inventoryScroll -= 2;

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
	for (int i = 0; i < _numObjects; ++i)
		_inventory[i]->disableProperty(CARRIED);
	_numObjects = 0;
	_inventoryScroll = 0;
}

Object *Inventory::get(int index) const {
	if (index < _numObjects)
		return _inventory[index];

	return _nullObject;
}

Object *Inventory::get(ObjectId id) const {
	for (int i = 0; i < _numObjects; ++i) {
		if (_inventory[i]->_id == id)
			return _inventory[i];
	}

	return _nullObject;
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
	memset(_text, 0, sizeof(_text));
}

void GuiElement::setText(const char *text) {
	strncpy(_text, text, sizeof(_text) - 1);
}

void GuiElement::setTextPosition(int x, int y) {
	_textPosition = Common::Point(x, y);
}

void GuiElement::setSize(int x1, int y1, int x2, int y2) {
	this->left = x1;
	this->top = y1;
	this->right = x2;
	this->bottom = y2;

	_textPosition = Common::Point(x1 + 1, y1 + 1);
}

void GuiElement::setColor(int bgColor, int textColor, int bgColorHighlighted, int textColorHightlighted) {
	_bgColor = bgColor;
	_textColor = textColor;
	_bgColorNormal = bgColor;
	_textColorNormal = textColor;
	_bgColorHighlighted = bgColorHighlighted;
	_textColorHighlighted = textColorHightlighted;
}

void GuiElement::setHighlight(bool isHighlighted_) {
	if (isHighlighted_) {
		_bgColor = _bgColorHighlighted;
		_textColor = _textColorHighlighted;
	} else {
		_bgColor = _bgColorNormal;
		_textColor = _textColorNormal;
	}
}

GameManager::GameManager(Supernova2Engine *vm)
	: _inventory(&_nullObject, _inventoryScroll)
	, _vm(vm)
    , _mouseClickType(Common::EVENT_INVALID) {
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
	delete _rooms[AIRPORT];
	delete _rooms[TAXISTAND];
}

void GameManager::initState() {
	_currentInputObject = &_nullObject;
	_inputObject[0] = &_nullObject;
	_inputObject[1] = &_nullObject;
	_inputVerb = ACTION_WALK;
	_processInput = false;
	_guiEnabled = true;
	_animationEnabled = true;
	_mouseClicked = false;
	_keyPressed = false;
	_mouseX = -1;
	_mouseY = -1;
	_mouseField = -1;
	_inventoryScroll = 0;
	_oldTime = g_system->getMillis();
	_timerPaused = 0;
	_timePaused = false;
	_messageDuration = 0;
	_animationTimer = 0;
	_taxi_possibility = 4;

	_currentSentence = -1;
	for (int i = 0 ; i < 6 ; ++i) {
		_sentenceNumber[i] = -1;
		_texts[i] = kNoString;
		_rows[i] = 0;
		_rowsStart[i] = 0;
		_dials[i] = 1;
	}

	_prevImgId = 0;

	_state._money = 20;
	_state._addressKnown = false;
	_state._previousRoom = _currentRoom;
}

void GameManager::initRooms() {
	_rooms[INTRO] = new Intro(_vm, this);
	_rooms[AIRPORT] = new Airport(_vm, this);
	_rooms[TAXISTAND] = new TaxiStand(_vm, this);
}

void GameManager::initGui() {
	int cmdCount = ARRAYSIZE(_guiCommandButton);
	int cmdAvailableSpace = 320 - (cmdCount - 1) * 2;
	for (int i = 0; i < cmdCount; ++i) {
		const Common::String &text = _vm->getGameString(guiCommands[i]);
		cmdAvailableSpace -= Screen::textWidth(text);
	}

	int commandButtonX = 0;
	for (int i = 0; i < ARRAYSIZE(_guiCommandButton); ++i) {
		const Common::String &text = _vm->getGameString(guiCommands[i]);
		int width;
		if (i < cmdCount - 1) {
			int space = cmdAvailableSpace / (cmdCount - i);
			cmdAvailableSpace -= space;
			width = Screen::textWidth(text) + space;
		} else
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

void GameManager::updateEvents() {
	handleTime();
	if (_animationEnabled && !_vm->_screen->isMessageShown() && _animationTimer == 0)
		_currentRoom->animation();

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
		//_vm->setTextSpeed();
		break;
	case Common::KEYCODE_F5:
		// load/save
		break;
	case Common::KEYCODE_x:
		if (state.flags & Common::KBD_ALT) {
			//if (_vm->quitGameDialog())
				_vm->quitGame();
		}
		break;
	case Common::KEYCODE_d:
		if (state.flags & Common::KBD_CTRL)
			_vm->_console->attach();
		break;
	default:
		break;
	}
}

void GameManager::resetInputState() {
	setObjectNull(_inputObject[0]);
	setObjectNull(_inputObject[1]);
	_inputVerb = ACTION_WALK;
	_processInput = false;
	_mouseClicked = false;
	_keyPressed = false;
	_key.reset();
	_mouseClickType = Common::EVENT_MOUSEMOVE;

	processInput();
}


void GameManager::processInput() {
	enum {
		onNone,
		onObject,
		onCmdButton,
		onInventory,
		onInventoryArrowUp,
		onInventoryArrowDown
	} mouseLocation;

	if (_mouseField >= 0 && _mouseField < 256)
		mouseLocation = onObject;
	else if (_mouseField >= 256 && _mouseField < 512)
		mouseLocation = onCmdButton;
	else if (_mouseField >= 512 && _mouseField < 768)
		mouseLocation = onInventory;
	else if (_mouseField == 768)
		mouseLocation = onInventoryArrowUp;
	else if (_mouseField == 769)
		mouseLocation = onInventoryArrowDown;
	else
		mouseLocation = onNone;

	if (_mouseClickType == Common::EVENT_LBUTTONUP) {
		if (_vm->_screen->isMessageShown()) {
			// Hide the message and consume the event
			_vm->removeMessage();
			if (mouseLocation != onCmdButton)
				return;
		}

		switch(mouseLocation) {
		case onObject:
		case onInventory:
			// Fallthrough
			if (_inputVerb == ACTION_GIVE || _inputVerb == ACTION_USE) {
				if (isNullObject(_inputObject[0])) {
					_inputObject[0] = _currentInputObject;
					if (!_inputObject[0]->hasProperty(COMBINABLE))
						_processInput = true;
				} else {
					_inputObject[1] = _currentInputObject;
					_processInput = true;
				}
			} else {
				_inputObject[0] = _currentInputObject;
				if (!isNullObject(_currentInputObject))
					_processInput = true;
			}
			break;
		case onCmdButton:
			resetInputState();
			_inputVerb = static_cast<Action>(_mouseField - 256);
			break;
		case onInventoryArrowUp:
			if (_inventoryScroll >= 2)
				_inventoryScroll -= 2;
			break;
		case onInventoryArrowDown:
			if (_inventoryScroll < _inventory.getSize() - ARRAYSIZE(_guiInventory))
				_inventoryScroll += 2;
			break;
		case onNone:
			break;
		}

	} else if (_mouseClickType == Common::EVENT_RBUTTONUP) {
		if (_vm->_screen->isMessageShown()) {
			// Hide the message and consume the event
			_vm->removeMessage();
			return;
		}

		if (isNullObject(_currentInputObject))
			return;

		if (mouseLocation == onObject || mouseLocation == onInventory) {
			_inputObject[0] = _currentInputObject;
			ObjectTypes type = _inputObject[0]->_type;
			if (type & OPENABLE)
				_inputVerb = (type & OPENED) ? ACTION_CLOSE : ACTION_OPEN;
			else if (type & PRESS)
				_inputVerb = ACTION_PRESS;
			else if (type & TALK)
				_inputVerb = ACTION_TALK;
			else
				_inputVerb = ACTION_LOOK;

			_processInput = true;
		}

	} else if (_mouseClickType == Common::EVENT_MOUSEMOVE) {
		int field = -1;
		int click = -1;

		if ((_mouseY >= _guiCommandButton[0].top) && (_mouseY <= _guiCommandButton[0].bottom)) {
			/* command row */
			field = 9;
			while (_mouseX < _guiCommandButton[field].left - 1)
				field--;
			field += 256;
		} else if ((_mouseX >= 283) && (_mouseX <= 317) && (_mouseY >= 163) && (_mouseY <= 197)) {
			/* exit box */
			field = _exitList[(_mouseX - 283) / 7 + 5 * ((_mouseY - 163) / 7)];
		} else if ((_mouseY >= 161) && (_mouseX <= 270)) {
			/* inventory box */
			field = (_mouseX + 1) / 136 + ((_mouseY - 161) / 10) * 2;
			if (field + _inventoryScroll < _inventory.getSize())
				field += 512;
			else
				field = -1;
		} else if ((_mouseY >= 161) && (_mouseX >= 271) && (_mouseX < 279)) {
			/* inventory arrows */
			field = (_mouseY > 180) ? 769 : 768;
		} else {
			/* normal item */
			for (int i = 0; (_currentRoom->getObject(i)->_id != INVALIDOBJECT) &&
							(field == -1) && i < kMaxObject; i++) {
				click = _currentRoom->getObject(i)->_click;
				const MS2Image *image = _vm->_screen->getCurrentImage();
				if (click != 255 && image) {
					const MS2Image::ClickField *clickField = image->_clickField;
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
			switch (mouseLocation) {
			case onInventoryArrowUp:
			case onInventoryArrowDown:
				// Fallthrough
				_guiInventoryArrow[_mouseField - 768].setHighlight(false);
				break;
			case onInventory:
				_guiInventory[_mouseField - 512].setHighlight(false);
				break;
			case onCmdButton:
				_guiCommandButton[_mouseField - 256].setHighlight(false);
				break;
			case onObject:
			case onNone:
				// Fallthrough
				break;
			}

			setObjectNull(_currentInputObject);

			_mouseField = field;
			if (_mouseField >= 0 && _mouseField < 256)
				mouseLocation = onObject;
			else if (_mouseField >= 256 && _mouseField < 512)
				mouseLocation = onCmdButton;
			else if (_mouseField >= 512 && _mouseField < 768)
				mouseLocation = onInventory;
			else if (_mouseField == 768)
				mouseLocation = onInventoryArrowUp;
			else if (_mouseField == 769)
				mouseLocation = onInventoryArrowDown;
			else
				mouseLocation = onNone;

			switch (mouseLocation) {
			case onInventoryArrowUp:
			case onInventoryArrowDown:
				// Fallthrough
				_guiInventoryArrow[_mouseField - 768].setHighlight(true);
				break;
			case onInventory:
				_guiInventory[_mouseField - 512].setHighlight(true);
				_currentInputObject = _inventory.get(_mouseField - 512 + _inventoryScroll);
				break;
			case onCmdButton:
				_guiCommandButton[_mouseField - 256].setHighlight(true);
				break;
			case onObject:
				_currentInputObject = _currentRoom->getObject(_mouseField);
				break;
			case onNone:
				break;
			}
		}
	}
}

void GameManager::setObjectNull(Object *&obj) {
	obj = &_nullObject;
}

bool GameManager::isNullObject(Object *obj) {
	return obj == &_nullObject;
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

void GameManager::edit(Common::String &input, int x, int y, uint length) {
	bool isEditing = true;
	uint cursorIndex = input.size();
	// NOTE: Pixels for char needed = kFontWidth + 2px left and right side bearing
	int overdrawWidth = ((int)((length + 1) * (kFontWidth + 2)) > (kScreenWidth - x)) ?
						kScreenWidth - x : (length + 1) * (kFontWidth + 2);

	while (isEditing) {
		_vm->_screen->setTextCursorPos(x, y);
		_vm->_screen->setTextCursorColor(kColorWhite99);
		_vm->renderBox(x, y - 1, overdrawWidth, 9, kColorWhite35);
		for (uint i = 0; i < input.size(); ++i) {
			// Draw char highlight depending on cursor position
			if (i == cursorIndex) {
				_vm->renderBox(_vm->_screen->getTextCursorPos().x, y - 1,
							   Screen::textWidth(input[i]), 9, kColorWhite99);
				_vm->_screen->setTextCursorColor(kColorWhite35);
				_vm->renderText(input[i]);
				_vm->_screen->setTextCursorColor(kColorWhite99);
			} else
				_vm->renderText(input[i]);
		}

		if (cursorIndex == input.size()) {
			_vm->renderBox(_vm->_screen->getTextCursorPos().x + 1, y - 1, 6, 9, kColorWhite35);
			_vm->renderBox(_vm->_screen->getTextCursorPos().x, y - 1, 1, 9, kColorWhite99);
		}

		getKeyInput(true);
		if (_vm->shouldQuit())
			break;
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
			if (cursorIndex != 0)
				--cursorIndex;
			break;
		case Common::KEYCODE_RIGHT:
			if (cursorIndex != input.size())
				++cursorIndex;
			break;
		case Common::KEYCODE_DELETE:
			if (cursorIndex != input.size())
				input.deleteChar(cursorIndex);
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

void GameManager::takeMoney(int amount) {
	_state._money += amount;
	_vm->setGameString(kStringMoney, Common::String::format("%d Xa", _state._money));
}

void GameManager::drawStatus() {
	int index = static_cast<int>(_inputVerb);
	_vm->renderBox(0, 140, 320, 9, kColorWhite25);
	_vm->renderText(_vm->getGameString(guiStatusCommands[index]), 1, 141, kColorDarkGreen);

	if (isNullObject(_inputObject[0]))
		_vm->renderText(_currentInputObject->_name);
	else {
		_vm->renderText(_inputObject[0]->_name);
		if (_inputVerb == ACTION_GIVE)
			_vm->renderText(kPhrasalVerbParticleGiveTo);
		else if (_inputVerb == ACTION_USE)
			_vm->renderText(kPhrasalVerbParticleUseWith);

		_vm->renderText(_currentInputObject->_name);
	}
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

void GameManager::say(StringId textId) {
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

void GameManager::reply(StringId textId, int aus1, int aus2) {
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

int GameManager::dialog(int num, byte rowLength[6], StringId text[6], int number) {
	_vm->_allowLoadGame = false;
	_guiEnabled = false;

	bool remove[6];
	for (int i = 0; i < 6; ++i)
		remove[i] = _currentRoom->sentenceRemoved(i, number);

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
		do {
			updateEvents();
			mousePosDialog(_mouseX, _mouseY);
			g_system->updateScreen();
			g_system->delayMillis(_vm->_delay);
		} while (!_mouseClicked && !_vm->shouldQuit());
	} while (_currentSentence == -1 && !_vm->shouldQuit());

	_vm->renderBox(0, 138, 320, 62, kColorBlack);

	if (number && _currentSentence != -1 && _texts[_rowsStart[_currentSentence]] != kStringDialogSeparator)
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

void GameManager::takeObject(Object &obj) {
	if (obj.hasProperty(CARRIED))
		return;

	if (obj._section != 0)
		_vm->renderImage(obj._section);
	obj._click = obj._click2 = 255;
	_inventory.add(obj);
}

void GameManager::drawCommandBox() {
	for (int i = 0; i < ARRAYSIZE(_guiCommandButton); ++i) {
		_vm->renderBox(_guiCommandButton[i]);
		int space = (_guiCommandButton[i].width() - Screen::textWidth(_guiCommandButton[i].getText())) / 2;
		_vm->renderText(_guiCommandButton[i].getText(),
						_guiCommandButton[i].getTextPos().x + space,
						_guiCommandButton[i].getTextPos().y,
						_guiCommandButton[i].getTextColor());
	}
}

void GameManager::drawInventory() {
	for (int i = 0; i < ARRAYSIZE(_guiInventory); ++i) {
		_vm->renderBox(_guiInventory[i]);
		_vm->renderText(_inventory.get(i + _inventoryScroll)->_name,
						_guiInventory[i].getTextPos().x,
						_guiInventory[i].getTextPos().y,
						_guiInventory[i].getTextColor());
	}

	_vm->renderBox(_guiInventoryArrow[0]);
	_vm->renderBox(_guiInventoryArrow[1]);
	if (_inventory.getSize() > ARRAYSIZE(_guiInventory)) {
		if (_inventoryScroll != 0)
			_vm->renderText(_guiInventoryArrow[0]);
		if (_inventoryScroll + ARRAYSIZE(_guiInventory) < _inventory.getSize())
			_vm->renderText(_guiInventoryArrow[1]);
	}
}

uint16 GameManager::getKeyInput(bool blockForPrintChar) {
	while (!_vm->shouldQuit()) {
		updateEvents();
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
	return 0;
}

void GameManager::getInput() {
	while (!_vm->shouldQuit()) {
		updateEvents();
		if (_mouseClicked || _keyPressed)
			break;
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
}

void GameManager::changeRoom(RoomId id) {
	_currentRoom = _rooms[id];
	_newRoom = true;
}

void GameManager::wait(int ticks) {
	uint32 end = g_system->getMillis() + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		updateEvents();
		g_system->updateScreen();
	} while (g_system->getMillis() < end && !_vm->shouldQuit());
}

void GameManager::waitOnInput(int ticks) {
	uint32 end = g_system->getMillis() + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		updateEvents();
		g_system->updateScreen();
	} while (g_system->getMillis() < end && !_vm->shouldQuit() && !_keyPressed && !_mouseClicked);
}

bool GameManager::waitOnInput(int ticks, Common::KeyCode &keycode) {
	keycode = Common::KEYCODE_INVALID;
	uint32 end = g_system->getMillis() + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		updateEvents();
		g_system->updateScreen();
		if (_keyPressed) {
			keycode = _key.keycode;
			_key.reset();
			return true;
		} else if (_mouseClicked)
			return true;
	} while (g_system->getMillis() < end  && !_vm->shouldQuit());
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

	if (_animationTimer > delta)
		_animationTimer -= delta;
	else
		_animationTimer = 0;

	_oldTime = newTime;
}

int GameManager::invertSection(int section) {
	if (section < 128)
		section += 128;
	else
		section -= 128;

	return section;
}

bool GameManager::genericInteract(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_OPEN && obj1._id == WALLET) {
		if (_rooms[TAXISTAND]->getObject(4)->_type & CARRIED)
			_vm->renderMessage(kStringEmpty);
		else {
			_vm->renderMessage(kStringWalletOpen);
			takeObject(*_rooms[TAXISTAND]->getObject(4));
			takeObject(*_rooms[TAXISTAND]->getObject(5));
			takeMoney(1);
		}
	} else if (verb == ACTION_PRESS && obj1._id == TRANSMITTER) {
		if (_currentRoom == _rooms[TAXISTAND]) {
			if (_currentRoom->getObject(0)->_type != EXIT) {
				_vm->renderImage(5);
				wait(3);
				_vm->renderImage(6);
				_currentRoom->getObject(0)->_type = EXIT;
				drawMapExits();
			}
		}
	} else
		return false;
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
				_vm->renderMessage(kStringGenericInteract1);
			} else if (!_inputObject[0]->hasProperty(EXIT)) {
				// You're already there.
				_vm->renderMessage(kStringGenericInteract2);
			} else if (_inputObject[0]->hasProperty(OPENABLE) && !_inputObject[0]->hasProperty(OPENED)) {
				// This is closed
				_vm->renderMessage(kStringGenericInteract3);
			} else
				changeRoom(_inputObject[0]->_exitRoom);

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
				//_sound->play(kAudioDoorOpen);
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
				//_sound->play(kAudioDoorClose);
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

void GameManager::executeRoom() {
	if (_processInput && !_vm->_screen->isMessageShown() && _guiEnabled) {
		handleInput();
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

	if (_vm->_screen->getViewportBrightness() == 0)
		_vm->paletteFadeIn();

	if (!_currentRoom->hasSeen() && _newRoom) {
		_newRoom = false;
		_currentRoom->onEntrance();
	}
}

void GameManager::leaveTaxi() {
	_currentRoom = _state._previousRoom;
	_vm->renderRoom(*_currentRoom);
	_guiEnabled = true;
}

void GameManager::taxiUnknownDestination() {
	_vm->renderImage(invertSection(2));
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->renderImage(4);
	waitOnInput(_vm->_textSpeed * 3);
	_vm->renderImage(invertSection(4));
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->renderImage(2);
}

void GameManager::taxiPayment(int price, int destination) {
	static StringId answers[] = {
		kStringPay,
		kStringLeaveTaxi
	};
	if (dialog(2, _dials, answers, 0)) {
		leaveTaxi();
	} else if (_state._money < price) {
		Common::String t = _vm->getGameString(kStringNotEnoughMoney);
		_vm->renderMessage(t);
		waitOnInput((t.size() + 20) * _vm->_textSpeed / 10);
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
		waitOnInput((t.size() + 20) * _vm->_textSpeed / 10);
		_vm->removeMessage();

		_vm->paletteFadeOut();
		_vm->_system->fillScreen(kColorBlack);
		_vm->paletteFadeIn();

		Common::String t2 = _vm->getGameString(kString5MinutesLater);
		_vm->renderMessage(t2);
		waitOnInput((t2.size() + 20) * _vm->_textSpeed / 10);
		_vm->removeMessage();

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
		}
	}
}

void GameManager::taxi() {
	static StringId dest[] = {
		kStringAirport,
		kStringDowntown,
		kStringCulturePalace,
		kStringEarth,
		kStringPrivateApartment,
		kStringLeaveTaxi
	};
	Common::String input;
	int possibility = _taxi_possibility;
	bool paid = false;

	_state._previousRoom = _currentRoom;
	_currentRoom = _rooms[INTRO];
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
			_taxi_possibility += 8;
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
					waitOnInput((t.size() + 20) * _vm->_textSpeed / 10);
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
	_rooms[INTRO]->addAllSentences(1);
	} while(answer == 3 && !_vm->shouldQuit());

}
}

