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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/vm_types.h"
#include "sci/engine/workarounds.h"

namespace Sci {

extern const char *opcodeNames[]; // from scriptdebug.cpp

reg_t reg_t::lookForWorkaround(const reg_t right) const {
	SciTrackOriginReply originReply;
	SciWorkaroundSolution solution = trackOriginAndFindWorkaround(0, arithmeticWorkarounds, &originReply);
	if (solution.type == WORKAROUND_NONE)
		error("arithmetic operation on non-integer (%04x:%04x, %04x:%04x) from method %s::%s (script %d, room %d, localCall %x)", 
		PRINT_REG(*this), PRINT_REG(right), originReply.objectName.c_str(), 
		originReply.methodName.c_str(), originReply.scriptNr, g_sci->getEngineState()->currentRoomNumber(),
		originReply.localCallOffset);
	assert(solution.type == WORKAROUND_FAKE);
	return make_reg(0, solution.value);
}

reg_t reg_t::operator+(const reg_t right) const {
	if (isPointer()) {
		// Pointer arithmetics. Only some pointer types make sense here
		SegmentObj *mobj = g_sci->getEngineState()->_segMan->getSegmentObj(segment);

		if (!mobj)
			error("[VM]: Attempt to add %d to invalid pointer %04x:%04x", right.offset, PRINT_REG(*this));

		switch (mobj->getType()) {
		case SEG_TYPE_LOCALS:
		case SEG_TYPE_SCRIPT:
		case SEG_TYPE_STACK:
		case SEG_TYPE_DYNMEM:
			// Make sure that we are adding an offset to the pointer
			if (!right.isNumber())
				return lookForWorkaround(right);
			return make_reg(segment, offset + right.toSint16());
		default:
			return lookForWorkaround(right);
		}	
	} else if (isNumber() && right.isPointer()) {
		// Adding a pointer to a number, flip the order
		return right + *this;
	} else {
		// Normal arithmetics. Make sure we're adding a number
		if (right.isPointer())
			return lookForWorkaround(right);
		// If the current variable is uninitialized, it'll be set
		// to zero in order to perform the operation. Such a case
		// happens in SQ1, room 28, when throwing the water at Orat.
		if (!isInitialized())
			return make_reg(0, right.toSint16());
		else if (!right.isInitialized())
			return *this;
		else
			return make_reg(0, toSint16() + right.toSint16());
	}
}

reg_t reg_t::operator-(const reg_t right) const {
	if (segment == right.segment) {
		// We can subtract numbers, or pointers with the same segment,
		// an operation which will yield a number like in C
		return make_reg(0, toSint16() - right.toSint16());
	} else {
		return *this + make_reg(right.segment, -right.offset);
	}
}

reg_t reg_t::operator*(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toSint16() * right.toSint16());
	else if (!isInitialized() || !right.isInitialized())
		return NULL_REG;	// unitialized variables - always return 0
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator/(const reg_t right) const {
	if (isNumber() && right.isNumber()) {
		if (right.isNull())
			return NULL_REG;	// division by zero
		else
			return make_reg(0, toSint16() / right.toSint16());
	} else
		return lookForWorkaround(right);
}

reg_t reg_t::operator%(const reg_t right) const {
	if (isNumber() && right.isNumber()) {
		// Support for negative numbers was added in Iceman, and perhaps in 
		// SCI0 0.000.685 and later. Theoretically, this wasn't really used
		// in SCI0, so the result is probably unpredictable. Such a case 
		// would indicate either a script bug, or a modulo on an unsigned
		// integer larger than 32767. In any case, such a case should be
		// investigated, instead of being silently accepted.
		if (getSciVersion() <= SCI_VERSION_0_LATE && (toSint16() < 0 || right.toSint16() < 0))
			warning("Modulo of a negative number has been requested for SCI0. This *could* lead to issues");
		int16 value = toSint16();
		int16 modulo = ABS(right.toSint16());
		int16 result = (modulo != 0 ? value % modulo : 0);
		if (result < 0)
			result += modulo;
		return make_reg(0, result);
	} else
		return lookForWorkaround(right);
}

reg_t reg_t::operator>>(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() >> right.toUint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator<<(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() << right.toUint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator+(int16 right) const {
	return *this + make_reg(0, right);
}

reg_t reg_t::operator-(int16 right) const {
	return *this - make_reg(0, right);
}

uint16 reg_t::requireUint16() const {
	if (isNumber())
		return toUint16();
	else
		return lookForWorkaround(NULL_REG).toUint16();
}

int16 reg_t::requireSint16() const {
	if (isNumber())
		return toSint16();
	else
		return lookForWorkaround(NULL_REG).toSint16();
}

reg_t reg_t::operator&(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() & right.toUint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator|(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() | right.toUint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator^(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() ^ right.toUint16());
	else
		return lookForWorkaround(right);
}

bool reg_t::operator>(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return toSint16() > right.toSint16();
	else if (isPointer() && segment == right.segment)
		return toUint16() > right.toUint16();	// pointer comparison
	else if (pointerComparisonWithInteger(right))
		return true;
	else if (right.pointerComparisonWithInteger(*this))
		return false;
	else
		return lookForWorkaround(right).toSint16();
}

bool reg_t::operator<(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return toSint16() < right.toSint16();
	else if (isPointer() && segment == right.segment)
		return toUint16() < right.toUint16();	// pointer comparison
	else if (pointerComparisonWithInteger(right))
		return false;
	else if (right.pointerComparisonWithInteger(*this))
		return true;
	else
		return lookForWorkaround(right).toSint16();
}

bool reg_t::gtU(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return toUint16() > right.toUint16();
	else if (isPointer() && segment == right.segment)
		return toUint16() > right.toUint16();	// pointer comparison
	else if (pointerComparisonWithInteger(right))
		return true;
	else if (right.pointerComparisonWithInteger(*this))
		return false;
	else
		return lookForWorkaround(right).toSint16();
}

bool reg_t::ltU(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return toUint16() < right.toUint16();
	else if (isPointer() && segment == right.segment)
		return toUint16() < right.toUint16();	// pointer comparison
	else if (pointerComparisonWithInteger(right))
		return false;
	else if (right.pointerComparisonWithInteger(*this))
		return true;
	else
		return lookForWorkaround(right).toSint16();
}

bool reg_t::pointerComparisonWithInteger(const reg_t right) const {
	// SCI0 - SCI1.1 scripts use this to check whether a parameter is a pointer
	// or a far text reference. It is used e.g. by the standard library Print
	// function to distinguish two ways of calling it:
	//
	// (Print "foo") // Pointer to a string
	// (Print 420 5) // Reference to the fifth message in text resource 420
	// It works because in those games, the maximum resource number is 999,
	// so any parameter value above that threshold must be a pointer.
	// PQ2 japanese compares pointers to 2000 to find out if its a pointer
	// or a resource ID.
	// There are cases where game scripts check for arbitrary numbers against
	// pointers, e.g.:
	// Hoyle 3, Pachisi, when any opponent is about to talk
	// SQ1, room 28, when throwing water at the Orat
	// SQ1, room 58, when giving the ID card to the robot
	// QFG3, room 440, when talking to Uhura
	// Thus we check for all integers <= 2000
	return (isPointer() && right.isNumber() && right.offset <= 2000 && getSciVersion() <= SCI_VERSION_1_1);
}

} // End of namespace Sci
