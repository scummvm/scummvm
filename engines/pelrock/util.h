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

#include "common/stream.h"
#include "common/types.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"

namespace Pelrock {

const int EXPECTED_SIZE = 640 * 400;
size_t rleDecompress(const uint8_t *data, size_t data_size, uint32_t offset, uint32_t size, uint8_t **out_data, bool untilBuda = true);
void readUntilBuda(Common::SeekableReadStream *stream, uint32_t startPos, byte *&buffer, size_t &outSize);
void drawSpriteToBuffer(byte *buffer, int bufferWidth, byte *sprite, int x, int y, int width, int height, int transparentColor);
void blitSurfaceToBuffer(Graphics::Surface *surface, byte *buffer, int bufferWidth, int bufferHeight, int destX, int destY);
void extractSingleFrame(byte *source, byte *dest, int frameIndex, int frameWidth, int frameHeight);
void drawRect(Graphics::ManagedSurface *surface, int x, int y, int w, int h, byte color);
void drawRect(Graphics::Surface *surface, int x, int y, int w, int h, byte color);
void drawRect(byte *screenBuffer, int x, int y, int w, int h, byte color);
void drawText(byte *screenBuffer, Graphics::Font *font, Common::String text, int x, int y, int w, byte color, Graphics::TextAlign align = Graphics::kTextAlignLeft);
void drawText(Graphics::Font *font, Common::String text, int x, int y, int w, byte color);
Common::String joinStrings(const Common::Array<Common::String> &strings, const Common::String &separator);
void drawPos(Graphics::ManagedSurface *surface, int x, int y, byte color);
byte decodeChar(byte b);

static const int special_chars[] = {
	131, // inverted ?
	130, // inverted !
	129, // capital N tilde
	128, // small n tilde
	127, // small u tilde
	126, // small o tilde
	125, // small i tilde
	124, // small e tilde
	123, // small a tilde
};

} // End of namespace Pelrock
#endif // PELROCK_UTIL_H
