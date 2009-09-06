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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_ENGINE_KERNEL_TYPES_H
#define SCI_ENGINE_KERNEL_TYPES_H

#include "sci/engine/vm_types.h"

namespace Sci {

#define KSIG_TERMINATOR 0

// Uncompiled signatures
#define KSIG_SPEC_ARITMETIC 'i'
#define KSIG_SPEC_LIST 'l'
#define KSIG_SPEC_NODE 'n'
#define KSIG_SPEC_OBJECT 'o'
#define KSIG_SPEC_REF 'r' // Said Specs and strings
#define KSIG_SPEC_ARITHMETIC 'i'
#define KSIG_SPEC_NULL 'z'
#define KSIG_SPEC_ANY '.'
#define KSIG_SPEC_ALLOW_INV '!' // Allow invalid pointers
#define KSIG_SPEC_ELLIPSIS '*' // Arbitrarily more TYPED arguments

#define KSIG_SPEC_SUM_DONE ('a' - 'A') // Use small letters to indicate end of sum type
/* Use capital letters for sum types, e.g.
** "LNoLr" for a function which takes two arguments:
** (1) list, node or object
** (2) list or ref
*/

// Compiled signatures
#define KSIG_LIST	0x01
#define KSIG_NODE	0x02
#define KSIG_OBJECT	0x04
#define KSIG_REF	0x08
#define KSIG_ARITHMETIC 0x10

#define KSIG_NULL	0x40
#define KSIG_ANY	0x5f
#define KSIG_ELLIPSIS	0x80
#define KSIG_ALLOW_INV  0x20
#define KSIG_INVALID	KSIG_ALLOW_INV

/**
 * Determines whether a list of registers matches a given signature.
 * If no signature is given (i.e., if sig is NULL), this is always
 * treated as a match.
 *
 * @param s		state to operate on
 * @param sig	signature to test against
 * @param argc	number of arguments to test
 * @param argv	argument list
 * @return true if the signature was matched, false otherwise
 */
bool kernel_matches_signature(SegManager *segMan, const char *sig, int argc, const reg_t *argv);

/**
 * Determines the type of the object indicated by reg.
 * @param segMan			the Segment manager
 * @param reg				register to check
 * @param allow_invalid		determines whether invalid pointer (=offset) values are allowed
 * @return one of KSIG_* below KSIG_NULL.
 *	       KSIG_INVALID set if the type of reg can be determined, but is invalid.
 *	       0 on error.
 */
int determine_reg_type(SegManager *segMan, reg_t reg, bool allow_invalid);

/**
 * Returns a textual description of the type of an object.
 * @param type		type value to describe
 * @return pointer to a (static) descriptive string
 */
const char *kernel_argtype_description(int type);

} // End of namespace Sci

#endif // SCI_ENGIENE_KERNEL_TYPES_H
