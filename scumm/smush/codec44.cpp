/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdafx.h>
#include "codec44.h"
#include "chunck.h"
#include "blitter.h"

bool Codec44Decoder::decode(Blitter & dst, Chunck & src) {
	int size_line;
	int num;
	int w, width = getRect().width() + 1;
	int h, height = getRect().height() + 1;
	bool zero;
#ifdef DEBUG_CODEC44
	debug(7, "codec44 : %dx%d", width, height);
#endif

	for(h = 0; h < height - 1; h++) {
		w = width;
		size_line = src.getWord(); // size of compressed line !
#ifdef DEBUG_CODEC44
		debug(7, "codec44 : h == %d, size_line == %d", h, size_line);
#endif
		zero = true;
		while(size_line > 1) {
			num = src.getWord();
			size_line -= 2;
			if(zero) {
#ifdef DEBUG_CODEC44
				debug(7, "codec44 : zeroing %d, entries", num);
#endif
				if(w == num)
					num--;
				w -= num;
				if(num)
					dst.put(0, num);
			} else {
				num += 1;
#ifdef DEBUG_CODEC44
				debug(7, "codec44 : blitting %d, entries", num);
#endif
				if(w == num)
					num--;
				w -= num;
				dst.blit(src, num);
				size_line -= num;
			}
			zero = !zero;
		}
	}
	return true;
}
