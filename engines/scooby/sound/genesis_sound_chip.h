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

#ifndef SCOOBY_GENESIS_SOUND_CHIP_H
#define SCOOBY_GENESIS_SOUND_CHIP_H

#include "common/scummsys.h"
#include "common/mutex.h"

#include "audio/softsynth/fmtowns_pc98/towns_pc98_fmsynth.h"

namespace Audio {
class Mixer;
}

namespace Scooby {

// Receives the chip-side events that the Genesis Z80 sound driver reacted to.
class GenesisSoundChipClient {
public:
	virtual ~GenesisSoundChipClient() {}

	// Called from the mixer thread, with the chip mutex held, every time YM2612 Timer A overflows.
	// The chip programs Timer A to the 59.92 Hz NTSC vertical-blank rate that drives the Z80 driver.
	virtual void onVerticalBlank() = 0;

	// Called from the mixer thread, with the chip mutex held, once per rendered chip sample
	// (GenesisSoundChip::kRenderRate). The original driver polls YM2612 Timer A to pace DAC writes;
	// the client counts these calls to reproduce that pacing.
	virtual void onRenderSample() = 0;
};

// Emulates the sound hardware of the Sega Genesis that the Z80 driver talks to:
//   - the YM2612 FM channels, using ScummVM's shared TownsPC98_FmSynth core (the YM2612 and the
//     FM part of the PC-98/FM Towns OPN chips are the same design),
//   - the YM2612 channel-6 DAC, which outputs 8-bit unsigned samples in place of FM channel 6,
//   - the SN76489-compatible PSG built into the VDP (three square-wave tones and one noise source).
// All register writes are expected to come from the client, either from its timer callbacks or
// while holding mutex().
class GenesisSoundChip : public TownsPC98_FmSynth {
public:
	// NTSC Genesis clocks.
	static const int kFmClock = 7670454;
	static const int kPsgClock = 3579545;
	// Rate at which TownsPC98_FmSynth renders internally and calls nextTickEx(). Timer A ticks every
	// two render samples (kFmClock / 144 Hz).
	static const int kRenderRateNumerator = kFmClock;
	static const int kRenderRateDenominator = 72;

	GenesisSoundChip(Audio::Mixer *mixer, GenesisSoundChipClient *client);
	~GenesisSoundChip() override;

	bool init() override;
	void reset() override;

	// Sets the output level (0-255) applied to FM, PSG, and DAC output.
	void setOutputVolume(int volume);

	// Writes one byte to the PSG data port (68k 0xC00011, Z80 0x7F11).
	void writePsg(uint8 value);

	// YM2612 register 0x2B bit 7: replaces FM channel 6 with the DAC.
	void setDacEnabled(bool enabled);

	// YM2612 register 0x2A: the unsigned 8-bit DAC sample.
	void setDacData(uint8 value) { _dacData = value; }

	Common::Mutex &mutex() { return _mutex; }

	// Detaches the client so no further callbacks reach it. Safe to call with the mutex held.
	void detachClient();

protected:
	void nextTickEx(int32 *buffer, uint32 bufferSize) override;
	void timerCallbackA() override;
	void timerCallbackB() override;

private:
	struct PsgTone {
		uint16 period = 0;
		int32 counter = 1;
		uint8 attenuation = 0x0F;
		bool high = false;
	};

	void resetPsg();
	void clockPsg();
	int32 renderPsgSample() const;

	GenesisSoundChipClient *_client;

	// PSG state.
	PsgTone _psgTones[3];
	uint8 _psgNoiseControl;
	uint8 _psgNoiseAttenuation;
	int32 _psgNoiseCounter;
	bool _psgNoiseHigh;
	uint16 _psgNoiseShift;
	uint8 _psgLatch;
	uint32 _psgPhase;
	int32 _psgLevels[16];

	// DAC state.
	bool _dacEnabled;
	uint8 _dacData;

	int _outputVolume;
	bool _chipReady;
};

} // namespace Scooby

#endif // SCOOBY_GENESIS_SOUND_CHIP_H
