/*
 *  Copyright (C) 2000-2001  The Exult Team
 *  Copyright (C) 2003-2004  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef ULTIMA8_MISC_COMMON_TYPES_H
#define ULTIMA8_MISC_COMMON_TYPES_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima8 {


enum {
	/* Output no message(s). For obvious reasons this probably should never need to be used... */
	MM_NONE = 0x00,
	/* general informative messages like the `virtual path "@home"...` stuff.
	    non-critical and in the general case non-useful unless trying to debug a problem. */
	MM_INFO = 0x01,
	/* similar in urgency to a general message, except it's just noting a possible, though
	    unlikely problem. The:
	    `Error mounting virtual path "@data": directory not found: /usr/local/share/pentagram`
	    message would be a good example of this, I believe. */
	MM_MINOR_WARN = 0x02,
	/* "We had a problem, and we're trying to recover from it, hopefully successfully." */
	MM_MINOR_ERR = 4,
	/* A message noting that we encountered a significant problem and that we're going to
	    (hopefully!) gracefully terminate the engine... Probably though a call to
	    CoreApp::application->ForceQuit();
	    "We had a rather significant problem. Shutting down pentagram now..."
	    */
	MM_MAJOR_ERR = 5,
};

//
// {s,u}int{8,16,32,ptr}
//
#ifndef BASIC_TYPES_DEFINED
#define BASIC_TYPES_DEFINED

#ifndef EX_TYPE_INT8
#define EX_TYPE_INT8 char
#endif

#ifndef EX_TYPE_INT16
#define EX_TYPE_INT16 int16
#endif

#ifndef EX_TYPE_INT32
#define EX_TYPE_INT32 int32
#endif

#ifndef EX_TYPE_INTPTR
#define EX_TYPE_INTPTR long long
#endif

typedef unsigned EX_TYPE_INTPTR uintptr;
typedef signed EX_TYPE_INTPTR sintptr;

#endif

// ObjId and ProcId

//! 16-Bit ID of an Object
typedef uint16 ObjId;

//! 16-Bit ID of a Process
typedef uint16 ProcId;

} // End of namespace Ultima8
} // End of namespace Ultima

#endif

