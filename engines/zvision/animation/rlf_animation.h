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

#ifndef ZVISION_RLF_ANIMATION_H
#define ZVISION_RLF_ANIMATION_H

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

private:
	Common::File _file;
	bool _stream;
	uint _lastFrameRead;

	uint _frameCount;
	uint _width;
	uint _height;
	uint32 _frameTime; // In milliseconds
	Frame *_frames;
	Common::Array<uint> _completeFrames;

	int _currentFrame;
	Graphics::Surface _currentFrameBuffer;
	uint32 _frameBufferByteSize;

public:
	uint frameCount() { return _frameCount; }
	uint width() { return _width; }
	uint height() { return _height; }
	uint32 frameTime() { return _frameTime; }

	/**
	 * Seeks to the frameNumber and updates the internal Surface with
	 * the new frame data. If frameNumber == -1, it only sets _currentFrame,
	 * the internal Surface is unchanged. This function requires _stream = false
	 *
	 * @param frameNumber    The frame number to seek to
	 */
	void seekToFrame(int frameNumber);

	/**
	 * Returns the pixel data of the frame specified. It will try to use
	 * getNextFrame() if possible. If not, it uses seekToFrame() to
	 * update the internal Surface and then returns a pointer to it.
	 * This function requires _stream = false
	 *
	 * @param frameNumber    The frame number to get data for
	 * @return               A pointer to the pixel data. Do NOT delete this.
	 */
	const Graphics::Surface *getFrameData(uint frameNumber);
	/**
	 * Returns the pixel data of the next frame. It is up to the user to
	 * check if the next frame is valid before calling this.
	 * IE. Use endOfAnimation()
	 *
	 * @return    A pointer to the pixel data. Do NOT delete this.
	 */
	const Graphics::Surface *getNextFrame();

	/**
	 * @return Is the currentFrame is the last frame in the animation?
	 */
	bool endOfAnimation() { return _currentFrame == (int)_frameCount - 1; }

private:
	/**
	 * Reads in the header of the RLF file
	 *
	 * @return    Will return false if the header magic number is wrong
	 */
	bool readHeader();
	/**
	 * Reads the next frame from the RLF file, stores the data in
	 * a Frame object, then returns the object
	 *
	 * @return    A Frame object representing the frame data
	 */
	Frame readNextFrame();

	/**
	 * Applies the frame corresponding to frameNumber on top of _currentFrameBuffer.
	 * This function requires _stream = false so it can look up the Frame object
	 * referenced by frameNumber.
	 *
	 * @param frameNumber    The frame number to apply to _currentFrameBuffer
	 */
	void applyFrameToCurrent(uint frameNumber);
	/**
	 * Applies the data from a Frame object on top of a _currentFrameBuffer.
	 *
	 * @param frame    A Frame object to apply to _currentFrameBuffer
	 */
	void applyFrameToCurrent(const RlfAnimation::Frame &frame);

	/**
	 * Decode frame data that uses masked run length encoding. This is the encoding
	 * used by P-frames.
	 *
	 * @param source        The source pixel data
	 * @param dest          The destination buffer
	 * @param sourceSize    The size of the source pixel data
	 * @param destSize      The size of the destination buffer
	 */
	void decodeMaskedRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const;
	/**
	 * Decode frame data that uses simple run length encoding. This is the encoding
	 * used by I-frames.
	 *
	 * @param source        The source pixel data
	 * @param dest          The destination buffer
	 * @param sourceSize    The size of the source pixel data
	 * @param destSize      The size of the destination buffer
	 */
	void decodeSimpleRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const;
};

} // End of namespace ZVision

#endif
