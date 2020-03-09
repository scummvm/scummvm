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

#ifndef ULTIMA8_KERNEL_SEGMENTED_POOL_H
#define ULTIMA8_KERNEL_SEGMENTED_POOL_H

#include "ultima/ultima8/kernel/pool.h"

namespace Ultima {
namespace Ultima8 {

// Think about adding magic to the front of the
// PoolNode to identify them.

class SegmentedPool;

struct SegmentedPoolNode;

/**
 * A pool with memory broken into even length segments.
 * SegmentedPool only allocate memory one segment at a time.
 * If the requested memory is larger than a segment, allocation will fail
 * and return nullptr.
 */
class SegmentedPool: public Pool {
public:
	SegmentedPool(size_t nodeCapacity, uint32 nodes);
	~SegmentedPool() override;

	ENABLE_RUNTIME_CLASSTYPE()

	void *allocate(size_t size) override;
	void deallocate(void *ptr) override;

	bool isFull() const override {
		return _freeNodeCount == 0;
	}
	bool isEmpty() const override {
		return _freeNodeCount == _nodes;
	}

	bool inPool(void *ptr) const override {
		return (ptr > _startOfPool && ptr < _endOfPool);
	}

	void printInfo() const override;

	size_t getNodeCapacity() const {
		return _nodeCapacity;
	}

	SegmentedPoolNode *getPoolNode(void *ptr);
private:
	uint8 *_startOfPool;
	uint8 *_endOfPool;

	size_t _nodeOffset;
	size_t _nodeCapacity;
	uint32 _nodes;
	uint32 _freeNodeCount;

	SegmentedPoolNode *_firstFree;
	SegmentedPoolNode *_lastFree;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
