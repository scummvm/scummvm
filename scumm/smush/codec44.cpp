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
#include "chunk.h"
#include "blitter.h"

bool Codec44Decoder::decode(Blitter & dst, Chunk & src) {
	int32 size_line, num;
	int32 length = src.getSize() - 14;
	int32 width = getRect().width();
	int32 height = getRect().height();
	byte * src2 = (byte*)malloc(length);
	byte * org_src2 = src2;
	src.read(src2, length);
	byte * dst2 = _buffer;
	byte val;

	do {
		size_line = READ_LE_UINT16(src2);
		src2 += 2;
		length -= 2;

		while (size_line != 0) {
			num = *src2++;
			val = *src2++;
			memset(dst2, val, num);
			dst2 += num;
			length -= 2;
			size_line -= 2;
			if (size_line != 0) {
				num = READ_LE_UINT16(src2) + 1;
				src2 += 2;
				memcpy(dst2, src2, num);
				dst2 += num;
				src2 += num;
				length -= num + 2;
				size_line -= num + 2;
			}
		}
		dst2--;

	} while (length > 1);

	dst.blit(_buffer, width * height);

	free(org_src2);

	return true;
}
