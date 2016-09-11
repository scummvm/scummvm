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

#include "common/scummsys.h"

/* VLC code
 *
 * Original copyright note: * Intel Indeo 4 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef IMAGE_CODECS_INDEO_VLC_H
#define IMAGE_CODECS_INDEO_VLC_H

#include "image/codecs/indeo/get_bits.h"

namespace Image {
namespace Indeo {

#define VLC_TYPE int16

enum VLCFlag {
	INIT_VLC_LE             = 2,
	INIT_VLC_USE_NEW_STATIC = 4
};

struct VLCcode {
	uint8 bits;
	uint16 symbol;

	/**
	 * codeword, with the first bit-to-be-read in the msb
	 * (even if intended for a little-endian bitstream reader)
	 */
	uint32 code;
};

struct VLC {
private:
	static int compareVlcSpec(const void *a, const void *b);

	/**
	 * Gets a value of a given size from a table
	 * @param table		Table to get data from
	 * @param idx		Index of value to retrieve
	 * @param wrap		Size of elements with alignment
	 * @param size		Size of elements
	 */
	static uint getData(const void *table, uint idx, uint wrap, uint size);
public:
	int _bits;
	VLC_TYPE (*_table)[2];	///< code, bits
	int _tableSize, _tableAllocated;

	VLC();

	/* Build VLC decoding tables suitable for use with get_vlc().

	'nbBits' sets the decoding table size (2^nbBits) entries. The
	bigger it is, the faster is the decoding. But it should not be too
	big to save memory and L1 cache. '9' is a good compromise.

	'nbCodes' : number of vlcs codes

	'bits' : table which gives the size (in bits) of each vlc code.

	'codes' : table which gives the bit pattern of of each vlc code.

	'symbols' : table which gives the values to be returned from get_vlc().

	'xxx_wrap' : give the number of bytes between each entry of the
	'bits' or 'codes' tables.

	'xxx_size' : gives the number of bytes of each entry of the 'bits'
	or 'codes' tables.

	'wrap' and 'size' make it possible to use any memory configuration and types
	(byte/word/long) to store the 'bits', 'codes', and 'symbols' tables.

	'useStatic' should be set to 1 for tables, which should be freed
	with av_free_static(), 0 if freeVlc() will be used.
	*/
	int init_vlc(int nbBits, int nbCodes, const void *bits, int bitsWrap,
		int bitsSize, const void *codes, int codesWrap, int codesSize,
		const void *symbols, int symbolsWrap, int symbolsSize, int flags);

	int init_vlc(int nbBits, int nbCodes, const void *bits, int bitsWrap, int bitsSize,
		const void *codes, int codesWrap, int codesSize, int flags);

	/**
	 * Free VLC data
	 */
	void freeVlc();


	/**
	 * Build VLC decoding tables suitable for use with get_vlc().
	 *
	 * @param tableNbBits    max length of vlc codes to store directly in this table
	 *                       (Longer codes are delegated to subtables.)
	 *
	 * @param nbCodes        number of elements in codes[]
	 *
	 * @param codes          descriptions of the vlc codes
	 *                       These must be ordered such that codes going into the same subtable are contiguous.
	 *                       Sorting by VLCcode.code is sufficient, though not necessary.
	 */
	int buildTable(int tableNbBits, int nbCodes, VLCcode *codes, int flags);

	int allocTable(int size, int useStatic);
};

} // End of namespace Indeo
} // End of namespace Image

#endif
