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

#include "ultima8/misc/pent_include.h"
#include "ultima8/kernel/segmented_pool.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SegmentedPool, Pool)

//	Memory is aligned to the next largest multiple of sizeof(x) from
//  the base address plus the size. Although, this may not be very helpful
//  if the base address is not a multiple of sizeof(X).
//	example: sizeof(x) = 0x8, object size = 0xFFE2:
//			0xFFE2 + 0x8 - 1 = 0xFFE9;
//			0xFFE9 & ~(0x8 - 0x1) -> 0xFFE9 & 0xFFF8 = 0xFFE8

#define OFFSET_ALIGN(X) ( (X + sizeof(uintptr) - 1) & ~(sizeof(uintptr) - 1) )

#ifdef USE_VALGRIND
const int redzoneSize = 8;
#else
const int redzoneSize = 0;
#endif

struct SegmentedPoolNode {
	SegmentedPool *pool;
	SegmentedPoolNode *nextFree;
#ifdef USE_VALGRIND
	int valgrind_handle;
#endif
	size_t size;
};


// We pad both the PoolNode and the memory to align it.

SegmentedPool::SegmentedPool(size_t nodeCapacity_, uint32 nodes_)
	: Pool(), nodes(nodes_), freeNodeCount(nodes_) {
	uint32 i;

	// Give it its real capacity.
	// One redzone is added after the memory block.
	nodeCapacity = OFFSET_ALIGN(nodeCapacity_ + redzoneSize);
	nodes = nodes_;

	// Node offsets are aligned to the next uintptr offset after the real size.
	// Another redzone is added between the node and the memory block.
	nodeOffset = OFFSET_ALIGN(sizeof(SegmentedPoolNode) + redzoneSize)
	             + nodeCapacity;

	startOfPool = new uint8[nodeOffset * nodes_];
	endOfPool = startOfPool + (nodeOffset * nodes_);

	VALGRIND_CREATE_MEMPOOL(startOfPool, redzoneSize, 0);

	firstFree = reinterpret_cast<SegmentedPoolNode *>(startOfPool);
	firstFree->pool = this;
	firstFree->size = 0;

	lastFree = firstFree;

	for (i = 1; i < nodes_; ++i) {
		lastFree->nextFree = reinterpret_cast<SegmentedPoolNode *>(startOfPool + i * nodeOffset);
		lastFree = lastFree->nextFree;

		lastFree->pool = this;
		lastFree->size = 0;
	}

	lastFree->nextFree = 0;
}

SegmentedPool::~SegmentedPool() {
	assert(isEmpty());

	VALGRIND_DESTROY_MEMPOOL(startOfPool);

	delete [] startOfPool;
}

void *SegmentedPool::allocate(size_t size) {
	SegmentedPoolNode *node;

	if (isFull() || size > nodeCapacity)
		return 0;

	--freeNodeCount;
	node = firstFree;
	node->size = size;

	if (isFull()) {
		firstFree = 0;
		lastFree = 0;
	} else {
		firstFree = firstFree->nextFree;
	}

	node->nextFree = 0;

//	con.Printf("Allocating Node 0x%08X\n", node);
	uint8 *p = reinterpret_cast<uint8 *>(node) +
	           OFFSET_ALIGN(sizeof(SegmentedPoolNode) + redzoneSize);

	VALGRIND_MEMPOOL_ALLOC(startOfPool, p, size);
#ifdef USE_VALGRIND
	node->valgrind_handle = VALGRIND_CREATE_BLOCK(p, size,
	                        "SegmentedPoolBlock");
#endif

	return p;
}

void SegmentedPool::deallocate(void *ptr) {
	SegmentedPoolNode *node;

	if (inPool(ptr)) {
		node = getPoolNode(ptr);
		node->size = 0;
		assert(node->pool == this);

		VALGRIND_MEMPOOL_FREE(startOfPool, ptr);
		VALGRIND_DISCARD(node->valgrind_handle);

//	con.Printf("Free Node 0x%08X\n", node);
		if (isFull()) {
			firstFree = node;
			lastFree = node;
		} else {
			lastFree->nextFree = node;
			lastFree = lastFree->nextFree;
		}
		++freeNodeCount;
	}
}

void SegmentedPool::printInfo() {
	uint16 i;
	size_t max, min, total;
	SegmentedPoolNode *node;

	con.Printf("   start address 0x%X\tend address 0x%X\tnodeOffset 0x%X\n",
	           startOfPool, endOfPool, nodeOffset);
	con.Printf("   nodeCapacity %d b\n   total nodes %d\tfree nodes %d\n",
	           nodeCapacity, nodes, freeNodeCount);
	con.Printf("   total memory: %d\tfree memory: %d\n",
	           nodeCapacity * nodes, nodeCapacity * freeNodeCount);

	max = 0;
	min = nodeCapacity;
	total = 0;

	for (i = 0; i < nodes; ++i) {
		node = reinterpret_cast<SegmentedPoolNode *>(startOfPool + i * nodeOffset);
		if (node->size > 0) {
			max = node->size > max ? node->size : max;
			min = node->size < min ? node->size : min;
			total += node->size;
		}
	}

	if (nodes > freeNodeCount) {
		con.Printf("   smallest node: %d b\tlargest node: %d b\taverage size: %d b\n",
		           min, max, total / (nodes - freeNodeCount));
	} else {
		con.Printf("   Empty pool!!!\n");
	}
}

SegmentedPoolNode *SegmentedPool::getPoolNode(void *ptr) {
	uint32 pos = (reinterpret_cast<uint8 *>(ptr) - startOfPool) / nodeOffset;
	return reinterpret_cast<SegmentedPoolNode *>(startOfPool + pos * nodeOffset);
}

} // End of namespace Ultima8
