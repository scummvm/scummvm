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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"

#include "audio/mixer.h"

#include "adl/sound.h"

namespace Adl {

// Generic PC-speaker synth
// This produces more accurate frequencies than Audio::PCSpeaker, but only
// does square waves.
class Speaker {
public:
	Speaker(int sampleRate);

	void startTone(double freq);
	void stopTone();
	void generateSamples(int16 *buffer, int numSamples);

private:
	int _rate;
	frac_t _halfWaveLen, _halfWaveRem;
	int16 _curSample;
};

Speaker::Speaker(int sampleRate) :
		_rate(sampleRate),
		_halfWaveLen(0),
		_halfWaveRem(0),
		_curSample(32767) { }

void Speaker::startTone(double freq) {
	_halfWaveLen = _halfWaveRem = doubleToFrac(_rate / freq / 2);

	if (_halfWaveLen < (frac_t)FRAC_ONE) {
		// Tone out of range at this sample rate
		stopTone();
	}
}

void Speaker::stopTone() {
	_halfWaveLen = 0;
}

void Speaker::generateSamples(int16 *buffer, int numSamples) {
	if (_halfWaveLen == 0) {
		// Silence
		memset(buffer, 0, numSamples * sizeof(int16));
		return;
	}

	int offset = 0;

	while (offset < numSamples) {
		if (_halfWaveRem >= 0 && _halfWaveRem < (frac_t)FRAC_ONE) {
			// Rising/falling edge
			// Switch level
			_curSample = ~_curSample;
			// Use transition point fraction for current sample value
			buffer[offset++] = _halfWaveRem ^ _curSample;
			// Compute next transition point
			_halfWaveRem += _halfWaveLen - FRAC_ONE;
		} else {
			// Low/high level
			// Generate as many samples as we can
			const int samples = MIN(numSamples - offset, (int)fracToInt(_halfWaveRem));
			Common::fill(buffer + offset, buffer + offset + samples, _curSample);
			offset += samples;

			// Count down to level transition point
			_halfWaveRem -= intToFrac(samples);
		}
	}
}

Sound::Sound(const Tones &tones) :
		_tones(tones),
		_toneIndex(0),
		_samplesRem(0) {

	_rate = g_system->getMixer()->getOutputRate();
	_speaker = new Speaker(_rate);
}

Sound::~Sound() {
	delete _speaker;
}

bool Sound::endOfData() const {
	return _samplesRem == 0 && _toneIndex == _tones.size();
}

int Sound::readBuffer(int16 *buffer, const int numSamples) {
	int offset = 0;

	while (offset < numSamples) {
		if (_samplesRem == 0) {
			// Set up next tone

			if (_toneIndex == _tones.size()) {
				// No more tones
				return offset;
			}

			if (_tones[_toneIndex].freq == 0.0)
				_speaker->stopTone();
			else
				_speaker->startTone(_tones[_toneIndex].freq);

			// Compute length of tone
			_samplesRem = _rate * _tones[_toneIndex++].len / 1000;
		}

		// Generate as many samples as we can
		const int samples = MIN(numSamples - offset, _samplesRem);
		_speaker->generateSamples(buffer + offset, samples);

		_samplesRem -= samples;
		offset += samples;
	}

	return numSamples;
}

} // End of namespace Adl
