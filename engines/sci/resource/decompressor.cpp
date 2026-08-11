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
	uint32 bytesRead = src->read(dest, nPacked);
	return (bytesRead == nPacked) ? SCI_ERROR_NONE : SCI_ERROR_IO_ERROR;
}

void Decompressor::init(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
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

int DecompressorHuffman::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
	init(src, dest, nPacked, nUnpacked);

	byte numnodes = _src->readByte();
	uint16 terminator = _src->readByte() | 0x100;
	_nodes = new byte [numnodes << 1];
	_src->read(_nodes, numnodes << 1);

	int16 c;
	while ((c = getc2()) != terminator && (c >= 0) && !isFinished())
		putByte(c);

	delete[] _nodes;
	return (_dwWrote == _szUnpacked) ? SCI_ERROR_NONE : SCI_ERROR_DECOMPRESSION_ERROR;
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
	int result = SCI_ERROR_NONE;

	switch (_compression) {
	case kCompLZW:	// SCI0 LZW compression
	case kCompLZW1: // SCI01/1 LZW compression
		return unpackLZW(src, dest, nPacked, nUnpacked);
	case kCompLZW1View:
		buffer = new byte[nUnpacked];
		result = unpackLZW(src, buffer, nPacked, nUnpacked);
		if (result == SCI_ERROR_NONE) {
			unpackView(buffer, dest);
		}
		break;
	case kCompLZW1Pic:
		buffer = new byte[nUnpacked];
		result = unpackLZW(src, buffer, nPacked, nUnpacked);
		if (result == SCI_ERROR_NONE) {
			unpackPic(buffer, dest, nUnpacked);
		}
		break;
	default:
		break;
	}
	delete[] buffer;
	return result;
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

	bool terminatorFound = false;
	while (!isFinished()) {
		uint16 code = (_compression == kCompLZW) ?
		              getBitsLSB(codeBitLength) :
		              getBitsMSB(codeBitLength);

		if (code >= tableSize) {
			warning("LZW code %x exceeds table size %x", code, tableSize);
			break;
		}

		if (code == 257) { // terminator
			terminatorFound = true;
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

	// Reaching the compression terminator before writing _szUnpacked bytes is
	// expected. For kCompLZW1View and kCompLZW1Pic, the unpacked size is not
	// the LZW output size, it is the final size after processing LZW output.
	return (terminatorFound || (_dwWrote == _szUnpacked)) ?
		SCI_ERROR_NONE:
		SCI_ERROR_DECOMPRESSION_ERROR;
}

void DecompressorLZW::unpackView(byte *src, byte *dest) {
	byte *seeker = src;
	byte *writer = dest;

	// read compressed header
	byte *celLengths = src + READ_LE_UINT16(seeker) + 2;
	seeker += 2;
	byte loopCount = *seeker++;
	byte loopHeaderCount = *seeker++; // non-mirrored loops
	uint16 mirrorMask = READ_LE_UINT16(seeker);
	seeker += 2;
	uint16 version = READ_LE_UINT16(seeker);
	seeker += 2;
	uint16 paletteOffset = READ_LE_UINT16(seeker); // offset in output view
	seeker += 2;
	int celHeaderCount = READ_LE_UINT16(seeker); // non-mirrored loops
	seeker += 2;

	// read cel counts for each loop header
	byte *celCounts = seeker;
	seeker += loopHeaderCount;

	// read lengths of decoded cels
	uint16 *celDecodedLengths = new uint16[celHeaderCount];
	for (int i = 0; i < celHeaderCount; i++) {
		celDecodedLengths[i] = READ_LE_UINT16(celLengths + (2 * i));
	}

	// locate pixel data by seeking past each cel's RLE data
	byte *rleData = celLengths + (2 * celHeaderCount);
	byte *pixelData = rleData;
	for (int i = 0; i < celHeaderCount; i++) {
		skipRLE(&pixelData, celDecodedLengths[i]);
	}

	// write view header
	*writer++ = loopCount;
	*writer++ = 0x80; // flags for a VGA pic with RLE compression
	WRITE_LE_UINT16(writer, mirrorMask);
	writer += 2;
	WRITE_LE_UINT16(writer, version);
	writer += 2;
	WRITE_LE_UINT16(writer, paletteOffset);
	writer += 2;

	// skip loop offset table, we will write it while writing loops
	byte *loopOffsets = writer;
	writer += 2 * loopCount;

	// write loops and their cels
	int loopTableIndex = 0;
	int celTableIndex = 0;
	for (int loop = 0; loop < loopCount; loop++) {
		if (mirrorMask & (1 << loop)) {
			// mirrored loop: write previous loop offset to table
			memcpy(loopOffsets, loopOffsets - 2, 2);
		} else {
			// write loop offset to table
			WRITE_LE_UINT16(loopOffsets, writer - dest);

			// write loop header
			byte celCount = celCounts[loopTableIndex];
			*writer++ = celCount;
			memset(writer, 0, 3);
			writer += 3;

			// write cel offset table
			int celOffset = (writer - dest) + (2 * celCount);
			for (int cel = 0; cel < celCount; cel++) {
				WRITE_LE_UINT16(writer, celOffset);
				writer += 2;
				celOffset += 8 + celDecodedLengths[celTableIndex + cel];
			}

			// write cels
			for (int cel = 0; cel < celCount; cel++) {
				// cel header
				memcpy(writer, seeker, 7);
				seeker += 7;
				writer += 7;
				*writer++ = 0;

				// cel data
				int celDecodedLength = celDecodedLengths[celTableIndex + cel];
				decodeRLE(&rleData, &pixelData, writer, celDecodedLength);
				writer += celDecodedLength;
			}

			celTableIndex += celCount;
			loopTableIndex++;
		}
		loopOffsets += 2;
	}

	// write palette
	if (paletteOffset != 0) {
		*writer++ = 'P';
		*writer++ = 'A';
		*writer++ = 'L';

		// palette translation map
		for (int i = 0; i < 256; i++) {
			*writer++ = i;
		}

		// palette time stamp and palette colors
		// include the unrelated four bytes before the palette so that
		// they are written to the palette time stamp. this is a bogus
		// value but it's what sierra's decompressor did, so we do it
		// to be consistent with it and other SCI tools. this field is
		// ignored by the interpreter when reading a view.
		memcpy(writer, seeker - 4, 4 + 1024);
	}

	delete[] celDecodedLengths;
}

void DecompressorLZW::unpackPic(byte *src, byte *dest, int unpackedSize) {
	byte *seeker = src;
	byte *writer = dest;

	// begin writing set-palette instruction
	*writer++ = 0xfe; // PIC_OP_OPX
	*writer++ = 0x02; // PIC_OPX_VGA_SET_PALETTE

	// write palette translation map
	for (int i = 0; i < 256; i++) {
		*writer++ = i;
	}

	// write palette time stamp
	memset(writer, 0, 4);
	writer += 4;

	// read compressed header
	uint16 viewCelSize = READ_LE_UINT16(seeker);
	seeker += 2;
	uint16 viewOpPos = READ_LE_UINT16(seeker); // offset in output pic
	seeker += 2;
	uint16 viewPixelDataSize = READ_LE_UINT16(seeker);
	seeker += 2;
	byte *viewHeader = seeker;
	seeker += 7;

	// write palette colors
	memcpy(writer, seeker, 1024);
	seeker += 1024;
	writer += 1024;

	// write optional data before embedded-view instruction
	if (viewOpPos != 1286) { // 1286 = set-palette instruction size
		int preViewDataSize = viewOpPos - 1286;
		memcpy(writer, seeker, preViewDataSize);
		seeker += preViewDataSize;
		writer += preViewDataSize;
	}

	// write optional data after embedded-view instruction (written to end)
	int postViewDataPosition = viewOpPos + 15 + viewCelSize;
	if (unpackedSize != postViewDataPosition) {
		int postViewDataSize = unpackedSize - postViewDataPosition;
		memcpy(dest + postViewDataPosition, seeker, postViewDataSize);
		seeker += postViewDataSize;
	}

	// begin writing embedded-view instruction
	*writer++ = 0xfe; // PIC_OP_OPX
	*writer++ = 0x01; // PIC_OPX_VGA_EMBEDDED_VIEW

	// write coordinates (0, 0)
	memset(writer, 0, 3);
	writer += 3;

	// write cel size
	WRITE_LE_UINT16(writer, 8 + viewCelSize);
	writer += 2;

	// write view header
	memcpy(writer, viewHeader, 7);
	writer += 7;
	*writer++ = 0;

	// write view cel (stored as pixel data followed by RLE data)
	byte *viewRleData = seeker + viewPixelDataSize;;
	decodeRLE(&viewRleData, &seeker, writer, viewCelSize);
}

/***
 * Decodes a cel's data from two separate RLE and pixel buffers.
 */
void DecompressorLZW::decodeRLE(byte **rleData, byte **pixelData, byte *dest, int decodedSize) {
	int pos = 0;
	byte *rleSeeker = *rleData;
	byte *pixelSeeker = *pixelData;

	while (pos < decodedSize) {
		byte currentByte = *rleSeeker++;
		*dest++ = currentByte;
		pos++;

		byte upperBits = (currentByte & 0xc0);
		if (upperBits <= 0x40) { // 00, 40
			memcpy(dest, pixelSeeker, currentByte);
			pixelSeeker += currentByte;
			dest += currentByte;
			pos += currentByte;
		} else if (upperBits == 0x80) {
			*dest++ = *pixelSeeker++;
			pos++;
		}
	}

	*rleData = rleSeeker;
	*pixelData = pixelSeeker;
}

/**
 * Seeks past a cel's RLE data without decoding.
 * Used to seek past all RLE data to locate the start of pixel data.
 */
void DecompressorLZW::skipRLE(byte **rleData, int decodedSize) {
	int pos = 0;
	byte *rleSeeker = *rleData;

	while (pos < decodedSize) {
		byte currentByte = *(rleSeeker++);
		pos++;

		byte upperBits = (currentByte & 0xc0);
		if (upperBits <= 0x40) { // 00, 40
			pos += currentByte;
		} else if (upperBits == 0x80) {
			pos++;
		}
	}

	*rleData = rleSeeker;
}

//----------------------------------------------
// DCL decompressor for SCI1.1
//----------------------------------------------

int DecompressorDCL::unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked) {
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
	return (_dwWrote == _szUnpacked) ? 0 : SCI_ERROR_DECOMPRESSION_ERROR;
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
