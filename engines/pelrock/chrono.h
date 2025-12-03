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
#ifndef PELROCK_CHRONO_H
#define PELROCK_CHRONO_H

#include "common/scummsys.h"

namespace Pelrock {

const int kTickMs = 18;
// const int kTickMs = 55;
// const int kTickMs = 15;
const int kHalfTickMultiplier = 2;

class ChronoManager {
private:
	uint32 _lastTick = 0;
	byte _tickCount = 0;
	byte _speedMultiplier = 1;

public:
	ChronoManager();
	~ChronoManager();
	void updateChrono();
	void changeSpeed();
	void delay(uint32 ms);
	void waitForKey();

	bool _gameTick = false;
	bool _gameTickHalfSpeed = false;
	long _textTtl = 0;
};

} // End of namespace Pelrock
#endif
