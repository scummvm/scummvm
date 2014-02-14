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

ShootEmUp::ShootEmUp(AvalancheEngine *vm) {
	_vm = vm;

	_time = 0;
}

void ShootEmUp::run() {
	_vm->_graphics->seuDrawTitle();
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
	warning("STUB: ShootEmUp::nextPage()");
}

void ShootEmUp::instructions() {
	warning("STUB: ShootEmUp::instructions()");
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
