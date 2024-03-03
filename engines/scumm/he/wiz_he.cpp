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
	_lUseWizClipRect = false;
	_uses16BitColor = (_vm->_game.features & GF_16BIT_COLOR);
}

void Wiz::clearWizBuffer() {
	_wizBufferIndex = 0;
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
		if (flags & kWRFVFlip) {
			const int dy = (srcy < 0) ? srcy : (srch - r1.height());
			r1.translate(0, dy);
		}
		if (flags & kWRFHFlip) {
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
		if (flags & kWRFVFlip) {
			const int dy = (srcy < 0) ? srcy : (srch - r1.height());
			r1.translate(0, dy);
		}
		if (flags & kWRFHFlip) {
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
	if (flags & kWRFVFlip) {
		const int dy = (srcy < 0) ? srcy : (srch - srcRect.height());
		srcRect.translate(0, dy);
	}
	if (flags & kWRFHFlip) {
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
	if (flags & kWRFHFlip) {
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
		if (flags & kWRFHFlip) {
			int l = r1.left;
			int r = r1.right;
			r1.left = srcw - r;
			r1.right = srcw - l;
		}
		if (flags & kWRFVFlip) {
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
		if (flags & kWRFHFlip) {
			int l = r1.left;
			int r = r1.right;
			r1.left = srcw - r;
			r1.right = srcw - l;
		}
		if (flags & kWRFVFlip) {
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

	if (flags & kWRFVFlip) {
		dstPtr += (h - 1) * dstPitch;
		dstPitch = -dstPitch;
	}
	dstInc = 2;
	if (flags & kWRFHFlip) {
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

	if (flags & kWRFVFlip) {
		dstPtr += (h - 1) * dstPitch;
		dstPitch = -dstPitch;
	}
	dstInc = bitDepth;
	if (flags & kWRFHFlip) {
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

void Wiz::processWizImageCaptureCmd(const WizImageCommand *params) {
	bool compressIt = (params->compressionType == kWCTTRLE);
	bool background = (params->flags & kWRFBackground) != 0;

	takeAWiz(params->image, params->box.left, params->box.top, params->box.right, params->box.bottom, background, compressIt);

	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::takeAWiz(int globnum, int x1, int y1, int x2, int y2, bool back, bool compress) {
	int bufferWidth, bufferHeight;
	Common::Rect rect, clipRect;
	WizRawPixel *srcPtr;

	VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
	bufferWidth = pvs->w;
	bufferHeight = pvs->h;

	if (back) {
		srcPtr = (WizRawPixel *)pvs->getPixels(0, 0);
	} else {
		srcPtr = (WizRawPixel *)pvs->getBackPixels(0, 0);
	}

	rect.left = x1;
	rect.top = y1;
	rect.right = x2;
	rect.bottom = y2;

	clipRect.left = 0;
	clipRect.top = 0;
	clipRect.right = bufferWidth - 1;
	clipRect.bottom = bufferHeight - 1;

	if (!findRectOverlap(&rect, &clipRect)) {
		error("Capture rect invalid (%-4d,%4d,%-4d,%4d)", x1, y1, x2, y2);
	}

	uint8 *palPtr = nullptr;
	if (_vm->_game.heversion >= 99) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
	} else {
		palPtr = _vm->_currentPalette;
	}

	buildAWiz(
		srcPtr, bufferWidth, bufferHeight,
		palPtr, &rect,
		(compress) ? kWCTTRLE : kWCTNone,
		globnum, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
}

void Wiz::simpleDrawAWiz(int image, int state, int x, int y, int flags) {
	if (!_vm->_fullRedraw) {
		drawAWiz(image, state, x, y, 0, flags, 0, 0, nullptr, 0, nullptr);
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

byte *Wiz::drawAWiz(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, Common::Rect *optionalClipRect, int whichPalette, WizSimpleBitmap *optionalBitmapOverride) {
	return drawAWizEx(image, state, x, y, z, flags,
		optionalShadowImage, optionalZBufferImage, optionalClipRect,
		whichPalette, optionalBitmapOverride, nullptr);
}

byte *Wiz::drawAWizEx(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, Common::Rect *optionalClipRect, int whichPalette, WizSimpleBitmap *optionalBitmapOverride, const WizImageCommand *optionalICmdPtr) {
	const WizRawPixel *colorConversionTable;
	Common::Rect *clipRectPtr;

	if (whichPalette) {
		colorConversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(whichPalette);
	} else {
		colorConversionTable = nullptr;
	}

	// Get the "shadow"...
	if (!optionalShadowImage) {
		if (_lWizActiveShadow && (flags & kWRFUseShadow)) {
			optionalShadowImage = _lWizActiveShadow;
		}
	}

	if (!(flags & kWRFPolygon)) {
		// Get the clipping rect if any...
		if (!optionalClipRect) {
			if (_lUseWizClipRect && !(flags & (kWRFPrint | kWRFAlloc))) {
				clipRectPtr = &_lWizClipRect;
			} else {
				clipRectPtr = nullptr;
			}
		} else {
			clipRectPtr = optionalClipRect;
		}

		// Call the primitive renderer.
		return (byte *)drawAWizPrimEx(image, state, x, y, z,
			optionalShadowImage, optionalZBufferImage, clipRectPtr,
			flags, optionalBitmapOverride, colorConversionTable, optionalICmdPtr);
	} else {
		WARPWIZ_DrawWiz(
			image, state, x, flags, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
			optionalBitmapOverride, colorConversionTable, optionalShadowImage);

		return nullptr;
	}
}

void *Wiz::drawAWizPrim(int globNum, int state, int x, int y, int z, int shadowImage, int zbufferImage, const Common::Rect *optionalClipRect, int flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	return drawAWizPrimEx(globNum, state, x, y, z,
		shadowImage, zbufferImage, optionalClipRect, flags,
		optionalBitmapOverride, optionalColorConversionTable, 0);
}

void *Wiz::drawAWizPrimEx(int globNum, int state, int x, int y, int z, int shadowImage, int zbufferImage, const Common::Rect *optionalClipRect, int flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable, const WizImageCommand *optionalICmdPtr) {
	int dest_w, dest_h, src_w, src_h, src_c, remapID;
	byte *src_d, *src_p, *pp, *remap_p, *shadow_p;
	Common::Rect dest_r, clip_r;
	bool markUpdates;
	WizRawPixel *dest_p;

	markUpdates = true;
	remap_p = nullptr;

	// Set the optional remap table up to the default if one isn't specified
	if (!optionalColorConversionTable) {
		optionalColorConversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(1);
	}

	// If using a shadow make sure both globs are in ram!!!!
	if (shadowImage) {
		shadow_p = (byte *)getColorMixBlockPtrForWiz(shadowImage);

		// TODO: I don't think this is needed here?
		//if (shadow_p) {
		//	shadow_p += 8;
		//}
	} else {
		shadow_p = nullptr;
	}

	// If using a z-buffer make sure both globs are in ram!!!!
	if (zbufferImage) {
		// uncompressed 16-bit z-buffers only for now
		byte *pzbHeader = (byte *)getWizStateHeaderPrim(zbufferImage, 0);

		assert(pzbHeader);

		int zbComp = READ_LE_UINT32(pzbHeader + _vm->_resourceHeaderSize);

		if (zbComp != kWCTNone16Bpp) {
			error("Wiz::drawAWizPrimEx(): 16-bit uncompressed z-buffers are the only currently supported format");
		}
	}

	// Get the header for this "state"
	pp = getWizStateHeaderPrim(globNum, state);
	if (!pp)
		error("Wiz::drawAWizPrimEx(): %d state %d missing header", globNum, state);

	src_c = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 0);
	src_w = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 4);
	src_h = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 8);

	if ((!isUncompressedFormatTypeID(src_c)) && (src_c != kWCTTRLE)) {
		error("Wiz::drawAWizPrimEx(): %d has invalid compression type %d", globNum, src_c);
	}

	// Get the data block for this "state"
	src_d = (byte *)getWizStateDataPrim(globNum, state);
	if (!src_d)
		error("Wiz::drawAWizPrimEx(): %d state %d missing data block", globNum, state);

	// Copy the palette from this "state"?
	if (flags & kWRFUsePalette) {
		src_p = (byte *)getWizStatePaletteDataPrim(globNum, state);
		if (!src_p)
			error("Wiz::drawAWizPrimEx(): %d state %d missing palette block", globNum, state);

		_vm->setPaletteFromPtr(src_p + _vm->_resourceHeaderSize, 256);
	}

	// Remap this wiz "state"?
	if (flags & kWRFRemap) {
		remap_p = (byte *)getWizStateRemapDataPrim(globNum, state);
		if (!remap_p)
			error("Wiz::drawAWizPrimEx(): %d state %d is missing a remap palette block", globNum, state);

		remapID = READ_LE_UINT32(remap_p + _vm->_resourceHeaderSize);

		if (remapID != WIZ_MAGIC_REMAP_NUMBER) {
			if (remapID != _vm->_paletteChangedCounter) {
				WRITE_LE_UINT32(remap_p + _vm->_resourceHeaderSize, _vm->_paletteChangedCounter);

				src_p = (byte *)getWizStatePaletteDataPrim(globNum, state);
				if (!src_p)
					error("Wiz::drawAWizPrimEx(): %d state %d missing palette block", globNum, state);

				_vm->remapHEPalette(src_p, remap_p + 4);
			}
		}
	}

	// Get the dest pointer...
	if (flags & (kWRFPrint | kWRFAlloc)) {
		dest_w = src_w;
		dest_h = src_h;
		if (_uses16BitColor) {
			dest_p = (WizRawPixel *)malloc(dest_w * dest_h * sizeof(WizRawPixel16));
		} else {
			dest_p = (WizRawPixel *)malloc(dest_w * dest_h * sizeof(WizRawPixel8));
		}

		if (!dest_p) {
			warning("Wiz::drawAWizPrimEx(): Not enough memory for image operation (print / other)");
			return nullptr;
		} else if (flags & kWRFAlloc) {
			memset8BppConversion(
				dest_p,
				_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
				dest_w * dest_h,
				optionalColorConversionTable);
		}

	} else {
		if (optionalBitmapOverride) {
			dest_w = optionalBitmapOverride->bitmapWidth;
			dest_h = optionalBitmapOverride->bitmapHeight;
			dest_p = optionalBitmapOverride->bufferPtr;

			markUpdates = false;
		} else {
			VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
			dest_w = pvs->w;
			dest_h = pvs->h;

			if (flags & kWRFForeground) {
				dest_p = (WizRawPixel *)pvs->getPixels(0, pvs->topline);
			} else {
				dest_p = (WizRawPixel *)pvs->getBackPixels(0, pvs->topline);
			}
		}
	}

	// Make the clipping rect equal to the limits of the draw buffer
	clip_r.left = 0;
	clip_r.top = 0;
	clip_r.right = dest_w - 1;
	clip_r.bottom = dest_h - 1;

	if (optionalClipRect && (!(flags & (kWRFPrint | kWRFAlloc)))) {
		if (!findRectOverlap(&clip_r, optionalClipRect)) {
			return nullptr;
		}
	}

	// Get down to business and decompress the image...
	if (zbufferImage) {
		WizSimpleBitmap sbZBuffer;
		sbZBuffer.bitmapHeight = 0;
		sbZBuffer.bitmapWidth = 0;
		sbZBuffer.bufferPtr = nullptr;

		dwSetSimpleBitmapStructFromImage(zbufferImage, 0, &sbZBuffer);

		// Validate destination for z-buffer
		if ((dest_w != sbZBuffer.bitmapWidth) ||
			(dest_h != sbZBuffer.bitmapHeight)) {
			error("Wiz::drawAWizPrimEx(): destination size must match z-buffer size d:%dx%d  z:%dx%d",
					   dest_w, dest_h, sbZBuffer.bitmapWidth, sbZBuffer.bitmapHeight);
		}

		WizSimpleBitmap sbDst;

		sbDst.bufferPtr = dest_p;
		sbDst.bitmapWidth = dest_w;
		sbDst.bitmapHeight = dest_h;

		if (src_c != kWCTTRLE) {
			pgDrawImageWith16BitZBuffer(&sbDst, &sbZBuffer, src_d + _vm->_resourceHeaderSize, x, y, z, src_w, src_h, &clip_r);
		}
	} else if (src_c == kWCTTRLE) {
		if (flags & kWRFZPlaneOn) {
			if (_vm->_gdi->_numZBuffer <= 1) {
				error("Wiz::drawAWizPrimEx(): No zplane %d (limit 0 to %d)", 1, (_vm->_gdi->_numZBuffer - 1));
			}

			auxDrawZplaneFromTRLEImage(_vm->getMaskBuffer(0, 0, 1), src_d + _vm->_resourceHeaderSize, dest_w, dest_h, x, y, src_w, src_h, &clip_r, kWZOIgnore, kWZOSet);
		} else if (flags & kWRFZPlaneOff) {
			if (_vm->_gdi->_numZBuffer <= 1) {
				error("Wiz::drawAWizPrimEx(): No zplane %d (limit 0 to %d)", 1, (_vm->_gdi->_numZBuffer - 1));
			}

			auxDrawZplaneFromTRLEImage(_vm->getMaskBuffer(0, 0, 1), src_d + _vm->_resourceHeaderSize, dest_w, dest_h, x, y, src_w, src_h, &clip_r, kWZOIgnore, kWZOClear);
		} else {
			void *dataPtr = nullptr;

			if (shadow_p)
				dataPtr = shadow_p;

			if (flags & kWRFRemap)
				dataPtr = remap_p + _vm->_resourceHeaderSize + 4;

			TRLEFLIP_DecompressImage(
				dest_p, src_d + _vm->_resourceHeaderSize, dest_w, dest_h,
				x, y, src_w, src_h, &clip_r, flags, dataPtr,
				optionalColorConversionTable,
				optionalICmdPtr);
		}

	} else {
		int transColorOverride;
		void *dataPtr = nullptr;

		if (shadow_p)
			dataPtr = shadow_p;

		if (flags & kWRFRemap)
			dataPtr = remap_p + _vm->_resourceHeaderSize + 4;

		if (doesRawWizStateHaveTransparency(globNum, state)) {
			transColorOverride = _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR);
		} else {
			transColorOverride = -1;
		}

		if (src_c != kWCTNone16Bpp && src_c != kWCTNone16BppBigEndian) {
			if (src_c == kWCTNone) {
				pgDraw8BppFormatImage(
					dest_p, (byte *)(src_d + _vm->_resourceHeaderSize), dest_w, dest_h,
					x, y, src_w, src_h, &clip_r, flags, dataPtr, transColorOverride,
					optionalColorConversionTable);
			} else {
				error("Wiz::drawAWizPrimEx(): Raw data type mismatch for mode %d vs %d", src_c, kWCTNone16Bpp);
			}

		} else {
			// Use the native transfer function
			if (optionalColorConversionTable &&
				((WizRawPixel *)_vm->getHEPaletteSlot(1) != optionalColorConversionTable)) {
				flags |= kWRFRemap;
				dataPtr = (void *)optionalColorConversionTable;
			}

			pgDrawRawDataFormatImage(
				dest_p, (WizRawPixel *)(src_d + _vm->_resourceHeaderSize), dest_w, dest_h,
				x, y, src_w, src_h, &clip_r, flags, dataPtr, transColorOverride);
		}
	}

	// Is this a print operation?
	if (flags & kWRFPrint) {
		warning("Wiz::drawAWizPrimEx(): Printing not yet supported");

		if ((flags & kWRFAlloc) == 0) {
			free(dest_p);
			dest_p = nullptr;
		}
	} else {
		if (!(flags & kWRFAlloc) && markUpdates) {
			dest_r.left = x;
			dest_r.top = y;
			dest_r.right = x + src_w - 1;
			dest_r.bottom = y + src_h - 1;

			if (findRectOverlap(&dest_r, &clip_r)) {
				// If neither foreground or background, copy to both
				if ((flags & kWRFBackground) || ((flags & (kWRFBackground | kWRFForeground)) == 0)) {
					_vm->backgroundToForegroundBlit(dest_r);
				} else {
					++dest_r.bottom;
					_vm->markRectAsDirty(kMainVirtScreen, dest_r);
				}
			}
		}
	}

	return dest_p;
}

void Wiz::buildAWiz(const WizRawPixel *bufPtr, int bufWidth, int bufHeight, const byte *palettePtr, const Common::Rect *rectPtr, int compressionType, int globNum, int transparentColor) {
	int dataSize, globSize, dataOffset, counter, height, width;
	Common::Rect compRect;
	byte *ptr;

	// Make sure that the coords passed in make sense.
	compRect.left = 0;
	compRect.top = 0;
	compRect.right = bufWidth - 1;
	compRect.bottom = bufHeight - 1;

	dataSize = 0;

	if (rectPtr) {
		if (!findRectOverlap(&compRect, rectPtr)) {
			error("Build wiz incorrect size (%d,%d,%d,%d)", rectPtr->left, rectPtr->top, rectPtr->right, rectPtr->bottom);
		}
	}

	// Force the compression type if in hi-color mode.
	if (_uses16BitColor) {
		compressionType = kWCTNone16Bpp;
	}

	// Estimate the size of the wiz.
	globSize = (_vm->_resourceHeaderSize * 3) + 12; // AWIZ, WIZH + data (12), WIZD

	if (!palettePtr) {
		globSize += (_vm->_resourceHeaderSize * 2) + 768 + 256 + 4; // RGBS + 768, RMAP + 256 + 4
	}

	if (compressionType == kWCTTRLE) {
		dataSize = TRLE_CompressImageArea(
			nullptr, bufPtr, bufWidth, compRect.left, compRect.top, compRect.right, compRect.bottom,
			(WizRawPixel)transparentColor);
	} else if (isUncompressedFormatTypeID(compressionType)) {
		dataSize = ((getRectWidth(&compRect) *  getRectHeight(&compRect)) * sizeof(WizRawPixel));
	} else {
		error("Unknown compression type %d", compressionType);
	}

	// Make sure that the "glob" is even sized...
	if (dataSize & 1) {
		dataSize++;
	}

	// Finalize the glob size!
	globSize += dataSize;

	// Actually build the wiz!
	ptr = (byte *)_vm->_res->createResource(rtImage, globNum, globSize);

	dataOffset = 0;

	// AWIZ block
	WRITE_BE_UINT32(ptr + 0, MKTAG('A', 'W', 'I', 'Z'));
	WRITE_BE_UINT32(ptr + 4, globSize); dataOffset += _vm->_resourceHeaderSize;

	// WIZH
	WRITE_BE_UINT32(ptr + dataOffset + 0, MKTAG('W', 'I', 'Z', 'H'));
	WRITE_BE_UINT32(ptr + dataOffset + 4, (12 + _vm->_resourceHeaderSize)); dataOffset += _vm->_resourceHeaderSize;
	WRITE_LE_UINT32(ptr + dataOffset, compressionType); dataOffset += 4; // COMPRESSION-TYPE
	WRITE_LE_UINT32(ptr + dataOffset, getRectWidth(&compRect)); dataOffset += 4; // WIDTH
	WRITE_LE_UINT32(ptr + dataOffset, getRectHeight(&compRect)); dataOffset += 4; // HEIGHT

	if (!palettePtr) {
		// RGBS
		WRITE_BE_UINT32(ptr + dataOffset + 0, MKTAG('R', 'G', 'B', 'S'));
		WRITE_BE_UINT32(ptr + dataOffset + 4, (768 + _vm->_resourceHeaderSize)); dataOffset += _vm->_resourceHeaderSize;
		memcpy(ptr + dataOffset, palettePtr, 768); dataOffset += 768;

		// RMAP
		WRITE_BE_UINT32(ptr + dataOffset + 0, MKTAG('R', 'M', 'A', 'P'));
		WRITE_BE_UINT32(ptr + dataOffset + 4, (256 + 4 + _vm->_resourceHeaderSize)); dataOffset += _vm->_resourceHeaderSize;
		WRITE_LE_UINT32(ptr + dataOffset, 0); dataOffset += 4; // Remapped flag

		for (counter = 0; counter < 256; counter++) {
			*(ptr + dataOffset) = counter;
			dataOffset++;
		}
	}

	// WIZD
	WRITE_BE_UINT32(ptr + dataOffset + 0, MKTAG('W', 'I', 'Z', 'D'));
	WRITE_BE_UINT32(ptr + dataOffset + 4, (dataSize + _vm->_resourceHeaderSize)); dataOffset += _vm->_resourceHeaderSize;

	if (compressionType == kWCTTRLE) {
		if (!_uses16BitColor) {
			TRLE_CompressImageArea(
				ptr + dataOffset, bufPtr, bufWidth,
				compRect.left, compRect.top, compRect.top, compRect.bottom,
				(byte)transparentColor);
		} else {
			error("Incorrect type %d for current pixel mode 16 bit", compressionType);
		}
	} else {
		WizSimpleBitmap srcBitmap, dstBitmap;
		Common::Rect dstRect;

		// Src setup
		srcBitmap.bufferPtr = (WizRawPixel *)bufPtr;
		srcBitmap.bitmapWidth = bufWidth;
		srcBitmap.bitmapHeight = bufHeight;

		// Dst setup
		width = getRectWidth(&compRect);
		height = getRectHeight(&compRect);

		dstBitmap.bufferPtr = (WizRawPixel *)(ptr + dataOffset);
		dstBitmap.bitmapWidth = width;
		dstBitmap.bitmapHeight = height;

		dstRect.left = 0;
		dstRect.top = 0;
		dstRect.right = width - 1;
		dstRect.bottom = height - 1;

		// Call the blit primitive. There should be a rendering
		// pipeline. That would allow things to happen much smoother.
		pgSimpleBlit(&dstBitmap, &dstRect, &srcBitmap, &compRect);
	}

	// Error check.
	dataOffset += dataSize;
	if (globSize != dataOffset) {
		error("WIZ size mismatch!");
	}
}

int Wiz::pixelHitTestWiz(int image, int state, int x, int y, int32 flags) {
	return pixelHitTestWizPrim(image, state, x, y, flags);
}

int Wiz::pixelHitTestWizPrim(int globNum, int state, int x, int y, int32 flags) {

#ifdef SCUMM_XTL_BRIDGE // 5/9/2000 BPT
	int outValue = 0;

	if (PU_OverrideImagePixelHitTest(&outValue, globType, globNum, state, x, y, flags)) {
		return outValue;
	}
#endif

	int src_c, src_w, src_h;
	byte *src_d;
	byte *pp;

	// Get the header (width, height + compression)
	pp = getWizStateHeaderPrim(globNum, state);

	src_c = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 0);
	src_w = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 4);
	src_h = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 8);

	// If the compression type is TRLE...
	if (src_c == kWCTTRLE) {
		src_d = getWizStateDataPrim(globNum, state);

		// Where should the color lookup happen?
		int pixel = auxPixelHitTestTRLEImageRelPos(
			src_d + _vm->_resourceHeaderSize, x, y, src_w, src_h,
			_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));

		int compType = getWizCompressionType(globNum, state);

		if (LITTLE_ENDIAN_WIZ(compType)) {
			return pixel;
		} else {
			if (WIZ_16BPP(compType)) {
				return SWAP_BYTES_16(pixel);
			} else {
				return pixel;
			}
		}

	} else if (isUncompressedFormatTypeID(src_c)) {
		WizSimpleBitmap srcBitmap;

		src_d = getWizStateDataPrim(globNum, state);

		srcBitmap.bufferPtr = (WizRawPixel *)(src_d + _vm->_resourceHeaderSize);
		srcBitmap.bitmapWidth = src_w;
		srcBitmap.bitmapHeight = src_h;

		int pixel = pgReadPixel(&srcBitmap, x, y, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
		int compType = getWizCompressionType(globNum, state);

		if (LITTLE_ENDIAN_WIZ(compType)) {
			return pixel;
		} else {
			if (WIZ_16BPP(compType)) {
				return SWAP_BYTES_16(pixel);
			} else {
				return pixel;
			}
		}
	} else {
		return _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR);
	}
}

int Wiz::hitTestWiz(int image, int state, int x, int y, int32 flags) {
	return hitTestWizPrim(image, state, x, y, flags);
}

int Wiz::hitTestWizPrim(int globNum, int state, int x, int y, int32 flags) {

#ifdef SCUMM_XTL_BRIDGE // 5/9/2000 BPT
	int outValue = 0;

	if (PU_OverrideImageHitTest(&outValue, globType, globNum, state, x, y, flags)) {
		return outValue;
	}
#endif

	int src_c, src_w, src_h;
	byte *src_d;
	byte *pp;

	// Get the header (width, height + compression)
	pp = getWizStateHeaderPrim(globNum, state);

	src_c = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 0);
	src_w = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 4);
	src_h = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 8);

	if (_vm->_game.heversion > 98) {
		// Flip the test coords if needed and do simple point rejection
		// (should be faster than calling the various functions)

		if (flags & kWRFHFlip) {
			x = ((src_w - 1) - x);

			if (x < 0) {
				return 0;
			}

		} else if ((x >= src_w) || (x < 0)) {
			return 0;
		}

		if (flags & kWRFVFlip) {
			y = ((src_h - 1) - y);

			if (y < 0) {
				return 0;
			}

		} else if ((y >= src_h) || (y < 0)) {
			return 0;
		}
	}

	// If the compression type is TRLE...
	if (src_c == kWCTTRLE) {
		src_d = getWizStateDataPrim(globNum, state);

		return auxHitTestTRLEImageRelPos(src_d + _vm->_resourceHeaderSize, x, y, src_w, src_h) ? 1 : 0;
	} else if (_vm->_game.heversion > 98 && isUncompressedFormatTypeID(src_c)) {
		WizSimpleBitmap srcBitmap;

		src_d = getWizStateDataPrim(globNum, state);

		srcBitmap.bufferPtr = (WizRawPixel *)(src_d + _vm->_resourceHeaderSize);
		srcBitmap.bitmapWidth = src_w;
		srcBitmap.bitmapHeight = src_h;

		return (_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) !=
				pgReadPixel(&srcBitmap, x, y, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR)))
					? 1
					: 0;
	} else {
		return 0;
	}
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
		if (_vm->_game.heversion <= 80 || READ_BE_UINT32(rmap) != WIZ_MAGIC_REMAP_NUMBER) {
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
	} else if (_lUseWizClipRect) {
		if (rScreen.intersects(_lWizClipRect)) {
			rScreen.clip(_lWizClipRect);
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
				_vm->backgroundToForegroundBlit(rImage);
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
		if (drawFlags & (kWRFHFlip | kWRFVFlip)) {
			uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), wizPtr, subState, 0);
			assert(wizh);
			srcw1 = READ_LE_UINT32(wizh + 0x4);
			srch1 = READ_LE_UINT32(wizh + 0x8);
		}

		if (drawFlags & kWRFHFlip)
			xPos = (srcw - (xPos + srcw1));

		if (drawFlags & kWRFVFlip)
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
	int polygon1, polygon2, compressionType, srcImage = 0, shadow = 0, state = 0;
	bool bIsHintColor = false;
	int iHintColor = 0;

	// Get all the options...
	if (params->actionFlags & kWAFPolygon) {
		polygon1 = params->polygon;
	} else {
		error("Image capture poly: no polygon 1 specified.");
	}

	if (params->actionFlags & kWAFPolygon2) {
		polygon2 = params->polygon2;
	} else {
		polygon2 = polygon1;
	}

	if (params->actionFlags & kWAFCompressionType) {
		compressionType = params->compressionType;
	} else {
		compressionType = kWCTNone;
	}

	if (params->actionFlags & kWAFShadow) {
		shadow = params->shadow;
	}

	if (params->actionFlags & kWAFDestImage) {
		error("Image capture poly: destination 'image' not supported, use 'source image'");
	}

	if (params->actionFlags & kWAFSourceImage) {
		srcImage = params->sourceImage;
	}

	if (params->actionFlags & kWAFState) {
		state = params->state;
	}

	if (params->actionFlags & kWAFProperty) {
		if (params->propertyNumber == 1) { // Color hint property
			if (!shadow) {
				debug(7, "ProcessWizImagePolyCaptureCmd: color hint does nothing for an unfiltered scale.");
			}

			bIsHintColor = true;
			iHintColor = params->propertyValue;
		}
	}

	// validate the parameters
	bool bPoly1Found = false;
	bool bPoly2Found = false;

	for (int polyIndex = 0; polyIndex < ARRAYSIZE(_polygons); ++polyIndex) {
		if (polygon1 == _polygons[polyIndex].id) {
			bPoly1Found = true;
			polygon1 = polyIndex;
			if (_polygons[polyIndex].numPoints != 5)
				error("Invalid point count");
		}

		if (polygon2 == _polygons[polyIndex].id) {
			bPoly2Found = true;
			polygon2 = polyIndex;
			if (_polygons[polyIndex].numPoints != 5)
				error("Invalid point count");
		}

		if (bPoly1Found && bPoly2Found) {
			break;
		}
	}

	if (!bPoly1Found) {
		error("Image capture poly: Polygon %d not defined", polygon1);
	}

	if (!bPoly2Found) {
		error("Image capture poly: Polygon %d not defined", polygon2);
	}

	if (_polygons[polygon1].numPoints != _polygons[polygon2].numPoints) {
		error("Image capture poly: Polygons MUST have same number of points.");
	}

	// create the buffers to hold the source and dest image bits
	// since a lot of drawing commands will be called, and we can't gurantee
	// that the source image will stick around, we make a buffer big enough
	// to contain it and then copy the bits in.

	WizSimpleBitmap srcBitmap, destBitmap;
	srcBitmap.bufferPtr = nullptr;
	destBitmap.bufferPtr = nullptr;

	// we need to get the poly points

	// build a bounding rect for the polys and set the appropriate sizes in the bitmaps
	Common::Rect destPolyRect;
	Common::Rect srcPolyRect;

	polyBuildBoundingRect(_polygons[polygon2].points, _polygons[polygon2].numPoints, destPolyRect);
	destBitmap.bitmapWidth = getRectWidth(&destPolyRect);
	destBitmap.bitmapHeight = getRectHeight(&destPolyRect);
	destBitmap.bufferPtr = (WizRawPixel *)malloc(destBitmap.bitmapWidth * destBitmap.bitmapHeight * sizeof(WizRawPixel));

	if (destBitmap.bufferPtr == 0) {
		error("Image capture poly: Could not allocate destination buffer");
	}

	// fill with transparent color
	rawPixelMemset(destBitmap.bufferPtr,
					_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
					destBitmap.bitmapWidth * destBitmap.bitmapHeight);

	// get the bound rect for the poly
	polyBuildBoundingRect(_polygons[polygon1].points, _polygons[polygon1].numPoints, srcPolyRect);

	// we need to get the points so that they can be offset to the correct position in
	// the source buffer in the case of a screen capture (since the whole screen may not
	// be captured, and the polygon may be offset into it, so we only create a buffer of the size needed
	int iPointCt;
	Common::Point srcPoints[5];
	for (iPointCt = 0; iPointCt < 5; ++iPointCt) {
		srcPoints[iPointCt].x = _polygons[polygon1].points[iPointCt].x;
		srcPoints[iPointCt].y = _polygons[polygon1].points[iPointCt].y;
	}

	// check for one to one rectangle, which will set up for an image copy later
	bool bOneToOneRect = false;
	// see if they are both rectangles, passing in 4 because it turns out you can only create a 4 vertex polygon in scumm
	// according to set4Polygon
	if (polyIsRectangle(_polygons[polygon1].points, 4) && polyIsRectangle(_polygons[polygon2].points, 4)) {
		// check if the points are all the same
		for (iPointCt = 0; iPointCt < 4; ++iPointCt) {
			if ((_polygons[polygon1].points[iPointCt].x != _polygons[polygon2].points[iPointCt].x) ||
				(_polygons[polygon1].points[iPointCt].y != _polygons[polygon2].points[iPointCt].y)) {
				break;
			}
		}

		if (iPointCt == 4) {
			bOneToOneRect = true;
		}
	}

	// if there is a source image, get its bits, otherwise capture from the screen
	if (srcImage) {
		// get the wiz size
		Common::Rect clipRect;
		int w, h;
		getWizImageDim(srcImage, state, w, h);

		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = w;
		clipRect.bottom = h;

		// make sure capture area isn't outside or bigger than the source image
		Common::Rect testRect;
		combineRects(&testRect, &srcPolyRect, &clipRect);

		if ((getRectWidth(&testRect) * getRectHeight(&testRect)) >
			(getRectWidth(&clipRect) * getRectHeight(&clipRect))) {
			error("Image capture poly: Specified polygon captures points outside bounds of source image");
		}

		// clip poly to image and verify it's within the image
		if (!findRectOverlap(&srcPolyRect, &clipRect)) {
			error("Image capture poly: Specified polygon doesn't intersect source image.");
		}

		srcBitmap.bitmapWidth = getRectWidth(&srcPolyRect);
		srcBitmap.bitmapHeight = getRectHeight(&srcPolyRect);

		if ((srcBitmap.bitmapWidth == 0) || (srcBitmap.bitmapHeight == 0)) {
			error("Image capture poly: Poly or source image invalid");
		}

		// create the bitmap
		srcBitmap.bufferPtr = (WizRawPixel *)malloc(srcBitmap.bitmapWidth * srcBitmap.bitmapHeight * sizeof(WizRawPixel));
		if (srcBitmap.bufferPtr == 0) {
			error("Image capture poly: Could not allocate source buffer");
		}

		// clear to transparent
		rawPixelMemset(srcBitmap.bufferPtr,
						_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
						srcBitmap.bitmapWidth * srcBitmap.bitmapHeight);

		drawAWiz(srcImage, state, 0, 0, 0, 0, 0, 0, &srcPolyRect, 0, &srcBitmap);
	} else { // we must be capturing from screen

		// get the window size
		VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
		int iWindowWidth =  pvs->w;
		int iWindowHeight = pvs->h;

		// intersect the bound rect and the window rect
		Common::Rect clipRect;
		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = iWindowWidth - 1;
		clipRect.bottom = iWindowHeight - 1;

		if (!findRectOverlap(&srcPolyRect, &clipRect)) {
			error("Image capture poly: Specified polygon doesn't intersect screen.");
		}

		srcBitmap.bitmapWidth = getRectWidth(&srcPolyRect);
		srcBitmap.bitmapHeight = getRectHeight(&srcPolyRect);

		if ((srcBitmap.bitmapWidth == 0) || (srcBitmap.bitmapHeight == 0)) {
			error("Image capture poly: Specified screen rectangle invalid.");
		}

		// create the bitmap
		srcBitmap.bufferPtr = (WizRawPixel *)malloc(srcBitmap.bitmapWidth * srcBitmap.bitmapHeight * sizeof(WizRawPixel));
		if (!srcBitmap.bufferPtr) {
			error("Image capture poly: Could not allocate source buffer");
		}

		// clear to transparent
		rawPixelMemset(srcBitmap.bufferPtr,
						_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
						srcBitmap.bitmapWidth * srcBitmap.bitmapHeight);

		// fill it with screen data
		WizRawPixel *screenPtr = (WizRawPixel *)pvs->getPixels(srcPolyRect.left, srcPolyRect.top);
		WizRawPixel *destPtr;

		destPtr = srcBitmap.bufferPtr;

		int iScreenRowLen = 640 * sizeof(WizRawPixel);
		int iDestRowLen = srcBitmap.bitmapWidth * sizeof(WizRawPixel);

		for (int iRow = 0; iRow < srcBitmap.bitmapHeight; ++iRow) {
			memcpy(destPtr, screenPtr, iDestRowLen);
			screenPtr += iScreenRowLen;
			destPtr += iDestRowLen;
		}

		// translate the polygon so it is in the correct place in the buffer
		int iDX = 0, iDY = 0;
		iDX = 0 - srcPolyRect.left;
		iDY = 0 - srcPolyRect.top;
		polyMovePolygonPoints(srcPoints, _polygons[polygon1].numPoints, iDX, iDY);
	}

	// if there is an xmap, do filtered warp
	if (shadow) {
		// get the color map, bypass the header information
		byte *pXmapColorTable = (byte *)getColorMixBlockPtrForWiz(shadow);

		if (!pXmapColorTable) {
			error("Image capture poly: Shadow specified but not present in image.");
		}

		pXmapColorTable += _vm->_resourceHeaderSize;

		WARPWIZ_NPt2NPtNonClippedWarpFiltered(
			&destBitmap, _polygons[polygon2].points, &srcBitmap, srcPoints,
			_polygons[polygon1].numPoints, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
			pXmapColorTable, bIsHintColor, (WizRawPixel)iHintColor);

	} else if (bOneToOneRect) { // if a one to one copy is performed, just copy this bits
		memcpy(destBitmap.bufferPtr, srcBitmap.bufferPtr, destBitmap.bitmapHeight * destBitmap.bitmapWidth);
	} else { // otherwise do "old" warp
		WARPWIZ_NPt2NPtNonClippedWarp(
			&destBitmap, _polygons[polygon2].points, &srcBitmap, srcPoints,
			_polygons[polygon1].numPoints, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
	}

	// now build a wiz with the destination bitmap and throw the bitmaps away
	if (srcBitmap.bufferPtr) {
		free(srcBitmap.bufferPtr);
		srcBitmap.bufferPtr = nullptr;
	}

	uint8 *palPtr = nullptr;
	if (_vm->_game.heversion >= 99) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
	} else {
		palPtr = _vm->_currentPalette;
	}

	buildAWiz(destBitmap.bufferPtr,
			  destBitmap.bitmapWidth,
			  destBitmap.bitmapHeight,
			  palPtr,
			  &destPolyRect,
			  compressionType,
			  params->image,
			  _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));

	if (destBitmap.bufferPtr) {
		free(destBitmap.bufferPtr);
		destBitmap.bufferPtr = nullptr;
	}

	_vm->_res->setModified(rtImage, params->image);
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
	if (wp->numPoints != 5) {
		error("Invalid point count %d for Polygon %d", wp->numPoints, id);
	}

	drawWizPolygonTransform(resNum, state, wp->points, flags, shadow, dstResNum, palette);
}

void Wiz::drawWizPolygonTransform(int resNum, int state, Common::Point *wp, int flags, int shadow, int dstResNum, int palette) {
	debug(7, "drawWizPolygonTransform(resNum %d, flags 0x%X, shadow %d dstResNum %d palette %d)", resNum, flags, shadow, dstResNum, palette);
	const Common::Rect *r = nullptr;
	uint8 *srcWizBuf = nullptr;
	bool freeBuffer = true;

	if (_vm->_game.heversion >= 99) {
		if (getWizImageData(resNum, state, 0) != 0 || (flags & (kWRFRemap | kWRFHFlip | kWRFVFlip)) || palette != 0) {
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
		++bound.bottom;
		_vm->markRectAsDirty(kMainVirtScreen, bound);
	} else {
		_vm->backgroundToForegroundBlit(bound);
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

void Wiz::flushAWizBuffer() {
	for (int i = 0; i < _wizBufferIndex; ++i) {
		WizBufferElement *pwi = &_wizBuffer[i];
		if (pwi->flags & kWRFPolygon) {
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
	// TODO
	//uint8 *cursor = drawWizImage(resId, 0, 0, 0, 0, 0, 0, 0, 0, r, kWRFAlloc, 0, _vm->getHEPaletteSlot(palette), 0);
	uint8 *cursor = (uint8 *)drawAWizPrim(resId, 0, 0, 0, 0, 0, 0, 0, kWRFAlloc, 0, (WizRawPixel *)_vm->getHEPaletteSlot(palette));
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
	int shadowImage, state, angle, scale, paletteNumber, sourceImage;
	int zbufferImage = 0;
	const WizRawPixel *colorConversionTablePtr;
	Common::Rect *optionalRect;
	WizSimpleBitmap *destBitmap;
	WizSimpleBitmap fakeBitmap;
	Common::Rect clipRect;
	int32 flags;
	Common::Point pt;

	int requiredImages[5] = {0, 0, 0, 0, 0};
	int requiredImageCount = 0;

	ADD_REQUIRED_IMAGE(params->image);

	if (params->actionFlags & kWAFSourceImage) {
		sourceImage = params->sourceImage;
		ADD_REQUIRED_IMAGE(sourceImage);
	} else {
		sourceImage = 0;
	}

	if (params->actionFlags & kWAFPalette) {
		paletteNumber = params->palette;
	} else {
		paletteNumber = 0;
	}

	if (params->actionFlags & kWAFScale) {
		scale = params->scale;
	} else {
		scale = 256;
	}

	if (params->actionFlags & kWAFAngle) {
		angle = params->angle;
	} else {
		angle = 0;
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
		pt.x = params->xPos;
		pt.y = params->yPos;
	} else {
		pt.x = 0;
		pt.y = 0;
	}

	if (params->actionFlags & kWAFShadow) {
		shadowImage = params->shadow;
		ADD_REQUIRED_IMAGE(shadowImage);
	} else {
		shadowImage = 0;
	}

	if (params->actionFlags & kWAFZBufferImage) {
		zbufferImage = params->zbufferImage;
		ADD_REQUIRED_IMAGE(zbufferImage);
	}

	if (params->actionFlags & kWAFRect) {
		clipRect.left = params->box.left;
		clipRect.top = params->box.top;
		clipRect.right = params->box.right;
		clipRect.bottom = params->box.bottom;
		optionalRect = &clipRect;
	} else {
		optionalRect = nullptr;
	}

	if (params->actionFlags & kWAFDestImage) {
		ADD_REQUIRED_IMAGE(params->destImageNumber);
	}

	if (requiredImageCount > 0) {
		// Mark the all the image globs as stuck
		for (int i = 0; i < requiredImageCount; i++) {
			_vm->_res->lock(rtImage, requiredImages[i]);
		}

		// Make sure that the globs are in RAM by requesting their address
		// Do this twice to hopefully ensure that there will be no heap movement
		for (int i = 0; i < requiredImageCount; i++) {
			_vm->ensureResourceLoaded(rtImage, requiredImages[i]);
		}

		// Mark the all the image globs as NOT stuck
		for (int i = 0; i < requiredImageCount; i++) {
			_vm->_res->unlock(rtImage, requiredImages[i]);
		}

		// Validate that all of the images are on the heap!
		for (int i = 0; i < requiredImageCount; i++) {
			if (!_vm->getResourceAddress(rtImage, requiredImages[i])) {
				error("Wiz::processWizImageDrawCmd(): Image %d missing for image operation", requiredImages[i]);
			}
		}

		// See if the images are in their native format and twiddle if need be.
		for (int i = 0; i < requiredImageCount; i++) {
			ensureNativeFormatImageForState(requiredImages[i], state);
		}
	}

	if (params->actionFlags & kWAFDestImage) {
		// Get the rendering surface for this image
		if (!dwSetSimpleBitmapStructFromImage(params->destImageNumber, 0, &fakeBitmap)) {
			error("Wiz::processWizImageDrawCmd(): Image %d is invalid for rendering into", params->destImageNumber);
		}

		destBitmap = &fakeBitmap;
	} else {
		destBitmap = nullptr;
	}

	if (_vm->_game.heversion >= 99 && (params->actionFlags & kWAFRemapList)) {
		processWizImageModifyCmd(params);
		flags |= kWRFRemap;
	}

	// Dispatch the command...
	if (!_vm->_fullRedraw || destBitmap != 0) {
		if (sourceImage != 0) {
			dwAltSourceDrawWiz(
				params->image, state, pt.x, pt.y,
				sourceImage, 0, flags, paletteNumber,
				optionalRect, destBitmap);
		} else {
			if (!(params->actionFlags & (kWAFScale | kWAFAngle))) {
				drawAWizEx(
					params->image, state,
					pt.x, pt.y, params->zPos,
					flags,
					shadowImage, zbufferImage,
					optionalRect,
					paletteNumber,
					destBitmap,
					params);
			} else {
				if (paletteNumber) {
					colorConversionTablePtr = (WizRawPixel *)_vm->getHEPaletteSlot(paletteNumber);
				} else {
					colorConversionTablePtr = nullptr;
				}

				dwHandleComplexImageDraw(
					params->image, state, pt.x, pt.y, shadowImage, angle, scale,
					optionalRect, flags, destBitmap, colorConversionTablePtr);
			}
		}
	} else {
		if (sourceImage != 0 || (params->actionFlags & (kWAFScale | kWAFAngle)))
			error("Can't do this command in the enter script");

		bufferAWiz(params->image, state, pt.x, pt.y, params->zPos, flags, shadowImage, zbufferImage, paletteNumber);
	}
}

#undef ADD_REQUIRED_IMAGE

void Wiz::dwCreateRawWiz(int imageNum, int w, int h, int flags, int bitsPerPixel, int optionalSpotX, int optionalSpotY) {
	int compressionType, wizdSize;

	int globSize = _vm->_resourceHeaderSize; // AWIZ header size
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

	globSize += _vm->_resourceHeaderSize; // WIZD header size
	wizdSize = (w * h * (bitsPerPixel / 8));
	globSize += wizdSize;

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
		const uint8 *palPtr;
		if (_vm->_game.heversion >= 99) {
			palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
		} else {
			palPtr = _vm->_currentPalette;
		}

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

	// Get the image header
	wizHeader = (byte *)getWizStateHeaderPrim(imageNum, imageState);

	if (!wizHeader) {
		return false;
	}

	// Double check the image header compression type
	compType = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize);

	if (!isUncompressedFormatTypeID(compType)) {
		return false;
	}

	imageWidth = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize + 4);
	imageHeight = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize + 8);

	// Do some fun stuff
	dataPtr = (byte *)getWizStateDataPrim(imageNum, imageState);

	if (!dataPtr) {
		return false;
	}

	// Hook up the image info to the simple bitmap info
	destBM->bufferPtr = (WizRawPixel *)(dataPtr + _vm->_resourceHeaderSize);
	destBM->bitmapWidth = imageWidth;
	destBM->bitmapHeight = imageHeight;

	return true;
}

int Wiz::dwTryToLoadWiz(Common::SeekableReadStream *inFile, const WizImageCommand *params) {
	uint32 blockSize;
	uint32 blockId;
	byte *ptr;

	inFile->seek(0, SEEK_SET);
	blockId = inFile->readUint32BE();

	if ((blockId != MKTAG('A', 'W', 'I', 'Z')) && (blockId != MKTAG('M', 'U', 'L', 'T'))) {
		return DW_LOAD_NOT_TYPE;
	}

	blockSize = inFile->readUint32BE();
	inFile->seek(-8, SEEK_CUR);

	ptr = _vm->_res->createResource(rtImage, params->image, blockSize);

	if (inFile->read(ptr, blockSize) != blockSize) {
		_vm->_res->nukeResource(rtImage, params->image);
		return DW_LOAD_READ_FAILURE;
	}

	_vm->_res->setModified(rtImage, params->image);
	return DW_LOAD_SUCCESS;
}

void Wiz::dwAltSourceDrawWiz(int maskImage, int maskState, int x, int y, int sourceImage, int sourceState, int32 flags, int paletteNumber, const Common::Rect *optionalClipRect, const WizSimpleBitmap *destBitmapPtr) {
	int srcBitsPerPixel, sourceCompressionType, maskCompressionType;
	int srcBitmapWidth, srcBitmapHeight, maskWidth, maskHeight;
	Common::Rect clipRect, destRect;
	WizSimpleBitmap drawBufferBitmap;
	WizRawPixel *conversionTable;
	byte *sourceBufferPtr;
	byte *maskDataPtr;
	bool markUpdates;

	// Get the conversion table if any
	if (paletteNumber) {
		conversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(paletteNumber);
	} else {
		if (_uses16BitColor) {
			conversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(1); // Generic conversion table
		} else {
			conversionTable = nullptr;
		}
	}

	// Get the dest bitmap ptr
	if (!destBitmapPtr) {
		markUpdates = true;

		if (flags & kWRFForeground) {
			pgSimpleBitmapFromDrawBuffer(&drawBufferBitmap, false);
		} else {
			pgSimpleBitmapFromDrawBuffer(&drawBufferBitmap, true);
		}

		destBitmapPtr = &drawBufferBitmap;
	} else {
		markUpdates = false;
	}

	// Check for overlap with the optional clip rectangle, if any
	clipRect.left = 0;
	clipRect.top = 0;
	clipRect.right = destBitmapPtr->bitmapWidth - 1;
	clipRect.bottom = destBitmapPtr->bitmapHeight - 1;

	if (optionalClipRect) {
		if (!findRectOverlap(&clipRect, optionalClipRect)) {
			return; // We're done there is no update region.
		}
	}

	// Get the source (alt) bitmap ptr & general info
	sourceCompressionType = getWizCompressionType(sourceImage, sourceState);

	if (!dwIsUncompressedFormatTypeID(sourceCompressionType)) {
		error("Wiz::dwAltSourceDrawWiz(): Source image %d must be uncompressed.", sourceImage);
	}

	switch (sourceCompressionType) {

	default:
	case kWCTNone:
		srcBitsPerPixel = 8; // Default
		break;

	case kWCTNone16Bpp:
	case kWCTNone16BppBigEndian:
		srcBitsPerPixel = 16;
		break;
	}

	// Get the source wiz data pointer.
	sourceBufferPtr = (byte *)getWizStateDataPrim(sourceImage, sourceState);

	if (!sourceBufferPtr) {
		error("Wiz::dwAltSourceDrawWiz(): Image %d missing data block", sourceImage);
	}

	sourceBufferPtr += 8;
	getWizImageDim(sourceImage, sourceState, srcBitmapWidth, srcBitmapHeight);

	if ((destBitmapPtr->bitmapWidth != srcBitmapWidth) ||
		(destBitmapPtr->bitmapHeight != srcBitmapHeight)) {

		error(
			"Wiz::dwAltSourceDrawWiz(): Source image %d and dest image size mismatch (%d,%d) need (%d,%d)",
			sourceImage, srcBitmapWidth, srcBitmapHeight,
			destBitmapPtr->bitmapWidth, destBitmapPtr->bitmapHeight);
	}

	// Finally get the compressed data pointer
	maskCompressionType = getWizCompressionType(maskImage, maskState);

	if (!dwIsMaskCompatibleCompressionType(maskCompressionType)) {
		error("Wiz::dwAltSourceDrawWiz(): Mask image %d must be a maskable compression type.", maskImage);
	}

	maskDataPtr = (byte *)getWizStateDataPrim(maskImage, maskState);

	if (!maskDataPtr) {
		error("Wiz::dwAltSourceDrawWiz(): Image %d missing data block", maskImage);
	}

	maskDataPtr += 8;
	getWizImageDim(maskImage, maskState, maskWidth, maskHeight);

	// Make sure that we have an overlap before we call the decompressor
	destRect.left = x;
	destRect.top = y;
	destRect.right = x + maskWidth - 1;
	destRect.bottom = y + maskHeight - 1;

	if (!findRectOverlap(&destRect, &clipRect)) {
		// We're done, there is no update region...
		return;
	}

	// Finally call the primitive...
	if (maskCompressionType == kWCTTRLE) {
		TRLEFLIP_AltSource_DecompressImage(
			destBitmapPtr->bufferPtr, maskDataPtr,
			destBitmapPtr->bitmapWidth, destBitmapPtr->bitmapHeight,
			sourceBufferPtr, srcBitmapWidth, srcBitmapHeight, srcBitsPerPixel,
			x, y, maskWidth, maskHeight, &clipRect, flags, conversionTable,
			nullptr);

	} else if (maskCompressionType == kWCTMRLEWithLineSizePrefix) {
		MRLEFLIP_AltSource_DecompressImage(
			destBitmapPtr->bufferPtr, maskDataPtr,
			destBitmapPtr->bitmapWidth, destBitmapPtr->bitmapHeight,
			sourceBufferPtr, srcBitmapWidth, srcBitmapHeight, srcBitsPerPixel,
			x, y, maskWidth, maskHeight, &clipRect, flags, conversionTable);
	}

	// What type of update is necessary?
	if (!(flags & kWRFAlloc) && markUpdates) {
		// If neither foreground or background, copy to both...
		if ((flags & (kWRFBackground | kWRFForeground)) == 0) {
			_vm->backgroundToForegroundBlit(destRect);
		} else {
			++destRect.bottom;
			_vm->markRectAsDirty(kMainVirtScreen, destRect);
		}
	}
}

#define SWAP_POINTS(a, b) { \
     int t;                 \
     t = *b;                \
     *b = *a;               \
     *b = t;                \
}


void Wiz::dwHandleComplexImageDraw(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	int w, h, correctedAngle;
	Common::Point listOfPoints[4];
	byte *shadowPtr;

	// Set the optional remap table up to the default if one isn't specified
	if (!optionalColorConversionTable) {
		if (!_uses16BitColor) {
			optionalColorConversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(1);
		}
	}

	// Setup the initial quad (0,0) relative...
	getWizImageDim(image, state, w, h);

	listOfPoints[0].x = -w / 2;
	listOfPoints[0].y = -h / 2;

	listOfPoints[1].x = listOfPoints[0].x + w - 1;
	listOfPoints[1].y = listOfPoints[0].y;

	listOfPoints[2].x = listOfPoints[1].x;
	listOfPoints[2].y = listOfPoints[0].y + h - 1;

	listOfPoints[3].x = listOfPoints[0].x;
	listOfPoints[3].y = listOfPoints[2].y;

	// Hflip
	if (flags & kWRFHFlip) {
		SWAP_POINTS(&listOfPoints[0].x, &listOfPoints[1].x);
		SWAP_POINTS(&listOfPoints[2].x, &listOfPoints[3].x);
	}

	// VFlip
	if (flags & kWRFVFlip) {
		SWAP_POINTS(&listOfPoints[0].y, &listOfPoints[1].y);
		SWAP_POINTS(&listOfPoints[2].y, &listOfPoints[3].y);
	}

	// Scale the points?
	if (scale != 256) {
		for (int i = 0; i < 4; i++) {
			listOfPoints[i].x = (scale * listOfPoints[i].x) / 256;
			listOfPoints[i].y = (scale * listOfPoints[i].y) / 256;
		}
	}

	// Rotate the points
	if (angle) {
		polyRotatePoints(listOfPoints, 4, angle);
	}

	// Offset the points
	polyMovePolygonPoints(listOfPoints, 4, x, y);

	// Special case rotate 0,90,180,270 degree special cases
	if (scale == 256) {
		Common::Rect boundingRect;

		correctedAngle = abs(angle % 360);

		if (angle < 0) {
			correctedAngle = (360 - correctedAngle);
		}

		// Get the upper left point so that our blit matches in position
		// the normal warp drawing function.
		polyBuildBoundingRect(listOfPoints, 4, boundingRect);
		x = boundingRect.left;
		y = boundingRect.top;

		// Special case renderers don't use shadows...
		if (!shadow) {
			switch (correctedAngle) {
			case 0:
				handleRotate0SpecialCase(
					image, state, x, y, shadow, correctedAngle, scale, clipRect, flags,
					optionalBitmapOverride, optionalColorConversionTable);
				return;

			case 90:
				handleRotate90SpecialCase(
					image, state, x, y, shadow, correctedAngle, scale, clipRect, flags,
					optionalBitmapOverride, optionalColorConversionTable);
				return;

			case 180:
				handleRotate180SpecialCase(
					image, state, x, y, shadow, correctedAngle, scale, clipRect, flags,
					optionalBitmapOverride, optionalColorConversionTable);
				return;

			case 270:
				handleRotate270SpecialCase(
					image, state, x, y, shadow, correctedAngle, scale, clipRect, flags,
					optionalBitmapOverride, optionalColorConversionTable);
				return;
			}
		}
	}

	// If there is a shadow get it's address
	if (shadow) {
		shadowPtr = (byte *)getColorMixBlockPtrForWiz(shadow);
		shadowPtr += _vm->_resourceHeaderSize;
	} else {
		shadowPtr = nullptr;
	}

	// Finally call the renderer
	WARPWIZ_DrawWizTo4Points(
		image, state, listOfPoints, flags,
		_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
		clipRect, optionalBitmapOverride, optionalColorConversionTable,
		shadowPtr);
}

#undef SWAP_POINTS

bool Wiz::dwIsMaskCompatibleCompressionType(int compressionType) {
	return (kWCTTRLE == compressionType) || (kWCTMRLEWithLineSizePrefix == compressionType);
}

bool Wiz::dwIsUncompressedFormatTypeID(int id) {
	return ((kWCTNone == id)      ||
		    (kWCTNone16Bpp == id) ||
		    (kWCTNone16BppBigEndian == id));
}

int Wiz::dwGetImageGeneralProperty(int image, int state, int property) {
	switch (property) {
	case kWIPCompressionType:
		return getWizCompressionType(image, state);
		break;

	case kWIPPaletteBlockPresent:
		return doesStateContainBlock(image, state, MKTAG('R', 'G', 'B', 'S'));
		break;

	case kWIPRemapBlockPresent:
		return doesStateContainBlock(image, state, MKTAG('R', 'M', 'A', 'P'));
		break;

	case kWIPOpaqueBlockPresent:
		return doesRawWizStateHaveTransparency(image, state);
		break;

	case kWIPXMAPBlockPresent:
		return doesStateContainBlock(image, state, MKTAG('X', 'M', 'A', 'P'));
		break;

	default:
		debug("Wiz::dwGetImageGeneralProperty(): image %d state %d property %d (unknown property id).", image, state, property);
		break;
	}

	return 0;
}

void Wiz::handleRotate0SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	drawAWizPrim(image, state, x, y, 0, shadow, 0, clipRect, flags, optionalBitmapOverride, optionalColorConversionTable);
}

void Wiz::handleRotate90SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	WizSimpleBitmap srcBitmap, dstBitmap;
	Common::Rect updateRect;
	int compressionType;

	// Make the update rect and check it against the clip rect if one
	int w, h;
	getWizImageDim(image, state, w, h);
	makeSizedRectAt(&updateRect, x, y, h, w); // We are swapping height and width on purpose!

	if (clipRect) {
		if (!findRectOverlap(&updateRect, clipRect)) {
			return;
		}
	}

	// Check to see if this is yet another special case :-)
	compressionType = getWizCompressionType(image, state);

	if (compressionType == kWCTTRLE) {
		int dest_w, dest_h, src_w, src_h;
		const byte *compressedDataPtr;
		WizRawPixel *dest_p;

		// Get the size of the compressed image
		src_w = w;
		src_h = h;

		// Get the compressed data ptr!
		compressedDataPtr = (byte *)getWizStateDataPrim(image, state);
		compressedDataPtr += _vm->_resourceHeaderSize;

		// Get the write data...
		if (optionalBitmapOverride) {
			dest_p = optionalBitmapOverride->bufferPtr;
			dest_w = optionalBitmapOverride->bitmapWidth;
			dest_h = optionalBitmapOverride->bitmapHeight;
		} else {
			VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];

			dest_w = pvs->w;
			dest_h = pvs->h;

			if (flags & kWRFForeground) {
				dest_p = (WizRawPixel *)pvs->getPixels(0, 0);
			} else {
				dest_p = (WizRawPixel *)pvs->getBackPixels(0, 0);
			}
		}

		TRLEFLIP_Rotate90_DecompressImage(
			dest_p, compressedDataPtr, dest_w, dest_h, x, y, src_w, src_h,
			clipRect, flags, nullptr, optionalColorConversionTable,
			nullptr);

		// Update the screen? (If not writing to another bitmap...)
		if (!optionalBitmapOverride) {
			if (!(flags & kWRFForeground)) {
				_vm->backgroundToForegroundBlit(updateRect);
			} else {
				++updateRect.bottom;
				_vm->markRectAsDirty(kMainVirtScreen, updateRect);
			}
		}

		return;
	}

	// Get the image from the basic drawing function...
	srcBitmap.bufferPtr = (WizRawPixel *)drawAWizPrim(
		image, state, 0, 0, 0, 0, 0, 0, kWRFAlloc,
		0, optionalColorConversionTable);

	srcBitmap.bitmapWidth = w;
	srcBitmap.bitmapHeight = h;

	// Get the bitmap to render into...
	if (optionalBitmapOverride) {
		dstBitmap = *optionalBitmapOverride;
	} else {
		pgSimpleBitmapFromDrawBuffer(&dstBitmap, (kWRFForeground != (kWRFForeground & flags)));
	}

	// Call the 90 blit function...
	if (_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) == -1) {
		pgBlit90DegreeRotate(
			&dstBitmap, x, y, &srcBitmap, nullptr, clipRect,
			(flags & kWRFHFlip), (flags & kWRFVFlip));
	} else {
		pgBlit90DegreeRotateTransparent(
			&dstBitmap, x, y, &srcBitmap, nullptr, clipRect,
			(flags & kWRFHFlip), (flags & kWRFVFlip),
			(WizRawPixel)_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
	}

	// Free up working bitmap from
	free(srcBitmap.bufferPtr);
	srcBitmap.bufferPtr = nullptr;

	// Update the screen? (If not writing to another bitmap...)
	if (!optionalBitmapOverride) {
		if (!(flags & kWRFForeground)) {
			_vm->backgroundToForegroundBlit(updateRect);
		} else {
			++updateRect.bottom;
			_vm->markRectAsDirty(kMainVirtScreen, updateRect);
		}
	}
}

void Wiz::handleRotate180SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	flags ^= (kWRFVFlip | kWRFHFlip);

	drawAWizPrim(image, state, x, y, 0, shadow, 0, clipRect, flags, optionalBitmapOverride, optionalColorConversionTable);
}

void Wiz::handleRotate270SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	flags ^= (kWRFVFlip | kWRFHFlip);

	handleRotate90SpecialCase(
		image, state, x, y, shadow, angle, scale,
		clipRect, flags, optionalBitmapOverride,
		optionalColorConversionTable);
}

void Wiz::processWizImageRenderRectCmd(const WizImageCommand *params) {
	Common::Rect renderRect, clipRect, workClipRect;
	int whichState, w, h, whichImage;
	WizSimpleBitmap renderBitmap;
	WizRawPixel whatColor;

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
	WizRawPixel whatColor;
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
	WizRawPixel whatColor;
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

void Wiz::remapImage(int image, int state, int tableCount, const uint8 *remapList, const uint8 *remapTable) {
	int tableIndex;
	uint8 *resAddr, *basePtr, *tablePtr;

	resAddr = _vm->getResourceAddress(rtImage, image);
	assert(resAddr);
	basePtr = _vm->findWrappedBlock(MKTAG('R', 'M', 'A', 'P'), resAddr, state, 0);
	assert(basePtr);

	tablePtr = basePtr + 4;

	_vm->_res->setModified(rtImage, image);
	WRITE_BE_UINT32(basePtr, WIZ_MAGIC_REMAP_NUMBER);

	for (int i = 0; i < tableCount; i++) {
		tableIndex = *remapList++;
		tablePtr[tableIndex] = remapTable[tableIndex];
	}
}

int Wiz::createHistogramArrayForImage(int image, int state, const Common::Rect *optionalClipRect) {
	int src_c, src_w, src_h, globNum;
	Common::Rect realClippedRect;
	int histogramTable[256];
	byte *src_d;
	byte *pp;

	// This only makes sense in 8 bit color!!!!
	globNum = image;

	_vm->writeVar(0, 0);
	((ScummEngine_v72he *)_vm)->defineArray(0, _vm->kDwordArray, 0, 0, 0, 255);

	if (_vm->readVar(0) != 0) {
		// Get the header (width, height + compression)
		pp = getWizStateHeaderPrim(globNum, state);

		src_c = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 0);
		src_w = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 4);
		src_h = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 8);

		// Clip the passed in coords to the real dimensions of the image
		makeSizedRect(&realClippedRect, src_w, src_h);

		if (optionalClipRect) {
			if (!findRectOverlap(&realClippedRect, optionalClipRect)) {
				return _vm->readVar(0);
			}
		}

		// Get the data pointer
		src_d = getWizStateDataPrim(globNum, state);

		// Start with a clean array...
		memset(histogramTable, 0, sizeof(histogramTable));

		// Handle the different compression types...
		if (src_c == kWCTTRLE) {
			// Get the histogram...
			auxHistogramTRLEPrim(histogramTable, src_d + _vm->_resourceHeaderSize, &realClippedRect);
		} else if (src_c == kWCTNone) {
			WizSimpleBitmap srcBitmap;

			srcBitmap.bufferPtr = (WizRawPixel *)(src_d + _vm->_resourceHeaderSize);
			srcBitmap.bitmapWidth = src_w;
			srcBitmap.bitmapHeight = src_h;

			pgHistogramBitmapSubRect(histogramTable, &srcBitmap, &realClippedRect);
		} else {
			warning("Wiz::createHistogramArrayForImage(): Unable to return histogram for type %d", src_c);
		}

		// Fill in the scumm array with the values...
		if (_vm->readVar(0) != 0) {
			for (int i = 0; i < 256; i++) {
				((ScummEngine_v72he *)_vm)->writeArray(0, 0, i, histogramTable[i]);
			}
		}
	}

	return _vm->readVar(0); // Too much fun!!!!
}

void Wiz::ensureNativeFormatImageForState(int image, int state) {
	// If AWIZ block is an XMAP, we don't want to do anything with it
	if (dwGetImageGeneralProperty(image, state, kWIPXMAPBlockPresent)) {
		return;
	}

	int compType = getWizCompressionType(image, state);
	bool wiz16bpp = WIZ_16BPP(compType);
	bool native = NATIVE_WIZ_TYPE(compType);

	if (wiz16bpp && !native) {
		uint16 *ptr = (uint16 *)getWizStateDataPrim(image, state);

		int w, h;
		getWizImageDim(image, state, w, h);
		int32 pixelCount = w * h; // Number of pixels to twiddle

		if (pixelCount <= 0) {
			error("Width or height 0 for image %d state %d", image, state);
		}

		uint16 *thisPixel = ptr + 2; // Skip the "WIZD" header

		for (int i = 0; i < pixelCount; i++) {
			thisPixel[i] = SWAP_BYTES_16(thisPixel[i]);
		}

		int newCompType = compType;
		switch (compType) {
		case kWCTNone16Bpp:
			newCompType += kWCTNone16BppBigEndian - kWCTNone16Bpp;
			break;
		case kWCTTRLE16Bpp:
			newCompType += kWCTTRLE16BppBigEndian - kWCTTRLE16Bpp;
			break;
		case kWCTNone16BppBigEndian:
			newCompType -= kWCTNone16BppBigEndian - kWCTNone16Bpp;
			break;
		case kWCTTRLE16BppBigEndian:
			newCompType -= kWCTTRLE16BppBigEndian - kWCTTRLE16Bpp;
			break;
		}

		// Reset the compression type
		setWizCompressionType(image, state, newCompType);
	}
}

void Wiz::processWizImageModifyCmd(const WizImageCommand *params) {
	int state;

	if (params->actionFlags & kWAFState) {
		state = params->state;
	} else {
		state = 0;
	}

	if (params->actionFlags & kWAFRemapList) {
		remapImage(params->image, state, params->remapCount, params->remapList, params->remapTable);
	}
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

void Wiz::processWizImageRenderFloodFillCmd(const WizImageCommand *params) {
	Common::Rect renderRect, clipRect, workClipRect;
	int whichState, w, h, whichImage;
	WizSimpleBitmap renderBitmap;
	WizRawPixel whatColor;
	Common::Point pt;

	// Get the rendering coords or bail if none...
	if (params->actionFlags & kWAFRenderCoords) {
		pt.x = params->renderCoords.left;
		pt.y = params->renderCoords.top;
	} else {
		return;
	}

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

	// What is the rendering color
	if (params->actionFlags & kWAFColor) {
		whatColor = params->colorValue;
	} else {
		whatColor = _vm->VAR(_vm->VAR_COLOR_BLACK);
	}

	// Get the simple bitmap
	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Image %d state %d invalid for rendering.", whichImage, whichState);
	}

	// If we're here we must be able to render into the image (clipped)...
	if (isPointInRect(&clipRect, &pt)) {
		floodSimpleFill(&renderBitmap, pt.x, pt.y, whatColor, &clipRect, &renderRect);

		if (_vm->_game.heversion > 99) {
			_vm->_res->setModified(rtImage, params->image);
		}
	}
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
	Common::SeekableReadStream *inFile;
	int result;

	if (params->actionFlags & kWAFFilename) {
		inFile = _vm->openFileForReading(params->filename);

		if (!inFile) {
			_vm->VAR(_vm->VAR_GAME_LOADED) = DW_LOAD_OPEN_FAILURE;
			_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_LOAD_OPEN_FAILURE;

			debug(0, "Wiz::processWizImageLoadCmd(): Unable to open for read '%s'", params->filename);
			return;
		}

		result = dwTryToLoadWiz(inFile, params);
		_vm->VAR(_vm->VAR_GAME_LOADED) = result;
		_vm->VAR(_vm->VAR_OPERATION_FAILURE) = result;

		if (result == DW_LOAD_SUCCESS) {
			debug(7, "Wiz::processWizImageLoadCmd(): Correctly loaded file '%s'", params->filename);
		} else if (result == DW_LOAD_READ_FAILURE) {
			debug(0, "Wiz::processWizImageLoadCmd(): Got DW_LOAD_READ_FAILURE for file '%s'", params->filename);
		}

		delete inFile;
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
	// TODO
	//if (PU_ProcessWizImageCmd(params)) {
	//	return;
	//}
	debug(5, "processWizImageCmd: actionMode %d for image %d", params->actionMode, params->image);

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
	return ((kWCTNone == id) || (kWCTNone16Bpp == id) || (kWCTNone16BppBigEndian == id));
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
	// TODO: Fix
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

byte *Wiz::getWizStateHeaderPrim(int resNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, resNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), data, state, false) - _vm->_resourceHeaderSize;
}

byte *Wiz::getWizStateDataPrim(int resNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, resNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'D'), data, state, false) - _vm->_resourceHeaderSize;
}

byte *Wiz::getWizStatePaletteDataPrim(int globNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, globNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('R', 'G', 'B', 'S'), data, state, false) - _vm->_resourceHeaderSize;
}

byte *Wiz::getWizStateRemapDataPrim(int globNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, globNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('R', 'M', 'A', 'P'), data, state, false) - _vm->_resourceHeaderSize;
}

const byte *Wiz::getColorMixBlockPtrForWiz(int image) {
	byte *data = _vm->getResourceAddress(rtImage, image);
	assert(data);
	return _vm->findResourceData(MKTAG('X', 'M', 'A', 'P'), data);
}

void Wiz::setWizCompressionType(int image, int state, int newType) {
	byte *data = getWizStateHeaderPrim(image, state);
	assert(data);

	WRITE_LE_UINT32(data + _vm->_resourceHeaderSize, newType);
}

int Wiz::getWizCompressionType(int image, int state) {
	byte *data = (byte *)getWizStateHeaderPrim(image, state);
	assert(data);

	return READ_LE_UINT32(data + _vm->_resourceHeaderSize);
}

bool Wiz::doesRawWizStateHaveTransparency(int globNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, globNum);
	assert(data);

	return _vm->findWrappedBlock(MKTAG('T', 'R', 'N', 'S'), data, state, false) != nullptr;
}

bool Wiz::doesStateContainBlock(int globNum, int state, uint32 blockID) {
	byte *data = _vm->getResourceAddress(rtImage, globNum);
	assert(data);

	return _vm->findWrappedBlock(blockID, data, state, false) != nullptr;
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
		WizRawPixel color = 0xffff;
		drawWizImageEx((byte *)&color, data, 0, 2, kDstMemory, 1, 1, -x, -y, w, h, state, 0, 0, 0, 0, 2, 0, 0);

		return color != 0xffff;
	}

	uint8 *wizd = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), data, state, 0);
	assert(wizd);
	if (x >= 0 && x < w && y >= 0 && y < h) {
		if (flags & kWRFHFlip) {
			x = w - x - 1;
		}
		if (flags & kWRFVFlip) {
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
