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

/* CLOSING		The closing screen and error handler. */

#include "avalanche/avalanche.h"

#include "avalanche/closing2.h"
#include "avalanche/gyro2.h"
#include "avalanche/lucerna2.h"

#include "common/textconsole.h"
#include "common/random.h"



namespace Avalanche {

Closing::Closing(AvalancheEngine *vm) {
	_vm = vm;
	warning("STUB: Closing::Closing()");
}

void Closing::getScreen(ScreenType which) {
	warning("STUB: Closing::getScreen()");
}

void Closing::showScreen() {
	warning("STUB: Closing::showScreen()");
}

void Closing::putIn(Common::String str, uint16 where) {
	warning("STUB: Closing::putIn()");
}

void Closing::exitGame() {
	const Common::String nouns[12] = {
		"sackbut", "harpsichord", "camel", "conscience", "ice-cream", "serf",
		"abacus", "castle", "carrots", "megaphone", "manticore", "drawbridge"
	};

	const Common::String verbs[12] = {
		"haunt", "daunt", "tickle", "gobble", "erase", "provoke", "surprise",
		"ignore", "stare at", "shriek at", "frighten", "quieten"
	};

	Common::String result;

	//nosound();
	warning("STUB: Closing::exitGame()");

	getScreen(kScreenNagScreen);
	result = nouns[_vm->_rnd->getRandomNumber(12)] + " will " + verbs[_vm->_rnd->getRandomNumber(12)] + " you";
	putIn(result, 1628);
	showScreen(); // No halt- it's already set up.
}

void Closing::handleBug() {
	warning("STUB: Closing::handleBug()");
}

} // End of namespace Avalanche.
