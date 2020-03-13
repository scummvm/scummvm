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
#include "ultima/ultima8/kernel/segmented_allocator.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SegmentedAllocator, Allocator)

SegmentedAllocator::SegmentedAllocator(size_t nodeCapacity, uint32 nodes): Allocator(), _nodes(nodes) {
	_pools.push_back(new SegmentedPool(nodeCapacity, nodes));
	_nodeCapacity = _pools[0]->getNodeCapacity();
//	pout << "Initial Pool Created: Nodes - " << _nodes << ", Node Capacity - "
//		<< _nodeCapacity << Std::endl;
}

SegmentedAllocator::~SegmentedAllocator() {
	Std::vector<SegmentedPool *>::iterator i;
	for (i = _pools.begin(); i != _pools.end(); ++i) {
		delete *i;
	}

	_pools.clear();
}

void *SegmentedAllocator::allocate(size_t size) {
	Std::vector<SegmentedPool *>::iterator i;
	SegmentedPool *p;

	if (size > _nodeCapacity)
		return nullptr;

	for (i = _pools.begin(); i != _pools.end(); ++i) {
		if (!(*i)->isFull())
			return (*i)->allocate(size);
	}

	// else we need a new pool
	p = new SegmentedPool(_nodeCapacity, _nodes);
	if (p) {
//		pout << "New Pool Created: Nodes - " << _nodes << ", Node Capacity - "
//			<< _nodeCapacity << Std::endl;

		_pools.push_back(p);
		return p->allocate(size);
	}

	// fail
	return nullptr;
}

Pool *SegmentedAllocator::findPool(void *ptr) {
	Std::vector<SegmentedPool *>::iterator i;
	for (i = _pools.begin(); i != _pools.end(); ++i) {
		if ((*i)->inPool(ptr))
			return *i;
	}
	return nullptr;
}

void SegmentedAllocator::freeResources() {
	if (_pools.empty())
		return;

	// Pop back only -- it should suffice.
	while (_pools.back()->isEmpty()) {
		delete _pools.back();
		_pools.pop_back();

		if (_pools.empty())
			return;
	}
}

void SegmentedAllocator::printInfo() {
	Std::vector<SegmentedPool *>::iterator it;
	int i = 0;

	pout << "Pools: " <<  _pools.size() << Std::endl;
	for (it = _pools.begin(); it != _pools.end(); ++it) {
		pout << "  Pool " << i++ << ":" << Std::endl;
		(*it)->printInfo();
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
