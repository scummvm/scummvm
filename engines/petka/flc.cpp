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

#include "common/system.h"
#include "common/stream.h"

#include "graphics/surface.h"

#include "petka/flc.h"
#include "flc.h"


namespace Petka {

const Graphics::Surface *FlicDecoder::getCurrentFrame() const {
	const Track *track = getTrack(0);
	if (track)
		return ((const FlicVideoTrack *)track)->getSurface();
	return nullptr;
}

void FlicDecoder::load(Common::SeekableReadStream *stream, Common::SeekableReadStream *mskStream) {
	close();

	/* uint32 frameSize = */ stream->readUint32LE();
	uint16 frameType = stream->readUint16LE();

	// Check FLC magic number
	if (frameType != 0xAF12) {
		warning("FlicDecoder::loadStream(): attempted to load non-FLC data (type = 0x%04X)", frameType);
	}

	uint16 frameCount = stream->readUint16LE();
	uint16 width = stream->readUint16LE();
	uint16 height = stream->readUint16LE();
	uint16 colorDepth = stream->readUint16LE();
	if (colorDepth != 8) {
		warning("FlicDecoder::loadStream(): attempted to load an FLC with a palette of color depth %d. Only 8-bit color palettes are supported", colorDepth);
	}

	FlicVideoTrack *track = new FlicVideoTrack(stream, frameCount, width, height);
	addTrack(track);
	decodeNextFrame();
	assert(track->getPalette());
	if (mskStream)
		track->loadMsk(*mskStream);
	delete mskStream;
}

const Common::Rect &FlicDecoder::getBounds() const {
	const Track *track = getTrack(0);
	if (track)
		return ((const FlicVideoTrack *)track)->getBounds();

	return *(new Common::Rect(0, 0));
}

const Common::Array<Common::Rect> &FlicDecoder::getMskRects() const {
	const Track *track = getTrack(0);
	if (track)
		return ((const FlicVideoTrack *)track)->getMskRects();

	return *(new Common::Array<Common::Rect>());
}

uint32 FlicDecoder::getTransColor(const Graphics::PixelFormat &fmt) const {
	const Track *track = getTrack(0);
	if (track) {
		const FlicVideoTrack *flc = ((const FlicVideoTrack *)track);
		byte r = flc->getPalette()[0];
		byte g = flc->getPalette()[1];
		byte b = flc->getPalette()[2];
		return fmt.RGBToColor(r, g, b);
	}
	return 0;
}

void FlicDecoder::setFrame(int frame) {
	FlicVideoTrack *flc = ((FlicVideoTrack *)getTrack(0));
	if (!flc || flc->getFrameCount() == 1 || flc->getCurFrame() + 1 == frame)
		return;

	if (frame == -1) {
		if (flc->getCurFrame() + 1 == flc->getFrameCount()) {
			flc->rewind();
		}
		flc->decodeNextFrame();
		return;
	}

	flc->rewind();
	do {
		flc->decodeNextFrame();
	} while (flc->getCurFrame() + 1 != frame);
}

uint FlicDecoder::getDelay() const {
	const FlicVideoTrack *flc = ((const FlicVideoTrack *)getTrack(0));
	if (flc)
		return flc->getDelay();
	return 0;
}

FlicDecoder::FlicVideoTrack::FlicVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height, bool skipHeader)
	: Video::FlicDecoder::FlicVideoTrack(stream, frameCount, width, height, skipHeader) {}

bool FlicDecoder::FlicVideoTrack::loadMsk(Common::SeekableReadStream &stream) {
	_msk.resize(_frameCount);
	for (uint i = 0; i < _frameCount; ++i) {
		_msk[i].resize(stream.readUint32LE());
		for (uint j = 0; j < _msk[i].size(); ++j) {
			_msk[i][j].left = stream.readSint16LE();
			_msk[i][j].top = stream.readSint16LE();
			_msk[i][j].right = stream.readSint16LE();
			_msk[i][j].bottom = stream.readSint16LE();
		}
	}
	stream.skip(_frameCount * 4);
	_bounds.left = (int16)stream.readSint32LE();
	_bounds.top = (int16)stream.readSint32LE();
	_bounds.right = (int16)stream.readSint32LE();
	_bounds.bottom = (int16)stream.readSint32LE();

	if (_surface->w <= _bounds.right) {
		_bounds.right = _surface->w - 1;
	}
	if (_surface->h <= _bounds.bottom) {
		_bounds.bottom = _surface->h - 1;
	}

	if (_bounds.left > _bounds.right) {
		SWAP(_bounds.left, _bounds.right);
	}
	if (_bounds.top > _bounds.bottom) {
		SWAP(_bounds.top, _bounds.bottom);
	}

	_bounds = _bounds.findIntersectingRect(Common::Rect(0, 0, _bounds.right, 479));
	_bounds.right++;
	_bounds.bottom++;

	return true;
}

const Common::Rect &FlicDecoder::FlicVideoTrack::getBounds() const {
	return _bounds;
}

const Graphics::Surface *FlicDecoder::FlicVideoTrack::getSurface() const {
	return _surface;
}

const Common::Array<Common::Rect> &FlicDecoder::FlicVideoTrack::getMskRects() const {
	assert(_curFrame >= 0);
	return _msk[_curFrame];
}

uint FlicDecoder::FlicVideoTrack::getDelay() const {
	return _frameDelay;
}

#define FRAME_TYPE            0xF1FA
#define FLC_FILE_HEADER       0xAF12
#define FLC_FILE_HEADER_SIZE  0x80

const Graphics::Surface *FlicDecoder::FlicVideoTrack::decodeNextFrame() {
	// attempt to fix broken flics
	while (true) {
		/*uint32 frameSize = */_fileStream->readUint32LE();
		uint16 frameType = _fileStream->readUint16LE();

		bool processed = true;
		switch (frameType) {
		case FRAME_TYPE:
			handleFrame();
			break;
		case FLC_FILE_HEADER:
			// Skip 0x80 bytes of file header subtracting 6 bytes of header
			_fileStream->skip(FLC_FILE_HEADER_SIZE - 6);
			break;
		default:
			processed = false;
			_fileStream->seek(-5, SEEK_CUR);
			break;
		}
		if (processed)
			break;
	}

	_curFrame++;
	_nextFrameStartTime += _frameDelay;

	if (_atRingFrame) {
		// If we decoded the ring frame, seek to the second frame
		_atRingFrame = false;
		_fileStream->seek(_offsetFrame2);
	}

	return _surface;
}

} // End of namespace Petka
