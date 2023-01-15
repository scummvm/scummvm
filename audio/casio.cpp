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

#include "audio/casio.h"

#include "common/config-manager.h"

MidiDriver_Casio::ActiveNote::ActiveNote() {
	clear();
}

void MidiDriver_Casio::ActiveNote::clear() {
	source = 0x7F;
	channel = 0xFF;
	note = 0xFF;
	sustained = false;
}

const int MidiDriver_Casio::CASIO_CHANNEL_POLYPHONY[] = { 6, 4, 2, 4 };

const uint8 MidiDriver_Casio::INSTRUMENT_REMAPPING_CT460_TO_MT540[] {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10, 0x0C, 0x07,
	0x0B, 0x1B, 0x11, 0x08, 0x09, 0x14, 0x0A, 0x15, 0x0D, 0x0E,
	0x0F, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1C, 0x1D, 0x12, 0x13
};

const uint8 MidiDriver_Casio::INSTRUMENT_REMAPPING_MT540_TO_CT460[] {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x09, 0x0D, 0x0E,
	0x10, 0x0A, 0x08, 0x12, 0x13, 0x14, 0x07, 0x0C, 0x1C, 0x1D,
	0x0F, 0x11, 0x15, 0x16, 0x17, 0x18, 0x19, 0x0B, 0x1A, 0x1B
};

const uint8 MidiDriver_Casio::RHYTHM_INSTRUMENT_MT540 = 0x08;
const uint8 MidiDriver_Casio::RHYTHM_INSTRUMENT_CT460 = 0x0D;
const uint8 MidiDriver_Casio::BASS_INSTRUMENT_MT540 = 0x12;
const uint8 MidiDriver_Casio::BASS_INSTRUMENT_CT460 = 0x1C;

MidiDriver_Casio::MidiDriver_Casio(MusicType midiType) : _midiType(midiType), _driver(nullptr),
		_deviceType(MT_CT460), _isOpen(false), _rhythmNoteRemapping(nullptr), _sendUntrackedNoteOff(true) {
	if (!(_midiType == MT_MT540 || _midiType == MT_CT460)) {
		error("MidiDriver_Casio - Unsupported music data type %i", _midiType);
	}
	Common::fill(_instruments, _instruments + ARRAYSIZE(_instruments), 0);
	Common::fill(_rhythmChannel, _rhythmChannel + ARRAYSIZE(_rhythmChannel), false);
	Common::fill(_sustain, _sustain + ARRAYSIZE(_sustain), false);
}

MidiDriver_Casio::~MidiDriver_Casio() {
	close();

	if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		delete _driver;
		_driver = nullptr;
	}
}

int MidiDriver_Casio::open() {
	assert(!_driver);

	// Detect the output device.
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_PREFER_GM);
	MusicType deviceMusicType = MidiDriver::getMusicType(dev);
	// System MIDI ports have type GM. This driver supports no other type.
	if (deviceMusicType != MT_GM)
		error("MidiDriver_Casio::open - Detected device has unsupported type %i", deviceMusicType);

	// Create the driver.
	MidiDriver *driver = MidiDriver::createMidi(dev);

	// Check the MIDI mode setting to determine if the device connected to the
	// system MIDI port is an MT-540 or a CT-460/CSM-1.
	int midiMode = ConfMan.getInt("midi_mode");
	MusicType deviceType;
	if (midiMode == 3) {
		deviceType = MT_MT540;
	} else if (midiMode == 4) {
		deviceType = MT_CT460;
	} else {
		error("MidiDriver_Casio::open - Unsupported MIDI mode %i", midiMode);
	}

	return open(driver, deviceType);
}

int MidiDriver_Casio::open(MidiDriver* driver, MusicType deviceType) {
	assert(!_driver);

	_driver = driver;
	_deviceType = deviceType;
	if (!(_deviceType == MT_MT540 || _deviceType == MT_CT460)) {
		error("MidiDriver_Casio::open - Unsupported device type %i", _deviceType);
	}

	if (!_driver)
		return 255;

	int result = _driver->open();
	if (result != MidiDriver::MERR_ALREADY_OPEN && result != 0) {
		return result;
	}

	// Initialize the device by setting the instrument to 0 on all channels.
	for (int i = 0; i < 4; i++) {
		programChange(i, 0, -1);
	}

	_timerRate = _driver->getBaseTempo();
	_driver->setTimerCallback(this, timerCallback);

	_isOpen = true;

	return 0;
}

void MidiDriver_Casio::close() {
	if (_driver && _isOpen) {
		stopAllNotes();

		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
		_isOpen = false;
	}
}

bool MidiDriver_Casio::isOpen() const {
	return _isOpen;
}

void MidiDriver_Casio::send(int8 source, uint32 b) {
	byte dataChannel = b & 0xf;
	int8 outputChannel = source < 0 ? dataChannel : mapSourceChannel(source, dataChannel);
	if (outputChannel < 0 || outputChannel >= 4)
		// Only process events for channels 0-3.
		return;

	processEvent(source, b, outputChannel);
}

void MidiDriver_Casio::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	assert(source < MAXIMUM_SOURCES);

	if (type == 0x2F && source >= 0) // End of Track
		deinitSource(source);

	_driver->metaEvent(type, data, length);
}

int8 MidiDriver_Casio::mapSourceChannel(uint8 source, uint8 dataChannel) {
	// TODO Multisource functionality has not been fully implemented. Current
	// implementation assumes 1 source with access to all channels.
	return dataChannel;
}

void MidiDriver_Casio::processEvent(int8 source, uint32 b, uint8 outputChannel) {
	assert(source < MAXIMUM_SOURCES);

	byte command = b & 0xF0;
	byte op1 = (b >> 8) & 0xFF;
	byte op2 = (b >> 16) & 0xFF;

	// The only commands supported by the Casio devices are note on, note off
	// and program change.
	switch (command) {
	case MIDI_COMMAND_NOTE_OFF:
		noteOff(outputChannel, command, op1, op2, source);
		break;
	case MIDI_COMMAND_NOTE_ON:
		noteOn(outputChannel, op1, op2, source);
		break;
	case MIDI_COMMAND_PROGRAM_CHANGE:
		programChange(outputChannel, op1, source);
		break;
	case MIDI_COMMAND_CONTROL_CHANGE:
		controlChange(outputChannel, op1, op2, source);
		break;
	default:
		warning("MidiDriver_Casio::processEvent - Received unsupported event %02x", command);
		break;
	}
}

void MidiDriver_Casio::noteOff(byte outputChannel, byte command, byte note, byte velocity, int8 source) {
	// Apply rhythm note mapping.
	int8 mappedNote = mapNote(outputChannel, note);
	if (mappedNote < 0)
		// Rhythm note with no Casio equivalent.
		return;

	_mutex.lock();

	// Remove this note from the active note registry.
	bool foundActiveNote = false;
	for (int i = 0; i < ARRAYSIZE(_activeNotes); i++) {
		if (_activeNotes[i].channel == outputChannel && _activeNotes[i].note == mappedNote &&
				_activeNotes[i].source == source && !_activeNotes[i].sustained) {
			foundActiveNote = true;
			if (outputChannel < 4 && _sustain[outputChannel]) {
				_activeNotes[i].sustained = true;
			} else {
				_activeNotes[i].clear();
			}
			break;
		}
	}

	_mutex.unlock();

	if (!foundActiveNote && !_sendUntrackedNoteOff)
		return;

	_driver->send(command | outputChannel, mappedNote, velocity);
}

void MidiDriver_Casio::noteOn(byte outputChannel, byte note, byte velocity, int8 source) {
	if (velocity == 0) {
		// Note on with velocity 0 is a note off.
		noteOff(outputChannel, MIDI_COMMAND_NOTE_ON, note, velocity, source);
		return;
	}

	// Apply rhythm note mapping.
	int8 mappedNote = mapNote(outputChannel, note);
	if (mappedNote < 0)
		// Rhythm note with no Casio equivalent.
		return;

	_mutex.lock();

	// Add this note to the active note registry.
	for (int i = 0; i < ARRAYSIZE(_activeNotes); i++) {
		if (_activeNotes[i].note == 0xFF) {
			_activeNotes[i].channel = outputChannel;
			_activeNotes[i].note = mappedNote;
			_activeNotes[i].source = source;
			break;
		}
	}

	_mutex.unlock();

	byte calculatedVelocity = calculateVelocity(source, velocity);

	_driver->send(MIDI_COMMAND_NOTE_ON | outputChannel, mappedNote, calculatedVelocity);
}

int8 MidiDriver_Casio::mapNote(byte outputChannel, byte note) {
	int8 mappedNote = note;

	if (_rhythmNoteRemapping && isRhythmChannel(outputChannel)) {
		mappedNote = _rhythmNoteRemapping[note];
		if (mappedNote == 0)
			mappedNote = -1;
	}

	return mappedNote;
}

byte MidiDriver_Casio::calculateVelocity(int8 source, byte velocity) {
	byte calculatedVelocity = velocity;
	// Apply volume settings to velocity.
	// The Casio devices do not apply note velocity, so the only volume control
	// possible is setting the velocity to 0 if one of the volume settings is 0.
	if (source >= 0) {
		// Scale to source volume.
		if (_sources[source].volume == 0)
			calculatedVelocity = 0;
	}
	if (_userVolumeScaling) {
		if (_userMute) {
			calculatedVelocity = 0;
		} else {
			// Scale to user volume.
			uint16 userVolume = _sources[source].type == SOURCE_TYPE_SFX ? _userSfxVolume : _userMusicVolume;
			if (userVolume == 0)
				calculatedVelocity = 0;
		}
	}

	return calculatedVelocity;
}

void MidiDriver_Casio::programChange(byte outputChannel, byte patchId, int8 source, bool applyRemapping) {
	if (outputChannel < 4)
		// Register the new instrument.
		_instruments[outputChannel] = patchId;

	// Apply instrument mapping.
	byte mappedInstrument = mapInstrument(patchId, applyRemapping);

	if (outputChannel < 4) {
		_rhythmChannel[outputChannel] =
			mappedInstrument == (_deviceType == MT_MT540 ? RHYTHM_INSTRUMENT_MT540 : RHYTHM_INSTRUMENT_CT460);
	}

	_driver->send(MIDI_COMMAND_PROGRAM_CHANGE | outputChannel | (mappedInstrument << 8));
}

byte MidiDriver_Casio::mapInstrument(byte program, bool applyRemapping) {
	byte mappedInstrument = program;

	if (applyRemapping && _instrumentRemapping)
		// Apply custom instrument mapping.
		mappedInstrument = _instrumentRemapping[program];

	// Apply MT-540 <> CT-460 instrument mapping if necessary.
	if (mappedInstrument < ARRAYSIZE(INSTRUMENT_REMAPPING_MT540_TO_CT460)) {
		if (_midiType == MT_MT540 && _deviceType == MT_CT460) {
			mappedInstrument = INSTRUMENT_REMAPPING_MT540_TO_CT460[mappedInstrument];
		} else if (_midiType == MT_CT460 && _deviceType == MT_MT540) {
			mappedInstrument = INSTRUMENT_REMAPPING_CT460_TO_MT540[mappedInstrument];
		}
	}

	return mappedInstrument;
}

void MidiDriver_Casio::controlChange(byte outputChannel, byte controllerNumber, byte controllerValue, int8 source) {
	if (outputChannel >= 4)
		return;

	if (controllerNumber == MIDI_CONTROLLER_SUSTAIN) {
		_sustain[outputChannel] = controllerValue >= 0x40;

		if (!_sustain[outputChannel]) {
			// Remove sustained notes if sustain is turned off.

			_mutex.lock();

			for (int i = 0; i < ARRAYSIZE(_activeNotes); i++) {
				if (_activeNotes[i].channel == outputChannel && _activeNotes[i].sustained) {
					_activeNotes[i].clear();
				}
			}

			_mutex.unlock();
		}

		_driver->send(MIDI_COMMAND_CONTROL_CHANGE | outputChannel | (controllerNumber << 8) | (controllerValue << 16));
	} else {
		// No other controllers are supported.
		//warning("MidiDriver_Casio::controlChange - Received event for unsupported controller %02x", controllerNumber);
	}
}

bool MidiDriver_Casio::isRhythmChannel(uint8 outputChannel) {
	if (outputChannel >= 4)
		return false;

	return _rhythmChannel[outputChannel];
}

void MidiDriver_Casio::stopAllNotes(bool stopSustainedNotes) {
	stopAllNotes(0xFF, 0xFF);
}

void MidiDriver_Casio::stopAllNotes(uint8 source, uint8 channel) {
	_mutex.lock();

	// Turn off sustain.
	for (int i = 0; i < 4; i++) {
		if (channel == 0xFF || i == channel) {
			controlChange(i, MIDI_CONTROLLER_SUSTAIN, 0, source > 127 ? -1 : source);
		}
	}

	// Send note off events for all notes in the active note registry for this
	// source and channel.
	for (int i = 0; i < ARRAYSIZE(_activeNotes); i++) {
		if (_activeNotes[i].note != 0xFF && (source == 0xFF || _activeNotes[i].source == source) &&
				(channel == 0xFF || _activeNotes[i].channel == channel)) {
			_driver->send(MIDI_COMMAND_NOTE_OFF | _activeNotes[i].channel, _activeNotes[i].note, 0);
			_activeNotes[i].clear();
		}
	}

	_mutex.unlock();
}

void MidiDriver_Casio::applySourceVolume(uint8 source) {
	// Because the Casio devices do not support the volume controller, source
	// volume is applied to note velocity and it cannot be applied directly.
}

MidiChannel *MidiDriver_Casio::allocateChannel() {
	// MidiChannel objects are not supported by this driver.
	return nullptr;
}

MidiChannel *MidiDriver_Casio::getPercussionChannel() {
	// MidiChannel objects are not supported by this driver.
	return nullptr;
}

uint32 MidiDriver_Casio::getBaseTempo() {
	if (_driver) {
		return _driver->getBaseTempo();
	}
	return 0;
}

void MidiDriver_Casio::timerCallback(void *data) {
	MidiDriver_Casio *driver = static_cast<MidiDriver_Casio *>(data);
	driver->onTimer();
}
