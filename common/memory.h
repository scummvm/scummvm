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
 */

#ifndef COMMON_MEMORY_H
#define COMMON_MEMORY_H

#include "common/scummsys.h"

#ifdef USE_LIBSTDCPP_REPLACEMENT
/**
 * Placement new (taken from <new>) for systems which do not offer that header.
 */
inline void *operator new(size_t, void *p) { return p; }
#else
#include <new>
#endif

namespace Common {

/**
 * A simple allocator in the spirit of std::allocator.
 */
template<class T>
class Allocator {
public:
	typedef T ValueType;

	typedef size_t SizeType;

	typedef T *Pointer;
	typedef const T *ConstPointer;

	/**
	 * Allocate memory for n objects of type T.
	 *
	 * This does *not* initialize the memory.
	 */
	Pointer allocate(SizeType n) {
		return (Pointer)malloc(sizeof(T) * n);
	}

	/**
	 * Free the memory of n objects.
	 *
	 * This does *not* destroy the objects.
	 */
	void deallocate(Pointer p, SizeType n) {
		free(p);
	}

	/**
	 * Construct one object T at the location p with the value value.
	 */
	void construct(Pointer p, const T &value) { new (p) T(value); }

	/**
	 * Destroy one object at location p.
	 */
	void destroy(Pointer p) { p->~T(); }

	/**
	 * Try to free up as much space as possible.
	 */
	void freeReservedMemory() {}

	/**
	 * Create an allocator for another type U.
	 */
	template<class U>
	struct Rebind {
		typedef Allocator<U> Other;
	};
};

/**
 * Copies data from the range [first, last) to [dst, dst + (last - first)).
 * It requires the range [dst, dst + (last - first)) to be valid.
 * It also requires dst not to be in the range [first, last).
 *
 * Unlike copy this works on uninitialized dst memory!
 */
template<class In, class Out>
Out *uninitialized_copy(In first, In last, Out *dst) {
	while (first != last)
		new ((void *)dst++) Out(*first++);
	return dst;
}

} // End of namespace Common

#endif
