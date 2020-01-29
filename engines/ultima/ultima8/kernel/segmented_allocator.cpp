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

SegmentedAllocator::SegmentedAllocator(size_t nodeCapacity_, uint32 nodes_): Allocator(), nodes(nodes_) {
	pools.push_back(new SegmentedPool(nodeCapacity_, nodes_));
	nodeCapacity = pools[0]->getNodeCapacity();
//	pout << "Initial Pool Created: Nodes - " << nodes << ", Node Capacity - "
//		<< nodeCapacity << Std::endl;
}

SegmentedAllocator::~SegmentedAllocator() {
	Std::vector<SegmentedPool *>::iterator i;
	for (i = pools.begin(); i != pools.end(); ++i) {
		delete *i;
	}

	pools.clear();
}

void *SegmentedAllocator::allocate(size_t size) {
	Std::vector<SegmentedPool *>::iterator i;
	SegmentedPool *p;

	if (size > nodeCapacity)
		return 0;

	for (i = pools.begin(); i != pools.end(); ++i) {
		if (!(*i)->isFull())
			return (*i)->allocate(size);
	}

	// else we need a new pool
	p = new SegmentedPool(nodeCapacity, nodes);
	if (p) {
//		pout << "New Pool Created: Nodes - " << nodes << ", Node Capacity - "
//			<< nodeCapacity << Std::endl;

		pools.push_back(p);
		return p->allocate(size);
	}

	// fail
	return 0;
}

Pool *SegmentedAllocator::findPool(void *ptr) {
	Std::vector<SegmentedPool *>::iterator i;
	for (i = pools.begin(); i != pools.end(); ++i) {
		if ((*i)->inPool(ptr))
			return *i;
	}
	return 0;
}

void SegmentedAllocator::freeResources() {
	if (pools.empty())
		return;

	// Pop back only -- it should suffice.
	while (pools.back()->isEmpty()) {
		delete pools.back();
		pools.pop_back();

		if (pools.empty())
			return;
	}
}

void SegmentedAllocator::printInfo() {
	Std::vector<SegmentedPool *>::iterator it;
	int i = 0;

	pout << "Pools: " <<  pools.size() << Std::endl;
	for (it = pools.begin(); it != pools.end(); ++it) {
		pout << "  Pool " << i++ << ":" << Std::endl;
		(*it)->printInfo();
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
