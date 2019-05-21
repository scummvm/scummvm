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

#ifndef CRYOMNI3D_VIDEO_HNM_DECODER_H
#define CRYOMNI3D_VIDEO_HNM_DECODER_H

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
 *  - cryomni3d
 */
class HNMDecoder : public VideoDecoder {
public:
	HNMDecoder(bool loop = false);
	virtual ~HNMDecoder();
	bool loadStream(Common::SeekableReadStream *stream);
	void readNextPacket();
	void close();

	void setRegularFrameDelay(uint32 regularFrameDelay) { _regularFrameDelay = regularFrameDelay; }

private:
	class HNM4VideoTrack : public VideoTrack {
	public:
		HNM4VideoTrack(uint32 width, uint32 height, uint32 frameSize, uint32 frameCount,
		               uint32 regularFrameDelay);
		~HNM4VideoTrack();

		// When _frameCount is 0, it means we are looping
		bool endOfTrack() const { return (_frameCount == 0) ? false : VideoTrack::endOfTrack(); }
		uint16 getWidth() const { return _surface.w; }
		uint16 getHeight() const { return _surface.h; }
		Graphics::PixelFormat getPixelFormat() const { return _surface.format; }
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		uint32 getNextFrameStartTime() const { return _nextFrameStartTime; }
		const Graphics::Surface *decodeNextFrame() { return &_surface; }
		const byte *getPalette() const { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const { return _dirtyPalette; }

		/** Decode a video chunk. */
		void decodePalette(Common::SeekableReadStream *stream, uint32 size);
		void decodeInterframe(Common::SeekableReadStream *stream, uint32 size);
		void decodeIntraframe(Common::SeekableReadStream *stream, uint32 size);

		void restart() { _nextFrameDelay = -1; _nextNextFrameDelay = -1; }
		void setFrameDelay(uint32 frameDelay);

	private:
		Graphics::Surface _surface;

		uint32 _regularFrameDelay;
		uint32 _nextFrameDelay;
		uint32 _nextNextFrameDelay;
		uint32 _nextFrameStartTime;

		uint32 _frameCount;
		int _curFrame;

		byte _palette[256 * 3];
		mutable bool _dirtyPalette;

		byte *_frameBufferC;
		byte *_frameBufferP;
	};

	class DPCMAudioTrack : public AudioTrack {
	public:
		DPCMAudioTrack(uint16 channels, uint16 bits, unsigned int sampleRate,
		               Audio::Mixer::SoundType soundType);
		~DPCMAudioTrack();

		Audio::Timestamp decodeSound(Common::SeekableReadStream *stream, uint32 size);
	protected:
		Audio::AudioStream *getAudioStream() const { return _audioStream; }
	private:
		uint16 _channels;
		uint16 _bits;
		Audio::QueuingAudioStream *_audioStream;
		bool _gotLUT;
		uint16 _lut[256];
		uint16 _lastSample;
	};

	bool _loop;

	uint32 _regularFrameDelay;
	// These two pointer are owned by VideoDecoder
	HNM4VideoTrack *_videoTrack;
	DPCMAudioTrack *_audioTrack;

	Common::SeekableReadStream *_stream;
};

} // End of namespace Video

#endif
