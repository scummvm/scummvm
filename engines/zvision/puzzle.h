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
 */

#ifndef ZVISION_PUZZLE_H
#define ZVISION_PUZZLE_H

#include "common/list.h"

namespace ZVision {

class ResultAction;

/** How criteria should be decided */
enum CriteriaOperator {
	EQUAL_TO,
	NOT_EQUAL_TO,
	GREATER_THAN,
	LESS_THAN
};

/** Criteria for a Puzzle result to be fired */
struct Criteria {
	/** The key of a global state */
	uint32 key;
	/**  
	 * What we're comparing the value of the global state against
	 * This can either be a pure value or it can be the key of another global state
	 */
	uint32 argument;
	/** How to do the comparison */
	CriteriaOperator criteriaOperator;
	/** Whether 'argument' is the key of a global state (true) or a pure value (false) */
	bool argumentIsAKey;
};

enum StateFlags {
	ONCE_PER_INST = 0x01,
	DO_ME_NOW = 0x02,
	DISABLED = 0x04
};

class Puzzle {
public:
	Puzzle() {}
	~Puzzle();
	Puzzle(const Puzzle &other);

	uint32 key;
	Common::List<Criteria> criteriaList;
	// This has to be list of pointers because ResultAction is abstract
	Common::List<ResultAction *> resultActions;
	byte flags;

	// Used by the ScriptManager to allow unique-ification of _referenceTable
	// The unique-ification is done by sorting, then iterating and removing duplicates
	// The sort uses operator<
	inline bool operator<(const Puzzle &other) const {
		return key < other.key;
	}
};

} // End of namespace ZVision

#endif
