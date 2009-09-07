/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

// ADPCM .cpp/.h files are based upon files of the same name from ScummVM

#include "engines/stark/adpcm.h"

#include "common/endian.h"
#include "sound/audiostream.h"


namespace Stark {

class ADPCMInputStream : public Audio::AudioStream {
private:
	Common::SeekableReadStream *_stream;
	bool _disposeAfterUse;
	int32 _startpos;
	int32 _endpos;
	int _channels;
	typesADPCM _type;
	uint32 _blockAlign;
	uint32 _blockPos;
	uint8 _chunkPos;
	uint16 _chunkData;
	int _blockLen;
	int _rate;
	uint _numLoops;
	uint _curLoop;

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

		// MS ADPCM
		ADPCMChannelStatus ch[2];

		// Tinsel
		double predictor;
		double K0, K1;
		double d0, d1;
	} _status;

	void reset();
	int16 stepAdjust(byte);
	int16 decodeOKI(byte);
	int16 decodeIMA(byte code, int channel = 0); // Default to using the left channel/using one channel
	int16 decodeMS(ADPCMChannelStatus *c, byte);
	int16 decodeTinsel(int16, double);

public:
	ADPCMInputStream(Common::SeekableReadStream *stream, bool disposeAfterUse, uint32 size, typesADPCM type, int rate, int channels = 2, uint32 blockAlign = 0, uint numLoops = 1);
	~ADPCMInputStream();

	int readBuffer(int16 *buffer, const int numSamples);
	int readBufferOKI(int16 *buffer, const int numSamples);
	int readBufferIMA(int16 *buffer, const int numSamples);
	int readBufferMSIMA1(int16 *buffer, const int numSamples);
	int readBufferMSIMA2(int16 *buffer, const int numSamples);
	int readBufferMS(int channels, int16 *buffer, const int numSamples);
	void readBufferTinselHeader();
	int readBufferTinsel4(int channels, int16 *buffer, const int numSamples);
	int readBufferTinsel6(int channels, int16 *buffer, const int numSamples);
	int readBufferTinsel8(int channels, int16 *buffer, const int numSamples);
	int readBufferISS(int16 *buffer, const int numSamples);

	bool endOfData() const { return (_stream->eos() || _stream->pos() >= _endpos); }
	bool isStereo() const	{ return _channels == 2; }
	int getRate() const	{ return _rate; }
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

ADPCMInputStream::ADPCMInputStream(Common::SeekableReadStream *stream, bool disposeAfterUse, uint32 size, typesADPCM type, int rate, int channels, uint32 blockAlign, uint numLoops)
	: _stream(stream), _disposeAfterUse(disposeAfterUse), _channels(channels), _type(type), _blockAlign(blockAlign), _rate(rate), _numLoops(numLoops) {

	if (type == kADPCMMSIma && blockAlign == 0)
		error("ADPCMInputStream(): blockAlign isn't specified for MS IMA ADPCM");
	if (type == kADPCMMS && blockAlign == 0)
		error("ADPCMInputStream(): blockAlign isn't specified for MS ADPCM");

	if (type == kADPCMTinsel4 && blockAlign == 0)
		error("ADPCMInputStream(): blockAlign isn't specified for Tinsel 4-bit ADPCM");
	if (type == kADPCMTinsel6 && blockAlign == 0)
		error("ADPCMInputStream(): blockAlign isn't specified for Tinsel 6-bit ADPCM");
	if (type == kADPCMTinsel8 && blockAlign == 0)
		error("ADPCMInputStream(): blockAlign isn't specified for Tinsel 8-bit ADPCM");
		
	if (type == kADPCMISS && blockAlign == 0)
		error("ADPCMInputStream(): blockAlign isn't specified for ISS IMA ADPCM");

	if (type == kADPCMTinsel4 && channels != 1)
		error("ADPCMInputStream(): Tinsel 4-bit ADPCM only supports mono");
	if (type == kADPCMTinsel6 && channels != 1)
		error("ADPCMInputStream(): Tinsel 6-bit ADPCM only supports mono");
	if (type == kADPCMTinsel8 && channels != 1)
		error("ADPCMInputStream(): Tinsel 8-bit ADPCM only supports mono");

	_startpos = stream->pos();
	_endpos = _startpos + size;
	_curLoop = 0;
	_blockPos = 0;
	reset();
}

ADPCMInputStream::~ADPCMInputStream() {
	if (_disposeAfterUse)
		delete _stream;
}

void ADPCMInputStream::reset() {
	memset(&_status, 0, sizeof(_status));
	_blockLen = 0;
	_blockPos = _blockAlign; // To make sure first header is read
	_chunkPos = 0;
}

int ADPCMInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesDecoded = 0;
	switch (_type) {
	case kADPCMOki:
		samplesDecoded = readBufferOKI(buffer, numSamples);
		break;
	case kADPCMMSIma:
		if (_channels == 1)
			samplesDecoded = readBufferMSIMA1(buffer, numSamples);
		else
			samplesDecoded = readBufferMSIMA2(buffer, numSamples);
		break;
	case kADPCMMS:
		samplesDecoded = readBufferMS(_channels, buffer, numSamples);
		break;
	case kADPCMTinsel4:
		samplesDecoded = readBufferTinsel4(_channels, buffer, numSamples);
		break;
	case kADPCMTinsel6:
		samplesDecoded = readBufferTinsel6(_channels, buffer, numSamples);
		break;
	case kADPCMTinsel8:
		samplesDecoded = readBufferTinsel8(_channels, buffer, numSamples);
		break;
	case kADPCMIma:
		samplesDecoded = readBufferIMA(buffer, numSamples);
		break;
	case kADPCMISS:
		samplesDecoded = readBufferISS(buffer, numSamples);
		break;
	default:
		error("Unsupported ADPCM encoding");
		break;
	}

	// Loop if necessary
	if (samplesDecoded < numSamples || _stream->pos() == _endpos) {
		_curLoop++;
		if (_numLoops == 0 || _curLoop < _numLoops) {
			reset();
			_stream->seek(_startpos);
			return samplesDecoded + readBuffer(buffer + samplesDecoded, numSamples - samplesDecoded);
		}
	}

	return samplesDecoded;
}

int ADPCMInputStream::readBufferOKI(int16 *buffer, const int numSamples) {
	int samples;
	byte data;

	assert(numSamples % 2 == 0);

	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
		data = _stream->readByte();
		buffer[samples] = TO_LE_16(decodeOKI((data >> 4) & 0x0f));
		buffer[samples + 1] = TO_LE_16(decodeOKI(data & 0x0f));
	}
	return samples;
}

int ADPCMInputStream::readBufferIMA(int16 *buffer, const int numSamples) {
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

int ADPCMInputStream::readBufferMSIMA1(int16 *buffer, const int numSamples) {
	int samples = 0;
	byte data;

	assert(numSamples % 2 == 0);

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos == _blockAlign) {
			// read block header
			_status.ima_ch[0].last = _stream->readSint16LE();
			_status.ima_ch[0].stepIndex = _stream->readSint16LE();
			_blockPos = 4;
		}

		for (; samples < numSamples && _blockPos < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
			data = _stream->readByte();
			_blockPos++;
			buffer[samples] = TO_LE_16(decodeIMA(data & 0x0f));
			buffer[samples + 1] = TO_LE_16(decodeIMA((data >> 4) & 0x0f));
		}
	}
	return samples;
}

int ADPCMInputStream::readBufferISS(int16 *buffer, const int numSamples) {
	int samples;
	
	assert(numSamples % 2 == 0);
	
	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
		if (_blockPos == _blockAlign) {
			// read block header
			for (byte i = 0; i < _channels; i++) {
				_status.ima_ch[i].last = _stream->readSint16LE();
				_status.ima_ch[i].stepIndex = _stream->readSint16LE();
			}
			_blockPos = 4 * _channels;
		}
	
		byte data = _stream->readByte();
		buffer[samples] = decodeIMA((data >> 4) & 0x0f);
		buffer[samples + 1] = decodeIMA(data & 0x0f, _channels == 2 ? 1 : 0);
		_blockPos++;
	}
	
	return samples;
}


// Microsoft as usual tries to implement it differently. This method
// is used for stereo data.
int ADPCMInputStream::readBufferMSIMA2(int16 *buffer, const int numSamples) {
	int samples;
	uint32 data;
	int nibble;
	byte k;

	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos;) {
		for (int channel = 0; channel < 2; channel++) {
			data = _stream->readUint32LE();

			for (nibble = 0; nibble < 8; nibble++) {
				k = ((data & 0xf0000000) >> 28);
				buffer[samples + channel + nibble * 2] = TO_LE_16(decodeIMA(k));
				data <<= 4;
			}
		}
		samples += 16;
	}
	return samples;
}

static const int MSADPCMAdaptCoeff1[] = {
	256, 512, 0, 192, 240, 460, 392
};

static const int MSADPCMAdaptCoeff2[] = {
	0, -256, 0, 64, 0, -208, -232
};

int ADPCMInputStream::readBufferMS(int channels, int16 *buffer, const int numSamples) {
	int samples;
	byte data;
	int i = 0;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos == _blockAlign) {
			// read block header
			for (i = 0; i < channels; i++) {
				_status.ch[i].predictor = CLIP(_stream->readByte(), (byte)0, (byte)6);
				_status.ch[i].coeff1 = MSADPCMAdaptCoeff1[_status.ch[i].predictor];
				_status.ch[i].coeff2 = MSADPCMAdaptCoeff2[_status.ch[i].predictor];
			}

			for (i = 0; i < channels; i++)
				_status.ch[i].delta = _stream->readSint16LE();

			for (i = 0; i < channels; i++)
				_status.ch[i].sample1 = _stream->readSint16LE();

			for (i = 0; i < channels; i++) {
				_status.ch[i].sample2 = _stream->readSint16LE();
				buffer[samples++] = TO_LE_16(_status.ch[i].sample2);
			}

			for (i = 0; i < channels; i++)
				buffer[samples++] = TO_LE_16(_status.ch[i].sample1);

			_blockPos = channels * 7;
		}

		for (; samples < numSamples && _blockPos < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2) {
			data = _stream->readByte();
			_blockPos++;
			buffer[samples] = TO_LE_16(decodeMS(&_status.ch[0], (data >> 4) & 0x0f));
			buffer[samples + 1] = TO_LE_16(decodeMS(&_status.ch[channels - 1], data & 0x0f));
		}
	}

	return samples;
}

static const double TinselFilterTable[4][2] = {
	{0, 0 },
	{0.9375, 0},
	{1.796875, -0.8125},
	{1.53125, -0.859375}
};

void ADPCMInputStream::readBufferTinselHeader() {
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

int ADPCMInputStream::readBufferTinsel4(int channels, int16 *buffer, const int numSamples) {
	int samples;
	uint16 data;
	const double eVal = 1.142822265;

	samples = 0;

	assert(numSamples % 2 == 0);

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos == _blockAlign) {
			readBufferTinselHeader();
			_blockPos = 0;
		}

		for (; samples < numSamples && _blockPos < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples += 2, _blockPos++) {
			// Read 1 byte = 8 bits = two 4 bit blocks
			data = _stream->readByte();
			buffer[samples] = decodeTinsel((data << 8) & 0xF000, eVal);
			buffer[samples+1] = decodeTinsel((data << 12) & 0xF000, eVal);
		}
	}

	return samples;
}

int ADPCMInputStream::readBufferTinsel6(int channels, int16 *buffer, const int numSamples) {
	int samples;
	const double eVal = 1.032226562;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos == _blockAlign) {
			readBufferTinselHeader();
			_blockPos = 0;
			_chunkPos = 0;
		}

		for (; samples < numSamples && _blockPos < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples++, _chunkPos = (_chunkPos + 1) % 4) {

			switch (_chunkPos) {
			case 0:
				_chunkData = _stream->readByte();
				buffer[samples] = decodeTinsel((_chunkData << 8) & 0xFC00, eVal);
				break;
			case 1:
				_chunkData = (_chunkData << 8) | (_stream->readByte());
				buffer[samples] = decodeTinsel((_chunkData << 6) & 0xFC00, eVal);
				_blockPos++;
				break;
			case 2:
				_chunkData = (_chunkData << 8) | (_stream->readByte());
				buffer[samples] = decodeTinsel((_chunkData << 4) & 0xFC00, eVal);
				_blockPos++;
				break;
			case 3:
				_chunkData = (_chunkData << 8);
				buffer[samples] = decodeTinsel((_chunkData << 2) & 0xFC00, eVal);
				_blockPos++;
				break;
			}

		}

	}

	return samples;
}

int ADPCMInputStream::readBufferTinsel8(int channels, int16 *buffer, const int numSamples) {
	int samples;
	byte data;
	const double eVal = 1.007843258;

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		if (_blockPos == _blockAlign) {
			readBufferTinselHeader();
			_blockPos = 0;
		}

		for (; samples < numSamples && _blockPos < _blockAlign && !_stream->eos() && _stream->pos() < _endpos; samples++, _blockPos++) {
			// Read 1 byte = 8 bits = one 8 bit block
			data = _stream->readByte();
			buffer[samples] = decodeTinsel(data << 8, eVal);
		}
	}

	return samples;
}

static const int MSADPCMAdaptationTable[] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};


int16 ADPCMInputStream::decodeMS(ADPCMChannelStatus *c, byte code) {
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
int16 ADPCMInputStream::stepAdjust(byte code) {
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
int16 ADPCMInputStream::decodeOKI(byte code) {
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

int16 ADPCMInputStream::decodeIMA(byte code, int channel) {
	int32 E = (2 * (code & 0x7) + 1) * imaStepTable[_status.ima_ch[channel].stepIndex] / 8;
	int32 diff = (code & 0x08) ? -E : E;
	int32 samp = CLIP<int32>(_status.ima_ch[channel].last + diff, -32768, 32767);

	_status.ima_ch[channel].last = samp;
	_status.ima_ch[channel].stepIndex += stepAdjust(code);
	_status.ima_ch[channel].stepIndex = CLIP<int32>(_status.ima_ch[channel].stepIndex, 0, ARRAYSIZE(imaStepTable) - 1);

	return samp;
}

int16 ADPCMInputStream::decodeTinsel(int16 code, double eVal) {
	double sample;

	sample = (double) code;
	sample *= eVal * _status.predictor;
	sample += (_status.d0 * _status.K0) + (_status.d1 * _status.K1);

	_status.d1 = _status.d0;
	_status.d0 = sample;

	return (int16) CLIP<double>(sample, -32768.0, 32767.0);
}

Audio::AudioStream *makeADPCMStream(Common::SeekableReadStream *stream, bool disposeAfterUse, uint32 size, typesADPCM type, int rate, int channels, uint32 blockAlign, uint numLoops) {
	return new ADPCMInputStream(stream, disposeAfterUse, size, type, rate, channels, blockAlign, numLoops);
}

} // End of namespace Audio
