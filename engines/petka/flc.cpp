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
		return ((FlicVideoTrack *)track)->getBounds();
}

const Common::Array<Common::Rect> &FlicDecoder::getMskRects() const {
	const Track *track = getTrack(0);
	if (track)
		return ((FlicVideoTrack *)track)->getMskRects();
}

uint32 FlicDecoder::getTransColor(const Graphics::PixelFormat &fmt) const {
	const Track *track = getTrack(0);
	if (track) {
		const FlicVideoTrack *flc = ((FlicVideoTrack *) track);
		byte r = flc->getPalette()[0];
		byte g = flc->getPalette()[1];
		byte b = flc->getPalette()[2];
		return fmt.RGBToColor(r, g, b);
	}
	return 0;
}

void FlicDecoder::setFrame(int frame) {
	FlicVideoTrack *flc = ((FlicVideoTrack *) getTrack(0));
	if (!flc || flc->getCurFrame() + 1 == frame)
		return;

	flc->rewind();
	do {
		flc->decodeNextFrame();
	} while (flc->getCurFrame() + 1 != frame);
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

	if (_bounds.left > _bounds.right) {
		int16 t = _bounds.left;
		_bounds.left = _bounds.right;
		_bounds.right = t;
	}
	if (_bounds.top > _bounds.bottom) {
		int16 t = _bounds.top;
		_bounds.top = _bounds.bottom;
		_bounds.bottom = t;
	}
	assert(stream.size() == stream.pos());
	if (getWidth() <= _bounds.right) {
		_bounds.right = getWidth() - 1;
	}
	if (getHeight() <= _bounds.bottom) {
		_bounds.bottom = getHeight() - 1;
	}
	_bounds = _bounds.findIntersectingRect(Common::Rect(0, 0, _bounds.right, 479));
	_bounds.right++;
	_bounds.bottom++;
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

} // End of namespace Petka
