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

#include "darkseed/adlib_dsf.h"

namespace Darkseed {

// F-num values used for the 12 octave notes.
const uint16 MidiDriver_DarkSeedFloppy_AdLib::OPL_NOTE_FREQUENCIES[12] = {
	0x157, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287
};

MidiDriver_DarkSeedFloppy_AdLib::MidiDriver_DarkSeedFloppy_AdLib(OPL::Config::OplType oplType, int timerFrequency) :
		MidiDriver_ADLIB_Multisource::MidiDriver_ADLIB_Multisource(oplType, timerFrequency) {

	_dsfInstrumentBank = new OplInstrumentDefinition[128];
	_instrumentBank = _dsfInstrumentBank;
	Common::fill(_sourcePriority, _sourcePriority + sizeof(_sourcePriority), 0);
	
	_defaultChannelVolume = 0x7F;
	// Dark Seed uses rhythm instrument definitions with instrument numbers 0x0 - 0xE
	_rhythmInstrumentMode = RHYTHM_INSTRUMENT_MODE_RHYTHM_TYPE;
	_instrumentWriteMode = INSTRUMENT_WRITE_MODE_FIRST_NOTE_ON;
}

MidiDriver_DarkSeedFloppy_AdLib::~MidiDriver_DarkSeedFloppy_AdLib() {
	delete[] _dsfInstrumentBank;
}

int MidiDriver_DarkSeedFloppy_AdLib::open() {
	int result = MidiDriver_ADLIB_Multisource::open();
	if (result == 0)
		// Dark Seed has the OPL rhythm mode always on
		setRhythmMode(true);

	return result;
}

void MidiDriver_DarkSeedFloppy_AdLib::deinitSource(uint8 source) {
	MidiDriver_ADLIB_Multisource::deinitSource(source);

	_sourcePriority[source] = 0;
}

void MidiDriver_DarkSeedFloppy_AdLib::setSourcePriority(uint8 source, uint8 priority) {
	assert(source < MAXIMUM_SOURCES);
	_sourcePriority[source] = priority;
}

void MidiDriver_DarkSeedFloppy_AdLib::loadInstrumentBank(uint8 *instrumentBankData) {
	// Dark Seed stores instruments in SIT files. Most music tracks have their
	// own instrument bank, but there are only 2 significantly different banks.
	// START loads the instrument bank for each of the 8 tracks it plays, but
	// each bank is effectively the same. TOS only loads the TOS1.SIT 
	// instrument bank; the SIT files from the music tracks it plays are
	// ignored.
	// 
	// All SIT files contain 256 instruments in the 16 byte AdLib BNK format.
	// Only the first 128 instruments are actually loaded; the rest is usually
	// empty.
	for (int i = 0; i < 128; i++) {
		AdLibIbkInstrumentDefinition _ibkInstrument;
		_ibkInstrument.o0FreqMultMisc = *instrumentBankData++;
		_ibkInstrument.o1FreqMultMisc = *instrumentBankData++;
		_ibkInstrument.o0Level = *instrumentBankData++;
		_ibkInstrument.o1Level = *instrumentBankData++;
		_ibkInstrument.o0DecayAttack = *instrumentBankData++;
		_ibkInstrument.o1DecayAttack = *instrumentBankData++;
		_ibkInstrument.o0ReleaseSustain = *instrumentBankData++;
		_ibkInstrument.o1ReleaseSustain = *instrumentBankData++;
		_ibkInstrument.o0WaveformSelect = *instrumentBankData++;
		_ibkInstrument.o1WaveformSelect = *instrumentBankData++;
		_ibkInstrument.connectionFeedback = *instrumentBankData++;

		// The first 15 instruments are rhythm instrument definitions, meant
		// for the 5 OPL rhythm mode instruments. 0-2 are bass drum instruments,
		// 3-5 are snare drum instruments, etc.
		uint8 rhythmType = 0;
		if (i < 15) {
			rhythmType = 6 + (i / 3);
		}
		_ibkInstrument.rhythmType = rhythmType;
		_ibkInstrument.rhythmNote = 0;
		_ibkInstrument.transpose = 0;

		_ibkInstrument.toOplInstrumentDefinition(_dsfInstrumentBank[i]);

		// Skip padding bytes
		instrumentBankData += 5;
	}
}

uint8 MidiDriver_DarkSeedFloppy_AdLib::allocateOplChannel(uint8 channel, uint8 source, InstrumentInfo &instrumentInfo) {
	uint8 allocatedChannel = 0xFF;

	_allocationMutex.lock();
	_activeNotesMutex.lock();

	if (instrumentInfo.instrumentId <= 0xE) {
		// The first 15 instruments are rhythm instruments. These get assigned
		// to the corresponding OPL rhythm instruments.
		// Note: original code also processes instrument 0xF, leading to
		// undefined behavior.

		// The order of the rhythm instruments is flipped compared to the order
		// in the _activeRhythmNotes array.
		uint8 rhythmInstType = 4 - (instrumentInfo.instrumentId / 3);
		allocatedChannel = OPL_RHYTHM_INSTRUMENT_CHANNELS[rhythmInstType];
		if (_activeRhythmNotes[rhythmInstType].channelAllocated && _activeRhythmNotes[rhythmInstType].source != source) {
			// OPL rhythm instrument is already allocated
			if (_sourcePriority[_activeRhythmNotes[rhythmInstType].source] >= _sourcePriority[source]) {
				// Current source priority is equal to or higher than the new
				// source priority. Do not re-allocate this rhythm instrument.
				allocatedChannel = 0xFF;
			} else {
				// Current source priority is lower than the new source
				// priority. Deallocate the channel from the current source.
				if (_activeRhythmNotes[rhythmInstType].noteActive)
					writeKeyOff(allocatedChannel, static_cast<OplInstrumentRhythmType>(rhythmInstType + 1));
				_channelAllocations[_activeRhythmNotes[rhythmInstType].source][_activeRhythmNotes[rhythmInstType].channel] = 0xFF;
			}
		}

		if (allocatedChannel != 0xFF) {
			// Allocate the OPL channel to the source and rhythm instrument.
			_activeRhythmNotes[rhythmInstType].channelAllocated = true;
			_activeRhythmNotes[rhythmInstType].source = source;
			_activeRhythmNotes[rhythmInstType].channel = channel;
		}
	}
	else {
		// For melodic instruments, the following OPL channel is allocated:
		// - The OPL channel already allocated to this data channel.
		// - The OPL channel with the lowest priority, if it is lower than the
		//   priority of the new source.
		uint8 lowestPriority = 0x64;
		for (int i = 0; i < _numMelodicChannels; i++) {
			uint8 oplChannel = _melodicChannels[i];
			if (_activeNotes[oplChannel].channelAllocated && _activeNotes[oplChannel].source == source && _activeNotes[oplChannel].channel == channel) {
				// This OPL channel is already allocated to this source and
				// data channel. Use this OPL channel.
				allocatedChannel = oplChannel;
				lowestPriority = 0;
				break;
			}
			// Unallocated channels are treated as having priority 0, the
			// lowest priority.
			uint8 currentChannelPriority = !_activeNotes[oplChannel].channelAllocated ? 0 : _sourcePriority[_activeNotes[oplChannel].source];
			if (currentChannelPriority < lowestPriority) {
				// Found an OPL channel with a lower priority than the
				// previously found OPL channel.
				allocatedChannel = i;
				lowestPriority = currentChannelPriority;
			}
		}

		if (_sourcePriority[source] <= lowestPriority) {
			// New source priority is lower than the lowest found OPL channel
			// priority. Do not re-allocate this OPL channel.
			allocatedChannel = 0xFF;
		}

		if (allocatedChannel != 0xFF) {
			if (_activeNotes[allocatedChannel].channelAllocated && _activeNotes[allocatedChannel].source != source) {
				// OPL channel is already allocated. De-allocate it.
				if (_activeNotes[allocatedChannel].noteActive)
					writeKeyOff(allocatedChannel);
				_channelAllocations[_activeRhythmNotes[allocatedChannel].source][_activeRhythmNotes[allocatedChannel].channel] = 0xFF;
			}

			// Allocate the OPL channel to the source and data channel.
			_activeNotes[allocatedChannel].channelAllocated = true;
			_activeNotes[allocatedChannel].source = source;
			_activeNotes[allocatedChannel].channel = channel;
		}
	}

	if (allocatedChannel != 0xFF) {
		_channelAllocations[source][channel] = allocatedChannel;
	}

	_allocationMutex.unlock();
	_activeNotesMutex.unlock();

	return allocatedChannel;
}

uint16 MidiDriver_DarkSeedFloppy_AdLib::calculateFrequency(uint8 channel, uint8 source, uint8 note) {
	uint8 octaveNote = ((note >= 120) ? 11 : (note % 12));
	uint8 block;
	if (note < 12) {
		block = 0;
	} else if (note >= 108) {
		block = 7;
	} else {
		block = (note / 12) - 1;
	}
	uint16 fnum = OPL_NOTE_FREQUENCIES[octaveNote];

	return fnum | (block << 10);
}

uint8 MidiDriver_DarkSeedFloppy_AdLib::calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, const OplInstrumentDefinition &instrumentDef, uint8 operatorNum) {
	uint8 instrumentLevel = instrumentDef.getOperatorDefinition(operatorNum).level & 0x3F;
	if (instrumentDef.getNumberOfOperators() >= 2 && operatorNum == 0) {
		// For operator 0 of a 2 operator instrument, the level from the
		// instrument definition is used without scaling, even if the
		// connection type is additive.
		return instrumentLevel;
	}
	return 0x3F - (((velocity + 0x80) * (0x3F - instrumentLevel)) >> 8);
}

} // namespace Darkseed
