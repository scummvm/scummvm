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

#ifndef TITANIC_TIMER_H
#define TITANIC_TIMER_H

#include "common/algorithm.h"
#include "titanic/core/list.h"

namespace Titanic {

class CTreeItem;
class CProjectItem;

class CTimeEventInfo : public ListItem {
private:
	/**
	 * Increments the counter
	 */
	void lock() { ++_lockCounter; }

	/**
	 * Called at the end of both post load and post save actions
	 */
	void unlock() {
		_lockCounter = MAX(_lockCounter - 1, 0);
	}
public:
	static uint _nextId;
public:
	int _lockCounter;
	uint _id;
	bool _repeated;
	uint _firstDuration;
	uint _repeatDuration;
	CTreeItem *_target;
	uint _actionVal;
	CString _action;
	uint _timerCtr;
	uint _lastTimerTicks;
	uint _relativeTicks;
	bool _done;
	bool _persisent;
	CString _targetName;
public:
	CLASSDEF;
	CTimeEventInfo();
	CTimeEventInfo(uint ticks, bool repeated, uint firstDuration, uint repeatDuration,
		CTreeItem *target, int endVal, const CString &action);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;

	/**
	 * Called after loading a game has finished
	 */
	void postLoad(uint ticks, CProjectItem *project);

	/**
	 * Called when a game is about to be saved
	 */
	void preSave(uint ticks);

	/**
	 * Called when a game has finished being saved
	 */
	void postSave();

	bool update(uint ticks);

	/**
	 * Flags whether the timer will be persisent across save & loads
	 */
	void setPersisent(bool val) { _persisent = val; }
};

class CTimeEventInfoList : public List<CTimeEventInfo> {
public:
	/**
	 * Called after loading a game has finished
	 */
	void postLoad(uint ticks, CProjectItem *project);

	/**
	 * Called when a game is about to be saved
	 */
	void preSave(uint ticks);

	/**
	 * Called when a game has finished being saved
	 */
	void postSave();

	/**
	 * Handles an update
	 */
	void update(uint ticks);

	/**
	 * Remove an item with the given Id
	 */
	void stop(uint id);

	/**
	 * Sets whether a timer with a given Id will be persisent across saves
	 */
	void setPersisent(uint id, bool flag);
};

} // End of namespace Titanic

#endif /* TITANIC_TIMER_H */
