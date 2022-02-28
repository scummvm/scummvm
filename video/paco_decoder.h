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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VIDEO_PACODECODER_H
#define VIDEO_PACODECODER_H

#include "video/video_decoder.h"
#include "common/list.h"
#include "common/rect.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
struct Surface;
}

namespace Video {

/**
 * Decoder for PACo videos.
 *
 * Video decoder used in engines:
 *  - director
 */
class PacoDecoder : public VideoDecoder {
public:
	PacoDecoder();
	virtual ~PacoDecoder();

	virtual bool loadStream(Common::SeekableReadStream *stream);

	const Common::List<Common::Rect> *getDirtyRects() const;
	void clearDirtyRects();
	void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);
	const byte *getPalette();

protected:

	class PacoVideoTrack : public FixedRateVideoTrack {
	public:
		PacoVideoTrack(
			Common::SeekableReadStream *stream, uint16 frameRate, uint16 frameCount,
			bool hasAudio, uint16 width, uint16 height);
		~PacoVideoTrack();

		bool endOfTrack() const;
		virtual bool isRewindable() const { return false; }

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		virtual const Graphics::Surface *decodeNextFrame();
		virtual void handleFrame(uint32 chunkSize);
		const byte *getPalette() const { return _palette; }
		bool hasDirtyPalette() const { return false; }

		const Common::List<Common::Rect> *getDirtyRects() const { return &_dirtyRects; }
		void clearDirtyRects() { _dirtyRects.clear(); }
		void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);
		Common::Rational getFrameRate() const { return Common::Rational(_frameRate, 1); }


	protected:
		Common::SeekableReadStream *_fileStream;
		Graphics::Surface *_surface;

		int _curFrame;

		byte *_palette;

		int _frameSizes[65536]; // can be done differently?
		mutable bool _dirtyPalette;

		uint32 _frameCount;
		uint32 _frameDelay;
		uint16 _frameRate;

		Common::List<Common::Rect> _dirtyRects;
	};
};

} // End of namespace Video

#endif
