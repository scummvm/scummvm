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

#ifndef ZVISION_RLF_DECODER_H
#define ZVISION_RLF_DECODER_H

#include "common/file.h"
#include "video/video_decoder.h"

#include "graphics/surface.h"

namespace ZVision {

class RLFDecoder : public Video::VideoDecoder {
public:
	RLFDecoder() {}
	~RLFDecoder() override;

	bool loadStream(Common::SeekableReadStream *stream) override;

private:
	class RLFVideoTrack : public FixedRateVideoTrack {
	public:
		RLFVideoTrack(Common::SeekableReadStream *stream);
		~RLFVideoTrack() override;

		uint16 getWidth() const override { return _width; }
		uint16 getHeight() const override { return _height; }
		Graphics::PixelFormat getPixelFormat() const override { return Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0); /* RGB 555 */ }
		int getCurFrame() const override { return _displayedFrame; }
		int getFrameCount() const override { return _frameCount; }
		const Graphics::Surface *decodeNextFrame() override;
		bool isSeekable() const override { return true; }
		bool seek(const Audio::Timestamp &time) override;

	protected:
		Common::Rational getFrameRate() const override { return Common::Rational(1000, _frameTime); }

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

		uint _lastFrameRead;

		uint _frameCount;
		uint _width;
		uint _height;
		uint32 _frameTime; // In milliseconds
		Frame *_frames;
		Common::Array<uint> _completeFrames;

		int _displayedFrame;
		Graphics::Surface _currentFrameBuffer;
		uint32 _frameBufferByteSize;

		Common::SeekableReadStream *_readStream;
	};	// RLFVideoTrack
};

} // End of namespace ZVision

#endif
