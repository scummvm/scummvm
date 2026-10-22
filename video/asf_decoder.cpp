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

#include "video/asf_decoder.h"
#include "video/asf.h"
#include "common/bufferedstream.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "image/codecs/codec.h"
#ifdef USE_VC1
#include "image/codecs/vc1.h"
#endif
#include "graphics/surface.h"

namespace Video {

class ASFDecoder::ASFVideoTrack : public VideoTrack {
public:
	ASFVideoTrack() : _codec(nullptr), _pending(nullptr), _width(0), _height(0),
		_frame(-1), _time(0), _vc1(false) {}
	~ASFVideoTrack() override { delete _codec; _surface.free(); }
	bool open(const ASFDemuxer::Stream &stream) {
		if (stream.typeData.size() < 51)
			return false;
		Common::MemoryReadStream format(stream.typeData.data(), stream.typeData.size());
		format.skip(9);
		format.readUint16LE();
		uint32 headerSize = format.readUint32LE();
		int32 width = format.readSint32LE();
		int32 height = format.readSint32LE();
		format.readUint16LE();
		uint16 bits = format.readUint16LE();
		uint32 tag = format.readUint32BE();
		if (headerSize < 40 || headerSize > stream.typeData.size() - 11 ||
		    width <= 0 || width > 65535 || height <= 0 || height > 65535)
			return false;
		_width = width;
		_height = height;
		_vc1 = tag == MKTAG('W','M','V','3') || tag == MKTAG('w','m','v','3') ||
		       tag == MKTAG('W','V','C','1') || tag == MKTAG('w','v','c','1') || tag == MKTAG('W','M','V','A');
		_codec = Image::createBitmapCodec(tag, 0, width, height, bits);
		if (!_codec)
			return false;
#ifdef USE_VC1
		if (_vc1) {
			Common::MemoryReadStream extra(stream.typeData.data() + 51, headerSize - 40);
			if (!static_cast<Image::WMV3Decoder *>(_codec)->setExtraData(extra))
				return false;
		}
#endif
		return true;
	}
	uint16 getWidth() const override { return _width; }
	uint16 getHeight() const override { return _height; }
	Graphics::PixelFormat getPixelFormat() const override { return _codec->getPixelFormat(); }
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override {
		return format == getPixelFormat();
	}
	int getCurFrame() const override { return _frame; }
	uint32 getNextFrameStartTime() const override { return _time; }
	bool endOfTrack() const override { return !_pending; }
	const Graphics::Surface *decodeNextFrame() override { return _surface.getPixels() ? &_surface : nullptr; }
	void consume() {
		if (_pending) {
			_surface.copyFrom(*_pending);
			_pending = nullptr;
			++_frame;
		}
	}
	bool decode(const ASFDemuxer::Object &object) {
		_times.push_back(object.pts != ASFDemuxer::kNoTimestamp ? object.pts : object.dts);
		Common::MemoryReadStream data(object.data.data(), object.data.size());
		_pending = _codec->decodeFrame(data);
		return setTime();
	}
	bool drain() {
#ifdef USE_VC1
		if (_vc1)
			_pending = static_cast<Image::WMV3Decoder *>(_codec)->drainFrame();
#endif
		return setTime();
	}
	bool failed() const {
#ifdef USE_VC1
		if (_vc1)
			return static_cast<Image::WMV3Decoder *>(_codec)->hasError();
#endif
		return !_vc1 && !_pending;
	}
private:
	bool setTime() {
		if (!_pending)
			return false;
		if (_times.empty())
			return false;
		uint first = 0;
		for (uint i = 1; i < _times.size(); ++i)
			if (_times[i] < _times[first])
				first = i;
		_time = MAX<int64>(0, _times[first]);
		_times.remove_at(first);
		return true;
	}
	Image::Codec *_codec;
	const Graphics::Surface *_pending;
	Graphics::Surface _surface;
	Common::Array<int64> _times;
	uint16 _width, _height;
	int _frame;
	uint32 _time;
	bool _vc1;
};

class ASFDecoder::ASFAudioTrack : public AudioTrack {
public:
	ASFAudioTrack(Audio::Mixer::SoundType soundType) : AudioTrack(soundType), _queue(nullptr) {}
	~ASFAudioTrack() override { delete _queue; }
	bool open(const ASFDemuxer::Stream &stream) {
		if (!_codec.open(stream))
			return false;
		_queue = Audio::makeQueuingAudioStream(_codec.getRate(), _codec.isStereo());
		return true;
	}
	void decode(const ASFDemuxer::Object &object) {
		Audio::AudioStream *audio = _codec.decode(object);
		if (audio)
			_queue->queueAudioStream(audio, DisposeAfterUse::YES);
	}
	void finish() { _queue->finish(); }
	bool hasEnoughPackets() const { return _queue->numQueuedStreams() > 25; }
protected:
	Audio::AudioStream *getAudioStream() const override { return _queue; }
private:
	ASFAudioCodec _codec;
	Audio::QueuingAudioStream *_queue;
};

ASFDecoder::ASFDecoder() : _input(nullptr), _videoQueueBytes(0), _video(nullptr), _audio(nullptr),
	_videoID(0), _audioID(0), _ended(false) {}

ASFDecoder::~ASFDecoder() {
	close();
}

void ASFDecoder::close() {
	VideoDecoder::close();
	_video = nullptr;
	_audio = nullptr;
	_demux.close();
	_videoObjects.clear();
	_videoQueueBytes = 0;
	delete _input;
	_input = nullptr;
	_videoID = _audioID = 0;
	_ended = false;
}

bool ASFDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();
	_input = Common::wrapBufferedSeekableReadStream(stream, 1024 * 1024, DisposeAfterUse::YES);
	if (!_demux.open(_input)) {
		close();
		return false;
	}
	for (const auto &info : _demux.getStreams()) {
		if (info.type == ASFDemuxer::kVideo && !_video) {
			ASFVideoTrack *track = new ASFVideoTrack();
			if (!track->open(info)) {
				delete track;
				delete _audio;
				_audio = nullptr;
				close();
				return false;
			}
			_video = track;
			_videoID = info.id;
		} else if (info.type == ASFDemuxer::kAudio && !_audio) {
			ASFAudioTrack *track = new ASFAudioTrack(getSoundType());
			if (!track->open(info)) {
				delete track;
				delete _video;
				_video = nullptr;
				close();
				return false;
			}
			_audio = track;
			_audioID = info.id;
		}
	}
	if (!_video || !prepareFrame()) {
		delete _video;
		delete _audio;
		_video = nullptr;
		_audio = nullptr;
		close();
		return false;
	}
	bufferPackets();
	addTrack(_video);
	if (_audio)
		addTrack(_audio);
	return true;
}

bool ASFDecoder::readObject(ASFDemuxer::Object &object) {
	ASFDemuxer::ReadResult result = _demux.readObject(object);
	if (result == ASFDemuxer::kObject)
		return true;
	_ended = true;
	if (_audio)
		_audio->finish();
	if (result == ASFDemuxer::kError)
		warning("ASF: Invalid packet");
	return false;
}

void ASFDecoder::bufferPackets() {
	while (!_ended) {
		if (_videoQueueBytes > 15 * 1024 * 1024 ||
		    (_videoObjects.size() > 25 && (!_audio || _audio->hasEnoughPackets())))
			break;
		ASFDemuxer::Object object;
		if (!readObject(object))
			break;
		if (_audio && object.stream == _audioID) {
			_audio->decode(object);
		} else if (object.stream == _videoID) {
			_videoQueueBytes += object.data.size();
			_videoObjects.push(object);
		}
	}
}

bool ASFDecoder::prepareFrame() {
	while (!_videoObjects.empty() || !_ended) {
		ASFDemuxer::Object object;
		if (!_videoObjects.empty()) {
			object = _videoObjects.pop();
			_videoQueueBytes -= object.data.size();
		} else if (!readObject(object))
			break;
		if (_audio && object.stream == _audioID) {
			_audio->decode(object);
		} else if (_video && object.stream == _videoID) {
			if (_video->decode(object))
				return true;
			if (_video->failed()) {
				warning("ASF: Video decoding failed");
				_ended = true;
				_videoObjects.clear();
				_videoQueueBytes = 0;
				if (_audio)
					_audio->finish();
				return false;
			}
		}
	}
	return _video && _video->drain();
}

void ASFDecoder::readNextPacket() {
	if (_video && !_video->endOfTrack()) {
		_video->consume();
		prepareFrame();
		bufferPackets();
	}
}

} // End of namespace Video
