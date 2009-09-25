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

#ifndef SCI_ENGINE_VM_TYPES_H
#define SCI_ENGINE_VM_TYPES_H

#include "common/scummsys.h"

namespace Sci {

// Segment ID type
typedef uint16 SegmentId;

struct reg_t {
	SegmentId segment;
	uint16 offset;

	bool isNull() const {
		return !(offset || segment);
	}

	bool operator==(const reg_t &x) const {
		return (offset == x.offset) && (segment == x.segment);
	}

	bool operator!=(const reg_t &x) const {
		return (offset != x.offset) || (segment != x.segment);
	}

	uint16 toUint16() const {
		return offset;
	}

	int16 toSint16() const {
		return (int16) offset;
	}
};

#define PRINT_REG(r) (0xffff) & (unsigned) (r).segment, (unsigned) (r).offset

// Stack pointer type
typedef reg_t *StackPtr;

// Selector ID
typedef int Selector;
#define NULL_SELECTOR -1

// Offset sent to indicate an error, or that an operation has finished
// (depending on the case)
#define SIGNAL_OFFSET 0xffff

#define PRINT_STK(v) (unsigned) (v - s->stack_base)

static inline reg_t make_reg(SegmentId segment, uint16 offset) {
	reg_t r;
	r.offset = offset;
	r.segment = segment;
	return r;
}

extern reg_t NULL_REG;

} // End of namespace Sci

#endif // SCI_ENGINE_VM_TYPES_H
