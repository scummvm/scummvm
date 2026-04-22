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
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/sort.h"
#include "mads/madsv2/core/mem.h"

namespace MADS {
namespace MADSV2 {

void Color::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(r, g, b, x16, cycle, group);
}

bool ColorList::load(Load &load_handle, int size) {
	// Load in the needed data
	byte *buffer = (byte *)malloc(size);
	bool result = loader_read(buffer, size, 1, &load_handle);

	if (result) {
		Common::MemoryReadStream src(buffer, size);

		num_colors = src.readUint16LE();
		for (int i = 0; i < num_colors; ++i)
			table[i].load(&src);
	}

	free(buffer);
	return result;
}

void ColorList::load(Common::SeekableReadStream *src) {
	num_colors = src->readUint16LE();

	for (int i = 0; i < COLOR_MAX_USER_COLORS; ++i)
		table[i].load(src);
}

void Cycle::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(num_colors, first_list_color, first_palette_color, ticks);
}

void CycleList::load(Common::SeekableReadStream *src) {
	num_cycles = src->readUint16LE();

	for (int i = 0; i < COLOR_MAX_CYCLES; ++i)
		table[i].load(src);
}

void ShadowList::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(num_shadow_colors);
	for (int i = 0; i < COLOR_MAX_SHADOW_COLORS; ++i)
		shadow_color[i] = src->readSint16LE();
}

//====================================================================

byte color_thatch(int color, int thatching) {
	byte thatch;

	if (color == 16) {
		thatch = COLOR_NO_X16;
	} else if (color == thatching) {
		thatch = (byte)color;
	} else {
		if (thatching == 0) {
			thatch = (byte)(((COLOR_BLACK_THATCH) << 4) + color);
		} else {
			thatch = (byte)((thatching << 4) + color);
		}
	}

	return (thatch);
}

void color_list_start_scan(byte *list_flags) {
	int count;

	for (count = 0; count < COLOR_MAX_USER_COLORS; count++) {
		list_flags[count] = false;
	}
}

int color_list_update(ColorListPtr list, Buffer *scan_buf, Palette *scan_pal, byte *list_flags, byte *palette_map, CycleListPtr cycle) {
	int count;
	int row, col;
	int found;
	int result;
	int hi, lo;
	int num;
	int cycle_color;
	int cycle_count;
	byte color_flags[256];
	byte *scan;
	byte color;
	byte r, g, b;
	word cycle_value;
	word value[256];
	byte reverse_map[256];
	ColorList temp_list;

	result = 0;

	for (count = 0; count <= COLOR_HIGHEST; count++) {
		color_flags[count] = (byte)false;
	}

	for (count = COLOR_HIGHEST + 1; count < 256; count++) {
		color_flags[count] = (byte)true;
	}

	// First, deal with any colors involved in cycling
	if (cycle != NULL) {
		for (cycle_count = 0; cycle_count < cycle->num_cycles; cycle_count++) {
			lo = cycle->table[cycle_count].first_palette_color;
			num = cycle->table[cycle_count].num_colors;
			hi = lo + num - 1;
			for (cycle_color = lo; cycle_color <= hi; cycle_color++) {
				if (!color_flags[cycle_color]) {
					color_flags[cycle_color] = true;
					r = (*scan_pal)[cycle_color].r;
					g = (*scan_pal)[cycle_color].g;
					b = (*scan_pal)[cycle_color].b;
					found = false;
					
					if (!found) {
						if (list->num_colors < COLOR_MAX_USER_COLORS) {
							count = list->num_colors;
							list->table[count].r = r;
							list->table[count].g = g;
							list->table[count].b = b;
							list->table[count].x16 = (byte)COLOR_NO_X16;
							list->table[count].cycle = (byte)(cycle_count + 1);
							list->table[count].group = 0;
							list_flags[count] = true;
							if (palette_map != NULL) {
								palette_map[cycle_color] = (byte)count;
							}
							reverse_map[count] = (byte)cycle_color;
							list->num_colors++;
						} else {
							result = COLOR_ERR_LISTOVERFLOW;
						}
					}
				}
			}
		}
	}

	// Now deal with any other colors used in the picture
	scan = (byte *)mem_normalize(scan_buf->data);

	for (row = 0; row < scan_buf->y; row++) {
		for (col = 0; col < scan_buf->x; col++) {
			color = *scan;
			if (!color_flags[color]) {
				r = (*scan_pal)[color].r;
				g = (*scan_pal)[color].g;
				b = (*scan_pal)[color].b;
				found = false;
				for (count = 0; (count < list->num_colors) && (!found); count++) {
					if ((r == list->table[count].r) &&
						(g == list->table[count].g) &&
						(b == list->table[count].b) &&
						(list->table[count].cycle == 0)) {
						found = true;
						list_flags[count] = true;
						if (palette_map != NULL) {
							palette_map[color] = (byte)count;
						}
						reverse_map[count] = color;
					}
				}
				if (!found) {
					if (list->num_colors < COLOR_MAX_USER_COLORS) {
						count = list->num_colors;
						list->table[count].r = r;
						list->table[count].g = g;
						list->table[count].b = b;
						list->table[count].x16 = (byte)COLOR_NO_X16;
						list->table[count].cycle = 0;
						list->table[count].group = 0;
						list_flags[count] = true;
						if (palette_map != NULL) {
							palette_map[color] = (byte)count;
						}
						reverse_map[count] = color;
						list->num_colors++;
					} else {
						result = COLOR_ERR_LISTOVERFLOW;
					}
				}
				color_flags[color] = (byte)true;
			}
			scan++;
		}
		scan = (byte *)mem_check_overflow(scan);
	}

	// Sort color list so that cycle colors appear first
	// The various tables must also be corrected.
	if (cycle != NULL) {
		for (count = 0; count < list->num_colors; count++) {
			cycle_value = list->table[count].cycle;
			if ((cycle_value == 0) || (!list_flags[count])) cycle_value = 100;
			value[count] = (cycle_value << 8) + reverse_map[count];
			color_flags[count] = (byte)count;
		}
		sort_insertion_16(list->num_colors, color_flags, value);
		temp_list.num_colors = list->num_colors;
		for (count = 0; count < list->num_colors; count++) {
			memcpy(&temp_list.table[count], &list->table[color_flags[count]], sizeof(Color));
		}
		if (palette_map != NULL) {
			for (count = 0; count < 256; count++) {
				palette_map[count] = color_flags[palette_map[count]];
			}
		}
		for (count = 0; count < list->num_colors; count++) {
			value[count] = list_flags[count];
		}
		for (count = 0; count < list->num_colors; count++) {
			list_flags[count] = (byte)value[color_flags[count]];
		}
		memcpy(list, &temp_list, sizeof(ColorList));

		// Switch cycle list color #'s from palette to list
		for (cycle_count = 0; cycle_count < cycle->num_cycles; cycle_count++) {
			found = false;
			for (count = 0; !found && (count < list->num_colors); count++) {
				if (list->table[count].cycle == (byte)(cycle_count + 1)) {
					found = true;
					cycle->table[cycle_count].first_list_color = (byte)count;
				}
			}
		}
	}

	if (result == 0) {
		result = list->num_colors;
	}

	return result;
}

int color_list_purge(ColorListPtr list, byte *list_flags) {
	int count, count2;

	for (count = 0; count < list->num_colors; count++) {
		if (!list_flags[count]) {
			for (count2 = count; count2 < list->num_colors - 1; count2++) {
				list_flags[count2] = list_flags[count2 + 1];
				memcpy(&list->table[count2], &list->table[count2 + 1], sizeof(Color));
			}
			list->num_colors--;
			count--;
		}
	}

	return list->num_colors;
}

int color_list_palette(ColorListPtr list, Buffer *scan_buf, Palette *scan_pal, int base_color, byte *palette_map, CycleListPtr cycle) {
	int error_flag = false;
	int mark;
	int count, count2;
	int cycle_count;
	int cycle_id;
	int row, col;
	int map_me;
	int stop_doing_this;
	byte map[256];
	byte flags[256];
	byte r, g, b;
	byte *scan;
	RGBcolor *c2ptr;
	Color *c1ptr;
	Palette out_pal;

	// Make a copy of our starting palette
	memcpy(out_pal, *scan_pal, sizeof(RGBcolor) * 256);

	// Mark all list colors as unmapped
	for (count = 0; count < 256; count++) {
		flags[count] = (byte)false;
	}

	mark = base_color;
	for (count = 0; count < list->num_colors; count++) {
		if (mark > 255) {
			mark = 255;
			error_flag = true;
		}
		c1ptr = &list->table[count];
		c2ptr = &out_pal[mark];
		memcpy(c2ptr, c1ptr, sizeof(RGBcolor));
		palette_map[mark] = (byte)count;
		stop_doing_this = false;
		for (count2 = 0; (count2 <= COLOR_HIGHEST) && (!stop_doing_this); count2++) {
			r = (*scan_pal)[count2].r;
			g = (*scan_pal)[count2].g;
			b = (*scan_pal)[count2].b;
			if ((r == c2ptr->r) && (g == c2ptr->g) && (b == c2ptr->b)) {
				if (cycle != NULL) {
					cycle_id = 0;
					for (cycle_count = 0; cycle_count < cycle->num_cycles; cycle_count++) {
						if ((count2 >= (int)cycle->table[cycle_count].first_palette_color) &&
							(count2 <= (int)(cycle->table[cycle_count].first_palette_color + (cycle->table[cycle_count].num_colors - 1)))) {
							cycle_id = cycle_count + 1;
						}
					}
					map_me = (list->table[count].cycle == (byte)cycle_id);
					if (cycle_id != 0) {
						map_me &= (!flags[count2]);
						stop_doing_this = map_me;
					}
				} else {
					map_me = true;
				}
				if (map_me) {
					map[count2] = (byte)mark;
					flags[count2] = (byte)true;
				}
			}
		}
		mark++;
	}

	map[COLOR_TRANSPARENT] = COLOR_TRANSPARENT;

	scan = (byte *)mem_normalize(scan_buf->data);

	for (row = 0; row < scan_buf->y; row++) {
		for (col = 0; col < scan_buf->x; col++) {
			*scan = map[*scan];
			scan++;
		}
		scan = (byte *)mem_check_overflow(scan);
	}
	memcpy(*scan_pal, out_pal, sizeof(RGBcolor) * 256);

	if (cycle != NULL) {
		mark = base_color;
		for (cycle_count = 0; cycle_count < cycle->num_cycles; cycle_count++) {
			cycle->table[cycle_count].first_palette_color = (byte)mark;
			mark += cycle->table[cycle_count].num_colors;
		}
	}

	return error_flag;
}

void color_list_conform(ColorListPtr list, Buffer *scan_buf, Palette *scan_pal, Palette *main_pal, int base_color) {
	int mark;
	int count, count2;
	int row, col;
	int found;
	int finish;
	byte map[256];
	byte r, g, b;
	byte *scan;
	Palette out_pal;

	memcpy(out_pal, *main_pal, sizeof(RGBcolor) * 256);

	mark = base_color + list->num_colors;
	for (count = 0; count <= COLOR_HIGHEST; count++) {
		r = (*scan_pal)[count].r;
		g = (*scan_pal)[count].g;
		b = (*scan_pal)[count].b;
		found = false;
		finish = (count < 16) ? 0 : COLOR_FIRST_USER_COLOR - 1;
		for (count2 = COLOR_FIRST_USER_COLOR; (!found) && (count2 != finish); ) {
			if ((r == out_pal[count2].r) &&
				(g == out_pal[count2].g) &&
				(b == out_pal[count2].b)) {
				found = true;
				map[count] = (byte)count2;
			}
			count2 = (count2 + 1) % mark;
		}
		if (!found) {
			if (mark < 255) {
				out_pal[mark].r = r;
				out_pal[mark].g = g;
				out_pal[mark].b = b;
				map[count] = (byte)mark;
				mark++;
			} else {
				map[count] = 15;
			}
		}
	}

	for (count = 0; count < 16; count++) {
		map[count] = (byte)count;
	}

	scan = (byte *)mem_normalize(scan_buf->data);

	map[COLOR_TRANSPARENT] = COLOR_TRANSPARENT;

	for (row = 0; row < scan_buf->y; row++) {
		for (col = 0; col < scan_buf->x; col++) {
			*scan = map[*scan];
			scan++;
		}
		scan = (byte *)mem_check_overflow(scan);
	}

	memcpy(*main_pal, out_pal, sizeof(RGBcolor) * 256);
}

void color_trans_show_buf(Buffer from, Buffer unto,
	int from_x, int from_y,
	int unto_x, int unto_y,
	int size_x, int size_y,
	ColorListPtr list,
	byte *palette_map, int mask_flag) {
	int row, col;
	int hi;
	byte *source, *dest;
	byte *source_scan, *dest_scan;
	byte item;
	byte trans;
	byte thatch;
	int color, thatching;

	source = (byte *)mem_normalize(from.data);
	for (row = 0; row < from_y; row++) {
		source = (byte *)mem_check_overflow(source + from.x);
	}
	source = source + from_x;

	dest = (byte *)mem_normalize(unto.data);
	for (row = 0; row < unto_y; row++) {
		dest = (byte *)mem_check_overflow(dest + unto.x);
	}
	dest = dest + unto_x;

	for (row = 0; row < size_y; row++) {
		source_scan = source;
		dest_scan = dest;
		for (col = 0; col < size_x; col++) {
			item = *source_scan;
			trans = list->table[palette_map[item]].x16;
			color_split_thatch((int)trans, &color, &thatching);
			if (item == COLOR_TRANSPARENT) color = 16;
			if (color != 16) {
				hi = ((row + from_y) ^ (col + from_x)) & 1;
				if (hi) {
					thatch = (byte)thatching;
				} else {
					thatch = (byte)color;
				}
				if (mask_flag) {
					*dest_scan = hi_white;
				} else {
					*dest_scan = thatch;
				}
			} else {
				if (mask_flag) {
					*dest_scan = black;
				} else {
					*dest_scan = item;
				}
			}
			source_scan++;
			dest_scan++;
		}
		source = (byte *)mem_check_overflow(source + from.x);
		dest = (byte *)mem_check_overflow(dest + unto.x);
	}
}

void color_trans_fill_buf(Buffer unto, int unto_x, int unto_y,
		int size_x, int size_y, byte thatch_color) {
	int row, col;
	int hi;
	int color, thatching;
	byte thatch[2];
	byte *dest;
	byte *dest_scan;

	dest = (byte *)mem_normalize(unto.data);
	for (row = 0; row < unto_y; row++) {
		dest = (byte *)mem_check_overflow(dest + unto.x);
	}
	dest += unto_x;

	color_split_thatch((int)thatch_color, &color, &thatching);
	thatch[0] = (byte)color;
	thatch[1] = (byte)thatching;
	if (thatch[0] == 16) {
		thatch[0] = 0;
		thatch[1] = 0;
	}

	for (row = 0; row < size_y; row++) {
		dest_scan = dest;
		for (col = 0; col < size_x; col++) {
			hi = ((row + unto_y) ^ (col + unto_x)) & 1;
			*dest_scan = thatch[hi];
			dest_scan++;
		}
		dest = (byte *)mem_check_overflow(dest + unto.x);
	}
}

void color_buffer_palette_to_list(ColorListPtr list, Buffer *scan_buf, byte *palette_map) {
	int row, col;
	byte *scan_ptr;
	byte scan;

	scan_ptr = (byte *)mem_normalize(scan_buf->data);

	for (row = 0; row < scan_buf->y; row++) {
		for (col = 0; col < scan_buf->x; col++) {
			scan = *scan_ptr;
			*scan_ptr = palette_map[scan];
			scan_ptr++;
		}
		scan_ptr = (byte *)mem_check_overflow(scan_ptr);
	}
}

void color_buffer_list_to_palette(Buffer *scan_buf, int marker) {
	int row, col;
	byte scan;
	byte *scan_ptr;

	scan_ptr = (byte *)mem_normalize(scan_buf->data);

	for (row = 0; row < scan_buf->y; row++) {
		for (col = 0; col < scan_buf->x; col++) {
			scan = *scan_ptr;
			*scan_ptr = (byte)marker + scan;
			scan_ptr++;
		}
		scan_ptr = (byte *)mem_check_overflow(scan_ptr);
	}
}

void color_transparent_swap(Buffer *scan_buf, Palette *scan_pal, byte transparent) {
	byte *scan = scan_buf->data;
	word mysize = scan_buf->x * scan_buf->y;

	for (word i = 0; i < mysize; i++) {
		if (scan[i] == transparent)
			scan[i] = COLOR_TRANSPARENT;
	}

	memcpy(&(*scan_pal)[COLOR_TRANSPARENT], &(*scan_pal)[transparent], sizeof(RGBcolor));
}
void color_buffer_list_to_main(ColorListPtr color_list, Buffer *scan_buf) {
	byte *scan_ptr = (byte *)mem_normalize(scan_buf->data);
	int   row = scan_buf->y;
	int   col = scan_buf->x;
	Color *table_ptr = &color_list->table[0];

	// table_ptr->x16 is at offset +3 in the Color struct (skipping 3 bytes
	// to reach the X16 translated colour field, as the ASM does "add si, 3")
	byte *table_x16 = (byte *)table_ptr + 3;

	for (int r = 0; r < row; r++) {
		for (int c = 0; c < col; c++) {
			byte pixel = scan_ptr[c];
			// Original: ax = pixel, bx = pixel*2, ax = pixel*4,
			//           bx = pixel*2 + pixel*4 = pixel*6
			int offset = (int)pixel * 6;
			scan_ptr[c] = table_x16[offset];
		}
		scan_ptr += col;
	}
}

void color_split_thatch(int thatch, int *color, int *thatching) {
	if (thatch == COLOR_NO_X16) {
		*color = 16;
		*thatching = *color;
	} else {
		*color = thatch & 0x0f;
		*thatching = thatch >> 4;
		if (*thatching == 0) {
			*thatching = *color;
		} else if (*thatching == COLOR_BLACK_THATCH) {
			*thatching = 0;
		}
	}
}

void color_buffer_list_to_x16(ColorListPtr color_list, Buffer *scan_buf) {
	int row, col;
	int thatch, color_1, color_2;
	byte thatching;
	byte scan;
	byte *scan_ptr;

	scan_ptr = (byte *)mem_normalize(scan_buf->data);

	for (row = 0; row < scan_buf->y; row++) {
		thatching = (byte)(row & 1);
		for (col = 0; col < scan_buf->x; col++) {
			scan = *scan_ptr;
			thatch = color_list->table[scan].x16;
			color_split_thatch(thatch, &color_1, &color_2);
			*scan_ptr = (byte)(thatching ? color_2 : color_1);
			scan_ptr++;
			thatching = (byte)(!thatching);
		}
	}
}

} // namespace MADSV2
} // namespace MADS
