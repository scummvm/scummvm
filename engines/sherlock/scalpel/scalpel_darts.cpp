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

#include "sherlock/scalpel/scalpel_darts.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {

namespace Scalpel {

enum {
	STATUS_INFO_X = 218,
	STATUS_INFO_Y = 53,
	DART_INFO_X = 218,
	DART_INFO_Y = 103,
	DARTBARHX = 35,
	DARTHORIZY = 190,
	DARTBARVX = 1,
	DARTHEIGHTY = 25,
	DARTBARSIZE = 150,
	DART_BAR_FORE = 8
};

enum {
	DART_COL_FORE = 5,
	PLAYER_COLOR = 11
};
#define OPPONENTS_COUNT 4

const char *const OPPONENT_NAMES[OPPONENTS_COUNT] = {
	"Skipper", "Willy", "Micky", "Tom"
};

/*----------------------------------------------------------------*/

Darts::Darts(ScalpelEngine *vm) : _vm(vm) {
	_dartImages = nullptr;
	_level = 0;
	_computerPlayer = 1;
	_playerDartMode = false;
	_dartScore1 = _dartScore2 = 0;
	_roundNumber = 0;
	_playerDartMode = false;
	_roundScore = 0;
	_oldDartButtons = false;
}

void Darts::playDarts() {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	int playerNumber = 0;
	int lastDart;

	// Change the font
	int oldFont = screen.fontNumber();
	screen.setFont(2);

	loadDarts();
	initDarts();

	bool done = false;
	do {
		int score, roundStartScore;
		roundStartScore = score = playerNumber == 0 ? _dartScore1 : _dartScore2;

		// Show player details
		showNames(playerNumber);
		showStatus(playerNumber);
		_roundScore = 0;

		if (_vm->shouldQuit())
			return;

		for (int idx = 0; idx < 3; ++idx) {
			// Throw a single dart
			if (_computerPlayer == 1)
				lastDart = throwDart(idx + 1, playerNumber * 2);
			else if (_computerPlayer == 2)
				lastDart = throwDart(idx + 1, playerNumber + 1);
			else
				lastDart = throwDart(idx + 1, 0);

			score -= lastDart;
			_roundScore += lastDart;

			screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(DART_INFO_X, DART_INFO_Y - 1),
				Common::Rect(DART_INFO_X, DART_INFO_Y - 1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
			screen.print(Common::Point(DART_INFO_X, DART_INFO_Y), DART_COL_FORE, "Dart # %d", idx + 1);
			screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 10), DART_COL_FORE, "Scored %d points", lastDart);

			if (score != 0 && playerNumber == 0)
				screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 30), DART_COL_FORE, "Press a key");

			if (score == 0) {
				// Some-one has won
				screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 20), PLAYER_COLOR, "GAME OVER!");

				if (playerNumber == 0) {
					screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 30), PLAYER_COLOR, "Holmes Wins!");
					if (_level < OPPONENTS_COUNT)
						_vm->setFlagsDirect(318 + _level);
				} else {
					screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 30), PLAYER_COLOR, "%s Wins!", _opponent.c_str());
				}

				screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 4), DART_COL_FORE, "Press a key");

				idx = 10;
				done = true;
			} else if (score < 0) {
				screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 20), PLAYER_COLOR, "BUSTED!");

				idx = 10;
				score = roundStartScore;
			}

			if (playerNumber == 0)
				_dartScore1 = score;
			else
				_dartScore2 = score;

			showStatus(playerNumber);
			events.clearKeyboard();

			if ((playerNumber == 0 && _computerPlayer == 1) || _computerPlayer == 0 || done) {
				int dartKey;
				while (!(dartKey = dartHit()) && !_vm->shouldQuit())
					events.delay(10);

				if (dartKey == Common::KEYCODE_ESCAPE) {
					idx = 10;
					done = true;
				}
			} else {
				events.wait(20);
			}

			screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(DART_INFO_X, DART_INFO_Y - 1),
				Common::Rect(DART_INFO_X, DART_INFO_Y - 1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
			screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
		}

		playerNumber ^= 1;
		if (!playerNumber)
			++_roundNumber;

		done |= _vm->shouldQuit();

		if (!done) {
			screen._backBuffer2.SHblitFrom((*_dartImages)[0], Common::Point(0, 0));
			screen._backBuffer1.SHblitFrom(screen._backBuffer2);
			screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
		}
	} while (!done);

	closeDarts();
	screen.fadeToBlack();

	// Restore font
	screen.setFont(oldFont);
}

void Darts::loadDarts() {
	Screen &screen = *_vm->_screen;

	_dartImages = new ImageFile("darts.vgs");
	screen.setPalette(_dartImages->_palette);

	screen._backBuffer1.SHblitFrom((*_dartImages)[0], Common::Point(0, 0));
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
}

void Darts::initDarts() {
	_dartScore1 = _dartScore2 = 301;
	_roundNumber = 1;

	if (_level == 9) {
		// No computer players
		_computerPlayer = 0;
		_level = 0;
	} else if (_level == 8) {
		_level = _vm->getRandomNumber(3);
		_computerPlayer = 2;
	} else {
		// Check flags for opponents
		for (int idx = 0; idx < OPPONENTS_COUNT; ++idx) {
			if (_vm->readFlags(314 + idx))
				_level = idx;
		}
	}

	_opponent = OPPONENT_NAMES[_level];
}

void Darts::closeDarts() {
	delete _dartImages;
	_dartImages = nullptr;
}

void Darts::showNames(int playerNum) {
	Screen &screen = *_vm->_screen;
	byte color = playerNum == 0 ? PLAYER_COLOR : DART_COL_FORE;

	// Print Holmes first
	if (playerNum == 0)
		screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y), PLAYER_COLOR + 3, "Holmes");
	else
		screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y), color, "Holmes");

	screen._backBuffer1.fillRect(Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10,
		STATUS_INFO_X + 31, STATUS_INFO_Y + 12), color);
	screen.slamArea(STATUS_INFO_X, STATUS_INFO_Y + 10, 31, 12);

	// Second player
	color = playerNum == 1 ? PLAYER_COLOR : DART_COL_FORE;

	if (playerNum != 0)
		screen.print(Common::Point(STATUS_INFO_X + 50, STATUS_INFO_Y), PLAYER_COLOR + 3,
			"%s", _opponent.c_str());
	else
		screen.print(Common::Point(STATUS_INFO_X + 50, STATUS_INFO_Y), color,
			"%s", _opponent.c_str());

	screen._backBuffer1.fillRect(Common::Rect(STATUS_INFO_X + 50, STATUS_INFO_Y + 10,
		STATUS_INFO_X + 81, STATUS_INFO_Y + 12), color);
	screen.slamArea(STATUS_INFO_X + 50, STATUS_INFO_Y + 10, 81, 12);

	// Make a copy of the back buffer to the secondary one
	screen._backBuffer2.SHblitFrom(screen._backBuffer1);
}

void Darts::showStatus(int playerNum) {
	Screen &screen = *_vm->_screen;
	byte color;

	// Copy scoring screen from secondary back buffer. This will erase any previously displayed status/score info
	screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 10),
		Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10, SHERLOCK_SCREEN_WIDTH, STATUS_INFO_Y + 48));

	color = (playerNum == 0) ? PLAYER_COLOR : DART_COL_FORE;
	screen.print(Common::Point(STATUS_INFO_X + 6, STATUS_INFO_Y + 13), color, "%d", _dartScore1);

	color = (playerNum == 1) ? PLAYER_COLOR : DART_COL_FORE;
	screen.print(Common::Point(STATUS_INFO_X + 56, STATUS_INFO_Y + 13), color, "%d", _dartScore2);
	screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 25), PLAYER_COLOR, "Round: %d", _roundNumber);
	screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 35), PLAYER_COLOR, "Turn Total: %d", _roundScore);
	screen.slamRect(Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10, SHERLOCK_SCREEN_WIDTH, STATUS_INFO_Y + 48));
}

int Darts::throwDart(int dartNum, int computer) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point targetNum;
	int width, height;

	events.clearKeyboard();

	erasePowerBars();
	screen.print(Common::Point(DART_INFO_X, DART_INFO_Y), DART_COL_FORE, "Dart # %d", dartNum);

	if (!computer) {
		screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 10), DART_COL_FORE, "Hit a key");
		screen.print(Common::Point(DART_INFO_X, DART_INFO_Y + 18), DART_COL_FORE, "to start");
	}

	if (!computer) {
		while (!_vm->shouldQuit() && !dartHit())
			;
	} else {
		events.delay(10);
	}

	if (_vm->shouldQuit())
		return 0;

	screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(DART_INFO_X, DART_INFO_Y - 1),
		Common::Rect(DART_INFO_X, DART_INFO_Y - 1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
	screen.slamRect(Common::Rect(DART_INFO_X, DART_INFO_Y - 1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	// If it's a computer player, choose a dart destination
	if (computer)
		targetNum = getComputerDartDest(computer - 1);

	width = doPowerBar(Common::Point(DARTBARHX, DARTHORIZY), DART_BAR_FORE, targetNum.x, false);
	height = 101 - doPowerBar(Common::Point(DARTBARVX, DARTHEIGHTY), DART_BAR_FORE, targetNum.y, true);

	// For human players, slight y adjustment
	if (computer == 0)
		height += 2;

	// Copy the bars to the secondary back buffer so that they remain fixed at their selected values
	// whilst the dart is being animated at being thrown at the board
	screen._backBuffer2.SHblitFrom(screen._backBuffer1, Common::Point(DARTBARHX - 1, DARTHORIZY - 1),
		Common::Rect(DARTBARHX - 1, DARTHORIZY - 1, DARTBARHX + DARTBARSIZE + 3, DARTHORIZY + 10));
	screen._backBuffer2.SHblitFrom(screen._backBuffer1, Common::Point(DARTBARVX - 1, DARTHEIGHTY - 1),
		Common::Rect(DARTBARVX - 1, DARTHEIGHTY - 1, DARTBARVX + 11, DARTHEIGHTY + DARTBARSIZE + 3));

	// Convert height and width to relative range of -50 to 50, where 0,0 is the exact centre of the board
	height -= 50;
	width -= 50;

	Common::Point dartPos(111 + width * 2, 99 + height * 2);
	drawDartThrow(dartPos);

	return dartScore(dartPos);
}

void Darts::drawDartThrow(const Common::Point &pt) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point pos(pt.x, pt.y + 2);
	Common::Rect oldDrawBounds;
	int delta = 9;

	for (int idx = 4; idx < 23; ++idx) {
		ImageFrame &frame = (*_dartImages)[idx];

		// Adjust draw position for animating dart
		if (idx < 13)
			pos.y -= delta--;
		else if (idx == 13)
			delta = 1;
		else
			pos.y += delta++;

		// Draw the dart
		Common::Point drawPos(pos.x - frame._width / 2, pos.y - frame._height);
		screen._backBuffer1.SHtransBlitFrom(frame, drawPos);
		screen.slamArea(drawPos.x, drawPos.y, frame._width, frame._height);

		// Handle erasing old dart frame area
		if (!oldDrawBounds.isEmpty())
			screen.slamRect(oldDrawBounds);

		oldDrawBounds = Common::Rect(drawPos.x, drawPos.y, drawPos.x + frame._width, drawPos.y + frame._height);
		screen._backBuffer1.SHblitFrom(screen._backBuffer2, drawPos, oldDrawBounds);

		events.wait(2);
	}

	// Draw dart in final "stuck to board" form
	screen._backBuffer1.SHtransBlitFrom((*_dartImages)[22], Common::Point(oldDrawBounds.left, oldDrawBounds.top));
	screen._backBuffer2.SHtransBlitFrom((*_dartImages)[22], Common::Point(oldDrawBounds.left, oldDrawBounds.top));
	screen.slamRect(oldDrawBounds);
}

void Darts::erasePowerBars() {
	Screen &screen = *_vm->_screen;

	screen._backBuffer1.fillRect(Common::Rect(DARTBARHX, DARTHORIZY, DARTBARHX + DARTBARSIZE, DARTHORIZY + 10), BLACK);
	screen._backBuffer1.fillRect(Common::Rect(DARTBARVX, DARTHEIGHTY, DARTBARVX + 10, DARTHEIGHTY + DARTBARSIZE), BLACK);
	screen._backBuffer1.SHtransBlitFrom((*_dartImages)[2], Common::Point(DARTBARHX - 1, DARTHORIZY - 1));
	screen._backBuffer1.SHtransBlitFrom((*_dartImages)[3], Common::Point(DARTBARVX - 1, DARTHEIGHTY - 1));
	screen.slamArea(DARTBARHX - 1, DARTHORIZY - 1, DARTBARSIZE + 3, 11);
	screen.slamArea(DARTBARVX - 1, DARTHEIGHTY - 1, 11, DARTBARSIZE + 3);
}

int Darts::doPowerBar(const Common::Point &pt, byte color, int goToPower, bool isVertical) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	bool done;
	int idx = 0;

	events.clearEvents();
	events.delay(100);

	// Display loop
	do {
		done = _vm->shouldQuit() || idx >= DARTBARSIZE;

		if (idx == (goToPower - 1))
			// Reached target power for a computer player
			done = true;
		else if (goToPower == 0) {
			// Check for press
			if (dartHit())
				done = true;
		}

		if (isVertical) {
			screen._backBuffer1.hLine(pt.x, pt.y + DARTBARSIZE - 1 - idx, pt.x + 8, color);
			screen._backBuffer1.SHtransBlitFrom((*_dartImages)[3], Common::Point(pt.x - 1, pt.y - 1));
			screen.slamArea(pt.x, pt.y + DARTBARSIZE - 1 - idx, 8, 2);
		} else {
			screen._backBuffer1.vLine(pt.x + idx, pt.y, pt.y + 8, color);
			screen._backBuffer1.SHtransBlitFrom((*_dartImages)[2], Common::Point(pt.x - 1, pt.y - 1));
			screen.slamArea(pt.x + idx, pt.y, 1, 8);
		}

		if (!(idx % 8))
			events.wait(1);

		++idx;
	} while (!done);

	return MIN(idx * 100 / DARTBARSIZE, 100);
}

int Darts::dartHit() {
	Events &events = *_vm->_events;

	// Process pending events
	events.pollEventsAndWait();

	if (events.kbHit()) {
		// Key was pressed, so return it
		Common::KeyState keyState = events.getKey();
		return keyState.keycode;
	}

	_oldDartButtons = events._pressed;
	events.setButtonState();

	// Only return true if the mouse button is newly pressed
	return (events._pressed && !_oldDartButtons) ? 1 : 0;
}

int Darts::dartScore(const Common::Point &pt) {
	Common::Point pos(pt.x - 37, pt.y - 33);
	Graphics::Surface &scoreImg = (*_dartImages)[1]._frame;

	if (pos.x < 0 || pos.y < 0 || pos.x >= scoreImg.w || pos.y >= scoreImg.h)
		// Not on the board
		return 0;

	// On board, so get the score from the pixel at that position
	int score = *(const byte *)scoreImg.getBasePtr(pos.x, pos.y);
	return score;
}

Common::Point Darts::getComputerDartDest(int playerNum) {
	Common::Point target;
	int score = playerNum == 0 ? _dartScore1 : _dartScore2;

	if (score > 50) {
		// Aim for the bullseye
		target.x = target.y = 76;

		if (_level <= 1 &&  _vm->getRandomNumber(1) == 1) {
			// Introduce margin of error
			target.x += _vm->getRandomNumber(21) - 10;
			target.y += _vm->getRandomNumber(21) - 10;
		}
	} else {
		int aim = score;

		bool done;
		Common::Point pt;
		do {
			done = findNumberOnBoard(aim, pt);
			--aim;
		} while (!done);

		target.x = 75 + ((pt.x - 75) * 20 / 27);
		target.y = 75 + ((pt.y - 75) * 2 / 3);
	}

	// Pick a level of accuracy. The higher the level, the more accurate their throw will be
	int accuracy = _vm->getRandomNumber(10) + _level * 2;

	if (accuracy <= 2) {
		target.x += _vm->getRandomNumber(71) - 35;
		target.y += _vm->getRandomNumber(71) - 35;
	} else if (accuracy <= 4) {
		target.x += _vm->getRandomNumber(51) - 25;
		target.y += _vm->getRandomNumber(51) - 25;
	} else if (accuracy <= 6) {
		target.x += _vm->getRandomNumber(31) - 15;
		target.y += _vm->getRandomNumber(31) - 15;
	} else if (accuracy <= 8) {
		target.x += _vm->getRandomNumber(21) - 10;
		target.y += _vm->getRandomNumber(21) - 10;
	} else if (accuracy <= 10) {
		target.x += _vm->getRandomNumber(11) - 5;
		target.y += _vm->getRandomNumber(11) - 5;
	}

	if (target.x < 1)
		target.x = 1;
	if (target.y < 1)
		target.y = 1;

	return target;
}

bool Darts::findNumberOnBoard(int aim, Common::Point &pt) {
	ImageFrame &board = (*_dartImages)[1];

	// Scan board image for the special "center" pixels
	bool done = false;
	for (int yp = 0; yp < 132 && !done; ++yp) {
		const byte *srcP = (const byte *)board._frame.getBasePtr(0, yp);
		for (int xp = 0; xp < 147 && !done; ++xp, ++srcP) {
			int score = *srcP;

			// Check for match
			if (score == aim) {
				done = true;

				// Aim at non-double/triple numbers where possible
				if (aim < 21) {
					pt.x = xp + 5;
					pt.y = yp + 5;

					score = *(const byte *)board._frame.getBasePtr(xp + 10, yp + 10);
					if (score != aim)
						// Not aiming at non-double/triple number yet
						done = false;
				} else {
					// Aiming at a double or triple
					pt.x = xp + 3;
					pt.y = yp + 3;
				}
			}
		}
	}

	if (aim == 3)
		pt.x += 15;
	pt.y = 132 - pt.y;

	return done;
}

} // End of namespace Scalpel

} // End of namespace Sherlock
