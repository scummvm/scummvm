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

// NanoJPEG -- KeyJ's Tiny Baseline JPEG Decoder
// version 1.3.5 (2016-11-14)
// Copyright (c) 2009-2016 Martin J. Fiedler <martin.fiedler@gmx.net>
// published under the terms of the MIT license
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "common/scummsys.h"

#include "engines/icb/jpeg_decode.h"

namespace ICB {

typedef enum _nj_result {
	NJ_OK = 0,        // no error, decoding successful
	NJ_NO_JPEG,       // not a JPEG file
	NJ_UNSUPPORTED,   // unsupported format
	NJ_OUT_OF_MEM,    // out of memory
	NJ_INTERNAL_ERR,  // internal error
	NJ_SYNTAX_ERROR,  // syntax error
	NJ_FINISHED,      // used internally
} nj_result_t;

typedef struct _nj_code {
	byte bits, code;
} nj_vlc_code_t;

typedef struct _nj_cmp {
	int cid;
	int ssx, ssy;
	int width, height;
	int stride;
	int qtsel;
	int actabsel, dctabsel;
	int dcpred;
	byte *pixels;
} nj_component_t;

typedef struct _nj_ctx {
	nj_result_t error;
	const byte *pos;
	int size;
	int length;
	int width, height;
	int mbwidth, mbheight;
	int mbsizex, mbsizey;
	int ncomp;
	nj_component_t comp[3];
	int curcomp;
	int qtused, qtavail;
	byte qtab[4][64];
	nj_vlc_code_t vlctab[4][65536];
	int buf, bufbits;
	int block[64];
} nj_context_t;

static nj_context_t nj;

static const char njZZ[64] = {
	0,   1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

static FORCEINLINE byte njClip(const int x) {
	return (x < 0) ? 0 : ((x > 0xFF) ? 0xFF : (byte)x);
}

#define W1 2841
#define W2 2676
#define W3 2408
#define W5 1609
#define W6 1108
#define W7 565

static FORCEINLINE void njRowIDCT(int *blk) {
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;

	if (!((x1 = blk[4] << 11) |
	      (x2 = blk[6]) |
	      (x3 = blk[2]) |
	      (x4 = blk[1]) |
	      (x5 = blk[7]) |
	      (x6 = blk[5]) |
	      (x7 = blk[3]))) {
		blk[0] = blk[1] = blk[2] = blk[3] = blk[4] = blk[5] = blk[6] = blk[7] = blk[0] << 3;
		return;
	}
	x0 = (blk[0] << 11) + 128;
	x8 = W7 * (x4 + x5);
	x4 = x8 + (W1 - W7) * x4;
	x5 = x8 - (W1 + W7) * x5;
	x8 = W3 * (x6 + x7);
	x6 = x8 - (W3 - W5) * x6;
	x7 = x8 - (W3 + W5) * x7;
	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6 * (x3 + x2);
	x2 = x1 - (W2 + W6) * x2;
	x3 = x1 + (W2 - W6) * x3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;
	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181 * (x4 + x5) + 128) >> 8;
	x4 = (181 * (x4 - x5) + 128) >> 8;
	blk[0] = (x7 + x1) >> 8;
	blk[1] = (x3 + x2) >> 8;
	blk[2] = (x0 + x4) >> 8;
	blk[3] = (x8 + x6) >> 8;
	blk[4] = (x8 - x6) >> 8;
	blk[5] = (x0 - x4) >> 8;
	blk[6] = (x3 - x2) >> 8;
	blk[7] = (x7 - x1) >> 8;
}

static FORCEINLINE void njColIDCT(const int *blk, byte *out, int stride) {
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;

	if (!((x1 = blk[8 * 4] << 8) |
	      (x2 = blk[8 * 6]) |
	      (x3 = blk[8 * 2]) |
	      (x4 = blk[8 * 1]) |
	      (x5 = blk[8 * 7]) |
	      (x6 = blk[8 * 5]) |
	      (x7 = blk[8 * 3]))) {
		x1 = njClip(((blk[0] + 32) >> 6) + 128);
		for (x0 = 8; x0; --x0) {
			*out = (byte)x1;
			out += stride;
		}
		return;
	}
	x0 = (blk[0] << 8) + 8192;
	x8 = W7 * (x4 + x5) + 4;
	x4 = (x8 + (W1 - W7) * x4) >> 3;
	x5 = (x8 - (W1 + W7) * x5) >> 3;
	x8 = W3 * (x6 + x7) + 4;
	x6 = (x8 - (W3 - W5) * x6) >> 3;
	x7 = (x8 - (W3 + W5) * x7) >> 3;
	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6 * (x3 + x2) + 4;
	x2 = (x1 - (W2 + W6) * x2) >> 3;
	x3 = (x1 + (W2 - W6) * x3) >> 3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;
	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181 * (x4 + x5) + 128) >> 8;
	x4 = (181 * (x4 - x5) + 128) >> 8;
	*out = njClip(((x7 + x1) >> 14) + 128); out += stride;
	*out = njClip(((x3 + x2) >> 14) + 128); out += stride;
	*out = njClip(((x0 + x4) >> 14) + 128); out += stride;
	*out = njClip(((x8 + x6) >> 14) + 128); out += stride;
	*out = njClip(((x8 - x6) >> 14) + 128); out += stride;
	*out = njClip(((x0 - x4) >> 14) + 128); out += stride;
	*out = njClip(((x3 - x2) >> 14) + 128); out += stride;
	*out = njClip(((x7 - x1) >> 14) + 128);
}

#define njThrow(e) do { nj.error = e; return; } while (0)
#define njCheckError() do { if (nj.error) return; } while (0)

static int njShowBits(int bits) {
	if (!bits)
		return 0;

	while (nj.bufbits < bits) {
		if (nj.size <= 0) {
			nj.buf = (nj.buf << 8) | 0xFF;
			nj.bufbits += 8;
			continue;
		}
		byte newbyte = *nj.pos++;
		nj.size--;
		nj.bufbits += 8;
		nj.buf = (nj.buf << 8) | newbyte;
	}

	return (nj.buf >> (nj.bufbits - bits)) & ((1 << bits) - 1);
}

static void njResetBufBits() {
	while ((nj.bufbits -= 8) > 0) {
		nj.pos--;
		nj.size++;
	}
	nj.buf = 0;
	nj.bufbits = 0;
}

static FORCEINLINE void njSkipBits(int bits) {
	if (nj.bufbits < bits)
		njShowBits(bits);
	nj.bufbits -= bits;
}

static FORCEINLINE int njGetBits(int bits) {
	int res = njShowBits(bits);
	njSkipBits(bits);
	return res;
}

static void njSkip(int count) {
	nj.pos += count;
	nj.size -= count;
	nj.length -= count;
	if (nj.size < 0)
		nj.error = NJ_SYNTAX_ERROR;
}

static FORCEINLINE uint16 njDecode16(const byte *pos) {
	return (pos[1] << 8) | pos[0];
}

static void njDecodeLength() {
	if (nj.size < 2)
		njThrow(NJ_SYNTAX_ERROR);
	nj.length = njDecode16(nj.pos);
	if (nj.length > nj.size)
		njThrow(NJ_SYNTAX_ERROR);
	njSkip(2);
}

static void njDecodeSOF() {
	int i, ssxmax = 0, ssymax = 0;
	nj_component_t *c;

	njDecodeLength();
	njCheckError();
	if (nj.length < 9)
		njThrow(NJ_SYNTAX_ERROR);
	if (nj.pos[0] != 8)
		njThrow(NJ_UNSUPPORTED);

	nj.height = 480;
	nj.width = 640;
	nj.ncomp = nj.pos[1];
	njSkip(2);

	switch (nj.ncomp) {
	case 3:
		break;
	default:
		njThrow(NJ_UNSUPPORTED);
	}

	if (nj.length < (nj.ncomp * 3))
		njThrow(NJ_SYNTAX_ERROR);
	for (i = 0, c = nj.comp; i < nj.ncomp; ++i, ++c) {
		c->cid = nj.pos[0];
	if (!(c->ssx = nj.pos[1] >> 4))
		njThrow(NJ_SYNTAX_ERROR);
	if (c->ssx & (c->ssx - 1))
		njThrow(NJ_UNSUPPORTED);  // non-power of two
	if (!(c->ssy = nj.pos[1] & 15))
		njThrow(NJ_SYNTAX_ERROR);
	if (c->ssy & (c->ssy - 1))
		njThrow(NJ_UNSUPPORTED);  // non-power of two
	if (c->ssx != 1 || c->ssy != 1)
		njThrow(NJ_SYNTAX_ERROR);
	if ((c->qtsel = nj.pos[2]) & 0xFC)
		njThrow(NJ_SYNTAX_ERROR);
	njSkip(3);
	nj.qtused |= 1 << c->qtsel;
	if (c->ssx > ssxmax)
		ssxmax = c->ssx;
	if (c->ssy > ssymax)
		ssymax = c->ssy;
	}
	if (nj.ncomp == 1) {
		c = nj.comp;
		c->ssx = c->ssy = ssxmax = ssymax = 1;
	}
	nj.mbsizex = ssxmax << 3;
	nj.mbsizey = ssymax << 3;
	nj.mbwidth = (nj.width + nj.mbsizex - 1) / nj.mbsizex;
	nj.mbheight = (nj.height + nj.mbsizey - 1) / nj.mbsizey;
	for (i = 0, c = nj.comp; i < nj.ncomp;  ++i, ++c) {
		c->width = (nj.width * c->ssx + ssxmax - 1) / ssxmax;
		c->height = (nj.height * c->ssy + ssymax - 1) / ssymax;
		c->stride = nj.mbwidth * c->ssx << 3;
		if (((c->width < 3) && (c->ssx != ssxmax)) || ((c->height < 3) && (c->ssy != ssymax)))
			njThrow(NJ_UNSUPPORTED);
		if (!(c->pixels = (byte *)malloc(c->stride * nj.mbheight * c->ssy << 3)))
			njThrow(NJ_OUT_OF_MEM);
	}
	njSkip(nj.length - 4);
}

static void njDecodeDHT() {
	njDecodeLength();
	njCheckError();

	while (nj.length >= 17) {
		byte counts[16];
		int codelen;
		int i = nj.pos[0];
		if (i & 0xEC)
			njThrow(NJ_SYNTAX_ERROR);
		if (i & 0x02)
			njThrow(NJ_UNSUPPORTED);
		i = (i | (i >> 3)) & 3;  // combined DC/AC + tableid value
		for (codelen = 1; codelen <= 16; ++codelen)
			counts[codelen - 1] = nj.pos[codelen];
		njSkip(17);
		nj_vlc_code_t *vlc = &nj.vlctab[i][0];
		int remain = 65536;
		int spread = remain;
		for (codelen = 1; codelen <= 16; ++codelen) {
			spread >>= 1;
			int currcnt = counts[codelen - 1];
			if (!currcnt)
				continue;
			if (nj.length < currcnt)
				njThrow(NJ_SYNTAX_ERROR);
			remain -= currcnt << (16 - codelen);
			if (remain < 0)
				njThrow(NJ_SYNTAX_ERROR);
			for (i = 0; i < currcnt; ++i) {
				byte code = nj.pos[i];
				for (int j = spread; j; --j) {
					vlc->bits = (byte)codelen;
					vlc->code = code;
					++vlc;
				}
			}
			njSkip(currcnt);
		}
		while (remain--) {
			vlc->bits = 0;
			++vlc;
		}
	}
	if (nj.length)
		njThrow(NJ_SYNTAX_ERROR);
}

static void njDecodeDQT() {
	njDecodeLength();
	njCheckError();

	while (nj.length >= 65) {
		int i = nj.pos[0];
		if (i & 0xFC)
			njThrow(NJ_SYNTAX_ERROR);
		nj.qtavail |= 1 << i;
		byte *t = &nj.qtab[i][0];
		for (i = 0; i < 64; ++i)
			t[i] = nj.pos[i + 1];
		njSkip(65);
	}
	if (nj.length)
		njThrow(NJ_SYNTAX_ERROR);
}

static int njGetVLC(nj_vlc_code_t *vlc, byte *code) {
	int value = njShowBits(16);
	int bits = vlc[value].bits;
	if (!bits) {
		nj.error = NJ_SYNTAX_ERROR;
		return 0;
	}
	njSkipBits(bits);
	value = vlc[value].code;
	if (code)
		*code = (byte)value;
	bits = value & 15;
	if (!bits)
		return 0;
	value = njGetBits(bits);
	if (value < (1 << (bits - 1)))
		value += ((-1) << bits) + 1;
	return value;
}

static void njDecodeBlock(nj_component_t *c, byte *out) {
	byte code = 0;
	int value, coef = 0;

	memset(nj.block, 0, sizeof(nj.block));
	c->dcpred += njGetVLC(&nj.vlctab[c->dctabsel][0], nullptr);
	nj.block[0] = (c->dcpred) * nj.qtab[c->qtsel][0];
	do {
		value = njGetVLC(&nj.vlctab[c->actabsel][0], &code);
		if (!code)
			break;  // EOB
		if (!(code & 0x0F) && (code != 0xF0))
			njThrow(NJ_SYNTAX_ERROR);
		coef += (code >> 4) + 1;
		if (coef > 63)
			njThrow(NJ_SYNTAX_ERROR);
		nj.block[(int)njZZ[coef]] = value * nj.qtab[c->qtsel][coef];
	} while (coef < 63);
	for (coef = 0;  coef < 64;  coef += 8)
		njRowIDCT(&nj.block[coef]);
	for (coef = 0;  coef < 8;  ++coef)
		njColIDCT(&nj.block[coef], &out[coef], c->stride);
}

static void njDecodeScan() {
	nj_component_t *c = &nj.comp[nj.curcomp];

	njDecodeLength();
	njCheckError();
	if (nj.pos[0] != 1)
		njThrow(NJ_UNSUPPORTED);
	if (nj.length < (4 + 2))
		njThrow(NJ_SYNTAX_ERROR);
	njSkip(1);

	if (nj.pos[0] != c->cid)
		njThrow(NJ_SYNTAX_ERROR);
	if (nj.pos[1] & 0xEE)
		njThrow(NJ_SYNTAX_ERROR);
	c->dctabsel = nj.pos[1] >> 4;
	c->actabsel = (nj.pos[1] & 1) | 2;
	njSkip(2);

	if (nj.pos[0] || (nj.pos[1] != 63) || nj.pos[2])
		njThrow(NJ_UNSUPPORTED);
	njSkip(nj.length);

	for (int mby = 0; mby < nj.mbheight; mby++) {
		for (int mbx = 0; mbx < nj.mbwidth; mbx++) {
			njDecodeBlock(c, &c->pixels[(mby * c->stride + mbx) << 3]);
			if (nj.error)
				return;
		}
	}
	njResetBufBits();
	c->dcpred = 0;
	nj.curcomp++;
	nj.error = NJ_OK;
}

static nj_result_t njDecode(const byte *jpeg, const int size) {
	memset(&nj, 0, sizeof(nj_context_t));
	nj.pos = jpeg;
	nj.size = size & 0x7FFFFFFF;
	if (nj.size < 2)
		return NJ_NO_JPEG;
	if ((nj.pos[0] ^ 0xFF) | (nj.pos[1] ^ 0xD8))
		return NJ_NO_JPEG;
	njSkip(2);

	while (!nj.error) {
		if ((nj.size < 2) || (nj.pos[0] != 0xFF))
			return NJ_SYNTAX_ERROR;
		njSkip(2);
		switch (nj.pos[-1]) {
		case 0xC0:
			njDecodeSOF();
			break;
		case 0xC4:
			njDecodeDHT();
			break;
		case 0xDB:
			njDecodeDQT();
			break;
		case 0xD9:
			nj.error = NJ_FINISHED;
			break;
		case 0xDA:
			njDecodeScan();
			break;
		default:
			nj.error = NJ_UNSUPPORTED;
			break;
		}
	}
	if (nj.error != NJ_FINISHED)
		return nj.error;
	nj.error = NJ_OK;

	return nj.error;
}

static void njDone() {
	for (int i = 0; i < 3; ++i) {
		if (nj.comp[i].pixels)
			free(nj.comp[i].pixels);
	}
}

static void njConvert(byte *dst) {
	const byte *py  = nj.comp[0].pixels;
	const byte *pcb = nj.comp[1].pixels;
	const byte *pcr = nj.comp[2].pixels;
	for (int yy = nj.height; yy; --yy) {
		for (int x = 0; x < nj.width; ++x) {
			int y = py[x] << 8;
			int cb = pcb[x] - 128;
			int cr = pcr[x] - 128;
			*dst++ = njClip((y + 454 * cb            + 128) >> 8);
			*dst++ = njClip((y -  88 * cb - 183 * cr + 128) >> 8);
			*dst++ = njClip((y            + 359 * cr + 128) >> 8);
			*dst++ = 255;
		}
		py += nj.comp[0].stride;
		pcb += nj.comp[1].stride;
		pcr += nj.comp[2].stride;
	}
}

Graphics::Surface *JpegDecode(const byte *data, const int size) {
	Graphics::Surface *surface = nullptr;
	if (njDecode(data, size) == NJ_OK) {
		surface = new Graphics::Surface();
		surface->create(nj.width, nj.height, Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
		njConvert((byte *)surface->getPixels());
	}
	njDone();

	return surface;
}

} // End of namespace ICB
