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

/**
 * @defgroup graphics_maccursor Mac cursor
 * @ingroup graphics
 *
 * @brief Macintosh cursor decoding.
 *
 * Used in engines:
 * - mohawk
 * - sci
 * - scumm
 *
 * @{
 */

#ifndef GRAPHICS_MACCURSOR_H
#define GRAPHICS_MACCURSOR_H

#include "common/stream.h"

#include "graphics/cursor.h"
#include "graphics/palette.h"

namespace Graphics {

/**
 * A Mac crsr or CURS cursor
 */
class MacCursor : public Cursor {
public:
	MacCursor();
	~MacCursor();

	/** Return the cursor's width. */
	uint16 getWidth() const { return 16; }
	/** Return the cursor's height. */
	uint16 getHeight() const { return 16; }
	/** Return the cursor's hotspot's x coordinate. */
	uint16 getHotspotX() const { return _hotspotX; }
	/** Return the cursor's hotspot's y coordinate. */
	uint16 getHotspotY() const { return _hotspotY; }
	/** Return the cursor's transparent key. */
	virtual byte getKeyColor() const { return 0xFF; }

	const byte *getSurface() const { return _surface; }

	virtual const byte *getPalette() const { return _palette.data; }
	byte getPaletteStartIndex() const { return 0; }
	uint16 getPaletteCount() const { return _palette.size; }

	/** Read the cursor's data out of a stream. */
	bool readFromStream(Common::SeekableReadStream &stream, bool forceMonochrome = false, byte monochromeInvertedPixelColor = 0xff, bool forceCURSFormat = false);

protected:
	bool readFromCURS(Common::SeekableReadStream &stream, byte monochromeInvertedPixelColor);
	bool readFromCRSR(Common::SeekableReadStream &stream, bool forceMonochrome, byte monochromeInvertedPixelColor);

	byte *_surface;
	Palette _palette;

	uint16 _hotspotX; ///< The cursor's hotspot's x coordinate.
	uint16 _hotspotY; ///< The cursor's hotspot's y coordinate.

	/** Clear the cursor. */
	void clear();
};
 /** @} */
} // End of namespace Graphics

#endif
