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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/sound/decoders/adpcm.cpp $
 * $Id: adpcm.cpp 54204 2010-11-11 17:04:07Z mthreepwood $
 *
 */

//#include <assert.h>
//#include <string.h>
#include "decoder/wave/endian.h"

#include "decoder/wave/adpcm.h"
#include "audiostream.h"

#ifdef MIN
#undef MIN
#endif

template<typename T> inline T MIN(T a, T b)    {
	return (a < b) ? a : b;
}

template<typename T> inline T CLIP(T v, T amin, T amax) {
	if (v < amin) return amin;
	else if (v > amax) return amax;
	else return v;
}

namespace Audio {

class ADPCMStream : public RewindableAudioStream {
protected:
	Common::SeekableReadStream *_stream;
	const DisposeAfterUse::Flag _disposeAfterUse;
	const sint32 _startpos;
	const sint32 _endpos;
	const int _channels;
	const uint32 _blockAlign;
	uint32 _blockPos[2];
	const int _rate;

	struct {
		// OKI/IMA
		struct {
			sint32 last;
			sint32 stepIndex;
		} ima_ch[2];
	} _status;

	virtual void reset();
	sint16 stepAdjust(uint8);

public:
	ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign);
	~ADPCMStream();

	virtual bool endOfData() const {
		return (_stream->eos() || _stream->pos() >= _endpos);
	}
	virtual bool isStereo() const   {
		return _channels == 2;
	}
	virtual int getRate() const {
		return _rate;
	}

	virtual bool rewind();
};

// Routines to convert 12 bit linear samples to the
// Dialogic or Oki ADPCM coding format aka VOX.
// See also <http://www.comptek.ru/telephony/tnotes/tt1-13.html>
//
// IMA ADPCM support is based on
//   <http://wiki.multimedia.cx/index.php?title=IMA_ADPCM>
//
// In addition, also MS IMA ADPCM is supported. See
//   <http://wiki.multimedia.cx/index.php?title=Microsoft_IMA_ADPCM>.

ADPCMStream::ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
	: _stream(stream),
	  _disposeAfterUse(disposeAfterUse),
	  _startpos(stream->pos()),
	  _endpos(_startpos + size),
	  _channels(channels),
	  _blockAlign(blockAlign),
	  _rate(rate) {

	reset();
}

ADPCMStream::~ADPCMStream() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _stream;
}

void ADPCMStream::reset() {
	memset(&_status, 0, sizeof(_status));
	_blockPos[0] = _blockPos[1] = _blockAlign; // To make sure first header is read
}

bool ADPCMStream::rewind() {
	// TODO: Error checking.
	reset();
	_stream->seek(_startpos);
	return true;
}


#pragma mark -


class Oki_ADPCMStream : public ADPCMStream {
public:
	Oki_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(sint16 *buffer, const int numSamples);

protected:
	sint16 decodeOKI(uint8);
};

int Oki_ADPCMStream::readBuffer(sint16 *buffer, const int numSamples) {
	int samples;
	uint8 data;

	assert(numSamples % 2 == 0);

	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
		data = _stream->readByte();
		buffer[samples] = decodeOKI((data >> 4) & 0x0f);
		buffer[samples + 1] = decodeOKI(data & 0x0f);
	}
	return samples;
}

static const sint16 okiStepSize[49] = {
	16,   17,   19,   21,   23,   25,   28,   31,
	34,   37,   41,   45,   50,   55,   60,   66,
	73,   80,   88,   97,  107,  118,  130,  143,
	157,  173,  190,  209,  230,  253,  279,  307,
	337,  371,  408,  449,  494,  544,  598,  658,
	724,  796,  876,  963, 1060, 1166, 1282, 1411,
	1552
};

// Decode Linear to ADPCM
sint16 Oki_ADPCMStream::decodeOKI(uint8 code) {
	sint16 diff, E, samp;

	E = (2 * (code & 0x7) + 1) * okiStepSize[_status.ima_ch[0].stepIndex] / 8;
	diff = (code & 0x08) ? -E : E;
	samp = _status.ima_ch[0].last + diff;
	// Clip the values to +/- 2^11 (supposed to be 12 bits)
	samp = CLIP<sint16>(samp, -2048, 2047);

	_status.ima_ch[0].last = samp;
	_status.ima_ch[0].stepIndex += stepAdjust(code);
	_status.ima_ch[0].stepIndex = CLIP<sint32>(_status.ima_ch[0].stepIndex, 0, ARRAYSIZE(okiStepSize) - 1);

	// * 16 effectively converts 12-bit input to 16-bit output
	return samp * 16;
}


#pragma mark -


class Ima_ADPCMStream : public ADPCMStream {
protected:
	sint16 decodeIMA(uint8 code, int channel = 0); // Default to using the left channel/using one channel

public:
	Ima_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		memset(&_status, 0, sizeof(_status));
	}

	virtual int readBuffer(sint16 *buffer, const int numSamples);
};

int Ima_ADPCMStream::readBuffer(sint16 *buffer, const int numSamples) {
	int samples;
	uint8 data;

	assert(numSamples % 2 == 0);

	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
		data = _stream->readByte();
		buffer[samples] = decodeIMA((data >> 4) & 0x0f);
		buffer[samples + 1] = decodeIMA(data & 0x0f, _channels == 2 ? 1 : 0);
	}
	return samples;
}

#pragma mark -


class Apple_ADPCMStream : public Ima_ADPCMStream {
protected:
	// Apple QuickTime IMA ADPCM
	sint32 _streamPos[2];
	sint16 _buffer[2][2];
	uint8 _chunkPos[2];

	void reset() {
		Ima_ADPCMStream::reset();
		_chunkPos[0] = 0;
		_chunkPos[1] = 0;
		_streamPos[0] = 0;
		_streamPos[1] = _blockAlign;
	}

public:
	Apple_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		_chunkPos[0] = 0;
		_chunkPos[1] = 0;
		_streamPos[0] = 0;
		_streamPos[1] = _blockAlign;
	}

	virtual int readBuffer(sint16 *buffer, const int numSamples);

};

int Apple_ADPCMStream::readBuffer(sint16 *buffer, const int numSamples) {
	// Need to write at least one samples per channel
	assert((numSamples % _channels) == 0);

	// Current sample positions
	int samples[2] = { 0, 0};

	// Number of samples per channel
	int chanSamples = numSamples / _channels;

	for (int i = 0; i < _channels; i++) {
		_stream->seek(_streamPos[i]);

		while ((samples[i] < chanSamples) &&
		        // Last byte read and a new one needed
		        !((_stream->eos() || (_stream->pos() >= _endpos)) && (_chunkPos[i] == 0))) {

			if (_blockPos[i] == _blockAlign) {
				// 2 byte header per block
				uint16 temp = _stream->readUint16BE();

				// First 9 bits are the upper bits of the predictor
				_status.ima_ch[i].last      = (sint16)(temp & 0xFF80);
				// Lower 7 bits are the step index
				_status.ima_ch[i].stepIndex =          temp & 0x007F;

				// Clip the step index
				_status.ima_ch[i].stepIndex = CLIP<sint32>(_status.ima_ch[i].stepIndex, 0, 88);

				_blockPos[i] = 2;
			}

			if (_chunkPos[i] == 0) {
				// Decode data
				uint8 data = _stream->readByte();
				_buffer[i][0] = decodeIMA(data &  0x0F, i);
				_buffer[i][1] = decodeIMA(data >>    4, i);
			}

			// The original is interleaved block-wise, we want it sample-wise
			buffer[_channels * samples[i] + i] = _buffer[i][_chunkPos[i]];

			if (++_chunkPos[i] > 1) {
				// We're about to decode the next byte, so advance the block position
				_chunkPos[i] = 0;
				_blockPos[i]++;
			}

			samples[i]++;

			if (_channels == 2)
				if (_blockPos[i] == _blockAlign)
					// We're at the end of the block.
					// Since the channels are interleaved, skip the next block
					_stream->skip(MIN<uint32>(_blockAlign, _endpos - _stream->pos()));

			_streamPos[i] = _stream->pos();
		}
	}

	return samples[0] + samples[1];
}

#pragma mark -


class MSIma_ADPCMStream : public Ima_ADPCMStream {
public:
	MSIma_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign, bool invertSamples = false)
		: Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign), _invertSamples(invertSamples) {
		if (blockAlign == 0)
			DEBUG(0, LEVEL_ERROR, "ADPCMStream(): blockAlign isn't specified for MS IMA ADPCM");
	}

	virtual int readBuffer(sint16 *buffer, const int numSamples) {
		if (_channels == 1)
			return readBufferMSIMA1(buffer, numSamples);
		else
			return readBufferMSIMA2(buffer, numSamples);
	}

	int readBufferMSIMA1(sint16 *buffer, const int numSamples);
	int readBufferMSIMA2(sint16 *buffer, const int numSamples);

private:
	bool _invertSamples;    // Some implementations invert the way samples are decoded
};

int MSIma_ADPCMStream::readBufferMSIMA1(sint16 *buffer, const int numSamples) {
	int samples = 0;
	uint8 data;

	assert(numSamples % 2 == 0);

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			// read block header
			_status.ima_ch[0].last = _stream->readSint16LE();
			_status.ima_ch[0].stepIndex = _stream->readSint16LE();
			_blockPos[0] = 4;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
			data = _stream->readByte();
			_blockPos[0]++;
			buffer[samples] = decodeIMA(_invertSamples ? (data >> 4) & 0x0f : data & 0x0f);
			buffer[samples + 1] = decodeIMA(_invertSamples ? data & 0x0f : (data >> 4) & 0x0f);
		}
	}
	return samples;
}


// Microsoft as usual tries to implement it differently. This method
// is used for stereo data.
int MSIma_ADPCMStream::readBufferMSIMA2(sint16 *buffer, const int numSamples) {
	int samples;
	uint32 data;
	int nibble;
	uint8 k;

	// TODO: Currently this implementation only supports
	// reading a multiple of 16 samples at once. We might
	// consider changing that so it could read an arbitrary
	// sample pair count.
	assert(numSamples % 16 == 0);

	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos;) {
		for (int channel = 0; channel < 2; channel++) {
			data = _stream->readUint32LE();

			for (nibble = 0; nibble < 8; nibble++) {
				k = ((data & 0xf0000000) >> 28);
				buffer[samples + channel + nibble * 2] = decodeIMA(k);
				data <<= 4;
			}
		}
		samples += 16;
	}
	return samples;
}


#pragma mark -


static const int MSADPCMAdaptCoeff1[] = {
	256, 512, 0, 192, 240, 460, 392
};

static const int MSADPCMAdaptCoeff2[] = {
	0, -256, 0, 64, 0, -208, -232
};

static const int MSADPCMAdaptationTable[] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};


class MS_ADPCMStream : public ADPCMStream {
protected:
	struct ADPCMChannelStatus {
		uint8 predictor;
		sint16 delta;
		sint16 coeff1;
		sint16 coeff2;
		sint16 sample1;
		sint16 sample2;
	};

	struct {
		// MS ADPCM
		ADPCMChannelStatus ch[2];
	} _status;

	void reset() {
		ADPCMStream::reset();
		memset(&_status, 0, sizeof(_status));
	}

public:
	MS_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		if (blockAlign == 0)
			DEBUG(0, LEVEL_ERROR, "MS_ADPCMStream(): blockAlign isn't specified for MS ADPCM");
		memset(&_status, 0, sizeof(_status));
	}

	virtual int readBuffer(sint16 *buffer, const int numSamples);

protected:
	sint16 decodeMS(ADPCMChannelStatus *c, uint8);
};

sint16 MS_ADPCMStream::decodeMS(ADPCMChannelStatus *c, uint8 code) {
	sint32 predictor;

	predictor = (((c->sample1) * (c->coeff1)) + ((c->sample2) * (c->coeff2))) / 256;
	predictor += (signed)((code & 0x08) ? (code - 0x10) : (code)) * c->delta;

	predictor = CLIP<sint32>(predictor, -32768, 32767);

	c->sample2 = c->sample1;
	c->sample1 = predictor;
	c->delta = (MSADPCMAdaptationTable[(int)code] * c->delta) >> 8;

	if (c->delta < 16)
		c->delta = 16;

	return (sint16)predictor;
}

int MS_ADPCMStream::readBuffer(sint16 *buffer, const int numSamples) {
	int samples;
	uint8 data;
	int i = 0;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			// read block header
			for (i = 0; i < _channels; i++) {
				_status.ch[i].predictor = CLIP(_stream->readByte(), (uint8)0, (uint8)6);
				_status.ch[i].coeff1 = MSADPCMAdaptCoeff1[_status.ch[i].predictor];
				_status.ch[i].coeff2 = MSADPCMAdaptCoeff2[_status.ch[i].predictor];
			}

			for (i = 0; i < _channels; i++)
				_status.ch[i].delta = _stream->readSint16LE();

			for (i = 0; i < _channels; i++)
				_status.ch[i].sample1 = _stream->readSint16LE();

			for (i = 0; i < _channels; i++)
				buffer[samples++] = _status.ch[i].sample2 = _stream->readSint16LE();

			for (i = 0; i < _channels; i++)
				buffer[samples++] = _status.ch[i].sample1;

			_blockPos[0] = _channels * 7;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
			data = _stream->readByte();
			_blockPos[0]++;
			buffer[samples] = decodeMS(&_status.ch[0], (data >> 4) & 0x0f);
			buffer[samples + 1] = decodeMS(&_status.ch[_channels - 1], data & 0x0f);
		}
	}

	return samples;
}



#pragma mark -


class Tinsel_ADPCMStream : public ADPCMStream {
protected:
	struct {
		// Tinsel
		double predictor;
		double K0, K1;
		double d0, d1;
	} _status;

	void reset() {
		ADPCMStream::reset();
		memset(&_status, 0, sizeof(_status));
	}

	sint16 decodeTinsel(sint16, double);
	void readBufferTinselHeader();

public:
	Tinsel_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {

		if (blockAlign == 0)
			DEBUG(0, LEVEL_ERROR, "Tinsel_ADPCMStream(): blockAlign isn't specified");

		if (channels != 1)
			DEBUG(0, LEVEL_ERROR, "Tinsel_ADPCMStream(): Tinsel ADPCM only supports mono");

		memset(&_status, 0, sizeof(_status));
	}

};

static const double TinselFilterTable[4][2] = {
	{0, 0 },
	{0.9375, 0},
	{1.796875, -0.8125},
	{1.53125, -0.859375}
};

void Tinsel_ADPCMStream::readBufferTinselHeader() {
	uint8 start = _stream->readByte();
	uint8 filterVal = (start & 0xC0) >> 6;

	if ((start & 0x20) != 0) {
		//Lower 6 bit are negative

		// Negate
		start = ~(start | 0xC0) + 1;

		_status.predictor = 1 << start;
	} else {
		// Lower 6 bit are positive

		// Truncate
		start &= 0x1F;

		_status.predictor = ((double) 1.0) / (1 << start);
	}

	_status.K0 = TinselFilterTable[filterVal][0];
	_status.K1 = TinselFilterTable[filterVal][1];
}

sint16 Tinsel_ADPCMStream::decodeTinsel(sint16 code, double eVal) {
	double sample;

	sample = (double) code;
	sample *= eVal * _status.predictor;
	sample += (_status.d0 * _status.K0) + (_status.d1 * _status.K1);

	_status.d1 = _status.d0;
	_status.d0 = sample;

	return (sint16) CLIP<double>(sample, -32768.0, 32767.0);
}

class Tinsel4_ADPCMStream : public Tinsel_ADPCMStream {
public:
	Tinsel4_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Tinsel_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(sint16 *buffer, const int numSamples);
};

int Tinsel4_ADPCMStream::readBuffer(sint16 *buffer, const int numSamples) {
	int samples;
	uint16 data;
	const double eVal = 1.142822265;

	samples = 0;

	assert(numSamples % 2 == 0);

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			readBufferTinselHeader();
			_blockPos[0] = 0;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2, _blockPos[0]++) {
			// Read 1 byte = 8 bits = two 4 bit blocks
			data = _stream->readByte();
			buffer[samples] = decodeTinsel((data << 8) & 0xF000, eVal);
			buffer[samples + 1] = decodeTinsel((data << 12) & 0xF000, eVal);
		}
	}

	return samples;
}

class Tinsel6_ADPCMStream : public Tinsel_ADPCMStream {
protected:
	uint8 _chunkPos;
	uint16 _chunkData;

	void reset() {
		ADPCMStream::reset();
		_chunkPos = 0;
		_chunkData = 0;
	}

public:
	Tinsel6_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Tinsel_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		_chunkPos = 0;
		_chunkData = 0;
	}

	virtual int readBuffer(sint16 *buffer, const int numSamples);
};

int Tinsel6_ADPCMStream::readBuffer(sint16 *buffer, const int numSamples) {
	int samples;
	const double eVal = 1.032226562;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			readBufferTinselHeader();
			_blockPos[0] = 0;
			_chunkPos = 0;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples++, _chunkPos = (_chunkPos + 1) % 4) {

			switch (_chunkPos) {
			case 0:
				_chunkData = _stream->readByte();
				buffer[samples] = decodeTinsel((_chunkData << 8) & 0xFC00, eVal);
				break;
			case 1:
				_chunkData = (_chunkData << 8) | (_stream->readByte());
				buffer[samples] = decodeTinsel((_chunkData << 6) & 0xFC00, eVal);
				_blockPos[0]++;
				break;
			case 2:
				_chunkData = (_chunkData << 8) | (_stream->readByte());
				buffer[samples] = decodeTinsel((_chunkData << 4) & 0xFC00, eVal);
				_blockPos[0]++;
				break;
			case 3:
				_chunkData = (_chunkData << 8);
				buffer[samples] = decodeTinsel((_chunkData << 2) & 0xFC00, eVal);
				_blockPos[0]++;
				break;
			}

		}

	}

	return samples;
}

class Tinsel8_ADPCMStream : public Tinsel_ADPCMStream {
public:
	Tinsel8_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Tinsel_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(sint16 *buffer, const int numSamples);
};

int Tinsel8_ADPCMStream::readBuffer(sint16 *buffer, const int numSamples) {
	int samples;
	uint8 data;
	const double eVal = 1.007843258;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			readBufferTinselHeader();
			_blockPos[0] = 0;
		}

		for (; samples < numSamples && _blockPos[0] < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples++, _blockPos[0]++) {
			// Read 1 byte = 8 bits = one 8 bit block
			data = _stream->readByte();
			buffer[samples] = decodeTinsel(data << 8, eVal);
		}
	}

	return samples;
}


#pragma mark -

// Duck DK3 IMA ADPCM Decoder
// Based on FFmpeg's decoder and http://wiki.multimedia.cx/index.php?title=Duck_DK3_IMA_ADPCM

class DK3_ADPCMStream : public Ima_ADPCMStream {
protected:

	void reset() {
		Ima_ADPCMStream::reset();
		_topNibble = false;
	}

public:
	DK3_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {

		// DK3 only works as a stereo stream
		assert(channels == 2);
		_topNibble = false;
	}

	virtual int readBuffer(sint16 *buffer, const int numSamples);

private:
	uint8 _nibble, _lastByte;
	bool _topNibble;
};

#define DK3_READ_NIBBLE() \
	do { \
		if (_topNibble) { \
			_nibble = _lastByte >> 4; \
			_topNibble = false; \
		} else { \
			if (_stream->pos() >= _endpos) \
				break; \
			if ((_stream->pos() % _blockAlign) == 0) \
				continue; \
			_lastByte = _stream->readByte(); \
			_nibble = _lastByte & 0xf; \
			_topNibble = true; \
		} \
	} while (0)


int DK3_ADPCMStream::readBuffer(sint16 *buffer, const int numSamples) {
	int samples = 0;

	assert((numSamples % 4) == 0);

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if ((_stream->pos() % _blockAlign) == 0) {
			_stream->readUint16LE(); // Unknown
			uint16 rate = _stream->readUint16LE(); // Copy of rate
			_stream->skip(6); // Unknown
			// Get predictor for both sum/diff channels
			_status.ima_ch[0].last = _stream->readSint16LE();
			_status.ima_ch[1].last = _stream->readSint16LE();
			// Get index for both sum/diff channels
			_status.ima_ch[0].stepIndex = _stream->readByte();
			_status.ima_ch[1].stepIndex = _stream->readByte();

			if (_stream->eos())
				break;

			// Sanity check
			assert(rate == getRate());
		}

		DK3_READ_NIBBLE();
		decodeIMA(_nibble, 0);

		DK3_READ_NIBBLE();
		decodeIMA(_nibble, 1);

		buffer[samples++] = _status.ima_ch[0].last + _status.ima_ch[1].last;
		buffer[samples++] = _status.ima_ch[0].last - _status.ima_ch[1].last;

		DK3_READ_NIBBLE();
		decodeIMA(_nibble, 0);

		buffer[samples++] = _status.ima_ch[0].last + _status.ima_ch[1].last;
		buffer[samples++] = _status.ima_ch[0].last - _status.ima_ch[1].last;
	}

	return samples;
}


#pragma mark -


// adjust the step for use on the next sample.
sint16 ADPCMStream::stepAdjust(uint8 code) {
	static const sint16 adjusts[] = { -1, -1, -1, -1, 2, 4, 6, 8};

	return adjusts[code & 0x07];
}

static const uint16 imaStepTable[89] = {
	7,    8,    9,   10,   11,   12,   13,   14,
	16,   17,   19,   21,   23,   25,   28,   31,
	34,   37,   41,   45,   50,   55,   60,   66,
	73,   80,   88,   97,  107,  118,  130,  143,
	157,  173,  190,  209,  230,  253,  279,  307,
	337,  371,  408,  449,  494,  544,  598,  658,
	724,  796,  876,  963, 1060, 1166, 1282, 1411,
	1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767
};

sint16 Ima_ADPCMStream::decodeIMA(uint8 code, int channel) {
	sint32 E = (2 * (code & 0x7) + 1) * imaStepTable[_status.ima_ch[channel].stepIndex] / 8;
	sint32 diff = (code & 0x08) ? -E : E;
	sint32 samp = CLIP<sint32>(_status.ima_ch[channel].last + diff, -32768, 32767);

	_status.ima_ch[channel].last = samp;
	_status.ima_ch[channel].stepIndex += stepAdjust(code);
	_status.ima_ch[channel].stepIndex = CLIP<sint32>(_status.ima_ch[channel].stepIndex, 0, ARRAYSIZE(imaStepTable) - 1);

	return samp;
}

RewindableAudioStream *makeADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, typesADPCM type, int rate, int channels, uint32 blockAlign) {
	// If size is 0, report the entire size of the stream
	if (!size)
		size = stream->size();

	switch (type) {
	case kADPCMOki:
		return new Oki_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMMSIma:
		return new MSIma_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMMSImaLastExpress:
		return new MSIma_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign, true);
	case kADPCMMS:
		return new MS_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMTinsel4:
		return new Tinsel4_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMTinsel6:
		return new Tinsel6_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMTinsel8:
		return new Tinsel8_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMIma:
		return new Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMApple:
		return new Apple_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMDK3:
		return new DK3_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	default:
		DEBUG(0, LEVEL_ERROR, "Unsupported ADPCM encoding");
		break;
	}

	return NULL;
}

} // End of namespace Audio
