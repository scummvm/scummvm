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

#ifndef VIDEO_HNM_DECODER_H
#define VIDEO_HNM_DECODER_H

#include "common/rational.h"
#include "graphics/pixelformat.h"
#include "video/video_decoder.h"
#include "graphics/surface.h"
#include "audio/audiostream.h"

namespace Common {
class SeekableReadStream;
}

namespace Video {

/**
 * Decoder for HNM videos.
 *
 * Video decoder used in engines:
 *  - cryo
 *  - cryomni3d
 */
class HNMDecoder : public VideoDecoder {
public:
	HNMDecoder(bool loop = false, byte *initialPalette = nullptr);
	~HNMDecoder() override;
	bool loadStream(Common::SeekableReadStream *stream) override;
	void readNextPacket() override;
	void close() override;

	void setRegularFrameDelay(uint32 regularFrameDelay) { _regularFrameDelayMs = regularFrameDelay; }

private:
	class HNMVideoTrack : public VideoTrack {
	public:
		HNMVideoTrack(uint32 frameCount, uint32 regularFrameDelayMs, uint32 audioSampleRate);

		// When _frameCount is 0, it means we are looping
		bool endOfTrack() const override { return (_frameCount == 0) ? false : VideoTrack::endOfTrack(); }
		int getCurFrame() const override { return _curFrame; }
		int getFrameCount() const override { return _frameCount; }
		uint32 getNextFrameStartTime() const override { return _nextFrameStartTime.msecs(); }

		void restart() { _lastFrameDelaySamps = 0; }
		void newFrame(uint32 frameDelay);

		virtual void decodeChunk(byte *data, uint32 size,
		                         uint16 chunkType, uint16 flags) = 0;

	protected:
		uint32 _regularFrameDelayMs;
		uint32 _lastFrameDelaySamps;
		Audio::Timestamp _nextFrameStartTime;

		uint32 _frameCount;
		int _curFrame;
	};

	class HNM45VideoTrack : public HNMVideoTrack {
	public:
		// When _frameCount is 0, it means we are looping
		uint16 getWidth() const override { return _surface.w; }
		uint16 getHeight() const override { return _surface.h; }
		Graphics::PixelFormat getPixelFormat() const override { return _surface.format; }
		const Graphics::Surface *decodeNextFrame() override { return &_surface; }
		const byte *getPalette() const override { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const override { return _dirtyPalette; }

	protected:
		HNM45VideoTrack(uint32 width, uint32 height, uint32 frameSize, uint32 frameCount,
		                uint32 regularFrameDelayMs, uint32 audioSampleRate,
		                const byte *initialPalette = nullptr);
		~HNM45VideoTrack() override;

		/** Decode a video chunk. */
		void decodePalette(byte *data, uint32 size);

		Graphics::Surface _surface;

		byte _palette[256 * 3];
		mutable bool _dirtyPalette;

		byte *_frameBufferC;
		byte *_frameBufferP;
	};

	class HNM4VideoTrack : public HNM45VideoTrack {
	public:
		HNM4VideoTrack(uint32 width, uint32 height, uint32 frameSize, uint32 frameCount,
		               uint32 regularFrameDelayMs, uint32 audioSampleRate,
		               const byte *initialPalette = nullptr);
		~HNM4VideoTrack() override;

		/** Decode a video chunk. */
		void decodeChunk(byte *data, uint32 size,
		                 uint16 chunkType, uint16 flags) override;

	protected:
		/* Really decode */
		void decodeInterframe(byte *data, uint32 size);
		void decodeInterframeA(byte *data, uint32 size);
		void decodeIntraframe(byte *data, uint32 size);
		void presentFrame(uint16 flags);

		byte *_frameBufferF;
	};

	class HNM5VideoTrack : public HNM45VideoTrack {
	public:
		HNM5VideoTrack(uint32 width, uint32 height, uint32 frameSize, uint32 frameCount,
		               uint32 regularFrameDelayMs, uint32 audioSampleRate,
		               const byte *initialPalette = nullptr) :
			HNM45VideoTrack(width, height, frameSize, frameCount, regularFrameDelayMs, audioSampleRate,
			                initialPalette) {}
		/** Decode a video chunk. */
		void decodeChunk(byte *data, uint32 size,
		                 uint16 chunkType, uint16 flags) override;

	protected:
		/** Really decode */
		void decodeFrame(byte *data, uint32 size);
	};

	class DPCMAudioTrack : public AudioTrack {
	public:
		DPCMAudioTrack(uint16 format, uint16 bits, uint sampleRate, bool stereo,
		               Audio::Mixer::SoundType soundType);
		~DPCMAudioTrack() override;

		uint32 decodeSound(byte *data, uint32 size);
	protected:
		Audio::AudioStream *getAudioStream() const override { return _audioStream; }
	private:
		Audio::QueuingAudioStream *_audioStream;
		bool _gotLUT;
		uint16 _lut[256];
		uint16 _lastSampleL;
		uint16 _lastSampleR;
		uint _sampleRate;
		bool _stereo;
	};

	bool _loop;
	byte *_initialPalette;

	uint32 _regularFrameDelayMs;
	// These two pointer are owned by VideoDecoder
	HNMVideoTrack *_videoTrack;
	DPCMAudioTrack *_audioTrack;

	Common::SeekableReadStream *_stream;
	byte *_dataBuffer;
	uint32 _dataBufferAlloc;
};

} // End of namespace Video

#endif
