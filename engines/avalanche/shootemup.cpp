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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include "avalanche/shootemup.h"

#include "common/random.h"

namespace Avalanche {

const byte ShootEmUp::kFacingRight = 87;
const byte ShootEmUp::kStocks = 27;

ShootEmUp::ShootEmUp(AvalancheEngine *vm) {
	_vm = vm;

	_time = 0;
}

void ShootEmUp::run() {
	CursorMan.showMouse(false);
	_vm->_graphics->saveScreen();
	_vm->fadeOut();
	_vm->_graphics->seuDrawTitle();
	_vm->fadeIn();

	_vm->_graphics->seuLoad();

	// Should we show the instructions?
	while (!_vm->shouldQuit()) {
		Common::Event event;
		_vm->getEvent(event);
		if (event.type == Common::EVENT_KEYDOWN) {
			if ((event.kbd.keycode == Common::KEYCODE_i) || (event.kbd.keycode == Common::KEYCODE_F1))
				instructions();
			break; // We don't show the instructions and simply go on with the minigame if not I or F1 was pressed.
		}
	}

	setup();
	initRunner(20, 70, 48, 54, _vm->_rnd->getRandomNumber(4) + 1, _vm->_rnd->getRandomNumber(3) - 2);
	initRunner(600, 70, 48, 54, _vm->_rnd->getRandomNumber(4) + 1, _vm->_rnd->getRandomNumber(3) - 2);
	initRunner(600, 100, 61, 67, -(_vm->_rnd->getRandomNumber(4)) + 1, _vm->_rnd->getRandomNumber(3) - 2);
	initRunner(20, 100, 61, 67, -(_vm->_rnd->getRandomNumber(4)) + 1, _vm->_rnd->getRandomNumber(3) - 2);
	do {
		blankIt();
		hitPeople();
		plotThem();
		moveThem();
		moveAvvy();
		bumpFolk();
		peopleRunning();
		animate();
		escapeCheck();
		collisionCheck();
		updateTime();
		check321();
		readKbd();
	} while (_time != 0);

	_vm->fadeOut();
	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();
	_vm->fadeIn();
	CursorMan.showMouse(true);
}

bool ShootEmUp::overlap(uint16 a1x, uint16 a1y, uint16 a2x, uint16 a2y, uint16 b1x, uint16 b1y, uint16 b2x, uint16 b2y) {
	warning("STUB: ShootEmUp::overlap()");
	return false;
}

byte ShootEmUp::getStockNumber(byte x) {
	warning("STUB: ShootEmUp::getStockNumber()");
	return 0;
}

void ShootEmUp::blankIt() {
	warning("STUB: ShootEmUp::blankIt()");
}

void ShootEmUp::moveThem() {
	warning("STUB: ShootEmUp::moveThem()");
}

void ShootEmUp::plotThem() {
	warning("STUB: ShootEmUp::plotThem()");
}

void ShootEmUp::define(int16 xx, int16 yy, byte pp, int8 ixx, int8 iyy, int16 time, bool isAMissile, bool doWeWipe) {
	warning("STUB: ShootEmUp::define()");
}

void ShootEmUp::defineCameo(int16 xx, int16 yy, byte pp, int16 time) {
	warning("STUB: ShootEmUp::defineCameo()");
}

void ShootEmUp::showStock(byte x) {
	warning("STUB: ShootEmUp::showStock()");
}

void ShootEmUp::showScore() {
	warning("STUB: ShootEmUp::showScore()");
}

void ShootEmUp::showTime() {
	warning("STUB: ShootEmUp::showTime()");
}

void ShootEmUp::gain(int8 howMuch) {
	warning("STUB: ShootEmUp::gain()");
}

void ShootEmUp::newEscape() {
	warning("STUB: ShootEmUp::newEscape()");
}

void ShootEmUp::nextPage() {
	_vm->_graphics->drawNormalText("Press a key for next page >", _vm->_font, 8, 400, 190, kColorWhite);
	_vm->_graphics->refreshScreen();

	while (!_vm->shouldQuit()) {
		Common::Event event;
		_vm->getEvent(event);
		if (event.type == Common::EVENT_KEYDOWN) {
			break;
		}
	}

	_vm->_graphics->drawFilledRectangle(Common::Rect(0, 0, 640, 200), kColorBlack);
}

void ShootEmUp::instructions() {
	_vm->_graphics->drawFilledRectangle(Common::Rect(0, 0, 640, 200), kColorBlack); // Black out the whole screen.
	_vm->_graphics->seuDrawPicture(25, 25, kFacingRight);
	_vm->_graphics->drawNormalText("< Avvy, our hero, needs your help - you must move him around.", _vm->_font, 8, 60, 35, kColorWhite);
	_vm->_graphics->drawNormalText("(He''s too terrified to move himself!)", _vm->_font, 8, 80, 45, kColorWhite);
	_vm->_graphics->drawNormalText("Your task is to prevent the people in the stocks from escaping", _vm->_font, 8, 0, 75, kColorWhite);
	_vm->_graphics->drawNormalText("by pelting them with rotten fruit, eggs and bread. The keys are:", _vm->_font, 8, 0, 85, kColorWhite);
	_vm->_graphics->drawNormalText("LEFT SHIFT", _vm->_font, 8, 80, 115, kColorWhite);
	_vm->_graphics->drawNormalText("Move left.", _vm->_font, 8, 200, 115, kColorWhite);
	_vm->_graphics->drawNormalText("RIGHT SHIFT", _vm->_font, 8, 72, 135, kColorWhite);
	_vm->_graphics->drawNormalText("Move right.", _vm->_font, 8, 200, 135, kColorWhite);
	_vm->_graphics->drawNormalText("ALT", _vm->_font, 8, 136, 155, kColorWhite);
	_vm->_graphics->drawNormalText("Throw something.", _vm->_font, 8, 200, 155, kColorWhite);

	nextPage();

	_vm->_graphics->seuDrawPicture(25, 35, kStocks);
	_vm->_graphics->drawNormalText("This man is in the stocks. Your job is to stop him getting out.", _vm->_font, 8, 80, 35, kColorWhite);
	_vm->_graphics->drawNormalText("UNFORTUNATELY... the locks on the stocks are loose, and every", _vm->_font, 8, 88, 45, kColorWhite);
	_vm->_graphics->drawNormalText("so often, someone will discover this and try to get out.", _vm->_font, 8, 88, 55, kColorWhite);
	_vm->_graphics->seuDrawPicture(25, 85, kStocks + 2);
	_vm->_graphics->drawNormalText("< Someone who has found a way out!", _vm->_font, 8, 80, 85, kColorWhite);
	_vm->_graphics->drawNormalText("You MUST IMMEDIATELY hit people smiling like this, or they", _vm->_font, 8, 88, 95, kColorWhite);
	_vm->_graphics->drawNormalText("will disappear and lose you points.", _vm->_font, 8, 88, 105, kColorWhite);
	_vm->_graphics->seuDrawPicture(25, 125, kStocks + 5);
	_vm->_graphics->seuDrawPicture(25, 155, kStocks + 4);
	_vm->_graphics->drawNormalText("< Oh dear!", _vm->_font, 8, 80, 125, kColorWhite);

	nextPage();

	_vm->_graphics->drawNormalText("Your task is made harder by:", _vm->_font, 8, 0, 35, kColorWhite);
	_vm->_graphics->seuDrawPicture(25, 55, 48);
	_vm->_graphics->drawNormalText("< Yokels. These people will run in front of you. If you hit", _vm->_font, 8, 60, 55, kColorWhite);
	_vm->_graphics->drawNormalText("them, you will lose MORE points than you get hitting people", _vm->_font, 8, 68, 65, kColorWhite);
	_vm->_graphics->drawNormalText("in the stocks. So BEWARE!", _vm->_font, 8, 68, 75, kColorWhite);
	_vm->_graphics->drawNormalText("Good luck with the game!", _vm->_font, 8, 80, 125, kColorWhite);

	nextPage();
}

void ShootEmUp::setup() {
	warning("STUB: ShootEmUp::setup()");
}

void ShootEmUp::initRunner(int16 xx, int16 yy, byte f1, byte f2, int8 ixx, int8 iyy) {
	warning("STUB: ShootEmUp::initRunner()");
}

void ShootEmUp::moveAvvy() {
	warning("STUB: ShootEmUp::moveAvvy()");
}

void ShootEmUp::readKbd() {
	warning("STUB: ShootEmUp::readKbd()");
}

void ShootEmUp::animate() {
	warning("STUB: ShootEmUp::animate()");
}

void ShootEmUp::collisionCheck() {
	warning("STUB: ShootEmUp::collisionCheck()");
}

void ShootEmUp::turnAround(byte who, bool randomX) {
	warning("STUB: ShootEmUp::turnAround()");
}

void ShootEmUp::bumpFolk() {
	warning("STUB: ShootEmUp::bumpFolk()");
}

void ShootEmUp::peopleRunning() {
	warning("STUB: ShootEmUp::peopleRunning()");
}

void ShootEmUp::updateTime() {
	warning("STUB: ShootEmUp::updateTime()");
}

void ShootEmUp::hitPeople() {
	warning("STUB: ShootEmUp::hitPeople()");
}

void ShootEmUp::escapeCheck() {
	warning("STUB: ShootEmUp::escapeCheck()");
}

void ShootEmUp::check321() {
	warning("STUB: ShootEmUp::check321()");
}

} // End of namespace Avalanche
