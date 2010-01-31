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

#include "common/endian.h"

#include "sound/decoders/adpcm.h"
#include "sound/audiostream.h"


namespace Audio {

class ADPCMStream : public RewindableAudioStream {
protected:
	Common::SeekableReadStream *_stream;
	const DisposeAfterUse::Flag _disposeAfterUse;
	const int32 _startpos;
	const int32 _endpos;
	const int _channels;
	const uint32 _blockAlign;
	uint32 _blockPos[2];
	uint8 _chunkPos;
	uint16 _chunkData;
	int _blockLen;
	const int _rate;

	struct ADPCMChannelStatus {
		byte predictor;
		int16 delta;
		int16 coeff1;
		int16 coeff2;
		int16 sample1;
		int16 sample2;
	};

	struct adpcmStatus {
		// OKI/IMA
		struct {
			int32 last;
			int32 stepIndex;
		} ima_ch[2];

		// Apple QuickTime IMA ADPCM
		int32 streamPos[2];

		// MS ADPCM
		ADPCMChannelStatus ch[2];

		// Tinsel
		double predictor;
		double K0, K1;
		double d0, d1;
	} _status;

	void reset();
	int16 stepAdjust(byte);
	int16 decodeIMA(byte code, int channel = 0); // Default to using the left channel/using one channel

public:
	ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign);
	~ADPCMStream();

	virtual bool endOfData() const { return (_stream->eos() || _stream->pos() >= _endpos); }
	virtual bool isStereo() const	{ return _channels == 2; }
	virtual int getRate() const	{ return _rate; }

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
	_blockLen = 0;
	_blockPos[0] = _blockPos[1] = _blockAlign; // To make sure first header is read
	_status.streamPos[0] = 0;
	_status.streamPos[1] = _blockAlign;
	_chunkPos = 0;
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

	virtual int readBuffer(int16 *buffer, const int numSamples);

protected:
	int16 decodeOKI(byte);
};

int Oki_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	byte data;

	assert(numSamples % 2 == 0);

	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
		data = _stream->readByte();
		buffer[samples] = decodeOKI((data >> 4) & 0x0f);
		buffer[samples + 1] = decodeOKI(data & 0x0f);
	}
	return samples;
}

#pragma mark -


class Ima_ADPCMStream : public ADPCMStream {
public:
	Ima_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(int16 *buffer, const int numSamples);
};

int Ima_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	byte data;

	assert(numSamples % 2 == 0);

	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
		data = _stream->readByte();
		buffer[samples] = decodeIMA((data >> 4) & 0x0f);
		buffer[samples + 1] = decodeIMA(data & 0x0f, _channels == 2 ? 1 : 0);
	}
	return samples;
}

#pragma mark -


class Apple_ADPCMStream : public ADPCMStream {
public:
	Apple_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(int16 *buffer, const int numSamples);
};

int Apple_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	// Need to write 2 samples per channel
	assert(numSamples % (2 * _channels) == 0);

	// Current sample positions
	int    samples[2] = {   0,    0};
	// Current data bytes
	byte      data[2] = {   0,    0};
	// Current nibble selectors
	bool lowNibble[2] = {true, true};

	// Number of samples per channel
	int chanSamples = numSamples / _channels;

	for (int i = 0; i < _channels; i++) {
		_stream->seek(_status.streamPos[i]);

		while ((samples[i] < chanSamples) &&
		       // Last byte read and a new one needed
		       !((_stream->eos() || (_stream->pos() >= _endpos)) && lowNibble[i])) {

			if (_blockPos[i] == _blockAlign) {
				// 2 byte header per block
				uint16 temp = _stream->readUint16BE();

				// First 9 bits are the upper bits of the predictor
				_status.ima_ch[i].last      = (int16) (temp & 0xFF80);
				// Lower 7 bits are the step index
				_status.ima_ch[i].stepIndex =          temp & 0x007F;

				// Clip the step index
				_status.ima_ch[i].stepIndex = CLIP<int32>(_status.ima_ch[i].stepIndex, 0, 88);

				_blockPos[i] = 2;
			}

			// First decode the lower nibble, then the upper
			if (lowNibble[i])
				data[i] = _stream->readByte();

			int16 sample;
			if (lowNibble[i])
				sample = decodeIMA(data[i] &  0x0F, i);
			else
				sample = decodeIMA(data[i] >>    4, i);

			// The original is interleaved block-wise, we want it sample-wise
			buffer[_channels * samples[i] + i] = sample;

			samples[i]++;

			// Different nibble
			lowNibble[i] = !lowNibble[i];

			// We're about to decode a new lower nibble again, so advance the block position
			if (lowNibble[i])
				_blockPos[i]++;

			if (_channels == 2)
				if (_blockPos[i] == _blockAlign)
					// We're at the end of the block.
					// Since the channels are interleaved, skip the next block
					_stream->skip(MIN<uint32>(_blockAlign, _endpos - _stream->pos()));

			_status.streamPos[i] = _stream->pos();
		}
	}

	return samples[0] + samples[1];
}

#pragma mark -


class MSIma_ADPCMStream : public ADPCMStream {
public:
	MSIma_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		if (blockAlign == 0)
			error("ADPCMStream(): blockAlign isn't specified for MS IMA ADPCM");
	}

	virtual int readBuffer(int16 *buffer, const int numSamples) {
		if (_channels == 1)
			return readBufferMSIMA1(buffer, numSamples);
		else
			return readBufferMSIMA2(buffer, numSamples);
	}

	int readBufferMSIMA1(int16 *buffer, const int numSamples);
	int readBufferMSIMA2(int16 *buffer, const int numSamples);
};

int MSIma_ADPCMStream::readBufferMSIMA1(int16 *buffer, const int numSamples) {
	int samples = 0;
	byte data;

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
			buffer[samples] = decodeIMA(data & 0x0f);
			buffer[samples + 1] = decodeIMA((data >> 4) & 0x0f);
		}
	}
	return samples;
}


// Microsoft as usual tries to implement it differently. This method
// is used for stereo data.
int MSIma_ADPCMStream::readBufferMSIMA2(int16 *buffer, const int numSamples) {
	int samples;
	uint32 data;
	int nibble;
	byte k;

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

class MS_ADPCMStream : public ADPCMStream {
public:
	MS_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		if (blockAlign == 0)
			error("MS_ADPCMStream(): blockAlign isn't specified for MS ADPCM");
	}

	virtual int readBuffer(int16 *buffer, const int numSamples);

protected:
	int16 decodeMS(ADPCMChannelStatus *c, byte);
};

int MS_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	byte data;
	int i = 0;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos[0] == _blockAlign) {
			// read block header
			for (i = 0; i < _channels; i++) {
				_status.ch[i].predictor = CLIP(_stream->readByte(), (byte)0, (byte)6);
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
public:
	Tinsel_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {

		if (blockAlign == 0)
			error("Tinsel_ADPCMStream(): blockAlign isn't specified");

		if (channels != 1)
			error("Tinsel_ADPCMStream(): Tinsel ADPCM only supports mono");

	}

	int16 decodeTinsel(int16, double);

	void readBufferTinselHeader();
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

class Tinsel4_ADPCMStream : public Tinsel_ADPCMStream {
public:
	Tinsel4_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Tinsel_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(int16 *buffer, const int numSamples);
};

int Tinsel4_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
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
			buffer[samples+1] = decodeTinsel((data << 12) & 0xF000, eVal);
		}
	}

	return samples;
}

class Tinsel6_ADPCMStream : public Tinsel_ADPCMStream {
public:
	Tinsel6_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Tinsel_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	virtual int readBuffer(int16 *buffer, const int numSamples);
};

int Tinsel6_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
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

	virtual int readBuffer(int16 *buffer, const int numSamples);
};

int Tinsel8_ADPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	byte data;
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


static const int MSADPCMAdaptationTable[] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};


int16 MS_ADPCMStream::decodeMS(ADPCMChannelStatus *c, byte code) {
	int32 predictor;

	predictor = (((c->sample1) * (c->coeff1)) + ((c->sample2) * (c->coeff2))) / 256;
	predictor += (signed)((code & 0x08) ? (code - 0x10) : (code)) * c->delta;

	predictor = CLIP<int32>(predictor, -32768, 32767);

	c->sample2 = c->sample1;
	c->sample1 = predictor;
	c->delta = (MSADPCMAdaptationTable[(int)code] * c->delta) >> 8;

	if (c->delta < 16)
		c->delta = 16;

	return (int16)predictor;
}

// adjust the step for use on the next sample.
int16 ADPCMStream::stepAdjust(byte code) {
	static const int16 adjusts[] = {-1, -1, -1, -1, 2, 4, 6, 8};

	return adjusts[code & 0x07];
}

static const int16 okiStepSize[49] = {
	   16,   17,   19,   21,   23,   25,   28,   31,
	   34,   37,   41,   45,   50,   55,   60,   66,
	   73,   80,   88,   97,  107,  118,  130,  143,
	  157,  173,  190,  209,  230,  253,  279,  307,
	  337,  371,  408,  449,  494,  544,  598,  658,
	  724,  796,  876,  963, 1060, 1166, 1282, 1411,
	 1552
};

// Decode Linear to ADPCM
int16 Oki_ADPCMStream::decodeOKI(byte code) {
	int16 diff, E, samp;

	E = (2 * (code & 0x7) + 1) * okiStepSize[_status.ima_ch[0].stepIndex] / 8;
	diff = (code & 0x08) ? -E : E;
	samp = _status.ima_ch[0].last + diff;
	// Clip the values to +/- 2^11 (supposed to be 12 bits)
	samp = CLIP<int16>(samp, -2048, 2047);

	_status.ima_ch[0].last = samp;
	_status.ima_ch[0].stepIndex += stepAdjust(code);
	_status.ima_ch[0].stepIndex = CLIP<int32>(_status.ima_ch[0].stepIndex, 0, ARRAYSIZE(okiStepSize) - 1);

	// * 16 effectively converts 12-bit input to 16-bit output
	return samp * 16;
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
	 7132, 7845, 8630, 9493,10442,11487,12635,13899,
	15289,16818,18500,20350,22385,24623,27086,29794,
	32767
};

int16 ADPCMStream::decodeIMA(byte code, int channel) {
	int32 E = (2 * (code & 0x7) + 1) * imaStepTable[_status.ima_ch[channel].stepIndex] / 8;
	int32 diff = (code & 0x08) ? -E : E;
	int32 samp = CLIP<int32>(_status.ima_ch[channel].last + diff, -32768, 32767);

	_status.ima_ch[channel].last = samp;
	_status.ima_ch[channel].stepIndex += stepAdjust(code);
	_status.ima_ch[channel].stepIndex = CLIP<int32>(_status.ima_ch[channel].stepIndex, 0, ARRAYSIZE(imaStepTable) - 1);

	return samp;
}

int16 Tinsel_ADPCMStream::decodeTinsel(int16 code, double eVal) {
	double sample;

	sample = (double) code;
	sample *= eVal * _status.predictor;
	sample += (_status.d0 * _status.K0) + (_status.d1 * _status.K1);

	_status.d1 = _status.d0;
	_status.d0 = sample;

	return (int16) CLIP<double>(sample, -32768.0, 32767.0);
}

RewindableAudioStream *makeADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, typesADPCM type, int rate, int channels, uint32 blockAlign) {
	switch (type) {
	case kADPCMOki:
		return new Oki_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
	case kADPCMMSIma:
		return new MSIma_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign);
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
	default:
		error("Unsupported ADPCM encoding");
		break;
	}
}

} // End of namespace Audio
