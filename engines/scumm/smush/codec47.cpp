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


#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "scumm/bomp.h"
#include "scumm/smush/codec47.h"

namespace Scumm {

#if defined(SCUMM_NEED_ALIGNMENT)

#define COPY_4X1_LINE(dst, src) \
	do {                        \
		(dst)[0] = (src)[0];    \
		(dst)[1] = (src)[1];    \
		(dst)[2] = (src)[2];    \
		(dst)[3] = (src)[3];    \
	} while (0)

#define COPY_2X1_LINE(dst, src) \
	do {                        \
		(dst)[0] = (src)[0];    \
		(dst)[1] = (src)[1];    \
	} while (0)


#else /* SCUMM_NEED_ALIGNMENT */

#define COPY_4X1_LINE(dst, src)               \
	*(uint32 *)(dst) = *(const uint32 *)(src)

#define COPY_2X1_LINE(dst, src)               \
	*(uint16 *)(dst) = *(const uint16 *)(src)

#endif

#define FILL_4X1_LINE(dst, val) \
	do {                        \
		(dst)[0] = val;         \
		(dst)[1] = val;         \
		(dst)[2] = val;         \
		(dst)[3] = val;         \
	} while (0)

#define FILL_2X1_LINE(dst, val) \
	do {                        \
		(dst)[0] = val;         \
		(dst)[1] = val;         \
	} while (0)

#define MOTION_OFFSET_TABLE_SIZE 0xF8
#define PROCESS_SUBBLOCKS        0xFF
#define FILL_SINGLE_COLOR        0xFE
#define DRAW_GLYPH               0xFD
#define COPY_PREV_BUFFER         0xFC

static const  int8 codecGlyph4XVec[] = {
  0, 1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0, 1, 2, 2, 1,
};

static const int8 codecGlyph4YVec[] = {
  0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 1, 1, 2, 2,
};

static const int8 codecGlyph8XVec[] = {
  0, 2, 5, 7, 7, 7, 7, 7, 7, 5, 2, 0, 0, 0, 0, 0,
};

static const int8 codecGlyph8YVec[] = {
  0, 0, 0, 0, 1, 3, 4, 6, 7, 7, 7, 7, 6, 4, 3, 1,
};

static const int8 codecTable[] = {
	  0,   0,  -1, -43,   6, -43,  -9, -42,  13, -41,
	-16, -40,  19, -39, -23, -36,  26, -34,  -2, -33,
	  4, -33, -29, -32,  -9, -32,  11, -31, -16, -29,
	 32, -29,  18, -28, -34, -26, -22, -25,  -1, -25,
	  3, -25,  -7, -24,   8, -24,  24, -23,  36, -23,
	-12, -22,  13, -21, -38, -20,   0, -20, -27, -19,
	 -4, -19,   4, -19, -17, -18,  -8, -17,   8, -17,
	 18, -17,  28, -17,  39, -17, -12, -15,  12, -15,
	-21, -14,  -1, -14,   1, -14, -41, -13,  -5, -13,
	  5, -13,  21, -13, -31, -12, -15, -11,  -8, -11,
	  8, -11,  15, -11,  -2, -10,   1, -10,  31, -10,
	-23,  -9, -11,  -9,  -5,  -9,   4,  -9,  11,  -9,
	 42,  -9,   6,  -8,  24,  -8, -18,  -7,  -7,  -7,
	 -3,  -7,  -1,  -7,   2,  -7,  18,  -7, -43,  -6,
	-13,  -6,  -4,  -6,   4,  -6,   8,  -6, -33,  -5,
	 -9,  -5,  -2,  -5,   0,  -5,   2,  -5,   5,  -5,
	 13,  -5, -25,  -4,  -6,  -4,  -3,  -4,   3,  -4,
	  9,  -4, -19,  -3,  -7,  -3,  -4,  -3,  -2,  -3,
	 -1,  -3,   0,  -3,   1,  -3,   2,  -3,   4,  -3,
	  6,  -3,  33,  -3, -14,  -2, -10,  -2,  -5,  -2,
	 -3,  -2,  -2,  -2,  -1,  -2,   0,  -2,   1,  -2,
	  2,  -2,   3,  -2,   5,  -2,   7,  -2,  14,  -2,
	 19,  -2,  25,  -2,  43,  -2,  -7,  -1,  -3,  -1,
	 -2,  -1,  -1,  -1,   0,  -1,   1,  -1,   2,  -1,
	  3,  -1,  10,  -1,  -5,   0,  -3,   0,  -2,   0,
	 -1,   0,   1,   0,   2,   0,   3,   0,   5,   0,
	  7,   0, -10,   1,  -7,   1,  -3,   1,  -2,   1,
	 -1,   1,   0,   1,   1,   1,   2,   1,   3,   1,
	-43,   2, -25,   2, -19,   2, -14,   2,  -5,   2,
	 -3,   2,  -2,   2,  -1,   2,   0,   2,   1,   2,
	  2,   2,   3,   2,   5,   2,   7,   2,  10,   2,
	 14,   2, -33,   3,  -6,   3,  -4,   3,  -2,   3,
	 -1,   3,   0,   3,   1,   3,   2,   3,   4,   3,
	 19,   3,  -9,   4,  -3,   4,   3,   4,   7,   4,
	 25,   4, -13,   5,  -5,   5,  -2,   5,   0,   5,
	  2,   5,   5,   5,   9,   5,  33,   5,  -8,   6,
	 -4,   6,   4,   6,  13,   6,  43,   6, -18,   7,
	 -2,   7,   0,   7,   2,   7,   7,   7,  18,   7,
	-24,   8,  -6,   8, -42,   9, -11,   9,  -4,   9,
	  5,   9,  11,   9,  23,   9, -31,  10,  -1,  10,
	  2,  10, -15,  11,  -8,  11,   8,  11,  15,  11,
	 31,  12, -21,  13,  -5,  13,   5,  13,  41,  13,
	 -1,  14,   1,  14,  21,  14, -12,  15,  12,  15,
	-39,  17, -28,  17, -18,  17,  -8,  17,   8,  17,
	 17,  18,  -4,  19,   0,  19,   4,  19,  27,  19,
	 38,  20, -13,  21,  12,  22, -36,  23, -24,  23,
	 -8,  24,   7,  24,  -3,  25,   1,  25,  22,  25,
	 34,  26, -18,  28, -32,  29,  16,  29, -11,  31,
	  9,  32,  29,  32,  -4,  33,   2,  33, -26,  34,
	 23,  36, -19,  39,  16,  40, -13,  41,   9,  42,
	 -6,  43,   1,  43,   0,   0,   0,   0,   0,   0
};

enum Edge {
    kEdgeLeft,
    kEdgeTop,
    kEdgeRight,
    kEdgeBottom,
    kEdgeNone,
};

#define NGLYPHS 256

void SmushDeltaGlyphsDecoder::makeTablesInterpolation(int sideLength) {
	int32 pos, npoints;
	int32 edge0, edge1;
	int32 x1, x0, y1, y0;
	int32 tableSmallBig[64], s;
	const int8 *xGlyph = nullptr, *yGlyph = nullptr;
	int32 *ptrSmallBig;
	byte *ptr;
	int i, x, y;

	if (sideLength == 8) {
		xGlyph = codecGlyph8XVec;
		yGlyph = codecGlyph8YVec;
		ptr = _tableBig;
		for (i = 0; i < NGLYPHS; i++) {
			ptr[384] = 0;
			ptr[385] = 0;
			ptr += 388;
		}
	} else if (sideLength == 4) {
		xGlyph = codecGlyph4XVec;
		yGlyph = codecGlyph4YVec;
		ptr = _tableSmall;
		for (i = 0; i < NGLYPHS; i++) {
			ptr[96] = 0;
			ptr[97] = 0;
			ptr += 128;
		}
	} else {
		error("SmushDeltaGlyphsDecoder::makeTablesInterpolation(): ERROR: Unknown sideLength %d.", sideLength);
	}

	s = 0;
	for (x = 0; x < 16; x++) {
		x0 = xGlyph[x];
		y0 = yGlyph[x];

		if (y0 == 0) {
			edge0 = kEdgeBottom;
		} else if (y0 == sideLength - 1) {
			edge0 = kEdgeTop;
		} else if (x0 == 0) {
			edge0 = kEdgeLeft;
		} else if (x0 == sideLength - 1) {
			edge0 = kEdgeRight;
		} else {
			edge0 = kEdgeNone;
		}

		for (y = 0; y < 16; y++) {
			x1 = xGlyph[y];
			y1 = yGlyph[y];

			if (y1 == 0) {
				edge1 = kEdgeBottom;
			} else if (y1 == sideLength - 1) {
				edge1 = kEdgeTop;
			} else if (x1 == 0) {
				edge1 = kEdgeLeft;
			} else if (x1 == sideLength - 1) {
				edge1 = kEdgeRight;
			} else {
				edge1 = kEdgeNone;
			}

			memset(tableSmallBig, 0, sideLength * sideLength * 4);

			npoints = MAX(ABS(y1 - y0), ABS(x1 - x0));

			for (pos = 0; pos <= npoints; pos++) {
				int32 yPoint, xPoint;

				if (npoints > 0) {
					// Linearly interpolate between x0 and x1
					// respectively y0 and y1.
					xPoint = (x0 * pos + x1 * (npoints - pos) + npoints / 2) / npoints;
					yPoint = (y0 * pos + y1 * (npoints - pos) + npoints / 2) / npoints;
				} else {
					xPoint = x0;
					yPoint = y0;
				}
				ptrSmallBig = &tableSmallBig[sideLength * yPoint + xPoint];
				*ptrSmallBig = 1;

				if ((edge0 == kEdgeLeft && edge1 == kEdgeRight) || (edge1 == kEdgeLeft && edge0 == kEdgeRight) ||
				    (edge0 == kEdgeBottom && edge1 != kEdgeTop) || (edge1 == kEdgeBottom && edge0 != kEdgeTop)) {
					if (yPoint >= 0) {
						i = yPoint + 1;
						while (i--) {
							*ptrSmallBig = 1;
							ptrSmallBig -= sideLength;
						}
					}
				} else if ((edge1 != kEdgeBottom && edge0 == kEdgeTop) || (edge0 != kEdgeBottom && edge1 == kEdgeTop)) {
					if (sideLength > yPoint) {
						i = sideLength - yPoint;
						while (i--) {
							*ptrSmallBig = 1;
							ptrSmallBig += sideLength;
						}
					}
				} else if ((edge0 == kEdgeLeft && edge1 != kEdgeRight) || (edge1 == kEdgeLeft && edge0 != kEdgeRight)) {
					if (xPoint >= 0) {
						i = xPoint + 1;
						while (i--) {
							*(ptrSmallBig--) = 1;
						}
					}
				} else if ((edge0 == kEdgeBottom && edge1 == kEdgeTop) || (edge1 == kEdgeBottom && edge0 == kEdgeTop) ||
				           (edge0 == kEdgeRight && edge1 != kEdgeLeft) || (edge1 == kEdgeRight && edge0 != kEdgeLeft)) {
					if (sideLength > xPoint) {
						i = sideLength - xPoint;
						while (i--) {
							*(ptrSmallBig++) = 1;
						}
					}
				}
			}

			if (sideLength == 8) {
				for (i = 64 - 1; i >= 0; i--) {
					if (tableSmallBig[i] != 0) {
						_tableBig[256 + s + _tableBig[384 + s]] = (byte)i;
						_tableBig[384 + s]++;
					} else {
						_tableBig[320 + s + _tableBig[385 + s]] = (byte)i;
						_tableBig[385 + s]++;
					}
				}
				s += 388;
			}
			if (sideLength == 4) {
				for (i = 16 - 1; i >= 0; i--) {
					if (tableSmallBig[i] != 0) {
						_tableSmall[64 + s + _tableSmall[96 + s]] = (byte)i;
						_tableSmall[96 + s]++;
					} else {
						_tableSmall[80 + s + _tableSmall[97 + s]] = (byte)i;
						_tableSmall[97 + s]++;
					}
				}
				s += 128;
			}
		}
	}
}

void SmushDeltaGlyphsDecoder::makeCodecTables(int width) {
	if (_lastTableWidth == width)
		return;

	_lastTableWidth = width;

	int32 a, c, d;
	int16 tmp;

	for (int l = 0; l < ARRAYSIZE(codecTable); l += 2) {
		_table[l / 2] = (int16)(codecTable[l + 1] * width + codecTable[l]);
	}
	// Note: _table[255] is never inited; but since only the first 0xF8
	// entries of it are used anyway, this doesn't matter.

	a = 0;
	c = 0;
	do {
		for (d = 0; d < _tableSmall[96 + c]; d++) {
			tmp = _tableSmall[64 + c + d];
			tmp = (int16)((byte)(tmp >> 2) * width + (tmp & 3));
			_tableSmall[c + d * 2] = (byte)tmp;
			_tableSmall[c + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableSmall[97 + c]; d++) {
			tmp = _tableSmall[80 + c + d];
			tmp = (int16)((byte)(tmp >> 2) * width + (tmp & 3));
			_tableSmall[32 + c + d * 2] = (byte)tmp;
			_tableSmall[32 + c + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableBig[384 + a]; d++) {
			tmp = _tableBig[256 + a + d];
			tmp = (int16)((byte)(tmp >> 3) * width + (tmp & 7));
			_tableBig[a + d * 2] = (byte)tmp;
			_tableBig[a + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableBig[385 + a]; d++) {
			tmp = _tableBig[320 + a + d];
			tmp = (int16)((byte)(tmp >> 3) * width + (tmp & 7));
			_tableBig[128 + a + d * 2] = (byte)tmp;
			_tableBig[128 + a + d * 2 + 1] = tmp >> 8;
		}

		a += 388;
		c += 128;
	} while (c < 32768);
}

#ifdef USE_ARM_SMUSH_ASM

#ifndef IPHONE
#define ARM_Smush_decode2 _ARM_Smush_decode2
#endif

extern "C" void ARM_Smush_decode2(      byte  *dst,
								  const byte  *src,
										int    width,
										int    height,
								  const byte  *param_ptr,
										int16 *_table,
										byte  *_tableBig,
										int32  offset1,
										int32  offset2,
										byte  *_tableSmall);

#define decode2(SRC,DST,WIDTH,HEIGHT,PARAM) \
 ARM_Smush_decode2(SRC,DST,WIDTH,HEIGHT,PARAM,_table,_tableBig, \
				   _offset1,_offset2,_tableSmall)

#else
void SmushDeltaGlyphsDecoder::level3(byte *dDst) {
	int32 tmp;
	byte code = *_dSrc++;

	if (code < MOTION_OFFSET_TABLE_SIZE) {
		tmp = _table[code] + _offset1;
		COPY_2X1_LINE(dDst, dDst + tmp);
		COPY_2X1_LINE(dDst + _dPitch, dDst + _dPitch + tmp);
	} else if (code == PROCESS_SUBBLOCKS) {
		COPY_2X1_LINE(dDst, _dSrc + 0);
		COPY_2X1_LINE(dDst + _dPitch, _dSrc + 2);
		_dSrc += 4;
	} else if (code == FILL_SINGLE_COLOR) {
		byte t = *_dSrc++;
		FILL_2X1_LINE(dDst, t);
		FILL_2X1_LINE(dDst + _dPitch, t);
	} else if (code == COPY_PREV_BUFFER) {
		tmp = _offset2;
		COPY_2X1_LINE(dDst, dDst + tmp);
		COPY_2X1_LINE(dDst + _dPitch, dDst + _dPitch + tmp);
	} else {
		byte t = _paramPtr[code];
		FILL_2X1_LINE(dDst, t);
		FILL_2X1_LINE(dDst + _dPitch, t);
	}
}

void SmushDeltaGlyphsDecoder::level2(byte *d_dst) {
	int32 tmp;
	byte code = *_dSrc++;
	int i;

	if (code < MOTION_OFFSET_TABLE_SIZE) {
		tmp = _table[code] + _offset1;
		for (i = 0; i < 4; i++) {
			COPY_4X1_LINE(d_dst, d_dst + tmp);
			d_dst += _dPitch;
		}
	} else if (code == PROCESS_SUBBLOCKS) {
		level3(d_dst);
		d_dst += 2;
		level3(d_dst);
		d_dst += _dPitch * 2 - 2;
		level3(d_dst);
		d_dst += 2;
		level3(d_dst);
	} else if (code == FILL_SINGLE_COLOR) {
		byte t = *_dSrc++;
		for (i = 0; i < 4; i++) {
			FILL_4X1_LINE(d_dst, t);
			d_dst += _dPitch;
		}
	} else if (code == DRAW_GLYPH) {
		byte *tmpPtr = _tableSmall + *_dSrc++ * 128;
		int32 l = tmpPtr[96];
		byte val = *_dSrc++;
		int16 *tmpPtr2 = (int16 *)tmpPtr;
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmpPtr2)) = val;
			tmpPtr2++;
		}
		l = tmpPtr[97];
		val = *_dSrc++;
		tmpPtr2 = (int16 *)(tmpPtr + 32);
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmpPtr2)) = val;
			tmpPtr2++;
		}
	} else if (code == COPY_PREV_BUFFER) {
		tmp = _offset2;
		for (i = 0; i < 4; i++) {
			COPY_4X1_LINE(d_dst, d_dst + tmp);
			d_dst += _dPitch;
		}
	} else {
		byte t = _paramPtr[code];
		for (i = 0; i < 4; i++) {
			FILL_4X1_LINE(d_dst, t);
			d_dst += _dPitch;
		}
	}
}

void SmushDeltaGlyphsDecoder::level1(byte *d_dst) {
	int32 tmp;
	byte code = *_dSrc++;
	int i;

	if (code < MOTION_OFFSET_TABLE_SIZE) {
		tmp = _table[code] + _offset1;
		for (i = 0; i < 8; i++) {
			COPY_4X1_LINE(d_dst + 0, d_dst + tmp);
			COPY_4X1_LINE(d_dst + 4, d_dst + tmp + 4);
			d_dst += _dPitch;
		}
	} else if (code == PROCESS_SUBBLOCKS) {
		level2(d_dst);
		d_dst += 4;
		level2(d_dst);
		d_dst += _dPitch * 4 - 4;
		level2(d_dst);
		d_dst += 4;
		level2(d_dst);
	} else if (code == FILL_SINGLE_COLOR) {
		byte t = *_dSrc++;
		for (i = 0; i < 8; i++) {
			FILL_4X1_LINE(d_dst, t);
			FILL_4X1_LINE(d_dst + 4, t);
			d_dst += _dPitch;
		}
	} else if (code == DRAW_GLYPH) {
		tmp = *_dSrc++;
		byte *tmpPtr = _tableBig + tmp * 388;
		byte l = tmpPtr[384];
		byte val = *_dSrc++;
		int16 *tmpPtr2 = (int16 *)tmpPtr;
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmpPtr2)) = val;
			tmpPtr2++;
		}
		l = tmpPtr[385];
		val = *_dSrc++;
		tmpPtr2 = (int16 *)(tmpPtr + 128);
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmpPtr2)) = val;
			tmpPtr2++;
		}
	} else if (code == COPY_PREV_BUFFER) {
		tmp = _offset2;
		for (i = 0; i < 8; i++) {
			COPY_4X1_LINE(d_dst + 0, d_dst + tmp);
			COPY_4X1_LINE(d_dst + 4, d_dst + tmp + 4);
			d_dst += _dPitch;
		}
	} else {
		byte t = _paramPtr[code];
		for (i = 0; i < 8; i++) {
			FILL_4X1_LINE(d_dst, t);
			FILL_4X1_LINE(d_dst + 4, t);
			d_dst += _dPitch;
		}
	}
}

void SmushDeltaGlyphsDecoder::decode2(byte *dst, const byte *src, int width, int height, const byte *param_ptr) {
	_dSrc = src;
	_paramPtr = param_ptr - MOTION_OFFSET_TABLE_SIZE;
	int bw = (width + 7) / 8;
	int bh = (height + 7) / 8;
	int nextLine = width * 7;
	_dPitch = width;

	do {
		int tmpBw = bw;
		do {
			level1(dst);
			dst += 8;
		} while (--tmpBw);
		dst += nextLine;
	} while (--bh);
}
#endif

SmushDeltaGlyphsDecoder::SmushDeltaGlyphsDecoder(int width, int height) {
	_lastTableWidth = -1;
	_width = width;
	_height = height;
	_tableBig = (byte *)malloc(NGLYPHS * 388);
	_tableSmall = (byte *)malloc(NGLYPHS * 128);
	if ((_tableBig != nullptr) && (_tableSmall != nullptr)) {
		makeTablesInterpolation(4);
		makeTablesInterpolation(8);
	}

	_frameSize = _width * _height;
	_deltaSize = _frameSize * 3;
	_deltaBuf = (byte *)malloc(_deltaSize);
	_deltaBufs[0] = _deltaBuf;
	_deltaBufs[1] = _deltaBuf + _frameSize;
	_curBuf = _deltaBuf + _frameSize * 2;
}

SmushDeltaGlyphsDecoder::~SmushDeltaGlyphsDecoder() {
	if (_tableBig) {
		free(_tableBig);
		_tableBig = nullptr;
	}
	if (_tableSmall) {
		free(_tableSmall);
		_tableSmall = nullptr;
	}
	_lastTableWidth = -1;
	if (_deltaBuf) {
		free(_deltaBuf);
		_deltaSize = 0;
		_deltaBuf = nullptr;
		_deltaBufs[0] = nullptr;
		_deltaBufs[1] = nullptr;
	}
}

bool SmushDeltaGlyphsDecoder::decode(byte *dst, const byte *src) {
	if ((_tableBig == nullptr) || (_tableSmall == nullptr) || (_deltaBuf == nullptr))
		return false;

	_offset1 = _deltaBufs[1] - _curBuf;
	_offset2 = _deltaBufs[0] - _curBuf;

	int32 seqNb = READ_LE_UINT16(src + 0);

	const byte *gfxData = src + 26;

	if (seqNb == 0) {
		makeCodecTables(_width);
		memset(_deltaBufs[0], src[12], _frameSize);
		memset(_deltaBufs[1], src[13], _frameSize);
		_prevSeqNb = -1;
	}

	if ((src[4] & 1) != 0) {
		gfxData += 32896;
	}

	switch (src[2]) {
	case 0:
		memcpy(_curBuf, gfxData, _frameSize);
		break;
	case 1:
		// Used by Outlaws, but not by any SCUMM game.
		error("SmushDeltaGlyphsDecoder::decode(): ERROR: Case 1 not implemented (used by Outlaws).");
		break;
	case 2:
		if (seqNb == _prevSeqNb + 1) {
			decode2(_curBuf, gfxData, _width, _height, src + 8);
		}
		break;
	case 3:
		memcpy(_curBuf, _deltaBufs[1], _frameSize);
		break;
	case 4:
		memcpy(_curBuf, _deltaBufs[0], _frameSize);
		break;
	case 5:
		bompDecodeLine(_curBuf, gfxData, READ_LE_UINT32(src + 14));
		break;
	default:
		break;
	}

	memcpy(dst, _curBuf, _frameSize);

	if (seqNb == _prevSeqNb + 1) {
		if (src[3] == 1) {
			SWAP(_curBuf, _deltaBufs[1]);
		} else if (src[3] == 2) {
			SWAP(_deltaBufs[0], _deltaBufs[1]);
			SWAP(_deltaBufs[1], _curBuf);
		}
	}
	_prevSeqNb = seqNb;

	return true;
}

} // End of namespace Scumm
