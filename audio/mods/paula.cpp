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

/*
 * The low-pass filter code is based on UAE's audio filter code
 * found in audio.c. UAE is licensed under the terms of the GPLv2.
 *
 * audio.c in UAE states the following:
 * Copyright 1995, 1996, 1997 Bernd Schmidt
 * Copyright 1996 Marcus Sundberg
 * Copyright 1996 Manfred Thole
 * Copyright 2006 Toni Wilen
 */

#include <math.h>

#include "common/scummsys.h"

#include "audio/mods/paula.h"
#include "audio/null.h"

namespace Audio {

Paula::Paula(bool stereo, int rate, uint interruptFreq, FilterMode filterMode, int periodScaleDivisor) :
		_stereo(stereo), _rate(rate), _periodScale((double)kPalPaulaClock / (rate * periodScaleDivisor)), _intFreq(interruptFreq) {

	_filterState.mode      = filterMode;
	_filterState.ledFilter = false;
	filterResetState();

	_filterState.a0[0] = filterCalculateA0(rate,  6200);
	_filterState.a0[1] = filterCalculateA0(rate, 20000);
	_filterState.a0[2] = filterCalculateA0(rate,  7000);

	clearVoices();
	_voice[0].panning = 191;
	_voice[1].panning = 63;
	_voice[2].panning = 63;
	_voice[3].panning = 191;

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
	_voice[voice].volume = 0;
	_voice[voice].offset = Offset(0);
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

/* Denormals are very small floating point numbers that force FPUs into slow
 * mode. All lowpass filters using floats are suspectible to denormals unless
 * a small offset is added to avoid very small floating point numbers.
 */
#define DENORMAL_OFFSET (1E-10)

/* Based on UAE.
 * Original comment in UAE:
 *
 * Amiga has two separate filtering circuits per channel, a static RC filter
 * on A500 and the LED filter. This code emulates both.
 *
 * The Amiga filtering circuitry depends on Amiga model. Older Amigas seem
 * to have a 6 dB/oct RC filter with cutoff frequency such that the -6 dB
 * point for filter is reached at 6 kHz, while newer Amigas have no filtering.
 *
 * The LED filter is complicated, and we are modelling it with a pair of
 * RC filters, the other providing a highboost. The LED starts to cut
 * into signal somewhere around 5-6 kHz, and there's some kind of highboost
 * in effect above 12 kHz. Better measurements are required.
 *
 * The current filtering should be accurate to 2 dB with the filter on,
 * and to 1 dB with the filter off.
 */
inline int32 filter(int32 input, Paula::FilterState &state, int voice) {
	float normalOutput, ledOutput;

	switch (state.mode) {
	case Paula::kFilterModeA500:
		state.rc[voice][0] = state.a0[0] * input + (1 - state.a0[0]) * state.rc[voice][0] + DENORMAL_OFFSET;
		state.rc[voice][1] = state.a0[1] * state.rc[voice][0] + (1-state.a0[1]) * state.rc[voice][1];
		normalOutput = state.rc[voice][1];

		state.rc[voice][2] = state.a0[2] * normalOutput        + (1 - state.a0[2]) * state.rc[voice][2];
		state.rc[voice][3] = state.a0[2] * state.rc[voice][2]  + (1 - state.a0[2]) * state.rc[voice][3];
		state.rc[voice][4] = state.a0[2] * state.rc[voice][3]  + (1 - state.a0[2]) * state.rc[voice][4];

		ledOutput = state.rc[voice][4];
		break;

	case Paula::kFilterModeA1200:
		normalOutput = input;

		state.rc[voice][1] = state.a0[2] * normalOutput        + (1 - state.a0[2]) * state.rc[voice][1] + DENORMAL_OFFSET;
		state.rc[voice][2] = state.a0[2] * state.rc[voice][1]  + (1 - state.a0[2]) * state.rc[voice][2];
		state.rc[voice][3] = state.a0[2] * state.rc[voice][2]  + (1 - state.a0[2]) * state.rc[voice][3];

		ledOutput = state.rc[voice][3];
		break;

	case Paula::kFilterModeNone:
	default:
		return input;

	}

	return CLIP<int32>(state.ledFilter ? ledOutput : normalOutput, -32768, 32767);
}

template<bool stereo>
inline int mixBuffer(int16 *&buf, const int8 *data, Paula::Offset &offset, frac_t rate, int neededSamples, uint bufSize, byte volume, byte panning, Paula::FilterState &filterState, int voice) {
	int samples;
	for (samples = 0; samples < neededSamples && offset.int_off < bufSize; ++samples) {
		const int32 tmp = filter(((int32) data[offset.int_off]) * volume, filterState, voice);
		if (stereo) {
			*buf++ += (tmp * (255 - panning)) >> 7;
			*buf++ += (tmp * (panning)) >> 7;
		} else
			*buf++ += tmp;

		// Step to next source sample
		offset.rem_off += rate;
		if (offset.rem_off >= (frac_t)FRAC_ONE) {
			offset.int_off += fracToInt(offset.rem_off);
			offset.rem_off &= FRAC_LO_MASK;
		}
	}

	return samples;
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

			// The Paula chip apparently run at 7.0937892 MHz in the PAL
			// version and at 7.1590905 MHz in the NTSC version. We divide this
			// by the requested the requested output sampling rate _rate
			// (typically 44.1 kHz or 22.05 kHz) obtaining the value _periodScale.
			// This is then divided by the "period" of the channel we are
			// processing, to obtain the correct output 'rate'.
			frac_t rate = doubleToFrac(_periodScale / _voice[voice].period);
			// Cap the volume
			_voice[voice].volume = MIN((byte) 0x40, _voice[voice].volume);


			Channel &ch = _voice[voice];
			int16 *p = buffer;
			int neededSamples = nSamples;

			// NOTE: A Protracker (or other module format) player might actually
			// push the offset past the sample length in its interrupt(), in which
			// case the first mixBuffer() call should not mix anything, and the loop
			// should be triggered.
			// Thus, doing an assert(ch.offset.int_off < ch.length) here is wrong.
			// An example where this happens is a certain Protracker module played
			// by the OS/2 version of Hopkins FBI.

			// Mix the generated samples into the output buffer
			neededSamples -= mixBuffer<stereo>(p, ch.data, ch.offset, rate, neededSamples, ch.length, ch.volume, ch.panning, _filterState, voice);

			// Wrap around if necessary
			if (ch.offset.int_off >= ch.length) {
				// Important: Wrap around the offset *before* updating the voice length.
				// Otherwise, if length != lengthRepeat we would wrap incorrectly.
				// Note: If offset >= 2*len ever occurs, the following would be wrong;
				// instead of subtracting, we then should compute the modulus using "%=".
				// Since that requires a division and is slow, and shouldn't be necessary
				// in practice anyway, we only use subtraction.
				ch.offset.int_off -= ch.length;
				ch.dmaCount++;

				ch.data = ch.dataRepeat;
				ch.length = ch.lengthRepeat;
			}

			// If we have not yet generated enough samples, and looping is active: loop!
			if (neededSamples > 0 && ch.length > 2) {
				// Repeat as long as necessary.
				while (neededSamples > 0) {
					// Mix the generated samples into the output buffer
					neededSamples -= mixBuffer<stereo>(p, ch.data, ch.offset, rate, neededSamples, ch.length, ch.volume, ch.panning, _filterState, voice);

					if (ch.offset.int_off >= ch.length) {
						// Wrap around. See also the note above.
						ch.offset.int_off -= ch.length;
						ch.dmaCount++;
					}
				}
			}

		}
		buffer += _stereo ? nSamples * 2 : nSamples;
		_curInt -= nSamples;
		samples -= nSamples;
	}
	return numSamples;
}

void Paula::filterResetState() {
	for (int i = 0; i < NUM_VOICES; i++)
		for (int j = 0; j < 5; j++)
			_filterState.rc[i][j] = 0.0f;
}

/* Based on UAE.
 * Original comment in UAE:
 *
 * This computes the 1st order low-pass filter term b0.
 * The a1 term is 1.0 - b0. The center frequency marks the -3 dB point.
 */
float Paula::filterCalculateA0(int rate, int cutoff) {
	float omega;
	/* The BLT correction formula below blows up if the cutoff is above nyquist. */
	if (cutoff >= rate / 2)
		return 1.0;

	omega = 2 * M_PI * cutoff / rate;
	/* Compensate for the bilinear transformation. This allows us to specify the
	 * stop frequency more exactly, but the filter becomes less steep further
	 * from stopband. */
	omega = tan(omega / 2) * 2;
	return 1 / (1 + 1 / omega);
}

} // End of namespace Audio


//	Plugin interface
//	(This can only create a null driver since apple II gs support seeems not to be implemented
//  and also is not part of the midi driver architecture. But we need the plugin for the options
//  menu in the launcher and for MidiDriver::detectDevice() which is more or less used by all engines.)

class AmigaMusicPlugin : public NullMusicPlugin {
public:
	const char *getName() const {
		return _s("Amiga Audio emulator");
	}

	const char *getId() const {
		return "amiga";
	}

	MusicDevices getDevices() const;
};

MusicDevices AmigaMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_AMIGA));
	return devices;
}

//#if PLUGIN_ENABLED_DYNAMIC(AMIGA)
	//REGISTER_PLUGIN_DYNAMIC(AMIGA, PLUGIN_TYPE_MUSIC, AmigaMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(AMIGA, PLUGIN_TYPE_MUSIC, AmigaMusicPlugin);
//#endif
