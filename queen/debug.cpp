/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/debug.h"
#include "queen/defs.h"
#include "queen/graphics.h"
#include "queen/input.h"
#include "queen/logic.h"
#include "queen/resource.h"
#include "queen/structs.h"

namespace Queen {

Debug::Debug(Input *input, Logic *logic, Graphics *graphics)
:	_passwordCharCount(0), _stubCount(0), _input(input), _logic(logic), _graphics(graphics) {

	memset(_password, 0, sizeof(_password));

	registerStub("zeroxpark", &Debug::jumpToRoom);
	registerStub("grimley",   &Debug::printInfo);
	registerStub("kowamori",  &Debug::toggleFastMode);
}


void Debug::registerStub(const char *password, DebugFunc debugFunc) {

	assert(_stubCount < MAX_STUB);
	_stub[_stubCount].password = password;
	_stub[_stubCount].function = debugFunc;
	++_stubCount;
}


void Debug::update(int c) {

	if (c >= 'a' && c <= 'z') {
		_password[_passwordCharCount] = (char)c;
		++_passwordCharCount;
		_passwordCharCount &= 15;

		uint k;
		for (k = 0; k < _stubCount; ++k) {
			const char *pass = _stub[k].password;
			int i = strlen(pass) - 1;
			int j = _passwordCharCount - 1;
			bool match = true;
			for (; i >= 0; --i, --j) {
				if (_password[j & 15] != pass[i]) {
					match = false;
					break;
				}
			}
			if (match) {
				(this->*(_stub[k].function))();
				break;
			}
		}
	}
}


void Debug::jumpToRoom() {

	debug(9, "Debug::jumpToRoom()");

	_graphics->textCurrentColor(INK_JOE);
	_graphics->textSet(0, 142, "Enter new room");
	_logic->update();

	int room;
	_digitTextCount = 0;
	if (_input->waitForNumber(room, digitKeyPressed, this)) {
		_logic->joeX(0);
		_logic->joeY(0);
		_logic->newRoom(room);
		_logic->entryObj(_logic->roomData(room) + 1);
		_graphics->textClear(0, 199);
	}
}


void Debug::toggleFastMode() {

	debug(9, "Debug::toggleFastMode()");
	_input->fastMode(!_input->fastMode());
}


void Debug::printInfo() {

	debug(9, "Debug::printInfo()");

	_graphics->textClear(0, 199);
	_graphics->textCurrentColor(INK_JOE);

	char buf[100];

	snprintf(buf, sizeof(buf), "Version : %s", _logic->resource()->JASVersion());
	_graphics->textSet(110, 20, buf);

	snprintf(buf, sizeof(buf), "Room number : %d", _logic->currentRoom());
	_graphics->textSet(110, 40, buf);

	snprintf(buf, sizeof(buf), "Room name : %s", _logic->roomName(_logic->currentRoom()));
	_graphics->textSet(110, 60, buf);

	_logic->update();

	char c;
	if (_input->waitForCharacter(c)) {
		switch (c) {
		case 'a':
			toggleAreasDrawing();
			break;
		case 's' :
			changeGameState();
            break;
        case 'x' :
			printGameState();
            break;
        case 'i' :
			giveAllItems();
            break;
		}
	}
	_graphics->textClear(0, 199);
}


void Debug::toggleAreasDrawing() {

	debug(9, "Debug::toggleAreasDrawing()");
	warning("Debug::toggleAreasDrawing() unimplemented");
}


void Debug::changeGameState() {

	debug(9, "Debug::changeGameState()");
	_graphics->textSet(0, 142, "Set GAMESTATE");
	_logic->update();
	int slot, value;
	_digitTextCount = 0;
	if (_input->waitForNumber(slot, digitKeyPressed, this)) {
		_graphics->textClear(0, 199);
		_graphics->textSet(0, 142, "to");
		_logic->update();
		_digitTextCount = 0;
		if (_input->waitForNumber(value, digitKeyPressed, this)) {
			_logic->gameState(slot, value);
		}
	}
}


void Debug::printGameState() {

	debug(9, "Debug::printGameState()");
	_graphics->textSet(0, 142, "Show GAMESTATE");
	_logic->update();
	int slot;
	_digitTextCount = 0;
	if (_input->waitForNumber(slot, digitKeyPressed, this)) {
		_graphics->textClear(0, 199);
		char buf[50];
		snprintf(buf, sizeof(buf), "Currently - %d", _logic->gameState(slot));
		_graphics->textSet(0, 142, buf);
		_logic->update();
		char c;
		_input->waitForCharacter(c);
	}
}


void Debug::giveAllItems() {

	debug(9, "Debug::giveAllItems()");
	int n = _logic->itemDataCount();
	ItemData *item = _logic->itemData(1);
	while (n--) {
		item->name = ABS(item->name);
		++item;
	}
}


void Debug::digitKeyPressed(void *refCon, int key) {

	Debug *debug = (Debug *)refCon;
	if(key != -1 && debug->_digitTextCount < sizeof(debug->_digitText) - 1) {
		debug->_digitText[debug->_digitTextCount] = (char)key;
		++debug->_digitTextCount;
	}
	else if (debug->_digitTextCount > 0) {
		--debug->_digitTextCount;
	}
	debug->_digitText[debug->_digitTextCount] = '\0';
	debug->_graphics->textSet(0, 151, debug->_digitText);
	debug->_logic->update();
}


}
