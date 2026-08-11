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

#ifndef GRIM_SMUSH_DECODER_H
#define GRIM_SMUSH_DECODER_H

#include "audio/audiostream.h"

#include "video/video_decoder.h"

#include "graphics/surface.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Grim {

class Codec48Decoder;
class Blocky8;
class Blocky16;

class SmushDecoder : public Video::VideoDecoder {
public:
	SmushDecoder();
	~SmushDecoder();

	int getX() const { return _videoTrack->_x; }
	int getY() const { return _videoTrack->_y; }
	void setLooping(bool l);
	bool isRewindable() const override { return true; }
	bool isSeekable() const override { return true; }
	bool rewind() override;
	bool seekIntern(const Audio::Timestamp &time) override;
	bool loadStream(Common::SeekableReadStream *stream) override;

protected:
	bool readHeader();
	void handleFrameDemo();
	void handleFrame();
	bool handleFramesHeader();
	void handleFRME(Common::SeekableReadStream *stream, uint32 size);
	void init();
	void close() override;
	const Graphics::Surface *decodeNextFrame() override;
	class SmushVideoTrack : public FixedRateVideoTrack {
	public:
		SmushVideoTrack(int width, int height, int fps, int numFrames, bool is16Bit);
		~SmushVideoTrack();

		uint16 getWidth() const override { return _width; }
		uint16 getHeight() const override { return _height; }
		Graphics::PixelFormat getPixelFormat() const override { return _format; }
		int getCurFrame() const override { return _curFrame; }
		void setCurFrame(int frame) { _curFrame = frame; }
		int getFrameCount() const override { return _nbframes; }
		Common::Rational getFrameRate() const override { return _frameRate; }
		void setMsPerFrame(int ms);

		void finishFrame();
		bool isSeekable() const override { return true; }
		bool seek(const Audio::Timestamp &time) override { return true; }
		void setFrameStart(int frame);

		void handleBlocky16(Common::SeekableReadStream *stream, uint32 size);
		void handleFrameObject(Common::SeekableReadStream *stream, uint32 size);
		void handleDeltaPalette(Common::SeekableReadStream *stream, int32 size);
		void init();
		Graphics::Surface *decodeNextFrame() override;
		const byte *getPalette() const override { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const override { return _dirtyPalette; }

		byte *getPal() { _dirtyPalette = true; return _palette; }
		int _x, _y;
	private:
		void convertDemoFrame();
		bool _is16Bit;
		int32 _curFrame;
		byte _palette[0x300];
		int16 _deltaPal[0x300];
		mutable bool _dirtyPalette;
		int _width, _height;
		Graphics::Surface _surface;
		Graphics::PixelFormat _format;
		Common::Rational _frameRate;
		Blocky8 *_blocky8;
		Blocky16 *_blocky16;
		Codec48Decoder *_codec48;
		int32 _nbframes;
		int _frameStart;
	};

	class SmushAudioTrack : public AudioTrack {
	public:
		SmushAudioTrack(Audio::Mixer::SoundType soundType, bool isVima, int freq = 22050, int channels = -1);
		~SmushAudioTrack();

		Audio::AudioStream *getAudioStream() const override { return _queueStream; }
		bool isSeekable() const override { return true; }
		bool seek(const Audio::Timestamp &time) override;
		void skipSamples(int samples);
		inline int getRate() const { return _queueStream->getRate(); }

		void handleVIMA(Common::SeekableReadStream *stream, uint32 size);
		void handleIACT(Common::SeekableReadStream *stream, int32 size);
		void init();
	private:
		bool _isVima;
		byte _IACToutput[4096];
		int32 _IACTpos;
		int _channels;
		int _freq;
		Audio::QueuingAudioStream *_queueStream;
	};
private:
	void initFrames();

	SmushAudioTrack *_audioTrack;
	SmushVideoTrack *_videoTrack;

	Common::SeekableReadStream *_file;

	uint32 _startPos;

	bool _videoPause;
	bool _videoLooping;
	struct Frame {
		int frame;
		int pos;
		bool keyframe;
	};
	Frame *_frames;
	static bool _demo;
};

} // end of namespace Grim

#endif
