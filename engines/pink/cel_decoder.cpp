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

#include "common/stream.h"

#include "graphics/surface.h"

#include "pink/cel_decoder.h"

namespace Pink {

bool CelDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	/* uint32 frameSize = */ stream->readUint32LE();
	uint16 frameType = stream->readUint16LE();

	// Check FLC magic number
	if (frameType != 0xAF12) {
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

	addTrack(new CelVideoTrack(stream, frameCount, width, height));
	return true;
}


uint16 CelDecoder::getTransparentColourIndex() const {
	const CelVideoTrack *track = (const CelVideoTrack *)getTrack(0);
	if (!track)
		return 0;
	return track->getTransparentColourIndex();
}

const Graphics::Surface *CelDecoder::getCurrentFrame() const {
	const CelVideoTrack *track = (const CelVideoTrack *)getTrack(0);
	if (!track)
		return 0;
	return track->getCurrentFrame();
}

Common::Point CelDecoder::getCenter() const {
	const CelVideoTrack *track = (const CelVideoTrack *)getTrack(0);
	if (!track)
		return Common::Point(0, 0);
	return track->getCenter();
}

void CelDecoder::skipFrame() {
	CelVideoTrack *track = (CelVideoTrack *)getTrack(0);
	track->skipFrame();
}

void CelDecoder::setEndOfTrack() {
	CelVideoTrack *track = (CelVideoTrack *)getTrack(0);
	track->setEndOfTrack();
}

CelDecoder::CelVideoTrack::CelVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height, bool skipHeader)
		: FlicVideoTrack(stream, frameCount, width, height, 1), _center(0, 0), _transparentColourIndex(0) {
	readHeader();
}

#define PREFIX_TYPE 0xF100
#define CEL_DATA 3

void CelDecoder::CelVideoTrack::readPrefixChunk() {
	_fileStream->seek(0x80);
	/* uint32 chunkSize = */_fileStream->readUint32LE();
	uint16 chunkType = _fileStream->readUint16LE();
	if (chunkType != PREFIX_TYPE)
		return;
	//uint32 offset = 6;

	uint32 subchunkSize = _fileStream->readUint32LE();
	uint16 subchunkType = _fileStream->readUint16LE();

	switch (subchunkType) {
	case CEL_DATA:
		_fileStream->readUint16LE();
		_center.x = _fileStream->readUint16LE();
		_center.y = _fileStream->readUint16LE();
		break;
	default:
		error("Unknown subchunk type");
		_fileStream->skip(subchunkSize - 6);
		break;
	}
}

#undef PREFIX_TYPE
#undef CEL_DATA

void CelDecoder::CelVideoTrack::readHeader() {
	_fileStream->readUint16LE();

	_frameDelay = _startFrameDelay = _fileStream->readUint32LE();

	_fileStream->seek(80);
	_offsetFrame1 = _fileStream->readUint32LE();
	_offsetFrame2 = _fileStream->readUint32LE();

	if (_offsetFrame1 > 0x80)
		readPrefixChunk();

	_fileStream->seek(_offsetFrame1);
}

uint16 CelDecoder::CelVideoTrack::getTransparentColourIndex() const {
	return _transparentColourIndex;
}

const Graphics::Surface *CelDecoder::CelVideoTrack::getCurrentFrame() const {
	return _surface;
}

Common::Point CelDecoder::CelVideoTrack::getCenter() const {
	return _center;
}

#define FRAME_TYPE 0xF1FA

void CelDecoder::CelVideoTrack::skipFrame() {
	// Read chunk
	/*uint32 frameSize = */ _fileStream->readUint32LE();
	uint16 frameType = _fileStream->readUint16LE();

	switch (frameType) {
	case FRAME_TYPE:
		handleFrame();
		break;
	default:
		error("FlicDecoder::decodeFrame(): unknown main chunk type (type = 0x%02X)", frameType);
		break;
	}

	_curFrame++;
	//_nextFrameStartTime += _frameDelay;

	if (_atRingFrame) {
		// If we decoded the ring frame, seek to the second frame
		_atRingFrame = false;
		_fileStream->seek(_offsetFrame2);
	}

	if (_curFrame == 0)
		_transparentColourIndex = *(byte *)_surface->getBasePtr(0, 0);

}

const Graphics::Surface *CelDecoder::CelVideoTrack::decodeNextFrame() {
	// Read chunk
	/*uint32 frameSize = */ _fileStream->readUint32LE();
	uint16 frameType = _fileStream->readUint16LE();

	switch (frameType) {
	case FRAME_TYPE:
		handleFrame();
		break;
	default:
		error("FlicDecoder::decodeFrame(): unknown main chunk type (type = 0x%02X)", frameType);
		break;
	}

	_curFrame++;
	_nextFrameStartTime += _frameDelay;

	if (_atRingFrame) {
		// If we decoded the ring frame, seek to the second frame
		_atRingFrame = false;
		if (_frameCount == 1) {
			_fileStream->seek(_offsetFrame1);
		} else
			_fileStream->seek(_offsetFrame2);
	}

	if (_curFrame == 0)
		_transparentColourIndex = *(byte *)_surface->getBasePtr(0, 0);

	return _surface;
}

#undef FRAME_TYPE

bool CelDecoder::CelVideoTrack::rewind() {
	// this method is overriden for 2 reasons:
	// 1) bug in Flic rewind(curFrame)
	// 2) I changed behaviour of endOfTrack
	_nextFrameStartTime = 0;

	if (_curFrame >= (int)_frameCount - 1 && _fileStream->pos() < _fileStream->size())
		_atRingFrame = true;
	else
		_fileStream->seek(_offsetFrame1);

	_curFrame = -1;
	_frameDelay = _startFrameDelay;
	return true;
}

} // End of namepsace Pink
