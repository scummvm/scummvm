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

#include "video/video_decoder.h"

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
	class StreamVideoTrack : public FixedRateVideoTrack  {
	public:
		StreamVideoTrack(uint32 width, uint32 height, uint32 codecTag, uint32 frameCount, uint32 frameRate);
		~StreamVideoTrack();

		uint16 getWidth() const { return _width; }
		uint16 getHeight() const { return _height; }
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		const Graphics::Surface *decodeNextFrame() { return _surface; }

		void decodeFrame(Common::SeekableReadStream *stream);

	protected:
		Common::Rational getFrameRate() const { return _frameRate; }

	private:
		const Graphics::Surface *_surface;
		uint32 _frameRate;

		uint32 _frameCount;
		Image::Codec *_codec;
		uint16 _width, _height;
		int _curFrame;
	};

	class StreamAudioTrack : public AudioTrack {
	public:
		StreamAudioTrack(uint32 codecTag, uint32 sampleRate, uint32 channels);
		~StreamAudioTrack();

		void queueAudio(Common::SeekableReadStream *stream, uint32 length);

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audioStream;

	private:
		int16 decodeSample(uint8 dataNibble);

		int16 _lastSample;
		int16 _stepIndex;
	};

	Common::SeekableReadStream *_stream;
	StreamVideoTrack *_videoTrack;
	StreamAudioTrack *_audioTrack;
};

// Testing
extern void Scalpel3DOMoviePlay(const char *filename);

} // End of namespace Sherlock

#endif
