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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_GAMERATE_H
#define SAGA2_GAMERATE_H

namespace Saga2 {

enum {
	grFramesPerSecond = 0
};

class frameCounter {
	uint32 _ticksPerSecond;
	uint32 _lastTime;

protected:
	uint32 _frames;
	float _instantFrameCount;

public:
	frameCounter(uint32 perSec, uint32 now) {
		_ticksPerSecond = perSec;
		_frames = 0;
		_lastTime = now;
		_instantFrameCount = 0;
	}

	virtual ~frameCounter() {}

	virtual void updateFrameCount() {
		int32 frameTime = gameTime - _lastTime;
		_lastTime = gameTime;
		_frames++;
		_instantFrameCount = frameTime ? _ticksPerSecond / frameTime : 100;
	}

	virtual float frameStat(int statID = grFramesPerSecond) {
		return _instantFrameCount;
	}
};


enum {
	grFramesPerKilosecond   = 1,
	grFPKS1SecAvgNewest     = 2,
	grFPKS1SecAvgNew        = 3,
	grFPKS1SecAvg           = 4,
	grFPKS1SecAvgOld        = 5,
	grFPKS1SecAvgOldest     = 6,
	grFPKS5SecAvg           = 7,
	grFPKSAvg1SecAvg        = 8,
	grFPKS1SecVarNewest     = 9,
	grFPKS1SecVarNew        = 10,
	grFPKS1SecVar           = 11,
	grFPKS1SecVarOld        = 12,
	grFPKS1SecVarOldest     = 13,
	grFPKS5SecVar           = 14,
	grFPKSVar1SecAvg        = 15
};

class frameSmoother: public frameCounter {
	float _desiredFPS;

	uint32 _historySize;
	float *_frameHistory;

	float _avg1Sec[5];
	float _avg5Sec;
	float _secAvg;
	float _dif1Sec[5];
	float _dif5Sec;
	float _secDif;

	float ksHistory(int32 i) {
		return 1000.0 * _frameHistory[i];
	}

	void calculateAverages() {
		// clear averages
		for (int i = 0; i < 5; i++)
			_avg1Sec[i] = 0;

		_avg5Sec = 0;

		// get totals
		for (uint i = 0; i < _historySize; i++)
			_avg1Sec[i / int(_desiredFPS)] += ksHistory(i);

		// get averages
		for (uint i = 0; i < 5; i++) {
			_avg5Sec += _avg1Sec[i];
			_avg1Sec[i] /= _desiredFPS;
		}

		// get broad averages
		_secAvg = _avg5Sec / 5;
		_avg5Sec /= (5 * _desiredFPS);
	}

	void calculateVariance() {
		// clear variances
		for (int i = 0; i < 5; i++)
			_dif1Sec[i] = 0;

		_dif5Sec = 0;

		// get variance totals
		for (uint i = 0; i < _historySize; i++) {
			_dif1Sec[i / int(_desiredFPS)] += ABS(ksHistory(i) - _avg1Sec[i / int(_desiredFPS)]);
			_dif5Sec += ABS(ksHistory(i) - _avg5Sec);
		}

		// get average variances
		for (uint i = 0; i < 5; i++) {
			_secDif += _avg1Sec[i] - _secAvg;
			_dif1Sec[i] /= _desiredFPS;
		}

		// get broad variance
		_dif5Sec /= (5 * _desiredFPS);
	}


public:
	frameSmoother(int32 fps, uint32 perSec, uint32 now);


	virtual ~frameSmoother() {
		if (_frameHistory)
			delete[] _frameHistory;

		_frameHistory = nullptr;
	}

	virtual void updateFrameCount() {
		frameCounter::updateFrameCount();
		_frameHistory[_frames % _historySize] = _instantFrameCount;
		if (0 == (_frames % int(_desiredFPS))) {
			calculateAverages();
			calculateVariance();
		}
	}

	virtual float frameStat(int statID = grFramesPerSecond) {
		int oldestOffset = (_frames % _historySize) / _desiredFPS;
		switch (statID) {
		case grFramesPerKilosecond  :
			return 1000 * _instantFrameCount;
		case grFPKS1SecAvgNewest    :
			return _avg1Sec[4 + oldestOffset];
		case grFPKS1SecAvgNew       :
			return _avg1Sec[3 + oldestOffset];
		case grFPKS1SecAvg          :
			return _avg1Sec[2 + oldestOffset];
		case grFPKS1SecAvgOld       :
			return _avg1Sec[1 + oldestOffset];
		case grFPKS1SecAvgOldest    :
			return _avg1Sec[0 + oldestOffset];
		case grFPKS5SecAvg          :
			return _avg5Sec;
		case grFPKSAvg1SecAvg       :
			return _secAvg;
		case grFPKS1SecVarNewest    :
			return _dif1Sec[4 + oldestOffset];
		case grFPKS1SecVarNew       :
			return _dif1Sec[3 + oldestOffset];
		case grFPKS1SecVar          :
			return _dif1Sec[2 + oldestOffset];
		case grFPKS1SecVarOld       :
			return _dif1Sec[1 + oldestOffset];
		case grFPKS1SecVarOldest    :
			return _dif1Sec[0 + oldestOffset];
		case grFPKS5SecVar          :
			return _dif5Sec;
		case grFPKSVar1SecAvg       :
			return _secDif;
		default:
			return frameCounter::frameStat(statID);
		}
	}
};

frameSmoother::frameSmoother(int32 fps, uint32 perSec, uint32 now)
	: frameCounter(perSec, now) {
	assert(fps);
	_desiredFPS = fps;
	_historySize = fps * 5;
	_frameHistory = new float[_historySize];

	for (uint32 i = 0; i < _historySize; i++)
		_frameHistory[i] = 0;

	for (int i = 0; i < 5; i++)
		_dif1Sec[i] = _avg1Sec[i] = 0;

	_dif5Sec = 0;
	_avg5Sec = 0;
	_secDif = 0;
	_secAvg = 0;
}

} // end of namespace Saga2

#endif
