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

#include "awe/awe.h"
#include "awe/video.h"
#include "awe/resource.h"
#include "awe/serializer.h"
#include "awe/system_stub.h"

namespace Awe {

void Polygon::init(const uint8 *p, uint16 zoom) {
	bbw = (*p++) * zoom / 64;
	bbh = (*p++) * zoom / 64;
	numPoints = *p++;
	assert((numPoints & 1) == 0 && numPoints < MAX_POINTS);
	for (int i = 0; i < numPoints; ++i) {
		Point *pt = &points[i];
		pt->x = (*p++) * zoom / 64;
		pt->y = (*p++) * zoom / 64;
	}
}

Video::Video(Resource *res, SystemStub *stub) 
	: _res(res), _stub(stub) {
}

void Video::init() {
	_newPal = 0xFF;
	for (int i = 0; i < 4; ++i) {
		_pagePtrs[i] = allocPage();
	}
	_curPagePtr3 = getPagePtr(1);
	_curPagePtr2 = getPagePtr(2);
	changePagePtr1(0xFE);
	_interpTable[0] = 0x4000;
	for (int i = 1; i < 0x400; ++i) {
		_interpTable[i] = 0x4000 / i;
	}
}

void Video::setDataBuffer(uint8 *dataBuf, uint16 offset) {
	_dataBuf = dataBuf;
	_pData.pc = dataBuf + offset;
}

void Video::drawShape(uint8 color, uint16 zoom, const Point &pt) {
	uint8 i = _pData.fetchByte();
	if (i >= 0xC0) {
		if (color & 0x80) {
			color = i & 0x3F;
		}
		_pg.init(_pData.pc, zoom);
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

void Video::fillPolygon(uint16 color, uint16 zoom, const Point &pt) {
	if (_pg.bbw == 0 && _pg.bbh == 1 && _pg.numPoints == 4) {
		drawPoint(color, pt.x, pt.y);
		return;
	}
	
	int16 x1 = pt.x - _pg.bbw / 2;
	int16 x2 = pt.x + _pg.bbw / 2;
	int16 y1 = pt.y - _pg.bbh / 2;
	int16 y2 = pt.y + _pg.bbh / 2;

	if (x1 > 319 || x2 < 0 || y1 > 199 || y2 < 0)
		return;

	_hliney = y1;
	
	uint16 i, j;
	i = 0;
	j = _pg.numPoints - 1;
	
	x2 = _pg.points[i].x + x1;
	x1 = _pg.points[j].x + x1;

	++i;
	--j;

	drawLine pdl;
	if (color < 0x10) {
		pdl = &Video::drawLineN;
	} else if (color > 0x10) {
		pdl = &Video::drawLineP;
	} else {
		pdl = &Video::drawLineT;
	}

	uint32 cpt1 = x1 << 16;
	uint32 cpt2 = x2 << 16;

	while (1) {
		_pg.numPoints -= 2;
		if (_pg.numPoints == 0) {
			return;
		}
		uint16 h;
		int32 step1 = calcStep(_pg.points[j + 1], _pg.points[j], h);
		int32 step2 = calcStep(_pg.points[i - 1], _pg.points[i], h);

		++i;
		--j;

		cpt1 = (cpt1 & 0xFFFF0000) | 0x7FFF;
		cpt2 = (cpt2 & 0xFFFF0000) | 0x8000;

		if (h == 0) {	
			cpt1 += step1;
			cpt2 += step2;
		} else {
			for (; h != 0; --h) {
				if (_hliney >= 0) {
					x1 = cpt1 >> 16;
					x2 = cpt2 >> 16;
					if (x1 <= 319 && x2 >= 0) {
						if (x1 < 0) x1 = 0;
						if (x2 > 319) x2 = 319;
						(this->*pdl)(x1, x2, color);
					}
				}
				cpt1 += step1;
				cpt2 += step2;
				++_hliney;					
				if (_hliney > 199) return;
			}
		}
	}
}

void Video::drawShapeParts(uint16 zoom, const Point &pgc) {
	Point pt(pgc);
	pt.x -= _pData.fetchByte() * zoom / 64;
	pt.y -= _pData.fetchByte() * zoom / 64;
	int16 n = _pData.fetchByte();
	debugC(kDebugVideo, "Video::drawShapeParts n=%d", n);
	for ( ; n >= 0; --n) {
		uint16 off = _pData.fetchWord();
		Point po(pt);
		po.x += _pData.fetchByte() * zoom / 64;
		po.y += _pData.fetchByte() * zoom / 64;
		uint16 color = 0xFF;
		uint16 _bp = off;
		off &= 0x7FFF;
		if (_bp & 0x8000) {
			color = *_pData.pc & 0x7F;
			_pData.pc += 2;
		}
		uint8 *bak = _pData.pc;
		_pData.pc = _dataBuf + off * 2;
		drawShape(color, zoom, po);
		_pData.pc = bak;
	}
}

int32 Video::calcStep(const Point &p1, const Point &p2, uint16 &dy) {
	dy = p2.y - p1.y;
	return (p2.x - p1.x) * _interpTable[dy] * 4;
}

void Video::drawString(uint8 color, uint16 x, uint16 y, uint16 strId) {
	const StrEntry *se = g_engine->isDemo() ?
		_stringsTableDemo : _stringsTableEng;

	while (se->id != 0xFFFF && se->id != strId) ++se;
	debugC(kDebugVideo, "drawString(%d, %d, %d, '%s')", color, x, y, se->str);
	uint16 xx = x;
	int len = strlen(se->str);
	for (int i = 0; i < len; ++i) {
		if (se->str[i] == '\n') {
			y += 8;
			x = xx;
		} else {
			drawChar(se->str[i], x, y, color, _curPagePtr1);
			++x;
		}
	}
}

void Video::drawChar(uint8 c, uint16 x, uint16 y, uint8 color, uint8 *buf) {
	if (x <= 39 && y <= 192) {
		const uint8 *ft = _font + (c - 0x20) * 8;
		uint8 *p = buf + x * 4 + y * 160;
		for (int j = 0; j < 8; ++j) {
			uint8 ch = *(ft + j);
			for (int i = 0; i < 4; ++i) {
				uint8 b = *(p + i);
				uint8 cmask = 0xFF;
				uint8 colb = 0;
				if (ch & 0x80) {
					colb |= color << 4;
					cmask &= 0x0F;
				}
				ch <<= 1;
				if (ch & 0x80) {
					colb |= color;
					cmask &= 0xF0;
				}
				ch <<= 1;
				*(p + i) = (b & cmask) | colb;
			}
			p += 160;
		}
	}
}

void Video::drawPoint(uint8 color, int16 x, int16 y) {
	debugC(kDebugVideo, "drawPoint(%d, %d, %d)", color, x, y);
	if (x >= 0 && x <= 319 && y >= 0 && y <= 199) {
		uint16 off = y * 160 + x / 2;
	
		uint8 cmasko, cmaskn;
		if (x & 1) {
			cmaskn = 0x0F;
			cmasko = 0xF0;
		} else {
			cmaskn = 0xF0;
			cmasko = 0x0F;
		}

		uint8 colb = (color << 4) | color;
		if (color == 0x10) {
			cmaskn &= 0x88;
			cmasko = ~cmaskn;
			colb = 0x88;		
		} else if (color == 0x11) {
			colb = *(_pagePtrs[0] + off);
		}
		uint8 b = *(_curPagePtr1 + off);
		*(_curPagePtr1 + off) = (b & cmasko) | (colb & cmaskn);
	}
}

void Video::drawLineT(int16 x1, int16 x2, uint8 color) {
	debugC(kDebugVideo, "drawLineT(%d, %d, %d)", x1, x2, color);
	int16 xmax = MAX(x1, x2);
	int16 xmin = MIN(x1, x2);
	uint8 *p = _curPagePtr1 + _hliney * 160 + xmin / 2;

	uint16 w = xmax / 2 - xmin / 2 + 1;
	uint8 cmaske = 0;
	uint8 cmasks = 0;	
	if (xmin & 1) {
		--w;
		cmasks = 0xF7;
	}
	if (!(xmax & 1)) {
		--w;
		cmaske = 0x7F;
	}

	if (cmasks != 0) {
		*p = (*p & cmasks) | 0x08;
		++p;
	}
	while (w--) {
		*p = (*p & 0x77) | 0x88;
		++p;
	}
	if (cmaske != 0) {
		*p = (*p & cmaske) | 0x80;
		++p;
	}
}

void Video::drawLineN(int16 x1, int16 x2, uint8 color) {
	debugC(kDebugVideo, "drawLineN(%d, %d, %d)", x1, x2, color);
	int16 xmax = MAX(x1, x2);
	int16 xmin = MIN(x1, x2);
	uint8 *p = _curPagePtr1 + _hliney * 160 + xmin / 2;

	uint16 w = xmax / 2 - xmin / 2 + 1;
	uint8 cmaske = 0;
	uint8 cmasks = 0;	
	if (xmin & 1) {
		--w;
		cmasks = 0xF0;
	}
	if (!(xmax & 1)) {
		--w;
		cmaske = 0x0F;
	}

	uint8 colb = ((color & 0xF) << 4) | (color & 0xF);	
	if (cmasks != 0) {
		*p = (*p & cmasks) | (colb & 0x0F);
		++p;
	}
	while (w--) {
		*p++ = colb;
	}
	if (cmaske != 0) {
		*p = (*p & cmaske) | (colb & 0xF0);
		++p;		
	}
}

void Video::drawLineP(int16 x1, int16 x2, uint8 color) {
	debugC(kDebugVideo, "drawLineP(%d, %d, %d)", x1, x2, color);
	int16 xmax = MAX(x1, x2);
	int16 xmin = MIN(x1, x2);
	uint16 off = _hliney * 160 + xmin / 2;
	uint8 *p = _curPagePtr1 + off;
	uint8 *q = _pagePtrs[0] + off;

	uint8 w = xmax / 2 - xmin / 2 + 1;
	uint8 cmaske = 0;
	uint8 cmasks = 0;	
	if (xmin & 1) {
		--w;
		cmasks = 0xF0;
	}
	if (!(xmax & 1)) {
		--w;
		cmaske = 0x0F;
	}

	if (cmasks != 0) {
		*p = (*p & cmasks) | (*q & 0x0F);
		++p;
		++q;
	}
	while (w--) {
		*p++ = *q++;			
	}
	if (cmaske != 0) {
		*p = (*p & cmaske) | (*q & 0xF0);
		++p;
		++q;
	}
}

uint8 *Video::getPagePtr(uint8 page) {
	uint8 *p;
	if (page <= 3) {
		p = _pagePtrs[page];
	} else {
		switch (page) {
		case 0xFF:
			p = _curPagePtr3;
			break;
		case 0xFE:
			p = _curPagePtr2;
			break;
		default:
			p = _pagePtrs[0]; // XXX check
			warning("Video::getPagePtr() p != [0,1,2,3,0xFF,0xFE] == 0x%X", page);
			break;
		}
	}
	return p;
}

void Video::changePagePtr1(uint8 page) {
	debugC(kDebugVideo, "Video::changePagePtr1(%d)", page);
	_curPagePtr1 = getPagePtr(page);
}

void Video::fillPage(uint8 page, uint8 color) {
	debugC(kDebugVideo, "Video::fillPage(%d, %d)", page, color);
	uint8 *p = getPagePtr(page);
	uint8 c = (color << 4) | color;
	memset(p, c, VID_PAGE_SIZE);
}

void Video::copyPage(uint8 src, uint8 dst, int16 vscroll) {
	debugC(kDebugVideo, "Video::copyPage(%d, %d)", src, dst);
	if (src >= 0xFE || !((src &= 0xBF) & 0x80)) {
		uint8 *p = getPagePtr(src);
		uint8 *q = getPagePtr(dst);
		if (p != q) {
			memcpy(q, p, VID_PAGE_SIZE);
		}		
	} else {
		uint8 *p = getPagePtr(src & 3);
		uint8 *q = getPagePtr(dst);
		if (p != q && vscroll >= -199 && vscroll <= 199) {
			uint16 h = 200;
			if (vscroll < 0) {
				h += vscroll;
				p += -vscroll * 160;
			} else {
				h -= vscroll;
				q += vscroll * 160;
			}
			memcpy(q, p, h * 160);
		}
	}
}

void Video::copyPagePtr(const uint8 *src) {
	debugC(kDebugVideo, "Video::copyPagePtr()");
	uint8 *dst = _pagePtrs[0];
	int h = 200;
	while (h--) {
		int w = 40;
		while (w--) {
			uint8 p[] = {
				*(src + 8000 * 3),
				*(src + 8000 * 2),
				*(src + 8000 * 1),
				*(src + 8000 * 0)
			};
			for(int j = 0; j < 4; ++j) {
				uint8 acc = 0;
				for (int i = 0; i < 8; ++i) {
					acc <<= 1;
					acc |= (p[i & 3] & 0x80) ? 1 : 0;
					p[i & 3] <<= 1;
				}
				*dst++ = acc;
			}			
			++src;
		}
	}
}

uint8 *Video::allocPage() {
	uint8 *buf = (uint8 *)malloc(VID_PAGE_SIZE);
	memset(buf, 0, VID_PAGE_SIZE);
	return buf;
}

void Video::changePal(uint8 palNum) {
	if (palNum < 32) {
		uint8 *p = _res->_segVideoPal + palNum * 32;
		uint8 pal[16 * 3];
		for (int i = 0; i < 16; ++i) {
			uint8 c1 = *(p + 0);
			uint8 c2 = *(p + 1);
			p += 2;
			pal[i * 3 + 0] = ((c1 & 0x0F) << 2) | ((c1 & 0x0F) >> 2); // r
			pal[i * 3 + 1] = ((c2 & 0xF0) >> 2) | ((c2 & 0xF0) >> 6); // g
			pal[i * 3 + 2] = ((c2 & 0x0F) >> 2) | ((c2 & 0x0F) << 2); // b
		}
		_stub->setPalette(0, 16, pal);
		_curPal = palNum;
	}
}

void Video::updateDisplay(uint8 page) {
	debugC(kDebugVideo, "Video::updateDisplay(%d)", page);
	if (page != 0xFE) {
		if (page == 0xFF) {
			SWAP(_curPagePtr2, _curPagePtr3);
		} else {
			_curPagePtr2 = getPagePtr(page);
		}
	}
	if (_newPal != 0xFF) {
		changePal(_newPal);
		_newPal = 0xFF;
	}
	_stub->copyRect(0, 0, 320, 200, _curPagePtr2, 160);
}

void Video::saveOrLoad(Serializer &ser) {
	uint8 mask = 0;
	if (ser._mode == Serializer::SM_SAVE) {
		for (int i = 0; i < 4; ++i) {
			if (_pagePtrs[i] == _curPagePtr1)
				mask |= i << 4;
			if (_pagePtrs[i] == _curPagePtr2)
				mask |= i << 2;
			if (_pagePtrs[i] == _curPagePtr3)
				mask |= i << 0;
		}		
	}
	Serializer::Entry entries[] = {
		SE_INT(&_curPal, Serializer::SES_INT8, VER(1)),
		SE_INT(&_newPal, Serializer::SES_INT8, VER(1)),
		SE_INT(&mask, Serializer::SES_INT8, VER(1)),
		SE_ARRAY(_pagePtrs[0], Video::VID_PAGE_SIZE, Serializer::SES_INT8, VER(1)),
		SE_ARRAY(_pagePtrs[1], Video::VID_PAGE_SIZE, Serializer::SES_INT8, VER(1)),
		SE_ARRAY(_pagePtrs[2], Video::VID_PAGE_SIZE, Serializer::SES_INT8, VER(1)),
		SE_ARRAY(_pagePtrs[3], Video::VID_PAGE_SIZE, Serializer::SES_INT8, VER(1)),
		SE_END()
	};
	ser.saveOrLoadEntries(entries);
	if (ser._mode == Serializer::SM_LOAD) {
		_curPagePtr1 = _pagePtrs[(mask >> 4) & 0x3];
		_curPagePtr2 = _pagePtrs[(mask >> 2) & 0x3];
		_curPagePtr3 = _pagePtrs[(mask >> 0) & 0x3];
		changePal(_curPal);
	}
}

} // namespace Awe
