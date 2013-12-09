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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRAPHICS_FONTS_TTF_H
#define GRAPHICS_FONTS_TTF_H

#include "common/scummsys.h"

#ifdef USE_FREETYPE2

#include "common/stream.h"

namespace Graphics {

class Font;

/**
 * Loads a TTF font file from a given data stream object.
 *
 * @param stream     Stream object to load font data from.
 * @param size       The point size to load.
 * @param dpi        The dpi to use for size calculations, by default 72dpi
 *                   are used.
 * @param monochrome Whether the font should be loaded in pure monochrome
 *                   mode. In case this is true no aliasing is used.
 * @param mapping    A mapping from code points 0-255 into UTF-32 code points.
 *                   This can be used to support various 8bit character sets.
 *                   In case the msb of the UTF-32 code point is set the font
 *                   loading fails in case no glyph for it is found. When this
 *                   is non-null only characters given in the mapping are
 *                   supported.
 * @return 0 in case loading fails, otherwise a pointer to the Font object.
 */
Font *loadTTFFont(Common::SeekableReadStream &stream, int size, uint dpi = 0, bool monochrome = false, const uint32 *mapping = 0);

void shutdownTTF();

} // End of namespace Graphics

#endif

#endif

