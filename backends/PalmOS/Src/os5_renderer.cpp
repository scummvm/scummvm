/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#include "be_os5.h"

void OSystem_PalmOS5::render_1x(RectangleType &r, PointType &p) {
	Coord o = 0;

	if (_overlayVisible) {
		int16 *src = _overlayP;
		int16 *dst =  _workScreenP;
		MemMove(dst, src, _screenWidth * _screenHeight * 2);

	} else {
		byte *src = _offScreenP;
		int16 *dst =  _workScreenP;
		int cnt = _screenWidth * _screenHeight;
		o = _current_shake_pos;

		do {
			*dst++ = _nativePal[*src++];
		} while (--cnt);
	}

	p.x = _screenOffset.x;
	p.y = _screenOffset.y + o;
	RctSetRectangle(&r, 0, 0, _screenWidth, _screenHeight - o);
}

void OSystem_PalmOS5::render_landscape(RectangleType &r, PointType &p) {
	Coord x, y, o = 0;
	int16 *dst =  _workScreenP;

	if (_overlayVisible) {
		int16 *src = _overlayP;

		for (y = 0; y < 100; y++) {
			// draw 2 lines
			for (x = 0; x < 320; x++) {
				*dst++ = *src++;
				*dst++ = *src;
				*dst++ = *src++;
			}
			// copy the second to the next line
			MemMove(dst, dst - 480, 480 * 2);
			dst += 480;
		}

	} else {
		byte *src = _offScreenP;
		o = _current_shake_pos;

		for (y = 0; y < 100; y++) {
			// draw 2 lines
			for (x = 0; x < 320; x++) {
				*dst++ = _nativePal[*src++];
				*dst++ = _nativePal[*src];
				*dst++ = _nativePal[*src++];
			}
			// copy the second to the next line
			MemMove(dst, dst - 480, 480 * 2);
			dst += 480;
		}
	}

	p.x = _screenOffset.x;
	p.y = _screenOffset.y + o;
	RctSetRectangle(&r, 0, 0, 480, 300 - o);
}

void OSystem_PalmOS5::render_portrait(RectangleType &r, PointType &p) {
	Coord x, y, o = 0;
	int16 *dst =  _workScreenP;

	if (_overlayVisible) {
		int16 *src = _overlayP + 320 - 1;
		int16 *src2 = src;

		for (x = 0; x < 160; x++) {
			for (y = 0; y < 100; y++) {
				*dst++ = *src;
				src += 320;
				*dst++ = *src;
				*dst++ = *src;
				src += 320;
			}
			src = --src2;

			for (y = 0; y < 100; y++) {
				*dst++ = *src;
				src += 320;
				*dst++ = *src;
				*dst++ = *src;
				src += 320;
			}
			src = --src2;

			MemMove(dst, dst - 300, 300 * 2);	// 300 = 200 x 1.5
			dst += 300;
		}

	} else {
		byte *src = _offScreenP + 320 - 1;
		byte *src2 = src;
		o = _current_shake_pos;

		for (x = 0; x < 160; x++) {
			for (y = 0; y < 100; y++) {
				*dst++ = _nativePal[*src];
				src += 320;
				*dst++ = _nativePal[*src];
				*dst++ = _nativePal[*src];
				src += 320;
			}
			src = --src2;

			for (y = 0; y < 100; y++) {
				*dst++ = _nativePal[*src];
				src += 320;
				*dst++ = _nativePal[*src];
				*dst++ = _nativePal[*src];
				src += 320;
			}
			src = --src2;

			MemMove(dst, dst - 300, 300 * 2);	// 300 = 200 x 1.5
			dst += 300;
		}
	}

	p.y = _screenOffset.x;
	p.x = _screenOffset.y + o;
	RctSetRectangle(&r, 0, 0, 300 - o, 480);
}

