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

#include "audio/audiostream.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/palette.h"
#include "video/video_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
struct Surface;
} // namespace Graphics

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
	void close() override;

	virtual bool loadStream(Common::SeekableReadStream *stream) override;

	const Common::List<Common::Rect> *getDirtyRects() const;
	void clearDirtyRects();
	void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);
	const byte *getPalette();
	virtual void readNextPacket() override;


protected:
	class PacoVideoTrack : public FixedRateVideoTrack {
	public:
		PacoVideoTrack(
			uint16 frameRate, uint16 frameCount,  uint16 width, uint16 height);
		~PacoVideoTrack();

		bool endOfTrack() const override;
		virtual bool isRewindable() const override { return false; }

		uint16 getWidth() const override;
		uint16 getHeight() const override;
		Graphics::PixelFormat getPixelFormat() const override;
		int getCurFrame() const override { return _curFrame; }
		int getFrameCount() const override { return _frameCount; }
		virtual const Graphics::Surface *decodeNextFrame() override;
		virtual void handleFrame(Common::SeekableReadStream *fileStream, uint32 chunkSize, int curFrame);
		void handlePalette(Common::SeekableReadStream *fileStream);
		const byte *getPalette() const override;
		bool hasDirtyPalette() const override { return _dirtyPalette; }

		const Common::List<Common::Rect> *getDirtyRects() const { return &_dirtyRects; }
		void clearDirtyRects() { _dirtyRects.clear(); }
		void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);
		Common::Rational getFrameRate() const override { return Common::Rational(_frameRate, 1); }

	protected:
		Graphics::Surface *_surface;
		Graphics::Palette _palette;

		mutable bool _dirtyPalette;

		int _curFrame;
		uint32 _frameCount;
		uint16 _frameRate;

		Common::List<Common::Rect> _dirtyRects;
	};

	class PacoAudioTrack : public AudioTrack {
	public:
		PacoAudioTrack(int samplingRate);
		~PacoAudioTrack();
		void queueSound(Common::SeekableReadStream *fileStream, uint32 chunkSize);

	protected:
		Audio::AudioStream *getAudioStream() const { return _packetStream; }

	private:
		Audio::PacketizedAudioStream *_packetStream;
		int _samplingRate;
	};

private:
	PacoVideoTrack *_videoTrack;
	PacoAudioTrack *_audioTrack;
	Common::SeekableReadStream *_fileStream;
	int _curFrame = 0;
	int _frameSizes[65536]; // can be done differently?
	int getAudioSamplingRate();
};

} // End of namespace Video

#endif
