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

#include "avalanche/clock.h"
#include "avalanche/avalanche.h"

#include "common/system.h"

namespace Avalanche {

Clock::Clock(AvalancheEngine *vm) {
	_vm = vm;
	// Magic value to determine if we just created the instance
	_oldHour = _oldHourAngle = _oldMinute = 17717;
	_hour = _minute = _second = 0;
	_hourAngle = 0;
}

void Clock::update() {
	TimeDate t;
	_vm->_system->getTimeAndDate(t);
	_hour = t.tm_hour;
	_minute = t.tm_min;
	_second = t.tm_sec;

	_hourAngle = (_hour % 12) * 30 + _minute / 2;

	if (_oldHour != _hour)  {
		plotHands();
		chime();
	}

	if (_oldMinute != _minute)
		plotHands();

	if ((_hour == 0) && (_oldHour != 0) && (_oldHour != 17717)) {
		Common::String tmpStr = Common::String::format("Good morning!%c%cYes, it's just past " \
			"midnight. Are you having an all-night Avvy session? Glad you like the game that much!",
			kControlNewLine, kControlNewLine);
		_vm->_dialogs->displayText(tmpStr);
	}
	_oldHour = _hour;
	_oldHourAngle = _hourAngle;
	_oldMinute = _minute;
}

Common::Point Clock::calcHand(uint16 angle, uint16 length, Color color) {
	if (angle > 900) {
		return(Common::Point(177, 177));
	}

	return(_vm->_graphics->drawScreenArc(kCenterX, kCenterY, 449 - angle, 450 - angle, length, color));
}

void Clock::drawHand(const Common::Point &endPoint, Color color) {
	if (endPoint.x == 177)
		return;

	_vm->_graphics->drawScreenLine(kCenterX, kCenterY, endPoint.x, endPoint.y, color);
}

void Clock::plotHands() {
	_clockHandHour = calcHand(_oldHourAngle, 14, kColorYellow);
	_clockHandMinute = calcHand(_oldMinute * 6, 17, kColorYellow);
	drawHand(_clockHandHour, kColorBrown);
	drawHand(_clockHandMinute, kColorBrown);

	_clockHandHour = calcHand(_hourAngle, 14, kColorBrown);
	_clockHandMinute = calcHand(_minute * 6, 17, kColorBrown);
	drawHand(_clockHandHour, kColorYellow);
	drawHand(_clockHandMinute, kColorYellow);
}

void Clock::chime() {
	// Too high - must be first time around
	// Mute - skip the sound generation
	if ((_oldHour == 17717) || (!_vm->_soundFx))
		return;

	byte hour = _hour % 12;
	if (hour == 0)
		hour = 12;

	_vm->_graphics->loadMouse(kCurWait);

	for (int i = 1; i <= hour; i++) {
		for (int j = 1; j <= 3; j++)
			_vm->_sound->playNote((i % 3) * 64 + 140 - j * 30, 50 - j * 12);
		if (i != hour)
			_vm->_system->delayMillis(100);
	}
}

} // End of namespace Avalanche
