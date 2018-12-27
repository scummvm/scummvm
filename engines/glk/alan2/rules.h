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

#ifndef GLK_ALAN2_RULES
#define GLK_ALAN2_RULES

#include "common/stack.h"
#include "glk/alan2/alan2.h"
#include "glk/alan2/types.h"

namespace Glk {
namespace Alan2 {

class Interpreter;

class Rules {
public:
	Rules(RulElem *rules, Alan2Stack *stack, Interpreter *interpreter) : _ruls(rules), _stack(stack), _interpreter(interpreter) {}
	void parseRules();

private:
	bool endOfTable(RulElem *addr);

	RulElem *_ruls;
	Alan2Stack *_stack;
	Interpreter *_interpreter;
};

} // End of namespace Alan2
} // Engine of namespace GLK

#endif
