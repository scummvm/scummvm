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
 * $URL$
 * $Id$
 */

#ifndef COMMON_IFF_CONTAINER_H
#define COMMON_IFF_CONTAINER_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"

namespace Common {

typedef uint32 IFF_ID;

#define ID_FORM     MKID_BE('FORM')
/* EA IFF 85 group identifier */
#define ID_CAT      MKID_BE('CAT ')
/* EA IFF 85 group identifier */
#define ID_LIST     MKID_BE('LIST')
/* EA IFF 85 group identifier */
#define ID_PROP     MKID_BE('PROP')
/* EA IFF 85 group identifier */
#define ID_END      MKID_BE('END ')
/* unofficial END-of-FORM identifier (see Amiga RKM Devices Ed.3
page 376) */
#define ID_ILBM     MKID_BE('ILBM')
/* EA IFF 85 raster bitmap form */
#define ID_DEEP     MKID_BE('DEEP')
/* Chunky pixel image files (Used in TV Paint) */
#define ID_RGB8     MKID_BE('RGB8')
/* RGB image forms, Turbo Silver (Impulse) */
#define ID_RGBN     MKID_BE('RGBN')
/* RGB image forms, Turbo Silver (Impulse) */
#define ID_PBM      MKID_BE('PBM ')
/* 256-color chunky format (DPaint 2 ?) */
#define ID_ACBM     MKID_BE('ACBM')
/* Amiga Contiguous Bitmap (AmigaBasic) */
#define ID_8SVX     MKID_BE('8SVX')
/* Amiga 8 bits voice */

/* generic */

#define ID_FVER     MKID_BE('FVER')
/* AmigaOS version string */
#define ID_JUNK     MKID_BE('JUNK')
/* always ignore this chunk */
#define ID_ANNO     MKID_BE('ANNO')
/* EA IFF 85 Generic Annotation chunk */
#define ID_AUTH     MKID_BE('AUTH')
/* EA IFF 85 Generic Author chunk */
#define ID_CHRS     MKID_BE('CHRS')
/* EA IFF 85 Generic character string chunk */
#define ID_NAME     MKID_BE('NAME')
/* EA IFF 85 Generic Name of art, music, etc. chunk */
#define ID_TEXT     MKID_BE('TEXT')
/* EA IFF 85 Generic unformatted ASCII text chunk */
#define ID_copy     MKID_BE('(c) ')
/* EA IFF 85 Generic Copyright text chunk */

/* ILBM chunks */

#define ID_BMHD     MKID_BE('BMHD')
/* ILBM BitmapHeader */
#define ID_CMAP     MKID_BE('CMAP')
/* ILBM 8bit RGB colormap */
#define ID_GRAB     MKID_BE('GRAB')
/* ILBM "hotspot" coordiantes */
#define ID_DEST     MKID_BE('DEST')
/* ILBM destination image info */
#define ID_SPRT     MKID_BE('SPRT')
/* ILBM sprite identifier */
#define ID_CAMG     MKID_BE('CAMG')
/* Amiga viewportmodes */
#define ID_BODY     MKID_BE('BODY')
/* ILBM image data */
#define ID_CRNG     MKID_BE('CRNG')
/* color cycling */
#define ID_CCRT     MKID_BE('CCRT')
/* color cycling */
#define ID_CLUT     MKID_BE('CLUT')
/* Color Lookup Table chunk */
#define ID_DPI      MKID_BE('DPI ')
/* Dots per inch chunk */
#define ID_DPPV     MKID_BE('DPPV')
/* DPaint perspective chunk (EA) */
#define ID_DRNG     MKID_BE('DRNG')
/* DPaint IV enhanced color cycle chunk (EA) */
#define ID_EPSF     MKID_BE('EPSF')
/* Encapsulated Postscript chunk */
#define ID_CMYK     MKID_BE('CMYK')
/* Cyan, Magenta, Yellow, & Black color map (Soft-Logik) */
#define ID_CNAM     MKID_BE('CNAM')
/* Color naming chunk (Soft-Logik) */
#define ID_PCHG     MKID_BE('PCHG')
/* Line by line palette control information (Sebastiano Vigna) */
#define ID_PRVW     MKID_BE('PRVW')
/* A mini duplicate ILBM used for preview (Gary Bonham) */
#define ID_XBMI     MKID_BE('XBMI')
/* eXtended BitMap Information (Soft-Logik) */
#define ID_CTBL     MKID_BE('CTBL')
/* Newtek Dynamic Ham color chunk */
#define ID_DYCP     MKID_BE('DYCP')
/* Newtek Dynamic Ham chunk */
#define ID_SHAM     MKID_BE('SHAM')
/* Sliced HAM color chunk */
#define ID_ABIT     MKID_BE('ABIT')
/* ACBM body chunk */
#define ID_DCOL     MKID_BE('DCOL')
/* unofficial direct color */
#define ID_DPPS     MKID_BE('DPPS')
/* ? */
#define ID_TINY     MKID_BE('TINY')
/* ? */
#define ID_DPPV     MKID_BE('DPPV')
/* ? */

/* 8SVX chunks */

#define ID_VHDR     MKID_BE('VHDR')
/* 8SVX Voice8Header */


char * ID2string(Common::IFF_ID id);


class IFFChunk : public Common::ReadStream {

protected:
	Common::ReadStream *_input;
	uint32 bytesRead;

public:
	IFF_ID id;
	uint32 size;

	IFFChunk(Common::ReadStream *input): _input(input) {
		size = bytesRead = 0;
	}

	void incBytesRead(uint32 inc) {
		bytesRead += inc;
		if (bytesRead > size) {
			error("Chunk overread");
		}
	}

	void readHeader() {
		id = _input->readUint32BE();
		size = _input->readUint32BE();
		bytesRead = 0;
	}

	bool hasReadAll() const {
		return (size - bytesRead) == 0;
	}

	void feed() {
		if (size % 2) {
			size++;
		}
		while (!hasReadAll()) {
			readByte();
		}
	}

	// Common::ReadStream implementation
	bool eos() const { return _input->eos(); }
	bool err() const { return _input->err(); }
	void clearErr() { _input->clearErr(); }

	uint32 read(void *dataPtr, uint32 dataSize) {
		incBytesRead(dataSize);
		return _input->read(dataPtr, dataSize);
	}

};

class IFFParser {
public:
	IFFParser(Common::ReadStream &input) : _formChunk(&input), _chunk(&input) {
		_formChunk.readHeader();
		if (_formChunk.id != ID_FORM) {
			error("IFFDecoder input is not a FORM type IFF file");
		}
		_typeId = _formChunk.readUint32BE();
	}

	virtual ~IFFParser() {}

	IFFChunk *nextChunk() {
		_chunk.feed();
		_formChunk.incBytesRead(_chunk.size);

		if (_formChunk.hasReadAll())
			return 0;

		_formChunk.incBytesRead(8);
		_chunk.readHeader();

		return &_chunk;
	}

	IFF_ID	_typeId;

protected:
	IFFChunk	_formChunk;
	IFFChunk	_chunk;
};

} // namespace Common

#endif
