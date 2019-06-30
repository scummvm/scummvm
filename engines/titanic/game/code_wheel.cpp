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

#include "titanic/game/code_wheel.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CodeWheel, CBomb)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(CheckCodeWheelsMsg)
END_MESSAGE_MAP()

static const int START_FRAMES_EN[15] = {
	0, 5, 10, 15, 19, 24, 28, 33, 38, 42, 47, 52, 57, 61, 66
};
static const int END_FRAMES_EN[15] = {
	5, 10, 15, 19, 24, 28, 33, 38, 42, 47, 52, 57, 61, 66, 70
};

static const int CORRECT_VALUES_DE[3][8] = {
	{ 2, 7, 4, 8, 18, 18, 4, 17 },
	{ 12, 0, 6, 10, 11, 20, 6, 18 },
	{ 13, 8, 4, 12, 0, 13, 3, 26 }
};
static const int START_FRAMES_DE[28] = {
	0, 7, 15, 22, 29, 37, 44, 51, 58, 66,
	73, 80, 88, 95, 102, 110, 117, 125, 132, 139,
	146, 154, 161, 168, 175, 183, 190, 0
};
static const int END_FRAMES_DE[28] = {
	7, 15, 22, 29, 37, 44, 51, 58, 66, 73,
	80, 88, 95, 102, 110, 117, 125, 132, 139, 146,
	154, 161, 168, 175, 183, 190, 198, 0
};
static const int START_FRAMES_REV_DE[28] = {
	390, 383, 375, 368, 361, 353, 346, 339, 331, 324,
	317, 309, 302, 295, 287, 280, 272, 265, 258, 251,
	244, 236, 229, 221, 214, 207, 199, 0
};
static const int END_FRAMES_REV_DE[28] = {
	397, 390, 383, 375, 368, 361, 353, 346, 339, 331,
	324, 317, 309, 302, 295, 287, 280, 272, 265, 258,
	251, 244, 236, 229, 221, 214, 207, 0
};

CodeWheel::CodeWheel() : CBomb(), _correctValue(0), _value(4),
		_matched(false), _column(0), _row(0) {
}

void CodeWheel::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_correctValue, indent);
	file->writeNumberLine(_value, indent);
	file->writeNumberLine(_matched, indent);

	if (g_language == Common::DE_DEU) {
		file->writeNumberLine(_row, indent);
		file->writeNumberLine(_column, indent);
	}

	CBomb::save(file, indent);
}

void CodeWheel::load(SimpleFile *file) {
	file->readNumber();
	_correctValue = file->readNumber();
	_value = file->readNumber();
	_matched = file->readNumber();

	if (g_language == Common::DE_DEU) {
		_row = file->readNumber();
		_column = file->readNumber();

		assert(_column >= 1 && _column <= 8);
		assert(_row >= 0 && _row <= 2);
		_correctValue = CORRECT_VALUES_DE[_row][_column - 1];
	}

	CBomb::load(file);
}

bool CodeWheel::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	int yp = _bounds.top + _bounds.height() / 2;
	_matched = false;

	if (msg->_mousePos.y > yp) {
		_value = (_value + 1) % TRANSLATE(15, 27);

		playMovie(TRANSLATE(START_FRAMES_EN[_value], START_FRAMES_DE[_value]),
			TRANSLATE(END_FRAMES_EN[_value], END_FRAMES_DE[_value]),
			MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);

	} else {
		playMovie(TRANSLATE(START_FRAMES_EN[14 - _value] + 68, START_FRAMES_REV_DE[_value]),
			TRANSLATE(END_FRAMES_EN[14 - _value] + 68, END_FRAMES_REV_DE[_value]),
			MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);

		_value = (_value <= 0) ? TRANSLATE(14, 26) : _value - 1;
	}

	if (_value == _correctValue)
		_matched = true;

	playSound(TRANSLATE("z#59.wav", "z#590.wav"));
	return true;
}

bool CodeWheel::EnterViewMsg(CEnterViewMsg *msg) {
	// WORKAROUND: Don't keep resetting code wheels back to default
	loadFrame(TRANSLATE(END_FRAMES_EN[_value], END_FRAMES_DE[_value]));
	return true;
}

bool CodeWheel::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	return true;
}

bool CodeWheel::MovieEndMsg(CMovieEndMsg *msg) {
	sleep(200);

	// Signal that a code wheel has changed
	CStatusChangeMsg changeMsg;
	changeMsg.execute("Bomb");

	return true;
}

bool CodeWheel::CheckCodeWheelsMsg(CCheckCodeWheelsMsg *msg) {
	if (_value != _correctValue)
		msg->_isCorrect = false;
	return true;
}

void CodeWheel::reset() {
	_value = TRANSLATE(4, 14);
}

} // End of namespace Titanic
