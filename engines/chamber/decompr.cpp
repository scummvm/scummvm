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
 */

#include "chamber/chamber.h"

namespace Chamber {


static struct {
	unsigned char codes[256];
	unsigned char prefix[256];
	unsigned char suffix[256];
	unsigned char coddict[256];
	unsigned char codlink[256];
	unsigned char stackpos;
} DecompCtx;

unsigned char decode_string(unsigned char code, unsigned char prev_n, unsigned char *stack) {
	unsigned char n;
	while ((n = DecompCtx.coddict[code]) != 0) {
		while (n >= prev_n) {
			n = DecompCtx.codlink[n];
			if (n == 0)
				return code;
		}
		/*if(DecompCtx.stackpos == 255) exit("stack exploded");*/
		stack[DecompCtx.stackpos++] = prev_n = n;
		code = DecompCtx.prefix[n];
	}
	return code;
}

unsigned long decompress(unsigned char huge *data, unsigned char huge *result) {
	unsigned char dict_size, more;
	unsigned int compsize;
	unsigned int i;
	unsigned char code, n, suffix;
	unsigned char stack[256];
	unsigned long decompsize = 0;

	do {
		dict_size = *data++;
		more = *data++;
		compsize = *data++;
		compsize |= (*data++) << 8;

		if (dict_size == 0) {
			/* uncompressed block */
			decompsize += compsize;
			while (compsize--)
				*result++ = *data++;
		} else {
			/* compressed block */
			/*
			memcpy(DecompCtx.codes + 1, data, dict_size); data += dict_size;
			memcpy(DecompCtx.prefix + 1, data, dict_size); data += dict_size;
			memcpy(DecompCtx.suffix + 1, data, dict_size); data += dict_size;
			*/
			for (n = 1, i = 0; i < dict_size; n++, i++) DecompCtx.codes[n] = *data++;
			for (n = 1, i = 0; i < dict_size; n++, i++) DecompCtx.prefix[n] = *data++;
			for (n = 1, i = 0; i < dict_size; n++, i++) DecompCtx.suffix[n] = *data++;

			memset(DecompCtx.coddict, 0, 256);
			for (n = 1, i = 0; i < dict_size; n++, i++) {
				code = DecompCtx.codes[n];
				DecompCtx.codlink[n] = DecompCtx.coddict[code];
				DecompCtx.coddict[code] = n;
			}

			while (compsize-- > 0) {
				code = *data++;

				if (DecompCtx.coddict[code] == 0) {
					/* literal */
					*result++ = code;
					decompsize += 1;
				} else {
					/* string */
					DecompCtx.stackpos = 0;

					n = DecompCtx.coddict[code];

					stack[DecompCtx.stackpos++] = n;

					for (code = DecompCtx.prefix[n];; code = DecompCtx.suffix[n]) {
						suffix = decode_string(code, n, stack);
						*result++ = suffix;
						decompsize += 1;
						if (DecompCtx.stackpos == 0)
							break;
						n = stack[--DecompCtx.stackpos];
					}
				}
			}
		}
	} while (more != 0);

	return decompsize;
}

} // End of namespace Chamber

