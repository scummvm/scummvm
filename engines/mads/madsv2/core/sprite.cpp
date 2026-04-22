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

#include "common/memstream.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/xms.h"
#include "mads/madsv2/core/fileio.h"

namespace MADS {
namespace MADSV2 {

int sprite_error = 0;
byte *sprite_force_memory = NULL;
long sprite_force_size;
byte color_table[7] = { 7, 246, 247, 248, 249, 250, 251 };
int kidney = false;


// Disable Visual studio unreferenced local variables warnings
#pragma warning(push)
#pragma warning(disable: 4101)
#pragma warning(disable: 4102)
#pragma warning(disable: 4189)

//=== sprite_draw =====================================
#define three_d         false           /* depth coding  OFF */
#define bresenham       false           /* bresenham     OFF */
#define packed_attr     false           /* Attr packing  OFF */
#define translate       false           /* 16-color      OFF */
#define interface       false           /* Interface     OFF */


void sprite_draw(SeriesPtr series, int id, Buffer *buf, int target_x, int target_y) {
	#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_scaled =====================================
#define three_d         false           /* depth coding  OFF */
#define bresenham       true            /* bresenham     ON  */
#define packed_attr     false           /* Attr packing  OFF */
#define translate       false           /* 16-color      OFF */

#define interface       false           /* Interface     OFF */

void sprite_draw_scaled(SeriesPtr series, int id, Buffer *buf,
	int target_x, int target_y, int scale_factor) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_3d_big =====================================
#define three_d         true            /* depth coding  ON  */
#define bresenham       false           /* bresenham     OFF */
#define packed_attr     true            /* Attr packing  ON  */
#define translate       false           /* 16-color      OFF */

#define interface       false           /* Interface     OFF */

void sprite_draw_3d_big(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
		int target_x, int target_y, int target_depth, int view_port_x, int view_port_y) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_3d_scaled_big =====================================
#define three_d         true            /* depth coding  ON  */
#define bresenham       true            /* bresenham     ON  */
#define packed_attr     true            /* Attr packing  ON  */
#define translate       false           /* 16-color      OFF */

#define interface       false           /* Interface     OFF */

void sprite_draw_3d_scaled_big(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
		int target_x, int target_y, int target_depth, int scale_factor,
		int view_port_x, int view_port_y) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_3d_x16 =====================================
#define three_d         true            /* depth coding  ON  */
#define bresenham       false           /* bresenham     OFF */
#define packed_attr     false           /* Attr packing  OFF */
#define translate       true            /* 16-color      ON  */

#define interface       false           /* Interface     OFF */

void sprite_draw_3d_x16(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_3d_scaled_x16 =====================================
#define three_d         true            /* depth coding  ON  */
#define bresenham       true            /* bresenham     ON  */
#define packed_attr     false           /* Attr packing  OFF */
#define translate       true            /* 16-color      ON  */

#define interface       false           /* Interface     OFF */

void sprite_draw_3d_scaled_x16(SeriesPtr series, int id,
	Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth,
	int scale_factor) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_3d_big_x16 =====================================
#define three_d         true            /* depth coding  ON  */
#define bresenham       false           /* bresenham     OFF */
#define packed_attr     true            /* Attr packing  ON  */
#define translate       true            /* 16-color      ON  */

#define interface       false           /* Interface     OFF */

void sprite_draw_3d_big_x16(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth,
	int view_port_x, int view_port_y) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_3d_big_x16 =====================================
#define three_d         true            /* depth coding  ON  */
#define bresenham       true            /* bresenham     ON  */
#define packed_attr     true            /* Attr packing  ON  */
#define translate       true            /* 16-color      ON  */

#define interface       false           /* Interface     OFF */

void sprite_draw_3d_big_x16(SeriesPtr series, int id,
	Buffer *buf, Buffer *attr, int target_x, int target_y, int target_depth,
	int scale_factor, int view_port_x, int view_port_y) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_x16 =====================================
#define three_d         false           /* depth coding  OFF */
#define bresenham       false           /* bresenham     OFF */
#define packed_attr     false           /* Attr packing  OFF */
#define translate       true            /* 16-color      ON  */

#define interface       false           /* Interface     OFF */

void sprite_draw_x16(SeriesPtr series, int id, Buffer * buf,
		int target_x, int target_y) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_interface =====================================
#define three_d         false           /* depth coding  OFF */
#define bresenham       false           /* bresenham     OFF */
#define packed_attr     false           /* Attr packing  OFF */
#define translate       false           /* 16-color      OFF */

#define interface       true            /* Interface     ON  */

void sprite_draw_interface(SeriesPtr series, int id, Buffer *buf,
	int target_x, int target_y) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

//=== sprite_draw_3d_scaled_to_attr =====================================
#define three_d         true            /* depth coding  ON  */
#define bresenham       true            /* bresenham     ON  */
#define packed_attr     true            /* Attr packing  ON  */
#define translate       false           /* 16-color      OFF */

#define interface       false           /* Interface     OFF */

#define attribute       true            /* Draw to attribute */

void sprite_draw_3d_scaled_to_attr
(SeriesPtr series, int id,
	Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth,
	int scale_factor, int view_port_x, int view_port_y) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface
#undef attribute

//=== sprite_draw_3d_scaled_mono =====================================
#define three_d         true            /* depth coding  ON  */
#define bresenham       true            /* bresenham     ON  */
#define packed_attr     false           /* Attr packing  OFF */
#define translate       false           /* 16-color      OFF */

#define interface       false           /* Interface     OFF */

#define monodraw        true            /* Draw mono color   */

void sprite_draw_3d_scaled_mono(SeriesPtr series, int id,
	Buffer *buf, Buffer *attr, int target_x, int target_y, int target_depth,
	int scale_factor, byte color) {
#include "mads/madsv2/core/sprite_0.cpp"
}

#undef three_d    
#undef bresenham
#undef packed_attr
#undef translate
#undef interface

#pragma warning(pop)

//====================================================================

void WalkerInfo::load(Load &load_handle) {
	// Load in the needed data
	byte buffer[SIZE];
	loader_read(buffer, SIZE, 1, &load_handle);

	Common::MemoryReadStream src(buffer, SIZE);
	load(&src);
}

void WalkerInfo::load(Common::SeekableReadStream *src) {
	// Read in the fields
	num_primary = src->readUint16LE();
	num_secondary = src->readUint16LE();
	src->readMultipleLE(sequence_start);
	src->readMultipleLE(sequence_stop);
	src->readMultipleLE(sequence_chance);

	velocity = src->readSint16LE();
	frame_rate = src->readByte();
	center_of_gravity = src->readByte();
}


bool FileSeries::loadHeader(Load &load_handle) {
	// Load in the needed data
	byte buffer[HEADER_SIZE];
	if (!loader_read(buffer, HEADER_SIZE, 1, &load_handle))
		return false;

	// Wrap it in a memory stream for reading convenience
	Common::MemoryReadStream src(buffer, HEADER_SIZE);

	// Read in the fields
	pack_by_sprite = src.readByte();
	compression = src.readByte();
	delta_series = src.readUint16LE();
	base_mode = src.readUint16LE();
	src.readMultipleLE(misc);
	num_sprites = src.readUint16LE();
	walker.load(&src);

	offset_x_view = src.readUint16LE();
	offset_y_view = src.readUint16LE();
	total_data_size = src.readUint32LE();

	return true;
}

void FileSprite::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(file_offset, memory_needed, x, y, xs, ys);
}

//====================================================================

SeriesPtr sprite_series_load(const char *filename, int load_flags) {
	register int count;
	int len;
	int len2;
	int found, low_color, color_pointer;
	byte *base_pointer;
	byte *sprite_marker;
	char temp_buf[80];
	char block_name[20];
	char *mark;
	long base_quantity, quantity;
	long initial_quantity;
	long total_color_size;
	long largest_block;
	long total_offset;
	FileSeries header;
	FileSpritePtr sprite = NULL;
	SeriesPtr target = NULL;
	SeriesPtr result = NULL;
	SpritePageInfoPtr  page_info;
	SpritePageTablePtr page_table;
	ColorListPtr color_list = NULL;
	/* ColorList color_list; */
	Load load_handle;

	mem_last_alloc_loader = MODULE_SPRITE_LOADER;

	load_handle.open = false;

	Common::strcpy_s(temp_buf, filename);
	if (strchr(temp_buf, '.') == NULL) {
		Common::strcat_s(temp_buf, ".SS");
	}

	Common::strcpy_s(block_name, "S$");
	mark = temp_buf;
	mads_strupr(temp_buf);
	if (*mark == '*') mark++;
	if ((*mark == 'R') && (*(mark + 1) == 'M')) {
		mark += 2;
	}
	strncat(block_name, mark, 6);


	/* Open our input file */
	if (loader_open(&load_handle, temp_buf, "rb", true)) {
		sprite_error = SS_ERR_OPENFILE;
		goto done;
	}

	/* Set default error condition */

	sprite_error = SS_ERR_READFILE;

	/* Determine length of header, and read it */

	len = sizeof(FileSeries) - sizeof(FileSprite);
	if (!header.loadHeader(load_handle))
		goto done;

	if (header.misc_is_a_walker)
		load_flags |= SPRITE_LOAD_WALKER_INFO;

	/* Determine length of index record array */

	len2 = sizeof(FileSprite) * header.num_sprites;

	base_quantity = sizeof(Series) + (sizeof(Sprite) * (header.num_sprites - 1));
	if (load_flags & SPRITE_LOAD_WALKER_INFO) {
		base_quantity += sizeof(WalkerInfo);
	}
	initial_quantity = base_quantity;
	if (header.pack_by_sprite) {
		base_quantity += (SPRITE_COLOR_TABLE_SIZE + sizeof(SpritePageInfo) + (sizeof(SpritePageTable) * header.num_sprites));
	}
	quantity = base_quantity;

	if (!(load_flags & SPRITE_LOAD_HEADER_ONLY)) {
		if (!header.pack_by_sprite) {
			quantity += header.total_data_size;
		}
	}

	/* Allocate memory for entire series (target) and for the file-formatted */
	/* index record array (sprite)                                           */

	if (sprite_force_memory != NULL) {
		if (quantity <= sprite_force_size) {
			target = (SeriesPtr)sprite_force_memory;
		}
	}

	if (target == NULL) target = (SeriesPtr)mem_get_name(quantity, block_name);
	if (target == NULL) {
		sprite_error = SS_ERR_NOMOREMEMORY;
		goto done;
	}

	sprite = (FileSpritePtr)mem_get_name(len2, "$sp-load");
	if (sprite == NULL) {
		sprite_error = SS_ERR_NOMOREMEMORY;
		goto done;
	}

	target->walker = NULL;
	target->color_table = NULL;
	target->page_info = NULL;
	target->page_table = NULL;
	target->arena = NULL;

	// Read the sprites array
	{
		byte *buffer = (byte *)malloc(header.num_sprites * FileSprite::SIZE);
		if (!loader_read(buffer, FileSprite::SIZE, header.num_sprites, &load_handle)) {
			free(buffer);
			sprite_error = SS_ERR_READFILE;
			goto done;
		}

		Common::MemoryReadStream src(buffer, header.num_sprites * FileSprite::SIZE);
		for (int i = 0; i < header.num_sprites; ++i)
			sprite[i].load(&src);

		free(buffer);
	}

	// Read the color list
	total_color_size = load_handle.pack.strategy[load_handle.pack_list_marker].size;

	color_list = (ColorListPtr)mem_get_name(total_color_size, "$color$");
	if (color_list == NULL) {
		sprite_error = SS_ERR_NOMOREMEMORY;
		goto done;
	}

	if (!color_list->load(load_handle, total_color_size))
		goto done;

	/* Copy relevant header data to target header */

	target->pack_by_sprite = header.pack_by_sprite;
	target->delta_series = header.delta_series && (header.base_mode < SS_INDIVIDUAL);
	target->base_mode = header.base_mode;
	target->num_sprites = header.num_sprites;
	target->offset_x_view = header.offset_x_view;
	target->offset_y_view = header.offset_y_view;

	for (count = 0; count < 16; count++) {
		target->misc[count] = header.misc[count];
	}

	/* Copy walker information, if requested */

	if (load_flags & SPRITE_LOAD_WALKER_INFO) {
		target->walker = (WalkerInfoPtr)(((byte *)(target)) + (initial_quantity - sizeof(WalkerInfo)));
		target->walker = (WalkerInfoPtr)mem_normalize(target->walker);
		memcpy(target->walker, &header.walker, sizeof(WalkerInfo));
	}

	/* base_pointer points to the beginning of the memory block at which */
	/* the sprite data will be loaded.                                   */

	base_pointer = (byte *) (((byte *)target) + base_quantity);
	base_pointer = (byte *)mem_normalize(base_pointer);
	sprite_marker = base_pointer;

	/* Set up the target index record for each sprite, including a pointer */
	/* to the memory block designated for its sprite data.                 */

	for (count = 0; count < target->num_sprites; count++) {
		target->index[count].x = sprite[count].x;
		target->index[count].y = sprite[count].y;
		target->index[count].xs = sprite[count].xs;
		target->index[count].ys = sprite[count].ys;
		if (!(load_flags & SPRITE_LOAD_HEADER_ONLY) && !header.pack_by_sprite) {
			target->index[count].data = sprite_marker;
			sprite_marker = (byte *)mem_normalize(sprite_marker + sprite[count].memory_needed);
		} else {
			target->index[count].data = NULL;
		}
	}

	/* Load all of the sprite data in at the base address */

	if (!(load_flags & SPRITE_LOAD_HEADER_ONLY) && !header.pack_by_sprite) {
		if (!loader_read(base_pointer, header.total_data_size, 1, &load_handle)) goto done;
	}

	/* Perform palette allocation and color list transformation */

	if (load_flags & (SPRITE_LOAD_TRANSLATE | SPRITE_LOAD_SPINNING_OBJECT)) {
		target->color_handle = 0;
		if (load_flags & (SPRITE_LOAD_SPINNING_OBJECT)) {
			color_pointer = 0;
			for (count = 0; count < color_list->num_colors; count++) {
				found = false;
				for (low_color = 0; !found && (low_color < 4); low_color++) {
					if (memcmp(&color_list->table[count].r, &master_palette[low_color].r, sizeof(RGBcolor)) == 0) {
						found = true;
						color_list->table[count].x16 = (byte)low_color;
					}
				}
				if (!found) {
					memcpy(&master_palette[color_table[color_pointer]].r,
						&color_list->table[count].r, sizeof(RGBcolor));
					color_list->table[count].x16 = (byte)color_table[color_pointer];
					color_pointer = MIN(6, color_pointer + 1);
				}
			}
			sprite_color_translate(target, color_list);
		}
	} else {

		target->color_handle = pal_allocate(color_list, master_shadow, load_flags & PAL_MAP_MASK);
		if (target->color_handle < 0) {
			sprite_error = SS_ERR_TOOMANYCOLORS;
			goto done;
		}

		if (!(load_flags & SPRITE_LOAD_HEADER_ONLY) && !header.pack_by_sprite) {
			sprite_color_translate(target, color_list);
		}
	}

	/* If series was packed by sprite (and we are therefore expected to */
	/* stream-load it), we need to set up paging tables for the series. */
	/* This process is different depending on whether the sprite data   */
	/* is to be loaded from disk or from the EMS preload area.          */

	if (header.pack_by_sprite) {
		target->color_table = ((byte *)target) + initial_quantity;
		page_info = target->page_info = (SpritePageInfoPtr)(((byte *)target->color_table) + SPRITE_COLOR_TABLE_SIZE);
		page_table = target->page_table = (SpritePageTablePtr)(((byte *)page_info) + sizeof(SpritePageInfo));

		for (count = 0; count < color_list->num_colors; count++) {
			target->color_table[count] = color_list->table[count].x16;
		}

		page_info->packing_mode = header.compression;
		page_info->paging_source = (byte)load_handle.mode;

		if ((page_info->paging_source == LOADER_EMS) ||
			(page_info->paging_source == LOADER_XMS)) {
			page_info->ems_handle = load_handle.ems_handle;
			page_info->ems_page_marker = load_handle.ems_page_marker;
			page_info->ems_page_offset = load_handle.ems_page_offset;
			page_info->xms_handle = load_handle.xms_handle;
			page_info->xms_offset = load_handle.xms_offset;
			largest_block = 0;
			total_offset = 0;
			for (count = 0; count < target->num_sprites; count++) {
				page_table[count].file_offset = total_offset;
				page_table[count].memory_needed = sprite[count].memory_needed;
				total_offset += page_table[count].memory_needed;
				largest_block = MAX(largest_block, page_table[count].memory_needed);
			}
		} else {
			page_info->handle = load_handle.handle;
			total_offset = load_handle.handle->pos();
			page_info->base_sprite_offset = total_offset;

			largest_block = 0;
			for (count = 0; count < target->num_sprites; count++) {
				page_table[count].file_offset = sprite[count].file_offset;
				page_table[count].memory_needed = sprite[count].memory_needed;
				largest_block = MAX(largest_block, page_table[count].memory_needed);
			}
		}

		if (!(load_flags & SPRITE_LOAD_HEADER_ONLY)) {
			if (color_list != NULL) mem_free(color_list);
			color_list = NULL;

			if (sprite != NULL) mem_free(sprite);
			sprite = NULL;

			target->arena = (byte *)mem_get_name(largest_block, "$arena$");
			if (target->arena == NULL) goto done;

			memcpy(&target->misc_largest_block, &largest_block, sizeof(long));

			load_handle.open = false;   /* Hack handle so it won't get closed */
		} else {
			target->arena = NULL;
		}
	}

	result = target;

done:
	if (load_handle.open) loader_close(&load_handle);
	if (color_list != NULL) mem_free(color_list);
	if (sprite != NULL) mem_free(sprite);
	if ((target != NULL) && (target != (SeriesPtr)sprite_force_memory) && (result == NULL)) mem_free(target);

	return (result);
}

void sprite_get_scaled_matte(SeriesPtr series, int id, int target_x, int target_y,
	int scale_factor, SpritePtr matte) {
	word xs, ys;
	SpritePtr sprite;

	sprite = &series->index[id - 1];

	xs = sprite->xs;
	ys = sprite->ys;

	xs = ((xs * scale_factor) + 50) / 100;
	ys = ((ys * scale_factor) + 50) / 100;

	matte->xs = xs;
	matte->ys = ys;

	matte->x = target_x - (xs >> 1);
	matte->y = (target_y - ys) + 1;
}

word sprite_pack_line_rle(byte *target, Buffer *source, byte *palette_map, byte transparent, int x1, int x2, int y) {
	byte run_byte, run_len;
	byte *from;
	byte *unto;
	byte b;
	int  a, rec, lastpel;

	unto = target;
	run_byte = SS_IRLE;       /* Shouldn't match ANY pixels, forces ... */
	/* start of new run with first pixel.     */
	run_len = 0;

	/* find logical end of line */
	from = (byte *)(source->data + x2 + y * source->x);
	for (lastpel = x2; lastpel >= x1; lastpel--, from--)
		if (*from != transparent)
			break;
	lastpel++;

	if (lastpel == x1)  /* If no pels on line, don't even mark line type */
	{
		*(unto++) = SS_EOL;
		return((word)(unto - target));
	}

	*(unto++) = SS_RLE;  /* Mark as a RLE type line */

	from = (byte *) (source->data + x1 + y * source->x);
	for (a = x1; a <= x2; a++, from++) {
		/* if no more real pixels */
		if (a == lastpel) {
			*(unto++) = run_len;
			*(unto++) = run_byte;
			*(unto++) = SS_EOL;
			return ((word)(unto - target));
		}

		b = *from;

		if (b == transparent) {
			rec = SS_SKIP;
		} else {
			rec = palette_map[b];      /* Translate to relative color    */
		}

		b = (byte)rec;

		/* if a new byte, write run and clear */
		if (b != run_byte) {
			/* if run exists, write it */
			if (run_len > 0) {
				*(unto++) = run_len;
				*(unto++) = run_byte;
			}
			run_byte = b;       /* set up for new run */
			run_len = 1;
		} else {
			/* if same byte as current run */
			/* if MAX len, write and reset */
			if (run_len == SS_LASTVALID) {
				*(unto++) = run_len;
				*(unto++) = run_byte;
				run_len = 0;
			}
			run_len++;
		}
	}
	*(unto++) = run_len;
	*(unto++) = run_byte;
	*(unto++) = SS_EOL;

	return (word)(unto - target);
}

word sprite_pack_line_irle(byte *target, Buffer *source, byte *palette_map, byte transparent, int x1, int x2, int y) {
	byte run_byte, run_len;
	byte *from, *from1;
	byte *unto;    /* 'TO' is reserved word in other languages */
	byte b;
	int  a, rec, lastpel;

	unto = target;
	run_byte = SS_IRLE;       /* Shouldn't match ANY pixels, forces ... */
	/* start of new run with first pixel.     */
	run_len = 0;

	/* find logical end of line */
	from1 = from = (byte *)(source->data + x2 + y * source->x);

	for (lastpel = x2; lastpel >= x1; lastpel--, from--)
		if (*from != transparent)
			break;
	lastpel++;

	/* If no pels on line, don't even mark line type */
	if (lastpel == x1) {
		*(unto++) = SS_EOL;
		return((word)(unto - target));
	}

	*(unto++) = SS_IRLE;  /* Mark as an IRLE type line */

	from = (byte *)(source->data + x1 + y * source->x);

	for (a = x1; a <= x2; a++, from++) {
		/* if no more real pixels */
		if (a == lastpel) {
			switch (run_len) {
			case 3: *(unto++) = run_byte;
			case 2: *(unto++) = run_byte;
			case 1: *(unto++) = run_byte;    break;
			default:
				*(unto++) = SS_RUN;      /* mark as a run */
				*(unto++) = run_len;
				*(unto++) = run_byte;
			}

			*(unto++) = SS_EOL;
			return (word)(unto - target);
		}

		b = *from;

		if (b == transparent)
			rec = SS_SKIP;
		else
			rec = palette_map[b];        /* Translate to relative color */

		b = (byte)rec;

		/* if a new byte, write run and clear */
		if (b != run_byte) {
			/* if run exists, write it */
			if (run_len > 0) {
				switch (run_len) {
				case 3: *(unto++) = run_byte;
				case 2: *(unto++) = run_byte;
				case 1: *(unto++) = run_byte;    break;
				default:
					*(unto++) = SS_RUN;      /* mark as a run */
					*(unto++) = run_len;
					*(unto++) = run_byte;
				}
			}

			run_byte = b;       /* set up for new run */
			run_len = 1;
		} else {
			/* if same byte as current run */
			if (run_len == SS_LASTVALID) {
				/* if MAX len, write and reset */
				*(unto++) = SS_RUN;
				*(unto++) = run_len;
				*(unto++) = run_byte;
				run_len = 0;
			}

			run_len++;
		}
	}

	switch (run_len) {
	case 3: *(unto++) = run_byte;  /* intentional fall-through */
	case 2: *(unto++) = run_byte;
	case 1: *(unto++) = run_byte;    break;
	default:
		*(unto++) = SS_RUN;      /* mark as a run */
		*(unto++) = run_len;
		*(unto++) = run_byte;
	}

	*(unto++) = SS_EOL;
	*from1 = 15;

	return (word)(unto - target);
}

void sprite_set_bounds(HagSpritePtr sprite, int x, int y, int xs, int ys) {
	sprite->x = x;
	sprite->y = y;
	sprite->xs = xs;
	sprite->ys = ys;
}

void sprite_merge_background(Buffer *source, Buffer *background, byte transparent) {
	byte *source_ptr = source->data;
	byte *delta_ptr = background->data;
	word  total_size = (word)source->x * (word)source->y;

	for (word i = 0; i < total_size; i++) {
		if (source_ptr[i] == transparent) {
			byte bg = delta_ptr[i];
			if (bg != (byte)-1)
				source_ptr[i] = bg;
		}
	}
}

void sprite_delta_compute(HagSpritePtr sprite, Buffer *source, Buffer *delta) {
	int ul_x = 10000, ul_y = 10000;
	int lr_x = -1, lr_y = -1;
	int wrap = source->x;
	int done = source->y;
	byte *source_ptr = source->data;
	byte *delta_ptr = delta->data;

	for (int y = 0; y < done; y++) {
		for (int x = 0; x < wrap; x++) {
			int i = y * wrap + x;
			byte src = source_ptr[i];
			byte dlt = delta_ptr[i];

			if (src != dlt) {
				delta_ptr[i] = src;
				source_ptr[i] = COLOR_TRANSPARENT;

				if (x < ul_x) ul_x = x;
				if (x > lr_x) lr_x = x;
				if (y < ul_y) ul_y = y;
				if (y > lr_y) lr_y = y;
			}
		}
	}

	if (ul_x > (wrap - 1)) {
		ul_x = ul_y = 0;
		lr_x = lr_y = -1;
	}
	sprite->x = ul_x;
	sprite->xs = (lr_x - ul_x) + 1;
	sprite->y = ul_y;
	sprite->ys = (lr_y - ul_y) + 1;
}

void sprite_remove_non_delta(Buffer *source, Buffer *delta, byte transparent) {
	byte *source_ptr = source->data;
	byte *delta_ptr = delta->data;
	int   total_size = source->x * source->y;

	for (int i = 0; i < total_size; i++) {
		byte src = source_ptr[i];
		byte dlt = delta_ptr[i];

		if (src != dlt)
			delta_ptr[i] = src;
		else
			source_ptr[i] = transparent;
	}
}

long sprite_pack_image(byte *target, FileSpritePtr sprite, Buffer *source, byte *palette_map, byte transparent) {
	word size1, size2;
	byte buf1[2048];
	byte buf2[1024];
	byte *target_ptr;
	int count_y;
	int x, y, xs, ys;
	long out;

	target_ptr = target;
	out = 0;

	x = sprite->x;
	y = sprite->y;
	xs = sprite->xs;
	ys = sprite->ys;

	for (count_y = y; count_y < y + ys; count_y++) {
		size1 = sprite_pack_line_rle(buf1, source, palette_map, transparent, x, x + xs - 1, count_y);
		size2 = sprite_pack_line_irle(buf2, source, palette_map, transparent, x, x + xs - 1, count_y);

		if (size1 < size2) {
			memcpy(target_ptr, buf1, size1);
			target_ptr += size1;
			out += size1;
		} else {
			memcpy(target_ptr, buf2, size2);
			target_ptr += size2;
			out += size2;
		}
	}

	*(target_ptr++) = SS_EOI;
	out++;

	return out;
}

void sprite_color_translate(SeriesPtr series, ColorListPtr list) {
	byte *table = &list->table[0].x16;

	for (int id = 0; id < series->num_sprites; id++) {
		byte *scan = series->index[id].data;
		if (scan == NULL)
			goto done;

		while (1) { /* scan_main_loop */
			byte ctrl = *scan++;

			if (ctrl == SS_EOI) break;       /* scan_finished */
			if (ctrl == SS_EOL) continue;    /* scan_main_loop */

			if (ctrl == SS_RLE) {
				/* scan_rle_main_loop */
				while (1) {
					byte len = *scan++;
					if (len == SS_EOL)
						break; /* back to scan_main_loop */

					if (*scan != SS_SKIP) {
						byte color = *scan;
						int  idx = color * 6;
						*scan = table[idx];
					}
					scan++;
				}
				continue;
			}

			/* scan_irle_main_loop */
			while (1) {
				byte b = *scan;

				if (b == SS_EOL) {
					scan++; /* scan_irle_finished: consume EOL and return to outer */
					break;
				}

				if (b == SS_RUN) {
					scan += 2;      /* skip RUN marker and count */
					b = *scan;      /* get the run's color value */
				}

				if (b != SS_SKIP) {
					int idx = b * 6;
					*scan = table[idx];
				}
				scan++;
			}
		}
	}

done:
	;
}

void sprite_single_color_translate(SeriesPtr series, int id) {
	id -= 1;
	byte *table = series->color_table;
	byte *scan = series->index[id].data;
	if (scan == NULL)
		goto done;

	while (1) { /* scan_main_loop */
		byte ctrl = *scan++;

		if (ctrl == SS_EOI) break;
		if (ctrl == SS_EOL) continue;

		if (ctrl == SS_RLE) {
			while (1) {
				byte len = *scan++;
				if (len == SS_EOL)
					break;

				if (*scan != SS_SKIP)
					*scan = table[*scan];
				scan++;
			}
			continue;
		}

		/* IRLE */
		while (1) {
			byte b = *scan;

			if (b == SS_EOL) {
				scan++;
				break;
			}

			if (b == SS_RUN) {
				scan += 2;
				b = *scan;
			}

			if (b != SS_SKIP)
				*scan = table[b];
			scan++;
		}
	}

done:
	;
}

int sprite_data_load(SeriesPtr series, int id, byte *target) {
	int error_flag = true;
	int ems_page_marker = -1;
	int ems_page_offset;
	int count;
	int already_unpacked = false;
	long my_offset;
	long my_ems_marker;
	long my_ems_offset;
	long decompress_size;
	byte *decompress_buffer = NULL;
	SpritePageInfoPtr  page_info;
	SpritePageTablePtr page_table;

	id -= 1;

	page_info = series->page_info;
	page_table = series->page_table;

	if (page_table[id].memory_needed == 0) goto ok;

	if (page_info->paging_source == LOADER_EMS) {
		my_offset = page_table[id].file_offset;
		my_ems_marker = my_offset / EMS_PAGE_SIZE;
		my_ems_offset = my_offset - (my_ems_marker * EMS_PAGE_SIZE);
		for (count = 0; count < (int)my_ems_marker; count++) {
			ems_page_marker = ems_next_handle_page(page_info->ems_handle, ems_page_marker);
		}
		ems_page_offset = (int)my_ems_offset + page_info->ems_page_offset;
		if (ems_page_offset >= EMS_PAGE_SIZE) {
			ems_page_offset -= EMS_PAGE_SIZE;
			ems_page_marker = ems_next_handle_page(page_info->ems_handle, ems_page_marker);
		}

		if (!ems_copy_it_down(page_info->ems_handle,
			&ems_page_marker,
			&ems_page_offset,
			target,
			page_table[id].memory_needed)) goto done;
	} else if (page_info->paging_source == LOADER_XMS) {
		my_offset = page_table[id].file_offset;
		if (!xms_copy(page_table[id].memory_needed,
			page_info->xms_handle, (XMS)(page_info->xms_offset + my_offset),
			MEM_CONV, target)) goto done;
	} else {
		my_offset = page_info->base_sprite_offset +
			page_table[id].file_offset;

		fileio_setpos(page_info->handle, my_offset);

		pack_strategy = (page_info->packing_mode == PACK_EXPLODE) ? PACK_PFAB : PACK_NONE;

		if (id < series->num_sprites - 1) {
			decompress_size = page_table[id + 1].file_offset - page_table[id].file_offset;
			decompress_buffer = (byte *)mem_get_name(decompress_size, "$sp_data");
			if (decompress_buffer != NULL) {
				if (!fileio_fread_f(decompress_buffer, decompress_size, 1, page_info->handle)) goto done;

				if (pack_data(page_info->packing_mode, page_table[id].memory_needed,
					FROM_MEMORY, decompress_buffer,
					TO_MEMORY, target) != page_table[id].memory_needed) goto done;

				mem_free(decompress_buffer);
				decompress_buffer = NULL;

				already_unpacked = true;
			}
		}

		if (!already_unpacked) {
			if (pack_data(page_info->packing_mode, page_table[id].memory_needed,
				FROM_DISK, page_info->handle,
				TO_MEMORY, target) != page_table[id].memory_needed) goto done;
		}
	}

ok:
	series->index[id].data = target;

	sprite_single_color_translate(series, id + 1);

	error_flag = false;

done:
	if (decompress_buffer != NULL) mem_free(decompress_buffer);
	return (error_flag);
}


void dont_frag_the_palette(void) {
	/* this will tell sprite_free to not execute pal_deallocate. This will */
	/* make it so the colors are not freed and you won't get the */
	/* palette fragging. */

	kidney = true;
}

void go_ahead_and_frag_the_palette(void) {
	kidney = false;
}

void sprite_free(SeriesPtr *series, int free_memory) {
	if (*series == NULL) goto done;

	if ((*series)->pack_by_sprite) {
		if ((*series)->page_info->paging_source == LOADER_DISK) {
			delete (*series)->page_info->handle;
		}
		if ((*series)->arena != NULL) {
			if (free_memory)
				mem_free((*series)->arena);
		}
	}

	if (kidney) {
		/* release the flag - dont deallocate the colors from the list */
		if ((*series)->color_handle)
			pal_deallocate((*series)->color_handle);
		/* flag_used[(*series)->color_handle] = false; */
	} else {
		/* deallocate the colors from the list */
		if ((*series)->color_handle)
			pal_deallocate((*series)->color_handle);
	}

	if (free_memory)
		mem_free(*series);
	*series = NULL;

done:
	;
}

} // namespace MADSV2
} // namespace MADS
