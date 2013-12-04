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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "avalanche/avalanche.h"
#include "avalanche/nim.h"

namespace Avalanche {

const char * const Nim::kNames[2] = {"Avalot", "Dogfood"};

Nim::Nim(AvalancheEngine *vm) {
	_vm = vm;

	_playedNim = 0;
}

void Nim::resetVariables() {
	_playedNim = 0;
}

void Nim::synchronize(Common::Serializer &sz) {
	if (sz.isLoading() && sz.getVersion() < 2)
		return;

	sz.syncAsByte(_playedNim);
}

void Nim::playNim() {
	if (_vm->_wonNim) { // Already won the game.
		_vm->_dialogs->displayScrollChain('Q', 6);
		return;
	}

	if (!_vm->_askedDogfoodAboutNim) {
		_vm->_dialogs->displayScrollChain('Q', 84);
		return;
	}

	_vm->_dialogs->displayScrollChain('Q', 3);
	_playedNim++;
	_vm->fadeOut();

	_vm->_graphics->saveScreen();

	CursorMan.showMouse(false);
	setup();
	board();
	CursorMan.showMouse(true);

	do {
		startMove();
		if (_dogfoodsTurn)
			dogFood();
		else
			takeSome();
		_stones[_row] -= _number;
		showChanges();
	} while (_stonesLeft != 0);

	endOfGame(); // Winning sequence is A1, B3, B1, C1, C1, btw.

	_vm->fadeOut();
	CursorMan.showMouse(false);

	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();

	CursorMan.showMouse(true);
	_vm->fadeIn();

	if (_dogfoodsTurn) {
		// Dogfood won - as usual.
		if (_playedNim == 1)   // Your first game.
			_vm->_dialogs->displayScrollChain('Q', 4); // Goody! Play me again?
		else
			_vm->_dialogs->displayScrollChain('Q', 5); // Oh, look at that! I've won again!
		_vm->decreaseMoney(4); // And you've just lost 4d!
	} else {
		// You won - strange!
		_vm->_dialogs->displayScrollChain('Q', 7);
		_vm->_objects[kObjectLute - 1] = true;
		_vm->refreshObjectList();
		_vm->_wonNim = true;
		_vm->_background->draw(-1, -1, 0); // Show the settle with no lute on it.
		
		// 7 points for winning!
		_vm->incScore(7);
	}

	if (_playedNim == 1) {
		// 3 points for playing your 1st game.
		_vm->incScore(3);
	}
}

void Nim::chalk(int x,int y, Common::String z) {
	warning("STUB: Nim::chalk()");
}

void Nim::setup() {
	warning("STUB: Nim::setup()");
}

void Nim::plotStone(byte x,byte y) {
	warning("STUB: Nim::plotStone()");
}

void Nim::board() {
	warning("STUB: Nim::board()");
}

void Nim::startMove() {
	warning("STUB: Nim::startMove()");
}

void Nim::showChanges() {
	warning("STUB: Nim::showChanges()");
}

void Nim::blip() {
	warning("STUB: Nim::blip()");
}

void Nim::checkMouse() {
	warning("STUB: Nim::checkMouse()");
}

void Nim::less() {
	warning("STUB: Nim::less()");
}

void Nim::takeSome() {
	warning("STUB: Nim::takeSome()");
}

void Nim::endOfGame() {
	warning("STUB: Nim::endOfGame()");
}

void Nim::dogFood() {
	warning("STUB: Nim::dogFood()");
}

bool Nim::find(byte x) {
	warning("STUB: Nim::find()");
	return true;
}

void Nim::findAp(byte start,byte stepsize) {
	warning("STUB: Nim::findAp()");
}

} // End of namespace Avalanche
