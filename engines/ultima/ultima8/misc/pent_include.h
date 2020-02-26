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

#ifndef ULTIMA8_MISC_PENT_INCLUDE_H
#define ULTIMA8_MISC_PENT_INCLUDE_H

#define PENTAGRAM_NEW

//
// Common/base types
//
#include "ultima/ultima8/misc/common_types.h"


//
// p_dynamic_cast support
//
#include "ultima/ultima8/misc/p_dynamic_cast.h"


//
// Strings
//

#include "ultima/ultima8/misc/istring.h"

//
// Base Errors
//

#include "ultima/ultima8/misc/errors.h"


//
// The Debugger
//

#include "ultima/ultima8/misc/debugger.h"


//
// Debugging
//
#ifdef DEBUG
#  define POUT(x)       do { pout << x << Std::endl; pout.flush(); } while (0)
#  define PERR(x)       do { perr << x << Std::endl; perr.flush(); } while (0)
#else
#  define POUT(x)       do { } while(0)
#  define PERR(x)       do { } while(0)
#endif

// Two very useful macros that one should use instead of pure delete; they
// will additionally set the old object pointer to 0, thus helping prevent
// double deletes (note that "delete 0" is a no-op).
#define FORGET_OBJECT(x) do { delete x; x = 0; } while(0)
#define FORGET_ARRAY(x) do { delete [] x; x = 0; } while(0)


//
// Can't happen.
// For things that really can't happen. Or shouldn't anyway.
//
#define CANT_HAPPEN() do { assert(false); } while(0)

//
// Can't happen return.
// If we're guaranteed to return before this, but we want to shut the
// compiler warning up.
//
#define CANT_HAPPEN_RETURN() do { assert(false); return 0; } while(0)

//
// Can't happen with a message
//
// Allows a message to be supplied.
// May not work on all compilers or runtimes as expected
//
#define CANT_HAPPEN_MSG(msg) do { assert(msg && false); } while(0)

//
// Wrapper around valgrind functions.
#include "ultima/ultima8/misc/pent_valgrind.h"

namespace Ultima {
namespace Ultima8 {

// Memory Management through Allocators
typedef void *(*allocFunc)(size_t size);
typedef void (*deallocFunc)(void *ptr);

extern allocFunc palloc;
extern deallocFunc pfree;
void setAllocationFunctions(allocFunc a, deallocFunc d);

#define ENABLE_CUSTOM_MEMORY_ALLOCATION()                           \
	static void * operator new(size_t size);                        \
	static void operator delete(void * ptr);

#define DEFINE_CUSTOM_MEMORY_ALLOCATION(Classname)                  \
	void * Classname::operator new(size_t size) {                       \
		return palloc(size);                                 \
	}                                                                   \
	\
	void Classname::operator delete(void * ptr) {                       \
		pfree(ptr);                                          \
	}

//
// Precompiled Header Support
//
#ifdef USE_PRECOMPILED_HEADER

// C Standard Library and STL
#include <fstream>
#include <cstdio>
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/containers.h"

// Useful Pentagram headers
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/kernel/object.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/graphics/shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/usecode/usecode.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/uc_process.h"
#endif

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
