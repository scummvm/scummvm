/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/stream.h"

#include "sound/audiostream.h"
#include "sound/adpcm.h"


// Routines to convert 12 bit linear samples to the
// Dialogic or Oki ADPCM coding format aka VOX.
// See also <http://www.comptek.ru/telephony/tnotes/tt1-13.html>
//
// In addition, also IMA ADPCM is supported.
template <typesADPCM TYPE>
class ADPCMInputStream : public AudioStream {
private:
	bool _evenPos;
	byte _lastByte;
	Common::SeekableReadStream *_stream;
	uint32 _endpos;

	struct adpcmStatus {
		int16 last;
		int16 stepIndex;
	} _status;

	int16 stepAdjust(byte);
	int16 decode(byte);

public:
	ADPCMInputStream(Common::SeekableReadStream *stream, uint32 size);
	~ADPCMInputStream() {};

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const { return (_stream->eos() || _stream->pos() >= _endpos); }
	bool isStereo() const	{ return false; }
	int getRate() const	{ return 22050; }
};


template <typesADPCM TYPE>
ADPCMInputStream<TYPE>::ADPCMInputStream(Common::SeekableReadStream *stream, uint32 size)
	: _stream(stream), _evenPos(true) {

	_status.last = 0;
	_status.stepIndex = 0;
	_endpos = stream->pos() + size;
}

template <typesADPCM TYPE>
int ADPCMInputStream<TYPE>::readBuffer(int16 *buffer, const int numSamples) {
	int samples;

	// Since we process high and low nibbles separately never check buffer end
	// on low nibble
	for (samples = 0; !_evenPos || samples < numSamples && !_stream->eos() && _stream->pos() < _endpos; samples++) {
		if (_evenPos) {
			_lastByte = _stream->readByte();
			buffer[samples] = decode((_lastByte >> 4) & 0x0f);
		} else {
			buffer[samples] = decode(_lastByte & 0x0f);
		}
		_evenPos = !_evenPos;
	}
	return samples;
}

// adjust the step for use on the next sample.
template <typesADPCM TYPE>
int16 ADPCMInputStream<TYPE>::stepAdjust(byte code) {
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
template <>
int16 ADPCMInputStream<kADPCMOki>::decode(byte code) {
	int16 diff, E, SS, samp;

	SS = okiStepSize[_status.stepIndex];
	E = SS/8;
	if (code & 0x01)
		E += SS/4;
	if (code & 0x02)
		E += SS/2;
	if (code & 0x04)
		E += SS;
	diff = (code & 0x08) ? -E : E;
	samp = _status.last + diff;

    // Clip the values to +/- 2^11 (supposed to be 12 bits)
	if (samp > 2048)
		samp = 2048;
	if (samp < -2048)
		samp = -2048;

	_status.last = samp;
	_status.stepIndex += stepAdjust(code);
	if (_status.stepIndex < 0)
		_status.stepIndex = 0;
	if (_status.stepIndex > 48)
		_status.stepIndex = 48;

	// * 16 effectively converts 12-bit input to 16-bit output
	return samp * 16;
}


static const uint16 imaStepTable[89] = {
		7,	  8,	9,	 10,   11,	 12,   13,	 14,
	   16,	 17,   19,	 21,   23,	 25,   28,	 31,
	   34,	 37,   41,	 45,   50,	 55,   60,	 66,
	   73,	 80,   88,	 97,  107,	118,  130,	143,
	  157,	173,  190,	209,  230,	253,  279,	307,
	  337,	371,  408,	449,  494,	544,  598,	658,
	  724,	796,  876,	963, 1060, 1166, 1282, 1411,
	 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	 7132, 7845, 8630, 9493,10442,11487,12635,13899,
	15289,16818,18500,20350,22385,24623,27086,29794,
	32767
};

template <>
int16 ADPCMInputStream<kADPCMIma>::decode(byte code) {
	int32 diff, E, SS, samp;

	SS = imaStepTable[_status.stepIndex];
	E = SS/8;
	if (code & 0x01)
		E += SS/4;
	if (code & 0x02)
		E += SS/2;
	if (code & 0x04)
		E += SS;
	diff = (code & 0x08) ? -E : E;
	samp = _status.last + diff;

	if (samp < -32768)
		samp = -32768;
	else if (samp > 32767)
		samp = 32767;

	_status.last = samp;
	_status.stepIndex += stepAdjust(code);
	if (_status.stepIndex < 0)
		_status.stepIndex = 0;
	if (_status.stepIndex > 88)
		_status.stepIndex = 88;

	return samp;
}

AudioStream *makeADPCMStream(Common::SeekableReadStream &stream, uint32 size, typesADPCM type) {
	AudioStream *audioStream;

	switch (type) {
	case kADPCMOki:
		audioStream = new ADPCMInputStream<kADPCMOki>(&stream, size);
		break;
	case kADPCMIma:
		audioStream = new ADPCMInputStream<kADPCMIma>(&stream, size);
		break;
	default:
		error("Unsupported ADPCM encoding");
		break;
	}

	return audioStream;
}
