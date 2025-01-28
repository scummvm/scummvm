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

#ifndef AGOS_SIMON1_ADLIB_H
#define AGOS_SIMON1_ADLIB_H

#include "audio/adlib_ms.h"

namespace AGOS {

class MidiDriver_Simon1_AdLib : public MidiDriver_ADLIB_Multisource {
private:
	struct RhythmMapEntry {
		// The OPL rhythm instrument to use.
		// The original interpreter would move a note played on the MIDI rhythm
		// channel to one of MIDI channels 11-15, each corresponding to an OPL
		// rhythm instrument.
		uint8 channel;
		// The instrument bank entry used to play the rhythm note.
		uint8 program;
		// The MIDI note number that is actually played.
		uint8 note;
	};

public:
	MidiDriver_Simon1_AdLib(OPL::Config::OplType oplType, const byte *instrumentData);
	~MidiDriver_Simon1_AdLib();

	int open() override;

	void noteOn(uint8 channel, uint8 note, uint8 velocity, uint8 source) override;
	void programChange(uint8 channel, uint8 program, uint8 source) override;

	void deinitSource(uint8 source) override;

	// Turns off rhythm notes for sources with type MUSIC (typically source 0).
	// This should be called when a SFX source that uses rhythm notes starts
	// playing to prevent conflicts on the rhythm channels. Deinitializing a
	// SFX source will turn rhythm notes back on.
	void disableMusicRhythmNotes();

private:
	static const RhythmMapEntry RHYTHM_MAP[];
	static const uint16 FREQUENCY_TABLE[];

	uint8 allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) override;
	uint16 calculateFrequency(uint8 channel, uint8 source, uint8 note) override;
	uint8 calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity,
								  const OplInstrumentDefinition &instrumentDef, uint8 operatorNum) override;
	void parseInstrumentData(const byte *instrumentData);

	// True if rhythm notes for sources with type MUSIC should not be played.
	bool _musicRhythmNotesDisabled;
};

MidiDriver_Multisource *createMidiDriverSimon1AdLib(const char *instrumentFilename, OPL::Config::OplType);

} // End of namespace AGOS

#endif
