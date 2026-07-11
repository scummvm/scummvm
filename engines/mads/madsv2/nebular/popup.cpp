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

static constexpr int DIALOG_BLACK_COLOR = 0;

int dialog_content_seed;

void popup_init() {
	dialog_content_seed = -1;
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

void popup_draw() {
	int askY;

	int y2 = box->y + box->ys;
	int x2 = box->x + box->xs;

	if (x2 > 320)
		x2 = -(x2 - 320);
	if (y2 > 200)
		y2 = -(y2 - 200);

	askY = (box_param.font->max_y_size + 1) * box->ask_y;

	// Fill area
	buffer_rect_fill(scr_main, box->x, box->y, box->xs, box->ys, REX_DIALOG_BACKGROUND_COLOR);

	// Edge lines
	buffer_rect_fill(scr_main, box->x + 1, box->y + box->ys - 2, box->xs - 1, 1, REX_DIALOG_EDGE_COLOR);
	buffer_rect_fill(scr_main, box->x, box->y + box->ys - 1, box->xs, 1, REX_DIALOG_EDGE_COLOR);

	// Right edge
	buffer_rect_fill(scr_main, box->x + box->xs - 2, box->y + 2, 1, box->ys - 2, REX_DIALOG_EDGE_COLOR);
	buffer_rect_fill(scr_main, box->x + box->xs - 1, box->y + 1, 1, box->ys - 1, REX_DIALOG_EDGE_COLOR);

	dialog_content_seed = popup_draw_content(box->x + 2, box->y + 2, box->xs - 4, askY, 0,
		REX_DIALOG_CONTENT2_COLOR, REX_DIALOG_CONTENT1_COLOR, box->ys - 4, scr_main);

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
