/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/frotz/pics_decoder.h"
#include "glk/frotz/pics.h"
#include "common/memstream.h"

namespace Glk {
namespace Frotz {

PictureDecoder::PictureDecoder() {
	_tableVal = new byte[3 * 3840];
	_tableRef = (uint16 *)(_tableVal + 3840);
}

PictureDecoder::~PictureDecoder() {
	delete[] _tableVal;
}

Common::SeekableReadStream *PictureDecoder::decode(Common::ReadStream &src, uint flags,
		const Common::Array<byte> &palette, uint display, size_t width, size_t height) {
    static const int raise_bits[4] = { 0x0100, 0x0300, 0x0700, 0x0000 };
	Common::MemoryWriteStreamDynamic out(DisposeAfterUse::NO);
	byte buf[512];
    byte transparent;
    int colour_shift;
	int first_colour;
    int code, prev_code = 0;
    int next_entry;
    int bits_per_code;
    int bits_shift;
    int bits;
    int bufpos = 0;
    int i;

	/*
	 * Write out dimensions of image
	 */
	out.writeUint16LE(width);
	out.writeUint16LE(height);

    /* Set up the color mapping. This is only used for MCGA pictures; the colour
	 * map affects every picture on the screen. The first colour to be defined is
     * colour 2. Every map defines up to 14 colours (colour 2 to 15). These colours
	 * are not related to the standard Z-machine colour scheme which remains unchanged.
	 * (This is based on the Amiga interpreter which had to work with 16 colours.
	 * Colours 0 and 1 were used for text; changing the text colours actually changed
     * palette entries 0 and 1. This interface uses the same trick in Amiga mode.)
	 */

	switch (display) {
	case CGA:
		colour_shift = -2;
		break;
	case EGA:
		colour_shift = 0;
		break;
	case MCGA:
		colour_shift = 32;
		first_colour = 34;
		break;
	case AMIGA:
		colour_shift = -1;
		first_colour = 65;
		break;
	default:
		break;
	}
	
	// Note: we don't actually use paletted indexes, so adjust colour_shift
	// relative to first_colour
	colour_shift -= first_colour;

	out.writeUint16LE(palette.size() / 3);
	if (!palette.empty())
		out.write(&palette[0], palette.size());

    /* Bit 0 of "flags" indicates that the picture uses a transparent colour,
	 * the top four bits tell us which colour it is. For CGA and MCGA pictures
	 * this is always 0; for EGA pictures it can be any colour between 0 and 15.
	 */
    transparent = 0xff;
    if (flags & 1)
		transparent = flags >> 12;
	out.writeByte(transparent);

    /* The uncompressed picture is a long sequence of bytes. Every byte holds
	 * the colour of a pixel, starting at the top left, stopping at the bottom right.
	 * We keep track of our position in the current line. (There is a special case:
	 * CGA pictures with no transparent colour are stored as bit patterns, i.e.
	 * every byte holds the pattern for eight pixels. A pixel must be white if the
	 * corresponding bit is set, otherwise it must be black.)
	 */
//    current_x = 1 + width;
//    current_y = 1 - 1;

    /* The compressed picture is a stream of bits. We read the file byte-wise,
	 * storing the current byte in the variable "bits". Several bits make one code;
	 * the variable "bits_shift" helps us to build the next code.
	 */
    bits_shift = 0;
    bits = 0;

reset_table:
    /* Clear the table. We use a table of 3840 entries. Each entry consists of both
	 * a value and a reference to another table entry. Following these references
	 * we get a sequence of values. At the start of decompression all table entries
	 * are undefined. Later we see how entries are set and used.
	 */
    next_entry = 1;

    /* At the start of decompression 9 bits make one code; during the process this can
	 * rise to 12 bits per code. 9 bits are sufficient to address both 256 literal values
	 * and 256 table entries; 12 bits are sufficient to address both 256 literal values
	 * and all 3840 table entries. The number of bits per code rises with the number of
	 * table entries. When the table is cleared, the number of bits per code drops back to 9.
	 */
    bits_per_code = 9;

next_code:

    /* Read the next code from the graphics file. This requires some confusing bit operations.
	 * Note that low bits always come first. Usually there are a few bits left over from
	 * the previous code; these bits must be used before further bits are read from the
	 * graphics file.
	 */
    code = bits >> (8 - bits_shift);

    do {
		bits = src.readByte();
		code |= bits << bits_shift;

		bits_shift += 8;
	} while (bits_shift < bits_per_code);

	bits_shift -= bits_per_code;

	code &= 0xfff >> (12 - bits_per_code);

	/* There are two codes with a special meaning. The first one is 256 which clears
	 * the table and sets the number of bits per code to 9. (This is necessary when
	 * the table is full.) The second one is 257 which marks the end of the picture.
	 * For the sake of efficiency, we drecement the code by 256.
	 */
	code -= 256;

	if (code == 0)
		goto reset_table;
	if (code == 1) {
		bool t[256];
		// *******DEBUG*******
		Common::fill(&t[0], &t[256], false);
		for (uint idx = 0; idx < out.size(); ++idx)
			t[*((byte *)out.getData() + idx)] = true;

		return new Common::MemoryReadStream(out.getData(), out.size(), DisposeAfterUse::YES);
	}

	/* Codes from 0 to 255 are literals, i.e. they represent a plain byte value.
	 * Codes from 258 onwards are references to table entries, i.e. they represent
	 * a sequence of byte values (see the remarks on the table above). This means
	 * that for each code one or several byte values are added to the decompressed
	 * picture. But there is yet more work to do: Every time we read a code one
	 * table entry is set. As we said above, a table entry consist of both a value
	 * and a reference to another table entry. If the current code is a literal,
	 * then the value has to be set to this literal; but if the code refers to a
	 * sequence of byte values, then the value has to be set to the last byte of
	 * this sequence. In any case, the reference is set to the previous code.
	 * Finally, one should be aware that a code may legally refer to the table entry
	 * which is currently being set. This requires some extra care.
	 */
	_tableRef[next_entry] = prev_code;

	prev_code = code;

	while (code >= 0) {
		buf[bufpos++] = _tableVal[code];
		code = (short) _tableRef[code];
	}

    if (next_entry == prev_code)
		buf[0] = code;

    _tableVal[next_entry] = code;

    /* The number of bits per code is incremented when the current number of bits
	 * no longer suffices to address all defined table entries; but in any case
	 * the number of bits may never be greater than 12.
	 */
    next_entry++;

    if (next_entry == raise_bits[bits_per_code - 9])
		bits_per_code++;

reverse_buffer:
    /* Output the sequence of byte values (pixels). The order of the sequence
	 * must be reversed. (This is why we have stored the sequence in a buffer;
	 * experiments show that a buffer of 512 bytes suffices.)
	 *
	 * Either add a single pixel or a pattern of eight bits (b/w CGA pictures without
	 * a transparent colour) to the current line. Increment our position by 1 or 8
	 * respectively. The pixel may have to be painted several times if the scaling
	 * factor is greater than one.
	 */
    if (display == CGA && transparent == 0xff) {
		// TODO
    } else {
		byte v = code;

		if (v != transparent) {
			v += colour_shift;

			if (display != MCGA) {
				// TODO
			} else {
				// position shift
			}

			out.writeByte(v);

			if (display == AMIGA) {
				// TODO
			}
		}
    }

    /* If there are no more values in the buffer then read the next code from the file. 
	 * Otherwise fetch the next byte value from the buffer and continue outputing the picture.
	 */
    if (bufpos == 0)
		goto next_code;

	code = (code & ~0xff) | buf[--bufpos];
    goto reverse_buffer;
}

} // End of namespace Frotz
} // End of namespace Glk
