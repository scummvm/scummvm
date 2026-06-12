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
#include "audio/decoders/adpcm_intern.h"
#include "audio/decoders/raw.h"
#include "common/bitstream.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/managed_surface.h"
#include "video/4xm_utils.h"

namespace Video {

FourXMDecoder::~FourXMDecoder() {
	close();
}

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

static const int8_t mv[256][2] = {
	{0, 0}, {0, -1}, {-1, 0}, {1, 0}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}, {0, -2}, {-2, 0}, {2, 0}, {0, 2}, {-1, -2}, {1, -2}, {-2, -1}, {2, -1}, {-2, 1}, {2, 1}, {-1, 2}, {1, 2}, {-2, -2}, {2, -2}, {-2, 2}, {2, 2}, {0, -3}, {-3, 0}, {3, 0}, {0, 3}, {-1, -3}, {1, -3}, {-3, -1}, {3, -1}, {-3, 1}, {3, 1}, {-1, 3}, {1, 3}, {-2, -3}, {2, -3}, {-3, -2}, {3, -2}, {-3, 2}, {3, 2}, {-2, 3}, {2, 3}, {0, -4}, {-4, 0}, {4, 0}, {0, 4}, {-1, -4}, {1, -4}, {-4, -1}, {4, -1}, {4, 1}, {-1, 4}, {1, 4}, {-3, -3}, {-3, 3}, {3, 3}, {-2, -4}, {-4, -2}, {4, -2}, {-4, 2}, {-2, 4}, {2, 4}, {-3, -4}, {3, -4}, {4, -3}, {-5, 0}, {-4, 3}, {-3, 4}, {3, 4}, {-1, -5}, {-5, -1}, {-5, 1}, {-1, 5}, {-2, -5}, {2, -5}, {5, -2}, {5, 2}, {-4, -4}, {-4, 4}, {-3, -5}, {-5, -3}, {-5, 3}, {3, 5}, {-6, 0}, {0, 6}, {-6, -1}, {-6, 1}, {1, 6}, {2, -6}, {-6, 2}, {2, 6}, {-5, -4}, {5, 4}, {4, 5}, {-6, -3}, {6, 3}, {-7, 0}, {-1, -7}, {5, -5}, {-7, 1}, {-1, 7}, {4, -6}, {6, 4}, {-2, -7}, {-7, 2}, {-3, -7}, {7, -3}, {3, 7}, {6, -5}, {0, -8}, {-1, -8}, {-7, -4}, {-8, 1}, {4, 7}, {2, -8}, {-2, 8}, {6, 6}, {-8, 3}, {5, -7}, {-5, 7}, {8, -4}, {0, -9}, {-9, -1}, {1, 9}, {7, -6}, {-7, 6}, {-5, -8}, {-5, 8}, {-9, 3}, {9, -4}, {7, -7}, {8, -6}, {6, 8}, {10, 1}, {-10, 2}, {9, -5}, {10, -3}, {-8, -7}, {-10, -4}, {6, -9}, {-11, 0}, {11, 1}, {-11, -2}, {-2, 11}, {7, -9}, {-7, 9}, {10, 6}, {-4, 11}, {8, -9}, {8, 9}, {5, 11}, {7, -10}, {12, -3}, {11, 6}, {-9, -9}, {8, 10}, {5, 12}, {-11, 7}, {13, 2}, {6, -12}, {10, 9}, {-11, 8}, {-7, 12}, {0, 14}, {14, -2}, {-9, 11}, {-6, 13}, {-14, -4}, {-5, -14}, {5, 14}, {-15, -1}, {-14, -6}, {3, -15}, {11, -11}, {-7, 14}, {-5, 15}, {8, -14}, {15, 6}, {3, 16}, {7, -15}, {-16, 5}, {0, 17}, {-16, -6}, {-10, 14}, {-16, 7}, {12, 13}, {-16, 8}, {-17, 6}, {-18, 3}, {-7, 17}, {15, 11}, {16, 10}, {2, -19}, {3, -19}, {-11, -16}, {-18, 8}, {-19, -6}, {2, -20}, {-17, -11}, {-10, -18}, {8, 19}, {-21, -1}, {-20, 7}, {-4, 21}, {21, 5}, {15, 16}, {2, -22}, {-10, -20}, {-22, 5}, {20, -11}, {-7, -22}, {-12, 20}, {23, -5}, {13, -20}, {24, -2}, {-15, 19}, {-11, 22}, {16, 19}, {23, -10}, {-18, -18}, {-9, -24}, {24, -10}, {-3, 26}, {-23, 13}, {-18, -20}, {17, 21}, {-4, 27}, {27, 6}, {1, -28}, {-11, 26}, {-17, -23}, {7, 28}, {11, -27}, {29, 5}, {-23, -19}, {-28, -11}, {-21, 22}, {-30, 7}, {-17, 26}, {-27, 16}, {13, 29}, {19, -26}, {10, -31}, {-14, -30}, {20, -27}, {-29, 18}, {-16, -31}, {-28, -22}, {21, -30}, {-25, 28}, {26, -29}, {25, -32}, {-32, -32}};

} // namespace

namespace {

enum Raw4XMChunkId {
	kRaw4XMFile = 0xfb814000,
	kRaw4XMFrameContainer = 0xfb814100,
	kRaw4XMFullFrame = 0xfb814210,
	kRaw4XMDeltaFrame = 0xfb814220,
	kRaw4XMCompressedAudio = 0xfb814230,
	kRaw4XMCachedFrame = 0xfb814240,
	kRaw4XMRawAudio = 0xfb814250
};

struct Raw4XMCacheEntry {
	int32 frame = -1;
	uint32 declaredSize = 0;
	Common::Array<byte> data;
};

struct Raw4XMDeltaDecoder {
	FourXM::RawDeltaReader reader;
	uint16 *dst = nullptr;
	const uint16 *src = nullptr;
	int frameWidth = 0;
	int frameHeight = 0;
	const Common::Array<int> &fullOffsets;
	const Common::Array<int> &expOffsets;

	Raw4XMDeltaDecoder(const byte *data, uint32 size, uint16 *dstPixels, const uint16 *srcPixels,
					   int width, int height, const Common::Array<int> &fullMotionOffsets, const Common::Array<int> &expMotionOffsets) : reader(data, size), dst(dstPixels), src(srcPixels), frameWidth(width), frameHeight(height),
																																		 fullOffsets(fullMotionOffsets), expOffsets(expMotionOffsets) {}

	bool copyLeaf(int dstOffset, int blockWidth, int blockHeight, uint32 op) {
		uint32 motionIndex = 0;
		uint32 transform = 0;
		uint32 addFlag = 0;
		if (reader.mode == 0) {
			uint16 packed = reader.readPair();
			motionIndex = packed & 0xfff;
			transform = (packed >> 12) & 7;
			addFlag = packed >> 15;
		} else {
			motionIndex = reader.readByteIndex();
			uint32 packed = reader.readNibble();
			transform = packed & 7;
			addFlag = (packed & 0xf) >> 3;
		}

		int add = 0;
		if (op == 3 && !addFlag)
			return true;

		if (addFlag) {
			uint16 packed = reader.readPair();
			if (op == 3)
				add = (((int8)(((packed >> 5) & 0x1f) - 0x10) & 0xf) * 0x40 +
					   ((int8)(((packed >> 10) & 0x1f) - 0x10) & 0xf) * 0x800 +
					   ((byte)((((byte)packed & 0x1f) - 0x10) * 2) & 0x1f));
			else
				add = ((((int8)(((packed >> 10) & 0x1f) - 0x10)) & 0x1f) * 0x20 +
					   (((int8)(((packed >> 5) & 0x1f) - 0x10)) & 0x1f)) *
						  0x20 +
					  (((int8)((packed & 0x1f) - 0x10)) & 0x1f);
		}

		const Common::Array<int> &motionTable = reader.mode == 0 ? fullOffsets : expOffsets;
		int srcOffset = dstOffset;
		if (motionIndex < motionTable.size())
			srcOffset += motionTable[motionIndex];
		const int sourceWidth = transform < 4 ? blockWidth : blockHeight;
		const int sourceHeight = transform < 4 ? blockHeight : blockWidth;
		if (srcOffset < 0 || srcOffset + frameWidth * (sourceHeight - 1) + sourceWidth - 1 >= frameWidth * frameHeight)
			return false;

		FourXM::copyRawBlock(dst + dstOffset, src + srcOffset, frameWidth, blockWidth, blockHeight, transform, add);
		return true;
	}

	bool decodeDctBlock(int dstOffset) {
		byte scaleY = reader.readNibble();
		byte scaleCb = reader.readNibble();
		byte scaleCr = reader.readNibble();
		int coeffs[3][64] = {};
		int blocks[3][64] = {};
		FourXM::readRawCoefficients(reader, coeffs);
		FourXM::transformRawCoefficients(coeffs[0], blocks[0], scaleY, false);
		FourXM::transformRawCoefficients(coeffs[1], blocks[1], scaleCb, true);
		FourXM::transformRawCoefficients(coeffs[2], blocks[2], scaleCr, true);
		FourXM::writeRawDctBlock(blocks[0], blocks[1], blocks[2], dst + dstOffset, frameWidth);
		return true;
	}

	bool decodeCopyBlock(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		return copyLeaf(dstOffset, blockWidth, blockHeight, 0);
	}

	bool decodeVerticalSplit(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		if (blockHeight < 2)
			return false;
		decodeBlock(dstOffset, blockWidth, blockHeight / 2, false);
		decodeBlock(dstOffset + frameWidth * (blockHeight / 2), blockWidth, blockHeight / 2, false);
		return true;
	}

	bool decodeHorizontalSplit(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		if (blockWidth < 2)
			return false;
		decodeBlock(dstOffset, blockWidth / 2, blockHeight, false);
		decodeBlock(dstOffset + blockWidth / 2, blockWidth / 2, blockHeight, false);
		return true;
	}

	bool decodeDctOrCopyBlock(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		uint32 subOp = reader.readControl2();
		if (subOp == 0 && allowDct)
			return decodeDctBlock(dstOffset);
		if (subOp == 2)
			return copyLeaf(dstOffset, blockWidth, blockHeight, 3);
		return false;
	}

	bool decodeBlock(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		typedef bool (Raw4XMDeltaDecoder::*BlockOp)(int, int, int, bool);
		static const BlockOp kBlockOps[4] = {
			&Raw4XMDeltaDecoder::decodeCopyBlock,
			&Raw4XMDeltaDecoder::decodeVerticalSplit,
			&Raw4XMDeltaDecoder::decodeHorizontalSplit,
			&Raw4XMDeltaDecoder::decodeDctOrCopyBlock};

		return (this->*kBlockOps[reader.readControl2() & 3])(dstOffset, blockWidth, blockHeight, allowDct);
	}
};

static bool applyRaw4XMDelta(const byte *data, uint32 size, uint16 *dst, const uint16 *src,
							 int width, int height, const Common::Array<int> &fullOffsets, const Common::Array<int> &expOffsets) {
	Raw4XMDeltaDecoder decoder(data, size, dst, src, width, height, fullOffsets, expOffsets);
	if (!decoder.reader.valid())
		return false;

	const int blocksX = (width + 7) / 8;
	const int blocksY = (height + 7) / 8;
	int blockOffset = 0;

	for (int y = 0; y < blocksY; ++y) {
		for (int x = 0; x < blocksX; ++x) {
			decoder.decodeBlock(blockOffset, 8, 8, true);
			blockOffset += 8;
		}
		blockOffset += width * 7;
	}

	return true;
}

static Raw4XMCacheEntry &raw4XMFindCacheEntry(Common::Array<Raw4XMCacheEntry> &cache, int32 frame) {
	for (uint i = 0; i < cache.size(); ++i) {
		if (cache[i].frame == frame)
			return cache[i];
	}

	for (uint i = 0; i < cache.size(); ++i) {
		if (cache[i].frame == -1) {
			cache[i].frame = frame;
			cache[i].declaredSize = 0;
			cache[i].data.clear();
			return cache[i];
		}
	}

	cache[0].frame = frame;
	cache[0].declaredSize = 0;
	cache[0].data.clear();
	return cache[0];
}

Audio::PacketizedAudioStream *makeAudioStream(byte audioType, uint audioChannels, uint sampleRate) {
	switch (audioType) {
	case 0: {
		byte flags = Audio::FLAG_16BITS;
#ifdef SCUMM_LITTLE_ENDIAN
		flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
		if (audioChannels > 1)
			flags |= Audio::FLAG_STEREO;
		return Audio::makePacketizedRawStream(sampleRate, flags);
	}
	case 1:
		return Audio::makePacketizedADPCMStream(Audio::ADPCMType::kADPCM4XM, sampleRate, audioChannels);
	default:
		error("FourXMAudioTrack: unknown audio type: %d", audioType);
	}
}

} // namespace

class FourXMDecoder::FourXMAudioTrack : public AudioTrack {
	uint _audioType;
	Common::ScopedPtr<Audio::PacketizedAudioStream> _output;

public:
	FourXMAudioTrack(byte audioType, uint audioChannels, uint sampleRate) : AudioTrack(Audio::Mixer::SoundType::kPlainSoundType), _audioType(audioType),
																			_output(makeAudioStream(audioType, audioChannels, sampleRate)) {}

	byte getAudioType() const { return _audioType; }

	void decode(Common::SeekableReadStream *input) {
		_output->queuePacket(input);
	}

private:
	Audio::AudioStream *getAudioStream() const override { return _output.get(); }
};

class FourXMDecoder::FourXMRawAudioTrack : public AudioTrack {
	uint _audioType;
	uint _channels;
	uint _bits;
	int16 _predictor[2] = {0, 0};
	int _stepIndex[2] = {0, 0};
	Common::ScopedPtr<Audio::QueuingAudioStream> _output;

public:
	FourXMRawAudioTrack(uint audioType, uint channels, uint sampleRate, uint bits) : AudioTrack(Audio::Mixer::SoundType::kPlainSoundType), _audioType(audioType),
																					 _channels(channels), _bits(bits) {
		if ((audioType == 0 || audioType == 1) && sampleRate > 0 && (channels == 1 || channels == 2) && bits == 16)
			_output.reset(Audio::makeQueuingAudioStream(sampleRate, channels == 2));
	}

	void queueRaw(const byte *payload, uint32 payloadSize) {
		if (!_output || payloadSize < 4)
			return;

		byte *data = (byte *)malloc(payloadSize - 4);
		if (!data)
			return;

		Common::copy(payload + 4, payload + payloadSize, data);
		_output->queueBuffer(data, payloadSize - 4, DisposeAfterUse::YES, audioFlags());
	}

	void queueADPCM(const byte *payload, uint32 payloadSize) {
		if (!_output || _audioType != 1 || _bits != 16 || payloadSize < 4)
			return;

		const uint32 decodedBytes = READ_LE_UINT32(payload);
		if (decodedBytes == 0)
			return;

		const uint32 samplesPerChannel = decodedBytes / (_channels * 2);
		if (samplesPerChannel == 0)
			return;

		byte *out = (byte *)malloc(decodedBytes);
		if (!out)
			return;
		Common::fill(out, out + decodedBytes, 0);
		int16 *pcm = (int16 *)out;

		const byte *src = payload + 4;
		const uint32 srcSize = payloadSize - 4;
		if (_channels == 1) {
			decodeADPCMChannel(src, srcSize, 0, pcm, samplesPerChannel);
		} else {
			Common::Array<int16> left;
			Common::Array<int16> right;
			left.resize(samplesPerChannel);
			right.resize(samplesPerChannel);

			const uint32 compressedChannelSize = ((samplesPerChannel + 7) / 8) * 4;
			decodeADPCMChannel(src, MIN<uint32>(srcSize, compressedChannelSize), 0, left.data(), samplesPerChannel);
			if (srcSize > compressedChannelSize)
				decodeADPCMChannel(src + compressedChannelSize, srcSize - compressedChannelSize, 1, right.data(), samplesPerChannel);

			for (uint32 i = 0; i < samplesPerChannel; ++i) {
				pcm[i * 2] = left[i];
				pcm[i * 2 + 1] = right[i];
			}
		}

		_output->queueBuffer(out, decodedBytes, DisposeAfterUse::YES, audioFlags());
	}

private:
	byte audioFlags() const {
		byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
		if (_channels == 2)
			flags |= Audio::FLAG_STEREO;
		return flags;
	}

	int16 decodeNibble(byte nibble, uint channel) {
		const int step = Audio::Ima_ADPCMStream::_imaTable[_stepIndex[channel]];
		const int diff = ((step / 2) + (nibble & 7) * step) >> 3;
		const int sample = CLIP<int>(_predictor[channel] + ((nibble & 8) ? -diff : diff), -32768, 32767);
		_predictor[channel] = sample;
		_stepIndex[channel] = CLIP<int>(_stepIndex[channel] + Audio::ADPCMStream::_stepAdjustTable[nibble], 0, 88);
		return _predictor[channel];
	}

	void decodeADPCMChannel(const byte *src, uint32 srcSize, uint channel, int16 *dst, uint32 samples) {
		for (uint32 i = 0; i < samples; ++i) {
			uint32 byteOffset = (i >> 3) * 4 + ((i & 7) >> 1);
			if (byteOffset >= srcSize)
				break;

			byte packed = src[byteOffset];
			byte nibble = (i & 1) ? (packed >> 4) : (packed & 0xf);
			dst[i] = decodeNibble(nibble, channel);
		}
	}

	Audio::AudioStream *getAudioStream() const override { return _output.get(); }
};

class FourXMDecoder::FourXMVideoTrack : public FixedRateVideoTrack {
	FourXMDecoder *_dec;
	Common::Rational _frameRate;
	uint _w, _h;
	uint16 _version = 0;
	Common::ScopedPtr<Graphics::ManagedSurface> _framePtr, _lastFramePtr;
	Graphics::Surface *_frame = nullptr, *_lastFrame = nullptr;
	using HuffmanBitStream = Common::BitStreamMemory32LEMSB;
	using HuffmanType = Common::Huffman<HuffmanBitStream>;
	Common::ScopedPtr<HuffmanType> _blockType[4];
	int _mv[256];
	Common::HashMap<byte, Common::Array<byte>> _cframes;

public:
	FourXMVideoTrack(FourXMDecoder *dec, const Common::Rational &frameRate, uint w, uint h, uint16 version) : _dec(dec), _frameRate(frameRate), _w(w), _h(h), _version(version) {
		_blockType[0].reset(new HuffmanType(HuffmanType::fromFrequencies({16, 8, 4, 2, 1, 1})));
		_blockType[1].reset(new HuffmanType(HuffmanType::fromFrequencies({8, 0, 4, 2, 1, 1})));
		_blockType[2].reset(new HuffmanType(HuffmanType::fromFrequencies({8, 4, 0, 2, 1, 1})));
		_blockType[3].reset(new HuffmanType(HuffmanType::fromFrequencies({8, 0, 0, 4, 2, 1, 1})));
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

	void decode(uint32 tag, Common::SeekableReadStream *stream);
	void decode_ifrm(Common::SeekableReadStream *stream);
	void decode_pfrm(Common::SeekableReadStream *stream);
	void decode_cfrm(Common::SeekableReadStream *stream);

private:
	void decode_pfrm_block(uint16 *dst, const uint16 *src, int stride, int log2w, int log2h, Common::BitStreamMemory32LEMSB &bitStream, Common::MemoryReadStream &wordStream, Common::MemoryReadStream &byteStream);
	Common::Rational getFrameRate() const override { return _frameRate; }
};

const Graphics::Surface *FourXMDecoder::FourXMVideoTrack::decodeNextFrame() {
	if (!_framePtr) {
		_framePtr.reset(new Graphics::ManagedSurface());
		_framePtr->create(_w, _h, getPixelFormat());
		_frame = _framePtr->surfacePtr();
		_lastFramePtr.reset(new Graphics::ManagedSurface());
		_lastFramePtr->create(_w, _h, getPixelFormat());
		_lastFrame = _lastFramePtr->surfacePtr();
		assert(_framePtr->pitch == _lastFramePtr->pitch);
		auto pitch = _framePtr->pitch / _framePtr->format.bytesPerPixel;

		for (uint i = 0; i < 256; i++)
			_mv[i] = mv[i][0] + mv[i][1] * pitch;
	}
	_dec->decodeNextFrameImpl();
	return _lastFrame;
}

int FourXMDecoder::FourXMVideoTrack::getCurFrame() const {
	return _dec->_curFrame;
}

int FourXMDecoder::FourXMVideoTrack::getFrameCount() const {
	return _dec->_frames.size();
}

FourXMDecoder::FourXMVideoTrack::~FourXMVideoTrack() = default;

class FourXMDecoder::FourXMRawVideoTrack : public FixedRateVideoTrack {
	FourXMDecoder *_dec;
	Common::Rational _frameRate;
	uint _w, _h;
	Common::ScopedPtr<Graphics::ManagedSurface> _surface;
	Common::Array<uint16> _frameBuffer1;
	Common::Array<uint16> _frameBuffer2;
	Common::Array<uint16> *_frame = nullptr;
	Common::Array<uint16> *_previousFrame = nullptr;
	Common::Array<int> _fullMotionOffsets;
	Common::Array<int> _expMotionOffsets;
	Common::Array<Raw4XMCacheEntry> _cache;

public:
	FourXMRawVideoTrack(FourXMDecoder *dec, const Common::Rational &frameRate, uint w, uint h) : _dec(dec), _frameRate(frameRate), _w(w), _h(h) {
		_surface.reset(new Graphics::ManagedSurface());
		_surface->create(w, h, getPixelFormat());
		_frameBuffer1.resize(w * h);
		_frameBuffer2.resize(w * h);
		_frame = &_frameBuffer1;
		_previousFrame = &_frameBuffer2;
		_cache.resize(256);
		FourXM::buildRawMotionTables(w, _fullMotionOffsets, _expMotionOffsets);
	}

	uint16 getWidth() const override { return _w; }
	uint16 getHeight() const override { return _h; }

	Graphics::PixelFormat getPixelFormat() const override {
		return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	}

	int getCurFrame() const override { return _dec->_curFrame; }
	int getFrameCount() const override { return _dec->_frames.size(); }

	const Graphics::Surface *decodeNextFrame() override {
		if (_dec->_curFrame >= _dec->_frames.size())
			return _surface->surfacePtr();

		const FourXMDecoder::Frame &frameInfo = _dec->_frames[_dec->_curFrame];
		_dec->_stream->seek(frameInfo.offset);
		const uint32 chunkId = _dec->_stream->readUint32LE();
		const uint32 chunkSize = _dec->_stream->readUint32LE();
		if (chunkId != kRaw4XMFrameContainer || chunkSize < 8 || frameInfo.offset + chunkSize > frameInfo.end) {
			warning("invalid raw 4XM frame at offset %" PRId64, frameInfo.offset);
			++_dec->_curFrame;
			return _surface->surfacePtr();
		}

		Common::Array<byte> payload;
		payload.resize(chunkSize - 8);
		_dec->_stream->read(payload.data(), payload.size());
		const bool changed = decodeContainerPayload(payload.data(), payload.size(), _dec->_curFrame);
		if (changed) {
			copyFrameToSurface();
			SWAP(_frame, _previousFrame);
		}

		++_dec->_curFrame;
		return _surface->surfacePtr();
	}

private:
	Common::Rational getFrameRate() const override { return _frameRate; }

	void copyFrameToSurface() {
		Graphics::Surface frame;
		frame.init(_w, _h, _w * sizeof(uint16), _frame->data(), Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));
		_surface->convertFrom(frame, getPixelFormat());
	}

	bool decodeFullFrame(const byte *payload, uint32 payloadSize) {
		if (payloadSize < _w * _h * 2)
			return false;

		Common::MemoryReadStream fullFrameStream(payload, payloadSize);
		for (uint i = 0; i < _w * _h; ++i)
			(*_frame)[i] = fullFrameStream.readUint16LE();
		return true;
	}

	bool decodeCachedFrame(const byte *payload, uint32 payloadSize, uint32 currentFrame) {
		if (payloadSize < 8)
			return false;

		const int32 cacheFrame = (int32)READ_LE_UINT32(payload);
		const uint32 declaredSize = READ_LE_UINT32(payload + 4);
		Raw4XMCacheEntry &entry = findCacheEntry(cacheFrame);
		entry.declaredSize = declaredSize;
		const uint oldSize = entry.data.size();
		entry.data.resize(oldSize + payloadSize - 8);
		Common::copy(payload + 8, payload + payloadSize, entry.data.begin() + oldSize);

		if (cacheFrame != (int32)currentFrame || entry.data.size() < 8 ||
			READ_LE_UINT32(entry.data.data()) != kRaw4XMFrameContainer)
			return false;

		const uint32 nestedDeclaredSize = entry.declaredSize >= 8 ? entry.declaredSize : READ_LE_UINT32(entry.data.data() + 4);
		const uint32 nestedPayloadSize = MIN<uint32>(entry.data.size() - 8, nestedDeclaredSize - 8);
		const bool changed = decodeContainerPayload(entry.data.data() + 8, nestedPayloadSize, currentFrame);

		entry.frame = -1;
		entry.declaredSize = 0;
		entry.data.clear();
		return changed;
	}

	bool decodeContainerPayload(const byte *payload, uint32 payloadSize, uint32 currentFrame) {
		bool changed = false;
		uint32 pos = 0;
		while (pos + 8 <= payloadSize) {
			const uint32 subChunkId = READ_LE_UINT32(payload + pos);
			const uint32 subChunkSize = READ_LE_UINT32(payload + pos + 4);
			if (subChunkSize < 8 || pos + subChunkSize > payloadSize)
				break;

			const byte *subPayload = payload + pos + 8;
			const uint32 subPayloadSize = subChunkSize - 8;
			if (subChunkId == kRaw4XMFullFrame) {
				changed = decodeFullFrame(subPayload, subPayloadSize) || changed;
			} else if (subChunkId == kRaw4XMDeltaFrame) {
				changed = applyRaw4XMDelta(subPayload, subPayloadSize, _frame->data(), _previousFrame->data(), _w, _h,
										   _fullMotionOffsets, _expMotionOffsets) ||
						  changed;
			} else if (subChunkId == kRaw4XMCachedFrame) {
				changed = decodeCachedFrame(subPayload, subPayloadSize, currentFrame) || changed;
			} else if (subChunkId == kRaw4XMCompressedAudio && _dec->_rawAudio) {
				_dec->_rawAudio->queueADPCM(subPayload, subPayloadSize);
			} else if (subChunkId == kRaw4XMRawAudio && _dec->_rawAudio) {
				_dec->_rawAudio->queueRaw(subPayload, subPayloadSize);
			}

			pos += subChunkSize;
		}

		return changed;
	}

	Raw4XMCacheEntry &findCacheEntry(int32 frame) {
		return raw4XMFindCacheEntry(_cache, frame);
	}
};

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
	auto bitstreamSize = stream->readUint32LE();

	Common::Array<byte> bitstreamData(bitstreamSize);
	stream->read(bitstreamData.data(), bitstreamData.size());

	auto prefixSize = stream->readUint32LE();
	/* auto tokenCount = */ stream->readUint32LE();

	Common::Array<byte> prefixStream(prefixSize * 4);
	stream->read(prefixStream.data(), prefixStream.size());
	assert(stream->pos() == stream->size());

	const auto *huffPtr = prefixStream.data();
	uint huffOffset = 0;
	auto prefixDecoder = FourXM::loadStatistics<HuffmanType>(huffPtr, huffOffset);
	huffOffset = (huffOffset + 3) & ~3u;

	Common::BitStreamMemoryStream huffMs(huffPtr + huffOffset, prefixStream.size() - huffOffset);
	Common::BitStreamMemory32LEMSB huffBs{&huffMs};

	Common::BitStreamMemoryStream bitstreamInput(bitstreamData.data(), bitstreamData.size());
	Common::BitStreamMemory8MSB bitstream(&bitstreamInput);
	int lastDC = 0;
	auto &format = _frame->format;
	const auto dstPitch = _frame->pitch / format.bytesPerPixel - 16;
	for (int mbY = 0; mbY < _frame->h; mbY += 16) {
		for (int mbX = 0; mbX < _frame->w; mbX += 16) {
			int16_t block[6][64] = {};
			auto readBlock = [&](byte blockIdx, int16_t *ac) {
				int dc = prefixDecoder.getSymbol(huffBs);
				if (dc >> 4)
					error("dc run code");
				dc = FourXM::readInt(bitstream, dc);
				dc = lastDC + dc * iquant[0];
				lastDC = dc;
				ac[0] = dc;
				for (uint idx = 1; idx < 64;) {
					auto b = prefixDecoder.getSymbol(huffBs);
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
							ac[ac_idx] = iquant[ac_idx] * FourXM::readInt(bitstream, l);
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
			auto *dst = static_cast<uint16 *>(_frame->getBasePtr(mbX, mbY));
			for (byte y = 0; y != 16; ++y, dst += dstPitch) {
				for (byte x = 0; x != 16; ++x) {
					auto yb = y & 7;
					auto xb = x & 7;
					auto mbBlockIdx = ((y >> 3) << 1) | (x >> 3);
					auto Y = block[mbBlockIdx][acIdx(yb, xb)];
					auto cblockIdx = acIdx(y >> 1, x >> 1);
					auto CB = blockCB[cblockIdx];
					auto CR = blockCR[cblockIdx];
					int CG = (CB + CR) >> 1;
					CB *= 2;
					auto color = format.RGBToColor(Y + CR, Y - CG, Y + CB);
					*dst++ = color;
				}
			}
		}
	}
	SWAP(_frame, _lastFrame);
}

namespace {

template<bool Scale>
void mcdc(uint16_t *dst, const uint16_t *src, int log2w,
		  int log2h, int stride, uint dc) {
	dc |= dc << 16;
	int h = 1 << log2h;
	if (Scale) {
		for (int i = 0; i < h; ++i) {
			switch (log2w) {
			case 3:
				WRITE_UINT32(dst + 4, READ_UINT32(src + 4) + dc);
				WRITE_UINT32(dst + 6, READ_UINT32(src + 6) + dc);
				// fall through
			case 2:
				WRITE_UINT32(dst + 2, READ_UINT32(src + 2) + dc);
				// fall through
			case 1:
				WRITE_UINT32(dst, READ_UINT32(src) + dc);
				break;
			case 0:
				*dst = *src + dc;
			}
			src += stride;
			dst += stride;
		}
	} else {
		for (int i = 0; i < h; ++i) {
			switch (log2w) {
			case 3:
				WRITE_UINT32(dst + 4, dc);
				WRITE_UINT32(dst + 6, dc);
				// fall through
			case 2:
				WRITE_UINT32(dst + 2, dc);
				// fall through
			case 1:
				WRITE_UINT32(dst, dc);
				break;
			case 0:
				*dst = dc;
			}
			dst += stride;
		}
	}
}

} // namespace

void FourXMDecoder::FourXMVideoTrack::decode_pfrm_block(uint16 *dst, const uint16 *src, int stride, int log2w, int log2h, Common::BitStreamMemory32LEMSB &bs, Common::MemoryReadStream &wordStream, Common::MemoryReadStream &byteStream) {
	assert(log2w >= 0 && log2h >= 0);
	auto index = size2index[log2h][log2w];
	assert(index >= 0);
	auto &huff = *_blockType[index];
	auto code = huff.getSymbol(bs);

	if (code == 1) {
		--log2h;
		decode_pfrm_block(dst, src, stride, log2w, log2h, bs, wordStream, byteStream);
		auto offset = stride << log2h;
		decode_pfrm_block(dst + offset, src + offset, stride, log2w, log2h, bs, wordStream, byteStream);
		return;
	} else if (code == 2) {
		--log2w;
		decode_pfrm_block(dst, src, stride, log2w, log2h, bs, wordStream, byteStream);
		auto offset = 1 << log2w;
		decode_pfrm_block(dst + offset, src + offset, stride, log2w, log2h, bs, wordStream, byteStream);
		return;
	} else if (code == 6) {
		assert(wordStream.pos() + 4 <= wordStream.size());
		if (log2w) {
			dst[0] = wordStream.readUint16LE();
			dst[1] = wordStream.readUint16LE();
		} else {
			dst[0] = wordStream.readUint16LE();
			dst[stride] = wordStream.readUint16LE();
		}
		return;
	}
	if (code == 3)
		return;

	if (code == 0) {
		assert(byteStream.pos() < byteStream.size());
		src += _mv[byteStream.readByte()];
		mcdc<true>(dst, src, log2w, log2h, stride, 0);
	} else if (code == 4) {
		assert(byteStream.pos() < byteStream.size());
		assert(wordStream.pos() + 2 <= wordStream.size());
		src += _mv[byteStream.readByte()];
		auto dc = wordStream.readUint16LE();
		mcdc<true>(dst, src, log2w, log2h, stride, dc);
	} else if (code == 5) {
		assert(wordStream.pos() + 2 <= wordStream.size());
		auto dc = wordStream.readUint16LE();
		mcdc<false>(dst, src, log2w, log2h, stride, dc);
	} else {
		error("invalid code %d (steps %u,%u)", code, log2w, log2h);
	}
}

void FourXMDecoder::FourXMVideoTrack::decode_pfrm(Common::SeekableReadStream *stream) {
	stream->skip(8);
	auto bitStreamSize = stream->readUint32LE();
	auto wordStreamSize = stream->readUint32LE();
	auto byteStreamSize = stream->readUint32LE();

	Common::Array<byte> bitStreamData(bitStreamSize);
	stream->read(bitStreamData.data(), bitStreamData.size());
	Common::Array<byte> wordStreamData(wordStreamSize);
	stream->read(wordStreamData.data(), wordStreamData.size());
	Common::Array<byte> byteStreamData(byteStreamSize);
	stream->read(byteStreamData.data(), byteStreamData.size());

	Common::BitStreamMemoryStream bitStreamInput(bitStreamData.data(), bitStreamData.size());
	Common::BitStreamMemory32LEMSB bitStream(&bitStreamInput);
	Common::MemoryReadStream wordStream(wordStreamData.data(), wordStreamData.size());
	Common::MemoryReadStream byteStream(byteStreamData.data(), byteStreamData.size());

	uint16 *dst = static_cast<uint16 *>(_frame->getPixels());
	const uint16 *src = static_cast<const uint16 *>(_lastFrame->getPixels());
	assert(_frame->format.bytesPerPixel == 2);
	auto stride = _frame->pitch / _frame->format.bytesPerPixel;
	assert(stride == _frame->pitch / _frame->format.bytesPerPixel);
	for (int y = 0, h = _frame->h; y < h; y += 8) {
		for (int x = 0, w = _frame->w; x < w; x += 8) {
			decode_pfrm_block(dst + x, src + x, stride, 3, 3, bitStream, wordStream, byteStream);
		}
		dst += stride << 3;
		src += stride << 3;
	}
	SWAP(_frame, _lastFrame);
}

void FourXMDecoder::FourXMVideoTrack::decode_cfrm(Common::SeekableReadStream *stream) {
	auto frameIdx = stream->readUint32LE();
	auto frameSize = stream->readUint32LE();
	auto streamSize = stream->size() - stream->pos();
	auto &cframe = _cframes[frameIdx];
	auto dstOffset = cframe.size();
	cframe.resize(cframe.size() + streamSize);
	stream->read(cframe.data() + dstOffset, streamSize);
	if (cframe.size() >= frameSize) {
		assert(_dec->_curFrame == frameIdx);
		Common::MemoryReadStream ms(cframe.data(), cframe.size());
		decode_pfrm(&ms);
		_cframes.erase(frameIdx);
	}
}

void FourXMDecoder::FourXMVideoTrack::decode(uint32 tag, Common::SeekableReadStream *stream) {
	Common::ScopedPtr<Common::SeekableReadStream> ms(stream);
	switch (tag) {
	case MKTAG('i', 'f', 'r', 'm'):
		decode_ifrm(ms.get());
		break;
	case MKTAG('p', 'f', 'r', 'm'):
		decode_pfrm(ms.get());
		break;
	case MKTAG('c', 'f', 'r', 'm'):
		decode_cfrm(ms.get());
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
		auto pos = _stream->pos();

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
						_audio->decode(_stream->readStream(packetSize));
					} else {
						_stream->skip(packetSize);
					}
					offset += packetSize + 8;
				}
			} break;
			case 1: {
				auto trackIdx = _stream->readUint32LE();
				_stream->skip(4);
				if (trackIdx == 0 && _audio) {
					_audio->decode(_stream->readStream(size - 8));
				}
			} break;
			default:
				warning("unknown audio type");
			}
		} break;
		case MKTAG('i', 'f', 'r', 'm'):
		case MKTAG('p', 'f', 'r', 'm'):
		case MKTAG('c', 'f', 'r', 'm'): {
			auto trackIdx = _stream->readUint32LE();
			if (trackIdx == 0)
				_video->decode(tag, _stream->readStream(size - 4));
		} break;
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
	while (_stream->pos() < listEnd) {
		uint32 tag = _stream->readUint32BE();
		uint32 size = _stream->readUint32LE();
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
				addTrack(_audio = new FourXMAudioTrack(audioType, audioChannels, sampleRate));
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

bool FourXMDecoder::loadRawStream() {
	_stream->seek(0);
	if (_stream->readUint32LE() != kRaw4XMFile || _stream->readUint32LE() != 0x01000000)
		return false;

	const uint32 width = _stream->readUint32LE();
	const uint32 height = _stream->readUint32LE();
	_stream->skip(8);
	_stream->skip(4);
	const float frameRate = _stream->readFloatLE();
	const uint16 audioCodec = _stream->readUint16LE();
	const uint16 audioChannels = _stream->readUint16LE();
	const uint32 sampleRate = _stream->readUint32LE();
	_stream->skip(6);
	const uint16 bits = _stream->readUint16LE();

	if (width == 0 || height == 0)
		return false;

	_frameRate = floatToRational(frameRate > 0.0f ? frameRate : 15.0f);
	debug("raw 4XM video %ux%u, frame rate: %d/%d", width, height, _frameRate.getNumerator(), _frameRate.getDenominator());

	_frames.clear();
	_stream->seek(0x88);
	while (_stream->pos() + 8 <= _stream->size()) {
		const int64 frameOffset = _stream->pos();
		const uint32 chunkId = _stream->readUint32LE();
		const uint32 chunkSize = _stream->readUint32LE();
		const int64 frameEnd = frameOffset + chunkSize;
		if (chunkSize < 8 || frameEnd > _stream->size())
			break;

		if (chunkId == kRaw4XMFrameContainer)
			_frames.push_back({frameOffset, frameEnd});
		_stream->seek(frameEnd);
	}

	if (_frames.empty())
		return false;

	addTrack(_rawVideo = new FourXMRawVideoTrack(this, _frameRate, width, height));
	if ((audioCodec == 0 || audioCodec == 1) && sampleRate > 0 &&
		(audioChannels == 1 || audioChannels == 2) && bits == 16) {
		_rawAudio = new FourXMRawAudioTrack(audioCodec, audioChannels, sampleRate, bits);
		addTrack(_rawAudio);
	}

	return getNumTracks() != 0;
}

bool FourXMDecoder::loadStream(Common::SeekableReadStream *stream) {
	_stream.reset(stream);
	if (!stream->size()) {
		return false;
	}

	if (stream->readUint32LE() == kRaw4XMFile)
		return loadRawStream();
	stream->seek(0);

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

	while (stream->pos() < fileSize) {
		uint32 tag = stream->readUint32BE();
		uint32 size = stream->readUint32LE();
		auto pos = stream->pos();
		if (tag == MKTAG('L', 'I', 'S', 'T')) {
			readList(pos + size);
		}
		stream->seek(pos + size + (size & 1));
	}

	return getNumTracks() != 0;
}

} // namespace Video
