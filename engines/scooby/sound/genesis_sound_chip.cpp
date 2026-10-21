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

#include "scooby/sound/genesis_sound_chip.h"

#include "audio/mixer.h"
#include "common/util.h"

#include <math.h>

namespace Scooby {

namespace {

// Timer A counts once per kFmClock / 144 Hz (53267 Hz). The vertical blank interrupt that paces the
// original Z80 driver runs at 59.92 Hz on NTSC hardware, i.e. every 889 Timer A counts. The 10-bit
// reload value 1024 - 889 = 135 is split into register 0x24 (bits 9-2) and register 0x25 (bits 1-0).
const uint8 kVerticalBlankTimerHigh = 135 >> 2;
const uint8 kVerticalBlankTimerLow = 135 & 3;

// YM2612 register 0x27: load and enable Timer A. The CH3 mode bits (7-6) are preserved by callers.
const uint8 kTimerAEnable = 0x05;

// Output levels. TownsPC98_FmSynth outputs a full-volume FM carrier at about +/-32672, four times the
// YM2612's 14-bit channel range (+/-8192), so its output is scaled by kFmGain to match the chip. The DAC
// replaces channel 6 at the same full-scale level ((sample - 0x80) << 6 on the real chip), and a full
// PSG channel is mixed at about half of a full FM channel, the balance commonly used by Genesis
// emulators. Tweak these constants to rebalance the mix.
const int kFmGainNumerator = 1;
const int kFmGainDenominator = 4;
const int32 kDacScale = 64;
const int32 kPsgMaxLevel = 4096;

// Render-sample to PSG-step conversion. The PSG counters decrement at kPsgClock / 16 Hz while the FM
// core renders at kFmClock / 72 Hz, so each rendered sample advances the PSG by
// (kPsgClock * 72) / (kFmClock * 16) steps.
const uint32 kPsgPhaseIncrement = static_cast<uint32>(GenesisSoundChip::kPsgClock) * 72u;
const uint32 kPsgPhasePerStep = static_cast<uint32>(GenesisSoundChip::kFmClock) * 16u;

const uint16 kPsgNoiseInitialShift = 0x8000;

} // namespace

GenesisSoundChip::GenesisSoundChip(Audio::Mixer *mixer, GenesisSoundChipClient *client)
	: TownsPC98_FmSynth(mixer, TownsPC98_FmSynth::kTypeTowns), _client(client), _psgNoiseControl(0),
	  _psgNoiseAttenuation(0x0F), _psgNoiseCounter(1), _psgNoiseHigh(false),
	  _psgNoiseShift(kPsgNoiseInitialShift), _psgLatch(0), _psgPhase(0), _dacEnabled(false), _dacData(0x80),
	  _outputVolume(255), _chipReady(false) {
	// The SN76489 attenuates in 2 dB steps; attenuation 15 is silence.
	for (int i = 0; i < 15; i++) {
		_psgLevels[i] = static_cast<int32>(kPsgMaxLevel * pow(10.0, -0.1 * i));
	}
	_psgLevels[15] = 0;
}

GenesisSoundChip::~GenesisSoundChip() {
	detachClient();
	deinit();
}

bool GenesisSoundChip::init() {
	if (!TownsPC98_FmSynth::init()) {
		return false;
	}

	Common::StackLock lock(_mutex);
	// All six FM channels follow the single output volume.
	setVolumeChannelMasks(0x3F, 0);
	setVolumeIntern(_outputVolume * kFmGainNumerator / kFmGainDenominator, Audio::Mixer::kMaxMixerVolume);
	resetPsg();
	_dacEnabled = false;
	_dacData = 0x80;

	writeReg(0, 0x24, kVerticalBlankTimerHigh);
	writeReg(0, 0x25, kVerticalBlankTimerLow);
	writeReg(0, 0x27, kTimerAEnable);
	_chipReady = true;
	return true;
}

void GenesisSoundChip::reset() {
	Common::StackLock lock(_mutex);
	TownsPC98_FmSynth::reset();
	resetPsg();
	_dacEnabled = false;
	_dacData = 0x80;

	// TownsPC98_FmSynth::reset() stops both timers; restart the vertical-blank timer.
	writeReg(0, 0x24, kVerticalBlankTimerHigh);
	writeReg(0, 0x25, kVerticalBlankTimerLow);
	writeReg(0, 0x27, kTimerAEnable);
}

void GenesisSoundChip::setOutputVolume(int volume) {
	Common::StackLock lock(_mutex);
	_outputVolume = CLIP<int>(volume, 0, Audio::Mixer::kMaxMixerVolume);
	setVolumeIntern(_outputVolume * kFmGainNumerator / kFmGainDenominator, Audio::Mixer::kMaxMixerVolume);
}

void GenesisSoundChip::detachClient() {
	Common::StackLock lock(_mutex);
	_client = nullptr;
}

void GenesisSoundChip::setDacEnabled(bool enabled) {
	_dacEnabled = enabled;
}

void GenesisSoundChip::resetPsg() {
	for (int i = 0; i < 3; i++) {
		_psgTones[i] = PsgTone();
	}
	_psgNoiseControl = 0;
	_psgNoiseAttenuation = 0x0F;
	_psgNoiseCounter = 1;
	_psgNoiseHigh = false;
	_psgNoiseShift = kPsgNoiseInitialShift;
	_psgLatch = 0;
	_psgPhase = 0;
}

void GenesisSoundChip::writePsg(uint8 value) {
	Common::StackLock lock(_mutex);
	if (value & 0x80) {
		// Latch/data byte: bits 6-4 select the register, bits 3-0 carry data.
		_psgLatch = (value >> 4) & 7;
		uint8 data = value & 0x0F;
		int channel = _psgLatch >> 1;
		if (_psgLatch & 1) {
			if (channel < 3) {
				_psgTones[channel].attenuation = data;
			} else {
				_psgNoiseAttenuation = data;
			}
		} else if (channel < 3) {
			_psgTones[channel].period = static_cast<uint16>((_psgTones[channel].period & 0x3F0) | data);
		} else {
			_psgNoiseControl = data & 7;
			_psgNoiseShift = kPsgNoiseInitialShift;
		}
		return;
	}

	// Data byte: applies to the latched register.
	int channel = _psgLatch >> 1;
	if (_psgLatch & 1) {
		if (channel < 3) {
			_psgTones[channel].attenuation = value & 0x0F;
		} else {
			_psgNoiseAttenuation = value & 0x0F;
		}
	} else if (channel < 3) {
		_psgTones[channel].period =
			static_cast<uint16>((_psgTones[channel].period & 0x00F) | ((value & 0x3F) << 4));
	} else {
		_psgNoiseControl = value & 7;
		_psgNoiseShift = kPsgNoiseInitialShift;
	}
}

void GenesisSoundChip::clockPsg() {
	for (int i = 0; i < 3; i++) {
		PsgTone &tone = _psgTones[i];
		if (--tone.counter <= 0) {
			tone.counter = tone.period ? tone.period : 1;
			tone.high = !tone.high;
		}
	}

	if (--_psgNoiseCounter <= 0) {
		int rate = _psgNoiseControl & 3;
		_psgNoiseCounter = (rate == 3) ? (_psgTones[2].period ? _psgTones[2].period : 1) : (0x10 << rate);
		_psgNoiseHigh = !_psgNoiseHigh;
		if (_psgNoiseHigh) {
			// The Sega PSG uses a 16-bit shift register. White noise taps bits 0 and 3.
			uint16 feedback = (_psgNoiseControl & 4) ? ((_psgNoiseShift ^ (_psgNoiseShift >> 3)) & 1)
													 : (_psgNoiseShift & 1);
			_psgNoiseShift = static_cast<uint16>((_psgNoiseShift >> 1) | (feedback << 15));
		}
	}
}

int32 GenesisSoundChip::renderPsgSample() const {
	int32 sample = 0;
	for (int i = 0; i < 3; i++) {
		int32 level = _psgLevels[_psgTones[i].attenuation & 0x0F];
		sample += _psgTones[i].high ? level : -level;
	}
	int32 noiseLevel = _psgLevels[_psgNoiseAttenuation & 0x0F];
	sample += (_psgNoiseShift & 1) ? noiseLevel : -noiseLevel;
	return sample;
}

void GenesisSoundChip::nextTickEx(int32 *buffer, uint32 bufferSize) {
	if (!_chipReady) {
		return;
	}

	// Register 0xB6 of the second part holds channel 6's left (bit 7) and right (bit 6) enables, which
	// also route the DAC.
	uint8 channel6Output = readReg(1, 0xB6);
	bool dacLeft = (channel6Output & 0x80) != 0;
	bool dacRight = (channel6Output & 0x40) != 0;

	for (uint32 i = 0; i < bufferSize; i++) {
		if (_client) {
			_client->onRenderSample();
		}

		_psgPhase += kPsgPhaseIncrement;
		while (_psgPhase >= kPsgPhasePerStep) {
			_psgPhase -= kPsgPhasePerStep;
			clockPsg();
		}

		int32 psg = (renderPsgSample() * _outputVolume) / Audio::Mixer::kMaxMixerVolume;
		buffer[i * 2] += psg;
		buffer[i * 2 + 1] += psg;

		if (_dacEnabled) {
			int32 dac = ((static_cast<int32>(_dacData) - 0x80) * kDacScale * _outputVolume) /
						Audio::Mixer::kMaxMixerVolume;
			if (dacLeft) {
				buffer[i * 2] += dac;
			}
			if (dacRight) {
				buffer[i * 2 + 1] += dac;
			}
		}
	}
}

void GenesisSoundChip::timerCallbackA() {
	if (_client) {
		_client->onVerticalBlank();
	}
}

void GenesisSoundChip::timerCallbackB() {
}

} // namespace Scooby
