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

// Resource library

#include "common/compression/dcl.h"
#include "common/util.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "sci/resource/decompressor.h"
#include "sci/sci.h"
#include "sci/resource/resource.h"

namespace Sci {
int Decompressor::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
	while (nPacked && !(src->eos() || src->err())) {
		uint32 chunk = MIN<uint32>(1024, nPacked);
		src->read(dest, chunk);
		nPacked -= chunk;
		dest += chunk;
	}
	return (src->eos() || src->err()) ? 1 : 0;
}

void Decompressor::init(Common::ReadStream *src, byte *dest, uint32 nPacked,
						uint32 nUnpacked) {
	_src = src;
	_dest = dest;
	_szPacked = nPacked;
	_szUnpacked = nUnpacked;
	_nBits = 0;
	_dwRead = _dwWrote = 0;
	_dwBits = 0;
}

void Decompressor::fetchBitsMSB() {
	while (_nBits <= 24) {
		_dwBits |= ((uint32)_src->readByte()) << (24 - _nBits);
		_nBits += 8;
		_dwRead++;
	}
}

uint32 Decompressor::getBitsMSB(int n) {
	// fetching more data to buffer if needed
	if (_nBits < n)
		fetchBitsMSB();
	uint32 ret = _dwBits >> (32 - n);
	_dwBits <<= n;
	_nBits -= n;
	return ret;
}

byte Decompressor::getByteMSB() {
	return getBitsMSB(8);
}

void Decompressor::fetchBitsLSB() {
	while (_nBits <= 24) {
		_dwBits |= ((uint32)_src->readByte()) << _nBits;
		_nBits += 8;
		_dwRead++;
	}
}

uint32 Decompressor::getBitsLSB(int n) {
	// fetching more data to buffer if needed
	if (_nBits < n)
		fetchBitsLSB();
	uint32 ret = (_dwBits & ~(0xFFFFFFFFU << n));
	_dwBits >>= n;
	_nBits -= n;
	return ret;
}

byte Decompressor::getByteLSB() {
	return getBitsLSB(8);
}

void Decompressor::putByte(byte b) {
	_dest[_dwWrote++] = b;
}
//-------------------------------
//  Huffman decompressor
//-------------------------------
int DecompressorHuffman::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked,
								uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);
	byte numnodes;
	int16 c;
	uint16 terminator;

	numnodes = _src->readByte();
	terminator = _src->readByte() | 0x100;
	_nodes = new byte [numnodes << 1];
	_src->read(_nodes, numnodes << 1);

	while ((c = getc2()) != terminator && (c >= 0) && !isFinished())
		putByte(c);

	delete[] _nodes;
	return _dwWrote == _szUnpacked ? 0 : 1;
}

int16 DecompressorHuffman::getc2() {
	byte *node = _nodes;
	int16 next;
	while (node[1]) {
		if (getBitsMSB(1)) {
			next = node[1] & 0x0F; // use lower 4 bits
			if (next == 0)
				return getByteMSB() | 0x100;
		} else
			next = node[1] >> 4; // use higher 4 bits
		node += next << 1;
	}
	return (int16)(*node | (node[1] << 8));
}

//-------------------------------
// LZW Decompressor for SCI0/01/1
//-------------------------------

int DecompressorLZW::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
	byte *buffer = nullptr;

	switch (_compression) {
	case kCompLZW:	// SCI0 LZW compression
	case kCompLZW1: // SCI01/1 LZW compression
		return unpackLZW(src, dest, nPacked, nUnpacked);
	case kCompLZW1View:
		buffer = new byte[nUnpacked];
		unpackLZW(src, buffer, nPacked, nUnpacked);
		reorderView(buffer, dest);
		break;
	case kCompLZW1Pic:
		buffer = new byte[nUnpacked];
		unpackLZW(src, buffer, nPacked, nUnpacked);
		reorderPic(buffer, dest, nUnpacked);
		break;
	default:
		break;
	}
	delete[] buffer;
	return 0;
}

// Decompresses SCI0 LZW and SCI01/1 LZW, depending on _compression value.
//
// SCI0:    LSB-first.
// SCI01/1: MSB-first, code size is increased one code earlier than necessary.
//          This is known as an "early change" bug in LZW implementations.
int DecompressorLZW::unpackLZW(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);

	uint16 codeBitLength = 9;
	uint16 tableSize = 258;
	uint16 codeLimit = (_compression == kCompLZW) ? 512 : 511;

	// LZW table
	uint16 *stringOffsets = new uint16[4096]; // 0-257: unused
	uint16 *stringLengths = new uint16[4096]; // 0-257: unused

	while (!isFinished()) {
		uint16 code = (_compression == kCompLZW) ?
		              getBitsLSB(codeBitLength) :
		              getBitsMSB(codeBitLength);

		if (code >= tableSize) {
			warning("LZW code %x exceeds table size %x", code, tableSize);
			break;
		}

		if (code == 257) { // terminator
			break;
		}

		if (code == 256) { // reset command
			codeBitLength = 9;
			tableSize = 258;
			codeLimit = (_compression == kCompLZW) ? 512 : 511;
			continue;
		}

		uint16 newStringOffset = _dwWrote;
		if (code <= 255) {
			// Code is a literal byte
			putByte(code);
		} else {
			// Code is a table index

			// Boundary check included because the previous decompressor had a
			// comment saying it's "a normal situation" for a string to attempt
			// to write beyond the destination. I have not seen this occur.
			for (int i = 0; i < stringLengths[code] && !isFinished(); i++) {
				putByte(dest[stringOffsets[code] + i]);
			}
		}

		// Stop adding to the table once it is full
		if (tableSize >= 4096) {
			continue;
		}

		// Increase code size once a bit limit has been reached
		if (tableSize == codeLimit && codeBitLength < 12) {
			codeBitLength++;
			codeLimit = 1 << codeBitLength;
			if (_compression != kCompLZW) {
				codeLimit--;
			}
		}

		// Append code to table
		stringOffsets[tableSize] = newStringOffset;
		stringLengths[tableSize] = _dwWrote - newStringOffset + 1;
		tableSize++;
	}

	delete[] stringOffsets;
	delete[] stringLengths;

	return _dwWrote == _szUnpacked ? 0 : SCI_ERROR_DECOMPRESSION_ERROR;
}

#define PAL_SIZE 1284
#define EXTRA_MAGIC_SIZE 15
#define VIEW_HEADER_COLORS_8BIT 0x80

void DecompressorLZW::decodeRLE(byte **rledata, byte **pixeldata, byte *outbuffer, int size) {
	int pos = 0;
	byte nextbyte;
	byte *rd = *rledata;
	byte *ob = outbuffer;
	byte *pd = *pixeldata;

	while (pos < size) {
		nextbyte = *rd++;
		*ob++ = nextbyte;
		pos++;
		switch (nextbyte & 0xC0) {
		case 0x40:
		case 0x00:
			memcpy(ob, pd, nextbyte);
			pd += nextbyte;
			ob += nextbyte;
			pos += nextbyte;
			break;
		case 0xC0:
		default:
			break;
		case 0x80:
			nextbyte = *pd++;
			*ob++ = nextbyte;
			pos++;
			break;
		}
	}

	*rledata = rd;
	*pixeldata = pd;
}

/**
 * Does the same this as decodeRLE, only to determine the length of the
 * compressed source data.
 */
int DecompressorLZW::getRLEsize(byte *rledata, int dsize) {
	int pos = 0;
	int size = 0;

	while (pos < dsize) {
		byte nextbyte = *(rledata++);
		pos++;
		size++;

		switch (nextbyte & 0xC0) {
		case 0x40:
		case 0x00:
			pos += nextbyte;
			break;
		case 0xC0:
		default:
			break;
		case 0x80:
			pos++;
			break;
		}
	}

	return size;
}

enum {
	PIC_OPX_EMBEDDED_VIEW = 1,
	PIC_OPX_SET_PALETTE = 2,
	PIC_OP_OPX = 0xfe
};

void DecompressorLZW::reorderPic(byte *src, byte *dest, int dsize) {
	uint16 view_size, view_start, cdata_size;
	int i;
	byte *seeker = src;
	byte *writer = dest;
	char viewdata[7];
	byte *cdata, *cdata_start;

	*writer++ = PIC_OP_OPX;
	*writer++ = PIC_OPX_SET_PALETTE;

	for (i = 0; i < 256; i++) /* Palette translation map */
		*writer++ = i;

	WRITE_LE_UINT32(writer, 0); /* Palette stamp */
	writer += 4;

	view_size = READ_LE_UINT16(seeker);
	seeker += 2;
	view_start = READ_LE_UINT16(seeker);
	seeker += 2;
	cdata_size = READ_LE_UINT16(seeker);
	seeker += 2;

	memcpy(viewdata, seeker, sizeof(viewdata));
	seeker += sizeof(viewdata);

	memcpy(writer, seeker, 4*256); /* Palette */
	seeker += 4*256;
	writer += 4*256;

	if (view_start != PAL_SIZE + 2) { /* +2 for the opcode */
		memcpy(writer, seeker, view_start-PAL_SIZE-2);
		seeker += view_start - PAL_SIZE - 2;
		writer += view_start - PAL_SIZE - 2;
	}

	if (dsize != view_start + EXTRA_MAGIC_SIZE + view_size) {
		memcpy(dest + view_size + view_start + EXTRA_MAGIC_SIZE, seeker,
		       dsize - view_size - view_start - EXTRA_MAGIC_SIZE);
		seeker += dsize - view_size - view_start - EXTRA_MAGIC_SIZE;
	}

	cdata_start = cdata = (byte *)malloc(cdata_size);
	memcpy(cdata, seeker, cdata_size);
	seeker += cdata_size;

	writer = dest + view_start;
	*writer++ = PIC_OP_OPX;
	*writer++ = PIC_OPX_EMBEDDED_VIEW;
	*writer++ = 0;
	*writer++ = 0;
	*writer++ = 0;
	WRITE_LE_UINT16(writer, view_size + 8);
	writer += 2;

	memcpy(writer, viewdata, sizeof(viewdata));
	writer += sizeof(viewdata);

	*writer++ = 0;

	decodeRLE(&seeker, &cdata, writer, view_size);

	free(cdata_start);
}

void DecompressorLZW::buildCelHeaders(byte **seeker, byte **writer, int celindex, int *cc_lengths, int max) {
	for (int c = 0; c < max; c++) {
		memcpy(*writer, *seeker, 6);
		*seeker += 6;
		*writer += 6;
		int w = *((*seeker)++);
		WRITE_LE_UINT16(*writer, w); /* Zero extension */
		*writer += 2;

		*writer += cc_lengths[celindex];
		celindex++;
	}
}

void DecompressorLZW::reorderView(byte *src, byte *dest) {
	byte *cellengths;
	int loopheaders;
	int lh_present;
	int lh_mask;
	int pal_offset;
	int cel_total;
	int unknown;
	byte *seeker = src;
	char celcounts[100];
	byte *writer = dest;
	byte *lh_ptr;
	byte *rle_ptr, *pix_ptr;
	int l, lb, c, celindex, lh_last = -1;
	int chptr;
	int w;
	int *cc_lengths;
	byte **cc_pos;

	/* Parse the main header */
	cellengths = src + READ_LE_UINT16(seeker) + 2;
	seeker += 2;
	loopheaders = *seeker++;
	lh_present = *seeker++;
	lh_mask = READ_LE_UINT16(seeker);
	seeker += 2;
	unknown = READ_LE_UINT16(seeker);
	seeker += 2;
	pal_offset = READ_LE_UINT16(seeker);
	seeker += 2;
	cel_total = READ_LE_UINT16(seeker);
	seeker += 2;

	cc_pos = (byte **) malloc(sizeof(byte *) * cel_total);
	cc_lengths = (int *) malloc(sizeof(int) * cel_total);

	for (c = 0; c < cel_total; c++)
		cc_lengths[c] = READ_LE_UINT16(cellengths + 2 * c);

	*writer++ = loopheaders;
	*writer++ = VIEW_HEADER_COLORS_8BIT;
	WRITE_LE_UINT16(writer, lh_mask);
	writer += 2;
	WRITE_LE_UINT16(writer, unknown);
	writer += 2;
	WRITE_LE_UINT16(writer, pal_offset);
	writer += 2;

	lh_ptr = writer;
	writer += 2 * loopheaders; /* Make room for the loop offset table */

	pix_ptr = writer;

	memcpy(celcounts, seeker, lh_present);
	seeker += lh_present;

	lb = 1;
	celindex = 0;

	rle_ptr = pix_ptr = cellengths + (2 * cel_total);
	w = 0;

	for (l = 0; l < loopheaders; l++) {
		if (lh_mask & lb) { /* The loop is _not_ present */
			if (lh_last == -1) {
				warning("Error: While reordering view: Loop not present, but can't re-use last loop");
				lh_last = 0;
			}
			WRITE_LE_UINT16(lh_ptr, lh_last);
			lh_ptr += 2;
		} else {
			lh_last = writer - dest;
			WRITE_LE_UINT16(lh_ptr, lh_last);
			lh_ptr += 2;
			WRITE_LE_UINT16(writer, celcounts[w]);
			writer += 2;
			WRITE_LE_UINT16(writer, 0);
			writer += 2;

			/* Now, build the cel offset table */
			chptr = (writer - dest) + (2 * celcounts[w]);

			for (c = 0; c < celcounts[w]; c++) {
				WRITE_LE_UINT16(writer, chptr);
				writer += 2;
				cc_pos[celindex+c] = dest + chptr;
				chptr += 8 + READ_LE_UINT16(cellengths + 2 * (celindex + c));
			}

			buildCelHeaders(&seeker, &writer, celindex, cc_lengths, celcounts[w]);

			celindex += celcounts[w];
			w++;
		}

		lb = lb << 1;
	}

	if (celindex < cel_total) {
		warning("View decompression generated too few (%d / %d) headers", celindex, cel_total);
		free(cc_pos);
		free(cc_lengths);
		return;
	}

	/* Figure out where the pixel data begins. */
	for (c = 0; c < cel_total; c++)
		pix_ptr += getRLEsize(pix_ptr, cc_lengths[c]);

	rle_ptr = cellengths + (2 * cel_total);
	for (c = 0; c < cel_total; c++)
		decodeRLE(&rle_ptr, &pix_ptr, cc_pos[c] + 8, cc_lengths[c]);

	if (pal_offset) {
		*writer++ = 'P';
		*writer++ = 'A';
		*writer++ = 'L';

		for (c = 0; c < 256; c++)
			*writer++ = c;

		seeker -= 4; /* The missing four. Don't ask why. */
		memcpy(writer, seeker, 4*256 + 4);
	}

	free(cc_pos);
	free(cc_lengths);
}

//----------------------------------------------
// DCL decompressor for SCI1.1
//----------------------------------------------

int DecompressorDCL::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked,
							uint32 nUnpacked) {
	return Common::decompressDCL(src, dest, nPacked, nUnpacked) ? 0 : SCI_ERROR_DECOMPRESSION_ERROR;
}

#ifdef ENABLE_SCI32

//----------------------------------------------
// STACpack/LZS decompressor for SCI32
// Based on Andre Beck's code from
// https://web.archive.org/web/20070817214826/http://micky.ibh.de/~beck/stuff/lzs4i4l/
//----------------------------------------------
int DecompressorLZS::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);
	return unpackLZS();
}

int DecompressorLZS::unpackLZS() {
	uint16 offs = 0;
	uint32 clen;

	while (!isFinished()) {
		if (getBitsMSB(1)) { // Compressed bytes follow
			if (getBitsMSB(1)) { // Seven bit offset follows
				offs = getBitsMSB(7);
				if (!offs) // This is the end marker - a 7 bit offset of zero
					break;
				if (!(clen = getCompLen())) {
					warning("lzsDecomp: length mismatch");
					return SCI_ERROR_DECOMPRESSION_ERROR;
				}
				copyComp(offs, clen);
			} else { // Eleven bit offset follows
				offs = getBitsMSB(11);
				if (!(clen = getCompLen())) {
					warning("lzsDecomp: length mismatch");
					return SCI_ERROR_DECOMPRESSION_ERROR;
				}
				copyComp(offs, clen);
			}
		} else // Literal byte follows
			putByte(getByteMSB());
	} // end of while ()
	return _dwWrote == _szUnpacked ? 0 : SCI_ERROR_DECOMPRESSION_ERROR;
}

uint32 DecompressorLZS::getCompLen() {
	uint32 clen;
	int nibble;
	// The most probable cases are hardcoded
	switch (getBitsMSB(2)) {
	case 0:
		return 2;
	case 1:
		return 3;
	case 2:
		return 4;
	default:
		switch (getBitsMSB(2)) {
		case 0:
			return 5;
		case 1:
			return 6;
		case 2:
			return 7;
		default:
		// Ok, no shortcuts anymore - just get nibbles and add up
			clen = 8;
			do {
				nibble = getBitsMSB(4);
				clen += nibble;
			} while (nibble == 0xf);
			return clen;
		}
	}
}

void DecompressorLZS::copyComp(int offs, uint32 clen) {
	int hpos = _dwWrote - offs;

	while (clen--)
		putByte(_dest[hpos++]);
}

#endif	// #ifdef ENABLE_SCI32

} // End of namespace Sci
