/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef MOHAWK_VIDEO_QDM2_H
#define MOHAWK_VIDEO_QDM2_H

#include "sound/audiostream.h"
#include "common/stream.h"

namespace Mohawk {

enum {
	SOFTCLIP_THRESHOLD = 27600,
	HARDCLIP_THRESHOLD = 35716,
	MPA_MAX_CHANNELS = 2,
	MPA_FRAME_SIZE = 1152,
	FF_INPUT_BUFFER_PADDING_SIZE = 8
};

typedef int8 sb_int8_array[2][30][64];

/* bit input */
/* buffer, buffer_end and size_in_bits must be present and used by every reader */
struct GetBitContext {
	const uint8 *buffer, *bufferEnd;
	int index;
	int sizeInBits;
};

struct QDM2SubPacket {
	int type;
	unsigned int size;
	const uint8 *data; // pointer to subpacket data (points to input data buffer, it's not a private copy)
};

struct QDM2SubPNode {
	QDM2SubPacket *packet;
	struct QDM2SubPNode *next; // pointer to next packet in the list, NULL if leaf node
};

struct QDM2Complex {
	float re;
	float im;
};

struct FFTTone {
	float level;
	QDM2Complex *complex;
	const float *table;
	int phase;
	int phase_shift;
	int duration;
	short time_index;
	short cutoff;
};

struct FFTCoefficient {
	int16 sub_packet;
	uint8 channel;
	int16 offset;
	int16 exp;
	uint8 phase;
};

struct VLC {
	int32 bits;
	int16 (*table)[2]; // code, bits
	int32 table_size;
	int32 table_allocated;
};

#include "common/pack-start.h"
struct QDM2FFT {
	QDM2Complex complex[MPA_MAX_CHANNELS][256];
} PACKED_STRUCT;
#include "common/pack-end.h"

enum RDFTransformType {
	RDFT,
	IRDFT,
	RIDFT,
	IRIDFT
};

struct FFTComplex {
	float re, im;
};

struct FFTContext {
	int nbits;
	int inverse;
	uint16 *revtab;
	FFTComplex *exptab;
	FFTComplex *tmpBuf;
	int mdctSize; // size of MDCT (i.e. number of input data * 2)
	int mdctBits; // n = 2^nbits
	// pre/post rotation tables
	float *tcos;
	float *tsin;
	void (*fftPermute)(struct FFTContext *s, FFTComplex *z);
	void (*fftCalc)(struct FFTContext *s, FFTComplex *z);
	void (*imdctCalc)(struct FFTContext *s, float *output, const float *input);
	void (*imdctHalf)(struct FFTContext *s, float *output, const float *input);
	void (*mdctCalc)(struct FFTContext *s, float *output, const float *input);
	int splitRadix;
	int permutation;
};

enum {
	FF_MDCT_PERM_NONE = 0,
	FF_MDCT_PERM_INTERLEAVE = 1
};

struct RDFTContext {
	int nbits;
	int inverse;
	int signConvention;

	// pre/post rotation tables
	float *tcos;
	float *tsin;
	FFTContext fft;
};

class QDM2Stream : public Audio::AudioStream {
public:
	QDM2Stream(Common::SeekableReadStream *stream, Common::SeekableReadStream *extraData);
	~QDM2Stream();

	bool isStereo() const { return _channels == 2; }
	bool endOfData() const { return ((_stream->pos() == _stream->size()) && (_outputSamples.size() == 0)); }
	int getRate() const { return _sampleRate; }
	int readBuffer(int16 *buffer, const int numSamples);

private:
	Common::SeekableReadStream *_stream;

	// Parameters from codec header, do not change during playback
	uint8 _channels;
	uint16 _sampleRate;
	uint16 _bitRate;
	uint16 _blockSize;  // Group
	uint16 _frameSize;  // FFT
	uint16 _packetSize; // Checksum

	// Parameters built from header parameters, do not change during playback
	int _groupOrder;       // order of frame group
	int _fftOrder;         // order of FFT (actually fft order+1)
	int _fftFrameSize;     // size of fft frame, in components (1 comples = re + im)
	int _sFrameSize;        // size of data frame
	int _frequencyRange;
	int _subSampling;      // subsampling: 0=25%, 1=50%, 2=100% */
	int _coeffPerSbSelect; // selector for "num. of coeffs. per subband" tables. Can be 0, 1, 2
	int _cmTableSelect;    // selector for "coding method" tables. Can be 0, 1 (from init: 0-4)

	// Packets and packet lists
	QDM2SubPacket _subPackets[16];    // the packets themselves
	QDM2SubPNode _subPacketListA[16]; // list of all packets
	QDM2SubPNode _subPacketListB[16]; // FFT packets B are on list
	int _subPacketsB;                 // number of packets on 'B' list
	QDM2SubPNode _subPacketListC[16]; // packets with errors?
	QDM2SubPNode _subPacketListD[16]; // DCT packets

	// FFT and tones
	FFTTone _fftTones[1000];
	int _fftToneStart;
	int _fftToneEnd;
	FFTCoefficient _fftCoefs[1000];
	int _fftCoefsIndex;
	int _fftCoefsMinIndex[5];
	int _fftCoefsMaxIndex[5];
	int _fftLevelExp[6];
	//RDFTContext _rdftCtx;
	QDM2FFT _fft;

	// I/O data
	uint8 *_compressedData;
	float _outputBuffer[1024];
	Common::Array<int16> _outputSamples;

	// Synthesis filter
	int16 ff_mpa_synth_window[512];
	int16 _synthBuf[MPA_MAX_CHANNELS][512*2];
	int _synthBufOffset[MPA_MAX_CHANNELS];
	int32 _sbSamples[MPA_MAX_CHANNELS][128][32];

	// Mixed temporary data used in decoding
	float _toneLevel[MPA_MAX_CHANNELS][30][64];
	int8 _codingMethod[MPA_MAX_CHANNELS][30][64];
	int8 _quantizedCoeffs[MPA_MAX_CHANNELS][10][8];
	int8 _toneLevelIdxBase[MPA_MAX_CHANNELS][30][8];
	int8 _toneLevelIdxHi1[MPA_MAX_CHANNELS][3][8][8];
	int8 _toneLevelIdxMid[MPA_MAX_CHANNELS][26][8];
	int8 _toneLevelIdxHi2[MPA_MAX_CHANNELS][26];
	int8 _toneLevelIdx[MPA_MAX_CHANNELS][30][64];
	int8 _toneLevelIdxTemp[MPA_MAX_CHANNELS][30][64];

	// Flags
	bool _hasErrors;         // packet has errors
	int _superblocktype_2_3; // select fft tables and some algorithm based on superblock type
	int _doSynthFilter;      // used to perform or skip synthesis filter

	uint8 _subPacket; // 0 to 15
	int _noiseIdx; // index for dithering noise table
	
	byte _emptyBuffer[FF_INPUT_BUFFER_PADDING_SIZE];

	VLC _vlcTabLevel;
	VLC _vlcTabDiff;
	VLC _vlcTabRun;
	VLC _fftLevelExpAltVlc;
	VLC _fftLevelExpVlc;
	VLC _fftStereoExpVlc;
	VLC _fftStereoPhaseVlc;
	VLC _vlcTabToneLevelIdxHi1;
	VLC _vlcTabToneLevelIdxMid;
	VLC _vlcTabToneLevelIdxHi2;
	VLC _vlcTabType30;
	VLC _vlcTabType34;
	VLC _vlcTabFftToneOffset[5];
	bool _vlcsInitialized;
	void initVlc(void);

	uint16 _softclipTable[HARDCLIP_THRESHOLD - SOFTCLIP_THRESHOLD + 1];
	void softclipTableInit(void);

	float _noiseTable[4096];
	byte _randomDequantIndex[256][5];
	byte _randomDequantType24[128][3];
	void rndTableInit(void);

	float _noiseSamples[128];
	void initNoiseSamples(void);
	
	RDFTContext _rdftCtx;

	void average_quantized_coeffs(void);
	void build_sb_samples_from_noise(int sb);
	void fix_coding_method_array(int sb, int channels, sb_int8_array coding_method);
	void fill_tone_level_array(int flag);
	void fill_coding_method_array(sb_int8_array tone_level_idx, sb_int8_array tone_level_idx_temp,
	                              sb_int8_array coding_method, int nb_channels,
	                              int c, int superblocktype_2_3, int cm_table_select);
	void synthfilt_build_sb_samples(GetBitContext *gb, int length, int sb_min, int sb_max);
	void init_quantized_coeffs_elem0(int8 *quantized_coeffs, GetBitContext *gb, int length);
	void init_tone_level_dequantization(GetBitContext *gb, int length);
	void process_subpacket_9(QDM2SubPNode *node);
	void process_subpacket_10(QDM2SubPNode *node, int length);
	void process_subpacket_11(QDM2SubPNode *node, int length);
	void process_subpacket_12(QDM2SubPNode *node, int length);
	void process_synthesis_subpackets(QDM2SubPNode *list);
	void qdm2_decode_super_block(void);
	void qdm2_fft_init_coefficient(int sub_packet, int offset, int duration, 
	                               int channel, int exp, int phase);
	void qdm2_fft_decode_tones(int duration, GetBitContext *gb, int b);
	void qdm2_decode_fft_packets(void);
	void qdm2_fft_generate_tone(FFTTone *tone);
	void qdm2_fft_tone_synthesizer(uint8 sub_packet);
	void qdm2_calculate_fft(int channel);
	void qdm2_synthesis_filter(uint8 index);
	int qdm2_decodeFrame(Common::SeekableReadStream *in);
};

} // End of namespace Mohawk

#endif
