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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_PRIQUEUE_H
#define SAGA2_PRIQUEUE_H

namespace Saga2 {

template <class ITEM, int size>
class PriorityQueue {
	int16           tail;                   // end index of queue
	ITEM            queue[size + 1];

	static int16 parentIndex(int16 index) {
		return index >> 1;
	}
	static int16 child1Index(int16 index) {
		return index << 1;
	}
	static int16 child2Index(int16 index) {
		return (index >> 1) + 1;
	}

public:
	PriorityQueue() {                    // constructor
		tail = 1;
	}

	bool insert(ITEM &newItem);              // insert an item
	bool remove(ITEM &result);           // remove an item
	void clear() {
		tail = 1;    // clear the queue
	}
	int16 getCount() {
		return tail - 1;
	}
};

//  Function to insert an element into the queue

template <class ITEM, int size>
bool PriorityQueue<ITEM, size>::insert(ITEM &newItem) {
	int16           index,
	                parentIndex,
	                newVal = (int)newItem;
	ITEM            *qi,
	                *parentItem;

	if (tail >= size + 1) return false;

	for (index = tail, qi = &queue[index];
	        index > 1;
	        index = parentIndex, qi = parentItem) {
		parentIndex = PriorityQueue::parentIndex(index);
		parentItem = &queue[parentIndex];

		if ((int)*parentItem <= newVal) break;
		*qi = *parentItem;
	}
	*qi = newItem;
	tail++;

	return true;
}

//  Function to remove the lowest element from the queue

template <class ITEM, int size>
bool PriorityQueue<ITEM, size>::remove(ITEM &result) {
	ITEM            *item = &queue[1],
	                 *child;
	int16           itemNum = 1,
	                childNum,
	                tailVal;

	if (tail <= 1) return false;

	result = *item;
	tail--;
	tailVal = (int)queue[tail];

	for (;;) {
		childNum = child1Index(itemNum);
		if (childNum >= tail) break;

		child = &queue[childNum];

		//  Select the lowest of the two children
		if (childNum + 1 < tail
		        && (int)child[0] > (int)child[1]) {
			childNum++;
			child++;
		}

		if ((int)*child >= tailVal) break;
		*item = *child;
		item = child;
		itemNum = childNum;
	}

	if (itemNum != tail) {
		*item = queue[tail];
	}
	return true;
}

} // end of namepsace Saga2

#endif
