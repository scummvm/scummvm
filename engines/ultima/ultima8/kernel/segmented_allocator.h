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

#ifndef ULTIMA8_KERNEL_SEGMENTED_ALLOCATOR_H
#define ULTIMA8_KERNEL_SEGMENTED_ALLOCATOR_H

#include "ultima/ultima8/kernel/allocator.h"
#include "ultima/ultima8/kernel/segmented_pool.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class SegmentedAllocator: public Allocator {
public:
	SegmentedAllocator(size_t nodeCapacity, uint32 nodes);
	virtual ~SegmentedAllocator();

	ENABLE_RUNTIME_CLASSTYPE()

	virtual void *allocate(size_t size);

	virtual Pool *findPool(void *ptr);

	virtual void freeResources();

	virtual size_t getCapacity() {
		return nodeCapacity;
	}

	void printInfo();

private:
	std::vector<SegmentedPool *> pools;

	size_t nodeCapacity;
	uint32 nodes;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
