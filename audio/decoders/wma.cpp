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

// Based on xoreos' WMA code which is in turn
// Largely based on the WMA implementation found in FFmpeg.

#include "common/util.h"
#include "common/math.h"
#include "common/sinewindows.h"
#include "common/error.h"
#include "common/memstream.h"
#include "common/mdct.h"
#include "common/huffman.h"

#include "audio/audiostream.h"

#include "audio/decoders/util.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wma.h"
#include "audio/decoders/wmadata.h"

namespace Audio {

static inline void butterflyFloats(float *v1, float *v2, int len) {
	while (len-- > 0) {
		float t = *v1 - *v2;

		*v1++ += *v2;
		*v2++  = t;
	}
}

static inline void vectorFMulAdd(float *dst, const float *src0,
						  const float *src1, const float *src2, int len) {
	while (len-- > 0)
		*dst++ = *src0++ * *src1++ + *src2++;
}

static inline void vectorFMulReverse(float *dst, const float *src0,
									 const float *src1, int len) {
	src1 += len - 1;

	while (len-- > 0)
		*dst++ = *src0++ * *src1--;
}


WMACodec::WMACodec(int version, uint32 sampleRate, uint8 channels,
		uint32 bitRate, uint32 blockAlign, Common::SeekableReadStream *extraData) :
	_version(version), _sampleRate(sampleRate), _channels(channels),
	_bitRate(bitRate), _blockAlign(blockAlign), _audioFlags(0),
	_resetBlockLengths(true), _curFrame(0), _frameLen(0), _frameLenBits(0),
	_blockSizeCount(0), _framePos(0), _curBlock(0), _blockLen(0), _blockLenBits(0),
	_nextBlockLenBits(0), _prevBlockLenBits(0), _byteOffsetBits(0),
	_hgainHuffman(nullptr), _expHuffman(nullptr), _lastSuperframeLen(0), _lastBitoffset(0) {

	for (int i = 0; i < 2; i++) {
		_coefHuffman[i] = nullptr;

		_coefHuffmanRunTable  [i] = nullptr;
		_coefHuffmanLevelTable[i] = nullptr;
		_coefHuffmanIntTable  [i] = nullptr;
	}

	if ((_version != 1) && (_version != 2))
		error("WMACodec::init(): Unsupported WMA version %d", _version);

	if ((_sampleRate == 0) || (_sampleRate > 50000))
		error("WMACodec::init(): Invalid sample rate %d", _sampleRate);
	if ((_channels == 0) || (_channels > kChannelsMax))
		error("WMACodec::init(): Unsupported number of channels %d",
		                        _channels);

	_audioFlags = FLAG_16BITS;

#ifdef SCUMM_LITTLE_ENDIAN
	_audioFlags |= FLAG_LITTLE_ENDIAN;
#endif

	if (_channels == 2) {
		_audioFlags |= FLAG_STEREO;
	}

	init(extraData);
}

WMACodec::~WMACodec() {
	delete _expHuffman;
	delete _hgainHuffman;

	for (int i = 0; i < 2; i++) {
		delete[] _coefHuffmanRunTable  [i];
		delete[] _coefHuffmanLevelTable[i];
		delete[] _coefHuffmanIntTable  [i];

		delete _coefHuffman[i];
	}

	for (Common::Array<Common::MDCT *>::iterator m = _mdct.begin(); m != _mdct.end(); ++m)
		delete *m;
}

void WMACodec::init(Common::SeekableReadStream *extraData) {
	// Flags
	uint16 flags = getFlags(extraData);
	evalFlags(flags, extraData);

	// Frame length
	_frameLenBits = getFrameBitLength();
	_frameLen     = 1 << _frameLenBits;

	// Number of MDCT block sizes
	_blockSizeCount = getBlockSizeCount(flags);

	float bps = ((float) _bitRate) / ((float) (_channels * _sampleRate));

	_byteOffsetBits = Common::intLog2((int) (bps * _frameLen / 8.0 + 0.05)) + 2;

	// Compute high frequency value and choose if noise coding should be activated
	float highFreq;
	_useNoiseCoding = useNoiseCoding(highFreq, bps);

	// Compute the scale factor band sizes for each MDCT block size
	evalMDCTScales(highFreq);

	// Init the noise generator
	initNoise();

	// Init the coefficient huffman codes
	initCoefHuffman(bps);

	// Init MDCTs
	initMDCT();

	// Init exponent codes
	initExponents();

	// Clear the sample output buffers
	memset(_output  , 0, sizeof(_output));
	memset(_frameOut, 0, sizeof(_frameOut));
}

uint16 WMACodec::getFlags(Common::SeekableReadStream *extraData) {
	if ((_version == 1) && extraData && (extraData->size() >= 4)) {
		extraData->seek(2);
		return extraData->readUint16LE();
	}

	if ((_version == 2) && extraData && (extraData->size() >= 6)) {
		extraData->seek(4);
		return extraData->readUint16LE();
	}

	return 0;
}

void WMACodec::evalFlags(uint16 flags, Common::SeekableReadStream *extraData) {
	_useExpHuffman       = (flags & 0x0001) != 0;
	_useBitReservoir     = (flags & 0x0002) != 0;
	_useVariableBlockLen = (flags & 0x0004) != 0;

	if ((_version == 2) && extraData && (extraData->size() >= 8)) {
		extraData->seek(4);
		if ((extraData->readUint16LE() == 0x000D) && _useVariableBlockLen) {
			// Apparently, this fixes ffmpeg "issue1503"

			_useVariableBlockLen = false;
		}
	}
}

int WMACodec::getFrameBitLength() {
	if (_sampleRate <= 16000)
		return 9;

	if ((_sampleRate <= 22050) || (_sampleRate <= 32000 && _version == 1))
		return 10;

	if (_sampleRate <= 48000)
		return 11;

	if (_sampleRate <= 96000)
		return 12;

	return 13;
}

int WMACodec::getBlockSizeCount(uint16 flags) {
	if (!_useVariableBlockLen)
		return 1;

	int count = ((flags >> 3) & 3) + 1;

	if ((_bitRate / _channels) >= 32000)
		count += 2;

	const int maxCount = _frameLenBits - kBlockBitsMin;

	return MIN(count, maxCount) + 1;
}

uint32 WMACodec::getNormalizedSampleRate() {
	// Sample rates are only normalized in WMAv2
	if (_version != 2)
		return _sampleRate;

	if (_sampleRate>= 44100)
		return 44100;

	if (_sampleRate >= 22050)
		return 22050;

	if (_sampleRate >= 16000)
		return 16000;

	if (_sampleRate >= 11025)
		return 11025;

	if (_sampleRate >=  8000)
		return 8000;

	return _sampleRate;
}

bool WMACodec::useNoiseCoding(float &highFreq, float &bps) {
	highFreq = _sampleRate * 0.5;

	uint32 rateNormalized = getNormalizedSampleRate();

	float bpsOrig = bps;
	if (_channels == 2)
		bps = bpsOrig * 1.6;

	if (rateNormalized == 44100) {
		if (bps >= 0.61)
			return false;

		highFreq = highFreq * 0.4;
		return true;
	}

	if (rateNormalized == 22050) {
		if (bps >= 1.16)
			return false;

		if (bps >= 0.72)
			highFreq = highFreq * 0.7;
		else
			highFreq = highFreq * 0.6;

		return true;
	}

	if (rateNormalized == 16000) {
		if (bpsOrig > 0.5)
			highFreq = highFreq * 0.5;
		else
			highFreq = highFreq * 0.3;

		return true;
	}

	if (rateNormalized == 11025) {
		highFreq = highFreq * 0.7;
		return true;
	}

	if (rateNormalized == 8000) {
		if (bpsOrig > 0.75)
			return false;

		if (bpsOrig <= 0.625)
			highFreq = highFreq * 0.5;
		else
			highFreq = highFreq * 0.65;

		return true;
	}


	if (bpsOrig >= 0.8)
		highFreq = highFreq * 0.75;
	else if (bpsOrig >= 0.6)
		highFreq = highFreq * 0.6;
	else
		highFreq = highFreq * 0.5;

	return true;
}

void WMACodec::evalMDCTScales(float highFreq) {
	if (_version == 1)
		_coefsStart = 3;
	else
		_coefsStart = 0;

	for (int k = 0; k < _blockSizeCount; k++) {
		int blockLen = _frameLen >> k;

		if (_version == 1) {
			int i, lpos = 0;

			for (i = 0; i < 25; i++) {
				int a   = wmaCriticalFreqs[i];
				int b   = _sampleRate;
				int pos = ((blockLen * 2 * a) + (b >> 1)) / b;

				if (pos > blockLen)
					pos = blockLen;

				_exponentBands[0][i] = pos - lpos;
				if (pos >= blockLen) {
					i++;
					break;
				}
				lpos = pos;
			}

			_exponentSizes[0] = i;

		} else {
			// Hardcoded tables
			const uint8 *table = nullptr;

			int t = _frameLenBits - kBlockBitsMin - k;
			if (t < 3) {
				if (_sampleRate >= 44100)
					table = exponentBand44100[t];
				else if (_sampleRate >= 32000)
					table = exponentBand32000[t];
				else if (_sampleRate >= 22050)
					table = exponentBand22050[t];
			}

			if (table) {
				int n = *table++;

				for (int i = 0; i < n; i++)
					_exponentBands[k][i] = table[i];

				_exponentSizes[k] = n;

			} else {
				int j = 0, lpos = 0;

				for (int i = 0; i < 25; i++) {
					int a   = wmaCriticalFreqs[i];
					int b   = _sampleRate;
					int pos = ((blockLen * 2 * a) + (b << 1)) / (4 * b);

					pos <<= 2;
					if (pos > blockLen)
						pos = blockLen;

					if (pos > lpos)
						_exponentBands[k][j++] = pos - lpos;

					if (pos >= blockLen)
						break;

					lpos = pos;
				}

				_exponentSizes[k] = j;
			}

		}

		// Max number of coefs
		_coefsEnd[k] = (_frameLen - ((_frameLen * 9) / 100)) >> k;

		// High freq computation
		_highBandStart[k] = (int)((blockLen * 2 * highFreq) / _sampleRate + 0.5);

		int n   = _exponentSizes[k];
		int j   = 0;
		int pos = 0;

		for (int i = 0; i < n; i++) {
			int start, end;

			start = pos;
			pos  += _exponentBands[k][i];
			end   = pos;

			if (start < _highBandStart[k])
				start = _highBandStart[k];

			if (end > _coefsEnd[k])
				end = _coefsEnd[k];

			if (end > start)
				_exponentHighBands[k][j++] = end - start;

		}

		_exponentHighSizes[k] = j;
	}
}

void WMACodec::initNoise() {
	if (!_useNoiseCoding)
		return;

	_noiseMult  = _useExpHuffman ? 0.02 : 0.04;
	_noiseIndex = 0;

	uint  seed = 1;
	float norm = (1.0 / (float)(1LL << 31)) * sqrt(3.0f) * _noiseMult;

	for (int i = 0; i < kNoiseTabSize; i++) {
		seed = seed * 314159 + 1;

		_noiseTable[i] = (float)((int)seed) * norm;
	}

	_hgainHuffman = new HuffmanDecoder(0, ARRAYSIZE(hgainHuffCodes),
	                                    hgainHuffCodes, hgainHuffBits);
}

void WMACodec::initCoefHuffman(float bps) {
	// Choose the parameter table
	int coefHuffTable = 2;
	if (_sampleRate >= 32000) {
		if (bps < 0.72) {
			coefHuffTable = 0;
		} else if (bps < 1.16) {
			coefHuffTable = 1;
		}
	}

	_coefHuffmanParam[0] = &coefHuffmanParam[coefHuffTable * 2    ];
	_coefHuffmanParam[1] = &coefHuffmanParam[coefHuffTable * 2 + 1];

	_coefHuffman[0] = initCoefHuffman(_coefHuffmanRunTable[0], _coefHuffmanLevelTable[0],
	                                  _coefHuffmanIntTable[0], *_coefHuffmanParam[0]);
	_coefHuffman[1] = initCoefHuffman(_coefHuffmanRunTable[1], _coefHuffmanLevelTable[1],
	                                  _coefHuffmanIntTable[1], *_coefHuffmanParam[1]);
}

void WMACodec::initMDCT() {
	_mdct.reserve(_blockSizeCount);
	for (int i = 0; i < _blockSizeCount; i++)
		_mdct.push_back(new Common::MDCT(_frameLenBits - i + 1, true, 1.0));

	// Init MDCT windows (simple sine window)
	_mdctWindow.reserve(_blockSizeCount);
	for (int i = 0; i < _blockSizeCount; i++)
		_mdctWindow.push_back(Common::getSineWindow(_frameLenBits - i));
}

void WMACodec::initExponents() {
	if (_useExpHuffman)
		_expHuffman = new HuffmanDecoder(0, ARRAYSIZE(scaleHuffCodes),
		                                  scaleHuffCodes, scaleHuffBits);
	else
		initLSPToCurve();
}

WMACodec::HuffmanDecoder *WMACodec::initCoefHuffman(uint16 *&runTable, float *&levelTable,
		uint16 *&intTable, const WMACoefHuffmanParam &params) {

	HuffmanDecoder *huffman =
		new HuffmanDecoder(0, params.n, params.huffCodes, params.huffBits);

	runTable   = new uint16[params.n];
	levelTable = new  float[params.n];
	intTable   = new uint16[params.n];

	uint16 *iLevelTable = new uint16[params.n];

	int i = 2;
	int level = 1;
	int k = 0;

	while (i < params.n) {
		intTable[k] = i;

		int l = params.levels[k++];

		for (int j = 0; j < l; j++) {
			runTable   [i] = j;
			iLevelTable[i] = level;
			levelTable [i] = level;

			i++;
		}

		level++;
	}

	delete[] iLevelTable;

	return huffman;
}

void WMACodec::initLSPToCurve() {
	float wdel = (float)M_PI / _frameLen;

	for (int i = 0; i < _frameLen; i++)
		_lspCosTable[i] = 2.0f * cosf(wdel * i);

	// Tables for x^-0.25 computation
	for (int i = 0; i < 256; i++) {
		int e = i - 126;

		_lspPowETable[i] = powf(2.0, e * -0.25);
	}

	// NOTE: These two tables are needed to avoid two operations in pow_m1_4
	float b = 1.0;
	for (int i = (1 << kLSPPowBits) - 1; i >= 0; i--) {
		int   m = (1 << kLSPPowBits) + i;
		float a = (float) m * (0.5 / (1 << kLSPPowBits));

		a = pow(a, -0.25f);

		_lspPowMTable1[i] = 2 * a - b;
		_lspPowMTable2[i] = b - a;

		b = a;
	}
}

AudioStream *WMACodec::decodeFrame(Common::SeekableReadStream &data) {
	Common::SeekableReadStream *stream = decodeSuperFrame(data);
	if (!stream)
		return nullptr;

	return makeRawStream(stream, _sampleRate, _audioFlags, DisposeAfterUse::YES);
}

Common::SeekableReadStream *WMACodec::decodeSuperFrame(Common::SeekableReadStream &data) {
	uint32 size = data.size();
	if (size < _blockAlign) {
		warning("WMACodec::decodeSuperFrame(): size < _blockAlign");
		return nullptr;
	}

	if (_blockAlign)
		size = _blockAlign;

	Common::BitStream8MSB bits(data);

	int    outputDataSize = 0;
	int16 *outputData     = nullptr;

	_curFrame = 0;

	if (_useBitReservoir) {
		// This superframe consists of more than just one frame

		bits.skip(4); // Super frame index

		// Number of frames in this superframe
		int newFrameCount = bits.getBits<4>() - 1;
		if (newFrameCount < 0) {
			warning("WMACodec::decodeSuperFrame(): newFrameCount == %d", newFrameCount);

			_resetBlockLengths = true;
			_lastSuperframeLen = 0;
			_lastBitoffset     = 0;

			return nullptr;
		}

		// Number of frames in this superframe + overhang from the last superframe
		int frameCount = newFrameCount;
		if (_lastSuperframeLen > 0)
			frameCount++;

		// PCM output data
		outputDataSize = frameCount * _channels * _frameLen;
		outputData     = new int16[outputDataSize]();


		// Number of bits data that completes the last superframe's overhang.
		int bitOffset = bits.getBits(_byteOffsetBits + 3);

		if (_lastSuperframeLen > 0) {
			// We have overhang data from the last superframe. Paste the
			// complementary data from this superframe at the end and
			// decode it as another frame.

			byte *lastSuperframeEnd = _lastSuperframe + _lastSuperframeLen;

			while (bitOffset > 7) { // Full bytes
				*lastSuperframeEnd++ = bits.getBits<8>();

				bitOffset          -= 8;
				_lastSuperframeLen += 1;
			}

			if (bitOffset > 0) { // Remaining bits
				*lastSuperframeEnd++ = bits.getBits(bitOffset) << (8 - bitOffset);

				bitOffset           = 0;
				_lastSuperframeLen += 1;
			}

			Common::MemoryReadStream lastSuperframe(_lastSuperframe, _lastSuperframeLen);
			Common::BitStream8MSB lastBits(lastSuperframe);

			lastBits.skip(_lastBitoffset);

			decodeFrame(lastBits, outputData);

			_curFrame++;
		}

		// Skip any complementary data we haven't used
		bits.skip(bitOffset);

		// New superframe = New block lengths
		_resetBlockLengths = true;

		// Decode the frames
		for (int i = 0; i < newFrameCount; i++, _curFrame++)
			if (!decodeFrame(bits, outputData))
				return nullptr;

		// Check if we've got new overhang data
		int remainingBits = bits.size() - bits.pos();
		if (remainingBits > 0) {
			// We do: Save it

			_lastSuperframeLen = remainingBits >> 3;
			_lastBitoffset     = 8 - (remainingBits - (_lastSuperframeLen << 3));

			if (_lastBitoffset > 0)
				_lastSuperframeLen++;

			data.seek(data.size() - _lastSuperframeLen);
			data.read(_lastSuperframe, _lastSuperframeLen);
		} else {
			// We don't

			_lastSuperframeLen = 0;
			_lastBitoffset     = 0;
		}

	} else {
		// This superframe has only one frame

		// PCM output data
		outputDataSize = _channels * _frameLen;
		outputData     = new int16[outputDataSize]();


		// Decode the frame
		if (!decodeFrame(bits, outputData)) {
			delete[] outputData;
			return nullptr;
		}
	}

	// And return our PCM output data as a stream, if available

	if (!outputData)
		return nullptr;

	return new Common::MemoryReadStream((byte *) outputData, outputDataSize * 2, DisposeAfterUse::YES);
}

bool WMACodec::decodeFrame(Common::BitStream8MSB &bits, int16 *outputData) {
	_framePos = 0;
	_curBlock = 0;

	// Decode all blocks
	int finished = 0;
	while (finished == 0)
		finished = decodeBlock(bits);

	// Check for error
	if (finished < 0)
		return false;

	// Convert output into interleaved PCM data

	const float *floatOut[kChannelsMax];
	for (int i = 0; i < kChannelsMax; i++)
		floatOut[i] = _frameOut[i];

	int16 *pcmOut = outputData + _curFrame * _channels * _frameLen;

	floatToInt16Interleave(pcmOut, floatOut, _frameLen, _channels);

	// Prepare for the next frame
	for (int i = 0; i < _channels; i++)
		memmove(&_frameOut[i][0], &_frameOut[i][_frameLen], _frameLen * sizeof(float));

	return true;
}

int WMACodec::decodeBlock(Common::BitStream8MSB &bits) {
	// Computer new block length
	if (!evalBlockLength(bits))
		return -1;

	// Block size

	int bSize = _frameLenBits - _blockLenBits;
	assert((bSize >= 0) && (bSize < _blockSizeCount));

	// MS Stereo?

	bool msStereo = false;
	if (_channels == 2)
		msStereo = bits.getBit();

	// Which channels are encoded?

	bool hasChannels = false;
	bool hasChannel[kChannelsMax];
	for (int i = 0; i < kChannelsMax; i++)
		hasChannel[i] = false;

	for (int i = 0; i < _channels; i++) {
		hasChannel[i] = bits.getBit();
		if (hasChannel[i])
			hasChannels = true;
	}

	// Decode channels

	if (hasChannels)
		if (!decodeChannels(bits, bSize, msStereo, hasChannel))
			return -1;

	// Calculate IMDCTs

	if (!calculateIMDCT(bSize, msStereo, hasChannel))
		return -1;

	// Update block number

	_curBlock += 1;
	_framePos += _blockLen;

	// Finished
	if (_framePos >= _frameLen)
		return 1;

	// Need more blocks
	return 0;
}

bool WMACodec::decodeChannels(Common::BitStream8MSB &bits, int bSize,
							  bool msStereo, bool *hasChannel) {

	int totalGain    = readTotalGain(bits);
	int coefBitCount = totalGainToBits(totalGain);

	int coefCount[kChannelsMax];
	calculateCoefCount(coefCount, bSize);

	if (!decodeNoise(bits, bSize, hasChannel, coefCount))
		return false;

	if (!decodeExponents(bits, bSize, hasChannel))
		return false;

	if (!decodeSpectralCoef(bits, msStereo, hasChannel, coefCount, coefBitCount))
		return false;

	float mdctNorm = getNormalizedMDCTLength();

	calculateMDCTCoefficients(bSize, hasChannel, coefCount, totalGain, mdctNorm);

	if (msStereo && hasChannel[1]) {
		// Nominal case for ms stereo: we do it before MDCT
		// No need to optimize this case because it should almost never happen

		if (!hasChannel[0]) {
			memset(_coefs[0], 0, sizeof(float) * _blockLen);
			hasChannel[0] = true;
		}

		butterflyFloats(_coefs[0], _coefs[1], _blockLen);
	}

	return true;
}

bool WMACodec::calculateIMDCT(int bSize, bool msStereo, bool *hasChannel) {
	Common::MDCT &mdct = *_mdct[bSize];

	for (int i = 0; i < _channels; i++) {
		int n4 = _blockLen / 2;

		if (hasChannel[i])
			mdct.calcIMDCT(_output, _coefs[i]);
		else if (!(msStereo && (i == 1)))
			memset(_output, 0, sizeof(_output));

		// Multiply by the window and add in the frame
		int index = (_frameLen / 2) + _framePos - n4;
		window(&_frameOut[i][index]);
	}

	return true;
}

bool WMACodec::evalBlockLength(Common::BitStream8MSB &bits) {
	if (_useVariableBlockLen) {
		// Variable block lengths

		int n = Common::intLog2(_blockSizeCount - 1) + 1;

		if (_resetBlockLengths) {
			// Completely new block lengths

			_resetBlockLengths = false;

			const int prev     = bits.getBits(n);
			const int prevBits = _frameLenBits - prev;
			if (prev >= _blockSizeCount) {
				warning("WMACodec::evalBlockLength(): _prevBlockLenBits %d out of range", prevBits);
				return false;
			}

			_prevBlockLenBits = prevBits;

			const int cur     = bits.getBits(n);
			const int curBits = _frameLenBits - cur;
			if (cur >= _blockSizeCount) {
				warning("WMACodec::evalBlockLength(): _blockLenBits %d out of range", curBits);
				return false;
			}

			_blockLenBits = curBits;

		} else {
			// Update block lengths

			_prevBlockLenBits = _blockLenBits;
			_blockLenBits     = _nextBlockLenBits;
		}

		const int next     = bits.getBits(n);
		const int nextBits = _frameLenBits - next;
		if (next >= _blockSizeCount) {
			warning("WMACodec::evalBlockLength(): _nextBlockLenBits %d out of range", nextBits);
			return false;
		}

		_nextBlockLenBits = nextBits;

	} else {
		// Fixed block length

		_nextBlockLenBits = _frameLenBits;
		_prevBlockLenBits = _frameLenBits;
		_blockLenBits     = _frameLenBits;
	}

	// Sanity checks

	if (_frameLenBits - _blockLenBits >= _blockSizeCount) {
		warning("WMACodec::evalBlockLength(): _blockLenBits not initialized to a valid value");
		return false;
	}

	_blockLen = 1 << _blockLenBits;
	if ((_framePos + _blockLen) > _frameLen) {
		warning("WMACodec::evalBlockLength(): frame length overflow");
		return false;
	}

	return true;
}

void WMACodec::calculateCoefCount(int *coefCount, int bSize) const {
	const int coefN = _coefsEnd[bSize] - _coefsStart;

	for (int i = 0; i < _channels; i++)
		coefCount[i] = coefN;
}

bool WMACodec::decodeNoise(Common::BitStream8MSB &bits, int bSize,
						   bool *hasChannel, int *coefCount) {
	if (!_useNoiseCoding)
		return true;

	for (int i = 0; i < _channels; i++) {
		if (!hasChannel[i])
			continue;

		const int n = _exponentHighSizes[bSize];
		for (int j = 0; j < n; j++) {
			bool a = bits.getBit() != 0;
			_highBandCoded[i][j] = a;

			// With noise coding, the coefficients are not transmitted
			if (a)
				coefCount[i] -= _exponentHighBands[bSize][j];
		}
	}

	for (int i = 0; i < _channels; i++) {
		if (!hasChannel[i])
			continue;

		const int n   = _exponentHighSizes[bSize];
		      int val = (int) 0x80000000;

		for (int j = 0; j < n; j++) {
			if (!_highBandCoded[i][j])
				continue;

			if (val != (int) 0x80000000) {
				int code = _hgainHuffman->getSymbol(bits);
				if (code < 0) {
					warning("WMACodec::decodeNoise(): HGain Huffman invalid");
					return false;
				}

				val += code - 18;

			} else
				val = bits.getBits<7>() - 19;

			_highBandValues[i][j] = val;

		}
	}

	return true;
}

bool WMACodec::decodeExponents(Common::BitStream8MSB &bits, int bSize, bool *hasChannel) {
	// Exponents can be reused in short blocks
	if (!((_blockLenBits == _frameLenBits) || bits.getBit()))
		return true;

	for (int i = 0; i < _channels; i++) {
		if (!hasChannel[i])
			continue;

		if (_useExpHuffman) {
			if (!decodeExpHuffman(bits, i))
				return false;
		} else {
			if (!decodeExpLSP(bits, i))
				return false;
		}

		_exponentsBSize[i] = bSize;
	}

	return true;
}

bool WMACodec::decodeSpectralCoef(Common::BitStream8MSB &bits, bool msStereo, bool *hasChannel,
								  int *coefCount, int coefBitCount) {
	// Simple RLE encoding

	for (int i = 0; i < _channels; i++) {
		if (hasChannel[i]) {
			// Special Huffman tables are used for MS stereo
			// because there is potentially less energy there.
			const int tindex = ((i == 1) && msStereo);

			float *ptr = &_coefs1[i][0];
			memset(ptr, 0, _blockLen * sizeof(float));

			if (!decodeRunLevel(bits, *_coefHuffman[tindex],
			                    _coefHuffmanLevelTable[tindex], _coefHuffmanRunTable[tindex],
			                    0, ptr, 0, coefCount[i], _blockLen, _frameLenBits, coefBitCount))
				return false;
		}

		if ((_version == 1) && (_channels >= 2))
			bits.skip(-bits.pos() & 7);
	}

	return true;
}

float WMACodec::getNormalizedMDCTLength() const {
	const int n4 = _blockLen / 2;

	float mdctNorm = 1.0 / (float) n4;
	if (_version == 1)
		mdctNorm *= sqrt((float) n4);

	return mdctNorm;
}

void WMACodec::calculateMDCTCoefficients(int bSize, bool *hasChannel,
										int *coefCount, int totalGain, float mdctNorm) {

	for (int i = 0; i < _channels; i++) {
		if (!hasChannel[i])
			continue;

		      float *coefs     = _coefs[i];
		const float *coefs1    = _coefs1[i];
		const float *exponents = _exponents[i];

		const int eSize = _exponentsBSize[i];

		const float mult = (pow(10, totalGain * 0.05) / _maxExponent[i]) * mdctNorm;

		if (_useNoiseCoding) {

			// Very low freqs: noise
			for (int j = 0; j < _coefsStart; j++) {
				*coefs++ = _noiseTable[_noiseIndex] * exponents[(j << bSize) >> eSize] * mult;

				_noiseIndex = (_noiseIndex + 1) & (kNoiseTabSize - 1);
			}

			// Compute power of high bands
			float expPower[kHighBandSizeMax];

			const int n1 = _exponentHighSizes[bSize];
			exponents = _exponents[i] + ((_highBandStart[bSize] << bSize) >> eSize);

			int lastHighBand = 0;
			for (int k = 0; k < n1; k++) {
				const int n = _exponentHighBands[_frameLenBits - _blockLenBits][k];

				if (_highBandCoded[i][k]) {
					float e2 = 0;

					for (int j = 0; j < n; j++) {
						const float v = exponents[(j << bSize) >> eSize];

						e2 += v * v;
					}

					expPower[k] = e2 / n;
					lastHighBand = k;
				}

				exponents += (n << bSize) >> eSize;
			}

			// Main freqs and high freqs
			exponents = _exponents[i] + ((_coefsStart << bSize) >> eSize);

			for (int k = -1; k < n1; k++) {

				int n;
				if (k < 0)
					n = _highBandStart[bSize] - _coefsStart;
				else
					n = _exponentHighBands[_frameLenBits - _blockLenBits][k];

				if (k >= 0 && _highBandCoded[i][k]) {
					// Use noise with specified power

					float mult1 = sqrt(expPower[k] / expPower[lastHighBand]);

					mult1 *= pow(10, _highBandValues[i][k] * 0.05);
					mult1 /= _maxExponent[i] * _noiseMult;
					mult1 *= mdctNorm;

					for (int j = 0; j < n; j++) {
						float noise = _noiseTable[_noiseIndex];

						_noiseIndex = (_noiseIndex + 1) & (kNoiseTabSize - 1);
						*coefs++    = noise * exponents[(j << bSize) >> eSize] * mult1;
					}

					exponents += (n << bSize) >> eSize;

				} else {
					// Coded values + small noise

					for (int j = 0; j < n; j++) {
						float noise = _noiseTable[_noiseIndex];

						_noiseIndex = (_noiseIndex + 1) & (kNoiseTabSize - 1);
						*coefs++    = ((*coefs1++) + noise) * exponents[(j << bSize) >> eSize] * mult;
					}

					exponents += (n << bSize) >> eSize;
				}

			}

			// Very high freqs: Noise
			const int   n     = _blockLen - _coefsEnd[bSize];
			const float mult1 = mult * exponents[(-(1 << bSize)) >> eSize];

			for (int j = 0; j < n; j++) {
				*coefs++    = _noiseTable[_noiseIndex] * mult1;
				_noiseIndex = (_noiseIndex + 1) & (kNoiseTabSize - 1);
			}

		} else {

			for (int j = 0; j < _coefsStart; j++)
				*coefs++ = 0.0;

			for (int j = 0;j < coefCount[i]; j++) {
				*coefs = coefs1[j] * exponents[(j << bSize) >> eSize] * mult;
				coefs++;
			}

			int n = _blockLen - _coefsEnd[bSize];
			for (int j = 0; j < n; j++)
				*coefs++ = 0.0;

		}

	}

}

static const float powTab[] = {
	1.7782794100389e-04, 2.0535250264571e-04,
	2.3713737056617e-04, 2.7384196342644e-04,
	3.1622776601684e-04, 3.6517412725484e-04,
	4.2169650342858e-04, 4.8696752516586e-04,
	5.6234132519035e-04, 6.4938163157621e-04,
	7.4989420933246e-04, 8.6596432336006e-04,
	1.0000000000000e-03, 1.1547819846895e-03,
	1.3335214321633e-03, 1.5399265260595e-03,
	1.7782794100389e-03, 2.0535250264571e-03,
	2.3713737056617e-03, 2.7384196342644e-03,
	3.1622776601684e-03, 3.6517412725484e-03,
	4.2169650342858e-03, 4.8696752516586e-03,
	5.6234132519035e-03, 6.4938163157621e-03,
	7.4989420933246e-03, 8.6596432336006e-03,
	1.0000000000000e-02, 1.1547819846895e-02,
	1.3335214321633e-02, 1.5399265260595e-02,
	1.7782794100389e-02, 2.0535250264571e-02,
	2.3713737056617e-02, 2.7384196342644e-02,
	3.1622776601684e-02, 3.6517412725484e-02,
	4.2169650342858e-02, 4.8696752516586e-02,
	5.6234132519035e-02, 6.4938163157621e-02,
	7.4989420933246e-02, 8.6596432336007e-02,
	1.0000000000000e-01, 1.1547819846895e-01,
	1.3335214321633e-01, 1.5399265260595e-01,
	1.7782794100389e-01, 2.0535250264571e-01,
	2.3713737056617e-01, 2.7384196342644e-01,
	3.1622776601684e-01, 3.6517412725484e-01,
	4.2169650342858e-01, 4.8696752516586e-01,
	5.6234132519035e-01, 6.4938163157621e-01,
	7.4989420933246e-01, 8.6596432336007e-01,
	1.0000000000000e+00, 1.1547819846895e+00,
	1.3335214321633e+00, 1.5399265260595e+00,
	1.7782794100389e+00, 2.0535250264571e+00,
	2.3713737056617e+00, 2.7384196342644e+00,
	3.1622776601684e+00, 3.6517412725484e+00,
	4.2169650342858e+00, 4.8696752516586e+00,
	5.6234132519035e+00, 6.4938163157621e+00,
	7.4989420933246e+00, 8.6596432336007e+00,
	1.0000000000000e+01, 1.1547819846895e+01,
	1.3335214321633e+01, 1.5399265260595e+01,
	1.7782794100389e+01, 2.0535250264571e+01,
	2.3713737056617e+01, 2.7384196342644e+01,
	3.1622776601684e+01, 3.6517412725484e+01,
	4.2169650342858e+01, 4.8696752516586e+01,
	5.6234132519035e+01, 6.4938163157621e+01,
	7.4989420933246e+01, 8.6596432336007e+01,
	1.0000000000000e+02, 1.1547819846895e+02,
	1.3335214321633e+02, 1.5399265260595e+02,
	1.7782794100389e+02, 2.0535250264571e+02,
	2.3713737056617e+02, 2.7384196342644e+02,
	3.1622776601684e+02, 3.6517412725484e+02,
	4.2169650342858e+02, 4.8696752516586e+02,
	5.6234132519035e+02, 6.4938163157621e+02,
	7.4989420933246e+02, 8.6596432336007e+02,
	1.0000000000000e+03, 1.1547819846895e+03,
	1.3335214321633e+03, 1.5399265260595e+03,
	1.7782794100389e+03, 2.0535250264571e+03,
	2.3713737056617e+03, 2.7384196342644e+03,
	3.1622776601684e+03, 3.6517412725484e+03,
	4.2169650342858e+03, 4.8696752516586e+03,
	5.6234132519035e+03, 6.4938163157621e+03,
	7.4989420933246e+03, 8.6596432336007e+03,
	1.0000000000000e+04, 1.1547819846895e+04,
	1.3335214321633e+04, 1.5399265260595e+04,
	1.7782794100389e+04, 2.0535250264571e+04,
	2.3713737056617e+04, 2.7384196342644e+04,
	3.1622776601684e+04, 3.6517412725484e+04,
	4.2169650342858e+04, 4.8696752516586e+04,
	5.6234132519035e+04, 6.4938163157621e+04,
	7.4989420933246e+04, 8.6596432336007e+04,
	1.0000000000000e+05, 1.1547819846895e+05,
	1.3335214321633e+05, 1.5399265260595e+05,
	1.7782794100389e+05, 2.0535250264571e+05,
	2.3713737056617e+05, 2.7384196342644e+05,
	3.1622776601684e+05, 3.6517412725484e+05,
	4.2169650342858e+05, 4.8696752516586e+05,
	5.6234132519035e+05, 6.4938163157621e+05,
	7.4989420933246e+05, 8.6596432336007e+05,
};

bool WMACodec::decodeExpHuffman(Common::BitStream8MSB &bits, int ch) {
	const float  *ptab  = powTab + 60;
	const uint32 *iptab = (const uint32 *) ptab;

	const uint16 *ptr = _exponentBands[_frameLenBits - _blockLenBits];

	uint32 *q = (uint32 *) _exponents[ch];
	uint32 *qEnd = q + _blockLen;

	float maxScale = 0;

	int lastExp;
	if (_version == 1) {

		lastExp = bits.getBits<5>() + 10;

		float   v = ptab[lastExp];
		uint32 iv = iptab[lastExp];

		maxScale = v;

		int n = *ptr++;

		switch (n & 3) do {
			case 0: *q++ = iv; // fall through
			case 3: *q++ = iv; // fall through
			case 2: *q++ = iv; // fall through
			case 1: *q++ = iv;
		} while ((n -= 4) > 0);

	} else
		lastExp = 36;

	while (q < qEnd) {
		int code = _expHuffman->getSymbol(bits);
		if (code < 0) {
			warning("WMACodec::decodeExpHuffman(): Exponent invalid");
			return false;
		}

		// NOTE: This offset is the same as MPEG4 AAC!
		lastExp += code - 60;
		if ((unsigned) lastExp + 60 >= ARRAYSIZE(powTab)) {
			warning("WMACodec::decodeExpHuffman(): Exponent out of range: %d", lastExp);
			return false;
		}

		float   v = ptab[lastExp];
		uint32 iv = iptab[lastExp];

		if (v > maxScale)
			maxScale = v;

		int n = *ptr++;

		switch (n & 3) do {
			case 0: *q++ = iv; // fall through
			case 3: *q++ = iv; // fall through
			case 2: *q++ = iv; // fall through
			case 1: *q++ = iv;
		} while ((n -= 4) > 0);

	}

	_maxExponent[ch] = maxScale;

	return true;
}

void WMACodec::lspToCurve(float *out, float *val_max_ptr, int n, float *lsp) {
	float val_max = 0;

	for (int i = 0; i < n; i++) {
		float p = 0.5f;
		float q = 0.5f;
		float w = _lspCosTable[i];

		for (int j = 1; j < kLSPCoefCount; j += 2) {
			q *= w - lsp[j - 1];
			p *= w - lsp[j];
		}

		p *= p * (2.0f - w);
		q *= q * (2.0f + w);

		float v = p + q;
		v = pow_m1_4(v);

		if (v > val_max)
			val_max = v;

		out[i] = v;
	}

	*val_max_ptr = val_max;
}

// Decode exponents coded with LSP coefficients (same idea as Vorbis)
bool WMACodec::decodeExpLSP(Common::BitStream8MSB &bits, int ch) {
	float lspCoefs[kLSPCoefCount];

	for (int i = 0; i < kLSPCoefCount; i++) {
		int val;

		if (i == 0 || i >= 8)
			val = bits.getBits<3>();
		else
			val = bits.getBits<4>();

		lspCoefs[i] = lspCodebook[i][val];
	}

	lspToCurve(_exponents[ch], &_maxExponent[ch], _blockLen, lspCoefs);
	return true;
}

bool WMACodec::decodeRunLevel(Common::BitStream8MSB &bits, const HuffmanDecoder &huffman,
	const float *levelTable, const uint16 *runTable, int version, float *ptr,
	int offset, int numCoefs, int blockLen, int frameLenBits, int coefNbBits) {

	const uint32 *ilvl = (const uint32*) levelTable;
	uint32 *iptr = (uint32 *) ptr;

	const unsigned int coefMask = blockLen - 1;

	for (; offset < numCoefs; offset++) {
		const int code = huffman.getSymbol(bits);

		if (code > 1) {
			// Normal code

			const int sign = bits.getBit() - 1;

			offset += runTable[code];

			iptr[offset & coefMask] = ilvl[code] ^ (sign << 31);

		} else if (code == 1) {
			// EOB

			break;

		} else {
			// Escape

			int level;

			if (!version) {

				level   = bits.getBits(coefNbBits);
				// NOTE: This is rather suboptimal. reading blockLenBits would be better
				offset += bits.getBits(frameLenBits);

			} else {
				level = getLargeVal(bits);

				// Escape decode
				if (bits.getBit()) {
					if (bits.getBit()) {
						if (bits.getBit()) {
							warning("WMACodec::decodeRunLevel(): Broken escape sequence");
							return false;
						} else
							offset += bits.getBits(frameLenBits) + 4;
					} else
						offset += bits.getBits<2>() + 1;
				}

			}

			const int sign = bits.getBit() - 1;

			ptr[offset & coefMask] = (level ^ sign) - sign;

		}
	}

	// NOTE: EOB can be omitted
	if (offset > numCoefs) {
		warning("WMACodec::decodeRunLevel(): Overflow in spectral RLE, ignoring");
		return true;
	}

	return true;
}

/** Apply MDCT window and add into output.
 *
 *  We ensure that when the windows overlap their squared sum
 *  is always 1 (MDCT reconstruction rule).
 */
void WMACodec::window(float *out) const {
	const float *in = _output;

	// Left part
	if (_blockLenBits <= _prevBlockLenBits) {

		const int bSize = _frameLenBits - _blockLenBits;

		vectorFMulAdd(out, in, _mdctWindow[bSize], out, _blockLen);

	} else {

		const int blockLen = 1 << _prevBlockLenBits;
		const int n = (_blockLen - blockLen) / 2;

		const int bSize = _frameLenBits - _prevBlockLenBits;

		vectorFMulAdd(out + n, in + n, _mdctWindow[bSize], out + n, blockLen);

		memcpy(out + n + blockLen, in + n + blockLen, n * sizeof(float));
	}

	out += _blockLen;
	in  += _blockLen;

	// Right part
	if (_blockLenBits <= _nextBlockLenBits) {

		const int bSize = _frameLenBits - _blockLenBits;

		vectorFMulReverse(out, in, _mdctWindow[bSize], _blockLen);

	} else {

		const int blockLen = 1 << _nextBlockLenBits;
		const int n = (_blockLen - blockLen) / 2;

		const int bSize = _frameLenBits - _nextBlockLenBits;

		memcpy(out, in, n*sizeof(float));

		vectorFMulReverse(out + n, in + n, _mdctWindow[bSize], blockLen);

		memset(out + n + blockLen, 0, n * sizeof(float));
	}
}

float WMACodec::pow_m1_4(float x) const {
	union {
		float f;
		unsigned int v;
	} u, t;

	u.f = x;

	const unsigned int e =  u.v >>  23;
	const unsigned int m = (u.v >> (23 - kLSPPowBits)) & ((1 << kLSPPowBits) - 1);

	// Build interpolation scale: 1 <= t < 2
	t.v = ((u.v << kLSPPowBits) & ((1 << 23) - 1)) | (127 << 23);

	const float a = _lspPowMTable1[m];
	const float b = _lspPowMTable2[m];

	return _lspPowETable[e] * (a + b * t.f);
}

int WMACodec::readTotalGain(Common::BitStream8MSB &bits) {
	int totalGain = 1;

	int v = 127;
	while (v == 127) {
		v = bits.getBits<7>();

		totalGain += v;
	}

	return totalGain;
}

int WMACodec::totalGainToBits(int totalGain) {
	     if (totalGain < 15) return 13;
	else if (totalGain < 32) return 12;
	else if (totalGain < 40) return 11;
	else if (totalGain < 45) return 10;
	else                     return  9;
}

uint32 WMACodec::getLargeVal(Common::BitStream8MSB &bits) {
	// Consumes up to 34 bits

	if (bits.getBit()) {
		if (bits.getBit()) {
			if (bits.getBit()) {
				return bits.getBits<31>();
			} else {
				return bits.getBits<24>();
			}
		} else {
			return bits.getBits<16>();
		}
	} else {
		return bits.getBits<8>();
	}
}

} // End of namespace Audio
