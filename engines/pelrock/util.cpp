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
#include "common/stream.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"

#include "pelrock/pelrock.h"
#include "pelrock/types.h"
#include "pelrock/util.h"
#include "util.h"

namespace Pelrock {

void drawRect(Graphics::ManagedSurface *surface, int x, int y, int w, int h, byte color) {
	surface->drawLine(x, y, x + w, y, color);
	surface->drawLine(x, y + h, x + w, y + h, color);
	surface->drawLine(x, y, x, y + h, color);
	surface->drawLine(x + w, y, x + w, y + h, color);
}

void drawText(Graphics::Font *font, Common::String text, int x, int y, int w, byte color) {
	Common::Rect rect = font->getBoundingBox(text.c_str());
	if (x + rect.width() > 640) {
		x = 640 - rect.width() - 2;
	}
	if (y + rect.height() > 400) {
		y = 400 - rect.height();
	}
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
	// Draw main text on top
	font->drawString(g_engine->_screen, text.c_str(), x, y, w, color, Graphics::kTextAlignCenter);
}

void drawText(Graphics::ManagedSurface &dest, Graphics::Font *font, Common::String text, int x, int y, int w, byte color, Graphics::TextAlign align) {
	Common::Rect rect = font->getBoundingBox(text.c_str());
	int bboxW = rect.width();
	int bboxH = rect.height();

	Graphics::Surface surface;
	surface.create(bboxW, bboxH, Graphics::PixelFormat::createFormatCLUT8());
	surface.fillRect(Common::Rect(0, 0, bboxW, bboxH), 255);
	if (x + bboxW > 640) {
		x = 640 - bboxW - 2;
	}
	if (y + bboxH > 400) {
		y = 400 - bboxH - 2;
	}
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}

	font->drawString(&surface, text.c_str(), 0, 0, bboxW, color, align);
	dest.transBlitFrom(surface, Common::Point(x, y), 255);
	surface.free();
}

size_t rleDecompress(
	const byte *input,
	size_t inputSize,
	uint32 offset,
	uint32 expectedSize,
	byte **out_data,
	bool untilBuda) {
	// Check for uncompressed markers
	if (inputSize == 0x8000 || inputSize == 0x6800) {
		*out_data = (byte *)malloc(inputSize);
		if (!*out_data)
			return 0;
		memcpy(*out_data, input + offset, inputSize);
		return inputSize;
	}

	// RLE compressed
	size_t bufferCapacity;
	size_t result_size = 0;
	uint32 pos = offset;
	byte last_value = 0;

	if (untilBuda || expectedSize == 0) {
		// Dynamic allocation mode - grow buffer as needed
		bufferCapacity = 4096;
		*out_data = (byte *)malloc(bufferCapacity);
		if (!*out_data)
			return 0;
	} else {
		// Fixed size mode
		bufferCapacity = expectedSize;
		*out_data = (byte *)malloc(bufferCapacity);
		if (!*out_data)
			return 0;
	}

	while (pos + 2 <= inputSize) {
		// Read the RLE pair
		byte count = input[pos];
		byte value = input[pos + 1];
		last_value = value;

		// Write pixels for this pair
		for (int i = 0; i < count; i++) {
			// Grow buffer if needed
			if (result_size >= bufferCapacity) {
				if (untilBuda || expectedSize == 0) {
					bufferCapacity *= 2;
				} else {
					// In fixed mode, we've hit the limit - something is wrong
					// but grow minimally to avoid crash
					// bufferCapacity += 256;
					break;

				}
				byte *newBuf = (byte *)realloc(*out_data, bufferCapacity);
				if (!newBuf) {
					free(*out_data);
					*out_data = nullptr;
					return 0;
				}
				*out_data = newBuf;
			}
			(*out_data)[result_size++] = value;
		}

		// Advance to next pair
		pos += 2;

		// Check for BUDA marker at new position
		if (untilBuda && pos + 4 <= inputSize &&
			input[pos] == 'B' && input[pos + 1] == 'U' &&
			input[pos + 2] == 'D' && input[pos + 3] == 'A') {
			// Game writes one final pixel after BUDA marker
			// Grow buffer if needed
			if (result_size >= bufferCapacity) {
				bufferCapacity++;
				byte *newBuf = (byte *)realloc(*out_data, bufferCapacity);
				if (!newBuf) {
					free(*out_data);
					*out_data = nullptr;
					return 0;
				}
				*out_data = newBuf;
			}
			(*out_data)[result_size++] = last_value;
			break;
		}

		// In fixed size mode, stop when we reach expected size
		if (!untilBuda && expectedSize > 0 && result_size >= expectedSize) {
			break;
		}
	}

	return result_size;
}

void readUntilBuda(Common::SeekableReadStream *stream, uint32 startPos, byte *&buffer, size_t &outSize) {
	const int markerLen = 4;
	size_t bufferSize = 4096;
	size_t pos = 0;

	buffer = (byte *)malloc(bufferSize);
	stream->seek(startPos, SEEK_SET);
	while (!stream->eos()) {
		byte b = stream->readByte();
		if (pos + 1 > bufferSize) {
			bufferSize *= 2;
			buffer = (byte *)realloc(buffer, bufferSize);
		}
		buffer[pos++] = b;

		// Check for marker at the end of buffer
		if (pos >= markerLen &&
			buffer[pos - 4] == 'B' &&
			buffer[pos - 3] == 'U' &&
			buffer[pos - 2] == 'D' &&
			buffer[pos - 1] == 'A') {
			break;
		}
	}
	outSize = pos;
}

void rleDecompressSingleBuda(Common::SeekableReadStream *stream, uint32 startPos, byte *&outBuffer, size_t &outSize){
	byte *buffer = nullptr;
	size_t size = 0;
	readUntilBuda(stream, startPos, buffer, size);
	outSize = rleDecompress(buffer, size, 0, 0, &outBuffer, true);
	free(buffer);
}

// ManagedSurface overload: wraps sprite data in a Surface and uses transBlitFrom
void drawSpriteToBuffer(Graphics::ManagedSurface &dest, byte *sprite, int x, int y, int width, int height, int transparentColor) {
	Graphics::Surface spriteSurf;
	spriteSurf.init(width, height, width, sprite, Graphics::PixelFormat::createFormatCLUT8());
	dest.transBlitFrom(spriteSurf, Common::Point(x, y), transparentColor);
}

void extractSingleFrame(byte *source, byte *dest, int frameIndex, int frameWidth, int frameHeight) {
	for (int y = 0; y < frameHeight; y++) {
		for (int x = 0; x < frameWidth; x++) {
			unsigned int src_pos = (frameIndex * frameHeight * frameWidth) + (y * frameWidth) + x;
			// debug("Copying pixel from source pos %u to dest pos %d", src_pos, y * frameWidth + x);
			dest[y * frameWidth + x] = source[src_pos];
		}
	}
}

Common::String joinStrings(const Common::Array<Common::String> &strings, const Common::String &separator) {
	Common::String result;
	for (uint i = 0; i < strings.size(); i++) {
		result += strings[i];
		if (i < strings.size() - 1)
			result += separator;
	}
	return result;
}
void drawPos(Graphics::ManagedSurface *surface, int x, int y, byte color) {
	if (x < 640 && y < 400 && x >= 0 && y >= 0) {
		surface->setPixel(x, y, 100);
		surface->drawEllipse(x - 3, y - 3, x + 3, y + 3, color, true);
	}
}

byte decodeChar(byte b) {

	switch (b) {
	case 0x82:
		return special_chars[1];
	case 0x83:
		return special_chars[0];
	case 0x80:
		return special_chars[3]; // n tilde
	case 0x7F:
		return special_chars[4];
	case 0x7E:
		return special_chars[5];
	case 0x7D:
		return special_chars[6];
	case 0x7C:
		return special_chars[7];
	case 0x7B:
		return special_chars[8];
	default:
		return b;
	}
}

void changeGameSpeed(Common::Event e) {
	if (e.type == Common::EVENT_KEYDOWN) {
		if (e.kbd.hasFlags(Common::KBD_CTRL)) {
			if (e.kbd.keycode == Common::KEYCODE_f) {
				g_engine->_chrono->changeSpeed();
			}
		}
	}
}

void drawPaletteSquares(Graphics::ManagedSurface &dest, byte *palette) {
	const int squareSize = 6;
	const int colorsPerRow = 16;
	const int startX = 10;
	const int startY = 10;
	const int spacing = 1;

	for (int colorIndex = 0; colorIndex < 256; colorIndex++) {
		int row = colorIndex / colorsPerRow;
		int col = colorIndex % colorsPerRow;

		int x = startX + col * (squareSize + spacing);
		int y = startY + row * (squareSize + spacing);

		dest.fillRect(Common::Rect(x, y, x + squareSize, y + squareSize), colorIndex);
	}
}

} // End of namespace Pelrock
