/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/hodjnpodj/fuge/options.h"
#include "bagel/hodjnpodj/fuge/defines.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

Options::Options() : View("FugeOptions"),
		_numBallsRect(22, 22, 135, 35),
		_startLevelRect(22, 57, 135, 70),
		_ballSpeedRect(22, 92, 135, 105),
		_paddleSizeRect(22, 127, 110, 140),
		_numBallsScroll("NumBalls",
			Common::Rect(22, 35, 92, 53), this),
		_startLevelScroll("StartLevel",
			Common::Rect(22, 70, 92, 88), this),
		_ballSpeedScroll("BallSpeed",
			Common::Rect(22, 105, 92, 123), this),
		_paddleSizeScroll("PaddleSize",
			Common::Rect(22, 140, 92, 158), this),
		_okButton(DialogRect(87, 16, 31, 14), this),
		_cancelButton(DialogRect(87, 32, 31, 14), this),
		_defaultsButton(DialogRect(87, 48, 31, 14), this),
		_outerWallCheck("OuterWall", DialogRect(67, 84, 52, 11), this),
		_settings(g_engine->_settings["Fuge"]) {

	_numBallsScroll.setScrollRange(BALLS_MIN, BALLS_MAX);
	_startLevelScroll.setScrollRange(LEVEL_MIN, LEVEL_MAX);
	_ballSpeedScroll.setScrollRange(SPEED_MIN, SPEED_MAX);
	_paddleSizeScroll.setScrollRange(PSIZE_MIN, PSIZE_MAX);
	_outerWallCheck.setText("Breakout");
}

bool Options::msgOpen(const OpenMessage &msg) {
	loadIniSettings();

	// Load the scroll background and center the view
	_background.loadBitmap("fuge/art/sscroll.bmp");
	_background.setTransparentColor(WHITE);

	Common::Rect r(0, 0, _background.w, _background.h);
	r.moveTo((GAME_WIDTH - _background.w) / 2,
		(GAME_HEIGHT - _background.h) / 2);
	setBounds(r);

	return View::msgOpen(msg);
}

bool Options::msgClose(const CloseMessage &msg) {
	_background.clear();
	return View::msgClose(msg);
}

bool Options::msgGame(const GameMessage &msg) {
	Common::String ctl = msg._stringValue;

	if (msg._name == "BUTTON") {
		if (ctl == "OK") {
			saveIniSettings();
			close();
		} else if (ctl == "CANCEL") {
			close();
		} else if (ctl == "DEFAULTS") {
			reset();
		}

		return true;

	} else if (msg._name == "SCROLL") {
		if (ctl == "NumBalls") {
			_numBalls = msg._value;
			_hasChanges = true;
		} else if (ctl == "StartLevel") {
			_startLevel = msg._value;
			_hasChanges = true;
		} else if (ctl == "BallSpeed") {
			_ballSpeed = msg._value;
			_hasChanges = true;
		} else if (ctl == "PaddleSize") {
			_paddleSize = msg._value;
			_hasChanges = true;
		}

		redraw();
		return true;

	} else if (msg._name == "CHECKBOX") {
		_outerWall = msg._value;
		_hasChanges = true;
		redraw();
		return true;
	}

	return false;
}

void Options::draw() {
	GfxSurface s = getSurface();
	Common::String text;
	s.setFontSize(8);

	// Copy the background
	s.blitFrom(_background);

	// Write the scrollbars header text
	text = Common::String::format("Number of Balls: %d", _numBalls);
	s.writeString(text, _numBallsRect);
	text = Common::String::format("Starting Level: %d", _startLevel);
	s.writeString(text, _startLevelRect);
	text = Common::String::format("Ball Speed: %d", _ballSpeed);
	s.writeString(text, _ballSpeedRect);
	text = Common::String::format("Paddle Size: %d", _paddleSize);
	s.writeString(text, _paddleSizeRect);
}

void Options::reset() {
	_numBalls = BALLS_DEF;
	_startLevel = LEVEL_DEF;
	_ballSpeed = SPEED_DEF;
	_paddleSize = PSIZE_DEF;
	_outerWall = false;

	putDialogData();
}

void Options::putDialogData() {
	_numBallsScroll.setScrollPos(_numBalls);
	_startLevelScroll.setScrollPos(_startLevel);
	_ballSpeedScroll.setScrollPos(_ballSpeed);
	_paddleSizeScroll.setScrollPos(_paddleSize);
	_outerWallCheck.setCheck(_outerWall);
	_hasChanges = false;

	redraw();
}

void Options::loadIniSettings() {
	_numBalls = !_settings.hasKey("NumberOfBalls") ? BALLS_DEF :
		_settings.getInt("NumberOfBalls");
	if ((_numBalls < BALLS_MIN) || (_numBalls > BALLS_MAX))
		_numBalls = BALLS_DEF;

	_startLevel = !_settings.hasKey("StartingLevel") ? LEVEL_DEF :
		_settings.getInt("StartingLevel");
	if ((_startLevel < LEVEL_MIN) || (_startLevel > LEVEL_MAX))
		_startLevel = LEVEL_DEF;

	_ballSpeed = !_settings.hasKey("BallSpeed") ? SPEED_DEF :
		_settings.getInt("BallSpeed");
	if ((_ballSpeed < SPEED_MIN) || (_ballSpeed > SPEED_MAX))
		_ballSpeed = SPEED_DEF;

	_paddleSize = !_settings.hasKey("PaddleSize") ? PSIZE_DEF :
		_settings.getInt("PaddleSize");
	if ((_paddleSize < PSIZE_MIN) || (_paddleSize > PSIZE_MAX))
		_paddleSize = PSIZE_DEF;

	_outerWall = !_settings.hasKey("OuterWall") ? false :
		_settings.getBool("OuterWall");

	_hasChanges = false;
}

void Options::saveIniSettings() {
	if (!_hasChanges)
		return;

	_settings.setInt("NumberOfBalls", _numBalls);
	_settings.setInt("StartingLevel", _startLevel);
	_settings.setInt("BallSpeed", _ballSpeed);
	_settings.setInt("PaddleSize", _paddleSize);
	_settings.setBool("OuterWall", _outerWall);
}

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel
