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

#include "m4/graphics/krn_pal.h"
#include "m4/graphics/gr_pal.h"
#include "m4/vars.h"

namespace M4 {

void krn_ChangeBufferLuminance(Buffer *target, int32 percent) {
	int32		x, y, r, g, b, i;
	uint8 *inverse_palette, pixel, *tempPtr;
	frac16	fracPercent;
	RGB8 *pal;
	uint8		luminancePal[256];

	//paremeter verification
	if ((!target) || (!target->data)) {
		return;
	}
	if ((percent < 0) || (percent == 100)) {
		return;
	}

	if (percent == 0) {
		gr_color_set(__BLACK);
		gr_buffer_rect_fill(target, 0, 0, target->W, target->h);
		return;
	}

	//calculate the frac16 form of the percent
	fracPercent = (percent * 255) / 100;

	//get the palette and the inverse palette
	pal = &_G(master_palette)[0];
	inverse_palette = _G(inverse_pal)->get_ptr();
	if ((!pal) || (!inverse_palette)) {
		return;
	}

	//calculate the luminance Pal table
	for (i = 0; i < 256; i++) {
		r = ((((pal[i].r * fracPercent) >> 10) >> 1)) & 0x1f;
		g = ((((pal[i].g * fracPercent) >> 10) >> 1)) & 0x1f;
		b = ((((pal[i].b * fracPercent) >> 10) >> 1)) & 0x1f;
		luminancePal[i] = inverse_palette[(r << 10) + (g << 5) + b];
	}

	// note: this loop should be y0 to y1, x0 to x1, not a stride*h loop.
	//loop through every pixel replacing it with the index into the luminance table
	tempPtr = target->data;
	for (y = 0; y < target->h; y++) {
		for (x = 0; x < target->stride; x++) {
			*tempPtr = luminancePal[*tempPtr];
			tempPtr++;
			pixel = *tempPtr;
		}
	}

	_G(inverse_pal)->release();
}

} // namespace M4
