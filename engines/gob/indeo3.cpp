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

/* Intel Indeo 3 decompressor, derived from ffmpeg.
 *
 * Original copyright note: * Intel Indeo 3 (IV31, IV32, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#include "common/system.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/frac.h"
#include "common/file.h"

#include "gob/indeo3.h"
#include "gob/indeo3data.h"

namespace Gob {

#define SQR(x) ((x) * (x))
PaletteLUT::PaletteLUT(byte depth, PaletteFormat format) {
	assert((depth > 1) && (depth < 9));

	_depth1 = depth;
	_depth2 = 2 * _depth1;
	_shift = 8 - _depth1;

	_dim1 = (1 << _depth1);
	_dim2 = _dim1 * _dim1;
	_dim3 = _dim1 * _dim1 * _dim1;

	_format = format;

	_got = _dim1;
	_lut = new byte[_dim3];
	_gots = new byte[_dim1];

	memset(_lutPal, 0, 768);
	memset(_realPal, 0, 768);
	memset(_gots, 1, _dim1);
}

void PaletteLUT::setPalette(const byte *palette, PaletteFormat format, byte depth) {
	assert((depth > 1) && (depth < 9));

	warning("Building new palette LUT");

	int shift = 8 - depth;

	if ((_format == kPaletteRGB) && (format == kPaletteYUV)) {
		byte *newPal = _realPal;
		const byte *oldPal = palette;
		for (int i = 0; i < 256; i++, newPal += 3, oldPal += 3)
			YUV2RGB(oldPal[0] << shift, oldPal[1] << shift, oldPal[2] << shift,
					newPal[0], newPal[1], newPal[2]);
	} else if ((_format == kPaletteYUV) && (format == kPaletteRGB)) {
		byte *newPal = _realPal;
		const byte *oldPal = palette;
		for (int i = 0; i < 256; i++, newPal += 3, oldPal += 3)
			RGB2YUV(oldPal[0] << shift, oldPal[1] << shift, oldPal[2] << shift,
					newPal[0], newPal[1], newPal[2]);
	} else
		memcpy(_realPal, palette, 768);

	byte *newPal = _lutPal, *oldPal = _realPal;
	for (int i = 0; i < 768; i++)
		*newPal++ = (*oldPal++) >> _shift;

	_got = 0;
	memset(_gots, 0, _dim1);
}

PaletteLUT::~PaletteLUT() {
	delete[] _lut;
	delete[] _gots;
}

void PaletteLUT::buildNext() {
	if (_got >= _dim1)
		return;

	build(_got++);
}

void PaletteLUT::build(int d1) {
	byte *lut = _lut + d1 * _dim2;

	warning("LUT %d/%d", d1, _dim1 - 1);

	for (int j = 0; j < _dim1; j++) {
		for (int k = 0; k < _dim1; k++) {
			const byte *p = _lutPal;
			uint32 d = 0xFFFFFFFF;
			byte n = 0;

			for (int c = 0; c < 256; c++, p += 3) {
				uint32 di = SQR(d1 - p[0]) + SQR(j - p[1]) + SQR(k - p[2]);
				if (di < d) {
					d = di;
					n = c;
					if (d == 0)
						break;
				}
			}

			*lut++ = n;
		}
	}

	_gots[d1] = 1;
}

inline int PaletteLUT::getIndex(byte c1, byte c2, byte c3) const {
	return ((c1 >> _shift) << _depth2) | ((c2 >> _shift) << _depth1) | (c3 >> _shift);
}

byte PaletteLUT::findNearest(byte c1, byte c2, byte c3) {
	return _lut[getIndex(c1, c2, c3)];
}

byte PaletteLUT::findNearest(byte c1, byte c2, byte c3, byte &nC1, byte &nC2, byte &nC3) {
	if (!_gots[c1 >> _shift])
		build(c1 >> _shift);

	int palIndex = _lut[getIndex(c1, c2, c3)];
	int i = palIndex * 3;

	nC1 = _realPal[i + 0];
	nC2 = _realPal[i + 1];
	nC3 = _realPal[i + 2];

	return palIndex;
}

SierraLite::SierraLite(int16 width, int16 height, PaletteLUT *palLUT) {
	assert((width > 0) && (height > 0));

	_width = width;
	_height = height;
	_palLUT = palLUT;

	_errorBuf = new int32[3 * (2 * (_width + 2*1))];
	memset(_errorBuf, 0, (3 * (2 * (_width + 2*1))) * sizeof(int32));

	_curLine = 0;
	_errors[0] = _errorBuf + 3;
	_errors[1] = _errors[0] + 3 * (_width + 2*1);
}

SierraLite::~SierraLite() {
	delete[] _errorBuf;
}

void SierraLite::newFrame() {
	_curLine = 0;
	memset(_errors[0], 0, 3 * _width * sizeof(int32));
	memset(_errors[1], 0, 3 * _width * sizeof(int32));
}

void SierraLite::nextLine() {
	// Clear the finished line, it will become the last line in the buffer
	memset(_errors[_curLine], 0, 3 * _width * sizeof(int32));

	_curLine = (_curLine + 1) % 2;
}

byte SierraLite::dither(byte c1, byte c2, byte c3, uint32 x) {
	assert(_palLUT);

	int32 eC1, eC2, eC3;

	getErrors(x, eC1, eC2, eC3);

	// Apply error on values
	c1 = CLIP<int>(c1 + eC1, 0, 255);
	c2 = CLIP<int>(c2 + eC2, 0, 255);
	c3 = CLIP<int>(c3 + eC3, 0, 255);

	// Find color
	byte newC1, newC2, newC3;
	byte newPixel = _palLUT->findNearest(c1, c2, c3, newC1, newC2, newC3);

	// Calculate new error
	eC1 = c1 - newC1;
	eC2 = c2 - newC2;
	eC3 = c3 - newC3;

	// Add them
	addErrors(x, eC1, eC2, eC3);

	return newPixel;
}

inline void SierraLite::getErrors(uint32 x, int32 &eC1, int32 &eC2, int32 &eC3) {
	int32 *errCur = _errors[_curLine];

	x *= 3;
	eC1 = errCur[x + 0] >> 2;
	eC2 = errCur[x + 1] >> 2;
	eC3 = errCur[x + 2] >> 2;
}

inline void SierraLite::addErrors(uint32 x, int32 eC1, int32 eC2, int32 eC3) {
	int32 *errCur  = _errors[_curLine];
	int32 *errNext = _errors[(_curLine + 1) % 2];

	// Indices for current error
	int x0 = 3 * (x + 1);
	int x1 = 3 * (x + 0);
	int x2 = 3 * (x - 1);

	errCur [x0 + 0] += eC1 << 1;
	errCur [x0 + 1] += eC2 << 1;
	errCur [x0 + 2] += eC3 << 1;
	errNext[x1 + 0] += eC1;
	errNext[x1 + 1] += eC2;
	errNext[x1 + 2] += eC3;
	errNext[x2 + 0] += eC1;
	errNext[x2 + 1] += eC2;
	errNext[x2 + 2] += eC3;
}

Indeo3::Indeo3(int16 width, int16 height, PaletteLUT *palLUT) {
	assert((width > 0) && (height > 0));

	_width = width;
	_height = height;
	_palLUT = palLUT;

	_ditherSL = 0;
	setDither(kDitherSierraLite);

	buildModPred();
	allocFrames();
}

Indeo3::~Indeo3() {
	delete[] _iv_frame[0].the_buf;
	delete[] _ModPred;
	delete[] _corrector_type;
	delete _ditherSL;
}

bool Indeo3::isIndeo3(byte *data, uint32 dataLen) {
	// No data, no Indeo 3
	if (!data)
		return false;

	// Less than 16 bytes? This can't be right
	if (dataLen < 16)
		return false;

	// Unknown, but according to the docs, always 0
	if (READ_LE_UINT32(data + 4) != 0)
		return false;

	uint32 id;
	id  = READ_LE_UINT32(data     ); // frame number
	id ^= READ_LE_UINT32(data +  4); // unknown
	id ^= READ_LE_UINT32(data +  8); // checksum
	id ^= READ_LE_UINT32(data + 12); // frame data length

	// These 4 uint32s XOR'd need to spell "FRMH"
	if (id != MKID_BE('FRMH'))
		return false;

	return true;
}

void Indeo3::setDither(DitherAlgorithm dither) {
	delete _ditherSL;
	_ditherSL = 0;

	_dither = dither;

	switch(dither) {
	case kDitherSierraLite:
		_ditherSL = new SierraLite(_width, _height, _palLUT);
		break;

	default:
		return;
	}
}

void Indeo3::buildModPred() {
	_ModPred = new byte[8 * 128];

	for (int i = 0; i < 128; i++) {
		_ModPred[i+0*128] = (i > 126) ? 254 : 2*((i + 1) - ((i + 1) % 2));
		_ModPred[i+1*128] = (i == 7)  ?  20 : ((i == 119 || i == 120)
		                              ? 236 : 2*((i + 2) - ((i + 1) % 3)));
		_ModPred[i+2*128] = (i > 125) ? 248 : 2*((i + 2) - ((i + 2) % 4));
		_ModPred[i+3*128] =                   2*((i + 1) - ((i - 3) % 5));
		_ModPred[i+4*128] = (i == 8)  ?  20 : 2*((i + 1) - ((i - 3) % 6));
		_ModPred[i+5*128] =                   2*((i + 4) - ((i + 3) % 7));
		_ModPred[i+6*128] = (i > 123) ? 240 : 2*((i + 4) - ((i + 4) % 8));
		_ModPred[i+7*128] =                   2*((i + 5) - ((i + 4) % 9));
	}

	_corrector_type = new uint16[24 * 256];

	for (int i = 0; i < 24; i++) {
		for (int j = 0; j < 256; j++) {
			_corrector_type[i*256+j] =
				 (j < _corrector_type_0[i])         ? 1 :
			  ((j < 248 || (i == 16 && j == 248)) ? 0 :
			  _corrector_type_2[j - 248]);
		}
	}
}

void Indeo3::allocFrames() {
	int32 luma_width   = (_width  + 3) & (~3);
	int32 luma_height  = (_height + 3) & (~3);

	int32 chroma_width  = ((luma_width >> 2) + 3) & (~3);
	int32 chroma_height = ((luma_height>> 2) + 3) & (~3);

	int32 luma_pixels = luma_width * luma_height;
	int32 chroma_pixels = chroma_width * chroma_height;

	uint32 bufsize = luma_pixels * 2 + luma_width * 3 +
		(chroma_pixels + chroma_width) * 4;

	_iv_frame[0].y_w  = _iv_frame[1].y_w  = luma_width;
	_iv_frame[0].y_h  = _iv_frame[1].y_h  = luma_height;
	_iv_frame[0].uv_w = _iv_frame[1].uv_w = chroma_width;
	_iv_frame[0].uv_h = _iv_frame[1].uv_h = chroma_height;

	_iv_frame[0].the_buf_size = bufsize;
	_iv_frame[1].the_buf_size = 0;

	_iv_frame[0].the_buf = new byte[bufsize];
	memset(_iv_frame[0].the_buf, 0, bufsize);
	_iv_frame[1].the_buf = 0;

	uint32 offs = 0;

	_iv_frame[0].Ybuf = _iv_frame[0].the_buf + luma_width;
	offs += luma_pixels + luma_width * 2;
	_iv_frame[1].Ybuf = _iv_frame[0].the_buf + offs;
	offs += (luma_pixels + luma_width);
	_iv_frame[0].Ubuf = _iv_frame[0].the_buf + offs;
	offs += (chroma_pixels + chroma_width);
	_iv_frame[1].Ubuf = _iv_frame[0].the_buf + offs;
	offs += (chroma_pixels + chroma_width);
	_iv_frame[0].Vbuf = _iv_frame[0].the_buf + offs;
	offs += (chroma_pixels + chroma_width);
	_iv_frame[1].Vbuf = _iv_frame[0].the_buf + offs;

	for(int i = 1; i <= luma_width; i++)
		_iv_frame[0].Ybuf[-i] = _iv_frame[1].Ybuf[-i] =
			_iv_frame[0].Ubuf[-i] = 0x80;

	for(int i = 1; i <= chroma_width; i++) {
		_iv_frame[1].Ubuf[-i] = 0x80;
		_iv_frame[0].Vbuf[-i] = 0x80;
		_iv_frame[1].Vbuf[-i] = 0x80;
		_iv_frame[1].Vbuf[chroma_pixels+i-1] = 0x80;
	}
}

bool Indeo3::decompressFrame(byte *inData, uint32 dataLen,
		byte *outData, uint16 width, uint16 height) {

	// Not Indeo 3? Fail
	if (!isIndeo3(inData, dataLen))
		return false;

	assert(outData);
	assert(_palLUT);

	uint32 frameDataLen = READ_LE_UINT32(inData + 12);

	// Less data than the frame should have? Fail
	if (dataLen < (frameDataLen - 16))
		return false;

	Common::MemoryReadStream frame(inData, dataLen);

	frame.skip(16); // Header
	frame.skip(2);  // Unknown

	uint16 flags1 = frame.readUint16LE();
	uint32 flags3 = frame.readUint32LE();
	uint8  flags2 = frame.readByte();

	// Finding the reference frame
	if(flags1 & 0x200) {
		_cur_frame = _iv_frame + 1;
		_ref_frame = _iv_frame;
	} else {
		_cur_frame = _iv_frame;
		_ref_frame = _iv_frame + 1;
	}

	if (flags3 == 0x80)
		return true;

	frame.skip(3);

	uint16 fHeight = frame.readUint16LE();
	uint16 fWidth  = frame.readUint16LE();

	uint32 chromaHeight = ((fHeight >> 2) + 3) & 0x7FFC;
	uint32 chromaWidth  = ((fWidth  >> 2) + 3) & 0x7FFC;

	uint32 offs;
	uint32 offsY = frame.readUint32LE() + 16;
	uint32 offsU = frame.readUint32LE() + 16;
	uint32 offsV = frame.readUint32LE() + 16;

	frame.skip(4);

	uint32 hPos = frame.pos();

	byte *hdr_pos = inData + hPos;
	byte *buf_pos;

	// Luminance Y
	frame.seek(offsY);
	buf_pos = inData + offsY + 4;
	offs = frame.readUint32LE();
	decodeChunk(_cur_frame->Ybuf, _ref_frame->Ybuf, fWidth, fHeight,
			buf_pos + offs * 2, flags2, hdr_pos, buf_pos, MIN<int>(fWidth, 160));

	// Chrominance U
	frame.seek(offsU);
	buf_pos = inData + offsU + 4;
	offs = frame.readUint32LE();
	decodeChunk(_cur_frame->Vbuf, _ref_frame->Vbuf, chromaWidth, chromaHeight,
			buf_pos + offs * 2, flags2, hdr_pos, buf_pos, MIN<int>(chromaWidth, 40));

	// Chrominance V
	frame.seek(offsV);
	buf_pos = inData + offsV + 4;
	offs = frame.readUint32LE();
	decodeChunk(_cur_frame->Ubuf, _ref_frame->Ubuf, chromaWidth, chromaHeight,
			buf_pos + offs * 2, flags2, hdr_pos, buf_pos, MIN<int>(chromaWidth, 40));

	BlitState blitState;

	blitState.widthY        = _cur_frame->y_w;
	blitState.widthUV       = _cur_frame->uv_w;
	blitState.uwidthUV      = chromaWidth;
	blitState.uwidthOut     = fWidth;
	blitState.heightY       = _cur_frame->y_h;
	blitState.heightUV      = _cur_frame->uv_h;
	blitState.uheightUV     = chromaHeight;
	blitState.uheightOut    = fHeight;
	blitState.scaleWYUV     = blitState.widthY  / blitState.widthUV;
	blitState.scaleHYUV     = blitState.heightY / blitState.heightUV;
	blitState.scaleWYOut    = blitState.widthY  / blitState.uwidthOut;
	blitState.scaleHYOut    = blitState.heightY / blitState.uheightOut;
	blitState.lineWidthOut  = blitState.scaleWYOut * blitState.uwidthOut;
	blitState.lineHeightOut = blitState.scaleHYOut * blitState.uheightOut;
	blitState.bufY          = _cur_frame->Ybuf;
	blitState.bufU          = _cur_frame->Ubuf;
	blitState.bufV          = _cur_frame->Vbuf;
	blitState.bufOut        = outData;

	blitFrame(blitState);

	return true;
}

void Indeo3::blitFrame(BlitState &s) {
	if (_ditherSL)
		_ditherSL->newFrame();

	for (s.curY = 0; s.curY < s.uheightOut; s.curY++) {
		if (_dither == kDitherNone)
			blitLine(s);
		else
			blitLineDither(s);
	}
}

void Indeo3::blitLine(BlitState &s) {
	byte *lineU = s.bufU + (s.curY >> 2) * s.uwidthUV;
	byte *lineV = s.bufV + (s.curY >> 2) * s.uwidthUV;

	for (s.curX = 0; s.curX < s.uwidthOut; s.curX++) {
		byte dataY = *s.bufY++;
		byte dataU = lineU[s.curX >> 2];
		byte dataV = lineV[s.curX >> 2];

		for (int n = 0; n < s.scaleWYOut; n++)
			*s.bufOut++ = _palLUT->findNearest(dataY, dataU, dataV);
	}

	byte *lineDest = s.bufOut - s.lineWidthOut;
	for (int n = 1; n < s.scaleHYOut; n++) {
		memcpy(s.bufOut, lineDest, s.lineWidthOut);
		s.bufOut += s.lineWidthOut;
	}
}

void Indeo3::blitLineDither(BlitState &s) {
	byte *lineU = s.bufU + (s.curY >> 2) * s.uwidthUV;
	byte *lineV = s.bufV + (s.curY >> 2) * s.uwidthUV;

	for (uint16 i = 0; i < s.scaleHYOut; i++) {
		byte *bufY = s.bufY;

		for (s.curX = 0; s.curX < s.uwidthOut; s.curX++) {
			byte dataY = *bufY++;
			byte dataU = lineU[s.curX >> 2];
			byte dataV = lineV[s.curX >> 2];

			for (int n = 0; n < s.scaleWYOut; n++)
				*s.bufOut++ = _ditherSL->dither(dataY, dataU, dataV, s.curX * s.scaleWYOut + n);

		}

		_ditherSL->nextLine();
	}

	s.bufY += s.uwidthOut;
}

typedef struct {
	int32 xpos;
	int32 ypos;
	int32 width;
	int32 height;
	int32 split_flag;
	int32 split_direction;
	int32 usl7;
} ustr_t;

/* ---------------------------------------------------------------------- */

#define LV1_CHECK(buf1,rle_v3,lv1,lp2)  \
	if((lv1 & 0x80) != 0) {   \
		if(rle_v3 != 0)         \
			rle_v3 = 0;           \
		else {                  \
			rle_v3 = 1;           \
			buf1 -= 2;            \
		}                       \
	}                         \
	lp2 = 4;


#define RLE_V3_CHECK(buf1,rle_v1,rle_v2,rle_v3)  \
	if(rle_v3 == 0) {         \
		rle_v2 = *buf1;         \
		rle_v1 = 1;             \
		if(rle_v2 > 32) {       \
			rle_v2 -= 32;         \
			rle_v1 = 0;           \
		}                       \
		rle_v3 = 1;             \
	}                         \
	buf1--;


#define LP2_CHECK(buf1,rle_v3,lp2)  \
	if(lp2 == 0 && rle_v3 != 0)     \
		rle_v3 = 0;           \
	else {                  \
		buf1--;               \
		rle_v3 = 1;           \
	}


#define RLE_V2_CHECK(buf1,rle_v2, rle_v3,lp2) \
	rle_v2--;             \
	if(rle_v2 == 0) {     \
		rle_v3 = 0;         \
		buf1 += 2;          \
	}                     \
	lp2 = 4;

void Indeo3::decodeChunk(byte *cur, byte *ref, int width, int height,
		const byte *buf1, uint32 fflags2, const byte *hdr,
		const byte *buf2, int min_width_160) {

	byte bit_buf;
	uint32 bit_pos, lv, lv1, lv2;
	int32 *width_tbl, width_tbl_arr[10];
	const int8 *ref_vectors;
	byte *cur_frm_pos, *ref_frm_pos, *cp, *cp2;
	uint32 *cur_lp, *ref_lp;
	const uint32 *correction_lp[2], *correctionloworder_lp[2], *correctionhighorder_lp[2];
	uint16 *correction_type_sp[2];
	ustr_t strip_tbl[20], *strip;
	int i, j, k, lp1, lp2, flag1, cmd;
	int blks_width, blks_height, region_160_width;
	int rle_v1, rle_v2, rle_v3;
	uint16 res;

	bit_buf = 0;
	ref_vectors = NULL;

	width_tbl = width_tbl_arr + 1;
	i = (width < 0 ? width + 3 : width)/4;
	for(j = -1; j < 8; j++)
		width_tbl[j] = i * j;

	strip = strip_tbl;

	for(region_160_width = 0; region_160_width < (width - min_width_160); region_160_width += min_width_160);

	strip->ypos = strip->xpos = 0;
	for(strip->width = min_width_160; width > strip->width; strip->width *= 2);
	strip->height = height;
	strip->split_direction = 0;
	strip->split_flag = 0;
	strip->usl7 = 0;

	bit_pos = 0;

	rle_v1 = rle_v2 = rle_v3 = 0;

	while(strip >= strip_tbl) {
		if(bit_pos <= 0) {
			bit_pos = 8;
			bit_buf = *buf1++;
		}

		bit_pos -= 2;
		cmd = (bit_buf >> bit_pos) & 0x03;

		if(cmd == 0) {
			strip++;
			memcpy(strip, strip-1, sizeof(ustr_t));
			strip->split_flag = 1;
			strip->split_direction = 0;
			strip->height = (strip->height > 8 ? ((strip->height+8)>>4)<<3 : 4);
			continue;
		} else if(cmd == 1) {
			strip++;
			memcpy(strip, strip-1, sizeof(ustr_t));
			strip->split_flag = 1;
			strip->split_direction = 1;
			strip->width = (strip->width > 8 ? ((strip->width+8)>>4)<<3 : 4);
			continue;
		} else if(cmd == 2) {
			if(strip->usl7 == 0) {
				strip->usl7 = 1;
				ref_vectors = NULL;
				continue;
			}
		} else if(cmd == 3) {
			if(strip->usl7 == 0) {
				strip->usl7 = 1;
				ref_vectors = (const signed char*)buf2 + (*buf1 * 2);
				buf1++;
				continue;
			}
		}

		cur_frm_pos = cur + width * strip->ypos + strip->xpos;

		if((blks_width = strip->width) < 0)
			blks_width += 3;
		blks_width >>= 2;
		blks_height = strip->height;

		if(ref_vectors != NULL) {
			ref_frm_pos = ref + (ref_vectors[0] + strip->ypos) * width +
				ref_vectors[1] + strip->xpos;
		} else
			ref_frm_pos = cur_frm_pos - width_tbl[4];

		if(cmd == 2) {
			if(bit_pos <= 0) {
				bit_pos = 8;
				bit_buf = *buf1++;
			}

			bit_pos -= 2;
			cmd = (bit_buf >> bit_pos) & 0x03;

			if(cmd == 0 || ref_vectors != NULL) {
				for(lp1 = 0; lp1 < blks_width; lp1++) {
					for(i = 0, j = 0; i < blks_height; i++, j += width_tbl[1])
						((uint32 *)cur_frm_pos)[j] = ((uint32 *)ref_frm_pos)[j];
					cur_frm_pos += 4;
					ref_frm_pos += 4;
				}
			} else if(cmd != 1)
				return;
		} else {
			k = *buf1 >> 4;
			j = *buf1 & 0x0f;
			buf1++;
			lv = j + fflags2;

			if((lv - 8) <= 7 && (k == 0 || k == 3 || k == 10)) {
				cp2 = _ModPred + ((lv - 8) << 7);
				cp = ref_frm_pos;
				for(i = 0; i < blks_width << 2; i++) {
						int v = *cp >> 1;
						*(cp++) = cp2[v];
				}
			}

			if(k == 1 || k == 4) {
				lv = (hdr[j] & 0xf) + fflags2;
				correction_type_sp[0] = _corrector_type + (lv << 8);
				correction_lp[0] = correction + (lv << 8);
				lv = (hdr[j] >> 4) + fflags2;
				correction_lp[1] = correction + (lv << 8);
				correction_type_sp[1] = _corrector_type + (lv << 8);
			} else {
				correctionloworder_lp[0] = correctionloworder_lp[1] = correctionloworder + (lv << 8);
				correctionhighorder_lp[0] = correctionhighorder_lp[1] = correctionhighorder + (lv << 8);
				correction_type_sp[0] = correction_type_sp[1] = _corrector_type + (lv << 8);
				correction_lp[0] = correction_lp[1] = correction + (lv << 8);
			}

			switch(k) {
				case 1:
				case 0:                    /********** CASE 0 **********/
					for( ; blks_height > 0; blks_height -= 4) {
						for(lp1 = 0; lp1 < blks_width; lp1++) {
							for(lp2 = 0; lp2 < 4; ) {
								k = *buf1++;
								cur_lp = ((uint32 *)cur_frm_pos) + width_tbl[lp2];
								ref_lp = ((uint32 *)ref_frm_pos) + width_tbl[lp2];

								switch(correction_type_sp[0][k]) {
									case 0:
										*cur_lp = FROM_LE_32(((FROM_LE_32(*ref_lp) >> 1) + correction_lp[lp2 & 0x01][k]) << 1);
										lp2++;
										break;
									case 1:
										res = ((FROM_LE_16(((uint16 *)(ref_lp))[0]) >> 1) + correction_lp[lp2 & 0x01][*buf1]) << 1;
										((uint16 *)cur_lp)[0] = FROM_LE_16(res);
										res = ((FROM_LE_16(((uint16 *)(ref_lp))[1]) >> 1) + correction_lp[lp2 & 0x01][k]) << 1;
										((uint16 *)cur_lp)[1] = FROM_LE_16(res);
										buf1++;
										lp2++;
										break;
									case 2:
										if(lp2 == 0) {
											for(i = 0, j = 0; i < 2; i++, j += width_tbl[1])
												cur_lp[j] = ref_lp[j];
											lp2 += 2;
										}
										break;
									case 3:
										if(lp2 < 2) {
											for(i = 0, j = 0; i < (3 - lp2); i++, j += width_tbl[1])
												cur_lp[j] = ref_lp[j];
											lp2 = 3;
										}
										break;
									case 8:
										if(lp2 == 0) {
											RLE_V3_CHECK(buf1,rle_v1,rle_v2,rle_v3)

											if(rle_v1 == 1 || ref_vectors != NULL) {
												for(i = 0, j = 0; i < 4; i++, j += width_tbl[1])
													cur_lp[j] = ref_lp[j];
											}

											RLE_V2_CHECK(buf1,rle_v2, rle_v3,lp2)
											break;
										} else {
											rle_v1 = 1;
											rle_v2 = *buf1 - 1;
										}
									case 5:
											LP2_CHECK(buf1,rle_v3,lp2)
									case 4:
										for(i = 0, j = 0; i < (4 - lp2); i++, j += width_tbl[1])
											cur_lp[j] = ref_lp[j];
										lp2 = 4;
										break;

									case 7:
										if(rle_v3 != 0)
											rle_v3 = 0;
										else {
											buf1--;
											rle_v3 = 1;
										}
									case 6:
										if(ref_vectors != NULL) {
											for(i = 0, j = 0; i < 4; i++, j += width_tbl[1])
												cur_lp[j] = ref_lp[j];
										}
										lp2 = 4;
										break;

									case 9:
										lv1 = *buf1++;
										lv = (lv1 & 0x7F) << 1;
										lv += (lv << 8);
										lv += (lv << 16);
										for(i = 0, j = 0; i < 4; i++, j += width_tbl[1])
											cur_lp[j] = lv;

										LV1_CHECK(buf1,rle_v3,lv1,lp2)
										break;
									default:
										return;
								}
							}

							cur_frm_pos += 4;
							ref_frm_pos += 4;
						}

						cur_frm_pos += ((width - blks_width) * 4);
						ref_frm_pos += ((width - blks_width) * 4);
					}
					break;

				case 4:
				case 3:                    /********** CASE 3 **********/
					if(ref_vectors != NULL)
						return;
					flag1 = 1;

					for( ; blks_height > 0; blks_height -= 8) {
						for(lp1 = 0; lp1 < blks_width; lp1++) {
							for(lp2 = 0; lp2 < 4; ) {
								k = *buf1++;

								cur_lp = ((uint32 *)cur_frm_pos) + width_tbl[lp2 * 2];
								ref_lp = ((uint32 *)cur_frm_pos) + width_tbl[(lp2 * 2) - 1];

								switch(correction_type_sp[lp2 & 0x01][k]) {
									case 0:
										cur_lp[width_tbl[1]] = FROM_LE_32(((FROM_LE_32(*ref_lp) >> 1) + correction_lp[lp2 & 0x01][k]) << 1);
										if(lp2 > 0 || flag1 == 0 || strip->ypos != 0)
											cur_lp[0] = ((cur_lp[-width_tbl[1]] >> 1) + (cur_lp[width_tbl[1]] >> 1)) & 0xFEFEFEFE;
										else
											cur_lp[0] = FROM_LE_32(((FROM_LE_32(*ref_lp) >> 1) + correction_lp[lp2 & 0x01][k]) << 1);
										lp2++;
										break;

									case 1:
										res = ((FROM_LE_16(((uint16 *)ref_lp)[0]) >> 1) + correction_lp[lp2 & 0x01][*buf1]) << 1;
										((uint16 *)cur_lp)[width_tbl[2]] = FROM_LE_16(res);
										res = ((FROM_LE_16(((uint16 *)ref_lp)[1]) >> 1) + correction_lp[lp2 & 0x01][k]) << 1;
										((uint16 *)cur_lp)[width_tbl[2]+1] = FROM_LE_16(res);

										if(lp2 > 0 || flag1 == 0 || strip->ypos != 0)
											cur_lp[0] = ((cur_lp[-width_tbl[1]] >> 1) + (cur_lp[width_tbl[1]] >> 1)) & 0xFEFEFEFE;
										else
											cur_lp[0] = cur_lp[width_tbl[1]];
										buf1++;
										lp2++;
										break;

									case 2:
										if(lp2 == 0) {
											for(i = 0, j = 0; i < 4; i++, j += width_tbl[1])
												cur_lp[j] = *ref_lp;
											lp2 += 2;
										}
										break;

									case 3:
										if(lp2 < 2) {
											for(i = 0, j = 0; i < 6 - (lp2 * 2); i++, j += width_tbl[1])
												cur_lp[j] = *ref_lp;
											lp2 = 3;
										}
										break;

									case 6:
										lp2 = 4;
										break;

									case 7:
										if(rle_v3 != 0)
											rle_v3 = 0;
										else {
											buf1--;
											rle_v3 = 1;
										}
										lp2 = 4;
										break;

									case 8:
										if(lp2 == 0) {
											RLE_V3_CHECK(buf1,rle_v1,rle_v2,rle_v3)

											if(rle_v1 == 1) {
												for(i = 0, j = 0; i < 8; i++, j += width_tbl[1])
													cur_lp[j] = ref_lp[j];
											}

											RLE_V2_CHECK(buf1,rle_v2, rle_v3,lp2)
											break;
										} else {
											rle_v2 = (*buf1) - 1;
											rle_v1 = 1;
										}
									case 5:
											LP2_CHECK(buf1,rle_v3,lp2)
									case 4:
										for(i = 0, j = 0; i < 8 - (lp2 * 2); i++, j += width_tbl[1])
											cur_lp[j] = *ref_lp;
										lp2 = 4;
										break;

									case 9:
										warning("Indeo3::decodeChunk: Untested (1)");
										lv1 = *buf1++;
										lv = (lv1 & 0x7F) << 1;
										lv += (lv << 8);
										lv += (lv << 16);

										for(i = 0, j = 0; i < 4; i++, j += width_tbl[1])
											cur_lp[j] = lv;

										LV1_CHECK(buf1,rle_v3,lv1,lp2)
										break;

									default:
										return;
								}
							}

							cur_frm_pos += 4;
						}

						cur_frm_pos += (((width * 2) - blks_width) * 4);
						flag1 = 0;
					}
					break;

				case 10:                    /********** CASE 10 **********/
					if(ref_vectors == NULL) {
						flag1 = 1;

						for( ; blks_height > 0; blks_height -= 8) {
							for(lp1 = 0; lp1 < blks_width; lp1 += 2) {
								for(lp2 = 0; lp2 < 4; ) {
									k = *buf1++;
									cur_lp = ((uint32 *)cur_frm_pos) + width_tbl[lp2 * 2];
									ref_lp = ((uint32 *)cur_frm_pos) + width_tbl[(lp2 * 2) - 1];
									lv1 = ref_lp[0];
									lv2 = ref_lp[1];
									if(lp2 == 0 && flag1 != 0) {
#if defined(SCUMM_BIG_ENDIAN)
										lv1 = lv1 & 0xFF00FF00;
										lv1 = (lv1 >> 8) | lv1;
										lv2 = lv2 & 0xFF00FF00;
										lv2 = (lv2 >> 8) | lv2;
#else
										lv1 = lv1 & 0x00FF00FF;
										lv1 = (lv1 << 8) | lv1;
										lv2 = lv2 & 0x00FF00FF;
										lv2 = (lv2 << 8) | lv2;
#endif
									}

									switch(correction_type_sp[lp2 & 0x01][k]) {
										case 0:
											cur_lp[width_tbl[1]] = FROM_LE_32(((FROM_LE_32(lv1) >> 1) + correctionloworder_lp[lp2 & 0x01][k]) << 1);
											cur_lp[width_tbl[1]+1] = FROM_LE_32(((FROM_LE_32(lv2) >> 1) + correctionhighorder_lp[lp2 & 0x01][k]) << 1);
											if(lp2 > 0 || strip->ypos != 0 || flag1 == 0) {
												cur_lp[0] = ((cur_lp[-width_tbl[1]] >> 1) + (cur_lp[width_tbl[1]] >> 1)) & 0xFEFEFEFE;
												cur_lp[1] = ((cur_lp[-width_tbl[1]+1] >> 1) + (cur_lp[width_tbl[1]+1] >> 1)) & 0xFEFEFEFE;
											} else {
												cur_lp[0] = cur_lp[width_tbl[1]];
												cur_lp[1] = cur_lp[width_tbl[1]+1];
											}
											lp2++;
											break;

										case 1:
											cur_lp[width_tbl[1]] = FROM_LE_32(((FROM_LE_32(lv1) >> 1) + correctionloworder_lp[lp2 & 0x01][*buf1]) << 1);
											cur_lp[width_tbl[1]+1] = FROM_LE_32(((FROM_LE_32(lv2) >> 1) + correctionloworder_lp[lp2 & 0x01][k]) << 1);
											if(lp2 > 0 || strip->ypos != 0 || flag1 == 0) {
												cur_lp[0] = ((cur_lp[-width_tbl[1]] >> 1) + (cur_lp[width_tbl[1]] >> 1)) & 0xFEFEFEFE;
												cur_lp[1] = ((cur_lp[-width_tbl[1]+1] >> 1) + (cur_lp[width_tbl[1]+1] >> 1)) & 0xFEFEFEFE;
											} else {
												cur_lp[0] = cur_lp[width_tbl[1]];
												cur_lp[1] = cur_lp[width_tbl[1]+1];
											}
											buf1++;
											lp2++;
											break;

										case 2:
											if(lp2 == 0) {
												if(flag1 != 0) {
													for(i = 0, j = width_tbl[1]; i < 3; i++, j += width_tbl[1]) {
														cur_lp[j] = lv1;
														cur_lp[j+1] = lv2;
													}
													cur_lp[0] = ((cur_lp[-width_tbl[1]] >> 1) + (cur_lp[width_tbl[1]] >> 1)) & 0xFEFEFEFE;
													cur_lp[1] = ((cur_lp[-width_tbl[1]+1] >> 1) + (cur_lp[width_tbl[1]+1] >> 1)) & 0xFEFEFEFE;
												} else {
													for(i = 0, j = 0; i < 4; i++, j += width_tbl[1]) {
														cur_lp[j] = lv1;
														cur_lp[j+1] = lv2;
													}
												}
												lp2 += 2;
											}
											break;

										case 3:
											if(lp2 < 2) {
												if(lp2 == 0 && flag1 != 0) {
													for(i = 0, j = width_tbl[1]; i < 5; i++, j += width_tbl[1]) {
														cur_lp[j] = lv1;
														cur_lp[j+1] = lv2;
													}
													cur_lp[0] = ((cur_lp[-width_tbl[1]] >> 1) + (cur_lp[width_tbl[1]] >> 1)) & 0xFEFEFEFE;
													cur_lp[1] = ((cur_lp[-width_tbl[1]+1] >> 1) + (cur_lp[width_tbl[1]+1] >> 1)) & 0xFEFEFEFE;
												} else {
													for(i = 0, j = 0; i < 6 - (lp2 * 2); i++, j += width_tbl[1]) {
														cur_lp[j] = lv1;
														cur_lp[j+1] = lv2;
													}
												}
												lp2 = 3;
											}
											break;

										case 8:
											if(lp2 == 0) {
												RLE_V3_CHECK(buf1,rle_v1,rle_v2,rle_v3)
												if(rle_v1 == 1) {
													if(flag1 != 0) {
														for(i = 0, j = width_tbl[1]; i < 7; i++, j += width_tbl[1]) {
															cur_lp[j] = lv1;
															cur_lp[j+1] = lv2;
														}
														cur_lp[0] = ((cur_lp[-width_tbl[1]] >> 1) + (cur_lp[width_tbl[1]] >> 1)) & 0xFEFEFEFE;
														cur_lp[1] = ((cur_lp[-width_tbl[1]+1] >> 1) + (cur_lp[width_tbl[1]+1] >> 1)) & 0xFEFEFEFE;
													} else {
														for(i = 0, j = 0; i < 8; i++, j += width_tbl[1]) {
															cur_lp[j] = lv1;
															cur_lp[j+1] = lv2;
														}
													}
												}
												RLE_V2_CHECK(buf1,rle_v2, rle_v3,lp2)
												break;
											} else {
												rle_v1 = 1;
												rle_v2 = (*buf1) - 1;
											}
										case 5:
												LP2_CHECK(buf1,rle_v3,lp2)
										case 4:
											if(lp2 == 0 && flag1 != 0) {
												for(i = 0, j = width_tbl[1]; i < 7; i++, j += width_tbl[1]) {
													cur_lp[j] = lv1;
													cur_lp[j+1] = lv2;
												}
												cur_lp[0] = ((cur_lp[-width_tbl[1]] >> 1) + (cur_lp[width_tbl[1]] >> 1)) & 0xFEFEFEFE;
												cur_lp[1] = ((cur_lp[-width_tbl[1]+1] >> 1) + (cur_lp[width_tbl[1]+1] >> 1)) & 0xFEFEFEFE;
											} else {
												for(i = 0, j = 0; i < 8 - (lp2 * 2); i++, j += width_tbl[1]) {
													cur_lp[j] = lv1;
													cur_lp[j+1] = lv2;
												}
											}
											lp2 = 4;
											break;

										case 6:
											lp2 = 4;
											break;

										case 7:
											if(lp2 == 0) {
												if(rle_v3 != 0)
													rle_v3 = 0;
												else {
													buf1--;
													rle_v3 = 1;
												}
												lp2 = 4;
											}
											break;

										case 9:
											warning("Indeo3::decodeChunk: Untested (2)");
											lv1 = *buf1;
											lv = (lv1 & 0x7F) << 1;
											lv += (lv << 8);
											lv += (lv << 16);
											for(i = 0, j = 0; i < 8; i++, j += width_tbl[1])
												cur_lp[j] = lv;
											LV1_CHECK(buf1,rle_v3,lv1,lp2)
											break;

										default:
											return;
									}
								}

								cur_frm_pos += 8;
							}

							cur_frm_pos += (((width * 2) - blks_width) * 4);
							flag1 = 0;
						}
					} else {
						for( ; blks_height > 0; blks_height -= 8) {
							for(lp1 = 0; lp1 < blks_width; lp1 += 2) {
								for(lp2 = 0; lp2 < 4; ) {
									k = *buf1++;
									cur_lp = ((uint32 *)cur_frm_pos) + width_tbl[lp2 * 2];
									ref_lp = ((uint32 *)ref_frm_pos) + width_tbl[lp2 * 2];

									switch(correction_type_sp[lp2 & 0x01][k]) {
										case 0:
											lv1 = correctionloworder_lp[lp2 & 0x01][k];
											lv2 = correctionhighorder_lp[lp2 & 0x01][k];
											cur_lp[0] = FROM_LE_32(((FROM_LE_32(ref_lp[0]) >> 1) + lv1) << 1);
											cur_lp[1] = FROM_LE_32(((FROM_LE_32(ref_lp[1]) >> 1) + lv2) << 1);
											cur_lp[width_tbl[1]] = FROM_LE_32(((FROM_LE_32(ref_lp[width_tbl[1]]) >> 1) + lv1) << 1);
											cur_lp[width_tbl[1]+1] = FROM_LE_32(((FROM_LE_32(ref_lp[width_tbl[1]+1]) >> 1) + lv2) << 1);
											lp2++;
											break;

										case 1:
											lv1 = correctionloworder_lp[lp2 & 0x01][*buf1++];
											lv2 = correctionloworder_lp[lp2 & 0x01][k];
											cur_lp[0] = FROM_LE_32(((FROM_LE_32(ref_lp[0]) >> 1) + lv1) << 1);
											cur_lp[1] = FROM_LE_32(((FROM_LE_32(ref_lp[1]) >> 1) + lv2) << 1);
											cur_lp[width_tbl[1]] = FROM_LE_32(((FROM_LE_32(ref_lp[width_tbl[1]]) >> 1) + lv1) << 1);
											cur_lp[width_tbl[1]+1] = FROM_LE_32(((FROM_LE_32(ref_lp[width_tbl[1]+1]) >> 1) + lv2) << 1);
											lp2++;
											break;

										case 2:
											if(lp2 == 0) {
												for(i = 0, j = 0; i < 4; i++, j += width_tbl[1]) {
													cur_lp[j] = ref_lp[j];
													cur_lp[j+1] = ref_lp[j+1];
												}
												lp2 += 2;
											}
											break;

										case 3:
											if(lp2 < 2) {
												for(i = 0, j = 0; i < 6 - (lp2 * 2); i++, j += width_tbl[1]) {
													cur_lp[j] = ref_lp[j];
													cur_lp[j+1] = ref_lp[j+1];
												}
												lp2 = 3;
											}
											break;

										case 8:
											if(lp2 == 0) {
												RLE_V3_CHECK(buf1,rle_v1,rle_v2,rle_v3)
												for(i = 0, j = 0; i < 8; i++, j += width_tbl[1]) {
													((uint32 *)cur_frm_pos)[j] = ((uint32 *)ref_frm_pos)[j];
													((uint32 *)cur_frm_pos)[j+1] = ((uint32 *)ref_frm_pos)[j+1];
												}
												RLE_V2_CHECK(buf1,rle_v2, rle_v3,lp2)
												break;
											} else {
												rle_v1 = 1;
												rle_v2 = (*buf1) - 1;
											}
										case 5:
										case 7:
												LP2_CHECK(buf1,rle_v3,lp2)
										case 6:
										case 4:
											for(i = 0, j = 0; i < 8 - (lp2 * 2); i++, j += width_tbl[1]) {
												cur_lp[j] = ref_lp[j];
												cur_lp[j+1] = ref_lp[j+1];
											}
											lp2 = 4;
											break;

										case 9:
											warning("Indeo3::decodeChunk: Untested (3)");
											lv1 = *buf1;
											lv = (lv1 & 0x7F) << 1;
											lv += (lv << 8);
											lv += (lv << 16);
											for(i = 0, j = 0; i < 8; i++, j += width_tbl[1])
												((uint32 *)cur_frm_pos)[j] = ((uint32 *)cur_frm_pos)[j+1] = lv;
											LV1_CHECK(buf1,rle_v3,lv1,lp2)
											break;

										default:
											return;
									}
								}

								cur_frm_pos += 8;
								ref_frm_pos += 8;
							}

							cur_frm_pos += (((width * 2) - blks_width) * 4);
							ref_frm_pos += (((width * 2) - blks_width) * 4);
						}
					}
					break;

				case 11:                    /********** CASE 11 **********/
					if(ref_vectors == NULL)
						return;

					for( ; blks_height > 0; blks_height -= 8) {
						for(lp1 = 0; lp1 < blks_width; lp1++) {
							for(lp2 = 0; lp2 < 4; ) {
								k = *buf1++;
								cur_lp = ((uint32 *)cur_frm_pos) + width_tbl[lp2 * 2];
								ref_lp = ((uint32 *)ref_frm_pos) + width_tbl[lp2 * 2];

								switch(correction_type_sp[lp2 & 0x01][k]) {
									case 0:
										cur_lp[0] = FROM_LE_32(((FROM_LE_32(*ref_lp) >> 1) + correction_lp[lp2 & 0x01][k]) << 1);
										cur_lp[width_tbl[1]] = FROM_LE_32(((FROM_LE_32(ref_lp[width_tbl[1]]) >> 1) + correction_lp[lp2 & 0x01][k]) << 1);
										lp2++;
										break;

									case 1:
										lv1 = (uint16)(correction_lp[lp2 & 0x01][*buf1++]);
										lv2 = (uint16)(correction_lp[lp2 & 0x01][k]);
										res = (uint16)(((FROM_LE_16(((uint16 *)ref_lp)[0]) >> 1) + lv1) << 1);
										((uint16 *)cur_lp)[0] = FROM_LE_16(res);
										res = (uint16)(((FROM_LE_16(((uint16 *)ref_lp)[1]) >> 1) + lv2) << 1);
										((uint16 *)cur_lp)[1] = FROM_LE_16(res);
										res = (uint16)(((FROM_LE_16(((uint16 *)ref_lp)[width_tbl[2]]) >> 1) + lv1) << 1);
										((uint16 *)cur_lp)[width_tbl[2]] = FROM_LE_16(res);
										res = (uint16)(((FROM_LE_16(((uint16 *)ref_lp)[width_tbl[2]+1]) >> 1) + lv2) << 1);
										((uint16 *)cur_lp)[width_tbl[2]+1] = FROM_LE_16(res);
										lp2++;
										break;

									case 2:
										if(lp2 == 0) {
											for(i = 0, j = 0; i < 4; i++, j += width_tbl[1])
												cur_lp[j] = ref_lp[j];
											lp2 += 2;
										}
										break;

									case 3:
										if(lp2 < 2) {
											for(i = 0, j = 0; i < 6 - (lp2 * 2); i++, j += width_tbl[1])
												cur_lp[j] = ref_lp[j];
											lp2 = 3;
										}
										break;

									case 8:
										if(lp2 == 0) {
											RLE_V3_CHECK(buf1,rle_v1,rle_v2,rle_v3)

											for(i = 0, j = 0; i < 8; i++, j += width_tbl[1])
												cur_lp[j] = ref_lp[j];

											RLE_V2_CHECK(buf1,rle_v2, rle_v3,lp2)
											break;
										} else {
											rle_v1 = 1;
											rle_v2 = (*buf1) - 1;
										}
									case 5:
									case 7:
											LP2_CHECK(buf1,rle_v3,lp2)
									case 4:
									case 6:
										for(i = 0, j = 0; i < 8 - (lp2 * 2); i++, j += width_tbl[1])
											cur_lp[j] = ref_lp[j];
										lp2 = 4;
										break;

								case 9:
									warning("Indeo3::decodeChunk: Untested (4)");
									lv1 = *buf1++;
									lv = (lv1 & 0x7F) << 1;
									lv += (lv << 8);
									lv += (lv << 16);
									for(i = 0, j = 0; i < 4; i++, j += width_tbl[1])
										cur_lp[j] = lv;
									LV1_CHECK(buf1,rle_v3,lv1,lp2)
									break;

									default:
										return;
								}
							}

							cur_frm_pos += 4;
							ref_frm_pos += 4;
						}

						cur_frm_pos += (((width * 2) - blks_width) * 4);
						ref_frm_pos += (((width * 2) - blks_width) * 4);
					}
					break;

				default:
					return;
			}
		}

		if(strip < strip_tbl)
			return;

		for( ; strip >= strip_tbl; strip--) {
			if(strip->split_flag != 0) {
				strip->split_flag = 0;
				strip->usl7 = (strip-1)->usl7;

				if(strip->split_direction) {
					strip->xpos += strip->width;
					strip->width = (strip-1)->width - strip->width;
					if(region_160_width <= strip->xpos && width < strip->width + strip->xpos)
						strip->width = width - strip->xpos;
				} else {
					strip->ypos += strip->height;
					strip->height = (strip-1)->height - strip->height;
				}
				break;
			}
		}
	}
}

} // End of namespace Gob
