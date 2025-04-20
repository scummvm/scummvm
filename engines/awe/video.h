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
	uint16_t id;
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

	static const StrEntry _stringsTableFr[];
	static const StrEntry _stringsTableEng[];
	static const StrEntry _stringsTableDemo[];
	static const uint16_t _stringsId15th[];
	static const char *_stringsTable15th[];
	static const char *_str0x194AtariDemo;
	static const StrEntry _stringsTable3DO[];
	static const char *_noteText3DO;
	static const char *_endText3DO;
	static const uint8_t *_vertices3DO[201];
	static const uint8_t _paletteEGA[];

	static bool _useEGA;

	Resource *_res;
	Gfx *_graphics = nullptr;
	bool _hasHeadSprites = false;
	bool _displayHead = false;

	uint8_t _nextPal = 0, _currentPal = 0;
	uint8_t _buffers[3] = { 0 };
	Ptr _pData;
	uint8_t *_dataBuf = nullptr;
	const StrEntry *_stringsTable = nullptr;
	uint8_t _tempBitmap[BITMAP_W * BITMAP_H] = { 0 };
	uint16_t _bitmap555[BITMAP_W * BITMAP_H] = { 0 };
	const Scaler *_scaler = nullptr;
	int _scalerFactor = 0;
	uint8_t *_scalerBuffer = nullptr;

	Video(Resource *res);
	~Video();
	void init();

	void setScaler(const char *name, int factor);
	void setDefaultFont();
	void setFont(const uint8_t *font);
	void setHeads(const uint8_t *src);
	void setDataBuffer(uint8_t *dataBuf, uint16_t offset);
	void drawShape(uint8_t color, uint16_t zoom, const Point *pt);
	void drawShapePart3DO(int color, int part, const Point *pt);
	void drawShape3DO(int color, int zoom, const Point *pt);
	void fillPolygon(uint16_t color, uint16_t zoom, const Point *pt);
	void drawShapeParts(uint16_t zoom, const Point *pt);
	void drawString(uint8_t color, uint16_t x, uint16_t y, uint16_t strId);
	uint8_t getPagePtr(uint8_t page);
	void setWorkPagePtr(uint8_t page);
	void fillPage(uint8_t page, uint8_t color);
	void copyPage(uint8_t src, uint8_t dst, int16_t vscroll);
	void scaleBitmap(const uint8_t *src, int fmt);
	void copyBitmapPtr(const uint8_t *src, uint32_t size = 0);
	void changePal(uint8_t pal);
	void updateDisplay(uint8_t page, SystemStub *stub);
	void setPaletteColor(uint8_t color, int r, int g, int b);
	void drawRect(uint8_t page, uint8_t color, int x1, int y1, int x2, int y2);
	void drawBitmap3DO(const char *name, SystemStub *stub);
};

} // namespace Awe

#endif
