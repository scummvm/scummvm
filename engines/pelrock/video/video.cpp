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
#include "graphics/paletteman.h"

#include "pelrock/chrono.h"
#include "pelrock/pelrock.h"
#include "pelrock/video/video.h"
#include "video.h"

namespace Pelrock {

VideoManager::VideoManager(Graphics::Screen *screen, PelrockEventManager *events, ChronoManager *chrono) : _screen(screen), _events(events), _chrono(chrono) {
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
	videoFile.seek(0, SEEK_SET);

	memset(_currentKeyFrame, 0, 256000);
	for (int sequence = 0; sequence < 1; sequence++) {
		int frameCounter = 0;
		int chunksInBuffer = 0;
		bool videoExitFlag = false;

		while (!videoExitFlag && !g_engine->shouldQuit()) {
			_chrono->updateChrono();
			_events->pollEvent();

			if(_chrono->_gameTick) {
				ChunkHeader chunk;
				readChunk(videoFile, chunk);

				switch (chunk.chunkType) {
				case 1:
				case 2:
					processFrame(chunk, frameCounter++);
					break;
				case 3:
					videoExitFlag = true;
					break;
				case 4:
					loadPalette(chunk);
					break;
				default:
					debug("Unknown chunk type %d encountered", chunk.chunkType);
					break;
				}
				presentFrame();
			}
			g_system->delayMillis(10);
		}
	}

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

void VideoManager::loadPalette(ChunkHeader &chunk) {
	byte palette[768];
	for (int i = 0; i < 256; i++) {
		palette[i * 3 + 0] = chunk.data[i * 3 + 0] << 2;
		palette[i * 3 + 1] = chunk.data[i * 3 + 1] << 2;
		palette[i * 3 + 2] = chunk.data[i * 3 + 2] << 2;
	}
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
}

byte *VideoManager::decodeCopyBlock(byte *data, uint32 offset) {

	byte *buf = new byte[256000];
	memset(buf, 0, 256000);
	uint32 pos = offset + 0x04;
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

	return buf;
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

void VideoManager::readChunk(Common::SeekableReadStream &stream, ChunkHeader &chunk) {
	chunk.blockCount = stream.readUint32LE();
	chunk.dataOffset = stream.readUint32LE();
	chunk.chunkType = stream.readByte();

	chunk.data = new byte[chunk.blockCount * chunkSize + 9];
	stream.read(chunk.data, chunk.blockCount * chunkSize - 9);
}

void VideoManager::processFrame(ChunkHeader &chunk, const int frameCount) {
	byte *frameData = nullptr;
	if(chunk.chunkType == 1) {
		// Video data chunk
		frameData = decodeRLE(chunk.data, chunk.blockCount * chunkSize, 0x04);
	} else if (chunk.chunkType == 2) {
		// Block copy chunk
		frameData = decodeCopyBlock(chunk.data, 0);
	}

	if(frameCount == 0) {
		memcpy(_currentKeyFrame, frameData, 256000);
	} else {
		// Subsequent frames, XOR with previous frame
		for (int i = 0; i < 256000; i++) {
			_currentKeyFrame[i] ^= frameData[i];
		}
	}
	delete[] frameData;

}

void VideoManager::presentFrame() {
	memcpy(_screen->getPixels(), _currentKeyFrame, 640 * 400);
	_screen->markAllDirty();
	_screen->update();
}

} // End of namespace Pelrock
