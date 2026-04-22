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

#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/heap.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/sort.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

#define magic_red       30
#define magic_green     59
#define magic_blue      11

#define MAX_RADIUS      210

/*
/*      If the "magic_color_flag" for a color gun is set, then the grey
/*      intensity ramp for its pixels is used during the fade, shifted
/*      left by its "magic_color_value" (0 means no shift, negative means
/*      shift right).  If "magic_color_flag" is false, then then that gun
/*      fades to the precise color value specified by magic_color_value.
*/
byte magic_color_flags[3] = { true, true, true };
byte magic_color_values[3] = { 0, 0, 0 };

int magic_special_center_x = -1;
int magic_special_center_y = -1;

int magic_low_fade_bound = 1;
int magic_high_fade_bound = 252;


void magic_get_grey_values(Palette *pal, byte *grey_value,
	int base_color, int num_colors) {
	int i;
	for (i = 0; i < num_colors; i++) {
		grey_value[i] = (byte)(magic_hash_color(&(*pal)[base_color + i]) >> 7);
	}
}

void magic_grey_palette(Palette *pal) {
	byte grey[256];
	int  count;

	magic_get_grey_values(pal, grey, 0, 256);

	for (count = 0; count < 256; count++) {
		(*pal)[count].r = grey[count];
		(*pal)[count].g = grey[count];
		(*pal)[count].b = grey[count];
	}
}


void magic_grey_popularity(byte *grey_list, byte *grey_table, int num_colors) {
	int i;
	memset(grey_table, 0, 64);
	for (i = 0; i < num_colors; i++) {
		grey_table[grey_list[i]]++;
	}
}

void magic_set_color_flags(byte r, byte g, byte b) {
	magic_color_flags[0] = r;
	magic_color_flags[1] = g;
	magic_color_flags[2] = b;
}

void magic_set_color_values(byte r, byte g, byte b) {
	magic_color_values[0] = r;
	magic_color_values[1] = g;
	magic_color_values[2] = b;
}

void magic_map_to_grey_ramp(Palette *pal,
	int base_color, int num_colors,
	int base_grey, int num_greys,
	MagicGreyPtr magic_map) {
	byte grey_list[256];
	byte grey_mapping[256];
	byte grey_table[64];
	byte grey_intensity[64];
	byte *pal_pointer;
	int count, greys, rescan;
	word grey_sum, grey_colors, grey_accum;
	int grey_scan, grey_mark, first_color;
	int color;
	int intensity, shift_sign;

	magic_get_grey_values(pal, grey_list, base_color, num_colors);
	magic_grey_popularity(grey_list, grey_table, num_colors);

	for (count = 0; count < num_colors; count++) {
		grey_mapping[count] = (byte)count;
		for (rescan = 0; rescan < 3; rescan++) {
			magic_map[count].accum[rescan] = 0;
		}
	}

	for (count = 0; count < 256; count++) {
		magic_map[count].map_color = (byte)count;
	}

	sort_insertion_8(num_colors, grey_mapping, grey_list);

	grey_sum = 0;
	grey_scan = 0;
	grey_mark = 0;
	grey_colors = 0;
	grey_accum = 0;
	first_color = 0;

	for (greys = 0; greys < 64; greys++) {
		for (count = 0; count < (int)grey_table[greys]; count++) {
			grey_sum += grey_list[grey_scan++];
			grey_colors++;

			grey_accum += num_greys;
			while (grey_accum >= (word)num_colors) {
				grey_accum -= num_colors;
				if (grey_colors > 0) {
					grey_intensity[grey_mark] = (byte)(grey_sum / grey_colors);
				}
				for (rescan = first_color; rescan < grey_scan; rescan++) {
					magic_map[grey_mapping[rescan]].intensity = grey_intensity[grey_mark];
					magic_map[grey_mapping[rescan]].map_color = (byte)(grey_mark + base_grey);
				}
				first_color = grey_scan;
				grey_sum = 0;
				grey_colors = 0;
				grey_mark++;
			}
		}
	}

	// pal_pointer = (byte *) &(((RGBcolor *) pal)[base_grey]);
	pal_pointer = ((byte *) pal + (base_grey * 3));
	for (greys = 0; greys < num_greys; greys++) {
		for (color = 0; color < 3; color++) {
			if (magic_color_flags[color]) {
				shift_sign = (char)magic_color_values[color];
				if (shift_sign >= 0) {
					intensity = grey_intensity[greys] << shift_sign;
				} else {
					intensity = grey_intensity[greys] >> abs(shift_sign);
				}
			} else {
				intensity = magic_color_values[color];
			}
			*(pal_pointer++) = (byte)intensity;
		}
	}
}


void magic_grey_ramp_palette(Palette pal, int num_greys) {
	int base_grey;
	int base_color, num_colors;
	int count;
	MagicGrey magic_map[256];

	base_grey = 256 - num_greys;
	base_color = 0;
	num_colors = base_grey;

	magic_map_to_grey_ramp((Palette *)pal, base_color, num_colors, base_grey, num_greys, magic_map);

	for (count = 0; count < num_colors; count++) {
		pal[count] = pal[magic_map[count].map_color];
	}
}


void magic_fade_to_grey(Palette pal, byte *map_pointer,
	int base_color, int num_colors,
	int base_grey, int num_greys,
	int tick_delay, int steps) {
	int count;
	int color;
	int index;
	int dif;
	int step;
	int intensity;
	int shift_sign;
	long base_timing, now_timing;
	long memory_needed;
	byte *work_memory = NULL;
	Heap magic_heap;
	byte *pal_index;
	char *signs;
	MagicGrey *magic_map;
	// Palette temp_pal;
	// char signs[256][3];
	// MagicGrey magic_map[256];
	memory_needed = (sizeof(Palette) << 1) + (sizeof(MagicGrey) << 8);

	if (timer_low_semaphore) {
		work_memory = timer_get_interrupt_stack();
	} else {
		work_memory = (byte *)mem_get_name(memory_needed, "$magic$");
		if (work_memory == NULL) {
			mcga_setpal((Palette *)pal);
			goto done;
		}
	}

	heap_declare(&magic_heap, MODULE_MATTE, (char *)work_memory, memory_needed);

	pal_index = (byte *)heap_get(&magic_heap, sizeof(Palette));
	signs = (char *)heap_get(&magic_heap, 768);
	magic_map = (MagicGrey *)heap_get(&magic_heap, sizeof(MagicGrey) << 8);

	magic_map_to_grey_ramp((Palette *)pal, base_color, num_colors,
		base_grey, num_greys, magic_map);

	for (count = base_color; count < (base_color + num_colors); count++) {
		index = count - base_color;
		for (color = 0; color < 3; color++) {
			if (magic_color_flags[color]) {
				shift_sign = (char)magic_color_values[color];
				if (shift_sign >= 0) {
					intensity = magic_map[index].intensity << shift_sign;
				} else {
					intensity = magic_map[index].intensity >> abs(shift_sign);
				}
			} else {
				intensity = magic_color_values[color];
			}
			dif = (intensity - pal_color(pal, count, color));
			*(pal_index + (color << 8) + count) = (byte)abs(dif);
			*(signs + (color << 8) + count) = (char)sgn(dif);
			// pal_color(temp_pal,count,color) = (byte)abs(dif);
			// signs[count][color] = (char)sgn(dif);
		}
	}

	base_timing = timer_read_600();

	for (step = 0; step < steps; step++) {
		for (count = base_color; count < (base_color + num_colors); count++) {
			index = count - base_color;
			for (color = 0; color < 3; color++) {
				magic_map[index].accum[color] += *(pal_index + (color << 8) + count);
				// magic_map[index].accum[color] += pal_color(temp_pal, count, color);
				while (magic_map[index].accum[color] >= (word)steps) {
					magic_map[index].accum[color] -= steps;
					pal_color(pal, count, color) += *(signs + (color << 8) + count);
					// pal_color(pal,count,color) += signs[count][color];
				}
			}
		}

		mcga_setpal((Palette *)pal);

		do {
			now_timing = timer_read_600();
		} while (now_timing < (base_timing + tick_delay));
		base_timing = now_timing;
	}

	if (map_pointer != NULL) {
		for (count = 0; count < num_colors; count++) {
			map_pointer[count] = magic_map[count].map_color;
		}
	}

done:
	if (!timer_low_semaphore) {
		if (work_memory != NULL) mem_free(work_memory);
	}
}


void magic_fade_from_grey(RGBcolor *pal, Palette target,
	int base_color, int num_colors,
	int base_grey, int num_greys,
	int tick_delay, int steps) {
	int count;
	int color;
	int index;
	int dif;
	int step;
	int intensity;
	int shift_sign;
	long base_timing, now_timing;
	long memory_needed;
	byte *work_memory = NULL;
	Heap magic_heap;
	byte *pal_index;
	char *signs;
	MagicGrey *magic_map;
	// Palette temp_pal;
	// char signs[256][3];
	// MagicGrey magic_map[256];
	memory_needed = (sizeof(Palette) << 1) + (sizeof(MagicGrey) << 8);

	if (timer_low_semaphore) {
		work_memory = timer_get_interrupt_stack();
	} else {
		work_memory = (byte *)mem_get_name(memory_needed, "$magic$");
		if (work_memory == NULL) {
			mcga_setpal((Palette *)pal);
			goto done;
		}
	}

	heap_declare(&magic_heap, MODULE_MATTE, (char *)work_memory, memory_needed);

	pal_index = (byte *)heap_get(&magic_heap, sizeof(Palette));
	signs = (char *)heap_get(&magic_heap, 768);
	magic_map = (MagicGrey *)heap_get(&magic_heap, sizeof(MagicGrey) << 8);

	memcpy(pal_index, target, sizeof(Palette));
	magic_map_to_grey_ramp((Palette *)pal_index,
		base_color, num_colors,
		base_grey, num_greys, magic_map);

	for (count = base_color; count < (base_color + num_colors); count++) {
		index = count - base_color;
		for (color = 0; color < 3; color++) {
			if (magic_color_flags[color]) {
				shift_sign = (char)magic_color_values[color];
				if (shift_sign >= 0) {
					intensity = magic_map[index].intensity << shift_sign;
				} else {
					intensity = magic_map[index].intensity >> abs(shift_sign);
				}
			} else {
				intensity = magic_color_values[color];
			}
			dif = (pal_color(target, count, color) - intensity);
			*(pal_index + (count * 3) + color) = (byte)abs(dif);
			// pal_color(pal_index,count,color) = (byte)abs(dif);
			*(signs + (color << 8) + count) = (char)sgn(dif);
			// signs[count][color] = (char)sgn(dif);
			magic_map[index].accum[color] = 0;
		}
	}

	base_timing = timer_read_600();

	for (step = 0; step < steps; step++) {
		for (count = base_color; count < (base_color + num_colors); count++) {
			index = count - base_color;
			for (color = 0; color < 3; color++) {
				magic_map[index].accum[color] += *(pal_index + (count * 3) + color);
				// magic_map[index].accum[color] += pal_color(pal_index ,count, color);
				while (magic_map[index].accum[color] >= (word)steps) {
					magic_map[index].accum[color] -= steps;
					*((byte *)pal + (count * 3) + color) += *(signs + (color << 8) + count);
					// (((byte *)&pal[count])+color) += *(signs + (color << 8) + count);
					// (((byte *)&pal[count])+color) += signs[count][color];
				}
			}
		}

		mcga_setpal((Palette *)pal);

		do {
			now_timing = timer_read_600();
		} while (now_timing < (base_timing + tick_delay));
		base_timing = now_timing;
	}

done:
	if (!timer_low_semaphore) {
		if (work_memory != NULL) mem_free(work_memory);
	}
}


void magic_screen_change_corner(Buffer *new_screen, Palette pal,
	int corner_id,
	int buffer_base_x, int buffer_base_y,
	int screen_base_x, int screen_base_y,
	int thru_black, int set_palette,
	int tick_delay) {
	int x, y;
	int x1, x2, y1, y2;
	int start_x = 0;
	int start_y = 0;
	int delta_x, delta_y;
	int size_x, size_y;
	int at_x, at_y;
	int *vx = NULL;
	int *hy = NULL;
	int accum;
	int loop, count;
	int loop_start;
	long base_timing, now_timing;
	byte background_swap[256];
	Buffer scr_live = { 200, 320, mcga_video };

	x = MIN(new_screen->x, video_x);
	y = new_screen->y;
	if (y > video_y) y = display_y;

	switch (corner_id) {
	case MAGIC_LOWER_LEFT:
		start_y = y - 1;
		hy = &y1;
		vx = &x2;
		break;
	case MAGIC_LOWER_RIGHT:
		start_x = x - 1;
		start_y = y - 1;
		hy = &y1;
		vx = &x1;
		break;
	case MAGIC_UPPER_LEFT:
		hy = &y2;
		vx = &x2;
		break;
	case MAGIC_UPPER_RIGHT:
		start_x = x - 1;
		hy = &y2;
		vx = &x1;
		break;
	}

	delta_x = (start_x ? -1 : 1);
	delta_y = (start_y ? -1 : 1);

	if (video_mode != mcga_mode) {
		thru_black = false;
		set_palette = false;
	}

	loop_start = (thru_black == MAGIC_THRU_BLACK) ? 0 : 1;

	if (set_palette && !thru_black) {
		mcga_setpal((Palette *)pal);
	}

	base_timing = timer_read_600();

	mouse_hide();

	if (thru_black == MAGIC_SECRET_FUJI) {
		magic_swap_foreground(background_swap, pal);
	}

	for (loop = loop_start; loop < 2; loop++) {
		at_x = start_x;
		at_y = start_y;
		accum = 0;
		for (count = 0; count < x - 1; count++) {
			x1 = MIN(start_x, at_x);
			x2 = MAX(start_x, at_x);
			y1 = MIN(start_y, at_y);
			y2 = MAX(start_y, at_y);
			size_x = x2 - x1 + 1;
			size_y = y2 - y1 + 1;

			if (loop) {
				if (thru_black == MAGIC_SECRET_FUJI) {
					buffer_rect_translate(*new_screen, scr_live,
						x1 + buffer_base_x, *hy + buffer_base_y,
						x1 + screen_base_x, *hy + screen_base_y,
						size_x, 1, background_swap);
					buffer_rect_translate(*new_screen, scr_live,
						*vx + buffer_base_x, y1 + buffer_base_y,
						*vx + screen_base_x, y1 + screen_base_y,
						1, size_y, background_swap);
				} else {
					video_update(new_screen,
						x1 + buffer_base_x, *hy + buffer_base_y,
						x1 + screen_base_x, *hy + screen_base_y,
						size_x, 1);
					video_update(new_screen,
						*vx + buffer_base_x, y1 + buffer_base_y,
						*vx + screen_base_x, y1 + screen_base_y,
						1, size_y);
#ifdef sixteen_colors
					if (video_mode == ega_mode) {
						video_flush_ega(y1 + screen_base_y,
							y1 + screen_base_y + size_y - 1);
					}
#endif
				}
			} else {
				buffer_rect_fill(scr_live,
					x1 + screen_base_x, *hy + screen_base_y,
					size_x, 1, 0);
				buffer_rect_fill(scr_live,
					*vx + screen_base_x, y1 + screen_base_y,
					1, size_y, 0);
			}

			do {
				now_timing = timer_read_600();
			} while (now_timing < (base_timing + tick_delay));
			base_timing = now_timing;

			at_x += delta_x;
			accum += y;
			while (accum >= x) {
				accum -= x;
				at_y += delta_y;
			}

		}


		if ((set_palette && !loop) || (thru_black == MAGIC_SECRET_FUJI)) {
			mcga_setpal((Palette *)pal);
		}
	}

	if (thru_black == MAGIC_SECRET_FUJI) {
		video_update(new_screen,
			buffer_base_x, buffer_base_y,
			screen_base_x, screen_base_y,
			x, y);
	}


	mouse_show();
}


void magic_screen_change_edge(Buffer *new_screen, Palette pal,
	int edge_id,
	int buffer_base_x, int buffer_base_y,
	int screen_base_x, int screen_base_y,
	int thru_black, int set_palette,
	int tick_delay) {
	int x, y;
	int y1, y2;
	int start_x = 0;
	int delta_x;
	int size_y;
	int at_x;
	int loop;
	int loop_start;
	int count;
	long base_timing, now_timing;
	byte background_swap[256];
	Buffer scr_live = { 200, 320, mcga_video };

	x = MIN(new_screen->x, video_x);
	y = new_screen->y;
	if (y > video_y) y = display_y;

	if (edge_id == MAGIC_RIGHT) {
		start_x = x - 1;
	}

	delta_x = (start_x ? -1 : 1);

	if (video_mode != mcga_mode) {
		thru_black = false;
		set_palette = false;
	}

	loop_start = (thru_black == MAGIC_THRU_BLACK) ? 0 : 1;

	if (set_palette && !thru_black) {
		mcga_setpal((Palette *)pal);
	}

	base_timing = timer_read_600();

	y1 = 0;
	y2 = y - 1;
	size_y = y2 - y1 + 1;

	mouse_hide();

	if (thru_black == MAGIC_SECRET_FUJI) {
		magic_swap_foreground(background_swap, pal);
	}

	for (loop = loop_start; loop < 2; loop++) {
		at_x = start_x;
		for (count = 0; count < x; count++) {

			if (loop) {
				if (thru_black == MAGIC_SECRET_FUJI) {
					buffer_rect_translate(*new_screen, scr_live,
						at_x + buffer_base_x, y1 + buffer_base_y,
						at_x + screen_base_x, y1 + screen_base_y,
						1, size_y, background_swap);
				} else {
					video_update(new_screen,
						at_x + buffer_base_x, y1 + buffer_base_y,
						at_x + screen_base_x, y1 + screen_base_y,
						1, size_y);
#ifdef sixteen_colors
					if (video_mode == ega_mode) {
						video_flush_ega(y1 + screen_base_y, y1 + screen_base_y + size_y - 1);
					}
#endif
				}
			} else {
				buffer_rect_fill(scr_live,
					at_x + screen_base_x, y1 + screen_base_y,
					1, size_y, 0);
			}

			do {
				now_timing = timer_read_600();
			} while (now_timing < (base_timing + tick_delay));
			base_timing = now_timing;

			at_x += delta_x;
		}

		if ((set_palette && !loop) || (thru_black == MAGIC_SECRET_FUJI)) {
			mcga_setpal((Palette *)pal);
		}
	}

	if (thru_black == MAGIC_SECRET_FUJI) {
		video_update(new_screen,
			buffer_base_x, buffer_base_y,
			screen_base_x, screen_base_y,
			x, y);
	}

	mouse_show();
}


void magic_screen_change_circle(Buffer *new_screen, Palette pal,
	int inward_flag,
	int buffer_base_x, int buffer_base_y,
	int screen_base_x, int screen_base_y,
	int thru_black, int set_palette,
	int tick_delay, int pixel_rate) {
	int x, y;
	int x1, x2, xs;
	int xa, xb;
	int center_x;
	int center_y;
	int this_y;
	int radius;
	int delta_radius;
	int start_radius;
	int at_radius;
	int previous_radius;
	int loop;
	int loop_start;
	int done;
	int at_y, x_sign, y_sign;
	int count;
	int y_limit;
	int first_time;
	byte background_swap[256];
	word circle_buffer_now[MAX_RADIUS + 1];
	word circle_buffer_before[MAX_RADIUS + 1];
	long base_timing, now_timing;
	Buffer scr_live = { video_y, video_x, mcga_video };

	x = MIN(new_screen->x, video_x);
	y = new_screen->y;
	if (y > video_y) y = display_y;

	center_x = x >> 1;
	center_y = y >> 1;

	if (magic_special_center_x >= 0) center_x = magic_special_center_x;
	if (magic_special_center_y >= 0) center_y = magic_special_center_y;

	radius = MAX_RADIUS;
	done = false;

	if (inward_flag) {
		start_radius = radius - 1;
		delta_radius = -1 * pixel_rate;
	} else {
		start_radius = 0;
		delta_radius = pixel_rate;
	}

	if (video_mode != mcga_mode) {
		thru_black = false;
		set_palette = false;
	}

	loop_start = (thru_black == MAGIC_THRU_BLACK) ? 0 : 1;

	if (set_palette && !thru_black) {
		mcga_setpal((Palette *)pal);
	}

	base_timing = timer_read_600();

	mouse_hide();

	if (thru_black == MAGIC_SECRET_FUJI) {
		magic_swap_foreground(background_swap, pal);
	}

	for (loop = loop_start; loop < 2; loop++) {
		for (count = 0; count < MAX_RADIUS; count++) {
			circle_buffer_before[count] = inward_flag ? MAX_RADIUS - 1 : 0;
		}
		at_radius = start_radius;
		first_time = true;
		done = false;
		while (!done) {
			if ((at_radius > 0) && !first_time) {
				memcpy(circle_buffer_before, circle_buffer_now, at_radius << 1);
			}
			first_time = false;
			previous_radius = at_radius;
			at_radius += delta_radius;
			imath_circular_arc(circle_buffer_now, at_radius);
			if (inward_flag) {
				for (count = at_radius; count < previous_radius; count++) {
					circle_buffer_now[count] = 0;
				}
			}
			y_limit = inward_flag ? previous_radius : at_radius;
			for (at_y = 0; at_y < y_limit; at_y++) {
				for (y_sign = -1; y_sign < 2; y_sign += 2) {
					this_y = center_y + sgn_in(at_y, y_sign);
					if ((this_y >= 0) && (this_y < y)) {
						for (x_sign = -1; x_sign < 2; x_sign += 2) {
							xa = center_x + sgn_in(circle_buffer_now[at_y] - 1, x_sign);
							xb = center_x + sgn_in(circle_buffer_before[at_y], x_sign);
							x1 = MIN(x - 1, MAX(0, MIN(xa, xb)));
							x2 = MIN(x - 1, MAX(0, MAX(xa, xb)));
							xs = x2 - x1 + 1;

							if ((xa < 0) && (xb < 0))   xs = 0;
							if ((xa >= x) && (xb >= x)) xs = 0;

							if (inward_flag) {
								if (at_radius <= 0) done = true;
							} else {
								if ((this_y == 0) && (x1 == 0)) done = true;
							}

							if (xs) {
								if (loop) {
									if (thru_black == MAGIC_SECRET_FUJI) {
										buffer_rect_translate(*new_screen, scr_live,
											x1 + buffer_base_x, this_y + buffer_base_y,
											x1 + screen_base_x, this_y + screen_base_y,
											xs, 1, background_swap);
									} else {
										video_update(new_screen,
											x1 + buffer_base_x, this_y + buffer_base_y,
											x1 + screen_base_x, this_y + screen_base_y,
											xs, 1);
#ifdef sixteen_colors
										if (video_mode == ega_mode) {
											video_flush_ega(this_y + screen_base_y,
												this_y + screen_base_y);
										}
#endif
									}
								} else {
									buffer_rect_fill(scr_live,
										x1 + screen_base_x, this_y + screen_base_y,
										xs, 1, 0);
								}
							}
						}
					}
				}
			}

			do {
				now_timing = timer_read_600();
			} while (now_timing < (base_timing + tick_delay));
			base_timing = now_timing;
		}

		if ((set_palette && !loop) || (thru_black == MAGIC_SECRET_FUJI)) {
			mcga_setpal((Palette *)pal);
		}
	}

	if (thru_black == MAGIC_SECRET_FUJI) {
		video_update(new_screen,
			buffer_base_x, buffer_base_y,
			screen_base_x, screen_base_y,
			x, y);
	}

	mouse_show();
}

void magic_shrink_buffer(Buffer *from, Buffer *unto) {
	char *from_ptr;
	char *unto_ptr;
	int x_count, y_count;
	int from_wrap, unto_wrap;
	int from_size, unto_size;
	byte bres_x_table[(video_x << 1)];
	int i, bres;

	from_ptr = (char *)from->data;
	from_wrap = from->x;
	unto_ptr = (char *)unto->data;
	unto_wrap = unto->x;
	from_size = from->y;
	unto_size = unto->y;
	y_count = from->y;
	x_count = from->x;

	// Build X Bresenham table
	bres = 0;
	for (i = 0; i < from_wrap; i++) {
		bres += unto_wrap;
		if (bres >= from_wrap) {
			bres -= from_wrap;
			bres_x_table[i] = true;
		} else {
			bres_x_table[i] = false;
		}
	}

	// Shrink pixels using Y Bresenham + X table
	bres = 0;
	do {
		bres += unto_size;
		if (bres >= from_size) {
			bres -= from_size;
			char *src_row = from_ptr;
			char *dst_row = unto_ptr;
			for (i = 0; i < x_count; i++) {
				if (bres_x_table[i]) {
					*dst_row++ = *src_row;
				}
				src_row++;
			}
			unto_ptr += unto_wrap;
		}
		from_ptr += from_wrap;
		y_count--;
	} while (y_count != 0);
}


int  magic_shrinking_buffer(Buffer *source, Buffer *rear,
	int grow_flag,
	int buffer_base_x, int buffer_base_y,
	int screen_base_x, int screen_base_y,
	int tick_delay) {
	Buffer *from;
	Buffer shrink = { 0, 0, NULL };
	Buffer extra = { 0, 0, NULL };
	Buffer bonus = { 0, 0, NULL };
	Buffer scr_live = { video_y, video_x, mcga_video };
	int x, y;
	int center_x, center_y;
	int at_x, at_y;
	int to_x, to_y;
	int size_x, size_y;
	int dif_x, dif_y;
	int last_at_x, last_at_y;
	int last_to_x, last_to_y;
	int last_size_x, last_size_y;
	int count;
	int error_flag = true;
	long base_timing, now_timing;

	mouse_hide();

	from = source;
	if ((source->x > video_x) || (source->y > video_y)) {
		buffer_init(&extra, video_x, display_y);
		if (extra.data == NULL) goto done;
		buffer_rect_copy_2(*source, extra, buffer_base_x, buffer_base_y, 0, 0, video_x, display_y);
		from = &extra;
	}

	x = from->x;
	y = from->y;

	buffer_init(&shrink, x, y);
	if (shrink.data == NULL) goto done;

	if ((video_mode == ega_mode) && (rear == NULL)) {
		rear = &bonus;
		buffer_init(&bonus, x, y);
		if (bonus.data == NULL) goto done;
		buffer_fill(bonus, 0);
	}

	center_x = x >> 1;
	center_y = y >> 1;

	base_timing = timer_read_600();

	if (grow_flag) {
		for (count = 1; count <= center_y; count++) {
			size_y = count << 1;
			size_x = (int)(((long)size_y * (long)x) / (long)y);
			size_x &= 0xfffe;
			at_y = center_y - count;
			at_x = center_x - (size_x >> 1);

			shrink.x = size_x;
			shrink.y = size_y;
			magic_shrink_buffer(from, &shrink);

			video_update(&shrink,
				0, 0,
				at_x + screen_base_x, at_y + screen_base_y,
				size_x, size_y);

#ifdef sixteen_colors
			if (video_mode == ega_mode) {
				video_flush_ega(at_y + screen_base_y, at_y + screen_base_y + size_y - 1);
			}
#endif

			do {
				now_timing = timer_read_600();
			} while (now_timing < (base_timing + tick_delay));
			base_timing = now_timing;

		}
	} else {

		last_size_y = y;
		last_size_x = x;
		last_at_y = 0;
		last_at_x = 0;
		last_to_y = y - 1;
		last_to_x = x - 1;

		for (count = center_y - 1; count >= 0; count--) {
			size_y = count << 1;
			size_x = (int)(((long)size_y * (long)x) / (long)y);
			size_x &= 0xfffe;
			at_y = center_y - count;
			at_x = center_x - (size_x >> 1);
			to_y = at_y + size_y - 1;
			to_x = at_x + size_x - 1;

			dif_x = (at_x - last_at_x);
			dif_y = (at_y - last_at_y);

			if (size_y > 0) {
				shrink.x = size_x;
				shrink.y = size_y;
				magic_shrink_buffer(from, &shrink);

				video_update(&shrink,
					0, 0,
					at_x + screen_base_x, at_y + screen_base_y,
					size_x, size_y);
			}

			last_at_x += screen_base_x;
			last_at_y += screen_base_y;
			last_to_x += screen_base_x;
			last_to_y += screen_base_y;

			if (rear != NULL) {
				video_update(rear, last_at_x, last_at_y,
					last_at_x, last_at_y,
					dif_x, last_size_y);
				video_update(rear, last_at_x, last_at_y,
					last_at_x, last_at_y,
					last_size_x, dif_y);
				video_update(rear, last_at_x, last_to_y - (dif_y - 1),
					last_at_x, last_to_y - (dif_y - 1),
					last_size_x, dif_y);
				video_update(rear, last_to_x - (dif_x - 1), last_at_y,
					last_to_x - (dif_x - 1), last_at_y,
					dif_x, last_size_y);

#ifdef sixteen_colors
				if (video_mode == ega_mode) {
					video_flush_ega(last_at_y, last_to_y);
				}
#endif
			} else {
				buffer_rect_fill(scr_live, last_at_x, last_at_y,
					dif_x, last_size_y, 0);
				buffer_rect_fill(scr_live, last_at_x, last_at_y,
					last_size_x, dif_y, 0);
				buffer_rect_fill(scr_live, last_at_x, last_to_y - (dif_y - 1),
					last_size_x, dif_y, 0);
				buffer_rect_fill(scr_live, last_to_x - (dif_x - 1), last_at_y,
					dif_x, last_size_y, 0);
			}

			last_size_x = size_x;
			last_size_y = size_y;
			last_at_x = at_x;
			last_at_y = at_y;
			last_to_x = to_x;
			last_to_y = to_y;

			do {
				now_timing = timer_read_600();
			} while (now_timing < (base_timing + tick_delay));
			base_timing = now_timing;
		}
	}

	error_flag = false;

done:
	if (bonus.data != NULL) buffer_free(&bonus);
	if (shrink.data != NULL) buffer_free(&shrink);
	if (extra.data != NULL) buffer_free(&extra);

	mouse_show();

	return (error_flag);
}


void magic_swap_me_in_the_dark_baby(byte *swap,
	RGBcolor *pal,
	int start) {
	int static_start;
	int count;
	byte *static_list;
	byte *dynamic_list;

	dynamic_list = &pal[start].r;

	static_start = 1 - start;
	static_list = &pal[static_start].r;

	for (count = 0; count < 256; count++) {
		swap[count] = (byte)count;
	}

	for (count = 0; count < 128; count++) {
		if ((start >= magic_low_fade_bound) && (start <= magic_high_fade_bound)) {
			swap[start] = (byte)((magic_closest_color((RGBcolor *)dynamic_list,
				static_list,
				6, 128) << 1) + static_start);
		}
		dynamic_list += 6;
		start += 2;
	}
}


void magic_swap_foreground(byte *background_table,
	Palette background_palette) {
	int     count;
	byte *old_palette;
	byte    swap_table[256];
	// Palette old_palette;
	long    memory_needed;
	byte *work_memory = NULL;
	Heap    magic_heap;
	Buffer  scr_live = { video_y, video_x, mcga_video };

	memory_needed = sizeof(Palette);

	if (timer_low_semaphore) {
		work_memory = timer_get_interrupt_stack();
	} else {
		work_memory = (byte *)mem_get_name(memory_needed, "$magic$");
		if (work_memory == NULL) goto done;
	}

	heap_declare(&magic_heap, MODULE_MATTE, (char *)work_memory, memory_needed);

	old_palette = (byte *)heap_get(&magic_heap, sizeof(Palette));

	mcga_getpal((Palette *)old_palette);

	magic_swap_me_in_the_dark_baby(swap_table,
		(RGBcolor *)old_palette,
		MAGIC_SWAP_FOREGROUND);

	magic_swap_me_in_the_dark_baby(background_table,
		(RGBcolor *)background_palette,
		MAGIC_SWAP_BACKGROUND);

	for (count = MAGIC_SWAP_FOREGROUND; count < 256; count += 2) {
		((RGBcolor *)old_palette)[count] = background_palette[count];
	}

	buffer_rect_translate(scr_live, scr_live,
		0, 0, 0, 0, video_x, video_y,
		swap_table);

	mcga_setpal((Palette *)old_palette);

done:
	if (!timer_low_semaphore) {
		if (work_memory != NULL) mem_free(work_memory);
	}
}

int magic_closest_color(RGBcolor *match_color,
	byte *list,
	int list_wrap,
	int list_length) {
	int best_color = 0;
	int this_color = 0;
	int best_diff = 0x7fff;
	int i, diff;
	byte *entry;

	for (this_color = 0; this_color < list_length; this_color++) {
		entry = list + (this_color * list_wrap);
		diff = 0;
		for (i = 0; i < 3; i++) {
			int d = (signed char)(entry[i] - ((byte *)match_color)[i]);
			diff += d * d;
		}
		if (diff <= best_diff) {
			best_color = this_color;
			best_diff = diff;
		}
	}
	return best_color;
}

int magic_hash_color(RGBcolor *hash_color) {
	return (hash_color->r * 38) + (hash_color->g * 76) + (hash_color->b * 14);
}

} // namespace MADSV2
} // namespace MADS
