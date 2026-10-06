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
#include "common/compression/dcl.h"
#include "common/memstream.h"
#include "mads/core/loader.h"
#include "mads/core/env.h"
#include "mads/core/general.h"
#include "mads/core/error.h"
#include "mads/core/fileio.h"
#include "mads/core/mem.h"
#include "mads/core/pack.h"
#include "mads/core/popup.h"
#include "mads/core/timer.h"

namespace MADS {

char loader_last[14] = "";


LoaderReadStream::LoaderReadStream(LoadHandle load, long size) {
	byte *data;

	if (size < 0) {
		// No explicit size was given, so the caller wants the whole file. The
		// pack list may hold multiple index entries (e.g. header/data split into
		// separate compressed sections), and loader_read() only ever decompresses
		// one entry per call. Read each remaining entry in turn using its own
		// declared uncompressed size, advancing the destination pointer, instead
		// of asking for the full decompressed size in a single call.
		size = load->decompress_size;
		data = (byte *)malloc(size);
		byte *dest = data;

		while (load->pack_list_marker < (int)load->pack.num_records) {
			long entrySize = load->pack.strategy[load->pack_list_marker].size;
			(void)loader_read(dest, 1, entrySize, load);
			dest += entrySize;
		}
	} else {
		data = (byte *)malloc(size);
		(void)loader_read(data, 1, size, load);
	}

	_data = new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}

LoaderReadStream::~LoaderReadStream() {
	delete _data;
}

//====================================================================

int loader_open(LoadHandle handle, const char *filename, const char *options, int flags) {
	int error_flag = true;
	int reading;
	int count;

	strncpy(loader_last, filename, 13);

	handle->open = false;

	reading = strchr(options, 'r') != NULL;

	handle->mode = LOADER_DISK;
	handle->handle = env_open(filename, options);
	if (handle->handle == NULL) goto done;

	handle->reading = reading;
	handle->pack_list_marker = 0;

	if (reading) {
		if (!handle->pack.load(handle->handle))
			goto done;

		handle->decompress_size = 0;
		for (count = 0; count < (int)handle->pack.num_records; count++) {
			handle->decompress_size += handle->pack.strategy[count].size;
		}
	} else {
		error("Open for writing not supported in ScummVM");
	}

	handle->open = true;

	error_flag = false;

done:
	return error_flag;
}

void loader_set_priority(LoadHandle handle, int priority) {
	handle->pack.strategy[0].priority = (byte)priority;
}

int loader_close(LoadHandle handle) {
	int error_flag = false;

	if (handle->open) {
		if (!handle->reading) {
			error("loader_close for writing not supported in ScummVM");
		}

		delete handle->handle;
	}
	handle->open = false;

	return error_flag;
}

long loader_read(void *target, long record_size, long record_count, LoadHandle handle) {
	long total_size;
	long result;
	long file_position;
	long compressed_size;
	byte *decompress_buffer = NULL;
	int packing_flag;
	int marker;
	int already_unpacked = false;

	if (!record_size)
		return 0;

	total_size = record_size * record_count;

	marker = handle->pack_list_marker++;

	result = 0;
	pack_strategy = handle->pack.strategy[marker].type;
	compressed_size = handle->pack.strategy[marker].compressed_size;
	packing_flag = (pack_strategy != PACK_NONE) ? PACK_EXPLODE : PACK_RAW_COPY;

	if (packing_flag == PACK_EXPLODE) {
		decompress_buffer = (byte *)mem_get(compressed_size);
		if (decompress_buffer != NULL) {
			if (!fileio_fread_f(decompress_buffer, compressed_size, 1, handle->handle)) goto done;

			if (pack_strategy == PACK_DCL) {
				Common::MemoryReadStream stream(decompress_buffer, compressed_size);
				result = Common::decompressDCL(&stream, (byte *)target,
					compressed_size, total_size) ? total_size : 0;
			} else {
				result = pack_data(packing_flag, total_size,
					FROM_MEMORY, decompress_buffer,
					TO_MEMORY, target);
			}
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

done:
	if (decompress_buffer != NULL)
		mem_free(decompress_buffer);

	if (result == record_size) {
		return 1;
	} else {
		return result / record_size;
	}
}

void loader_skip_entries(LoadHandle handle, int count) {
	long skip = 0;
	for (int i = 0; i < count; ++i)
		skip += handle->pack.strategy[handle->pack_list_marker + i].compressed_size;

	if (skip != 0)
		handle->handle->seek(skip, SEEK_CUR);

	handle->pack_list_marker += count;
}

long loader_write(void *target, long record_size, long record_count, LoadHandle handle) {
	long total_size, result = 0;
	long file_pos;
	int packing_flag;
	int marker;
	int strategy;
	int going = true;

	if (!record_size) return 0;

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

	return result / record_size;
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

} // namespace MADS
