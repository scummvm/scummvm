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

#ifndef ULTIMA8_MISC_IDMAN_H
#define ULTIMA8_MISC_IDMAN_H

#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

//
// idMan. Used to allocate and keep track of unused ids.
// Supposed to be used by Kernel and World for pID and ObjId
//
// The idMan itself uses a nifty linked list that is also an array.
// As such it has the advantages of both. Adds and removals are fast,
// As is checking to see if an ID is used.
//
// idMan works as a LILO (last in last out) for id recycling. So an id that has
// been cleared will not be used until all other currently unused ids have been
// allocated.
//

class idMan {
	uint16      _begin;          //!< start of the available range of IDs
	uint16      _end;            //!< current end of the range
	uint16      _maxEnd;         //!< end of the available range
	uint16      _startCount;     //!< number of IDs to make available initially

	uint16      _usedCount;      //!< number of IDs currently in use

	Std::vector<uint16> _ids;    //!< the 'next' field in a list of free IDs
	uint16      _first;          //!< the first ID in the free list
	uint16      _last;           //!< the last ID in the last list
public:
	//! \param begin start of the range of available IDs
	//! \param max_end end of the range of available IDs
	//! \param startcount number of IDs to make available initially (0 = all)
	idMan(uint16 begin, uint16 max_end, uint16 startcount = 0);
	~idMan();

	//! check if this idMan is full
	bool        isFull() const {
		return _first == 0 && _end >= _maxEnd;
	}

	//! clear all IDs, reset size to the startcount, and set max_end to new_max
	void        clearAll(uint16 new_max = 0);

	//! get a free ID
	//! \return a free ID, or 0 if none are available
	uint16      getNewID();

	//! mark a given ID as used, expanding if necessary.
	//! Note: reserveID is O(n), so don't use too often.
	//! \return false if the ID was already used or is out of range
	bool        reserveID(uint16 id);

	//! release an id
	void        clearID(uint16 id);

	//! check if an ID is in use
	bool        isIDUsed(uint16 id) const {
		return id >= _begin && id <= _end && _ids[id] == 0 && id != _last;
	}

	//! increase the maximum size
	//! Note: this shouldn't be used in normal circumstances.
	//!       It exists for dumpMap currently. If that is rewritten not
	//!       to need more than 32768 object IDs, this function should be
	//!       deleted.
	void        setNewMax(uint16 maxEnd) {
		_maxEnd = maxEnd;
	}

	void save(Common::WriteStream *ws) const;
	bool load(Common::ReadStream *rs, uint32 version);

private:
	//! double the amount of available IDs (up to the maximum passed
	//! to the constructor)
	void expand();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
