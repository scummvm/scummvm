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

#ifndef SCI_VIDEO_SEQ_DECODER_H
#define SCI_VIDEO_SEQ_DECODER_H

#include "common/rational.h"
#include "graphics/pixelformat.h"
#include "video/video_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Sci {

/**
 * Implementation of the Sierra SEQ decoder, used in KQ6 DOS floppy/CD and GK1 DOS
 */
class SEQDecoder : public Video::VideoDecoder {
public:
	SEQDecoder(uint frameDelay);
	~SEQDecoder() override;

	bool loadStream(Common::SeekableReadStream *stream) override;

private:
	class SEQVideoTrack : public FixedRateVideoTrack {
	public:
		SEQVideoTrack(Common::SeekableReadStream *stream, uint frameDelay);
		~SEQVideoTrack() override;

		uint16 getWidth() const override { return SEQ_SCREEN_WIDTH; }
		uint16 getHeight() const override { return SEQ_SCREEN_HEIGHT; }
		Graphics::PixelFormat getPixelFormat() const override { return Graphics::PixelFormat::createFormatCLUT8(); }
		int getCurFrame() const override { return _curFrame; }
		int getFrameCount() const override { return _frameCount; }
		const Graphics::Surface *decodeNextFrame() override;
		const byte *getPalette() const override;
		bool hasDirtyPalette() const override { return _dirtyPalette; }

	protected:
		Common::Rational getFrameRate() const override { return Common::Rational(60, _frameDelay); }

	private:
		enum {
			SEQ_SCREEN_WIDTH = 320,
			SEQ_SCREEN_HEIGHT = 200
		};

		void readPaletteChunk(uint16 chunkSize);
		bool decodeFrame(byte *rleData, int rleSize, byte *litData, int litSize, byte *dest, int left, int width, int height, int colorKey);

		Common::SeekableReadStream *_fileStream;
		int _curFrame, _frameCount;
		byte _palette[256 * 3];
		mutable bool _dirtyPalette;
		Graphics::Surface *_surface;
		uint _frameDelay;
	};

	uint _frameDelay;
};

} // End of namespace Sci

#endif // SCI_VIDEO_SEQ_DECODER_H
