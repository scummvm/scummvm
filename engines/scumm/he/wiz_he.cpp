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

#ifdef ENABLE_HE

#include "common/archive.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/primitives.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/he/wiz_he.h"
#include "scumm/he/moonbase/moonbase.h"

namespace Scumm {

Wiz::Wiz(ScummEngine_v71he *vm) : _vm(vm) {
	_wizBufferIndex = 0;
	memset(&_wizBuffer, 0, sizeof(_wizBuffer));
	memset(&_polygons, 0, sizeof(_polygons));
	_cursorImage = false;
	_rectOverrideEnabled = false;
	_uses16BitColor = (_vm->_game.features & GF_16BIT_COLOR);
}

void Wiz::clearWizBuffer() {
	_wizBufferIndex = 0;
}

void Wiz::polygonClear() {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].flag == 1)
			_polygons[i].reset();
	}
}

void Wiz::polygonLoad(const uint8 *polData) {
	int slots = READ_LE_UINT32(polData);
	polData += 4;

	bool flag = 1;
	int id, points, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y;
	while (slots--) {
		id = READ_LE_UINT32(polData);
		points = READ_LE_UINT32(polData + 4);
		if (points != 4)
			error("Illegal polygon with %d points", points);
		vert1x = READ_LE_UINT32(polData + 8);
		vert1y = READ_LE_UINT32(polData + 12);
		vert2x = READ_LE_UINT32(polData + 16);
		vert2y = READ_LE_UINT32(polData + 20);
		vert3x = READ_LE_UINT32(polData + 24);
		vert3y = READ_LE_UINT32(polData + 28);
		vert4x = READ_LE_UINT32(polData + 32);
		vert4y = READ_LE_UINT32(polData + 36);

		polData += 40;
		polygonStore(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
	}
}

void Wiz::polygonStore(int id, bool flag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y) {
	WizPolygon *wp = nullptr;
	for (int i = 0; i < ARRAYSIZE(_polygons); ++i) {
		if (_polygons[i].id == 0) {
			wp = &_polygons[i];
			break;
		}
	}
	if (!wp) {
		error("Wiz::polygonStore: out of polygon slot, max = %d", ARRAYSIZE(_polygons));
	}

	wp->vert[0].x = vert1x;
	wp->vert[0].y = vert1y;
	wp->vert[1].x = vert2x;
	wp->vert[1].y = vert2y;
	wp->vert[2].x = vert3x;
	wp->vert[2].y = vert3y;
	wp->vert[3].x = vert4x;
	wp->vert[3].y = vert4y;
	wp->vert[4].x = vert1x;
	wp->vert[4].y = vert1y;
	wp->id = id;
	wp->numVerts = 5;
	wp->flag = flag;

	polygonCalcBoundBox(wp->vert, wp->numVerts, wp->bound);
}

void Wiz::polygonRotatePoints(Common::Point *pts, int num, int angle) {
	double alpha = angle * M_PI / 180.;
	double cos_alpha = cos(alpha);
	double sin_alpha = sin(alpha);

	for (int i = 0; i < num; ++i) {
		int16 x = pts[i].x;
		int16 y = pts[i].y;
		pts[i].x = (int16)(x * cos_alpha - y * sin_alpha);
		pts[i].y = (int16)(y * cos_alpha + x * sin_alpha);
	}
}

void Wiz::polygonTransform(int resNum, int state, int po_x, int po_y, int angle, int scale, Common::Point *pts) {
	int32 w, h;

	getWizImageDim(resNum, state, w, h);

	// set the transformation origin to the center of the image
	if (_vm->_game.heversion >= 99) {
		pts[0].x = pts[3].x = -(w / 2);
		pts[1].x = pts[2].x = w / 2 - 1;
		pts[0].y = pts[1].y = -(h / 2);
		pts[2].y = pts[3].y = h / 2 - 1;
	} else {
		pts[1].x = pts[2].x = w / 2 - 1;
		pts[0].x = pts[0].y = pts[1].y = pts[3].x = -(w / 2);
		pts[2].y = pts[3].y = h / 2 - 1;
	}

	// scale
	if (scale != 0 && scale != 256) {
		for (int i = 0; i < 4; ++i) {
			pts[i].x = pts[i].x * scale / 256;
			pts[i].y = pts[i].y * scale / 256;
		}
	}

	// rotate
	if (angle != 0)
		polygonRotatePoints(pts, 4, angle);

	// translate
	for (int i = 0; i < 4; ++i) {
		pts[i].x += po_x;
		pts[i].y += po_y;
	}
}

void Wiz::polygonCalcBoundBox(Common::Point *vert, int numVerts, Common::Rect &bound) {
	bound.left = 10000;
	bound.top = 10000;
	bound.right = -10000;
	bound.bottom = -10000;

	// compute bounding box
	for (int j = 0; j < numVerts; j++) {
		Common::Rect r(vert[j].x, vert[j].y, vert[j].x + 1, vert[j].y + 1);
		bound.extend(r);
	}
}

void Wiz::polygonErase(int fromId, int toId) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].id >= fromId && _polygons[i].id <= toId)
			_polygons[i].reset();
	}
}

int Wiz::polygonHit(int id, int x, int y) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if ((id == 0 || _polygons[i].id == id) && _polygons[i].bound.contains(x, y)) {
			if (polygonContains(_polygons[i], x, y)) {
				return _polygons[i].id;
			}
		}
	}
	return 0;
}

bool Wiz::polygonDefined(int id) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++)
		if (_polygons[i].id == id)
			return true;
	return false;
}

bool Wiz::polygonContains(const WizPolygon &pol, int x, int y) {
	int pi = pol.numVerts - 1;
	bool diry = (y < pol.vert[pi].y);
	bool curdir;
	bool r = false;

	for (int i = 0; i < pol.numVerts; i++) {
		curdir = (y < pol.vert[i].y);

		if (curdir != diry) {
			if (((pol.vert[pi].y - pol.vert[i].y) * (pol.vert[i].x - x) <
				 (pol.vert[pi].x - pol.vert[i].x) * (pol.vert[i].y - y)) == diry)
				r = !r;
		}

		pi = i;
		diry = curdir;
	}

	// HE80+
	int a, b;
	pi = pol.numVerts - 1;
	if (r == 0) {
		for (int i = 0; i < pol.numVerts; i++) {
			if (pol.vert[i].y == y && pol.vert[i].y == pol.vert[pi].y) {

				a = pol.vert[i].x;
				b = pol.vert[pi].x;

				if (pol.vert[i].x >= pol.vert[pi].x)
					a = pol.vert[pi].x;

				if (pol.vert[i].x > pol.vert[pi].x)
					b = pol.vert[i].x;

				if (x >= a && x <= b)
					return 1;

			} else if (pol.vert[i].x == x && pol.vert[i].x == pol.vert[pi].x) {

				a = pol.vert[i].y;
				b = pol.vert[i].y;

				if (pol.vert[i].y >= pol.vert[pi].y)
					a = pol.vert[pi].y;

				if (pol.vert[i].y <= pol.vert[pi].y)
					b = pol.vert[pi].y;

				if (y >= a && y <= b)
					return 1;
			}
			pi = i;
		}
	}

	return r;
}

void Wiz::copyAuxImage(uint8 *dst1, uint8 *dst2, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, uint8 bitDepth) {
	assert(bitDepth == 1);

	Common::Rect dstRect(srcx, srcy, srcx + srcw, srcy + srch);
	dstRect.clip(dstw, dsth);

	int rw = dstRect.width();
	int rh = dstRect.height();
	if (rh <= 0 || rw <= 0)
		return;

	uint8 *dst1Ptr = dst1 + dstRect.top * dstw + dstRect.left;
	uint8 *dst2Ptr = dst2 + dstRect.top * dstw + dstRect.left;
	const uint8 *dataPtr = src;

	while (rh--) {
		uint16 off = READ_LE_UINT16(dataPtr); dataPtr += 2;
		const uint8 *dataPtrNext = off + dataPtr;
		uint8 *dst1PtrNext = dst1Ptr + dstw;
		uint8 *dst2PtrNext = dst2Ptr + dstw;
		if (off != 0) {
			int w = rw;
			while (w > 0) {
				uint8 code = *dataPtr++;
				if (code & 1) {
					code >>= 1;
					dst1Ptr += code;
					dst2Ptr += code;
					w -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					w -= code;
					if (w >= 0) {
						memset(dst1Ptr, *dataPtr++, code);
						dst1Ptr += code;
						dst2Ptr += code;
					} else {
						code += w;
						memset(dst1Ptr, *dataPtr, code);
					}
				} else {
					code = (code >> 2) + 1;
					w -= code;
					if (w >= 0) {
						memcpy(dst1Ptr, dst2Ptr, code);
						dst1Ptr += code;
						dst2Ptr += code;
					} else {
						code += w;
						memcpy(dst1Ptr, dst2Ptr, code);
					}
				}
			}
		}
		dataPtr = dataPtrNext;
		dst1Ptr = dst1PtrNext;
		dst2Ptr = dst2PtrNext;
	}
}

static bool calcClipRects(int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h, const Common::Rect *rect, Common::Rect &srcRect, Common::Rect &dstRect) {
	srcRect = Common::Rect(src_w, src_h);
	dstRect = Common::Rect(src_x, src_y, src_x + src_w, src_y + src_h);
	Common::Rect r3;
	int diff;

	if (rect) {
		r3 = *rect;
		Common::Rect r4(dst_w, dst_h);
		if (r3.intersects(r4)) {
			r3.clip(r4);
		} else {
			return false;
		}
	} else {
		r3 = Common::Rect(dst_w, dst_h);
	}
	diff = dstRect.left - r3.left;
	if (diff < 0) {
		srcRect.left -= diff;
		dstRect.left -= diff;
	}
	diff = dstRect.right - r3.right;
	if (diff > 0) {
		srcRect.right -= diff;
		dstRect.right -= diff;
	}
	diff = dstRect.top - r3.top;
	if (diff < 0) {
		srcRect.top -= diff;
		dstRect.top -= diff;
	}
	diff = dstRect.bottom - r3.bottom;
	if (diff > 0) {
		srcRect.bottom -= diff;
		dstRect.bottom -= diff;
	}

	return srcRect.isValidRect() && dstRect.isValidRect();
}

void Wiz::writeColor(uint8 *dstPtr, int dstType, uint16 color) {
	switch (dstType) {
	case kDstCursor:
	case kDstScreen:
		WRITE_UINT16(dstPtr, color);
		break;
	case kDstMemory:
	case kDstResource:
		WRITE_LE_UINT16(dstPtr, color);
		break;
	default:
		error("writeColor: Unknown dstType %d", dstType);
	}
}

#ifdef USE_RGB_COLOR
void Wiz::copy16BitWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *xmapPtr) {
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		dst += r2.top * dstPitch + r2.left * 2;
		if (flags & kWRFFlipY) {
			const int dy = (srcy < 0) ? srcy : (srch - r1.height());
			r1.translate(0, dy);
		}
		if (flags & kWRFFlipX) {
			const int dx = (srcx < 0) ? srcx : (srcw - r1.width());
			r1.translate(dx, 0);
		}
		if (xmapPtr) {
			decompress16BitWizImage<kWizXMap>(dst, dstPitch, dstType, src, r1, flags, xmapPtr);
		} else {
			decompress16BitWizImage<kWizCopy>(dst, dstPitch, dstType, src, r1, flags);
		}
	}
}
#endif

void Wiz::copyWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitDepth) {
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		dst += r2.top * dstPitch + r2.left * bitDepth;
		if (flags & kWRFFlipY) {
			const int dy = (srcy < 0) ? srcy : (srch - r1.height());
			r1.translate(0, dy);
		}
		if (flags & kWRFFlipX) {
			const int dx = (srcx < 0) ? srcx : (srcw - r1.width());
			r1.translate(dx, 0);
		}
		if (xmapPtr) {
			decompressWizImage<kWizXMap>(dst, dstPitch, dstType, src, r1, flags, palPtr, xmapPtr, bitDepth);
		} else if (palPtr) {
			decompressWizImage<kWizRMap>(dst, dstPitch, dstType, src, r1, flags, palPtr, nullptr, bitDepth);
		} else {
			decompressWizImage<kWizCopy>(dst, dstPitch, dstType, src, r1, flags, nullptr, nullptr, bitDepth);
		}
	}
}

static void decodeWizMask(uint8 *&dst, uint8 &mask, int w, int maskType) {
	switch (maskType) {
	case 0:
		while (w--) {
			mask >>= 1;
			if (mask == 0) {
				mask = 0x80;
				++dst;
			}
		}
		break;
	case 1:
		while (w--) {
			*dst &= ~mask;
			mask >>= 1;
			if (mask == 0) {
				mask = 0x80;
				++dst;
			}
		}
		break;
	case 2:
		while (w--) {
			*dst |= mask;
			mask >>= 1;
			if (mask == 0) {
				mask = 0x80;
				++dst;
			}
		}
		break;
	default:
		break;
	}
}

#ifdef USE_RGB_COLOR
void Wiz::copyMaskWizImage(uint8 *dst, const uint8 *src, const uint8 *mask, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr) {
	Common::Rect srcRect, dstRect;
	if (!calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, srcRect, dstRect)) {
		return;
	}
	dst += dstRect.top * dstPitch + dstRect.left * 2;
	if (flags & kWRFFlipY) {
		const int dy = (srcy < 0) ? srcy : (srch - srcRect.height());
		srcRect.translate(0, dy);
	}
	if (flags & kWRFFlipX) {
		const int dx = (srcx < 0) ? srcx : (srcw - srcRect.width());
		srcRect.translate(dx, 0);
	}

	const uint8 *dataPtr, *dataPtrNext;
	const uint8 *maskPtr, *maskPtrNext;
	uint8 code, *dstPtr, *dstPtrNext;
	int h, w, dstInc;

	dataPtr = src;
	dstPtr = dst;
	maskPtr = mask;

	// Skip over the first 'srcRect->top' lines in the data
	dataPtr += dstRect.top * dstPitch + dstRect.left * 2;

	h = dstRect.height();
	w = dstRect.width();
	if (h <= 0 || w <= 0)
		return;

	dstInc = 2;
	if (flags & kWRFFlipX) {
		dstPtr += (w - 1) * 2;
		dstInc = -2;
	}

	while (h--) {
		w = dstRect.width();
		uint16 lineSize = READ_LE_UINT16(maskPtr); maskPtr += 2;
		dataPtrNext = dataPtr + dstPitch;
		dstPtrNext = dstPtr + dstPitch;
		maskPtrNext = maskPtr + lineSize;
		if (lineSize != 0) {
			while (w > 0) {
				code = *maskPtr++;
				if (code & 1) {
					code >>= 1;
					dataPtr += dstInc * code;
					dstPtr += dstInc * code;
					w -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						if (*maskPtr != 5)
							write16BitColor<kWizCopy>(dstPtr, dataPtr, dstType, palPtr);
						dataPtr += 2;
						dstPtr += dstInc;
					}
					maskPtr++;
				} else {
					code = (code >> 2) + 1;
					w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						if (*maskPtr != 5)
							write16BitColor<kWizCopy>(dstPtr, dataPtr, dstType, palPtr);
						dataPtr += 2;
						dstPtr += dstInc;
						maskPtr++;
					}
				}
			}
		}
		dataPtr = dataPtrNext;
		dstPtr = dstPtrNext;
		maskPtr = maskPtrNext;
	}
}
#endif

void Wiz::copyWizImageWithMask(uint8 *dst, const uint8 *src, int dstPitch, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int maskT, int maskP) {
	Common::Rect srcRect, dstRect;
	if (!calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, srcRect, dstRect)) {
		return;
	}
	dstPitch /= 8;
	dst += dstRect.top * dstPitch + dstRect.left / 8;

	const uint8 *dataPtr, *dataPtrNext;
	uint8 code, mask, *dstPtr, *dstPtrNext;
	int h, w, xoff;
	uint16 off;

	dstPtr = dst;
	dataPtr = src;

	// Skip over the first 'srcRect->top' lines in the data
	h = srcRect.top;
	while (h--) {
		dataPtr += READ_LE_UINT16(dataPtr) + 2;
	}
	h = srcRect.height();
	w = srcRect.width();
	if (h <= 0 || w <= 0)
		return;

	while (h--) {
		xoff = srcRect.left;
		w = srcRect.width();
		mask = revBitMask(dstRect.left & 7);
		off = READ_LE_UINT16(dataPtr); dataPtr += 2;
		dstPtrNext = dstPtr + dstPitch;
		dataPtrNext = dataPtr + off;
		if (off != 0) {
			while (w > 0) {
				code = *dataPtr++;
				if (code & 1) {
					code >>= 1;
					if (xoff > 0) {
						xoff -= code;
						if (xoff >= 0)
							continue;

						code = -xoff;
					}
					decodeWizMask(dstPtr, mask, code, maskT);
					w -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						++dataPtr;
						if (xoff >= 0)
							continue;

						code = -xoff;
						--dataPtr;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					decodeWizMask(dstPtr, mask, code, maskP);
					dataPtr++;
				} else {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						dataPtr += code;
						if (xoff >= 0)
							continue;

						code = -xoff;
						dataPtr += xoff;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					decodeWizMask(dstPtr, mask, code, maskP);
					dataPtr += code;
				}
			}
		}
		dataPtr = dataPtrNext;
		dstPtr = dstPtrNext;
	}
}

#ifdef USE_RGB_COLOR
void Wiz::copyRaw16BitWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, int transColor) {
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		if (flags & kWRFFlipX) {
			int l = r1.left;
			int r = r1.right;
			r1.left = srcw - r;
			r1.right = srcw - l;
		}
		if (flags & kWRFFlipY) {
			int t = r1.top;
			int b = r1.bottom;
			r1.top = srch - b;
			r1.bottom = srch - t;
		}
		int h = r1.height();
		int w = r1.width();
		src += (r1.top * srcw + r1.left) * 2;
		dst += r2.top * dstPitch + r2.left * 2;
		while (h--) {
			for (int i = 0; i < w; ++ i) {
				uint16 col = READ_LE_UINT16(src + 2 * i);
				if (transColor == -1 || transColor != col) {
					writeColor(dst + i * 2, dstType, col);
				}
			}
			src += srcw * 2;
			dst += dstPitch;
		}
	}
}
#endif

void Wiz::copyRawWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, int transColor, uint8 bitDepth) {
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		if (flags & kWRFFlipX) {
			int l = r1.left;
			int r = r1.right;
			r1.left = srcw - r;
			r1.right = srcw - l;
		}
		if (flags & kWRFFlipY) {
			int t = r1.top;
			int b = r1.bottom;
			r1.top = srch - b;
			r1.bottom = srch - t;
		}
		int h = r1.height();
		int w = r1.width();
		src += r1.top * srcw + r1.left;
		dst += r2.top * dstPitch + r2.left * bitDepth;
		if (palPtr) {
			decompressRawWizImage<kWizRMap>(dst, dstPitch, dstType, src, srcw, w, h, transColor, palPtr, bitDepth);
		} else {
			decompressRawWizImage<kWizCopy>(dst, dstPitch, dstType, src, srcw, w, h, transColor, nullptr, bitDepth);
		}
	}
}

#ifdef USE_RGB_COLOR
template<int type>
void Wiz::write16BitColor(uint8 *dstPtr, const uint8 *dataPtr, int dstType, const uint8 *xmapPtr) {
	uint16 col = READ_LE_UINT16(dataPtr);
	if (type == kWizXMap) {
		uint16 srcColor = (col >> 1) & 0x7DEF;
		uint16 dstColor = (READ_UINT16(dstPtr) >> 1) & 0x7DEF;
		uint16 newColor = srcColor + dstColor;
		writeColor(dstPtr, dstType, newColor);
	}
	if (type == kWizCopy) {
		writeColor(dstPtr, dstType, col);
	}
}

template<int type>
void Wiz::decompress16BitWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *xmapPtr) {
	const uint8 *dataPtr, *dataPtrNext;
	uint8 code;
	uint8 *dstPtr, *dstPtrNext;
	int h, w, xoff, dstInc;

	if (type == kWizXMap) {
		assert(xmapPtr != 0);
	}

	dstPtr = dst;
	dataPtr = src;

	// Skip over the first 'srcRect->top' lines in the data
	h = srcRect.top;
	while (h--) {
		dataPtr += READ_LE_UINT16(dataPtr) + 2;
	}
	h = srcRect.height();
	w = srcRect.width();
	if (h <= 0 || w <= 0)
		return;

	if (flags & kWRFFlipY) {
		dstPtr += (h - 1) * dstPitch;
		dstPitch = -dstPitch;
	}
	dstInc = 2;
	if (flags & kWRFFlipX) {
		dstPtr += (w - 1) * 2;
		dstInc = -2;
	}

	while (h--) {
		xoff = srcRect.left;
		w = srcRect.width();
		uint16 lineSize = READ_LE_UINT16(dataPtr); dataPtr += 2;
		dstPtrNext = dstPtr + dstPitch;
		dataPtrNext = dataPtr + lineSize;
		if (lineSize != 0) {
			while (w > 0) {
				code = *dataPtr++;
				if (code & 1) {
					code >>= 1;
					if (xoff > 0) {
						xoff -= code;
						if (xoff >= 0)
							continue;

						code = -xoff;
					}
					dstPtr += dstInc * code;
					w -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						dataPtr += 2;
						if (xoff >= 0)
							continue;

						code = -xoff;
						dataPtr -= 2;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						write16BitColor<type>(dstPtr, dataPtr, dstType, xmapPtr);
						dstPtr += dstInc;
					}
					dataPtr += 2;
				} else {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						dataPtr += code * 2;
						if (xoff >= 0)
							continue;

						code = -xoff;
						dataPtr += xoff * 2;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						write16BitColor<type>(dstPtr, dataPtr, dstType, xmapPtr);
						dataPtr += 2;
						dstPtr += dstInc;
					}
				}
			}
		}
		dataPtr = dataPtrNext;
		dstPtr = dstPtrNext;
	}
}
#endif

template<int type>
void Wiz::write8BitColor(uint8 *dstPtr, const uint8 *dataPtr, int dstType, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitDepth) {
	if (bitDepth == 2) {
		if (type == kWizXMap) {
			uint16 color = READ_LE_UINT16(palPtr + *dataPtr * 2);
			uint16 srcColor = (color >> 1) & 0x7DEF;
			uint16 dstColor = (READ_UINT16(dstPtr) >> 1) & 0x7DEF;
			uint16 newColor = srcColor + dstColor;
			writeColor(dstPtr, dstType, newColor);
		}
		if (type == kWizRMap) {
			writeColor(dstPtr, dstType, READ_LE_UINT16(palPtr + *dataPtr * 2));
		}
		if (type == kWizCopy) {
			writeColor(dstPtr, dstType, *dataPtr);
		}
	} else {
		if (type == kWizXMap) {
			*dstPtr = xmapPtr[*dataPtr * 256 + *dstPtr];
		}
		if (type == kWizRMap) {
			*dstPtr = palPtr[*dataPtr];
		}
		if (type == kWizCopy) {
			*dstPtr = *dataPtr;
		}
	}
}

template<int type>
void Wiz::decompressWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitDepth) {
	const uint8 *dataPtr, *dataPtrNext;
	uint8 code, *dstPtr, *dstPtrNext;
	int h, w, xoff, dstInc;

	if (type == kWizXMap) {
		assert(xmapPtr != 0);
	}
	if (type == kWizRMap) {
		assert(palPtr != 0);
	}

	dstPtr = dst;
	dataPtr = src;

	// Skip over the first 'srcRect->top' lines in the data
	h = srcRect.top;
	while (h--) {
		dataPtr += READ_LE_UINT16(dataPtr) + 2;
	}
	h = srcRect.height();
	w = srcRect.width();
	if (h <= 0 || w <= 0)
		return;

	if (flags & kWRFFlipY) {
		dstPtr += (h - 1) * dstPitch;
		dstPitch = -dstPitch;
	}
	dstInc = bitDepth;
	if (flags & kWRFFlipX) {
		dstPtr += (w - 1) * bitDepth;
		dstInc = -bitDepth;
	}

	while (h--) {
		xoff = srcRect.left;
		w = srcRect.width();
		uint16 lineSize = READ_LE_UINT16(dataPtr); dataPtr += 2;
		dstPtrNext = dstPtr + dstPitch;
		dataPtrNext = dataPtr + lineSize;
		if (lineSize != 0) {
			while (w > 0) {
				code = *dataPtr++;
				if (code & 1) {
					code >>= 1;
					if (xoff > 0) {
						xoff -= code;
						if (xoff >= 0)
							continue;

						code = -xoff;
					}
					dstPtr += dstInc * code;
					w -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						++dataPtr;
						if (xoff >= 0)
							continue;

						code = -xoff;
						--dataPtr;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						write8BitColor<type>(dstPtr, dataPtr, dstType, palPtr, xmapPtr, bitDepth);
						dstPtr += dstInc;
					}
					dataPtr++;
				} else {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						dataPtr += code;
						if (xoff >= 0)
							continue;

						code = -xoff;
						dataPtr += xoff;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						write8BitColor<type>(dstPtr, dataPtr, dstType, palPtr, xmapPtr, bitDepth);
						dataPtr++;
						dstPtr += dstInc;
					}
				}
			}
		}
		dataPtr = dataPtrNext;
		dstPtr = dstPtrNext;
	}
}

// NOTE: These templates are used outside this file. We don't want the compiler to optimize them away, so we need to explicitely instantiate them.
template void Wiz::decompressWizImage<kWizXMap>(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitDepth);
template void Wiz::decompressWizImage<kWizRMap>(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitDepth);
template void Wiz::decompressWizImage<kWizCopy>(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitDepth);

template<int type>
void Wiz::decompressRawWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, int srcPitch, int w, int h, int transColor, const uint8 *palPtr, uint8 bitDepth) {
	if (type == kWizRMap) {
		assert(palPtr != 0);
	}

	if (w <= 0 || h <= 0) {
		return;
	}
	while (h--) {
		for (int i = 0; i < w; ++i) {
			uint8 col = src[i];
			if (transColor == -1 || transColor != col) {
				if (type == kWizRMap) {
					if (bitDepth == 2) {
						writeColor(dst + i * 2, dstType, READ_LE_UINT16(palPtr + col * 2));
					} else {
						dst[i] = palPtr[col];
					}
				}
				if (type == kWizCopy) {
					if (bitDepth == 2) {
						writeColor(dst + i * 2, dstType, col);
					} else {
						dst[i] = col;
					}
				}
			}
		}
		src += srcPitch;
		dst += dstPitch;
	}
}

int Wiz::isPixelNonTransparent(const uint8 *data, int x, int y, int w, int h, uint8 bitDepth) {
	if (x < 0 || x >= w || y < 0 || y >= h) {
		return 0;
	}
	while (y != 0) {
		data += READ_LE_UINT16(data) + 2;
		--y;
	}
	uint16 off = READ_LE_UINT16(data); data += 2;
	if (off == 0) {
		return 0;
	}
	while (x > 0) {
		uint8 code = *data++;
		if (code & 1) {
			code >>= 1;
			if (code > x) {
				return 0;
			}
			x -= code;
		} else if (code & 2) {
			code = (code >> 2) + 1;
			if (code > x) {
				return 1;
			}
			x -= code;
			data += bitDepth;
		} else {
			code = (code >> 2) + 1;
			if (code > x) {
				return 1;
			}
			x -= code;
			data += code * bitDepth;
		}
	}

	if (bitDepth == 2)
		return (~READ_LE_UINT16(data)) & 1;
	else
		return (~data[0]) & 1;
}

uint16 Wiz::getWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 bitDepth, uint16 color) {
	if (x < 0 || x >= w || y < 0 || y >= h) {
		return color;
	}
	while (y != 0) {
		data += READ_LE_UINT16(data) + 2;
		--y;
	}
	uint16 off = READ_LE_UINT16(data); data += 2;
	if (off == 0) {
		return color;
	}
	while (x > 0) {
		uint8 code = *data++;
		if (code & 1) {
			code >>= 1;
			if (code > x) {
				return color;
			}
			x -= code;
		} else if (code & 2) {
			code = (code >> 2) + 1;
			if (code > x) {
				return (bitDepth == 2) ? READ_LE_UINT16(data) : data[0];
			}
			x -= code;
			data += bitDepth;
		} else {
			code = (code >> 2) + 1;
			if (code > x) {
				return (bitDepth == 2) ? READ_LE_UINT16(data + x) : data[x];
			}
			x -= code;
			data += code * bitDepth;
		}
	}

	if (bitDepth == 2)
		return (READ_LE_UINT16(data) & 1) ? color : READ_LE_UINT16(data + 2);
	else
		return (data[0] & 1) ? color : data[1];

}

uint16 Wiz::getRawWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 bitDepth, uint16 color) {
	if (x < 0 || x >= w || y < 0 || y >= h) {
		return color;
	}
	if (bitDepth == 2)
		return READ_LE_UINT16(data + (y * w + x) * 2);
	else
		return data[y * w + x];
}

void Wiz::computeWizHistogram(uint32 *histogram, const uint8 *data, const Common::Rect &rCapt) {
	int h = rCapt.top;
	while (h--) {
		data += READ_LE_UINT16(data) + 2;
	}

	h = rCapt.height();
	while (h--) {
		uint16 off = READ_LE_UINT16(data); data += 2;
		if (off != 0) {
			const uint8 *p = data;
			int xoffs = rCapt.left;
			int w = rCapt.width();
			uint8 code;
			while (xoffs > 0) {
				code = *p++;
				if (code & 1) {
					code >>= 1;
					if (code > xoffs) {
						code -= xoffs;
						w -= code;
						break;
					}
					xoffs -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					if (code > xoffs) {
						code -= xoffs;
						goto dec_sub2;
					}
					xoffs -= code;
					p++;
				} else {
					code = (code >> 2) + 1;
					if (code > xoffs) {
						code -= xoffs;
						p += xoffs;
						goto dec_sub3;
					}
					xoffs -= code;
					p += code;
				}
			}
			while (w > 0) {
				code = *p++;
				if (code & 1) {
					code >>= 1;
					w -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
dec_sub2:				w -= code;
					if (w < 0) {
						code += w;
					}
					histogram[*p++] += code;
				} else {
					code = (code >> 2) + 1;
dec_sub3:				w -= code;
					if (w < 0) {
						code += w;
					}
					int n = code;
					while (n--) {
						++histogram[*p++];
					}
				}
			}
			data += off;
		}
	}
}

void Wiz::computeRawWizHistogram(uint32 *histogram, const uint8 *data, int srcPitch, const Common::Rect &rCapt) {
	data += rCapt.top * srcPitch + rCapt.left;
	int iw = rCapt.width();
	int ih = rCapt.height();
	while (ih--) {
		for (int i = 0; i < iw; ++i) {
			++histogram[data[i]];
		}
		data += srcPitch;
	}
}

#ifdef USE_RGB_COLOR
static int wizPackType2(uint8 *dst, const uint8 *src, int srcPitch, const Common::Rect& rCapt) {
	debug(9, "wizPackType2([%d,%d,%d,%d])", rCapt.left, rCapt.top, rCapt.right, rCapt.bottom);
	int w = rCapt.width();
	int h = rCapt.height();
	int size = w * h * 2;
	if (dst) {
		src += rCapt.top * srcPitch + rCapt.left * 2;
		while (h--) {
			for (int i = 0; i < w; i++)
				WRITE_LE_UINT16(dst + i * 2, READ_UINT16(src + i * 2));
			dst += w * 2;
			src += srcPitch;
		}
	}
	return size;
}
#endif

static int wizPackType1(uint8 *dst, const uint8 *src, int srcPitch, const Common::Rect& rCapt, uint8 transColor) {
	debug(9, "wizPackType1(%d, [%d,%d,%d,%d])", transColor, rCapt.left, rCapt.top, rCapt.right, rCapt.bottom);
	src += rCapt.top * srcPitch + rCapt.left;
	int w = rCapt.width();
	int h = rCapt.height();
	int dataSize = 0;
	while (h--) {
		uint8 *dstLine = dst;
		if (dst) {
			dst += 2;
		}
		uint8 diffBuffer[0x40];
		int runCountSame = 0;
		int runCountDiff = 0;
		uint8 prevColor = src[0];
		for (int i = 1; i < w; ) {
			uint8 color = src[i++];
			if (i == 2) {
				if (prevColor == color) {
					runCountSame = 1;
				} else {
					diffBuffer[0] = prevColor;
					runCountDiff = 1;
				}
			}
			if (prevColor == color) {
				if (runCountDiff != 0) {
					runCountSame = 1;
					if (runCountDiff > 1) {
						--runCountDiff;
						if (dst) {
							*dst++ = ((runCountDiff - 1) << 2) | 0;
							memcpy(dst, diffBuffer, runCountDiff);
							dst += runCountDiff;
						}
						dataSize += runCountDiff + 1;
					}
					runCountDiff = 0;
				}
				++runCountSame;
				if (prevColor == transColor) {
					if (runCountSame == 0x7F) {
						if (dst) {
							*dst++ = (runCountSame << 1) | 1;
						}
						++dataSize;
						runCountSame = 0;
					}
				} else {
					if (runCountSame == 0x40) {
						if (dst) {
							*dst++ = ((runCountSame - 1) << 2) | 2;
							*dst++ = prevColor;
						}
						dataSize += 2;
						runCountSame = 0;
					}
				}
			} else {
				if (runCountSame != 0) {
					if (prevColor == transColor) {
						if (dst) {
							*dst++ = (runCountSame << 1) | 1;
						}
						++dataSize;
					} else {
						if (dst) {
							*dst++ = ((runCountSame - 1) << 2) | 2;
							*dst++ = prevColor;
						}
						dataSize += 2;
					}
					runCountSame = 0;
				}
				assert(runCountDiff < ARRAYSIZE(diffBuffer));
				diffBuffer[runCountDiff++] = color;
				if (runCountDiff == 0x40) {
					if (dst) {
						*dst++ = ((runCountDiff - 1) << 2) | 0;
						memcpy(dst, diffBuffer, runCountDiff);
						dst += runCountDiff;
					}
					dataSize += runCountDiff + 1;
					runCountDiff = 0;
				}
			}
			prevColor = color;
		}
		if (runCountSame != 0) {
			if (prevColor == transColor) {
				if (dst) {
					*dst++ = (runCountSame << 1) | 1;
				}
				++dataSize;
			} else {
				if (dst) {
					*dst++ = ((runCountSame - 1) << 2) | 2;
					*dst++ = prevColor;
				}
				dataSize += 2;
			}
		}
		if (runCountDiff != 0) {
			if (dst) {
				*dst++ = ((runCountDiff - 1) << 2) | 0;
				memcpy(dst, diffBuffer, runCountDiff);
				dst += runCountDiff;
			}
			dataSize += runCountDiff + 1;
		}
		if (dst) {
			WRITE_LE_UINT16(dstLine, dst - dstLine - 2);
		}
		dataSize += 2;
		src += srcPitch;
	}
	return dataSize;
}

static int wizPackType0(uint8 *dst, const uint8 *src, int srcPitch, const Common::Rect& rCapt) {
	debug(9, "wizPackType0([%d,%d,%d,%d])", rCapt.left, rCapt.top, rCapt.right, rCapt.bottom);
	int w = rCapt.width();
	int h = rCapt.height();
	int size = w * h;
	if (dst) {
		src += rCapt.top * srcPitch + rCapt.left;
		while (h--) {
			memcpy(dst, src, w);
			dst += w;
			src += srcPitch;
		}
	}
	return size;
}

void Wiz::processWizImageCaptureCmd(const WizImageCommand *params) {
	takeAWiz(params->image, params->box, (params->flags & kWRFBackground) != 0, params->compressionType);
}

void Wiz::takeAWiz(int resNum, const Common::Rect &r, bool backBuffer, int compType) {
	uint8 *src = nullptr;
	VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
	if (backBuffer) {
		src = pvs->getBackPixels(0, 0);
	} else {
		src = pvs->getPixels(0, 0);
	}
	captureImage(src, pvs->pitch, pvs->w, pvs->h, resNum, r, compType);
}

void Wiz::captureImage(uint8 *src, int srcPitch, int srcw, int srch, int resNum, const Common::Rect& r, int compType) {
	debug(7, "captureImage(%d, %d, [%d,%d,%d,%d])", resNum, compType, r.left, r.top, r.right, r.bottom);
	Common::Rect rCapt(srcw, srch);
	if (rCapt.intersects(r)) {
		rCapt.clip(r);
		const uint8 *palPtr;
		if (_vm->_game.heversion >= 99) {
			palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
		} else {
			palPtr = _vm->_currentPalette;
		}

		int w = rCapt.width();
		int h = rCapt.height();
		int transColor = (_vm->VAR_WIZ_TRANSPARENT_COLOR != 0xFF) ? _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) : 5;

		if (_vm->_game.features & GF_16BIT_COLOR)
			compType = 2;

		// compute compressed size
		int dataSize = 0;
		int headerSize = palPtr ? 1080 : 36;
		switch (compType) {
		case 0:
			dataSize = wizPackType0(0, src, srcPitch, rCapt);
			break;
		case 1:
			dataSize = wizPackType1(0, src, srcPitch, rCapt, transColor);
			break;
#ifdef USE_RGB_COLOR
		case 2:
			dataSize = wizPackType2(0, src, srcPitch, rCapt);
			break;
#endif
		default:
			error("unhandled compression type %d", compType);
			break;
		}

		// alignment
		dataSize = (dataSize + 1) & ~1;
		int wizSize = headerSize + dataSize;
		// write header
		uint8 *wizImg = _vm->_res->createResource(rtImage, resNum, dataSize + headerSize);
		WRITE_BE_UINT32(wizImg + 0x00, 'AWIZ');
		WRITE_BE_UINT32(wizImg + 0x04, wizSize);
		WRITE_BE_UINT32(wizImg + 0x08, 'WIZH');
		WRITE_BE_UINT32(wizImg + 0x0C, 0x14);
		WRITE_LE_UINT32(wizImg + 0x10, compType);
		WRITE_LE_UINT32(wizImg + 0x14, w);
		WRITE_LE_UINT32(wizImg + 0x18, h);
		int curSize = 0x1C;
		if (palPtr) {
			WRITE_BE_UINT32(wizImg + 0x1C, 'RGBS');
			WRITE_BE_UINT32(wizImg + 0x20, 0x308);
			memcpy(wizImg + 0x24, palPtr, 0x300);
			WRITE_BE_UINT32(wizImg + 0x324, 'RMAP');
			WRITE_BE_UINT32(wizImg + 0x328, 0x10C);
			WRITE_BE_UINT32(wizImg + 0x32C, 0);
			curSize = 0x330;
			for (int i = 0; i < 256; ++i) {
				wizImg[curSize] = i;
				++curSize;
			}
		}
		WRITE_BE_UINT32(wizImg + curSize + 0x0, 'WIZD');
		WRITE_BE_UINT32(wizImg + curSize + 0x4, dataSize + 8);
		curSize += 8;

		// write compressed data
		switch (compType) {
		case 0:
			wizPackType0(wizImg + headerSize, src, srcPitch, rCapt);
			break;
		case 1:
			wizPackType1(wizImg + headerSize, src, srcPitch, rCapt, transColor);
			break;
#ifdef USE_RGB_COLOR
		case 2:
			wizPackType2(wizImg + headerSize, src, srcPitch, rCapt);
			break;
#endif
		default:
			break;
		}
	}
	_vm->_res->setModified(rtImage, resNum);
}

void Wiz::simpleDrawAWiz(int image, int state, int x, int y, int flags) {
	if (!_vm->_fullRedraw) {
		if (flags & kWRFIsPolygon) {
			drawWizPolygon(image, state, x, flags, 0, 0, 0);
		} else {
			const Common::Rect *r = nullptr;
			drawWizImage(image, state, 0, 0, x, y, 0, 0, 0, r, flags, 0, _vm->getHEPaletteSlot(0), 0);
		}
	} else {
		bufferAWiz(image, state, x, y, 0, flags, 0, 0, 0);
	}
}

void Wiz::bufferAWiz(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, int whichPalette) {
	assert(_wizBufferIndex < ARRAYSIZE(_wizBuffer));
	WizBufferElement *wi = &_wizBuffer[_wizBufferIndex];
	wi->image = image;
	wi->x = x;
	wi->y = y;
	wi->z = z;
	wi->state = state;
	wi->flags = flags;
	wi->shadow = optionalShadowImage;
	wi->zbuffer = optionalZBufferImage;
	wi->palette = whichPalette;
	++_wizBufferIndex;
}

uint8 *Wiz::drawWizImage(int resNum, int state, int maskNum, int maskState, int x1, int y1, int zorder, int shadow, int zbuffer, const Common::Rect *clipBox, int flags, int dstResNum, const uint8 *palPtr, uint32 conditionBits) {
	debug(7, "drawWizImage(resNum %d, state %d maskNum %d maskState %d x1 %d y1 %d flags 0x%X zorder %d shadow %d zbuffer %d dstResNum %d conditionBits: 0x%x)", resNum, state, maskNum, maskState, x1, y1, flags, zorder, shadow, zbuffer, dstResNum, conditionBits);
	uint8 *dataPtr;
	uint8 *dst = nullptr;

	const uint8 *xmapPtr = nullptr;
	if (shadow) {
		dataPtr = _vm->getResourceAddress(rtImage, shadow);
		assert(dataPtr);
		xmapPtr = _vm->findResourceData(MKTAG('X','M','A','P'), dataPtr);
		assert(xmapPtr);
	}

	dataPtr = _vm->getResourceAddress(rtImage, resNum);
	assert(dataPtr);

	uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), dataPtr, state, 0);
	assert(wizh);
	uint32 comp   = READ_LE_UINT32(wizh + 0x0);
	uint32 width  = READ_LE_UINT32(wizh + 0x4);
	uint32 height = READ_LE_UINT32(wizh + 0x8);
	debug(7, "wiz_header.comp = %d wiz_header.w = %d wiz_header.h = %d", comp, width, height);

	uint8 *mask = NULL;
	if (maskNum) {
		uint8 *maskPtr = _vm->getResourceAddress(rtImage, maskNum);
		assert(maskPtr);

		wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), maskPtr, maskState, 0);
		assert(wizh);
		assert(comp == 2 && READ_LE_UINT32(wizh + 0x0) == 1);
		width  = READ_LE_UINT32(wizh + 0x4);
		height = READ_LE_UINT32(wizh + 0x8);

		mask = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), maskPtr, maskState, 0);
		assert(mask);
	}

	if (flags & kWRFUsePalette) {
		uint8 *pal = _vm->findWrappedBlock(MKTAG('R','G','B','S'), dataPtr, state, 0);
		assert(pal);
		_vm->setPaletteFromPtr(pal, 256);
	}

	uint8 *rmap = nullptr;
	if (flags & kWRFRemap) {
		rmap = _vm->findWrappedBlock(MKTAG('R','M','A','P'), dataPtr, state, 0);
		assert(rmap);
		if (_vm->_game.heversion <= 80 || READ_BE_UINT32(rmap) != 0x01234567) {
			uint8 *rgbs = _vm->findWrappedBlock(MKTAG('R','G','B','S'), dataPtr, state, 0);
			assert(rgbs);
			_vm->remapHEPalette(rgbs, rmap + 4);
		}
	}

	if (flags & kWRFPrint) {
		error("WizImage printing is unimplemented");
	}

	int32 dstPitch, dstType, cw, ch;
	if (flags & kWRFAlloc) {
		dst = (uint8 *)malloc(width * height * _vm->_bytesPerPixel);
		int transColor = (_vm->VAR_WIZ_TRANSPARENT_COLOR != 0xFF) ? (_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR)) : 5;

		if (_vm->_bytesPerPixel == 2) {
			uint8 *tmpPtr = dst;
			for (uint i = 0; i < height; i++) {
				for (uint j = 0; j < width; j++) {
					if (_cursorImage) {
						WRITE_UINT16(tmpPtr + j * 2, transColor);
					} else {
						WRITE_LE_UINT16(tmpPtr + j * 2, transColor);
					}
				}
				tmpPtr += width * 2;
			}
		} else {
			memset(dst, transColor, width * height);
		}
		cw = width;
		ch = height;
		dstPitch = cw * _vm->_bytesPerPixel;
		dstType = (_cursorImage) ? kDstCursor : kDstMemory;
	} else {
		if (dstResNum) {
			uint8 *dstPtr = _vm->getResourceAddress(rtImage, dstResNum);
			assert(dstPtr);
			dst = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), dstPtr, 0, 0);
			assert(dst);
			getWizImageDim(dstResNum, 0, cw, ch);
			dstPitch = cw * _vm->_bytesPerPixel;
			dstType = kDstResource;
		} else {
			VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
			if (flags & kWRFForeground) {
				dst = pvs->getPixels(0, pvs->topline);
			} else {
				dst = pvs->getBackPixels(0, pvs->topline);
			}
			cw = pvs->w;
			ch = pvs->h;
			dstPitch = pvs->pitch;
			dstType = kDstScreen;
		}
	}

	Common::Rect rScreen(cw, ch);
	if (clipBox) {
		Common::Rect clip(clipBox->left, clipBox->top, clipBox->right, clipBox->bottom);
		if (rScreen.intersects(clip)) {
			rScreen.clip(clip);
		} else {
			if (flags & kWRFAlloc)
				free(dst);

			return 0;
		}
	} else if (_rectOverrideEnabled) {
		if (rScreen.intersects(_rectOverride)) {
			rScreen.clip(_rectOverride);
		} else {
			if (flags & kWRFAlloc)
				free(dst);

			return 0;
		}
	}

	if (flags & kWRFRemap && _vm->_bytesPerPixel == 1) {
		palPtr = rmap + 4;
	}

	int transColor = -1;
	if (_vm->VAR_WIZ_TRANSPARENT_COLOR != 0xFF) {
		uint8 *trns = _vm->findWrappedBlock(MKTAG('T','R','N','S'), dataPtr, state, 0);
		transColor = (trns == nullptr) ? _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) : -1;
	}

	if (_vm->_game.id == GID_MOONBASE &&
			((ScummEngine_v100he *)_vm)->_moonbase->isFOW(resNum, state, conditionBits)) {
		((ScummEngine_v100he *)_vm)->_moonbase->renderFOW(dst, dstPitch, dstType, cw, ch, flags);
		x1 = 0;
		y1 = 0;
		width = rScreen.width();
		height = rScreen.height();
	} else {
		drawWizImageEx(dst, dataPtr, mask, dstPitch, dstType, cw, ch, x1, y1, width, height,
			state, &rScreen, flags, palPtr, transColor, _vm->_bytesPerPixel, xmapPtr, conditionBits);
	}

	if (!(flags & kWRFAlloc) && dstResNum == 0) {
		Common::Rect rImage(x1, y1, x1 + width, y1 + height);
		if (rImage.intersects(rScreen)) {
			rImage.clip(rScreen);
			if (!(flags & kWRFBackground) && (flags & (kWRFBackground | kWRFForeground))) {
				++rImage.bottom;
				_vm->markRectAsDirty(kMainVirtScreen, rImage);
			} else {
				_vm->restoreBackgroundHE(rImage);
			}
		}
	}

	return dst;
}

void Wiz::drawWizImageEx(uint8 *dst, uint8 *dataPtr, uint8 *maskPtr, int dstPitch, int dstType,
		int dstw, int dsth, int srcx, int srcy, int srcw, int srch, int state, const Common::Rect *rect,
		int flags, const uint8 *palPtr, int transColor, uint8 bitDepth, const uint8 *xmapPtr, uint32 conditionBits) {
	uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), dataPtr, state, 0);
	assert(wizh);
	uint32 comp   = READ_LE_UINT32(wizh + 0x0);
	uint32 width  = READ_LE_UINT32(wizh + 0x4);
	uint32 height = READ_LE_UINT32(wizh + 0x8);
	debug(7, "wiz_header.comp = %d wiz_header.w = %d wiz_header.h = %d", comp, width, height);

	uint8 *wizd = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), dataPtr, state, 0);
	assert(wizd);

	switch (comp) {
	case kWCTNone:
		copyRawWizImage(dst, wizd, dstPitch, dstType, dstw, dsth, srcx, srcy, srcw, srch, rect, flags, palPtr, transColor, bitDepth);
		break;
	case kWCTTRLE:
		if (flags & kWRFZPlaneOn) {
			dst = _vm->getMaskBuffer(0, 0, 1);
			dstPitch /= _vm->_bytesPerPixel;
			copyWizImageWithMask(dst, wizd, dstPitch, dstw, dsth, srcx, srcy, srcw, srch, rect, 0, 2);
		} else if (flags & kWRFZPlaneOff) {
			dst = _vm->getMaskBuffer(0, 0, 1);
			dstPitch /= _vm->_bytesPerPixel;
			copyWizImageWithMask(dst, wizd, dstPitch, dstw, dsth, srcx, srcy, srcw, srch, rect, 0, 1);
		} else {
			copyWizImage(dst, wizd, dstPitch, dstType, dstw, dsth, srcx, srcy, srcw, srch, rect, flags, palPtr, xmapPtr, bitDepth);
		}
		break;
#ifdef USE_RGB_COLOR
	case kWCTNone16Bpp:
		if (maskPtr) {
			copyMaskWizImage(dst, wizd, maskPtr, dstPitch, dstType, dstw, dsth, srcx, srcy, srcw, srch, rect, flags, palPtr);
		} else {
			copyRaw16BitWizImage(dst, wizd, dstPitch, dstType, dstw, dsth, srcx, srcy, srcw, srch, rect, flags, transColor);
		}
		break;
	case kWCTComposite:
		copyCompositeWizImage(dst, dataPtr, wizd, maskPtr, dstPitch, dstType, dstw, dsth, srcx, srcy, srcw, srch, state, rect, flags, palPtr, transColor, bitDepth, xmapPtr, conditionBits);
		break;
	case kWCTTRLE16Bpp:
		copy16BitWizImage(dst, wizd, dstPitch, dstType, dstw, dsth, srcx, srcy, srcw, srch, rect, flags, xmapPtr);
		break;
	case kWCTDataBlockDependent:
		copy555WizImage(dst, wizd, dstPitch, dstType, dstw, dsth, srcx, srcy, rect, conditionBits);
		break;
#endif
	default:
		error("drawWizImageEx: Unhandled wiz compression type %d", comp);
	}
}

#ifdef USE_RGB_COLOR

void Wiz::copyCompositeWizImage(uint8 *dst, uint8 *wizPtr, uint8 *compositeInfoBlockPtr, uint8 *maskPtr, int dstPitch, int dstType,
		int dstw, int dsth, int srcx, int srcy, int srcw, int srch, int state, const Common::Rect *clipBox,
		int flags, const uint8 *palPtr, int transColor, uint8 bitDepth, const uint8 *xmapPtr, uint32 conditionBits) {

	uint8 *nestedBlockHeader = _vm->heFindResource(MKTAG('N','E','S','T'), wizPtr);
	assert(nestedBlockHeader);

	uint8 *nestedWizHeader = _vm->heFindResource(MKTAG('M','U','L','T'), nestedBlockHeader);
	assert(nestedWizHeader);

	uint16 layerCount = READ_LE_UINT16(compositeInfoBlockPtr);
	compositeInfoBlockPtr += 2;

	uint16 defaultSubConditionBits = (conditionBits & kWMSBReservedBits);

	conditionBits &= ~kWMSBReservedBits;

	for (uint layerCounter = 0; layerCounter < layerCount; layerCounter++) {
		int cmdSize = READ_LE_UINT16(compositeInfoBlockPtr);
		uint8 *cmdPtr = compositeInfoBlockPtr + 2;

		compositeInfoBlockPtr += (cmdSize + 2);
		uint32 layerCmdDataBits = READ_LE_UINT32(cmdPtr);
		cmdPtr += 4;

		uint32 subConditionBits;

		if (layerCmdDataBits & kWCFConditionBits) {
			uint32 layerConditionBits = READ_LE_UINT32(cmdPtr);
			cmdPtr += 4;

			subConditionBits = (layerConditionBits & kWMSBReservedBits);
			layerConditionBits &= ~kWMSBReservedBits;

			if (subConditionBits == 0)
				subConditionBits = defaultSubConditionBits;

			uint32 conditionType = (layerConditionBits & kWSPCCTBits);
			layerConditionBits &= ~kWSPCCTBits;

			switch (conditionType) {
			case kWSPCCTAnd:
				if (layerConditionBits != (layerConditionBits & conditionBits))
					continue;
				break;

			case kWSPCCTNot:
				if (layerConditionBits & conditionBits)
					continue;
				break;

			case kWSPCCTOr:
			default:
				if (!(layerConditionBits & conditionBits))
					continue;
				break;
			}
		} else {
			subConditionBits = defaultSubConditionBits;
		}

		uint16 subState;
		if (layerCmdDataBits & kWCFSubState) {
			subState = READ_LE_UINT16(cmdPtr);
			cmdPtr += 2;
		} else {
			subState = 0;
		}

		int16 xPos;
		if (layerCmdDataBits & kWCFXDelta) {
			xPos = (int16)READ_LE_UINT16(cmdPtr);
			cmdPtr += 2;
		} else {
			xPos = 0;
		}

		int16 yPos;
		if (layerCmdDataBits & kWCFYDelta) {
			yPos = (int16)READ_LE_UINT16(cmdPtr);
			cmdPtr += 2;
		} else {
			yPos = 0;
		}

		uint32 drawFlags;
		if (layerCmdDataBits & kWCFDrawFlags) {
			drawFlags = READ_LE_UINT32(cmdPtr);
			cmdPtr += 4;
		} else {
			drawFlags = flags;
		}

		uint srcw1 = 0, srch1 = 0;
		if (drawFlags & (kWRFFlipX | kWRFFlipY)) {
			uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), wizPtr, subState, 0);
			assert(wizh);
			srcw1 = READ_LE_UINT32(wizh + 0x4);
			srch1 = READ_LE_UINT32(wizh + 0x8);
		}

		if (drawFlags & kWRFFlipX)
			xPos = (srcw - (xPos + srcw1));

		if (drawFlags & kWRFFlipY)
			yPos = (srch - (yPos + srch1));

		if (layerCmdDataBits & kWCFSubConditionBits) {
			subConditionBits = READ_LE_UINT32(cmdPtr);
			cmdPtr += 4;
		}

		drawWizImageEx(dst, nestedWizHeader, maskPtr, dstPitch, dstType, dstw, dsth, srcx + xPos, srcy + yPos, srcw, srch,
			subState, clipBox, drawFlags, palPtr, transColor, bitDepth, xmapPtr, subConditionBits);
	}
}

void Wiz::copy555WizImage(uint8 *dst, uint8 *wizd, int dstPitch, int dstType,
		int dstw, int dsth, int srcx, int srcy, const Common::Rect *clipBox, uint32 conditionBits) {

	int rawROP = conditionBits & kWMSBRopMask;
	int paramROP = (conditionBits & kWMSBReservedBits) >> kWMSBRopParamRShift;

	switch (rawROP) {
	default:
	case 1:
		rawROP = 1;
		// MMX_PREMUL_ALPHA_COPY
		break;

	case 2:
		//warning("T14: MMX_ADDITIVE");
		break;

	case 3:
		warning("T14: MMX_SUBTRACTIVE");
		break;

	case 4:
		warning("T14: MMX_CONSTANT_ALPHA");
		break;

	case 5:
		//warning("T14: MMX_CHEAP_50_50");
		break;

	case 6:
		warning("T14: COPY");
		break;

	case 7:
		warning("T14: CHEAP_50_50");
		break;
	}


	uint32 compID = READ_LE_UINT32(wizd);

	if (compID == 0x12340102) {
		((ScummEngine_v100he *)_vm)->_moonbase->blitT14WizImage(dst, dstw, dsth, dstPitch, clipBox, wizd, srcx, srcy, rawROP, paramROP);
	} else if (compID == 0x12340802) {
		((ScummEngine_v100he *)_vm)->_moonbase->blitDistortion(dst, dstw, dsth, dstPitch, clipBox, wizd, srcx, srcy, 0);
	} else if (compID == 0x12340902) {
		error("Unsupported Distortion");
	}
}

#endif

struct PolygonDrawData {
	struct PolygonArea {
		int32 xmin;
		int32 xmax;
		int32 x1;
		int32 y1;
		int32 x2;
		int32 y2;
	};
	struct ResultArea {
		int32 dst_offs;
		int32 x_step;
		int32 y_step;
		int32 x_s;
		int32 y_s;
		int32 w;
	};
	Common::Point mat[4];
	PolygonArea *pa;
	ResultArea *ra;
	int rAreasNum;
	int pAreasNum;

	PolygonDrawData(int n) {
		for (uint i = 0; i < ARRAYSIZE(mat); i++) {
			mat[i] = Common::Point();
		}
		pa = new PolygonArea[n];
		for (int i = 0; i < n; ++i) {
			pa[i].xmin = 0x7FFFFFFF;
			pa[i].xmax = 0x80000000;
		}
		ra = new ResultArea[n];
		rAreasNum = 0;
		pAreasNum = n;
	}

	~PolygonDrawData() {
		delete[] pa;
		delete[] ra;
	}

	void transform(const Common::Point *tp1, const Common::Point *tp2, const Common::Point *sp1, const Common::Point *sp2) {
		int32 tx_acc = tp1->x * (1 << 16);
		int32 sx_acc = sp1->x * (1 << 16);
		int32 sy_acc = sp1->y * (1 << 16);
		uint16 dy = ABS(tp2->y - tp1->y) + 1;
		int32 tx_step = ((tp2->x - tp1->x) * (1 << 16)) / dy;
		int32 sx_step = ((sp2->x - sp1->x) * (1 << 16)) / dy;
		int32 sy_step = ((sp2->y - sp1->y) * (1 << 16)) / dy;

		int y = tp1->y - mat[0].y;
		while (dy--) {
			assert(y >= 0 && y < pAreasNum);
			PolygonArea *ppa = &pa[y];
			int32 ttx = tx_acc / (1 << 16);
			int32 tsx = sx_acc / (1 << 16);
			int32 tsy = sy_acc / (1 << 16);

			if (ppa->xmin > ttx) {
				ppa->xmin = ttx;
				ppa->x1 = tsx;
				ppa->y1 = tsy;
			}
			if (ppa->xmax < ttx) {
				ppa->xmax = ttx;
				ppa->x2 = tsx;
				ppa->y2 = tsy;
			}

			tx_acc += tx_step;
			sx_acc += sx_step;
			sy_acc += sy_step;

			if (tp2->y <= tp1->y) {
				--y;
			} else {
				++y;
			}
		}
	}
};

void Wiz::processWizImagePolyCaptureCmd(const WizImageCommand *params) {
	int resNum = params->image;
	int maskNum = params->sourceImage;
	bool maskState = (params->actionFlags & kWAFState) ? params->state : 0;
	int id1 = params->polygon;
	int id2 = params->polygon2;
	int compType = params->compressionType;

	debug(7, "processWizImagePolyCaptureCmd: resNum %d, maskNum %d maskState %d, id1 %d id2 %d compType %d", resNum, maskNum, maskState, id1, id2, compType);

	int i, j;
	WizPolygon *wp;

	wp = nullptr;
	for (i = 0; i < ARRAYSIZE(_polygons); ++i) {
		if (_polygons[i].id == id1) {
			wp = &_polygons[i];
			break;
		}
	}
	if (!wp) {
		error("Polygon1 %d is not defined", id1);
	}
	if (wp->numVerts != 5) {
		error("Invalid point count %d for Polygon1 %d", wp->numVerts, id1);
	}

	wp = nullptr;
	for (i = 0; i < ARRAYSIZE(_polygons); ++i) {
		if (_polygons[i].id == id2) {
			wp = &_polygons[i];
			break;
		}
	}
	if (!wp) {
		error("Polygon2 %d is not defined", id2);
	}
	if (wp->numVerts != 5) {
		error("Invalid point count %d for Polygon2 %d", wp->numVerts, id2);
	}

	int32 dstw, dsth, dstpitch;
	int32 srcw, srch;
	uint8 *imageBuffer;

	assert(maskNum);
	const Common::Rect *r = nullptr;
	const uint8 *src = drawWizImage(maskNum, maskState, 0, 0, 0, 0, 0, 0, 0, r, kWRFAlloc, 0, 0, 0);
	getWizImageDim(maskNum, maskState, srcw, srch);

	dstw = wp->bound.width();
	dsth = wp->bound.height();
	dstpitch = dstw * _vm->_bytesPerPixel;
	imageBuffer = (uint8 *)malloc(dstw * dsth * _vm->_bytesPerPixel);
	assert(imageBuffer);

	const uint16 transColor = (_vm->VAR_WIZ_TRANSPARENT_COLOR != 0xFF) ? _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) : 5;
	if (_vm->_bytesPerPixel == 2) {
		uint8 *tmpPtr = imageBuffer;
		for (i = 0; i < dsth; i++) {
			for (j = 0; j < dstw; j++)
				WRITE_LE_UINT16(tmpPtr + j * 2, transColor);
			tmpPtr += dstpitch;
		}
	} else {
		memset(imageBuffer, transColor, dstw * dsth);
	}

	Common::Rect bound;
	drawWizPolygonImage(imageBuffer, src, nullptr, dstpitch, kDstMemory, dstw, dsth, srcw, srch, bound, wp->vert, _vm->_bytesPerPixel);

	captureImage(imageBuffer, dstpitch, dstw, dsth, resNum, wp->bound, compType);
	free(imageBuffer);
}

void Wiz::drawWizComplexPolygon(int resNum, int state, int po_x, int po_y, int shadow, int angle, int scale, const Common::Rect *r, int flags, int dstResNum, int palette) {
	Common::Point pts[4];

	polygonTransform(resNum, state, po_x, po_y, angle, scale, pts);
	drawWizPolygonTransform(resNum, state, pts, flags, shadow, dstResNum, palette);
}

void Wiz::drawWizPolygon(int resNum, int state, int id, int flags, int shadow, int dstResNum, int palette) {
	int i;
	WizPolygon *wp = nullptr;
	for (i = 0; i < ARRAYSIZE(_polygons); ++i) {
		if (_polygons[i].id == id) {
			wp = &_polygons[i];
			break;
		}
	}
	if (!wp) {
		error("Polygon %d is not defined", id);
	}
	if (wp->numVerts != 5) {
		error("Invalid point count %d for Polygon %d", wp->numVerts, id);
	}

	drawWizPolygonTransform(resNum, state, wp->vert, flags, shadow, dstResNum, palette);
}

void Wiz::drawWizPolygonTransform(int resNum, int state, Common::Point *wp, int flags, int shadow, int dstResNum, int palette) {
	debug(7, "drawWizPolygonTransform(resNum %d, flags 0x%X, shadow %d dstResNum %d palette %d)", resNum, flags, shadow, dstResNum, palette);
	const Common::Rect *r = nullptr;
	uint8 *srcWizBuf = nullptr;
	bool freeBuffer = true;

	if (_vm->_game.heversion >= 99) {
		if (getWizImageData(resNum, state, 0) != 0 || (flags & (kWRFRemap | kWRFFlipX | kWRFFlipY)) || palette != 0) {
			flags |= kWRFAlloc;

			if (flags & kWRFAreaSampleDuringWarp) {
				debug(7, "drawWizPolygonTransform() unhandled flag kWRFAreaSampleDuringWarp");
			}

			srcWizBuf = drawWizImage(resNum, state, 0, 0, 0, 0, 0, shadow, 0, r, flags, 0, _vm->getHEPaletteSlot(palette), 0);
		} else {
			assert(_vm->_bytesPerPixel == 1);
			uint8 *dataPtr = _vm->getResourceAddress(rtImage, resNum);
			assert(dataPtr);
			srcWizBuf = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), dataPtr, state, 0);
			assert(srcWizBuf);
			freeBuffer = false;
		}
	} else {
		if (getWizImageData(resNum, state, 0) != 0) {
			srcWizBuf = drawWizImage(resNum, state, 0, 0, 0, 0, 0, shadow, 0, r, kWRFAlloc, 0, _vm->getHEPaletteSlot(palette), 0);
		} else {
			uint8 *dataPtr = _vm->getResourceAddress(rtImage, resNum);
			assert(dataPtr);
			srcWizBuf = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), dataPtr, state, 0);
			assert(srcWizBuf);
			freeBuffer = false;
		}
	}

	assert(srcWizBuf);

	uint8 *dst;
	int32 dstw, dsth, dstpitch, dstType, wizW, wizH;
	VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];

	if (dstResNum) {
		uint8 *dstPtr = _vm->getResourceAddress(rtImage, dstResNum);
		assert(dstPtr);
		dst = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), dstPtr, 0, 0);
		assert(dst);
		getWizImageDim(dstResNum, 0, dstw, dsth);
		dstpitch = dstw * _vm->_bytesPerPixel;
		dstType = kDstResource;
	} else {
		if (flags & kWRFForeground) {
			dst = pvs->getPixels(0, 0);
		} else {
			dst = pvs->getBackPixels(0, 0);
		}
		dstw = pvs->w;
		dsth = pvs->h;
		dstpitch = pvs->pitch;
		dstType = kDstScreen;
	}

	Common::Rect bound;
	getWizImageDim(resNum, state, wizW, wizH);
	drawWizPolygonImage(dst, srcWizBuf, 0, dstpitch, dstType, dstw, dsth, wizW, wizH, bound, wp, _vm->_bytesPerPixel);

	if (flags & kWRFForeground) {
		_vm->markRectAsDirty(kMainVirtScreen, bound);
	} else {
		_vm->restoreBackgroundHE(bound);
	}

	if (freeBuffer)
		free(srcWizBuf);
}

void Wiz::drawWizPolygonImage(uint8 *dst, const uint8 *src, const uint8 *mask, int dstpitch, int dstType, int dstw, int dsth, int wizW, int wizH, Common::Rect &bound, Common::Point *wp, uint8 bitDepth) {
	int i, transColor = (_vm->VAR_WIZ_TRANSPARENT_COLOR != 0xFF) ? _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) : 5;

	Common::Point bbox[4];
	bbox[0].x = 0;
	bbox[0].y = 0;
	bbox[1].x = wizW - 1;
	bbox[1].y = 0;
	bbox[2].x = wizW - 1;
	bbox[2].y = wizH - 1;
	bbox[3].x = 0;
	bbox[3].y = wizH - 1;

	int16 xmin_p, xmax_p, ymin_p, ymax_p;
	xmin_p = ymin_p = (int16)0x7FFF;
	xmax_p = ymax_p = (int16)0x8000;

	for (i = 0; i < 4; ++i) {
		xmin_p = MIN(wp[i].x, xmin_p);
		xmax_p = MAX(wp[i].x, xmax_p);
		ymin_p = MIN(wp[i].y, ymin_p);
		ymax_p = MAX(wp[i].y, ymax_p);
	}

	int16 xmin_b, xmax_b, ymin_b, ymax_b;
	xmin_b = ymin_b = (int16)0x7FFF;
	xmax_b = ymax_b = (int16)0x8000;

	for (i = 0; i < 4; ++i) {
		xmin_b = MIN(bbox[i].x, xmin_b);
		xmax_b = MAX(bbox[i].x, xmax_b);
		ymin_b = MIN(bbox[i].y, ymin_b);
		ymax_b = MAX(bbox[i].y, ymax_b);
	}

	PolygonDrawData pdd(ymax_p - ymin_p + 1);
	pdd.mat[0].x = xmin_p;
	pdd.mat[0].y = ymin_p;
	pdd.mat[1].x = xmax_p;
	pdd.mat[1].y = ymax_p;
	pdd.mat[2].x = xmin_b;
	pdd.mat[2].y = ymin_b;
	pdd.mat[3].x = xmax_b;
	pdd.mat[3].y = ymax_b;

	// precompute the transformation which remaps 'bbox' pixels to 'wp'
	for (i = 0; i < 3; ++i) {
		pdd.transform(&wp[i], &wp[i + 1], &bbox[i], &bbox[i + 1]);
	}
	pdd.transform(&wp[3], &wp[0], &bbox[3], &bbox[0]);

	pdd.rAreasNum = 0;
	PolygonDrawData::ResultArea *pra = &pdd.ra[0];
	int32 yoff = pdd.mat[0].y * dstpitch;
	int16 y_start = pdd.mat[0].y;
	for (i = 0; i < pdd.pAreasNum; ++i) {
		PolygonDrawData::PolygonArea *ppa = &pdd.pa[i];
		if (y_start >= 0 && y_start < dsth) {
			int16 x1 = ppa->xmin;
			if (x1 < 0) {
				x1 = 0;
			}
			int16 x2 = ppa->xmax;
			if (x2 >= dstw) {
				x2 = dstw - 1;
			}
			int16 w = x2 - x1 + 1;
			if (w > 0) {
				int16 width = ppa->xmax - ppa->xmin + 1;
				pra->x_step = ((ppa->x2 - ppa->x1) * (1 << 16)) / width;
				pra->y_step = ((ppa->y2 - ppa->y1) * (1 << 16)) / width;
				pra->dst_offs = yoff + x1 * _vm->_bytesPerPixel;
				pra->w = w;
				pra->x_s = ppa->x1 * (1 << 16);
				pra->y_s = ppa->y1 * (1 << 16);
				int16 tmp = x1 - ppa->xmin;
				if (tmp != 0) {
					pra->x_s += pra->x_step * tmp;
					pra->y_s += pra->y_step * tmp;
				}
				++pra;
				++pdd.rAreasNum;
			}
		}
		++ppa;
		yoff += dstpitch;
		++y_start;
	}

	pra = &pdd.ra[0];
	for (i = 0; i < pdd.rAreasNum; ++i, ++pra) {
		uint8 *dstPtr = dst + pra->dst_offs;
		int32 w = pra->w;
		int32 x_acc = pra->x_s;
		int32 y_acc = pra->y_s;
		while (w--) {
			int32 src_offs = (y_acc / (1 << 16)) * wizW + (x_acc / (1 << 16));
			assert(src_offs < wizW * wizH);
			x_acc += pra->x_step;
			y_acc += pra->y_step;
			if (bitDepth == 2) {
				if (transColor == -1 || transColor != READ_LE_UINT16(src + src_offs * 2)) {
					writeColor(dstPtr, dstType, READ_LE_UINT16(src + src_offs * 2));
				}
			} else {
				if (transColor == -1 || transColor != src[src_offs])
					*dstPtr = src[src_offs];
			}
			dstPtr += bitDepth;
		}
	}

	bound.left = xmin_p;
	bound.top = ymin_p;
	bound.right = xmax_p + 1;
	bound.bottom = ymax_p + 1;
}

void Wiz::flushWizBuffer() {
	for (int i = 0; i < _wizBufferIndex; ++i) {
		WizBufferElement *pwi = &_wizBuffer[i];
		if (pwi->flags & kWRFIsPolygon) {
			drawWizPolygon(pwi->image, pwi->state, pwi->x, pwi->flags, pwi->shadow, 0, pwi->palette);
		} else {
			const Common::Rect *r = nullptr;
			drawWizImage(pwi->image, pwi->state, 0, 0, pwi->x, pwi->y, pwi->z, pwi->shadow, pwi->zbuffer, r, pwi->flags, 0, _vm->getHEPaletteSlot(pwi->palette), 0);
		}
	}
	_wizBufferIndex = 0;
}

void Wiz::loadWizCursor(int resId, int palette) {
	int32 x, y;
	getWizImageSpot(resId, 0, x, y);
	if (x < 0) {
		x = 0;
	} else if (x > 32) {
		x = 32;
	}
	if (y < 0) {
		y = 0;
	} else if (y > 32) {
		y = 32;
	}

	const Common::Rect *r = nullptr;
	_cursorImage = true;
	uint8 *cursor = drawWizImage(resId, 0, 0, 0, 0, 0, 0, 0, 0, r, kWRFAlloc, 0, _vm->getHEPaletteSlot(palette), 0);
	_cursorImage = false;

	int32 cw, ch;
	getWizImageDim(resId, 0, cw, ch);
	_vm->setCursorHotspot(x, y);
	_vm->setCursorFromBuffer(cursor, cw, ch, cw * _vm->_bytesPerPixel);

	// Since we set up cursor palette for default cursor, disable it now
	CursorMan.disableCursorPalette(true);

	free(cursor);
}

#define ADD_REQUIRED_IMAGE(whatImageIsRequired) {                                                                                    \
	if (ARRAYSIZE(requiredImages) <= (requiredImageCount + 1)) {                                                                     \
		error("Wiz::processWizImageDrawCmd(): Too many required images for image operation (limit %d).", ARRAYSIZE(requiredImages)); \
	}                                                                                                                                \
	requiredImages[requiredImageCount] = whatImageIsRequired;                                                                        \
	++requiredImageCount;                                                                                                            \
}

void Wiz::processWizImageDrawCmd(const WizImageCommand *params) {
	int requiredImages[5];
	int requiredImageCount = 0;
	int sourceImage, palette, scale, rotationAngle, state, flags, po_x, po_y, shadow;
	int zbuffer = 0;
	int dstResNum = 0;
	const Common::Rect *r;

	ADD_REQUIRED_IMAGE(params->image);

	if (params->actionFlags & kWAFSourceImg) {
		sourceImage = params->sourceImage;
		ADD_REQUIRED_IMAGE(sourceImage);
	} else {
		sourceImage = 0;
	}

	if (params->actionFlags & kWAFPalette) {
		palette = params->palette;
	} else {
		palette = 0;
	}

	if (params->actionFlags & kWAFScale) {
		scale = params->scale;
	} else {
		scale = 256;
	}

	if (params->actionFlags & kWAFAngle) {
		rotationAngle = params->angle;
	} else {
		rotationAngle = 0;
	}

	if (params->actionFlags & kWAFState) {
		state = params->state;
	} else {
		state = 0;
	}

	if (params->actionFlags & kWAFFlags) {
		flags = params->flags;
	} else {
		flags = 0;
	}

	if (params->actionFlags & kWAFSpot) {
		po_x = params->xPos;
		po_y = params->yPos;
	} else {
		po_x = 0;
		po_y = 0;
	}

	if (params->actionFlags & kWAFShadow) {
		shadow = params->shadow;
		ADD_REQUIRED_IMAGE(shadow);
	} else {
		shadow = 0;
	}

	if (params->actionFlags & kWAFZBufferImage) {
		zbuffer = params->zbufferImage;
		ADD_REQUIRED_IMAGE(zbuffer);
	}

	if (params->actionFlags & kWAFRect) {
		r = &params->box;
	} else {
		r = nullptr;
	}

	if (params->actionFlags & kWAFDestImage) {
		dstResNum = params->destImageNumber;
		ADD_REQUIRED_IMAGE(dstResNum);
	}

	if (requiredImageCount > 0) {
		// TODO
	}

	// TODO:
	//if (params->actionFlags & kWAFDestImage) {
	//	// Get the rendering surface for this image
	//	if (!dwSetSimpleBitmapStructFromImage(params->destImageNumber, 0, &fakeBitmap)) {
	//		error("Image %d is invalid for rendering into", params->destImageNumber);
	//	}
	//
	//	destBitmap = &fakeBitmap;
	//} else {
	//	destBitmap = nullptr;
	//}

	if (_vm->_game.heversion >= 99 && params->actionFlags & kWAFRemapList) {
		processWizImageModifyCmd(params);
		flags |= kWRFRemap;
	}

	// Dispatch the command...

	if (!_vm->_fullRedraw || dstResNum != 0) {
		if (sourceImage != 0) {
			drawWizImage(params->sourceImage, 0, params->image, state, po_x, po_y, params->zPos, shadow, zbuffer, r, flags, dstResNum, _vm->getHEPaletteSlot(palette), 0);
		} else {
			if (params->actionFlags & (kWAFScale | kWAFAngle)) {
				drawWizComplexPolygon(params->image, state, po_x, po_y, shadow, rotationAngle, scale, r, flags, dstResNum, palette);
			} else {
				if (flags & kWRFIsPolygon) {
					drawWizPolygon(params->image, state, po_x, flags, shadow, dstResNum, palette);
				} else {
					drawWizImage(params->image, state, 0, 0, po_x, po_y, params->zPos, shadow, zbuffer, r, flags, dstResNum, _vm->getHEPaletteSlot(palette), params->extendedRenderInfo.conditionBits);
				}
			}
		}
	} else {
		if (sourceImage != 0 || (params->actionFlags & (kWAFScale | kWAFAngle)))
			error("Can't do this command in the enter script");

		bufferAWiz(params->image, state, po_x, po_y, params->zPos, flags, shadow, zbuffer, palette);
	}
}

#undef ADD_REQUIRED_IMAGE

void Wiz::dwCreateRawWiz(int imageNum, int w, int h, int flags, int bitsPerPixel, int optionalSpotX, int optionalSpotY) {
	int compressionType, wizdSize;
	int globSize = 8; // AWIZ header size

	globSize += WIZBLOCK_WIZH_SIZE;

	if (flags & kCWFPalette) {
		globSize += WIZBLOCK_RGBS_SIZE;
	}

	if (flags & kCWFSpot) {
		globSize += WIZBLOCK_SPOT_SIZE;
	}

	if (flags & kCWFRemapTable) {
		globSize += WIZBLOCK_RMAP_SIZE;
	}

	globSize += 8; // WIZD header size
	wizdSize = w * h * (bitsPerPixel / 8);
	globSize += wizdSize;

	const uint8 *palPtr;
	if (_vm->_game.heversion >= 99) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
	} else {
		palPtr = _vm->_currentPalette;
	}

	uint8 *writePtr = _vm->_res->createResource(rtImage, imageNum, globSize);

	if (!writePtr) {
		_vm->VAR(_vm->VAR_OPERATION_FAILURE) = -1;
		return;
	} else {
		_vm->VAR(_vm->VAR_OPERATION_FAILURE) = 0;
	}

	switch (bitsPerPixel) {
	case 8:
		compressionType = kWCTNone;
		break;
	case 16:
		compressionType = kWCTNone16Bpp;
		break;
	case 32:
		compressionType = kWCTNone32Bpp;
		break;
	default:
		error("Unsupported image bits size %d", bitsPerPixel);
		break;
	}

	WRITE_BE_UINT32(writePtr, 'AWIZ'); writePtr += 4;
	WRITE_BE_UINT32(writePtr, globSize); writePtr += 4;
	WRITE_BE_UINT32(writePtr, 'WIZH'); writePtr += 4;
	WRITE_BE_UINT32(writePtr, WIZBLOCK_WIZH_SIZE); writePtr += 4;
	WRITE_LE_UINT32(writePtr, compressionType); writePtr += 4;
	WRITE_LE_UINT32(writePtr, w); writePtr += 4;
	WRITE_LE_UINT32(writePtr, h); writePtr += 4;

	if (flags & kCWFPalette) {
		WRITE_BE_UINT32(writePtr, 'RGBS'); writePtr += 4;
		WRITE_BE_UINT32(writePtr, WIZBLOCK_RGBS_SIZE); writePtr += 4;
		memcpy(writePtr, palPtr, WIZBLOCK_RGBS_DATA_SIZE);

		writePtr += WIZBLOCK_RGBS_DATA_SIZE;
	}

	if (flags & kCWFSpot) {
		WRITE_BE_UINT32(writePtr, 'SPOT'); writePtr += 4;
		WRITE_BE_UINT32(writePtr, WIZBLOCK_SPOT_SIZE); writePtr += 4;
		WRITE_BE_UINT32(writePtr + 0, optionalSpotX);
		WRITE_BE_UINT32(writePtr + 4, optionalSpotY);

		writePtr += WIZBLOCK_SPOT_DATA_SIZE;
	}

	if (flags & kCWFRemapTable) {
		WRITE_BE_UINT32(writePtr, 'RMAP'); writePtr += 4;
		WRITE_BE_UINT32(writePtr, WIZBLOCK_RMAP_SIZE); writePtr += 4;
		WRITE_BE_UINT32(writePtr, 0); writePtr += 4;

		for (int i = 0; i < 256; ++i) {
			*writePtr++ = i;
		}
	}

	WRITE_BE_UINT32(writePtr, 'WIZD'); writePtr += 4;
	WRITE_BE_UINT32(writePtr, 8 + wizdSize); writePtr += 4;
}

bool Wiz::dwSetSimpleBitmapStructFromImage(int imageNum, int imageState, WizSimpleBitmap *destBM) {
	int compType, imageWidth, imageHeight;
	byte *wizHeader;
	byte *dataPtr;
	int blockHeaderSize = 8;

	// Get the image header
	wizHeader = (byte *)getWizStateHeaderPrim(imageNum, imageState);

	if (!wizHeader) {
		return false;
	}

	// Double check the image header compression type
	compType = READ_LE_UINT32(wizHeader + blockHeaderSize);

	if (!isUncompressedFormatTypeID(compType)) {
		return false;
	}

	imageWidth = READ_LE_UINT32(wizHeader + blockHeaderSize + 4);
	imageHeight = READ_LE_UINT32(wizHeader + blockHeaderSize + 8);

	// Do some fun stuff
	dataPtr = (byte *)getWizStateDataPrim(imageNum, imageState);

	if (!dataPtr) {
		return false;
	}

	// Hook up the image info to the simple bitmap info
	destBM->bufferPtr = (RAWPIXEL *)(dataPtr + blockHeaderSize);
	destBM->bitmapWidth = imageWidth;
	destBM->bitmapHeight = imageHeight;

	return true;
}

void Wiz::processWizImageRenderRectCmd(const WizImageCommand *params) {
	Common::Rect renderRect, clipRect, workClipRect;
	int whichState, w, h, whichImage;
	WizSimpleBitmap renderBitmap;
	RAWPIXEL whatColor;

	// What state is going to rendered into?
	if (params->actionFlags & kWAFState) {
		whichState = params->state;
	} else {
		whichState = 0;
	}

	whichImage = params->image;

	// Make the clipping rect for this image / state
	getWizImageDim(whichImage, whichState, w, h);
	makeSizedRectAt(&clipRect, 0, 0, w, h);

	if (params->actionFlags & kWAFRect) {
		workClipRect.left = params->box.left;
		workClipRect.top = params->box.top;
		workClipRect.right = params->box.right;
		workClipRect.bottom = params->box.bottom;

		// Bail out if there isn't overlap between the clipping rects
		if (!findRectOverlap(&clipRect, &workClipRect)) {
			return;
		}
	}

	// Get the rendering coords or assume the entire
	if (params->actionFlags & kWAFRenderCoords) {
		renderRect = params->renderCoords;
	} else {
		renderRect = clipRect;
	}

	// What is the rendering color
	if (params->actionFlags & kWAFColor) {
		whatColor = params->colorValue;
	} else {
		whatColor = _vm->VAR(_vm->VAR_COLOR_BLACK);
	}

	// Get the simple bitmap
	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Wiz::processWizImageRenderRectCmd(): Image %d state %d invalid for rendering", whichImage, whichState);
	}

	// If we're here we must be able to render into the image (clipped)...
	if (findRectOverlap(&renderRect, &clipRect)) {
		pgDrawSolidRect(&renderBitmap, &renderRect, whatColor);
		_vm->_res->setModified(rtImage, params->image);
	}
}

void Wiz::processWizImageRenderLineCmd(const WizImageCommand *params) {
	Common::Rect clipRect, workClipRect;
	int whichState, w, h, whichImage;
	WizSimpleBitmap renderBitmap;
	RAWPIXEL whatColor;
	int iPropertyNumber = 0, iPropertyValue = 0;

	if (!(params->actionFlags & kWAFRenderCoords)) {
		return;
	}

	if (params->actionFlags & kWAFState) {
		whichState = params->state;
	} else {
		whichState = 0;
	}

	if (params->actionFlags & kWAFProperty) {
		iPropertyNumber = params->propertyNumber;
		iPropertyValue = params->propertyValue;
	}

	whichImage = params->image;

	getWizImageDim(whichImage, whichState, w, h);
	makeSizedRectAt(&clipRect, 0, 0, w, h);

	if (params->actionFlags & kWAFRect) {
		workClipRect.left = params->box.left;
		workClipRect.top = params->box.top;
		workClipRect.right = params->box.right;
		workClipRect.bottom = params->box.bottom;

		// Bail out if there isn't overlap between the clipping rects
		if (!findRectOverlap(&clipRect, &workClipRect)) {
			return;
		}
	}

	// What is the rendering color
	if (params->actionFlags & kWAFColor) {
		whatColor = params->colorValue;
	} else {
		whatColor = _vm->VAR(_vm->VAR_COLOR_BLACK);
	}

	// Get the simple bitmap
	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Wiz::processWizImageRenderLineCmd(): Image %d state %d invalid for rendering", whichImage, whichState);
	}

	// If we're here we must be able to render into the image (clipped)...
	switch (iPropertyNumber) {
	case 0:
		pgClippedLineDraw(
			&renderBitmap,
			params->renderCoords.left, params->renderCoords.top,
			params->renderCoords.right, params->renderCoords.bottom,
			&clipRect, whatColor);

		break;
	case 1:
		pgClippedThickLineDraw(
			&renderBitmap,
			params->renderCoords.left, params->renderCoords.top,
			params->renderCoords.right, params->renderCoords.bottom,
			&clipRect,
			iPropertyValue,
			whatColor);

		break;
	}

	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::processWizImageRenderPixelCmd(const WizImageCommand *params) {
	Common::Rect clipRect, workClipRect;
	int whichState, w, h, whichImage;
	WizSimpleBitmap renderBitmap;
	RAWPIXEL whatColor;
	Common::Point pt;

	if (params->actionFlags & kWAFRenderCoords) {
		pt.x = params->renderCoords.left;
		pt.y = params->renderCoords.top;
	} else {
		return;
	}

	if (params->actionFlags & kWAFState) {
		whichState = params->state;
	} else {
		whichState = 0;
	}

	whichImage = params->image;

	getWizImageDim(whichImage, whichState, w, h);
	makeSizedRectAt(&clipRect, 0, 0, w, h);

	if (params->actionFlags & kWAFRect) {
		workClipRect.left = params->box.left;
		workClipRect.top = params->box.top;
		workClipRect.right = params->box.right;
		workClipRect.bottom = params->box.bottom;

		// Bail out if there isn't overlap between the clipping rects
		if (!findRectOverlap(&clipRect, &workClipRect)) {
			return;
		}
	}

	if (params->actionFlags & kWAFColor) {
		whatColor = params->colorValue;
	} else {
		whatColor = _vm->VAR(_vm->VAR_COLOR_BLACK);
	}

	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Wiz::processWizImageRenderPixelCmd(): Image %d state %d invalid for rendering.", whichImage, whichState);
	}

	if (isPointInRect(&clipRect,&pt)) {
		pgWritePixel(&renderBitmap, pt.x, pt.y, whatColor);
		_vm->_res->setModified(rtImage, params->image);
	}
}

void Wiz::processWizImageModifyCmd(const WizImageCommand *params) {
	int st = (params->actionFlags & kWAFState) ? params->state : 0;
	int num = params->remapCount;
	const uint8 *index = params->remapList;
	uint8 *iwiz = _vm->getResourceAddress(rtImage, params->image);
	assert(iwiz);
	uint8 *rmap = _vm->findWrappedBlock(MKTAG('R','M','A','P'), iwiz, st, 0);
	assert(rmap);
	WRITE_BE_UINT32(rmap, 0x01234567);
	while (num--) {
		uint8 idx = *index++;
		rmap[4 + idx] = params->remapTable[idx];
	}
	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::processWizImageRenderEllipseCmd(const WizImageCommand *params) {
	int iWhichState = 0, iPropertyNumber = 0, iPropertyValue = 0;
	int iWidth = 0, iHeight = 0;

	if (params->actionFlags & kWAFProperty) {
		iPropertyNumber = params->propertyNumber;
		iPropertyValue = params->propertyValue;
	}

	// What state is going to rendered into?
	if (params->actionFlags & kWAFState) {
		iWhichState = params->state;
	}

	int iWhichImage = params->image;

	// Make the clipping rect for this image / state
	getWizImageDim(iWhichImage, iWhichState, iWidth, iHeight);

	Common::Rect clipRect;
	makeSizedRectAt(&clipRect, 0, 0, iWidth, iHeight);

	// Get the simple bitmap
	WizSimpleBitmap renderBitmap;

	if (!dwSetSimpleBitmapStructFromImage(iWhichImage, iWhichState, &renderBitmap)) {
		error("Wiz::processWizImageRenderEllipseCmd(): Image %d state %d invalid for rendering.", iWhichImage, iWhichState);
	}

	pgDrawClippedEllipse(&renderBitmap,
						 params->ellipseProperties.px, params->ellipseProperties.py,
						 params->ellipseProperties.qx, params->ellipseProperties.qy,
						 params->ellipseProperties.kx, params->ellipseProperties.ky,
						 params->ellipseProperties.lod,
						 &clipRect,
						 iPropertyValue,
						 params->ellipseProperties.color);

	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::processWizImageFontStartCmd(const WizImageCommand *params) {
	// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
	// TODO: Start Font
}

void Wiz::processWizImageFontEndCmd(const WizImageCommand *params) {
	// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
	// TODO: End Font
}

void Wiz::processWizImageFontCreateCmd(const WizImageCommand *params) {
	// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
	// TODO: Create Font
}

void Wiz::processWizImageFontRenderCmd(const WizImageCommand *params) {
	// TODO: Render Font String
	error("Wiz::processWizImageFontRenderCmd(): Render Font String");
}

void Wiz::processNewWizImageCmd(const WizImageCommand *params) {
	int width, height;
	int propertyNumber = 0, propertyValue = 0;
	int hotspotX, hotspotY;

	if (params->actionFlags & kWAFWidth) {
		width = params->width;
	} else {
		width = 640;
	}

	if (params->actionFlags & kWAFHeight) {
		height = params->height;
	} else {
		height = 480;
	}

	if (params->actionFlags & kWAFSpot) {
		hotspotX = params->xPos;
		hotspotY = params->yPos;
	} else {
		hotspotX = 0;
		hotspotY = 0;
	}

	// Determine pixel depth...
	if (params->actionFlags & kWAFProperty) {
		propertyNumber = params->propertyNumber;
		propertyValue = params->propertyValue;
	}

	int pixelDepth = (_vm->_game.features & GF_16BIT_COLOR) ? 16 : 8;
	if (propertyNumber == 1) {
		pixelDepth = propertyValue;

		if ((pixelDepth != 16) && (pixelDepth != 8)) {
			error("Wiz::processNewWizImageCmd(): Currently the only pixel depths supported for a new image are 16 and 8. You picked %d.\n", pixelDepth);
		}
	}

	dwCreateRawWiz(params->image, width, height, kCWFDefault, pixelDepth, hotspotX, hotspotY);

	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::processWizImageLoadCmd(const WizImageCommand *params) {
	if (params->actionFlags & kWAFFilename) {
		Common::SeekableReadStream *f = _vm->openFileForReading(params->filename);

		if (f) {
			uint32 id = f->readUint32BE();

			if (id == MKTAG('A', 'W', 'I', 'Z') || id == MKTAG('M', 'U', 'L', 'T')) {
				uint32 size = f->readUint32BE();
				f->seek(0, SEEK_SET);

				byte *p = _vm->_res->createResource(rtImage, params->image, size);

				if (f->read(p, size) != size) {
					_vm->_res->nukeResource(rtImage, params->image);
					error("i/o error when reading '%s'", params->filename);
					_vm->VAR(_vm->VAR_GAME_LOADED) = DW_LOAD_READ_FAILURE;
					_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_LOAD_READ_FAILURE;
				} else {
					_vm->_res->setModified(rtImage, params->image);
					_vm->VAR(_vm->VAR_GAME_LOADED) = DW_LOAD_SUCCESS;
					_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_LOAD_SUCCESS;
				}

			} else {
				_vm->VAR(_vm->VAR_GAME_LOADED) = DW_LOAD_NOT_TYPE;
				_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_LOAD_NOT_TYPE;
			}

			delete f;
		} else {
			_vm->VAR(_vm->VAR_GAME_LOADED) = DW_LOAD_OPEN_FAILURE;
			_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_LOAD_OPEN_FAILURE;
			debug(0, "Unable to open for read '%s'", params->filename);
		}
	}
}

void Wiz::processWizImageSaveCmd(const WizImageCommand *params) {
	if (params->actionFlags & kWAFFilename) {
		switch (params->fileType) {
		case DW_SAVE_RAW_FORMAT:
			// Ignore on purpose...
			_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_NOT_TYPE;
			break;
		case DW_SAVE_PCX_FORMAT:
			// TODO Write image to file
			break;
		case DW_SAVE_WIZ_FORMAT: {
			Common::WriteStream *f = _vm->openSaveFileForWriting(params->filename);

			if (!f) {
				debug(0, "Unable to open for write '%s'", params->filename);
				_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_CREATE_FAILURE;
			} else {
				byte *p = _vm->getResourceAddress(rtImage, params->image);
				uint32 size = READ_BE_UINT32(p + 4);

				if (f->write(p, size) != size) {
					error("i/o error when writing '%s'", params->filename);
					_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_WRITE_FAILURE;
				} else {
					_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_SUCCESS;
				}

				f->finalize();
				delete f;
			}

			break;
		}
		default:
			error("processWizImageCmd: actionMode kWASave unhandled fileType %d", params->fileType);
		}
	}
}

void Wiz::processWizImageCmd(const WizImageCommand *params) {
	debug(7, "processWizImageCmd: actionMode %d", params->actionMode);
	switch (params->actionMode) {
	case kWAUnknown:
		// Do nothing...
		break;
	case kWADraw:
		processWizImageDrawCmd(params);
		break;
	case kWACapture:
		processWizImageCaptureCmd(params);
		break;
	case kWALoad:
		processWizImageLoadCmd(params);
		break;
	case kWASave:
		processWizImageSaveCmd(params);
		break;
	case kWAGlobalState:
		// Do nothing...
		break;
	case kWAModify:
		processWizImageModifyCmd(params);
		break;
	// HE 99+
	case kWAPolyCapture:
		processWizImagePolyCaptureCmd(params);
		break;
	case kWANew:
		processNewWizImageCmd(params);
		break;
	case kWARenderRectangle:
		processWizImageRenderRectCmd(params);
		break;
	case kWARenderLine:
		processWizImageRenderLineCmd(params);
		break;
	case kWARenderPixel:
		processWizImageRenderPixelCmd(params);
		break;
	case kWARenderFloodFill:
		processWizImageRenderFloodFillCmd(params);
		break;
	case kWAFontStart:
		processWizImageFontStartCmd(params);
		break;
	case kWAFontEnd:
		processWizImageFontEndCmd(params);
		break;
	case kWAFontCreate:
		processWizImageFontCreateCmd(params);
		break;
	case kWAFontRender:
		processWizImageFontRenderCmd(params);
		break;
	case kWARenderEllipse:
		processWizImageRenderEllipseCmd(params);
		break;
	default:
		error("Unhandled processWizImageCmd mode %d", params->actionMode);
	}
}

bool Wiz::isUncompressedFormatTypeID(int id) {
	return ((kWCTNone == id) || (kWCTNone16Bpp == id) ||
			(kWCTNone32Bpp == id) || (kWCTNone16BppBigEndian == id) ||
			(kWCTNone32BppBigEndian == id));
}

void Wiz::getWizImageDim(int resNum, int state, int32 &w, int32 &h) {
	uint8 *dataPtr = _vm->getResourceAddress(rtImage, resNum);
	assert(dataPtr);
	getWizImageDim(dataPtr, state, w, h);
}

void Wiz::getWizImageDim(uint8 *dataPtr, int state, int32 &w, int32 &h) {
	uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), dataPtr, state, 0);
	assert(wizh);
	w = READ_LE_UINT32(wizh + 0x4);
	h = READ_LE_UINT32(wizh + 0x8);
}

void Wiz::getWizImageSpot(int resId, int state, int32 &x, int32 &y) {
	uint8 *dataPtr = _vm->getResourceAddress(rtImage, resId);
	assert(dataPtr);
	getWizImageSpot(dataPtr, state, x, y);
}

void Wiz::getWizImageSpot(uint8 *dataPtr, int state, int32 &x, int32 &y) {
	uint8 *spotPtr = _vm->findWrappedBlock(MKTAG('S','P','O','T'), dataPtr, state, 0);
	if (spotPtr) {
		x = READ_LE_UINT32(spotPtr + 0);
		y = READ_LE_UINT32(spotPtr + 4);
	} else {
		x = 0;
		y = 0;
	}
}

int Wiz::getWizImageData(int resNum, int state, int type) {
	uint8 *dataPtr, *wizh;

	dataPtr = _vm->getResourceAddress(rtImage, resNum);
	assert(dataPtr);

	switch (type) {
	case 0:
		wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), dataPtr, state, 0);
		assert(wizh);
		return READ_LE_UINT32(wizh + 0x0);
	case 1:
		return (_vm->findWrappedBlock(MKTAG('R','G','B','S'), dataPtr, state, 0) != nullptr) ? 1 : 0;
	case 2:
		return (_vm->findWrappedBlock(MKTAG('R','M','A','P'), dataPtr, state, 0) != nullptr) ? 1 : 0;
	case 3:
		return (_vm->findWrappedBlock(MKTAG('T','R','N','S'), dataPtr, state, 0) != nullptr) ? 1 : 0;
	case 4:
		return (_vm->findWrappedBlock(MKTAG('X','M','A','P'), dataPtr, state, 0) != nullptr) ? 1 : 0;
	default:
		error("getWizImageData: Unknown type %d", type);
	}
}

int Wiz::getWizImageStates(int resNum) {
	const uint8 *dataPtr = _vm->getResourceAddress(rtImage, resNum);
	assert(dataPtr);

	return getWizImageStates(dataPtr);
}

int Wiz::getWizImageStates(const uint8 *dataPtr) {
	if (READ_BE_UINT32(dataPtr) == MKTAG('M','U','L','T')) {
		const byte *offs, *wrap;

		wrap = _vm->findResource(MKTAG('W','R','A','P'), dataPtr);
		if (wrap == nullptr)
			return 1;

		offs = _vm->findResourceData(MKTAG('O','F','F','S'), wrap);
		if (offs == nullptr)
			return 1;

		return _vm->getResourceDataSize(offs) / 4;
	} else {
		return 1;
	}
}

void *Wiz::getWizStateHeaderPrim(int resNum, int state) {
	uint8 *data = _vm->getResourceAddress(rtImage, resNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), data, state, false) - 8;
}

void *Wiz::getWizStateDataPrim(int resNum, int state) {
	uint8 *data = _vm->getResourceAddress(rtImage, resNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'D'), data, state, false) - 8;
}

int Wiz::isWizPixelNonTransparent(int resNum, int state, int x, int y, int flags) {
	uint8 *data = _vm->getResourceAddress(rtImage, resNum);
	assert(data);

	return isWizPixelNonTransparent(data, state, x, y, flags);
}

int Wiz::isWizPixelNonTransparent(uint8 *data, int state, int x, int y, int flags) {
	int ret = 0;
	uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), data, state, 0);
	assert(wizh);
	int c = READ_LE_UINT32(wizh + 0x0);
	int w = READ_LE_UINT32(wizh + 0x4);
	int h = READ_LE_UINT32(wizh + 0x8);

	if (_vm->_game.id == GID_MOONBASE) {
		RAWPIXEL color = 0xffff;
		drawWizImageEx((byte *)&color, data, 0, 2, kDstMemory, 1, 1, -x, -y, w, h, state, 0, 0, 0, 0, 2, 0, 0);

		return color != 0xffff;
	}

	uint8 *wizd = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), data, state, 0);
	assert(wizd);
	if (x >= 0 && x < w && y >= 0 && y < h) {
		if (flags & kWRFFlipX) {
			x = w - x - 1;
		}
		if (flags & kWRFFlipY) {
			y = h - y - 1;
		}
		switch (c) {
		case 0:
			if (_vm->_game.heversion >= 99) {
				ret = getRawWizPixelColor(wizd, x, y, w, h, 1, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR)) != _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) ? 1 : 0;
			} else {
				ret = 0;
			}
			break;
		case 1:
			ret = isPixelNonTransparent(wizd, x, y, w, h, 1);
			break;
#ifdef USE_RGB_COLOR
		case 2:
			ret = getRawWizPixelColor(wizd, x, y, w, h, 2, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR)) != _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) ? 1 : 0;
			break;
		case 4: {
			uint16 color = 0xffff;
			copyCompositeWizImage((byte *)&color, data, wizd, 0, 2, kDstMemory, 1, 1, -x, -y, w, h, state, 0, 0, 0, 0, 2, 0, 0);
			ret = color != 0xffff;
			break;
		}
		case 5:
			ret = isPixelNonTransparent(wizd, x, y, w, h, 2);
			break;
#endif
		default:
			error("isWizPixelNonTransparent: Unhandled wiz compression type %d", c);
			break;
		}
	}
	return ret;
}

uint16 Wiz::getWizPixelColor(int resNum, int state, int x, int y) {
	uint16 color = 0;
	uint8 *data = _vm->getResourceAddress(rtImage, resNum);
	assert(data);
	uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), data, state, 0);
	assert(wizh);
	int c = READ_LE_UINT32(wizh + 0x0);
	int w = READ_LE_UINT32(wizh + 0x4);
	int h = READ_LE_UINT32(wizh + 0x8);

	if (_vm->_game.id == GID_MOONBASE) {
		drawWizImageEx((byte *)&color, data, 0, 2, kDstMemory, 1, 1, -x, -y, w, h, state, 0, 0, 0, 0, 2, 0, 0);

		return color;
	}

	uint8 *wizd = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), data, state, 0);
	assert(wizd);
	switch (c) {
	case 0:
		if (_vm->_game.heversion >= 99) {
			color = getRawWizPixelColor(wizd, x, y, w, h, 1, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
		} else {
			color = _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR);
		}
		break;
	case 1:
		color = getWizPixelColor(wizd, x, y, w, h, 1, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
		break;
#ifdef USE_RGB_COLOR
	case 2:
		color = getRawWizPixelColor(wizd, x, y, w, h, 2, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
		break;
	case 4:
		copyCompositeWizImage((byte *)&color, data, wizd, 0, 2, kDstMemory, 1, 1, -x, -y, w, h, state, 0, 0, 0, 0, 2, 0, 0);
		break;
	case 5:
		color = getWizPixelColor(wizd, x, y, w, h, 2, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
		break;
#endif
	default:
		error("getWizPixelColor: Unhandled wiz compression type %d", c);
		break;
	}
	return color;
}

int ScummEngine_v90he::computeWizHistogram(int resNum, int state, int x, int y, int w, int h) {
	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, 255);
	if (readVar(0) != 0) {
		Common::Rect rCapt(x, y, w + 1, h + 1);
		uint8 *data = getResourceAddress(rtImage, resNum);
		assert(data);
		uint8 *wizh = findWrappedBlock(MKTAG('W','I','Z','H'), data, state, 0);
		assert(wizh);
		int c = READ_LE_UINT32(wizh + 0x0);
		w = READ_LE_UINT32(wizh + 0x4);
		h = READ_LE_UINT32(wizh + 0x8);
		Common::Rect rWiz(w, h);
		uint8 *wizd = findWrappedBlock(MKTAG('W','I','Z','D'), data, state, 0);
		assert(wizd);
		if (rCapt.intersects(rWiz)) {
			rCapt.clip(rWiz);
			uint32 histogram[256];
			memset(histogram, 0, sizeof(histogram));
			switch (c) {
			case 0:
				_wiz->computeRawWizHistogram(histogram, wizd, w, rCapt);
				break;
			case 1:
				_wiz->computeWizHistogram(histogram, wizd, rCapt);
				break;
			default:
				error("computeWizHistogram: Unhandled wiz compression type %d", c);
				break;
			}
			for (int i = 0; i < 256; ++i) {
				writeArray(0, 0, i, histogram[i]);
			}
		}
	}
	return readVar(0);
}

} // End of namespace Scumm

#endif // ENABLE_HE
