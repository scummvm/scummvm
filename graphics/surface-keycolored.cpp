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
 * $URL$
 * $Id$
 */
#include "graphics/surface-keycolored.h"

namespace Graphics {

void SurfaceKeyColored::blit(Surface *surf_src, int16 x, int16 y, OverlayColor transparent) {
	
	if (bytesPerPixel != sizeof(OverlayColor) || surf_src->bytesPerPixel != sizeof(OverlayColor)) return ;

	const OverlayColor *src = (const OverlayColor*)surf_src->pixels;
	int blitW = surf_src->w;
	int blitH = surf_src->h;

	// clip co-ordinates
	if (x < 0) {
		blitW += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		blitH += y;
		src -= y * surf_src->w;
		y = 0;
	}
	if (blitW > w - x) blitW = w - x;
	if (blitH > h - y) blitH = h - y;
	if (blitW <= 0 || blitH <= 0)
		return;

	OverlayColor *dst = (OverlayColor*) getBasePtr(x, y);
	int dstAdd = w - blitW;
	int srcAdd = surf_src->w - blitW;

	for (int i = 0; i < blitH; ++i) { 
		for (int j = 0; j < blitW; ++j, ++dst, ++src) { 
			OverlayColor col = *src;
			if (col != transparent)
				*dst = col;
		}
		dst += dstAdd;
		src += srcAdd; 
	}
}

} // end of namespace Graphics
