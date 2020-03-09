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
#include "ultima/ultima8/kernel/segmented_pool.h"

namespace Ultima {
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

SegmentedPool::SegmentedPool(size_t nodeCapacity_, uint32 nodes)
	: Pool(), _nodes(nodes), _freeNodeCount(nodes) {
	uint32 i;

	// Give it its real capacity.
	// One redzone is added after the memory block.
	_nodeCapacity = OFFSET_ALIGN(nodeCapacity_ + redzoneSize);
	_nodes = nodes;

	// Node offsets are aligned to the next uintptr offset after the real size.
	// Another redzone is added between the node and the memory block.
	_nodeOffset = OFFSET_ALIGN(sizeof(SegmentedPoolNode) + redzoneSize) + _nodeCapacity;

	_startOfPool = new uint8[_nodeOffset * nodes];
	_endOfPool = _startOfPool + (_nodeOffset * nodes);

	VALGRIND_CREATE_MEMPOOL(_startOfPool, redzoneSize, 0);

	_firstFree = reinterpret_cast<SegmentedPoolNode *>(_startOfPool);
	_firstFree->pool = this;
	_firstFree->size = 0;

	_lastFree = _firstFree;

	for (i = 1; i < nodes; ++i) {
		_lastFree->nextFree = reinterpret_cast<SegmentedPoolNode *>(_startOfPool + i * _nodeOffset);
		_lastFree = _lastFree->nextFree;

		_lastFree->pool = this;
		_lastFree->size = 0;
	}

	_lastFree->nextFree = 0;
}

SegmentedPool::~SegmentedPool() {
	assert(isEmpty());

	VALGRIND_DESTROY_MEMPOOL(_startOfPool);

	delete [] _startOfPool;
}

void *SegmentedPool::allocate(size_t size) {
	SegmentedPoolNode *node;

	if (isFull() || size > _nodeCapacity)
		return nullptr;

	--_freeNodeCount;
	node = _firstFree;
	node->size = size;

	if (isFull()) {
		_firstFree = nullptr;
		_lastFree = nullptr;
	} else {
		_firstFree = _firstFree->nextFree;
	}

	node->nextFree = nullptr;

//	debugN"Allocating Node 0x%08X\n", node);
	uint8 *p = reinterpret_cast<uint8 *>(node) +
	           OFFSET_ALIGN(sizeof(SegmentedPoolNode) + redzoneSize);

	VALGRIND_MEMPOOL_ALLOC(_startOfPool, p, size);
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

		VALGRIND_MEMPOOL_FREE(_startOfPool, ptr);
		VALGRIND_DISCARD(node->valgrind_handle);

//	debugN"Free Node 0x%08X\n", node);
		if (isFull()) {
			_firstFree = node;
			_lastFree = node;
		} else {
			_lastFree->nextFree = node;
			_lastFree = _lastFree->nextFree;
		}
		++_freeNodeCount;
	}
}

void SegmentedPool::printInfo() const {
	uint16 i;
	uint32 max, min, total;
	SegmentedPoolNode *node;

	debug(MM_INFO, "start address 0x%p\tend address 0x%p\tnodeOffset 0x%x",
		_startOfPool, _endOfPool, (uint32)_nodeOffset);
	debug(MM_INFO, "_nodeCapacity %u b\n   total _nodes %u\tfree _nodes %u",
		(uint32)_nodeCapacity, _nodes, _freeNodeCount);
	debug(MM_INFO, "total memory: %u\tfree memory: %u",
		(uint32)(_nodeCapacity * _nodes), (uint32)(_nodeCapacity * _freeNodeCount));

	max = 0;
	min = _nodeCapacity;
	total = 0;

	for (i = 0; i < _nodes; ++i) {
		node = reinterpret_cast<SegmentedPoolNode *>(_startOfPool + i * _nodeOffset);
		if (node->size > 0) {
			max = node->size > max ? node->size : max;
			min = node->size < min ? node->size : min;
			total += node->size;
		}
	}

	if (_nodes > _freeNodeCount) {
		debug(MM_INFO, "smallest node: %u b\tlargest node: %u b\taverage size: %u b",
			min, max, total / (_nodes - _freeNodeCount));
	} else {
		debug(MM_INFO, "Empty pool!!!");
	}
}

SegmentedPoolNode *SegmentedPool::getPoolNode(void *ptr) {
	uint32 pos = (reinterpret_cast<uint8 *>(ptr) - _startOfPool) / _nodeOffset;
	return reinterpret_cast<SegmentedPoolNode *>(_startOfPool + pos * _nodeOffset);
}

} // End of namespace Ultima8
} // End of namespace Ultima
