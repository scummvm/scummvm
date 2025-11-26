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

#include "pelrock/util.h"
#include "pelrock/types.h"

namespace Pelrock {

void drawRect(Graphics::ManagedSurface *surface, int x, int y, int w, int h, byte color) {
	// debug("Drawing rect at (%d,%d) w=%d h=%d color=%d", x, y, w, h, color);
	surface->drawLine(x, y, x + w, y, color);
	surface->drawLine(x, y + h, x + w, y + h, color);
	surface->drawLine(x, y, x, y + h, color);
	surface->drawLine(x + w, y, x + w, y + h, color);
}

void drawRect(Graphics::Surface *surface, int x, int y, int w, int h, byte color) {
	// debug("Drawing rect at (%d,%d) w=%d h=%d color=%d", x, y, w, h, color);
	surface->drawLine(x, y, x + w, y, color);
	surface->drawLine(x, y + h, x + w, y + h, color);
	surface->drawLine(x, y, x, y + h, color);
	surface->drawLine(x + w, y, x + w, y + h, color);
}

size_t rleDecompress(const uint8_t *data, size_t data_size, uint32_t offset, uint32_t size, uint8_t **out_data) {
	// Check for uncompressed markers
	if (size == 0x8000 || size == 0x6800) {
		*out_data = (uint8_t *)malloc(size);
		memcpy(*out_data, data + offset, size);
		return size;
	}

	// RLE compressed
	*out_data = (uint8_t *)malloc(EXPECTED_SIZE * 2); // Allocate enough space
	size_t result_size = 0;

	uint32_t pos = offset;
	uint32_t end = offset + size;

	while (pos + 2 <= end && pos + 2 <= data_size) {
		// Check for BUDA marker
		if (pos + 4 <= data_size &&
			data[pos] == 'B' && data[pos + 1] == 'U' &&
			data[pos + 2] == 'D' && data[pos + 3] == 'A') {
			break;
		}

		uint8_t count = data[pos];
		uint8_t value = data[pos + 1];

		for (int i = 0; i < count; i++) {
			(*out_data)[result_size++] = value;
		}

		pos += 2;
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
void drawSpriteToBuffer(byte *buffer, int bufferWidth,
						byte *sprite, int x, int y,
						int width, int height,
						int transparentColor) {
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


void extractSingleFrame(byte *source, byte *dest, int frameIndex, int frameWidth, int frameHeight) {
	for (int y = 0; y < frameHeight; y++) {
		for (int x = 0; x < frameWidth; x++) {
			unsigned int src_pos = (frameIndex * frameHeight * frameWidth) + (y * frameWidth) + x;
			dest[y * frameWidth + x] = source[src_pos];
		}
	}
}

} // End of namespace Pelrock
