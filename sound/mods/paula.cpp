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

	clearVoices();
	_voice[0].panning = 63;
	_voice[1].panning = 191;
	_voice[2].panning = 191;
	_voice[3].panning = 63;

	if (_intFreq <= 0)
		_intFreq = _rate;

	_curInt = _intFreq;
	_playing = false;
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
	Common::StackLock lock(_mutex);

	memset(buffer, 0, numSamples * 2);
	if (!_playing) {
		return numSamples;
	}

	if (_stereo)
		return readBufferIntern<true>(buffer, numSamples);
	else
		return readBufferIntern<false>(buffer, numSamples);
}


template<bool stereo>
inline void mixBuffer(int16 *&buf, const int8 *data, frac_t &offset, frac_t rate, int end, byte volume, byte panning) {
	for (int i = 0; i < end; i++) {
		const int32 tmp = ((int32) data[fracToInt(offset)]) * volume;
		if (stereo) {
			*buf++ += (tmp * (255 - panning)) >> 7;
			*buf++ += (tmp * (panning)) >> 7;
		} else
			*buf++ += tmp;

		offset += rate;
	}
}

template<bool stereo>
int Paula::readBufferIntern(int16 *buffer, const int numSamples) {
	int voice;
	int samples;
	int nSamples;

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

			const double frequency = (7093789.2 / 2.0) / _voice[voice].period;
			frac_t rate = doubleToFrac(frequency / _rate);
			frac_t offset = _voice[voice].offset;
			frac_t sLen = intToFrac(_voice[voice].length);

			const int8 *data = _voice[voice].data;
			int16 *p = buffer;
			int end = 0;

			_voice[voice].volume = MIN((byte) 0x40, _voice[voice].volume);
			// If looping has been enabled and we see that we will have to loop
			// to generate enough samples, then use the "loop" branch.
			if ((_voice[voice].lengthRepeat > 2) &&
					(offset + nSamples * rate >= sLen)) {
				int neededSamples = nSamples;

				while (neededSamples > 0) {
					if (sLen - offset < rate) {
						// This means that "rate" is too high, bigger than the sample size.
						// So we scale it down according to the euclidean algorithm.
						rate %= sLen - offset;
					}

					end = MIN(neededSamples, (sLen - offset) / rate);
					mixBuffer<stereo>(p, data, offset, rate, end, _voice[voice].volume, _voice[voice].panning);
					_voice[voice].offset = offset;
					neededSamples -= end;

					// If we read beyond the sample end, loop back to the start.
					// TODO: Shouldn't we wrap around here?
					if (_voice[voice].offset + FRAC_ONE > sLen) {
						_voice[voice].data = data = _voice[voice].dataRepeat;
						_voice[voice].length = _voice[voice].lengthRepeat;
						_voice[voice].offset = offset = 0;
						sLen = intToFrac(_voice[voice].length);
					}
				}
			} else {
				if (offset < sLen) {	// Sample data left?
					end = MIN(nSamples, (sLen - offset) / rate);
					mixBuffer<stereo>(p, data, offset, rate, end, _voice[voice].volume, _voice[voice].panning);
					_voice[voice].offset = offset;
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
