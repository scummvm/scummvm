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

#ifndef GRAPHICS_PALETTE_H
#define GRAPHICS_PALETTE_H

#include "common/hashmap.h"

namespace Graphics {

enum ColorDistanceMethod {
	kColorDistanceNaive,	///< Weighted red 30%, green 50%, blue 20%
	kColorDistanceRedmean,	///< Common low-cost approximation
};

/**
 * Constants available for use in paletted code
 */
#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

/**
 * @brief Simple class for handling a palette data.
 *
 * The palette data is specified in interleaved RGB format. That is, the
 * first byte of the memory block 'colors' points at is the red component
 * of the first new color; the second byte the green component of the first
 * new color; the third byte the blue component, the last byte to the alpha
 * (transparency) value. Then the second color starts, and so on. So memory
 * looks like this: R1-G1-B1-R2-G2-B2-R3-...
 */
class Palette {
	byte *_data;
	uint16 _size;

public:
	static const uint16 npos = 0xFFFF;

	/**
	 * @brief Construct a new Palette object
	 *
	 * @param size   the number of palette entries
	 */
	Palette(uint size);

	/**
	 * @brief Construct a new Palette object
	 *
	 * @param data   the palette data, in interleaved RGB format
	 * @param size   the number of palette entries
	 */
	Palette(const byte *data, uint size);

	Palette(const Palette &p);

	~Palette();

	/**
	 * Constructs a new palette containing the standarad EGA palette
	 */
	static Palette createEGAPalette();

	Palette &operator=(const Palette &rhs);
	bool operator==(const Palette &rhs) const { return equals(rhs); }
	bool operator!=(const Palette &rhs) const { return !equals(rhs); }

	bool equals(const Palette &p) const;

	bool contains(const Palette &p) const;

	const byte *data() const { return _data; }
	uint size() const { return _size; }

	void set(uint entry, byte r, byte g, byte b) {
		assert(entry < _size);
		_data[entry * 3 + 0] = r;
		_data[entry * 3 + 1] = g;
		_data[entry * 3 + 2] = b;
	}

	void get(uint entry, byte &r, byte &g, byte &b) const {
		assert(entry < _size);
		r = _data[entry * 3 + 0];
		g = _data[entry * 3 + 1];
		b = _data[entry * 3 + 2];
	}

	/**
	 * Finds the index of an exact color from the palette.
	 *
	 * @return the palette index or npos if not found
	 */
	uint find(byte r, byte g, byte b) const {
		for (uint i = 0; i < _size; i++) {
			if (_data[i * 3 + 0] == r && _data[i * 3 + 1] == g && _data[i * 3 + 2] == b)
				return i;
		}
		return npos;
	}

	/**
	 * Finds the index of the closest color from the palette.
	 *
	 * @param method           the method used to determine the closest color
	 *
	 * @return the palette index
	 */
	byte findBestColor(byte r, byte g, byte b, ColorDistanceMethod method = kColorDistanceRedmean) const;

	void clear();

	/**
	 * Replace the specified range of the palette with new colors.
	 * The palette entries from 'start' till (start+num-1) will be replaced - so
	 * a full palette update is accomplished via start=0, num=256.
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note It is an error if start+num exceeds 256.
	 */
	void set(const byte *colors, uint start, uint num);
	void set(const Palette &p, uint start, uint num);

	/**
	 * Grabs a specified part of the currently active palette.
	 * The format is the same as for setPalette.
	 *
	 * @param colors	the palette data, in interleaved RGB format
	 * @param start		the first platte entry to be read
	 * @param num		the number of palette entries to be read
	 *
	 * @note It is an error if start+num exceeds 256.
	 */
	void grab(byte *colors, uint start, uint num) const;
	void grab(Palette &p, uint start, uint num) const;
};

class PaletteLookup {
public:
	PaletteLookup();
	/**
	 * @brief Construct a new Palette Lookup object
	 *
	 * @param palette   the palette data, in interleaved RGB format
	 * @param len       the number of palette entries to be read
	 */
	PaletteLookup(const byte *palette, uint len);

	/**
	 * @brief Pass palette to the look up. It also compares given palette
	 * with the current one and resets cache only when their contents is different.
	 *
	 * @param palette   the palette data, in interleaved RGB format
	 * @param len       the number of palette entries to be read
	 *
	 * @return true if palette was changed and false if it was the same
	 */
	bool setPalette(const byte *palette, uint len);

	/**
	 * @brief This method returns closest color from the palette
	 *        and it uses cache for faster lookups
	 *
	 * @param method           the method used to determine the closest color
	 *
	 * @return the palette index
	 */
	byte findBestColor(byte r, byte g, byte b, ColorDistanceMethod method = kColorDistanceRedmean);

	/**
	 * @brief This method creates a map from the given palette
	 *        that can be used by crossBlitMap().
	 *
	 * @param palette   the palette data, in interleaved RGB format
	 * @param len       the number of palette entries to be read
	 * @param method    the method used to determine the closest color
	 *
	 * @return the created map, or nullptr if one isn't needed.
	 */
	uint32 *createMap(const byte *srcPalette, uint len, ColorDistanceMethod method = kColorDistanceRedmean);

private:
	Palette _palette;
	uint _paletteSize;
	Common::HashMap<int, byte> _colorHash;
};

} //  // end of namespace Graphics
#endif
