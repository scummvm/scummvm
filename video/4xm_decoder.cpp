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
#include "video/4xm_utils.h"

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

	byte getAudioType() const { return _audioType; }

	void decode(uint32 tag, byte *buf, uint size) {
		if (_audioType == 0) {
			// Raw PCM data
			byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
			if (_audioChannels > 1)
				flags |= Audio::FLAG_STEREO;
			_output->queueBuffer(buf, size, DisposeAfterUse::YES, flags);
		} else if (_audioType == 1) {
			auto *input = new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
			_output->queueAudioStream(Audio::makeADPCMStream(input, DisposeAfterUse::YES, size, Audio::ADPCMType::kADPCM4XM, _sampleRate, _audioChannels));
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
	uint16 _version = 0;
	Graphics::Surface *_frame;
	FourXM::HuffmanDecoder _blockType[2][4] = {};

public:
	FourXMVideoTrack(FourXMDecoder *dec, const Common::Rational &frameRate, uint w, uint h, uint16 version) : _dec(dec), _frameRate(frameRate), _w(w), _h(h), _version(version), _frame(nullptr) {
		_blockType[0][0].initStatistics({2, 1, 1, 2, 1, 1});
		_blockType[0][1].initStatistics({2, 0, 2, 2, 1, 1});
		_blockType[0][2].initStatistics({2, 2, 0, 2, 1, 1});
		_blockType[0][3].initStatistics({2, 0, 0, 2, 2, 1, 1});

		_blockType[1][0].initStatistics({16, 8, 4, 2, 1, 1});
		_blockType[1][1].initStatistics({8, 0, 4, 2, 1, 1});
		_blockType[1][2].initStatistics({8, 4, 0, 2, 1, 1});
		_blockType[1][3].initStatistics({8, 0, 0, 4, 2, 1, 1});
	}
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
	void decode_ifrm(Common::SeekableReadStream *stream);
	void decode_pfrm(Common::SeekableReadStream *stream);
	void decode_cfrm(Common::SeekableReadStream *stream);

private:
	void decode_pfrm_block(Graphics::Surface *frame, int x, int y, int log2w, int log2h, FourXM::BEByteBitStream &bitStream, Common::MemoryReadStream &wordStream, Common::MemoryReadStream &byteStream);
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

namespace {
static const uint8_t iquant[64] = {
	16,
	15,
	13,
	19,
	24,
	31,
	28,
	17,
	17,
	23,
	25,
	31,
	36,
	63,
	45,
	21,
	18,
	24,
	27,
	37,
	52,
	59,
	49,
	20,
	16,
	28,
	34,
	40,
	60,
	80,
	51,
	20,
	18,
	31,
	48,
	66,
	68,
	86,
	56,
	21,
	19,
	38,
	56,
	59,
	64,
	64,
	48,
	20,
	27,
	48,
	55,
	55,
	56,
	51,
	35,
	15,
	20,
	35,
	34,
	32,
	31,
	22,
	15,
	8,
};

const byte zigzag[] = {
	0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4,
	5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7,
	14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22,
	15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39,
	46, 53, 60, 61, 54, 47, 55, 62, 63};

static const int8_t size2index[4][4] = {
	{-1, 3, 1, 1},
	{3, 0, 0, 0},
	{2, 0, 0, 0},
	{2, 0, 0, 0},
};

} // namespace

void FourXMDecoder::FourXMVideoTrack::decode_ifrm(Common::SeekableReadStream *stream) {
	stream->skip(4);
	auto bitstreamSize = stream->readUint32LE();

	Common::Array<byte> bitstreamData(bitstreamSize);
	stream->read(bitstreamData.data(), bitstreamData.size());

	auto prefixSize = stream->readUint32LE();
	auto tokenCount = stream->readUint32LE();
	debug("i-frame, bitstream: %u, prefix stream: %u, tokens: %u", bitstreamSize, prefixSize, tokenCount);

	Common::Array<byte> prefixStream(prefixSize * 4);
	stream->read(prefixStream.data(), prefixStream.size());
	assert(stream->pos() == stream->size());

	auto prefixData = FourXM::HuffmanDecoder::unpack(prefixStream.data(), prefixStream.size(), 4);
	FourXM::BEByteBitStream bitstream(bitstreamData.data(), bitstreamData.size(), 0);
	uint prefixOffset = 0;
	int lastDC = 0;
	for (int mbY = 0; mbY < _frame->h; mbY += 16) {
		for (int mbX = 0; mbX < _frame->w; mbX += 16) {
			int16_t block[6][64] = {};
			auto readBlock = [&](byte blockIdx, int16_t *ac) {
				int dc = prefixData[prefixOffset++];
				if (dc >> 4)
					error("dc run code");
				dc = bitstream.readInt(dc);
				dc = lastDC + dc * iquant[0];
				lastDC = dc;
				ac[0] = dc;
				for (uint idx = 1; idx < 64;) {
					auto b = prefixData[prefixOffset++];
					if (b == 0x00) {
						break;
					} else if (b == 0xf0) {
						idx += 16;
					} else {
						auto h = b >> 4;
						auto l = b & 0x0f;
						idx += h;
						if (l && idx < 64) {
							auto ac_idx = zigzag[idx];
							ac[ac_idx] = iquant[ac_idx] * bitstream.readInt(l);
							++idx;
						}
					}
				}
				if (blockIdx < 4)
					ac[0] += 0x80 * 8 * 8;

				FourXM::idct(ac);
			};

			for (int b = 0; b != 6; ++b)
				readBlock(b, block[b]);

			auto acIdx = [](byte y, byte x) {
				assert(y < 8 && x < 8);
				return y << 3 | x;
			};
			auto blockCB = block[4];
			auto blockCR = block[5];
			for (byte y = 0; y != 16; ++y) {
				for (byte x = 0; x != 16; ++x) {
					auto yb = y & 7;
					auto xb = x & 7;
					auto mbBlockIdx = ((y >> 3) << 1) | (x >> 3);
					auto Y = block[mbBlockIdx][acIdx(yb, xb)];
					auto cblockIdx = acIdx(y >> 1, x >> 1);
					auto CB = blockCB[cblockIdx];
					auto CR = blockCR[cblockIdx];
					int CG = (CB + CR) >> 1;
					CB += CB;
					auto color = _frame->format.RGBToColor(Y + CR, Y - CG, Y + CB);
					_frame->setPixel(mbX | x, mbY | y, color);
				}
			}
		}
	}
	assert(prefixOffset == prefixData.size());
}

namespace {
#define LE_CENTRIC_MUL(dst, src, scale, dc)                     \
	{                                                           \
		unsigned tmpval = READ_LE_UINT32(src) * (scale) + (dc); \
		*(dst) = tmpval;                                        \
	}

void mcdc(uint16_t *dst, const uint16_t *src, int log2w,
		  int h, int stride, int scale, unsigned dc) {
	int i;
	dc *= 0x10001;

	switch (log2w) {
	case 0:
		for (i = 0; i < h; i++) {
			dst[0] = scale * src[0] + dc;
			if (scale)
				src += stride;
			dst += stride;
		}
		break;
	case 1:
		for (i = 0; i < h; i++) {
			LE_CENTRIC_MUL(dst, src, scale, dc);
			if (scale)
				src += stride;
			dst += stride;
		}
		break;
	case 2:
		for (i = 0; i < h; i++) {
			LE_CENTRIC_MUL(dst, src, scale, dc);
			LE_CENTRIC_MUL(dst + 2, src + 2, scale, dc);
			if (scale)
				src += stride;
			dst += stride;
		}
		break;
	case 3:
		for (i = 0; i < h; i++) {
			LE_CENTRIC_MUL(dst, src, scale, dc);
			LE_CENTRIC_MUL(dst + 2, src + 2, scale, dc);
			LE_CENTRIC_MUL(dst + 4, src + 4, scale, dc);
			LE_CENTRIC_MUL(dst + 6, src + 6, scale, dc);
			if (scale)
				src += stride;
			dst += stride;
		}
		break;
	default:
		error("invalid log2w");
	}
}
} // namespace

void FourXMDecoder::FourXMVideoTrack::decode_pfrm_block(Graphics::Surface *frame, int x, int y, int log2w, int log2h, FourXM::BEByteBitStream &bs, Common::MemoryReadStream &wordStream, Common::MemoryReadStream &byteStream) {
	assert(log2w >= 0 && log2h >= 0);
	auto index = size2index[log2h][log2w];
	assert(index >= 0);
	auto h = 1 << log2h, w = 1 << log2w;
	auto &huff = _blockType[_version > 1][index];
	auto code = huff.next(bs);
	int scale = 0;
	int dc = 0;
	if (code == 5) {
		dc = wordStream.readSint16LE();
	} else {
		error("invalid code %d", code);
	}
	mcdc(static_cast<uint16 *>(frame->getBasePtr(x, y)), static_cast<const uint16 *>(_frame->getBasePtr(x, y)), log2w, h, frame->pitch / frame->format.bytesPerPixel, scale, dc);
}

void FourXMDecoder::FourXMVideoTrack::decode_pfrm(Common::SeekableReadStream *stream) {
	stream->skip(12);
	auto bitStreamSize = stream->readUint32LE();
	auto wordStreamSize = stream->readUint32LE();
	auto byteStreamSize = stream->readUint32LE();
	debug("p-frame, bitstream: %u, wordstream: %u, bytestream: %u", bitStreamSize, wordStreamSize, byteStreamSize);

	Common::Array<byte> bitStreamData(bitStreamSize);
	stream->read(bitStreamData.data(), bitStreamData.size());
	Common::Array<byte> wordStreamData(wordStreamSize);
	stream->read(wordStreamData.data(), wordStreamData.size());
	Common::Array<byte> byteStreamData(byteStreamSize);
	stream->read(byteStreamData.data(), byteStreamData.size());

	FourXM::BEByteBitStream bitStream(bitStreamData.data(), bitStreamData.size(), 0);
	Common::MemoryReadStream wordStream(wordStreamData.data(), wordStreamData.size());
	Common::MemoryReadStream byteStream(byteStreamData.data(), byteStreamData.size());

	Common::ScopedPtr<Graphics::Surface> frame(new Graphics::Surface());
	frame->copyFrom(*_frame);
	for (int y = 0, h = _frame->h; y < h; y += 8) {
		for (int x = 0, w = _frame->w; x < w; x += 8) {
			decode_pfrm_block(frame.get(), x, y, 3, 3, bitStream, wordStream, byteStream);
		}
	}
	_frame = frame.release();
}

void FourXMDecoder::FourXMVideoTrack::decode_cfrm(Common::SeekableReadStream *stream) {
	stream->skip(4);
	auto frameIdx = stream->readUint32LE();
	auto frameSize = stream->readUint32LE();
	debug("c-frame, frame id: %u, size: %u", frameIdx, frameSize);
}

void FourXMDecoder::FourXMVideoTrack::decode(uint32 tag, byte *buf, uint size) {
	Common::MemoryReadStream ms(buf, size, DisposeAfterUse::YES);
	switch (tag) {
	case MKTAG('i', 'f', 'r', 'm'):
		decode_ifrm(&ms);
		break;
	case MKTAG('p', 'f', 'r', 'm'):
		decode_pfrm(&ms);
		break;
	case MKTAG('c', 'f', 'r', 'm'):
		decode_cfrm(&ms);
		break;
	default:
		warning("uknown video frame %s", tagName(tag).c_str());
		break;
	}
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
			if (!_audio)
				error("no audio track but got sound frame");
			switch (_audio->getAudioType()) {
			case 0: {
				uint offset = 0;
				while (offset < size) {
					auto trackIdx = _stream->readUint32LE();
					auto packetSize = _stream->readUint32LE();
					if (trackIdx == 0 && _audio) {
						_audio->decode(tag, loadBuf(packetSize), packetSize);
					} else {
						_stream->skip(packetSize);
						offset += packetSize + 8;
					}
				}
			} break;
			case 1: {
				auto trackIdx = _stream->readUint32LE();
				_stream->skip(4);
				if (trackIdx == 0 && _audio) {
					_audio->decode(tag, loadBuf(size - 8), size - 8);
				} else {
					_stream->skip(size - 8);
				}
			} break;
			default:
				warning("unknown audio type");
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
				_stream->skip(8);
				auto extra = _stream->readUint32LE();
				_stream->skip(16);
				auto w = _stream->readUint32LE();
				auto h = _stream->readUint32LE();
				debug("video %ux%u, version: %u", w, h, extra >> 16);
				addTrack(_video = new FourXMVideoTrack(this, _frameRate, w, h, extra >> 16));
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
