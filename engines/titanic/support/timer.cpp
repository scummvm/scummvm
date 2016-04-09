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

#include "titanic/support/timer.h"
#include "titanic/core/project_item.h"

namespace Titanic {

void CTimerList::postLoad(uint ticks, CProjectItem *project) {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->postLoad(ticks, project);
}

void CTimerList::preSave() {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->preSave();
}

void CTimerList::postSave() {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->postSave();
}

void CTimerList::update(uint ticks) {
	// Remove any items that are done
	for (iterator i = begin(); i != end(); ) {
		CTimer *item = *i;
		if (item->_done) {
			i = erase(i);
			delete item;
		} else {
			++i;
		}
	}

	// Handle updating the items
	for (iterator i = begin(); i != end(); ) {
		CTimer *item = *i;
		if (!item->update(ticks)) {
			++i;
		} else {
			i = erase(i);
			delete item;
		}
	}
}

void CTimerList::stop(uint id) {
	for (iterator i = begin(); i != end(); ++i) {
		CTimer *item = *i;
		if (item->_id == id) {
			item->_done = true;
			return;
		}
	}
}

void CTimerList::set44(uint id, uint val) {
	for (iterator i = begin(); i != end(); ++i) {
		CTimer *item = *i;
		if (item->_id == id) {
			item->set44(val);
			return;
		}
	}
}

/*------------------------------------------------------------------------*/

CTimer::CTimer() : _id(0), _done(false),
		_field44(0) {
}

void CTimer::postLoad(uint ticks, CProjectItem *project) {
	warning("TODO");
}

void CTimer::preSave() {
	warning("TODO: CTimer::preSave");
}

void CTimer::postSave() {
	warning("TODO: CTimer::postSave");
}

bool CTimer::update(uint ticks) {
	return false;
}

} // End of namespace Titanic
