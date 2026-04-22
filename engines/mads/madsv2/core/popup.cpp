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
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/popup.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/heap.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {


#define popup_padding_width     3       /* Extra space on each side */

Box text_box = { false };
Box *box = &text_box;

int popup_key = 0;
int popup_esc_key = false;
int popup_asking_number = false;

int popup_available = false;

int popup_preserve_initiator[3] = { BUFFER_PRESERVE,
								   BUFFER_PRESERVE,
								   BUFFER_PRESERVE };

byte popup_colors[24] = { 18, 19, 20, 21, 22, 23, 24, 25,
						 25, 24,  0,  0,  0,  3,  0,  0,
						 0,   0,  0,  0,  3,  0,  0,  0 };

BoxParam box_param = { NULL };
Popup *popup = NULL;
word popup_default_status = POPUP_STATUS_BAR;

static char *popup_savelist_string(PopupItem *item, int element);


int popup_create(int horiz_pieces, int x, int y) {
	int error_flag = true;
	int middle_width;
	int count;
	long mem_to_get;

	if (box == NULL) goto done;

	box->dialog_system = false;

	if (horiz_pieces < 0) {
		horiz_pieces = neg(horiz_pieces);
		box->dialog_system = true;
	}

	box->icon = NULL;
	box->icon_width = 0;
	box->icon_height = 0;
	box->icon_center = false;

	box->base_x = x;
	box->base_y = y;

	box->horiz_pieces = horiz_pieces;

	middle_width = pop_xs(POPUP_UPPER_CENTER) + ((pop_xs(POPUP_TOP) << 1) * horiz_pieces);

	box->xs = pop_xs(POPUP_UPPER_LEFT) + pop_xs(POPUP_UPPER_RIGHT) + middle_width;

	box->window_xs = box_param.extra_x + middle_width;

	box->cursor_x = 0;

	box->text_x = 0;
	box->text_y = 0;

	box->dont_add_space = false;

	font_set_colors(-1, POPUP_TEXT_COLOR, POPUP_TEXT_COLOR, POPUP_TEXT_COLOR);

	box->text_xs = box->window_xs - (popup_padding_width << 1);

	box->text_width = (box->text_xs / box_param.font->max_x_size) << 1;

	box->screen_buffer.data = NULL;
	box->depth_buffer.data  = NULL;

	box->screen_saved = false;
	box->depth_saved = false;

	box->active = true;

	if (!box->dialog_system) {
		mem_to_get = (box->text_width + 1) * POPUP_MAX_LINES;
		box->text[0] = (char *)mem_get_name(mem_to_get, "$poptext");

		if (box->text[0] == NULL) goto done;

		for (count = 1; count < POPUP_MAX_LINES; count++) {
			box->text[count] = box->text[count - 1] + box->text_width + 1;
		}

		for (count = 0; count < POPUP_MAX_LINES; count++) {
			*box->text[count] = 0;
			box->tab[count] = 0;
		}
	} else {
		box->text[0] = NULL;
	}

	error_flag = false;

done:
	return error_flag;
}

void popup_add_icon(SeriesPtr series, int id, int center) {
	box->icon = series;
	box->icon_id = id;

	box->icon_width = series->index[id - 1].xs + popup_padding_width;
	box->icon_height = series->index[id - 1].ys + popup_padding_width;

	box->icon_center = center;

	if (center) {
		box->cursor_x = 0;
		box->text_y = ((box->icon_height - 1) / (box_param.font->max_y_size + 1)) + 2;
	} else {
		box->cursor_x = box->icon_width;
	}
}



void popup_next_line(void) {
	int y_sum;

	box->text_y++;

	y_sum = box->text_y * (box_param.font->max_y_size + 1);
	if (y_sum > box->icon_height) {
		box->cursor_x = 0;
	} else {
		box->cursor_x = box->icon_width;
	}

	box->text_x = 0;
	// box->dont_add_space = false;
	if (box->text_y >= POPUP_MAX_LINES) {
		error_report(ERROR_POPUP_TOO_MANY_LINES, ERROR, MODULE_POPUP, box->text_y, POPUP_MAX_LINES);
	}
}


void popup_set_ask(void) {
	box->ask_x = box->text_x + 1;
	box->ask_y = box->text_y;
	popup_next_line();
}


void popup_add_string(const char *string) {
	int len, width;

	len = strlen(string);
	box->text_x += len;

	width = font_string_width(box_param.font, string, box_param.font_spacing) + box_param.font_spacing;
	box->cursor_x += width;

	Common::strcat_s(box->text[box->text_y], 65536, string);
}



void popup_write_string(const char *string) {
	char word[80];
	char word2[80];
	const char *marker;
	char *word_ptr;
	int any_space;
	int any_hyphen;
	int stop_on_hyphen = false;
	int going;
	int len;
	int width;
	int cr;

	marker = string;

	while (*marker != 0) {

		word_ptr = word;
		any_space = false;
		any_hyphen = false;
		cr = false;
		going = true;

		stop_on_hyphen = false;

		while (going) {

			*word_ptr++ = *marker;

			if (*marker == 0) {
				going = false;
			} else if (*marker == '\n') {
				going = false;
				cr = true;
				marker++;
				word_ptr--;
			} else if (*marker == 0x20) {  // Soft space
				marker++;
				any_space = true;
			} else if (*marker == '~') {  // Hard space
				word_ptr--;
				*word_ptr++ = ' ';
				marker++;
			} else if (any_space) {  // (Break after space)
				word_ptr--;
				going = false;
			} else if (*marker == '=') {  // Hard hyphen
				word_ptr--;
				*word_ptr++ = '-';
				if (*(marker + 1) == '-') {
					*(word_ptr - 1) = '{';
					marker++;
				}
				marker++;
			} else if (*marker == '-') {  // Soft hyphen
				if (*(marker + 1) == '-') {
					*(word_ptr - 1) = '{';  // Double hyphen
					marker++;
				}
				marker++;
				any_hyphen = true;
			} else if (any_hyphen) {  // (Break after hyphen)
				word_ptr--;
				going = false;
				stop_on_hyphen = true;
			} else {
				marker++;
			}
		}

		*word_ptr = 0;

		len = strlen(word);
		if (len > 0) {
			if (word[len - 1] == 0x20) {
				word[len - 1] = 0;
			}
		}

		word2[0] = 0;

		if ((box->text_x > 0) && !box->dont_add_space) {
			Common::strcat_s(word2, " ");
		}
		Common::strcat_s(word2, word);

		box->dont_add_space = stop_on_hyphen;

		len = strlen(word2);
		width = font_string_width(box_param.font, word2, POPUP_SPACING);	 // - POPUP_SPACING

		if (((box->text_x + len) > box->text_width) || ((box->cursor_x + width) > box->text_xs)) {
			popup_next_line();
			popup_add_string(word);
		} else {
			popup_add_string(word2);
		}
		if (cr) popup_next_line();
	}
}


void popup_bar(void) {
	if ((box->cursor_x > 0) || (box->text_x > 0)) {
		popup_next_line();
	}
	box->tab[box->text_y] = POPUP_BAR;
	popup_next_line();
}


void popup_underline(void) {
	box->tab[box->text_y] |= POPUP_UNDERLINE;
}


void popup_downpixel(void) {
	box->tab[box->text_y] |= POPUP_DOWNPIXEL;
}


void popup_tab(int tab_level) {
	box->tab[box->text_y] |= tab_level;
}


void popup_center_string(const char *string, int underline) {
	int width;

	if ((box->cursor_x > 0) || (box->text_x > 0)) {
		popup_next_line();
	}

	width = font_string_width(box_param.font, string, box_param.font_spacing);
	if ((width >= (int)box->text_xs) || ((int)strlen(string) >= box->text_width)) {
		popup_write_string(string);
	} else {
		box->tab[box->text_y] |= ((box->text_xs >> 1) - (width >> 1));

		Common::strcpy_s(box->text[box->text_y], 65536, string);

		if (underline) popup_underline();
	}

	popup_next_line();
}


static void pop_draw(int sprite, int x, int y, int depth_code) {
	matte_map_work_screen();

	sprite_draw(box_param.series, sprite, &scr_main, x, y);

	if (depth_code) {
		sprite_draw(box_param.series, sprite, &scr_orig, x + picture_map.pan_offset_x,
			y + picture_map.pan_offset_y);

		x += (pop_xs(sprite) >> 1);
		y += (pop_ys(sprite) - 1);

		sprite_draw_3d_scaled_to_attr(box_param.series, sprite,
			&scr_work, &scr_depth,
			x, y, 0, 100, picture_map.pan_offset_x,
			picture_map.pan_offset_y);
	}
}


int popup_draw(int save_screen, int depth_code) {
	int error_flag = true;
	int count;
	int base_text_size;
	int base_sprite_size;
	int base_size;
	int text_size;
	int top, left, right;
	int x_bonus, y_bonus;
	int depth_x2;
	int bottom_pieces;
	int x, y;
	int attr_depth_x;
	int attr_depth_xs;
	int attr_depth_x2;
	int loc_x, loc_y;
	int temp;
	int temp_y;
	int icon_padding;
	int icon_x;
	int sum_y;

	// Arrow cursor
	cursor_id = 1;
	if (cursor_id != cursor_last) {
		mouse_cursor_sprite(cursor, cursor_id);
		cursor_last = cursor_id;
	}

	if (!box->cursor_x) box->text_y--;

	// Find out how many vertical pieces are needed to contain the text,
	// and determine the resulting size.
	if (!box->dialog_system) {
		base_text_size = ((box_param.font->max_y_size + 1) * (box->text_y + 1)) + (popup_padding_width << 1);
	} else {
		base_text_size = (int)box->request_y_size;
	}

	base_sprite_size = 0;
	if (box->icon != NULL) {
		base_sprite_size = box->icon->index[box->icon_id - 1].ys + (popup_padding_width << 1);
	}
	base_size = MAX(base_text_size, base_sprite_size);

	box->vert_pieces = ((base_size - (box_param.extra_y + 1)) / pop_ys(POPUP_LEFT)) + 1;
	box->vert_pieces = MAX(0, box->vert_pieces);
	text_size = (pop_ys(POPUP_LEFT) * box->vert_pieces) + box_param.extra_y;

	box->window_ys = text_size;
	box->text_extra = text_size - base_text_size;

	box->ys = text_size - (box_param.extra_y)
		+ pop_ys(POPUP_UPPER_LEFT)
		+ pop_ys(POPUP_LOWER_LEFT);

	// Determine the true size of the entire window (figure out which border
	// pieces extend the furthest).
	top = pop_y(POPUP_UPPER_LEFT);
	top = MIN(top, pop_y(POPUP_UPPER_CENTER));
	top = MIN(top, pop_y(POPUP_TOP));

	y_bonus = pop_y(POPUP_UPPER_LEFT) - top;
	box->ys += y_bonus;



	left = pop_x(POPUP_UPPER_LEFT);
	left = MIN(left, pop_x(POPUP_LEFT));
	left = MIN(left, pop_x(POPUP_LOWER_LEFT));

	x_bonus = pop_x(POPUP_UPPER_LEFT) - left;
	box->xs += x_bonus;

	right = pop_x2(POPUP_UPPER_RIGHT);
	right = MAX(right, pop_x2(POPUP_RIGHT));
	right = MAX(right, pop_x2(POPUP_LOWER_RIGHT));

	box->xs += (right - pop_x2(POPUP_UPPER_RIGHT));

	// Determine the popup's coordinates on the screen
	if (box->base_x & POPUP_CENTER) {
		temp = (box->base_x & (~POPUP_CENTER));
		if (!temp) temp = (video_x >> 1);
		box->x = temp - (box->xs >> 1);
	} else {
		box->x = box->base_x;
	}

	if (box->base_y & POPUP_CENTER) {
		temp = (box->base_y & (~POPUP_CENTER));
		if (!temp) temp = (video_y >> 1);
		box->y = temp - (box->ys >> 1);
	} else {
		box->y = box->base_y;
	}

	while ((box->x > 0) && ((box->x + box->xs) > video_x - 1)) {
		box->x--;
	}

	while (((box->x + box->xs) <= video_x) && (box->x < 1)) {
		box->x++;
	}

	while ((box->y > 0) && ((box->y + box->ys) > video_y - 1)) {
		box->y--;
	}

	while (((box->y + box->ys) <= video_y) && (box->y < 1)) {
		box->y++;
	}

	// Compute base border point
	box->base_x = box->x + x_bonus;
	box->base_y = box->y + y_bonus;

	// Compute window location
	box->window_x = box->base_x + box_param.offset_x;
	box->window_y = box->base_y + box_param.offset_y;

	if (save_screen) {
		matte_map_work_screen();

		if (depth_code) {
			buffer_init(&box->screen_buffer, box->xs, box->ys);
			if (box->screen_buffer.data != NULL)
				buffer_rect_copy_2(scr_orig, box->screen_buffer,
					box->x + picture_map.pan_offset_x,
					box->y + picture_map.pan_offset_y,
					0, 0, box->xs, box->ys);
		} else {
			buffer_init(&box->screen_buffer, box->xs, box->ys);
			if (box->screen_buffer.data != NULL)
				buffer_rect_copy_2(scr_main, box->screen_buffer,
					box->x, box->y, 0, 0, box->xs, box->ys);
		}

		box->screen_saved = (box->screen_buffer.data != NULL);

		if (depth_code) {
			box->depth_x = box->x + picture_map.pan_offset_x;
			box->depth_xs = box->xs;
			if (box->depth_x & 1) {
				box->depth_x--;
				box->depth_xs++;
			}
			depth_x2 = box->depth_x + box->depth_xs - 1;
			if (!(depth_x2 & 1)) {
				box->depth_xs++;
			}
			box->depth_x = box->depth_x >> 1;
			box->depth_xs = box->depth_xs >> 1;

			buffer_init(&box->depth_buffer, box->depth_xs, box->ys);
			if (box->depth_buffer.data != NULL)
				buffer_rect_copy_2(scr_depth, box->depth_buffer,
					box->depth_x,
					box->y + picture_map.pan_offset_y,
					0, 0, box->depth_xs, box->ys);

			box->depth_saved = (box->depth_buffer.data != NULL);
		}
	}

	matte_map_work_screen();

	x = box->base_x;
	y = box->base_y;

	// Draw top edge
	pop_draw(POPUP_UPPER_LEFT, x, y, depth_code);

	x += pop_xs(POPUP_UPPER_LEFT);

	for (count = 0; count < box->horiz_pieces; count++) {
		pop_draw(POPUP_TOP, x, y + box_param.top_adjust_y, depth_code);
		x += pop_xs(POPUP_TOP);
	}

	pop_draw(POPUP_UPPER_CENTER, x, y + box_param.center_adjust_y, depth_code);
	x += pop_xs(POPUP_UPPER_CENTER);

	for (count = 0; count < box->horiz_pieces; count++) {
		pop_draw(POPUP_TOP, x, y + box_param.top_adjust_y, depth_code);
		x += pop_xs(POPUP_TOP);
	}

	pop_draw(POPUP_UPPER_RIGHT, x, y + box_param.upper_right_adjust_y, depth_code);

	// Draw right edge
	y += pop_ys(POPUP_UPPER_RIGHT);

	for (count = 0; count < box->vert_pieces; count++) {
		pop_draw(POPUP_RIGHT, x + box_param.right_adjust_x, y, depth_code);
		y += pop_ys(POPUP_RIGHT);
	}

	// Draw left edge
	x = box->base_x;
	y = box->base_y + pop_ys(POPUP_UPPER_LEFT);

	for (count = 0; count < box->vert_pieces; count++) {
		pop_draw(POPUP_LEFT, x + box_param.left_adjust_x, y, depth_code);
		y += pop_ys(POPUP_LEFT);
	}

	pop_draw(POPUP_LOWER_LEFT, x + box_param.lower_left_adjust_x, y, depth_code);

	// Draw bottom edge
	x += pop_xs(POPUP_LOWER_LEFT);

	bottom_pieces = (box->horiz_pieces << 1) + box_param.pieces_per_center;

	for (count = 0; count < bottom_pieces; count++) {
		pop_draw(POPUP_BOTTOM, x, y + box_param.bottom_adjust_y, depth_code);
		x += pop_xs(POPUP_BOTTOM);
	}

	pop_draw(POPUP_LOWER_RIGHT, x, y + box_param.lower_right_adjust_y, depth_code);

	// Fill center
	loc_y = box->window_y + popup_padding_width + (box->text_extra >> 1);
	loc_y += ((box_param.font->max_y_size + 1) * box->ask_y);


	box->fill_accum = buffer_rect_fill_pattern(scr_main, box->window_x, box->window_y,
		box->window_xs, box->window_ys,
		box->window_x, box->window_y, box->window_xs,
		POPUP_FILL_COLOR, POPUP_FILL_COLOR_2,
		0, loc_y);

	if (depth_code) {
		matte_guard_depth_0 = true;
		box->fill_accum = buffer_rect_fill_pattern(scr_orig,
			box->window_x + picture_map.pan_offset_x,
			box->window_y + picture_map.pan_offset_y,
			box->window_xs, box->window_ys,
			box->window_x + picture_map.pan_offset_x,
			box->window_y + picture_map.pan_offset_y,
			box->window_xs,
			POPUP_FILL_COLOR, POPUP_FILL_COLOR_2,
			0, loc_y);

		attr_depth_x = box->window_x + picture_map.pan_offset_x;
		attr_depth_xs = box->window_xs;
		if (attr_depth_x & 1) {
			attr_depth_x--;
			attr_depth_xs++;
		}
		attr_depth_x2 = attr_depth_x + attr_depth_xs - 1;
		if (!(attr_depth_x2 & 1)) {
			attr_depth_xs++;
		}
		attr_depth_x = attr_depth_x >> 1;
		attr_depth_xs = attr_depth_xs >> 1;

		buffer_rect_fill(scr_depth, attr_depth_x, box->window_y + picture_map.pan_offset_y,
			attr_depth_xs, box->window_ys, 0);
	}

	// Draw icon if any
	if (box->icon != NULL) {
		icon_x = 0;

		if (box->icon_center) {
			icon_x = ((box->window_xs - (popup_padding_width << 1)) - box->icon_width);
			if (icon_x > 0) {
				icon_x = icon_x >> 1;
			} else {
				icon_x = 0;
			}
		}

		sprite_draw(box->icon, box->icon_id, &scr_main,
			box->window_x + icon_x + popup_padding_width,
			box->window_y + popup_padding_width);

		if (depth_code) {
			sprite_draw(box->icon, box->icon_id, &scr_orig,
				box->window_x + icon_x + popup_padding_width + picture_map.pan_offset_x,
				box->window_y + popup_padding_width + picture_map.pan_offset_y);
		}
	}

	if (box->dialog_system) {
		error_flag = false;
		goto done;
	}

	// Draw text in box
	loc_y = box->window_y + popup_padding_width + (box->text_extra >> 1);
	sum_y = 0;

	for (count = 0; count < box->text_y + 1; count++) {
		if (box->tab[count] == POPUP_BAR) {
			buffer_rect_fill(scr_main,
				box->window_x, loc_y + ((box_param.font->max_y_size + 1) >> 1),
				box->window_xs, 1, (byte)POPUP_TEXT_COLOR);

			if (depth_code) {
				buffer_rect_fill(scr_orig,
					box->window_x + picture_map.pan_offset_x,
					loc_y + ((box_param.font->max_y_size + 1) >> 1) + picture_map.pan_offset_y,
					box->window_xs, 1, (byte)POPUP_TEXT_COLOR);
			}
		} else {
			icon_padding = (sum_y > box->icon_height) ? 0 : box->icon_width;
			loc_x = box->window_x + popup_padding_width + icon_padding +
				+(box->tab[count] & (~(POPUP_UNDERLINE | POPUP_DOWNPIXEL)));
			temp_y = loc_y;
			if (box->tab[count] & POPUP_DOWNPIXEL) {
				temp_y++;
			}
			font_write(box_param.font, &scr_main, box->text[count], loc_x, temp_y, box_param.font_spacing);
			if (depth_code) {
				font_write(box_param.font, &scr_orig, box->text[count],
					loc_x + picture_map.pan_offset_x,
					temp_y + picture_map.pan_offset_y, box_param.font_spacing);
			}
			if (box->tab[count] & POPUP_UNDERLINE) {
				buffer_rect_fill(scr_main, loc_x, temp_y + box_param.font->max_y_size,
					font_string_width(box_param.font, box->text[count], box_param.font_spacing),
					1, (byte)POPUP_TEXT_COLOR);
				if (depth_code) {
					buffer_rect_fill(scr_orig,
						loc_x + picture_map.pan_offset_x,
						temp_y + box_param.font->max_y_size + picture_map.pan_offset_y,
						font_string_width(box_param.font, box->text[count], box_param.font_spacing),
						1, (byte)POPUP_TEXT_COLOR);
				}
			}
		}

		loc_y += (box_param.font->max_y_size + 1);
		sum_y += (box_param.font->max_y_size + 1);
	}

	// Update live video
	mouse_hide();
	video_update(&scr_main, box->x, box->y, box->x, box->y, box->xs, box->ys);
	mouse_show();

	error_flag = false;

done:
	return error_flag;
}


void popup_destroy(void) {
	int x, y;
	int xs, ys;

	if (box->active && box->screen_saved) {
		if (box->depth_saved) {
			if (box->screen_buffer.data != NULL) {
				buffer_rect_copy_2(box->screen_buffer, scr_orig,
					0, 0,
					box->x + picture_map.pan_offset_x,
					box->y + picture_map.pan_offset_y,
					box->xs, box->ys);
				buffer_free(&box->screen_buffer);
			}
			if (box->depth_buffer.data != NULL) {
				buffer_rect_copy_2(box->depth_buffer, scr_depth,
					0, 0,
					box->depth_x, box->y + picture_map.pan_offset_y,
					box->depth_xs, box->ys);
				buffer_free(&box->depth_buffer);
			}

			matte_guard_depth_0 = false;

			matte_refresh_work();

			if (viewing_at_y) {
				matte_disable_screen_update = true;

				matte_map_work_screen();
				buffer_rect_fill(scr_main, 0, 0, video_x, viewing_at_y, 0);
				buffer_rect_fill(scr_main, 0, viewing_at_y + display_y, video_x, video_y, 0);
				matte_frame(false, false);

				matte_map_work_screen();

				mouse_hide();
				video_update(&scr_main, 0, 0, 0, 0, video_x, video_y);
				mouse_show();

				matte_disable_screen_update = false;
			}

		} else {
			matte_map_work_screen();
			if (box->screen_buffer.data != NULL) {
				buffer_rect_copy_2(box->screen_buffer, scr_main,
					0, 0, box->x, box->y, box->xs, box->ys);
				buffer_free(&box->screen_buffer);
			}

			matte_map_work_screen();

			x = box->x;
			y = box->y;
			xs = box->xs;
			ys = box->ys;

			buffer_conform(&scr_main, &x, &y, &xs, &ys);

			mouse_hide();
			video_update(&scr_main, x, y,
				x, y,
				xs, ys);
			mouse_show();
		}
	}

	if (box->active) {
		if (box->text[0] != NULL) {
			mem_free(box->text[0]);
		}
	}

	box->screen_saved = false;
	box->depth_saved = false;
	box->active = false;
}


int popup_and_wait(int save_screen) {
	bool error_flag = true;
	bool waiting;

	if (cursor != NULL) {
		cursor_last = 1;
		mouse_cursor_sprite(cursor, 1);
	}

	popup_key = 0;

	if (popup_draw(save_screen, false)) goto done;

	waiting = true;
	mouse_init_cycle();

	while (waiting) {
		mouse_begin_cycle(false);

		if (keys_any()) {
			popup_key = keys_get();
			waiting = false;
		}

		if (mouse_stop_stroke) waiting = false;

		mouse_end_cycle(false, waiting);

		if (g_engine->shouldQuit())
			waiting = false;
	}

	popup_destroy();

	error_flag = false;

done:
	return error_flag;
}


int popup_and_dont_wait(int save_screen) {
	int error_flag = true;

	if (cursor != NULL) {
		cursor_last = 1;
		mouse_cursor_sprite(cursor, 1);
	}

	popup_key = 0;

	if (popup_draw(save_screen, false)) goto done;

	error_flag = false;

done:
	return error_flag;
}



void popup_update_ask(char *string, int maxlen) {
	int x1, y1, x2, x3, xs, ys, xs2, xs3;

	xs = box->text_xs;
	ys = (box_param.font->max_y_size + 1);
	x1 = box->window_x + popup_padding_width;
	y1 = box->window_y + popup_padding_width + (ys * box->ask_y);

	matte_map_work_screen();

	buffer_rect_fill_pattern(scr_main, x1, y1, xs, ys,
		x1, y1, xs,
		POPUP_FILL_COLOR, POPUP_FILL_COLOR_2,
		box->fill_accum, 0);

	font_set_colors(-1, POPUP_TEXT_COLOR, POPUP_TEXT_COLOR, POPUP_TEXT_COLOR);

	x2 = font_write(box_param.font, &scr_main, box->text[box->ask_y], x1, y1, box_param.font_spacing);
	xs2 = (font_string_width(box_param.font, "W", box_param.font_spacing) * maxlen) + 4;

	x3 = x2 + 2;
	xs3 = font_string_width(box_param.font, string, box_param.font_spacing) + 2;

	buffer_rect_fill(scr_main, x2 - 1, y1 - 3, xs2, 1, 0);
	buffer_rect_fill(scr_main, x2 - 1, y1 + ys, xs2, 1, 0);
	buffer_rect_fill(scr_main, x2 - 1, y1 - 3, 1, ys + 4, 0);
	buffer_rect_fill(scr_main, x2 + xs2 - 1, y1 - 3, 1, ys + 4, 0);

	buffer_rect_fill_swap(scr_main, x3 - 1, y1 - 1, xs2 - 4, ys, POPUP_FILL_COLOR, POPUP_HILITE_COLOR);
	buffer_rect_fill_swap(scr_main, x3 - 1, y1 - 1, xs2 - 4, ys, POPUP_FILL_COLOR_2, POPUP_HILITE_COLOR_2);

	font_set_colors(-1, POPUP_ASK_COLOR, POPUP_ASK_COLOR, POPUP_ASK_COLOR);
	font_write(box_param.font, &scr_main, string, x3, y1, box_param.font_spacing);

	mouse_hide();
	video_update(&scr_main, x1, y1 - 3, x1, y1 - 3, xs, ys + 4);
	mouse_show();
}


int popup_ask_string(char *target, int maxlen, int save_screen) {
	int error_flag = true;
	int never_ever;
	int going, len, mykey;
	int value;
	char temp_buf[80];

	never_ever = box_param.erase_on_first;

	popup_esc_key = false;

	Common::strcpy_s(temp_buf, target);

	if (box->text[box->ask_y][strlen(box->text[box->ask_y]) - 1] != ' ') {
		Common::strcat_s(box->text[box->ask_y], 65536, " ");
	}

	if (popup_draw(save_screen, false)) goto done;

	popup_update_ask(temp_buf, maxlen);

	going = true;

	keys_disable();

	mouse_init_cycle();

	while (going) {
		len = strlen(temp_buf);
		while (!keys_any()) {
			mouse_begin_cycle(false);
			if (mouse_stop_stroke) {
				error_flag = 1;
				popup_esc_key = true;
				going = false;
				goto done;
			}
			mouse_end_cycle(false, true);
		}
		mykey = keys_get();
		switch (mykey) {
		case esc_key:
		case alt_x_key:
		case alt_q_key:
		case ctrl_q_key:
		case ctrl_x_key:
			error_flag = 1;
			popup_esc_key = true;
			going = false;
			goto done;
			break;
		case enter_key:
			going = false;
			break;
		case pgup_key:
		case pgdn_key:
		case home_key:
		case end_key:
			if (popup_asking_number) {
				value = atoi(temp_buf);
				switch (mykey) {
				case pgup_key:
					value--;
					break;
				case pgdn_key:
					value++;
					break;
				case home_key:
					value -= 10;
					break;
				case end_key:
				default:
					value += 10;
					break;
				}
				mads_itoa(value, temp_buf, 10);
				never_ever = true;
			}
			break;

		case bksp_key:
			never_ever = false;
			if (len > 0) {
				temp_buf[len - 1] = 0;
			}
			break;
		default:
			if (never_ever) {
				len = 0;
				temp_buf[0] = 0;
				never_ever = false;
			}
			if (Common::isPrint(mykey)) {
				if (len < maxlen) {
					temp_buf[len] = (byte)mykey;
					temp_buf[len + 1] = 0;
				}
			}
			break;
		}
		popup_update_ask(temp_buf, maxlen);
	}

	error_flag = false;

done:
	keys_enable();
	popup_destroy();
	Common::strcpy_s(target, 65536, temp_buf);
	return error_flag;
}



int popup_ask_number(long *value, int maxlen, int save_screen) {
	int error_flag = true;
	char temp_buf[80];

	popup_asking_number = true;

	if (value) {
		snprintf(temp_buf, 80, "%ld", *value);
	} else {
		temp_buf[0] = 0;
	}

	if (popup_ask_string(temp_buf, maxlen, save_screen)) goto done;

	*value = atol(temp_buf);

	error_flag = false;

done:
	popup_asking_number = false;
	return error_flag;
}



int popup_estimate_pieces(int maxlen) {
	int font_len;
	int pieces;
	int estimate;

	font_len = maxlen * (box_param.font->max_x_size + box_param.font_spacing);
	pieces = ((font_len - 1) / pop_xs(POPUP_TOP)) + 1;
	estimate = (pieces - box_param.pieces_per_center) >> 1;
	return (estimate);
}


int popup_get_string(char *target, const char *top, const char *left, int maxlen) {
	int result = -1;

	if (!popup_create(popup_estimate_pieces(strlen(top) + 4), POPUP_CENTER, POPUP_CENTER)) {
		popup_center_string(top, true);
		popup_write_string("\n");
		popup_write_string(left);
		popup_set_ask();
		popup_write_string("\n");
		result = popup_ask_string(target, maxlen, true);
	}

	return (result);
}


int popup_get_long(long *value, const char *top, const char *left, int maxlen) {
	int error_flag = true;

	if (!popup_create(popup_estimate_pieces(strlen(top) + 4), POPUP_CENTER, POPUP_CENTER)) {
		popup_center_string(top, true);
		popup_write_string("\n");
		popup_write_string(left);
		popup_set_ask();
		popup_write_string("\n");
		error_flag = popup_ask_number(value, maxlen, true);
	}

	return error_flag;
}


int popup_get_number(int16 *value, const char *top, const char *left, int maxlen) {
	int result;
	long temp;

	temp = *value;

	result = popup_get_long(&temp, top, left, maxlen);
	if (!result) {
		*value = (int)temp;
	}
	return (result);
}


int popup_alert(int width, const char *message_line, ...) {
	int mykey = -1;
	int error_flag = true;
	int popup_created = false;
	int first_time = true;
	va_list marker;
	const char *my_message = message_line;

	if (popup_create(popup_estimate_pieces(width), POPUP_CENTER, POPUP_CENTER)) goto done;
	popup_created = true;

	va_start(marker, message_line);
	while (my_message != NULL) {
		popup_center_string(my_message, first_time);
		my_message = va_arg(marker, char *);
		first_time = false;
	}

	error_flag = popup_and_wait(true);

	mykey = popup_key;

done:
	if (error_flag && popup_created) popup_destroy();
	return mykey;
}

int popup_box_load(void) {
	int error_flag = true;
	int count;
	int top_base, bottom_base, left_base, right_base;
	int extra_x1, extra_y1;
	int extra_x2, extra_y2;
	byte special_color;
	Buffer data_buf;

	data_buf.x = 1;  // A one pixel buffer
	data_buf.y = 1;
	data_buf.data = &special_color;

	if (box_param.series != NULL) {
		sprite_free(&box_param.series, true);
	}

	palette_low_search_limit = 0;
	palette_high_search_limit = POPUP_BASE_COLOR + POPUP_NUM_COLORS;

	for (count = 0; count < POPUP_NUM_COLORS; count++) {
		color_status[POPUP_BASE_COLOR + count] = 0;
	}

	box_param.series = sprite_series_load(box_param.name, PAL_MAP_RESERVED);
	if (box_param.series == NULL) goto done;

	box_param.logo = sprite_series_load("*LOGO.SS", PAL_MAP_RESERVED);
	box_param.menu = sprite_series_load("*MENU.SS", PAL_MAP_RESERVED);

	if (box_param.menu != NULL) {
		box_param.menu_left_width = box_param.menu->index[0].xs;
		box_param.menu_middle_width = box_param.menu->index[1].xs;
		box_param.menu_right_width = box_param.menu->index[2].xs;

		box_param.menu_text_x_offset = 3;
		box_param.menu_text_y_offset = 5;
		box_param.menu_text_x_bonus = 1;
		box_param.menu_text_y_bonus = 1;
	}

	mcga_setpal_range(&master_palette, POPUP_BASE_COLOR, POPUP_NUM_COLORS);

	for (count = 0; count < POPUP_NUM_COLORS; count++) {
		color_status[POPUP_BASE_COLOR + count] |= PAL_RESERVED;
	}

	sprite_draw(box_param.series, 10, &data_buf, 0, 0);
	POPUP_FILL_COLOR = special_color;
	sprite_draw(box_param.series, 11, &data_buf, 0, 0);
	POPUP_FILL_COLOR_2 = special_color;
	sprite_draw(box_param.series, 12, &data_buf, 0, 0);
	POPUP_TEXT_COLOR = special_color;

	sprite_draw(box_param.series, 13, &data_buf, 0, 0);
	POPUP_DIALOG_TEXT_COLOR = special_color;
	sprite_draw(box_param.series, 14, &data_buf, 0, 0);
	POPUP_DIALOG_BUTTON_COLOR_1 = special_color;
	sprite_draw(box_param.series, 15, &data_buf, 0, 0);
	POPUP_DIALOG_BUTTON_COLOR_2 = special_color;
	sprite_draw(box_param.series, 16, &data_buf, 0, 0);
	POPUP_DIALOG_BORDER_COLOR_1 = special_color;
	sprite_draw(box_param.series, 17, &data_buf, 0, 0);
	POPUP_DIALOG_BORDER_COLOR_2 = special_color;
	sprite_draw(box_param.series, 18, &data_buf, 0, 0);
	POPUP_DIALOG_SELECT_COLOR = special_color;
	sprite_draw(box_param.series, 19, &data_buf, 0, 0);
	POPUP_DIALOG_STRING_COLOR = special_color;

	sprite_draw(box_param.series, 20, &data_buf, 0, 0);
	POPUP_DIALOG_MENU_TEXT_1 = special_color;
	sprite_draw(box_param.series, 21, &data_buf, 0, 0);
	POPUP_DIALOG_MENU_TEXT_2 = special_color;
	sprite_draw(box_param.series, 22, &data_buf, 0, 0);
	POPUP_DIALOG_MENU_TEXT_3 = special_color;


	top_base = pop_y(POPUP_UPPER_LEFT);

	box_param.top_adjust_y = pop_y(POPUP_TOP) - top_base;
	box_param.center_adjust_y = pop_y(POPUP_UPPER_CENTER) - top_base;
	box_param.upper_right_adjust_y = pop_y(POPUP_UPPER_RIGHT) - top_base;

	left_base = pop_x(POPUP_UPPER_LEFT);

	box_param.left_adjust_x = pop_x(POPUP_LEFT) - left_base;
	box_param.lower_left_adjust_x = pop_x(POPUP_LOWER_LEFT) - left_base;

	bottom_base = pop_y(POPUP_LOWER_LEFT);

	box_param.bottom_adjust_y = pop_y(POPUP_BOTTOM) - bottom_base;
	box_param.lower_right_adjust_y = pop_y(POPUP_LOWER_RIGHT) - bottom_base;

	right_base = pop_x(POPUP_UPPER_RIGHT);

	box_param.right_adjust_x = pop_x(POPUP_RIGHT) - right_base;

	box_param.offset_y = (pop_y2(POPUP_TOP) - pop_y(POPUP_UPPER_LEFT)) + 1;
	box_param.offset_x = (pop_x2(POPUP_LEFT) - pop_x(POPUP_UPPER_LEFT)) + 1;

	extra_x1 = pop_xs(POPUP_UPPER_LEFT) - box_param.offset_x;
	extra_y1 = pop_ys(POPUP_UPPER_LEFT) - box_param.offset_y;

	extra_x2 = pop_x(POPUP_RIGHT) - pop_x(POPUP_UPPER_RIGHT);
	extra_y2 = pop_y2(POPUP_UPPER_RIGHT) - pop_y2(POPUP_TOP);

	box_param.extra_x = extra_x1 + extra_x2;
	box_param.extra_y = extra_y1 + extra_y2;

	box_param.pieces_per_center = pop_xs(POPUP_UPPER_CENTER) / pop_xs(POPUP_BOTTOM);

	box_param.erase_on_first = true;

	box_param.font = font_inter;
	box_param.font_spacing = 0;

	box_param.menu_font = font_menu;
	box_param.menu_font_spacing = 0;

	error_flag = false;

done:
	palette_low_search_limit = 0;
	palette_high_search_limit = 256;

	return error_flag;
}


/**
 * Causes the specified popup structure to become active (the active
 * popup structure is considered the target of all popup requests
 * which do not include a Popup structure or pointer in their
 * parameter lists).
 */
static Popup *popup_activate(Popup *newpop) {
	if (newpop != NULL) {
		popup = newpop;
	}
	return (popup);
}

/**
 * Executes the specified function vector for the specified item.
 * Returns function's return value if function exists; if vector
 * is NULL, always returns 0.  A far pointer to the PopupItem structure
 * is passed on the stack to the function routine.
 */
static int popup_exec_function(PopupItem *item, int function) {
	if (item == NULL || item->vector[function] == NULL)
		return 0;

	return item->vector[function](item);
}

/**
 * Initializes an empty item.
 */
static void popup_item_init(PopupItem *item) {
	int count;

	item->type = ITEM_BLANK;
	item->status = 0;
	item->x = 0;
	item->y = 0;
	item->xs = 0;
	item->ys = 0;
	item->font_x = 0;
	item->font_y = 0;
	item->prompt = NULL;
	item->buffer = NULL;
	item->list = NULL;

	for (count = 0; count < POPUP_ITEM_VECTORS; count++) {
		item->vector[count] = NULL;
	}
}


Popup *popup_dialog_create(void *memory, long heap_size, int max_items) {
	int   count;
	byte  *block = NULL;
	word  status;
	long  item_memory;
	Popup *result = NULL;
	Popup *dlg;

	status = popup_default_status;

	if ((heap_size == 0) && (memory == NULL)) heap_size = 2048;
	if (!max_items) max_items = 10;

	if (heap_size < (sizeof(Popup) + 400)) goto done;

	if (memory == NULL) {

		status |= POPUP_STATUS_DYNAMIC;
		block = (byte *)mem_get_name(heap_size, "$popheap");
		if (block == NULL) goto done;

		dlg = (Popup *)block;
		memory = block;

	} else {

		dlg = (Popup *)memory;

	}

	heap_declare(&dlg->heap, MODULE_POPUP, (char *)memory + sizeof(Popup),
		heap_size - sizeof(Popup));

	item_memory = sizeof(PopupItem) * max_items;
	dlg->item = (PopupItem *)heap_get(&dlg->heap, item_memory);
	dlg->max_items = max_items;
	dlg->num_items = 0;

	for (count = 0; count < dlg->max_items; count++) {
		popup_item_init(&dlg->item[count]);
	}

	dlg->x = POPUP_CENTER;
	dlg->y = POPUP_CENTER;
	dlg->width = 0;
	dlg->y_position = 0;

	dlg->y_spacing = 2;

	dlg->button_spacing = 4;
	dlg->button_left_fill = 0;
	dlg->button_right_fill = 0;
	dlg->button_bar_color = POPUP_DIALOG_BORDER_COLOR_2;

	status |= POPUP_STATUS_VALID;

	dlg->status = status;

	dlg->active_item = NULL;
	dlg->enter_item = NULL;
	dlg->cancel_item = NULL;
	dlg->string_item = NULL;

	dlg->list_item = NULL;
	dlg->clear_item = NULL;

	dlg->key = 0;
	dlg->key_handled = true;

	dlg->mouse_status = 0;

	popup_activate(dlg);
	result = dlg;

done:
	if ((block != NULL) && (result != (Popup *)block)) {
		mem_free(block);
	}
	return (result);
}


Popup *popup_dialog_destroy(void) {
	popup->status &= ~(POPUP_STATUS_VALID);

	if (popup->status & POPUP_STATUS_DYNAMIC) {
		mem_free(popup);
	}

	return (NULL);
}


static int extract_keystroke(char *string) {
	int keystroke = 0;
	char *mark;

	mark = strchr(string, '~');
	if (mark != NULL) {
		keystroke = *(mark + 1);
		Common::strcpy_s(mark, 65536, mark + 1);
	}

	return keystroke;
}


static void *popup_heap(long mem_to_get) {
	return (heap_get(&popup->heap, mem_to_get));
}


static char *string_to_heap(char *string) {
	char *mem_we_got;

	mem_we_got = (char *)popup_heap(strlen(string) + 1);

	Common::strcpy_s(mem_we_got, 65536, string);

	return (mem_we_got);
}


static void set_prompt(PopupItem *item, const char *string) {
	char temp_buf[80];

	Common::strcpy_s(temp_buf, string);

	item->keystroke = extract_keystroke(temp_buf);
	item->prompt = string_to_heap(temp_buf);
}


static PopupItem *item_allocate(int can_be_default) {
	PopupItem *item;

	if (popup->num_items >= popup->max_items) {
		error_report(ERROR_POPUP_NO_ITEMS, ERROR, MODULE_POPUP, popup->max_items, 0);
	}

	item = &popup->item[popup->num_items++];

	if (can_be_default && (popup->active_item == NULL)) {
		popup->active_item = item;
	}

	return (item);
}



static PopupList *list_allocate(void) {
	PopupList *list;

	list = (PopupList *)popup_heap(sizeof(PopupList));
	return (list);
}


static PopupBuffer *buffer_allocate(int size) {
	PopupBuffer *buffer;

	buffer = (PopupBuffer *)popup_heap(sizeof(PopupBuffer));
	buffer->data = (char *)popup_heap(size);
	buffer->max_length = size;

	*buffer->data = 0;

	return (buffer);
}



static int popup_font_size(const char *string) {
	return (font_string_width(box_param.font, string, box_param.font_spacing));
}


static void popup_y_placement(PopupItem *item, int y) {
	int16 bottom;

	if (y == POPUP_FILL) {
		item->y = popup->y_position;
		popup->y_position += item->ys + popup->y_spacing;
	} else {
		item->y = y;
		bottom = item->y + item->ys + popup->y_spacing;
		popup->y_position = MAX(popup->y_position, bottom);
	}
}



static void popup_x_width_check(PopupItem *item) {
	int16 new_width;

	if (item->x & POPUP_CENTER) {
		new_width = item->xs;
		new_width += (new_width & 1);
	} else if (item->x & POPUP_RIGHT_JUST) {
		new_width = item->xs + (item->x & (~(POPUP_RIGHT_JUST)));
	} else {
		new_width = item->x + item->xs;
	}

	popup->width = MAX(popup->width, new_width);
}


void popup_width_force(int width) {
	popup->width = MAX<int16>(popup->width, width);
}


static void popup_coord_adjust(PopupItem *item) {
	if (item->x & POPUP_CENTER) {
		item->x = popup->x + (popup->xs >> 1) - (item->xs >> 1);
	} else if (item->x & POPUP_BUTTON_RIGHT) {
		item->x = (popup->x + popup->xs) - (item->x & ~POPUP_BUTTON_RIGHT);
	} else if (item->x & POPUP_RIGHT_JUST) {
		item->x = (popup->x + popup->xs) - ((item->x & ~POPUP_RIGHT_JUST) + item->xs);
	} else {
		item->x += popup->x;
	}

	if (item->y == POPUP_BUTTON_ROW) {
		item->y = popup->button_y + popup->y;
	} else {
		item->y += popup->y;
	}

	item->font_x = item->x + item->font_x;
	item->font_y = item->y + item->font_y;
}


static int popup_mouse_refresh(void) {
	mouse_freeze();
	mouse_set_work_buffer(scr_main.data, video_x);
	mouse_set_view_port_loc(0, 0, video_x - 1, video_y - 1);
	mouse_set_view_port(0, 0);
	return (mouse_refresh_view_port());
}


static void popup_mouse_refresh_2(int refresh) {
	if (refresh) mouse_refresh_done();
	mouse_thaw();
}


static void popup_to_screen(void) {
	int refresh;

	refresh = popup_mouse_refresh();
	video_update(&scr_main, box->x, box->y,
		box->x, box->y,
		box->xs, box->ys);
	popup_mouse_refresh_2(refresh);
}


static void popup_item_to_screen(PopupItem *item) {
	int refresh;

	refresh = popup_mouse_refresh();
	video_update(&scr_main, item->x, item->y,
		item->x, item->y,
		item->xs, item->ys);
	popup_mouse_refresh_2(refresh);
}


static void popup_screen_clear(void) {
	buffer_rect_fill_pattern(scr_main,
		box->window_x, box->window_y, box->window_xs, box->window_ys,
		box->window_x, box->window_y, box->window_xs,
		POPUP_FILL_COLOR, POPUP_FILL_COLOR_2, 0, 0);
}


static void popup_item_clear(PopupItem *item) {
	if (mouse_y > 150 && item->y > 150) {
		buffer_rect_fill(scr_main, item->x, item->y, item->xs, 1, 0);
		buffer_rect_fill(scr_main, item->x, item->y, 1, item->ys, 0);
		buffer_rect_fill(scr_main, item->x, item->y + item->ys, item->xs + 1, 1, 0);
		buffer_rect_fill(scr_main, item->x + item->xs, item->y, 1, item->ys, 0);

	} else {
		buffer_rect_fill_pattern(scr_main,
			item->x, item->y, item->xs, item->ys,
			box->window_x, box->window_y, box->window_xs,
			POPUP_FILL_COLOR, POPUP_FILL_COLOR_2, 0, 0);
	}

}


static void popup_redraw_item(PopupItem *item) {
	popup_item_clear(item);
	popup_exec_function(item, VECTOR_DRAW);
	popup_item_to_screen(item);
}


static void popup_update_item(PopupItem *item) {
	if (item) {
		if (item->vector[VECTOR_UPDATE] != NULL) {
			popup_exec_function(item, VECTOR_UPDATE);
		} else {
			popup_redraw_item(item);
		}
	}
}


static void popup_full_draw(void) {
	int count;
	int bar_y;
	int bar_x;
	int bar_xs;

	popup_screen_clear();

	for (count = 0; count < popup->num_items; count++) {
		popup_exec_function(&popup->item[count], VECTOR_DRAW);
	}

	if (popup->status & POPUP_STATUS_BUTTON) {
		if (popup->status & POPUP_STATUS_BAR) {
			if (popup->status & POPUP_STATUS_STEAL) {
				bar_x = box->window_x;
				bar_xs = box->window_xs;
			} else {
				bar_x = popup->x - popup_padding_width;
				bar_xs = popup->xs + (popup_padding_width << 1);
			}
			bar_y = popup->button_y - (popup_padding_width + 1);
			buffer_rect_fill(scr_main, bar_x, bar_y,
				bar_xs, 1, (byte)popup->button_bar_color);
		}
	}

	popup_to_screen();
}


static void popup_activate_item(PopupItem *item) {
	PopupItem *old_active;

	old_active = popup->active_item;
	popup->active_item = item;

	popup_update_item(old_active);
	popup_update_item(popup->active_item);
	popup_update_item(popup->enter_item);
}



static PopupItem *popup_next_item(PopupItem *item, int activate, int any_non_button) {
	int count;
	int found = -1;
	PopupItem *result;

	if (item == NULL) {
		result = &popup->item[0];
		goto done;
	}

	if (any_non_button != 1) {
		any_non_button = (any_non_button != 0) || ((item->type != ITEM_BUTTON) && !(item->status & ITEM_STATUS_INERT));
		if (any_non_button)
			any_non_button = -1;
	}

	for (count = 0; (found < 0) && (count < popup->num_items); count++) {
		if (item == &popup->item[count]) found = count;
	}

	if ((found >= popup->num_items - 1) || (found < 0)) {
		result = &popup->item[0];
	} else {
		result = &popup->item[found + 1];
	}

	if (activate && (result->status & ITEM_STATUS_INERT)) {
		result = popup_next_item(result, true, any_non_button);
	}

	if (any_non_button != 1) {
		any_non_button = (any_non_button != 0) || ((result->type != ITEM_BUTTON) && !(result->status & ITEM_STATUS_INERT));
		if (any_non_button) any_non_button = -1;
	}

	if (activate && (result == popup->enter_item) && any_non_button && (any_non_button != 1)) {
		result = popup_next_item(result, true, 1);
	}

done:
	return (result);
}


static PopupItem *popup_last_item(PopupItem *item, int activate, int any_non_button) {
	int found = -1;
	int count;
	PopupItem *result;

	if (item == NULL) {
		result = &popup->item[popup->num_items - 1];
		goto done;
	}

	if (any_non_button != 1) {
		any_non_button = (any_non_button != 0) || ((item->type != ITEM_BUTTON) && !(item->status & ITEM_STATUS_INERT));
		if (any_non_button) any_non_button = -1;
	}

	for (count = 0; (found < 0) && (count < popup->num_items); count++) {
		if (item == &popup->item[count]) found = count;
	}

	if (found <= 0) {
		result = &popup->item[popup->num_items - 1];
	} else {
		result = &popup->item[found - 1];
	}

	if (activate && (result->status & ITEM_STATUS_INERT)) {
		result = popup_last_item(result, true, any_non_button);
	}

	if (any_non_button != 1) {
		any_non_button = (any_non_button != 0) || ((result->type != ITEM_BUTTON) && !(result->status & ITEM_STATUS_INERT));
		if (any_non_button) any_non_button = -1;
	}

	if (activate && (result == popup->enter_item) && any_non_button && (any_non_button != 1)) {
		result = popup_last_item(result, true, 1);
	}

done:
	return (result);
}



static int popup_in_item(PopupItem *item) {
	int in_item;
	int x, y;
	int x2, y2;

	x = item->x;
	y = item->y;
	x2 = x + item->xs - 1;
	y2 = y + item->ys - 1;

	in_item = ((mouse_x >= x) && (mouse_x <= x2) &&
		(mouse_y >= y) && (mouse_y <= y2));

	return (in_item);
}


static void popup_find_item(void) {
	int count;

	popup->mouse_item = NULL;

	for (count = 0; (popup->mouse_item == NULL) && (count < popup->num_items); count++) {
		if (popup_in_item(&popup->item[count])) {
			popup->mouse_item = &popup->item[count];
		}
	}

	if (popup->mouse_item != NULL) {
		popup_activate_item(popup->mouse_item);
	}
}


static void popup_double_box(int x, int y, int xs, int ys,
	byte color, byte color2) {
	if (popup->status & POPUP_STATUS_BUTTON) {
		buffer_rect_fill(scr_main, x, y, xs, 1, color);
		buffer_rect_fill(scr_main, x, y, 1, ys, color);
		buffer_rect_fill(scr_main, x, y + ys, xs + 1, 1, color);
		buffer_rect_fill(scr_main, x + xs, y, 1, ys, color);

	} else {
		buffer_rect_fill(scr_main, x + 1, y, xs - 2, 1, color);
		buffer_rect_fill(scr_main, x, y + 1, 2, 1, color);
		buffer_rect_fill(scr_main, x + 2, y + 1, xs - 2, 1, color2);

		buffer_rect_fill(scr_main, x, y + 2, 1, ys - 3, color);
		buffer_rect_fill(scr_main, x + 1, y + 2, 1, ys - 2, color2);

		buffer_rect_fill(scr_main, x + xs - 2, y + 2, 1, ys - 4, color);
		buffer_rect_fill(scr_main, x + xs - 1, y + 2, 1, ys - 3, color2);

		buffer_rect_fill(scr_main, x + 2, y + ys - 2, xs - 4, 1, color);
		buffer_rect_fill(scr_main, x + xs - 2, y + ys - 2, 1, 1, color2);
		buffer_rect_fill(scr_main, x + 1, y + ys - 1, xs - 2, 1, color2);
	}
}


static void popup_double_box_2(int x, int y, int xs, int ys,
	byte color, byte color2) {
	int count;

	for (count = 0; count < 2; count++) {
		buffer_rect_fill(scr_main, x, y, xs, 1, color);
		buffer_rect_fill(scr_main, x, y + ys - 1, xs, 1, color);
		buffer_rect_fill(scr_main, x, y, 1, ys, color);
		buffer_rect_fill(scr_main, x + xs - 1, y, 1, ys, color);

		x++;
		y++;
		xs -= 2;
		ys -= 2;
		color = color2;
	}
}





/*******************************************************/
/*             Item service routines                   */
/*******************************************************/

/***********/
/* Buttons */
/***********/

static int popup_button_x_size(PopupItem *item) {
	int size;

	size = popup_font_size(item->prompt);
	size += 6 + 2;

	return (size);
}


static int popup_button_y_size(PopupItem *item) {
	item = NULL;  // delete if this routine is to be used
	return (box_param.font->max_y_size + 4 + 2);
}


static int popup_button_draw(PopupItem *item) {
	byte color;
	byte color2;
	byte color_text;
	int selected;
	int big_button;
	int x, y;
	int refresh;

	selected = (popup->active_item == item) && (item->status & ITEM_STATUS_ACTIVE) != 0;

	if (selected /* && (POPUP_DIALOG_BUTTON_COLOR_1 == POPUP_DIALOG_BUTTON_COLOR_2) */) {
		buffer_rect_fill(scr_main, item->x, item->y, item->xs, 1, POPUP_DIALOG_TEXT_COLOR);
		buffer_rect_fill(scr_main, item->x, item->y, 1, item->ys, POPUP_DIALOG_TEXT_COLOR);
		buffer_rect_fill(scr_main, item->x, item->y + item->ys, item->xs + 1, 1, POPUP_DIALOG_TEXT_COLOR);
		buffer_rect_fill(scr_main, item->x + item->xs, item->y, 1, item->ys, POPUP_DIALOG_TEXT_COLOR);

		refresh = popup_mouse_refresh();
		video_update(&scr_main, item->x, item->y,
			item->x, item->y,
			item->xs + 5, item->ys + 5);
		popup_mouse_refresh_2(refresh);
	}

	// color = selected ? POPUP_ASK_COLOR : POPUP_TEXT_COLOR;
	if (popup->active_item->type == ITEM_BUTTON) {
		big_button = (item == popup->active_item);
	} else {
		big_button = (item == popup->enter_item);
	}

	color_text = POPUP_DIALOG_TEXT_COLOR;
	if (!selected) {
		color = POPUP_DIALOG_BUTTON_COLOR_1;
		color2 = POPUP_DIALOG_BUTTON_COLOR_2;
	} else {
		color2 = POPUP_DIALOG_BUTTON_COLOR_1;
		color = POPUP_DIALOG_BUTTON_COLOR_2;
	}

	if (!selected) {
		if (!big_button) {
			buffer_rect_fill(scr_main, item->x, item->y, item->xs, 1, color2);
			buffer_rect_fill(scr_main, item->x, item->y, 1, item->ys, color2);
			buffer_rect_fill(scr_main, item->x, item->y + item->ys, item->xs + 1, 1, color2);
			buffer_rect_fill(scr_main, item->x + item->xs, item->y, 1, item->ys, color2);

		} else {
			popup_double_box(item->x, item->y, item->xs, item->ys, color, color2);
		}
	}

	font_set_colors(-1, color_text, color_text, color_text);

	x = item->font_x;
	y = item->font_y;

	font_write(box_param.font, &scr_main, item->prompt,
		x + 1, y + 1, box_param.font_spacing);

	if (!(popup->status & POPUP_STATUS_EXIT)) {
		refresh = popup_mouse_refresh();
		video_update(&scr_main, item->x, item->y,
			item->x, item->y,
			item->xs + 5, item->ys + 5);
		popup_mouse_refresh_2(refresh);
	}

	return 0;
}





static int popup_button_mouse(PopupItem *item) {
	int in_item;
	int count;
	word status;
	char *text_locator;
	PopupList *list;
	PopupBuffer *buffer;

	status = item->status;

	in_item = popup_in_item(item);

	if (mouse_stroke_going && !in_item) {
		for (count = 0; count < popup->num_items; count++) {
			if (popup_in_item(&popup->item[count]) && (popup->item[count].type == ITEM_BUTTON)) {
				popup->mouse_item = &popup->item[count];
				popup_activate_item(popup->mouse_item);
				popup_exec_function(popup->mouse_item, VECTOR_MOUSE);
				goto done;
			}
		}
	}

	if (mouse_stroke_going && in_item) {
		item->status |= ITEM_STATUS_ACTIVE;
	} else {
		item->status &= ~ITEM_STATUS_ACTIVE;
	}

	if (status != item->status) {
		popup_redraw_item(item);
	}

	if (mouse_stop_stroke && popup_in_item(item)) {
		if (item == popup->clear_item) {
			list = popup->list_item->list;
			popup->list_item->status &= ~(ITEM_STATUS_DIRTY | ITEM_STATUS_VIRGIN);
			text_locator = popup_savelist_string(popup->list_item, list->picked_element);
			*text_locator = 0;
			popup_redraw_item(popup->list_item);
		} else {

			if (item != popup->cancel_item) {
				if (popup->list_item != NULL) {
					if (popup->list_item->status & ITEM_STATUS_DIRTY) {
						list = popup->list_item->list;
						buffer = popup->list_item->buffer;
						Common::strcpy_s(popup_savelist_string(popup->list_item, list->picked_element), 65536, buffer->data);
						popup->list_item->status &= ~ITEM_STATUS_DIRTY;
					}
				}
			}

			popup->status |= POPUP_STATUS_EXIT;
		}
	}

done:
	;
	return 0;
}


static int popup_button_key(PopupItem *item) {
	int count;

	for (count = 0; count < popup->num_items; count++) {
		if (popup->item[count].type == ITEM_BUTTON) {
			if (toupper(popup->key) == popup->item[count].keystroke) {
				popup_activate_item(&popup->item[count]);
				popup->status |= POPUP_STATUS_EXIT;
				popup->key_handled = true;
			}
		}
	}

	return 0;
}


static void popup_button_create(PopupItem *item) {
	item->vector[VECTOR_X_SIZE] = popup_button_x_size;
	item->vector[VECTOR_Y_SIZE] = popup_button_y_size;
	item->vector[VECTOR_DRAW] = popup_button_draw;
	item->vector[VECTOR_MOUSE] = popup_button_mouse;
	item->vector[VECTOR_KEY] = popup_button_key;
}





/****************/
/* Menu Buttons */
/****************/

static int popup_menu_x_size(PopupItem *item) {
	int size;
	int middles;
	PopupList *list;

	list = item->list;

	size = box_param.menu_left_width + box_param.menu_right_width;

	middles = 0;

	while ((size < list->box_xs) && box_param.menu_middle_width) {
		size += box_param.menu_middle_width;
		middles++;
	}

	item->sprite = middles;

	return (size);
}


static int popup_menu_y_size(PopupItem *item) {
	item = NULL;  // delete if this routine is to be used
	return(box_param.menu->index[0].ys);
}


static int popup_menu_draw(PopupItem *item) {
	int count;
	int selected;
	int x, y;
	int left_sprite;
	int middle_sprite;
	int right_sprite;
	int font_size;
	int center_xs = 0;
	char temp_buf[256];
	char *mark;
	char *text_locator;
	PopupList *list;

	list = item->list;

	selected = (popup->active_item == item) && (item->status & ITEM_STATUS_ACTIVE) != 0;

	left_sprite = 1;
	middle_sprite = 2;
	right_sprite = 3;

	if (selected) {
		left_sprite = 4;
		middle_sprite = 5;
		right_sprite = 6;
	}

	x = item->x;
	y = item->y;

	sprite_draw(box_param.menu, left_sprite, &scr_main, x, y);

	x += box_param.menu_left_width;

	for (count = 0; count < item->sprite; count++) {
		sprite_draw(box_param.menu, middle_sprite, &scr_main, x, y);
		x += box_param.menu_middle_width;
	}

	sprite_draw(box_param.menu, right_sprite, &scr_main, x, y);

	Common::strcpy_s(temp_buf, 65536, item->prompt);
	mark = strchr(temp_buf, '@');
	if (mark != NULL) {
		*mark = 0;
		center_xs = font_string_width(box_param.menu_font, temp_buf, box_param.menu_font_spacing);
		text_locator = list->data + (list->picked_element * list->element_offset);
		Common::strcat_s(temp_buf, text_locator);
	}

	font_size = font_string_width(box_param.menu_font, temp_buf, box_param.menu_font_spacing);

	if (mark != NULL) {
		x = item->x + (((item->xs >> 1) + list->extra_xs) - center_xs);
	} else {
		x = item->x + ((item->xs >> 1) - (font_size >> 1));
	}
	y = item->font_y;

	if (selected) {
		x += box_param.menu_text_x_bonus;
		y += box_param.menu_text_y_bonus;
	}

	font_set_colors(-1, POPUP_DIALOG_MENU_TEXT_1,
		POPUP_DIALOG_MENU_TEXT_2,
		POPUP_DIALOG_MENU_TEXT_3);

	font_write(box_param.menu_font, &scr_main, temp_buf,
		x, y, box_param.menu_font_spacing);

	return 0;
}





static int popup_menu_mouse(PopupItem *item) {
	int force_redraw = false;
	int in_item;
	int count;
	word status;
	PopupList *list;

	list = item->list;

	status = item->status;

	in_item = popup_in_item(item);

	if (mouse_stroke_going && !in_item) {
		for (count = 0; count < popup->num_items; count++) {
			if (popup_in_item(&popup->item[count]) && (popup->item[count].type == ITEM_MENU)) {
				popup->mouse_item = &popup->item[count];
				popup_activate_item(popup->mouse_item);
				popup_exec_function(popup->mouse_item, VECTOR_MOUSE);
				goto done;
			}
		}
	}

	if (mouse_stroke_going && in_item) {
		item->status |= ITEM_STATUS_ACTIVE;
	} else {
		item->status &= ~ITEM_STATUS_ACTIVE;
	}

	if (mouse_stop_stroke && popup_in_item(item)) {
		if (list->elements > 1) {
			list->picked_element = (list->picked_element + 1) % list->elements;
		} else {
			popup->status |= POPUP_STATUS_EXIT;
		}
		force_redraw = true;
	}

	if (force_redraw || (status != item->status)) {
		popup_redraw_item(item);
	}

done:
	;
	return 0;
}


static int popup_menu_key(PopupItem *item) {
	return 0;
}


static void popup_menu_create(PopupItem *item) {
	item->vector[VECTOR_X_SIZE] = popup_menu_x_size;
	item->vector[VECTOR_Y_SIZE] = popup_menu_y_size;
	item->vector[VECTOR_DRAW] = popup_menu_draw;
	item->vector[VECTOR_MOUSE] = popup_menu_mouse;
	item->vector[VECTOR_KEY] = popup_menu_key;
}



/*****************/
/* Text Messages */
/*****************/


static int popup_message_x_size(PopupItem *item) {
	return popup_font_size(item->prompt);
}


static int popup_message_y_size(PopupItem *) {
	return box_param.font->max_y_size;
}


static int popup_message_draw(PopupItem *item) {
	font_set_colors(-1, POPUP_DIALOG_TEXT_COLOR,
		POPUP_DIALOG_TEXT_COLOR,
		POPUP_DIALOG_TEXT_COLOR);
	font_write(box_param.font, &scr_main, item->prompt,
		item->font_x, item->font_y, box_param.font_spacing);
	return 0;
}


static void popup_message_create(PopupItem *item) {
	item->vector[VECTOR_X_SIZE] = popup_message_x_size;
	item->vector[VECTOR_Y_SIZE] = popup_message_y_size;
	item->vector[VECTOR_DRAW] = popup_message_draw;
}


/******************/
/* static Sprites */
/******************/

static int popup_sprite_x_size(PopupItem *item) {
	return (item->series->index[item->sprite - 1].xs);
}


static int popup_sprite_y_size(PopupItem *item) {
	return (item->series->index[item->sprite - 1].ys);
}


static int popup_sprite_draw(PopupItem *item) {
	sprite_draw(item->series, item->sprite, &scr_main,
		item->x, item->y);
	return 0;
}

static void popup_sprite_create(PopupItem *item) {
	item->vector[VECTOR_X_SIZE] = popup_sprite_x_size;
	item->vector[VECTOR_Y_SIZE] = popup_sprite_y_size;
	item->vector[VECTOR_DRAW] = popup_sprite_draw;
}


/**********************/
/* Complex Save Lists */
/**********************/

static int popup_savelist_x_size(PopupItem *item) {
	int size;
	int work_size;
	int number_width;
	int scroll_width;
	PopupList *list;

	list = item->list;

	size = list->box_xs;

	work_size = size;
	work_size -= 2;

	number_width = popup_font_size("94") + 3;
	scroll_width = popup_font_size("<") + 4;

	list->extra_xs = number_width;
	list->scroll.xs = scroll_width;

	work_size -= (number_width + scroll_width + 4);

	list->list_xs = work_size;

	return (size);
}


static int popup_savelist_y_size(PopupItem *item) {
	int size;
	int row_size;
	PopupList *list;

	list = item->list;

	row_size = box_param.font->max_y_size + 2;
	list->list_ys = row_size;

	size = (row_size * list->rows) + 4;
	list->scroll.ys = size - 4;
	list->extra_ys = size - 4;
	list->box_ys = size;

	list->scroll.arrow_size = (box_param.font->max_y_size) + 2;

	return (size);
}


static int popup_savelist_adjust(PopupItem *item) {
	PopupList *list;

	list = item->list;

	list->box_x += item->x;
	list->list_x += item->x;
	list->extra_x += item->x;
	list->scroll.x += item->x;
	list->font_x += item->x;

	list->box_y += item->y;
	list->list_y += item->y;
	list->extra_y += item->y;
	list->scroll.y += item->y;
	list->font_y += item->y;

	list->scroll.arrow_1_base += item->y;
	list->scroll.arrow_2_base += item->y;

	return 0;
}


static char *popup_savelist_string(PopupItem *item, int element) {
	char *text_locator;
	PopupList *list;

	list = item->list;

	text_locator = list->data + (list->element_offset * element);

	return (text_locator);
}



static int popup_savelist_vertical(PopupItem *item, int relative) {
	int y;
	PopupList *list;

	list = item->list;

	y = list->list_y + (list->list_ys * relative);

	return (y);
}


static void popup_savelist_number_draw(PopupItem *item, int relative, int number) {
	char temp_buf[10];
	int x, y;
	PopupList *list;

	list = item->list;

	mads_itoa(number, temp_buf, 10);

	x = list->extra_x + popup_font_size("94") + 1 - popup_font_size(temp_buf);
	y = popup_savelist_vertical(item, relative) + 1;

	font_set_colors(-1, POPUP_DIALOG_TEXT_COLOR,
		POPUP_DIALOG_TEXT_COLOR,
		POPUP_DIALOG_TEXT_COLOR);
	font_write(box_param.font, &scr_main,
		temp_buf, x, y, box_param.font_spacing);
}


static void popup_savelist_element_draw(PopupItem *item, int element) {
	int relative;
	int color;
	int x, y;
	char temp_buf[256];
	char *text_locator;
	PopupList *list;

	list = item->list;

	relative = element - list->base_element;
	if ((relative < 0) || (relative >= list->rows)) goto done;

	popup_savelist_number_draw(item, relative, element + 1);

	x = list->list_x;
	y = popup_savelist_vertical(item, relative);

	if (element == list->picked_element) {
		buffer_rect_fill(scr_main, x, y, list->list_xs, list->list_ys, POPUP_DIALOG_SELECT_COLOR);
		color = POPUP_DIALOG_STRING_COLOR;
	} else {
		color = POPUP_DIALOG_TEXT_COLOR;
	}

	if ((element == list->picked_element) && (item->status & ITEM_STATUS_DIRTY)) {
		Common::strcpy_s(temp_buf, item->buffer->data);
	} else {
		text_locator = popup_savelist_string(item, element);
		Common::strcpy_s(temp_buf, text_locator);
		if (!strlen(temp_buf) && (item->prompt != NULL) && (element != list->picked_element)) {
			Common::strcpy_s(temp_buf, item->prompt);
		}
	}

	if ((element == list->picked_element) &&
		(item->status & ITEM_STATUS_INPUT) &&
		!(popup->status & POPUP_STATUS_EXIT)) {
		Common::strcat_s(temp_buf, "_");
	}

	while (strlen(temp_buf) && (popup_font_size(temp_buf) > (list->list_xs - 2))) {
		temp_buf[strlen(temp_buf) - 1] = 0;
	}

	font_set_colors(-1, color, color, color);
	font_write(box_param.font, &scr_main,
		temp_buf, x + 1, y + 1, box_param.font_spacing);

done:
	;
}


static void popup_savelist_full_draw(PopupItem *item) {
	int count;
	int element;

	for (count = 0; count < item->list->rows; count++) {
		element = count + item->list->base_element;
		if (element < item->list->elements) {
			popup_savelist_element_draw(item, element);
		}
	}
}


static void popup_savelist_scroll_draw(PopupItem *item) {
	int count;
	int x, y;
	int mask;
	byte color;
	byte color2;
	char arrow[2];
	PopupList *list;

	list = item->list;

	color = POPUP_DIALOG_BORDER_COLOR_1;
	color2 = POPUP_DIALOG_BORDER_COLOR_2;

	// Scroll Bar
	buffer_rect_fill(scr_main, list->scroll.x, list->scroll.y,
		1, list->scroll.ys, color);
	buffer_rect_fill(scr_main, list->scroll.x + 1, list->scroll.y,
		1, list->scroll.ys, color2);

	buffer_rect_fill(scr_main, list->scroll.x, list->scroll.arrow_1_base + list->scroll.arrow_size,
		list->scroll.xs, 1, color);
	buffer_rect_fill(scr_main, list->scroll.x, list->scroll.arrow_1_base + list->scroll.arrow_size + 1,
		list->scroll.xs, 1, color2);

	buffer_rect_fill(scr_main, list->scroll.x, list->scroll.arrow_2_base - 2,
		list->scroll.xs, 1, color);
	buffer_rect_fill(scr_main, list->scroll.x, list->scroll.arrow_2_base - 1,
		list->scroll.xs, 1, color2);

	arrow[1] = 0;

	for (count = 0; count < 2; count++) {
		switch (count) {
		case 0:
			y = list->scroll.arrow_1_base;
			arrow[0] = '<';
			mask = SCROLL_STATUS_UP;
			break;

		case 1:
		default:
			y = list->scroll.arrow_2_base;
			arrow[0] = '>';
			mask = SCROLL_STATUS_DOWN;
			break;
		}

		x = list->scroll.x + 2;

		if (list->scroll.status & mask) {
			buffer_rect_fill(scr_main, x, y,
				list->scroll.xs - 2, list->scroll.arrow_size,
				POPUP_DIALOG_SELECT_COLOR);
		}

		x += 1;
		y += 1;

		color = POPUP_DIALOG_TEXT_COLOR;
		font_set_colors(-1, color, color, color);
		font_write(box_param.font, &scr_main, arrow, x, y, box_param.font_spacing);
	}
}


static void popup_savelist_scroll_clear(PopupItem *item) {
	PopupList *list;

	list = item->list;

	buffer_rect_fill_pattern(scr_main,
		list->scroll.x, list->scroll.y,
		list->scroll.xs, list->scroll.ys,
		box->window_x, box->window_y, box->window_xs,
		POPUP_FILL_COLOR, POPUP_FILL_COLOR_2, 0, 0);
}


static void popup_savelist_item_clear(PopupItem *item, int relative) {
	int x, y;
	PopupList *list;

	list = item->list;

	x = list->list_x;
	y = popup_savelist_vertical(item, relative);

	buffer_rect_fill_pattern(scr_main, x, y,
		list->list_xs, list->list_ys,
		box->window_x, box->window_y, box->window_xs,
		POPUP_FILL_COLOR, POPUP_FILL_COLOR_2, 0, 0);

	x = list->extra_x;

	buffer_rect_fill_pattern(scr_main, x, y,
		list->extra_xs - 2, list->list_ys,
		box->window_x, box->window_y, box->window_xs,
		POPUP_FILL_COLOR, POPUP_FILL_COLOR_2, 0, 0);

}




static void popup_savelist_scroll_to_screen(PopupItem *item) {
	int refresh;
	PopupList *list;

	list = item->list;

	refresh = popup_mouse_refresh();
	video_update(&scr_main, list->scroll.x, list->scroll.y,
		list->scroll.x, list->scroll.y,
		list->scroll.xs, list->scroll.ys);
	popup_mouse_refresh_2(refresh);
}



static void popup_savelist_item_to_screen(PopupItem *item, int relative) {
	int x, y;
	int refresh;
	PopupList *list;

	list = item->list;

	x = list->list_x;
	y = popup_savelist_vertical(item, relative);

	refresh = popup_mouse_refresh();
	video_update(&scr_main, x, y, x, y, list->list_xs, list->list_ys);
	popup_mouse_refresh_2(refresh);
}


static void popup_savelist_scroll_refresh(PopupItem *item) {
	popup_savelist_scroll_clear(item);
	popup_savelist_scroll_draw(item);
	popup_savelist_scroll_to_screen(item);
}


static void popup_savelist_item_refresh(PopupItem *item, int element) {
	int relative;

	relative = element - item->list->base_element;

	popup_savelist_item_clear(item, relative);
	if (element <= item->list->elements) {
		popup_savelist_element_draw(item, element);
	}
	popup_savelist_item_to_screen(item, relative);
}


static int popup_savelist_draw(PopupItem *item) {
	byte color;
	byte color2;
	PopupList *list;

	list = item->list;

	color = POPUP_DIALOG_BORDER_COLOR_1;
	color2 = POPUP_DIALOG_BORDER_COLOR_2;

	// Basic box
	popup_double_box_2(item->x, item->y, item->xs, item->ys, color, color2);
	// buffer_rect_fill (scr_main, item->x, item->y,
	// item->xs, 1,
	// color);
	// buffer_rect_fill (scr_main, item->x, item->y,
	// 1, item->ys,
	// color);
	// buffer_rect_fill (scr_main, item->x + item->xs - 1, item->y,
	// 1, item->ys,
	// color);
	// buffer_rect_fill (scr_main, item->x, item->y + item->ys - 1,
	// item->xs, 1,
	// color);
	// Number list
	buffer_rect_fill(scr_main, list->extra_x + list->extra_xs - 2, list->extra_y,
		1, list->extra_ys, color);
	buffer_rect_fill(scr_main, list->extra_x + list->extra_xs - 1, list->extra_y,
		1, list->extra_ys, color2);


	popup_savelist_full_draw(item);
	popup_savelist_scroll_draw(item);

	return 0;
}


static void popup_adjust_elements(PopupList *list) {
	list->base_element = MAX(0, list->base_element);
	list->base_element = MIN(list->base_element, list->elements - 1);
	list->picked_element = MAX(list->base_element, list->picked_element);
	list->picked_element = MIN(list->picked_element, list->base_element + list->rows - 1);
	list->picked_element = MIN(list->picked_element, list->elements - 1);
}




static int popup_savelist_mouse(PopupItem *item) {
	bool in_scroll_bar;
	bool in_up_arrow;
	bool in_down_arrow;
	bool in_main_box;
	bool in_main_range;
	int update_sign;
	int force_update;
	int old_status;
	int picked_element;
	int base_element;
	int y, relative, actual;
	long now_clock;

	PopupList *list;

	list = item->list;

	picked_element = list->picked_element;
	base_element = list->base_element;

	in_scroll_bar = ((mouse_x >= list->scroll.x) &&
		(mouse_x <= (list->scroll.x + list->scroll.xs - 1)) &&
		(mouse_y >= list->scroll.y) &&
		(mouse_y <= (list->scroll.y + list->scroll.ys - 1)));

	in_up_arrow = in_scroll_bar && (mouse_y <= (list->scroll.arrow_1_base + list->scroll.arrow_size));
	in_down_arrow = in_scroll_bar && (mouse_y >= (list->scroll.arrow_2_base - 1));

	in_main_box = popup_in_item(item) && ((mouse_y > item->y) &&
		(mouse_y < (item->y + item->ys - 1)));

	force_update = mouse_start_stroke || (item->status & ITEM_STATUS_FORCED) != 0;

	if (mouse_start_stroke) {
		if (item->status & ITEM_STATUS_DIRTY) {
			item->status &= ~ITEM_STATUS_DIRTY;
		}

		item->status &= ~ITEM_STATUS_VIRGIN;

		popup->mouse_status = 0;
		if (in_scroll_bar) {
			popup->mouse_status |= POPUP_MOUSE_SCROLL;
		}
		if (in_up_arrow) {
			popup->mouse_status |= POPUP_MOUSE_UP;
		}
		if (in_down_arrow) {
			popup->mouse_status |= POPUP_MOUSE_DOWN;
		}
	}

	old_status = list->scroll.status;

	if (mouse_stroke_going) {
		list->scroll.status = 0;
		update_sign = 0;
		if (popup->mouse_status & POPUP_MOUSE_SCROLL) {
			if (popup->mouse_status & POPUP_MOUSE_UP) {
				if (in_up_arrow) {
					list->scroll.status = SCROLL_STATUS_UP;
					update_sign = -1;
				}
			}

			if (popup->mouse_status & POPUP_MOUSE_DOWN) {
				if (in_down_arrow) {
					list->scroll.status = SCROLL_STATUS_DOWN;
					update_sign = 1;
				}
			}

			if (mouse_button) update_sign = update_sign << 2;

			if (update_sign && (force_update || (old_status != list->scroll.status))) {
				list->base_element += update_sign;
				popup_adjust_elements(list);
				if ((update_sign < 0) && (list->base_element == base_element)) {
					list->picked_element--;
					popup_adjust_elements(list);
				}
			}

		} else {

			if (in_main_range) {
				y = mouse_y - (item->y + 2);
				relative = y / list->list_ys;
				if (relative < 0) relative = 0;
				if (relative >= list->rows) relative = list->rows - 1;

				actual = list->base_element + relative;

				if (actual <= list->elements) {
					list->picked_element = actual;
				}

				item->status |= ITEM_STATUS_IN_RANGE;

			} else {

				if (mouse_y <= item->y) {
					update_sign = -1;
				} else {
					update_sign = 1;
				}

				if (force_update || (item->status & ITEM_STATUS_IN_RANGE)) {
					list->base_element += update_sign;
					if (update_sign < 0) {
						list->picked_element = list->base_element;
					} else {
						list->picked_element = list->base_element + list->rows - 1;
					}
					popup_adjust_elements(list);
				}

				item->status &= ~ITEM_STATUS_IN_RANGE;
			}
		}
	}

	if (mouse_stop_stroke && list->scroll.status) {
		list->scroll.status = 0;
	}

	if (mouse_stop_stroke && !popup->mouse_status) {
		now_clock = timer_read();
		if ((now_clock - list->double_clock) < POPUP_DOUBLE_CLICK_THRESHOLD) {
			if (list->double_element == list->picked_element) {
				popup_activate_item(popup->enter_item);
				popup->status |= POPUP_STATUS_EXIT;
			}
		}
		list->double_clock = now_clock;
		list->double_element = list->picked_element;
	}

	if (list->base_element != base_element) {
		popup_redraw_item(item);
	} else if (list->picked_element != picked_element) {
		popup_savelist_item_refresh(item, picked_element);
		popup_savelist_item_refresh(item, list->picked_element);
	} else if (list->scroll.status != old_status) {
		popup_savelist_scroll_refresh(item);
	}

	return 0;
}


static void popup_savelist_select(PopupItem *item, int element) {
	PopupList *list;

	list = item->list;

	list->picked_element = element;
	if ((list->base_element > list->picked_element) ||
		(list->picked_element >= (list->base_element + list->rows))) {
		list->base_element = list->picked_element - (list->rows >> 1);
	}

	popup_adjust_elements(list);
}



static void popup_savelist_select_free(PopupItem *item) {
	int count;
	int found = -1;
	char *text_locator;
	PopupList *list;

	list = item->list;

	for (count = 0; (found < 0) && (count < list->elements); count++) {
		text_locator = popup_savelist_string(item, count);
		if (*text_locator == 0) found = count;
	}

	if (found >= 0) {
		popup_savelist_select(item, found);
	}
}



static int popup_savelist_key(PopupItem *item) {
	int base_element;
	int picked_element;
	int new_string = false;
	char newStr[2];
	PopupList *list;
	PopupBuffer *buffer;

	if (!(item->status & ITEM_STATUS_INPUT)) goto done;

	list = item->list;
	buffer = item->buffer;

	base_element = list->base_element;
	picked_element = list->picked_element;

	switch (popup->key) {
	case enter_key:
		if (item->status & ITEM_STATUS_DIRTY) {
			Common::strcpy_s(popup_savelist_string(item, list->picked_element), 65536, buffer->data);
			item->status &= ~ITEM_STATUS_DIRTY;
		}
		new_string = true;
		goto skip_handled;
		break;

	case bksp_key:
		if (!(item->status & ITEM_STATUS_DIRTY)) {
			Common::strcpy_s(buffer->data, 65536, popup_savelist_string(item, list->picked_element));
			item->status |= ITEM_STATUS_DIRTY;
		}

		if (strlen(buffer->data)) {
			buffer->data[strlen(buffer->data) - 1] = 0;
			new_string = true;
		}

		goto handled;
		break;

	case ctrl_c_key:
		item->status |= ITEM_STATUS_DIRTY;
		*buffer->data = 0;
		new_string = true;
		goto handled;
		break;

	case ctrl_r_key:
		item->status &= ~ITEM_STATUS_DIRTY;
		new_string = true;
		goto handled;
		break;

	default:
		if (Common::isPrint(popup->key)) {
			if (item->status & ITEM_STATUS_VIRGIN) {
				if (strlen(popup_savelist_string(item, list->picked_element))) {
					popup_savelist_select_free(item);
				}
			}

			if (!(item->status & ITEM_STATUS_DIRTY)) {
				Common::strcpy_s(buffer->data, 65536, popup_savelist_string(item, list->picked_element));
				item->status |= ITEM_STATUS_DIRTY;
			}

			if (!strlen(buffer->data)) popup->key = toupper(popup->key);
			newStr[0] = (byte)popup->key;
			newStr[1] = 0;

			if ((popup_font_size(buffer->data) + popup_font_size(newStr)) <= (list->list_xs - 2)) {
				if ((int)strlen(buffer->data) < (buffer->max_length - 1)) {
					Common::strcat_s(buffer->data, 65536, newStr);
					new_string = true;
				}
			}

			goto handled;

		}
		break;
	}

	goto done;

handled:
	popup->key_handled = true;

skip_handled:
	item->status &= ~ITEM_STATUS_VIRGIN;

	if (mouse_latched && mouse_stroke_going) {
		mouse_init_cycle();
		popup_redraw_item(item);
	} else if (list->base_element != base_element) {
		popup_redraw_item(item);
	} else if (list->picked_element != picked_element) {
		popup_savelist_item_refresh(item, picked_element);
		popup_savelist_item_refresh(item, list->picked_element);
	} else if (new_string) {
		popup_savelist_item_refresh(item, list->picked_element);
	}

done:
	;
	return 0;
}


static int popup_savelist_update(PopupItem *item) {
	return 0;
}




static void popup_savelist_create(PopupItem *item) {
	item->vector[VECTOR_X_SIZE] = popup_savelist_x_size;
	item->vector[VECTOR_Y_SIZE] = popup_savelist_y_size;
	item->vector[VECTOR_DRAW] = popup_savelist_draw;
	item->vector[VECTOR_MOUSE] = popup_savelist_mouse;
	item->vector[VECTOR_KEY] = popup_savelist_key;
	item->vector[VECTOR_ADJUST] = popup_savelist_adjust;
	item->vector[VECTOR_UPDATE] = popup_savelist_update;
}




static void item_constructor(PopupItem *item, int type) {
	item->type = (byte)type;

	switch (type) {
	case ITEM_BUTTON:
		popup_button_create(item);
		break;
	case ITEM_MESSAGE:
		popup_message_create(item);
		break;
	case ITEM_SAVELIST:
		popup_savelist_create(item);
		break;
	case ITEM_SPRITE:
		popup_sprite_create(item);
		break;
	case ITEM_MENU:
		popup_menu_create(item);
		break;
	}
}



PopupItem *popup_button(const char *prompt, int x) {
	int first_button;
	int16 new_width;
	PopupItem *item;

	first_button = (popup->status & POPUP_STATUS_BUTTON) == 0;

	popup->status |= POPUP_STATUS_BUTTON;

	item = item_allocate(true);
	item_constructor(item, ITEM_BUTTON);

	set_prompt(item, prompt);

	item->y = POPUP_BUTTON_ROW;
	item->ys = popup_exec_function(item, VECTOR_Y_SIZE);

	item->font_x = 4;
	item->font_y = 3;

	item->x = x;
	item->xs = popup_exec_function(item, VECTOR_X_SIZE);

	if (item->x == POPUP_BUTTON_LEFT) {
		item->x = popup->button_left_fill;
		popup->button_left_fill += item->xs + popup->button_spacing;
	} else if (item->x == POPUP_BUTTON_RIGHT) {
		popup->button_right_fill += item->xs;
		item->x = POPUP_BUTTON_RIGHT | popup->button_right_fill;
		popup->button_right_fill += popup->button_spacing;
	}

	new_width = popup->button_left_fill + popup->button_right_fill;
	popup->width = MAX(popup->width, new_width);

	if (first_button) popup->enter_item = item;

	return (item);
}


PopupItem *popup_cancel_button(const char *prompt) {
	PopupItem *item;

	if (prompt == NULL) {
		item = popup_button("Cancel", POPUP_BUTTON_RIGHT);
	} else {
		item = popup_button(prompt, POPUP_BUTTON_RIGHT);
	}

	popup->cancel_item = item;

	return item;
}


void popup_menu_option(PopupItem *item, char *option) {
	PopupList *list;
	char *text_locator;

	list = item->list;

	if (list->base_element < list->elements) {
		text_locator = list->data + (list->base_element * list->element_offset);
		Common::strcpy_s(text_locator, 65536, option);
		list->base_element++;
	}
}


PopupItem *popup_menu(const char *prompt,
	int x, int y, int pixel_width,
	int off_center_x,
	int elements, int element_max_length,
	int default_element) {
	int heap_to_get;
	PopupItem *item;
	PopupList *list;

	item = item_allocate(true);
	item_constructor(item, ITEM_MENU);

	item->status |= ITEM_STATUS_INERT;

	set_prompt(item, prompt);

	item->list = list = list_allocate();

	heap_to_get = elements * (element_max_length + 1);

	if (heap_to_get > 0) {
		list->data = (char *)popup_heap(heap_to_get);
	}

	list->elements = elements;
	list->element_offset = element_max_length + 1;
	list->element_max_length = element_max_length;

	list->base_element = 0;
	list->picked_element = default_element;
	list->picked_element = MAX(list->picked_element, 0);
	list->picked_element = MIN(list->picked_element, elements - 1);

	list->box_xs = pixel_width;

	list->extra_xs = off_center_x;

	item->xs = popup_exec_function(item, VECTOR_X_SIZE);
	item->ys = popup_exec_function(item, VECTOR_Y_SIZE);

	item->font_x = box_param.menu_text_x_offset;
	item->font_y = box_param.menu_text_y_offset;

	popup_y_placement(item, y);

	item->x = x;

	popup_x_width_check(item);

	return (item);
}



PopupItem *popup_message(const char *prompt, int x, int y) {
	PopupItem *item;

	item = item_allocate(false);
	item_constructor(item, ITEM_MESSAGE);

	item->status |= ITEM_STATUS_INERT;

	set_prompt(item, prompt);

	item->ys = popup_exec_function(item, VECTOR_Y_SIZE);
	item->xs = popup_exec_function(item, VECTOR_X_SIZE);

	popup_y_placement(item, y);

	item->x = x;

	popup_x_width_check(item);

	return item;
}



void popup_blank(int num_lines) {
	popup->y_position += num_lines;
}


void popup_blank_line(void) {
	popup_blank(box_param.font->max_y_size + popup->y_spacing);
}



PopupItem *popup_sprite(SeriesPtr series, int sprite, int x, int y) {
	PopupItem *item;

	item = item_allocate(false);
	item_constructor(item, ITEM_SPRITE);

	item->status |= ITEM_STATUS_INERT;

	item->series = series;
	item->sprite = sprite;

	item->ys = popup_exec_function(item, VECTOR_Y_SIZE);
	item->xs = popup_exec_function(item, VECTOR_X_SIZE);

	popup_y_placement(item, y);

	item->x = x;

	popup_x_width_check(item);

	return (item);
}





PopupItem *popup_savelist(const char *data,
	const char *empty_string,
	int elements,
	int element_offset,
	int element_max_length,
	int pixel_width,
	int rows,
	int accept_input,
	int default_element) {
	PopupItem *item;
	PopupList *list;
	PopupBuffer *buffer;

	item = item_allocate(true);
	item_constructor(item, ITEM_SAVELIST);

	item->status |= ITEM_STATUS_CLOCK;
	item->mouse_interval = 4;

	if (accept_input) {
		item->status |= ITEM_STATUS_STRING | ITEM_STATUS_INPUT | ITEM_STATUS_VIRGIN;
	}

	item->buffer = buffer = buffer_allocate(element_max_length + 1);

	item->list = list = list_allocate();

	item->prompt = (char *)empty_string;
	list->data = (char *)data;

	list->elements = elements;
	list->element_offset = element_offset;
	list->element_max_length = element_max_length;
	list->rows = rows;
	list->columns = 1;

	list->picked_element = default_element;
	list->picked_element = MAX(list->picked_element, 0);
	list->picked_element = MIN(list->picked_element, elements - 1);
	list->base_element = list->picked_element - ((rows >> 1) - 1);
	list->base_element = MAX(list->base_element, 0);

	list->box_xs = pixel_width;

	item->ys = popup_exec_function(item, VECTOR_Y_SIZE);
	item->xs = popup_exec_function(item, VECTOR_X_SIZE);

	popup_y_placement(item, POPUP_FILL);

	item->x = 0;

	list->extra_x = 2;
	list->list_x = list->extra_x + list->extra_xs + 1;
	list->font_x = list->list_x + 1;
	list->scroll.x = list->list_x + list->list_xs + 1;

	list->scroll.y = 2;
	list->extra_y = 2;
	list->list_y = 2;
	list->font_y = 3;

	list->scroll.arrow_1_base = list->scroll.y;
	list->scroll.arrow_2_base = item->ys - (list->scroll.arrow_size + 2);

	list->scroll.status = 0;

	list->double_clock = 0;
	list->double_element = 0;

	popup_x_width_check(item);

	popup->list_item = item;

	if (popup->string_item == NULL) {
		popup->string_item = item;
	}

	return (item);
}





PopupItem *popup_execute(void) {
	int base_x_size;
	int horiz_pieces;
	int request_pieces;
	int actual_x_size;
	int text_x_extra;
	int count;
	long now_clock;
	Box popup_box;
	Box *keep_box;
	PopupItem *result_item = NULL;

	keep_box = box;
	box = &popup_box;

	base_x_size = popup->width + (popup_padding_width << 1);

	horiz_pieces = ((base_x_size - (box_param.extra_x + 1)) / pop_xs(POPUP_TOP)) + 1;

	request_pieces = horiz_pieces - box_param.pieces_per_center;
	request_pieces = MAX(0, request_pieces);
	if (request_pieces & 1) request_pieces++;
	request_pieces = request_pieces >> 1;

	horiz_pieces = (request_pieces << 1) + box_param.pieces_per_center;

	actual_x_size = (horiz_pieces * pop_xs(POPUP_TOP)) + box_param.extra_x;
	text_x_extra = actual_x_size - base_x_size;

	if (popup->status & POPUP_STATUS_BUTTON) {
		popup->y_position += popup->y_spacing;
		if (popup->status & POPUP_STATUS_BAR) {
			popup->y_position += 1 + popup_padding_width;
		}
		popup->button_y = popup->y_position;
		popup->y_position += popup_exec_function(popup->enter_item, VECTOR_Y_SIZE);
	}

	box->request_y_size = popup->y_position + (popup_padding_width << 1);

	if (popup_create(0 - request_pieces, popup->x, popup->y)) {
		error_report(ERROR_KERNEL_NO_POPUP, ERROR, MODULE_POPUP_DIALOG, request_pieces, 1);
		goto done;
	}

	if (popup_draw(true, false)) {
		error_report(ERROR_KERNEL_NO_POPUP, ERROR, MODULE_POPUP_DIALOG, box->request_y_size, 2);
		goto done;
	}

	popup->x = box->window_x + popup_padding_width + (text_x_extra >> 1);
	popup->y = box->window_y + popup_padding_width + (box->text_extra >> 1);
	popup->xs = popup->width;
	popup->ys = popup->y_position;

	if (popup->status & POPUP_STATUS_BUTTON) {
		popup->button_y += (box->text_extra - (box->text_extra >> 1));

		if (popup->status & POPUP_STATUS_STEAL) {
			popup->button_bar_color = *buffer_pointer(&scr_main,
				box->window_x - 1,
				box->window_y);
		}
	}

	for (count = 0; count < popup->num_items; count++) {
		popup_coord_adjust(&popup->item[count]);
		popup_exec_function(&popup->item[count], VECTOR_ADJUST);
	}

	popup->button_y += popup->y;

	popup->mouse_item = NULL;

	popup_full_draw();

	mouse_init_cycle();

	do {
		mouse_begin_cycle(false);

		if (keys_any()) {
			popup->key = keys_get();
			popup->key_handled = false;

			if (Common::isPrint(popup->key) || (popup->key == bksp_key)) {
				if (!(popup->active_item->status & ITEM_STATUS_STRING)) {
					if (popup->string_item != NULL) {
						popup_activate_item(popup->string_item);
					}
				}
			}

			popup_exec_function(popup->active_item, VECTOR_KEY);

			if (!popup->key_handled) {
				switch (popup->key) {

				case alt_x_key:
				case alt_q_key:
				case ctrl_q_key:
				case ctrl_x_key:
				case f1_key:
				case f2_key:
				case f3_key:
				case f4_key:
				case f5_key:
				case f6_key:
				case f7_key:
				case f8_key:
				case f9_key:
				case f10_key:

				case esc_key:
					popup_activate_item(popup->cancel_item);
					popup->status |= POPUP_STATUS_EXIT;
					break;

				case enter_key:
					popup_activate_item(popup->enter_item);
					popup->status |= POPUP_STATUS_EXIT;
					break;

				case tab_key:
				case down_key:
					popup_activate_item(popup_next_item(popup->active_item, true, false));
					break;

				case backtab_key:
				case up_key:
					popup_activate_item(popup_last_item(popup->active_item, true, false));
					break;

				}
			}
		}

		if (!(popup->status & POPUP_STATUS_EXIT)) {
			if (mouse_start_stroke) popup_find_item();
			if (popup->mouse_item != NULL) {
				popup->mouse_item->status &= ~ITEM_STATUS_FORCED;

				if (popup->mouse_item->status & ITEM_STATUS_CLOCK) {
					if (mouse_start_stroke) {
						popup->mouse_clock = timer_read();
					}
					now_clock = timer_read();
					if (now_clock >= popup->mouse_clock) {
						popup->mouse_item->status |= ITEM_STATUS_FORCED;
						popup->mouse_clock = now_clock + popup->mouse_item->mouse_interval;
					}
				}
				if (mouse_changed || (popup->mouse_item->status & ITEM_STATUS_FORCED)) {
					popup_exec_function(popup->mouse_item, VECTOR_MOUSE);
				}
			}
		}

		mouse_end_cycle(false, (popup->status & POPUP_STATUS_EXIT) == 0);

		if (!game.going) {
			result_item = nullptr;
			goto done;
		}

	} while (!(popup->status & POPUP_STATUS_EXIT));

	result_item = popup->active_item;

done:
	popup_destroy();
	box = keep_box;
	return result_item;
}

} // namespace MADSV2
} // namespace MADS
