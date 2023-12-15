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

#ifndef TWP_THREAD_H
#define TWP_THREAD_H

#include "common/array.h"
#include "common/str.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

class Thread {
public:
	Thread();
	~Thread();

	bool call();
	bool update(float elapsed);
	void suspend();
	void resume();
	bool isDead();
	bool isSuspended();
	void stop();

public:
	uint64 _id=0;
	Common::String _name;
    bool _global = false;
	HSQOBJECT _obj, _threadObj, _envObj, _closureObj;
	Common::Array<HSQOBJECT> _args;
	bool _paused=false;
	float _waitTime = 0.f;
	int _numFrames = 0;
	bool _stopRequest = false;
};
}

#endif
