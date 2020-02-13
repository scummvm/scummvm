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

#include "glk/archetype/expression.h"

namespace Glk {
namespace Archetype {

bool Right_Assoc[NUM_OPERS + 2];
bool Binary[NUM_OPERS + 2];
int8 Precedence[NUM_OPERS + 2];

void expression_init() {
	Binary[OP_NOP]        = false;
	Binary[OP_LPAREN]     = false;
	Binary[OP_DOT]        = true;
	Binary[OP_CHS]        = false;
	Binary[OP_NUMERIC]    = false;
	Binary[OP_STRING]     = false;
	Binary[OP_RANDOM]     = false;
	Binary[OP_LENGTH]     = false;
	Binary[OP_POWER]      = true;
	Binary[OP_MULTIPLY]   = true;
	Binary[OP_DIVIDE]     = true;
	Binary[OP_PLUS]       = true;
	Binary[OP_MINUS]      = true;
	Binary[OP_CONCAT]     = true;
	Binary[OP_WITHIN]     = true;
	Binary[OP_LEFTFROM]   = true;
	Binary[OP_RIGHTFROM]  = true;
	Binary[OP_EQ]         = true;
	Binary[OP_NE]         = true;
	Binary[OP_GT]         = true;
	Binary[OP_LT]         = true;
	Binary[OP_GE]         = true;
	Binary[OP_LE]         = true;
	Binary[OP_NOT]        = false;
	Binary[OP_AND]        = true;
	Binary[OP_OR]         = true;
	Binary[OP_C_MULTIPLY] = true;
	Binary[OP_C_DIVIDE]   = true;
	Binary[OP_C_PLUS]     = true;
	Binary[OP_C_MINUS]    = true;
	Binary[OP_C_CONCAT]   = true;
	Binary[OP_ASSIGN]     = true;
	Binary[OP_SEND]       = true;
	Binary[OP_PASS]       = true;

	// Initialize the Right_Assoc table as follows : anything unary must be right-associative;
	// all others are assumed left-associative.After the loop, right-associative binary operators
	// are explicity set
	for (int i = 0; i <= NUM_OPERS; ++i)
		Right_Assoc[i] = !Binary[i];

	Right_Assoc[OP_POWER]      = true;
	Right_Assoc[OP_C_MULTIPLY] = true;
	Right_Assoc[OP_C_DIVIDE]   = true;
	Right_Assoc[OP_C_PLUS]     = true;
	Right_Assoc[OP_C_MINUS]    = true;
	Right_Assoc[OP_C_CONCAT]   = true;
	Right_Assoc[OP_ASSIGN]     = true;


	Precedence[OP_LPAREN]     = 14;		// must always be the higest
	Precedence[OP_DOT]        = 13;

	Precedence[OP_CHS]        = 12;
	Precedence[OP_NUMERIC]    = 12;
	Precedence[OP_STRING]     = 12;
	Precedence[OP_RANDOM]     = 12;
	Precedence[OP_LENGTH]     = 12;

	Precedence[OP_POWER]      = 11;

	Precedence[OP_MULTIPLY]   = 10;
	Precedence[OP_DIVIDE]     = 10;

	Precedence[OP_PLUS]       = 9;
	Precedence[OP_MINUS]      = 9;
	Precedence[OP_CONCAT]     = 9;

	Precedence[OP_WITHIN]     = 8;

	Precedence[OP_LEFTFROM]   = 7;
	Precedence[OP_RIGHTFROM]  = 7;

	Precedence[OP_SEND]       = 6;
	Precedence[OP_PASS]       = 6;

	Precedence[OP_EQ]         = 5;
	Precedence[OP_NE]         = 5;
	Precedence[OP_GT]         = 5;
	Precedence[OP_LT]         = 5;
	Precedence[OP_GE]         = 5;
	Precedence[OP_LE]         = 5;

	Precedence[OP_NOT]        = 4;
	Precedence[OP_AND]        = 3;
	Precedence[OP_OR]         = 2;

	Precedence[OP_C_MULTIPLY] = 1;
	Precedence[OP_C_DIVIDE]   = 1;
	Precedence[OP_C_PLUS]     = 1;
	Precedence[OP_C_MINUS]    = 1;
	Precedence[OP_C_CONCAT]   = 1;
	Precedence[OP_ASSIGN]     = 1;
}

} // End of namespace Archetype
} // End of namespace Glk
