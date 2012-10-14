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

#ifndef NANCY_VIDEO_H
#define NANCY_VIDEO_H

#include "graphics/pixelformat.h"
#include "video/video_decoder.h"
#include "common/rational.h"

namespace Common {
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

	bool loadStream(Common::SeekableReadStream *stream);

private:
	class AVFVideoTrack : public FixedRateVideoTrack {
	public:
		AVFVideoTrack(Common::SeekableReadStream *stream);
		~AVFVideoTrack();

		uint16 getWidth() const { return _width; }
		uint16 getHeight() const { return _height; }
		Graphics::PixelFormat getPixelFormat() const { return _pixelFormat; }
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		const Graphics::Surface *decodeNextFrame();

	protected:
		Common::Rational getFrameRate() const { return 20; }

	private:
		struct ChunkInfo {
			uint16 index;
			uint32 offset;
			uint32 compressedSize;
			uint32 size;
			byte type;
		};

		bool decodeFrame(byte *rleData, int rleSize, byte *litData, int litSize, byte *dest, int left, int width, int height, int colorKey);

		Common::SeekableReadStream *_fileStream;
		Graphics::PixelFormat _pixelFormat;
		uint _width, _height, _depth;
		int _curFrame;
		uint _frameCount;
		bool _compressed;
		Graphics::Surface *_surface;
		Common::Array<ChunkInfo> _chunkInfo;
		Decompressor *_dec;
	};
};

} // End of namespace Nancy

#endif
