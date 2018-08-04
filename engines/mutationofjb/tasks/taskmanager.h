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

#ifndef MUTATIONOFJB_TASKMANAGER_H
#define MUTATIONOFJB_TASKMANAGER_H

#include "common/array.h"
#include "task.h"

namespace MutationOfJB {

class Game;
class Task;

/**
 * Handles task management.
 *
 * Tasks are a way run game logic asynchronously.
 */
class TaskManager {
public:
	TaskManager(Game &game) : _game(game) {}

	/**
	 * Adds the task to the internal list and starts it.
	 *
	 * When the task is finished, it is automatically removed from the list.
	 * stopTask does not need to be called for that.
	 */
	void startTask(const TaskPtr &task);

	/**
	 * Stops the task and removes it from the internal list.
	 *
	 * Call this only if you need to explicitly stop the task (usually before it's finished).
	 */
	void stopTask(const TaskPtr &task);

	/**
	 * Gets task shared pointer from raw pointer.
	 *
	 * Since task lifetime is under control of SharedPtr, raw pointers shouldn't be used.
	 * However, if only a raw pointer is available (e.g. this),
	 * the method can be used to obtain a SharedPtr.
	 */
	TaskPtr getTask(Task *task);

	void update();

	Game &getGame() {
		return _game;
	}

private:
	TaskPtrs _tasks;
	Game &_game;
};

}

#endif
