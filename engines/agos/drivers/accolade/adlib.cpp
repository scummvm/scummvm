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

#include "agos/drivers/accolade/adlib.h"

#include "agos/drivers/accolade/mididriver.h"

namespace AGOS {

// hardcoded, dumped from Accolade music system
// same for INSTR.DAT + MUSIC.DRV, except that MUSIC.DRV does the lookup differently
// Numbers 6-A correspond to MIDI channels in the original driver, but here they
// are directly mapped to rhythm instrument types (bass drum, snare drum,
// tom tom, cymbal and hi-hat respectively). F means there is no instrument
// defined for the rhythm note.
const byte MidiDriver_Accolade_AdLib::RHYTHM_NOTE_INSTRUMENT_TYPES[] = {
	0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x0A, 0x08, 0x0A, 0x08,
	0x0A, 0x08, 0x08, 0x09, 0x08, 0x09, 0x0F, 0x0F, 0x0A, 0x0F,
	0x0A, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x0A, 0x0F, 0x0F, 0x08, 0x0F, 0x08
};

// hardcoded, dumped from Accolade music system (INSTR.DAT variant)
const uint16 MidiDriver_Accolade_AdLib::OPL_NOTE_FREQUENCIES_INSTR_DAT[] = {
	0x02B2, 0x02DB, 0x0306, 0x0334, 0x0365, 0x0399,
	0x03CF, 0xFE05, 0xFE23, 0xFE44, 0xFE67, 0xFE8B
};

// hardcoded, dumped from Accolade music system (MUSIC.DRV variant)
const uint16 MidiDriver_Accolade_AdLib::OPL_NOTE_FREQUENCIES_MUSIC_DRV[] = {
	0x0205, 0x0223, 0x0244, 0x0267, 0x028B, 0x02B2,
	0x02DB, 0x0306, 0x0334, 0x0365, 0x0399, 0x03CF
};

// Accolade adlib music driver
//
// Remarks:
//
// There are at least 2 variants of this sound system.
// One for the game Elvira 1
// It seems it was also used for the game "Altered Destiny"
// Another one for the games Elvira 2 + Waxworks + Simon, the Sorcerer 1 Demo
//
// First one uses the file INSTR.DAT for instrument data, channel mapping etc.
// Second one uses the file MUSIC.DRV, which actually contains driver code + instrument data + channel mapping, etc.
//
// The second variant supported dynamic channel allocation for the FM voice channels, but this
// feature was at least definitely disabled for Simon, the Sorcerer 1 demo and for the Waxworks demo too.
//
// I have currently not implemented dynamic channel allocation.
MidiDriver_Accolade_AdLib::MidiDriver_Accolade_AdLib(OPL::Config::OplType oplType, bool newVersion, int timerFrequency) :
		MidiDriver_ADLIB_Multisource(oplType, timerFrequency) {
	_instrumentBank = nullptr;
	_rhythmBank = nullptr;
	_newVersion = newVersion;
	_oplNoteFrequencies = _newVersion ? OPL_NOTE_FREQUENCIES_MUSIC_DRV : OPL_NOTE_FREQUENCIES_INSTR_DAT;

	Common::fill(_channelRemapping, _channelRemapping + ARRAYSIZE(_channelRemapping), 0);
	Common::fill(_instrumentRemapping, _instrumentRemapping + ARRAYSIZE(_instrumentRemapping), 0);
	Common::fill(_volumeAdjustments, _volumeAdjustments + ARRAYSIZE(_volumeAdjustments), 0);
	Common::fill(_sfxNoteFractions, _sfxNoteFractions + ARRAYSIZE(_sfxNoteFractions), 0);
	memset(_sfxInstruments, 0, sizeof(_sfxInstruments));
}

MidiDriver_Accolade_AdLib::~MidiDriver_Accolade_AdLib() {
	if (_instrumentBank)
		delete[] _instrumentBank;
	if (_rhythmBank)
		delete[] _rhythmBank;
}

int MidiDriver_Accolade_AdLib::open() {
	_modulationDepth = MODULATION_DEPTH_LOW;
	_vibratoDepth = VIBRATO_DEPTH_LOW;

	int result = MidiDriver_ADLIB_Multisource::open();

	if (result == 0) {
		// Rhythm mode is always on.
		setRhythmMode(true);

		// The original driver writes out default instruments to all channels
		// here. This implementation writes instruments before note on, so this
		// is not necessary.

		// driver initialization does this here:
		// INSTR.DAT
		// noteOn(9, 0x29, 0);
		// noteOff(9, 0x26, false);
		// MUSIC.DRV
		// noteOn(9, 0x26, 0);
		// noteOff(9, 0x26, false);
	}

	return result;
}

void MidiDriver_Accolade_AdLib::send(int8 source, uint32 b) {
	// Remap the MIDI channel according to the channel map.
	// (Seems to be 1 on 1 for AdLib...)
	byte channel = b & 0xF;
	channel = _channelRemapping[channel];
	b &= 0xFFFFFFF0;
	b |= channel;
	byte command = b & 0xF0;

	if (_oplType != OPL::Config::kOpl3 && _sources[source].type != SOURCE_TYPE_SFX && command != MIDI_COMMAND_PROGRAM_CHANGE) {
		// Filter out events for channels used by SFX.
		// Program change events are always accepted; they just set the program
		// for the music source and do not affect the SFX notes.
		if (_activeNotes[channel].channelAllocated)
			return;
	}

	MidiDriver_ADLIB_Multisource::send(source, b);
}

void MidiDriver_Accolade_AdLib::deinitSource(uint8 source) {
	if (_sources[source].type == SOURCE_TYPE_SFX) {
		// When a sound effect ends, the original driver will immediately
		// rewrite the music instrument for the channel used by this sound
		// effect. This has the effect of stopping the release phase of the
		// sound effect. This is reproduced here to make sure the sound effects
		// sound the same.
		byte channel = _channelAllocations[source][0];
		// OPL3 mode has no fixed instrument assignment to the OPL channel, so
		// just use instrument 0.
		byte program = 0;
		if (_oplType != OPL::Config::kOpl3) {
			// For OPL2, get the current music instrument for this OPL channel.
			program = _controlData[0][channel].program;
			// Apply instrument remapping to instrument channels.
			program = _instrumentRemapping[program];
		}

		InstrumentInfo instrument { };
		instrument.instrumentId = program;
		instrument.instrumentDef = &_instrumentBank[program];
		instrument.oplNote = 0;

		writeInstrument(channel, instrument);

		// Clear other SFX data.
		_sfxNoteFractions[source - 1] = 0;
	}

	MidiDriver_ADLIB_Multisource::deinitSource(source);
}

uint8 MidiDriver_Accolade_AdLib::allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) {
	Common::StackLock lock(_allocationMutex);

	if (_sources[source].type == SOURCE_TYPE_SFX) {
		if (_channelAllocations[source][0] == 0xFF) {
			// Allocate a channel for this SFX source.
			byte allocatedChannel;
			if (_oplType != OPL::Config::kOpl3) {
				// For OPL2, use channels 5 and 4.
				allocatedChannel = 6 - source;
			} else {
				// For OPL3, use the dynamic allocation algorithm.
				allocatedChannel = MidiDriver_ADLIB_Multisource::allocateOplChannel(channel, source, instrumentId);
			}

			_activeNotesMutex.lock();

			ActiveNote *activeNote = &_activeNotes[allocatedChannel];
			if (activeNote->noteActive) {
				// Turn off the note currently playing on this OPL channel.
				writeKeyOff(allocatedChannel, activeNote->instrumentDef->rhythmType);
			}
			_channelAllocations[source][0] = allocatedChannel;
			activeNote->channelAllocated = true;
			activeNote->source = source;
			activeNote->channel = channel;
			activeNote->oplNote = 0;

			_activeNotesMutex.unlock();
		}

		// Return the allocated channel.
		return _channelAllocations[source][0];
	}

	// Channel allocation for music sources.
	if (_oplType != OPL::Config::kOpl3) {
		// For OPL2, discard events for channels 6 and 7 and channels allocated
		// for SFX.
		if (channel >= 6 || _activeNotes[channel].channelAllocated)
			return 0xFF;

		// Then just map MIDI channels 0-5 to OPL channels 0-5.
		return channel;
	} else {
		// For OPL3, use the dynamic allocation algorithm.
		return MidiDriver_ADLIB_Multisource::allocateOplChannel(channel, source, instrumentId);
	}
}

byte MidiDriver_Accolade_AdLib::getNumberOfSfxSources() {
	// With OPL3 more channels are available for SFX.
	return _oplType == OPL::Config::kOpl3 ? 4 : 2;
}

void MidiDriver_Accolade_AdLib::loadSfxInstrument(uint8 source, byte *instrumentData) {
	if (source > (_oplType == OPL::Config::kOpl3 ? 4 : 2))
		return;

	// Copy instrument data into SFX instruments bank.
	loadInstrumentData(_sfxInstruments[source - 1], instrumentData, RHYTHM_TYPE_UNDEFINED, 0, _newVersion);

	_activeNotesMutex.lock();

	// Allocate a channel
	programChange(0, 0, source);
	InstrumentInfo instrument = determineInstrument(0, source, 0);
	uint8 oplChannel = allocateOplChannel(0, source, instrument.instrumentId);

	// Update the active note data.
	ActiveNote *activeNote = &_activeNotes[oplChannel];
	activeNote->instrumentId = instrument.instrumentId;
	activeNote->instrumentDef = instrument.instrumentDef;

	_activeNotesMutex.unlock();
}

void MidiDriver_Accolade_AdLib::setSfxNoteFraction(uint8 source, uint16 noteFraction) {
	// Note is in the upper byte.
	_activeNotes[_channelAllocations[source][0]].oplNote = noteFraction >> 8;
	// Note fraction is in the lower byte.
	_sfxNoteFractions[source - 1] = noteFraction & 0xFF;
}

void MidiDriver_Accolade_AdLib::updateSfxNote(uint8 source) {
	writeFrequency(_channelAllocations[source][0]);
}

void MidiDriver_Accolade_AdLib::patchE1Instruments() {
	// WORKAROUND One instrument in Elvira 1 has a very slow attack. This
	// causes a problem in OPL3 mode (see patchWwInstruments for more details).
	// This is fixed by shortening the attack and compensating by making the
	// decay longer.

	if (_oplType != OPL::Config::kOpl3)
		// This workaround is only needed for OPL3 mode.
		return;

	// Patch the attack and decay of instrument 0x18.
	_instrumentBank[0x18].operator0.decayAttack = 0x42; // Was 0x24
}

void MidiDriver_Accolade_AdLib::patchWwInstruments() {
	// WORKAROUND Several instruments in Waxworks have a very slow attack (it
	// takes a long time for a note to reach maximum volume). When a note
	// played by this instrument is very short, only a small part of the attack
	// phase is played and the note is barely audible. Example: the rapid notes
	// in track 10 (played at the start of the London scenario).
	// This problem only occurs in OPL3 mode. In OPL2 mode, these notes are all
	// played on the same OPL channel. This means that each successive note
	// builds on the volume reached by the previous note and apart from the
	// first couple of notes they can be heard clearly. In OPL3 mode, each note
	// is played on its own channel, so each note starts from 0 volume.
	// This is fixed here by patching the attack value of this instrument to be
	// 1/4th of the original length (from 3 to 5). The notes do not sound
	// exactly as on OPL2, but they are clearly audible.

	if (_oplType != OPL::Config::kOpl3)
		// This workaround is only needed for OPL3 mode.
		return;

	// Patch the attack of instrument 0x22.
	_instrumentBank[0x22].operator1.decayAttack &= 0x0F;
	_instrumentBank[0x22].operator1.decayAttack |= 0x50;

	// Patch the attack of instrument 0x25.
	_instrumentBank[0x25].operator1.decayAttack &= 0x0F;
	_instrumentBank[0x25].operator1.decayAttack |= 0x60;

	// Patch the attack of instrument 0x7F.
	_instrumentBank[0x7F].operator0.decayAttack &= 0x0F;
	_instrumentBank[0x7F].operator0.decayAttack |= 0x60;
	_instrumentBank[0x7F].operator1.decayAttack &= 0x0F;
	_instrumentBank[0x7F].operator1.decayAttack |= 0x90;
}

MidiDriver_Accolade_AdLib::InstrumentInfo MidiDriver_Accolade_AdLib::determineInstrument(uint8 channel, uint8 source, uint8 note) {
	if (_sources[source].type == SOURCE_TYPE_SFX) {
		// For SFX sources, return an instrument from the SFX bank.
		InstrumentInfo instrument { };
		instrument.instrumentId = 0xFFFF - source;
		instrument.instrumentDef = &_sfxInstruments[source - 1];
		instrument.oplNote = note;
		return instrument;
	} else {
		return MidiDriver_ADLIB_Multisource::determineInstrument(channel, source, note);
	}
}

uint16 MidiDriver_Accolade_AdLib::calculateFrequency(uint8 channel, uint8 source, uint8 note) {
	if (!_newVersion) {
		// Elvira 1 version.
		if (channel != MIDI_RHYTHM_CHANNEL) {
			// All melodic notes are lowered by 1 octave, except the lowest notes.
			while (note < 0x18)
				note += 0xC;
			note -= 0xC;
		}
		// Highest 32 notes are clipped.
		if (note > 0x5F)
			note = 0x5F;
	} else {
		// Elvira 2 / Waxworks version.
		// Notes 19 and higher are transposed down 19 semitones.
		// Note that this is about 1.5 octave, which implies that notes 0-18 are
		// not played accurately by this driver.
		if (note >= 0x13)
			note -= 0x13;
	}

	// Determine octave and note within octave, and look up the matching OPL
	// frequency.
	int8 block = note / 12;
	if (!_newVersion)
		// Elvira 1 version lowers the octave by 1 (note that melodic notes
		// were lowered 1 octave earlier).
		block--;
	uint8 octaveNote = note % 12;

	// Look up the note frequency.
	uint16 baseFrequency = _oplNoteFrequencies[octaveNote];
	uint16 frequency;
	if (!_newVersion) {
		// Elvira 1 version has a negative frequency lookup value for notes
		// which are in a higher octave than the others.
		if (baseFrequency & 0x8000)
			block++;
		// Clear the high bits of the negative lookup values.
		frequency = baseFrequency & 0x3FF;
		if (block < 0) {
			// If octave is now negative, halve the frequency and increase
			// octave.
			frequency >>= 1;
			block++;
		}
	} else {
		// Elvira 2 / Waxworks version adds the note fraction for SFX.
		uint16 fractionFrequency = 0;
		if (_sources[source].type == SOURCE_TYPE_SFX) {
			// Because the frequency differences between notes are not constant
			// the fraction is multiplied by a factor depending on the note.
			fractionFrequency = (((octaveNote + 1) / 6) + 2) * (_sfxNoteFractions[source - 1] >> 4);
		}
		frequency = baseFrequency + fractionFrequency;
	}
	// Note that when processing sound effects, the note can be higher than the
	// MIDI maximum value of 0x7F. The original interpreter depends on this for
	// correct playback of the sound effect. However, this can cause block to
	// overflow the 3 bit range available to it in the OPL registers.
	block &= 0x7;

	return block << 10 | frequency;
}

uint8 MidiDriver_Accolade_AdLib::calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, OplInstrumentDefinition &instrumentDef, uint8 operatorNum) {
	// A volume adjustment is applied to the velocity of melodic notes.
	int8 volumeAdjustment = 0;
	if (_sources[source].type != SOURCE_TYPE_SFX) {
		if (instrumentDef.rhythmType == RHYTHM_TYPE_UNDEFINED) {
			byte program = _controlData[source][channel].program;
			volumeAdjustment = _volumeAdjustments[program];
		} else if (!_newVersion) {
			// For rhythm notes, the Elvira 1 version of the driver checks the
			// current "instrument" of channel 9. In this driver channel 9
			// corresponds to the cymbal rhythm instrument, which is set to
			// instrument definition 4. It then reads the volume adjustment
			// for instrument 4 and applies this to all rhythm notes. This
			// seems quite dubious and might be a bug, but it is reproduced
			// here so rhythm volume is the same as the original interpreter.
			// The Elvira 2 / Waxworks driver skips volume adjustment
			// completely for rhythm notes.
			volumeAdjustment = _volumeAdjustments[4];
		}
	}
	// Note velocity and the volume adjustment are added, clipped to normal
	// velocity range and divided by 2 to get an OPL volume value.
	uint8 vol = CLIP(velocity + volumeAdjustment, 0, 0x7F);

	if (!_newVersion) {
		// The Elvira 1 version raises the volume a bit and clips the highest
		// values.
		vol += 0x18;
		if (vol > 0x78)
			vol = 0x78;
	}

	// Invert the volume.
	return 0x3F - (vol >> 1);
}

void MidiDriver_Accolade_AdLib::writePanning(uint8 oplChannel, OplInstrumentRhythmType rhythmType) {
	// The Elvira 1 driver does not write the Cx register for rhythm
	// instruments except the bass drum; the Elvira 2 / Waxworks driver does
	// not write it for the bass drum either.
	if (rhythmType == RHYTHM_TYPE_UNDEFINED || (rhythmType == RHYTHM_TYPE_BASS_DRUM && !_newVersion))
		MidiDriver_ADLIB_Multisource::writePanning(oplChannel, rhythmType);
}

void MidiDriver_Accolade_AdLib::writeFrequency(uint8 oplChannel, OplInstrumentRhythmType rhythmType) {
	// The original driver does not write the frequency for the cymbal and
	// hi-hat instruments.
	if (rhythmType != RHYTHM_TYPE_HI_HAT && rhythmType != RHYTHM_TYPE_CYMBAL)
		MidiDriver_ADLIB_Multisource::writeFrequency(oplChannel, rhythmType);
}

void MidiDriver_Accolade_AdLib::loadInstrumentData(OplInstrumentDefinition &definition, byte *instrumentData,
		OplInstrumentRhythmType rhythmType, byte rhythmNote, bool newVersion) {
	definition.fourOperator = false;

	definition.connectionFeedback0 = instrumentData[8];
	definition.operator0.freqMultMisc = instrumentData[0];
	// The original driver does not add the KSL bits to the calculated
	// volume when writing the level registers. To replicate this, the KSL
	// bits are set to 0 for operators affected by volume.
	// Note that the Elvira 2 / Waxworks driver has a bug which will cause
	// the operator 0 level register of the bass drum instrument to be
	// overwritten by the connection bit (usually 0) of another instrument
	// if the bass drum connection is FM (and it is). This is fixed here by
	// setting the correct value. The Elvira 1 version does not have this
	// bug.
	definition.operator0.level = (definition.connectionFeedback0 & 1) ? 0 : instrumentData[1];
	definition.operator0.decayAttack = instrumentData[2];
	definition.operator0.releaseSustain = instrumentData[3];
	// The original driver only writes 0 to the waveform select registers
	// during initialization, so only sine waveform is used.
	definition.operator0.waveformSelect = 0;
	definition.operator1.freqMultMisc = instrumentData[4];
	definition.operator1.level = 0;
	definition.operator1.decayAttack = instrumentData[6];
	definition.operator1.releaseSustain = instrumentData[7];
	definition.operator1.waveformSelect = 0;
	if (newVersion) {
		// The Elvira 2 / Waxworks driver always sets the last two bits of
		// the sustain value.
		// This was done during "programChange" in the original driver
		definition.operator0.releaseSustain |= 3;
		definition.operator1.releaseSustain |= 3;
	}

	definition.rhythmType = rhythmType;
	definition.rhythmNote = rhythmNote;
}

void MidiDriver_Accolade_AdLib::readDriverData(byte *driverData, uint16 driverDataSize, bool newVersion) {
	uint16 minDataSize = newVersion ? 468 : 354;
	if (driverDataSize < minDataSize)
		error("ACCOLADE-ADLIB: Expected minimum driver data size of %d - got %d", minDataSize, driverDataSize);

	// INSTR.DAT Data is like this:
	// 128 bytes  instrument mapping
	// 128 bytes  instrument volume adjust (signed!)
	//  16 bytes  unknown
	//  16 bytes  channel mapping
	//  64 bytes  key note mapping (not used for MT32)
	//   1 byte   instrument count
	//   1 byte   bytes per instrument
	//   x bytes  no instruments used for MT32

	// music.drv is basically a driver, but with a few fixed locations for certain data

	uint16 channelMappingOffset = newVersion ? 396 : 256 + 16;
	Common::copy(driverData + channelMappingOffset, driverData + channelMappingOffset + ARRAYSIZE(_channelRemapping), _channelRemapping);

	uint16 instrumentMappingOffset = newVersion ? 140 : 0;
	Common::copy(driverData + instrumentMappingOffset, driverData + instrumentMappingOffset + ARRAYSIZE(_instrumentRemapping), _instrumentRemapping);
	setInstrumentRemapping(_instrumentRemapping);

	uint16 volumeAdjustmentsOffset = newVersion ? 140 + 128 : 128;
	int8 *volumeAdjustmentsData = (int8 *)driverData + volumeAdjustmentsOffset;
	Common::copy(volumeAdjustmentsData, volumeAdjustmentsData + ARRAYSIZE(_volumeAdjustments), _volumeAdjustments);

	if (!newVersion) {
		byte instrDatBytesPerInstrument = driverData[256 + 16 + 16 + 64 + 1];

		// We expect 9 bytes per instrument
		if (instrDatBytesPerInstrument != 9)
			error("ACCOLADE-ADLIB: Expected instrument definitions of length 9 - got length %d", instrDatBytesPerInstrument);
	}

	byte instrumentDefinitionCount = newVersion ? 128 : driverData[256 + 16 + 16 + 64];
	uint16 rhythmNoteOffset = newVersion ? 376 + 36 : 256 + 16 + 16;
	uint16 instrumentDataOffset = newVersion ? 722 : 256 + 16 + 16 + 64 + 2;

	_instrumentBank = new OplInstrumentDefinition[instrumentDefinitionCount];
	for (int i = 0; i < instrumentDefinitionCount; i++) {
		byte *instrumentData = driverData + instrumentDataOffset + (i * 9);
		loadInstrumentData(_instrumentBank[i], instrumentData, RHYTHM_TYPE_UNDEFINED, 0, newVersion);
	}

	_rhythmBank = new OplInstrumentDefinition[40];
	_rhythmBankFirstNote = 36;
	_rhythmBankLastNote = 75;
	// Elvira 1 version uses instruments 1-5 for rhythm, Elvira 2 / Waxworks
	// version uses 0x80-0x84.
	byte *rhythmInstrumentDefinitions = driverData + instrumentDataOffset + ((newVersion ? 0x80 : 1) * 9);
	byte *rhythmNotes = driverData + rhythmNoteOffset;
	for (int i = 0; i < 40; i++) {
		byte instrumentDefNumber = RHYTHM_NOTE_INSTRUMENT_TYPES[i] > 0xA ? 0 : RHYTHM_NOTE_INSTRUMENT_TYPES[i] - 6;
		OplInstrumentRhythmType rhythmType = RHYTHM_NOTE_INSTRUMENT_TYPES[i] > 0xA ? RHYTHM_TYPE_UNDEFINED :
			static_cast<OplInstrumentRhythmType>(11 - RHYTHM_NOTE_INSTRUMENT_TYPES[i]);
		byte *instrumentData = rhythmInstrumentDefinitions + (instrumentDefNumber * 9);

		loadInstrumentData(_rhythmBank[i], instrumentData, rhythmType, rhythmNotes[i], newVersion);
	}
}

MidiDriver_Multisource *MidiDriver_Accolade_AdLib_create(Common::String driverFilename, OPL::Config::OplType oplType, int timerFrequency) {
	byte *driverData = nullptr;
	uint16 driverDataSize = 0;
	bool newVersion = false;

	MidiDriver_Accolade_readDriver(driverFilename, MT_ADLIB, driverData, driverDataSize, newVersion);
	if (!driverData)
		error("ACCOLADE-ADLIB: error during readDriver()");

	MidiDriver_Accolade_AdLib *driver = new MidiDriver_Accolade_AdLib(oplType, newVersion, timerFrequency);
	if (!driver)
		error("ACCOLADE-ADLIB: could not create driver");

	driver->readDriverData(driverData, driverDataSize, newVersion);

	delete[] driverData;
	return driver;
}

} // End of namespace AGOS
