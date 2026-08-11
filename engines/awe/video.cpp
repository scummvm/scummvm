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

#include "graphics/surface.h"
#include "awe/video.h"
#include "awe/bitmap.h"
#include "awe/gfx.h"
#include "awe/resource.h"
#include "awe/resource_3do.h"
#include "awe/system_stub.h"

namespace Awe {

Video::Video(Resource *res) : _res(res) {
}

void Video::init() {
	_nextPal = _currentPal = 0xFF;
	_buffers[2] = getPagePtr(1);
	_buffers[1] = getPagePtr(2);
	setWorkPagePtr(0xFE);
	_pData.byteSwap = (_res->getDataType() == DT_3DO);
}

void Video::setDefaultFont() {
	_graphics->setFont(nullptr, 0, 0);
}

void Video::setFont(const uint8 *font) {
	int w, h;
	uint8 *buf = decode_bitmap(font, true, -1, &w, &h);
	if (buf) {
		_graphics->setFont(buf, w, h);
		free(buf);
	}
}

void Video::setHeads(const uint8 *src) {
	int w, h;
	uint8 *buf = decode_bitmap(src, true, 0xF06080, &w, &h);
	if (buf) {
		_graphics->setSpriteAtlas(buf, w, h, 2, 2);
		free(buf);
		_hasHeadSprites = true;
	}
}

void Video::setDataBuffer(uint8 *dataBuf, uint16 offset) {
	_dataBuf = dataBuf;
	_pData.pc = dataBuf + offset;
}

void Video::drawShape(uint8 color, uint16 zoom, const Point *pt) {
	uint8 i = _pData.fetchByte();
	if (i >= 0xC0) {
		if (color & 0x80) {
			color = i & 0x3F;
		}
		fillPolygon(color, zoom, pt);
	} else {
		i &= 0x3F;
		if (i == 1) {
			warning("Video::drawShape() ec=0x%X (i != 2)", 0xF80);
		} else if (i == 2) {
			drawShapeParts(zoom, pt);
		} else {
			warning("Video::drawShape() ec=0x%X (i != 2)", 0xFBB);
		}
	}
}

void Video::drawShapePart3DO(int color, int part, const Point *pt) {
	assert(part < (int)ARRAYSIZE(VERTICES_3DO));
	const uint8 *vertices = VERTICES_3DO[part];
	const int w = *vertices++;
	const int h = *vertices++;
	const int x = pt->x - w / 2;
	const int y = pt->y - h / 2;
	QuadStrip qs;
	qs.numVertices = 2 * h;
	assert(qs.numVertices < QuadStrip::MAX_VERTICES);
	for (int i = 0; i < h; ++i) {
		qs.vertices[i].x = x + *vertices++;
		qs.vertices[i].y = y + i;
		qs.vertices[2 * h - 1 - i].x = x + *vertices++;
		qs.vertices[2 * h - 1 - i].y = y + i;
	}
	_graphics->drawQuadStrip(_buffers[0], color, &qs);
}

void Video::drawShape3DO(int color, int zoom, const Point *pt) {
	const int code = _pData.fetchByte();
	debugC(kDebugVideo, "Video::drawShape3DO() code=0x%x pt=%d,%d", code, pt->x, pt->y);
	if (color == 0xFF) {
		color = code & 31;
	}
	switch (code & 0xE0) {
	case 0x00:
	{
		const int x0 = pt->x - _pData.fetchByte() * zoom / 64;
		const int y0 = pt->y - _pData.fetchByte() * zoom / 64;
		int count = _pData.fetchByte() + 1;
		do {
			uint16 offset = _pData.fetchWord();
			Point po;
			po.x = x0 + _pData.fetchByte() * zoom / 64;
			po.y = y0 + _pData.fetchByte() * zoom / 64;
			color = 0xFF;
			if (offset & 0x8000) {
				color = _pData.fetchByte();
				const int num = _pData.fetchByte();
				if (color & 0x80) {
					drawShapePart3DO(color & 0xF, num, &po);
					continue;
				}
			}
			offset <<= 1;
			uint8 *bak = _pData.pc;
			_pData.pc = _dataBuf + offset;
			drawShape3DO(color, zoom, &po);
			_pData.pc = bak;
		} while (--count != 0);
	}
	break;
	case 0x20:
	{ // rect
		const int w = _pData.fetchByte() * zoom / 64;
		const int h = _pData.fetchByte() * zoom / 64;
		const int x1 = pt->x - w / 2;
		const int y1 = pt->y - h / 2;
		const int x2 = x1 + w;
		const int y2 = y1 + h;
		if (x1 > 319 || x2 < 0 || y1 > 199 || y2 < 0) {
			break;
		}
		QuadStrip qs;
		qs.numVertices = 4;
		qs.vertices[0].x = x1;
		qs.vertices[0].y = y1;
		qs.vertices[1].x = x1;
		qs.vertices[1].y = y2;
		qs.vertices[2].x = x2;
		qs.vertices[2].y = y2;
		qs.vertices[3].x = x2;
		qs.vertices[3].y = y1;
		_graphics->drawQuadStrip(_buffers[0], color, &qs);
	}
	break;
	case 0x40:
	{ // pixel
		if (pt->x > 319 || pt->x < 0 || pt->y > 199 || pt->y < 0) {
			break;
		}
		_graphics->drawPoint(_buffers[0], color, pt);
	}
	break;
	case 0xC0:
	{ // polygon
		const int w = _pData.fetchByte() * zoom / 64;
		const int h = _pData.fetchByte() * zoom / 64;
		const int count = _pData.fetchByte();
		QuadStrip qs;
		qs.numVertices = count * 2;
		assert(qs.numVertices < QuadStrip::MAX_VERTICES);
		const int x0 = pt->x - w / 2;
		const int y0 = pt->y - h / 2;
		if (x0 > 319 || pt->x + w / 2 < 0 || y0 > 199 || pt->y + h / 2 < 0) {
			break;
		}
		for (int i = 0, j = count * 2 - 1; i < count; ++i, --j) {
			const int x1 = _pData.fetchByte() * zoom / 64;
			const int x2 = _pData.fetchByte() * zoom / 64;
			const int y = _pData.fetchByte() * zoom / 64;
			qs.vertices[i].x = x0 + x2;
			qs.vertices[(i + 1) % count].y = y0 + y;
			qs.vertices[j].x = x0 + x1;
			qs.vertices[count * 2 - 1 - (i + 1) % count].y = y0 + y;
		}
		_graphics->drawQuadStrip(_buffers[0], color, &qs);
	}
	break;
	default:
		warning("Video::drawShape3DO() unhandled code 0x%X", code);
		break;
	}
}

void Video::fillPolygon(uint16 color, uint16 zoom, const Point *pt) {
	const uint8 *p = _pData.pc;

	const uint16 bbw = (*p++) * zoom / 64;
	const uint16 bbh = (*p++) * zoom / 64;

	const int16 x1 = pt->x - bbw / 2;
	const int16 x2 = pt->x + bbw / 2;
	const int16 y1 = pt->y - bbh / 2;
	const int16 y2 = pt->y + bbh / 2;

	if (x1 > 319 || x2 < 0 || y1 > 199 || y2 < 0)
		return;

	QuadStrip qs;
	qs.numVertices = *p++;
	if ((qs.numVertices & 1) != 0) {
		warning("Unexpected number of vertices %d", qs.numVertices);
		return;
	}
	assert(qs.numVertices < QuadStrip::MAX_VERTICES);

	for (int i = 0; i < qs.numVertices; ++i) {
		Point *v = &qs.vertices[i];
		v->x = x1 + (*p++) * zoom / 64;
		v->y = y1 + (*p++) * zoom / 64;
	}

	if (qs.numVertices == 4 && bbw == 0 && bbh <= 1) {
		_graphics->drawPoint(_buffers[0], color, pt);
	} else {
		_graphics->drawQuadStrip(_buffers[0], color, &qs);
	}
}

void Video::drawShapeParts(uint16 zoom, const Point *pgc) {
	Point pt;
	pt.x = pgc->x - _pData.fetchByte() * zoom / 64;
	pt.y = pgc->y - _pData.fetchByte() * zoom / 64;
	int16 n = _pData.fetchByte();
	debugC(kDebugVideo, "Video::drawShapeParts n=%d", n);
	for (; n >= 0; --n) {
		uint16 offset = _pData.fetchWord();
		Point po(pt);
		po.x += _pData.fetchByte() * zoom / 64;
		po.y += _pData.fetchByte() * zoom / 64;
		uint16 color = 0xFF;
		if (offset & 0x8000) {
			color = _pData.fetchByte();
			const int num = _pData.fetchByte();
			if (Gfx::_is1991) {
				if (!_hasHeadSprites && (color & 0x80) != 0) {
					_graphics->drawSprite(_buffers[0], num, &po, color & 0x7F);
					continue;
				}
			} else if (_hasHeadSprites && _displayHead) {
				switch (num) {
				case 0x4A:
				{ // facing right
					Point pos(po.x - 4, po.y - 7);
					_graphics->drawSprite(_buffers[0], 0, &pos, color);
				}
				case 0x4D:
					return;
				case 0x4F:
				{ // facing left
					Point pos(po.x - 4, po.y - 7);
					_graphics->drawSprite(_buffers[0], 1, &pos, color);
				}
				case 0x50:
					return;

				default:
					break;
				}
			}
			color &= 0x7F;
		}
		offset <<= 1;
		uint8 *bak = _pData.pc;
		_pData.pc = _dataBuf + offset;
		drawShape(color, zoom, &po);
		_pData.pc = bak;
	}
}

static const int NTH_EDITION_STRINGS_COUNT = 157;

static const char *findString(const StrEntry *stringsTable, int id) {
	for (const StrEntry *se = stringsTable; se->id != 0xFFFF; ++se) {
		if (se->id == id) {
			return se->str;
		}
	}
	return nullptr;
}

void Video::drawString(uint8 color, uint16 x, uint16 y, uint16 strId) {
	bool escapedChars = false;
	const char *str = nullptr;
	if (_res->getDataType() == DT_15TH_EDITION || _res->getDataType() == DT_20TH_EDITION) {
		for (int i = 0; i < NTH_EDITION_STRINGS_COUNT; ++i) {
			if (Video::STRINGS_ID_15TH[i] == strId) {
				str = _res->getString(i);
				if (str) {
					escapedChars = true;
				} else {
					str = Video::STRINGS_TABLE_15TH[i];
				}
				break;
			}
		}
	} else if (_res->getDataType() == DT_WIN31) {
		str = _res->getString(strId);
	} else if (_res->getDataType() == DT_3DO) {
		str = findString(STRINGS_TABLE_3DO, strId);
	} else if (_res->getDataType() == DT_ATARI_DEMO && strId == 0x194) {
		str = _str0x194AtariDemo;
	} else {
		if (_res->getDataType() == DT_AMIGA || _res->getDataType() == DT_ATARI) {
			str = findString(STRINGS_TABLE_AMIGA_CODES, strId);
		}
		if (!str) {
			str = findString(_stringsTable, strId);
		}
		if (!str && _res->getDataType() == DT_DOS) {
			str = findString(STRINGS_TABLE_DEMO, strId);
		}
	}
	if (!str) {
		warning("Unknown string id %d", strId);
		return;
	}
	debugC(kDebugVideo, "drawString(%d, %d, %d, '%s')", color, x, y, str);
	const uint16 xx = x;
	const int len = strlen(str);
	for (int i = 0; i < len; ++i) {
		if (str[i] == '\n' || str[i] == '\r') {
			y += 8;
			x = xx;
		} else if (str[i] == '\\' && escapedChars) {
			++i;
			if (i < len) {
				switch (str[i]) {
				case 'n':
					y += 8;
					x = xx;
					break;

				default:
					break;
				}
			}
		} else {
			Point pt(x * 8, y);
			_graphics->drawStringChar(_buffers[0], color, str[i], &pt);
			++x;
		}
	}
}

uint8 Video::getPagePtr(uint8 page) {
	uint8 p;
	if (page <= 3) {
		p = page;
	} else {
		switch (page) {
		case 0xFF:
			p = _buffers[2];
			break;
		case 0xFE:
			p = _buffers[1];
			break;
		default:
			p = 0; // XXX check
			warning("Video::getPagePtr() p != [0,1,2,3,0xFF,0xFE] == 0x%X", page);
			break;
		}
	}
	return p;
}

void Video::setWorkPagePtr(uint8 page) {
	debugC(kDebugVideo, "Video::setWorkPagePtr(%d)", page);
	_buffers[0] = getPagePtr(page);
}

void Video::fillPage(uint8 page, uint8 color) {
	debugC(kDebugVideo, "Video::fillPage(%d, %d)", page, color);
	_graphics->clearBuffer(getPagePtr(page), color);
}

void Video::copyPage(uint8 src, uint8 dst, int16 vscroll) {
	debugC(kDebugVideo, "Video::copyPage(%d, %d)", src, dst);
	if (src >= 0xFE || ((src &= ~0x40) & 0x80) == 0) { // no vscroll
		_graphics->copyBuffer(getPagePtr(dst), getPagePtr(src));
	} else {
		const uint8 sl = getPagePtr(src & 3);
		const uint8 dl = getPagePtr(dst);
		if (sl != dl && vscroll >= -199 && vscroll <= 199) {
			_graphics->copyBuffer(dl, sl, vscroll);
		}
	}
}

static void decode_amiga(const uint8 *src, uint8 *dst) {
	static const int plane_size = 200 * 320 / 8;
	for (int y = 0; y < 200; ++y) {
		for (int x = 0; x < 320; x += 8) {
			for (int b = 0; b < 8; ++b) {
				const int mask = 1 << (7 - b);
				uint8 color = 0;
				for (int p = 0; p < 4; ++p) {
					if (src[p * plane_size] & mask) {
						color |= 1 << p;
					}
				}
				*dst++ = color;
			}
			++src;
		}
	}
}

static void decode_atari(const uint8 *src, uint8 *dst) {
	for (int y = 0; y < 200; ++y) {
		for (int x = 0; x < 320; x += 16) {
			for (int b = 0; b < 16; ++b) {
				const int mask = 1 << (15 - b);
				uint8 color = 0;
				for (int p = 0; p < 4; ++p) {
					if (READ_BE_UINT16(src + p * 2) & mask) {
						color |= 1 << p;
					}
				}
				*dst++ = color;
			}
			src += 8;
		}
	}
}

static void deinterlace555(const uint8 *src, int w, int h, uint16 *dst) {
	for (int y = 0; y < h / 2; ++y) {
		for (int x = 0; x < w; ++x) {
			dst[x] = READ_BE_UINT16(src) & 0x7FFF; src += 2;
			dst[w + x] = READ_BE_UINT16(src) & 0x7FFF; src += 2;
		}
		dst += w * 2;
	}
}

static void yflip(const uint8 *src, int w, int h, uint8 *dst) {
	for (int y = 0; y < h; ++y) {
		memcpy(dst + (h - 1 - y) * w, src, w);
		src += w;
	}
}

void Video::scaleBitmap(const uint8 *src, int fmt) {
	_graphics->drawBitmap(0, src, BITMAP_W, BITMAP_H, fmt);
}

void Video::copyBitmapPtr(const uint8 *src, uint32 size) {
	if (_res->getDataType() == DT_DOS || _res->getDataType() == DT_AMIGA) {
		decode_amiga(src, _tempBitmap);
		scaleBitmap(_tempBitmap, FMT_CLUT);
	} else if (_res->getDataType() == DT_ATARI) {
		decode_atari(src, _tempBitmap);
		scaleBitmap(_tempBitmap, FMT_CLUT);
	} else if (_res->getDataType() == DT_WIN31) {
		yflip(src, BITMAP_W, BITMAP_H, _tempBitmap);
		scaleBitmap(_tempBitmap, FMT_CLUT);
	} else if (_res->getDataType() == DT_3DO) {
		deinterlace555(src, BITMAP_W, BITMAP_H, _bitmap555);
		scaleBitmap((const uint8 *)_bitmap555, FMT_RGB555);
	} else { // .BMP
		if (Gfx::_is1991) {
			const int w = READ_LE_UINT32(src + 0x12);
			const int h = READ_LE_UINT32(src + 0x16);
			if (w == BITMAP_W && h == BITMAP_H) {
				const uint8 *data = src + READ_LE_UINT32(src + 0xA);
				yflip(data, w, h, _tempBitmap);
				scaleBitmap(_tempBitmap, FMT_CLUT);
			}
		} else {
			int w, h;
			uint8 *buf = decode_bitmap(src, false, -1, &w, &h);
			if (buf) {
				_graphics->drawBitmap(_buffers[0], buf, w, h, FMT_RGB);
				free(buf);
			}
		}
	}
}

static void readPaletteWin31(const uint8 *buf, int num, Color pal[16]) {
	const uint8 *p = buf + num * 16 * sizeof(uint16);
	for (int i = 0; i < 16; ++i) {
		const uint16 index = READ_LE_UINT16(p); p += 2;
		const uint32 color = READ_LE_UINT32(buf + 0xC04 + index * sizeof(uint32));
		pal[i].r = color & 255;
		pal[i].g = (color >> 8) & 255;
		pal[i].b = (color >> 16) & 255;
	}
}

static void readPalette3DO(const uint8 *buf, int num, Color pal[16]) {
	const uint8 *p = buf + num * 16 * sizeof(uint16);
	for (int i = 0; i < 16; ++i) {
		const uint16 color = READ_BE_UINT16(p); p += 2;
		const int r = (color >> 10) & 31;
		const int g = (color >> 5) & 31;
		const int b = color & 31;
		pal[i].r = (r << 3) | (r >> 2);
		pal[i].g = (g << 3) | (g >> 2);
		pal[i].b = (b << 3) | (b >> 2);
	}
}

static void readPaletteEGA(const uint8 *buf, int num, Color pal[16]) {
	const uint8 *p = buf + num * 16 * sizeof(uint16);
	p += 1024; // EGA colors are stored after VGA (Amiga)
	for (int i = 0; i < 16; ++i) {
		const uint16 color = READ_BE_UINT16(p); p += 2;
		if (1) {
			const uint8 *ega = &Video::PALETTE_EGA[3 * ((color >> 12) & 15)];
			pal[i].r = ega[0];
			pal[i].g = ega[1];
			pal[i].b = ega[2];
		} else { // lower 12 bits hold other colors
			const uint8 r = (color >> 8) & 0xF;
			const uint8 g = (color >> 4) & 0xF;
			const uint8 b = color & 0xF;
			pal[i].r = (r << 4) | r;
			pal[i].g = (g << 4) | g;
			pal[i].b = (b << 4) | b;
		}
	}
}

static void readPaletteAmiga(const uint8 *buf, int num, Color pal[16]) {
	const uint8 *p = buf + num * 16 * sizeof(uint16);
	for (int i = 0; i < 16; ++i) {
		const uint16 color = READ_BE_UINT16(p); p += 2;
		const uint8 r = (color >> 8) & 0xF;
		const uint8 g = (color >> 4) & 0xF;
		const uint8 b = color & 0xF;
		pal[i].r = (r << 4) | r;
		pal[i].g = (g << 4) | g;
		pal[i].b = (b << 4) | b;
	}
}

void Video::changePal(uint8 palNum) {
	if (palNum < 32 && palNum != _currentPal) {
		Color pal[16];
		if (_res->getDataType() == DT_WIN31) {
			readPaletteWin31(_res->_segVideoPal, palNum, pal);
		} else if (_res->getDataType() == DT_3DO) {
			readPalette3DO(_res->_segVideoPal, palNum, pal);
		} else if (_res->getDataType() == DT_DOS && _useEGA) {
			readPaletteEGA(_res->_segVideoPal, palNum, pal);
		} else {
			readPaletteAmiga(_res->_segVideoPal, palNum, pal);
		}
		_graphics->setPalette(pal, 16);
		_currentPal = palNum;
	}
}

void Video::updateDisplay(uint8 page, SystemStub *stub) {
	debugC(kDebugVideo, "Video::updateDisplay(%d)", page);
	if (page != 0xFE) {
		if (page == 0xFF) {
			SWAP(_buffers[1], _buffers[2]);
		} else {
			_buffers[1] = getPagePtr(page);
		}
	}
	if (_nextPal != 0xFF) {
		changePal(_nextPal);
		_nextPal = 0xFF;
	}
	_graphics->drawBuffer(_buffers[1], stub);
}

void Video::setPaletteColor(uint8 color, int r, int g, int b) {
	Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	_graphics->setPalette(&c, 1);
}

void Video::drawRect(uint8 page, uint8 color, int x1, int y1, int x2, int y2) {
	Point pt;
	pt.x = x1;
	pt.y = y1;
	_graphics->drawRect(page, color, &pt, x2 - x1, y2 - y1);
}

void Video::drawBitmap3DO(const char *name, SystemStub *stub) {
	assert(_res->getDataType() == DT_3DO);
	int w, h;
	uint16 *data = _res->_3do->loadShape555(name, &w, &h);
	if (data) {
		Graphics::Surface s;
		s.setPixels(data);
		s.w = s.pitch = w;
		s.h = h;
		s.format = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);

		_graphics->drawBitmapOverlay(s, FMT_RGB555, stub);
		free(data);
	}
}

} // namespace Awe
