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

#include "common/scummsys.h"

namespace Sci {

/* Config parameters:
** FUNCTION_NAME: Name of the blitter function
** USE_PRIORITY: Whether to care about the priority buffer
** BYTESPP: Bytes per pixel
*/
template <int BYTESPP, bool USE_PRIORITY, bool REVERSE_ALPHA>
void _gfx_crossblit(byte *dest, byte *src, int bytes_per_dest_line, int bytes_per_src_line,
	int xl, int yl, byte *alpha, int bytes_per_alpha_line, int bytes_per_alpha_pixel,
	unsigned int alpha_test_mask, unsigned int alpha_min,
	byte *priority_buffer, int bytes_per_priority_line, int bytes_per_priority_pixel, int priority
	) {
	int x, y;
	int alpha_end = xl * bytes_per_alpha_pixel;

	for (y = 0; y < yl; y++) {
		int pixel_offset = 0;
		int alpha_offset = 0;
		int priority_offset = 0;

		for (x = 0; x < alpha_end; x += bytes_per_alpha_pixel) {
			if (((alpha_test_mask & alpha[x]) < alpha_min) ^ REVERSE_ALPHA) {

				if (USE_PRIORITY) {
					if (priority_buffer[priority_offset] <= priority) {
						priority_buffer[priority_offset] = priority;
						memcpy(dest + pixel_offset, src + pixel_offset, BYTESPP);
					}
				} else {
					memcpy(dest + pixel_offset, src + pixel_offset, BYTESPP);
				}
			}

			pixel_offset += BYTESPP;
			alpha_offset += bytes_per_alpha_pixel;
			if (USE_PRIORITY)
				priority_offset += bytes_per_priority_pixel;
		}

		dest += bytes_per_dest_line;
		src += bytes_per_src_line;
		alpha += bytes_per_alpha_line;
		if (USE_PRIORITY)
			priority_buffer += bytes_per_priority_line;
	}
}

} // End of namespace Sci
