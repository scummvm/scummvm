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
#include "graphics/surface.h"
#include "video/flic_decoder.h"

#include "tot/decoder/TotFlicDecoder.h"

namespace Tot {

#define FLI_SETPAL 4
#define FLI_SS2 7
#define FLI_BLACK 13
#define FLI_BRUN 15
#define FLI_COPY 16
#define PSTAMP 18
#define FLC_FILE_HEADER 0x2420

bool TotFlicDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	/* uint32 frameSize = */ stream->readUint32LE();
	uint16 frameType = stream->readUint16LE();

	// Check FLC magic number
	if (frameType != FLC_FILE_HEADER) {
		warning("FlicDecoder::loadStream(): attempted to load non-FLC data (type = 0x%04X)", frameType);
		return false;
	}

	uint16 frameCount = stream->readUint16LE();
	uint16 width = stream->readUint16LE();
	uint16 height = stream->readUint16LE();
	uint16 colorDepth = stream->readUint16LE();
	if (colorDepth != 8) {
		warning("FlicDecoder::loadStream(): attempted to load an FLC with a palette of color depth %d. Only 8-bit color palettes are supported", colorDepth);
		return false;
	}

	addTrack(new TotVideoTrack(stream, frameCount, width, height));
	return true;
}

TotFlicDecoder::TotVideoTrack::TotVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height) : Video::FlicDecoder::FlicVideoTrack(stream, frameCount, width, height, true) {

	FlicDecoder::FlicVideoTrack::readHeader();
}

TotFlicDecoder::TotVideoTrack::~TotVideoTrack() {
}

void TotFlicDecoder::TotVideoTrack::handleFrame() {
	uint16 chunkCount = _fileStream->readUint16LE();
	// Note: The overridden delay is a 16-bit integer (word), whereas the normal delay is a 32-bit integer (dword)
	// the frame delay is the FLIC "speed", in milliseconds.
	uint16 newFrameDelay = _fileStream->readUint16LE(); // "speed", in milliseconds
	if (newFrameDelay > 0)
		_frameDelay = newFrameDelay;

	_fileStream->readUint16LE(); // reserved, always 0
	uint16 newWidth = _fileStream->readUint16LE();
	uint16 newHeight = _fileStream->readUint16LE();

	if ((newWidth != 0) || (newHeight != 0)) {
		if (newWidth == 0)
			newWidth = _surface->w;
		if (newHeight == 0)
			newHeight = _surface->h;

		_surface->free();
		delete _surface;
		_surface = new Graphics::Surface();
		_surface->create(newWidth, newHeight, Graphics::PixelFormat::createFormatCLUT8());
	}

	// Read subchunks
	for (uint32 i = 0; i < chunkCount; ++i) {
		uint32 frameSize = _fileStream->readUint32LE();
		uint16 frameType = _fileStream->readUint16LE();
		uint8 *data;

		if (frameType == FLI_COPY) {
			// data seems to be in a different format for COPY chunks so we adjust for that
			data = new uint8[frameSize - 4];
			_fileStream->read(data, frameSize - 4);
		} else {
			data = new uint8[frameSize - 6];
			_fileStream->read(data, frameSize - 6);
		}

		switch (frameType) {
		case FLI_SETPAL:
			unpackPalette(data);
			_dirtyPalette = true;
			break;
		case FLI_SS2:
			decodeDeltaFLC(data);
			break;
		case FLI_BLACK:
			_surface->fillRect(Common::Rect(0, 0, getWidth(), getHeight()), 0);
			_dirtyRects.clear();
			_dirtyRects.push_back(Common::Rect(0, 0, getWidth(), getHeight()));
			break;
		case FLI_BRUN:
			decodeByteRun(data);
			break;
		case FLI_COPY:
			copyFrame(data);
			break;
		case PSTAMP:
			/* PSTAMP - skip for now */
			break;
		default:
			error("FlicDecoder::decodeNextFrame(): unknown subchunk type (type = 0x%02X)", frameType);
			break;
		}

		delete[] data;
	}
}

} // End of namespace Tot
