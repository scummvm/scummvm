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

#include "agos/drivers/simon1/adlib.h"

#include "common/file.h"

namespace AGOS {

// Rhythm map hardcoded in the Simon 1 executable.
const MidiDriver_Simon1_AdLib::RhythmMapEntry MidiDriver_Simon1_AdLib::RHYTHM_MAP[39] = {
	{ 11, 123,  40 },
	{ 12, 127,  50 },
	{ 12, 124,   1 },
	{ 12, 124,  90 },
	{ 13, 125,  50 },
	{ 13, 125,  25 },
	{ 15, 127,  80 },
	{ 13, 125,  25 },
	{ 15, 127,  40 },
	{ 13, 125,  35 },
	{ 15, 127,  90 },
	{ 13, 125,  35 },
	{ 13, 125,  45 },
	{ 14, 126,  90 },
	{ 13, 125,  45 },
	{ 15, 127,  90 },
	{  0,   0,   0 },
	{ 15, 127,  60 },
	{  0,   0,   0 },
	{ 13, 125,  60 },
	{  0,   0,   0 },
	{  0,   0,   0 },
	{  0,   0,   0 },
	{ 13, 125,  45 },
	{ 13, 125,  40 },
	{ 13, 125,  35 },
	{ 13, 125,  30 },
	{ 13, 125,  25 },
	{ 13, 125,  80 },
	{ 13, 125,  40 },
	{ 13, 125,  80 },
	{ 13, 125,  40 },
	{ 14, 126,  40 },
	{ 15, 127,  60 },
	{  0,   0,   0 },
	{  0,   0,   0 },
	{ 14, 126,  80 },
	{  0,   0,   0 },
	{ 13, 125, 100 }
};

// Frequency table hardcoded in the Simon 1 executable.
// Note that only the first 12 entries are used.
const uint16 MidiDriver_Simon1_AdLib::FREQUENCY_TABLE[16] = {
	0x0157, 0x016B, 0x0181, 0x0198, 0x01B0, 0x01CA, 0x01E5, 0x0202,
	0x0220, 0x0241, 0x0263, 0x0287, 0x2100, 0xD121, 0xA307, 0x46A4
};

MidiDriver_Simon1_AdLib::MidiDriver_Simon1_AdLib(OPL::Config::OplType oplType, const byte *instrumentData) : MidiDriver_ADLIB_Multisource(oplType), _musicRhythmNotesDisabled(false) {
	// The Simon 1 MIDI data is written for MT-32 and rhythm notes are played
	// by quickly turning a note on and off. The note off has no effect on an
	// MT-32, but it will cut off the rhythm note on OPL. To prevent this, note
	// off events for rhythm notes are ignored and the rhythm note is turned
	// off right before this rhythm instrument is played again. The original
	// interpreter does this as well.
	_rhythmModeIgnoreNoteOffs = true;

	parseInstrumentData(instrumentData);
}

MidiDriver_Simon1_AdLib::~MidiDriver_Simon1_AdLib() {
	delete[] _instrumentBank;
	delete[] _rhythmBank;
}

int MidiDriver_Simon1_AdLib::open() {
	int result = MidiDriver_ADLIB_Multisource::open();
	if (result >= 0)
		// Simon 1 has the OPL rhythm mode permanently enabled.
		setRhythmMode(true);

	return result;
}

void MidiDriver_Simon1_AdLib::parseInstrumentData(const byte *instrumentData) {
	const byte *dataPtr = instrumentData;

	// The instrument data consists of 128 16-byte entries.
	OplInstrumentDefinition *instrumentBank = new OplInstrumentDefinition[128];

	for (int i = 0; i < 128; i++) {
		instrumentBank[i].fourOperator = false;

		instrumentBank[i].operator0.freqMultMisc = *dataPtr++;
		instrumentBank[i].operator1.freqMultMisc = *dataPtr++;
		instrumentBank[i].operator0.level = *dataPtr++;
		instrumentBank[i].operator1.level = *dataPtr++;
		instrumentBank[i].operator0.decayAttack = *dataPtr++;
		instrumentBank[i].operator1.decayAttack = *dataPtr++;
		instrumentBank[i].operator0.releaseSustain = *dataPtr++;
		instrumentBank[i].operator1.releaseSustain = *dataPtr++;
		instrumentBank[i].operator0.waveformSelect = *dataPtr++;
		instrumentBank[i].operator1.waveformSelect = *dataPtr++;

		instrumentBank[i].connectionFeedback0 = *dataPtr++;
		instrumentBank[i].connectionFeedback1 = 0;
		instrumentBank[i].rhythmNote = 0;
		instrumentBank[i].rhythmType = RHYTHM_TYPE_UNDEFINED;

		// Remaining bytes seem to be unused.
		dataPtr += 5;
	}

	// Construct a rhythm bank from the original rhythm map data.
	OplInstrumentDefinition *rhythmBank = new OplInstrumentDefinition[39];
	// MIDI note range 36-74.
	_rhythmBankFirstNote = 36;
	_rhythmBankLastNote = 36 + 39 - 1;

	for (int i = 0; i < 39; i++) {
		if (RHYTHM_MAP[i].channel == 0) {
			// Some notes in the range have no definition.
			rhythmBank[i].rhythmType = RHYTHM_TYPE_UNDEFINED;
		} else {
			// The rhythm bank makes use of instruments defined in the main instrument bank.
			rhythmBank[i] = _instrumentBank[RHYTHM_MAP[i].program];
			// The MIDI channels used in the rhythm map correspond to OPL rhythm instrument types:
			// 11 - bass drum
			// 12 - snare drum
			// 13 - tom tom
			// 14 - cymbal
			// 15 - hi-hat
			rhythmBank[i].rhythmType = static_cast<OplInstrumentRhythmType>(6 - (RHYTHM_MAP[i].channel - 10));
			rhythmBank[i].rhythmNote = RHYTHM_MAP[i].note;
		}
	}

	// Set the const class variables with our just allocated banks
	_instrumentBank = instrumentBank;
	_rhythmBank = rhythmBank;
}

void MidiDriver_Simon1_AdLib::noteOn(uint8 channel, uint8 note, uint8 velocity, uint8 source) {
	if (_musicRhythmNotesDisabled && _sources[source].type != SOURCE_TYPE_SFX && channel == MIDI_RHYTHM_CHANNEL)
		// A music source played a rhythm note while these are disabled.
		// Ignore this event.
		return;
	if (_sources[source].type == SOURCE_TYPE_SFX)
		// The original interpreter uses max velocity for all SFX notes.
		velocity = 0x7F;

	MidiDriver_ADLIB_Multisource::noteOn(channel, note, velocity, source);
}

void MidiDriver_Simon1_AdLib::programChange(uint8 channel, uint8 program, uint8 source) {
	MidiDriver_ADLIB_Multisource::programChange(channel, program, source);

	_activeNotesMutex.lock();

	// Deallocate all inactive OPL channels for this MIDI channel and source.
	for (int i = 0; i < _numMelodicChannels; i++) {
		uint8 oplChannel = _melodicChannels[i];
		if (_activeNotes[oplChannel].channelAllocated && !_activeNotes[oplChannel].noteActive &&
			_activeNotes[oplChannel].channel == channel && _activeNotes[oplChannel].source == source) {
			_activeNotes[oplChannel].channelAllocated = false;
		}
	}

	_activeNotesMutex.unlock();

	// Note: the original also sets up the new instrument on active OPL
	// channels, i.e. channels which are currently playing a note. This is
	// against the MIDI spec, which states that program changes should not
	// affect active notes, and against the behavior of the MT-32, for which
	// the music is composed. So instead, the new instrument is set up when a
	// new note is played on these OPL channels.
}

void MidiDriver_Simon1_AdLib::deinitSource(uint8 source) {
	if (_sources[source].type != SOURCE_TYPE_MUSIC)
		// When a sound effect has finished playing, re-enable music rhythm
		// notes.
		_musicRhythmNotesDisabled = false;

	MidiDriver_ADLIB_Multisource::deinitSource(source);
}

void MidiDriver_Simon1_AdLib::disableMusicRhythmNotes() {
	_musicRhythmNotesDisabled = true;
}

uint8 MidiDriver_Simon1_AdLib::allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) {
	// When allocating an OPL channel for playback of a note, the algorithm
	// looks for the following types of channels:
	// - An OPL channel already allocated to this source and MIDI channel that
	//   is not playing a note.
	// - An unallocated OPL channel.
	// - An OPL channel allocated to a different source and/or MIDI channel
	//   that is not playing a note.
	//
	// If no free OPL channel could be found, an active channel is "stolen" and
	// the note it is currently playing is cut off. This channel is always
	// channel 0.
	uint8 allocatedChannel = 0xFF;

	uint8 unallocatedChannel = 0xFF;
	uint8 inactiveChannel = 0xFF;
	for (int i = 0; i < _numMelodicChannels; i++) {
		uint8 oplChannel = _melodicChannels[i];
		if (_activeNotes[oplChannel].channelAllocated && _activeNotes[oplChannel].channel == channel &&
			_activeNotes[oplChannel].source == source && !_activeNotes[oplChannel].noteActive) {
			// Found an OPL channel already allocated to this source and MIDI
			// channel that is not playing a note.
			allocatedChannel = oplChannel;
			// Always use the first available channel of this type.
			break;
		}

		if (!_activeNotes[oplChannel].channelAllocated && unallocatedChannel == 0xFF)
			// Found an unallocated OPL channel.
			unallocatedChannel = oplChannel;

		if (!_activeNotes[oplChannel].noteActive && inactiveChannel == 0xFF)
			// Found an OPL channel allocated to a different source and/or MIDI
			// channel that is not playing a note.
			inactiveChannel = oplChannel;
	}
	if (allocatedChannel == 0xFF) {
		// No allocated channel found.
		if (unallocatedChannel != 0xFF) {
			// Found an unallocated channel - use this.
			allocatedChannel = unallocatedChannel;
		} else if (inactiveChannel != 0xFF) {
			// Found an inactive channel - use this.
			allocatedChannel = inactiveChannel;
		} else {
			// A channel already playing a note must be "stolen".

			// The original had some logic for a priority based reuse of
			// channels. However, the priority value is always 0, which causes
			// the first channel to be picked all the time.
			allocatedChannel = 0;
		}
	}

	if (_activeNotes[allocatedChannel].noteActive)
		// Turn off the current note if the channel was "stolen".
		writeKeyOff(allocatedChannel);
	_activeNotes[allocatedChannel].channelAllocated = true;
	_activeNotes[allocatedChannel].source = source;
	_activeNotes[allocatedChannel].channel = channel;

	return allocatedChannel;
}

uint16 MidiDriver_Simon1_AdLib::calculateFrequency(uint8 channel, uint8 source, uint8 note) {
	// Determine the octave note. Notes 120-127 are clipped to octave note 12.
	uint8 octaveNote = note >= 120 ? 12 : note % 12;
	// Determine the octave / block. Notes 12-96 are in octaves 0-7, with lower
	// and higher notes clipped to octave 0 and 7, respectively.
	uint8 octave = CLIP((note / 12) - 1, 0, 7);

	// Look up the OPL frequency / F-num.
	uint16 octaveNoteFrequency = FREQUENCY_TABLE[octaveNote];

	// Combine block and F-num in the format used by the OPL Ax and Bx
	// registers.
	return (octave << 10) | octaveNoteFrequency;
}

uint8 MidiDriver_Simon1_AdLib::calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, const OplInstrumentDefinition &instrumentDef, uint8 operatorNum) {
	if (channel == MIDI_RHYTHM_CHANNEL && _sources[source].type != SOURCE_TYPE_SFX)
		// The original interpreter halves the velocity for music rhythm notes.
		// Note that SFX notes always use max velocity.
		velocity >>= 1;

	// Invert the instrument definition attenuation.
	uint8 instDefVolume = 0x3F - (instrumentDef.getOperatorDefinition(operatorNum).level & 0x3F);
	// Calculate the note volume using velocity and instrument definition
	// volume.
	uint8 calculatedVolume = ((velocity | 0x80) * instDefVolume) >> 8;

	// Invert the calculated volume to an attenuation.
	return 0x3F - calculatedVolume;
}

MidiDriver_Multisource *createMidiDriverSimon1AdLib(const char *instrumentFilename, OPL::Config::OplType oplType) {
	// Load instrument data.
	Common::File ibk;

	if (!ibk.open(instrumentFilename)) {
		error("MidiDriver_Simon1_AdLib::createMidiDriverSimon1AdLib - Could not find AdLib instrument bank file %s", instrumentFilename);
	}

	// Check for the expected FourCC (IBK\x1A)
	if (ibk.readUint32BE() != 0x49424b1a) {
		error("MidiDriver_Simon1_AdLib::createMidiDriverSimon1AdLib - Invalid AdLib instrument bank file %s", instrumentFilename);
	}

	byte *instrumentData = new byte[128 * 16];
	if (ibk.read(instrumentData, 128 * 16) != 128 * 16) {
		// Failed to read the expected amount of data.
		delete[] instrumentData;
		error("MidiDriver_Simon1_AdLib::createMidiDriverSimon1AdLib - Unexpected AdLib instrument bank file %s size", instrumentFilename);
	}

	MidiDriver_Simon1_AdLib *driver = new MidiDriver_Simon1_AdLib(oplType, instrumentData);
	delete[] instrumentData;

	return driver;
}

} // End of namespace AGOS
