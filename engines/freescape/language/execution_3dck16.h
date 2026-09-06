/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FREESCAPE_LANGUAGE_EXECUTION_3DCK16_H
#define FREESCAPE_LANGUAGE_EXECUTION_3DCK16_H

#include "common/hashmap.h"
#include "freescape/language/execution.h"

namespace Freescape {

struct FCLKit16Loop {
	uint32 start = 0;
	uint16 remaining = 0;
};

// Each condition or animator keeps its own continuation and predicate.
struct FCLKit16ExecutionState : FCLExecutionFrame {
	const FCLInstructionVector *source = nullptr;
	uint32 restart = 0;
	Common::HashMap<uint32, FCLKit16Loop> loops;
	bool running = false;
	FCLPredicateState predicate = FCLPredicateState(true);
};

} // namespace Freescape

#endif
