/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "scumm/intern.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/wiz_he.h"

namespace Scumm {

Wiz::Wiz() {
	_imagesNum = 0;
	memset(&_images, 0, sizeof(_images));
	memset(&_polygons, 0, sizeof(_polygons));
}

void Wiz::polygonClear() {
	memset(&_polygons, 0, sizeof(_polygons));
}

void Wiz::polygonLoad(const uint8 *polData) {
	int slots = READ_LE_UINT32(polData);
	polData += 4;
	debug(1, "Loading %d polygon slots", slots);

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
	WizPolygon *wp = NULL;
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

	wp->bound.left = 10000;
	wp->bound.top = 10000;
	wp->bound.right = -10000;
	wp->bound.bottom = -10000;

	// compute bounding box
	for (int j = 0; j < wp->numVerts; j++) {
		Common::Rect r(wp->vert[j].x, wp->vert[j].y, wp->vert[j].x + 1, wp->vert[j].y + 1);
		wp->bound.extend(r);
	}
}

void Wiz::polygonErase(int fromId, int toId) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].id >= fromId && _polygons[i].id <= toId)
			memset(&_polygons[i], 0, sizeof(WizPolygon));
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
		curdir = (y <= pol.vert[i].y);

		if (curdir != diry) {
			if (((pol.vert[pi].y - pol.vert[i].y) * (pol.vert[i].x - x) <=
				 (pol.vert[pi].x - pol.vert[i].x) * (pol.vert[i].y - y)) == diry)
				r = !r;
		}

		pi = i;
		diry = curdir;
	}

	return r;
}

void Wiz::copyAuxImage(uint8 *dst1, uint8 *dst2, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch) {
	Common::Rect dstRect(srcx, srcy, srcx + srcw, srcy + srch);
	dstRect.clip(dstw, dsth);
	
	int rw = dstRect.width();
	int rh = dstRect.height();
	if (rh <= 0 || rw <= 0)
		return;
	
	uint8 *dst1Ptr = dst1 + dstRect.left + dstRect.top * dstw;
	uint8 *dst2Ptr = dst2 + dstRect.left + dstRect.top * dstw;
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
	Common::Rect r3;
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
	dstRect = Common::Rect(src_x, src_y, src_x + src_w, src_y + src_h);
	dstRect.clip(r3);
	srcRect = dstRect;
	srcRect.moveTo(0, 0);
	return srcRect.isValidRect() && dstRect.isValidRect();
}

void Wiz::copyWizImage(uint8 *dst, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect) {
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		dst += r2.left + r2.top * dstw;
		decompressWizImage(dst, dstw, r2, src, r1);
	}
}

void Wiz::copyRawWizImage(uint8 *dst, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, int transColor) {
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		if (flags & 0x400) {
			int l = r1.left;
			int r = r1.right;
			r1.left = srcw - r;
			r1.right = srcw - l;
		}
		if (flags & 0x800) {
			int t = r1.top;
			int b = r1.bottom;
			r1.top = srch - b;
			r1.bottom = srch - t;
		}
		byte imagePal[256];
		if (!palPtr) {
			for (int i = 0; i < 256; i++) {
				imagePal[i] = i;
			}
			palPtr = imagePal;
		}
		int h = r1.height();
		int w = r1.width();
		dst += r2.left + r2.top * dstw;
		while (h--) {
			for (int i = 0; i < w; ++i) {
				uint8 col = *src++;
				if (transColor == -1 || transColor != col) {
					dst[i] = palPtr[col];
				}
			}
			dst += dstw;
		}
	}
}

void Wiz::decompressWizImage(uint8 *dst, int dstPitch, const Common::Rect &dstRect, const uint8 *src, const Common::Rect &srcRect, const uint8 *imagePal) {
	const uint8 *dataPtr, *dataPtrNext;
	uint8 *dstPtr, *dstPtrNext;
	uint32 code;
	uint8 databit;
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
		off = READ_LE_UINT16(dataPtr);
		w = srcRect.right - srcRect.left;
		dstPtrNext = dstPitch + dstPtr;
		dataPtrNext = off + 2 + dataPtr;
		dataPtr += 2;
		if (off == 0)
			goto dec_next;

		// Skip over the leftmost 'srcRect->left' pixels.
		// TODO: This code could be merged (at a loss of efficency) with the
		// loop below which does the actual drawing.
		while (xoff > 0) {
			code = *dataPtr++;
			databit = code & 1;
			code >>= 1;
			if (databit) {
				xoff -= code;
				if (xoff < 0) {
					code = -xoff;
					goto dec_sub1;
				}
			} else {
				databit = code & 1;
				code = (code >> 1) + 1;
				if (databit) {
					++dataPtr;
					xoff -= code;
					if (xoff < 0) {
						code = -xoff;
						--dataPtr;
						goto dec_sub2;
					}
				} else {
					dataPtr += code;
					xoff -= code;
					if (xoff < 0) {
						dataPtr += xoff;
						code = -xoff;
						goto dec_sub3;
					}
				}
			}
		}

		while (w > 0) {
			code = *dataPtr++;
			databit = code & 1;
			code >>= 1;
			if (databit) {
dec_sub1:		dstPtr += code;
				w -= code;
			} else {
				databit = code & 1;
				code = (code >> 1) + 1;
				if (databit) {
dec_sub2:			w -= code;
					if (w < 0) {
						code += w;
					}
					uint8 color = *dataPtr++;
					if (imagePal) {
						color = imagePal[color];
					}
					memset(dstPtr, color, code);
					dstPtr += code;
				} else {
dec_sub3:			w -= code;
					if (w < 0) {
						code += w;
					}
					if (imagePal) {
						while (code--) {
							*dstPtr++ = imagePal[*dataPtr++];
						}
					} else {
						memcpy(dstPtr, dataPtr, code);
						dstPtr += code;
						dataPtr += code;
					}
				}
			}
		}
dec_next:
		dataPtr = dataPtrNext;
		dstPtr = dstPtrNext;
	}
}

int Wiz::isWizPixelNonTransparent(const uint8 *data, int x, int y, int w, int h) {
	int ret = 0;
	while (y != 0) {
		data += READ_LE_UINT16(data) + 2;
		--y;
	}
	uint16 off = READ_LE_UINT16(data); data += 2;
	if (off != 0) {
		if (x == 0) {
			ret = (~*data) & 1;			
		} else {
			do {
				uint8 code = *data++;
				if (code & 1) {
					code >>= 1;
					if (code > x) {
						ret = 0;
						break;
					}
					x -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					if (code > x) {
						ret = 1;
						break;
					}
					x -= code;
					++data;
				} else {
					code = (code >> 2) + 1;
					if (code > x) {
						ret = 1;
						break;
					}
					x -= code;
					data += code;
				}				
			} while (x > 0);
		}
	}
	return ret;
}

uint8 Wiz::getWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 color) {
	uint8 c = color;
	if (x >= 0 && x < w && y >= 0 && y < h) {
		while (y != 0) {
			data += READ_LE_UINT16(data) + 2;
			--y;
		}
		uint16 off = READ_LE_UINT16(data); data += 2;
		if (off != 0) {
			if (x == 0) {
				c = (*data & 1) ? color : *data;
			} else {
				do {
					uint8 code = *data++;
					if (code & 1) {
						code >>= 1;
						if (code > x) {
							c = color;
							break;
						}
						x -= code;
					} else if (code & 2) {
						code = (code >> 2) + 1;
						if (code > x) {
							c = *data;
							break;
						}
						x -= code;
						++data;
					} else {
						code = (code >> 2) + 1;
						if (code > x) {
							c = *(data + x);
							break;
						}
						x -= code;
						data += code;
					}				
				} while (x > 0);
			}
		}
	}
	return c;
}

uint8 Wiz::getRawWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 color) {
	uint8 c;
	if (x >= 0 && x < w && y >= 0 && y < h) {
		c = data[y * w + x];
	} else {
		c = color;
	}
	return c;
}

void Wiz::computeWizHistogram(uint32 *histogram, const uint8 *data, const Common::Rect *srcRect) {
	int y = srcRect->top;
	while (y != 0) {
		data += READ_LE_UINT16(data) + 2;
		--y;
	}
	int ih = srcRect->height();
	while (ih--) {
		uint16 off = READ_LE_UINT16(data); data += 2;
		if (off != 0) {
			const uint8 *p = data;
			int x1 = srcRect->left;
			int x2 = srcRect->right;
			uint8 code;
			while (x1 > 0) {
				code = *p++;
				if (code & 1) {
					code >>= 1;
					if (code > x1) {
						code -= x1;
						x2 -= code;
						break;
					}
					x1 -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					if (code > x1) {
						code -= x1;
						goto dec_sub2;
					}
					x1 -= code;
					++p;
				} else {
					code = (code >> 2) + 1;
					if (code > x1) {
						code -= x1;
						p += x1;
						goto dec_sub3;
					}
					x1 -= code;
					p += code;
				}
			}
			while (x2 > 0) {
				code = *p++;
				if (code & 1) {
					code >>= 1;
					x2 -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
dec_sub2:			x2 -= code;
					if (x2 < 0) {
						code += x2;
					}
					histogram[*p++] += code;
				} else {
					code = (code >> 2) + 1;
dec_sub3:			x2 -= code;
					if (x2 < 0) {
						code += x2;
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

void Wiz::computeRawWizHistogram(uint32 *histogram, const uint8 *data, int srcPitch, const Common::Rect *srcRect) {
	data += srcRect->top * srcPitch + srcRect->left;
	int iw = srcRect->width();
	int ih = srcRect->height();
	while (ih--) {
		for (int i = 0; i < iw; ++i) {
			++histogram[data[i]];
		}
		data += srcPitch;
	}
}

struct wizPackCtx {
	uint32 len;
	uint8 saveCode;
	uint8 saveBuf[0x100];
};

static void wizPackType1Helper1(uint8 *&dst, int len, byte newColor, byte prevColor, wizPackCtx *ctx) {
	assert(len > 0);
	if (newColor == prevColor) {
		do {
			int blockLen = MIN(len, 0x7F);
			len -= blockLen;
			if (dst) {
				*dst++ = (blockLen * 2) | 1;
			}
			++ctx->len;
		} while (len > 0);
	} else {
		do {
			int blockLen = MIN(len, 0x40);
			len -= blockLen;
			if (dst) {
				*dst++ = ((blockLen - 1) * 4) | 2;
			}
			++ctx->len;
			if (dst) {
				*dst++ = newColor;
			}
			++ctx->len;
		} while (len > 0);
	}
}

static void wizPackType1Helper2(uint8 *&dst, int len, wizPackCtx *ctx) {
	assert(len > 0);
	const uint8 *src = ctx->saveBuf;
	do {
		int blockLen = MIN(len, 0x40);
		len -= blockLen;
		if (dst) {
			*dst++ = (blockLen - 1) * 4;
		}
		++ctx->len;
		while (blockLen--) {
			if (dst) {
				*dst++ = *src++;
			}
			++ctx->len;
		}
	} while (len > 0);
}

static int wizPackType1(uint8 *dst, const uint8 *src, int srcPitch, const Common::Rect& rCapt, uint8 tColor) {
	debug(1, "wizPackType1(%d, [%d,%d,%d,%d])", tColor, rCapt.left, rCapt.top, rCapt.right, rCapt.bottom);
	wizPackCtx ctx;
	memset(&ctx, 0, sizeof(ctx));
	
	src += rCapt.top * srcPitch + rCapt.left;
	int w = rCapt.width();
	int h = rCapt.height();
	
	uint8 *nextDstPtr, *curDstPtr;
	uint8 curColor, prevColor;
	int saveBufPos;
	
	nextDstPtr = curDstPtr = 0;
	
	int dataSize = 0;
	while (h--) {
		if (dst) {
			curDstPtr = dst;
			nextDstPtr = dst;
			dst += 2;
		}
		dataSize += 2;
		int numBytes = 0;
		
		int i, code;
		for (i = 0; i < w; ++i) {
			if (src[i] != tColor)
				break;
		}
		if (i != w) {
			curDstPtr = dst;
			ctx.len = 0;
			prevColor = ctx.saveBuf[0] = *src;
			const uint8 *curSrcPtr = src + 1;
			saveBufPos = 1;
			code = (tColor - ctx.saveBuf[0] == 0) ? 1 : 0;
			int curw = w;
			while (curw--) {
				ctx.saveBuf[saveBufPos] = curColor = *curSrcPtr++;
				++saveBufPos;
				if (code == 0) {
					if (curColor == tColor) {
						--saveBufPos;
						wizPackType1Helper2(curDstPtr, saveBufPos, &ctx);
						code = saveBufPos = 1;
						ctx.saveBuf[0] = curColor;
						numBytes = 0;
						prevColor = curColor;
						continue;
					}
					if (saveBufPos > 0x80) {
						--saveBufPos;
						wizPackType1Helper2(curDstPtr, saveBufPos, &ctx);
						saveBufPos = 1;
						ctx.saveBuf[0] = curColor;
						numBytes = 0;
						prevColor = curColor;
						continue;
					}
					if (prevColor != curColor) {
						numBytes = saveBufPos - 1;
						prevColor = curColor;
						continue;
					}
					code = 1;
					if (numBytes != 0) {
						if (saveBufPos - numBytes < 3) {
							code = 0;
						} else {
							wizPackType1Helper2(curDstPtr, numBytes, &ctx);
						}
					}
				}
				if (prevColor != curColor || saveBufPos - numBytes > 0x80) {
					saveBufPos -= numBytes;
					--saveBufPos;
					wizPackType1Helper1(curDstPtr, saveBufPos, prevColor, tColor, &ctx);
					saveBufPos = 1;
					numBytes = 0;
					ctx.saveBuf[0] = curColor;
					code = (tColor - ctx.saveBuf[0] == 0) ? 1 : 0;
				}
				prevColor = curColor;
			}
			if (code == 0) {
				wizPackType1Helper2(curDstPtr, saveBufPos, &ctx);
			} else {
				saveBufPos -= numBytes;
				wizPackType1Helper1(curDstPtr, saveBufPos, prevColor, tColor, &ctx);
			}
			dataSize += ctx.len;
			src += srcPitch;
			if (dst) {
				dst += ctx.len;
				*(uint16 *)nextDstPtr = TO_LE_16(ctx.len);
			}
		}
	}
	return dataSize;
}

static int wizPackType0(uint8 *dst, const uint8 *src, int srcPitch, const Common::Rect& rCapt, uint8 tColor) {
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

void ScummEngine_v72he::captureWizImage(int resType, int resNum, const Common::Rect& r, bool frontBuffer, int compType) {
	debug(1, "ScummEngine_v72he::captureWizImage(%d, %d, %d, [%d,%d,%d,%d])", resType, resNum, compType, r.left, r.top, r.right, r.bottom);
	uint8 *src = NULL;
	VirtScreen *pvs = &virtscr[kMainVirtScreen];
	if (frontBuffer) {
		src = pvs->getPixels(0, 0);
	} else {
		src = pvs->getBackPixels(0, 0);
	}
	Common::Rect rCapt(0, 0, pvs->w, pvs->h);
	if (rCapt.intersects(r)) {
		rCapt.clip(r);
		const uint8 *palPtr = _currentPalette;

		int w = rCapt.width();
		int h = rCapt.height();
		int tColor = (VAR_WIZ_TCOLOR != 0xFF) ? VAR(VAR_WIZ_TCOLOR) : 5;

		// compute compressed size
		int dataSize = 0;
		int headerSize = palPtr ? 1080 : 36;
		switch (compType) {
		case 1:
			dataSize = wizPackType1(0, src, pvs->pitch, rCapt, tColor);
			break;
		case 0:
			dataSize = wizPackType0(0, src, pvs->pitch, rCapt, tColor);
			break;
		default:
			warning("unhandled compression type %d", compType);
			break;
		}

		// alignment
		dataSize = (dataSize + 1) & ~1;
		int wizSize = headerSize + dataSize;
		// write header
		uint8 *wizImg = createResource(resType, resNum, dataSize + headerSize);
		*(uint32 *)(wizImg + 0x00) = MKID('AWIZ');
		*(uint32 *)(wizImg + 0x04) = TO_BE_32(wizSize);
		*(uint32 *)(wizImg + 0x08) = MKID('WIZH');
		*(uint32 *)(wizImg + 0x0C) = TO_BE_32(0x14);
		*(uint32 *)(wizImg + 0x10) = TO_LE_32(compType);
		*(uint32 *)(wizImg + 0x14) = TO_LE_32(w);
		*(uint32 *)(wizImg + 0x18) = TO_LE_32(h);
		int curSize = 0x1C;
		if (palPtr) {
			*(uint32 *)(wizImg + 0x1C) = MKID('RGBS');
			*(uint32 *)(wizImg + 0x20) = TO_BE_32(0x308);
			memcpy(wizImg + 0x24, palPtr, 0x300);
			*(uint32 *)(wizImg + 0x324) = MKID('RMAP');
			*(uint32 *)(wizImg + 0x328) = TO_BE_32(0x10C);
			*(uint32 *)(wizImg + 0x32C) = 0;
			curSize = 0x330;
			for (int i = 0; i < 256; ++i) {
				wizImg[curSize] = i;
				++curSize;
			}
		}
		*(uint32 *)(wizImg + curSize + 0x0) = MKID('WIZD');
		*(uint32 *)(wizImg + curSize + 0x4) = TO_BE_32(dataSize + 8);
		curSize += 8;

		// write compressed data
		switch (compType) {
		case 1:
			wizPackType1(wizImg + headerSize, src, pvs->pitch, rCapt, tColor);
			break;
		case 0:
			wizPackType0(wizImg + headerSize, src, pvs->pitch, rCapt, tColor);
			break;
		default:
			break;
		}
	}
}

void ScummEngine_v72he::displayWizImage(const WizImage *pwi) {
	if (_fullRedraw) {
		assert(_wiz._imagesNum < ARRAYSIZE(_wiz._images));
		memcpy(&_wiz._images[_wiz._imagesNum], pwi, sizeof(WizImage));
		++_wiz._imagesNum;
	} else if (pwi->flags & 0x40) {
		drawWizPolygon(pwi->resNum, pwi->state, pwi->x1, pwi->flags);
	} else {
		drawWizImage(rtImage, pwi);
	}
}

void ScummEngine_v72he::getWizImageDim(int resnum, int state, int32 &w, int32 &h) {
	const uint8 *dataPtr = getResourceAddress(rtImage, resnum);
	if (dataPtr) {
		const uint8 *wizh = findWrappedBlock(MKID('WIZH'), dataPtr, state, 0);
		w = READ_LE_UINT32(wizh + 0x4);
		h = READ_LE_UINT32(wizh + 0x8);
	} else {
		w = 0;
		h = 0;
	}
}

uint8 *ScummEngine_v72he::drawWizImage(int restype, const WizImage *pwi) {
	debug(1, "drawWizImage(%d, %d, %d, %d, 0x%X)", restype, pwi->resNum, pwi->x1, pwi->y1, pwi->flags);
	uint8 *dst = NULL;
	const uint8 *dataPtr = getResourceAddress(restype, pwi->resNum);
	if (dataPtr) {
		const uint8 *rmap = NULL;
		const uint8 *xmap = findWrappedBlock(MKID('XMAP'), dataPtr, pwi->state, 0);
		
		const uint8 *wizh = findWrappedBlock(MKID('WIZH'), dataPtr, pwi->state, 0);
		assert(wizh);
		uint32 comp   = READ_LE_UINT32(wizh + 0x0);
		uint32 width  = READ_LE_UINT32(wizh + 0x4);
		uint32 height = READ_LE_UINT32(wizh + 0x8);
		assert(comp == 0 || comp == 1 || comp == 2 || comp == 3 || comp == 10 || comp == 11);
		
		const uint8 *wizd = findWrappedBlock(MKID('WIZD'), dataPtr, pwi->state, 0);
		assert(wizd);
		if (pwi->flags & 1) {
			const uint8 *pal = findWrappedBlock(MKID('RGBS'), dataPtr, pwi->state, 0);
			assert(pal);
			setPaletteFromPtr(pal, 256);
		}
		if (pwi->flags & 2) {
			rmap = findWrappedBlock(MKID('RMAP'), dataPtr, pwi->state, 0);
			assert(rmap);
			const uint8 *rgbs = findWrappedBlock(MKID('RGBS'), dataPtr, pwi->state, 0);
			assert(rgbs);
			warning("drawWizImage() unhandled flag 0x2");
			// XXX modify 'RMAP' buffer
		}
		if (pwi->flags & 4) {
			warning("WizImage printing is unimplemented");
			return NULL;
		}
		uint32 cw, ch;
		if (pwi->flags & 0x20) {
			dst = (uint8 *)malloc(width * height);
			int color = 255; // FIXME: should be (VAR_WIZ_TCOLOR != 0xFF) ? VAR(VAR_WIZ_TCOLOR) : 5;
			memset(dst, color, width * height);
			cw = width;
			ch = height;
		} else {
			VirtScreen *pvs = &virtscr[kMainVirtScreen];
			if (pwi->flags & 0x10) {
				dst = pvs->getPixels(0, pvs->topline);
			} else {
				dst = pvs->getBackPixels(0, pvs->topline);
			}
			cw = pvs->w;
			ch = pvs->h;
		}
		Common::Rect rScreen(cw, ch);
		// XXX handle 'XMAP' / 'RMAP' data
		if (comp == 1) {
			if (pwi->flags & 0x80) {
				warning("drawWizImage() unhandled flag 0x80");
			} else if (pwi->flags & 0x100) {
				warning("drawWizImage() unhandled flag 0x100");
			} else {
				_wiz.copyWizImage(dst, wizd, cw, ch, pwi->x1, pwi->y1, width, height, &rScreen);
			}
		} else if (comp == 0 || comp == 2 || comp == 3) {
			const uint8 *trns = findWrappedBlock(MKID('TRNS'), dataPtr, pwi->state, 0);
			int color = (trns == NULL) ? VAR(VAR_WIZ_TCOLOR) : -1;
			const uint8 *pal = xmap;
			if (pwi->flags & 2) {
				pal = rmap + 4;
			}
			_wiz.copyRawWizImage(dst, wizd, cw, ch, pwi->x1, pwi->y1, width, height, &rScreen, pwi->flags, pal, color);
		} else {
			warning("unhandled wiz compression type %d", comp);
		}

		if (!(pwi->flags & 0x20)) {
			Common::Rect rImage(pwi->x1, pwi->y1, pwi->x1 + width, pwi->y1 + height);
			if (rImage.intersects(rScreen)) {
				rImage.clip(rScreen);
				if (!(pwi->flags & 8) && pwi->flags & 0x18) {
					++rImage.bottom;
					markRectAsDirty(kMainVirtScreen, rImage);
				} else {
					gdi.copyVirtScreenBuffers(rImage);
				}
			}
		}
	}
	return dst;
}

struct PolygonDrawData {
	struct InterArea {
		bool valid;
		int32 xmin;
		int32 xmax;
		int32 x1;
		int32 y1;
		int32 x2;
		int32 y2;
	};
	Common::Point pto;
	InterArea *ia;
	int areasNum;
	
	PolygonDrawData(int n) {
		areasNum = n;
		ia = new InterArea[areasNum];
		memset(ia, 0, sizeof(InterArea) * areasNum);
	}
	
	~PolygonDrawData() {
		delete[] ia;
	}
	
	void calcIntersection(const Common::Point *p1, const Common::Point *p2, const Common::Point *p3, const Common::Point *p4) {
		int32 x1_acc = p1->x << 0x10;
		int32 x3_acc = p3->x << 0x10;
		int32 y3_acc = p3->y << 0x10;
  		uint16 dy = ABS(p2->y - p1->y) + 1;
  		int32 x1_step = ((p2->x - p1->x) << 0x10) / dy;
  		int32 x3_step = ((p4->x - p3->x) << 0x10) / dy;
  		int32 y3_step = ((p4->y - p3->y) << 0x10) / dy;

  		int iaidx = p1->y - pto.y;
  		while (dy--) {
  			assert(iaidx >= 0 && iaidx < areasNum);
  			InterArea *pia = &ia[iaidx];
  			int32 tx1 = x1_acc >> 0x10;
  			int32 tx3 = x3_acc >> 0x10;
  			int32 ty3 = y3_acc >> 0x10;
  			
  			if (!pia->valid || pia->xmin > tx1) {
  				pia->xmin = tx1;
  				pia->x1 = tx3;
  				pia->y1 = ty3;
			}
  			if (!pia->valid || pia->xmax < tx1) {
  				pia->xmax = tx1;
  				pia->x2 = tx3;
  				pia->y2 = ty3;
			}
  			pia->valid = true;

  			x1_acc += x1_step;
  			x3_acc += x3_step;
  			y3_acc += y3_step;
  			
  			if (p2->y <= p1->y) {
  				--iaidx;
  			} else {
  				++iaidx;
  			}  			
  		}
	}
};

void ScummEngine_v72he::drawWizPolygon(int resnum, int state, int id, int flags) {
	int i;
	WizPolygon *wp = NULL;
	for (i = 0; i < ARRAYSIZE(_wiz._polygons); ++i) {
		if (_wiz._polygons[i].id == id) {
			wp = &_wiz._polygons[i];
			break;
		}
	}
	if (!wp) {
		error("Polygon %d is not defined", id);
	}
	if (wp->numVerts != 5) {
		error("Invalid point count %d for Polygon %d", wp->numVerts, id);
	}
	WizImage wi;
	wi.resNum = resnum;
	wi.state = state;
	wi.x1 = wi.y1 = 0;
	wi.flags = 0x20;
	uint8 *srcWizBuf = drawWizImage(rtImage, &wi);
	if (srcWizBuf) {
		uint8 *dst;
		VirtScreen *pvs = &virtscr[kMainVirtScreen];
		if (flags & 0x10) {
			dst = pvs->getPixels(0, 0);
		} else {
			dst = pvs->getBackPixels(0, 0);
		}
		if (wp->bound.left < 0 || wp->bound.top < 0 || wp->bound.right >= pvs->w || wp->bound.bottom >= pvs->h) {
			error("Invalid coords polygon %d", wp->id);
		}

		int32 wizW, wizH;
		getWizImageDim(resnum, state, wizW, wizH);
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
  		xmin_p = xmax_p = wp->vert[0].x;
  		ymin_p = ymax_p = wp->vert[0].y;
  		for (i = 1; i < 4; ++i) {
  			xmin_p = MIN(wp->vert[i].x, xmin_p);
  			xmax_p = MAX(wp->vert[i].x, xmax_p);
  			ymin_p = MIN(wp->vert[i].y, ymin_p);
  			ymax_p = MAX(wp->vert[i].y, ymax_p);
  		}
  		
  		int16 xmin_b, xmax_b, ymin_b, ymax_b;
  		xmin_b = 0;
  		xmax_b = wizW - 1;
  		ymin_b = 0;
  		ymax_b = wizH - 1;

		PolygonDrawData pdd(ymax_p - ymin_p + 1);
		pdd.pto.x = xmin_p;
		pdd.pto.y = ymin_p;
		
		for (i = 0; i < 3; ++i) {
			pdd.calcIntersection(&wp->vert[i], &wp->vert[i + 1], &bbox[i], &bbox[i + 1]);
		}
		pdd.calcIntersection(&wp->vert[3], &wp->vert[0], &bbox[3], &bbox[0]);
				
		uint yoff = pdd.pto.y * pvs->w;
		for (i = 0; i < pdd.areasNum; ++i) {
			PolygonDrawData::InterArea *pia = &pdd.ia[i];
			uint16 dx = pia->xmax - pia->xmin + 1;
			uint8 *dstPtr = dst + pia->xmin + yoff;
			int32 x_acc = pia->x1 << 0x10;
			int32 y_acc = pia->y1 << 0x10;
			int32 x_step = ((pia->x2 - pia->x1) << 0x10) / dx;
			int32 y_step = ((pia->y2 - pia->y1) << 0x10) / dx;
			while (dx--) {
				uint srcWizOff = (y_acc >> 0x10) * wizW + (x_acc >> 0x10);
				assert(srcWizOff < (uint32)(wizW * wizH));
				x_acc += x_step;
				y_acc += y_step;
				*dstPtr++ = srcWizBuf[srcWizOff];
			}
			yoff += pvs->pitch;
		}

		if (flags & 0x10) {
			markRectAsDirty(kMainVirtScreen, wp->bound);
		} else {
			gdi.copyVirtScreenBuffers(wp->bound);
		}

		free(srcWizBuf);
	}
}

void ScummEngine_v72he::flushWizBuffer() {
	for (int i = 0; i < _wiz._imagesNum; ++i) {
		WizImage *pwi = &_wiz._images[i];
		if (pwi->flags & 0x40) {
			drawWizPolygon(pwi->resNum, pwi->state, pwi->x1, pwi->flags);
		} else {
			drawWizImage(rtImage, pwi);
		}
	}
	_wiz._imagesNum = 0;
}

void ScummEngine_v80he::loadImgSpot(int resId, int state, int16 &x, int16 &y) {
	const uint8 *dataPtr = getResourceAddress(rtImage, resId);
	if (!dataPtr) {
		warning("loadImgSpot: unknown Image %d", resId);
		x = y = 0;
		return;
	}

	const uint8 *spotPtr = findWrappedBlock(MKID('SPOT'), dataPtr, state, 0);
	if (spotPtr) {
		x = (int16)READ_LE_UINT32(spotPtr + 0);
		y = (int16)READ_LE_UINT32(spotPtr + 4);
	} else {
		x = 0;
		y = 0;
	}
}

void ScummEngine_v80he::loadWizCursor(int resId, int resType, bool state) {
	int16 x, y;
	loadImgSpot(resId, 0, x, y);
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

	WizImage wi;
	wi.resNum = resId;
	wi.x1 = wi.y1 = 0;
	wi.state = 0;
	wi.flags = 0x20;	
	uint8 *cursor = drawWizImage(rtImage, &wi);
	int32 cw, ch;	
	getWizImageDim(resId, 0, cw, ch);
	setCursorFromBuffer(cursor, cw, ch, cw);
	setCursorHotspot(x, y);
	free(cursor);
}

void ScummEngine_v90he::drawWizComplexPolygon(int resnum, int state, int po_x, int po_y, int arg14, int angle, int zoom, const Common::Rect *r) {
	Common::Point pts[4];
	int32 w, h;
	getWizImageDim(resnum, state, w, h);

	pts[1].x = pts[2].x = w / 2 - 1;
	pts[0].x = pts[0].y = pts[1].y = pts[3].x = -w / 2;
	pts[2].y = pts[3].y = h / 2 - 1;

	// transform points
	if (zoom != 256) {
		for (int i = 0; i < 4; ++i) {
			pts[i].x = pts[i].x * zoom / 256;
			pts[i].y = pts[i].y * zoom / 256;
		}
	}
	if (angle != 0) {
		double alpha = angle * PI / 180.;
		double cos_alpha = cos(alpha);
		double sin_alpha = sin(alpha);
		for (int i = 0; i < 4; ++i) {
			int16 x = pts[i].x;
			int16 y = pts[i].y;
			pts[i].x = (int16)(x * cos_alpha - y * sin_alpha);
			pts[i].y = (int16)(y * cos_alpha + x * sin_alpha);
		}
	}
	for (int i = 0; i < 4; ++i) {
		pts[i].x += po_x;
		pts[i].y += po_y;
	}
	// XXX drawWizPolygonPoints(resnum, state, pts, r, VAR(117));
	warning("ScummEngine_v90he::drawWizComplexPolygon() partially implemented");
}

void ScummEngine_v90he::displayWizComplexImage(const WizParameters *params) {
	// XXX merge with ScummEngine_v72he::displayWizImage
	int zoom = 256;
	if (params->processFlags & 0x8) {
		zoom = params->zoom;
	}
	int rotationAngle = 0;
	if (params->processFlags & 0x10) {
		rotationAngle = params->angle;
	}
	int state = 0;
	if (params->processFlags & 0x400) {
		state = params->img.state;
	}
	int flags = 0;
	if (params->processFlags & 0x20) {
		flags = params->img.flags;
	}
	int po_x = 0;
	int po_y = 0;
	if (params->processFlags & 0x1) {
		po_x = params->img.x1;
		po_y = params->img.y1;
	}
	int unk = 0;
	if (params->processFlags & 0x4) {
		unk = params->unk_15C;
	}
	const Common::Rect *r = NULL;
	if (params->processFlags & 0x200) {
		r = &params->box;
	}

	if (_fullRedraw) {
		assert(_wiz._imagesNum < ARRAYSIZE(_wiz._images));
		WizImage *pwi = &_wiz._images[_wiz._imagesNum];
		pwi->resNum = params->img.resNum;
		pwi->x1 = po_x;
		pwi->y1 = po_y;
		pwi->state = state;
		pwi->flags = flags;
		pwi->unk = unk;
		++_wiz._imagesNum;
	} else if (params->processFlags & 0x18) {
		drawWizComplexPolygon(params->img.resNum, state, po_x, po_y, unk, rotationAngle, zoom, r);
	} else if (flags & 0x40) {
		drawWizPolygon(params->img.resNum, state, po_x, flags); // XXX , VAR(117));
	} else {
		if ((flags & 0x200) || (flags & 0x24)) {
			warning("ScummEngine_v90he::displayWizComplexImage() unhandled flags = 0x%X", flags);
		}
		// XXX flags 0x200, 0x24
		WizImage wi;
		wi.resNum = params->img.resNum;
		wi.x1 = po_x;
		wi.y1 = po_y;
		wi.state = state;
		wi.flags = flags;
		wi.unk = unk;
		drawWizImage(rtImage, &wi);
	}
}

void ScummEngine_v90he::processWizImage(const WizParameters *params) {
	debug(1, "ScummEngine_v90he::processWizImage()");
	switch (params->processMode) {
	case 1:
		displayWizComplexImage(params);
		break;
	case 2:
 		captureWizImage(rtImage, params->img.resNum, params->box, (params->img.flags & kWIFBlitToFrontVideoBuffer) == kWIFBlitToFrontVideoBuffer, params->compType);
		break;
	case 3:
		if (params->processFlags & 0x800) {
			File f;
			if (!f.open((const char *)params->filename, File::kFileReadMode)) {
				warning("Unable to open for read '%s'", params->filename);
			} else {
				uint32 id = f.readUint32BE();
				if (id != MKID('AWIZ') && id != MKID('MULT')) {
					VAR(VAR_GAME_LOADED) = -1;
				} else {
					uint32 size = f.readUint32BE();
					f.seek(0, SEEK_SET);
					byte *p = createResource(rtImage, params->img.resNum, size);
					if (f.read(p, size) != size) {
						nukeResource(rtImage, params->img.resNum);
						warning("i/o error when reading '%s'", params->filename);
						VAR(VAR_GAME_LOADED) = -2;
					} else {
						VAR(VAR_GAME_LOADED) = 0;
					}
				}
				f.close();
			}
		}
		break;
	case 4:
		if (params->processFlags & 0x800) {
			if (params->unk_14C != 0) {
				VAR(119) = -1;
			} else {
				File f;
				if (!f.open((const char *)params->filename, File::kFileWriteMode)) {
					warning("Unable to open for write '%s'", params->filename);
					VAR(119) = -3;
				} else {
					byte *p = getResourceAddress(rtImage, params->img.resNum);
					uint32 size = READ_BE_UINT32(p + 4);
					if (f.write(p, size) != size) {
						warning("i/o error when writing '%s'", params->filename);
						VAR(119) = -2;
					} else {
						VAR(119) = 0;
					}
					f.close();
				}
			}
		}
		break;
	case 6:
	// HE 99+
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		warning("unhandled processWizImage mode %d", params->processMode);
		break;
	default:
		warning("invalid processWizImage mode %d", params->processMode);
	}
}

int ScummEngine_v90he::getWizImageStates(int resnum) {
	const uint8 *dataPtr = getResourceAddress(rtImage, resnum);
	assert(dataPtr);
	if (READ_UINT32(dataPtr) == MKID('MULT')) {
		const byte *offs, *wrap;

		wrap = findResource(MKID('WRAP'), dataPtr);
		if (wrap == NULL)
			return 1;

		offs = findResourceData(MKID('OFFS'), wrap);
		if (offs == NULL)
			return 1;

		return getResourceDataSize(offs) / 4;
	} else {
		return 1;
	}
}

int ScummEngine_v90he::isWizPixelNonTransparent(int restype, int resnum, int state, int x, int y, int flags) {
	int ret = 0;
	const uint8 *data = getResourceAddress(restype, resnum);
	assert(data);
	const uint8 *wizh = findWrappedBlock(MKID('WIZH'), data, state, 0);
	assert(wizh);
	uint32 c = READ_LE_UINT32(wizh + 0x0);
	int w = READ_LE_UINT32(wizh + 0x4);
	int h = READ_LE_UINT32(wizh + 0x8);
	const uint8 *wizd = findWrappedBlock(MKID('WIZD'), data, state, 0);
	assert(wizd);
	if (x >= 0 && x < w && y >= 0 && y < h) {
		if (flags & 0x400) {
			x = w - x - 1;
		}
		if (flags & 0x800) {
			y = h - y - 1;
		}
		if (c == 1) {
			ret = _wiz.isWizPixelNonTransparent(wizd, x, y, w, h);
		} else if (c == 0 || c == 2 || c == 3) {
			ret = _wiz.getRawWizPixelColor(wizd, x, y, w, h, VAR(VAR_WIZ_TCOLOR)) != VAR(VAR_WIZ_TCOLOR) ? 1 : 0;
		}
	}
	return ret;
}

uint8 ScummEngine_v90he::getWizPixelColor(int restype, int resnum, int state, int x, int y, int flags) {
	uint8 color;
	const uint8 *data = getResourceAddress(restype, resnum);
	assert(data);
	const uint8 *wizh = findWrappedBlock(MKID('WIZH'), data, state, 0);
	assert(wizh);
	uint32 c = READ_LE_UINT32(wizh + 0x0);
	uint32 w = READ_LE_UINT32(wizh + 0x4);
	uint32 h = READ_LE_UINT32(wizh + 0x8);
	const uint8 *wizd = findWrappedBlock(MKID('WIZD'), data, state, 0);
	assert(wizd);		
	if (c == 1) {
		color = _wiz.getWizPixelColor(wizd, x, y, w, h, VAR(VAR_WIZ_TCOLOR));
	} else if (c == 0 || c == 2 || c == 3) {
		color = _wiz.getRawWizPixelColor(wizd, x, y, w, h, VAR(VAR_WIZ_TCOLOR));
	} else {
		color = VAR(VAR_WIZ_TCOLOR);
	}
	return color;
}

int ScummEngine_v90he::computeWizHistogram(int resnum, int state, int x, int y, int w, int h) {
	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, 255);
	if (readVar(0) != 0) {
		const uint8 *data = getResourceAddress(rtImage, resnum);
		assert(data);
		const uint8 *wizh = findWrappedBlock(MKID('WIZH'), data, state, 0);
		assert(wizh);
		uint32 ic = READ_LE_UINT32(wizh + 0x0);
		uint32 iw = READ_LE_UINT32(wizh + 0x4);
		uint32 ih = READ_LE_UINT32(wizh + 0x8);
		const uint8 *wizd = findWrappedBlock(MKID('WIZD'), data, state, 0);
		assert(wizd);
		Common::Rect rWiz(iw, ih);
		Common::Rect rCap(x, y, w + 1, h + 1);
		if (rCap.intersects(rWiz)) {
			rCap.clip(rWiz);
			uint32 histogram[0x100];
			memset(histogram, 0, sizeof(histogram));
			if (ic == 1) {
				_wiz.computeWizHistogram(histogram, wizd, &rCap);
			} else if (ic == 0 || ic == 2 || ic == 3) {
				_wiz.computeRawWizHistogram(histogram, wizd, w, &rCap);
			} else {
				warning("Unable to return histogram for type %d", ic);
			}
			for (int i = 0; i < 0x100; ++i) {
				writeArray(0, 0, i, histogram[i]);
			}
		}
	}
	return readVar(0);
}

} // End of namespace Scumm
