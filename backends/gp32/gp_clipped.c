/* ScummVM - Scumm Interpreter
 * GP32 Blitting library
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2003,2004,2005  CARTIER Matthieu
 * Copyright (C) 2005 Won Star - GP32 Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

//Some global variables and macros

#define	screen_width	320
#define	screen_height	240

void gp_FastSolidBlit(void *framebuffer, int dx, const int dy, const int width, const int height, const unsigned char *src, const int trans, const int coul) {
	int xmin, ymin, xmax, ymax;
	int height2 = ( (height + 3) >> 2) << 2;

	if(dx < 0) {
		xmin = -dx;
	} else xmin = 0;
	if( (dx++ + width) > screen_width) {
		xmax = screen_width - dx;
	} else xmax = width - 1;
	if(dy < 0) {
		ymax = height + dy - 1;
	} else ymax = height - 1;
	if( (dy + height) > screen_height) {
		ymin = dy + height - screen_height;
	} else ymin = 0;
	if( (xmin > xmax) || (ymin > ymax) ) return;

	unsigned char *dst4 = framebuffer + (dx + xmax) * screen_height - height - dy + 1 + ymin;
	src += (xmax * height2 + ymin);
	ASMFastSolidBlit(src, dst4, xmax - xmin, ymax - ymin, height2, trans, coul);
}

//Copy from framebuffer to dest
void gp_SaveBitmap(void *framebuffer, int dx, const int dy, const int width, const int height, const unsigned char *dest) { //Sur l'icran
	int xmin, ymin, xmax, ymax;
	int height2 = ( (height + 3) >> 2) << 2;

	if(dx < 0) {
		xmin = -dx;
	} else xmin = 0;
	if( (dx++ + width) > screen_width) {
		xmax = screen_width - dx;
	} else xmax = width - 1;
	if(dy < 0) {
		ymax = height + dy - 1;
	} else ymax = height - 1;
	if( (dy + height) > screen_height) {
		ymin = dy + height - screen_height;
	} else ymin = 0;
	if( (xmin > xmax) || (ymin > ymax) ) return;

	unsigned char *src4 = framebuffer + (dx + xmax) * screen_height - height - dy + ymin;
	dest += (xmin * height2 + ymin + 1);
	ASMSaveBitmap(src4, dest, xmax - xmin, ymax - ymin, height2);
}

//Clears area with color #0, should not trigger clicky noise

void gp_FastClear(void *framebuffer, int dx, int dy, int width, int height) {
	int xmin, ymin, xmax, ymax;

	if(dx < 0) {
		xmin = -dx;
	} else xmin = 0;
	if( (dx++ + width) > screen_width) {
		xmax = screen_width - dx;
	} else xmax = width - 1;
	if(dy < 0) {
		ymax = height + dy - 1;
	} else ymax = height - 1;
	if( (dy + height) > screen_height) {
		ymin = dy + height - screen_height;
	} else ymin = 0;
	if( (xmin > xmax) || (ymin > ymax) ) return;

	int decaly = screen_height - height - dy;

	unsigned char *dst4 = framebuffer + (dx + xmax) * screen_height - height - dy + ymin;
	ASMFastClear(dst4, xmax - xmin, ymax - ymin);
}
