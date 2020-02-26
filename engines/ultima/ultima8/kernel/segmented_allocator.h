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
	~SegmentedAllocator() override;

	ENABLE_RUNTIME_CLASSTYPE()

	void *allocate(size_t size) override;

	Pool *findPool(void *ptr) override;

	void freeResources() override;

	size_t getCapacity() override {
		return _nodeCapacity;
	}

	void printInfo() override;

private:
	Std::vector<SegmentedPool *> _pools;

	size_t _nodeCapacity;
	uint32 _nodes;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
