/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

// Based on xoreos' WMA code which is in turn
// Largely based on the WMA implementation found in FFmpeg.

#ifndef AUDIO_DECODERS_WMA_H
#define AUDIO_DECODERS_WMA_H

#include "common/array.h"
#include "common/bitstream.h"

#include "audio/decoders/codec.h"

namespace Common {
template <class BITSTREAM>
class Huffman;
class MDCT;
}

namespace Audio {

struct WMACoefHuffmanParam;

class WMACodec : public Codec {
public:
	WMACodec(int version, uint32 sampleRate, uint8 channels,
	         uint32 bitRate, uint32 blockAlign, Common::SeekableReadStream *extraData = 0);
	~WMACodec();

	AudioStream *decodeFrame(Common::SeekableReadStream &data);

private:
	static const int kChannelsMax = 2; ///< Max number of channels we support.

	static const int kBlockBitsMin =  7; ///< Min number of bits in a block.
	static const int kBlockBitsMax = 11; ///< Max number of bits in a block.

	/** Max number of bytes in a block. */
	static const int kBlockSizeMax = (1 << kBlockBitsMax);

	static const int kBlockNBSizes = (kBlockBitsMax - kBlockBitsMin + 1);

	/** Max size of a superframe. */
	static const int kSuperframeSizeMax = 16384;

	/** Max size of a high band. */
	static const int kHighBandSizeMax = 16;

	/** Size of the noise table. */
	static const int kNoiseTabSize = 8192;

	/** Number of bits for the LSP power value. */
	static const int kLSPPowBits = 7;

	int _version; ///< WMA version.

	uint32 _sampleRate; ///< Output sample rate.
	uint8  _channels;   ///< Output channel count.
	uint32 _bitRate;    ///< Input bit rate.
	uint32 _blockAlign; ///< Input block align.
	byte   _audioFlags; ///< Output flags.

	bool _useExpHuffman;       ///< Exponents in Huffman code? Otherwise, in LSP.
	bool _useBitReservoir;     ///< Is each frame packet a "superframe"?
	bool _useVariableBlockLen; ///< Are the block lengths variable?
	bool _useNoiseCoding;      ///< Should perceptual noise be added?

	bool _resetBlockLengths; ///< Do we need new block lengths?

	int _curFrame;       ///< The number of the frame we're currently in.
	int _frameLen;       ///< The frame length.
	int _frameLenBits;   ///< log2 of the frame length.
	int _blockSizeCount; ///< Number of block sizes.
	int _framePos;       ///< The position within the frame we're currently in.

	int _curBlock;         ///< The number of the block we're currently in.
	int _blockLen;         ///< Current block length.
	int _blockLenBits;     ///< log2 of current block length.
	int _nextBlockLenBits; ///< log2 of next block length.
	int _prevBlockLenBits; ///< log2 of previous block length.

	int _byteOffsetBits;

	// Coefficients
	int    _coefsStart;                       ///< First coded coef
	int    _coefsEnd[kBlockNBSizes];          ///< Max number of coded coefficients
	int    _exponentSizes[kBlockNBSizes];
	uint16 _exponentBands[kBlockNBSizes][25];
	int    _highBandStart[kBlockNBSizes];     ///< Index of first coef in high band
	int    _exponentHighSizes[kBlockNBSizes];
	int    _exponentHighBands[kBlockNBSizes][kHighBandSizeMax];

	typedef Common::Huffman<Common::BitStream8MSB> HuffmanDecoder;
	HuffmanDecoder *_coefHuffman[2];                ///< Coefficients Huffman codes.
	const WMACoefHuffmanParam *_coefHuffmanParam[2]; ///< Params for coef Huffman codes.

	uint16 *_coefHuffmanRunTable[2];   ///< Run table for the coef Huffman.
	float  *_coefHuffmanLevelTable[2]; ///< Level table for the coef Huffman.
	uint16 *_coefHuffmanIntTable[2];   ///< Int tablre for the coef Huffman.

	// Noise
	float _noiseMult;                 ///< Noise multiplier.
	float _noiseTable[kNoiseTabSize]; ///< Noise table.
	int   _noiseIndex;

	HuffmanDecoder *_hgainHuffman; ///< Perceptual noise huffman code.

	// Exponents
	int   _exponentsBSize[kChannelsMax];
	float _exponents[kChannelsMax][kBlockSizeMax];
	float _maxExponent[kChannelsMax];

	HuffmanDecoder *_expHuffman; ///< Exponents huffman code.

	// Coded values in high bands
	bool _highBandCoded [kChannelsMax][kHighBandSizeMax];
	int  _highBandValues[kChannelsMax][kHighBandSizeMax];

	// Coefficients
	float _coefs1[kChannelsMax][kBlockSizeMax];
	float _coefs [kChannelsMax][kBlockSizeMax];

	// Line spectral pairs
	float _lspCosTable[kBlockSizeMax];
	float _lspPowETable[256];
	float _lspPowMTable1[(1 << kLSPPowBits)];
	float _lspPowMTable2[(1 << kLSPPowBits)];

	// MDCT
	Common::Array<Common::MDCT *> _mdct;       ///< MDCT contexts.
	Common::Array<const float *>  _mdctWindow; ///< MDCT window functions.

	/** Overhang from the last superframe. */
	byte _lastSuperframe[kSuperframeSizeMax + 4];
	int  _lastSuperframeLen; ///< Size of the overhang data. */
	int  _lastBitoffset;     ///< Bit position within the overhang. */

	// Output
	float _output[kBlockSizeMax * 2];
	float _frameOut[kChannelsMax][kBlockSizeMax * 2];


	// Init helpers

	void init(Common::SeekableReadStream *extraData);

	uint16 getFlags(Common::SeekableReadStream *extraData);
	void evalFlags(uint16 flags, Common::SeekableReadStream *extraData);
	int getFrameBitLength();
	int getBlockSizeCount(uint16 flags);
	uint32 getNormalizedSampleRate();
	bool useNoiseCoding(float &highFreq, float &bps);
	void evalMDCTScales(float highFreq);
	void initNoise();
	void initCoefHuffman(float bps);
	void initMDCT();
	void initExponents();

	HuffmanDecoder *initCoefHuffman(uint16 *&runTable, float *&levelTable,
	                                 uint16 *&intTable, const WMACoefHuffmanParam &params);
	void initLSPToCurve();

	// Decoding

	Common::SeekableReadStream *decodeSuperFrame(Common::SeekableReadStream &data);
	bool decodeFrame(Common::BitStream8MSB &bits, int16 *outputData);
	int decodeBlock(Common::BitStream8MSB &bits);

	// Decoding helpers

	bool evalBlockLength(Common::BitStream8MSB &bits);
	bool decodeChannels(Common::BitStream8MSB &bits, int bSize, bool msStereo, bool *hasChannel);
	bool calculateIMDCT(int bSize, bool msStereo, bool *hasChannel);

	void calculateCoefCount(int *coefCount, int bSize) const;
	bool decodeNoise(Common::BitStream8MSB &bits, int bSize, bool *hasChannel, int *coefCount);
	bool decodeExponents(Common::BitStream8MSB &bits, int bSize, bool *hasChannel);
	bool decodeSpectralCoef(Common::BitStream8MSB &bits, bool msStereo, bool *hasChannel,
	                        int *coefCount, int coefBitCount);
	float getNormalizedMDCTLength() const;
	void calculateMDCTCoefficients(int bSize, bool *hasChannel,
	                               int *coefCount, int totalGain, float mdctNorm);

	bool decodeExpHuffman(Common::BitStream8MSB &bits, int ch);
	bool decodeExpLSP(Common::BitStream8MSB &bits, int ch);
	bool decodeRunLevel(Common::BitStream8MSB &bits, const HuffmanDecoder &huffman,
		const float *levelTable, const uint16 *runTable, int version, float *ptr,
		int offset, int numCoefs, int blockLen, int frameLenBits, int coefNbBits);

	void lspToCurve(float *out, float *val_max_ptr, int n, float *lsp);

	void window(float *out) const;

	float pow_m1_4(float x) const;

	static int readTotalGain(Common::BitStream8MSB &bits);
	static int totalGainToBits(int totalGain);
	static uint32 getLargeVal(Common::BitStream8MSB &bits);
};

} // End of namespace Audio

#endif // AUDIO_DECODERS_WMA_H
