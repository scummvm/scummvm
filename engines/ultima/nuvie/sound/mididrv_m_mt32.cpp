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

#include "mididrv_m_mt32.h"

namespace Ultima {
namespace Nuvie {

const uint8 MidiDriver_M_MT32::MIDI_NOTE_MAP[] = {
	0x00, 0x0C, 0x0E, 0x10, 0x11, 0x13, 0x15, 0x17,
	0x00, 0x0D, 0x0F, 0x11, 0x12, 0x14, 0x16, 0x18,
	0x00, 0x0B, 0x0D, 0x0F, 0x10, 0x12, 0x14, 0x16
};

MidiDriver_M_MT32::MidiDriver_M_MT32() : MidiDriver_MT32GM(MT_MT32) {
	Common::fill(_mInstrumentMidiChannels, _mInstrumentMidiChannels + sizeof(_mInstrumentMidiChannels), 1);
	Common::fill(_mInstrumentMapping, _mInstrumentMapping + sizeof(_mInstrumentMapping), 0);
}

MidiDriver_M_MT32::~MidiDriver_M_MT32() { }

int MidiDriver_M_MT32::open(MidiDriver *driver, bool nativeMT32) {
	int result = MidiDriver_MT32GM::open(driver, nativeMT32);
	if (result == 0)
		setInstrumentRemapping(_mInstrumentMapping);

	return result;
}

void MidiDriver_M_MT32::send(int8 source, uint32 b) {
	if (!_isOpen) {
		// During the opening of the driver, some MIDI commands are sent to
		// initialize the device. These are not M commands so they are sent
		// straight to the device.
		MidiDriver_MT32GM::send(source, b);
		return;
	}

	byte mCommand = b & 0xF0;
	if (mCommand >= 0x80) {
		// These commands are either handled by the parser (call, return,
		// set loop point, delay) or are not implemented for MT-32
		// (load instrument, fade). Not all of them have the channel in the
		// low nibble, so they are filtered out here.
		return;
	}
	byte dataChannel = b & 0x0F;
	byte data = (b >> 8) & 0xFF;

	MChannelData &mChannelData = _mChannelData[dataChannel];

	// Get the MIDI output channel assigned to this M data channel.
	int8 outputChannel = source < 0 ? dataChannel : mapSourceChannel(source, dataChannel);
	if (outputChannel < 0) {
		warning("MidiDriver_M_MT32::send - Could not map data channel %i to an output channel", dataChannel);
		return;
	}

	MidiChannelControlData &controlData = *_controlData[outputChannel];

	byte midiNote;
	byte mNote;
	// Convert M to MIDI events
	switch (mCommand) {
	case 0x00: // Note off
		mNote = data & 0x1F;
		assert(mNote < 24);
		midiNote = MIDI_NOTE_MAP[mNote] + ((data >> 5) * 12);
		noteOnOff(outputChannel, MIDI_COMMAND_NOTE_OFF, midiNote, mChannelData.velocity, source, controlData);
		mChannelData.activeNote = -1;
		break;
	case 0x10: // Note on
	case 0x20: // Set pitch
		// In the original driver, for Note on events, Note off is explicitly
		// called first to turn off the previous note. However, the Note off
		// event is not sent if there is no note active. For Set pitch,
		// Note off is not explicitly called; Note on is called directly.
		// However, Note on turns off any active notes first before sending the
		// Note on event. So despite the different code paths, these events
		// effectively do the same thing: turn off the currently active note on
		// the channel, if there is one, then play the new note on the next
		// tick.

		if (mChannelData.activeNote >= 0) {
			noteOnOff(outputChannel, MIDI_COMMAND_NOTE_OFF, mChannelData.activeNote, mChannelData.velocity, source, controlData);
			mChannelData.activeNote = -1;
		}

		mNote = data & 0x1F;
		assert(mNote < 24);
		midiNote = MIDI_NOTE_MAP[mNote] + ((data >> 5) * 12);
		// The new note is queued for playback on the next timer tick
		// (see onTimer).
		if (mChannelData.queuedNote >= 0) {
			warning("MidiDriver_M_MT32::send - Note on on channel %i while a note is already queued", dataChannel);
		}
		mChannelData.queuedNote = midiNote;

		break;
	case 0x30: // Set level
		// The OPL level is converted to a MIDI note velocity, which is used
		// for notes subsequently played on the M channel. The active note is
		// not affected.
		mChannelData.velocity = (0x3F - (data & 0x3F)) * 1.5;
		break;
	case 0x70: // Program change
		// When instrument assignments are set on the driver, each M instrument
		// is assigned to a fixed MIDI output channel. When a program change
		// event is encountered on an M channel, the MIDI output channel of
		// that M channel is changed to the MIDI channel assigned to the new M
		// instrument.
		int8 newOutputChannel;
		assert(data < 16);
		newOutputChannel = _mInstrumentMidiChannels[data];
		if (newOutputChannel < 0) {
			warning("MidiDriver_M_MT32::send - Received program change for unmapped instrument %i", data);
			break;
		}
		if (newOutputChannel != outputChannel && mChannelData.activeNote >= 0) {
			// Turn off the active note.
			noteOnOff(outputChannel, MIDI_COMMAND_NOTE_OFF, mChannelData.activeNote, mChannelData.velocity, source, controlData);
			mChannelData.activeNote = -1;
		}
		_channelMap[source][dataChannel] = newOutputChannel;
		// Because the assignment of instruments to output channels is fixed,
		// a program change for each channel could be sent once when setting
		// instrument assignments. However, the original driver sends a program
		// change every time the instrument on an M channel is changed.
		programChange(newOutputChannel, data, source, controlData);
		break;
	default:
		// Modulation, slide and vibrato are not implemented for MT-32.
		break;
	}
}

void MidiDriver_M_MT32::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	// Load instrument is ignored for MT-32; instruments are set using
	// setInstrumentAssignments.
}

void MidiDriver_M_MT32::setInstrumentAssignments(const MInstrumentAssignment *assignments) {
	// Each M instrument used in the played track (up to 16) should get a MIDI
	// output channel and a MIDI instrument assigned to it. The MIDI instrument
	// is set on the output channel and when an M data channel switches to the
	// corresponding M instrument, the data channel is mapped to that output
	// channel.
	for (int i = 0; i < 16; i++) {
		if (assignments[i].midiChannel < 0)
			break;

		_mInstrumentMidiChannels[i] = assignments[i].midiChannel;
		_mInstrumentMapping[i] = assignments[i].midiInstrument;
	}
}

void MidiDriver_M_MT32::stopAllNotes(bool stopSustainedNotes) {
	MidiDriver_MT32GM::stopAllNotes();

	// Clear active and queued notes.
	for (int i = 0; i < 9; i++) {
		_mChannelData[i].activeNote = -1;
		_mChannelData[i].queuedNote = -1;
	}
}

void MidiDriver_M_MT32::onTimer() {
	// Play the queued notes for each M channel.
	for (int i = 0; i < 9; i++) {
		if (_mChannelData[i].queuedNote >= 0) {
			int8 outputChannel = mapSourceChannel(0, i);
			if (outputChannel < 0) {
				warning("MidiDriver_M_MT32::onTimer - Could not map data channel %i to an output channel", i);
				continue;
			}
			MidiChannelControlData &controlData = *_controlData[outputChannel];
			noteOnOff(outputChannel, MIDI_COMMAND_NOTE_ON, _mChannelData[i].queuedNote, _mChannelData[i].velocity, 0, controlData);
			_mChannelData[i].activeNote = _mChannelData[i].queuedNote;
			_mChannelData[i].queuedNote = -1;
		}
	}

	MidiDriver_MT32GM::onTimer();
}

} // End of namespace Nuvie
} // End of namespace Ultima
