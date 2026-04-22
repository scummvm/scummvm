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

#include "common/textconsole.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/xms.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/popup.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/pack.h"

namespace MADS {
namespace MADSV2 {


/* debug stuff */
#ifndef disable_statistics
long loader_found_in_ems = 0;
long loader_found_in_xms = 0;
long loader_found_on_disk = 0;
long loader_timing_ems = 0;
long loader_timing_xms = 0;
long loader_timing_disk = 0;
long loader_size_ems = 0;
long loader_size_xms = 0;
long loader_size_disk = 0;
#endif

int loader_ems_search_disabled = false;

char loader_last[14] = "";


uint32 LoaderReadStream::read(void *dataPtr, uint32 dataSize) {
	loader_read(dataPtr, dataSize, 1, _load);
	return dataSize;
}


int loader_open(LoadHandle handle, const char *filename, const char *options, int flags) {
	int error_flag = true;
	int found_himem = -1;
	int reading;
	int count;
	int search_himem;
	long base_position;

	strncpy(loader_last, filename, 13);

	search_himem = flags;
	handle->open = false;

	reading = strchr(options, 'r') != NULL;

	if (reading && search_himem && !loader_ems_search_disabled) {
		found_himem = himem_resident(filename);
	}

	if (found_himem >= 0) {
		handle->mode = (byte)((himem_directory_entry->memory_type == MEM_EMS) ? LOADER_EMS : LOADER_XMS);
		handle->xms_handle = himem_directory_entry->xms_handle;
		handle->xms_offset = 0;
		handle->ems_handle = found_himem;
		handle->ems_page_marker = -1;
		handle->ems_page_offset = EMS_PAGE_SIZE;
		handle->decompress_size = himem_directory_entry->size;
		handle->pack.num_records = himem_directory_entry->num_packets;
		handle->pack_list_marker = 0;
		handle->reading = true;

		// printf ("Opened %d with xms handle %d\n", found_himem, himem_directory_entry->xms_handle);
		for (count = 0; count < (int)handle->pack.num_records; count++) {
			handle->pack.strategy[count].type = PACK_NONE;
			handle->pack.strategy[count].size = himem_directory_entry->packet_size[count];
			handle->pack.strategy[count].compressed_size = himem_directory_entry->packet_size[count];
		}

#ifndef disable_statistics
		if (himem_directory_entry->memory_type == MEM_EMS) {
			loader_found_in_ems++;
		} else {
			loader_found_in_xms++;
		}
#endif
	} else {
		handle->mode = LOADER_DISK;
		handle->ems_handle = -1;
		handle->handle = env_open(filename, options);
		if (handle->handle == NULL) goto done;

		handle->reading = reading;
		handle->pack_list_marker = 0;

		if (reading) {
			base_position = handle->handle->pos();

			if (!handle->pack.load(handle->handle))
				goto done;

			base_position = handle->handle->pos();

			handle->decompress_size = 0;
			for (count = 0; count < (int)handle->pack.num_records; count++) {
				handle->decompress_size += handle->pack.strategy[count].size;
			}
		} else {
#ifdef TODO
			handle->pack.num_records = 0;
			handle->pack.strategy[0].type = (byte)flags;
			Common::strcpy_s(handle->pack.id_string, PACK_ID_STRING);
			if (!fileio_fwrite_f(&handle->pack, sizeof(PackList), 1, handle->handle)) goto done;
			handle->decompress_size = 0;
#else
			error("TODO: open for writing");
#endif
		}

#ifndef disable_statistics
		loader_found_on_disk++;
#endif
	}

	handle->open = true;

	error_flag = false;

done:

	if (error_flag && !found_himem) {
		if (handle->handle != NULL)
			delete handle->handle;
	}

	return error_flag;
}

void loader_set_priority(LoadHandle handle, int priority) {
	handle->pack.strategy[0].priority = (byte)priority;
}

int loader_close(LoadHandle handle) {
	int error_flag = false;

	if (handle->open) {
		if ((handle->mode == LOADER_EMS) || (handle->mode == LOADER_XMS)) {
			handle->ems_page_marker = -1;
			handle->ems_page_offset = EMS_PAGE_SIZE;
			handle->xms_offset = 0;
			// if (ems_paging_active) ems_unmap_all();
		} else {
			if (!handle->reading) {
#ifdef TODO
				handle->handle->seek(0);
				error_flag = !fileio_fwrite_f(&handle->pack, sizeof(PackList), 1, handle->handle);
#else
				error("TODO: loader_close for writing");
#endif
			}

			delete handle->handle;
		}
	}
	handle->open = false;

	return error_flag;
}

long loader_read(void *target, long record_size, long record_count, LoadHandle handle) {
	long total_size;
	long total_left;
	long result;
	long file_position;
	long compressed_size;
	byte *decompress_buffer = NULL;
	int packing_flag;
	int marker;
	int already_unpacked = false;

#ifndef disable_statistics
	long start_timing;
	long finish_timing;
	long total_timing;

	start_timing = timer_read_600();
#endif

	if (!record_size)
		return 0;

	total_size = record_size * record_count;
	total_left = total_size;

	marker = handle->pack_list_marker++;

	if (handle->mode == LOADER_EMS) {
		result = 0;
		if (ems_copy_it_down(handle->ems_handle,
			&handle->ems_page_marker,
			&handle->ems_page_offset,
			(byte *)target,
			total_size)) goto done;
		result = total_size;
	} else if (handle->mode == LOADER_XMS) {
		result = 0;

		// printf ("Reading (%d) at %ld for size %ld\n", handle->xms_handle, handle->xms_offset, total_size);
		if (xms_copy(total_size,
			handle->xms_handle, (XMS)handle->xms_offset,
			MEM_CONV, target)) goto done;

		result = total_size;
		handle->xms_offset += total_size;
	} else {
		result = 0;
		pack_strategy = handle->pack.strategy[marker].type;
		compressed_size = handle->pack.strategy[marker].compressed_size;
		packing_flag = (pack_strategy != PACK_NONE) ? PACK_EXPLODE : PACK_RAW_COPY;

		if (packing_flag == PACK_EXPLODE) {
			decompress_buffer = (byte *)mem_get(compressed_size);
			if (decompress_buffer != NULL) {
				if (!fileio_fread_f(decompress_buffer, compressed_size, 1, handle->handle)) goto done;

				result = pack_data(packing_flag, total_size,
					FROM_MEMORY, decompress_buffer,
					TO_MEMORY, target);
				already_unpacked = true;
			}
		}

		if (!already_unpacked) {
			file_position = handle->handle->pos();

			result = pack_data(packing_flag, total_size,
				FROM_DISK, handle->handle,
				TO_MEMORY, target);

			if (packing_flag == PACK_EXPLODE)
				handle->handle->seek(file_position + compressed_size);
		}
	}

#ifndef disable_statistics
	finish_timing = timer_read_600();
	total_timing = finish_timing - start_timing;

	if (handle->mode == LOADER_EMS) {
		loader_timing_ems += total_timing;
		loader_size_ems += total_size;
	} else if (handle->mode == LOADER_XMS) {
		loader_timing_xms += total_timing;
		loader_size_xms += total_size;
	} else {
		loader_timing_disk += total_timing;
		loader_size_disk += total_size;
	}
#endif

done:
	if (decompress_buffer != NULL)
		mem_free(decompress_buffer);

	if (result == record_size) {
		return 1;
	} else {
		return result / record_size;
	}
}

long loader_write(void *target, long record_size, long record_count, LoadHandle handle) {
	long total_size, result = 0;
	long file_pos;
	int packing_flag;
	int marker;
	int strategy;
	int going = true;

	if (!record_size) return (0);

	total_size = record_size * record_count;

	marker = handle->pack_list_marker++;
	strategy = handle->pack.strategy[0].type;

	file_pos = handle->handle->pos();

	while (going) {
		packing_flag = (strategy != PACK_NONE) ? PACK_IMPLODE : PACK_RAW_COPY;
		pack_strategy = strategy;
		result = pack_data(packing_flag, total_size,
			FROM_MEMORY, target,
			TO_DISK, handle->handle);

		if ((packing_flag == PACK_RAW_COPY) || (pack_write_count < total_size)) {
			going = false;
		} else {
			fileio_setpos(handle->handle, file_pos);
			strategy = PACK_NONE;
		}
	}

	handle->pack.strategy[marker].type = (byte)strategy;
	handle->pack.strategy[marker].priority = handle->pack.strategy[0].priority;
	handle->pack.strategy[marker].size = total_size;
	handle->pack.strategy[marker].compressed_size = pack_write_count;

	handle->pack.num_records++;

	handle->decompress_size += total_size;

	return (result / record_size);
}

long loader_write_2(Common::WriteStream *source_handle, long total_size, LoadHandle handle) {
	long result;
	long file_pos;
	long file_2_pos;
	int packing_flag;
	int marker;
	int strategy;
	int going = true;

	marker = handle->pack_list_marker++;
	strategy = handle->pack.strategy[0].type;

	file_pos = handle->handle->pos();
	file_2_pos = source_handle->pos();

	while (going) {
		packing_flag = (strategy != PACK_NONE) ? PACK_IMPLODE : PACK_RAW_COPY;
		pack_strategy = strategy;
		result = pack_data(packing_flag, total_size,
			FROM_DISK, source_handle,
			TO_DISK, handle->handle);

		if ((packing_flag == PACK_RAW_COPY) || (pack_write_count < total_size)) {
			going = false;
		} else {
			fileio_setpos(handle->handle, file_pos);
			fileio_setpos(source_handle, file_2_pos);
			strategy = PACK_NONE;
		}
	}

	handle->pack.strategy[marker].type = (byte)strategy;
	handle->pack.strategy[marker].priority = handle->pack.strategy[0].priority;
	handle->pack.strategy[marker].size = total_size;
	handle->pack.strategy[marker].compressed_size = pack_write_count;

	handle->pack.num_records++;

	handle->decompress_size += total_size;

	file_2_pos += total_size;
	fileio_setpos(source_handle, file_2_pos);

	return result == total_size;
}

} // namespace MADSV2
} // namespace MADS
