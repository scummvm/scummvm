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

#include "glk/glulx/glulx.h"

namespace Glk {
namespace Glulx {

uint Glulx::do_gestalt(uint val, uint val2) {
	switch (val) {

	case gestulx_GlulxVersion:
		return 0x00030102; /* Glulx spec version 3.1.2 */

	case gestulx_TerpVersion:
		return 0x00000504; /* Glulx version 0.5.4 */

	case gestulx_ResizeMem:
#ifdef FIXED_MEMSIZE
		return 0; /* The setmemsize opcodes are compiled out. */
#else /* FIXED_MEMSIZE */
		return 1; /* We can handle setmemsize. */
#endif /* FIXED_MEMSIZE */

	case gestulx_Undo:
		return 1; /* We can handle saveundo and restoreundo. */

	case gestulx_IOSystem:
		switch (val2) {
		case 0:
			return 1; /* The "null" system always works. */
		case 1:
			return 1; /* The "filter" system always works. */
		case 2:
			return 1; /* A Glk library is hooked up. */
		default:
			return 0;
		}

	case gestulx_Unicode:
		return 1; /* We can handle Unicode. */

	case gestulx_MemCopy:
		return 1; /* We can do mcopy/mzero. */

	case gestulx_MAlloc:
#ifdef FIXED_MEMSIZE
		return 0; /* The malloc opcodes are compiled out. */
#else /* FIXED_MEMSIZE */
		return 1; /* We can handle malloc/mfree. */
#endif /* FIXED_MEMSIZE */

	case gestulx_MAllocHeap:
		return heap_get_start();

	case gestulx_Acceleration:
		return 1; /* We can do accelfunc/accelparam. */

	case gestulx_AccelFunc:
		if (accel_find_func(val2))
			return 1; /* We know this accelerated function. */
		return 0;

	case gestulx_Float:
#ifdef FLOAT_SUPPORT
		return 1; /* We can do floating-point operations. */
#else /* FLOAT_SUPPORT */
		return 0; /* The floating-point opcodes are not compiled in. */
#endif /* FLOAT_SUPPORT */

#ifdef GLULX_EXTEND_GESTALT
		GLULX_EXTEND_GESTALT
#endif /* GLULX_EXTEND_GESTALT */

	default:
		return 0;

	}
}

} // End of namespace Glulx
} // End of namespace Glk
