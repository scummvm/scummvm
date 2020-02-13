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

#ifndef BLADERUNNER_TIME_H
#define BLADERUNNER_TIME_H

#include "common/types.h"

namespace BladeRunner {

class BladeRunnerEngine;

class Time {
	BladeRunnerEngine *_vm;

	uint32 _start;
	int _pauseCount;
	uint32 _offset;
	uint32 _pauseStart;

public:
	Time(BladeRunnerEngine *vm);

	uint32  currentSystem();
	uint32  current();
	int  pause();
	uint32  getPauseStart();
	int  resume();
	bool isLocked();
	void resetPauseStart();
};

} // End of namespace BladeRunner

#endif
