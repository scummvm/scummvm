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

#ifndef TWPVM_H
#define TWPVM_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "twp/squirrel/squirrel.h"
#include "twp/ggpack.h"

namespace Twp {
class Entity {
public:
	HSQOBJECT obj;
	Graphics::ManagedSurface surface;
	Common::Rect rect;
	int x;
	int y;
};

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

public:
	uint64 id;
	Common::String name;
    bool global;
	HSQOBJECT obj, threadObj, envObj, closureObj;
	Common::Array<HSQOBJECT> args;
	bool paused;
	float waitTime;
	int numFrames;
	bool stopRequest;
};

class Vm {
public:
	Vm();
	~Vm();

	void exec(const SQChar *code);

private:
	HSQUIRRELVM v;
};
} // End of namespace Twp

#endif // TWPVM_H
