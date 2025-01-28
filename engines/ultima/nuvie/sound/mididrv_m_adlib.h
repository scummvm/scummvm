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

#ifndef NUVIE_SOUND_MIDIDRV_M_ADLIB_H
#define NUVIE_SOUND_MIDIDRV_M_ADLIB_H

#include "audio/adlib_ms.h"

namespace Ultima {
namespace Nuvie {

/**
 * M driver for AdLib (OPL2).
 * This driver supports several effects by adjusting OPL frequency and level
 * values based on timer ticks: slide, vibrato and fade in/out. Only vibrato is
 * used by the tracks in Ultima 6.
 */
class MidiDriver_M_AdLib : public MidiDriver_ADLIB_Multisource {
protected:
	// The direction of a volume fade: in (increase) or out (decrease).
	enum FadeDirection {
		FADE_DIRECTION_NONE,
		FADE_DIRECTION_FADE_IN,
		FADE_DIRECTION_FADE_OUT
	};

	// The current direction of vibrato pitch change.
	enum VibratoDirection {
		VIBRATO_DIRECTION_RISING,
		VIBRATO_DIRECTION_FALLING
	};

	// Converts M note values to OPL frequency (F-num) values.
	static const uint16 FNUM_VALUES[24];

public:
	MidiDriver_M_AdLib();
	~MidiDriver_M_AdLib();

	using MidiDriver_Multisource::send;
	void send(int8 source, uint32 b) override;
	void metaEvent(int8 source, byte type, byte *data, uint16 length) override;

protected:
	void programChange(uint8 channel, uint8 program, uint8 source) override;
	void modulation(uint8 channel, uint8 modulation, uint8 source) override;

	uint8 allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) override;
	uint16 calculateFrequency(uint8 channel, uint8 source, uint8 note) override;
	uint8 calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, const OplInstrumentDefinition &instrumentDef, uint8 operatorNum) override;
	void writeVolume(uint8 oplChannel, uint8 operatorNum, OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED) override;

	void deinitSource(uint8 source) override;

	void onTimer() override;

	// Number of F-num units each channel will increase/decrease each tick.
	int8 _slideValues[9];
	// Maximum number of F-num units the frequency will be changed by vibrato,
	// before applying the factor, for each channel. This is the difference
	// between the lowest and highest value (so twice the amplitude).
	uint8 _vibratoDepths[9];
	// Multiplication factor for vibrato F-num values for each channel.
	uint8 _vibratoFactors[9];
	// The current "progression" of vibrato through the cycle for each channel.
	// This is before the vibrato factor is applied.
	uint8 _vibratoCurrentDepths[9];
	// The current direction in which the vibrato is progressing for each
	// channel (rising or falling frequency).
	VibratoDirection _vibratoDirections[9];
	// The direction of the fade currently active on each channel (in or out).
	// NONE indicates no fade is active.
	FadeDirection _fadeDirections[9];
	// The delay in ticks between each level increase or decrease for each
	// channel.
	uint8 _fadeStepDelays[9];
	// The current fade delay counter value for each channel.
	uint8 _fadeCurrentDelays[9];
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
