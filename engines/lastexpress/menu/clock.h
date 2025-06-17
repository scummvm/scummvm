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

#ifndef LASTEXPRESS_CLOCK_H
#define LASTEXPRESS_CLOCK_H

#include "common/scummsys.h"

namespace LastExpress {

class LastExpressEngine;

struct Seq;
struct Sprite;

class Clock {
public:
	Clock(LastExpressEngine *engine);
	~Clock();

	void startClock(int32 time);
	void endClock();
	void eraseAllSprites();
	void setClock(int32 time);
	void stopClock(int32 time);
	void turnOnClock(bool draw);
	void drawHands(int32 time);
	void drawTrainPosition(int32 time);
	void tickClock();
	bool statusClock();
	int32 getTimeShowing();
	int32 getTimeTo();

	// Debugger
	void showCurrentTime();

private:
	LastExpressEngine *_engine;

	int32 _timeTo = 0;
	int32 _timeShowing = 0;
	int32 _clockTickDelta = 0;

	Seq *_seqMinutes = nullptr;
	Seq *_seqHour = nullptr;
	Seq *_seqSun = nullptr;
	Seq *_seqDate = nullptr;
	Seq *_seqLine1 = nullptr;
	Seq *_seqLine2 = nullptr;

	Sprite *_clockHour = nullptr;
	Sprite *_clockMinutes = nullptr;
	Sprite *_clockSun = nullptr;
	Sprite *_clockDate = nullptr;
	Sprite *_trainLine1 = nullptr;
	Sprite *_trainLine2 = nullptr;

	int32 _trainLineTimes[31] = {
		1037700, 1148400, 1170900, 1228500, 1303200, 1335600,
		1359900, 1367100, 1490400, 1539000, 1563300, 1656000,
		1713600, 1739700, 1809900, 1852200, 1984500, 2049300,
		2075400, 2101500, 2154600, 2268000, 2383200, 2418300,
		2551500, 2952000, 3205800, 3492000, 3690000, 4320900,
		4941000
	};

	int32 _trainCitiesIndex[31] = {
		  0,   9,  11,  16,  21,  25,
		 35,  37,  40,  53,  56,  60,
		 63,  66,  68,  73,  84,  89,
		 97, 100, 104, 111, 120, 124,
		132, 148, 157, 165, 174, 198,
		210
	};
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_CLOCK_H
