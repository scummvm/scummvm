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

Paula::Paula(bool stereo, int rate, uint interruptFreq) :
		_stereo(stereo), _rate(rate), _periodScale((kPalSystemClock / 2.0) / rate), _intFreq(interruptFreq) {

	clearVoices();
	_voice[0].panning = 63;
	_voice[1].panning = 191;
	_voice[2].panning = 191;
	_voice[3].panning = 63;

	if (_intFreq == 0)
		_intFreq = _rate;

	_curInt = 0;
	_timerBase = 1;
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
	_voice[voice].periodRepeat = 0;
	_voice[voice].volume = 0;
	_voice[voice].offset = 0;
	_voice[voice].dmaCount = 0;
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
	int samples = _stereo ? numSamples / 2 : numSamples;
	while (samples > 0) {

		// Handle 'interrupts'. This gives subclasses the chance to adjust the channel data
		// (e.g. insert new samples, do pitch bending, whatever).
		if (_curInt == 0) {
			_curInt = _intFreq;
			interrupt();
		}

		// Compute how many samples to generate: at most the requested number of samples,
		// of course, but we may stop earlier when an 'interrupt' is expected.
		const uint nSamples = MIN((uint)samples, _curInt);

		// Loop over the four channels of the emulated Paula chip
		for (int voice = 0; voice < NUM_VOICES; voice++) {
			// No data, or paused -> skip channel
			if (!_voice[voice].data || (_voice[voice].period <= 0))
				continue;

			// The Paula chip apparently run at 7.0937892 MHz. We combine this with
			// the requested output sampling rate (typicall 44.1 kHz or 22.05 kHz)
			// as well as the "period" of the channel we are processing right now,
			// to compute the correct output 'rate'.
			frac_t rate = doubleToFrac(_periodScale / _voice[voice].period);

			// Cap the volume
			_voice[voice].volume = MIN((byte) 0x40, _voice[voice].volume);

			// Cache some data (helps the compiler to optimize the code, by
			// indirectly telling it that no data aliasing can occur).
			frac_t offset = _voice[voice].offset;
			frac_t sLen = intToFrac(_voice[voice].length);
			const int8 *data = _voice[voice].data;
			int dmaCount = _voice[voice].dmaCount;
			int16 *p = buffer;
			int end = 0;
			int neededSamples = nSamples;

			// Compute the number of samples to generate; that is, either generate
			// just as many as were requested, or until the buffer is used up.
			// Note that dividing two frac_t yields an integer (as the denominators
			// cancel out each other).
			// Note that 'end' could be 0 here. No harm in that :-).
			end = MIN(neededSamples, (int)((sLen - offset + rate - 1) / rate));
			mixBuffer<stereo>(p, data, offset, rate, end, _voice[voice].volume, _voice[voice].panning);
			neededSamples -= end;

			// If we have not yet generated enough samples, and looping is active: loop!
			if (neededSamples > 0 && _voice[voice].lengthRepeat > 2) {
				// At this point we know that we have used up all samples in the buffer, so reset it.
				_voice[voice].data = data = _voice[voice].dataRepeat;
				_voice[voice].length = _voice[voice].lengthRepeat;
				sLen = intToFrac(_voice[voice].length);

				if (_voice[voice].period != _voice[voice].periodRepeat) {
					_voice[voice].period = _voice[voice].periodRepeat;
					rate = doubleToFrac(_periodScale / _rate);
				}

				// If the "rate" exceeds the sample rate, we would have to perform constant
				// wrap arounds. So, apply the first step of the euclidean algorithm to
				// achieve the same more efficiently: Take rate modulo sLen
				// TODO: This messes up dmaCount
				if (sLen < rate)
					rate %= sLen;

				// Repeat as long as necessary.
				while (neededSamples > 0) {
					offset &= FRAC_LO_MASK;
					dmaCount++;

					// Compute the number of samples to generate (see above) and mix 'em.
					end = MIN(neededSamples, (int)((sLen - offset + rate - 1) / rate));
					mixBuffer<stereo>(p, data, offset, rate, end, _voice[voice].volume, _voice[voice].panning);
					neededSamples -= end;
				}
			}

			// TODO correctly handle setting registers after last 2 bytes red from channel
			if (offset > sLen) {
				offset &= FRAC_LO_MASK;
				dmaCount++;
			}

			// Write back the cached data
			_voice[voice].offset = offset;
			_voice[voice].dmaCount = dmaCount;

		}
		buffer += _stereo ? nSamples * 2 : nSamples;
		_curInt -= nSamples;
		samples -= nSamples;
	}
	return numSamples;
}

} // End of namespace Audio
