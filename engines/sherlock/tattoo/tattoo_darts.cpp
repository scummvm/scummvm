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
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

enum {
	DART_COLOR_FORE	= 5,
	PLAYER_COLOR	= 11,
};

const int STATUS_INFO_X = 430;
const int STATUS_INFO_Y = 50;
const int STATUS_INFO_WIDTH = 205;
const int STATUS_INFO_HEIGHT = 330;
const int STATUS2_INFO_X = 510;
const int STATUS2_X_ADD = STATUS2_INFO_X - STATUS_INFO_X;
const int DART_BAR_VX = 10;
const int DART_HEIGHT_Y = 121;
const int DART_BAR_SIZE = 150;
const int DARTBOARD_LEFT = 73;
const int DARTBOARD_WIDTH = 257;
const int DARTBOARD_HEIGHT = 256;

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
	_roundScore = 0;
	_level = 0;
	_oldDartButtons = false;
	_handX = 0;
}

void Darts::playDarts(GameType gameType) {
	Screen &screen = *_vm->_screen;
	int oldFontType = screen.fontNumber();
	int playerNum = 0;
	int roundStart, score;

	screen.setFont(7);
	_spacing = screen.fontHeight() + 2;

	while (!_vm->shouldQuit()) {
		roundStart = score = (playerNum == 0) ? _score1 : _score2;

		showNames(playerNum);
		showStatus(playerNum);
		_roundScore = 0;
	}
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

	_opponent = FIXED(Jock);
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

void Darts::showNames(int playerNum) {
	Screen &screen = *_vm->_screen;
	byte color;

	color = playerNum == 0 ? PLAYER_COLOR : DART_COLOR_FORE;
	screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y), 0, "%s", FIXED(Holmes));
	screen._backBuffer1.fillRect(Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + _spacing + 1, 
		STATUS_INFO_X + 50, STATUS_INFO_Y + _spacing + 3), color);
	screen.fillRect(Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + _spacing + 1,
		STATUS_INFO_X + 50, STATUS_INFO_Y + _spacing + 3), color);

	color = playerNum == 1 ? PLAYER_COLOR : DART_COLOR_FORE;
	screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y), 0, "%s", _opponent.c_str());
	screen._backBuffer1.fillRect(Common::Rect(STATUS2_INFO_X, STATUS_INFO_Y + _spacing + 1, 
		STATUS2_INFO_X + 50, STATUS_INFO_Y + _spacing + 3), color);
	screen.fillRect(Common::Rect(STATUS2_INFO_X, STATUS_INFO_Y + _spacing + 1,
		STATUS2_INFO_X + 50, STATUS_INFO_Y + _spacing + 3), color);

	screen._backBuffer2.blitFrom(screen._backBuffer1);
}

void Darts::showStatus(int playerNum) {
	Screen &screen = *_vm->_screen;
	byte color;
	const char *const CRICKET_SCORE_NAME[7] = { "20", "19", "18", "17", "16", "15", FIXED(Bull) };

	screen._backBuffer2.blitFrom(screen._backBuffer1, Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 10),
		Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10, STATUS_INFO_X + STATUS_INFO_WIDTH,
		STATUS_INFO_Y + STATUS_INFO_HEIGHT - 10));

	color = (playerNum == 0) ? PLAYER_COLOR : DART_COLOR_FORE;
	screen.print(Common::Point(STATUS_INFO_X + 30, STATUS_INFO_Y + _spacing + 4), 0, "%d", _score1);

	color = (playerNum == 1) ? PLAYER_COLOR : DART_COLOR_FORE;
	screen.print(Common::Point(STATUS2_INFO_X + 30, STATUS_INFO_Y + _spacing + 4), 0, "%d", _score2);

	int temp = (_gameType == GAME_CRICKET) ? STATUS_INFO_Y + 10 * _spacing + 5 : STATUS_INFO_Y + 55;
	screen.print(Common::Point(STATUS_INFO_X, temp), 0, "%s: %d", FIXED(Round), _roundNum);

	if (_gameType == GAME_301) {
		screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 75), 0, "%s: %d", FIXED(TurnTotal), _roundScore);
	} else {
		// Show cricket scores
		for (int x = 0; x < 7; ++x) {
			screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 40 + x * _spacing), 0, "%s:", CRICKET_SCORE_NAME[x]);

			for (int y = 0; y < 2; ++y) {
				color = (playerNum == y) ? PLAYER_COLOR : DART_COLOR_FORE;
				
				switch (CRICKET_SCORE_NAME[y][x]) {
				case 1:
					screen.print(Common::Point(STATUS_INFO_X + 38 + y*STATUS2_X_ADD, STATUS_INFO_Y + 40 + x * _spacing), 0, "/");
					break;
				case 2:
					screen.print(Common::Point(STATUS_INFO_X + 38 + y*STATUS2_X_ADD, STATUS_INFO_Y + 40 + x * _spacing), 0, "X");
					break;
				case 3:
					screen.print(Common::Point(STATUS_INFO_X + 38 + y * STATUS2_X_ADD - 1, STATUS_INFO_Y + 40 + x * _spacing), 0, "X");
					screen.print(Common::Point(STATUS_INFO_X + 37 + y * STATUS2_X_ADD, STATUS_INFO_Y + 40 + x * _spacing), 0, "O");
					break;
				default:
					break;
				}
			}
		}
	}

	screen.blitFrom(screen._backBuffer1, Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 10),
		Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10, STATUS_INFO_X + STATUS_INFO_WIDTH, 
			STATUS_INFO_Y + STATUS_INFO_HEIGHT - 10));
}

void Darts::erasePowerBars() {
	Screen &screen = *_vm->_screen;

	// Erase the old power bars and replace them with empty ones
	screen.fillRect(Common::Rect(DART_BAR_VX, DART_HEIGHT_Y, DART_BAR_VX + 9, DART_HEIGHT_Y + DART_BAR_SIZE), 0);
	screen._backBuffer1.transBlitFrom((*_dartGraphics)[0], Common::Point(DART_BAR_VX - 1, DART_HEIGHT_Y - 1));
	screen.slamArea(DART_BAR_VX - 1, DART_HEIGHT_Y - 1, 10, DART_BAR_SIZE + 2);
}

bool Darts::dartHit() {
	Events &events = *_vm->_events;
	events.pollEventsAndWait();

	// Keyboard check
	if (events.kbHit()) {
		events.clearEvents();
		return true;
	}

	bool result = events._pressed && !_oldDartButtons;
	_oldDartButtons = events._pressed;
	return result;
}

int Darts::doPowerBar(const Common::Point &pt, byte color, int goToPower, int orientation) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	int x = 0;

	events.clearEvents();
	events.delay(100);

	while (!_vm->shouldQuit()) {
		if (x >= DART_BAR_SIZE)
			break;

		if ((goToPower - 1) == x)
			break;
		else if (goToPower == 0) {
			if (dartHit())
				break;
		}

		screen._backBuffer1.fillRect(Common::Rect(pt.x, pt.y + DART_BAR_SIZE - 1 - x,
			pt.x + 8, pt.y + DART_BAR_SIZE - 2 - x), color);
		screen._backBuffer1.transBlitFrom((*_dartGraphics)[0], Common::Point(pt.x - 1, pt.y - 1));
		screen.slamArea(pt.x, pt.y + DART_BAR_SIZE - 1 - x, 8, 2);

		if (!(x % 8))
			events.wait(1);

		x += 1;
	}

	return MIN(x * 100 / DART_BAR_SIZE, 100);
}

int Darts::drawHand(int goToPower, int computer) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	const int HAND_OFFSET[2] = { 72, 44 };
	ImageFile *hands;
	int hand;

	goToPower = (goToPower * DARTBOARD_WIDTH) / 150;

	if (!computer) {
		hand = 0;
		hands = _hand1;
	} else {
		hand = 1;
		hands = _hand2;
	}

	_handSize.x = (*hands)[0]._offset.x + (*hands)[0]._width;
	_handSize.y = (*hands)[0]._offset.y + (*hands)[0]._height;

	// Clear keyboard buffer
	events.clearEvents();
	events.delay(100);

	Common::Point pt(DARTBOARD_LEFT - HAND_OFFSET[hand], SHERLOCK_SCREEN_HEIGHT - _handSize.y);
	int x = 0;

	while (!_vm->shouldQuit()) {
		if (x >= DARTBOARD_WIDTH)
			break;

		if ((goToPower - 1) == x)
			break;
		else if (goToPower == 0) {
			if (dartHit())
				break;
		}

		screen._backBuffer1.transBlitFrom((*hands)[0], pt);
		screen.slamArea(pt.x - 1, pt.y, _handSize.x + 1, _handSize.y);
		screen.restoreBackground(Common::Rect(pt.x, pt.y, pt.x + _handSize.x, pt.y + _handSize.y));

		if (!(x % 8))
			events.wait(1);

		++x;
		++pt.x;
	}

	_handX = pt.x - 1;

	return MIN(x * 100 / DARTBOARD_WIDTH, 100);
}

Common::Point Darts::convertFromScreenToScoreCoords(const Common::Point &pt) const {
	return Common::Point(CLIP((int)pt.x, 0, DARTBOARD_WIDTH), CLIP((int)pt.y, 0, DARTBOARD_HEIGHT));
}

} // End of namespace Tattoo

} // End of namespace Sherlock
