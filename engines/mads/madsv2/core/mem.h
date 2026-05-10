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

#ifndef MADS_CORE_MEM_H
#define MADS_CORE_MEM_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define MEM_NONE        0xff    /* Undefined (himem)   */

#define MEM_CONV        0       /* Conventional Memory */
#define MEM_UMB         1       /* Upper memory block  */
#define MEM_HMA         2       /* High memory area    */
#define MEM_EMS         3       /* Expanded memory     */
#define MEM_XMS         4       /* Extended memory     */


extern byte  mem_max_free_set;
extern byte  umb_max_free_set;

extern byte mem_last_alloc_failed;      /* True if last mem_get() failed   */
extern int  mem_last_alloc_loader;      /* Last loader to use mem_get()    */
extern long mem_last_alloc_size;        /* Last size attempted by mem_get  */
extern long mem_last_alloc_avail;       /* Last available memory           */
extern long mem_max_free;               /* Most memory that was ever free  */
extern long mem_min_free;               /* Least memory that was ever free */
extern long umb_max_free;               /* Most UMB that was ever free     */
extern long umb_min_free;               /* Least UMB that was ever free    */

extern void (*mem_manager_update)();    /* Called at memory updates        */
extern int  mem_manager_active;         /* Flag if memory manager active   */

inline void *mem_normalize(void *in) {
	return in;
}
inline void *mem_check_overflow(void *in) {
	return in;
}


extern void *mem_get(long size);

/**
 * Allocates the specified amount of memory and returns a pointer to it.
 */
extern void *mem_get_name(long size, const char * /*block_name */= nullptr);

/**
 * Deallocates a previously allocated chunk of memory.  Returns
 * true if an error occurred (?!), false otherwise.
 */
extern bool mem_free(void *block);

/**
 * Adjusts allocated block size
 */
extern int mem_adjust_impl(void *&target, long size);

template <typename T>
inline int mem_adjust(T *&target, long size) {
	return mem_adjust_impl(reinterpret_cast<void *&>(target), size);
}

extern void mem_save_free();
extern void mem_restore_free();

/**
 * Returns the string label of the memory block
 */
extern void mem_get_block_name(byte *block, char *block_name);

extern long mem_get_avail();
extern long mem_conv_get_avail();
extern void mem_set_video_mode(int mode);

extern long mem_program_block();

} // namespace MADSV2
} // namespace MADS

#endif
