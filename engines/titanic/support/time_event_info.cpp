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

#include "titanic/support/time_event_info.h"
#include "titanic/core/game_object.h"
#include "titanic/core/project_item.h"
#include "titanic/messages/messages.h"

namespace Titanic {

void CTimeEventInfoList::postLoad(uint ticks, CProjectItem *project) {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->postLoad(ticks, project);
}

void CTimeEventInfoList::preSave(uint ticks) {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->preSave(ticks);
}

void CTimeEventInfoList::postSave() {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->postSave();
}

void CTimeEventInfoList::update(uint ticks) {
	// Remove any items that are done
	for (iterator i = begin(); i != end(); ) {
		CTimeEventInfo *item = *i;
		if (item->_done) {
			i = erase(i);
			delete item;
		} else {
			++i;
		}
	}

	// Handle updating the items
	for (iterator i = begin(); i != end(); ) {
		CTimeEventInfo *item = *i;
		if (!item->update(ticks)) {
			++i;
		} else {
			i = erase(i);
			delete item;
		}
	}
}

void CTimeEventInfoList::stop(uint id) {
	for (iterator i = begin(); i != end(); ++i) {
		CTimeEventInfo *item = *i;
		if (item->_id == id) {
			item->_done = true;
			return;
		}
	}
}

void CTimeEventInfoList::setPersisent(uint id, bool flag) {
	for (iterator i = begin(); i != end(); ++i) {
		CTimeEventInfo *item = *i;
		if (item->_id == id) {
			item->setPersisent(flag);
			return;
		}
	}
}

/*------------------------------------------------------------------------*/

uint CTimeEventInfo::_nextId;

CTimeEventInfo::CTimeEventInfo() : ListItem(), _lockCounter(0),
		_repeated(false), _firstDuration(0), _repeatDuration(0),
		_target(nullptr), _actionVal(0), _timerCtr(0), _done(false),
		_lastTimerTicks(0), _relativeTicks(0), _persisent(true) {
	_id = _nextId++;
}

CTimeEventInfo::CTimeEventInfo(uint ticks, bool repeated, uint firstDuration,
		uint repeatDuration, CTreeItem *target, int endVal, const CString &action) :
		ListItem(), _lockCounter(0), _repeated(repeated), _firstDuration(firstDuration),
		_repeatDuration(repeatDuration), _target(target), _actionVal(endVal), _action(action),
		_done(false), _timerCtr(0), _lastTimerTicks(ticks), _relativeTicks(0), _persisent(true) {
	_id = _nextId++;
}

void CTimeEventInfo::save(SimpleFile *file, int indent) {
	file->writeNumberLine(0, indent);

	CString targetName;
	if (_target)
		targetName = _target->getName();
	file->writeQuotedLine(targetName, indent);
	file->writeNumberLine(_id, indent);
	file->writeNumberLine(_repeated, indent);
	file->writeNumberLine(_firstDuration, indent);
	file->writeNumberLine(_repeatDuration, indent);
	file->writeNumberLine(_actionVal, indent);
	file->writeQuotedLine(_action, indent);
	file->writeNumberLine(_timerCtr, indent);
	file->writeNumberLine(_relativeTicks, indent);
	file->writeNumberLine(_done, indent);
	file->writeNumberLine(_persisent, indent);
}

void CTimeEventInfo::load(SimpleFile *file) {
	lock();
	int val = file->readNumber();

	if (!val) {
		_targetName = file->readString();
		_id = file->readNumber();
		_repeated = file->readNumber();
		_firstDuration = file->readNumber();
		_repeatDuration = file->readNumber();
		_actionVal = file->readNumber();
		_action = file->readString();
		_timerCtr = file->readNumber();
		_relativeTicks = file->readNumber();
		_done = file->readNumber() != 0;
		_persisent = file->readNumber() != 0;
		_target = nullptr;
	}
}

void CTimeEventInfo::postLoad(uint ticks, CProjectItem *project) {
	if (!_persisent || _targetName.empty())
		_done = true;

	// Get the timer's target
	if (project)
		_target = project->findByName(_targetName);
	if (!_target)
		_done = true;

	_lastTimerTicks = ticks + _relativeTicks;
	if (_id >= _nextId)
		_nextId = _id + 1;

	unlock();
}

void CTimeEventInfo::preSave(uint ticks) {
	_relativeTicks = _lastTimerTicks - ticks;
	lock();
}

void CTimeEventInfo::postSave() {
	unlock();
}

bool CTimeEventInfo::update(uint ticks) {
	if (_lockCounter)
		return false;

	if (_timerCtr) {
		if (ticks > (_lastTimerTicks + _repeatDuration)) {
			++_timerCtr;
			_lastTimerTicks = ticks;

			if (_target) {
				CTimerMsg timerMsg(ticks, _timerCtr, _actionVal, _action);
				timerMsg.execute(_target);
			}
		}
	} else {
		if (ticks > (_lastTimerTicks + _firstDuration)) {
			_timerCtr = 1;
			_lastTimerTicks = ticks;

			if (_target) {
				CTimerMsg timerMsg(ticks, _timerCtr, _actionVal, _action);
				timerMsg.execute(_target);
			}

			if (!_repeated)
				// Event is done, and can be removed
				return true;
		}
	}

	return false;
}

} // End of namespace Titanic
