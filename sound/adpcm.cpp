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

#include "sound/adpcm.h"


// Routines to convert 12 bit linear samples to the
// Dialogic or Oki ADPCM coding format aka VOX.
// See also <http://www.comptek.ru/telephony/tnotes/tt1-13.html>
//
// In addition, also IMA ADPCM is supported.

ADPCMInputStream::ADPCMInputStream(Common::SeekableReadStream *stream, uint32 size, typesADPCM type, int channels, uint32 blockAlign)
	: _stream(stream), _channels(channels), _type(type), _blockAlign(blockAlign) {

	_status.last = 0;
	_status.stepIndex = 0;
	_endpos = stream->pos() + size;

	if (type == kADPCMIma && blockAlign == 0)
		error("ADPCMInputStream(): blockAlign isn't specifiled for MS ADPCM IMA");
}

int ADPCMInputStream::readBuffer(int16 *buffer, const int numSamples) {
	switch (_type) {
	case kADPCMOki:
		return readBufferOKI(buffer, numSamples);
		break;
	case kADPCMIma:
		if (_channels == 1)
			return readBufferMSIMA1(buffer, numSamples);
		else
			return readBufferMSIMA2(buffer, numSamples);
		break;
	default:
		error("Unsupported ADPCM encoding");
		break;
	}
	return 0;
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


int ADPCMInputStream::readBufferMSIMA1(int16 *buffer, const int numSamples) {
	int samples;
	byte data;
	int blockLen;
	int i;

	assert(numSamples % 2 == 0);

	samples = 0;

	while (samples < numSamples && !_stream->eos() && _stream->pos() < _endpos) {
		// read block header
		_status.last = _stream->readSint16LE();
		_status.stepIndex = _stream->readSint16LE();

		blockLen = MIN(_endpos - _stream->pos(), _blockAlign - 4);

		for (i = 0; i < blockLen && !_stream->eos() && _stream->pos() < _endpos; i++, samples += 2) {
			data = _stream->readByte();
			buffer[samples] = TO_LE_16(decodeMSIMA(data & 0x0f));
			buffer[samples + 1] = TO_LE_16(decodeMSIMA((data >> 4) & 0x0f));
		}
	}
	return samples;
}


// Microsoft as usual tries to implement it differently. This method
// is used for stereo data.
int ADPCMInputStream::readBufferMSIMA2(int16 *buffer, const int numSamples) {
	int samples;
	uint32 data;
	int nibble;

	for (samples = 0; samples < numSamples && !_stream->eos() && _stream->pos() < _endpos;) {
		for (int channel = 0; channel < 2; channel++) {
			data = _stream->readUint32LE();
			
			for (nibble = 0; nibble < 8; nibble++) {
				byte k = ((data & 0xf0000000) >> 28);
				buffer[samples + channel + nibble * 2] = TO_LE_16(decodeMSIMA(k));
				data <<= 4;
			}
		}
		samples += 16;
	}
	return samples;
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

int16 ADPCMInputStream::decodeMSIMA(byte code) {
	int32 diff, E, SS, samp;

	SS = imaStepTable[_status.stepIndex];
	E = SS >> 3;
	if (code & 0x04)
		E += SS;
	if (code & 0x02)
		E += SS >> 1;
	if (code & 0x01)
		E += SS >> 2;
	diff = (code & 0x08) ? -E : E;
	samp = _status.last + diff;

	if (samp < -0x8000)
		samp = -0x8000;
	else if (samp > 0x7fff)
		samp = 0x7fff;

	_status.last = samp;

	_status.stepIndex += stepAdjust(code);
	if (_status.stepIndex < 0)
		_status.stepIndex = 0;
	if (_status.stepIndex > 88)
		_status.stepIndex = 88;

	return samp;
}
