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

PuzzleTicTacToe::PuzzleTicTacToe(AsylumEngine *engine) : Puzzle(engine) {
	_ticker = 0;
	_frameIndex = 0;
	_lastMarkedField = 0;
	_needToInitialize = false;
	_strikeOutPosition = 0;

	memset(&_gameField, 0, sizeof(_gameField));
	memset(&_field, 0, sizeof(_field));

	_emptyCount = 0;
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
	updateField();
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
		mouseDown();
		break;
	}

	return true;
}

void PuzzleTicTacToe::mouseDown() {
	error("[PuzzleTicTacToe::mouseDown] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Init & update
//////////////////////////////////////////////////////////////////////////
void PuzzleTicTacToe::initField() {
	memset(&_gameField, 32, sizeof(_gameField)); // ' ' == 32
}

void PuzzleTicTacToe::updateField() {
	warning("[PuzzleTicTacToe::updateField] Not implemented!");
}

void PuzzleTicTacToe::updatePositions(uint32 field1, uint32 field2, uint32 field3) {
	error("[PuzzleTicTacToe::updatePositions] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Game
//////////////////////////////////////////////////////////////////////////
bool PuzzleTicTacToe::check() {
	error("[PuzzleTicTacToe::check] Not implemented!");
}

PuzzleTicTacToe::GameStatus PuzzleTicTacToe::checkField(uint32 field1, uint32 field2, uint32 field3, char mark, uint32 *counterX, uint32 *counterO) {
	error("[PuzzleTicTacToe::checkField] Not implemented!");
}

bool PuzzleTicTacToe::checkFields1() {
	error("[PuzzleTicTacToe::checkFields1] Not implemented!");
}

bool PuzzleTicTacToe::checkFields2() {
	error("[PuzzleTicTacToe::checkFields2] Not implemented!");
}

uint32 PuzzleTicTacToe::checkPosition(uint32 position1, uint32 position2, uint position32) {
	error("[PuzzleTicTacToe::checkPosition] Not implemented!");
}

bool PuzzleTicTacToe::checkWinner() {
	error("[PuzzleTicTacToe::checkWinner] Not implemented!");
}

bool PuzzleTicTacToe::checkWinnerHelper() {
	error("[PuzzleTicTacToe::checkWinnerHelper] Not implemented!");
}

bool PuzzleTicTacToe::checkWinningO() {
	error("[PuzzleTicTacToe::checkWinningO] Not implemented!");
}

bool PuzzleTicTacToe::checkWinningX() {
	error("[PuzzleTicTacToe::checkWinningX] Not implemented!");
}

bool PuzzleTicTacToe::countEmptyFields() {
	error("[PuzzleTicTacToe::countEmptyFields] Not implemented!");
}

void PuzzleTicTacToe::placeOpponentMark() {
	error("[PuzzleTicTacToe::placeOpponentMark] Not implemented!");
}

} // End of namespace Asylum
