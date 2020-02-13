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

#ifndef GLK_ALAN3_CHECKENTRY
#define GLK_ALAN3_CHECKENTRY

#include "glk/alan3/types.h"
#include "glk/alan3/acode.h"
#include "glk/jumps.h"

namespace Glk {
namespace Alan3 {

/* CONSTANTS */
#ifndef EXECUTE_CHECK_BODY_ON_FAIL
#define EXECUTE_CHECK_BODY_ON_FAIL TRUE
#define DONT_EXECUTE_CHECK_BODY_ON_FAIL FALSE
#endif


/* TYPES */
struct CheckEntry { /* CHECK TABLE */
	Aaddr exp;            /* ACODE address to expression code */
	Aaddr stms;           /* ACODE address to statement code */
};


/* DATA */
typedef CheckEntry CheckEntryArray[];


/* FUNCTIONS */
extern bool checksFailed(CONTEXT, Aaddr adr, bool execute);

} // End of namespace Alan3
} // End of namespace Glk

#endif
