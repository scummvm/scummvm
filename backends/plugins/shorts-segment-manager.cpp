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

#if defined(DYNAMIC_MODULES) //TODO: && defined (MIPS target)

#include "MIPS-loader.h"
#include "shorts-segment-manager.h"

DECLARE_SINGLETON(ShortSegmentManager);	// For singleton

ShortSegmentManager::ShortSegmentManager() {
	_shortsStart = &__plugin_hole_start ;
	_shortsEnd = &__plugin_hole_end;
}

ShortSegmentManager::Segment *ShortSegmentManager::newSegment(int size, char *origAddr) {
	char *lastAddress = origAddr;
	Common::List<Segment *>::iterator i;

	// Find a block that fits, starting from the beginning
	for (i = _list.begin(); i != _list.end(); ++i) {
		char *currAddress = (*i)->getStart();

		if ((int)(currAddress - lastAddress) >= size) break;

		lastAddress = (*i)->getEnd();
	}

	if ((Elf32_Addr)lastAddress & 3)
		lastAddress += 4 - ((Elf32_Addr)lastAddress & 3);	// Round up to multiple of 4

	if (lastAddress + size > _shortsEnd) {
		seterror("Error. No space in shorts segment for %x bytes. Last address is %p, max address is %p.\n",
		         size, lastAddress, _shortsEnd);
		return NULL;
	}

	Segment *seg = new Segment(lastAddress, size, origAddr);	// Create a new segment

	if (lastAddress + size > _highestAddress) _highestAddress = lastAddress + size;	// Keep track of maximum

	_list.insert(i, seg);

	DBG("Shorts segment size %x allocated. End = %p. Remaining space = %x. Highest so far is %p.\n",
	    size, lastAddress + size, _shortsEnd - _list.back()->getEnd(), _highestAddress);

	return seg;
}

void ShortSegmentManager::deleteSegment(ShortSegmentManager::Segment *seg) {
	DBG("Deleting shorts segment from %p to %p.\n\n", seg->getStart(), seg->getEnd());
	_list.remove(seg);
	delete seg;
}

#endif /* DYNAMIC_MODULES */
