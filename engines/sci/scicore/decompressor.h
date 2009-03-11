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

#ifndef SCI_SCICORE_DECOMPRESSOR_H
#define SCI_SCICORE_DECOMPRESSOR_H

#include "common/file.h"

namespace Sci {
enum ResourceCompression {
	kCompUnknown = -1,
	kCompNone = 0,
	kCompLZW,
	kCompHuffman,
	kComp3,			// LZW-like compression used in SCI01 and SCI1 
	kComp3View,		// Comp3 + view Post-processing
	kComp3Pic,		// Comp3 + pic Post-processing
	kCompDCL,
	kCompSTACpack	// ? Used in SCI32
};
//----------------------------------------------
// Base class for decompressors
// Simply copies nPacked bytes from src to dest 
//----------------------------------------------
class Decompressor {
public:
	Decompressor(){}
	virtual ~Decompressor(){}

	//! get a number of bits from _src stream
	/** @param n - number of bits to get
		@return (uint32) n-bits number
	  */
	virtual int unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
			uint32 nUnpacked);

protected:
	//! Initialize decompressor
	/** @param src - source stream to read from
		@param dest - destination stream to write to
		@param nPacked - size of packed data
		@param nUnpacket - size of unpacked data
		@return (int) 0 on success, non-zero on error
	  */
	virtual void init(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
			uint32 nUnpacked);	//! get one bit from _src stream
	/** @return (bool) bit;
	  */
	virtual bool getBit();
	//! get a number of bits from _src stream
	/** @param n - number of bits to get
		@return (uint32) n-bits number
	  */
	virtual uint32 getBits(int n);
	//! put byte to _dest stream
	/** @param b - byte to put
	  */
	virtual void putByte(byte b);
	virtual void fetchBits();
	int copyBytes(Common::ReadStream *src, Common::WriteStream *dest, uint32 nSize);

	uint32 _dwBits;
	byte _nBits;

	uint32 _szPacked;
	uint32 _szUnpacked;
	uint32 _dwRead;
	uint32 _dwWrote;

	Common::ReadStream *_src;
	Common::WriteStream *_dest;
};

//----------------------------------------------
// Huffman decompressor
//----------------------------------------------
class DecompressorHuffman : public Decompressor {
public:
	int unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
			uint32 nUnpacked);

protected:
	int16 getc2();

	byte *_nodes;
};

//----------------------------------------------
// LZW-like decompressor for SCI01/SCI1
// TODO: Needs clean-up of post-processing fncs
//----------------------------------------------
class DecompressorComp3 : public Decompressor {
public:
	DecompressorComp3(int nCompression) {
		_compression = nCompression;
	}
	void init(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked, uint32 nUnpacked);
	int unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
			uint32 nUnpacked);

protected:
	enum {
		PIC_OPX_EMBEDDED_VIEW = 1,
		PIC_OPX_SET_PALETTE = 2,
		PIC_OP_OPX = 0xfe,
	};
	// actual unpacking procedure
	int doUnpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
			uint32 nUnpacked);
	// functions to post-process view and pic resources 
	void decodeRLE(Common::ReadStream *src, Common::WriteStream *dest, byte *pixeldata, uint16 size);
	void reorderPic(Common::ReadStream *src, Common::WriteStream *dest, int dsize);
	// 	
	void decode_rle(byte **rledata, byte **pixeldata, byte *outbuffer, int size);
	int rle_size(byte *rledata, int dsize);
	void build_cel_headers(byte **seeker, byte **writer, int celindex, int *cc_lengths, int max);
	void view_reorder(byte *inbuffer, byte *outbuffer);
	// decompressor data
	struct tokenlist {
		byte data;
		uint16 next;
		} _tokens[0x1004];
	byte _stak[0x1014];
	byte _lastchar;
	uint16 _stakptr;
	uint16 _numbits, _lastbits;
	uint16 _curtoken, _endtoken;
	int _compression;
};

//----------------------------------------------
// LZW 9-12 bits decompressor for SCI0
// TODO : Needs clean-up of doUnpack()
//----------------------------------------------
class DecompressorLZW : public Decompressor {
public:
//	void init(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked, uint32 nUnpacked);
	int unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
			uint32 nUnpacked);

protected:
	int doUnpack(byte *src, byte *dest, int length, int complength);

};

//----------------------------------------------
// DCL decompressor for SCI1.1
// TODO : Needs clean-up of doUnpack()
//----------------------------------------------
class DecompressorDCL : public Decompressor {
public:
//	void init(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked, uint32 nUnpacked);
	int unpack(Common::ReadStream *src, Common::WriteStream *dest, uint32 nPacked,
			uint32 nUnpacked);

protected:
	int unpackDCL(byte *src, byte *dest, int length, int complength);
	int getbits(struct bit_read_struct *inp, int bits);
	int huffman_lookup(struct bit_read_struct *inp, int *tree);

};

} // End of namespace Sci

#endif // SCI_SCICORE_DECOMPRESSOR_H

