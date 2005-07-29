/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/stream.h"
#include "common/file.h"
#include "graphics/surface.h"

namespace Graphics {

typedef uint32 IFF_ID;

struct Chunk {
	IFF_ID id;
	uint32 size;
	uint32 bytesRead;
	Common::ReadStream *_input;

	Chunk(Common::ReadStream *input): _input(input) {
		size = bytesRead = 0;
	}

	void incBytesRead(uint32 inc) {
		bytesRead += inc;
		if (bytesRead > size) {
			error("Chunk overead");
		}
	}

	void readHeader() {
		id = _input->readUint32BE();
		size = _input->readUint32BE();
		if (size % 2) {
			size++;
		}
		bytesRead = 0;
	}

	bool eos() {
		return (size - bytesRead) == 0;
	}

	void feed() {		
		while(!eos()) {
			readByte();
		}
	}

	byte readByte() {
		incBytesRead(1);
		return _input->readByte();
	}

	int8 readSByte() {
		incBytesRead(1);
		return _input->readSByte();
	}

	uint16 readUint16() {
		incBytesRead(2);
		return _input->readUint16BE();
	}

	uint32 readUint32() {
		incBytesRead(4);
		return _input->readUint32BE();
	}

	int16 readSint16() {
		return (int16)readUint16();
	}

	int32 readSint32() {
		return (int32)readUint32();
	}	
};

static char * ID2string(IFF_ID id) {
	static char str[] = "abcd";

	str[0] = (char)(id >> 24 & 0xff);
	str[1] = (char)(id >> 16 & 0xff);
	str[2] = (char)(id >>  8 & 0xff);
	str[3] = (char)(id >>  0 & 0xff);

	return str;
}


struct BMHD {
	uint16 width, height;
	uint16 x, y;
	byte depth;
	byte masking;
	byte pack;
	byte flags;
	uint16 transparentColor;
	byte xAspect, yAspect;
	uint16 pageWidth, pageHeight;
	BMHD() {
		memset(this, 0, sizeof(*this));
	}
};

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
#define ID_TEXT     MKID_BE('TEXT'))     
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


void decodeILBM(Common::ReadStream &input, Surface &surface, byte *&colors) {
	IFF_ID typeId;
	BMHD bitmapHeader;
	Chunk formChunk(&input);
	Chunk chunk(&input);
	uint32 colorCount = 0, i, j, si;
	int8 byteRun;
	byte idx;
	colors = NULL;
	si = 0;

	formChunk.readHeader();
	if (formChunk.id != ID_FORM) {
		error("decodeILBM() input is not a FORM type IFF file");
	}

	typeId = formChunk.readUint32();
	if (typeId != ID_PBM) {
		error( "decodeILBM() input is not an PBM ");
	}

	while (!formChunk.eos()) {
		formChunk.incBytesRead(8);
		chunk.readHeader();
		formChunk.incBytesRead(chunk.size);

		switch(chunk.id) {
		case ID_BMHD:
			bitmapHeader.width = chunk.readUint16();
			bitmapHeader.height = chunk.readUint16();
			bitmapHeader.x = chunk.readUint16();
			bitmapHeader.y = chunk.readUint16();

			bitmapHeader.depth = chunk.readByte();
			if (bitmapHeader.depth > 8) {
				error("decodeILBM() depth > 8");
			}
			bitmapHeader.masking = chunk.readByte();			
			bitmapHeader.pack = chunk.readByte();
			if ((bitmapHeader.pack != 0) && (bitmapHeader.pack != 1)) {
				error("decodeILBM() unsupported pack");
			}
			bitmapHeader.flags = chunk.readByte();
			bitmapHeader.transparentColor = chunk.readUint16();
			bitmapHeader.xAspect = chunk.readByte();
			bitmapHeader.yAspect = chunk.readByte();
			bitmapHeader.pageWidth = chunk.readUint16();
			bitmapHeader.pageHeight = chunk.readUint16();


			colorCount = 1 << bitmapHeader.depth;
			colors = (byte*)malloc(sizeof(*colors) * colorCount * 3);
			surface.create(bitmapHeader.width, bitmapHeader.height, 1);
			break;
		case ID_CMAP:
			if (colors == NULL) {
				error("wrong input chunk sequence");
			}
			for (i = 0; i < colorCount; i++) {
				colors[i * 3 + 0] = chunk.readByte();
				colors[i * 3 + 1] = chunk.readByte();
				colors[i * 3 + 2] = chunk.readByte();
			}
			break;

		case ID_BODY:
			switch (bitmapHeader.pack) {
			case 0:
				while (!chunk.eos()) {
					idx = chunk.readByte();
					((byte*)surface.pixels)[si++] = idx;
					/*colorMap[idx];
					colorMap[idx];
					colorMap[idx];*/
				}
				break;
			case 1:
				while (!chunk.eos()) {
					byteRun = chunk.readSByte();
					if (byteRun >= 0) {
						i = byteRun + 1;
						for (j = 0; j < i; j++){
							idx = chunk.readByte();
							((byte*)surface.pixels)[si++] = idx;
							/*colorMap[idx];
							colorMap[idx];
							colorMap[idx];*/
						}
					} else if (byteRun == -128) {
					// nop
					} else {
						i = (-byteRun) + 1;
						idx = chunk.readByte();
						for (j = 0; j < i; j++) {
							((byte*)surface.pixels)[si++] = idx;
							/*colorMap[idx];
							colorMap[idx];
							colorMap[idx];*/
						}					
					}
				}
				break;
			}
			break;
		case ID_GRAB: case ID_CRNG: case ID_TINY: case ID_DPPS:
			break;
		default:
			error("unknown chunk : %s\n", ID2string(chunk.id));
		}

		chunk.feed();	
	}
}

}	// End of namespace Graphics
