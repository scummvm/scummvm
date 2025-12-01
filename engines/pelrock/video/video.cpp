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

#include "common/file.h"
#include "graphics/screen.h"

#include "pelrock/chrono.h"
#include "pelrock/pelrock.h"
#include "pelrock/video/video.h"
#include "video.h"

namespace Pelrock {

VideoManager::VideoManager(Graphics::Screen *screen) : _screen(screen) {
}

VideoManager::~VideoManager() {
}

void VideoManager::playIntro() {
	Common::File videoFile;
	if (!videoFile.open("ESCENAX.SSN")) {
		error("Could not open ESCENAX.SSN");
		return;
	}
	loadPalette(videoFile);
	videoFile.seek(frame0offset, SEEK_SET);
	size_t firstChunkSize = chunkSize * 13;
	byte *chunk0 = new byte[firstChunkSize];
	videoFile.read(chunk0, firstChunkSize);
	byte *background = decodeCopyBlock(chunk0, 0);

	for (int i = 0; i < 640; i++) {
		for (int j = 0; j < 400; j++) {
			_screen->setPixel(i, j, background[j * 640 + i]);
		}
	}
	_screen->markAllDirty();
	_screen->update();
	delete[] chunk0;
	g_engine->_chronoManager->waitForKey();

	size_t chunk1Size = chunkSize * 6;
	byte chunk1_data[chunk1Size];
	videoFile.seek(frame1offset, SEEK_SET);
	videoFile.read(chunk1_data, chunk1Size);

	byte *delta1 = decodeRLE(chunk1_data, chunk1Size, 0x0D);
	for (int j = 0; j < 256000; j++) {
		background[j] ^= delta1[j];
	}
	delete[] delta1;
	for (int x = 0; x < 640; x++) {
		for (int y = 0; y < 400; y++) {
			_screen->setPixel(x, y, background[y * 640 + x]);
		}
	}
	_screen->markAllDirty();
	_screen->update();
	g_engine->_chronoManager->waitForKey();

	for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
		byte *chunk = new byte[chunkSize];
		videoFile.seek(offsets[i], SEEK_SET);
		videoFile.read(chunk, chunkSize);
		byte *delta = decodeCopyBlock(chunk, 0);
		for (int j = 0; j < 256000; j++) {
			background[j] ^= delta[j];
		}
		delete[] delta;
		for (int x = 0; x < 640; x++) {
			for (int y = 0; y < 400; y++) {
				_screen->setPixel(x, y, background[y * 640 + x]);
			}
		}
		delete[] chunk;
		_screen->markAllDirty();
		_screen->update();
		g_engine->_chronoManager->waitForKey();
	}
	g_engine->_chronoManager->waitForKey();
	videoFile.close();
}
void VideoManager::loadPalette(Common::SeekableReadStream &stream) {

	byte paletteData[768];
	stream.seek(0x0009, SEEK_SET);
	stream.read(paletteData, 768);
	byte palette[768];
	for (int i = 0; i < 256; i++) {
		palette[i * 3 + 0] = paletteData[i * 3 + 0] << 2;
		palette[i * 3 + 1] = paletteData[i * 3 + 1] << 2;
		palette[i * 3 + 2] = paletteData[i * 3 + 2] << 2;
	}
	_screen->setPalette(palette);
}

byte *VideoManager::decodeCopyBlock(byte *data, uint32 offset) {

	byte *buf = new byte[256000];
	memset(buf, 0, 256000);
	uint32 pos = offset + 0x0D;
	// frames are encoded so that each block copy has a 5-byte header
	// the first 3 bytes are the offset within the screen to which to
	// copy the bytes. The 5th byte is the length of the block to copy.
	while (true) {
		byte dest_lo = data[pos];
		byte dest_mid = data[pos + 1];
		byte dest_hi = data[pos + 2];
		byte length = data[pos + 4];
		if (length == 0) {
			break;
		}
		uint32 dest_offset = dest_lo | (dest_mid << 8) | (dest_hi << 16);

		if (dest_offset + length > 256000) {
			break;
		}
		pos += 5;
		Common::copy(data + pos, data + pos + length, buf + dest_offset);
		pos += length;
	}

	return buf; // Placeholder
}

byte *VideoManager::decodeRLE(byte *data, size_t size, uint32 offset) {
	byte *buf = new byte[256000];
	memset(buf, 0, 256000);
	uint32 pos = offset;
	uint32 outPos = 0;
	while (outPos < 256000 && pos < size) {
		byte countByte = data[pos];
		pos += 1;

		if ((countByte & 0xC0) == 0xC0) {
			// RLE: count in lower 6 bits, next byte is value
			uint32 count = countByte & 0x3F;
			if (pos >= size) {
				break;
			}
			byte value = data[pos];
			pos += 1;
			for (uint32 i = 0; i < count && outPos < 256000; i++) {
				buf[outPos++] = value;
			}
		} else {
			// Literal: count is 1, this byte is the value
			buf[outPos++] = countByte;
		}
	}
	return buf;
}
} // End of namespace Pelrock
