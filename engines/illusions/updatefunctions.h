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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_UPDATEFUNCTIONS_H
#define ILLUSIONS_UPDATEFUNCTIONS_H

#include "common/func.h"
#include "common/list.h"

namespace Illusions {

enum {
	kUFNext      = 1,  // Run next update funtion
	kUFTerminate = 2   // Terminate update function
};

typedef Common::Functor1<uint, int> UpdateFunctionCallback;

class UpdateFunction {
public:
	int _priority;
	uint32 _sceneId;
	uint _flags;
	UpdateFunctionCallback *_callback;
	UpdateFunction() : _priority(0), _sceneId(0), _flags(0), _callback(0) {}
	~UpdateFunction() { delete _callback; }
	void terminate() { _flags |= 1; }
	int run() { return (*_callback)(_flags); }
};

class UpdateFunctions {
public:
	UpdateFunctions();
	~UpdateFunctions();
	void add(int priority, uint32 sceneId, UpdateFunctionCallback *callback);
	void update();
	void terminateByScene(uint32 sceneId);
protected:
	typedef Common::List<UpdateFunction*> UpdateFunctionList;
	typedef UpdateFunctionList::iterator UpdateFunctionListIterator;

	struct FindInsertionPosition : public Common::UnaryFunction<const UpdateFunction*, bool> {
		int _priority;
		FindInsertionPosition(int priority) : _priority(priority) {}
		bool operator()(const UpdateFunction *updateFunction) const {
			return updateFunction->_priority > _priority;
		}
	};

	Common::List<UpdateFunction*> _updateFunctions;
	uint32 _lastTimerUpdateTime;
};

} // End of namespace Illusions

#endif // ILLUSIONS_UPDATEFUNCTIONS_H
