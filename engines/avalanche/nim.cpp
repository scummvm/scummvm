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

	resetVariables();
}

void Nim::resetVariables() {
	_playedNim = 0;
	_turns = 0;
	_dogfoodsTurn = false;
	_stonesLeft = 0;
	_clicked = false;
	_row = 0;
	_number = 0;
	_squeak = false;
	_mNum = 0;
	_mRow = 0;

	for (int i = 0; i < 3; i++) {
		_old[i] = 0;
		_stones[i] = 0;
	}
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
	
	_vm->_graphics->saveScreen();
	_vm->fadeOut();

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

void Nim::chalk(int x, int y, Common::String text) {
	const Color greys[] = { kColorBlack, kColorDarkgray, kColorLightgray, kColorWhite };
	
	for (int i = 0; i < 4; i++) {
		_vm->_graphics->drawNormalText(text, _vm->_font, 8, x - i, y, greys[i]);
		_vm->_graphics->refreshScreen();
		int freq = i * 100 * text.size();
		if (freq == 0)
			_vm->_system->delayMillis(3);
		else
			_vm->_sound->playNote(freq, 3);
		_vm->_system->delayMillis(30);
	}
}

void Nim::setup() {
	_vm->fadeIn();
	_vm->_graphics->loadNim();

	_vm->_graphics->drawFilledRectangle(Common::Rect(0, 0, 640, 200), kColorBlack);
	// Upper left rectangle.
	_vm->_graphics->drawRectangle(Common::Rect(10, 5, 381, 71), kColorRed);
	_vm->_graphics->drawFilledRectangle(Common::Rect(11, 6, 380, 70), kColorBrown);
	// Bottom right rectangle.
	_vm->_graphics->drawRectangle(Common::Rect(394, 50, 635, 198), kColorRed);
	_vm->_graphics->drawFilledRectangle(Common::Rect(395, 51, 634, 197), kColorBrown);
		
	_vm->_graphics->drawNimLogo();
	_vm->_graphics->drawNimInitials();

	_vm->_graphics->drawNormalText("SCOREBOARD:", _vm->_font, 8, 475, 45, kColorWhite);
	_vm->_graphics->drawNormalText("Turn:", _vm->_font, 8, 420, 55, kColorYellow);
	_vm->_graphics->drawNormalText("Player:", _vm->_font, 8, 490, 55, kColorYellow);
	_vm->_graphics->drawNormalText("Move:", _vm->_font, 8, 570, 55, kColorYellow);
	
	chalk(27, 7, "Take pieces away with:");
	chalk(77, 17, "1) the mouse (click leftmost)");
	chalk(53, 27, "or 2) the keyboard:");
	chalk(220, 27, Common::String(24) + '/' + 25 + ": choose row,");
	chalk(164, 37, Common::String("+/- or ") + 27 + '/' + 26 + ": more/fewer,");
	chalk(204, 47, "Enter: take stones.");

	_vm->_graphics->refreshScreen();

	for (int i = 0; i < 3; i++)
		_stones[i] = i + 3;
	_stonesLeft = 12;

	_turns = 0;
	_dogfoodsTurn = true;

	_row = 1;
	_number = 1;
	for (int i = 0; i < 3; i++)
		_old[i] = 0;
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
