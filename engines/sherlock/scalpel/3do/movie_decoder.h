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

#ifndef SHERLOCK_SCALPEL_3DO_MOVIE_DECODER_H
#define SHERLOCK_SCALPEL_3DO_MOVIE_DECODER_H

#include "common/rect.h"
#include "video/video_decoder.h"
#include "audio/decoders/3do.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Image {
class Codec;
}

namespace Sherlock {

class Scalpel3DOMovieDecoder : public Video::VideoDecoder {
public:
	Scalpel3DOMovieDecoder();
	~Scalpel3DOMovieDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

protected:
	void readNextPacket();

private:
	int32 _streamVideoOffset; /* current stream offset for video decoding */
	int32 _streamAudioOffset; /* current stream offset for audio decoding */

private:
	class StreamVideoTrack : public VideoTrack  {
	public:
		StreamVideoTrack(uint32 width, uint32 height, uint32 codecTag, uint32 frameCount);
		~StreamVideoTrack();

		bool endOfTrack() const;

		uint16 getWidth() const { return _width; }
		uint16 getHeight() const { return _height; }
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		void setNextFrameStartTime(uint32 nextFrameStartTime) { _nextFrameStartTime = nextFrameStartTime; }
		uint32 getNextFrameStartTime() const { return _nextFrameStartTime; }
		const Graphics::Surface *decodeNextFrame() { return _surface; }

		void decodeFrame(Common::SeekableReadStream *stream, uint32 videoTimeStamp);

	private:
		const Graphics::Surface *_surface;

		int _curFrame;
		uint32 _frameCount;
		uint32 _nextFrameStartTime;

		Image::Codec *_codec;
		uint16 _width, _height;
	};

	class StreamAudioTrack : public AudioTrack {
	public:
		StreamAudioTrack(uint32 codecTag, uint32 sampleRate, uint32 channels, Audio::Mixer::SoundType soundType);
		~StreamAudioTrack();

		void queueAudio(Common::SeekableReadStream *stream, uint32 size);

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audioStream;
		uint32 _totalAudioQueued; /* total amount of milliseconds of audio, that we queued up already */

	public:
		uint32 getTotalAudioQueued() const { return _totalAudioQueued; }

	private:
		int16 decodeSample(uint8 dataNibble);

		uint32 _codecTag;
		uint16 _sampleRate;
		bool   _stereo;

		Audio::audio_3DO_ADP4_PersistentSpace _ADP4_PersistentSpace;
		Audio::audio_3DO_SDX2_PersistentSpace _SDX2_PersistentSpace;
	};

	Common::SeekableReadStream *_stream;
	StreamVideoTrack *_videoTrack;
	StreamAudioTrack *_audioTrack;
};

} // End of namespace Sherlock

#endif
