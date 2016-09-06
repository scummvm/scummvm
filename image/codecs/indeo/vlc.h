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
#define INIT_VLC_LE             2
#define INIT_VLC_USE_NEW_STATIC 4

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
	static int compare_vlcspec(const void *a, const void *b);
public:
    int bits;
    VLC_TYPE (*table)[2];	///< code, bits
    int table_size, table_allocated;

	VLC();
	~VLC() { ff_free_vlc(); }


	/* Build VLC decoding tables suitable for use with get_vlc().

	'nb_bits' sets the decoding table size (2^nb_bits) entries. The
	bigger it is, the faster is the decoding. But it should not be too
	big to save memory and L1 cache. '9' is a good compromise.

	'nb_codes' : number of vlcs codes

	'bits' : table which gives the size (in bits) of each vlc code.

	'codes' : table which gives the bit pattern of of each vlc code.

	'symbols' : table which gives the values to be returned from get_vlc().

	'xxx_wrap' : give the number of bytes between each entry of the
	'bits' or 'codes' tables.

	'xxx_size' : gives the number of bytes of each entry of the 'bits'
	or 'codes' tables.

	'wrap' and 'size' make it possible to use any memory configuration and types
	(byte/word/long) to store the 'bits', 'codes', and 'symbols' tables.

	'use_static' should be set to 1 for tables, which should be freed
	with av_free_static(), 0 if ff_free_vlc() will be used.
	*/
	int init_vlc(int nb_bits, int nb_codes, const void *bits, int bits_wrap,
		int bits_size, const void *codes, int codes_wrap, int codes_size,
		const void *symbols, int symbols_wrap, int symbols_size, int flags);

	int init_vlc(int nb_bits, int nb_codes, const void *bits, int bits_wrap, int bits_size,
		const void *codes, int codes_wrap, int codes_size, int flags);

	/**
	 * Free VLC data
	 */
	void ff_free_vlc();

	
	/**
	 * Build VLC decoding tables suitable for use with get_vlc().
	 *
	 * @param table_nb_bits  max length of vlc codes to store directly in this table
	 *                       (Longer codes are delegated to subtables.)
	 *
	 * @param nb_codes       number of elements in codes[]
	 *
	 * @param codes          descriptions of the vlc codes
	 *                       These must be ordered such that codes going into the same subtable are contiguous.
	 *                       Sorting by VLCcode.code is sufficient, though not necessary.
	 */
	int build_table(int table_nb_bits, int nb_codes,
		VLCcode *codes, int flags);

	int alloc_table(int size, int use_static);
};


/**
 * Reverse "nbits" bits of the value "val" and return the result
 * in the least significant bits.
 */
extern uint16 inv_bits(uint16 val, int nbits);

/**
 * Swap the order of the bytes in the passed value
 */
extern uint32 bitswap_32(uint32 x);

} // End of namespace Indeo
} // End of namespace Image

#endif
