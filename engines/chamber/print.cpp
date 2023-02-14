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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/cga.h"
#include "chamber/cursor.h"
#include "chamber/dialog.h"

namespace Chamber {


byte *cur_str_end;

byte draw_x;
byte draw_y;

/*
Calculate number of string's character until whitespace
Return current word's characters count and the next word ptr
*/
byte *CalcStringWordWidth(byte *str, uint16 *w) {
	uint16 ww = 0;
	byte c;

	if ((*str & 0x3F) == 0) {
		str++;
		ww++;
	}

	while (str != cur_str_end) {
		if ((*str & 0x3F) == 0)
			break;
		ww++;
		c = *str & 0xC0;
		if (c != 0) {
			if (c == 0x40) { /*space?*/
				str++;
				break;
			}
			ww++;
		}
		str++;
	}

	*w = ww;
	return str;
}

/*
Calculate number of text's words and max word width (in chars)
*/
void CalcStringSize(byte *str, uint16 *w, uint16 *n) {
	uint16 ww = 0, nw = 0, lw;
	byte *s = str;
	do {
		s = CalcStringWordWidth(s, &lw);
		if (lw > ww)
			ww = lw;
		nw += 1;
	} while (s != cur_str_end);
	*w = ww;
	*n = nw;
}

/*
Calculate number of text's lines with respect to set max width
If a line is longer, wrap it to the next line
*/
uint16 CalcTextLines(byte *str) {
	uint16 lines = 1;
	uint16 w, left = char_draw_max_width;
	while (str != cur_str_end) {
		str = CalcStringWordWidth(str, &w);
		if (left > w) {
			left = left - w - 1;
		} else {
			lines++;
			left = char_draw_max_width - w - 1;
		}
	}
	return lines;
}

/*; translate 1-bit raster (4 columns per byte) to 4 2-bit color pixels*/
byte chars_color_bonw[] = {0xFF, 0xFC, 0xF3, 0xF0, 0xCF, 0xCC, 0xC3, 0xC0, 0x3F, 0x3C, 0x33, 0x30, 0x0F, 0x0C,    3,    0}; /*black on white*/
byte chars_color_bonc[] = {0x55, 0x54, 0x51, 0x50, 0x45, 0x44, 0x41, 0x40, 0x15, 0x14, 0x11, 0x10,    5,    4,    1,    0}; /*black on cyan*/
byte chars_color_wonb[] = {   0,    3, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F, 0xC0, 0xC3, 0xCC, 0xCF, 0xF0, 0xF3, 0xFC, 0xFF}; /*white on black*/
byte chars_color_wonc[] = {0x55, 0x57, 0x5D, 0x5F, 0x75, 0xF7, 0x7D, 0x7F, 0xD5, 0xD7, 0xDD, 0xDF, 0xF5, 0xF7, 0xFD, 0xFF}; /*white on cyan*/

void PrintStringPad(uint16 w, byte *target) {
	while (w--)
		cga_PrintChar(0, target);
}

byte *PrintWord(byte *str, byte *target) {
	byte c, f;
	if ((*str & 0x3F) == 0)
		goto skip_1st;
	while (str != cur_str_end) {
		f = *str;
		c = f & 0x3F;
		if (c == 0) {
			if ((f & 0xC0) == 0)
				str++;
			return str;
		}
		cga_PrintChar(c, target);

skip_1st:
		;
		f = *str & 0xC0;
		str++;
		if (f) {
			if (f == 0x80)
				cga_PrintChar(0x25, target);
			else if (f != 0x40)
				cga_PrintChar(0x21, target);
			else
				return str;
		}
	}
	string_ended = 1;
	return str;
}

byte *PrintStringLine(byte *str, uint16 *left, byte *target) {
	uint16 mw = char_draw_max_width;
	for (;;) {
		uint16 w;
		CalcStringWordWidth(str, &w);
		if (mw < w)
			break;
		mw -= w;
		str = PrintWord(str, target);
		if (string_ended || (mw == 0))
			break;
		mw--;
		cga_PrintChar(0, target);
	}
	*left = mw;
	return str;
}

byte *PrintStringPadded(byte *str, byte *target) {
	uint16 w;

	if (g_vm->getLanguage() != Common::EN_USA) {
		uint16 n;
		CalcStringSize(str, &w, &n);
		if (w + 2 >= char_draw_max_width)
			char_draw_max_width = w + 2;
	}

	str = PrintStringLine(str, &w, target);
	if (w != 0)
		PrintStringPad(w, target);
	return str;
}

void PrintStringCentered(byte *str, byte *target) {
	byte pad = 0;
	uint16 ww = 0, lw;
	byte *s = str;
	do {
		s = CalcStringWordWidth(s, &lw);
		ww += lw;
	} while (s != cur_str_end);

	pad = (char_draw_max_width - ww) / 2;
	if (pad) {
		char_draw_max_width -= pad;
		PrintStringPad(pad, target);
	}
	string_ended = 0;   /*TODO: move me elsewhere*/
	PrintStringPadded(str, target);
}

void cga_DrawTextBox(byte *msg, byte *target) {
	uint16 x, y, w, i;
	uint16 ww, nw;

	char_xlat_table = chars_color_bonc;

	if (g_vm->getLanguage() == Common::EN_USA) {
		CalcStringSize(msg, &ww, &nw);
		if (ww >= char_draw_max_width)
			char_draw_max_width = ww;
	}

	x = draw_x * 4;
	y = draw_y;
	w = (char_draw_max_width + 2) * 4 - 2;

	cga_DrawHLine(x + 2, y, w - 2, 0, target);          /*box top*/
	for (i = 0; i < 3; i++)
		cga_DrawHLine(x + 1, y + 1 + i, w, 1, target);  /*top margin*/
	cga_DrawVLine(x,     y + 2, 2, 0, target);          /*left top corner 1*/
	cga_DrawVLine(x + 1, y + 1, 1, 0, target);          /*left top corner 2*/
	cga_DrawVLine(x + w, y + 1, 1, 0, target);      /*right top corner 1*/
	cga_DrawVLine(x + w + 1, y + 2, 2, 0, target);          /*right top corner 2*/

	char_draw_coords_y = draw_y + 4;
	string_ended = 0;
	do {
		char_draw_coords_x = draw_x;
		cga_PrintChar(0x3B, target);
		msg = PrintStringPadded(msg, target);
		cga_PrintChar(0x3C, target);
		char_draw_coords_y += 6;
	} while (!string_ended);

	x = draw_x * 4;
	y = char_draw_coords_y;
	cga_DrawHLine(x + 1, y, w, 1, target);              /*bottom margin*/
	cga_DrawVLine(x + 1, y, 1, 0, target);              /*bottom left corner 1*/
	cga_DrawHLine(x + 2, y + 1, w - 2, 0, target);      /*box bottom*/
	cga_DrawVLine(x + 1, y, 1, 0, target);              /*TODO: duplicated?*/
	cga_DrawVLine(x + w, y, 1, 0, target);              /*bottom right corner*/
}

void DrawMessage(byte *msg, byte *target) {
	uint16 x, y;
	uint16 w, h;
	CalcStringSize(msg, &w, &h);
	char_draw_max_width = (h < 5) ? (w + 2) : 20;
	char_draw_max_height = CalcTextLines(msg) * 6 + 7;

	x = cursor_x / 4;
	if (x < 9)
		x = 9;
	if (x + char_draw_max_width + 2 >= 73)
		x = 73 - (char_draw_max_width + 2);

	y = cursor_y;
	if (y + char_draw_max_height >= 200)
		y = 200 - char_draw_max_height;

	draw_x = x;
	draw_y = y;

	cga_BackupImageReal(cga_CalcXY_p(x, y), char_draw_max_width + 2, char_draw_max_height); /*backup orig. screen data*/
	cga_DrawTextBox(msg, target);                   /*draw box with text*/
	PromptWait();                                   /*wait keypress*/
	cga_RestoreBackupImage(target);                 /*restore screen data*/
}

#if 1
void DebugMessage(char *msg, ...) {
	int16 i;
	byte c;
	byte m[256];
	va_list ap;

	va_start(ap, msg);
	vsnprintf((char *)m, 256, msg, ap);
	va_end(ap);

	for (i = 0; m[i]; i++) {
		c = m[i];
		if (c >= 'A' && c <= 'Z')
			c = 0x21 + (c - 'A');
		else if (c >= 'a' && c <= 'z')
			c = 0x21 + (c - 'a');
		else if (c >= '0' && c <= '9')
			c = 0x10 + (c - '0');
		else if (c == ' ')
			c = 0x20;
		else if (c == '\n')
			c = 0x00;
		else
			c = 0x1F;
		m[i] = c;
	}

	cur_str_end = m + i;

	DrawMessage(m, frontbuffer);
}
#endif

} // End of namespace Chamber
