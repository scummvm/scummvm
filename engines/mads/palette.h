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
#include "common/stream.h"

namespace MADS {

class MADSEngine;

#define PALETTE_USAGE_COUNT 4

/**
 * Palette mapping options
 */
enum {
	PALFLAG_BACKGROUND		= 0x8000,  // Loading initial background       
	PALFLAG_RESERVED		= 0x4000,  // Enable mapping reserved colors 
	PALFLAG_ANY_TO_CLOSEST	= 0x2000,  // Any color can map to closest     
	PALFLAG_ALL_TO_CLOSEST	= 0x1000,  // Any colors that can map must map 
	PALFLAG_TOP_COLORS		= 0x0800,  // Allow mapping to high four colors 
	PALFLAG_DEFINE_RESERVED	= 0x0400,  // Define initial reserved color    
	PALFLAG_MASK			= 0xfc00   // Mask for all the palette flags   
};

struct RGB4 {
	byte r;
	byte g;
	byte b;
	byte u;

	RGB4() { r = g = b = u = 0; }
};

struct RGB6 {
	byte r;
	byte g;
	byte b;
	byte _palIndex;
	byte _u2;
	byte _flags;

	void load(Common::SeekableReadStream *f);
};

class PaletteUsage {
public:
	struct UsageEntry {
		uint16 _palIndex;
		int _sortValue;

		UsageEntry(int palIndex) { _palIndex = palIndex; _sortValue = -1; }
		UsageEntry() { _palIndex = 0; _sortValue = 0; }
	};
	struct UsageRange {
		byte _v1, _v2;

		UsageRange(byte v1, byte v2) { _v1 = v1; _v2 = v2; }
	};
private:
	MADSEngine *_vm;
	Common::Array<UsageEntry> *_data;

	int rgbMerge(RGB6 &palEntry);

	int getGamePalFreeIndex(int *palIndex);

	int rgbFactor(byte *palEntry, RGB6 &pal6);

	Common::Array<UsageEntry> _nullUsage;
public:
	/**
	 * Constructor
	 */
	PaletteUsage(MADSEngine *vm);

	void load(Common::Array<UsageEntry> *data);

	/**
	 * Returns whether the usage list is empty
	 */
	bool empty() const { return _data == nullptr; }

	uint16 &operator[](int index) { return (*_data)[index]._palIndex; }

	/**
	 * Assigns the class to an empty usage array
	 */
	void setEmpty() { _data = &_nullUsage; }

	/**
	 * Gets key entries from the passed palette
	 * @param palette	6-bit per entry read in palette
	 */
	void getKeyEntries(Common::Array<RGB6> &palette);

	/**
	 * Prioritizes the palette index list based on the intensity of the
	 * RGB values of the palette entries that they refer to
	 */
	void prioritize(Common::Array<RGB6> &palette);

	int process(Common::Array<RGB6> &palette, uint flags);

	void transform(Common::Array<RGB6> &palette);

	void updateUsage(Common::Array<int> &usageList, int sceneUsageIndex);
};

class RGBList {
private:
	bool _data[32];
public:
	RGBList() { clear(); }

	void clear();

	void reset();

	/**
	 * Scans for a free slot
	 */
	int scan();

	bool &operator[](int idx) { return _data[idx]; }
};

#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

class Palette {
private:
	/**
	 * Initialises the first 16 palette indexes with the equivalent
	 * standard VGA palette
	 */
	void initVGAPalette(byte *palette);
protected:
	MADSEngine *_vm;

	void reset();
public:
	byte _mainPalette[PALETTE_SIZE];
	byte _savedPalette[PALETTE_SIZE];
	uint32 _palFlags[PALETTE_COUNT];
	PaletteUsage _paletteUsage;
	RGBList _rgbList;
	bool _lockFl;
	int _lowRange;
	int _highRange;
public:
	/**
	 * Constructor
	 */
	Palette(MADSEngine *vm);
	
	/**
	 * Destructor
	 */
	virtual ~Palette() {}

	/**
	 * Sets a new palette
	 */
	void setPalette(const byte *colors, uint start, uint num);

	/**
	 * Set a palette entry
	 */
	void setEntry(byte palIndex, byte r, byte g, byte b);

	/**
	 * Returns a subset of the currently loaded palette
	 */
	void grabPalette(byte *colors, uint start, uint num);
	
	/**
	 * Gets the entire palette at once
	 */
	void getFullPalette(byte palette[PALETTE_SIZE]) {
		grabPalette(&palette[0], 0, PALETTE_COUNT);
	}

	/**
	 * Sets the entire palette at once
	 */
	void setFullPalette(byte palette[PALETTE_SIZE]) {
		setPalette(&palette[0], 0, PALETTE_COUNT);
	}

	/**
	 * Returns the palette index in the palette that most closely matches the
	 * specified RGB pair
	 */
	uint8 palIndexFromRgb(byte r, byte g, byte b, byte *paletteData = nullptr);

	/**
	 * Sets a small set of system/core colors needed by the game
	 */
	void setSystemPalette();

	/**
	 * Update a range of an arbitrary palette
	 */
	static void setGradient(byte *palette, int start, int count, int rgbValue1, int rgbValue2);

	/**
	 * Resets the game palette
	 */
	void resetGamePalette(int v1, int v2);

	/**
	 * Initialises the main palette
	 */
	void initPalette();

	/**
	 * Set the first four palette entries with preset values
	 */
	void setLowRange();

	/**
	 * Set up the palette as the game ends
	 */
	void close() {
		warning("TODO: Palette::close");
	}

	void fadeOut(byte palette[PALETTE_SIZE], int v1, int v2, int v3, int v4, int v5, int v6);

	void lock();
	void unlock();
};

} // End of namespace MADS

#endif /* MADS_PALETTE_H */
