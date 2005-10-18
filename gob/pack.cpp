/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/pack.h"

namespace Gob {

int32 unpackData(char *sourceBuf, char *destBuf) {
	uint32 realSize;
	uint32 counter;
	uint16 cmd;
	byte *src;
	byte *dest;
	byte *tmpBuf;
	int16 off;
	byte len;
	byte i;
	int16 j;
	uint16 tmpIndex;

	realSize = READ_LE_UINT32(sourceBuf);
	counter = READ_LE_UINT32(sourceBuf);

	tmpBuf = (byte *)malloc(4114);

	/*
	 * Can use assembler unpacker for small blocks - for speed.
	 * Don't need this anymore :)
	 */
	/*
	 * if (realSize < 65000)
	 * {
	 * asm_unpackData(sourceBuf, destBuf, tmpBuf);
	 * free(tmpBuf);
	 * return realSize;
	 * }
	 */

	if (tmpBuf == 0)
		return 0;

	for (j = 0; j < 4078; j++)
		tmpBuf[j] = 0x20;
	tmpIndex = 4078;

	src = (byte *)(sourceBuf + 4);
	dest = (byte *)destBuf;

	cmd = 0;
	while (1) {
		cmd >>= 1;
		if ((cmd & 0x0100) == 0) {
			cmd = *src | 0xff00;
			src++;
		}
		if ((cmd & 1) != 0) {	/* copy */
			*dest++ = *src;
			tmpBuf[tmpIndex] = *src;
			src++;
			tmpIndex++;
			tmpIndex %= 4096;
			counter--;
			if (counter == 0)
				break;
		} else {	/* copy string */

			off = *src++;
			off |= (*src & 0xf0) << 4;
			len = (*src & 0x0f) + 3;
			src++;
			for (i = 0; i < len; i++) {
				*dest++ = tmpBuf[(off + i) % 4096];
				counter--;
				if (counter == 0) {
					free(tmpBuf);
					return realSize;
				}
				tmpBuf[tmpIndex] = tmpBuf[(off + i) % 4096];
				tmpIndex++;
				tmpIndex %= 4096;
			}
		}
	}
	free(tmpBuf);
	return realSize;
}

}				// End of namespace Gob
