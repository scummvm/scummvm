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

#ifndef AUDIO_ADLIB_CTMIDI_H
#define AUDIO_ADLIB_CTMIDI_H

#include "audio/adlib_ms.h"

/**
 * General MIDI AdLib driver based on CTMIDI.DRV version 1.04
 */
class MidiDriver_ADLIB_CTMIDI : public MidiDriver_ADLIB_Multisource {
public:
	// Instrument bank from CTMIDI.DRV. Transpose is stored separately.
	static const AdLibIbkInstrumentDefinition CTMIDI_INSTRUMENT_BANK[128];
	// Instrument transpose parameter values
	static const int8 INSTRUMENT_TRANSPOSE[128];

	// Rhythm bank from CTMIDI.DRV. OPL note is stored separately.
	static const AdLibIbkInstrumentDefinition CTMIDI_RHYTHM_BANK[47];
	// Mapping from MIDI note value to rhythm instrument
	static const uint8 RHYTHM_NOTE_INSTRUMENT_MAP[47];
	// The OPL note to play for each rhythm instrument
	static const uint8 RHYTHM_NOTES[47];

	// F-num lookup table from CTMIDI.DRV
	static const uint16 CTMIDI_NOTE_FREQUENCIES[768];
	// Volume modifier lookup table
	static const int8 NOTE_VOLUME_MODIFIER_VALUES[18];

	MidiDriver_ADLIB_CTMIDI(OPL::Config::OplType oplType, int timerFrequency = OPL::OPL::kDefaultCallbackFrequency);
	~MidiDriver_ADLIB_CTMIDI() { };

	int open() override;
	void deinitSource(uint8 source) override;

protected:
	// Instrument bank after conversion
	OplInstrumentDefinition _ctmidiInstrumentBank[128];
	// Rhythm bank after conversion
	OplInstrumentDefinition _ctmidiRhythmBank[47];

	/**
	 * Sets the block / F-num registers for the rhythm instruments to default
	 * values.
	 */
	void initRhythmDefaults();
	void programChange(uint8 channel, uint8 program, uint8 source) override;
	uint8 allocateOplChannel(uint8 channel, uint8 source, InstrumentInfo &instrumentInfo) override;
	void writeFrequency(uint8 oplChannel, OplInstrumentRhythmType rhythmType) override;
	uint16 calculateFrequency(uint8 channel, uint8 source, uint8 note) override;
	uint8 calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, const OplInstrumentDefinition &instrumentDef, uint8 operatorNum) override;
};

#endif
