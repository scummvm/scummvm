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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/misc/id_man.h"

namespace Ultima {
namespace Ultima8 {

idMan::idMan(uint16 begin, uint16 maxEnd, uint16 startCount)
	: _begin(begin), _maxEnd(maxEnd), _startCount(startCount) {
	// 0 is always reserved, as is 65535
	if (_begin == 0) _begin = 1;
	if (_maxEnd == 65535) _maxEnd = 65534;
	if (_startCount == 0) _startCount = _maxEnd - _begin + 1;

	_end = _begin + _startCount - 1;
	if (_end > _maxEnd) _end = _maxEnd;

	_ids.resize(_end + 1);
	clearAll();
}

idMan::~idMan() {
}

void idMan::clearAll(uint16 new_max) {
	if (new_max)
		_maxEnd = new_max;

	_end = _begin + _startCount - 1;
	if (_end > _maxEnd) _end = _maxEnd;
	_ids.resize(_end + 1);

	_first = _begin;
	_last  = _end;
	_usedCount = 0;

	uint16 i;
	for (i = 0; i < _first; i++) _ids[i] = 0;     // NPCs always used
	for (; i < _last;  i++) _ids[i] = i + 1;       // Free IDs
	_ids[_last] = 0;                              // Terminates the list

}

uint16 idMan::getNewID() {
	// more than 75% used and room to expand?
	if (_usedCount * 4 > (_end - _begin + 1) * 3 && _end < _maxEnd) {
		expand();
	}

	// Uh oh, what to do when there is none
	if (!_first) {
		warning("Unable to allocate id (max = %d)", _maxEnd);
		return 0;
	}

	// Get the next id
	uint16 id = _first;

	// Set the _first in the list to next
	_first = _ids[id];

	// Set us to used
	_ids[id] = 0;

	// If there is no _first, there is no list, cause there's none left
	// So clear the _last pointer
	if (!_first) _last = 0;

	_usedCount++;

	return id;

}

void idMan::expand() {
	if (_end == _maxEnd) return;

	uint16 old_end = _end;
	unsigned int new_end = _end * 2;
	if (new_end > _maxEnd) new_end = _maxEnd;
	_end = new_end;
	_ids.resize(_end + 1);

#if 0
	perr << "Expanding idMan from (" << _begin << "-" << old_end << ") to ("
	     << _begin << "-" << _end << ")" << Std::endl;
#endif

	// insert the new free IDs at the start
	for (uint16 i = old_end + 1; i < _end; ++i) {
		_ids[i] = i + 1;
	}
	_ids[_end] = _first;
	_first = old_end + 1;
}

bool idMan::reserveID(uint16 id) {
	if (id < _begin || id > _maxEnd) {
		return false;
	}

	// expand until we're big enough to reserve this ID
	while (id > _end) {
		expand();
	}

	if (isIDUsed(id))
		return false; // already used

	_usedCount++;
	// more than 75% used and room to expand?
	if (_usedCount * 4 > (_end - _begin + 1) * 3 && _end < _maxEnd) {
		expand();
	}

	if (id == _first) {
		_first = _ids[id];
		_ids[id] = 0;
		if (!_first) _last = 0;
		return true;
	}

	uint16 node = _ids[_first];
	uint16 prev = _first;

	while (node != id && node != 0) {
		prev = node;
		node = _ids[node];
	}
	assert(node != 0); // list corrupt...

	_ids[prev] = _ids[node];
	_ids[node] = 0;
	if (_last == node)
		_last = prev;
	return true;
}

void idMan::clearID(uint16 id) {
	// Only clear IF it is used. We don't want to screw up the linked list
	// if an id gets cleared twice
	if (isIDUsed(id)) {
		// If there is a _last, then set the _last's next to us
		// or if there isn't a _last, obviously no list exists,
		// so set the _first to us
		if (_last) _ids[_last] = id;
		else _first = id;

		// Set the _end to us
		_last = id;

		// Set our next to terminate
		_ids[id] = 0;

		_usedCount--;
	}

	// double-check we didn't break the list
	assert(!_first || _last);
}

void idMan::save(Common::WriteStream *ws) const {
	ws->writeUint16LE(_begin);
	ws->writeUint16LE(_end);
	ws->writeUint16LE(_maxEnd);
	ws->writeUint16LE(_startCount);
	ws->writeUint16LE(_usedCount);
	uint16 cur = _first;
	while (cur) {
		ws->writeUint16LE(cur);
		cur = _ids[cur];
	}
	ws->writeUint16LE(0); // terminator
}

bool idMan::load(Common::ReadStream *rs, uint32 version) {
	_begin = rs->readUint16LE();
	_end = rs->readUint16LE();
	_maxEnd = rs->readUint16LE();
	_startCount = rs->readUint16LE();
	uint16 realusedcount = rs->readUint16LE();

	_ids.resize(_end + 1);

	for (unsigned int i = 0; i <= _end; ++i) {
		_ids[i] = 0;
	}
	_first = _last = 0;

	uint16 cur = rs->readUint16LE();
	while (cur) {
		clearID(cur);
		cur = rs->readUint16LE();
	}

	_usedCount = realusedcount;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
