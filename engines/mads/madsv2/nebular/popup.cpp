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

#include "graphics/palette.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/nebular/popup.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

static constexpr int BLACK = 0;
static constexpr int PALETTE_CYCLING_AREA = 6;
static constexpr int DIALOG_CONTENT1_COLOR = 248;
static constexpr int DIALOG_CONTENT2_COLOR = 249;
static constexpr int DIALOG_EDGE_COLOR = 250;
static constexpr int DIALOG_BACKGROUND_COLOR = 251;
static constexpr int DIALOG_FC_COLOR = 252;
static constexpr int DIALOG_FD_COLOR = 253;
static constexpr int DIALOG_FE_COLOR = 254;
static constexpr int DIALOG_BLACK_COLOR = 0;

int dialog_content_handle;
bool popup_vomitation_flag;


void popup_init() {
	dialog_content_handle = -1;
	popup_vomitation_flag = true;
}

int popup_create(FontPtr font, int horiz_pieces, int x, int y) {
	int count;

	box->x = x;
	box->y = y;
	box_param.font = font;
	box->xs = (box_param.font->max_x_size + 1) * horiz_pieces + 10;
	box->text_width = horiz_pieces * 2;

	box->cursor_x = 0;
	box->text_x = box->text_y = 0;
	font_set_colors(-1, BLACK, BLACK, BLACK);

	// Allocate text lines
	box->text[0] = (char *)malloc((box->text_width + 1) * POPUP_MAX_LINES);
	assert(box->text[0]);

	// Set pointers within text for remaining text array indexes
	for (count = 1; count < POPUP_MAX_LINES; ++count)
		box->text[count] = box->text[0] + (count * (box->text_width + 1));

	for (count = 0; count < POPUP_MAX_LINES; ++count) {
		*box->text[count] = '\0';
		box->tab[count] = 0;
	}


	memcpy(&cycling_palette[Graphics::PALETTE_COUNT - PALETTE_CYCLING_AREA].r,
		&master_palette[Graphics::PALETTE_COUNT - PALETTE_CYCLING_AREA].r,
		PALETTE_CYCLING_AREA * 3);
	pal_grey(master_palette, DIALOG_CONTENT1_COLOR, 2, 36, 32);
	pal_grey(master_palette, DIALOG_EDGE_COLOR, 2, 39, 28);
	pal_grey(master_palette, DIALOG_FC_COLOR, 2, 36, 32);
	pal_grey(master_palette, DIALOG_FE_COLOR, 1, 55, 55);
	mcga_setpal_range(&master_palette, DIALOG_CONTENT1_COLOR, 8);
	box->active = true;

	return 0;
}

void popup_destroy() {
	if (box->depth_saved) {
		buffer_restore(&scr_main, box->preserve_handle, 0, box->x, box->y, box->xs, box->ys);
		video_update(&scr_main, box->x, box->y - viewing_at_y, box->x, box->y, box->xs, box->ys);
	}
	box->depth_saved = false;

	if (box->active)
		mem_free(box->text);

	memcpy(&master_palette[Graphics::PALETTE_COUNT - PALETTE_CYCLING_AREA].r,
		&cycling_palette[Graphics::PALETTE_COUNT - PALETTE_CYCLING_AREA].r,
		PALETTE_CYCLING_AREA * 3);
	mcga_setpal_range(&master_palette, DIALOG_CONTENT1_COLOR, 8);
	box->active = false;
}

static uint16 rotr16(uint16 value, int amount) {
	return (value >> amount) | (value << (16 - amount));
}

static int popup_draw_content(int x, int y, int xs, int ys, int unknown, byte color1, byte color2, int height, Buffer &buffer) {
	int result = 0;
	int stride = buffer.x - xs;

	if (!buffer.data)
		return result;

	byte *ptr = (byte *)mem_normalize(buffer_pointer(&buffer, x, y));

	if (height == 0)
		return result;

	uint16 seed = unknown ? unknown : 0xB78E;

	for (int row = 0; row < height; ++row) {
		if (row == ys)
			result = seed;

		for (int col = 0; col < xs; ++col) {
			uint16 t = seed;
			seed += 0x181D;
			t = rotr16(t, 9);
			seed ^= t;
			t = rotr16(t, 3);
			seed += t;

			*ptr++ = (seed & 0x10) ? color1 : color2;
		}

		ptr += stride;
	}

	return result;
}

void popup_draw(bool save_screen) {
	int askY;

	if (box->cursor_x == 0)
		box->text_y--;

	box->ys = (box_param.font->max_y_size + 1) * (box->text_y + 1) + 10;

	if (box->x == POPUP_CENTER)
		box->x = 160 - box->xs / 2;
	if (box->y == POPUP_CENTER)
		box->y = 100 - box->ys / 2;

	int y2 = box->y + box->ys;
	int x2 = box->x + box->xs;

	if (x2 > 320)
		x2 = -(x2 - 320);
	if (y2 > 200)
		y2 = -(y2 - 200);

	if (save_screen) {
		box->preserve_handle = buffer_preserve(&scr_main, 0xfff8, 0, box->x, box->y, box->xs, box->ys);
		box->depth_saved = true;
	}

	askY = (box_param.font->max_y_size + 1) * box->ask_y;

	// Fill area
	buffer_rect_fill(scr_main, box->x, box->y, box->xs, box->ys, DIALOG_BACKGROUND_COLOR);

	// Edge lines
	buffer_rect_fill(scr_main, box->x + 1, box->y + box->ys - 2, box->xs - 1, 1, DIALOG_EDGE_COLOR);
	buffer_rect_fill(scr_main, box->x, box->y + box->ys - 1, box->xs, 1, DIALOG_EDGE_COLOR);

	// Right edge
	buffer_rect_fill(scr_main, box->x + box->xs - 2, box->y + 2, 1, box->ys - 2, DIALOG_EDGE_COLOR);
	buffer_rect_fill(scr_main, box->x + box->xs - 1, box->y + 1, 1, box->ys - 1, DIALOG_EDGE_COLOR);

	dialog_content_handle = popup_draw_content(box->x + 2, box->y + 2, box->xs - 4, askY, 0,
		DIALOG_CONTENT2_COLOR, DIALOG_CONTENT1_COLOR, box->ys - 4, scr_main);

	askY = box->y + 5;

	if (box->text_y + 1 > 0) {
		for (int lineCtr = 0; lineCtr <= box->text_y; ++lineCtr) {
			if (box->tab[lineCtr] == POPUP_BAR) {
				buffer_rect_fill(scr_main, box->x + 2, askY + (box_param.font->max_y_size + 1) / 2,
					box->xs - 4, 1, DIALOG_BLACK_COLOR);
			} else {
				int tab = box->tab[lineCtr];
				int xp = (tab & 0x7f) + box->x + 5;
				int yp = askY;
				if (tab & 0x40)
					yp++;

				font_write(box_param.font, &scr_main, box->text[lineCtr], xp, yp, 1);

				if (tab & 0x80) {
					buffer_rect_fill(scr_main, xp, yp + box_param.font->max_y_size,
						font_string_width(box_param.font, box->text[lineCtr], 1), 1, DIALOG_BLACK_COLOR);
				}
			}

			askY += box_param.font->max_y_size + 1;
		}
	}

	mouse_hide();
	video_update(&scr_main, box->x, box->y, box->x, box->y, box->xs, box->ys);
	mouse_show();
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
