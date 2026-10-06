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

#ifndef FREESCAPE_LANGUAGE_EXECUTION_H
#define FREESCAPE_LANGUAGE_EXECUTION_H

#include "freescape/language/instruction.h"

namespace Freescape {

// Default redraw cadence in 50 Hz ticks.
static const uint32 kFCLRedrawTicks = 8;

enum FCLExecutionResult {
	kFCLFinished,
	kFCLYielded, // Resume this script after the other scripts in the frame.
	kFCLPaused   // Resume this script before advancing the frame.
};

struct FCLExecutionFrame {
	const FCLInstructionVector *code;
	uint32 ip = 0;
	explicit FCLExecutionFrame(const FCLInstructionVector *instructions = nullptr) : code(instructions) {}
};

struct FCLPredicateState {
	bool value, previousValue;
	Token::Type operation = Token::UNKNOWN;
	explicit FCLPredicateState(bool initialValue = false) : value(initialValue), previousValue(initialValue) {}

	void combine(Token::Type op) {
		previousValue = value;
		operation = op;
	}

	void set(bool result) {
		if (operation == Token::AND)
			result = previousValue && result;
		else if (operation == Token::OR)
			result = previousValue || result;
		value = result;
		operation = Token::UNKNOWN;
	}
};

} // namespace Freescape

#endif
