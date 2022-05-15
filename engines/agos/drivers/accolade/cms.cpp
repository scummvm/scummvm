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

#include "agos/drivers/accolade/cms.h"

#include "common/debug.h"

namespace AGOS {

// Hardcoded in RUNVGA.EXE.
const byte MidiDriver_Accolade_Cms::CMS_NOTE_FREQUENCIES[] = {
	0x03, 0x1F, 0x3A, 0x53, 0x6B, 0x82,
	0x97, 0xAC, 0xBF, 0xD1, 0xE2, 0xF2
};

// Hardcoded in RUNVGA.EXE; probably also used for Tandy.
const byte MidiDriver_Accolade_Cms::CMS_VOLUME_ADJUSTMENTS[] = {
	0xF7, 0xFD, 0xFB, 0x00, 0xEC, 0xEE, 0x00, 0x1E, 0x08, 0x07, 0x02, 0x03, 0x08, 0x0A, 0x00, 0x00,
	0xF6, 0xF4, 0xEF, 0x05, 0xFB, 0xFC, 0xF5, 0xF5, 0xF6, 0x28, 0x00, 0xEF, 0x00, 0x00, 0x00, 0x00,
	0x29, 0x03, 0xE9, 0x00, 0xF5, 0x50, 0xF9, 0xF9, 0xF2, 0x1E, 0xFE, 0x0B, 0xF7, 0xF7, 0xF5, 0xF8,
	0xFD, 0xFD, 0xFD, 0xE4, 0xED, 0xFB, 0xFE, 0xFB, 0xD1, 0x1F, 0x00, 0xF1, 0xF1, 0xF6, 0x00, 0x0A,
	0xEE, 0xDB, 0x0E, 0xE5, 0x0B, 0x00, 0x0A, 0x0D, 0x03, 0x06, 0xF3, 0xF5, 0x1E, 0x1E, 0x0A, 0xFA,
	0xF8, 0xF9, 0x0A, 0x00, 0xFA, 0xFA, 0xF9, 0x00, 0xF2, 0xF9, 0x00, 0xF9, 0x00, 0x04, 0xF8, 0xF7,
	0xFC, 0xF1, 0x0A, 0xF1, 0x20, 0x20, 0xF7, 0xF2, 0xF6, 0xF7, 0xFA, 0xFF, 0x20, 0x20, 0x00, 0x00,
	0xEA, 0x15, 0x00, 0xFB, 0x0E, 0x00, 0x14, 0x12, 0x00, 0xE2, 0xF7, 0xFB, 0x00, 0x00, 0xF1, 0xFF
};

MidiDriver_Accolade_Cms::MidiDriver_Accolade_Cms() : _cms(nullptr), _isOpen(false), _timer_proc(nullptr), _timer_param(nullptr) {
	Common::fill(_instruments, _instruments + ARRAYSIZE(_instruments), 0);
	Common::fill(_activeNotes, _activeNotes + ARRAYSIZE(_activeNotes), 0xFF);
	Common::fill(_octaveRegisterValues, _octaveRegisterValues + ARRAYSIZE(_octaveRegisterValues), 0);
}

MidiDriver_Accolade_Cms::~MidiDriver_Accolade_Cms() {
	if (_isOpen)
		close();
}

int MidiDriver_Accolade_Cms::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_cms = CMS::Config::create();
	if (!_cms || !_cms->init())
		return MERR_CANNOT_CONNECT;

	_isOpen = true;

	cmsInit();

	_cms->start(new Common::Functor0Mem<void, MidiDriver_Accolade_Cms>(this, &MidiDriver_Accolade_Cms::onTimer));

	return 0;
}

void MidiDriver_Accolade_Cms::close() {
	if (_cms) {
		_cms->stop();
		delete _cms;
		_cms = nullptr;
	}
	_isOpen = false;
}

bool MidiDriver_Accolade_Cms::isOpen() const {
	return _isOpen;
}

uint32 MidiDriver_Accolade_Cms::getBaseTempo() {
	return 1000000 / CMS::CMS::DEFAULT_CALLBACK_FREQUENCY;
}

MidiChannel *MidiDriver_Accolade_Cms::allocateChannel() {
	return nullptr;
}

MidiChannel *MidiDriver_Accolade_Cms::getPercussionChannel() {
	return nullptr;
}

void MidiDriver_Accolade_Cms::cmsInit() {
	for (int i = 0; i < 2; i++) {
		int chipOffset = i * 0x100;
		// Set registers 0x00 - 0x19 to 0 (note that this includes a few unused
		// registers).
		for (int j = 0; j <= 0x19; j++) {
			writeRegister(j + chipOffset, 0);
		}
		// Frequency reset.
		writeRegister(REGISTER_RESET_SOUND_ENABLE + chipOffset, 0x02);
		// Sound enable.
		writeRegister(REGISTER_RESET_SOUND_ENABLE + chipOffset, 0x01);
	}
}

void MidiDriver_Accolade_Cms::send(uint32 b) {
	byte channel = b & 0x0F;

	// WORKAROUND This check is bugged in the original interpreter. The channel
	// is first resolved through the channel mapping, which is 1 on 1 for
	// channels 0-5, and the default FF for the other channels, which means
	// they should not be played. Next, the mapped channel is checked against
	// the maximum channels for the device, >= 6. However, this check is
	// signed, which means FF is interpreted as -1, and it passes the check.
	// This comes into play when drums are played on channel 9. The code for
	// handling events does not expect channel FF and this results in writes to
	// invalid registers and interference with notes on the other channels.
	// This check is fixed here to restore the intended behavior.
	// Note that this does result in an extended period of silence during the
	// intro track where only drums are played.
	if (channel >= 6)
		return;

	byte command = b & 0xF0;
	byte op1 = (b >> 8) & 0xFF;
	byte op2 = (b >> 16) & 0xFF;

	switch (command) {
	case MIDI_COMMAND_NOTE_OFF:
		noteOff(channel, op1);
		break;
	case MIDI_COMMAND_NOTE_ON:
		noteOn(channel, op1, op2);
		break;
	case MIDI_COMMAND_CONTROL_CHANGE:
		controlChange(channel, op1, op2);
		break;
	case MIDI_COMMAND_PROGRAM_CHANGE:
		programChange(channel, op1);
		break;
	default:
		// Other MIDI events are ignored.
		break;
	}
}

void MidiDriver_Accolade_Cms::noteOff(uint8 channel, uint8 note) {
	if (_activeNotes[channel] != note)
		return;

	// Remove the note from the active note registry.
	_activeNotes[channel] = 0xFF;

	// Turn off the frequency enable bit for the channel.
	byte freqEnableRegValue = determineFrequencyEnableRegisterValue();
	writeRegister(REGISTER_FREQUENCY_ENABLE, freqEnableRegValue);
}

void MidiDriver_Accolade_Cms::noteOn(uint8 channel, uint8 note, uint8 velocity) {
	if (velocity == 0) {
		// Note on with velocity 0 is a note off.
		noteOff(channel, note);
		return;
	}

	// Add the note to the active note registry.
	_activeNotes[channel] = note;

	// Add octaves to bring the note up into the 0x15 - 0x7F range, then lower
	// the note with 0x15.
	while (note <= 0x15)
		note += 0xC;
	note -= 0x15;

	// Determine the octave and note within the octave.
	byte octave = note / 0xC;
	byte octaveNote = note % 0xC;

	// Apply the volume adjustment for the current instrument on the channel.
	int8 volumeAdjustment = static_cast<int8>(CMS_VOLUME_ADJUSTMENTS[_instruments[channel]]);
	uint8 volume = CLIP(velocity + volumeAdjustment, 0, 0x7F);
	// Calculate and write the amplitude register value.
	byte amplitude = volume >> 4;
	writeRegister(REGISTER_BASE_AMPLITUDE + channel, amplitude | (amplitude << 4));

	// Look up and write the note frequecy.
	byte frequency = CMS_NOTE_FREQUENCIES[octaveNote];
	writeRegister(REGISTER_BASE_FREQUENCY + channel, frequency);

	// An octave register contains the octaves for 2 channels.
	// Get the current value of the register containing the octave value for
	// this channel.
	byte octaveRegisterValue = _octaveRegisterValues[channel / 2];
	// Clear the bits containing the octave value for this channel.
	if (channel & 1) {
		// Octave is in the upper nibble.
		octave <<= 4;
		octaveRegisterValue &= 0x0F;
	} else {
		// Octave is in the lower nibble.
		octaveRegisterValue &= 0xF0;
	}
	// Set and write the new octave value.
	octaveRegisterValue |= octave;
	_octaveRegisterValues[channel / 2] = octaveRegisterValue;
	writeRegister(REGISTER_BASE_OCTAVE + (channel / 2), octaveRegisterValue);

	// Turn on the frequency enable bit for the channel.
	byte freqEnableRegValue = determineFrequencyEnableRegisterValue();
	writeRegister(REGISTER_FREQUENCY_ENABLE, freqEnableRegValue);
}

void MidiDriver_Accolade_Cms::programChange(uint8 channel, uint8 instrument) {
	// Just keep track of the current instrument.
	_instruments[channel] = instrument;
}

void MidiDriver_Accolade_Cms::controlChange(uint8 channel, uint8 controller, uint8 value) {
	// Only All Note Off is processed; it will turn off the active note on this
	// channel (if there is one).
	if (controller != MIDI_CONTROLLER_ALL_NOTES_OFF || _activeNotes[channel] == 0xFF)
		return;

	noteOff(channel, _activeNotes[channel]);
}

byte MidiDriver_Accolade_Cms::determineFrequencyEnableRegisterValue() {
	byte freqEnableRegValue = 0;
	for (int i = 0; i < 6; i++) {
		// If a note is active on a channel, set the frequency enable bit for
		// that channel.
		if (_activeNotes[i] != 0xFF)
			freqEnableRegValue |= 1 << i;
	}

	return freqEnableRegValue;
}

void MidiDriver_Accolade_Cms::writeRegister(uint16 reg, uint8 value) {
	//debug("Writing register %02X %02X", reg, value);

	_cms->writeReg(reg, value);
}

void MidiDriver_Accolade_Cms::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timer_param = timer_param;
	_timer_proc = timer_proc;
}

void MidiDriver_Accolade_Cms::onTimer() {
	if (_timer_proc && _timer_param)
		_timer_proc(_timer_param);
}

}
