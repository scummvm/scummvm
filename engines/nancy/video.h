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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_VIDEO_H
#define NANCY_VIDEO_H

#include "video/video_decoder.h"

namespace Common {
class ReadStream;
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Nancy {

class Decompressor;

class AVFDecoder : public Video::VideoDecoder {
public:
	virtual ~AVFDecoder();

	bool loadStream(Common::SeekableReadStream *stream) override;
	const Graphics::Surface *decodeFrame(uint frameNr);
	void addFrameTime(const uint16 timeToAdd);
	bool atEnd() const;

private:
	class AVFVideoTrack : public FixedRateVideoTrack {
	friend class AVFDecoder;
	public:
		AVFVideoTrack(Common::SeekableReadStream *stream, uint32 chunkFileFormat);
		virtual ~AVFVideoTrack();

		uint16 getWidth() const override { return _width; }
		uint16 getHeight() const override { return _height; }
		Graphics::PixelFormat getPixelFormat() const override { return _pixelFormat; }
		int getCurFrame() const override { return _curFrame; }
		int getFrameCount() const override { return _frameCount; }
		bool isSeekable() const override { return true; }
		bool seek(const Audio::Timestamp &time) override;
		bool setReverse(bool reverse) override;
		bool isReversed() const override { return _reversed; }
		bool endOfTrack() const override;
		const Graphics::Surface *decodeNextFrame() override;
		const Graphics::Surface *decodeFrame(uint frameNr);

	protected:
		Common::Rational getFrameRate() const override { return Common::Rational(1000, _frameTime); }

	private:
		struct ChunkInfo {
			Common::String name;
			uint16 index;
			uint32 offset;
			uint32 compressedSize;
			uint32 size;
			byte type;
		};

		bool decode(byte *outBuf, uint32 frameSize, Common::ReadStream &inBuf) const;

		Common::SeekableReadStream *_fileStream;
		Graphics::PixelFormat _pixelFormat;
		uint _width, _height, _depth, _frameSize;
		int _curFrame;
		uint _frameCount;
		uint32 _frameTime;
		Graphics::Surface *_surface;
		int _refFrame;
		Common::Array<ChunkInfo> _chunkInfo;
		Decompressor *_dec;
		bool _reversed;
	};
};

} // End of namespace Nancy

#endif // NANCY_VIDEO_H
