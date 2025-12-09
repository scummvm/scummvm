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

#include "pelrock/types.h"
#include "pelrock/util.h"
#include "util.h"

namespace Pelrock {

void drawRect(Graphics::ManagedSurface *surface, int x, int y, int w, int h, byte color) {
	// debug("Drawing rect at (%d,%d) w=%d h=%d color=%d", x, y, w, h, color);
	surface->drawLine(x, y, x + w, y, color);
	surface->drawLine(x, y + h, x + w, y + h, color);
	surface->drawLine(x, y, x, y + h, color);
	surface->drawLine(x + w, y, x + w, y + h, color);
}

void drawRect(Graphics::Surface *surface, int x, int y, int w, int h, byte color) {
	surface->drawLine(x, y, x + w, y, color);
	surface->drawLine(x, y + h, x + w, y + h, color);
	surface->drawLine(x, y, x, y + h, color);
	surface->drawLine(x + w, y, x + w, y + h, color);
}

void drawRect(byte *screenBuffer, int x, int y, int w, int h, byte color) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	drawRect(surface, 0, 0, w, h, color);

	for (int py = 0; py < h; py++) {
		for (int px = 0; px < w; px++) {
			int destIdx = (y + py) * 640 + (x + px);
			int srcIdx = py * w + px;
			int color  = *((byte *)surface->getBasePtr(px, py));
			if(color != 0)
			screenBuffer[destIdx] = color;
		}
	}
}

Common::String printMovementFlags(uint8_t flags) {
	Common::String result;
	if (flags & MOVE_HORIZ) {
		result += "HORIZ ";
	}
	if (flags & MOVE_VERT) {
		result += "VERT ";
	}
	if (flags & MOVE_DOWN) {
		result += "DOWN ";
	}
	if (flags & MOVE_LEFT) {
		result += "LEFT ";
	}
	if (flags & MOVE_UP) {
		result += "UP ";
	}
	if (flags & MOVE_RIGHT) {
		result += "RIGHT ";
	}
	return result;
}

size_t rleDecompress(
	const uint8_t *input,
	size_t inputSize,
	uint32_t offset,
	uint32_t expectedSize,
	uint8_t **out_data,
	bool untilBuda) {
	// Check for uncompressed markers
	if (inputSize == 0x8000 || inputSize == 0x6800) {
		*out_data = (uint8_t *)malloc(inputSize);
		if (!*out_data)
			return 0;
		memcpy(*out_data, input + offset, inputSize);
		return inputSize;
	}

	// RLE compressed
	size_t bufferCapacity;
	size_t result_size = 0;
	uint32_t pos = offset;
	uint8_t last_value = 0;

	if (untilBuda || expectedSize == 0) {
		// Dynamic allocation mode - grow buffer as needed
		bufferCapacity = 4096;
		*out_data = (uint8_t *)malloc(bufferCapacity);
		if (!*out_data)
			return 0;
	} else {
		// Fixed size mode
		bufferCapacity = expectedSize;
		*out_data = (uint8_t *)malloc(bufferCapacity);
		if (!*out_data)
			return 0;
	}

	while (pos + 2 <= inputSize) {
		// Read the RLE pair
		uint8_t count = input[pos];
		uint8_t value = input[pos + 1];
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
					bufferCapacity += 256;
				}
				uint8_t *newBuf = (uint8_t *)realloc(*out_data, bufferCapacity);
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
				uint8_t *newBuf = (uint8_t *)realloc(*out_data, bufferCapacity);
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

void readUntilBuda(Common::SeekableReadStream *stream, uint32_t startPos, byte *&buffer, size_t &outSize) {
	const char marker[] = "BUDA";
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

// Helper function for transparent blitting
void drawSpriteToBuffer(byte *buffer, int bufferWidth, byte *sprite, int x, int y, int width, int height, int transparentColor) {

	for (int py = 0; py < height; py++) {
		for (int px = 0; px < width; px++) {
			int srcIdx = py * width + px;
			byte pixel = sprite[srcIdx];

			if (pixel != transparentColor) {
				int destX = x + px;
				int destY = y + py;

				if (destX >= 0 && destX < 640 &&
					destY >= 0 && destY < 400) {
					buffer[destY * bufferWidth + destX] = pixel;
				}
			}
		}
	}
}

void blitSurfaceToBuffer(Graphics::Surface *surface, byte *buffer, int bufferWidth, int bufferHeight, int destX, int destY) {
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			int px = destX + x;
			int py = destY + y;
			if (px >= 0 && px < bufferWidth && py >= 0 && py < bufferHeight) {

				byte pixel = *((byte *)surface->getBasePtr(x, y));
				if (pixel != 0) {
					buffer[py * bufferWidth + px] = pixel;
				}
			}
		}
	}
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

char32_t decodeCPByte(byte b) {
	return cp437_to_unicode[b];
}
} // End of namespace Pelrock
