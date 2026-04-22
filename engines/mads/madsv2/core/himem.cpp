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
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/xms.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/env.h"

namespace MADS {
namespace MADSV2 {

// EMS/XMS disabled in ScummVM
byte                himem_preload_ems_disabled = true;
byte                himem_preload_xms_disabled = true;

byte                himem_directory_allocation = MEM_NONE;
HimemDirectory      himem_directory_save_area;
HimemDirectory *himem_directory;
HimemDirectory *himem_directory_entry;
HimemDirectory *himem_ems_directory = NULL;

int himem_directory_xms_handle = -1;
int himem_directory_ems_active = false;
int himem_active = false;

int himem_ems_preloaded = 0;
int himem_xms_preloaded = 0;


int himem_activate_directory(void) {
	int error_flag = false;

	if (himem_directory_allocation == MEM_EMS) {
		himem_directory_ems_active = !ems_activate_page_map();

		error_flag = !himem_directory_ems_active;
	} else if (himem_directory_allocation == MEM_NONE) {
		// Added for ScummVM, since we don't have EMS
		error_flag = true;
	}

	return error_flag;
}

int himem_get_directory_entry(int id) {
	int error_flag = true;
	long dir_offset;

	if (himem_directory_allocation == MEM_EMS) {
		if (himem_activate_directory()) goto done;
		himem_directory_entry = &himem_ems_directory[id];
	} else {
		dir_offset = ((long)sizeof(HimemDirectory)) * id;
		if (xms_copy(sizeof(HimemDirectory),
			himem_directory_xms_handle, (XMS)dir_offset,
			MEM_CONV, &himem_directory_save_area)) goto done;
	}

	error_flag = false;

done:
	return (error_flag);
}

int himem_put_directory_entry(int id) {
	int error_flag = true;
	long dir_offset;

	if (himem_directory_allocation == MEM_EMS) {
		if (himem_activate_directory()) goto done;
		if (himem_directory_entry != &himem_ems_directory[id]) {
			himem_ems_directory[id] = *himem_directory_entry;
		}
	} else {
		dir_offset = ((long)sizeof(HimemDirectory)) * id;
		if (xms_copy(sizeof(HimemDirectory),
			MEM_CONV, himem_directory_entry,
			himem_directory_xms_handle, (XMS)dir_offset)) goto done;
	}

	error_flag = false;

done:
	return error_flag;
}

int himem_resident(const char *filename) {
	int id = -1;
	int count;
	const char *mark;
	long dir_size;
	HimemDirectory *himem_xms_directory = NULL;

	if (!himem_active) goto done;

	mark = strrchr(filename, '\\');
	if (mark == NULL) mark = strchr(filename, '*');
	if (mark == NULL) {
		mark = filename;
	} else {
		mark++;
	}

	if (himem_directory_allocation == MEM_EMS) {
		if (himem_activate_directory()) goto done;
	} else {
		dir_size = ((long)sizeof(HimemDirectory)) * HIMEM_MAX_RESIDENT;

		himem_directory = himem_xms_directory = (HimemDirectory *)mem_get_name(dir_size, "$HIMEM$");
		if (himem_xms_directory == NULL) goto done;

		if (xms_copy(dir_size,
			himem_directory_xms_handle, (XMS)0,
			MEM_CONV, himem_xms_directory)) goto done;
	}

	for (count = 0; (count < HIMEM_MAX_RESIDENT) && (id < 0); count++) {
		if (himem_directory[count].memory_type == MEM_NONE) {
			if (filename == NULL) id = count;
		} else {
			if (scumm_stricmp(mark, himem_directory[count].list) == 0) {
				id = count;
			}
		}
	}

done:
	if (id >= 0) himem_get_directory_entry(id);
	if (himem_xms_directory != NULL) mem_free(himem_xms_directory);
	return id;
}

int himem_directory_setup(void) {
	int  error_flag = true;
	int  count;
	long dir_size = 0;
	HimemDirectory *himem_xms_directory = NULL;

	if (ems_paging_active) {
		himem_directory_allocation = MEM_EMS;
		himem_directory = himem_ems_directory = (HimemDirectory *) (ems_page[EMS_PAGE_MAP_PAGE] + EMS_DIRECTORY_ADDRESS);
		if (himem_activate_directory()) goto done;
	} else if (!himem_preload_xms_disabled) {
		himem_directory_allocation = MEM_XMS;
		dir_size = ((long)sizeof(HimemDirectory)) * HIMEM_MAX_RESIDENT;

		himem_directory_xms_handle = xms_get(dir_size);
		if (himem_directory_xms_handle <= 0) goto done;

		himem_directory = himem_xms_directory = (HimemDirectory *)mem_get_name(dir_size, "$HIMEM$");
		if (himem_xms_directory == NULL) goto done;

		himem_directory_entry = &himem_directory_save_area;
	} else {
		goto done;
	}

	for (count = 0; count < HIMEM_MAX_RESIDENT; count++) {
		himem_directory[count].memory_type = MEM_NONE;
	}

	if (himem_directory_allocation == MEM_XMS) {
		if (xms_copy(dir_size,
			MEM_CONV, himem_xms_directory,
			himem_directory_xms_handle, (XMS)0)) goto done;
	}

	himem_active = true;
	error_flag = false;

done:
	if (himem_xms_directory != NULL) mem_free(himem_xms_directory);
	if (error_flag) {
		himem_directory_allocation = MEM_NONE;
		if (himem_directory_xms_handle > 0) {
			xms_free(himem_directory_xms_handle);
			himem_directory_xms_handle = -1;
		}
	}

	return error_flag;
}


void himem_shutdown(void) {
	int count;

	if (!himem_activate_directory()) {
		for (count = 0; count < HIMEM_MAX_RESIDENT; count++) {
			himem_get_directory_entry(count);
			if (himem_directory_entry->memory_type == MEM_XMS) {
				xms_free(himem_directory_entry->xms_handle);
			}
		}
	}

	if (himem_directory_allocation == MEM_XMS) {
		xms_free(himem_directory_xms_handle);
	}

	himem_active = false;

	xms_umb_purge();
	ems_shutdown();
}

void himem_startup(void) {
	ems_detect();
	ems_paging_setup();

	xms_detect();

	himem_directory_setup();
}

int himem_preload(char *filename, int level) {
	int preload_handle = -1;
	int new_handle = -1;
	int page_marker = -1;
	int page_offset = EMS_PAGE_SIZE;
	int count;
	int memory_type = MEM_NONE;
	int xms_handle = -1;
	long xms_offset = 0;
	long size, page_size;
	long read_size;
	char *mark;
	byte *load_buf = NULL;
	Load handle;

	handle.open = false;

	if ((himem_resident(filename) >= 0) || !himem_active) goto done;

	if (loader_open(&handle, filename, "rb", false)) goto done;

	size = handle.decompress_size;
	page_size = (size >> 14);
	if ((page_size << 14) < size)  page_size++;

	if (ems_paging_active && !himem_preload_ems_disabled) {
		new_handle = ems_get_page_handle((word)page_size);
		if (new_handle >= 0) {
			memory_type = MEM_EMS;
			himem_directory_entry->ems_page_handle = (byte)new_handle;
		}
	}

	if ((memory_type == MEM_NONE) && !himem_preload_xms_disabled) {
		xms_handle = xms_get((word)size);
		if (xms_handle > 0) {
			new_handle = himem_resident(NULL);
			if (new_handle >= 0) {
				memory_type = MEM_XMS;
				himem_directory_entry->xms_handle = xms_handle;
				himem_directory_entry->ems_page_handle = (byte)new_handle;
			} else {
				xms_free(xms_handle);
			}
		}
	}

	if (memory_type == MEM_NONE) goto done;

	himem_directory_entry->memory_type = (byte)memory_type;
	himem_directory_entry->size = size;
	himem_directory_entry->level = (byte)level;

	himem_directory_entry->num_packets = handle.pack.num_records;
	for (count = 0; count < (int)handle.pack.num_records; count++) {
		himem_directory_entry->packet_size[count] = handle.pack.strategy[count].size;
	}

	for (count = 0; count < (int)handle.pack.num_records; count++) {
		read_size = handle.pack.strategy[count].size;
		load_buf = (byte *)mem_get_name(read_size, "$PRELOAD");
		if (load_buf == NULL) goto done;

		if (!loader_read(load_buf, read_size, 1, &handle)) goto done;

		if (memory_type == MEM_EMS) {
			if (ems_copy_it_up(new_handle, &page_marker, &page_offset, load_buf, read_size)) goto done;
		} else {

			// printf ("Record %d (Size: %ld, Offset: %ld)\n", count, read_size, xms_offset);
			if (xms_copy(read_size,
				MEM_CONV, load_buf,
				xms_handle, (XMS)xms_offset)) goto done;
			xms_offset += read_size;
		}

		mem_free(load_buf);
		load_buf = NULL;
	}

	mark = strrchr(filename, '\\');
	if (mark == NULL) mark = strchr(filename, '*');
	if (mark == NULL) {
		mark = filename;
	} else {
		mark++;
	}

	Common::strcpy_s(himem_directory_entry->list, mark);
	mads_strlwr(himem_directory_entry->list);

	preload_handle = new_handle;

#ifndef disable_statistics
	if (memory_type == MEM_EMS) {
		himem_ems_preloaded++;
	} else {
		himem_xms_preloaded++;
	}
#endif

done:
	if (load_buf != NULL) mem_free(load_buf);
	if (handle.open) loader_close(&handle);
	if (preload_handle != new_handle) {
		if (memory_type == MEM_EMS) {
			ems_free_page_handle(new_handle);
		} else if (memory_type == MEM_XMS) {
			xms_free(xms_handle);
		}
		if (new_handle >= 0) {
			himem_directory_entry->memory_type = MEM_NONE;
		}
	}
	if (new_handle >= 0) {
		himem_put_directory_entry(new_handle);
	}

	return preload_handle;
}

int himem_preload_series(const char *fname, int level) {
	int preload_handle = -1;
	int new_handle = -1;
	int page_marker = -1;
	int page_offset = EMS_PAGE_SIZE;
	int count;
	int memory_type = 0;
	int xms_handle = 0;
	int num_records;
	int might_be_series = false;
	int sprite_packed = false;
	int tile_packed = false;
	int tile_type = 0;
	int packing_flag;
	int already_unpacked = false;
	long xms_offset = 0;
	long size, page_size;
	long read_size;
	long base_offset;
	long top_of_file;
	long compressed_size;
	char *mark;
	char *marker;
	char file_buf[80];
	char temp_buf[80];
	byte *load_buf = NULL;
	byte *decompress_buffer = NULL;
	Load handle;
	FileSeriesPtr file_series = NULL;
	FileSpritePtr file_sprite = NULL;
	TileResource *tile_resource = NULL;
	Tile         *tile = NULL;

	handle.open = false;

	Common::strcpy_s(file_buf, fname);
	char *filename = file_buf;

	fileio_add_ext(filename, "SS");  // Default to Sprite Series

	if ((himem_resident(filename) >= 0) || !himem_active) goto done;

	if (loader_open(&handle, filename, "rb", false)) goto done;

	top_of_file = handle.handle->pos();

	Common::strcpy_s(temp_buf, filename);
	mads_strupr(temp_buf);
	if (strstr(temp_buf, ".SS") != NULL) might_be_series = true;
	if (strstr(temp_buf, ".TT") != NULL) {
		tile_packed = true;
		tile_type = TILE_PICTURE;
		marker = strchr(temp_buf, '.');
		if (marker != NULL) {
			marker++;
			while (*marker && (*marker == 'T')) marker++;
			if (Common::isDigit(*marker)) {
				tile_type = TILE_ATTRIBUTE;
			}
		}
	}

	size = handle.decompress_size;
	page_size = (size >> 14);
	if ((page_size << 14) < size)  page_size++;

	memory_type = MEM_NONE;
	if (ems_paging_active && !himem_preload_ems_disabled) {
		new_handle = ems_get_page_handle((word)page_size);
		if (new_handle >= 0) {
			memory_type = MEM_EMS;
			himem_directory_entry->ems_page_handle = (byte)new_handle;
		}
	}

	if ((memory_type == MEM_NONE) && !himem_preload_xms_disabled) {
		xms_handle = xms_get((word)size);
		if (xms_handle > 0) {
			new_handle = himem_resident(NULL);
			if (new_handle >= 0) {
				memory_type = MEM_XMS;
				himem_directory_entry->xms_handle = xms_handle;
			} else {
				xms_free(xms_handle);
			}
		}
	}

	if (memory_type == MEM_NONE) goto done;

	himem_directory_entry->memory_type = (byte)memory_type;
	himem_directory_entry->size = size;
	himem_directory_entry->level = (byte)level;

	himem_directory_entry->num_packets = handle.pack.num_records;
	for (count = 0; count < (int)handle.pack.num_records; count++) {
		himem_directory_entry->packet_size[count] = handle.pack.strategy[count].size;
	}

	num_records = handle.pack.num_records;

	for (count = 0; count < (int)num_records; count++) {
		read_size = handle.pack.strategy[count].size;
		load_buf = (byte *)mem_get_name(read_size, "$PRELOAD");
		if (load_buf == NULL) goto done;

		if (!loader_read(load_buf, read_size, 1, &handle)) goto done;

		if ((count == 0) && might_be_series) {
			file_series = (FileSeriesPtr)load_buf;
			sprite_packed = file_series->pack_by_sprite;
			if (sprite_packed) {
				num_records--;
			}
			file_series = NULL;
		}


		if (memory_type == MEM_EMS) {
			if (ems_copy_it_up(new_handle, &page_marker, &page_offset, load_buf, read_size)) goto done;
		} else {
			if (xms_copy(read_size,
				MEM_CONV, load_buf,
				xms_handle, (XMS)xms_offset)) goto done;
			xms_offset += read_size;
		}

		if (sprite_packed) {
			if (count == 0) {
				file_series = (FileSeriesPtr)load_buf;
				load_buf = NULL;
			} else if (count == 1) {
				file_sprite = (FileSpritePtr)load_buf;
				load_buf = NULL;
			}
		}

		if (tile_packed) {
			if (count == 0) {
				tile_resource = (TileResource *) load_buf;
				load_buf = NULL;

				if (tile_type == TILE_PICTURE) {
					tile_resource->chunk_size = (long)tile_resource->tile_x * (long)tile_resource->tile_y;
				} else {
					tile_resource->chunk_size = ((((long)tile_resource->tile_x - 1) >> 1) + 1) * (long)tile_resource->tile_y;
				}
			} else if (count == 1) {
				tile = (Tile *) load_buf;
				load_buf = NULL;
			}
		}

		mem_free(load_buf);
		load_buf = NULL;
	}

	if (sprite_packed) {
		base_offset = handle.handle->pos();

		for (count = 0; count < file_series->num_sprites; count++) {
			read_size = file_sprite[count].memory_needed;

			load_buf = (byte *)mem_get(read_size);
			if (load_buf == NULL) goto done;

			fileio_setpos(handle.handle, base_offset + file_sprite[count].file_offset);

			if (pack_data(file_series->compression, read_size,
				FROM_DISK, handle.handle,
				TO_MEMORY, load_buf) != read_size) goto done;

			if (memory_type == MEM_EMS) {
				if (ems_copy_it_up(new_handle, &page_marker, &page_offset, load_buf, read_size)) goto done;
			} else {
				if (xms_copy(read_size,
					MEM_CONV, load_buf,
					xms_handle, (XMS)xms_offset)) goto done;
				xms_offset += read_size;
			}

			mem_free(load_buf);
			load_buf = NULL;
		}
	}

	if (tile_packed) {
		base_offset = handle.handle->pos();

		for (count = 0; count < tile_resource->num_tiles; count++) {
			read_size = tile_resource->chunk_size;

			load_buf = (byte *)mem_get(read_size);
			if (load_buf == NULL) goto done;

			fileio_setpos(handle.handle, base_offset + tile[count].file_offset);

			if (count < (tile_resource->num_tiles - 1)) {
				compressed_size = tile[count + 1].file_offset - tile[count].file_offset;
			} else {
				compressed_size = (env_get_file_size(handle.handle) - PACK_OVERHEAD) -
					(tile[count].file_offset + (base_offset - top_of_file));
			}

			pack_strategy = tile_resource->compression;
			packing_flag = (tile_resource->compression != PACK_NONE) ? PACK_EXPLODE : PACK_RAW_COPY;

			if (packing_flag == PACK_EXPLODE) {
				decompress_buffer = (byte *)mem_get_name(compressed_size, "$hipack");
				if (decompress_buffer != NULL) {

					if (!fileio_fread_f(decompress_buffer, compressed_size, 1, handle.handle)) goto done;

					if (pack_data(packing_flag, read_size,
						FROM_MEMORY, decompress_buffer,
						TO_MEMORY, load_buf) != read_size) goto done;

					mem_free(decompress_buffer);
					decompress_buffer = NULL;

					already_unpacked = true;
				}
			}

			if (!already_unpacked) {
				if (pack_data(packing_flag, read_size,
					FROM_DISK, handle.handle,
					TO_MEMORY, load_buf) != read_size) goto done;
			}

			if (memory_type == MEM_EMS) {
				if (ems_copy_it_up(new_handle, &page_marker, &page_offset, load_buf, read_size)) goto done;
			} else {
				if (xms_copy(read_size,
					MEM_CONV, load_buf,
					xms_handle, (XMS)xms_offset)) goto done;
				xms_offset += read_size;
			}

			mem_free(load_buf);
			load_buf = NULL;
		}
	}


	mark = strrchr(filename, '\\');
	if (mark == NULL) mark = strchr(filename, '*');
	if (mark == NULL) {
		mark = filename;
	} else {
		mark++;
	}

	Common::strcpy_s(himem_directory_entry->list, mark);
	mads_strlwr(himem_directory_entry->list);

	preload_handle = new_handle;

#ifndef disable_statistics
	if (memory_type == MEM_EMS) {
		himem_ems_preloaded++;
	} else {
		himem_xms_preloaded++;
	}
#endif

done:
	if (decompress_buffer != NULL) mem_free(decompress_buffer);
	if (load_buf != NULL) mem_free(load_buf);
	if (file_sprite != NULL) mem_free(file_sprite);
	if (file_series != NULL) mem_free(file_series);
	if (tile != NULL) mem_free(tile);
	if (tile_resource != NULL) mem_free(tile_resource);
	if (handle.open) loader_close(&handle);
	if (preload_handle != new_handle) {
		if (memory_type == MEM_EMS) {
			ems_free_page_handle(new_handle);
		} else if (memory_type == MEM_XMS) {
			xms_free(xms_handle);
		}
		if (new_handle >= 0) {
			himem_directory_entry->memory_type = MEM_NONE;
		}
	}
	if (new_handle >= 0) {
		himem_put_directory_entry(new_handle);
	}

	return preload_handle;
}

void himem_flush(int level) {
	int count;

	if (!himem_active) return;
	if (himem_activate_directory()) return;

	for (count = 0; count < HIMEM_MAX_RESIDENT; count++) {
		himem_get_directory_entry(count);
		if (himem_directory_entry->level == (byte)level) {
			if (himem_directory_entry->memory_type == MEM_EMS) {
				ems_free_page_handle(count);
			} else if (himem_directory_entry->memory_type == MEM_XMS) {
				xms_free(himem_directory_entry->xms_handle);
			}
			himem_directory_entry->memory_type = MEM_NONE;
			himem_put_directory_entry(count);
		}
	}
}

} // namespace MADSV2
} // namespace MADS
