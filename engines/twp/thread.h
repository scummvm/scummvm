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
	Thread(const Common::String &name, bool global, HSQOBJECT threadObj, HSQOBJECT envObj, HSQOBJECT closureObj, const Common::Array<HSQOBJECT> args);
	~Thread();

	int getId() const { return _id; }
	bool isGlobal() const { return _global; }
	HSQUIRRELVM getThread() const { return _threadObj._unVal.pThread; }
	const Common::String &getName() const { return _name; }

	bool call();
	bool update(float elapsed);

	void pause();
	void unpause();
	void stop();

	bool isSuspended() const;
	bool isDead() const;

	void suspend();
	void resume();

public:
	float _waitTime = 0.f;
	int _numFrames = 0;
	bool _pauseable = false;
	uint32 _lastUpdateTime = 0;

private:
	int _id = 0;
	Common::String _name;
	bool _stopRequest = false;
	bool _paused = false;
	bool _global = false;
	HSQOBJECT _threadObj, _envObj, _closureObj;
	Common::Array<HSQOBJECT> _args;
};

} // namespace Twp

#endif
