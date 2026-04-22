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

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/dialog.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/implode.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/pfab.h"

namespace MADS {
namespace MADSV2 {

byte *pack_special_buffer = NULL;
void (*(pack_special_function))() = NULL;
byte *pack_read_memory_ptr;
byte *pack_write_memory_ptr;
long pack_read_size;            /* Size left to read   */
long pack_read_count;           /* Size read so far    */
long pack_write_size;           /* Size left to write  */
long pack_write_count;          /* Size written so */

int pack_default = PACK_NONE;

int pack_ems_page_handle;
int pack_ems_page_marker;
int pack_ems_page_offset;

Common::SeekableReadStream *pack_read_file_handle;
Common::WriteStream *pack_write_file_handle;


/* Pointers to read and write routines */
word(*pack_read_routine)(char *buffer, word *size);
word(*pack_write_routine)(char *buffer, word *size);

word pack_mode = CMP_BINARY;    /* Packing mode for implode   */
byte *pack_buffer;          /* Packing buffer for implode */
word pack_buffer_size;          /* Size of packing buffer     */

byte pack_zip_enabled = false;         /* ZIP packing enabled        */
byte pack_pfab_enabled = false;         /* PFAB packing enabled       */
int  pack_strategy = PACK_PFAB;     /* Current packing strategy   */

/* All compression routines called through function pointers, so that */
/* we can determine at compile time which compression modules will be */
/* linked.                                                            */

word (*pack_implode_routine)(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff,
	word *type,
	word *dsize) = NULL;

word (*pack_explode_routine)(
	word(*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff) = NULL;

word (*pack_pFABcomp_routine)(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff,
	word *type,
	word *dsize) = NULL;

word (*pack_pFABexp0_routine)(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff) = NULL;

word (*pack_pFABexp1_routine)(
	word (*read_buff)(char *buffer, word *size),
	char *write_buf,
	char *work_buff) = NULL;

word (*pack_pFABexp2_routine)(
	byte *read_buf,
	byte *write_buf,
	char *work_buff) = NULL;


void PackStrategy::load(Common::SeekableReadStream *src) {
	type = src->readByte();
	priority = src->readByte();
	size = src->readUint32LE();
	compressed_size = src->readUint32LE();
}

bool PackList::load(Common::SeekableReadStream *src) {
	src->read(id_string, PACK_ID_LENGTH);
	num_records = src->readUint16LE();

	// Confirm that the ID is correct
	if (strncmp(id_string, PACK_ID_STRING, PACK_ID_CHECK) != 0)
		return false;

	// Read in the index. Note that only num_records worth of entries are
	// valid, but space is left in the file for the maximum amount of entries
	for (int i = 0; i < PACK_MAX_LIST_LENGTH; ++i)
		strategy[i].load(src);

	return true;
}


word pack_read_memory(char *buffer, word *mysize) {
	word cx = *mysize;
	uint32 remaining = pack_read_size;
	word return_value = 0;

	if ((uint32)pack_read_size != 0xFFFFFFFFUL) {
		if (remaining == 0)
			goto read_done;

		if (cx > remaining)
			cx = (word)remaining;

		pack_read_size -= (uint32)cx;
	}

	// read_infinite:
	pack_read_count += (uint32)cx;
	return_value = cx;

	if (cx != 0)
	{
		memcpy(buffer, pack_read_memory_ptr, cx);
		pack_read_memory_ptr = (byte *)pack_read_memory_ptr + cx;
	}

read_done:
	pack_read_memory_ptr = (byte *)mem_normalize(pack_read_memory_ptr);
	return return_value;
}

word pack_write_memory(char *buffer, word *mysize) {
	word cx = *mysize;
	uint32 remaining = pack_write_size;

	if (pack_write_size != 0xFFFFFFFFUL)  // not "infinite write"
	{
		if (remaining == 0)
			goto write_done;

		if (cx > remaining)
			cx = (word)remaining;

		pack_write_size -= (uint32)cx;
	}

	// write_infinite:
	pack_write_count += (uint32)cx;

	if (cx != 0) {
		memcpy(pack_write_memory_ptr, buffer, cx);
		pack_write_memory_ptr = (byte *)pack_write_memory_ptr + cx;
	}

write_done:
	pack_write_memory_ptr = (byte *)mem_normalize(pack_write_memory_ptr);

	return 0;
}

word pack_read_file(char *buffer, word *size) {
	word read_this_time;

	if (pack_read_size >= 0) {
		read_this_time = MIN<uint32>(*size, pack_read_size);
	} else {
		read_this_time = *size;
	}

	if (read_this_time == 0) goto done;

	read_this_time = (word)fileio_fread_f(buffer, 1, read_this_time, pack_read_file_handle);
	if (pack_read_size > 0) {
		pack_read_size -= read_this_time;
	}
	pack_read_count += read_this_time;

done:
	return read_this_time;
}

word pack_write_file(char *buffer, word *size) {
	word write_this_time;

	if (pack_write_size >= 0) {
		write_this_time = (word)MIN((long)*size, pack_write_size);
	} else {
		write_this_time = *size;
	}

	if (write_this_time == 0) goto done;

	if (!fileio_fwrite_f(buffer, write_this_time, 1, pack_write_file_handle)) {
		write_this_time = 0;
	} else {
		if (pack_write_size > 0) {
			pack_write_size -= write_this_time;
		}
		pack_write_count += write_this_time;
	}

done:
	return write_this_time;
}


word pack_raw_copy(void) {
	word result = CMP_NO_ERROR;
	word read_this_time;
	word read_back;

	while ((pack_read_size > 0) && (result == CMP_NO_ERROR)) {
		read_this_time = (word)MIN((long)pack_buffer_size, pack_read_size);
		read_back = (*pack_read_routine)((char *)pack_buffer, &read_this_time);
		if (read_back != read_this_time) {
			result = CMP_ABORT;
		} else {
			(*pack_write_routine)((char *)pack_buffer, &read_this_time);
		}
	}

	return (result);
}


word pack_a_packet(int packing_flag, int explode_mode) {
	word result;
	word pack_window_size;

	switch (packing_flag) {
	case PACK_IMPLODE:
		pack_window_size = PACK_WINDOW_SIZE;
		// while ((pack_read_size < (long)pack_window_size) && ((long)pack_window_size > PACK_MIN_WINDOW_SIZE)) {
		// pack_window_size = pack_window_size >> 1;
		// }
		if (pack_strategy == PACK_PFAB) {
			result = (*pack_pFABcomp_routine)(pack_read_routine, pack_write_routine, (char *)pack_buffer,
				&pack_mode, &pack_window_size);
		} else {
			result = (*pack_implode_routine)(pack_read_routine, pack_write_routine, (char *)pack_buffer,
				&pack_mode, &pack_window_size);
		}
		break;

	case PACK_EXPLODE:
		if (pack_strategy == PACK_PFAB) {
			switch (explode_mode) {
			case 2:
				result = (*pack_pFABexp2_routine) (pack_read_memory_ptr, pack_write_memory_ptr, (char *)pack_buffer);
				break;

			case 1:
				result = (*pack_pFABexp1_routine)(pack_read_routine, (char *)pack_write_memory_ptr, (char *)pack_buffer);
				break;

			case 0:
			default:
				result = (*pack_pFABexp0_routine) (pack_read_routine, pack_write_routine, (char *)pack_buffer);
				break;
			}
		} else {
			result = (*pack_explode_routine) (pack_read_routine, pack_write_routine, (char *)pack_buffer);
		}
		break;

	case PACK_RAW_COPY:
	default:
		result = pack_raw_copy();
		break;
	}

	return (result);
}

static void pack_activate(void) {
	if (pack_special_function)
		pack_special_function();
}

void pack_set_special_buffer(byte *buffer_address, void (*(special_function))()) {
	pack_special_buffer = buffer_address;
	pack_special_function = special_function;
}

long pack_data(int packing_flag, long size,
	int source_type, void *source, int dest_type, void *dest) {
	int explode_mode = 0;
	long *loop_value;
	long *return_value;
	EmsPtr *ems_dest;
	int result;

	// Select the read data routine
	if (source_type == FROM_MEMORY) {
		pack_read_routine = pack_read_memory;
		pack_read_memory_ptr = (byte *)source;
	} else {
		// FROM_DISK
		pack_read_routine = pack_read_file;
		pack_read_file_handle = (Common::SeekableReadStream *)source;
	}

	// Select the write data routine
	if (dest_type == TO_EMS) {
		pack_write_routine = pack_write_ems;
		ems_dest = (EmsPtr *)dest;
		pack_ems_page_handle = ems_dest->handle;
		pack_ems_page_marker = ems_dest->page_marker;
		pack_ems_page_offset = ems_dest->page_offset;
	} else if (dest_type == TO_MEMORY) {
		pack_write_routine = pack_write_memory;
		pack_write_memory_ptr = (byte *)dest;
	} else {
		// TO_DISK
		pack_write_routine = pack_write_file;
		pack_write_file_handle = (Common::WriteStream *)dest;
	}

	// Set up the packing parameters
	pack_read_count = pack_write_count = 0;

	switch (packing_flag) {
	case PACK_IMPLODE:
		if (pack_strategy == PACK_PFAB) {
			pack_buffer_size = PACK_PFABCOMP_SIZE;
			if (pack_pFABcomp_routine == NULL) {
				error_report(ERROR_EXPLODER_NULL, SEVERE, MODULE_EXPLODER, packing_flag, pack_strategy);
			}
		} else {
			pack_buffer_size = PACK_IMPLODE_SIZE;
			if (pack_implode_routine == NULL) {
				error_report(ERROR_EXPLODER_NULL, SEVERE, MODULE_EXPLODER, packing_flag, pack_strategy);
			}
		}
		pack_read_size = size;  // Stop after reading "size" bytes
		pack_write_size = -1;  // Write as many bytes as necessary
		loop_value = &pack_read_size;  // Loop control is # bytes to read
		return_value = &pack_read_count;  // Return value is # bytes read
		break;

	case PACK_EXPLODE:
		pack_read_size = -1;  // Read as many bytes as necessary
		pack_write_size = size;  // Stop after writing "size" bytes
		loop_value = &pack_write_size;  // Loop control is # bytes to write
		if (pack_strategy == PACK_PFAB) {
			if ((source_type == FROM_MEMORY) && (dest_type == FROM_MEMORY) &&
				(pack_pFABexp2_routine != NULL)) {
				return_value = &size;  // Fake return value
				pack_buffer_size = PACK_PFABEXP2_SIZE;
				explode_mode = 2;
			} else if ((dest_type == TO_DISK) || (dest_type == TO_EMS)) {
				return_value = &pack_write_count;  // Return value is # bytes written
				pack_buffer_size = PACK_PFABEXP0_SIZE;
				explode_mode = 0;
				if (pack_pFABexp0_routine == NULL) {
					error_report(ERROR_EXPLODER_NULL, SEVERE, MODULE_EXPLODER, packing_flag, pack_strategy);
				}
			} else {
				return_value = &size;  // Fake return value for file-to_mem
				pack_buffer_size = PACK_PFABEXP1_SIZE;
				explode_mode = 1;
				if (pack_pFABexp1_routine == NULL) {
					error_report(ERROR_EXPLODER_NULL, SEVERE, MODULE_EXPLODER, packing_flag, pack_strategy);
				}
			}
		} else {
			return_value = &pack_write_count;
			pack_buffer_size = PACK_EXPLODE_SIZE;
			if (pack_explode_routine == NULL) {
				error_report(ERROR_EXPLODER_NULL, SEVERE, MODULE_EXPLODER, packing_flag, pack_strategy);
			}
		}
		break;

	case PACK_RAW_COPY:
	default:
		pack_buffer_size = PACK_RAW_COPY_SIZE;
		pack_read_size = size;  // Stop after reading "size" bytes
		pack_write_size = size;  // ... or after writing "size" bytes
		loop_value = &pack_read_size;  // Loop control is # bytes to read
		return_value = &pack_write_count;  // Return value is # bytes written
		break;
	}

	// Get memory for packing buffer if necessary
	pack_buffer = NULL;

	if (pack_special_buffer == NULL) {
		pack_buffer = (byte *)mem_get_name(pack_buffer_size, "$pack$");
		if (pack_buffer == NULL) {
			*return_value = 0;
			goto done;
		}
	} else {
		pack_buffer = pack_special_buffer;
	}

	// Keep moving records until we run out of data or die
	if ((packing_flag == PACK_EXPLODE) && (dest_type == TO_MEMORY)) {
		result = pack_a_packet(packing_flag, explode_mode);
		if (result != CMP_NO_ERROR) {
			*return_value = 0;
			error_report(ERROR_EXPLODER_EXPLODED, SEVERE, MODULE_EXPLODER, (packing_flag * 1000) + explode_mode, result);
			goto done;
		}
	} else {
		while (*loop_value > 0) {
			if (pack_a_packet(packing_flag, false) != CMP_NO_ERROR) {
				*return_value = 0;
				error_report(ERROR_EXPLODER_EXPLODED, SEVERE, MODULE_EXPLODER, packing_flag, dest_type);
				goto done;
			}
		}
	}

	// Free memory and go away
done:
	if (pack_special_buffer == NULL) {
		if (pack_buffer != NULL) mem_free(pack_buffer);
	} else {
		pack_activate();
	}

	return *return_value;
}


int pack_check(void) {
	dialog_declare_ok(dialog);
	ItemPtr none_item, zip_item = nullptr, pfab_item = nullptr, default_item;
	int result;
	int zip_available;
	int pfab_available;
	int zip_place;

	zip_available = (pack_implode_routine != NULL);
	pfab_available = (pack_pFABcomp_routine != NULL);

	result = pack_default;

	dialog = dialog_create_default();
	if (dialog == NULL) goto done;

	dialog_add_blank(dialog);
	dialog_center_message(dialog, "Select Data Compression Mode");
	dialog_add_blank(dialog);
	none_item = dialog_left_button(dialog, "~None");


	if (zip_available) {
		zip_place = pfab_available ? DD_IX_CENTER : DD_IX_RIGHT;
		zip_item = dialog_add_button(dialog, zip_place, DD_IY_BUTTON, "~Zip");
		pack_default = PACK_ZIP;
	}

	if (pfab_available) {
		pfab_item = dialog_add_button(dialog, DD_IX_RIGHT, DD_IY_BUTTON, "~PFAB");
		pack_default = PACK_PFAB;
	}

	default_item = none_item;
	if (pack_default == PACK_ZIP)  default_item = zip_item;
	if (pack_default == PACK_PFAB) default_item = pfab_item;

	dialog->cancel_item = default_item;

	dialog_exec(dialog, default_item);

	if (result_item == none_item) {
		result = PACK_NONE;
	} else if (result_item == zip_item) {
		result = PACK_ZIP;
	} else {
		result = PACK_PFAB;
	}

	if (result != PACK_NONE) pack_strategy = result;

done:
	if (dialog != NULL) dialog_destroy(dialog);
	return (result);
}

void pack_enable_zip(void) {
	pack_strategy = PACK_ZIP;
	pack_zip_enabled = true;
	pack_implode_routine = implode;
	pack_explode_routine = explode;
}

void pack_enable_pfab(void) {
	pack_strategy = PACK_PFAB;
	pack_pfab_enabled = true;
	pack_pFABcomp_routine = pFABcomp;
	pack_pFABexp0_routine = pFABexp0;
	pack_pFABexp1_routine = pFABexp1;
	pack_pFABexp2_routine = pFABexp2;
}

void pack_enable_pfab_explode(void) {
	pack_strategy = PACK_PFAB;
	pack_pfab_enabled = true;
	pack_pFABcomp_routine = NULL;
	pack_pFABexp0_routine = pFABexp0;
	pack_pFABexp1_routine = pFABexp1;
	pack_pFABexp2_routine = pFABexp2;
}

void pack_enable_zip_explode(void) {
	pack_strategy = PACK_ZIP;
	pack_zip_enabled = true;
	pack_implode_routine = NULL;
	pack_explode_routine = explode;
}

long pack_rle(byte *target, byte *source, word source_size) {
	word dx = 0;
	byte *di = target;
	byte *si = source;
	byte al, bh, bl;
	word cx = source_size;

	al = *si++;
	cx--;

new_code:
	bh = al;
	bl = 1;

read_loop:
	al = *si++;
	if (bh != al || bl == 255)
		goto make_new;
	bl++;
	if (--cx != 0)
		goto read_loop;

	// loop fell through: store current run and finish
	*((word *)di) = (word)bh | ((word)bl << 8);
	di += 2;
	dx += 2;
	goto packed;

make_new:
	*((word *)di) = (word)bh | ((word)bl << 8);
	di += 2;
	dx += 2;
	if (--cx != 0)
		goto new_code;

	// store the final byte as a run of 1
	*((word *)di) = (word)1 | ((word)al << 8);
	di += 2;
	dx += 2;

packed:
	// store terminating zero word
	*((word *)di) = 0;
	di += 2;
	dx += 2;

	return (long)dx;
}


word pack_write_ems(char *buffer, word *mysize) {
	word write_this_time;
	word write_left;
	word write_this_loop;
	byte *from_pointer;
	byte *unto_pointer;

	if (pack_write_size >= 0) {
		write_this_time = (word)MIN((long)*mysize, pack_write_size);
	} else {
		write_this_time = *mysize;
	}

	if (write_this_time == 0) goto done;

	write_left = write_this_time;
	from_pointer = (byte *)buffer;
	unto_pointer = ems_page[0];

	do {
		if (pack_ems_page_offset >= EMS_PAGE_SIZE) {
			pack_ems_page_marker = ems_next_handle_page(pack_ems_page_handle,
				pack_ems_page_marker);
			pack_ems_page_offset = 0;
			if (pack_ems_page_marker < 0) goto done;
		}

		write_this_loop = MIN((int)(EMS_PAGE_SIZE - pack_ems_page_offset), (int)write_left);

		ems_map_page(0, pack_ems_page_marker);

		memcpy(unto_pointer + pack_ems_page_offset, from_pointer, write_this_loop);

		from_pointer = (byte *)mem_normalize(from_pointer + write_this_loop);

		write_left -= write_this_loop;
		pack_ems_page_offset += write_this_loop;

		if (pack_write_size > 0) {
			pack_write_size -= write_this_loop;
		}
		pack_write_count += write_this_loop;

	} while (write_left > 0);

done:
	return write_this_time;
}

} // namespace MADSV2
} // namespace MADS
