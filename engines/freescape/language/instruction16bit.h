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

#ifndef FREESCAPE_INSTRUCTION16BIT_H
#define FREESCAPE_INSTRUCTION16BIT_H

#include "common/hashmap.h"
#include "freescape/language/instruction.h"

namespace Freescape {

struct FCLLoop {
	uint32 start = 0;
	uint16 remaining = 0;
};

struct FCLExecutionState {
	const FCLInstructionVector *source = nullptr, *code = nullptr;
	uint32 ip = 0, restart = 0;
	Common::HashMap<uint32, FCLLoop> loops;
	bool running = false;
	bool predicate = true, previousPredicate = true;
	Token::Type booleanOp = Token::UNKNOWN;
};

enum FCLExecutionResult { kFCLFinished, kFCLYielded, kFCLPaused };

} // namespace Freescape

#endif
