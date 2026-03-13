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
#ifndef PELROCK_UTIL_H
#define PELROCK_UTIL_H

#include "common/events.h"
#include "common/stream.h"
#include "common/types.h"

#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"

namespace Pelrock {

size_t rleDecompress(const byte *data, size_t data_size, uint32 offset, uint32 size, byte **out_data, bool untilBuda = true);
void readUntilBuda(Common::SeekableReadStream *stream, uint32 startPos, byte *&buffer, size_t &outSize);
void rleDecompressSingleBuda(Common::SeekableReadStream *stream, uint32 startPos, byte *&buffer, size_t &outSize);
void drawSpriteToBuffer(Graphics::ManagedSurface &dest, byte *sprite, int x, int y, int width, int height, int transparentColor);
void extractSingleFrame(byte *source, byte *dest, int frameIndex, int frameWidth, int frameHeight);

void drawText(Graphics::ManagedSurface &dest, Graphics::Font *font, Common::String text, int x, int y, int w, byte color, Graphics::TextAlign align = Graphics::kTextAlignLeft);
void drawText(Graphics::Font *font, Common::String text, int x, int y, int w, byte color);
Common::String joinStrings(const Common::Array<Common::String> &strings, const Common::String &separator);
byte decodeChar(byte b);
void changeGameSpeed(Common::Event e);

// debug functions
void drawRect(Graphics::ManagedSurface *surface, int x, int y, int w, int h, byte color);
void drawPos(Graphics::ManagedSurface *surface, int x, int y, byte color);
void drawPaletteSquares(Graphics::ManagedSurface &dest, byte *palette);

static const int special_chars[] = {
	168, // inverted ?
	173, // inverted !
	165, // capital N tilde
	164, // small n tilde
	163, // small u tilde
	162, // small o tilde
	161, // small i tilde
	130, // small e tilde
	160, // small a tilde
};

} // End of namespace Pelrock
#endif // PELROCK_UTIL_H
