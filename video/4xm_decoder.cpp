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

class FourXMDecoder::FourXMAudioStream : public Audio::AudioStream {
	const FourXMAudioTrack *_track;
	Common::List<Common::Array<int16>> _buffers;

public:
	FourXMAudioStream(const FourXMAudioTrack *track) : _track(track) {}

	int readBuffer(int16 *buffer, const int numSamples) override {
		int offset = 0;
		while (!_buffers.empty() && offset < numSamples) {
			auto &next = _buffers.front();
			auto read = MIN<uint>(numSamples - offset, next.size());
			for (uint i = 0; i != read; ++i) {
				buffer[offset++] = next[i];
			}
			if (read == next.size()) {
				_buffers.pop_front();
			} else {
				next.erase(next.begin(), next.begin() + read);
			}
		}
		return offset;
	}

	bool isStereo() const override {
		return _track->_audioChannels > 1;
	}

	/** Sample rate of the stream. */
	virtual int getRate() const override {
		return _track->_sampleRate;
	}

	virtual bool endOfData() const override {
		auto *decoder = _track->_dec;
		return decoder->_curFrame >= decoder->_frames.size();
	}

	void decode(uint32 tag, const byte *data, uint size) {
		Common::MemoryReadStream ms(data, size);
		Common::ScopedPtr<Audio::SeekableAudioStream> x(Audio::makeADPCMStream(&ms, DisposeAfterUse::NO, size, Audio::kADPCM4XM, _track->_sampleRate, _track->_audioChannels));
		auto numChannels = _track->_audioChannels;
		int numSamples = (size - 4 * numChannels) * 2;
		Common::Array<int16> buf(numSamples);
		int r = x->readBuffer(buf.data(), numSamples);
		if (r < 0) {
			warning("ADPCMStream::readBuffer failed");
			return;
		}
		assert(r == numSamples);
		if (!buf.empty())
			_buffers.push_back(Common::move(buf));
	}
};

Audio::AudioStream *FourXMDecoder::FourXMAudioTrack::getAudioStream() const {
	if (!_dec->_audioStream)
		_dec->_audioStream = new FourXMAudioStream(this);
	return _dec->_audioStream;
}

void FourXMDecoder::FourXMVideoTrack::decode(uint32 tag, const byte *data, uint size) {
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

	Common::Array<byte> packet;
	while (_stream->pos() < frame.end) {
		uint32 tag = _stream->readUint32BE();
		uint32 size = _stream->readUint32LE();
		debug("%u: sub frame %s, %u bytes at %08lx", _curFrame, tagName(tag).c_str(), size, _stream->pos() - 4);
		auto pos = _stream->pos();
		if (size > packet.size())
			packet.resize(size);
		_stream->read(packet.data(), size);
		switch (tag) {
		case MKTAG('s', 'n', 'd', '_'):
			if (_audioStream)
				_audioStream->decode(tag, packet.data(), size);
			else
				warning("no audio stream to decode sample to");
			break;
		case MKTAG('i', 'f', 'r', 'm'):
		case MKTAG('p', 'f', 'r', 'm'):
		case MKTAG('c', 'f', 'r', 'm'):
			_video->decode(tag, packet.data(), size);
			break;
		default:
			warning("unknown frame type %s", tagName(tag).c_str());
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
				debug("audio track %u %u %u %u %u", trackIdx, audioType, audioChannels, sampleRate, sampleResolution);
				addTrack(_audio = new FourXMAudioTrack(this, trackIdx, audioType, audioChannels, sampleRate, sampleResolution));
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
