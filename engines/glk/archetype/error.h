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

#ifndef ARCHETYPE_ERROR
#define ARCHETYPE_ERROR

/* Writes out all kinds of compile-time errors.Does not perform a halt;
 * expects the program itself to "unravel" the process
 */

#include "glk/archetype/misc.h"

namespace Glk {
namespace Archetype {

extern void hit_eof(progfile &f, AclType expecting, int specific);
extern void expected(progfile &f, AclType expect_ttype, int expect_specific);
extern void expect_general(progfile &f, const String &general_desc);
extern void error_message(progfile &f, const String &message);

/**
 * Used when a particular token is insisted upon by the syntax, usually
 * for readability.It will be an error for the token not to exist.
 */
extern bool insist_on(progfile &f, AclType some_type, int some_number);

} // End of namespace Archetype
} // End of namespace Glk

#endif
