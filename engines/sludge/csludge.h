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

#ifndef SLUDGE_CSLUDGE_H
#define SLUDGE_CSLUDGE_H

namespace Sludge {

enum SludgeCommand {
	SLU_UNKNOWN,
	SLU_RETURN,
	SLU_BRANCH,
	SLU_BR_ZERO,
	SLU_SET_GLOBAL,
	SLU_SET_LOCAL,
	SLU_LOAD_GLOBAL,
	SLU_LOAD_LOCAL,
	SLU_PLUS,
	SLU_MINUS,
	SLU_MULT,
	SLU_DIVIDE,
	SLU_AND,
	SLU_OR,
	SLU_EQUALS,
	SLU_NOT_EQ,
	SLU_MODULUS,
	SLU_LOAD_VALUE,
	SLU_LOAD_BUILT,
	SLU_LOAD_FUNC,
	SLU_CALLIT,
	SLU_LOAD_STRING,
	SLU_LOAD_FILE, /*SLU_LOAD_SCENE,*/
	SLU_LOAD_OBJTYPE,
	SLU_NOT,
	SLU_LOAD_NULL,
	SLU_STACK_PUSH,
	SLU_LESSTHAN,
	SLU_MORETHAN,
	SLU_NEGATIVE,
	SLU_UNREG,
	SLU_LESS_EQUAL,
	SLU_MORE_EQUAL,
	SLU_INCREMENT_LOCAL,
	SLU_DECREMENT_LOCAL,
	SLU_INCREMENT_GLOBAL,
	SLU_DECREMENT_GLOBAL,
	SLU_INDEXSET,
	SLU_INDEXGET,
	SLU_INCREMENT_INDEX,
	SLU_DECREMENT_INDEX,
	SLU_QUICK_PUSH,
	numSludgeCommands
};

} // End of namespace Sludge

#endif
