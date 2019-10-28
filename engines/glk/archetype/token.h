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

#ifndef ARCHETYPE_TOKEN
#define ARCHETYPE_TOKEN

#include "glk/archetype/misc.h"
#include "common/str.h"

namespace Glk {
namespace Archetype {

/*
extern bool isLiteralType(char c);
extern bool isLetter(char c);
extern bool isDigit(char c);
extern bool isStartChar(char c);
extern bool isIdChar(char c);
extern bool isLongOper(char c);
extern bool isOperChar(char c);
*/

/**
 * State machine which passes out the next token from the file f_in.
 *
 * A token is a constant (including parse words and literal text),
 * a reserved word, or an operator (including the curly braces).
 * @param f		The input file
 * @returns		True if there is a token available, false if the file is empty
 */
extern bool get_token(progfile &f);

/**
 * Given a token type andtoken number, writes out the proper string (without terminating the line).
 * @param the_type		the token type
 * @param the_number	the token number
 */
extern void write_token(AclType the_type, int the_number);

} // End of namespace Archetype
} // End of namespace Glk

#endif
