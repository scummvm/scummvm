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

#ifndef AVALANCHE_CLOCK_H
#define AVALANCHE_CLOCK_H

#include "common/rect.h"
#include "avalanche/enums.h"

namespace Avalanche {
class AvalancheEngine;

class Clock {
public:
	Clock(AvalancheEngine *vm);

	void update();

private:
	static const int kCenterX = 510;
	static const int kCenterY = 183;

	AvalancheEngine *_vm;

	uint16 _hour, _minute, _second, _hourAngle, _oldHour, _oldMinute, _oldHourAngle;
	Common::Point _clockHandHour, _clockHandMinute;

	Common::Point calcHand(uint16 angle, uint16 length, Color color);
	void drawHand(const Common::Point &endPoint, Color color);
	void plotHands();
	void chime();
};

} // End of namespace Avalanche

#endif // AVALANCHE_CLOCK_H
