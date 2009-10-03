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

/*
 Each node contains handles to next and previous node and an optional key for searching
 Head node contains handles to first and last node
 */

namespace Sci {

typedef uint16 HEAPHANDLE;

class DblList {
public:
	DblList();
	DblList(HEAPHANDLE heap);
	~DblList(void);
protected:
	HEAPHANDLE _hFirst, _hLast;
public:
	// Add a new node to front of the list
	HEAPHANDLE AddToFront(HEAPHANDLE node, uint16 key = 0);
	HEAPHANDLE AddToEnd(HEAPHANDLE node, uint16 key = 0);
	HEAPHANDLE MoveToEnd(HEAPHANDLE node);
	HEAPHANDLE MoveToFront(HEAPHANDLE node);
	HEAPHANDLE AddAfter(HEAPHANDLE ref, HEAPHANDLE node, uint16 key = 0);
	HEAPHANDLE AddBefore(HEAPHANDLE ref, HEAPHANDLE node, uint16 key = 0);

	HEAPHANDLE FindKey(uint16 key);
	HEAPHANDLE DeleteKey(uint16 key);
	byte DeleteNode(HEAPHANDLE node);
	void DeleteList();
	void Dump(char*caption = ""); // for debug
	HEAPHANDLE getFirst() {
		return _hFirst;
	}
	HEAPHANDLE getLast() {
		return _hLast;
	}
	void toHeap(HEAPHANDLE heap);
	bool isEmpty() {
		return (_hFirst == 0 && _hLast == 0);
	}
	uint16 getSize();
	void set(HEAPHANDLE first, HEAPHANDLE last){
		_hFirst = first;
		_hLast = last;
	}

};

} // end of namespace
