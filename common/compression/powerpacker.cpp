/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/compression/powerpacker.h"
#include "common/memstream.h"
#include "common/debug.h"

namespace Common {

/* the decoder presented here is taken from pplib by Stuart Caie. The
 * following statement comes from the original source.
 *
 * pplib 1.0: a simple PowerPacker decompression and decryption library
 * placed in the Public Domain on 2003-09-18 by Stuart Caie.
 */

#define PP_READ_BITS(nbits, var) do {				\
	bit_cnt = (nbits); (var) = 0;				\
	while (bits_left < bit_cnt) {				\
		if (buf < src) return 0;			\
		bit_buffer |= *--buf << bits_left;		\
		bits_left += 8;					\
	}							\
	bits_left -= bit_cnt;					\
	while (bit_cnt--) {					\
		(var) = ((var) << 1) | (bit_buffer & 1);	\
		bit_buffer >>= 1;				\
	}							\
} while (0)

#define PP_BYTE_OUT(byte) do {					\
	if (out <= dest) return 0;				\
	*--out = (byte); written++;				\
} while (0)


int PowerPackerStream::ppDecrunchBuffer(const byte *src, byte *dest, uint32 src_len, uint32 dest_len) {
	const byte *buf, *off_lens;
	byte *out, *dest_end, bits_left = 0, bit_cnt;
	uint32 bit_buffer = 0, x, todo, offbits, offset, written = 0;

	if (!src || !dest) return 0;

	/* set up input and output pointers */
	off_lens = src; src = &src[4];
	buf = &src[src_len];

	out = dest_end = &dest[dest_len];

	/* skip the first few bits */
	PP_READ_BITS(src[src_len + 3], x);

	/* while there are input bits left */
	while (written < dest_len) {
		PP_READ_BITS(1, x);
		if (x == 0) {
			/* bit==0: literal, then match. bit==1: just match */
			todo = 1; do { PP_READ_BITS(2, x); todo += x; } while (x == 3);
			while (todo--) { PP_READ_BITS(8, x); PP_BYTE_OUT(x); }

			/* should we end decoding on a literal, break out of the main loop */
			if (written == dest_len) break;
		}

		/* match: read 2 bits for initial offset bitlength / match length */
		PP_READ_BITS(2, x);
		offbits = off_lens[x];
		todo = x+2;
		if (x == 3) {
			PP_READ_BITS(1, x);
			if (x == 0) offbits = 7;
			PP_READ_BITS(offbits, offset);
			do { PP_READ_BITS(3, x); todo += x; } while (x == 7);
		}
		else {
			PP_READ_BITS(offbits, offset);
		}
		if (&out[offset] > dest_end) return 0; /* match_overflow */
		while (todo--) { x = out[offset]; PP_BYTE_OUT(x); }
	}

	/* all output bytes written without error */
	return 1;
}

uint16 PowerPackerStream::getCrunchType(uint32 signature) {
	byte eff = 0;

	switch (signature) {
	case 0x50503230: /* PP20 */
	case 0x5041434b: /* PACK, non-standard header used in amiga floppy ITE.  */
		eff = 4;
		break;
	case 0x50504C53: /* PPLS */
		error("PPLS crunched files are not supported");
#if 0
		eff = 8;
		break;
#endif
	case 0x50583230: /* PX20 */
		error("PX20 crunched files are not supported");
#if 0
		eff = 6;
		break;
#endif
	default:
		eff = 0;

	}

	return eff;
}

PowerPackerStream::PowerPackerStream(Common::SeekableReadStream &stream) {
	_dispose = false;

	uint32 signature = stream.readUint32BE();
	if (getCrunchType(signature) == 0) {
		stream.seek(0, SEEK_SET);
		_stream = &stream;
		return;
	}

	stream.seek(-4, SEEK_END);
	uint32 decrlen = stream.readUint32BE() >> 8;
	byte *dest = (byte *)malloc(decrlen);

	uint32 crlen = stream.size() - 4;
	byte *src = (byte *)malloc(crlen);
	stream.seek(4, SEEK_SET);
	stream.read(src, crlen);

	ppDecrunchBuffer(src, dest, crlen-8, decrlen);

	free(src);
	_stream = new Common::MemoryReadStream(dest, decrlen, DisposeAfterUse::YES);
	_dispose = true;
}

byte *PowerPackerStream::unpackBuffer(const byte *input, uint32 input_len, uint32 &output_len) {
	if (input_len < 8) {
		output_len = 0;
		return nullptr;
	}
	uint32 signature = READ_BE_UINT32(input);
	if (getCrunchType(signature) == 0) {
		output_len = 0;
		return nullptr;
	}

	output_len = READ_BE_UINT32(input + input_len - 4) >> 8;
	byte *dest = new byte[output_len];

	ppDecrunchBuffer(input + 4, dest, input_len - 12, output_len);

	return dest;
}

}
