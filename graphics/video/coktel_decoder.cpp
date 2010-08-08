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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "graphics/video/coktel_decoder.h"

#ifdef GRAPHICS_VIDEO_COKTELDECODER_H

namespace Graphics {

CoktelDecoder::State::State() : left(0), top(0), right(0), bottom(0), flags(0), speechId(0) {
}


CoktelDecoder::CoktelDecoder(Audio::Mixer &mixer, Audio::Mixer::SoundType soundType) :
	_mixer(&mixer), _soundType(soundType), _width(0), _height(0), _frameCount(0), _paletteDirty(false) {

	memset(_palette, 0, 768);
}

CoktelDecoder::~CoktelDecoder() {
}

uint16 CoktelDecoder::getWidth() const {
	return _width;
}

uint16 CoktelDecoder::getHeight() const {
	return _height;
}

uint32 CoktelDecoder::getFrameCount() const {
	return _frameCount;
}

byte *CoktelDecoder::getPalette() {
	return _palette;
}

bool CoktelDecoder::hasDirtyPalette() const {
	return _paletteDirty;
}

Common::Rational CoktelDecoder::getFrameRate() const {
	return _frameRate;
}


PreIMDDecoder::PreIMDDecoder(uint16 width, uint16 height,
	Audio::Mixer &mixer, Audio::Mixer::SoundType soundType) : CoktelDecoder(mixer, soundType),
	_stream(0), _videoBuffer(0), _videoBufferSize(0) {

	_width  = width;
	_height = height;
}

PreIMDDecoder::~PreIMDDecoder() {
	close();
}

bool PreIMDDecoder::seek(int32 frame, int whence, bool restart) {
	if (!isVideoLoaded())
		// Nothing to do
		return false;

	// Find the frame to which to seek
	if      (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _frameCount - frame - 1;
	else if (whence == SEEK_SET)
		frame--;
	else
		return false;

	if ((frame < -1) || (((uint32) frame) >= _frameCount))
		// Out of range
		return false;

	if (frame == _curFrame)
		// Nothing to do
		return true;

	// Run through the frames
	_curFrame = -1;
	_stream->seek(2);
	while (_curFrame != frame) {
		uint16 frameSize = _stream->readUint16LE();

		_stream->skip(frameSize + 2);

		_curFrame++;
	}

	return true;
}

bool PreIMDDecoder::load(Common::SeekableReadStream &stream) {
	// Since PreIMDs don't have any width and height values stored,
	// we need them to be specified in the constructor
	assert((_width > 0) && (_height > 0));

	close();

	_stream = &stream;

	_stream->seek(0);

	_frameCount = _stream->readUint16LE();

	_surface.create(_width, _height, 1);

	_videoBufferSize = _width * _height;
	_videoBuffer     = new byte[_videoBufferSize];

	memset(_videoBuffer, 0, _videoBufferSize);

	return true;
}

void PreIMDDecoder::close() {
	reset();

	_surface.free();

	delete _stream;

	delete[] _videoBuffer;

	_stream = 0;

	_videoBuffer     = 0;
	_videoBufferSize = 0;
}

bool PreIMDDecoder::isVideoLoaded() const {
	return _stream != 0;
}

Surface *PreIMDDecoder::decodeNextFrame() {
	if (!isVideoLoaded() || endOfVideo())
		return 0;

	processFrame();
	renderFrame();

	_curFrame++;

	return &_surface;
}

void PreIMDDecoder::processFrame() {
	uint16 frameSize = _stream->readUint16LE();

	uint32 nextFramePos = _stream->pos() + frameSize + 2;

	byte cmd;

	cmd = _stream->readByte();
	frameSize--;

	if (cmd == 0) {
		// Palette. Ignored by Fascination, though

		_stream->skip(768);

		frameSize -= 769;

		cmd = _stream->readByte();
	}

	if (cmd != 2) {
		// Partial frame data

		uint32 fSize   = frameSize;
		uint32 vidSize = _videoBufferSize;

		byte *vidBuffer = _videoBuffer;

		while ((fSize > 0) && (vidSize > 0)) {
			uint32 n = _stream->readByte();
			fSize--;

			if ((n & 0x80) != 0) {
				// Data

				n = MIN<uint32>((n & 0x7F) + 1, MIN(fSize, vidSize));

				_stream->read(vidBuffer, n);

				vidBuffer += n;
				vidSize   -= n;
				fSize     -= n;

			} else {
				// Skip

				n = MIN<uint32>(n + 1, vidSize);

				vidBuffer += n;
				vidSize   -= n;
			}
		}

	} else {
		// Full direct frame

		uint32 vidSize = MIN<uint32>(_videoBufferSize, frameSize);

		_stream->read(_videoBuffer, vidSize);
	}

	_stream->seek(nextFramePos);
}

void PreIMDDecoder::renderFrame() {
	uint16 w = MIN<uint16>(_surface.w, _width);
	uint16 h = MIN<uint16>(_surface.h, _height);

	const byte *src = _videoBuffer;
	      byte *dst = (byte *) _surface.pixels; // + x/y

	uint32 frameDataSize = _videoBufferSize;

	while (h-- > 0) {
		uint32 n = MIN<uint32>(w, frameDataSize);

		memcpy(dst, src, n);

		src += _width;
		dst += _surface.pitch;

		frameDataSize -= n;
	}
}

PixelFormat PreIMDDecoder::getPixelFormat() const {
	return PixelFormat::createFormatCLUT8();
}

} // End of namespace Graphics

#endif // GRAPHICS_VIDEO_COKTELDECODER_H
