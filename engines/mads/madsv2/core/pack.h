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

#ifndef MADS_CORE_PACK_H
#define MADS_CORE_PACK_H

#include "common/stream.h"
#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define PACK_ID_STRING                "MADSPACK 2.0\032"
#define PACK_ID_LENGTH                14
#define PACK_ID_CHECK                 12

#define PACK_IMPLODE                  0       /* Currently imploding  */
#define PACK_EXPLODE                  1       /* Currently exploding  */
#define PACK_RAW_COPY                 2       /* Currently copying    */

#define FROM_MEMORY                   0       /* Read memory  */
#define FROM_DISK                     1       /* Read disk    */
#define TO_MEMORY                     0       /* Write memory */
#define TO_DISK                       1       /* Write disk   */
#define TO_EMS                        2       /* Write EMS    */

#define PACK_NONE                     0       /* No compression */
#define PACK_PFAB                     1       /* Dave's Stuff   */
#define PACK_ZIP                      2       /* Zipped         */

#define PACK_IMPLODE_SIZE             35256   /* pkzip implode buffer */
#define PACK_EXPLODE_SIZE             12574   /* pkzip explode buffer */

#define PACK_PFABCOMP_SIZE            0x71be
#define PACK_PFABEXP0_SIZE            0x382c
#define PACK_PFABEXP1_SIZE            0x0820
#define PACK_PFABEXP2_SIZE            0x0004
/*
#define PACK_PFABCOMP_SIZE            0xd1d0
#define PACK_PFABEXP0_SIZE            0x382e
#define PACK_PFABEXP1_SIZE            0x0822
#define PACK_PFABEXP2_SIZE            0x0004
*/

#define PACK_RAW_COPY_SIZE            0x1000

#define PACK_WINDOW_SIZE              4096    /* sliding window size   */
#define PACK_MIN_WINDOW_SIZE          1024    /* minimum possible size */

#define PACK_MAX_PACKET_SIZE          0xfc00  /* Max size of a packet  */

#define PACK_MAX_LIST_LENGTH          16      /* Max # blocks in one file */


#define PACK_PRIORITY_SPRITE_SERIES   4
#define PACK_PRIORITY_ANIMATIONS      5

#define PACK_PRIORITY_FONTS           7
#define PACK_PRIORITY_ROOM_DATA       8
#define PACK_PRIORITY_ROOM_HOTSPOTS   9
#define PACK_PRIORITY_INTERFACES      10
#define PACK_PRIORITY_ROOM_ART        11

#define PACK_OVERHEAD                 PackList::SIZE

struct PackStrategy {
	byte type;
	byte priority;
	long size;
	long compressed_size;

	void load(Common::SeekableReadStream *src);
	static constexpr int SIZE = 1 + 1 + 4 + 4;
};

typedef PackStrategy *PackStrategyPtr;

#define PACK_HEADER   (PACK_ID_LENGTH + 2)

struct PackList {
	char id_string[PACK_ID_LENGTH];
	word num_records;
	PackStrategy strategy[PACK_MAX_LIST_LENGTH];

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE = PACK_HEADER + PackStrategy::SIZE * PACK_MAX_LIST_LENGTH;
};

typedef PackList *PackListPtr;


extern byte *pack_read_memory_ptr;  /* Current read memory location  */
extern byte *pack_write_memory_ptr; /* Current write memory location */

extern Common::SeekableReadStream *pack_read_file_handle;	/* Current read file handle      */
extern Common::WriteStream *pack_write_file_handle;			/* Current write file handle     */

extern long pack_read_size;             /* Size left to read             */
extern long pack_read_count;            /* Size read so              */
extern long pack_write_size;            /* Size left to write            */
extern long pack_write_count;           /* Size written so           */

/* Pointer to read routine  */
extern word(*pack_read_routine)(char *buffer, word *size);
/* Pointer to write routine */
extern word(*pack_write_routine)(char *buffer, word *size);

extern word pack_mode;                  /* Packing mode (zip/none)       */
extern byte *pack_buffer;           /* Packing scrap buffer          */
extern word pack_buffer_size;           /* Size of packing buffer        */

extern int  pack_default;               /* Default packing mode          */

extern byte pack_zip_enabled;           /* ZIP packing enabled           */
extern byte pack_pfab_enabled;          /* PFAB packing enabled          */
extern int  pack_strategy;              /* Current packing strategy      */

extern int pack_ems_page_handle;
extern int pack_ems_page_marker;
extern int pack_ems_page_offset;

/* All compression routines called through function pointers, so that */
/* we can determine at compile time which compression modules will be */
/* linked.                                                            */

extern word (*pack_implode_routine)(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff,
	word *type,
	word *dsize);

extern word (*pack_explode_routine)(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff);

extern word (*pack_pFABcomp_routine)(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff,
	word *type,
	word *dsize);

extern word (*pack_pFABexp0_routine)(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff);

extern word (*pack_pFABexp1_routine)(
	word (*read_buff)(char *buffer, word *size),
	char *write_buf,
	char *work_buff);

extern word (*pack_pFABexp2_routine)(
	byte *read_buf,
	byte *write_buf,
	char *work_buff);


extern byte *pack_special_buffer;
extern void (*pack_special_function)();


extern word pack_read_memory(char *buffer, word *size);
extern word pack_write_memory(char *buffer, word *size);
extern word pack_read_file(char *buffer, word *size);
extern word pack_write_file(char *buffer, word *size);
/*
 * pack_a_packet()
 * Given that our packing parameters are set up (i.e.
 * pack_read_size, pack_write_size, and so forth), this
 * routine uses the specified packing strategy to move
 * a record.
 *
 * @param packing_flag	
 * @param explode_mode	
 * @return 
 */
extern word pack_a_packet(int packing_flag, int explode_mode);
/*
 * pack_data()
 * Transfers a data packet from the specified source to the specified
 * destination, using the specified packing strategy.
 * packing_flag    Specifies the packing strategy:
 * PACK_IMPLODE  (Compresses   data)
 * PACK_EXPLODE  (Decompresses data)
 * PACK_RAW_COPY (Copies data)
 * size            # of bytes to move
 * source_type     Specifies the source type:
 * FROM_DISK or FROM_MEMORY.
 * source          If FROM_DISK, then this is a FILE *handle.
 * If FROM_MEMORY, this is a far memory pointer.
 * dest_type       Specifies the destination type:
 * TO_DISK, TO_MEMORY, or TO_EMS.
 * dest            Same as "source" but for destination. For
 * TO_EMS, "dest" is a far pointer to an
 * EmsPtr structure.
 * Example:
 * result =pack_data (PACK_EXPLODE, 132000,
 * FROM_DISK, file_handle,
 * TO_MEMORY, memory_pointer);
 * (Decompresses 132000 bytes from the already open
 * disk file "file_handle", and writes it to memory
 * far the specified address.  Size is always the
 * uncompressed size of the data.  Result will be
 * the # of bytes actually written -- 132000 if successful).
 *
 * @param packing_flag	
 * @param size	
 * @param source_type	
 * @param source	
 * @param dest_type	
 * @param dest	
 * @return 
 */
extern long pack_data(int packing_flag, long size, int source_type, void *source,
	int dest_type, void *dest);
extern void pack_set_special_buffer(byte *buffer_address,
	void (*(special_function))());
/*
 * pack_check()
 * Asks user to choose between compressed and uncompressed data
 * formats.
 *
 * @return 
 */
extern int pack_check(void);
extern void pack_enable_zip(void);
extern void pack_enable_pfab(void);
extern void pack_enable_pfab_explode(void);
extern void pack_enable_zip_explode(void);
extern long pack_rle(byte *target, byte *source, word source_size);
extern word pack_write_ems(char *buffer, word *mysize);

} // namespace MADSV2
} // namespace MADS

#endif
