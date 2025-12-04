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

#include "video/4xm_decoder.h"
#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

namespace Video {

Common::Rational floatToRational(float value) {
	int num = static_cast<int>(1000 * value);
	int denom = 1000;
	return {num, denom};
}

namespace {
Common::String tagName(uint32 tag) {
	char name[5] = {char(tag >> 24), char(tag >> 16), char(tag >> 8), char(tag), 0};
	return {name};
}
} // namespace

class FourXMDecoder::FourXMAudioTrack : public AudioTrack {
	FourXMDecoder *_dec;
	uint _trackIdx;
	uint _audioType;
	uint _audioChannels;
	uint _sampleRate;
	Common::ScopedPtr<Audio::QueuingAudioStream> _output;

public:
	FourXMAudioTrack(FourXMDecoder *dec, uint trackIdx, uint audioType, uint audioChannels, uint sampleRate) : AudioTrack(Audio::Mixer::SoundType::kPlainSoundType), _dec(dec), _trackIdx(trackIdx), _audioType(audioType), _audioChannels(audioChannels), _sampleRate(sampleRate),
																											   _output(Audio::makeQueuingAudioStream(sampleRate, audioChannels > 1)) {
	}

	void decode(uint32 tag, byte *buf, uint size) {
		if (_audioType == 0) {
			// Raw PCM data
			byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
			if (_audioChannels > 1)
				flags |= Audio::FLAG_STEREO;
			_output->queueBuffer(buf, size, DisposeAfterUse::YES, flags);
		} else {
			free(buf);
			warning("unsupported audio type %u", _audioType);
		}
	}

private:
	Audio::AudioStream *getAudioStream() const override { return _output.get(); }
};

class FourXMDecoder::FourXMVideoTrack : public FixedRateVideoTrack {
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

	void decode(uint32 tag, byte *buf, uint size);

private:
	Common::Rational getFrameRate() const override { return _frameRate; }
};

const Graphics::Surface *FourXMDecoder::FourXMVideoTrack::decodeNextFrame() {
	if (!_frame) {
		_frame = new Graphics::Surface();
		_frame->create(_w, _h, getPixelFormat());
	}
	debug("decode next video frame");
	_dec->decodeNextFrameImpl();
	return _frame;
}

int FourXMDecoder::FourXMVideoTrack::getCurFrame() const {
	return _dec->_curFrame;
}

int FourXMDecoder::FourXMVideoTrack::getFrameCount() const {
	return _dec->_frames.size();
}

FourXMDecoder::FourXMVideoTrack::~FourXMVideoTrack() {
	if (_frame) {
		_frame->free();
		delete _frame;
	}
}

void FourXMDecoder::FourXMVideoTrack::decode(uint32 tag, byte *buf, uint size) {
	free(buf);
}

void FourXMDecoder::decodeNextFrameImpl() {
	if (_curFrame >= _frames.size())
		return;
	auto &frame = _frames[_curFrame];
	_stream->seek(frame.offset);
	uint32 listType = _stream->readUint32BE();
	if (listType != MKTAG('F', 'R', 'A', 'M')) {
		error("invalid FRAM offset for frame %u", _curFrame);
	}

	while (_stream->pos() < frame.end) {
		uint32 tag = _stream->readUint32BE();
		uint32 size = _stream->readUint32LE();
		debug("%u: sub frame %s, %u bytes at %08lx", _curFrame, tagName(tag).c_str(), size, _stream->pos());
		auto pos = _stream->pos();

		auto loadBuf = [this](uint bufSize) {
			byte *buf = static_cast<byte *>(malloc(bufSize));
			if (!buf)
				error("failed to allocate %u bytes", bufSize);
			_stream->read(buf, bufSize);
			return buf;
		};
		switch (tag) {
		case MKTAG('s', 'n', 'd', '_'): {
			uint offset = 0;
			while (offset < size) {
				auto trackIdx = _stream->readUint32LE();
				auto packetSize = _stream->readUint32LE();
				if (trackIdx == 0 && _audio) {
					_audio->decode(tag, loadBuf(packetSize), packetSize);
				} else
					offset += packetSize + 8;
			}
		} break;
		case MKTAG('i', 'f', 'r', 'm'):
		case MKTAG('p', 'f', 'r', 'm'):
		case MKTAG('c', 'f', 'r', 'm'):
			_video->decode(tag, loadBuf(size), size);
			break;
		default:
			warning("unknown frame type %s", tagName(tag).c_str());
			_stream->skip(size);
		}
		_stream->seek(pos + size + (size & 1));
	}
	++_curFrame;
}

void FourXMDecoder::readList(uint32 listEnd) {
	assert(_stream);
	uint32 listType = _stream->readUint32BE();
	if (listType == MKTAG('F', 'R', 'A', 'M')) {
		_frames.push_back({_stream->pos() - 4, listEnd});
		return;
	}
	debug("%08lx: list type %s", _stream->pos() - 4, tagName(listType).c_str());
	while (_stream->pos() < listEnd) {
		uint32 tag = _stream->readUint32BE();
		uint32 size = _stream->readUint32LE();
		if (listType != MKTAG('F', 'R', 'A', 'M'))
			debug("%08lx: tag %s, size %u/0x%x", _stream->pos() - 8, tagName(tag).c_str(), size, size);
		auto pos = _stream->pos();
		if (tag == MKTAG('L', 'I', 'S', 'T')) {
			readList(pos + size);
		}
		switch (listType) {
		case MKTAG('H', 'N', 'F', 'O'):
			switch (tag) {
			case MKTAG('s', 't', 'd', '_'):
				_dataRate = _stream->readUint32LE();
				_frameRate = floatToRational(_stream->readFloatLE());
				debug("data rate: %u, frame rate: %d/%d", _dataRate, _frameRate.getNumerator(), _frameRate.getDenominator());
				break;
			default:
				break;
			}
			break;
		case MKTAG('V', 'T', 'R', 'K'):
			switch (tag) {
			case MKTAG('v', 't', 'r', 'k'): {
				_stream->skip(28);
				auto w = _stream->readUint32LE();
				auto h = _stream->readUint32LE();
				debug("video %ux%u", w, h);
				addTrack(_video = new FourXMVideoTrack(this, _frameRate, w, h));
			} break;
			default:
				break;
			}
			break;
		case MKTAG('S', 'T', 'R', 'K'):
			switch (tag) {
			case MKTAG('s', 't', 'r', 'k'): {
				auto trackIdx = _stream->readUint32LE();
				auto audioType = _stream->readUint32LE();
				_stream->skip(20);
				auto audioChannels = _stream->readUint32LE();
				auto sampleRate = _stream->readUint32LE();
				auto sampleResolution = _stream->readUint32LE();
				debug("audio track idx: %u type: %u channels: %u sample rate: %u bits: %u", trackIdx, audioType, audioChannels, sampleRate, sampleResolution);
				if (sampleResolution != 16)
					error("only 16 bit audio is supported");
				addTrack(_audio = new FourXMAudioTrack(this, trackIdx, audioType, audioChannels, sampleRate));
			} break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		_stream->seek(pos + size + (size & 1));
	}
}

bool FourXMDecoder::loadStream(Common::SeekableReadStream *stream) {
	if (!stream->size()) {
		return false;
	}

	uint32 riffTag = stream->readUint32BE();
	if (riffTag != MKTAG('R', 'I', 'F', 'F')) {
		warning("Failed to find RIFF header");
		return false;
	}

	uint32 fileSize = stream->readUint32LE();
	uint32 riffType = stream->readUint32BE();

	if (riffType != MKTAG('4', 'X', 'M', 'V')) {
		warning("RIFF not an 4XM file, got: %08x", riffType);
		return false;
	}

	_stream = stream;

	debug("file size %u", fileSize);
	while (stream->pos() < fileSize) {
		uint32 tag = stream->readUint32BE();
		uint32 size = stream->readUint32LE();
		debug("%08lx: tag %s, size %u/0x%x", stream->pos() - 8, tagName(tag).c_str(), size, size);
		auto pos = stream->pos();
		if (tag == MKTAG('L', 'I', 'S', 'T')) {
			readList(pos + size);
		}
		stream->seek(pos + size + (size & 1));
	}

	debug("loaded %u frames", _frames.size());
	return getNumTracks() != 0;
}

} // namespace Video
