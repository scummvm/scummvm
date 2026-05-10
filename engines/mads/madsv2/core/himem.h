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

#ifndef MADS_CORE_HIMEM_H
#define MADS_CORE_HIMEM_H

#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/mem.h"

namespace MADS {
namespace MADSV2 {

#define         HIMEM_MAX_RESIDENT        150

typedef struct {
	byte memory_type;                           /* Memory type       */
	byte ems_page_handle;                       /* EMS paging info   */
	byte level;                                 /* Preload level     */
	char list[14];                              /* File name info    */
	word xms_handle;                            /* Handle in XMS     */
	long size;                                  /* Size in bytes     */
	int num_packets;                            /* Number of packets */
	long packet_size[PACK_MAX_LIST_LENGTH];     /* Sizes of packets  */
} HimemDirectory;


extern byte himem_preload_ems_disabled;
extern byte himem_preload_xms_disabled;

extern byte himem_directory_allocation;
extern HimemDirectory himem_directory_save_area;
extern HimemDirectory *himem_directory;
extern HimemDirectory *himem_directory_entry;
extern HimemDirectory *himem_ems_directory;

extern int himem_directory_xms_handle;
extern int himem_directory_ems_active;
extern int himem_active;

extern int himem_ems_preloaded;
extern int himem_xms_preloaded;


extern int himem_activate_directory();
extern int himem_get_directory_entry(int id);
extern int himem_put_directory_entry(int id);
extern int  himem_resident(const char *filename);
extern void himem_catalog();
extern int himem_directory_setup();
extern void himem_shutdown();
extern void himem_startup();
extern int himem_preload(char *filename, int level);
extern int himem_preload_series(const char *filename, int level);
extern void himem_flush(int level);

} // namespace MADSV2
} // namespace MADS

#endif
