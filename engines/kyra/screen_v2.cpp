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

#include "kyra/kyra_v2.h"
#include "kyra/screen_v2.h"

namespace Kyra {

Screen_v2::Screen_v2(KyraEngine_v2 *vm, OSystem *system)
	: Screen(vm, system) {
	_vm = vm;
	_wsaFrameAnimBuffer = new uint8[1024];
}

Screen_v2::~Screen_v2() {
	delete [] _wsaFrameAnimBuffer;
}

void Screen_v2::setScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_v2::setScreenDim(%d)", dim);
	if (_vm->game() == GI_KYRA2) {
		assert(dim < _screenDimTableCount);
		_curDim = &_screenDimTable[dim];
	} else {
		assert(dim < _screenDimTableCountK3);
		_curDim = &_screenDimTableK3[dim];
	}
}

const ScreenDim *Screen_v2::getScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_v2::getScreenDim(%d)", dim);
	if (_vm->game() == GI_KYRA2) {
		assert(dim < _screenDimTableCount);
		return &_screenDimTable[dim];
	} else {
		assert(dim < _screenDimTableCountK3);
		return &_screenDimTableK3[dim];
	}
}

void Screen_v2::generateGrayOverlay(const uint8 *srcPal, uint8 *grayOverlay, int factor, int addR, int addG, int addB, int lastColor, bool flag) {
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

uint8 *Screen_v2::generateOverlay(const uint8 *palette, uint8 *buffer, int startColor, int fac) {
	if (!palette || !buffer)
		return buffer;

	fac = MIN(0xFF, fac);

	byte col1, col2, col3;
	col1 = palette[startColor * 3 + 0];
	col2 = palette[startColor * 3 + 1];
	col3 = palette[startColor * 3 + 2];
	*buffer = 0;

	uint8 *dst = buffer + 1;
	for (int i = 1; i != 255; ++i) {
		byte procCol1, procCol2, procCol3;
		const uint8 *src = palette + i*3;
		const int factor = (fac >> 1) & 0xFF;
		byte col;
		
		col = *src++;
		col -= ((((col - col1) * factor) << 1) >> 8) & 0xFF;
		procCol1 = col;

		col = *src++;
		col -= ((((col - col2) * factor) << 1) >> 8) & 0xFF;
		procCol2 = col;

		col = *src++;
		col -= ((((col - col3) * factor) << 1) >> 8) & 0xFF;
		procCol3 = col;

		uint16 minValue = 0xFFFF;
		uint8 colorNumber = startColor;
		const uint8 *pal = palette + 3;
		for (int count = 0xFF, cur = 1; count > 0; --count, ++cur) {
			pal += 3;
			if (cur != i) {
				uint16 value = 0;
				pal -= 3;
				col = *pal++ - procCol1;
				col *= col;
				value += col;

				col = *pal++ - procCol2;
				col *= col;
				value += col;

				col = *pal++ - procCol3;
				col *= col;
				value += col;

				if (value == 0) {
					colorNumber = i;
					break;
				} else if (value < minValue) {
					minValue = value;
					colorNumber = cur;
				}
			}
		}

		*dst++ = colorNumber;
	}

	return buffer;
}

void Screen_v2::applyOverlay(int x, int y, int w, int h, int pageNum, const uint8 *overlay) {
	uint8 * dst = getPagePtr(pageNum) + y * 320 + x;
	while (h--) {
		for (int wi = 0; wi < w; ++wi) {
			uint8 index = *dst;
			*dst++ = overlay[index];
		}
		dst += 320 - w;
	}
}

int Screen_v2::findLeastDifferentColor(const uint8 *paletteEntry, const uint8 *palette, uint16 numColors) {
	int m = 0x7fff;
	int r = 0x101;

	for (int i = 0; i < numColors; i++) {
		int v = paletteEntry[0] - *palette++;
		int c = v * v;
		v = paletteEntry[1] - *palette++;
		c += (v * v);
		v = paletteEntry[2] - *palette++;
		c += (v * v);

		if (c <= m) {
			m = c;
			r = i;
		}
	}

	return r;
}

void Screen_v2::wsaFrameAnimationStep(int x1, int y1, int x2, int y2,
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

void Screen_v2::cmpFadeFrameStep(int srcPage, int srcW, int srcH, int srcX, int srcY, int dstPage, int dstW,
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

void Screen_v2::copyPageMemory(int srcPage, int srcPos, int dstPage, int dstPos, int numBytes) {
	const uint8 *src = getPagePtr(srcPage) + srcPos;
	uint8 *dst = getPagePtr(dstPage) + dstPos;
	memcpy(dst, src, numBytes);
}


void Screen_v2::copyRegionEx(int srcPage, int srcW, int srcH, int dstPage, int dstX,int dstY, int dstW, int dstH, const ScreenDim *dim, bool flag) {
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

bool Screen_v2::calcBounds(int w0, int h0, int &x1, int &y1, int &w1, int &h1, int &x2, int &y2, int &w2) {
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

void Screen_v2::copyWsaRect(int x, int y, int w, int h, int dimState, int plotFunc, const uint8 *src,
							int unk1, const uint8 *unkPtr1, const uint8 *unkPtr2) {
	uint8 *dstPtr = getPagePtr(_curPage);
	uint8 *origDst = dstPtr;

	const ScreenDim *dim = getScreenDim(dimState);
	int dimX1 = dim->sx << 3;
	int dimX2 = dim->w << 3;
	dimX2 += dimX1;

	int dimY1 = dim->sy;
	int dimY2 = dim->h;
	dimY2 += dimY1;

	int temp = y - dimY1;
	if (temp < 0) {
		if ((temp += h) <= 0)
			return;
		else {
			SWAP(temp, h);
			y += temp - h;
			src += (temp - h) * w;
		}
	}

	temp = dimY2 - y;
	if (temp <= 0)
		return;

	if (temp < h)
		h = temp;

	int srcOffset = 0;
	temp = x - dimX1;
	if (temp < 0) {
		temp = -temp;
		srcOffset = temp;
		x += temp;
		w -= temp;
	}

	int srcAdd = 0;

	temp = dimX2 - x;
	if (temp <= 0)
		return;

	if (temp < w) {
		SWAP(w, temp);
		temp -= w;
		srcAdd = temp;
	}

	dstPtr += y * SCREEN_W + x;
	uint8 *dst = dstPtr;

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, w, h);

	clearOverlayRect(_curPage, x, y, w, h);

	temp = h;
	while (h--) {
		src += srcOffset;
		int cW = w;

		switch (plotFunc) {
		case 0:
			memcpy(dst, src, cW);
			dst += cW; src += cW;
			break;

		case 1:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = unkPtr1[d];
				if (t != 0xFF)
					d = unkPtr2[*dst + (t << 8)];
				*dst++ = d;
			}
			break;

		case 4:
			while (cW--) {
				uint8 d = *src++;
				if (d)
					*dst = d;
				++dst;
			}
			break;

		case 5:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = unkPtr1[d];
					if (t != 0xFF)
						d = unkPtr2[*dst + (t << 8)];
					*dst = d;
				}
				++dst;
			}
			break;

		case 8:
		case 9:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = _shapePages[0][dst - origDst] & 7;
				if (unk1 < t)
					d = _shapePages[1][dst - origDst];
				*dst++ = d;
			}
			break;

		case 12:
		case 13:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = _shapePages[0][dst - origDst] & 7;
					if (unk1 < t)
						d = _shapePages[1][dst - origDst];
					*dst++ = d;
				} else {
					d = _shapePages[1][dst - origDst];
					*dst++ = d;
				}
			}
			break;

		default:
			break;
		}

		dst = (dstPtr += SCREEN_W);
		src += srcAdd;
	}
}

const uint8 *Screen_v2::getPtrToShape(const uint8 *shpFile, int shape) {
	debugC(9, kDebugLevelScreen, "Screen_v2::getPtrToShape(%p, %d)", (const void *)shpFile, shape);
	uint16 shapes = READ_LE_UINT16(shpFile);

	if (shapes <= shape)
		return 0;

	uint32 offset = READ_LE_UINT32(shpFile + (shape << 2) + 2);

	return shpFile + offset + 2;
}

uint8 *Screen_v2::getPtrToShape(uint8 *shpFile, int shape) {
	debugC(9, kDebugLevelScreen, "Screen_v2::getPtrToShape(%p, %d)", (void *)shpFile, shape);
	uint16 shapes = READ_LE_UINT16(shpFile);

	if (shapes <= shape)
		return 0;

	uint32 offset = READ_LE_UINT32(shpFile + (shape << 2) + 2);

	return shpFile + offset + 2;
}

int Screen_v2::getShapeScaledWidth(const uint8 *shpFile, int scale) {
	int width = READ_LE_UINT16(shpFile+3);
	return (width * scale) >> 8;
}

int Screen_v2::getShapeScaledHeight(const uint8 *shpFile, int scale) {
	int height = shpFile[2];
	return (height * scale) >> 8;
}

uint16 Screen_v2::getShapeSize(const uint8 *shp) {
	debugC(9, kDebugLevelScreen, "Screen_v2::getShapeSize(%p)", (const void *)shp);

	return READ_LE_UINT16(shp+6);
}

uint8 *Screen_v2::makeShapeCopy(const uint8 *src, int index) {
	debugC(9, kDebugLevelScreen, "Screen_v2::makeShapeCopy(%p, %d)", (const void *)src, index);

	const uint8 *shape = getPtrToShape(src, index);
	int size = getShapeSize(shape);

	uint8 *copy = new uint8[size];
	assert(copy);
	memcpy(copy, shape, size);

	return copy;
}

void Screen_v2::drawShape(uint8 page, const uint8 *shape, int x, int y, int sd, int flags, ...) {
	if (!shape)
		return;

	if (*shape & 1)
		flags |= 0x400;

	va_list args;
	va_start(args, flags);

	static int drawShapeVar1 = 0;
	static int drawShapeVar2[] = {
		1, 3, 2, 5, 4, 3, 2, 1
	};
	static int drawShapeVar3 = 1;
	static int drawShapeVar4 = 0;
	static int drawShapeVar5 = 0;

	uint8 *table = 0;
	int tableLoopCount = 0;
	int drawLayer = 0;
	const uint8 *table2 = 0;
	uint8 *table3 = 0;
	uint8 *table4 = 0;

	if (flags & 0x8000) {
		table2 = va_arg(args, uint8*);
	}
	if (flags & 0x100) {
		table = va_arg(args, uint8*);
		tableLoopCount = va_arg(args, int);
		if (!tableLoopCount)
			flags &= 0xFFFFFEFF;
	}
	if (flags & 0x1000) {
		table3 = va_arg(args, uint8*);
		table4 = va_arg(args, uint8*);
	}
	if (flags & 0x200) {
		drawShapeVar1 += 1;
		drawShapeVar1 &= 7;
		drawShapeVar3 = drawShapeVar2[drawShapeVar1];
		drawShapeVar4 = 0;
		drawShapeVar5 = 256;
	}
	if (flags & 0x4000) {
		drawShapeVar5 = va_arg(args, int);
	}
	if (flags & 0x800) {
		drawLayer = va_arg(args, int);
	}
	int scale_w, scale_h;
	if (flags & 0x04) {
		scale_w = va_arg(args, int);
		scale_h = va_arg(args, int);
	} else {
		scale_w = 0x100;
		scale_h = 0x100;
	}

	int ppc = (flags >> 8) & 0x3F;

	const uint8 *src = shape;
	uint16 shapeFlags = READ_LE_UINT16(src); src += 2;

	int shapeHeight = *src++;
	int scaledShapeHeight = (shapeHeight * scale_h) >> 8;
	if (scaledShapeHeight == 0) {
		va_end(args);
		return;
	}

	int shapeWidth = READ_LE_UINT16(src); src += 2;
	int scaledShapeWidth = (shapeWidth * scale_w) >> 8;
	if (scaledShapeWidth == 0) {
		va_end(args);
		return;
	}

	if (flags & 0x20) {
		x -= scaledShapeWidth >> 1;
		y -= scaledShapeHeight >> 1;
	}

	src += 3;

	uint16 frameSize = READ_LE_UINT16(src); src += 2;
	int colorTableColors = 0x10;

	if (shapeFlags & 4)
		colorTableColors = *src++;

	if (!(flags & 0x8000) && (shapeFlags & 1))
		table2 = src;

	if ((shapeFlags & 1) || (flags & 0x400))
		src += colorTableColors;

	if (!(shapeFlags & 2)) {
		decodeFrame4(src, _animBlockPtr, frameSize);
		src = _animBlockPtr;
	}

	int shapeSize = shapeWidth * shapeHeight;
	if (_decodeShapeBufferSize < shapeSize) {
		delete [] _decodeShapeBuffer;
		_decodeShapeBuffer = new uint8[shapeSize];
		_decodeShapeBufferSize = shapeSize;
	}
	if (!_decodeShapeBuffer) {
		_decodeShapeBufferSize = 0;
		va_end(args);
		return;
	}
	memset(_decodeShapeBuffer, 0, _decodeShapeBufferSize);
	uint8 *decodedShapeFrame = _decodeShapeBuffer;

	for (int j = 0; j < shapeHeight; ++j) {
		uint8 *dsbNextLine = decodedShapeFrame + shapeWidth;
		int count = shapeWidth;
		while (count > 0) {
			uint8 code = *src++;
			if (code != 0) {
				*decodedShapeFrame++ = code;
				--count;
			} else {
				code = *src++;
				decodedShapeFrame += code;
				count -= code;
			}
		}
		decodedShapeFrame = dsbNextLine;
	}

	uint16 sx1 = getScreenDim(sd)->sx << 3;
	uint16 sy1 = getScreenDim(sd)->sy;
	uint16 sx2 = sx1 + (getScreenDim(sd)->w << 3);
	uint16 sy2 = sy1 + getScreenDim(sd)->h;
	if (flags & 0x10) {
		x += sx1;
		y += sy1;
	}

	int x1, x2;
	if (x >= 0) {
		x1 = 0;
		if (x + scaledShapeWidth < sx2) {
			x2 = scaledShapeWidth;
		} else {
			x2 = sx2 - x;
		}
	} else {
		x2 = scaledShapeWidth;
		x1 = -x;
		x = 0;
		if (x2 > sx2) {
			x2 = sx2;
		}
	}

	int y1, y2;
	if (y >= 0) {
		y1 = 0;
		if (y + scaledShapeHeight < sy2) {
			y2 = scaledShapeHeight;
		} else {
			y2 = sy2 - y;
		}
	} else {
		y2 = scaledShapeHeight;
		y1 = -y;
		y = 0;
		if (y2 > sy2) {
			y2 = sy2;
		}
	}

	uint8 *dst = getPagePtr(page) + y * 320 + x;
	uint8 *dstStart = getPagePtr(page);
	if (page == 0 || page == 1)
		addDirtyRect(x, y, x2-x1, y2-y1);
	clearOverlayRect(page, x, y, x2-x1, y2-y1);

	int scaleYTable[200];
	for (y = y1; y < y2; ++y) {
		scaleYTable[y] = (y << 8) / scale_h;
	}
	int scaleXTable[320];
	for (x = x1; x < x2; ++x) {
		scaleXTable[x] = (x << 8) / scale_w;
	}

	const uint8 *shapeBuffer = _decodeShapeBuffer;
	if (flags & 0x02) {
		shapeBuffer += shapeWidth * (shapeHeight - 1);
	}
	if (flags & 0x01) {
		shapeBuffer += shapeWidth - 1;
	}

	for (y = y1; y < y2; ++y) {
		uint8 *dstNextLine = dst + 320;
		int j = scaleYTable[y];
		if (flags & 0x02) {
			j = -j;
		}
		for (x = x1; x < x2; ++x) {
			int xpos = scaleXTable[x];
			if (flags & 0x01)
				xpos = -xpos;
			uint8 color = shapeBuffer[j * shapeWidth + xpos];
			if (color != 0) {
				switch (ppc) {
				case 0:
					*dst = color;
					break;

				case 4:
					*dst = table2[color];
					break;

				case 8: {
						int layer = _shapePages[0][dst - dstStart] & 7;
						if (drawLayer < layer)
							color = _shapePages[1][dst - dstStart];
						*dst = color;
					} break;

				case 12: {
						int layer = _shapePages[0][dst - dstStart] & 7;
						if (drawLayer < layer)
							color = _shapePages[1][dst - dstStart];
						else
							color = table2[color];
						*dst = color;
					} break;

				default:
					warning("unhandled ppc: %d", ppc);
					break;
				}
			}
			++dst;
		}
		dst = dstNextLine;
	}
	va_end(args);
}

int Screen_v2::getRectSize(int w, int h) {
	if (w > 320 || h > 200)
		return 0;
	return w*h;
}

int Screen_v2::getLayer(int x, int y) {
	if (x < 0)
		x = 0;
	else if (x >= 320)
		x = 319;
	if (y < 0)
		y = 0;
	else if (y >= 144)
		y = 143;

	uint8 pixel = *(getCPagePtr(5) + y * 320 + x);
	pixel &= 0x7F;
	pixel >>= 3;

	if (pixel < 1)
		pixel = 1;
	else if (pixel > 15)
		pixel = 15;
	return pixel;
}

bool Screen_v2::isMouseVisible() const {
	return _mouseLockCount == 0;
}

void Screen_v2::setTextColorMap(const uint8 *cmap) {
	debugC(9, kDebugLevelScreen, "Screen_v2::setTextColorMap(%p)", (const void *)cmap);
	setTextColor(cmap, 0, 15);
}

} // end of namespace Kyra

