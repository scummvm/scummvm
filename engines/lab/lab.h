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
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_H
#define LAB_H

#include "common/array.h"

#include "engines/engine.h"
#include "lab/labfun.h"

struct ADGameDescription;

namespace Lab {

enum GameFeatures {
	GF_LOWRES = 1 << 0,
	GF_WINDOWS_TRIAL = 1 << 1
};

#define ONESECOND 1000

class LabEngine : public Engine {
public:
	LabEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~LabEngine();

	virtual Common::Error run();
	void go();

	const ADGameDescription *_gameDescription;
	Common::Platform getPlatform() const;
	uint32 getFeatures() const;

	bool hasFeature(EngineFeature f) const;
	Common::String generateSaveFileName(uint slot);

	//void showMainMenu();

	LargeSet *_conditions, *_roomsFound;

	// timing.cpp
	void getTime(uint32 *secs, uint32 *micros);
	void addCurTime(uint32 sec, uint32 micros, uint32 *timeSec, uint32 *timeMicros);
	void anyTimeDiff(uint32 sec1, uint32 micros1, uint32 sec2, uint32 micros2, uint32 *diffSecs, uint32 *diffMicros);
	void timeDiff(uint32 sec, uint32 micros, uint32 *diffSec, uint32 *diffMicros);
	void waitForTime(uint32 sec, uint32 micros);

private:
	uint32 _extraGameFeatures;

	// timing.cpp
	void microDelay(uint32 secs, uint32 micros);
};

extern LabEngine *g_lab;

} // End of namespace Lab

#endif // LAB_H
