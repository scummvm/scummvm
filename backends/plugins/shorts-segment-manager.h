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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SHORTS_SEGMENT_MANAGER_H
#define SHORTS_SEGMENT_MANAGER_H

#include "loader.h"
#include "common/singleton.h"

#define ShortsMan ShortSegmentManager::instance()

class ShortSegmentManager : public Common::Singleton<ShortSegmentManager> {
private:
	char *_shortsStart;
	char *_shortsEnd;

public:
	char *getShortsStart() {
		return _shortsStart;
	}
	bool inGeneralSegment(char *addr) {
		return ((char *)addr >= _shortsStart && (char *)addr < _shortsEnd);
	}

	class Segment {
	private:
		friend class ShortSegmentManager;
		Segment(char *start, int size, char *origAddr) : _startAddress(start), _size(size), _origAddress(origAddr) {}
		~Segment() {}
		char *_startAddress;		// Start of shorts segment in memory
		int  _size;					// Size of shorts segment
		char *_origAddress;			// Original address this segment was supposed to be at
	public:
		char *getStart() {
			return _startAddress;
		}
		char *getEnd() {
			return (_startAddress + _size);
		}
		Elf32_Addr getOffset() {
			return (Elf32_Addr)(_startAddress - _origAddress);
		}
		bool inSegment(char *addr) {
			return ((char *)addr >= _startAddress && (char *)addr <= _startAddress + _size);
		}
	};

	Segment *newSegment(int size, char *origAddr);
	void deleteSegment(Segment *);

private:
	ShortSegmentManager();
	friend class Common::Singleton<ShortSegmentManager>;
	Common::List<Segment *> _list;
	char *_highestAddress;
};

#endif /* SHORTS_SEGMENT_MANAGER_H */
