/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/debug.h"
#include "engines/icb/protocol.h"
#include "engines/icb/res_man.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/p4.h"

namespace ICB {

void Render_clip_character(int32 x, int32 y, uint32 width, uint32 height, _rgb *pen, uint8 *ad, uint32 pitch, uint8 *sprite_data_ad);

void Clip_text_print(_rgb *pen, uint32 x, uint32 y, uint8 *base, uint32 pitch, const char *format, ...) {
	// assumes surface is locked and can therefore be written to
	// also assumes a surface width of 640
	// text is printed with colour 0 being transparent

	_frameHeader *head;
	uint8 *charSet, *sprite_data;
	int chr;
	char ascii[150];
	va_list arg_ptr; // variable argument pointer
	int j = 0;

	va_start(arg_ptr, format);
	vsnprintf(ascii, 150, format, arg_ptr);

	charSet = rs_font->Res_open(SYS_FONT, sys_font_hash, font_cluster, font_cluster_hash);

	do {
		chr = (int)ascii[j++];
		chr -= 32;

		_ASSERT((chr >= 0));
		head = (_frameHeader *)FetchFrameHeader(charSet, (uint16)chr);
		sprite_data = (uint8 *)(head + 1);

		Render_clip_character(x, y, head->width, head->height, pen, base, pitch, sprite_data);

		x += head->width + 1; // move on the x coordinate

	} while ((ascii[j]) && j < 150);
}

void Render_clip_character(int32 x, int32 y, uint32 width, uint32 height, _rgb *pen, uint8 *ad, uint32 pitch, uint8 *sprite_data_ad) {
	// 640*480 screen
	// 8, 24 or 32bit colour
	// width and height are in pixels of character
	// data is byte per pixel

	uint32 xx, yy;
	uint32 data_pitch;
	uint32 screen_pitch; // after width of chr$ is taken into account

	data_pitch = 0; // start with no adjustment required

	// chr totaly off left
	if ((int32)(x + (width - 1)) < 0)
		return;
	// and right
	if (x > 639)
		return;

	// chr totally off top
	if ((int32)(y + (height - 1)) < (int32)0)
		return;
	// and bottom
	if (y > 479)
		return;

	// clip y
	// clip top
	if (y < 0) {
		sprite_data_ad += ((0 - y) * width);
		height -= (0 - y); // 0 - -5 == 5 so len=len-5
		y = 0;
	}
	// clip bottom
	if ((y + height) > 479) {
		height -= ((y + height) - 480);
	}

	// clip x
	// clip left
	if (x < 0) {
		data_pitch += (0 - x); // required adjustment after end of line print
		sprite_data_ad += (0 - x);
		width -= (0 - x); // 0 - -5 == 5 so len=len-5
		x = 0;
	}
	// clip right
	if ((x + width) > 639) {
		data_pitch += ((x + width) - 640); // required adjustment after end of line print
		width -= ((x + width) - 640);
	}

	// move to y
	ad += (y * pitch);

	// move to x
	ad += (x * 4);

	screen_pitch = pitch - (width * 4);

	for (yy = 0; yy < height; yy++) {
		for (xx = 0; xx < width; xx++) {
			if (*(sprite_data_ad++)) { // color
				*(ad++) = pen->blue;
				*(ad++) = pen->green;
				*(ad) = pen->red;
				ad += 2;
			} else
				ad += 4;
		}
		ad += screen_pitch;
		sprite_data_ad += data_pitch; // next line of data
	}
}

} // End of namespace ICB
