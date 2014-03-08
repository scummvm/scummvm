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

struct RGB4 {
	byte r;
	byte g;
	byte b;
	byte u;
};

struct RGB6 {
	byte r;
	byte g;
	byte b;
	byte palIndex;
	byte u2;
	byte flags;

	void load(Common::SeekableReadStream *f);
};

class PaletteUsage {
private:
	Common::Array<int> _data;

	int rgbMerge(RGB6 &palEntry);

	void prioritizeFromList(int lst[3]);
public:
	PaletteUsage();

	void load(int count, ...);

	/**
	 * Returns whether the usage list is empty
	 */
	bool empty() const { return _data.size() == 0;  }

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

	bool process(Common::Array<RGB6> &palette, int v) {
		warning("TODO: PaletteUsage::process");
		return 0;
	}

	void transform(Common::Array<RGB6> &palette);
};


#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

class Palette {
private:
	/**
	 * Initialises a stanadrd range of colours for the given palette
	 */
	void initRange(byte *palette);
protected:
	MADSEngine *_vm;
	bool _colorsChanged;

	bool _fading_in_progress;
	byte _originalPalette[PALETTE_COUNT * 4];
	byte _fadedPalette[PALETTE_COUNT * 4];
	int _usageCount[PALETTE_COUNT];

	void reset();
public:
	byte _mainPalette[PALETTE_SIZE];
	byte _savedPalette[PALETTE_SIZE];
	RGB4 _gamePalette[PALETTE_COUNT];
	PaletteUsage _paletteUsage;
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
	 * Returns the palette index in the palette that most closely matches the
	 * specified RGB pair
	 */
	uint8 palIndexFromRgb(byte r, byte g, byte b, byte *paletteData = nullptr);

	/**
	 * Decode a palette and return it, without affecting the Palette itself
	 */
	byte *decodePalette(Common::SeekableReadStream *palStream, int *numColors);

	/**
	 * Loads a palette from a stream
	 */
	int loadPalette(Common::SeekableReadStream *palStream, int indexStart = 0);

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
	 * Initialises game palette
	 */
	void initGamePalette();

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
};

} // End of namespace MADS

#endif /* MADS_PALETTE_H */
