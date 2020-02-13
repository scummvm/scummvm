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

#ifndef ACCESS_VIDEO_MOVIE_DECODER_H
#define ACCESS_VIDEO_MOVIE_DECODER_H

#include "video/video_decoder.h"
#include "audio/audiostream.h"

namespace Common {
class SeekableReadStream;
}

namespace Image {
class Codec;
}

namespace Access {

enum kDebugLevels {
	kVIDMovieChunkId_FullFrame = 0x00,
	kVIDMovieChunkId_FullFrameCompressed = 0x01,
	kVIDMovieChunkId_Palette = 0x02,
	kVIDMovieChunkId_FullFrameCompressedFill = 0x03,
	kVIDMovieChunkId_PartialFrameCompressed = 0x04,
	kVIDMovieChunkId_EndOfFile = 0x14,
	kVIDMovieChunkId_AudioFirstChunk = 0x7C,
	kVIDMovieChunkId_Audio = 0x7D
};

// This video format is used in at least the following Access engine games:
//  - Noctropolis
//  - Synnergist

class AccessVIDMovieDecoder : public Video::VideoDecoder {
public:
	AccessVIDMovieDecoder();
	~AccessVIDMovieDecoder() override;

	bool loadStream(Common::SeekableReadStream *stream) override;
	void close() override;

protected:
	void readNextPacket() override;

private:
	bool streamSkipFullFrameCompressedFill();

private:
	int32 _streamSeekOffset;  /* current stream offset, pointing to not-yet-indexed stream position */
	uint32 _streamVideoIndex; /* current stream index for video decoding */
	uint32 _streamAudioIndex; /* current stream index for audio decoding */

	struct IndexCacheEntry {
		byte   chunkId;
		int32 offset;
	};

	Common::Array<IndexCacheEntry> _indexCacheTable;

private:
	class StreamVideoTrack : public VideoTrack  {
	public:
		StreamVideoTrack(uint32 width, uint32 height, uint16 regularFrameDelay);
		~StreamVideoTrack() override;

		bool endOfTrack() const override;

		uint16 getWidth() const override { return _width; }
		uint16 getHeight() const override { return _height; }
		Graphics::PixelFormat getPixelFormat() const override;
		int getCurFrame() const override { return _curFrame; }
		void setNextFrameStartTime(uint32 nextFrameStartTime) { _nextFrameStartTime = nextFrameStartTime; }
		uint32 getNextFrameStartTime() const override { return _nextFrameStartTime; }
		const Graphics::Surface *decodeNextFrame() override { return _surface; }

		const byte *getPalette() const override;
		bool hasDirtyPalette() const override;

		void decodePalette(Common::SeekableReadStream *stream);
		void decodeFrame(Common::SeekableReadStream *stream, byte chunkId);
		bool skipOverFrame(Common::SeekableReadStream *stream, byte chunkId);
		bool skipOverPalette(Common::SeekableReadStream *stream);

		void setEndOfTrack() { _endOfTrack = true; }

	private:
		Graphics::Surface *_surface;

		int _curFrame;
		uint32 _nextFrameStartTime;

		byte _palette[3 * 256];
		mutable bool _dirtyPalette;
		uint16 _width, _height;

		uint16 _regularFrameDelay; // delay between frames (1 = 1/60 of a second)
		bool _endOfTrack;
	};

	class StreamAudioTrack : public AudioTrack {
	public:
		StreamAudioTrack(uint32 sampleRate, Audio::Mixer::SoundType soundType);
		~StreamAudioTrack() override;

		void queueAudio(Common::SeekableReadStream *stream, byte chunkId);
		bool skipOverAudio(Common::SeekableReadStream *stream, byte chunkId);

	protected:
		Audio::AudioStream *getAudioStream() const override;

	private:
		Audio::QueuingAudioStream *_audioStream;
		uint32 _totalAudioQueued; /* total amount of milliseconds of audio, that we queued up already */

	public:
		uint32 getTotalAudioQueued() const { return _totalAudioQueued; }

	private:
		int16 decodeSample(uint8 dataNibble);

		uint16 _sampleRate;
		bool   _stereo;
	};

	Common::SeekableReadStream *_stream;
	StreamVideoTrack *_videoTrack;
	StreamAudioTrack *_audioTrack;
};

} // End of namespace Access

#endif
