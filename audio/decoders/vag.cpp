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
 */

#include "audio/decoders/vag.h"
#include "audio/audiostream.h"
#include "common/stream.h"

namespace Audio {

class VagStream : public Audio::RewindableAudioStream {
public:
	VagStream(Common::SeekableReadStream *stream, int rate);
	~VagStream();

	bool isStereo() const { return false; }
	bool endOfData() const { return _stream->pos() == _stream->size(); }
	int getRate() const { return _rate; }
	int readBuffer(int16 *buffer, const int numSamples);

	bool rewind();
private:
	Common::SeekableReadStream *_stream;

	byte _predictor;
	double _samples[28];
	byte _samplesRemaining;
	int _rate;
	double _s1, _s2;
};

VagStream::VagStream(Common::SeekableReadStream *stream, int rate) : _stream(stream) {
	_samplesRemaining = 0;
	_predictor = 0;
	_s1 = _s2 = 0.0;
	_rate = rate;
}


VagStream::~VagStream() {
	delete _stream;
}

static const double s_vagDataTable[5][2] =
	{
		{  0.0, 0.0 },
		{  60.0 / 64.0,  0.0 },
		{  115.0 / 64.0, -52.0 / 64.0 },
		{  98.0 / 64.0, -55.0 / 64.0 },
		{  122.0 / 64.0, -60.0 / 64.0 }
	};

int VagStream::readBuffer(int16 *buffer, const int numSamples) {
	int32 samplesDecoded = 0;

	if (_samplesRemaining) {
		byte i = 0;

		for (i = 28 - _samplesRemaining; i < 28 && samplesDecoded < numSamples; i++) {
			_samples[i] = _samples[i] + _s1 * s_vagDataTable[_predictor][0] + _s2 * s_vagDataTable[_predictor][1];
			_s2 = _s1;
			_s1 = _samples[i];
			int16 d = (int) (_samples[i] + 0.5);
			buffer[samplesDecoded] = d;
			samplesDecoded++;
		}

#if 0
		assert(i == 28); // We're screwed if this fails :P
#endif
		// This might mean the file is corrupted, or that the stream has
		// been closed.
		if (i != 28) return 0;

		_samplesRemaining = 0;
	}

	while (samplesDecoded < numSamples) {
		byte i = 0;

		_predictor = _stream->readByte();
		byte shift = _predictor & 0xf;
		_predictor >>= 4;

		if (_stream->readByte() == 7)
			return samplesDecoded;

		for (i = 0; i < 28; i += 2) {
			byte d = _stream->readByte();
			int16 s = (d & 0xf) << 12;
			if (s & 0x8000)
				s |= 0xffff0000;
			_samples[i] = (double)(s >> shift);
			s = (d & 0xf0) << 8;
			if (s & 0x8000)
				s |= 0xffff0000;
			_samples[i + 1] = (double)(s >> shift);
		}

		for (i = 0; i < 28 && samplesDecoded < numSamples; i++) {
			_samples[i] = _samples[i] + _s1 * s_vagDataTable[_predictor][0] + _s2 * s_vagDataTable[_predictor][1];
			_s2 = _s1;
			_s1 = _samples[i];
			int16 d = (int) (_samples[i] + 0.5);
			buffer[samplesDecoded] = d;
			samplesDecoded++;
		}

		if (i != 27)
			_samplesRemaining = 28 - i;
	}

	return samplesDecoded;
}

bool VagStream::rewind() {
	_stream->seek(0);
	_samplesRemaining = 0;
	_predictor = 0;
	_s1 = _s2 = 0.0;

	return true;
}

RewindableAudioStream *makeVagStream(Common::SeekableReadStream *stream, int rate) {
	return new VagStream(stream, rate);
}

}
