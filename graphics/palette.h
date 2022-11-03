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

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/noncopyable.h"

/**
 * @defgroup graphics_palette PaletteManager
 * @ingroup graphics
 *
 * @brief The PaletteManager class.
 *
 * @{
 */

/**
 * The PaletteManager is part of the OSystem backend API and responsible
 * for handling the (possibly emulated) "hardware" palette needed for
 * many old games (e.g. in EGA and VGA mode).
 *
 * By itself it is a pure abstract class, i.e. an "interface"; you can
 * use the OSystem::getPaletteManager() method to obtain an instance
 * that you can use to perform actual palette modifications.
 */
class PaletteManager : Common::NonCopyable {
public:
	virtual ~PaletteManager() {}

	/**
	 * Replace the specified range of the palette with new colors.
	 * The palette entries from 'start' till (start+num-1) will be replaced - so
	 * a full palette update is accomplished via start=0, num=256.
	 *
	 * The palette data is specified in interleaved RGB format. That is, the
	 * first byte of the memory block 'colors' points at is the red component
	 * of the first new color; the second byte the green component of the first
	 * new color; the third byte the blue component, the last byte to the alpha
	 * (transparency) value. Then the second color starts, and so on. So memory
	 * looks like this: R1-G1-B1-R2-G2-B2-R3-...
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note It is an error if start+num exceeds 256, behavior is undefined
	 *       in that case (the backend may ignore it silently or assert).
	 * @note It is an error if this function gets called when the pixel format
	 *       in use (the return value of getScreenFormat) has more than one
	 *       byte per pixel.
	 *
	 * @see getScreenFormat
	 */
	virtual void setPalette(const byte *colors, uint start, uint num) = 0;

	/**
	 * Grabs a specified part of the currently active palette.
	 * The format is the same as for setPalette.
	 *
	 * This should return exactly the same RGB data as was setup via previous
	 * setPalette calls.
	 *
	 * For example, for every valid value of start and num of the following
	 * code:
	 *
	 * byte origPal[num*3];
	 * // Setup origPal's data however you like
	 * g_system->setPalette(origPal, start, num);
	 * byte obtainedPal[num*3];
	 * g_system->grabPalette(obtainedPal, start, num);
	 *
	 * the following should be true:
	 *
	 * memcmp(origPal, obtainedPal, num*3) == 0
	 *
	 * @see setPalette
	 * @param colors	the palette data, in interleaved RGB format
	 * @param start		the first platte entry to be read
	 * @param num		the number of palette entries to be read
	 *
	 * @note It is an error if this function gets called when the pixel format
	 *       in use (the return value of getScreenFormat) has more than one
	 *       byte per pixel.
	 *
	 * @see getScreenFormat
	 */
	virtual void grabPalette(byte *colors, uint start, uint num) const = 0;
};
 /** @} */

namespace Graphics {

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
	 * @param useNaiveAlg            if true, use a simpler algorithm (non-floating point calculations)
	 *
	 * @return the palette index
	 */
	byte findBestColor(byte r, byte g, byte b, bool useNaiveAlg = false);

private:
	byte _palette[256 * 3];
	uint _paletteSize;
	Common::HashMap<int, byte> _colorHash;
};

} //  // end of namespace Graphics
#endif
