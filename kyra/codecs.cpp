/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "codecs.h"

/*****************************************************************************
 * decode.c - Decoding routines for format80, format40, format20 
 * and format3 type graphics
 * Author: Olaf van der spek
 * Modified for FreeCNC by Kareem Dana
 * Modified for Kyra by Jack Burton
 * Format3 decoding added by Jack Burton
 * Modified for ScummVM by Johannes Schickel
 ****************************************************************************/

namespace Kyra {

/** decompress format 80 compressed data.
 * @param image_in	compressed data.
 * @param image_out	pointer to output uncompressed data.
 * @returns size of uncompressed data.
 */
int Compression::decode80(const uint8* image_in, uint8* image_out) {
	/*
	0 copy 0cccpppp p
	1 copy 10cccccc
	2 copy 11cccccc p p
	3 fill 11111110 c c v
	4 copy 11111111 c c p p
	*/

	const uint8* copyp;
	const uint8* readp = image_in;
	uint8* writep = image_out;
	uint16 code;
	uint16 count;

	while (1) {
		code = *readp++;
		if (~code & 0x80) {
			//bit 7 = 0
			//command 0 (0cccpppp p): copy
			count = (code >> 4) + 3;
			copyp = writep - (((code & 0xf) << 8) + *readp++);
			while (count--)
				*writep++ = *copyp++;
		} else {
			//bit 7 = 1
			count = code & 0x3f;
			if (~code & 0x40) {
				//bit 6 = 0
				if (!count)
					//end of image
					break;
				//command 1 (10cccccc): copy
				while (count--)
					*writep++ = *readp++;
			} else {
				//bit 6 = 1
				if (count < 0x3e) {
					//command 2 (11cccccc p p): copy
					count += 3;

					copyp = (const uint8*)&image_out[READ_LE_UINT16(readp)];
					readp += 2;

					// FIXME: Using memmove sometimes segfaults 
					// (reproducably for Ender), which suggests something Bad here
					//memmove(writep, copyp, count);
					//writep += count;
					//copyp += count;
					while (count--)
						*writep++ = *copyp++;
				} else if (count == 0x3e) {
					//command 3 (11111110 c c v): fill

					count = READ_LE_UINT16(readp);
					readp += 2;
					code = *readp++;
					memset(writep, code, count);
					writep += count;
				} else {
					//command 4 (copy 11111111 c c p p): copy

					count = READ_LE_UINT16(readp);
					readp += 2;

					copyp = (const uint8*)&image_out[READ_LE_UINT16(readp)];
					readp += 2;
					while (count--)
						*writep++ = *copyp++;
				}
			}
		}
	}

	return (writep - image_out);
}

/** decompress format 40 compressed data.
 * @param image_in	compressed data.
 * @param image_out	pointer to put uncompressed data in.
 * @returns size of uncompressed data.
 */
int Compression::decode40(const uint8* image_in, uint8* image_out) {
	/*
	0 fill 00000000 c v
	1 copy 0ccccccc
	2 skip 10000000 c 0ccccccc
	3 copy 10000000 c 10cccccc
	4 fill 10000000 c 11cccccc v
	5 skip 1ccccccc
	*/

	const uint8* readp = image_in;
	uint8* writep = image_out;
	uint16 code;
	uint16 count;

	while (1) {
		code = *readp++;
		if (~code & 0x80) {
			//bit 7 = 0
			if (!code) {
				//command 0 (00000000 c v): fill
				count = *readp++;
				code = *readp++;
				while (count--)
					*writep++ ^= code;
			} else {
				//command 1 (0ccccccc): copy
				count = code;
				while (count--)
					*writep++ ^= *readp++;
			}

		} else {
			//bit 7 = 1
			if (!(count = code & 0x7f)) {

				count = READ_LE_UINT16(readp);
				readp += 2;
				code = count >> 8;
				if (~code & 0x80) {
					//bit 7 = 0
					//command 2 (10000000 c 0ccccccc): skip
					if (!count)
						// end of image
						break;
					writep += count;
				} else {
					//bit 7 = 1
					count &= 0x3fff;
					if (~code & 0x40) {
						//bit 6 = 0
						//command 3 (10000000 c 10cccccc): copy
						while (count--)
							*writep++ ^= *readp++;
					} else {
						//bit 6 = 1
						//command 4 (10000000 c 11cccccc v): fill
						code = *readp++;
						while (count--)
							*writep++ ^= code;
					}
				}
			} else //command 5 (1ccccccc): skip
				writep += count;
		}
	}
	return (writep - image_out);
}

/** decompress format 3 compressed data.
 * @param image_in	compressed data.
 * @param image_out	pointer to put uncompressed data in.
 * @param size of uncompressed image.
 */
int Compression::decode3(const uint8* image_in, uint8* image_out, int size) {
	/* Untested on BIG-Endian machines */
	
	/*
	0 copy
	1 fill 
	2 fill 
	*/
	const uint8* readp = image_in;
	uint8* writep = image_out;
	int16 code;
	int16 count;
	
	do {
		code = *const_cast<int8*>((const int8*)readp++);
		if (code > 0) { // Copy 
			count = code ;
			while (count--)
				*writep++ = *readp++;
		} else if (code == 0) { // Fill(1)
			count = READ_BE_UINT16(readp);
 
			readp += 2;
			code = *readp++;
			while (count--)
				*writep++ = (uint8)code;
		} else if (code < 0) { // Fill (2)
			count = -code;
			code = *readp++;
			while (count--)
				*writep++ = (uint8)code;
		}
	} while ((writep - image_out) < size);	
	
	//and, to be uniform to other decomp. functions...				
	return (writep - image_out); 
}

/** decompress format 20 compressed data.
 * @param s	compressed data.
 * @param d	pointer to pu uncompressed data in.
 * @param cb_s	size of compressed data?
 * @returns size of uncompressed data?
 */
int Compression::decode2(const uint8* s, uint8* d, int cb_s) {
	const uint8* r = s;
	const uint8* r_end = s + cb_s;
	uint8* w = d;
	while (r < r_end) {
		int v = *r++;
		if (v)
			*w++ = v;
		else {
			v = *r++;
			memset(w, 0, v);
			w += v;
		}
	}
	return w - d;

}
} // end of namespace Kyra

