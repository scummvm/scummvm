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

#include "common/algorithm.h"
#include "common/stream.h"
#include "common/system.h"
#include "access/data.h"

namespace Access {

TimerList::TimerList() : Common::Array<TimerEntry>() {
	_timersSavedFlag = false;
}

void TimerList::saveTimers() {
	if (!_timersSavedFlag /* && !_flashbackFlag */) {
		_savedTimers = *this;
		_timersSavedFlag = true;
	}
}

void TimerList::restoreTimers() {
	if (_timersSavedFlag /* && !_flashbackFlag */) {
		clear();
		*static_cast<Common::Array<TimerEntry> *>(this) = _savedTimers;
		_timersSavedFlag = false;
	}
}

void TimerList::updateTimers() {
	for (uint i = 0; i < size(); ++i) {
		TimerEntry &te = (*this)[i];
		if (te._flag) {
			if (!--te._timer) {
				te._timer = te._initTm;
				te._flag = 0;
			}
		}
	}
}

void TimerList::synchronize(Common::Serializer &s) {
	int count = size();
	s.syncAsUint16LE(count);

	if (!s.isSaving())
		resize(count);

	for (int i = 0; i < count; ++i) {
		s.syncAsUint32LE((*this)[i]._initTm);
		s.syncAsUint32LE((*this)[i]._timer);
		s.syncAsByte((*this)[i]._flag);
	}
}

} // End of namespace Access
