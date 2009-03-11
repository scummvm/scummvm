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

#define _SCI_DECOMPRESS_DEBUG
namespace Sci {

int Decompressor::unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
                         uint32 nUnpacked) {
	return copyBytes(src, dest, nPacked);
}

int Decompressor::copyBytes(Common::ReadStream *src, Common::WriteStream *dest, uint32 nSize) {
	byte buff[1024];
	uint32 chunk;
	while (nSize && !src->ioFailed() && !dest->ioFailed()) {
		chunk = MIN<uint32>(1024, nSize);
		src->read(buff, chunk);
		dest->write(buff, chunk);
		nSize -= chunk;
	}
	return src->ioFailed() || dest->ioFailed() ? 1 : 0;
}
void Decompressor::init(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
                        uint32 nUnpacked) {
	_src = src;
	_dest = dest;
	_szPacked = nPacked;
	_szUnpacked = nUnpacked;
	_nBits = 0;
	_dwRead = _dwWrote = 0;
	_dwBits = 0;
}

void Decompressor::fetchBits() {
	while (_nBits <= 24) {
		_dwBits |= ((uint32)_src->readByte()) << (24 - _nBits);
		_nBits += 8;
		_dwRead++;
	}
}

bool Decompressor::getBit() {
	// fetching more bits to _dwBits buffer
	if (_nBits == 0)
		fetchBits();
	bool b = _dwBits & 0x80000000;
	_dwBits <<= 1;
	_nBits--;
	return b;
}

uint32 Decompressor::getBits(int n) {
	// fetching more data to buffer if needed
	if (_nBits < n)
		fetchBits();
	uint32 ret = _dwBits >> (32 - n);
	_dwBits <<= n;
	_nBits -= n;
	return ret;
}

byte Decompressor::getByte() {
	return getBits(8);
}

void Decompressor::putByte(byte b) {
	_dest->writeByte(b);
	_dwWrote++;
}
//-------------------------------
//  Huffman decompressor
//-------------------------------
int DecompressorHuffman::unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
                                uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);

	byte numnodes;
	int16 c;
	uint16 terminator;

	numnodes = _src->readByte();
	terminator = _src->readByte() | 0x100;
	_nodes = (byte *)malloc(numnodes << 1);
	_src->read(_nodes, numnodes << 1);

	while ((c = getc2()) != terminator && (c >= 0) && (_szUnpacked-- > 0))
		putByte(c);

	free(_nodes);
	return _dwWrote ? 0 : 1;
}

int16 DecompressorHuffman::getc2() {
	byte *node = _nodes;
	int16 next;
	while (node[1]) {
		if (getBit()) {
			next = node[1] & 0x0F; // use lower 4 bits
			if (next == 0)
				return getByte() | 0x100;
		} else
			next = node[1] >> 4; // use higher 4 bits
		node += next << 1;
	}
	return (int16)(*node | (node[1] << 8));
}


//-------------------------------
// LZW-like Decompressor
//-------------------------------
void DecompressorComp3::init(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked, uint32 nUnpacked) {
	Decompressor::init(src, dest, nPacked, nUnpacked);

	_lastchar = _lastbits = _stakptr = 0;
	_numbits = 9;
	_curtoken = 0x102;
	_endtoken = 0x1ff;
	memset(_tokens, 0, sizeof(_tokens));
}

int DecompressorComp3::unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
                              uint32 nUnpacked) {
	byte *buffer = NULL;
	byte *buffer2 = NULL;
	Common::MemoryWriteStream *pBuff = NULL;
	Common::MemoryReadStream *pBuffIn = NULL;

	switch (_compression) {
	case kComp3: // Comp3 compression
		return doUnpack(src, dest, nPacked, nUnpacked);
		break;
	case kComp3View:
	case kComp3Pic:
		buffer = new byte[nUnpacked];
		pBuff = new Common::MemoryWriteStream(buffer, nUnpacked);
		doUnpack(src, pBuff, nPacked, nUnpacked);
		if (_compression == kComp3View) {
			buffer2 = new byte[nUnpacked];
			view_reorder(buffer, buffer2);
			dest->write(buffer2, nUnpacked);
		} else {
			pBuffIn = new Common::MemoryReadStream(buffer, nUnpacked);
			reorderPic(pBuffIn, dest, nUnpacked);
		}
		delete[] buffer2;
		delete[] buffer;
		delete pBuff;
		delete pBuffIn;
		break;
	}
	return 0;
}

int DecompressorComp3::doUnpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
                                uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);

	byte decryptstart = 0;
	uint16 bitstring;
	uint16 token;
	bool bExit = false;

	while (_szUnpacked && !bExit) {
		switch (decryptstart) {
		case 0:
			bitstring = getBits(_numbits);
			if (bitstring == 0x101) {// found end-of-data signal
				bExit = true;
				continue;
			}
			putByte(bitstring);
			_szUnpacked--;
			_lastbits = bitstring;
			_lastchar = (bitstring & 0xff);
			decryptstart = 1;
			break;

		case 1:
			bitstring = getBits(_numbits);
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
				token = _lastbits;
				_stak[_stakptr++] = _lastchar;
			}
			while ((token > 0xff) && (token < 0x1004)) { // follow links back in data
				_stak[_stakptr++] = _tokens[token].data;
				token = _tokens[token].next;
			}
			_lastchar = _stak[_stakptr++] = token & 0xff;
			// put stack in buffer
			while (_stakptr > 0) {
				putByte(_stak[--_stakptr]);
				if (--_szUnpacked == 0) {
					bExit = true;
					continue;
				}
			}
			// put token into record
			if (_curtoken <= _endtoken) {
				_tokens[_curtoken].data = _lastchar;
				_tokens[_curtoken].next = _lastbits;
				_curtoken++;
				if (_curtoken == _endtoken && _numbits != 12) {
					_numbits++;
					_endtoken = (_endtoken << 1) + 1;
				}
			}
			_lastbits = bitstring;
			break;
		}
	}
	return _dwWrote ? 0 : 1;
}

#define PAL_SIZE 1284
#define EXTRA_MAGIC_SIZE 15
#define VIEW_HEADER_COLORS_8BIT 0x80

void DecompressorComp3::decodeRLE(Common::ReadStream *src, Common::WriteStream *dest, byte *pixeldata, uint16 size) {
	int pos = 0;
	byte nextbyte;
	while (pos < size) {
		nextbyte = src->readByte();
		dest->writeByte(nextbyte);
		pos ++;
		switch (nextbyte & 0xC0) {
		case 0x40:
		case 0x00:
			dest->write(pixeldata, nextbyte);
			pixeldata += nextbyte;
			pos += nextbyte;
			break;
		case 0xC0:
			break;
		case 0x80:
			dest->writeByte(*pixeldata++);
			break;
		}
	}
}

void DecompressorComp3::decode_rle(byte **rledata, byte **pixeldata, byte *outbuffer, int size) {
	int pos = 0;
	char nextbyte;
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
int DecompressorComp3::rle_size(byte *rledata, int dsize) {
	int pos = 0;
	char nextbyte;
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

void DecompressorComp3::reorderPic(Common::ReadStream *src, Common::WriteStream *dest, int dsize) {
	int view_size, view_start, cdata_size;
	byte viewdata[7];
	byte *cdata = NULL;
	byte *extra = NULL;

	// Setting palette
	dest->writeByte(PIC_OP_OPX);
	dest->writeByte(PIC_OPX_SET_PALETTE);

	for (int i = 0; i < 256; i++) // Palette translation map
		dest->writeByte(i);
	dest->writeUint32LE(0);  //Palette timestamp

	view_size = src->readUint16LE();
	view_start = src->readUint16LE();
	cdata_size = src->readUint16LE();
	src->read(viewdata, sizeof(viewdata));
	// Copy palette colors
	copyBytes(src, dest, 1024);
	// copy drawing opcodes
	if (view_start != PAL_SIZE + 2)
		copyBytes(src, dest, view_start - PAL_SIZE - 2);
	// storing extra opcodes to be pasted after the cel
	if (dsize != view_start + EXTRA_MAGIC_SIZE + view_size) {
		extra = new byte[dsize - view_size - view_start - EXTRA_MAGIC_SIZE];
		src->read(extra, dsize - view_size - view_start - EXTRA_MAGIC_SIZE);
	}
	// Writing picture cel opcode and header
	dest->writeByte(PIC_OP_OPX);
	dest->writeByte(PIC_OPX_EMBEDDED_VIEW);
	dest->writeByte(0);
	dest->writeUint16LE(0);
	dest->writeUint16LE(view_size + 8);
	dest->write(viewdata, sizeof(viewdata));
	dest->writeByte(0);
	// Unpacking RLE cel data
	cdata = new byte[cdata_size];
	src->read(cdata, cdata_size);
	decodeRLE(src, dest, cdata, view_size);
	// writing stored extra opcodes
	if (extra)
		dest->write(extra, dsize - view_size - view_start - EXTRA_MAGIC_SIZE);
	delete[] extra;
	delete[] cdata;
}

void DecompressorComp3::build_cel_headers(byte **seeker, byte **writer, int celindex, int *cc_lengths, int max) {
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

void DecompressorComp3::view_reorder(byte *inbuffer, byte *outbuffer) {
	byte *cellengths;
	int loopheaders;
	int lh_present;
	int lh_mask;
	int pal_offset;
	int cel_total;
	int unknown;
	byte *seeker = inbuffer;
	char celcounts[100];
	byte *writer = outbuffer;
	byte *lh_ptr;
	byte *rle_ptr, *pix_ptr;
	int l, lb, c, celindex, lh_last = -1;
	int chptr;
	int w;
	int *cc_lengths;
	byte **cc_pos;

	/* Parse the main header */
	cellengths = inbuffer + READ_LE_UINT16(seeker) + 2;
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
			lh_last = writer - outbuffer;
			WRITE_LE_UINT16(lh_ptr, lh_last);
			lh_ptr += 2;
			WRITE_LE_UINT16(writer, celcounts[w]);
			writer += 2;
			WRITE_LE_UINT16(writer, 0);
			writer += 2;

			/* Now, build the cel offset table */
			chptr = (writer - outbuffer) + (2 * celcounts[w]);

			for (c = 0; c < celcounts[w]; c++) {
				WRITE_LE_UINT16(writer, chptr);
				writer += 2;
				cc_pos[celindex+c] = outbuffer + chptr;
				chptr += 8 + READ_LE_UINT16(cellengths + 2 * (celindex + c));
			}

			build_cel_headers(&seeker, &writer, celindex, cc_lengths, celcounts[w]);

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
		pix_ptr += rle_size(pix_ptr, cc_lengths[c]);

	rle_ptr = cellengths + (2 * cel_total);
	for (c = 0; c < cel_total; c++)
		decode_rle(&rle_ptr, &pix_ptr, cc_pos[c] + 8, cc_lengths[c]);

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
// LZW 9-12 bits decompressor for SCI0
//----------------------------------------------
int DecompressorLZW::unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
                            uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);
	byte *buffout = new byte[nUnpacked];
	int error = unpackLZW(buffout);
	dest->write(buffout, nUnpacked);
	delete[] buffout;
	return error;
}

void DecompressorLZW::fetchBits() {
	while (_nBits <= 24) {
		_dwBits |= ((uint32)_src->readByte()) << (_nBits);
		_nBits += 8;
		_dwRead++;
	}
}

uint32 DecompressorLZW::getBits(int n) {
	if (_nBits < n)
		fetchBits();
	uint32 ret = (_dwBits & ~((~0) << n));
	_dwBits >>= n;
	_nBits -= n;
	return ret;
}
int DecompressorLZW::unpackLZW(byte *dest) {
	uint16 bitlen = 9; // no. of bits to read (max. 12)
	uint16 token; // The last received value
	uint16 maxtoken = 0x200; // The biggest token
	uint16 tokenlist[4096]; // pointers to dest[]
	uint16 tokenlengthlist[4096]; // char length of each token
	uint16 tokenctr = 0x102; // no. of registered tokens (starts here)
	uint16 tokenlastlength = 0;
	uint32 destctr = 0;

	while (_dwRead < _szPacked || _nBits) {
		token = getBits(bitlen);

		if (token == 0x101)
			return 0; // terminator
		if (token == 0x100) { // reset command
			maxtoken = 0x200;
			bitlen = 9;
			tokenctr = 0x0102;
		} else {
			{
				int i;
				if (token > 0xff) {
					if (token >= tokenctr) {
#ifdef _SCI_DECOMPRESS_DEBUG
						warning("unpackLZW: Bad token %x", token);
#endif
						// Well this is really bad
						// May be it should throw something like SCI_ERROR_DECOMPRESSION_INSANE
					} else {
						tokenlastlength = tokenlengthlist[token] + 1;
						if (destctr + tokenlastlength > _szUnpacked) {
#ifdef _SCI_DECOMPRESS_DEBUG
							// For me this seems a normal situation, It's necessary to handle it
							warning("unpackLZW: Trying to write beyond the end of array(len=%d, destctr=%d, tok_len=%d)",
							        _szUnpacked, destctr, tokenlastlength);
#endif
							i = 0;
							for (; destctr < _szUnpacked; destctr++) {
								dest[destctr++] = dest [tokenlist[token] + i];
								i++;
							}
						} else
							for (i = 0; i < tokenlastlength; i++) {
								dest[destctr++] = dest[tokenlist[token] + i];
							}
					}
				} else {
					tokenlastlength = 1;
					if (destctr >= _szUnpacked) {
#ifdef _SCI_DECOMPRESS_DEBUG
						warning("unpackLZW: Try to write single byte beyond end of array");
#endif
					} else
						dest[destctr++] = (byte)token;
				}

			}

			if (tokenctr == maxtoken) {
				if (bitlen < 12) {
					bitlen++;
					maxtoken <<= 1;
				} else
					continue; // no further tokens allowed
			}

			tokenlist[tokenctr] = destctr - tokenlastlength;
			tokenlengthlist[tokenctr++] = tokenlastlength;
		}
	}
	return 0;
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

int DecompressorDCL::unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
                            uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);

	byte *buffout = new byte[nUnpacked];
	int error = unpackDCL(buffout);
	dest->write(buffout, nUnpacked);
	delete[] buffout;
	return error;
}

void DecompressorDCL::fetchBits() {
	while (_nBits <= 24) {
		_dwBits |= ((uint32)_src->readByte()) << _nBits;
		_nBits += 8;
		_dwRead++;
	}
}

bool DecompressorDCL::getBit() {
	// fetching more bits to _dwBits buffer
	if (_nBits == 0) 
		fetchBits();
	bool b = _dwBits & 0x1;
	_dwBits >>= 1;
	_nBits--;
	return b;
}

uint32 DecompressorDCL::getBits(int n) {
	// fetching more data to buffer if needed
	if (_nBits < n)
		fetchBits();
	uint32 ret = (_dwBits & ~((~0) << n));
	_dwBits >>= n;
	_nBits -= n;
	return ret;
}

int DecompressorDCL::huffman_lookup(int *tree) {
	int pos = 0;
	int bit;

	while (!(tree[pos] & HUFFMAN_LEAF)) {
		bit = getBit();
		debugC(kDebugLevelDclInflate, "[%d]:%d->", pos, bit);
		pos = bit ? tree[pos] & 0xFFF : tree[pos] >> BRANCH_SHIFT;
	}
	debugC(kDebugLevelDclInflate, "=%02x\n", tree[pos] & 0xffff);
	return tree[pos] & 0xFFFF;
}

#define DCL_ASCII_MODE 1

int DecompressorDCL::unpackDCL(byte* dest) {
	int mode, length_param, value;
	uint32 write_pos = 0, val_distance, val_length;

	mode = getByte();
	length_param = getByte();

	if (mode == DCL_ASCII_MODE) {
		warning("DCL-INFLATE: Decompressing ASCII mode (untested)");
	} else if (mode) {
		warning("DCL-INFLATE: Error: Encountered mode %02x, expected 00 or 01\n", mode);
		return -1;
	}

	if (length_param < 3 || length_param > 6)
		warning("Unexpected length_param value %d (expected in [3,6])\n", length_param);

	while (write_pos < _szUnpacked) {
		if (getBit()) { // (length,distance) pair
			value = huffman_lookup(length_tree);

			if (value < 8)
				val_length = value + 2;
			else
				val_length = 8 + (1 << (value - 7)) + getBits(value - 7);

			debugC(kDebugLevelDclInflate, " | ");

			value = huffman_lookup(distance_tree);

			if (val_length == 2)
				val_distance = (value << 2) | getBits(2);
			else
				val_distance = (value << length_param) | getBits(length_param);
			val_distance ++;

			debugC(kDebugLevelDclInflate, "\nCOPY(%d from %d)\n", val_length, val_distance);

			if (val_length + write_pos > _szUnpacked) {
				warning("DCL-INFLATE Error: Write out of bounds while copying %d bytes", val_length);
				return SCI_ERROR_DECOMPRESSION_OVERFLOW;
			}

			if (write_pos < val_distance) {
				warning("DCL-INFLATE Error: Attempt to copy from before beginning of input stream");
				return SCI_ERROR_DECOMPRESSION_INSANE;
			}

			while (val_length) {
				uint32 copy_length = (val_length > val_distance) ? val_distance : val_length;
				assert(val_distance >= copy_length);
				memcpy(dest + write_pos, dest + write_pos - val_distance, copy_length);

				if (Common::isDebugChannelEnabled(kDebugLevelDclInflate)) {
					for (uint32 i = 0; i < copy_length; i++)
						debugC(kDebugLevelDclInflate, "\33[32;31m%02x\33[37;37m ", dest[write_pos + i]);
					debugC(kDebugLevelDclInflate, "\n");
				}

				val_length -= copy_length;
				val_distance += copy_length;
				write_pos += copy_length;
			}

		} else { // Copy byte verbatim
			value = (mode == DCL_ASCII_MODE) ? huffman_lookup(ascii_tree) : getByte();
			dest[write_pos++] = value;
			debugC(kDebugLevelDclInflate, "\33[32;31m%02x \33[37;37m", value);
		}
	}

	return 0;
}

} // End of namespace Sci
