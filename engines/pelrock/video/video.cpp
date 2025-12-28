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
#include "graphics/paletteman.h"
#include "graphics/screen.h"

#include "pelrock/chrono.h"
#include "pelrock/pelrock.h"
#include "pelrock/video/video.h"
#include "video.h"

namespace Pelrock {

VideoManager::VideoManager(
	Graphics::Screen *screen,
	PelrockEventManager *events,
	ChronoManager *chrono, LargeFont *largeFont, DialogManager *dialog) : _screen(screen), _events(events), _chrono(chrono), _largeFont(largeFont), _dialog(dialog) {
	_videoSurface.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	_textSurface.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
}

VideoManager::~VideoManager() {
	_videoSurface.free();
}

void VideoManager::playIntro() {
	initMetadata();
	Common::File videoFile;
	if (!videoFile.open("ESCENAX.SSN")) {
		error("Could not open ESCENAX.SSN");
		return;
	}
	videoFile.seek(0, SEEK_SET);

	_videoSurface.fillRect(Common::Rect(0, 0, 640, 400), 0);
	_textSurface.fillRect(Common::Rect(0, 0, 640, 400), 255);
	for (int sequence = 0; sequence < 1; sequence++) {
		int frameCounter = 0;
		int chunksInBuffer = 0;
		bool videoExitFlag = false;

		while (!videoExitFlag && !g_engine->shouldQuit()) {
			_chrono->updateChrono();
			_events->pollEvent();

			if (_chrono->_gameTick && _chrono->getFrameCount() % 2 == 0) {
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


				Subtitle *subtitle = getSubtitleForFrame(frameCounter);
				if (subtitle != nullptr) {
					Common::StringArray lines;
					lines.push_back(subtitle->text);
					byte color;
					_dialog->processColorAndTrim(lines, color);
					debug("Displaying subtitle: %s with color %d", subtitle->text.c_str(), color);
					_largeFont->drawString(&_textSurface, subtitle->text, subtitle->x, subtitle->y, 640, color);
				}

				presentFrame();
			}
			g_system->delayMillis(10);
		}
		debug("Total frames played: %d", frameCounter);
	}
	videoFile.close();
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
	if (chunk.chunkType == 1) {
		// Video data chunk
		frameData = decodeRLE(chunk.data, chunk.blockCount * chunkSize, 0x04);
	} else if (chunk.chunkType == 2) {
		// Block copy chunk
		frameData = decodeCopyBlock(chunk.data, 0);
	}

	byte *surfacePixels = (byte *)_videoSurface.getPixels();
	if (frameCount == 0) {
		memcpy(surfacePixels, frameData, 256000);
	} else {
		// Subsequent frames, XOR with previous frame
		for (int i = 0; i < 256000; i++) {
			surfacePixels[i] ^= frameData[i];
		}
	}
	delete[] frameData;
}

void VideoManager::presentFrame() {
	_screen->blitFrom(_videoSurface);
	_screen->transBlitFrom(_textSurface, 255);
	_screen->markAllDirty();
	_screen->update();
}

void VideoManager::initMetadata() {
	Common::File metadataFile;
	if (!metadataFile.open("ESCENAX.SCR")) {
		error("Could not open ESCENAX.SCR");
		return;
	}

	while (metadataFile.eos() == false) {
		char curChar = metadataFile.readByte();
		if (curChar == '/') {
			char nextChar = metadataFile.readByte();
			if (nextChar == 't') { // subtitle
				Subtitle subtitle;
				Common::String buffer;
				int values[4];
				int valueIndex = 0;

				// Skip spaces after "/t"
				while (!metadataFile.eos() && metadataFile.readByte() == ' ')
					;
				metadataFile.seek(-1, SEEK_CUR); // Step back one byte

				// Parse 4 space-delimited numbers
				while (!metadataFile.eos() && valueIndex < 4) {
					char c = metadataFile.readByte();

					if (c == ' ') {
						if (!buffer.empty()) {
							values[valueIndex++] = atoi(buffer.c_str());
							buffer.clear();
						}
					} else if (c >= '0' && c <= '9') {
						buffer += c;
					} else if (c == 0x08) {
						// End of numbers, start of text
						if (!buffer.empty()) {
							values[valueIndex++] = atoi(buffer.c_str());
						}
						break;
					}
				}
				metadataFile.skip(1); // Skip the extra space
				if (valueIndex == 4) {
					subtitle.startFrame = values[0];
					subtitle.endFrame = values[1];
					subtitle.x = values[2];
					subtitle.y = values[3];

					// Read text until CRLF (0x0D 0x0A)
					subtitle.text.clear();
					while (!metadataFile.eos()) {
						char c = metadataFile.readByte();

						if (c == 0x0D) {
							char next = metadataFile.readByte();
							if (next == 0x0A) {
								break;
							} else {
								subtitle.text += c;
								subtitle.text += next;
							}
						} else {
							if(c == 0x08)
								subtitle.text += '@';
							else
								subtitle.text += c;
						}
					}
					_subtitles.push_back(subtitle);
				}
			}
		}
	}

	debug("Loaded %d subtitles", _subtitles.size());
	debug("Loaded %d audio effects", _audioEffect.size());

	metadataFile.close();
}

Subtitle *VideoManager::getSubtitleForFrame(uint16 frameCounter) {
	// Check if current subtitle is still active
	if (_currentSubtitleIndex < _subtitles.size()) {
		Subtitle &sub = _subtitles[_currentSubtitleIndex];

		if (frameCounter >= sub.startFrame && frameCounter <= sub.endFrame) {
			return &sub; // Still showing this subtitle
		}

		if (frameCounter > sub.endFrame) {
			_currentSubtitleIndex++; // Move to next subtitle
			_textSurface.fillRect(Common::Rect(0, 0, 640, 400), 255);
			// Check if new subtitle should be active
			if (_currentSubtitleIndex < _subtitles.size()) {
				Subtitle &nextSub = _subtitles[_currentSubtitleIndex];
				if (frameCounter >= nextSub.startFrame && frameCounter <= nextSub.endFrame) {
					return &nextSub;
				}
			}
		}
	}

	return nullptr; // No active subtitle
}

} // End of namespace Pelrock
