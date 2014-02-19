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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_PALETTE_H
#define MADS_PALETTE_H

#include "common/scummsys.h"

namespace MADS {

class MADSEngine;

struct RGB8 {
	uint8 r, g, b, u;
};

class RGBList {
private:
	int _size;
	RGB8 *_data;
	byte *_palIndexes;
	bool _freeData;
public:
	RGBList(int numEntries = 256, RGB8 *srcData = NULL, bool freeData = true);
	~RGBList();

	RGB8 *data() { return _data; }
	byte *palIndexes() { return _palIndexes; }
	int size() { return _size; }
};

#define PALETTE_COUNT 256

class Palette {
private:
	MADSEngine *_vm;
	bool _colorsChanged;
	bool _fading_in_progress;
	byte _originalPalette[PALETTE_COUNT * 4];
	byte _fadedPalette[PALETTE_COUNT * 4];
	int _usageCount[PALETTE_COUNT];

	void reset();
public:
	Palette(MADSEngine *vm);

	void setPalette(const byte *colors, uint start, uint num);
	void setPalette(const RGB8 *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num);
	void grabPalette(RGB8 *colors, uint start, uint num) {
		grabPalette((byte *)colors, start, num);
	}
	uint8 palIndexFromRgb(byte r, byte g, byte b, RGB8 *paletteData = NULL);

	void fadeIn(int numSteps, uint delayAmount, RGB8 *destPalette, int numColors);
	void fadeIn(int numSteps, uint delayAmount, RGBList *destPalette);
	static RGB8 *decodeMadsPalette(Common::SeekableReadStream *palStream,  int *numColors);
	int setMadsPalette(Common::SeekableReadStream *palStream, int indexStart = 0);
	void setMadsSystemPalette();
	void fadeRange(RGB8 *srcPal, RGB8 *destPal,  int startIndex, int endIndex, 
		int numSteps, uint delayAmount);

	// Methods used for reference counting color usage
	void resetColorCounts();
	void blockRange(int startIndex, int size);
	void addRange(RGBList *list);
	void deleteRange(RGBList *list);
	void deleteAllRanges();

	// Color indexes
	uint8 BLACK;
	uint8 BLUE;
	uint8 GREEN;
	uint8 CYAN;
	uint8 RED;
	uint8 VIOLET;
	uint8 BROWN;
	uint8 LIGHT_GRAY;
	uint8 DARK_GRAY;
	uint8 LIGHT_BLUE;
	uint8 LIGHT_GREEN;
	uint8 LIGHT_CYAN;
	uint8 LIGHT_RED;
	uint8 PINK;
	uint8 YELLOW;
	uint8 WHITE;
};

} // End of namespace MADS

#endif /* MADS_PALETTE_H */
