/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/dialog.h"
#include "chamber/print.h"
#include "chamber/cga.h"
#include "chamber/script.h"
#include "chamber/cursor.h"
#include "chamber/input.h"

namespace Chamber {

uint16 cur_str_index;
uint16 cur_dlg_index;

dirty_rect_t dirty_rects[MAX_DIRTY_RECT];
dirty_rect_t *last_dirty_rect = dirty_rects;

void addDirtyRect(byte kind, byte x, byte y, byte w, byte h, uint16 offs) {
	int16 i;
	dirty_rect_t *r = dirty_rects;
	for (i = 0; i < MAX_DIRTY_RECT; i++, r++) /*TODO: may go oob*/
		if (r->kind == DirtyRectFree)
			break;
	r->kind = kind;
	r->offs = offs;
	r->width = w;
	r->height = h;
	r->y = y;
	r->x = x;
	script_byte_vars.dirty_rect_kind = dirty_rects[0].kind;
	last_dirty_rect = r;
}

void getDirtyRect(int16 index, byte *kind, byte *x, byte *y, byte *w, byte *h, uint16 *offs, byte newkind) {
	*kind = dirty_rects[index].kind;
	*offs = dirty_rects[index].offs;
	*w = dirty_rects[index].width;
	*h = dirty_rects[index].height;
	*y = dirty_rects[index].y;
	*x = dirty_rects[index].x;

	dirty_rects[index].kind = newkind;
	script_byte_vars.dirty_rect_kind = dirty_rects[0].kind;
}

void getDirtyRectAndFree(int16 index, byte *kind, byte *x, byte *y, byte *w, byte *h, uint16 *offs) {
	getDirtyRect(index - 1, kind, x, y, w, h, offs, DirtyRectFree);
}

void getDirtyRectAndSetSprite(int16 index, byte *kind, byte *x, byte *y, byte *w, byte *h, uint16 *offs) {
	getDirtyRect(index - 1, kind, x, y, w, h, offs, DirtyRectSprite);
}

int16 findDirtyRectAndFree(byte kind, byte *x, byte *y, byte *w, byte *h, uint16 *offs) {
	int16 i;
	for (i = 0; i < MAX_DIRTY_RECT; i++) {
		if (dirty_rects[i].kind == kind) {
			getDirtyRect(i, &kind, x, y, w, h, offs, DirtyRectFree);
			return 1;
		}
	}
	return 0;
}

/*Restore screen data from back buffer as specified by dirty rects of kind*/
void popDirtyRects(byte kind) {
	byte x, y;
	byte width, height;
	uint16 offs;
	while (findDirtyRectAndFree(kind, &x, &y, &width, &height, &offs)) {
		cga_CopyScreenBlock(backbuffer, width, height, frontbuffer, offs);
		if (kind == DirtyRectBubble) {
			/*pop bubble's spike*/
			cga_CopyScreenBlock(backbuffer, 2, 21, frontbuffer, offs = (x << 8) | y);
		}
	}
}

void desciTextBox(uint16 x, uint16 y, uint16 width, byte *msg) {
	draw_x = x;
	draw_y = y;
	char_draw_max_width = width;
	cga_DrawTextBox(msg, frontbuffer);
	addDirtyRect(DirtyRectText, draw_x, draw_y, char_draw_max_width + 2, char_draw_coords_y - draw_y + 8, cga_CalcXY_p(draw_x, draw_y));
}

/*Draw dialog bubble with text and spike*/
void drawPersonBubble(byte x, byte y, byte flags, byte *msg) {
	uint16 ofs;
	byte w, h;
	uint16 ww, nw;

	char_draw_max_width = flags & 0x1F;
	char_xlat_table = chars_color_bonw;

	if (g_vm->getLanguage() == Common::EN_USA) {
		calcStringSize(msg, &ww, &nw);
		if (ww >= char_draw_max_width)
			char_draw_max_width = ww;
	}

	/*upper border*/
	ofs = cga_CalcXY_p(x, y);
	ofs = cga_DrawHLineWithEnds(0xF00F,      0,    0, char_draw_max_width, CGA_SCREENBUFFER, ofs);
	ofs = cga_DrawHLineWithEnds(0xC003, 0x0FF0, 0xFF, char_draw_max_width, CGA_SCREENBUFFER, ofs);
	ofs = cga_DrawHLineWithEnds(0, 0x3FFC, 0xFF, char_draw_max_width, CGA_SCREENBUFFER, ofs);
	ofs = cga_DrawHLineWithEnds(0, 0x3FFC, 0xFF, char_draw_max_width, CGA_SCREENBUFFER, ofs);

	/*body*/
	char_draw_coords_x = x;
	char_draw_coords_y = y + 4;

	for (string_ended = 0; !string_ended; char_draw_coords_y += 6) {
		cga_PrintChar(0x3B, CGA_SCREENBUFFER);
		msg = printStringPadded(msg, CGA_SCREENBUFFER);
		cga_PrintChar(0x3C, CGA_SCREENBUFFER);
		char_draw_coords_x = x;
	}

	ofs = cga_CalcXY_p(x, char_draw_coords_y);
	ofs = cga_DrawHLineWithEnds(0xC003, 0x0FF0, 0xFF, char_draw_max_width, CGA_SCREENBUFFER, ofs);
	ofs = cga_DrawHLineWithEnds(0xF00F,      0,    0, char_draw_max_width, CGA_SCREENBUFFER, ofs);

	w = char_draw_max_width + 2;
	h = char_draw_coords_y - y + 2;

	/*draw spike*/
	switch (flags & SPIKE_MASK) {
	case SPIKE_UPLEFT:  /*upper-left spike*/
		ofs = cga_CalcXY_p(x + 1, y - 7);
		DrawSpriteN(18, x + 1, y - 7, CGA_SCREENBUFFER);
		break;
	case SPIKE_UPRIGHT: /*upper-right spike*/
		ofs = cga_CalcXY_p(x + char_draw_max_width, y - 7) - 1;
		DrawSpriteNFlip(18, x + char_draw_max_width, y - 7, CGA_SCREENBUFFER);
		break;
	case SPIKE_DNRIGHT: /*lower-right spike*/
		ofs = cga_CalcXY_p(x + char_draw_max_width, char_draw_coords_y + 1) - 1;
		DrawSpriteNFlip(21, x + char_draw_max_width, char_draw_coords_y + 1, CGA_SCREENBUFFER);
		break;
	case SPIKE_DNLEFT:  /*lower-left spike*/
		ofs = cga_CalcXY_p(x + 1, char_draw_coords_y + 1);
		DrawSpriteN(21, x + 1, char_draw_coords_y + 1, CGA_SCREENBUFFER);
		break;
	case SPIKE_BUBRIGHT:    /*lower-right bubbles*/
		ofs = cga_CalcXY_p(x + char_draw_max_width, char_draw_coords_y + 4);
		DrawSpriteN(20, x + char_draw_max_width, char_draw_coords_y + 4, CGA_SCREENBUFFER);
		break;
	case SPIKE_BUBLEFT: /*lower-left bubbles*/
		ofs = cga_CalcXY_p(x + 1, char_draw_coords_y + 4);
		DrawSpriteN(19, x + 1, char_draw_coords_y + 4, CGA_SCREENBUFFER);
		break;
	}

	addDirtyRect(DirtyRectBubble, ofs >> 8, ofs & 255, w, h, cga_CalcXY_p(x, y));
}

void showPromptAnim(void) {
	if (script_byte_vars.zone_index == 135)
		return;
	waitVBlank();
	DrawSpriteN(cursor_anim_phase ? 23 : 22, 300 / 4, 155, frontbuffer);
	cursor_anim_phase = ~cursor_anim_phase;
}

void promptWait(void) {
	cursor_anim_phase = 0;

	do {
		byte ticks = script_byte_vars.timer_ticks;
		if ((ticks % 8) == 0 && ticks != cursor_anim_ticks) {
			cursor_anim_ticks = ticks;
			showPromptAnim();
		}
		pollInputButtonsOnly();

		if (g_vm->_shouldQuit)
			break;

		g_system->updateScreen();
		g_system->delayMillis(10);
	} while (!buttons);

	if (cursor_anim_phase)
		showPromptAnim();
}

/*
Get string with index num from strings bank
*/
byte *seekToString(byte *bank, uint16 num) {
	byte len;
	byte *p = bank;

	cur_str_index = num;

	num -= 4;
	while (num--) {
		len = *p;
		p += len;
	}
	len = *p;
	cur_str_end = p + len;
	return p + 1;
}

/*
Get string with index num from strings bank, with large string index support for scripts
*/
byte *seekToStringScr(byte *bank, uint16 num, byte **ptr) {
	byte len;
	byte *p = bank;

	if (num < 4) {
		num = (num << 8) | *(++(*ptr));
	}
	cur_str_index = num;

	num -= 4;
	while (num--) {
		len = *p;
		p += len;
	}
	len = *p;
	cur_str_end = p + len;
	return p + 1;
}

} // End of namespace Chamber
