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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* CLOSING		The closing screen and error handler. */

#include "avalanche/avalanche.h"
#include "avalanche/closing.h"

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
	static const char nouns[12][14] = {
		"sackbut", "harpsichord", "camel",   "conscience", "ice-cream", "serf",
		"abacus",  "castle",      "carrots", "megaphone",  "manticore", "drawbridge"
	};

	static const char verbs[12][12] = {
		"haunt",    "daunt",  "tickle",   "gobble",    "erase",    "provoke",
		"surprise", "ignore", "stare at", "shriek at", "frighten", "quieten"
	};

	_vm->_sound->stopSound();

	getScreen(kScreenNagScreen);
	byte nounId = _vm->_rnd->getRandomNumber(11);
	byte verbId = _vm->_rnd->getRandomNumber(11);
	Common::String result = Common::String::format("%s will %s you", nouns[nounId], verbs[verbId]);
	putIn(result, 1628);
	showScreen(); // No halt- it's already set up.
}

} // End of namespace Avalanche.
