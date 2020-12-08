/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AUDIO_MIDI_H
#define AUDIO_MIDI_H

#include "audio/mt32gm.h"

#include "common/config-manager.h"
#include "common/debug.h"

// The initialization of the static const integral data members is done in the class definition,
// but we still need to provide a definition if they are odr-used.
const uint8 MidiDriver_MT32GM::MAXIMUM_SOURCES;
const uint16 MidiDriver_MT32GM::DEFAULT_SOURCE_NEUTRAL_VOLUME;
const uint8 MidiDriver_MT32GM::MT32_DEFAULT_CHANNEL_VOLUME;
const uint8 MidiDriver_MT32GM::GM_DEFAULT_CHANNEL_VOLUME;
const uint8 MidiDriver_MT32GM::MAXIMUM_MT32_ACTIVE_NOTES;
const uint8 MidiDriver_MT32GM::MAXIMUM_GM_ACTIVE_NOTES;
const uint16 MidiDriver_MT32GM::FADING_DELAY;

// These are the power-on default instruments of the Roland MT-32 family.
const byte MidiDriver_MT32GM::MT32_DEFAULT_INSTRUMENTS[8] = {
	0x44, 0x30, 0x5F, 0x4E, 0x29, 0x03, 0x6E, 0x7A
};

// These are the power-on default panning settings for channels 2-9 of the Roland MT-32 family.
// Internally, the MT-32 has 15 panning positions (0-E with 7 being center).
// This has been translated to the equivalent MIDI panning values (0-127).
// These are used for setting default panning on GM devices when using them with MT-32 data.
// Note that MT-32 panning is reversed compared to the MIDI specification. This is not reflected
// here; the driver is expected to flip these values based on the _midiDeviceReversePanning
// variable.
const byte MidiDriver_MT32GM::MT32_DEFAULT_PANNING[8] = {
	// 7,    8,    7,    8,    4,    A,    0,    E 
	0x40, 0x49,	0x40, 0x49, 0x25, 0x5B, 0x00, 0x7F
};

// This is the drum map for the Roland Sound Canvas SC-55 v1.xx. It had a fallback mechanism 
// to correct invalid drumkit selections. Some games rely on this mechanism to select the 
// correct Roland GS drumkit. Use this map to emulate this mechanism.
// E.g. correct invalid drumkit 50: GS_DRUMKIT_FALLBACK_MAP[50] == 48
const uint8 MidiDriver_MT32GM::GS_DRUMKIT_FALLBACK_MAP[128] = {
	 0,  0,  0,  0,  0,  0,  0,  0, // STANDARD
	 8,  8,  8,  8,  8,  8,  8,  8, // ROOM
	16, 16, 16, 16, 16, 16, 16, 16, // POWER
	24, 25, 24, 24, 24, 24, 24, 24, // ELECTRONIC; TR-808 (25)
	32, 32, 32, 32, 32, 32, 32, 32, // JAZZ
	40, 40, 40, 40, 40, 40, 40, 40, // BRUSH
	48, 48, 48, 48, 48, 48, 48, 48, // ORCHESTRA
	56, 56, 56, 56, 56, 56, 56, 56, // SFX
	 0,  0,  0,  0,  0,  0,  0,  0, // No drumkit defined (fall back to STANDARD)
	 0,  0,  0,  0,  0,  0,  0,  0, // No drumkit defined
	 0,  0,  0,  0,  0,  0,  0,  0, // No drumkit defined
	 0,  0,  0,  0,  0,  0,  0,  0, // No drumkit defined
	 0,  0,  0,  0,  0,  0,  0,  0, // No drumkit defined
	 0,  0,  0,  0,  0,  0,  0,  0, // No drumkit defined
	 0,  0,  0,  0,  0,  0,  0,  0, // No drumkit defined
	 0,  0,  0,  0,  0,  0,  0, 127 // No drumkit defined; CM-64/32L (127)
};

MidiDriver_MT32GM::MidiDriver_MT32GM(MusicType midiType) :
		_driver(0),
		_nativeMT32(false),
		_enableGS(false),
		_midiDataReversePanning(false),
		_midiDeviceReversePanning(false),
		_scaleGSPercussionVolumeToMT32(false),
		_userVolumeScaling(true),
		_userMusicVolume(192),
		_userSfxVolume(192),
		_userMute(false),
		_isOpen(false),
		_outputChannelMask(65535), // Channels 1-16
		_baseFreq(250),
		_timerRate(0),
		_fadeDelay(0),
		_sysExDelay(0),
		_timer_param(0),
		_timer_proc(0) {
	memset(_controlData, 0, sizeof(_controlData));

	switch (midiType) {
	case MT_MT32:
		_midiType = MT_MT32;
		break;
	case MT_GM:
	case MT_GS: // Treat GS same as GM
		_midiType = MT_GM;
		break;
	default:
		error("MidiDriver_MT32GM: Unsupported music type %i", midiType);
		break;
	}

	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		// Default source type: 0 = music, 1+ = SFX
		_sources[i].type = i == 0 ? SOURCE_TYPE_MUSIC : SOURCE_TYPE_SFX;
		// Default MIDI channel mapping: data channel == output channel
		for (int j = 0; j < MIDI_CHANNEL_COUNT; ++j) {
			_sources[i].channelMap[j] = j;
		}
	}

	// Default MT-32 <> GM instrument mappings.
	_mt32ToGMInstrumentMap = _mt32ToGm;
	_gmToMT32InstrumentMap = _gmToMt32;

	_maximumActiveNotes = _midiType == MT_MT32 ? MAXIMUM_MT32_ACTIVE_NOTES : MAXIMUM_GM_ACTIVE_NOTES;
	_activeNotes = new ActiveNote[_maximumActiveNotes];
	assert(_activeNotes);
}

MidiDriver_MT32GM::~MidiDriver_MT32GM() {
	if (_driver) {
		_driver->setTimerCallback(0, 0);
		_driver->close();
		delete _driver;
	}
	_driver = 0;

	if (_controlData) {
		for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
			delete _controlData[i];
		}
	}
	if (_activeNotes)
		delete[] _activeNotes;
}

int MidiDriver_MT32GM::open() {
	assert(!_driver);

	// Setup midi driver
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | (_midiType == MT_MT32 ? MDT_PREFER_MT32 : MDT_PREFER_GM));
	MusicType deviceMusicType = MidiDriver::getMusicType(dev);
	if (!(deviceMusicType == MT_MT32 || deviceMusicType == MT_GM || deviceMusicType == MT_GS))
		error("MidiDriver_MT32GM: detected music device uses unsupported music type %i", deviceMusicType);

	MidiDriver *driver = MidiDriver::createMidi(dev);
	bool nativeMT32 = deviceMusicType == MT_MT32 || ConfMan.getBool("native_mt32");

	return open(driver, nativeMT32);
}

int MidiDriver_MT32GM::open(MidiDriver *driver, bool nativeMT32) {
	assert(!_driver);

	_driver = driver;
	_nativeMT32 = nativeMT32;

	_enableGS = ConfMan.getBool("enable_gs");

	if (!_driver)
		return 255;

	if (_nativeMT32)
		_outputChannelMask = _midiType == MT_MT32 ? 1022 : 767; // Channels 2-10 / 1-8 and 10
	_driver->property(MidiDriver::PROP_CHANNEL_MASK, _outputChannelMask);

	int ret = _driver->open();
	if (ret != MidiDriver::MERR_ALREADY_OPEN && ret != 0)
		return ret;

	_timerRate = _driver->getBaseTempo();
	_driver->setTimerCallback(this, timerCallback);

	initControlData();
	initMidiDevice();
	syncSoundSettings();

	_isOpen = true;

	return 0;
}

void MidiDriver_MT32GM::initControlData() {
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		_controlData[i] = new MidiChannelControlData();
		_controlData[i]->volume = _controlData[i]->scaledVolume =
			(_nativeMT32 ? MT32_DEFAULT_CHANNEL_VOLUME : GM_DEFAULT_CHANNEL_VOLUME);
		if (_nativeMT32 && i >= 1 && i <= 8) {
			_controlData[i]->program = MT32_DEFAULT_INSTRUMENTS[i - 1];
			_controlData[i]->panPosition = MT32_DEFAULT_PANNING[i - 1];
		}
	}
}

void MidiDriver_MT32GM::initMidiDevice() {
	if (_nativeMT32) {
		initMT32(_midiType != MT_MT32);
	} else {
		initGM(_midiType == MT_MT32, _enableGS);
	}
}

void MidiDriver_MT32GM::initMT32(bool initForGM) {
	sendMT32Reset();

	if (initForGM) {
		// Set up MT-32 for GM data.
		// This is based on Roland's GM settings for MT-32.
		debug("Initializing MT-32 for General MIDI data");

		byte buffer[17];

		// Roland MT-32 SysEx for system area
		memcpy(&buffer[0], "\x41\x10\x16\x12\x10\x00", 6);

		// Set reverb parameters:
		// - Mode 2 (Plate)
		// - Time 3
		// - Level 4
		memcpy(&buffer[6], "\x01\x02\x03\x04\x66", 5);
		sysEx(buffer, 11);

		// Set partial reserve to match SC-55
		memcpy(&buffer[6], "\x04\x08\x04\x04\x03\x03\x03\x03\x02\x02\x4C", 11);
		sysEx(buffer, 17);

		// Use MIDI instrument channels 1-8 instead of 2-9
		memcpy(&buffer[6], "\x0D\x00\x01\x02\x03\x04\x05\x06\x07\x09\x3E", 11);
		sysEx(buffer, 17);

		// The MT-32 has reversed stereo panning compared to the MIDI spec.
		// GM does use panning as specified by the MIDI spec.
		_midiDeviceReversePanning = true;

		int i;

		// Set default GM panning (center on all channels)
		for (i = 0; i < 8; ++i) {
			send((0x40 << 16) | (MIDI_CONTROLLER_PANNING << 8) | (MIDI_COMMAND_CONTROL_CHANGE | i));
		}

		// Set default GM instruments (0 on all channels).
		// This is expected to be mapped to the MT-32 equivalent by the driver.
		for (i = 0; i < 8; ++i) {
			send((0 << 8) | (MIDI_COMMAND_PROGRAM_CHANGE | i));
		}

		// Set Pitch Bend Sensitivity to 2 semitones.
		for (i = 0; i < 8; ++i) {
			setPitchBendRange(i, 2);
		}
		setPitchBendRange(MIDI_RHYTHM_CHANNEL, 2);
	}
}

void MidiDriver_MT32GM::initGM(bool initForMT32, bool enableGS) {
	sendGMReset();

	if (initForMT32) {
		// Set up the GM device for MT-32 MIDI data.
		// Based on iMuse implementation (which is based on Roland's MT-32 settings for GS)
		debug("Initializing GM device for MT-32 MIDI data");

		// The MT-32 has reversed stereo panning compared to the MIDI spec.
		// GM does use panning as specified by the MIDI spec.
		_midiDeviceReversePanning = true;

		int i;

		// Set the default panning for the MT-32 instrument channels.
		for (i = 1; i < 9; ++i) {
			send((MT32_DEFAULT_PANNING[i - 1] << 16) | (MIDI_CONTROLLER_PANNING << 8) | (MIDI_COMMAND_CONTROL_CHANGE | i));
		}

		// Set Channels 1-16 Reverb to 64, which is the
		// equivalent of MT-32 default Reverb Level 5
		for (i = 0; i < 16; ++i)
			send((64 << 16) | (MIDI_CONTROLLER_REVERB << 8) | (MIDI_COMMAND_CONTROL_CHANGE | i));

		// Set Channels 1-16 Chorus to 0. The MT-32 has no chorus capability.
		// (This is probably the default for many GM devices with chorus anyway.)
		for (i = 0; i < 16; ++i)
			send((0 << 16) | (MIDI_CONTROLLER_CHORUS << 8) | (MIDI_COMMAND_CONTROL_CHANGE | i));

		// Set Channels 1-16 Pitch Bend Sensitivity to 12 semitones.
		for (i = 0; i < 16; ++i) {
			setPitchBendRange(i, 12);
		}

		if (enableGS) {
			// GS specific settings for MT-32 instrument mapping.
			debug("Additional initialization of GS device for MT-32 MIDI data");

			// Note: All Roland GS devices support CM-64/32L maps

			if (getPercussionChannel() != 0) {
				// Set Percussion Channel to SC-55 Map (CC#32, 01H), then
				// Switch Drum Map to CM-64/32L (MT-32 Compatible Drums)
				// Bank select MSB: bank 0
				getPercussionChannel()->controlChange(MIDI_CONTROLLER_BANK_SELECT_MSB, 0);
				// Bank select LSB: map 1 (SC-55)
				getPercussionChannel()->controlChange(MIDI_CONTROLLER_BANK_SELECT_LSB, 1);
			}
			// Patch change: 127 (CM-64/32L)
			send(127 << 8 | MIDI_COMMAND_PROGRAM_CHANGE | 9);

			// Set Channels 1-16 to SC-55 Map, then CM-64/32L Variation
			for (i = 0; i < 16; ++i) {
				if (getPercussionChannel() != 0 && i == getPercussionChannel()->getNumber())
					continue;
				// Bank select MSB: bank 127 (CM-64/32L)
				send((127 << 16) | (MIDI_CONTROLLER_BANK_SELECT_MSB << 8) | (MIDI_COMMAND_CONTROL_CHANGE | i));
				// Bank select LSB: map 1 (SC-55)
				send((1 << 16) | (MIDI_CONTROLLER_BANK_SELECT_LSB << 8) | (MIDI_COMMAND_CONTROL_CHANGE | i));
				// Patch change: 0 (causes bank select to take effect)
				send((0 << 16) | (0 << 8) | (MIDI_COMMAND_PROGRAM_CHANGE | i));
			}

			byte buffer[12];

			// Roland GS SysEx ID
			memcpy(&buffer[0], "\x41\x10\x42\x12", 4);

			// Set channels 1-16 Mod. LFO1 Pitch Depth to 4
			memcpy(&buffer[4], "\x40\x20\x04\x04\x18", 5);
			for (i = 0; i < 16; ++i) {
				buffer[5] = 0x20 + i;
				buffer[8] = 0x18 - i;
				sysEx(buffer, 9);
			}

			// In Roland's GS MT-32 emulation settings, percussion channel expression
			// is locked at 80. This corrects a difference in volume of the SC-55 MT-32
			// drum kit vs the drums of the MT-32. However, this approach has a problem:
			// the MT-32 supports expression on the percussion channel, so MIDI data
			// which uses this will play incorrectly. So instead, percussion channel
			// volume will be scaled by the driver by a factor 80/127.
			// Strangely, the regular GM drum kit does have a volume that matches the
			// MT-32 drums, so scaling is only necessary when using GS MT-32 emulation.
			_scaleGSPercussionVolumeToMT32 = true;

			// Change Reverb settings (as used by Roland):
			// - Character: 0
			// - Pre-LPF: 4
			// - Level: 35h
			// - Time: 6Ah
			memcpy(&buffer[4], "\x40\x01\x31\x00\x04\x35\x6A\x6B", 8);
			sysEx(buffer, 12);
		}

		// Set the default MT-32 patches. For non-GS devices these are expected to be
		// mapped to the GM equivalents by the driver.
		for (i = 1; i < 9; ++i) {
			send((MT32_DEFAULT_INSTRUMENTS[i - 1] << 8) | (MIDI_COMMAND_PROGRAM_CHANGE | i));
		}

		// Regarding Master Tune: 442 kHz was intended for the MT-32 family, but
		// apparently due to a firmware bug the master tune was actually 440 kHz for
		// all models (see MUNT source code for more details). So master tune is left
		// at 440 kHz for GM devices playing MT-32 MIDI data.
	}
}

void MidiDriver_MT32GM::close() {
	if (_driver) {
		_driver->close();
	}
}

uint32 MidiDriver_MT32GM::property(int prop, uint32 param) {
	switch (prop) {
	case PROP_USER_VOLUME_SCALING:
		if (param == 0xFFFF)
			return _userVolumeScaling ? 1 : 0;
		_userVolumeScaling = param > 0;
		break;
	case PROP_MIDI_DATA_REVERSE_PANNING:
		if (param == 0xFFFF)
			return _midiDataReversePanning ? 1 : 0;
		_midiDataReversePanning = param > 0;
		break;
	default:
		MidiDriver::property(prop, param);
		break;
	}
	return 0;
}

void MidiDriver_MT32GM::send(uint32 b) {
	send(-1, b);
}

void MidiDriver_MT32GM::send(int8 source, uint32 b) {
	byte dataChannel = b & 0xf;
	int8 outputChannel = source < 0 ? dataChannel : mapSourceChannel(source, dataChannel);

	MidiChannelControlData &controlData = *_controlData[outputChannel];

	processEvent(source, b, outputChannel, controlData);
}

// MIDI messages can be found at http://www.midi.org/techspecs/midimessages.php
void MidiDriver_MT32GM::processEvent(int8 source, uint32 b, uint8 outputChannel, MidiChannelControlData &controlData, bool channelLockedByOtherSource) {
	assert(source < MAXIMUM_SOURCES);

	byte command = b & 0xf0;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	if (command != MIDI_COMMAND_SYSTEM && controlData.source != source) {
		// A new source has sent an event on this channel.
		controlData.sourceVolumeApplied = false;
		controlData.source = source;
	}

	switch (command) {
	case MIDI_COMMAND_NOTE_OFF:
	case MIDI_COMMAND_NOTE_ON:
		if (!channelLockedByOtherSource)
			noteOnOff(outputChannel, command, op1, op2, source, controlData);
		break;
	case MIDI_COMMAND_PITCH_BEND:
		controlData.pitchWheel = ((uint16)op2 << 7) | (uint16)op1;
		// fall through
	case MIDI_COMMAND_POLYPHONIC_AFTERTOUCH: // Not supported by MT-32 or GM
	case MIDI_COMMAND_CHANNEL_AFTERTOUCH: // Not supported by MT-32
		if (!channelLockedByOtherSource)
			_driver->send(command | outputChannel, op1, op2);
		break;
	case MIDI_COMMAND_CONTROL_CHANGE:
		controlChange(outputChannel, op1, op2, source, controlData, channelLockedByOtherSource);
		break;
	case MIDI_COMMAND_PROGRAM_CHANGE:
		programChange(outputChannel, op1, source, controlData, channelLockedByOtherSource);
		break;
	case MIDI_COMMAND_SYSTEM:
		// The only supported system event is SysEx and that should be sent using the sysEx functions.
		warning("MidiDriver_MT32GM: send received system event (not processed): %x", b);
		break;
	default:
		warning("MidiDriver_MT32GM: Received unknown event %02x", command);
		break;
	}
}

void MidiDriver_MT32GM::noteOnOff(byte outputChannel, byte command, byte note, byte velocity, int8 source, MidiChannelControlData &controlData) {
	if (!isOutputChannelUsed(outputChannel))
		return;

	// Note On with velocity 0 is treated as Note Off
	bool addNote = command == MIDI_COMMAND_NOTE_ON && velocity != 0;
	if (addNote) {
		if (source >= 0 && !controlData.sourceVolumeApplied)
			// Source volume hasn't been applied yet. Do so now.
			controlChange(outputChannel, MIDI_CONTROLLER_VOLUME, controlData.volume, source, controlData);

		// Add the new note to the active note registration
		addActiveNote(outputChannel, note, source);
	} else {
		// Remove the note from the active note registration
		removeActiveNote(outputChannel, note, source);
	}

	_driver->send(command | outputChannel, note, velocity);
}

void MidiDriver_MT32GM::controlChange(byte outputChannel, byte controllerNumber, byte controllerValue, int8 source, MidiChannelControlData &controlData, bool channelLockedByOtherSource) {
	assert(source < MAXIMUM_SOURCES);

	// Standard MIDI controllers
	switch (controllerNumber) {
	case MIDI_CONTROLLER_BANK_SELECT_MSB:
		// Keep track of the current bank for each channel
		controlData.instrumentBank = controllerValue;
		break;
	case MIDI_CONTROLLER_MODULATION:
		controlData.modulation = controllerValue;
		break;
	case MIDI_CONTROLLER_VOLUME:
		controlData.volume = controllerValue;
		controlData.sourceVolumeApplied = true;
		if (source >= 0) {
			// Scale to source volume
			controllerValue = (controllerValue * _sources[source].volume) / _sources[source].neutralVolume;
		}
		if (_userVolumeScaling) {
			if (_userMute) {
				controllerValue = 0;
			} else {
				// Scale to user volume
				uint16 userVolume = _sources[source].type == SOURCE_TYPE_SFX ? _userSfxVolume : _userMusicVolume; // Treat SOURCE_TYPE_UNDEFINED as music
				controllerValue = (controllerValue * userVolume) >> 8;
			}
		}
		if (_scaleGSPercussionVolumeToMT32 && outputChannel == MIDI_RHYTHM_CHANNEL) {
			// Scale GS percussion channel volume to MT-32 level (80/127)
			controllerValue = (80 * controllerValue) >> 7;
		}
		// Source volume scaling might clip volume, so reduce to maximum
		controllerValue = MIN(controllerValue, (byte)0x7F);
		if (controlData.scaledVolume == controllerValue) {
			// Volume is already at this value, so no need to send it out
			// to the MIDI device.
			return;
		}
		controlData.scaledVolume = controllerValue;
		break;
	case MIDI_CONTROLLER_PANNING:
		if (_midiDeviceReversePanning != _midiDataReversePanning) {
			// Center panning is 0x40
			controllerValue = 0x80 - controllerValue;
			if (controllerValue > 0x7F)
				controllerValue = 0x7F;
		}
		break;
	case MIDI_CONTROLLER_EXPRESSION:
		controlData.expression = controllerValue;
		break;
	case MIDI_CONTROLLER_SUSTAIN:
		controlData.sustain = controllerValue >= 0x40;
		if (!channelLockedByOtherSource && !controlData.sustain) {
			removeActiveNotes(outputChannel, true);
		}
		break;
	case MIDI_CONTROLLER_RESET_ALL_CONTROLLERS:
		controlData.pitchWheel = MIDI_PITCH_BEND_DEFAULT;
		controlData.modulation = 0;
		controlData.expression = 0x7F;
		controlData.sustain = false;
		if (!channelLockedByOtherSource) {
			removeActiveNotes(outputChannel, true);
		}
		break;
	case MIDI_CONTROLLER_OMNI_ON:
	case MIDI_CONTROLLER_OMNI_OFF:
	case MIDI_CONTROLLER_MONO_ON:
	case MIDI_CONTROLLER_POLY_ON:
		// These act as an All Notes Off on MT-32, but also turn sustain off.
		// They are not part of GM, so should not be used in GM data.
		if (_midiType != MT_MT32) {
			warning("MidiDriver_MT32GM: unsupported GM controller %x", controllerNumber);
			return;
		}

		controlData.sustain = false;
		if (!channelLockedByOtherSource)
			removeActiveNotes(outputChannel, true);
		if (!_nativeMT32) {
			// MT-32 data on GM device.
			// These controllers might not be supported or have side effects
			// (changing omni or mono/poly mode). Send All Notes Off and
			// Sustain Off instead.
			if (!channelLockedByOtherSource) {
				controllerNumber = MIDI_CONTROLLER_ALL_NOTES_OFF;
				_driver->send(MIDI_COMMAND_CONTROL_CHANGE | outputChannel | (MIDI_CONTROLLER_SUSTAIN << 8) | (0 << 16));
			}
		}
		// fall through
	case MIDI_CONTROLLER_ALL_NOTES_OFF:
		if (!channelLockedByOtherSource)
			removeActiveNotes(outputChannel, false);
		break;
	default:
		break;
	}

	if (!channelLockedByOtherSource)
		_driver->send(MIDI_COMMAND_CONTROL_CHANGE | outputChannel | (controllerNumber << 8) | (controllerValue << 16));
}

bool MidiDriver_MT32GM::addActiveNote(uint8 outputChannel, uint8 note, int8 source) {
	Common::StackLock lock(_activeNotesMutex);

	for (int i = 0; i < _maximumActiveNotes; ++i) {
		ActiveNote &activeNote = _activeNotes[i];
		if (activeNote.channel == 0xFF) {
			// Add the new note.
			activeNote.source = source;
			activeNote.channel = outputChannel;
			activeNote.note = note;
			activeNote.sustain = false;
			return true;
		}
	}
	warning("MidiDriver_MT32GM: Could not add active note %x on channel %i", note, outputChannel);
	return false;
}

bool MidiDriver_MT32GM::removeActiveNote(uint8 outputChannel, uint8 note, int8 source) {
	Common::StackLock lock(_activeNotesMutex);

	for (int i = 0; i < _maximumActiveNotes; ++i) {
		ActiveNote &activeNote = _activeNotes[i];
		if (activeNote.channel == outputChannel && activeNote.source == source && activeNote.note == note) {
			if (_controlData[outputChannel]->sustain) {
				// Sustain is on, so the note should be turned off
				// when sustain is turned off.
				activeNote.sustain = true;
				return false;
			} else {
				// Turn off the existing note.
				activeNote.clear();
				return true;
			}
		}
	}
	//warning("MidiDriver_MT32GM: Could not find active note %x on channel %i when removing", note, outputChannel);
	return false;
}

void MidiDriver_MT32GM::removeActiveNotes(uint8 outputChannel, bool sustainedNotes) {
	Common::StackLock lock(_activeNotesMutex);

	// Remove sustained or non-sustained notes from the active notes registration
	for (int i = 0; i < _maximumActiveNotes; ++i) {
		if (_activeNotes[i].channel == outputChannel && _activeNotes[i].sustain == sustainedNotes) {
			_activeNotes[i].clear();
		}
	}
}

void MidiDriver_MT32GM::programChange(byte outputChannel, byte patchId, int8 source, MidiChannelControlData &controlData, bool channelLockedByOtherSource) {
	// remember patch id for the current MIDI-channel
	controlData.program = patchId;

	if (channelLockedByOtherSource)
		return;

	if (_midiType == MT_MT32) {
		if (outputChannel == MIDI_RHYTHM_CHANNEL)
			// Patch changes on the rhythm channel do nothing on an MT-32.
			// On GM/GS devices they might unintentionally change the drumkit.
			return;

		if (!_nativeMT32 && !_enableGS) {
			// GM device: map the patch to GM equivalent
			patchId = mapMT32InstrumentToGM(patchId);
		}
	} else {
		// GM/GS MIDI
		if (outputChannel == MIDI_RHYTHM_CHANNEL) {
			// Correct possible wrong GS drumkit number
			patchId = GS_DRUMKIT_FALLBACK_MAP[patchId];
		} else if (!_nativeMT32) {
			// Correct possible wrong bank / instrument variation
			byte correctedBank = correctInstrumentBank(controlData.instrumentBank, patchId);
			if (correctedBank != 0xFF) {
				// Send out a bank select for the corrected bank number
				controlChange(outputChannel, MIDI_CONTROLLER_BANK_SELECT_MSB, correctedBank, source, controlData);
				controlChange(outputChannel, MIDI_CONTROLLER_BANK_SELECT_LSB, 0, source, controlData);
			}
		} else {
			// GM on an MT-32: map the patch to the MT-32 equivalent
			patchId = mapGMInstrumentToMT32(patchId);
		}
	}

	// Finally send program change to MIDI device
	_driver->send(MIDI_COMMAND_PROGRAM_CHANGE | outputChannel | (patchId << 8));
}

byte MidiDriver_MT32GM::mapMT32InstrumentToGM(byte mt32Instrument) {
	return _mt32ToGMInstrumentMap[mt32Instrument];
}

byte MidiDriver_MT32GM::mapGMInstrumentToMT32(byte gmInstrument) {
	return _gmToMT32InstrumentMap[gmInstrument];
}

byte MidiDriver_MT32GM::correctInstrumentBank(byte instrumentBank, byte patchId) {
	if (instrumentBank == 0 || patchId >= 120 || instrumentBank >= 64)
		// Usually, no bank select has been sent and no correction is
		// necessary.
		// No correction is performed on banks 64-127 or on the SFX
		// instruments (120-127).
		return 0xFF;

	// Determine the intended bank. This emulates the behavior of the
	// Roland SC-55 v1.2x. Instruments have 2, 1 or 0 sub-capital tones.
	// Depending on the selected bank and the selected instrument, the
	// bank will "fall back" to a sub-capital tone or to the capital
	// tone (bank 0).
	byte correctedBank = 0xFF;

	switch (patchId) {
	case 25:  // Steel-String Guitar / 12-string Guitar / Mandolin
		// This instrument has 2 sub-capital tones. Bank selects 17-63 
		// are corrected to the second sub-capital tone at 16.
		if (instrumentBank >= 16) {
			correctedBank = 16;
			break;
		}
		// Corrections for values below 16 are handled below.

		// fall through
	case 4:   // Electric Piano 1 / Detuned Electric Piano 1
	case 5:   // Electric Piano 2 / Detuned Electric Piano 2
	case 6:   // Harpsichord / Coupled Harpsichord
	case 14:  // Tubular-bell / Church Bell
	case 16:  // Organ 1 / Detuned Organ 1
	case 17:  // Organ 2 / Detuned Organ 2
	case 19:  // Church Organ 1 / Church Organ 2
	case 21:  // Accordion Fr / Accordion It
	case 24:  // Nylon-string Guitar / Ukelele
	case 26:  // Jazz Guitar / Hawaiian Guitar
	case 27:  // Clean Guitar / Chorus Guitar
	case 28:  // Muted Guitar / Funk Guitar
	case 30:  // Distortion Guitar / Feedback Guitar
	case 31:  // Guitar Harmonics / Guitar Feedback
	case 38:  // Synth Bass 1 / Synth Bass 3
	case 39:  // Synth Bass 2 / Synth Bass 4
	case 48:  // Strings / Orchestra
	case 50:  // Synth Strings 1 / Synth Strings 3
	case 61:  // Brass 1 / Brass 2
	case 62:  // Synth Brass 1 / Synth Brass 3
	case 63:  // Synth Brass 2 / Synth Brass 4
	case 80:  // Square Wave / Sine Wave
	case 107: // Koto / Taisho Koto
	case 115: // Woodblock / Castanets
	case 116: // Taiko / Concert BD
	case 117: // Melodic Tom 1 / Melodic Tom 2
	case 118: // Synth Drum / 808 Tom
		// These instruments have one sub-capital tone. Bank selects 9-63
		// are corrected to the sub-capital tone at 8.
		if (instrumentBank >= 8) {
			correctedBank = 8;
			break;
		}
		// Corrections for values below 8 are handled below.

		// fall through
	default:
		// The other instruments only have a capital tone. Bank selects
		// 1-63 are corrected to the capital tone.
		correctedBank = 0;
		break;
	}

	// Return the corrected bank, or 0xFF if no correction is needed.
	return instrumentBank != correctedBank ? correctedBank : 0xFF;
}

void MidiDriver_MT32GM::sysEx(const byte *msg, uint16 length) {
	uint16 delay = sysExNoDelay(msg, length);

	if (delay > 0)
		g_system->delayMillis(delay);
}

uint16 MidiDriver_MT32GM::sysExNoDelay(const byte *msg, uint16 length) {
	if (!_nativeMT32 && length >= 3 && msg[0] == 0x41 && msg[2] == 0x16)
		// MT-32 SysExes have no effect on GM devices.
		return 0;

	// Send SysEx
	_driver->sysEx(msg, length);

	// Wait the time it takes to send the SysEx data
	uint16 delay = (length + 2) * 1000 / 3125;

	// Plus an additional delay for the MT-32 rev00
	if (_nativeMT32)
		delay += 40;

	return delay;
}

void MidiDriver_MT32GM::sysExQueue(const byte *msg, uint16 length) {
	SysExData sysEx;
	memcpy(sysEx.data, msg, length);
	sysEx.length = length;

	_sysExQueueMutex.lock();
	_sysExQueue.push(sysEx);
	_sysExQueueMutex.unlock();
}

uint16 MidiDriver_MT32GM::sysExMT32(const byte *msg, uint16 length, const uint32 targetAddress, bool queue, bool delay) {
	if (!_nativeMT32)
		// MT-32 SysExes have no effect on GM devices.
		return 0;

	byte   sysExMessage[270];
	uint16 sysExPos = 0;
	byte   sysExByte;
	uint16 sysExChecksum = 0;

	memset(&sysExMessage, 0, sizeof(sysExMessage));

	sysExMessage[0] = 0x41; // Roland
	sysExMessage[1] = 0x10;
	sysExMessage[2] = 0x16; // Model MT32
	sysExMessage[3] = 0x12; // Command DT1

	sysExChecksum = 0;

	sysExMessage[4] = (targetAddress >> 14) & 0x7F;
	sysExMessage[5] = (targetAddress >> 7) & 0x7F;
	sysExMessage[6] = targetAddress & 0x7F;

	for (byte targetAddressByte = 4; targetAddressByte < 7; targetAddressByte++) {
		assert(sysExMessage[targetAddressByte] < 0x80); // security check
		sysExChecksum -= sysExMessage[targetAddressByte];
	}

	sysExPos = 7;
	for (int i = 0; i < length; ++i) {
		sysExByte = *msg++;

		assert(sysExPos < sizeof(sysExMessage));
		assert(sysExByte < 0x80); // security check
		sysExMessage[sysExPos++] = sysExByte;
		sysExChecksum -= sysExByte;
	}

	// Calculate checksum
	assert(sysExPos < sizeof(sysExMessage));
	sysExMessage[sysExPos++] = sysExChecksum & 0x7F;

	if (queue) {
		sysExQueue(sysExMessage, sysExPos);
	} else if (!delay) {
		return sysExNoDelay(sysExMessage, sysExPos);
	} else {
		sysEx(sysExMessage, sysExPos);
	}

	return 0;
}

void MidiDriver_MT32GM::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	assert(source < MAXIMUM_SOURCES);

	if (type == 0x2F && source >= 0) // End of Track
		deinitSource(source);

	_driver->metaEvent(type, data, length);
}

void MidiDriver_MT32GM::stopAllNotes(bool stopSustainedNotes) {
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		if (stopSustainedNotes) {
			_driver->send(MIDI_COMMAND_CONTROL_CHANGE | i, MIDI_CONTROLLER_SUSTAIN, 0);
			_controlData[i]->sustain = false;
		}
		_driver->send(MIDI_COMMAND_CONTROL_CHANGE | i, MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
	}

	_activeNotesMutex.lock();

	for (int i = 0; i < _maximumActiveNotes; ++i) {
		_activeNotes[i].clear();
	}

	_activeNotesMutex.unlock();
}

void MidiDriver_MT32GM::startFade(uint16 duration, uint16 targetVolume) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		startFade(i, duration, targetVolume);
	}
}

void MidiDriver_MT32GM::startFade(uint8 source, uint16 duration, uint16 targetVolume) {
	assert(source < MAXIMUM_SOURCES);

	_fadingMutex.lock();

	_sources[source].fadePassedTime = 0;
	_sources[source].fadeStartVolume = _sources[source].volume;
	_sources[source].fadeEndVolume = targetVolume;
	_sources[source].fadeDuration = duration * 1000;

	_fadingMutex.unlock();
}

void MidiDriver_MT32GM::abortFade(FadeAbortType abortType) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		abortFade(i, abortType);
	}
}

void MidiDriver_MT32GM::abortFade(uint8 source, FadeAbortType abortType) {
	assert(source < MAXIMUM_SOURCES);

	if (!isFading(source)) {
		return;
	}

	_fadingMutex.lock();

	_sources[source].fadeDuration = 0;
	uint16 newSourceVolume;
	switch (abortType) {
	case FADE_ABORT_TYPE_END_VOLUME:
		newSourceVolume = _sources[source].fadeEndVolume;
		break;
	case FADE_ABORT_TYPE_START_VOLUME:
		newSourceVolume = _sources[source].fadeStartVolume;
		break;
	case FADE_ABORT_TYPE_CURRENT_VOLUME:
	default:
		_fadingMutex.unlock();
		return;
	}
	setSourceVolume(source, newSourceVolume);

	_fadingMutex.unlock();
}

bool MidiDriver_MT32GM::isFading() {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		if (isFading(i))
			return true;
	}
	return false;
}

bool MidiDriver_MT32GM::isFading(uint8 source) {
	assert(source < MAXIMUM_SOURCES);

	return _sources[source].fadeDuration > 0;
}

void MidiDriver_MT32GM::updateFading() {
	Common::StackLock lock(_fadingMutex);

	_fadeDelay -= _fadeDelay < _timerRate ? _fadeDelay : _timerRate;

	bool updatedVolume = false;
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {

		if (_sources[i].fadeDuration > 0) {
			_sources[i].fadePassedTime += _timerRate;

			if (_sources[i].fadePassedTime >= _sources[i].fadeDuration) {
				// Fade has finished
				setSourceVolume(i, _sources[i].fadeEndVolume);
				updatedVolume = true;
				_sources[i].fadeDuration = 0;
			} else if (_fadeDelay == 0) {
				setSourceVolume(i, ((_sources[i].fadePassedTime * (_sources[i].fadeEndVolume - _sources[i].fadeStartVolume)) /
					_sources[i].fadeDuration) + _sources[i].fadeStartVolume);
				updatedVolume = true;
			}
		}
	}

	if (updatedVolume)
		_fadeDelay = FADING_DELAY;
}

void MidiDriver_MT32GM::clearSysExQueue() {
	Common::StackLock lock(_sysExQueueMutex);

	_sysExQueue.clear();
}

MidiChannel *MidiDriver_MT32GM::allocateChannel() {
	if (_driver)
		return _driver->allocateChannel();
	return 0;
}

MidiChannel *MidiDriver_MT32GM::getPercussionChannel() {
	if (_driver)
		return _driver->getPercussionChannel();
	return 0;
}

bool MidiDriver_MT32GM::isOutputChannelUsed(int8 outputChannel) {
	return outputChannel >= 0 && outputChannel < MIDI_CHANNEL_COUNT &&
		_outputChannelMask & (1 << outputChannel);
}

uint32 MidiDriver_MT32GM::getBaseTempo() {
	if (_driver) {
		return _driver->getBaseTempo();
	}
	return 1000000 / _baseFreq;
}

bool MidiDriver_MT32GM::allocateSourceChannels(uint8 source, uint8 numChannels) {
	assert(source < MAXIMUM_SOURCES);

	deinitSource(source);

	_allocationMutex.lock();

	uint16 claimedChannels = 0;
	if (numChannels > 0) {
		for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
			if (!isOutputChannelUsed(i) || i == MIDI_RHYTHM_CHANNEL)
				continue;

			if (_controlData[i]->source == -1) {
				claimedChannels |= (1 << i);
				numChannels--;
			}
			if (numChannels == 0)
				break;
		}
	}

	if (numChannels > 0) {
		// Not enough channels available.
		_allocationMutex.unlock();
		return false;
	}

	// Allocate the channels.
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if ((claimedChannels >> i) & 1) {
			_controlData[i]->source = source;
		}
		// Clear the source channel mapping.
		if (i != MIDI_RHYTHM_CHANNEL)
			_sources[source].channelMap[i] = -1;
	}

	_allocationMutex.unlock();

	_sources[source].availableChannels = claimedChannels;

	return true;
}

int8 MidiDriver_MT32GM::mapSourceChannel(uint8 source, uint8 dataChannel) {
	int8 outputChannel = _sources[source].channelMap[dataChannel];
	if (outputChannel == -1) {
		for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
			if ((_sources[source].availableChannels >> i) & 1) {
				_sources[source].availableChannels &= ~(1 << i);
				_sources[source].channelMap[dataChannel] = i;
				outputChannel = i;
				break;
			}
		}
		if (outputChannel == -1) {
			warning("MidiDriver_MT32GM: Insufficient available channels for source %i", source);
		}
	}
	return outputChannel;
}

void MidiDriver_MT32GM::deinitSource(uint8 source) {
	assert(source < MAXIMUM_SOURCES);

	abortFade(source, FADE_ABORT_TYPE_END_VOLUME);

	// Free channels which were used by this source.
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		if (_controlData[i]->source == source)
			_controlData[i]->source = -1;
	}
	_sources[source].availableChannels = 0xFFFF;
	// Reset the data to output channel mapping
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		_sources[source].channelMap[i] = i;
	}

	_activeNotesMutex.lock();

	// Stop any active notes.
	for (int i = 0; i < _maximumActiveNotes; ++i) {
		if (_activeNotes[i].source == source) {
			if (_activeNotes[i].sustain) {
				// Turn off sustain
				controlChange(_activeNotes[i].channel, MIDI_CONTROLLER_SUSTAIN, 0x00, source, *_controlData[i]);
			} else {
				// Send note off
				noteOnOff(_activeNotes[i].channel, MIDI_COMMAND_NOTE_OFF, _activeNotes[i].note, 0x00, source, *_controlData[i]);
			}
		}
	}

	_activeNotesMutex.unlock();

	// TODO Optionally reset some controllers to their
	// default values? Pitch wheel, volume, sustain...
}

void MidiDriver_MT32GM::setSourceType(SourceType type) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		setSourceType(i, type);
	}
}

void MidiDriver_MT32GM::setSourceType(uint8 source, SourceType type) {
	assert(source < MAXIMUM_SOURCES);

	_sources[source].type = type;

	// Make sure music/sfx volume gets applied
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		if (_controlData[i]->source == source)
			controlChange(i, MIDI_CONTROLLER_VOLUME, _controlData[i]->volume, source, *_controlData[i]);
	}
}

void MidiDriver_MT32GM::setSourceVolume(uint16 volume) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		setSourceVolume(i, volume);
	}
}

void MidiDriver_MT32GM::setSourceVolume(uint8 source, uint16 volume) {
	assert(source < MAXIMUM_SOURCES);

	_sources[source].volume = volume;

	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		if (_controlData[i]->source == source)
			controlChange(i, MIDI_CONTROLLER_VOLUME, _controlData[i]->volume, source, *_controlData[i]);
	}
}

void MidiDriver_MT32GM::setSourceNeutralVolume(uint16 volume) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		setSourceNeutralVolume(i, volume);
	}
}

void MidiDriver_MT32GM::setSourceNeutralVolume(uint8 source, uint16 volume) {
	assert(source < MAXIMUM_SOURCES);

	_sources[source].neutralVolume = volume;
}

void MidiDriver_MT32GM::syncSoundSettings() {
	_userMusicVolume = MIN(256, ConfMan.getInt("music_volume"));
	_userSfxVolume = MIN(256, ConfMan.getInt("sfx_volume"));
	_userMute = ConfMan.getBool("mute");

	// Make sure music/sfx volume gets applied
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		controlChange(i, MIDI_CONTROLLER_VOLUME, _controlData[i]->volume, _controlData[i]->source, *_controlData[i]);
	}
}

void MidiDriver_MT32GM::onTimer() {
	updateFading();

	_sysExQueueMutex.lock();

	_sysExDelay -= (_sysExDelay > _timerRate) ? _timerRate : _sysExDelay;

	if (!_sysExQueue.empty() && _sysExDelay == 0) {
		// Ready to send next SysEx message to the MIDI device
		SysExData sysEx = _sysExQueue.pop();
		_sysExDelay = sysExNoDelay(sysEx.data, sysEx.length) * 1000;
	}

	_sysExQueueMutex.unlock();
}

#endif
