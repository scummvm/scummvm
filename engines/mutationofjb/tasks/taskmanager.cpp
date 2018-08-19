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

#include "mutationofjb/tasks/taskmanager.h"

#include "mutationofjb/tasks/task.h"

namespace MutationOfJB {

void TaskManager::startTask(const TaskPtr &task) {
	_tasks.push_back(task);
	task->setTaskManager(this);
	task->start();
}

void TaskManager::stopTask(const TaskPtr &task) {
	TaskPtrs::iterator it = Common::find(_tasks.begin(), _tasks.end(), task);
	if (it == _tasks.end()) {
		warning("Task is not registered in TaskManager");
		return;
	}

	task->stop();
	assert(task->getState() != Task::RUNNING);
	_tasks.erase(it);
}

TaskPtr TaskManager::getTask(Task *const task) {
	for (TaskPtrs::iterator it = _tasks.begin(); it != _tasks.end(); ++it) {
		if (it->get() == task) {
			return *it;
		}
	}

	return TaskPtr();
}

void TaskManager::update() {
	for (TaskPtrs::iterator it = _tasks.begin(); it != _tasks.end();) {
		const Task::State state = (*it)->getState();
		if (state == Task::RUNNING) {
			(*it)->update();
		}

		if (state == Task::FINISHED) {
			it = _tasks.erase(it);
		} else {
			++it;
		}
	}
}

}
