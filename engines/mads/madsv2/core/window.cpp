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
#include "mads/madsv2/core/window.h"
#include "mads/madsv2/core/cursor.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/screen.h"

namespace MADS {
namespace MADSV2 {

#define text_wrap  160

int window_line_type;       /* Line type used to draw last window border */


#define WINDOW_TRAP_WIDTH       80


WindowPtr trap_window;

void (*(window_any_char_routine))() = NULL;
int  window_num_trap_routines = 0;
byte *window_trap_routine[WINDOW_MAX_TRAP_ROUTINES];
char window_trap_string[WINDOW_MAX_TRAP_ROUTINES][WINDOW_TRAP_WIDTH];
byte window_incoming_char;
byte window_incoming_buffer[256];
byte *window_incoming_string = window_incoming_buffer;
byte *window_next_char = window_incoming_buffer;
byte *window_where_to;

int window_display_string = false;
int window_cursor_base_x;
int trap_window_color;
int trap_window_width;
int window_server_installed = false;
int window_box_mode = WINDOW_SINGLE;


void window_set(WindowPtr window, int ul_x, int ul_y, int lr_x, int lr_y) {
	window->ul_x = ul_x;
	window->ul_y = ul_y;
	window->lr_x = lr_x;
	window->lr_y = lr_y;
}

void window_line_across(WindowPtr window, int line_y) {
	short x_count;

	*(screen + screen_char_add(window->ul_x, line_y)) = left_join;
	*(screen + screen_char_add(window->lr_x, line_y)) = right_join;

	for (x_count = window->ul_x + 1; x_count < window->lr_x; x_count++) {
		*(screen + screen_char_add(x_count, line_y)) = horiz_frame;
	}
}

void window_color(WindowPtr window, int new_col) {
	short x_count, y_count;

	for (y_count = window->ul_y; y_count <= window->lr_y; y_count++) {
		for (x_count = window->ul_x; x_count <= window->lr_x; x_count++) {
			*(screen + screen_color_add(x_count, y_count)) = (byte)new_col;
		}
	}
}

void window_wipe(WindowPtr window) {
	short x_count, y_count;

	for (y_count = window->ul_y; y_count <= window->lr_y; y_count++) {
		for (x_count = window->ul_x; x_count <= window->lr_x; x_count++) {
			*(screen + screen_char_add(x_count, y_count)) = 0x20;
		}
	}
}

void window_shadow(WindowPtr window) {
	short x_count, y_count, max_x, max_y;
	short my_color;

	if (screen == nullptr /*mono_text_video*/) {
		my_color = colorbyte(black, black);
	} else {
		my_color = window_shadow_color;
	}

	if ((window->lr_x <= (screen_max_x - 2)) || (window->lr_y <= (screen_max_y - 1))) {

		if (window->lr_x <= (screen_max_x - 2)) {
			max_y = MIN(window->lr_y + 1, screen_max_y);
			for (x_count = window->lr_x + 1; x_count <= window->lr_x + 2; x_count++) {
				for (y_count = window->ul_y + 1; y_count <= max_y; y_count++) {
					*(screen + screen_color_add(x_count, y_count)) = (byte)my_color;
				}
			}
		}

		if (window->lr_y <= (screen_max_y - 1)) {
			max_x = MIN(window->lr_x + 2, screen_max_x);
			for (y_count = window->lr_y + 1; y_count < window->lr_y + 2; y_count++) {
				for (x_count = window->ul_x + 2; x_count <= max_x; x_count++) {
					*(screen + screen_color_add(x_count, y_count)) = (byte)my_color;
				}
			}
		}
	}
}

void window_define_scrollbar(WindowPtr window, ScrollBarPtr scroll, int vertical,
		int left_or_bottom, long min_value, long max_value, long scroll_value,
		long page_value, int normal_color, int select_color) {
	scroll->window = window;
	scroll->vertical = vertical;
	scroll->side = left_or_bottom;
	scroll->min_value = min_value;
	scroll->max_value = max_value;
	scroll->scroll_value = scroll_value;
	scroll->page_value = page_value;
	scroll->normal_color = normal_color;
	scroll->select_color = select_color;
}

static void window_scrollbar_axis(ScrollBarPtr scroll, int x, int y,
		int *control, int *passive) {
	if (scroll->vertical) {
		*control = y;
		*passive = x;
	} else {
		*control = x;
		*passive = y;
	}
}

static void window_scrollbar_coord(ScrollBarPtr scroll, int control,
		int passive, int *x, int *y) {
	if (scroll->vertical) {
		*y = control;
		*x = passive;
	} else {
		*x = control;
		*y = passive;
	}
}

static void window_scrollbar_parameters(ScrollBarPtr scroll, int *control_low,
		int *control_high, int *passive_low, int *passive_high, int *passive_main) {
	window_scrollbar_axis(scroll, scroll->window->ul_x, scroll->window->ul_y,
		control_low, passive_low);
	window_scrollbar_axis(scroll, scroll->window->lr_x, scroll->window->lr_y,
		control_high, passive_high);

	*passive_main = scroll->side ? *passive_low : *passive_high;
}

static void window_scrollbar_write(ScrollBarPtr scroll,
		int control, int passive, byte item, int select) {
	int x, y;
	byte *screen_ptr;

	window_scrollbar_coord(scroll, control, passive, &x, &y);

	screen_ptr = screen + screen_char_add(x, y);
	*(screen_ptr++) = item;
	*(screen_ptr++) = (byte)(select ? scroll->select_color : scroll->normal_color);
}

void window_draw_scrollbar(ScrollBarPtr scroll) {
	int control_low, control_high;
	int passive_low, passive_high;
	int passive_main;
	int count;
	char low_mark, high_mark;

	window_scrollbar_parameters(scroll, &control_low, &control_high,
		&passive_low, &passive_high,
		&passive_main);

	low_mark = (char)(scroll->vertical ? up_mark : left_mark);
	high_mark = (char)(scroll->vertical ? down_mark : right_mark);
	window_scrollbar_write(scroll, control_low + 1, passive_main, low_mark, true);
	window_scrollbar_write(scroll, control_high - 1, passive_main, high_mark, true);

	for (count = control_low + 2; count < control_high - 1; count++) {
		window_scrollbar_write(scroll, count, passive_main, scroll_bar, false);
	}
}

long window_translate_thumb(ScrollBarPtr scroll, long thumb) {
	int control_low, control_high;
	int passive_low, passive_high;
	int passive_main;
	long y_dif, val_dif, value;

	window_scrollbar_parameters(scroll, &control_low, &control_high,
		&passive_low, &passive_high,
		&passive_main);

	y_dif = (control_high - 2) - (control_low + 2) + 1;
	val_dif = (scroll->max_value - scroll->min_value) + 1;

	value = ((thumb - (control_low + 2)) * val_dif) / y_dif;
	value = MIN(value, scroll->max_value);
	value = MAX(value, scroll->min_value);

	return value;
}

long window_make_thumb(ScrollBarPtr scroll, long current_value) {
	int control_low, control_high;
	int passive_low, passive_high;
	int passive_main;
	long y_dif, val_dif, thumb;

	window_scrollbar_parameters(scroll, &control_low, &control_high,
		&passive_low, &passive_high,
		&passive_main);

	y_dif = (control_high - 2) - (control_low + 2) + 1;
	val_dif = (scroll->max_value - scroll->min_value) + 1;

	thumb = (current_value * y_dif) / val_dif;

	return thumb + (control_low + 2);
}

long window_draw_thumb(ScrollBarPtr scroll, long current_value) {
	int control_low, control_high;
	int passive_low, passive_high;
	int passive_main;
	int count;
	char low_mark, high_mark;
	long thumb;

	window_scrollbar_parameters(scroll, &control_low, &control_high,
		&passive_low, &passive_high,
		&passive_main);

	low_mark = (char)(scroll->vertical ? up_mark : left_mark);
	high_mark = (char)(scroll->vertical ? down_mark : right_mark);

	window_scrollbar_write(scroll, control_low + 1, passive_main, low_mark, true);
	window_scrollbar_write(scroll, control_high - 1, passive_main, high_mark, true);

	thumb = window_make_thumb(scroll, current_value);

	for (count = control_low + 2; count < control_high - 1; count++) {
		if (count == (int)thumb) {
			window_scrollbar_write(scroll, count, passive_main, ' ', false);
		} else {
			window_scrollbar_write(scroll, count, passive_main, scroll_bar, false);
		}
	}

	return thumb;
}

int window_scrollbar_detect(ScrollBarPtr scroll, int x, int y, long value,
		long *new_value, int prior_detect) {
	int control_low, control_high;
	int passive_low, passive_high;
	int passive_main;
	int control, passive;
	int start;
	int detect;
	int tight;
	long thumb;

	window_scrollbar_parameters(scroll, &control_low, &control_high,
		&passive_low, &passive_high,
		&passive_main);

	window_scrollbar_axis(scroll, x, y, &control, &passive);

	start = (prior_detect == WINDOW_DETECT_START);
	tight = start;
	detect = WINDOW_DETECT_NOTHING;

	if (!tight || (passive == passive_main)) {
		if (!tight || ((control > control_low) && (control < control_high))) {

			if ((start || (prior_detect == WINDOW_DETECT_SCROLL_LOWER)) && (control == (control_low + 1))) {
				detect = WINDOW_DETECT_SCROLL_LOWER;
				*new_value = MAX((value - scroll->scroll_value), scroll->min_value);
			} else if ((start || (prior_detect == WINDOW_DETECT_SCROLL_HIGHER)) && (control == (control_high - 1))) {
				detect = WINDOW_DETECT_SCROLL_HIGHER;
				*new_value = value + scroll->scroll_value;
			} else {

				thumb = window_make_thumb(scroll, value);

				if ((start || (prior_detect == WINDOW_DETECT_SCROLL)) && (thumb == control)) {
					detect = WINDOW_DETECT_SCROLL;
					*new_value = value;
				} else if (prior_detect == WINDOW_DETECT_SCROLL) {
					detect = WINDOW_DETECT_SCROLL;
					*new_value = window_translate_thumb(scroll, control);
				} else {
					if ((start || (prior_detect == WINDOW_DETECT_PAGE_LOWER)) && (control < (int)thumb)) {
						detect = WINDOW_DETECT_PAGE_LOWER;
						*new_value = MAX((value - scroll->page_value), scroll->min_value);
					} else if ((start || (prior_detect == WINDOW_DETECT_PAGE_HIGHER)) && (control > (int)thumb)) {
						detect = WINDOW_DETECT_PAGE_HIGHER;
						*new_value = value + scroll->page_value;
					}
				}
			}
		}
	}

	if ((detect == WINDOW_DETECT_NOTHING) && (prior_detect != WINDOW_DETECT_START)) {
		detect = prior_detect;
	}

	return detect;
}

int window_detect(ScrollBarPtr scroll, int x, int y,
	long value, long *new_value,
	int prior_detect,
	long base_value, long value_mult) {
	int control_low, control_high;
	int passive_low, passive_high;
	int passive_main;
	int control, passive;
	int detect;
	int tight;
	int start;

	*new_value = value;

	window_scrollbar_parameters(scroll, &control_low, &control_high,
		&passive_low, &passive_high,
		&passive_main);

	window_scrollbar_axis(scroll, x, y, &control, &passive);

	detect = window_scrollbar_detect(scroll, x, y,
		value, new_value,
		prior_detect);

	if (detect != WINDOW_DETECT_NOTHING) {
		prior_detect = detect;
	}

	start = tight = ((prior_detect == WINDOW_DETECT_START) &&
		(detect == WINDOW_DETECT_NOTHING));

	if (!tight || ((passive > passive_low) && (passive < passive_high))) {
		if ((start || (prior_detect >= WINDOW_DETECT_VALID)) && ((control > control_low) && (control < control_high))) {
			detect = control - (control_low + 1);
			*new_value = base_value + (detect * value_mult);
			*new_value = MAX(*new_value, scroll->min_value);
			*new_value = MIN(*new_value, scroll->max_value);
		} else if (!tight && (prior_detect >= WINDOW_DETECT_VALID)) {
			if (control <= control_low) {
				*new_value = MAX((value - scroll->scroll_value), scroll->min_value);
			} else if (control >= control_high) {
				*new_value = value + scroll->scroll_value;
			}
		}
	}

	if ((detect == WINDOW_DETECT_NOTHING) && (prior_detect != WINDOW_DETECT_START)) {
		detect = prior_detect;
	}

	return detect;
}

void window_vert_scrollbar(WindowPtr window, int barcolor) {
	short y_count;
	byte *screen_ptr;
	Window temp_win;
	int x;

	x = window->lr_x;

	*(screen + screen_char_add(x, window->ul_y + 1)) = up_mark;
	*(screen + screen_char_add(x, window->lr_y - 1)) = down_mark;

	screen_ptr = screen + screen_char_add(x, window->ul_y + 2);
	for (y_count = window->ul_y + 2; y_count < window->lr_y - 1; y_count++) {
		*screen_ptr = scroll_bar;
		screen_ptr += 160;
	}

	window_set(&temp_win, x, window->ul_y + 1,
		x, window->lr_y - 1);
	window_color(&temp_win, barcolor);
}

void window_horiz_scrollbar(WindowPtr window, int barcolor) {
	short x_count;
	byte *screen_ptr;
	Window temp_win;

	*(screen + screen_char_add(window->ul_x + 1, window->lr_y)) = left_mark;
	*(screen + screen_char_add(window->lr_x - 1, window->lr_y)) = right_mark;

	screen_ptr = screen + screen_char_add(window->ul_x + 2, window->lr_y);
	for (x_count = window->ul_x + 2; x_count < window->lr_x - 1; x_count++) {
		*screen_ptr = scroll_bar;
		screen_ptr += 2;
	}

	window_set(&temp_win, window->ul_x + 1, window->lr_y,
		window->lr_x - 1, window->lr_y);
	window_color(&temp_win, barcolor);
}

void window_title(WindowPtr window, const char *title, int title_color, int background_color) {
	short center_x, begin_x;
	byte  line_char;
	char  temp[6];

	mouse_hide();

	switch (window_line_type) {
	case WINDOW_SINGLE: line_char = horiz_frame; break;
	case WINDOW_DOUBLE: line_char = 205; break;
	case WINDOW_OBESE: line_char = 219; break;
	default: line_char = horiz_frame; break;
	};

	screen_wipe_line(window->ul_x + 1, window->ul_y, (window->lr_x - window->ul_x) - 1, background_color, line_char);

	center_x = window->ul_x + ((window->lr_x - window->ul_x) / 2);
	begin_x = center_x - ((strlen(title) + 2) / 2);

	Common::strcpy_s(temp, " ");
	// temp[0] = (byte)((title_color == background_color) ? right_join : ' ');
	begin_x = screen_put(temp, title_color, title_color, begin_x, window->ul_y);

	begin_x = screen_put(title, title_color, title_color, begin_x, window->ul_y);

	// temp[0] = (byte)((title_color == background_color) ? left_join : ' ');
	begin_x = screen_put(temp, title_color, title_color, begin_x, window->ul_y);

	mouse_show();
}

void window_abort(const char *message) {
	Window abort_win;

	window_set(&abort_win, 23, 10, 57, 14);
	window_wipe(&abort_win);
	window_color(&abort_win, abort_color);
	window_draw_box(&abort_win, WINDOW_SINGLE);
	window_shadow(&abort_win);
	screen_put(message, abort_color, abort_color,
		25, 12);
	mouse_hide();
	mouse_init(false, 0);
	error("Aborted");
}

byte *window_create(WindowPtr window) {
	byte *address;
	byte *storage;
	int baseadd, nextadd, rowbump, memory;
	int width, height;
	int width_bonus, height_bonus;

	baseadd = screen_char_add(window->ul_x, window->ul_y);
	nextadd = screen_char_add(window->ul_x, window->ul_y + 1);
	address = screen + baseadd;

	width_bonus = (window->lr_x < screen_max_x - 2) ? 3 : 1;
	height_bonus = (window->lr_y < screen_max_y - 1) ? 2 : 1;

	width = (window->lr_x - window->ul_x) + width_bonus;
	height = (window->lr_y - window->ul_y) + height_bonus;
	rowbump = (nextadd - baseadd) - (width << 1);
	memory = (width * height) << 1;

	storage = (byte *)mem_get(memory);
	window->storage = storage;
	if (storage == NULL) window_abort("Severe Window Memory Failure!");

	mouse_hide();

	{
		byte *src = address;
		word *dst = (word *)storage;
		int row, col;

		for (row = 0; row < height; row++) {
			word *src_w = (word *)src;
			for (col = 0; col < width; col++) {
				*dst++ = *src_w++;
			}
			src += (width << 1) + rowbump;
		}
	}

	mouse_show();
	return storage;
}

void window_destroy(WindowPtr window) {
	byte *address;
	byte *storage;
	int baseadd, nextadd, rowbump, memory;
	int width_bonus, height_bonus;
	int width, height;

	storage = window->storage;
	if (storage == NULL) return;

	baseadd = screen_char_add(window->ul_x, window->ul_y);
	nextadd = screen_char_add(window->ul_x, window->ul_y + 1);
	address = screen + baseadd;

	width_bonus = (window->lr_x < screen_max_x - 2) ? 3 : 1;
	height_bonus = (window->lr_y < screen_max_y - 1) ? 2 : 1;

	width = (window->lr_x - window->ul_x) + width_bonus;
	height = (window->lr_y - window->ul_y) + height_bonus;
	rowbump = (nextadd - baseadd) - (width << 1);
	memory = (width * height) << 1;

	mouse_hide();

	{
		word *src = (word *)storage;
		byte *dst = address;
		int row, col;

		for (row = 0; row < height; row++) {
			word *dst_w = (word *)dst;
			for (col = 0; col < width; col++) {
				*dst_w++ = *src++;
			}
			dst += (width << 1) + rowbump;
		}
	}

	mouse_show();
	mem_free(window->storage);
	window->storage = NULL;
}

int window_normal_color(WindowPtr window) {
	int my_color;

	my_color = *(screen + screen_color_add(window->ul_x, window->ul_y));

	return my_color;
}

int window_line_width(WindowPtr window) {
	int my_width;

	my_width = (window->lr_x - window->ul_x) - 3;

	return my_width;
}

void window_text_setup(WindowPtr window, int follow) {
	int my_color;

	cursor_set_follow(follow);
	cursor_set_pos(window->ul_x + 2, window->ul_y + 2);

	my_color = window_normal_color(window);

	screen_set_colors(my_color, my_color);
	screen_set_line_width(window_line_width(window));
}

void window_init_screen(WindowPtr main_window, char *name, char *author, char *version, char *lib_version, char *date) {
	char temp_buf[80];
	int len;

	window_set(main_window, 0, 0, 79, screen_max_y - 1);

	window_wipe(main_window);
	window_color(main_window, colorbyte(white, black));
	window_draw_box(main_window, WINDOW_SINGLE);

	Common::sprintf_s(temp_buf, " %s %s -- By %s -- Lib %s -- %s ", name, version, author, lib_version, date);
	len = strlen(temp_buf);
	screen_put(temp_buf, colorbyte(white, black), 7, 40 - (len >> 1), 0);
}

void window_draw_box(WindowPtr window, int type) {
	byte ul, ur, ll, lr, hl, vl;
	int a, xsize, ysize;
	byte *orig;
	byte *p1;
	byte *p2;

	switch (type) {
	case WINDOW_SINGLE: ul = ul_corner;
		ur = ur_corner;
		ll = ll_corner;
		lr = lr_corner;
		hl = horiz_frame;
		vl = vert_frame;
		break;

	case WINDOW_DOUBLE: ul = 201;
		ur = 187;
		ll = 200;
		lr = 188;
		hl = 205;
		vl = 186;
		break;


	case WINDOW_OBESE: ul = 219;
		ur = 219;
		ll = 219;
		lr = 219;
		hl = 219;
		vl = 219;
		break;

	default:
		return;  // Invalid line type
	};

	window_line_type = type;  // Set global for reference by other procs

	orig = screen + screen_char_add(window->ul_x, window->ul_y);
	xsize = window->lr_x - window->ul_x + 1;
	ysize = window->lr_y - window->ul_y + 1;

	p1 = orig;

	p2 = p1 + (ysize - 1) * text_wrap;
	*p1 = ul; p1 += 2;
	*p2 = ll; p2 += 2;

	for (a = 1; a <= xsize - 2; a++) {
		*p1 = hl;   p1 += 2;
		*p2 = hl;   p2 += 2;
	}

	*p1 = ur;
	*p2 = lr;

	p1 = orig + text_wrap;
	p2 = p1 + (xsize - 1) * 2;

	for (a = 1; a <= ysize - 2; a++) {
		*p1 = vl;  p1 += text_wrap;
		*p2 = vl;  p2 += text_wrap;
	}
}

/**
 * main interrupt 21 write string server
 */
static void window_server(void) {
	error("TODO: window_server");
}

static void window_21_install() {
	error("TODO: window_21_install");
}

static void window_21_remove() {
	error("TODO: window_21_remove");
}

static void window_21_server() {
	error("TODO: window_21_server");
}

void window_trap_output(WindowPtr window,
		void (*(any_char_routine))(),
		char *trap_string, ...) {
	va_list marker;
	char *next_trap_string = trap_string;
	byte *next_trap_routine = NULL;

	trap_window = window;

	if (trap_window != NULL) {
		trap_window_color = window_normal_color(window);
		screen_set_colors(trap_window_color, trap_window_color);

		trap_window_width = (window->lr_x - window->ul_x) - 1;
		screen_set_line_width(trap_window_width);

		cursor_set_pos(window->ul_x + 1, window->ul_y + 1);
	}

	window_incoming_string = window_incoming_buffer;
	window_next_char = window_incoming_buffer;

	window_any_char_routine = any_char_routine;

	window_num_trap_routines = 0;

	va_start(marker, trap_string);
	while (next_trap_string != NULL) {
		next_trap_routine = va_arg(marker, byte *);

		if (window_num_trap_routines < WINDOW_MAX_TRAP_ROUTINES) {
			Common::strcpy_s(window_trap_string[window_num_trap_routines], next_trap_string);
			window_trap_routine[window_num_trap_routines] = next_trap_routine;
			window_num_trap_routines++;
		}

		next_trap_string = va_arg(marker, char *);
	}

	window_21_install();

	window_server_installed = true;
}

void window_restore_output(void) {
	if (window_server_installed) {
		window_21_remove();
	}

	window_server_installed = false;
}

void window_define(WindowPtr window, int x1, int y1, int x2, int y2,
		int color, int shadow, int save) {
	window_set(window, x1, y1, x2, y2);
	if (save) window_create(window);

	window_wipe(window);
	window_color(window, color);

	if (shadow) {
		window_shadow(window);
	}

	window_draw_box(window, window_box_mode);
}

void window_center(WindowPtr window, const char *text, int line, int color) {
	int x, y;
	int width, len;

	len = strlen(text);
	y = window->ul_y + line + 1;
	width = (window->lr_x - window->ul_x) - 1;
	x = window->ul_x + (width >> 1) - (len >> 1) + 1;

	if (color == 0) color = window_normal_color(window);

	screen_put(text, color, color, x, y);
}

int window_show(WindowPtr window, const char *text, int xx, int yy) {
	int color;
	int x, y;
	int len;
	int width;
	int bonus;
	char temp_buf[80];

	y = window->ul_y + yy + 1;
	x = window->ul_x + xx + 1;

	width = (window->lr_x - window->ul_x) - 1;

	bonus = 0;
	if ((xx < 0) || (xx == width)) bonus = 1;
	len = strlen(text);
	len = MIN(len, (width - xx) + bonus);
	strncpy(temp_buf, text, len);
	temp_buf[len] = 0;

	color = window_normal_color(window);

	xx = screen_put(temp_buf, color, color, x, y);

	return xx - (window->ul_x + 1);
}

void window_repeat(WindowPtr window, char wipe_item, int x1, int x2, int y1) {
	int color;
	int x, y;
	int xx;
	int count;
	char temp[2];

	y = window->ul_y + y1 + 1;
	x = window->ul_x + x1 + 1;
	xx = window->ul_x + x2 + 1;

	color = window_normal_color(window);

	temp[0] = wipe_item;
	temp[1] = 0;

	for (count = x; count <= xx; count++) {
		screen_put(temp, color, color, count, y);
	}
}

void window_wipe_line(WindowPtr window, int yy) {
	window_repeat(window, ' ', 0, (window->lr_x - window->ul_x) - 2, yy);
}

int window_printf(WindowPtr window, int x, int y, char *string, ...) {
	va_list va;
	va_start(va, string);
	Common::String str = Common::String::vformat(string, va);
	va_end(va);

	window_show(window, str.c_str(), x, y);
	return 0;
}

void window_clear(WindowPtr window) {
	int count;
	int y_size;

	y_size = (window->lr_y - window->ul_y) - 1;

	for (count = 0; count < y_size; count++) {
		window_wipe_line(window, count);
	}
}

int window_show_color(WindowPtr window, char *text, int xx, int yy, int color) {
	int x, y;
	int len;
	int width;
	int bonus;
	char temp_buf[80];

	y = window->ul_y + yy + 1;
	x = window->ul_x + xx + 1;

	width = (window->lr_x - window->ul_x) - 1;

	bonus = 0;
	if ((xx < 0) || (xx == width)) bonus = 1;
	len = strlen(text);
	len = MIN(len, (width - xx) + bonus);
	strncpy(temp_buf, text, len);
	temp_buf[len] = 0;

	xx = screen_put(temp_buf, color, color, x, y);

	return xx - (window->ul_x + 1);
}

} // namespace MADSV2
} // namespace MADS
