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

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

idMan::idMan(uint16 Begin, uint16 MaxEnd, uint16 StartCount)
	: begin(Begin), max_end(MaxEnd), startcount(StartCount) {
	// 0 is always reserved, as is 65535
	if (begin == 0) begin = 1;
	if (max_end == 65535) max_end = 65534;
	if (startcount == 0) startcount = max_end - begin + 1;

	end = begin + startcount - 1;
	if (end > max_end) end = max_end;

	ids.resize(end + 1);
	clearAll();
}

idMan::~idMan() {

}

void idMan::clearAll(uint16 new_max) {
	if (new_max)
		max_end = new_max;

	end = begin + startcount - 1;
	if (end > max_end) end = max_end;
	ids.resize(end + 1);

	first = begin;
	last  = end;
	usedcount = 0;

	uint16 i;
	for (i = 0; i < first; i++) ids[i] = 0;     // NPCs always used
	for (; i < last;  i++) ids[i] = i + 1;       // Free IDs
	ids[last] = 0;                              // Terminates the list

}

uint16 idMan::getNewID() {
	// more than 75% used and room to expand?
	if (usedcount * 4 > (end - begin + 1) * 3 && end < max_end) {
		expand();
	}

	// Uh oh, what to do when there is none
	if (!first) {
		warning("Unable to allocate id (max = %d)", max_end);
		return 0;
	}

	// Get the next id
	uint16 id = first;

	// Set the first in the list to next
	first = ids[id];

	// Set us to used
	ids[id] = 0;

	// If there is no first, there is no list, cause there's none left
	// So clear the last pointer
	if (!first) last = 0;

	usedcount++;

	return id;

}

void idMan::expand() {
	if (end == max_end) return;

	uint16 old_end = end;
	unsigned int new_end = end * 2;
	if (new_end > max_end) new_end = max_end;
	end = new_end;
	ids.resize(end + 1);

#if 0
	perr << "Expanding idMan from (" << begin << "-" << old_end << ") to ("
	     << begin << "-" << end << ")" << std::endl;
#endif

	// insert the new free IDs at the start
	for (uint16 i = old_end + 1; i < end; ++i) {
		ids[i] = i + 1;
	}
	ids[end] = first;
	first = old_end + 1;
}

bool idMan::reserveID(uint16 id) {
	if (id < begin || id > max_end) {
		return false;
	}

	// expand until we're big enough to reserve this ID
	while (id > end) {
		expand();
	}

	if (isIDUsed(id))
		return false; // already used

	usedcount++;
	// more than 75% used and room to expand?
	if (usedcount * 4 > (end - begin + 1) * 3 && end < max_end) {
		expand();
	}

	if (id == first) {
		first = ids[id];
		ids[id] = 0;
		if (!first) last = 0;
		return true;
	}

	uint16 node = ids[first];
	uint16 prev = first;

	while (node != id && node != 0) {
		prev = node;
		node = ids[node];
	}
	assert(node != 0); // list corrupt...

	ids[prev] = ids[node];
	ids[node] = 0;
	if (last == node)
		last = prev;
	return true;
}

void idMan::clearID(uint16 id) {
	// Only clear IF it is used. We don't want to screw up the linked list
	// if an id gets cleared twice
	if (isIDUsed(id)) {
		// If there is a last, then set the last's next to us
		// or if there isn't a last, obviously no list exists,
		// so set the first to us
		if (last) ids[last] = id;
		else first = id;

		// Set the end to us
		last = id;

		// Set our next to terminate
		ids[id] = 0;

		usedcount--;
	}

	// double-check we didn't break the list
	assert(!first || last);
}

void idMan::save(ODataSource *ods) {
	ods->write2(begin);
	ods->write2(end);
	ods->write2(max_end);
	ods->write2(startcount);
	ods->write2(usedcount);
	uint16 cur = first;
	while (cur) {
		ods->write2(cur);
		cur = ids[cur];
	}
	ods->write2(0); // terminator
}

bool idMan::load(IDataSource *ds, uint32 version) {
	begin = ds->read2();
	end = ds->read2();
	max_end = ds->read2();
	startcount = ds->read2();
	uint16 realusedcount = ds->read2();

	ids.resize(end + 1);

	for (unsigned int i = 0; i <= end; ++i) {
		ids[i] = 0;
	}
	first = last = 0;

	uint16 cur = ds->read2();
	while (cur) {
		clearID(cur);
		cur = ds->read2();
	}

	usedcount = realusedcount;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
