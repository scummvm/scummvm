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

class FourXMDecoder::FourXMAudioTrack : public AudioTrack {
	FourXMDecoder *_dec;
	uint _trackIdx;
	uint _audioType;
	uint _audioChannels;
	uint _sampleRate;
	Common::MemoryReadWriteStream _bitStream;
	Common::ScopedPtr<Audio::AudioStream> _adpcm;
	Common::ScopedPtr<Audio::QueuingAudioStream> _output;

public:
	FourXMAudioTrack(FourXMDecoder *dec, uint trackIdx, uint audioType, uint audioChannels, uint sampleRate) : AudioTrack(Audio::Mixer::SoundType::kPlainSoundType), _dec(dec), _trackIdx(trackIdx), _audioType(audioType), _audioChannels(audioChannels), _sampleRate(sampleRate),
																											   _bitStream(DisposeAfterUse::YES), _output(Audio::makeQueuingAudioStream(sampleRate, audioChannels > 1)) {
	}

	void decode(uint32 tag, const byte *data, uint size) {
		debug("got %u of audio data", size);
		_bitStream.write(data, size);
		if (!_adpcm) {
			assert(_bitStream.pos() == 0);
			_adpcm.reset(Audio::makeADPCMStream(&_bitStream, DisposeAfterUse::NO, size, Audio::kADPCM4XM, _sampleRate, _audioChannels));
			// this is first bitstream write, getting preamble size from stream pos.
			size -= _bitStream.pos();
		}
		int numSamples = size * 2;

		auto bufSize = numSamples * sizeof(int16);
		int16 *buf = static_cast<int16 *>(malloc(bufSize));
		int r = _adpcm->readBuffer(buf, numSamples);
		if (r < 0) {
			free(buf);
			warning("ADPCMStream::readBuffer failed");
			return;
		}
		assert(_bitStream.pos() == _bitStream.size());
		assert(r == numSamples);
		debug("decoded %d samples", r);
		byte flags = Audio::FLAG_16BITS;
		if (_audioChannels > 1)
			flags |= Audio::FLAG_STEREO;
		_output->queueBuffer(reinterpret_cast<byte *>(buf), bufSize, DisposeAfterUse::YES, flags);
	}

private:
	Audio::AudioStream *getAudioStream() const override { return _output.get(); }
};

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
		debug("%u: sub frame %s, %u bytes at %08lx", _curFrame, tagName(tag).c_str(), size, _stream->pos());
		auto pos = _stream->pos();
		if (size > packet.size())
			packet.resize(size);
		_stream->read(packet.data(), size);
		switch (tag) {
		case MKTAG('s', 'n', 'd', '_'): {
			uint offset = 0;
			while (offset < size) {
				auto trackIdx = READ_UINT32(packet.data() + offset);
				auto packetSize = READ_UINT32(packet.data() + offset + 4);
				if (trackIdx == 0) {
					if (_audio)
						_audio->decode(tag, packet.data() + offset + 8, packetSize);
					else
						warning("no audio stream to decode sample to");
				}
				offset += packetSize + 8;
			}
		} break;
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
