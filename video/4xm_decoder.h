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

#include "video/video_decoder.h"

namespace Video {

/**
 * Decoder for 4XM videos.
 *
 * Video decoder used in engines:
 *  - phoenixvr
 */
class FourXMDecoder : public Video::VideoDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;

private:
	struct Frame {
		int64 offset;
		int64 end;
	};

	class FourXMVideoTrack : public FixedRateVideoTrack {
		FourXMDecoder *_dec;
		Common::Rational _frameRate;
		uint _w, _h;
		Graphics::Surface *_frame;

	public:
		FourXMVideoTrack(FourXMDecoder *dec, const Common::Rational &frameRate, uint w, uint h) : _dec(dec), _frameRate(frameRate), _w(w), _h(h), _frame(nullptr) {}
		~FourXMVideoTrack();

		uint16 getWidth() const override { return _w; }
		uint16 getHeight() const override { return _h; }

		Graphics::PixelFormat getPixelFormat() const override {
			return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0); // RGB565
		}
		int getCurFrame() const override;
		int getFrameCount() const override;
		const Graphics::Surface *decodeNextFrame() override;

		void decode(uint32 tag, const byte *data, uint size);

	private:
		Common::Rational getFrameRate() const override { return _frameRate; }
	};

	class FourXMAudioStream;

	class FourXMAudioTrack : public AudioTrack {
		friend class FourXMAudioStream;

		FourXMDecoder *_dec;
		uint _trackIdx;
		uint _audioType;
		uint _audioChannels;
		uint _sampleRate;
		uint _sampleResolution;

	public:
		FourXMAudioTrack(FourXMDecoder *dec, uint trackIdx, uint audioType, uint audioChannels, uint sampleRate, uint sampleResolution) : AudioTrack(Audio::Mixer::SoundType::kPlainSoundType), _dec(dec), _trackIdx(trackIdx), _audioType(audioType), _audioChannels(audioChannels), _sampleRate(sampleRate), _sampleResolution(sampleResolution) {
		}

	private:
		Audio::AudioStream *getAudioStream() const override;
	};

	void readList(uint32 size);
	void decodeNextFrameImpl();

	uint32 _dataRate = 0;
	Common::Rational _frameRate;
	Common::SeekableReadStream *_stream;
	Common::Array<Frame> _frames;
	uint _curFrame = 0;
	FourXMVideoTrack *_video = nullptr;
	FourXMAudioTrack *_audio = nullptr;
	FourXMAudioStream *_audioStream = nullptr;
};

} // namespace Video
