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
 */

#ifndef VIDEO_DXA_DECODER_H
#define VIDEO_DXA_DECODER_H

#include "common/rational.h"
#include "graphics/pixelformat.h"
#include "video/video_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Video {

/**
 * Decoder for DXA videos.
 *
 * Video decoder used in engines:
 *  - agos
 *  - sword1
 *  - sword2
 */
class DXADecoder : public FixedRateVideoDecoder {
public:
	DXADecoder();
	virtual ~DXADecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const { return _fileStream != 0; }
	uint16 getWidth() const { return _width; }
	uint16 getHeight() const { return _height; }
	uint32 getFrameCount() const { return _frameCount; }
	const Graphics::Surface *decodeNextFrame();
	Graphics::PixelFormat getPixelFormat() const { return Graphics::PixelFormat::createFormatCLUT8(); }
	const byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }

	/**
	 * Get the sound chunk tag of the loaded DXA file
	 */
	uint32 getSoundTag() { return _soundTag; }

protected:
	Common::Rational getFrameRate() const { return _frameRate; }

	Common::SeekableReadStream *_fileStream;

private:
	void decodeZlib(byte *data, int size, int totalSize);
	void decode12(int size);
	void decode13(int size);

	enum ScaleMode {
		S_NONE,
		S_INTERLACED,
		S_DOUBLE
	};

	Graphics::Surface *_surface;
	byte _palette[256 * 3];
	bool _dirtyPalette;

	byte *_frameBuffer1;
	byte *_frameBuffer2;
	byte *_scaledBuffer;
	byte *_inBuffer;
	uint32 _inBufferSize;
	byte *_decompBuffer;
	uint32 _decompBufferSize;
	uint16 _curHeight;
	uint32 _frameSize;
	ScaleMode _scaleMode;
	uint32 _soundTag;
	uint16 _width, _height;
	uint32 _frameRate;
	uint32 _frameCount;
};

} // End of namespace Video

#endif
