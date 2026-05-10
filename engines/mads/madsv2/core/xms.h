/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MADS_CORE_XMS_H
#define MADS_CORE_XMS_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define XMS_MAX_UMB     16              /* Max UMB blocks to allocate */

#define XMS             byte *      /* XMS fake pointer type      */

extern bool  xms_exists;           /* Flag if we've got some XMS memory  */
extern bool  xms_disabled;         /* Flag to disable XMS altogether     */
extern word  xms_version;          /* XMS driver version #               */
extern dword xms_controller;       /* XMS controller address             */
extern word  xms_chain_seg;        /* First UMB MCB segment for us       */

extern word xms_umb_list[XMS_MAX_UMB];
extern word xms_umb_mark;

inline int xms_detect() { return 0; }
inline long xms_umb_get_avail() { return 0;}
inline void *xms_umb_get(long mem_to_get) { return nullptr; }
inline void xms_umb_free(void *mem_to_free) {}
inline void xms_umb_purge() {}

/**
 * Returns the size of the largest free XMS block.
 */
inline long xms_get_avail() { return 0; }

/**
 * Allocates an XMS block of the appropriate size, and returns a handle for it.
 */
inline int  xms_get(long size_in_bytes) { return 0; }

/**
 * Deallocates an XMS block
 */
inline void xms_free(int xms_handle) {}

/**
 * Copy data to/from/between XMS memory blocks.  Note that the parameter
 * stack format corresponds precisely to the XMS function spec, so we
 * can just point directly at the parameter stack and call XMS.
 *
 * For conventional memory, use handle "0", and a traditional
 * segmented far pointer.
 *
 * For extended memory, use the proper handle number, and a full
 * 32-bit (unsegmented) pointer.
 */
inline int xms_copy(long copy_size, word source_handle, void *source, word dest_handle, void *dest) { return 0; }

} // namespace MADSV2
} // namespace MADS

#endif
