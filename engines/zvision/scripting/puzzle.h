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

#ifndef ZVISION_PUZZLE_H
#define ZVISION_PUZZLE_H

#include "zvision/scripting/actions.h"

#include "common/list.h"
#include "common/ptr.h"

namespace ZVision {

struct Puzzle {
	Puzzle() : key(0), addedBySetState(false) {}

	~Puzzle() {
		for (Common::List<ResultAction *>::iterator iter = resultActions.begin(); iter != resultActions.end(); ++iter) {
			delete *iter;
		}
	}

	/** How criteria should be decided */
	enum CriteriaOperator {
		EQUAL_TO,
		NOT_EQUAL_TO,
		GREATER_THAN,
		LESS_THAN
	};

	/** Criteria for a Puzzle result to be fired */
	struct CriteriaEntry {
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
		DISABLED = 0x02,
		DO_ME_NOW = 0x04
	};

	uint32 key;
	Common::List<Common::List <CriteriaEntry> > criteriaList;
	// This has to be list of pointers because ResultAction is abstract
	Common::List<ResultAction *> resultActions;
	bool addedBySetState;
};

} // End of namespace ZVision

#endif
