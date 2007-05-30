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

#include "sound/mods/paula.h"

namespace Audio {

Paula::Paula(bool stereo, int rate, int interruptFreq) :
		_stereo(stereo), _rate(rate), _intFreq(interruptFreq) {
	_playing = false;

	clearVoices();
	_voice[0].panning = 63;
	_voice[1].panning = 191;
	_voice[2].panning = 191;
	_voice[3].panning = 63;

	if (_intFreq <= 0)
		_intFreq = _rate;

	_curInt = _intFreq;
	_end = true;
}

Paula::~Paula() {
}

void Paula::clearVoice(byte voice) {
	assert(voice < NUM_VOICES);

	_voice[voice].data = 0;
	_voice[voice].dataRepeat = 0;
	_voice[voice].length = 0;
	_voice[voice].lengthRepeat = 0;
	_voice[voice].period = 0;
	_voice[voice].volume = 0;
	_voice[voice].offset = 0;
}

int Paula::readBuffer(int16 *buffer, const int numSamples) {
	int voice;
	int samples;
	int nSamples;
	int sLen;
	double frequency;
	double rate;
	double offset;
	int16 *p;
	const int8 *data;

	Common::StackLock lock(_mutex);

	memset(buffer, 0, numSamples * 2);
	if (!_playing) {
		return numSamples;
	}

	samples = _stereo ? numSamples / 2 : numSamples;
	while (samples > 0) {
		if (_curInt == _intFreq) {
			interrupt();
			_curInt = 0;
		}
		nSamples = MIN(samples, _intFreq - _curInt);
		for (voice = 0; voice < NUM_VOICES; voice++) {
			if (!_voice[voice].data || (_voice[voice].period <= 0))
				continue;

			frequency = (7093789.2 / 2.0) / _voice[voice].period;
			rate = frequency / _rate;
			offset = _voice[voice].offset;
			sLen = _voice[voice].length;
			data = _voice[voice].data;
			p = buffer;

			_voice[voice].volume = MIN((byte) 0x40, _voice[voice].volume);
			if ((_voice[voice].lengthRepeat > 2) &&
					((int)(offset + nSamples * rate) >= sLen)) {
				int neededSamples = nSamples;

				int end = (int)((sLen - offset) / rate);

				for (int i = 0; i < end; i++)
					mix(p, data[(int)(offset + rate * i)], voice);

				_voice[voice].length = sLen = _voice[voice].lengthRepeat;
				_voice[voice].data = data = _voice[voice].dataRepeat;
				_voice[voice].offset = offset = 0;
				neededSamples -= end;

				while (neededSamples > 0) {
					if (neededSamples >= (int) ((sLen - offset) / rate)) {
						while (rate > (sLen - offset))
							rate -= (sLen - offset);

						end = (int)((sLen - offset) / rate);

						for (int i = 0; i < end; i++)
							mix(p, data[(int)(offset + rate * i)], voice);

						_voice[voice].data = data = _voice[voice].dataRepeat;
						_voice[voice].length = sLen =
							_voice[voice].lengthRepeat;
						_voice[voice].offset = offset = 0;

						neededSamples -= end;
					} else {
						for (int i = 0; i < neededSamples; i++)
							mix(p, data[(int)(offset + rate * i)], voice);
						_voice[voice].offset += rate * neededSamples;
						if (ceil(_voice[voice].offset) >= sLen) {
							_voice[voice].data = data = _voice[voice].dataRepeat;
							_voice[voice].length = sLen =
								_voice[voice].lengthRepeat;
							_voice[voice].offset = offset = 0;
						}
						neededSamples = 0;
					}
				}
			} else {
				if (offset < sLen) {
					if ((int)(offset + nSamples * rate) >= sLen) {
						// The end of the sample is the limiting factor

						int end = (int)((sLen - offset) / rate);
						for (int i = 0; i < end; i++)
							mix(p, data[(int)(offset + rate * i)], voice);
						_voice[voice].offset = sLen;
					} else {
						// The requested number of samples is the limiting
						// factor, not the sample

						for (int i = 0; i < nSamples; i++)
							mix(p, data[(int)(offset + rate * i)], voice);
						_voice[voice].offset += rate * nSamples;
					}
				}
			}
		}
		buffer += _stereo ? nSamples * 2 : nSamples;
		_curInt += nSamples;
		samples -= nSamples;
	}
	return numSamples;
}

} // End of namespace Audio
