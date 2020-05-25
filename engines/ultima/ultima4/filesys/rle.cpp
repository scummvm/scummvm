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

#include "ultima/ultima4/filesys/rle.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Decompress an RLE encoded file.
 */
long rleDecompressFile(Common::ReadStream *in, long inlen, void **out) {
	void *indata;
	long outlen;

	/* input file should be longer than 0 bytes */
	if (inlen <= 0)
		return -1;

	/* load compressed file into memory */
	indata = malloc(inlen);
	in->read(indata, inlen);

	outlen = rleDecompressMemory(indata, inlen, out);

	free(indata);

	return outlen;
}

long rleDecompressMemory(void *in, long inlen, void **out) {
	byte *indata, *outdata;
	long outlen;

	/* input should be longer than 0 bytes */
	if (inlen <= 0)
		return -1;

	indata = (byte *)in;

	/* determine decompressed file size */
	outlen = rleGetDecompressedSize(indata, inlen);

	if (outlen <= 0)
		return -1;

	/* decompress file from inlen to outlen */
	outdata = (byte *) malloc(outlen);
	rleDecompress(indata, inlen, outdata, outlen);

	*out = outdata;

	return outlen;
}

/**
 * Determine the uncompressed size of RLE compressed data.
 */
long rleGetDecompressedSize(byte *indata, long inlen) {
	byte *p;
	byte ch, count;
	long len = 0;

	p = indata;
	while ((p - indata) < inlen) {
		ch = *p++;
		if (ch == RLE_RUNSTART) {
			count = *p++;
			p++;
			len += count;
		} else
			len++;
	}

	return len;
}

/**
 * Decompress a block of RLE encoded memory.
 */
long rleDecompress(byte *indata, long inlen, byte *outdata, long outlen) {
	int i;
	byte *p, *q;
	byte ch, count, val;

	p = indata;
	q = outdata;
	while ((p - indata) < inlen) {
		ch = *p++;
		if (ch == RLE_RUNSTART) {
			count = *p++;
			val = *p++;
			for (i = 0; i < count; i++) {
				*q++ = val;
				if ((q - outdata) >= outlen)
					break;
			}
		} else {
			*q++ = ch;
			if ((q - outdata) >= outlen)
				break;
		}
	}

	return q - outdata;
}

} // End of namespace Ultima4
} // End of namespace Ultima
