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

#ifndef MADS_CORE_EMS_H
#define MADS_CORE_EMS_H

#include "mads/madsv2/core/pack.h"

namespace MADS {
namespace MADSV2 {

#define EMS_PAGING_CLASSES      4       /* # of paging classes            */

#define EMS_PAGING_SYSTEM       0       /* System class - fully available */
#define EMS_PAGING_ROOM         1       /* Room class                     */
#define EMS_PAGING_SECTION      2       /* Section class                  */
#define EMS_PAGING_GLOBAL       3       /* Global class                   */

#define         EMS_TILE_PAGE           0
#define         EMS_PAGE_MAP_PAGE       1
#define         EMS_PRELOAD_PAGE        2

#define         EMS_MAX_RESIDENT        150
#define         EMS_PAGE_SIZE           16384

#define         EMS_DIRECTORY_ADDRESS   2048


typedef struct {
	byte flag;
	byte level;
	char list[14];
	long size;
	int num_packets;
	long packet_size[PACK_MAX_LIST_LENGTH];
} EmsDirectory;


typedef struct {
	int handle;         /* EMS block handle         */
	int page_marker;    /* Page marker in EMS block */
	int page_offset;    /* Offset within the page   */
} EmsPtr;


extern bool ems_driver;			/* Flag if EMS driver is installed      */
extern bool ems_exists;			/* Flag if we've got some EMS available */
extern word ems_page_frame;     /* Segment address of EMS page frame    */
extern word ems_handle;         /* EMS handle of our allocated pages    */
extern word ems_pages;          /* # of pages EMS allocated for us      */
extern int ems_disabled;       /* Flag if EMS disabled altogether      */

extern word ems_high_version;   /* High (integer) part of EMS version # */
extern word ems_low_version;    /* Low (fraction) part of EMS version # */

extern byte *ems_page[4];   /* Pointers to EMS physical pages       */

extern word ems_paging_active;  /* EMS paging system is active          */
extern word ems_pages_free;     /* # of pages available now             */
extern word ems_pages_reserved; /* # of pages reserved                  */
extern byte *ems_page_flag; /* Page allocation flag array           */

extern word ems_paging_reserve[EMS_PAGING_CLASSES]; /* Reserve pages    */


extern int ems_mapping_changed;/* EMS mapping has changed */

extern int ems_page_mapped[4];
extern int ems_page_stack[4];

/**
 * Detects whether or not EMS memory is present.  If EMS is detected,
 * attempts to allocate all available pages.
 * @return	"true" if one or more pages were successfully allocated.
 * Returns "false" if EMS did not exist, if no pages were available,
 * or if an error occurred.
 */
extern bool ems_detect();

/**
 * Frees up any EMS memory that might have been allocated by ems_detect().
 */
extern void ems_shutdown();

/**
 * Maps a logical page (0 - (ems_pages-1)) to a physical page (0-3).
 * Does not actually generate an EMS interrupt if the page is already
 * mapped. Call only if EMS memory was detected.
 */
inline int ems_map_page(word physical, word logical) { return 0; }

inline void ems_unmap_all() {}
inline void ems_push() {}
inline void ems_pop() {}
inline int ems_activate_directory() { return 0; }
inline int ems_activate_page_map() { return 0; }
inline int ems_paging_setup() { return 0; }
inline void ems_free_page_handle(word handle) {}
inline int ems_get_page_handle(word pages_needed) { return -1; }
inline int ems_next_handle_page(int handle, int page_marker) { return -1; }
inline void ems_paging_mode(int paging_mode) {}
inline void ems_paging_shutdown() {}
inline int ems_copy_it_up(int ems_paging_handle, int *ems_page_marker,
	int *ems_page_offset, byte *source, long read_size) { return 0; }
inline int ems_copy_it_down(int ems_paging_handle, int *ems_page_marker,
	int *ems_page_offset, byte *target, long write_size) { return 0; }
inline int ems_map_buffer(int buffer_page_handle) { return 0; }
inline int ems_search_page(int ems_handle_, int page_number) { return 0; }
inline void ems_buffer_to_buffer(int source_handle, int target_handle) {}

} // namespace MADSV2
} // namespace MADS

#endif
