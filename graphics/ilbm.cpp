/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "graphics/ilbm.h"

namespace Graphics {

static char * ID2string(IFF_ID id) {
	static char str[] = "abcd";

	str[0] = (char)(id >> 24 & 0xff);
	str[1] = (char)(id >> 16 & 0xff);
	str[2] = (char)(id >>  8 & 0xff);
	str[3] = (char)(id >>  0 & 0xff);

	return str;
}

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

void IFFDecoder::readBMHD() {

	_bitmapHeader.width = _chunk.readUint16();
	_bitmapHeader.height = _chunk.readUint16();
	_bitmapHeader.x = _chunk.readUint16();
	_bitmapHeader.y = _chunk.readUint16();

	_bitmapHeader.depth = _chunk.readByte();
	_bitmapHeader.masking = _chunk.readByte();
	_bitmapHeader.pack = _chunk.readByte();
	_bitmapHeader.flags = _chunk.readByte();
	_bitmapHeader.transparentColor = _chunk.readUint16();
	_bitmapHeader.xAspect = _chunk.readByte();
	_bitmapHeader.yAspect = _chunk.readByte();
	_bitmapHeader.pageWidth = _chunk.readUint16();
	_bitmapHeader.pageHeight = _chunk.readUint16();


	_colorCount = 1 << _bitmapHeader.depth;
	*_colors = (byte*)malloc(sizeof(**_colors) * _colorCount * 3);
	_surface->create(_bitmapHeader.width, _bitmapHeader.height, 1);

}

void IFFDecoder::readCRNG() {
}

void IFFDecoder::readCMAP() {
	if (*_colors == NULL) {
		error("wrong input chunk sequence");
	}
	for (uint32 i = 0; i < _colorCount; i++) {
		(*_colors)[i * 3 + 0] = _chunk.readByte();
		(*_colors)[i * 3 + 1] = _chunk.readByte();
		(*_colors)[i * 3 + 2] = _chunk.readByte();
	}
}

IFFDecoder::IFFDecoder(Common::ReadStream &input) : _formChunk(&input), _chunk(&input), _colorCount(0) {
	_formChunk.readHeader();
	if (_formChunk.id != ID_FORM) {
		error("IFFDecoder input is not a FORM type IFF file");
	}
}

void IFFDecoder::decode(Surface &surface, byte *&colors) {
	_surface = &surface;
	_colors = &colors;
	*_colors = 0;

	if (!isTypeSupported(_formChunk.readUint32())) {
		error( "IFFDecoder input is not a valid subtype");
	}

	while (!_formChunk.eos()) {
		_formChunk.incBytesRead(8);
		_chunk.readHeader();

		switch (_chunk.id) {
		case ID_BMHD:
			readBMHD();
			break;

		case ID_CMAP:
			readCMAP();
			break;

		case ID_BODY:
			readBODY();
			break;

		case ID_CRNG:
			readCRNG();
			break;

		case ID_GRAB: case ID_TINY: case ID_DPPS: case ID_DPPV: case ID_CAMG:
			break;

		default:
			error("unknown chunk : %s\n", ID2string(_chunk.id));
		}

		_chunk.feed();
		_formChunk.incBytesRead(_chunk.size);
	}
}

bool PBMDecoder::isTypeSupported(IFF_ID type) {
	return type == ID_PBM;
}

void PBMDecoder::readBODY() {
	byte byteRun;
	byte idx;
	uint32 si = 0, i, j;

	if (_bitmapHeader.depth > 8) {
		error("PBMDecoder depth > 8");
	}

	if ((_bitmapHeader.pack != 0) && (_bitmapHeader.pack != 1)) {
		error("PBMDecoder unsupported pack");
	}

	switch (_bitmapHeader.pack) {
	case 0:
		while (!_chunk.eos()) {
			idx = _chunk.readByte();
			((byte*)_surface->pixels)[si++] = idx;
		}
		break;
	case 1:
		while (!_chunk.eos()) {
			byteRun = _chunk.readByte();
			if (byteRun <= 127) {
				i = byteRun + 1;
				for (j = 0; j < i; j++){
					idx = _chunk.readByte();
					((byte*)_surface->pixels)[si++] = idx;
				}
			} else if (byteRun != 128) {
				i = (256 - byteRun) + 1;
				idx = _chunk.readByte();
				for (j = 0; j < i; j++) {
					((byte*)_surface->pixels)[si++] = idx;
				}
			}
		}
		break;
	}

}


bool ILBMDecoder::isTypeSupported(IFF_ID type) {
	return type == ID_ILBM;
}

void ILBMDecoder::expandLine(byte *buf, uint32 width) {

	byte byteRun;
	byte idx;

	uint32 si = 0, i, j;

	while (si != width) {
		byteRun = _chunk.readByte();
		if (byteRun <= 127) {
			i = byteRun + 1;
			for (j = 0; j < i; j++){
				idx = _chunk.readByte();
				buf[si++] = idx;
			}
		} else if (byteRun != 128) {
			i = (256 - byteRun) + 1;
			idx = _chunk.readByte();
			for (j = 0; j < i; j++) {
				buf[si++] = idx;
			}
		}
	}

}

void ILBMDecoder::fillPlane(byte *out, byte* buf, uint32 width, uint32 plane) {

	byte src, idx, set;
	byte mask = 1 << plane;

	for (uint32 j = 0; j < _bitmapHeader.width; j++) {
		src = buf[j >> 3];
		idx = 7 - (j & 7);
		set = src & (1 << idx);

		if (set)
			out[j] |= mask;
	}

}

void ILBMDecoder::readBODY() {

	if (_bitmapHeader.depth > 8) {
		error("ILBMDecoder depth > 8");
	}

	if (_bitmapHeader.pack != 1) {
		error("ILBMDecoder unsupported pack");
	}

	if (_bitmapHeader.masking == 1) {
		error("ILBMDecoder mask not supported");
	}

	uint32 scanWidth = _bitmapHeader.width >> 3;
	byte *scan = (byte*)malloc(scanWidth);
	byte *out = (byte*)_surface->pixels;

	switch (_bitmapHeader.pack) {
//	case 0:
//		while (!_chunk.eos()) {
//			idx = _chunk.readByte();
//			((byte*)_surface->pixels)[si++] = idx;
//		}
//		break;
	case 1:
		for (uint32 line = 0; line < _bitmapHeader.height; line++) {

			for (uint32 plane = 0; plane < _bitmapHeader.depth; plane++) {
				expandLine(scan, scanWidth);
				fillPlane(out, scan, scanWidth, plane);
			}

			out += _bitmapHeader.width;
		}
		break;
	}

	free(scan);

}

void ILBMDecoder::readCRNG() {
	// TODO: implement this. May require changing decode(), too, or adding
	// another parameter to ILBMDecoder constructor
}

ILBMDecoder::ILBMDecoder(Common::ReadStream &input) : IFFDecoder(input) {

}

ILBMDecoder::~ILBMDecoder() {

}



void decodeILBM(Common::ReadStream &input, Surface &surface, byte *&colors) {
	IFF_ID typeId;
	BMHD bitmapHeader;
	Chunk formChunk(&input);
	Chunk chunk(&input);
	uint32 colorCount = 0, i, j, si;
	byte byteRun;
	byte idx;
	colors = 0;
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

		switch (chunk.id) {
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
				}
				break;
			case 1:
				while (!chunk.eos()) {
					byteRun = chunk.readByte();
					if (byteRun <= 127) {
						i = byteRun + 1;
						for (j = 0; j < i; j++){
							idx = chunk.readByte();
							((byte*)surface.pixels)[si++] = idx;
						}
					} else if (byteRun != 128) {
						i = (256 - byteRun) + 1;
						idx = chunk.readByte();
						for (j = 0; j < i; j++) {
							((byte*)surface.pixels)[si++] = idx;
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
		formChunk.incBytesRead(chunk.size);
	}
}

}	// End of namespace Graphics

