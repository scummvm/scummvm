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

#include "sherlock/tattoo/tattoo_darts.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

Darts::Darts(SherlockEngine *vm) : _vm(vm) {
	_gameType = GAME_301;
	_hand1 = _hand2 = nullptr;
	_dartGraphics = nullptr;
	_dartsLeft = nullptr;
	_dartMap = nullptr;
	_dartBoard = nullptr;
	Common::fill(&_cricketScore[0][0], &_cricketScore[0][7], 0);
	Common::fill(&_cricketScore[1][0], &_cricketScore[1][7], 0);
	_score1 = _score2 = 0;
	_roundNum = 0;
	_level = 0;
}

void Darts::playDarts(GameType gameType) {

}

void Darts::initDarts() {
	_dartInfo = Common::Rect(430, 50, 430 + 205, 50 + 330);

	if (_gameType == GAME_CRICKET) {
		_dartInfo = Common::Rect(430, 245, 430 + 205, 245 + 150);
	}

	Common::fill(&_cricketScore[0][0], &_cricketScore[0][7], 0);
	Common::fill(&_cricketScore[1][0], &_cricketScore[1][7], 0);

	switch (_gameType) {
	case GAME_501:
		_score1 = _score2 = 501;
		_gameType = GAME_301;
		break;

	case GAME_301:
		_score1 = _score2 = 301;
		break;

	default:
		// Cricket
		_score1 = _score2 = 0;
		break;
	}

	_roundNum = 1;

	if (_level == 9) {
		// No computer players
		_compPlay = 0;
		_level = 0;
	} else if (_level == 8) {
		_level = _vm->getRandomNumber(3);
		_compPlay = 2;
	} else {
		// Check for opponent flags
		for (int idx = 0; idx < 4; ++idx) {
			if (_vm->readFlags(314 + idx))
				_level = idx;
		}
	}

	_opponent = "Jock";
}

void Darts::loadDarts() {
	Resources &res = *_vm->_res;
	Screen &screen = *_vm->_screen;
	byte palette[PALETTE_SIZE];

	// Load images
	_hand1 = new ImageFile("hand1.vgs");
	_hand2 = new ImageFile("hand2.vgs");
	_dartGraphics = new ImageFile("darts.vgs");
	_dartsLeft = new ImageFile("DartsLft.vgs");
	_dartMap = new ImageFile("DartMap.vgs");
	_dartBoard = new ImageFile("DartBd.vgs");

	// Load and set the palette
	Common::SeekableReadStream *stream = res.load("DartBoard.pal");
	stream->read(palette, PALETTE_SIZE);
	screen.translatePalette(palette);
	screen.setPalette(palette);
	delete stream;

	// Load the initial background
	screen._backBuffer1.blitFrom((*_dartBoard)[0], Common::Point(0, 0));
	screen._backBuffer2.blitFrom(screen._backBuffer1);
	screen.blitFrom(screen._backBuffer1);
}

void Darts::closeDarts() {
	delete _dartBoard;
	delete _dartsLeft;
	delete _dartGraphics;
	delete _dartMap;
	delete _hand1;
	delete _hand2;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
