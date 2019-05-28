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

#ifndef MUTATIONOFJB_TASK_H
#define MUTATIONOFJB_TASK_H

#include "common/scummsys.h"
#include "common/ptr.h"
#include "common/array.h"

namespace MutationOfJB {

class TaskManager;

/**
 * Base class for tasks.
 */
class Task {
public:
	enum State {
		IDLE,
		RUNNING,
		FINISHED
	};

	Task() : _taskManager(nullptr), _state(IDLE) {}
	virtual ~Task() {}

	virtual void start() = 0;
	virtual void update() = 0;
	virtual void stop() {
		assert(false);    // Assert by default - stopping might not be safe for all tasks.
	}

	void setTaskManager(TaskManager *taskMan) {
		_taskManager = taskMan;
	}

	TaskManager *getTaskManager() {
		return _taskManager;
	}

	State getState() const {
		return _state;
	}

protected:
	void setState(State state) {
		_state = state;
	}

private:
	TaskManager *_taskManager;
	State _state;
};

typedef Common::SharedPtr<Task> TaskPtr;
typedef Common::Array<Common::SharedPtr<Task> > TaskPtrs;

}

#endif
