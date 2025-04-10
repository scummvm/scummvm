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

#ifndef AWE_VIDEO_H
#define AWE_VIDEO_H

#include "intern.h"

namespace Awe {

struct StrEntry {
	uint16 id;
	const char *str;
};

struct Polygon {
	enum {
		MAX_POINTS = 50
	};

	uint16 bbw, bbh;
	uint8 numPoints;
	Point points[MAX_POINTS];

	void init(const uint8 *p, uint16 zoom);
};

struct Resource;
struct Serializer;
struct SystemStub;

struct Video {
	typedef void (Video::*drawLine)(int16 x1, int16 x2, uint8 col);

	enum {
		VID_PAGE_SIZE  = 320 * 200 / 2
	};

	static const uint8 _font[];
	static const StrEntry _stringsTableEng[];
	static const StrEntry _stringsTableDemo[];

	Resource *_res;
	SystemStub *_stub;

	uint8 _newPal, _curPal;
	uint8 *_pagePtrs[4];
	uint8 *_curPagePtr1, *_curPagePtr2, *_curPagePtr3;
	Polygon _pg;
	int16 _hliney;
	uint16 _interpTable[0x400];
	Ptr _pData;
	uint8 *_dataBuf;

	Video(Resource *res, SystemStub *stub);
	void init();

	void setDataBuffer(uint8 *dataBuf, uint16 offset);
	void drawShape(uint8 color, uint16 zoom, const Point &pt);
	void fillPolygon(uint16 color, uint16 zoom, const Point &pt);
	void drawShapeParts(uint16 zoom, const Point &pt);
	int32 calcStep(const Point &p1, const Point &p2, uint16 &dy);

	void drawString(uint8 color, uint16 x, uint16 y, uint16 strId);
	void drawChar(uint8 c, uint16 x, uint16 y, uint8 color, uint8 *buf);
	void drawPoint(uint8 color, int16 x, int16 y);
	void drawLineT(int16 x1, int16 x2, uint8 color);
	void drawLineN(int16 x1, int16 x2, uint8 color);
	void drawLineP(int16 x1, int16 x2, uint8 color);
	uint8 *getPagePtr(uint8 page);
	void changePagePtr1(uint8 page);
	void fillPage(uint8 page, uint8 color);
	void copyPage(uint8 src, uint8 dst, int16 vscroll);
	void copyPagePtr(const uint8 *src);
	uint8 *allocPage();
	void changePal(uint8 pal);
	void updateDisplay(uint8 page);
	
	void saveOrLoad(Serializer &ser);
};

} // namespace Awe

#endif
