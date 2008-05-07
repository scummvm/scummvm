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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "kyra/kyra_hof.h"
#include "kyra/screen_hof.h"

namespace Kyra {

Screen_HoF::Screen_HoF(KyraEngine_HoF *vm, OSystem *system)
	: Screen_v2(vm, system) {
	_vm = vm;
	_wsaFrameAnimBuffer = new uint8[1024];
}

Screen_HoF::~Screen_HoF() {
	delete[] _wsaFrameAnimBuffer;
}

void Screen_HoF::setScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_HoF::setScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	_curDim = &_screenDimTable[dim];
}

const ScreenDim *Screen_HoF::getScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_HoF::getScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	return &_screenDimTable[dim];
}

void Screen_HoF::generateGrayOverlay(const uint8 *srcPal, uint8 *grayOverlay, int factor, int addR, int addG, int addB, int lastColor, bool flag) {
	uint8 tmpPal[768];

	for (int i = 0; i != lastColor; i++) {
		if (flag) {
			int v = ((((srcPal[3 * i] & 0x3f) + (srcPal[3 * i + 1] & 0x3f)
				+ (srcPal[3 * i + 2] & 0x3f)) / 3) * factor) / 0x40;
			tmpPal[3 * i] = tmpPal[3 * i + 1] = tmpPal[3 * i + 2] = v & 0xff;
		} else {
			int v = (((srcPal[3 * i] & 0x3f) * factor) / 0x40) + addR;
			tmpPal[3 * i] = (v > 0x3f) ? 0x3f : v & 0xff;
			v = (((srcPal[3 * i + 1] & 0x3f) * factor) / 0x40) + addG;
			tmpPal[3 * i + 1] = (v > 0x3f) ? 0x3f : v & 0xff;
			v = (((srcPal[3 * i + 2] & 0x3f) * factor) / 0x40) + addB;
			tmpPal[3 * i + 2] = (v > 0x3f) ? 0x3f : v & 0xff;
		}
	}

	for (int i = 0; i < lastColor; i++)
		grayOverlay[i] = findLeastDifferentColor(tmpPal + 3 * i, srcPal, lastColor);
}

void Screen_HoF::wsaFrameAnimationStep(int x1, int y1, int x2, int y2,
	int w1, int h1, int w2, int h2, int srcPage, int dstPage, int dim) {

	if (!(w1 || h1 || w2 || h2))
		return;

	ScreenDim cdm = _screenDimTable[dim];
	cdm.sx <<= 3;
	cdm.w <<= 3;

	int na = 0, nb = 0, nc = w2;

	if (!calcBounds(cdm.w, cdm.h, x2, y2, w2, h2, na, nb, nc))
		return;

	const uint8 *src = getPagePtr(srcPage) + y1 * 320;
	uint8 *dst = getPagePtr(dstPage) + (y2 + cdm.sy) * 320;

	int u = -1;

	do {
		int t = (nb * h1) / h2;
		if (t != u) {
			u = t;
			const uint8 *s = src + (x1 + t) * 320;
			uint8 *dt = (uint8*) _wsaFrameAnimBuffer;

			t = w2 - w1;
			if (!t) {
				memcpy(dt, s, w2);
			} else if (t > 0) {
				if (w1 == 1) {
                    memset(dt, *s, w2);
				} else {
					t = ((((((w2 - w1 + 1) & 0xffff) << 8) / w1) + 0x100) & 0xffff) << 8;
					int bp = 0;
					for (int i = 0; i < w1; i++) {
						int cnt = (t >> 16);
						bp += (t & 0xffff);
						if (bp > 0xffff) {
							bp -= 0xffff;
							cnt++;
						}
						memset(dt, *s++, cnt);
						dt += cnt;
					}
				}
			} else {
				if (w2 == 1) {
					*dt = *s;
				} else {
					t = (((((w1 - w2) & 0xffff) << 8) / w2) & 0xffff) << 8;
					int bp = 0;
					for (int i = 0; i < w2; i++) {
						*dt++ = *s++;
						bp += (t & 0xffff);
						if (bp > 0xffff) {
							bp -= 0xffff;
							s++;
						}
						s += (t >> 16);
					}
				}
			}
		}
		memcpy(dst + x2 + cdm.sx, _wsaFrameAnimBuffer + na, w2);
		dst += 320;
	} while (++nb < h2);
}

void Screen_HoF::cmpFadeFrameStep(int srcPage, int srcW, int srcH, int srcX, int srcY, int dstPage, int dstW,
	int dstH, int dstX, int dstY, int cmpW, int cmpH, int cmpPage) {

	if (!(cmpW || cmpH ))
		return;

	int r1, r2, r3, r4, r5, r6;

	int X1 = srcX;
	int Y1 = srcY;
	int W1 = cmpW;
	int H1 = cmpH;

	if (!calcBounds(srcW, srcH, X1, Y1, W1, H1, r1, r2, r3))
		return;

	int X2 = dstX;
	int Y2 = dstY;
	int W2 = W1;
	int H2 = H1;

	if (!calcBounds(dstW, dstH, X2, Y2, W2, H2, r4, r5, r6))
		return;

	const uint8 *src = getPagePtr(srcPage) + srcW * (Y1 + r5);
	uint8 *dst = getPagePtr(dstPage) + dstW * (Y2 + r2);
	const uint8 *cmp = getPagePtr(cmpPage);

	while (H2--) {
		const uint8 *s = src + r4 + X1;
		uint8 *d = dst + r1 + X2;

		for (int i = 0; i < W2; i++) {
			int ix = (*s++ << 8) + *d;
			*d++ = cmp[ix];
		}

		src += W1;
		dst += W2;
	}
}

void Screen_HoF::copyPageMemory(int srcPage, int srcPos, int dstPage, int dstPos, int numBytes) {
	const uint8 *src = getPagePtr(srcPage) + srcPos;
	uint8 *dst = getPagePtr(dstPage) + dstPos;
	memcpy(dst, src, numBytes);
}


void Screen_HoF::copyRegionEx(int srcPage, int srcW, int srcH, int dstPage, int dstX,int dstY, int dstW, int dstH, const ScreenDim *dim, bool flag) {
	int x0 = dim->sx << 3;
	int y0 = dim->sy;
	int w0 = dim->w << 3;
	int h0 = dim->h;

	int x1 = dstX;
	int y1 = dstY;
	int w1 = dstW;
	int h1 = dstH;

	int x2, y2, w2;

	calcBounds(w0, h0, x1, y1, w1, h1, x2, y2, w2);

	const uint8 *src = getPagePtr(srcPage) + (320 * srcH) + srcW;
	uint8 *dst = getPagePtr(dstPage) + 320 * (y0 + y1);

	for (int y = 0; y < h1; y++) {
		const uint8 *s = src + x2;
		uint8 *d = dst + x0 + x1;

		if (flag)
			d += (h1 >> 1);

		for (int x = 0; x < w1; x++) {
			if (*s)
				*d = *s;
			s++;
			d++;
		}
		dst += 320;
		src += 320;
	}
}

bool Screen_HoF::calcBounds(int w0, int h0, int &x1, int &y1, int &w1, int &h1, int &x2, int &y2, int &w2) {
	x2 = 0;
	y2 = 0;
	w2 = w1;

	int t = x1 + w1;
	if (t < 1) {
		w1 = h1 = -1;
	} else {
		if (t <= x1) {
			x2 = w1 - t;
			w1 = t;
			x1 = 0;
		}
		t = w0 - x1;
		if (t < 1) {
			w1 = h1 = -1;
		} else {
			if (t <= w1) {
				w1 = t;
			}
			w2 -= w1;
			t = h1 + y1;
			if (t < 1) {
				w1 = h1 = -1;
			} else {
				if (t <= y1) {
					y2 = h1 - t;
					h1 = t;
					y1 = 0;
				}
                t = h0 - y1;
				if (t < 1) {
					w1 = h1 = -1;
				} else {
					if (t <= h1) {
						h1 = t;
					}
				}
			}
		}
	}

	return (w1 == -1) ? false : true;
}

} // end of namespace Kyra

