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

namespace Sci {

#define LINEMACRO(startx, starty, deltalinear, deltanonlinear, linearvar, nonlinearvar, \
                  linearend, nonlinearstart, linearmod, nonlinearmod) \
	incrNE = ((deltalinear) > 0) ? (deltalinear) : -(deltalinear); \
	incrNE <<= 1; \
	deltanonlinear <<= 1; \
	incrE = ((deltanonlinear) > 0) ? -(deltanonlinear) : (deltanonlinear);  \
	d = nonlinearstart - 1;  \
	while (linearvar != (linearend)) { \
		memcpy(buffer + linewidth * (starty) + (startx), &color, PIXELWIDTH); \
		linearvar += linearmod; \
		if ((d += incrE) < 0) { \
			d += incrNE; \
			nonlinearvar += nonlinearmod; \
		}; \
	}; \
	memcpy(buffer + linewidth * (starty) + (startx), &color, PIXELWIDTH);


template <int PIXELWIDTH>
void _gfx_draw_line_buffer(byte *buffer, int linewidth, Common::Point start, Common::Point end, unsigned int color) {
	int incrE, incrNE, d;
	int dx = ABS(end.x - start.x);
	int dy = ABS(end.y - start.y);
#ifdef SCUMM_BIG_ENDIAN
	color = SWAP_BYTES_32(color);
#endif

	if (dx > dy) {
		int sign1 = (end.x < start.x) ? -1 : 1;
		int sign2 = (end.y < start.y) ? -1 : 1;
		LINEMACRO(start.x, start.y, dx, dy, start.x, start.y, end.x, dx, sign1 * PIXELWIDTH, sign2);
	} else { // dx <= dy
		int sign1 = (end.y < start.y) ? -1 : 1;
		int sign2 = (end.x < start.x) ? -1 : 1;
		LINEMACRO(start.x, start.y, dy, dx, start.y, start.x, end.y, dy, sign1, sign2 * PIXELWIDTH);
	}
}


#undef LINEMACRO

} // End of namespace Sci
