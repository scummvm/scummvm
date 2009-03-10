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

int Decompressor::unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
			uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);
	byte buff[1024];
	uint32 chunk;
	while (_szPacked && !_src->ioFailed() && !_dest->ioFailed()) {
		chunk = MIN<uint32>(1024, _szPacked);
		_src->read(buff, chunk);
		_dest->write(buff, chunk);
		_szPacked -= chunk;
	}
	return _src->ioFailed() || _dest->ioFailed() ? 1 : 0;
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
		_dwBits |= ((uint32)_src->readByte()) << (24-_nBits);
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
	if(_nBits < n)
		fetchBits();
	uint32 ret = _dwBits >> (32-n);
	_dwBits <<= n;
	_nBits -= n;
	return ret;
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
				return getBits(8) | 0x100;
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

	switch (_compression) {
	case kComp3: // Comp3 compression
		return doUnpack(src, dest, nPacked, nUnpacked);
		break;
	case kComp3View:	
	case kComp3Pic:	
		buffer = new byte[nUnpacked];
		buffer2 = new byte[nUnpacked];
		pBuff = new Common::MemoryWriteStream(buffer, nUnpacked);
		doUnpack(src, pBuff, nPacked, nUnpacked);
		if (_compression == kComp3View)
			view_reorder(buffer, buffer2);
		else
			pic_reorder(buffer, buffer2, nUnpacked);
		dest->write(buffer2, nUnpacked);
		delete[] buffer2;
		delete[] buffer;
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

enum {
	PIC_OP_SET_COLOR = 0xf0,
	PIC_OP_DISABLE_VISUAL = 0xf1,
	PIC_OP_SET_PRIORITY = 0xf2,
	PIC_OP_DISABLE_PRIORITY = 0xf3,
	PIC_OP_SHORT_PATTERNS = 0xf4,
	PIC_OP_MEDIUM_LINES = 0xf5,
	PIC_OP_LONG_LINES = 0xf6,
	PIC_OP_SHORT_LINES = 0xf7,
	PIC_OP_FILL = 0xf8,
	PIC_OP_SET_PATTERN = 0xf9,
	PIC_OP_ABSOLUTE_PATTERN = 0xfa,
	PIC_OP_SET_CONTROL = 0xfb,
	PIC_OP_DISABLE_CONTROL = 0xfc,
	PIC_OP_MEDIUM_PATTERNS = 0xfd,
	PIC_OP_OPX = 0xfe,
	PIC_OP_TERMINATE = 0xff
};

enum {
	PIC_OPX_SET_PALETTE_ENTRIES = 0,
	PIC_OPX_EMBEDDED_VIEW = 1,
	PIC_OPX_SET_PALETTE = 2,
	PIC_OPX_PRIORITY_TABLE_EQDIST = 3,
	PIC_OPX_PRIORITY_TABLE_EXPLICIT = 4
};

#define PAL_SIZE 1284
#define CEL_HEADER_SIZE 7
#define EXTRA_MAGIC_SIZE 15

void DecompressorComp3::decode_rle(byte **rledata, byte **pixeldata, byte *outbuffer, int size) {
	int pos = 0;
	char nextbyte;
	byte *rd = *rledata;
	byte *ob = outbuffer;
	byte *pd = *pixeldata;

	while (pos < size) {
		nextbyte = *(rd++);
		*(ob++) = nextbyte;
		pos++;
		switch (nextbyte&0xC0) {
		case 0x40 :
		case 0x00 :
			memcpy(ob, pd, nextbyte);
			pd += nextbyte;
			ob += nextbyte;
			pos += nextbyte;
			break;
		case 0xC0 :
			break;
		case 0x80 :
			nextbyte = *(pd++);
			*(ob++) = nextbyte;
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
		case 0x40 :
		case 0x00 :
			pos += nextbyte;
			break;
		case 0xC0 :
			break;
		case 0x80 :
			pos++;
			break;
		}
	}

	return size;
}

void DecompressorComp3::pic_reorder(byte *inbuffer, byte *outbuffer, int dsize) {
	int view_size;
	int view_start;
	int cdata_size;
	int i;
	byte *seeker = inbuffer;
	byte *writer;
	char viewdata[CEL_HEADER_SIZE];
	byte *cdata, *cdata_start;
	
	writer = outbuffer;

	*writer++ = PIC_OP_OPX;
	*writer++ = PIC_OPX_SET_PALETTE;

	for (i = 0; i < 256; i++) /* Palette translation map */
		*writer++ = i;

	WRITE_LE_UINT16(writer, 0); /* Palette stamp */
	writer += 2;
	WRITE_LE_UINT16(writer, 0);
	writer += 2;

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

	if (dsize != view_start+EXTRA_MAGIC_SIZE+view_size) {
		memcpy(outbuffer+view_size+view_start+EXTRA_MAGIC_SIZE, seeker, 
		       dsize-view_size-view_start-EXTRA_MAGIC_SIZE);
		seeker += dsize-view_size-view_start-EXTRA_MAGIC_SIZE;
	}

	cdata_start = cdata = (byte *)malloc(cdata_size);
	memcpy(cdata, seeker, cdata_size);
	seeker += cdata_size;
	
	writer = outbuffer + view_start;
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
	
	decode_rle(&seeker, &cdata, writer, view_size);
	
	free(cdata_start);
}

#define VIEW_HEADER_COLORS_8BIT 0x80

void DecompressorComp3::build_cel_headers(byte **seeker, byte **writer, int celindex, int *cc_lengths, int max) {
	for (int c = 0; c < max; c++) {
		memcpy(*writer, *seeker, 6);
		*seeker += 6; *writer += 6;
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
	byte *rle_ptr,*pix_ptr;
	int l, lb, c, celindex, lh_last = -1;
	int chptr;
	int w;
	int *cc_lengths;
	byte **cc_pos;
	
	/* Parse the main header */
	cellengths = inbuffer+READ_LE_UINT16(seeker)+2;
	seeker += 2;
	loopheaders = *(seeker++);
	lh_present = *(seeker++);
	lh_mask = READ_LE_UINT16(seeker);
	seeker += 2;
	unknown = READ_LE_UINT16(seeker);
	seeker += 2;
	pal_offset = READ_LE_UINT16(seeker);
	seeker += 2;
	cel_total = READ_LE_UINT16(seeker);
	seeker += 2;

	cc_pos = (byte **) malloc(sizeof(byte *)*cel_total);
	cc_lengths = (int *) malloc(sizeof(int)*cel_total);
	
	for (c = 0; c < cel_total; c++)
		cc_lengths[c] = READ_LE_UINT16(cellengths+2*c);
	
	*writer++ = loopheaders;
	*writer++ = VIEW_HEADER_COLORS_8BIT;
	WRITE_LE_UINT16(writer, lh_mask);
	writer += 2;
	WRITE_LE_UINT16(writer, unknown);
	writer += 2;
	WRITE_LE_UINT16(writer, pal_offset);
	writer += 2;

	lh_ptr = writer;
	writer += 2*loopheaders; /* Make room for the loop offset table */

	pix_ptr = writer;
	
	memcpy(celcounts, seeker, lh_present);
	seeker += lh_present;

	lb = 1;
	celindex = 0;

	rle_ptr = pix_ptr = cellengths + (2*cel_total);
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
			lh_last = writer-outbuffer;
			WRITE_LE_UINT16(lh_ptr, lh_last);
			lh_ptr += 2;
			WRITE_LE_UINT16(writer, celcounts[w]);
			writer += 2;
			WRITE_LE_UINT16(writer, 0);
			writer += 2;

			/* Now, build the cel offset table */
			chptr = (writer - outbuffer) + (2*celcounts[w]);

			for (c = 0; c < celcounts[w]; c++) {
				WRITE_LE_UINT16(writer, chptr);
				writer += 2;
				cc_pos[celindex+c] = outbuffer + chptr;
				chptr += 8 + READ_LE_UINT16(cellengths+2*(celindex+c));
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

	rle_ptr = cellengths + (2*cel_total);
	for (c = 0; c < cel_total; c++)
		decode_rle(&rle_ptr, &pix_ptr, cc_pos[c]+8, cc_lengths[c]);

	*writer++ = 'P';
	*writer++ = 'A';
	*writer++ = 'L';
	
	for (c = 0; c < 256; c++)
		*writer++ = c;

	seeker -= 4; /* The missing four. Don't ask why. */
	memcpy(writer, seeker, 4*256+4);
	
	free(cc_pos);
	free(cc_lengths);
}

//----------------------------------------------
// LZW 9-12 bits decompressor for SCI0
//----------------------------------------------
int DecompressorLZW::unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
	uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);
	byte *buffin = new byte[nPacked];
	byte *buffout = new byte[nUnpacked];
	src->read(buffin, nPacked);
	
	doUnpack(buffin, buffout, nUnpacked, nPacked);
	
	dest->write(buffout, nUnpacked);
	delete[] buffin;
	delete[] buffout;
	return 0;
}

int DecompressorLZW::doUnpack(byte *src, byte *dest, int length, int complength) {
	uint16 bitlen = 9; // no. of bits to read (max. 12)
	uint16 bitmask = 0x01ff;
	uint16 bitctr = 0; // current bit position
	uint16 bytectr = 0; // current byte position
	uint16 token; // The last received value
	uint16 maxtoken = 0x200; // The biggest token

	uint16 tokenlist[4096]; // pointers to dest[]
	uint16 tokenlengthlist[4096]; // char length of each token
	uint16 tokenctr = 0x102; // no. of registered tokens (starts here)

	uint16 tokenlastlength = 0;

	uint16 destctr = 0;

	while (bytectr < complength) {

		uint32 tokenmaker = src[bytectr++] >> bitctr;
		if (bytectr < complength)
			tokenmaker |= (src[bytectr] << (8 - bitctr));
		if (bytectr + 1 < complength)
			tokenmaker |= (src[bytectr+1] << (16 - bitctr));

		token = tokenmaker & bitmask;

		bitctr += bitlen - 8;

		while (bitctr >= 8) {
			bitctr -= 8;
			bytectr++;
		}

		if (token == 0x101)
			return 0; // terminator
		if (token == 0x100) { // reset command
			maxtoken = 0x200;
			bitlen = 9;
			bitmask = 0x01ff;
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
						if (destctr + tokenlastlength > length) {
#ifdef _SCI_DECOMPRESS_DEBUG
							// For me this seems a normal situation, It's necessary to handle it
							warning("unpackLZW: Trying to write beyond the end of array(len=%d, destctr=%d, tok_len=%d)",
							       length, destctr, tokenlastlength);
#endif
							i = 0;
							for (; destctr < length; destctr++) {
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
					if (destctr >= length) {
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
					bitmask <<= 1;
					bitmask |= 1;
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

struct bit_read_struct {
	int length;
	int bitpos;
	int bytepos;
	byte *data;
};

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

#define CALLC(x) { if ((x) == -SCI_ERROR_DECOMPRESSION_OVERFLOW) return -SCI_ERROR_DECOMPRESSION_OVERFLOW; }

int DecompressorDCL::unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
	uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);
	byte *buffin = new byte[nPacked];
	byte *buffout = new byte[nUnpacked];
	src->read(buffin, nPacked);
	
	unpackDCL(buffin, buffout, nUnpacked, nPacked);
	
	dest->write(buffout, nUnpacked);
	delete[] buffin;
	delete[] buffout;
	return 0;
}

int DecompressorDCL::getbits(struct bit_read_struct *inp, int bits) {
	int morebytes = (bits + inp->bitpos - 1) >> 3;
	int result = 0;
	int i;

	if (inp->bytepos + morebytes >= inp->length) {
		warning("read out-of-bounds with bytepos %d + morebytes %d >= length %d",
		        inp->bytepos, morebytes, inp->length);
		return -7;
	}

	for (i = 0; i <= morebytes; i++)
		result |= (inp->data[inp->bytepos + i]) << (i << 3);

	result >>= inp->bitpos;
	result &= ~((~0) << bits);

	inp->bitpos += bits - (morebytes << 3);
	inp->bytepos += morebytes;

	debugC(kDebugLevelDclInflate, "(%d:%04x)", bits, result);

	return result;
}

int DecompressorDCL::huffman_lookup(struct bit_read_struct *inp, int *tree) {
	int pos = 0;
	int bit;

	while (!(tree[pos] & HUFFMAN_LEAF)) {
		CALLC(bit = getbits(inp, 1));
		debugC(kDebugLevelDclInflate, "[%d]:%d->", pos, bit);
		if (bit)
			pos = tree[pos] & ~(~0 << BRANCH_SHIFT);
		else
			pos = tree[pos] >> BRANCH_SHIFT;
	}
	debugC(kDebugLevelDclInflate, "=%02x\n", tree[pos] & 0xffff);
	return tree[pos] & 0xffff;
}

#define VALUE_M(i) ((i == 0)? 7 : (VALUE_M(i - 1) + 2**i));

#define DCL_ASCII_MODE 1

int DecompressorDCL::unpackDCL(uint8* src, uint8* dest, int length, int complength) {
	int mode, length_param, value, val_length, val_distance;
	int write_pos = 0;
	struct bit_read_struct reader;

	reader.length = complength;
	reader.bitpos = 0;
	reader.bytepos = 0;
	reader.data = src;

	CALLC(mode = getbits(&reader, 8));
	CALLC(length_param = getbits(&reader, 8));

	if (mode == DCL_ASCII_MODE) {
		warning("DCL-INFLATE: Decompressing ASCII mode (untested)");
	} else if (mode) {
		warning("DCL-INFLATE: Error: Encountered mode %02x, expected 00 or 01\n", mode);
		return -1;
	}

	if (Common::isDebugChannelEnabled(kDebugLevelDclInflate)) {
		for (int i = 0; i < reader.length; i++) {
			debugC(kDebugLevelDclInflate, "%02x ", reader.data[i]);
			if (!((i + 1) & 0x1f))
				debugC(kDebugLevelDclInflate, "\n");
		}


		debugC(kDebugLevelDclInflate, "\n---\n");
	}


	if (length_param < 3 || length_param > 6)
		warning("Unexpected length_param value %d (expected in [3,6])\n", length_param);

	while (write_pos < length) {
		CALLC(value = getbits(&reader, 1));

		if (value) { // (length,distance) pair
			CALLC(value = huffman_lookup(&reader, length_tree));

			if (value < 8)
				val_length = value + 2;
			else {
				int length_bonus;

				val_length = (1 << (value - 7)) + 8;
				CALLC(length_bonus = getbits(&reader, value - 7));
				val_length += length_bonus;
			}

			debugC(kDebugLevelDclInflate, " | ");

			CALLC(value = huffman_lookup(&reader, distance_tree));

			if (val_length == 2) {
				val_distance = value << 2;

				CALLC(value = getbits(&reader, 2));
				val_distance |= value;
			} else {
				val_distance = value << length_param;

				CALLC(value = getbits(&reader, length_param));
				val_distance |= value;
			}
			++val_distance;

			debugC(kDebugLevelDclInflate, "\nCOPY(%d from %d)\n", val_length, val_distance);

			if (val_length + write_pos > length) {
				warning("DCL-INFLATE Error: Write out of bounds while copying %d bytes", val_length);
				return SCI_ERROR_DECOMPRESSION_OVERFLOW;
			}

			if (write_pos < val_distance) {
				warning("DCL-INFLATE Error: Attempt to copy from before beginning of input stream");
				return SCI_ERROR_DECOMPRESSION_INSANE;
			}

			while (val_length) {
				int copy_length = (val_length > val_distance) ? val_distance : val_length;

				memcpy(dest + write_pos, dest + write_pos - val_distance, copy_length);

				if (Common::isDebugChannelEnabled(kDebugLevelDclInflate)) {
					for (int i = 0; i < copy_length; i++)
						debugC(kDebugLevelDclInflate, "\33[32;31m%02x\33[37;37m ", dest[write_pos + i]);
					debugC(kDebugLevelDclInflate, "\n");
				}

				val_length -= copy_length;
				val_distance += copy_length;
				write_pos += copy_length;
			}

		} else { // Copy byte verbatim
			if (mode == DCL_ASCII_MODE) {
				CALLC(value = huffman_lookup(&reader, ascii_tree));
			} else {
				CALLC(value = getbits(&reader, 8));
			}

			dest[write_pos++] = value;

			debugC(kDebugLevelDclInflate, "\33[32;31m%02x \33[37;37m", value);
		}
	}

	return 0;
}

} // End of namespace Sci
