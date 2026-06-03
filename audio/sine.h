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

#ifndef AUDIO_SINE_H
#define AUDIO_SINE_H

#include "audio/audiostream.h"
#include "common/util.h"

namespace Audio {

class SineStream : public AudioStream {
public:
	SineStream(int freq, int rate = 44100) {
		_rate = rate;
		_oscLength = rate / freq;
		_oscSamples = 0;
		_volume = 20; // The maximum volume is 255
	}


	int readBuffer(int16 *buffer, const int numSamples) override {
		for (int i = 0; i < numSamples; i++) {
			buffer[i] = generateSine(_oscSamples, _oscLength) * _volume;
			if (_oscSamples++ >= _oscLength)
				_oscSamples = 0;
		}

		return numSamples;
	}

	bool isStereo() const override { return false; }
	bool endOfData() const override { return false; }
	bool endOfStream() const override { return false; }
	int getRate() const override { return _rate; }

protected:
	int _rate;
	uint32 _oscLength;
	uint32 _oscSamples;
	byte _volume;

	int8 generateSine(uint32 x, uint32 oscLength) const {
		if (oscLength == 0)
			return 0;

		// TODO: Maybe using a look-up-table would be better?
		return CLIP<int16>((int16) (128 * sin(2.0 * M_PI * x / oscLength)), -128, 127);
	}
};

}

#endif
