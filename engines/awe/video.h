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

#include "awe/intern.h"

namespace Awe {

struct StrEntry {
	uint16 id;
	const char *str;
};

struct Gfx;
struct Resource;
struct Scaler;
struct SystemStub;

struct Video {
	enum {
		BITMAP_W = 320,
		BITMAP_H = 200
	};

	static const StrEntry STRINGS_TABLE_FR[];
	static const StrEntry STRINGS_TABLE_ENG[];
	static const StrEntry STRINGS_TABLE_DEMO[];
	static const uint16 STRINGS_ID_15TH[];
	static const char *STRINGS_TABLE_15TH[];
	static const char *_str0x194AtariDemo;
	static const StrEntry STRINGS_TABLE_3DO[];
	static const char *NOTE_TEXT_3DO;
	static const char *END_TEXT_3DO;
	static const uint8 *VERTICES_3DO[201];
	static const uint8 PALETTE_EGA[];

	static bool _useEGA;

	Resource *_res;
	Gfx *_graphics = nullptr;
	bool _hasHeadSprites = false;
	bool _displayHead = true;

	uint8 _nextPal = 0, _currentPal = 0;
	uint8 _buffers[3] = { 0 };
	Ptr _pData;
	uint8 *_dataBuf = nullptr;
	const StrEntry *_stringsTable = nullptr;
	uint8 _tempBitmap[BITMAP_W * BITMAP_H] = { 0 };
	uint16 _bitmap555[BITMAP_W * BITMAP_H] = { 0 };

	Video(Resource *res);
	~Video() {}
	void init();

	void setDefaultFont();
	void setFont(const uint8 *font);
	void setHeads(const uint8 *src);
	void setDataBuffer(uint8 *dataBuf, uint16 offset);
	void drawShape(uint8 color, uint16 zoom, const Point *pt);
	void drawShapePart3DO(int color, int part, const Point *pt);
	void drawShape3DO(int color, int zoom, const Point *pt);
	void fillPolygon(uint16 color, uint16 zoom, const Point *pt);
	void drawShapeParts(uint16 zoom, const Point *pt);
	void drawString(uint8 color, uint16 x, uint16 y, uint16 strId);
	uint8 getPagePtr(uint8 page);
	void setWorkPagePtr(uint8 page);
	void fillPage(uint8 page, uint8 color);
	void copyPage(uint8 src, uint8 dst, int16 vscroll);
	void scaleBitmap(const uint8 *src, int fmt);
	void copyBitmapPtr(const uint8 *src, uint32 size = 0);
	void changePal(uint8 pal);
	void updateDisplay(uint8 page, SystemStub *stub);
	void setPaletteColor(uint8 color, int r, int g, int b);
	void drawRect(uint8 page, uint8 color, int x1, int y1, int x2, int y2);
	void drawBitmap3DO(const char *name, SystemStub *stub);
};

} // namespace Awe

#endif
