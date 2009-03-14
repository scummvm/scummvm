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
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Resource library

#include <common/util.h>
#include <common/endian.h>
#include <common/debug.h>
#include "sci/scicore/decompressor.h"
#include "sci/sci.h"

namespace Sci {
int Decompressor::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
	uint32 chunk;
	while (nPacked && !src->ioFailed()) {
		chunk = MIN<uint32>(1024, nPacked);
		src->read(dest, chunk);
		nPacked -= chunk;
		dest += chunk;
	}
	return src->ioFailed() ? 1 : 0;
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

bool Decompressor::getBitMSB() {
	// fetching more bits to _dwBits buffer
	if (_nBits == 0)
		fetchBitsMSB();
	bool b = _dwBits & 0x80000000;
	_dwBits <<= 1;
	_nBits--;
	return b;
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

bool Decompressor::getBitLSB() {
	// fetching more bits to _dwBits buffer
	if (_nBits == 0) 
		fetchBitsLSB();
	bool b = _dwBits & 0x1;
	_dwBits >>= 1;
	_nBits--;
	return b;
}

uint32 Decompressor::getBitsLSB(int n) {
	// fetching more data to buffer if needed
	if (_nBits < n)
		fetchBitsLSB();
	uint32 ret = (_dwBits & ~((~0) << n));
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
		if (getBitMSB()) {
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
void DecompressorLZW::init(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
	Decompressor::init(src, dest, nPacked, nUnpacked);

	_numbits = 9;
	_curtoken = 0x102;
	_endtoken = 0x1ff;
}

int DecompressorLZW::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked,
								uint32 nUnpacked) {
	byte *buffer = NULL;

	switch (_compression) {
	case kCompLZW:	// SCI0 LZW compression
		return unpackLZW(src, dest, nPacked, nUnpacked);
		break;
	case kCompLZW1: // SCI01/1 LZW compression
		return unpackLZW1(src, dest, nPacked, nUnpacked);
		break;
	case kCompLZW1View:
		buffer = new byte[nUnpacked];
		unpackLZW1(src, buffer, nPacked, nUnpacked);
		reorderView(buffer, dest);
		break;
	case kCompLZW1Pic:
		buffer = new byte[nUnpacked];
		unpackLZW1(src, buffer, nPacked, nUnpacked);
		reorderPic(buffer, dest, nUnpacked);
		break;
	}
	delete[] buffer;
	return 0;
}

int DecompressorLZW::unpackLZW(Common::ReadStream *src, byte *dest, uint32 nPacked,
                                uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);

	uint16 token; // The last received value

	uint16 tokenlist[4096]; // pointers to dest[]
	uint16 tokenlengthlist[4096]; // char length of each token
	uint16 tokenlastlength = 0;

	while (!isFinished()) {
		token = getBitsLSB(_numbits);

		if (token == 0x101)
			return 0; // terminator
		if (token == 0x100) { // reset command
			_numbits = 9;
			_endtoken = 0x1FF;
			_curtoken = 0x0102;
		} else {
			if (token > 0xff) {
				if (token >= _curtoken) {
					warning("unpackLZW: Bad token %x", token);
					return SCI_ERROR_DECOMPRESSION_INSANE;
				} 
				tokenlastlength = tokenlengthlist[token] + 1;
				if (_dwWrote + tokenlastlength > _szUnpacked) {
					// For me this seems a normal situation, It's necessary to handle it
					warning("unpackLZW: Trying to write beyond the end of array(len=%d, destctr=%d, tok_len=%d)",
					        _szUnpacked, _dwWrote, tokenlastlength);
					for (int i = 0; _dwWrote < _szUnpacked; i++)
						putByte(dest [tokenlist[token] + i]);
				} else
					for (int i = 0; i < tokenlastlength; i++)
						putByte(dest[tokenlist[token] + i]);
			} else {
				tokenlastlength = 1;
				if (_dwWrote >= _szUnpacked)
					warning("unpackLZW: Try to write single byte beyond end of array");
				else
					putByte(token);
			}
			if (_curtoken > _endtoken && _numbits < 12) {
				_numbits++;
				_endtoken = (_endtoken << 1) + 1;
			}
			if (_curtoken <= _endtoken) {
				tokenlist[_curtoken] = _dwWrote - tokenlastlength;
				tokenlengthlist[_curtoken] = tokenlastlength;
				_curtoken++;
			}

		}
	}

	return _dwWrote == _szUnpacked ? 0 : SCI_ERROR_DECOMPRESSION_INSANE;
}

int DecompressorLZW::unpackLZW1(Common::ReadStream *src, byte *dest, uint32 nPacked,
                                uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);

	byte stak[0x1014];
	byte lastchar = 0;
	uint16 stakptr = 0, lastbits = 0;
	tokenlist tokens[0x1004];
	memset(tokens, 0, sizeof(tokens));


	byte decryptstart = 0;
	uint16 bitstring;
	uint16 token;
	bool bExit = false;

	while (!isFinished() && !bExit) {
		switch (decryptstart) {
		case 0:
			bitstring = getBitsMSB(_numbits);
			if (bitstring == 0x101) {// found end-of-data signal
				bExit = true;
				continue;
			}
			putByte(bitstring);
			lastbits = bitstring;
			lastchar = (bitstring & 0xff);
			decryptstart = 1;
			break;

		case 1:
			bitstring = getBitsMSB(_numbits);
			if (bitstring == 0x101) { // found end-of-data signal
				bExit = true;
				continue;
			}
			if (bitstring == 0x100) { // start-over signal
				_numbits = 9;
				_curtoken = 0x102;
				_endtoken = 0x1ff;
				decryptstart = 0;
				continue;
			}

			token = bitstring;
			if (token >= _curtoken) { // index past current point
				token = lastbits;
				stak[stakptr++] = lastchar;
			}
			while ((token > 0xff) && (token < 0x1004)) { // follow links back in data
				stak[stakptr++] = tokens[token].data;
				token = tokens[token].next;
			}
			lastchar = stak[stakptr++] = token & 0xff;
			// put stack in buffer
			while (stakptr > 0) {
				putByte(stak[--stakptr]);
				if (_dwWrote == _szUnpacked) {
					bExit = true;
					continue;
				}
			}
			// put token into record
			if (_curtoken <= _endtoken) {
				tokens[_curtoken].data = lastchar;
				tokens[_curtoken].next = lastbits;
				_curtoken++;
				if (_curtoken == _endtoken && _numbits < 12) {
					_numbits++;
					_endtoken = (_endtoken << 1) + 1;
				}
			}
			lastbits = bitstring;
			break;
		}
	}
	return _dwWrote == _szUnpacked ? 0 : SCI_ERROR_DECOMPRESSION_INSANE;
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

/*
 * Does the same this as above, only to determine the length of the compressed
 * source data.
 *
 * Yes, this is inefficient.
 */
int DecompressorLZW::getRLEsize(byte *rledata, int dsize) {
	int pos = 0;
	byte nextbyte;
	int size = 0;

	while (pos < dsize) {
		nextbyte = *(rledata++);
		pos++;
		size++;

		switch (nextbyte & 0xC0) {
		case 0x40:
		case 0x00:
			pos += nextbyte;
			break;
		case 0xC0:
			break;
		case 0x80:
			pos++;
			break;
		}
	}

	return size;
}

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
		return;
	}

	/* Figure out where the pixel data begins. */
	for (c = 0; c < cel_total; c++)
		pix_ptr += getRLEsize(pix_ptr, cc_lengths[c]);

	rle_ptr = cellengths + (2 * cel_total);
	for (c = 0; c < cel_total; c++)
		decodeRLE(&rle_ptr, &pix_ptr, cc_pos[c] + 8, cc_lengths[c]);

	*writer++ = 'P';
	*writer++ = 'A';
	*writer++ = 'L';

	for (c = 0; c < 256; c++)
		*writer++ = c;

	seeker -= 4; /* The missing four. Don't ask why. */
	memcpy(writer, seeker, 4*256 + 4);

	free(cc_pos);
	free(cc_lengths);
}

//----------------------------------------------
// DCL decompressor for SCI1.1
//----------------------------------------------
#define HUFFMAN_LEAF 0x40000000
#define BRANCH_SHIFT 12
#define BRANCH_NODE(pos, left, right)  ((left << BRANCH_SHIFT) | (right)),
#define LEAF_NODE(pos, value)  ((value) | HUFFMAN_LEAF),

static int length_tree[] = {
#include "treedef.1"
	0 // We need something witout a comma at the end
};

static int distance_tree[] = {
#include "treedef.2"
	0 // We need something witout a comma at the end
};

static int ascii_tree[] = {
#include "treedef.3"
	0 // We need something witout a comma at the end
};

int DecompressorDCL::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked,
                            uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);
	return unpackDCL(dest);
}


int DecompressorDCL::huffman_lookup(int *tree) {
	int pos = 0;
	int bit;

	while (!(tree[pos] & HUFFMAN_LEAF)) {
		bit = getBitLSB();
		debugC(kDebugLevelDclInflate, "[%d]:%d->", pos, bit);
		pos = bit ? tree[pos] & 0xFFF : tree[pos] >> BRANCH_SHIFT;
	}
	debugC(kDebugLevelDclInflate, "=%02x\n", tree[pos] & 0xffff);
	return tree[pos] & 0xFFFF;
}

#define DCL_ASCII_MODE 1

int DecompressorDCL::unpackDCL(byte* dest) {
	int mode, length_param, value;
	uint32 val_distance, val_length;

	mode = getByteLSB();
	length_param = getByteLSB();

	if (mode == DCL_ASCII_MODE) {
		warning("DCL-INFLATE: Decompressing ASCII mode (untested)");
	} else if (mode) {
		warning("DCL-INFLATE: Error: Encountered mode %02x, expected 00 or 01\n", mode);
		return -1;
	}

	if (length_param < 3 || length_param > 6)
		warning("Unexpected length_param value %d (expected in [3,6])\n", length_param);

	while (_dwWrote < _szUnpacked) {
		if (getBitLSB()) { // (length,distance) pair
			value = huffman_lookup(length_tree);

			if (value < 8)
				val_length = value + 2;
			else
				val_length = 8 + (1 << (value - 7)) + getBitsLSB(value - 7);

			debugC(kDebugLevelDclInflate, " | ");

			value = huffman_lookup(distance_tree);

			if (val_length == 2)
				val_distance = (value << 2) | getBitsLSB(2);
			else
				val_distance = (value << length_param) | getBitsLSB(length_param);
			val_distance ++;

			debugC(kDebugLevelDclInflate, "\nCOPY(%d from %d)\n", val_length, val_distance);

			if (val_length + _dwWrote > _szUnpacked) {
				warning("DCL-INFLATE Error: Write out of bounds while copying %d bytes", val_length);
				return SCI_ERROR_DECOMPRESSION_OVERFLOW;
			}

			if (_dwWrote < val_distance) {
				warning("DCL-INFLATE Error: Attempt to copy from before beginning of input stream");
				return SCI_ERROR_DECOMPRESSION_INSANE;
			}

			while (val_length) {
				uint32 copy_length = (val_length > val_distance) ? val_distance : val_length;
				assert(val_distance >= copy_length);
				uint32 pos = _dwWrote - val_distance;
				for (uint32 i = 0; i < copy_length; i++)
					putByte(dest[pos + i]);

				if (Common::isDebugChannelEnabled(kDebugLevelDclInflate)) {
					for (uint32 i = 0; i < copy_length; i++)
						debugC(kDebugLevelDclInflate, "\33[32;31m%02x\33[37;37m ", dest[pos + i]);
					debugC(kDebugLevelDclInflate, "\n");
				}

				val_length -= copy_length;
				val_distance += copy_length;
			}

		} else { // Copy byte verbatim
			value = (mode == DCL_ASCII_MODE) ? huffman_lookup(ascii_tree) : getByteLSB();
			putByte(value);
			debugC(kDebugLevelDclInflate, "\33[32;31m%02x \33[37;37m", value);
		}
	}

	return _dwWrote == _szUnpacked ? 0 : SCI_ERROR_DECOMPRESSION_INSANE;
}

//----------------------------------------------
// STACpack/LZS decompressor for SCI32
// Based on Andre Beck's code from http://micky.ibh.de/~beck/stuff/lzs4i4l/
//----------------------------------------------
int DecompressorLZS::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);
	return unpackLZS();
}

int DecompressorLZS::unpackLZS() {
	uint16 offs = 0, clen;

	while (!isFinished()) {
		if (getBitMSB()) { // Compressed bytes follow 
			if (getBitMSB()) { // Seven bit offset follows
				offs = getBitsMSB(7);
				if (!offs) // This is the end marker - a 7 bit offset of zero 
					break;
				if (!(clen = getCompLen())) {
					warning("lzsDecomp: length mismatch");
					return SCI_ERROR_DECOMPRESSION_INSANE;
				}
				copyComp(offs, clen);
			} else { // Eleven bit offset follows 
				offs = getBitsMSB(11);
				if (!(clen = getCompLen())) {
					warning("lzsDecomp: length mismatch");
					return SCI_ERROR_DECOMPRESSION_INSANE;
				}
				copyComp(offs, clen);
			}
		} else // Literal byte follows
			putByte(getByteMSB());
	} // end of while ()
	return _dwWrote == _szUnpacked ? 0 : SCI_ERROR_DECOMPRESSION_INSANE;
}

uint16 DecompressorLZS::getCompLen() {
	int clen, nibble;
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
			}while (nibble == 0xf);
			return clen;
		}
	}
}

void DecompressorLZS::copyComp(int offs, int clen) {
	int hpos = _dwWrote - offs;

	while (clen--)
		putByte(_dest[hpos++]);
}

} // End of namespace Sci
