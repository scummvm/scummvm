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
 * $URL$
 * $Id$
 *
 */

#include "asylum/puzzles/tictactoe.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const Common::Point puzzleTicTacToePolygons[36] = {
	Common::Point( 27, 381), Common::Point(172, 368),
	Common::Point(190, 474), Common::Point( 36, 476),
	Common::Point(176, 362), Common::Point(294, 328),
	Common::Point(331, 456), Common::Point(191, 472),
	Common::Point(304, 327), Common::Point(426, 306),
	Common::Point(457, 440), Common::Point(340, 460),
	Common::Point( 26, 257), Common::Point(151, 238),
	Common::Point(169, 356), Common::Point( 27, 373),
	Common::Point(162, 234), Common::Point(275, 214),
	Common::Point(299, 321), Common::Point(173, 355),
	Common::Point(283, 210), Common::Point(403, 173),
	Common::Point(437, 294), Common::Point(305, 317),
	Common::Point( 22, 120), Common::Point(132, 126),
	Common::Point(146, 223), Common::Point( 25, 247),
	Common::Point(144, 119), Common::Point(247,  87),
	Common::Point(268, 205), Common::Point(159, 222),
	Common::Point(259,  84), Common::Point(380,  73),
	Common::Point(405, 169), Common::Point(281, 201)
};

const Common::Point puzzleTicTacToePositions[9] = {
	Common::Point( 62, 367),
	Common::Point(193, 343),
	Common::Point(329, 319),
	Common::Point( 38, 247),
	Common::Point(167, 217),
	Common::Point(296, 193),
	Common::Point( 19, 121),
	Common::Point(147,  98),
	Common::Point(269,  70)
};

static const struct {
	uint32 field1;
	uint32 field2;
	uint32 field3;
	uint32 strikeOutPositionX;
	uint32 strikeOutPositionO;
	uint32 frameCount;
} fieldsToCheck[8] = {
	{0, 1, 2, 1,  9, 14},
	{3, 4, 5, 2, 10, 14},
	{6, 7, 8, 3, 11, 14},
	{0, 3, 6, 4, 12, 10},
	{4, 1, 7, 5, 13, 10},
	{8, 5, 2, 6, 14, 10},
	{4, 6, 2, 7, 15,  4},
	{0, 4, 8, 8, 16,  4}
};

PuzzleTicTacToe::PuzzleTicTacToe(AsylumEngine *engine) : Puzzle(engine) {
	_ticker = 0;
	_frameIndex = 0;
	_frameCount = 0;
	_lastMarkedField = 0;
	_needToInitialize = false;
	_strikeOutPosition = 0;

	// Field
	memset(&_gameField, 0, sizeof(_gameField));
	memset(&_field, 0, sizeof(_field));
	_emptyCount = 0;

	_var5 = 0;
}

PuzzleTicTacToe::~PuzzleTicTacToe() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleTicTacToe::init()  {
	_ticker = 0;
	_vm->clearGameFlag(kGameFlag114);
	_vm->clearGameFlag(kGameFlag215);
	_frameIndex = 0;
	_lastMarkedField = -1;
	_needToInitialize = false;
	_strikeOutPosition = -1;

	getScreen()->setPalette(getWorld()->graphicResourceIds[3]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[3], 0);

	getCursor()->show();
	getCursor()->set(getWorld()->graphicResourceIds[12], 4, kCursorAnimationLinear);

	initField();

	return true;
}

bool PuzzleTicTacToe::update()  {
	if (_ticker) {
		++_ticker;

		if (_ticker <= 25) {
			if (_ticker > 20) {
				if (check())
					placeOpponentMark();

				_ticker = 0;
			}
		} else {
			if (_ticker > 40) {
				getSound()->playSound(getWorld()->soundResourceIds[13], false, Config.sfxVolume - 100);

				_ticker = 0;
			}
		}
	}

	getScreen()->draw(getWorld()->graphicResourceIds[0]);
	drawField();
	getScene()->updateAmbientSounds();

	getScreen()->copyBackBufferToScreen();

	return true;
}

bool PuzzleTicTacToe::key(const AsylumEvent &evt) {
	switch (evt.kbd.keycode) {
	default:
		_vm->switchEventHandler(getScene());
		break;

	case Common::KEYCODE_TAB:
		getScreen()->takeScreenshot();
		break;
	}

	return true;
}

bool PuzzleTicTacToe::mouse(const AsylumEvent &evt) {
	switch (evt.type) {
	default:
		break;

	case Common::EVENT_RBUTTONDOWN:
		exit();
		break;

	case Common::EVENT_LBUTTONDOWN:
		mouseLeft();
		break;
	}

	return true;
}

void PuzzleTicTacToe::mouseLeft() {
	Common::Point mousePos = getCursor()->position();

	if (!_vm->isGameFlagNotSet(kGameFlag215) || !_vm->isGameFlagNotSet(kGameFlag114)) {
		getCursor()->show();
		exit();
		return;
	}

	if (_needToInitialize) {
		_needToInitialize = false;
		_frameIndex = 0;
		_lastMarkedField = -1;
		_strikeOutPosition = -1;
		initField();

		return;
	}

	for (uint32 i = 0; i < ARRAYSIZE(puzzleTicTacToePolygons) - 2; i++) {
		if (hitTest(&puzzleTicTacToePolygons[i], mousePos, 0)) {
			if (_gameField[i] == ' ') {
				getSound()->playSound(getWorld()->soundResourceIds[11], false, Config.sfxVolume - 100);
				_gameField[i] = 'X';
				_lastMarkedField = i;
				_frameIndex = 0;

				getCursor()->hide();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Init & update
//////////////////////////////////////////////////////////////////////////
void PuzzleTicTacToe::initField() {
	memset(&_gameField, 32, sizeof(_gameField)); // ' ' == 32
}

void PuzzleTicTacToe::drawField() {
	warning("[PuzzleTicTacToe::updateField] Not implemented!");
}

void PuzzleTicTacToe::updatePositions(uint32 field1, uint32 field2, uint32 field3) {
	if (_gameField[field1] != ' ') {
		_field[_emptyCount] = field3;
		_field[_emptyCount + 1] = field2;

		_emptyCount += 2;
	}

	if (_gameField[field3] != ' ') {
		_field[_emptyCount] = field1;
		_field[_emptyCount + 1] = field2;

		_emptyCount += 2;
	}

	if (_gameField[field2] != ' ') {
		_field[_emptyCount] = field3;
		_field[_emptyCount + 1] = field1;

		_emptyCount += 2;
	}
}

//////////////////////////////////////////////////////////////////////////
// Game
//////////////////////////////////////////////////////////////////////////
bool PuzzleTicTacToe::check() {
	if (_needToInitialize)
		return false;

	if (!checkWinning('X')
	 && !checkWinning('O')
	 && !checkFieldsUpdatePositions()
	 && !checkFields()
	 && !countEmptyFields()) {
		if (!_var5)
			getCursor()->show();

		_needToInitialize = true;

		return false;
	}

	return true;
}

PuzzleTicTacToe::GameStatus PuzzleTicTacToe::checkField(uint32 field1, uint32 field2, uint32 field3, char mark, uint32 *counterX, uint32 *counterO) {
	*counterX = 0;
	*counterO = 0;
	GameStatus status = kStatus0;

	if (_gameField[field1] == 'X')
		++*counterX;
	if (_gameField[field2] == 'X')
		++*counterX;
	if (_gameField[field3] == 'X')
		++*counterX;

	if (_gameField[field1] == 'O')
		++*counterO;
	if (_gameField[field2] == 'O')
		++*counterO;
	if (_gameField[field3] == 'O')
		++*counterO;

	if (mark == 'O') {
		if (*counterO == 1 && !*counterX)
			status = kStatusFree;
	} else if (mark == 'X') {
		if (!*counterO && *counterX == 1)
			status = kStatusFree;
	}

	if (mark == 'O') {
		if (!*counterO && *counterX == 2)
			status = kStatusNeedBlocking;
	} else if (mark == 'X') {
		if (*counterO == 2 && !*counterX)
			status = kStatusNeedBlocking;
	}

	return status;
}

bool PuzzleTicTacToe::checkFieldsUpdatePositions() {
	uint32 counterX = 0;
	uint32 counterO = 0;

	for (uint32 i = 0; i < ARRAYSIZE(fieldsToCheck) - 1; i++)
		if (checkField(fieldsToCheck[i].field1, fieldsToCheck[i].field2, fieldsToCheck[i].field3, 'O', &counterX, &counterO) == kStatusFree)
			updatePositions(fieldsToCheck[i].field1, fieldsToCheck[i].field2, fieldsToCheck[i].field3);

	return (_emptyCount != 0);
}

bool PuzzleTicTacToe::checkFields() {
	uint32 counterX = 0;
	uint32 counterO = 0;

	for (uint32 i = 0; i < ARRAYSIZE(fieldsToCheck) - 1; i++) {
		checkField(fieldsToCheck[i].field1, fieldsToCheck[i].field2, fieldsToCheck[i].field3, 'O', &counterX, &counterO);

		if (counterX || counterO)
			continue;

		_field[_emptyCount] = 0;
		_field[_emptyCount + 1] = fieldsToCheck[i].field3;
		_field[_emptyCount + 2] = fieldsToCheck[i].field2;

		_emptyCount += 3;
	}

	return (_emptyCount != 0);
}

uint32 PuzzleTicTacToe::checkPosition(uint32 position1, uint32 position2, uint position3) {
	if (_gameField[position1] == ' ')
		return position1;

	if (_gameField[position2] == ' ')
		return position2;

	return position3;
}

bool PuzzleTicTacToe::checkWinner() {
	if (_needToInitialize)
		return true;

	if (checkWinnerHelper() == 1) {
		_vm->setGameFlag(kGameFlag114);
		_ticker = 30;
		return true;
	}

	if (checkWinnerHelper() == -1) {
		_vm->setGameFlag(kGameFlag215);
		_ticker = 30;
		return true;
	}

	return false;
}

int32 PuzzleTicTacToe::checkWinnerHelper() {
	uint32 counterX = 0;
	uint32 counterO = 0;

	for (uint32 i = 0; i < ARRAYSIZE(fieldsToCheck) - 1; i++) {
		checkField(fieldsToCheck[i].field1, fieldsToCheck[i].field2, fieldsToCheck[i].field3, 'O', &counterX, &counterO);

		if (counterX == 3) {
			_strikeOutPosition = fieldsToCheck[i].strikeOutPositionX;
			_frameCount = fieldsToCheck[i].frameCount;
			_frameIndex = 0;
			return 1;
		}

		if (counterO == 3) {
			_strikeOutPosition = fieldsToCheck[i].strikeOutPositionO;
			_frameCount = fieldsToCheck[i].frameCount;
			_frameIndex = 0;
			return -1;
		}
	}

	return 0;
}

bool PuzzleTicTacToe::checkWinning(char mark) {
	uint32 counterX = 0;
	uint32 counterO = 0;
	_emptyCount = 0;

	for (uint32 i = 0; i < ARRAYSIZE(fieldsToCheck) - 1; i++) {
		if (checkField(fieldsToCheck[i].field1, fieldsToCheck[i].field2, fieldsToCheck[i].field3, 'O', &counterX, &counterO) == kStatusNeedBlocking) {
			_field[_emptyCount] = checkPosition(fieldsToCheck[i].field1, fieldsToCheck[i].field2, fieldsToCheck[i].field3);
			++_emptyCount;
		}
	}

	return (_emptyCount != 0);
}

bool PuzzleTicTacToe::countEmptyFields() {
	_emptyCount = 0;

	for (uint32 i = 0; i < ARRAYSIZE(_gameField); i++) {
		if (_gameField[i] == ' ') {
			_field[i] = i;
			++_emptyCount;
		}
	}

	return (_emptyCount != 0);
}

void PuzzleTicTacToe::placeOpponentMark() {
	_frameIndex = 0;
	_lastMarkedField = _field[rnd(_emptyCount)];
	_gameField[_lastMarkedField] = 'O';

	getSound()->playSound(getWorld()->soundResourceIds[12], false, Config.sfxVolume - 100);
}

} // End of namespace Asylum
