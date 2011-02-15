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

	uint16 requireUint16() const {
		if (isNumber())
			return toUint16();
		else
			// The results of this are likely unpredictable... It most likely
			// means that a kernel function is returning something wrong. If
			// such an error occurs, we usually need to find the last kernel
			// function called and check its return value.
			error("[VM] Attempt to read unsigned arithmetic value from non-zero segment %04x. Offset: %04x", segment, offset);
	}

	int16 requireSint16() const {
		if (isNumber())
			return toSint16();
		else
			// The results of this are likely unpredictable... It most likely
			// means that a kernel function is returning something wrong. If
			// such an error occurs, we usually need to find the last kernel
			// function called and check its return value.
			error("[VM] Attempt to read signed arithmetic value from non-zero segment %04x. Offset: %04x", segment, offset);
	}

	bool isNumber() const {
		return !segment;
	}
};

static inline reg_t make_reg(SegmentId segment, uint16 offset) {
	reg_t r;
	r.offset = offset;
	r.segment = segment;
	return r;
}

#define PRINT_REG(r) (0xffff) & (unsigned) (r).segment, (unsigned) (r).offset

// Stack pointer type
typedef reg_t *StackPtr;

enum {
	/**
	 * Special reg_t 'offset' used to indicate an error, or that an operation has
	 * finished (depending on the case).
	 * @see SIGNAL_REG
	 */
	SIGNAL_OFFSET = 0xffff
};

extern const reg_t NULL_REG;
extern const reg_t SIGNAL_REG;
extern const reg_t TRUE_REG;

// Selector ID
typedef int Selector;

enum {
	/** Special 'selector' value, used when calling add_exec_stack_entry. */
	NULL_SELECTOR = -1
};


} // End of namespace Sci

#endif // SCI_ENGINE_VM_TYPES_H
