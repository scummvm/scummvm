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

#ifndef ZVISION_RLF_ANIMATION_H
#define ZVISION_RLF_ANIMATION_H

#include "common/types.h"

#include "common/file.h"
#include "graphics/surface.h"


namespace Common {
class String;
}

namespace ZVision {

class RlfAnimation {
public:
	RlfAnimation(const Common::String &fileName, bool stream = true);
	~RlfAnimation();

private:
	enum EncodingType {
		Masked,
		Simple
	};

	struct Frame {
		EncodingType type;
		int8 *encodedData;
		uint32 encodedSize;
	};

	const static Graphics::PixelFormat _pixelFormat555;
	const static Graphics::PixelFormat _pixelFormat565;

private:
	Common::File _file;
	bool _stream;
	uint _lastFrameRead;

	uint _frameCount;
	uint _width;
	uint _height;
	uint32 _frameTime; // In milliseconds
	Frame *_frames;
	Common::List<uint> _completeFrames;

	int _currentFrame;
	uint16 *_currentFrameBuffer;
	uint32 _frameBufferByteSize;

public:
	uint frameCount() { return _frameCount; }
	uint width() { return _width; }
	uint height() { return _height; }
	uint32 frameTime() { return _frameTime; }

	void seekToFrame(int frameNumber);

	const uint16 *getFrameData(uint frameNumber);
	const uint16 *getNextFrame();

	bool endOfAnimation() { return _currentFrame == (int)_frameCount - 1; }

private:
	bool readHeader();
	Frame readNextFrame();

	void applyFrameToCurrent(uint frameNumber);
	void applyFrameToCurrent(const RlfAnimation::Frame &frame);

	void decodeMaskedRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const;
	void decodeSimpleRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const;
};

} // End of namespace ZVision

#endif
