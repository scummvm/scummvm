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
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */

#include "avalanche/avalanche.h"
#include "avalanche/highscore.h"

#include "common/savefile.h"
#include "common/system.h"

namespace Avalanche {

HighScore::HighScore(AvalancheEngine *vm) {
	_vm = vm;
}

void HighScore::displayHighScores() {
	warning("STUB: HighScore::displayHighScores(");
}

void HighScore::saveHighScores() {
	int firstSmaller = 0;
	while ((firstSmaller < 12) && (_data[firstSmaller]._score >= _vm->_score))
		firstSmaller++;

	if (firstSmaller < 12) {
		// Shift all the lower scores down a space:
		for (int i = firstSmaller; i < 11; i++)
			_data[i + 1] = _data[i];
		// Set the new high score:
		_data[firstSmaller]._name = "Player"; // TODO: Come up with something for that. In the original it wasn't implemented at all...
		_data[firstSmaller]._rank = _vm->_parser->rank();
		_data[firstSmaller]._score = _vm->_score;
	}

	Common::OutSaveFile *f = g_system->getSavefileManager()->openForSaving("scores.avd");
	if (!f) {
		warning("Can't create file 'scores.avd', high scores are not saved.");
		return;
	}
	Common::Serializer sz(NULL, f);
	syncHighScores(sz);
	f->finalize();
	delete f;
}

void HighScore::loadHighScroes() {
	Common::File file;
	if (!file.exists("scores.avd")) {
		produceDefaultHighScores();
	} else {
		Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading("scores.avd");
		if (!f)
			return;
		Common::Serializer sz(f, NULL);
		syncHighScores(sz);
		delete f;
	}
}

void HighScore::produceDefaultHighScores() {
	for (int i = 0; i < 12; i++) {
		_data[i]._score = 32 - (i + 1) * 2;
		_data[i]._rank = "...";
	}
	_data[0]._name = "Mike";
	_data[1]._name = "Liz";
	_data[2]._name = "Thomas";
	_data[3]._name = "Mark";
	_data[4]._name = "Mandy";
	_data[5]._name = "Andrew";
	_data[6]._name = "Lucy Tryphena";
	_data[7]._name = "Tammy the dog";
	_data[8]._name = "Avaricius";
	_data[9]._name = "Spellchick";
	_data[10]._name = "Caddelli";
	_data[11]._name = "Spludwick";
}

void HighScore::syncHighScores(Common::Serializer &sz) {
	for (int i = 0; i < 12; i++) {
		sz.syncString(_data[i]._name);
		sz.syncAsUint16LE(_data[i]._score);
		sz.syncString(_data[i]._rank);
	}
}

} // End of namespace Avalanche
