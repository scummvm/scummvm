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

#ifndef AGOS_DRIVERS_ACCOLADE_ADLIB_H
#define AGOS_DRIVERS_ACCOLADE_ADLIB_H

#include "audio/adlib_ms.h"

namespace AGOS {

class MidiDriver_Accolade_AdLib : public MidiDriver_ADLIB_Multisource {
protected:
	static const byte RHYTHM_NOTE_INSTRUMENT_TYPES[40];
	static const uint16 OPL_NOTE_FREQUENCIES_INSTR_DAT[12];
	static const uint16 OPL_NOTE_FREQUENCIES_MUSIC_DRV[12];

public:
	MidiDriver_Accolade_AdLib(OPL::Config::OplType oplType, bool newVersion, int timerFrequency);
	~MidiDriver_Accolade_AdLib() override;

	int open() override;
	using MidiDriver_ADLIB_Multisource::send;
	void send(int8 source, uint32 b) override;
	void deinitSource(uint8 source) override;

	// Read the specified data from INSTR.DAT or MUSIC.DRV.
	void readDriverData(byte *driverData, uint16 driverDataSize, bool isMusicDrv);

	// Returns the number of simultaneous SFX sources supported by the current
	// driver configuration.
	byte getNumberOfSfxSources();
	// Loads the specified instrument for the specified instrument source.
	void loadSfxInstrument(uint8 source, byte *instrumentData);
	// Sets the note (upper byte) and note fraction (lower byte; 1/256th notes)
	// for the specified SFX source.
	void setSfxNoteFraction(uint8 source, uint16 noteFraction);
	// Writes out the current frequency for the specified SFX source.
	void updateSfxNote(uint8 source);
	// Applies a workaround for an Elvira 1 OPL3 instrument issue.
	void patchE1Instruments();
	// Applies a workaround for a Waxworks OPL3 instrument issue.
	void patchWwInstruments();

protected:
	InstrumentInfo determineInstrument(uint8 channel, uint8 source, uint8 note) override;

	uint8 allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) override;
	uint16 calculateFrequency(uint8 channel, uint8 source, uint8 note) override;
	uint8 calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, OplInstrumentDefinition &instrumentDef, uint8 operatorNum) override;

	void writePanning(uint8 oplChannel, OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED) override;
	void writeFrequency(uint8 oplChannel, OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED) override;

	// Copies the specified instrument data (in INSTR.DAT/MUSIC.DRV format)
	// into the specified instrument definition.
	void loadInstrumentData(OplInstrumentDefinition &definition, byte *instrumentData,
		OplInstrumentRhythmType rhythmType, byte rhythmNote, bool newVersion);

	// False if the driver should have the behavior of the Elvira 1 driver;
	// true if it should have the behavior of the Elvira 2 / Waxworks version.
	bool _newVersion;

	// from INSTR.DAT/MUSIC.DRV - volume adjustment per instrument
	int8 _volumeAdjustments[128];
	// from INSTR.DAT/MUSIC.DRV - simple mapping between MIDI channel and AdLib channel
	byte _channelRemapping[16];
	// from INSTR.DAT/MUSIC.DRV - simple mapping between MIDI instruments and AdLib instruments
	byte _instrumentRemapping[128];
	// Points to one of the OPL_NOTE_FREQUENCIES arrays, depending on the driver version
	const uint16 *_oplNoteFrequencies;

	// Data used by AdLib SFX (Elvira 2 / Waxworks)

	// Instrument definition for each SFX source
	OplInstrumentDefinition _sfxInstruments[4];
	// Current MIDI note fraction (1/256th notes) for each SFX source
	byte _sfxNoteFractions[4];
};

} // End of namespace AGOS

#endif
