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

#include "sherlock/scalpel/darts.h"
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

const char *const OPPONENT_NAMES[5] = { 
	"Skipper", "Willy", "Micky", "Tom", "Bartender" 
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
	_oldDartButtons = 0;
}

/**
 * Main method for playing darts game
 */
void Darts::playDarts() {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	int score, roundStartScore;
	int playerNumber = 0;
	int lastDart;

	// Change the font
	int oldFont = screen.fontNumber();
	screen.setFont(4);

	loadDarts();
	initDarts();

	bool done = false;
	do {
		roundStartScore = score = playerNumber == 0 ? _dartScore1 : _dartScore2;
		
		// Show player details
		showNames(playerNumber);
		showStatus(playerNumber);
		_roundScore = 0;

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

			screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(DART_INFO_X, DART_INFO_Y - 1),
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
					if (_level < 4)
						setFlagsForDarts(318 + _level);
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

			screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(DART_INFO_X, DART_INFO_Y - 1),
				Common::Rect(DART_INFO_X, DART_INFO_Y - 1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
			screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
		}

		playerNumber ^= 1;
		if (!playerNumber)
			++_roundNumber;

		done |= _vm->shouldQuit();

		if (!done) {
			screen._backBuffer2.blitFrom((*_dartImages)[1], Common::Point(0, 0));
			screen._backBuffer1.blitFrom(screen._backBuffer2);
			screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
		}
	} while (!done);

	closeDarts();
	screen.fadeToBlack();

	// Restore font
	screen.setFont(oldFont);
}

/**
 * Load the graphics needed for the dart game
 */
void Darts::loadDarts() {
	Screen &screen = *_vm->_screen;

	_dartImages = new ImageFile("darts.vgs");
	screen._backBuffer1.blitFrom((*_dartImages)[1], Common::Point(0, 0));
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
}

/**
 * Initializes the variables needed for the dart game
 */
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
		for (int idx = 0; idx < 4; ++idx) {
			if (_vm->readFlags(314 + idx))
				_level = idx;
		}
	}

	_opponent = OPPONENT_NAMES[_level];
}

/**
 * Frees the images used by the dart game
 */
void Darts::closeDarts() {
	delete _dartImages;
	_dartImages = nullptr;
}

/**
 * Show the player names
 */
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
		screen.print(Common::Point(STATUS_INFO_X + 50, STATUS_INFO_Y + 10), PLAYER_COLOR + 3,
			_opponent.c_str());
	else
		screen.print(Common::Point(STATUS_INFO_X + 50, STATUS_INFO_Y + 10), color,
			_opponent.c_str());

	screen._backBuffer1.fillRect(Common::Rect(STATUS_INFO_X + 50, STATUS_INFO_Y + 10,
		STATUS_INFO_Y + 81, STATUS_INFO_Y + 12), color);
	screen.slamArea(STATUS_INFO_X + 50, STATUS_INFO_Y + 10, 81, 12);

	// Make a copy of the back buffer to the secondary one
	screen._backBuffer2.blitFrom(screen._backBuffer1);
}

/**
 * Show the player score and game status
 */
void Darts::showStatus(int playerNum) {
	Screen &screen = *_vm->_screen;
	byte color;

	screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 10),
		Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10, SHERLOCK_SCREEN_WIDTH, STATUS_INFO_Y + 38));

	color = (playerNum == 0) ? PLAYER_COLOR : DART_COL_FORE;
	screen.print(Common::Point(STATUS_INFO_X + 6, STATUS_INFO_Y + 13), color, "%d", _dartScore1);

	color = (playerNum == 1) ? PLAYER_COLOR : DART_COL_FORE;
	screen.print(Common::Point(STATUS_INFO_X + 56, STATUS_INFO_Y + 13), color, "%d", _dartScore2);
	screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 25), PLAYER_COLOR, "Round: %d", _roundNumber);
	screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 35), PLAYER_COLOR, "Turn Total: %d", _roundScore);
	screen.slamRect(Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10, SHERLOCK_SCREEN_WIDTH, STATUS_INFO_Y + 48));
}

/**
 * Throws a single dart.
 * @param dartNum	Dart number
 * @param computer	0 = Player, 1 = 1st player computer, 2 = 2nd player computer
 * @returns			Score for what dart hit
 */
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

	screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(DART_INFO_X, DART_INFO_Y - 1),
		Common::Rect(DART_INFO_X, DART_INFO_Y - 1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
	screen.slamRect(Common::Rect(DART_INFO_X, DART_INFO_Y - 1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	// If it's a computer player, choose a dart destination
	if (computer)
		targetNum = getComputerDartDest(computer - 1);
	
	width = doPowerBar(Common::Point(DARTBARHX, DARTHORIZY), DART_BAR_FORE, targetNum.x, false);
	height = 101 - doPowerBar(Common::Point(DARTBARVX, DARTHEIGHTY), DART_BAR_FORE, targetNum.y, true);
	
	// For human players, slight y adjustment
	if (computer == 0)
		height = 2;

	// Copy the bars to the secondary back buffer
	screen._backBuffer2.blitFrom(screen._backBuffer1, Common::Point(DARTBARHX - 1, DARTHORIZY - 1),
		Common::Rect(DARTBARHX - 1, DARTHORIZY - 1, DARTBARHX + DARTBARSIZE + 3, DARTHORIZY + 10));
	screen._backBuffer2.blitFrom(screen._backBuffer1, Common::Point(DARTBARVX - 1, DARTHEIGHTY - 1),
		Common::Rect(DARTBARVX - 1, DARTHEIGHTY - 1, DARTBARVX + 11, DARTHEIGHTY + DARTBARSIZE + 3));

	// Convert to relative range from -49 to 150
	height -= 50;
	width -= 50;

	Common::Point dartPos(111 + width * 2, 99 + height * 2);
	drawDartThrow(dartPos);

	return dartScore(dartPos);
}

/**
 * Draw a dart moving towards the board
 */
void Darts::drawDartThrow(const Common::Point &pt) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point pos(pt.x, pt.y + 2);
	Common::Rect oldDrawBounds;
	int delta = 9;

	for (int idx = 5; idx < 24; ++idx) {
		Graphics::Surface &frame = (*_dartImages)[idx]._frame;

		// Adjust draw position for animating dart
		if (idx < 14)
			pos.y -= delta--;
		else if (idx == 14)
			delta = 1;
		else
			pos.y += delta++;

		// Draw the dart
		Common::Point drawPos(pos.x - frame.w / 2, pos.y - frame.h);
		screen._backBuffer1.transBlitFrom(frame, drawPos);
		screen.slamArea(drawPos.x, drawPos.y, frame.w, frame.h);

		// Handle erasing old dart
		if (!oldDrawBounds.isEmpty())
			screen.slamRect(oldDrawBounds);

		oldDrawBounds = Common::Rect(drawPos.x, drawPos.y, drawPos.x + frame.w, drawPos.y + frame.h);
		if (idx != 23)
			screen._backBuffer1.blitFrom(screen._backBuffer2, drawPos, oldDrawBounds);

		events.wait(2);
	}

	// Draw dart in final "stuck to board" form
	screen._backBuffer1.transBlitFrom((*_dartImages)[23], Common::Point(oldDrawBounds.left, oldDrawBounds.top));
	screen._backBuffer2.transBlitFrom((*_dartImages)[23], Common::Point(oldDrawBounds.left, oldDrawBounds.top));
	screen.slamRect(oldDrawBounds);
}

/**
 * Erases the power bars
 */
void Darts::erasePowerBars() {
	Screen &screen = *_vm->_screen;

	screen._backBuffer1.fillRect(Common::Rect(DARTBARHX, DARTHORIZY, DARTBARHX + DARTBARSIZE, DARTHORIZY + 10), 0);
	screen._backBuffer1.fillRect(Common::Rect(DARTBARVX, DARTHEIGHTY, DARTBARVX + 10, DARTHEIGHTY + DARTBARSIZE), 0);
	screen._backBuffer1.transBlitFrom((*_dartImages)[3], Common::Point(DARTBARHX - 1, DARTHORIZY - 1));
	screen._backBuffer1.transBlitFrom((*_dartImages)[4], Common::Point(DARTBARVX - 1, DARTHEIGHTY - 1));
	screen.slamArea(DARTBARHX - 1, DARTHORIZY - 1, DARTBARSIZE + 3, 11);
	screen.slamArea(DARTBARVX - 1, DARTHEIGHTY - 1, 11, DARTBARSIZE + 3);
}

/**
 * Show a gradually incrementing incrementing power that bar. If goToPower is provided, it will
 * increment to that power level ignoring all keyboard input (ie. for computer throws).
 * Otherwise, it will increment until either a key/mouse button is pressed, or it reaches the end
 */
int Darts::doPowerBar(const Common::Point &pt, byte color, int goToPower, bool isVertical) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	bool done;
	int idx = 0;

	events.clearEvents();
	if (sound._musicOn)
		sound.waitTimerRoland(10);
	else
		events.delay(100);

	// Display loop
	do {
		done = _vm->shouldQuit() || idx >= DARTBARSIZE;

		if (idx == (goToPower - 1))
			// Reached target power for a computer player
			done = true;
		else if (goToPower == 0) {
			// Check for pres 
			if (dartHit())
				done = true;
		}

		if (isVertical) {
			screen._backBuffer1.hLine(pt.x, pt.y + DARTBARSIZE - 1 - idx, pt.x + 8, color);
			screen._backBuffer1.transBlitFrom((*_dartImages)[4], Common::Point(pt.x - 1, pt.y - 1));
			screen.slamArea(pt.x, pt.y + DARTBARSIZE - 1 - idx, 8, 2);
		} else {
			screen._backBuffer1.vLine(pt.x + idx, pt.y, pt.y + 8, color);
			screen._backBuffer1.transBlitFrom((*_dartImages)[3], Common::Point(pt.x - 1, pt.y - 1));
			screen.slamArea(pt.x + idx, pt.y, 1, 8);
		}

		if (sound._musicOn) {
			if (!(idx % 3))
				sound.waitTimerRoland(1);
		} else {
			if (!(idx % 8))
				events.wait(1);
		}

		++idx;
	} while (!done);

	return MIN(idx * 100 / DARTBARSIZE, 100);
}

/**
 * Returns true if a mouse button or key is pressed.
 */
int Darts::dartHit() {
	Events &events = *_vm->_events;

	if (events.kbHit()) {
		Common::KeyState keyState = events.getKey();

		events.clearKeyboard();
		return keyState.keycode;
	}
	
	events.setButtonState();
	return events._pressed && !_oldDartButtons ? 1 : 0;
}

/**
 * Return the score of the given location on the dart-board
 */
int Darts::dartScore(const Common::Point &pt) {
	Common::Point pos(pt.x - 37, pt.y - 33);

	if (pos.x < 0 || pos.y < 0 || pos.x >= 147 || pt.y >= 132)
		// Not on the board
		return 0;

	// On board, so get the score from the pixel at that position
	int score = *(const byte *)(*_dartImages)[2]._frame.getBasePtr(pos.x, pos.y);
	return score;
}

/**
 * Calculates where a computer player is trying to throw their dart, and choose the actual 
 * point that was hit with some margin of error
 */
Common::Point Darts::getComputerDartDest(int playerNum) {
	Common::Point target;
	int aim;
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
		aim = score;

		bool done;
		Common::Point pt;
		do {
			done = findNumberOnBoard(aim, pt);
			--aim;
		} while (!done);

		target.x = 75 + ((target.x - 75) * 20 / 27);
		target.y = 75 + ((target.y - 75) * 2 / 3);
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

/**
 * Returns the center position for the area of the dartboard with a given number
 */
bool Darts::findNumberOnBoard(int aim, Common::Point &pt) {
	ImageFrame &board = (*_dartImages)[2];

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

/**
 * Set a global flag to 0 or 1 depending on whether the passed flag is negative or positive.
 * @remarks		We don't use the global setFlags method because we don't want to check scene flags
 */
void Darts::setFlagsForDarts(int flagNum) {
	_vm->_flags[ABS(flagNum)] = flagNum >= 0;
}

} // End of namespace Scalpel

} // End of namespace Scalpel
