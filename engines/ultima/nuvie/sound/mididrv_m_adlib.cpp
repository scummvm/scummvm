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

#include "mididrv_m_adlib.h"

namespace Ultima {
namespace Nuvie {

const uint16 MidiDriver_M_AdLib::FNUM_VALUES[24] = {
	0x0, 0x158, 0x182, 0x1B0, 0x1CC, 0x203, 0x241, 0x286, 
	0x0, 0x16A, 0x196, 0x1C7, 0x1E4, 0x21E, 0x25F, 0x2A8, 
	0x0, 0x147, 0x16E, 0x19A, 0x1B5, 0x1E9, 0x224, 0x266
};

MidiDriver_M_AdLib::MidiDriver_M_AdLib() : MidiDriver_ADLIB_Multisource(OPL::Config::kOpl2, 60) {
	_modulationDepth = MODULATION_DEPTH_LOW;
	_vibratoDepth = VIBRATO_DEPTH_LOW;
	_allocationMode = ALLOCATION_MODE_STATIC;
	_instrumentWriteMode = INSTRUMENT_WRITE_MODE_PROGRAM_CHANGE;

	Common::fill(_slideValues, _slideValues + ARRAYSIZE(_slideValues), 0);
	Common::fill(_vibratoDepths, _vibratoDepths + ARRAYSIZE(_vibratoDepths), 0);
	Common::fill(_vibratoFactors, _vibratoFactors + ARRAYSIZE(_vibratoFactors), 0);
	Common::fill(_vibratoCurrentDepths, _vibratoCurrentDepths + ARRAYSIZE(_vibratoCurrentDepths), 0);
	Common::fill(_vibratoDirections, _vibratoDirections + ARRAYSIZE(_vibratoDirections), VIBRATO_DIRECTION_RISING);
	Common::fill(_fadeDirections, _fadeDirections + ARRAYSIZE(_fadeDirections), FADE_DIRECTION_NONE);
	Common::fill(_fadeStepDelays, _fadeStepDelays + ARRAYSIZE(_fadeStepDelays), 0);
	Common::fill(_fadeCurrentDelays, _fadeCurrentDelays + ARRAYSIZE(_fadeCurrentDelays), 0);

	_instrumentBank = _instrumentBankPtr = new OplInstrumentDefinition[16];
}

MidiDriver_M_AdLib::~MidiDriver_M_AdLib() {
	delete[] _instrumentBankPtr;
}

void MidiDriver_M_AdLib::send(int8 source, uint32 b) {
	byte command = b & 0xF0;
	byte channel = b & 0x0F;
	byte data = (b >> 8) & 0xFF;

	ActiveNote *activeNote;
	//uint16 channelOffset;
	//uint16 frequency;
	switch (command) {
	case 0x00: // Note off
		// The original driver always writes both F-num registers with the 
		// supplied note value; it does not check what the active note value 
		// is. The standard noteOff implementation checks if the active note 
		// value matches an active note on the data channel. If the note off 
		// does not match the active note, this could cause a hanging note.
		// None of the Ultima 6 tracks seem to have this problem however.

		/* DEBUG: Write Ax register
		// Calculate the frequency.
		channelOffset = determineChannelRegisterOffset(channel);
		frequency = calculateFrequency(channel, source, data);

		// Write the low 8 frequency bits.
		writeRegister(OPL_REGISTER_BASE_FNUMLOW + channelOffset, frequency & 0xFF);
		*/

		noteOff(channel, data, 0, source);
		break;

	case 0x10: // Note on
		// Stop vibrato (if active)
		_vibratoDirections[channel] = VIBRATO_DIRECTION_RISING;
		_vibratoCurrentDepths[channel] = 0;

		// The original driver always writes a note off before a note on, even 
		// if there is no note active. The standard noteOn implementation only 
		// writes a note off if a note is active. This causes no audible
		// difference however.

		/* DEBUG: Write note off
		_activeNotesMutex.lock();

		// Melodic instrument.
		activeNote = &_activeNotes[channel];
		
		// Calculate the frequency.
		channelOffset = determineChannelRegisterOffset(channel);
		frequency = calculateFrequency(channel, source, data);
		activeNote->oplFrequency = frequency;

		// Write the low 8 frequency bits.
		writeRegister(OPL_REGISTER_BASE_FNUMLOW + channelOffset, frequency & 0xFF);
		// Write the high 2 frequency bits and block and add the key on bit.
		writeRegister(OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON + channelOffset, frequency >> 8);

		// Update the active note data.
		activeNote->noteActive = false;
		activeNote->noteSustained = false;
		// Register the current note counter value when turning off a note.
		activeNote->noteCounterValue = _noteCounter;

		_activeNotesMutex.unlock();
		*/

		noteOn(channel, data, 0x7F, source);
		break;

	case 0x20: // Set pitch
		// If a note is already active on this channel, this will just update 
		// the pitch. Otherwise it is the same as a Note on.

		_activeNotesMutex.lock();

		// Determine the OPL channel to use and the active note data to update.
		uint8 oplChannel;
		oplChannel = 0xFF;
		activeNote = nullptr;
		// Allocate a melodic OPL channel.
		oplChannel = allocateOplChannel(channel, source, 0);
		if (oplChannel != 0xFF)
			activeNote = &_activeNotes[oplChannel];

		if (activeNote != nullptr) {
			if (!activeNote->noteActive) {
				// If there is no note active currently, treat this as a
				// regular note on.
				noteOn(channel, data, 0x7F, source);
			} else {
				// If there is a note active, only update the frequency.
				activeNote->note = data;
				activeNote->oplNote = data;
				// Calculate and write frequency and block and write key on bit.
				writeFrequency(oplChannel);
			}
		}

		_activeNotesMutex.unlock();

		break;

	case 0x30: // Set level
		// This directly writes the OPL level register of the carrier operator. 
		// This can also write the key scale level bits.
		// Note that the control data volume field is used for an OPL level 
		// value, not for a MIDI channel volume value as usual.

		// Stop fade (if active)
		_fadeDirections[channel] = FADE_DIRECTION_NONE;

		_controlData[source][channel].volume = data;
		if (_activeNotes[channel].instrumentDef)
			writeVolume(channel, 1);

		break;

	case 0x40: // Set modulation
		modulation(channel, data, source);
		break;

	case 0x50: // Set slide
		// Start or stop a pitch slide. The slide is processed by onTimer.
		_slideValues[channel] = (int8)data;
		break;

	case 0x60: // Set vibrato
		// Turns vibrato on or off or modifies the parameters. High nibble 
		// is the vibrato depth, low nibble is the vibrato factor. The vibrato
		// is processed by onTimer.
		_vibratoDepths[channel] = data >> 4;
		_vibratoFactors[channel] = data & 0xF;
		break;

	case 0x70: // Program change
		programChange(channel, data, source);
		break;

	case 0x80: // Subcommand
		uint8 subcommand;
		subcommand = channel;
		switch (subcommand) {
		case 0x1: // Call subroutine
		case 0x2: // Delay
			// These are handled by the parser.
			break;

		case 0x3: // Load instrument
			// This should be sent to the driver as a meta event.
			warning("MidiDriver_M_AdLib::send - Received load instrument as command");
			break;

		case 0x5: // Fade out
		case 0x6: // Fade in
			// Starts a volume fade in or out. The high nibble of the data byte
			// is the channel, the low nibble is the fade delay. The fade is
			// processed by onTimer.

			channel = data >> 4;
			_fadeDirections[channel] = (subcommand == 0x5 ? FADE_DIRECTION_FADE_OUT : FADE_DIRECTION_FADE_IN);
			uint8 delay;
			delay = (data & 0xF) + 1;
			_fadeStepDelays[channel] = _fadeCurrentDelays[channel] = delay;
			break;

		default: // Unknown subcommand
			break;
		}
		break;

	case 0xE0: // Set loop point
	case 0xF0: // Return
		// These are handled by the parser.
		break;

	default: // Unknown command
		break;
	}
}

void MidiDriver_M_AdLib::metaEvent(int8 source, byte type, byte* data, uint16 length) {
	if (type == 0x3) {
		// Load instrument
		// This loads an OPL instrument definition into the bank. The first 
		// byte is the instrument bank number. The next 11 bytes contain the 
		// instrument parameters.

		if (length < 12) {
			warning("Received a load instrument event with insufficient data length");
			return;
		}

		byte instrumentNumber = data[0];
		assert(instrumentNumber < 16);
		OplInstrumentDefinition *instrument = &_instrumentBankPtr[instrumentNumber];

		instrument->fourOperator = false;
		instrument->rhythmType = RHYTHM_TYPE_UNDEFINED;

		instrument->operator0.freqMultMisc = data[1];
		instrument->operator0.level = data[2];
		instrument->operator0.decayAttack = data[3];
		instrument->operator0.releaseSustain = data[4];
		instrument->operator0.waveformSelect = data[5];

		instrument->operator1.freqMultMisc = data[6];
		instrument->operator1.level = data[7];
		instrument->operator1.decayAttack = data[8];
		instrument->operator1.releaseSustain = data[9];
		instrument->operator1.waveformSelect = data[10];

		instrument->connectionFeedback0 = data[11];
	}
}

void MidiDriver_M_AdLib::programChange(uint8 channel, uint8 program, uint8 source) {
	assert(program < 16);

	// Changing the instrument overwrites the current volume and modulation
	// settings.
	_controlData[source][channel].volume = _instrumentBank[program].operator1.level;
	_controlData[source][channel].modulation = _instrumentBank[program].operator0.level;

	// Note that this will turn off an active note on the channel if there is
	// one. The original driver does not do this. Changing the instrument while
	// a note is playing would be strange though; none of the tracks in
	// Ultima 6 seem to do this.
	MidiDriver_ADLIB_Multisource::programChange(channel, program, source);
}

void MidiDriver_M_AdLib::modulation(uint8 channel, uint8 modulation, uint8 source) {
	// This directly writes the OPL level register of the modulator
	// operator.

	// Note that the control data modulation field is used for an OPL level
	// value, not for a MIDI channel modulation value as usual.
	_controlData[source][channel].modulation = modulation;

	uint16 registerOffset = determineOperatorRegisterOffset(channel, 0);
	writeRegister(OPL_REGISTER_BASE_LEVEL + registerOffset, modulation);
}

uint8 MidiDriver_M_AdLib::allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) {
	// Allocation of M data channels to OPL output channels is simply 1 on 1.
	return channel;
}

uint16 MidiDriver_M_AdLib::calculateFrequency(uint8 channel, uint8 source, uint8 note) {
	// An M note value consist of a note lookup value in the low 5 bits and
	// a block (octave) value in the high 3 bits.
	uint8 fnumIndex = note & 0x1F;
	assert(fnumIndex < 24);

	uint16 oplFrequency = FNUM_VALUES[fnumIndex];
	uint8 block = note >> 5;

	return oplFrequency | (block << 10);
}

uint8 MidiDriver_M_AdLib::calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, const OplInstrumentDefinition& instrumentDef, uint8 operatorNum) {
	// M directy uses OPL level values, so no calculation is necessary.
	return _controlData[source][channel].volume & OPL_MASK_LEVEL;
}

void MidiDriver_M_AdLib::writeVolume(uint8 oplChannel, uint8 operatorNum, OplInstrumentRhythmType rhythmType) {
	ActiveNote *activeNote = (rhythmType == RHYTHM_TYPE_UNDEFINED ? &_activeNotes[oplChannel] : &_activeRhythmNotes[rhythmType - 1]);

	// Calculate operator volume.
	uint16 registerOffset = determineOperatorRegisterOffset(
		oplChannel, operatorNum, rhythmType, activeNote->instrumentDef->fourOperator);
	uint8 level = calculateVolume(activeNote->channel, activeNote->source, activeNote->velocity,
								  *activeNote->instrumentDef, operatorNum);

	// Add key scaling level from the last written volume or modulation value
	// to the calculated level.
	MidiChannelControlData *controlData = &_controlData[activeNote->source][activeNote->channel];
	uint8 ksl = (operatorNum == 0 ? controlData->modulation : controlData->volume) & ~OPL_MASK_LEVEL;
	writeRegister(OPL_REGISTER_BASE_LEVEL + registerOffset, level | ksl);
}

void MidiDriver_M_AdLib::deinitSource(uint8 source) {
	// Reset effects status.
	Common::fill(_slideValues, _slideValues + ARRAYSIZE(_slideValues), 0);
	Common::fill(_vibratoFactors, _vibratoFactors + ARRAYSIZE(_vibratoFactors), 0);
	Common::fill(_vibratoCurrentDepths, _vibratoCurrentDepths + ARRAYSIZE(_vibratoCurrentDepths), 0);
	Common::fill(_vibratoDirections, _vibratoDirections + ARRAYSIZE(_vibratoDirections), VIBRATO_DIRECTION_RISING);
	Common::fill(_fadeDirections, _fadeDirections + ARRAYSIZE(_fadeDirections), FADE_DIRECTION_NONE);

	MidiDriver_ADLIB_Multisource::deinitSource(source);
}

void MidiDriver_M_AdLib::onTimer() {
	MidiDriver_ADLIB_Multisource::onTimer();

	_activeNotesMutex.lock();

	// Process effects.
	for (int i = 8; i >= 0; i--) {
		ActiveNote *activeNote = &_activeNotes[i];

		if (_slideValues[i] != 0) {
			// Process slide. A slide continually increases or decreases the
			// note frequency until it is turned off.

			// Increase or decrease the OPL frequency by the slide value.
			// Note that this can potentially over- or underflow the OPL
			// frequency, but there is no bounds checking in the original
			// driver either.
			activeNote->oplFrequency += _slideValues[i];

			// Write the low 8 frequency bits.
			uint16 channelOffset = determineChannelRegisterOffset(i);
			writeRegister(OPL_REGISTER_BASE_FNUMLOW + channelOffset, activeNote->oplFrequency & 0xFF);
			// Write the high 2 frequency bits and block and add the key on bit.
			writeRegister(OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON + channelOffset,
				(activeNote->oplFrequency >> 8) | (activeNote->noteActive ? OPL_MASK_KEYON : 0));
		} else if (_vibratoFactors[i] > 0 && activeNote->noteActive) {
			// Process vibrato. Vibrato will alternately increase and decrease
			// the frequency up to the maximum depth.
			// The depth is the difference between the minimum and maximum
			// frequency change, so a positive number, twice the amplitude.
			// The current depth is converted to the actual frequency offset by
			// subtracting half the total depth. The offset is then multiplied
			// by the vibrato factor.
			// Note that current depth starts at 0, so minimum depth, rather
			// than at neutral (half depth).

			// Flip vibrato direction if the maximum or minimum depth has been reached.
			if (_vibratoCurrentDepths[i] >= _vibratoDepths[i]) {
				_vibratoDirections[i] = VIBRATO_DIRECTION_FALLING;
			} else if (_vibratoCurrentDepths[i] == 0) {
				_vibratoDirections[i] = VIBRATO_DIRECTION_RISING;
			}

			// Update current depth.
			if (_vibratoDirections[i] == VIBRATO_DIRECTION_FALLING) {
				_vibratoCurrentDepths[i]--;
			} else {
				_vibratoCurrentDepths[i]++;
			}

			// Convert the depth to an OPL frequency offset.
			int vibratoOffset = _vibratoCurrentDepths[i] - (_vibratoDepths[i] >> 1);
			vibratoOffset *= _vibratoFactors[i];

			uint16 frequency = activeNote->oplFrequency + vibratoOffset;

			// Write the low 8 frequency bits.
			uint16 channelOffset = determineChannelRegisterOffset(i);
			writeRegister(OPL_REGISTER_BASE_FNUMLOW + channelOffset, frequency & 0xFF);
			// Write the high 2 frequency bits and block and add the key on bit.
			writeRegister(OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON + channelOffset,
				(frequency >> 8) | (activeNote->noteActive ? OPL_MASK_KEYON : 0));
		}

		if (_fadeDirections[i] != FADE_DIRECTION_NONE && --_fadeCurrentDelays[i] == 0) {
			// Process fade. A fade will continually increase or decrease the 
			// level (volume) until the maximum or minimum volume is reached.
			// Then the fade is stopped. A delay determines the speed of the
			// fade by increasing the number of ticks between each increase or
			// decrease.

			// Reset delay.
			_fadeCurrentDelays[i] = _fadeStepDelays[i];

			// Calculate new channel level.
			int newChannelLevel = _controlData[activeNote->source][i].volume + (_fadeDirections[i] == FADE_DIRECTION_FADE_IN ? -1 : 1);
			if (newChannelLevel < 0 || newChannelLevel > 0x3F) {
				// Minimum or maximum level reached. Stop the fade.
				newChannelLevel = (newChannelLevel < 0) ? 0 : 0x3F;
				_fadeDirections[i] = FADE_DIRECTION_NONE;
			}

			// Apply the new volume.
			_controlData[activeNote->source][i].volume = newChannelLevel;
			writeVolume(i, 1);
		}
	}

	_activeNotesMutex.unlock();
}

} // End of namespace Nuvie
} // End of namespace Ultima
