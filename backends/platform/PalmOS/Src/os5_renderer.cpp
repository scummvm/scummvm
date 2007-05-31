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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
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

void OSystem_PalmOS5::render_landscapeAny(RectangleType &r, PointType &p) {
	Coord x, y, o = 0;
	int16 *dst =  _workScreenP;

	if (_overlayVisible) {
		for (y = 0; y < _screenDest.h; y++) {
			int16 *src = _overlayP + *(_scaleTableY + y);
			for (x = 0; x < _screenDest.w; x++) {
				*dst++ = *(src + *(_scaleTableX + x));
			}
		}

	} else {
		o = _current_shake_pos;

		for (y = 0; y < _screenDest.h; y++) {
			byte *src = _offScreenP + *(_scaleTableY + y);
			for (x = 0; x < _screenDest.w; x++) {
				*dst++ = *(_nativePal + *(src + *(_scaleTableX + x)));
			}
		}
	}

	p.x = _screenOffset.x;
	p.y = _screenOffset.y + o;
	RctSetRectangle(&r, 0, 0,  _screenDest.w,  _screenDest.h - o);
}
