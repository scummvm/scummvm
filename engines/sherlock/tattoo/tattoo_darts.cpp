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
	DART_BAR_FORE	= 208
};

static const int STATUS_INFO_X = 430;
static const int STATUS_INFO_Y = 50;
static const int STATUS_INFO_WIDTH = 205;
static const int STATUS_INFO_HEIGHT = 330;
static const int STATUS2_INFO_X = 510;
static const int STATUS2_X_ADD = STATUS2_INFO_X - STATUS_INFO_X;
static const int DART_BAR_VX = 10;
static const int DART_HEIGHT_Y = 121;
static const int DART_BAR_SIZE = 150;
static const int DARTBOARD_LEFT = 73;
static const int DARTBOARD_TOP = 68;
static const int DARTBOARD_WIDTH = 257;
static const int DARTBOARD_HEIGHT = 256;
static const int DARTBOARD_TOTALX = DARTBOARD_WIDTH * 120 / 100;
static const int DARTBOARD_TOTALY = DARTBOARD_HEIGHT * 120 / 100;
static const int DARTBOARD_TOTALTOP = DARTBOARD_TOP - DARTBOARD_WIDTH / 10;
static const int DARTBOARD_TOTALLEFT = DARTBOARD_LEFT - DARTBOARD_HEIGHT / 10;
static const int CRICKET_VALUE[7] = { 20, 19, 18, 17, 16, 15, 25 };

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
	_compPlay = 1;
	_escapePressed = false;
	_spacing = 0;
}

void Darts::playDarts(GameType gameType) {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	int oldFontType = screen.fontNumber();
	int playerNum = 0;
	int lastDart;
	int numHits = 0;
	bool gameOver = false;
	bool done = false;

	// Set the game mode
	_gameType = gameType;

	screen.setFont(7);
	_spacing = screen.fontHeight() + 2;

	// Load dart graphics and initialize values
	loadDarts();
	initDarts();
	events.hideCursor();

	while (!done && !_vm->shouldQuit()) {
		int roundStart, score;
		roundStart = score = (playerNum == 0) ? _score1 : _score2;

		showNames(playerNum);
		showStatus(playerNum);
		_roundScore = 0;

		for (int idx = 0; idx < 3 && !_vm->shouldQuit(); ++idx) {
			if (_compPlay == 1)
				lastDart = throwDart(idx + 1, playerNum * 2);  /* Throw one dart */
			else
				if (_compPlay == 2)
					lastDart = throwDart(idx + 1, playerNum + 1);  /* Throw one dart */
				else
					lastDart = throwDart(idx + 1, 0);    /* Throw one dart */

			if (_gameType == GAME_301) {
				score -= lastDart;
				_roundScore += lastDart;
			} else {
				numHits = lastDart >> 16;
				if (numHits == 0)
					numHits = 1;
				if (numHits > 3)
					numHits = 3;

				lastDart = lastDart & 0xffff;
				updateCricketScore(playerNum, lastDart, numHits);
				score = (playerNum == 0) ? _score1 : _score2;
			}

			// Special case for ScummVM: I'm making pressing Escape to exit out of the Darts game as a way to skip
			// it entirely if you don't want to play all the way through it
			if (_escapePressed) {
				gameOver = true;
				done = true;
				playerNum = 0;
			}


			if (_gameType == GAME_301) {
				if (playerNum == 0)
					_score1 = score;
				else
					_score2 = score;

				if (score == 0)
					// Someone won
					gameOver = true;
			} else {
				// check for cricket game over
				bool allClosed = true;

				for (int y = 0; y < 7; y++) {
					if (_cricketScore[playerNum][y] < 3)
						allClosed = false;
				}

				if (allClosed) {
					int nOtherScore = (playerNum == 0) ? _score2 : _score1;
					if (score >= nOtherScore)
						gameOver = true;
				}
			}

			// Show scores
			showStatus(playerNum);
			screen._backBuffer2.SHblitFrom(screen._backBuffer1, Common::Point(_dartInfo.left, _dartInfo.top - 1),
				Common::Rect(_dartInfo.left, _dartInfo.top - 1, _dartInfo.right, _dartInfo.bottom - 1));
			screen.print(Common::Point(_dartInfo.left, _dartInfo.top), 0, FIXED(DartsCurrentDart), idx + 1);

			if (_gameType == GAME_301) {
				// "Scored x points"
				Common::String scoredPoints;

				// original treated 1 point and multiple points the same. Wrote "Scored 1 points"
				if (lastDart == 1) {
					scoredPoints = Common::String::format(FIXED(DartsScoredPoint), lastDart);
				} else {
					scoredPoints = Common::String::format(FIXED(DartsScoredPoints), lastDart);
				}

				screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing), 0, "%s", scoredPoints.c_str());
			} else {
				Common::String hitText;

				if (lastDart != 25) {
					// Regular hit
					switch (numHits) {
					case 1: // "Hit a X"
						hitText = Common::String::format(FIXED(DartsHitSingle), lastDart);
						break;
					case 2: // "Hit double X"
						hitText = Common::String::format(FIXED(DartsHitDouble), lastDart);
						break;
					case 3: // "Hit triple X"
						hitText = Common::String::format(FIXED(DartsHitTriple), lastDart);
						break;
					default:
						break;
					}
				} else {
					// Bullseye
					switch (numHits) {
					case 1:
						hitText = Common::String(FIXED(DartsHitSingleBullseye));
						break;
					case 2:
						hitText = Common::String(FIXED(DartsHitDoubleBullseye));
						break;
					case 3:
						hitText = Common::String(FIXED(DartsHitTripleBullseye));
						break;
					default:
						break;
					}
				}
				screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing), 0, "%s", hitText.c_str());
			}

			if (score != 0 && playerNum == 0 && !gameOver)
				screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing * 3), 0,
					"%s", FIXED(DartsPressKey));

			if (gameOver) {
				screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing * 3),
					0, "%s", FIXED(DartsGameOver));
				if (playerNum == 0) {
					screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing * 4), 0,
						FIXED(DartsWins), FIXED(DartsPlayerHolmes));
					_vm->setFlagsDirect(531);
				} else {
					screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing * 4), 0,
						FIXED(DartsWins), _opponent.c_str());
					_vm->setFlagsDirect(530);
				}

				screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing * 5), 0,
					"%s", FIXED(DartsPressKey));

				done = true;
				idx = 10;
			} else if (_gameType == GAME_301 && score < 0) {
				screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing * 2), 0,
					"%s!", FIXED(DartsBusted));

				// End turn
				idx = 10;
				score = roundStart;
				if (playerNum == 0)
					_score1 = score;
				else
					_score2 = score;
			}

			// Clear keyboard events
			events.clearEvents();

			if ((playerNum == 0 && _compPlay == 1) || _compPlay == 0 || done) {
				if (_escapePressed) {
					done = true;
					break;
				}
				// Wait for keypress
				do {
					events.pollEventsAndWait();
					events.setButtonState();
				} while (!_vm->shouldQuit() && !events.kbHit() && !events._pressed);
			} else {
				events.wait(40);
			}

			// Clears the status part of the board
			screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(_dartInfo.left, _dartInfo.top - 1),
				Common::Rect(_dartInfo.left, _dartInfo.top - 1, _dartInfo.right, _dartInfo.bottom - 1));
			screen.SHblitFrom(screen._backBuffer1);
		}

		playerNum ^= 1;
		if (!playerNum)
			++_roundNum;

		if (!done) {
			screen._backBuffer2.SHblitFrom((*_dartBoard)[0], Common::Point(0, 0));
			screen._backBuffer1.SHblitFrom(screen._backBuffer2);
			screen.SHblitFrom(screen._backBuffer2);
		}
	}

	// Wait for a keypress
	do {
		events.pollEventsAndWait();
		events.setButtonState();
	} while (!_vm->shouldQuit() && !events.kbHit() && !events._pressed);
	events.clearEvents();

	closeDarts();
	screen.fadeToBlack();
	screen.setFont(oldFontType);

	// Flag to return to the Billard's Academy scene
	scene._goToScene = 26;
}

void Darts::initDarts() {
	_dartInfo = Common::Rect(430, 245, 430 + 205, 245 + 150);
	_escapePressed = false;

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

	_opponent = FIXED(DartsPlayerJock);
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
	Common::SeekableReadStream *stream = res.load("DartBd.pal");
	stream->read(palette, PALETTE_SIZE);
	screen.translatePalette(palette);
	screen.setPalette(palette);
	delete stream;

	// Load the initial background
	screen._backBuffer1.SHblitFrom((*_dartBoard)[0], Common::Point(0, 0));
	screen._backBuffer2.SHblitFrom(screen._backBuffer1);
	screen.SHblitFrom(screen._backBuffer1);
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
	screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y), 0, "%s", FIXED(DartsPlayerHolmes));
	screen._backBuffer1.fillRect(Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + _spacing + 1,
		STATUS_INFO_X + 50, STATUS_INFO_Y + _spacing + 3), color);
	screen.fillRect(Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + _spacing + 1,
		STATUS_INFO_X + 50, STATUS_INFO_Y + _spacing + 3), color);

	color = playerNum == 1 ? PLAYER_COLOR : DART_COLOR_FORE;
	screen.print(Common::Point(STATUS2_INFO_X, STATUS_INFO_Y), 0, "%s", _opponent.c_str());
	screen._backBuffer1.fillRect(Common::Rect(STATUS2_INFO_X, STATUS_INFO_Y + _spacing + 1,
		STATUS2_INFO_X + 50, STATUS_INFO_Y + _spacing + 3), color);
	screen.fillRect(Common::Rect(STATUS2_INFO_X, STATUS_INFO_Y + _spacing + 1,
		STATUS2_INFO_X + 50, STATUS_INFO_Y + _spacing + 3), color);

	screen._backBuffer2.SHblitFrom(screen._backBuffer1);
}

void Darts::showStatus(int playerNum) {
	Screen &screen = *_vm->_screen;
	const char *const CRICKET_SCORE_NAME[7] = { "20", "19", "18", "17", "16", "15", FIXED(DartsBull) };

	screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 10),
		Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10, STATUS_INFO_X + STATUS_INFO_WIDTH,
		STATUS_INFO_Y + STATUS_INFO_HEIGHT - 10));
	screen.print(Common::Point(STATUS_INFO_X + 30, STATUS_INFO_Y + _spacing + 4), 0, "%d", _score1);

	screen.print(Common::Point(STATUS2_INFO_X + 30, STATUS_INFO_Y + _spacing + 4), 0, "%d", _score2);

	int temp = (_gameType == GAME_CRICKET) ? STATUS_INFO_Y + 10 * _spacing + 5 : STATUS_INFO_Y + 55;

	// "Round: x"
	Common::String dartsRoundStatus = Common::String::format(FIXED(DartsCurrentRound), _roundNum);
	screen.print(Common::Point(STATUS_INFO_X, temp), 0, "%s", dartsRoundStatus.c_str());

	if (_gameType == GAME_301) {
		// "Turn Total: x"
		Common::String dartsTotalPoints = Common::String::format(FIXED(DartsCurrentTotalPoints), _roundScore);
		screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 75), 0, "%s", dartsTotalPoints.c_str());
	} else {
		// Show cricket scores
		for (int x = 0; x < 7; ++x) {
			screen.print(Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 40 + x * _spacing), 0, "%s:", CRICKET_SCORE_NAME[x]);

			for (int y = 0; y < 2; ++y) {
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

	screen.SHblitFrom(screen._backBuffer1, Common::Point(STATUS_INFO_X, STATUS_INFO_Y + 10),
		Common::Rect(STATUS_INFO_X, STATUS_INFO_Y + 10, STATUS_INFO_X + STATUS_INFO_WIDTH,
			STATUS_INFO_Y + STATUS_INFO_HEIGHT - 10));
}

void Darts::erasePowerBars() {
	Screen &screen = *_vm->_screen;

	// Erase the old power bars and replace them with empty ones
	screen._backBuffer1.fillRect(Common::Rect(DART_BAR_VX, DART_HEIGHT_Y, DART_BAR_VX + 9, DART_HEIGHT_Y + DART_BAR_SIZE), 0);
	screen._backBuffer1.SHtransBlitFrom((*_dartGraphics)[0], Common::Point(DART_BAR_VX - 1, DART_HEIGHT_Y - 1));
	screen.slamArea(DART_BAR_VX - 1, DART_HEIGHT_Y - 1, 10, DART_BAR_SIZE + 2);
}

bool Darts::dartHit() {
	Events &events = *_vm->_events;
	events.pollEvents();
	events.setButtonState();

	// Keyboard check
	if (events.kbHit()) {
		if (events.getKey().keycode == Common::KEYCODE_ESCAPE)
			_escapePressed = true;

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
	int idx = 0;

	events.clearEvents();
	events.delay(100);

	while (!_vm->shouldQuit() && idx < DART_BAR_SIZE) {
		if ((goToPower - 1) == idx)
			break;
		else if (goToPower == 0) {
			if (dartHit())
				break;
		}

		screen._backBuffer1.hLine(pt.x, pt.y + DART_BAR_SIZE- 1 - idx, pt.x + 8, color);
		screen._backBuffer1.SHtransBlitFrom((*_dartGraphics)[0], Common::Point(pt.x - 1, pt.y - 1));
		screen.slamArea(pt.x, pt.y + DART_BAR_SIZE - 1 - idx, 8, 2);

		if (!(idx % 8))
			events.wait(1);

		++idx;
	}

	return MIN(idx * 100 / DART_BAR_SIZE, 100);
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

	while (!_vm->shouldQuit() && x < DARTBOARD_WIDTH) {
		if (computer && x >= (goToPower - 1))
			break;
		else if (goToPower == 0) {
			if (dartHit())
				break;
		}

		screen._backBuffer1.SHtransBlitFrom((*hands)[0], pt);
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

int Darts::dartScore(const Common::Point &pt) {
	Common::Point pos(pt.x - DARTBOARD_LEFT, pt.y - DARTBOARD_TOP);
	if (pos.x < 0 || pos.y < 0)
		return 0;
	int score;

	if (pos.x < DARTBOARD_WIDTH && pos.y < DARTBOARD_HEIGHT) {
		pos = convertFromScreenToScoreCoords(pos);
		score = *(const byte *)(*_dartMap)[0]._frame.getBasePtr(pos.x, pos.y);

		if (_gameType == GAME_301) {
			if (score >= 100) {
				if (score <= 120)
					// Hit a double
					score = (score - 100) * 2;
				else
					// Hit a triple
					score = (score - 120) * 3;
			}
		} else if (score >= 100) {
			if (score >= 120)
				// Hit a double
				score = (2 << 16) + (score - 100);
			else
				// Hit a triple
				score = (3 << 16) + (score - 120);
		}
	} else {
		score = 0;
	}

	return score;
}

void Darts::drawDartThrow(const Common::Point &dartPos, int computer) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	int cx, cy;
	int xSize = 0, ySize = 0, oldxSize = 0, oldySize = 0;
	int handOCx = 0, handOCy = 0;
	int ocx = 0, ocy = 0;
	int handOldxSize, handOldySize;
	int delta = 9;
	int dartNum;
	int hddy;
	Common::Point drawPos, oldDrawPos;

	// Draw the animation of the hand throwing the dart first
	// See which hand animation to use
	ImageFile &hands = !computer ? *_hand1 : *_hand2;
	int numFrames = !computer ? 14 : 13;

	oldxSize = oldySize = handOldxSize = handOldySize = 1;
	cx = dartPos.x;
	cy = SHERLOCK_SCREEN_HEIGHT - _handSize.y - 20;

	hddy = (cy - dartPos.y) / (numFrames - 7);
	hddy += 2;
	hddy = hddy * 10 / 8;
	if (dartPos.y > 275)
		hddy += 3;

	for (int idx = 0; idx < numFrames; ++idx) {
		_handSize.x = hands[idx]._offset.x + hands[idx]._width;
		_handSize.y = hands[idx]._offset.y + hands[idx]._height;
		int handCy = SHERLOCK_SCREEN_HEIGHT - _handSize.y;

		screen._backBuffer1.SHtransBlitFrom(hands[idx], Common::Point(_handX, handCy));
		screen.slamArea(_handX, handCy, _handSize.x + 1, _handSize.y);
		screen.slamArea(handOCx, handOCy, handOldxSize, handOldySize);
		screen.restoreBackground(Common::Rect(_handX, handCy, _handX + _handSize.x, handCy + _handSize.y));

		handOCx = _handX;
		handOCy = handCy;
		handOldxSize = _handSize.x;
		handOldySize = _handSize.y;

		if (idx > 6) {
			dartNum = idx - 6;
			if (computer)
				dartNum += 19;

			xSize = (*_dartGraphics)[dartNum]._width;
			ySize = (*_dartGraphics)[dartNum]._height;

			ocx = drawPos.x = cx - (*_dartGraphics)[dartNum]._width / 2;
			ocy = drawPos.y = cy - (*_dartGraphics)[dartNum]._height;

			// Draw dart
			screen._backBuffer1.SHtransBlitFrom((*_dartGraphics)[dartNum], drawPos);

			if (drawPos.x < 0) {
				xSize += drawPos.x;
				if (xSize < 0)
					xSize = 1;
				drawPos.x = 0;
			}

			if (drawPos.y < 0) {
				ySize += drawPos.y;
				if (ySize < 0)
					ySize = 1;
				drawPos.y = 0;
			}

			// Flush the drawn dart to the screen
			screen.slamArea(drawPos.x, drawPos.y, xSize, ySize);
			if (oldDrawPos.x != -1)
				// Flush the erased dart area
				screen.slamArea(oldDrawPos.x, oldDrawPos.y, oldxSize, oldySize);

			screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(drawPos.x, drawPos.y),
				Common::Rect(drawPos.x, drawPos.y, drawPos.x + xSize, drawPos.y + ySize));

			oldDrawPos.x = drawPos.x;
			oldDrawPos.y = drawPos.y;
			oldxSize = xSize;
			oldySize = ySize;

			cy -= hddy;
		}

		events.wait(1);
	}

	// Clear the last little bit of the hand from the screen
	screen.slamArea(handOCx, handOCy, handOldxSize, handOldySize);

	// Erase the old dart
	if (oldDrawPos.x != -1)
		screen.slamArea(oldDrawPos.x, oldDrawPos.y, oldxSize, oldySize);

	screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(drawPos.x, drawPos.y),
		Common::Rect(drawPos.x, drawPos.y, drawPos.x + xSize, drawPos.y + ySize));

	cx = dartPos.x;
	cy = dartPos.y + 2;
	oldDrawPos.x = oldDrawPos.y = -1;

	for (int idx = 5; idx <= 23; ++idx) {
		dartNum = idx - 4;
		if (computer)
			dartNum += 19;

		if (idx < 14)
			cy -= delta--;
		else
			if (idx == 14)
				delta = 1;
		if (idx > 14)
			cy += delta++;

		xSize = (*_dartGraphics)[dartNum]._width;
		ySize = (*_dartGraphics)[dartNum]._height;

		ocx = drawPos.x = cx - (*_dartGraphics)[dartNum]._width / 2;
		ocy = drawPos.y = cy - (*_dartGraphics)[dartNum]._height;

		screen._backBuffer1.SHtransBlitFrom((*_dartGraphics)[dartNum], Common::Point(drawPos.x, drawPos.y));

		if (drawPos.x < 0) {
			xSize += drawPos.x;
			if (xSize < 0)
				xSize = 1;
			drawPos.x = 0;
		}

		if (drawPos.y < 0) {
			ySize += drawPos.y;
			if (ySize < 0)
				ySize = 1;
			drawPos.y = 0;
		}

		// flush the dart
		screen.slamArea(drawPos.x, drawPos.y, xSize, ySize);
		if (oldDrawPos.x != -1)
			screen.slamArea(oldDrawPos.x, oldDrawPos.y, oldxSize, oldySize);

		if (idx != 23)
			screen._backBuffer1.SHblitFrom(screen._backBuffer2, drawPos,
				Common::Rect(drawPos.x, drawPos.y, drawPos.x + xSize, drawPos.y + ySize)); // erase dart

		events.wait(1);

		oldDrawPos = drawPos;
		oldxSize = xSize;
		oldySize = ySize;
	}

	dartNum = 19;
	if (computer)
		dartNum += 19;
	xSize = (*_dartGraphics)[dartNum]._width;
	ySize = (*_dartGraphics)[dartNum]._height;

	// Draw final dart on the board
	screen._backBuffer1.SHtransBlitFrom((*_dartGraphics)[dartNum], Common::Point(ocx, ocy));
	screen._backBuffer2.SHtransBlitFrom((*_dartGraphics)[dartNum], Common::Point(ocx, ocy));
	screen.slamArea(ocx, ocy, xSize, ySize);
}

int Darts::findNumberOnBoard(int aim, Common::Point &pt) {
	ImageFrame &img = (*_dartMap)[0];

	if ((aim > 20) && ((aim != 25) && (aim != 50))) {
		if ((aim <= 40) && ((aim & 1) == 0)) {
			aim /= 2;
			aim += 100;
		} else {
			aim /= 3;
			aim += 120;
		}
	}

	bool done = false;
	for (int y = 0; y < img._width && !done; ++y) {
		for (int x = 0; x < img._height && !done; ++x) {
			byte score = *(const byte *)img._frame.getBasePtr(x, y);

			if (score == aim) {
				// Found a match. Aim at non-double/triple numbers whenever possible.
				// ie. Aim at 18 instead of triple 6 or double 9
				done = true;

				if (aim < 21) {
					pt.x = x + 10;
					pt.y = y + 10;

					score = *(const byte *)img._frame.getBasePtr(x, y);
					if (score != aim)
						done = false;
				} else {
					// Aiming at double or triple
					pt.x = x + 3;
					pt.y = y + 3;
				}
			}
		}
	}

	pt = convertFromScreenToScoreCoords(pt);

	if (aim == 3)
		pt.y += 30;
	if (aim == 17)
		pt.y += 10;

	if (aim == 15) {
		pt.y += 5;
		pt.x += 5;
	}

	pt.y = DARTBOARD_HEIGHT - pt.y;
	return done;
}

void Darts::getComputerNumber(int playerNum, Common::Point &targetPos) {
	int score;
	int aim = 0;
	Common::Point pt;
	bool shootBull = false;

	score = (playerNum == 0) ? _score1 : _score2;

	if (_gameType == GAME_301) {
		// Try to hit number
		aim = score;
		if(score > 60)
			shootBull = true;
	} else {
		bool cricketaimset = false;
		if (_cricketScore[playerNum][6] < 3) {
			// shoot at bull first
			aim = CRICKET_VALUE[6];
			cricketaimset = true;
		} else {
			// Now check and shoot in this order: 20,19,18,17,16,15
			for (int idx = 0; idx < 7; ++idx) {
				if (_cricketScore[playerNum][idx] < 3) {
					aim = CRICKET_VALUE[idx];
					cricketaimset = true;
					break;
				}
			}
		}

		if (!cricketaimset) {
			// Everything is closed
			// just in case we don't get set in loop below, which should never happen
			aim = 14;
			for (int idx = 0; idx < 7; ++idx) {
				if (_cricketScore[playerNum^1][idx] < 3) {
					// Opponent has this open
					aim = CRICKET_VALUE[idx];

					if (idx == 6)
						shootBull = true;
				}
			}
		}
	}

	if (shootBull) {
		// Aim at bulls eye
		targetPos.x = targetPos.y = 75;

		if (_level <= 1) {
			if (_vm->getRandomNumber(1) == 1) {
				targetPos.x += (_vm->getRandomNumber(20)-10);
				targetPos.y += (_vm->getRandomNumber(20)-10);
			}
		}
	} else {
		// Loop in case number does not exist on board
		bool done = false;
		do {
			done = findNumberOnBoard(aim, pt);
			--aim;
		} while (!done);

		pt.x += DARTBOARD_TOTALLEFT * 70 / 100;
		pt.y += DARTBOARD_TOTALTOP * 70 / 100;

		// old * 3/2
		targetPos.x = pt.x * 100 / DARTBOARD_TOTALX * 3 / 2;
		targetPos.y = pt.y * 100 / DARTBOARD_TOTALY * 3 / 2;
	}

	// the higher the level, the more accurate the throw
	int v = _vm->getRandomNumber(9);
	v += _level * 2;

	if (v <= 2) {
		targetPos.x += _vm->getRandomNumber(70) - 35;
		targetPos.y += _vm->getRandomNumber(70) - 35;
	} else if (v <= 4) {
		targetPos.x += _vm->getRandomNumber(50) - 25;
		targetPos.y += _vm->getRandomNumber(50) - 25;
	} else if (v <= 6) {
		targetPos.x += _vm->getRandomNumber(30) - 15;
		targetPos.y += _vm->getRandomNumber(30) - 15;
	} else if (v <= 8) {
		targetPos.x += _vm->getRandomNumber(20) -10;
		targetPos.y += _vm->getRandomNumber(20) -10;
	} else if (v <= 10) {
		targetPos.x += _vm->getRandomNumber(11) - 5;
		targetPos.y += _vm->getRandomNumber(11) - 5;
	}

	if (targetPos.x < 1)
		targetPos.x = 1;
	if (targetPos.y < 1)
		targetPos.y = 1;
}

int Darts::throwDart(int dartNum, int computer) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	int height;
	int horiz;
	Common::Point targetPos;
	Common::String temp;

	/* clear keyboard buffer */
	events.clearEvents();

	erasePowerBars();

	// "Dart # x"
	Common::String currentDart = Common::String::format(FIXED(DartsCurrentDart), dartNum);
	screen.print(Common::Point(_dartInfo.left, _dartInfo.top), 0, "%s", currentDart.c_str());

	drawDartsLeft(dartNum, computer);

	if (!computer) {
		screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing), 0, "%s", FIXED(DartsStartPressKey1));
		screen.print(Common::Point(_dartInfo.left, _dartInfo.top + _spacing * 2), 0, "%s", FIXED(DartsStartPressKey2));
	}

	if (!computer) {
		// Wait for a hit
		while (!dartHit() && !_vm->shouldQuit())
			events.wait(1);
		if (_escapePressed)
			return 0;
	} else {
		events.wait(1);
	}

	drawDartsLeft(dartNum + 1, computer);
	screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(_dartInfo.left, _dartInfo.top - 1),
		Common::Rect(_dartInfo.left, _dartInfo.top - 1, _dartInfo.right, _dartInfo.bottom - 1));
	screen.SHblitFrom(screen._backBuffer1, Common::Point(_dartInfo.left, _dartInfo.top - 1),
		Common::Rect(_dartInfo.left, _dartInfo.top - 1, _dartInfo.right, _dartInfo.bottom - 1));

	if (computer) {
		getComputerNumber(computer - 1, targetPos);
	} else {
		// Keyboard control
		targetPos = Common::Point(0, 0);
	}

	horiz = drawHand(targetPos.x, computer);
	if (_escapePressed)
		return 0;

	height = doPowerBar(Common::Point(DART_BAR_VX, DART_HEIGHT_Y), DART_BAR_FORE, targetPos.y, 1);
	if (_escapePressed)
		return 0;

	// Invert height
	height = 101 - height;

	// Copy power bars to the secondary back buffer
	screen._backBuffer2.SHblitFrom(screen._backBuffer1, Common::Point(DART_BAR_VX - 1, DART_HEIGHT_Y - 1),
		Common::Rect(DART_BAR_VX - 1, DART_HEIGHT_Y - 1, DART_BAR_VX - 1 + 10,
		DART_HEIGHT_Y - 1 + DART_BAR_SIZE + 2));

	Common::Point dartPos(DARTBOARD_TOTALLEFT + horiz*DARTBOARD_TOTALX / 100,
		DARTBOARD_TOTALTOP + height * DARTBOARD_TOTALY / 100);

	dartPos.x += 2 - _vm->getRandomNumber(4);
	dartPos.y += 2 - _vm->getRandomNumber(4);

	drawDartThrow(dartPos, computer);
	return dartScore(dartPos);
}

void Darts::doCricketScoreHits(int player, int scoreIndex, int numHits) {
	while (numHits--) {
		if (_cricketScore[player][scoreIndex] < 3)
			_cricketScore[player][scoreIndex]++;
		else if (_cricketScore[player ^ 1][scoreIndex] < 3) {
			if (player == 0)
				_score1 += CRICKET_VALUE[scoreIndex];
			else
				_score2 += CRICKET_VALUE[scoreIndex];
		}
	}
}

void Darts::updateCricketScore(int player, int dartVal, int multiplier) {
	if (dartVal < 15)
		return;

	if (dartVal <= 20)
		doCricketScoreHits(player, 20 - dartVal, multiplier);
	else if (dartVal == 25)
		doCricketScoreHits(player, 6, multiplier);
}

void Darts::drawDartsLeft(int dartNum, int computer) {
	Screen &screen = *_vm->_screen;
	const int DART_X1[3] = { 391, 451, 507 };
	const int DART_Y1[3] = { 373, 373, 373 };
	const int DART_X2[3] = { 393, 441, 502 };
	const int DART_Y2[3] = { 373, 373, 373 };

	screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(DART_X1[0], DART_Y1[0]),
		Common::Rect(DART_X1[0], DART_Y1[0], SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	for (int idx = 2; idx >= dartNum - 1; --idx) {
		if (computer)
			screen._backBuffer1.SHtransBlitFrom((*_dartsLeft)[idx + 3], Common::Point(DART_X2[idx], DART_Y2[idx]));
		else
			screen._backBuffer1.SHtransBlitFrom((*_dartsLeft)[idx], Common::Point(DART_X1[idx], DART_Y1[idx]));
	}

	screen.slamArea(DART_X1[0], DART_Y1[0], SHERLOCK_SCREEN_WIDTH - DART_X1[0], SHERLOCK_SCREEN_HEIGHT - DART_Y1[0]);
}

} // End of namespace Tattoo

} // End of namespace Sherlock
