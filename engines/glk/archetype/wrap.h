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

#ifndef ARCHETYPE_WRAP
#define ARCHETYPE_WRAP

#include "glk/archetype/misc.h"

namespace Glk {
namespace Archetype {

extern int Rows;

/**
 * When we want to wrap a number
 */
extern void wrapint(int i, bool terminate);

/**
 * Given a string, writes it out to screen, making sure that if it exceeds the screen columns,
 * it is broken at natural word boundaries (i.e. white space)
 */
extern void wrapout(const String &s, bool terminate);

/**
 * Used for printing run-time errors.  It will print the error message on
 * a line by itself and pick up the next line at the exact same cursor position.
 */
extern void wraperr(const String &s);

/**
 * Hides the extra stack space necessary for performing a readln() so that
 * it won't affect eval_expr
 */
extern StringPtr ReadLine(bool full_line);

/**
 * Used for directly resetting the cursor position by means other than
 * physically wrapping it around
 */
extern void cursor_reset();

} // End of namespace Archetype
} // End of namespace Glk

#endif
