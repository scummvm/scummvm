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

#include "audio/miles.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/mutex.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Audio {

// Miles Audio MT-32 / General MIDI driver
//

#define MILES_MT32_TIMBREBANK_STANDARD_ROLAND 0
#define MILES_MT32_TIMBREBANK_MELODIC_MODULE 127

#define MILES_MT32_SYSEX_TERMINATOR 0xFF

/*
const byte milesMT32SysExResetParameters[] = {
	0x01, MILES_MT32_SYSEX_TERMINATOR
};
*/

const byte milesMT32SysExChansSetup[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, MILES_MT32_SYSEX_TERMINATOR
};

const byte milesMT32SysExPartialReserveTable[] = {
	0x03, 0x04, 0x03, 0x04, 0x03, 0x04, 0x03, 0x04, 0x04, MILES_MT32_SYSEX_TERMINATOR
};

const byte milesMT32SysExInitReverb[] = {
	0x00, 0x03, 0x02, MILES_MT32_SYSEX_TERMINATOR // Reverb mode 0, reverb time 3, reverb level 2
};

MidiDriver_Miles_Midi::MidiDriver_Miles_Midi(MusicType midiType, MilesMT32InstrumentEntry *instrumentTablePtr, uint16 instrumentTableCount) :
		_driver(NULL),
		_isOpen(false),
		_nativeMT32(false),
		_enableGS(false),
		_outputChannelMask(65535), // Channels 1-16
		_baseFreq(250),
		_timerRate(0),
		_noteCounter(0),
		_sysExDelay(0),
		_timer_param(0),
		_timer_proc(0) {
	switch (midiType) {
	case MT_MT32:
		_midiType = MT_MT32;
		break;
	case MT_GM:
	case MT_GS: // Treat GS same as GM
		_midiType = MT_GM;
		break;
	default:
		assert(false);
		break;
	}

	memset(_gsBank, 0, sizeof(_gsBank));
	memset(_patchesBank, 0, sizeof(_patchesBank));

	_instrumentTablePtr = instrumentTablePtr;
	_instrumentTableCount = instrumentTableCount;

	for (int i = 0; i < MILES_MAXIMUM_SOURCES; ++i) {
		// Default MIDI channel mapping: data channel == output channel
		for (int j = 0; j < MILES_MIDI_CHANNEL_COUNT; ++j) {
			_sources[i].channelMap[j] = j;
		}
	}

	_maximumActiveNotes = _midiType == MT_MT32 ? MILES_MT32_ACTIVE_NOTES : MILES_GM_ACTIVE_NOTES;
	_activeNotes = new ActiveNote[_maximumActiveNotes];
	assert(_activeNotes);
}

MidiDriver_Miles_Midi::~MidiDriver_Miles_Midi() {
	Common::StackLock lock(_mutex);
	if (_driver) {
		_driver->setTimerCallback(0, 0);
		_driver->close();
		delete _driver;
	}
	_driver = NULL;

	if (_activeNotes)
		delete[] _activeNotes;
}

int MidiDriver_Miles_Midi::open() {
	assert(!_driver);

	// Setup midi driver
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | (_midiType == MT_MT32 ? MDT_PREFER_MT32 : MDT_PREFER_GM));
	MusicType deviceMusicType = MidiDriver::getMusicType(dev);
	if (!(deviceMusicType == MT_MT32 || deviceMusicType == MT_GM || deviceMusicType == MT_GS))
		error("MILES-MIDI: detected music device uses unsupported music type %i", deviceMusicType);

	MidiDriver *driver = MidiDriver::createMidi(dev);
	bool nativeMT32 = deviceMusicType == MT_MT32 || ConfMan.getBool("native_mt32");

	return open(driver, nativeMT32);
}

int MidiDriver_Miles_Midi::open(MidiDriver *driver, bool nativeMT32) {
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

	initMidiDevice();

	return 0;
}

void MidiDriver_Miles_Midi::close() {
	if (_driver) {
		_driver->close();
	}
}

void MidiDriver_Miles_Midi::initMidiDevice() {
	if (_nativeMT32) {
		bool initForGM = _midiType != MT_MT32;

		// reset all internal parameters / patches
		initMT32(initForGM);

		if (!initForGM) {
			// init part/channel assignments
			MT32SysEx(0x10000D, milesMT32SysExChansSetup);

			// partial reserve table
			MT32SysEx(0x100004, milesMT32SysExPartialReserveTable);

			// init reverb
			MT32SysEx(0x100001, milesMT32SysExInitReverb);
		}
	} else {
		initGM(_midiType == MT_MT32, _enableGS);
	}

	// Set Miles default controller values
	// Note that AIL/MSS apparently did not get full support for GM until
	// version 3.00 in 09/1994. Many games used the MT-32 driver to
	// implement GM support. As a result, default parameters were only sent
	// out on the MT-32 channels (2-10). Also, the default MT-32 instrument
	// numbers were set on GM devices, even though they map to different
	// instruments. This is reproduced here to prevent possible issues with
	// games that depend on this behavior.

	for (int i = 1; i < 10; ++i) {
		// Volume 7F (max)
		send(-1, 0xB0 | i, MILES_CONTROLLER_VOLUME, 0x7F);
		if (_midiType == MT_MT32) {
			// Panning center - not the MT-32 default for all channels
			send(-1, 0xB0 | i, MILES_CONTROLLER_PANNING, 0x40);
		}
		// Patch
		if (i != MILES_RHYTHM_CHANNEL) {
			if (_midiType == MT_MT32) {
				// These are the default on the MT-32; just set them on the control data
				_midiChannels[i].currentData.program = _mt32DefaultInstruments[i - 1];
			} else {
				// Send the instruments out to GM devices.
				send(-1, 0xC0 | i, _mt32DefaultInstruments[i - 1], 0);
			}
		}
		// The following settings are also sent out by the AIL driver:
		// - Modulation 0
		// - Expression 7F (max)
		// - Sustain off
		// - Pitch bend neutral
		// These are the default MT-32 and GM settings, so it is not
		// necessary to send these.
	}
}

void MidiDriver_Miles_Midi::sysEx(const byte *msg, uint16 length) {
	uint16 delay = sysExNoDelay(msg, length);

	if (delay > 0)
		g_system->delayMillis(delay);
}

uint16 MidiDriver_Miles_Midi::sysExNoDelay(const byte *msg, uint16 length) {
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

void MidiDriver_Miles_Midi::MT32SysEx(const uint32 targetAddress, const byte *dataPtr, bool useSysExQueue) {
	if (!_nativeMT32)
		// MT-32 SysExes have no effect on GM devices.
		return;

	byte   sysExMessage[270];
	uint16 sysExPos      = 0;
	byte   sysExByte;
	uint16 sysExChecksum = 0;

	memset(&sysExMessage, 0, sizeof(sysExMessage));

	sysExMessage[0] = 0x41; // Roland
	sysExMessage[1] = 0x10;
	sysExMessage[2] = 0x16; // Model MT32
	sysExMessage[3] = 0x12; // Command DT1

	sysExChecksum = 0;

	sysExMessage[4] = (targetAddress >> 16) & 0xFF;
	sysExMessage[5] = (targetAddress >> 8) & 0xFF;
	sysExMessage[6] = targetAddress & 0xFF;

	for (byte targetAddressByte = 4; targetAddressByte < 7; targetAddressByte++) {
		assert(sysExMessage[targetAddressByte] < 0x80); // security check
		sysExChecksum -= sysExMessage[targetAddressByte];
	}

	sysExPos = 7;
	while (1) {
		sysExByte = *dataPtr++;
		if (sysExByte == MILES_MT32_SYSEX_TERMINATOR)
			break; // Message done

		assert(sysExPos < sizeof(sysExMessage));
		assert(sysExByte < 0x80); // security check
		sysExMessage[sysExPos++] = sysExByte;
		sysExChecksum -= sysExByte;
	}

	// Calculate checksum
	assert(sysExPos < sizeof(sysExMessage));
	sysExMessage[sysExPos++] = sysExChecksum & 0x7f;

	if (useSysExQueue) {
		SysExData sysEx;
		memcpy(sysEx.data, sysExMessage, sysExPos);
		sysEx.length = sysExPos;

		_sysExQueueMutex.lock();
		_sysExQueue.push(sysEx);
		_sysExQueueMutex.unlock();
	} else {
		sysEx(sysExMessage, sysExPos);
	}
}

void MidiDriver_Miles_Midi::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	assert(source < MILES_MAXIMUM_SOURCES);

	if (type == 0x2F && source >= 0) // End of Track
		deinitSource(source);

	_driver->metaEvent(type, data, length);
}

void MidiDriver_Miles_Midi::send(uint32 b) {
	send(-1, b);
}

	// MIDI messages can be found at http://www.midi.org/techspecs/midimessages.php
void MidiDriver_Miles_Midi::send(int8 source, uint32 b) {
	assert(source < MILES_MAXIMUM_SOURCES);

	byte command = b & 0xf0;
	byte dataChannel = b & 0xf;
	byte outputChannel = source < 0 ? dataChannel : _sources[source].channelMap[dataChannel];
	MidiChannelEntry &outputChannelEntry = _midiChannels[outputChannel];
	// Only send the message to the MIDI device if the channel is not locked or
	// if the source that locked the channel is sending the message
	bool sendMessage = source < 0 || !outputChannelEntry.locked ||
		(outputChannelEntry.locked && outputChannelEntry.currentData.source == source);
	// Track controller changes on the current data if the MIDI message is sent out,
	// or on the unlock data otherwise.
	MidiChannelControlData &controlData = sendMessage ? outputChannelEntry.currentData : outputChannelEntry.unlockData;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	if (command != 0xF0 && controlData.source != source) {
		// A new source has sent an event on this channel.
		controlData.sourceVolumeApplied = false;
		controlData.source = source;
	}

	switch (command) {
	case 0x80: // Note Off
	case 0x90: // Note On
		if (sendMessage) {
			// Note On with velocity 0 is treated as Note Off
			bool addNote = command == 0x90 && op2 != 0;
			if (addNote) {
				if (source >= 0 && !controlData.sourceVolumeApplied)
					// Source volume hasn't been applied yet. Do so now.
					controlChange(outputChannel, MILES_CONTROLLER_VOLUME, controlData.volume, source, controlData, sendMessage);
				// Add the new note to the active note registration
				for (int i = 0; i < _maximumActiveNotes; ++i) {
					ActiveNote &activeNote = _activeNotes[i];
					if (activeNote.channel == 0xFF) {
						// Add the new note.
						activeNote.source = source;
						activeNote.channel = outputChannel;
						activeNote.note = op1;
						activeNote.sustain = false;
						++outputChannelEntry.activeNotes;
						break;
					}
				}
			} else {
				// Remove the note from the active note registration
				for (int i = 0; i < _maximumActiveNotes; ++i) {
					ActiveNote &activeNote = _activeNotes[i];
					if (activeNote.channel == outputChannel && activeNote.source == source && activeNote.note == op1) {
						if (controlData.sustain) {
							// Sustain is on, so the note should be turned off
							// when sustain is turned off.
							activeNote.sustain = true;
						} else {
							// Turn off the existing note.
							activeNote.source = 0x7F;
							activeNote.channel = 0xFF;
							if (outputChannelEntry.activeNotes == 0) {
								warning("MILES-MIDI: active notes 0 on channel %d when turning off note %x", op1, outputChannel);
							} else {
								--outputChannelEntry.activeNotes;
							}
						}
						break;
					}
				}
			}
		}
		// fall through
	case 0xa0: // Polyphonic key pressure (aftertouch) (not supported by MT-32 or GM)
	case 0xd0: // Channel pressure (aftertouch) (not supported by MT-32)
	case 0xe0: // pitch bend change
		if (command == 0xe0)
			controlData.pitchWheel = ((uint16)op2 << 7) | (uint16)op1;

		_noteCounter++;
		if (controlData.usingCustomTimbre) {
			// Remember that this timbre got used now
			_customTimbres[controlData.currentCustomTimbreId].lastUsedNoteCounter = _noteCounter;
		}
		if (sendMessage) {
			_driver->send(command | outputChannel, op1, op2);
		}
		break;
	case 0xb0: // Control change
		controlChange(outputChannel, op1, op2, source, controlData, sendMessage);
		break;
	case 0xc0: // Program Change
		programChange(outputChannel, op1, source, controlData, sendMessage);
		break;
	case 0xf0: // SysEx
		warning("MILES-MIDI: SysEx: %x", b);
		break;
	default:
		warning("MILES-MIDI: Unknown event %02x", command);
	}
}

void MidiDriver_Miles_Midi::controlChange(byte outputChannel, byte controllerNumber, byte controllerValue, int8 source, MidiChannelControlData &controlData, bool sendMessage) {
	assert(source < MILES_MAXIMUM_SOURCES);

	// XMIDI controllers
	switch (controllerNumber) {
	case MILES_CONTROLLER_SELECT_PATCH_BANK:
		controlData.currentPatchBank = controllerValue;
		return;

	case MILES_CONTROLLER_PROTECT_TIMBRE:
		if (controlData.usingCustomTimbre) {
			// custom timbre set on current channel
			_customTimbres[controlData.currentCustomTimbreId].protectionEnabled = controllerValue >= 64;
		}
		return;

	case MILES_CONTROLLER_LOCK_CHANNEL:
		if (source >= 0) {
			if (controllerValue >= 0x40) {
				lockChannel(source, outputChannel);
			} else {
				unlockChannel(outputChannel);
			}
		}
		return;

	case MILES_CONTROLLER_PROTECT_CHANNEL:
		if (source >= 0 && !_midiChannels[outputChannel].locked) {
			_midiChannels[outputChannel].lockProtected = controllerValue >= 0x40;
			_midiChannels[outputChannel].protectedSource = controllerValue >= 0x40 ? source : -1;
		}
		return;

	default:
		break;
	}

	// XMIDI MT-32 specific controllers
	if (_midiType == MT_MT32 && _nativeMT32) {
		switch (controllerNumber) {
		case MILES_CONTROLLER_PATCH_REVERB:
			writePatchByte(controlData.program, 6, controllerValue);
			if (sendMessage)
				_driver->send(0xC0 | outputChannel | (controlData.program << 8)); // execute program change
			return;

		case MILES_CONTROLLER_PATCH_BENDER:
			writePatchByte(controlData.program, 4, controllerValue);
			if (sendMessage)
				_driver->send(0xC0 | outputChannel | (controlData.program << 8)); // execute program change
			return;

		case MILES_CONTROLLER_REVERB_MODE:
			writeToSystemArea(1, controllerValue);
			return;

		case MILES_CONTROLLER_REVERB_TIME:
			writeToSystemArea(2, controllerValue);
			return;

		case MILES_CONTROLLER_REVERB_LEVEL:
			writeToSystemArea(3, controllerValue);
			return;

		case MILES_CONTROLLER_RHYTHM_KEY_TIMBRE:
			if (controlData.usingCustomTimbre) {
				// custom timbre is set on current channel
				writeRhythmSetup(controllerValue, controlData.currentCustomTimbreId);
			}
			return;
		default:
			break;
		}
	}

	// XMIDI MT-32 SysEx controllers
	if (_midiType == MT_MT32 && (controllerNumber >= MILES_CONTROLLER_SYSEX_RANGE_BEGIN) && (controllerNumber <= MILES_CONTROLLER_SYSEX_RANGE_END)) {
		if (!_nativeMT32)
			return;

		// send SysEx
		byte sysExQueueNr = 0;

		// figure out which queue is accessed
		controllerNumber -= MILES_CONTROLLER_SYSEX_RANGE_BEGIN;
		while (controllerNumber > MILES_CONTROLLER_SYSEX_COMMAND_FINAL_DATA) {
			sysExQueueNr++;
			controllerNumber -= (MILES_CONTROLLER_SYSEX_COMMAND_FINAL_DATA + 1);
		}
		assert(sysExQueueNr < MILES_CONTROLLER_SYSEX_QUEUE_COUNT);

		byte sysExPos = _milesSysExQueues[sysExQueueNr].dataPos;
		bool sysExSend = false;

		switch(controllerNumber) {
		case MILES_CONTROLLER_SYSEX_COMMAND_ADDRESS1:
			_milesSysExQueues[sysExQueueNr].targetAddress &= 0x00FFFF;
			_milesSysExQueues[sysExQueueNr].targetAddress |= (controllerValue << 16);
			break;
		case MILES_CONTROLLER_SYSEX_COMMAND_ADDRESS2:
			_milesSysExQueues[sysExQueueNr].targetAddress &= 0xFF00FF;
			_milesSysExQueues[sysExQueueNr].targetAddress |= (controllerValue << 8);
			break;
		case MILES_CONTROLLER_SYSEX_COMMAND_ADDRESS3:
			_milesSysExQueues[sysExQueueNr].targetAddress &= 0xFFFF00;
			_milesSysExQueues[sysExQueueNr].targetAddress |= controllerValue;
			break;
		case MILES_CONTROLLER_SYSEX_COMMAND_DATA:
			if (sysExPos < MILES_CONTROLLER_SYSEX_QUEUE_SIZE) {
				// Space left? put current byte into queue
				_milesSysExQueues[sysExQueueNr].data[sysExPos] = controllerValue;
				sysExPos++;
				_milesSysExQueues[sysExQueueNr].dataPos = sysExPos;
				if (sysExPos >= MILES_CONTROLLER_SYSEX_QUEUE_SIZE) {
					// overflow? -> send it now
					sysExSend = true;
				}
			}
			break;
		case MILES_CONTROLLER_SYSEX_COMMAND_FINAL_DATA:
			if (sysExPos < MILES_CONTROLLER_SYSEX_QUEUE_SIZE) {
				// Space left? put current byte into queue
				_milesSysExQueues[sysExQueueNr].data[sysExPos] = controllerValue;
				sysExPos++;
				// Do not increment dataPos. Subsequent Final Data commands will
				// re-send the last address byte with the new controller value.
				sysExSend = true;
			}
			break;
		default:
			assert(0);
		}

		if (sysExSend) {
			if (sysExPos > 0) {
				// data actually available? -> send it
				_milesSysExQueues[sysExQueueNr].data[sysExPos] = MILES_MT32_SYSEX_TERMINATOR; // put terminator

				// Execute SysEx
				MT32SysEx(_milesSysExQueues[sysExQueueNr].targetAddress, _milesSysExQueues[sysExQueueNr].data);

				// Adjust target address to point at the final data byte, or at the
				// end of the current data in case of an overflow
				// Note that the address bytes are actually 7 bits
				byte addressByte1 = (_milesSysExQueues[sysExQueueNr].targetAddress & 0xFF0000) >> 16;
				byte addressByte2 = (_milesSysExQueues[sysExQueueNr].targetAddress & 0x00FF00) >> 8;
				byte addressByte3 = _milesSysExQueues[sysExQueueNr].targetAddress & 0x0000FF;
				addressByte3 += _milesSysExQueues[sysExQueueNr].dataPos;
				if (addressByte3 > 0x7F) {
					addressByte3 -= 0x80;
					addressByte2++;
				}
				if (addressByte2 > 0x7F) {
					addressByte2 -= 0x80;
					addressByte1++;
				}
				_milesSysExQueues[sysExQueueNr].targetAddress = addressByte1 << 16 | addressByte2 << 8 | addressByte3;

				// reset queue data buffer
				_milesSysExQueues[sysExQueueNr].dataPos = 0;
			}
		}
		return;
	}

	if ((controllerNumber >= MILES_CONTROLLER_XMIDI_RANGE_BEGIN) && (controllerNumber <= MILES_CONTROLLER_XMIDI_RANGE_END)) {
		// XMIDI controllers? Don't send these to the MIDI device
		return;
	}

	// Standard MIDI controllers
	switch (controllerNumber) {
	case MILES_CONTROLLER_BANK_SELECT_MSB:
		// Keep track of the current bank for each channel
		_gsBank[outputChannel] = controllerValue;
		break;
	case MILES_CONTROLLER_MODULATION:
		controlData.modulation = controllerValue;
		break;
	case MILES_CONTROLLER_VOLUME:
		controlData.volume = controllerValue;
		controlData.sourceVolumeApplied = true;
		if (source >= 0) {
			// Scale to source volume
			controllerValue = (_sources[source].volume * controllerValue) >> 8;
		}
		if (_scaleGSPercussionVolumeToMT32 && outputChannel == MILES_RHYTHM_CHANNEL) {
			// Scale GS percussion channel volume to MT-32 level (80/127)
			controllerValue = (80 * controllerValue) >> 7;
		}
		if (controlData.scaledVolume == controllerValue) {
			// Volume is already at this value, so no need to send it out
			// to the MIDI device.
			return;
		}
		controlData.scaledVolume = controllerValue;
		break;
	case MILES_CONTROLLER_PANNING:
		if (_reversePanning) {
			// Center panning is 0x40
			controllerValue = 0x80 - controllerValue;
			if (controllerValue > 0x7F)
				controllerValue = 0x7F;
		}
		controlData.panPosition = controllerValue;
		break;
	case MILES_CONTROLLER_EXPRESSION:
		controlData.expression = controllerValue;
		break;
	case MILES_CONTROLLER_RESET_ALL:
		controlData.modulation = 0;
		controlData.expression = 0x7F;
		controlData.pitchWheel = MILES_PITCHBENDER_DEFAULT;
		controlData.sustain = false;
		if (sendMessage) {
			removeActiveNotes(outputChannel, true);
		}
		break;
	case MILES_CONTROLLER_SUSTAIN:
		controlData.sustain = controllerValue >= 0x40;
		if (sendMessage && !controlData.sustain) {
			removeActiveNotes(outputChannel, true);
		}
		break;
	case MILES_CONTROLLER_OMNI_ON:
	case MILES_CONTROLLER_OMNI_OFF:
	case MILES_CONTROLLER_MONO_ON:
	case MILES_CONTROLLER_POLY_ON:
		// These act as an All Notes Off on MT-32, but also turn sustain off.
		// They are not part of GM, so should not be used in GM data.
		if (_midiType != MT_MT32) {
			warning("MILES-MIDI: unsupported GM controller %x", controllerNumber);
			return;
		}

		controlData.sustain = false;
		if (sendMessage)
			removeActiveNotes(outputChannel, true);
		if (!_nativeMT32) {
			// MT-32 data on GM device.
			// These controllers might not be supported or have side effects
			// (changing omni or mono/poly mode). Send All Notes Off and
			// Sustain Off instead.
			if (sendMessage) {
				controllerNumber = MILES_CONTROLLER_ALL_NOTES_OFF;
				_driver->send(0xB0 | outputChannel | (MILES_CONTROLLER_SUSTAIN << 8) | (0 << 16));
			}
		}
		// fall through
	case MILES_CONTROLLER_ALL_NOTES_OFF:
		if (sendMessage) {
			removeActiveNotes(outputChannel, false);
		}
		break;
	default:
		break;
	}

	if (sendMessage) {
		_driver->send(0xB0 | outputChannel | (controllerNumber << 8) | (controllerValue << 16));
	}
}

void MidiDriver_Miles_Midi::removeActiveNotes(uint8 outputChannel, bool sustainedNotes) {
	// Remove sustained notes from the active notes registration
	for (int i = 0; i < _maximumActiveNotes; ++i) {
		if (_activeNotes[i].channel == outputChannel && _activeNotes[i].sustain == sustainedNotes) {
			_activeNotes[i].source = 0x7F;
			_activeNotes[i].channel = 0xFF;
			if (_midiChannels[outputChannel].activeNotes == 0) {
				if (sustainedNotes)
					warning("MILES-MIDI: active notes 0 on channel %d when turning off sustained notes", outputChannel);
				else
					warning("MILES-MIDI: active notes 0 on channel %d when turning all notes off", outputChannel);
				continue;
			}
			--_midiChannels[outputChannel].activeNotes;
		}
	}
}

void MidiDriver_Miles_Midi::lockChannel(uint8 source, uint8 dataChannel) {
	assert(source < MILES_MAXIMUM_SOURCES);

	int8 lockChannel = findLockChannel();
	if (lockChannel == -1)
		// Try again, but consider lock protected channels
		lockChannel = findLockChannel(true);
	if (lockChannel == -1)
		// Could not find a channel to lock
		return;

	stopNotesOnChannel(lockChannel);

	_midiChannels[lockChannel].locked = true;
	_midiChannels[lockChannel].lockDataChannel = dataChannel;
	_sources[source].channelMap[dataChannel] = lockChannel;
	// Copy current controller values so they can be restored when unlocking the channel
	_midiChannels[lockChannel].unlockData = _midiChannels[lockChannel].currentData;
	_midiChannels[lockChannel].currentData.source = source;

	// Send volume change to apply the new source volume
	controlChange(lockChannel, MILES_CONTROLLER_VOLUME, 0x7F, source, _midiChannels[lockChannel].currentData, true);

	// Note that other controller values might be "inherited" from the source
	// which was previously playing on the locked MIDI channel. The KYRA engine
	// does not seem to take any precautions against this.
	// Controllers could be set to default values here.
}

int8 MidiDriver_Miles_Midi::findLockChannel(bool useProtectedChannels) {
	// Starting at the highest (non-rhythm) channel, find the channel
	// with the least active notes that isn't already locked.
	// If useProtectedChannels is false, channels that are protected
	// from channel locking will not be considered.
	int8 potentialLockChannel = -1;
	uint8 notes = 255;
	for (int i = MILES_MIDI_CHANNEL_COUNT - 1; i >= 0; --i) {
		if (!isOutputChannelUsed(i) || i == MILES_RHYTHM_CHANNEL || _midiChannels[i].locked || (!useProtectedChannels && _midiChannels[i].lockProtected))
			continue;
		if (_midiChannels[i].activeNotes < notes) {
			potentialLockChannel = i;
			notes = _midiChannels[i].activeNotes;
		}
	}
	return potentialLockChannel;
}

void MidiDriver_Miles_Midi::unlockChannel(uint8 outputChannel) {
	MidiChannelEntry &channel = _midiChannels[outputChannel];
	if (!channel.locked)
		return;

	stopNotesOnChannel(outputChannel);

	// Unlock the channel
	channel.locked = false;
	_sources[channel.currentData.source].channelMap[channel.lockDataChannel] = channel.lockDataChannel;
	channel.lockDataChannel = -1;
	channel.currentData.source = channel.unlockData.source;

	// Send the unlock channel data to the MIDI device to reset the channel parameters
	if (channel.unlockData.volume != 0xFF) {
		controlChange(outputChannel, MILES_CONTROLLER_VOLUME, channel.unlockData.volume, channel.currentData.source, channel.currentData, true);
	} else {
		channel.currentData.volume = 0xFF;
	}
	if (channel.currentData.modulation != channel.unlockData.modulation)
		controlChange(outputChannel, MILES_CONTROLLER_MODULATION, channel.unlockData.modulation, channel.currentData.source, channel.currentData, true);
	if (channel.currentData.panPosition != channel.unlockData.panPosition)
	controlChange(outputChannel, MILES_CONTROLLER_PANNING, channel.unlockData.panPosition, channel.currentData.source, channel.currentData, true);
	if (channel.currentData.expression != channel.unlockData.expression)
		controlChange(outputChannel, MILES_CONTROLLER_EXPRESSION, channel.unlockData.expression, channel.currentData.source, channel.currentData, true);
	if (channel.currentData.sustain != channel.unlockData.sustain)
		controlChange(outputChannel, MILES_CONTROLLER_SUSTAIN, channel.unlockData.sustain ? 0x7F : 0x00, channel.currentData.source, channel.currentData, true);
	if (channel.currentData.currentPatchBank != channel.unlockData.currentPatchBank)
		controlChange(outputChannel, MILES_CONTROLLER_SELECT_PATCH_BANK, channel.unlockData.currentPatchBank, channel.currentData.source, channel.currentData, true);
	if (channel.unlockData.program != 0xFF && (channel.currentData.program != channel.unlockData.program || channel.currentData.currentPatchBank != channel.unlockData.currentPatchBank))
		programChange(outputChannel, channel.unlockData.program, channel.currentData.source, channel.currentData, true);
	if (channel.currentData.pitchWheel != channel.unlockData.pitchWheel)
		send(channel.currentData.source, 0xE0 | outputChannel, channel.unlockData.pitchWheel & 0x7F, (channel.unlockData.pitchWheel >> 7) & 0x7F);
}

void MidiDriver_Miles_Midi::stopNotesOnChannel(uint8 outputChannelNumber) {
	MidiChannelEntry &channel = _midiChannels[outputChannelNumber];
	if (channel.currentData.sustain) {
		controlChange(outputChannelNumber, MILES_CONTROLLER_SUSTAIN, 0, channel.currentData.source, channel.currentData, true);
	}
	if (channel.activeNotes > 0) {
		controlChange(outputChannelNumber, MILES_CONTROLLER_ALL_NOTES_OFF, 0, channel.currentData.source, channel.currentData, true);
	}
}

void MidiDriver_Miles_Midi::stopAllNotes(bool stopSustainedNotes) {
	for (int i = 0; i < MILES_MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		if (stopSustainedNotes) {
			_driver->send(0xB0 | i, MILES_CONTROLLER_SUSTAIN, 0);
			_midiChannels[i].currentData.sustain = false;
		}
		_driver->send(0xB0 | i, MILES_CONTROLLER_ALL_NOTES_OFF, 0);
		_midiChannels[i].activeNotes = 0;
	}
	for (int i = 0; i < _maximumActiveNotes; ++i) {
		_activeNotes[i].source = 0x7F;
		_activeNotes[i].channel = 0xFF;
	}
}

void MidiDriver_Miles_Midi::programChange(byte outputChannel, byte patchId, uint8 source, MidiChannelControlData &controlData, bool sendMessage) {
	// remember patch id for the current MIDI-channel
	controlData.program = patchId;

	if (_midiType == MT_MT32) {
		byte channelPatchBank = controlData.currentPatchBank;
		byte activePatchBank = _patchesBank[patchId];

		//warning("patch channel %d, patch %x, bank %x", midiChannel, patchId, channelPatchBank);

		if (channelPatchBank != activePatchBank) {
			// associate patch with timbre
			setupPatch(channelPatchBank, patchId);
		}

		// If this is a custom patch, remember customTimbreId
		int16 customTimbre = searchCustomTimbre(channelPatchBank, patchId);
		if (customTimbre >= 0) {
			controlData.usingCustomTimbre = true;
			controlData.currentCustomTimbreId = customTimbre;
		} else {
			controlData.usingCustomTimbre = false;
		}

		if (outputChannel == MILES_RHYTHM_CHANNEL)
			// Patch changes on the rhythm channel are used by AIL to setup custom
			// rhythm timbres. There's no need to actually send them to the MT-32,
			// because it won't respond to them. On GM/GS devices they might
			// unintentionally change the drumkit.
			return;

		if (!_nativeMT32 && !_enableGS) {
			// GM device: map the patch to GM equivalent
			// TODO It would be nice if the patch bank could be taken into account
			// when using a custom ScummVM game-specific MT-32 to GM mapping.
			patchId = _mt32ToGm[patchId];
		}
	} else {
		// GM/GS MIDI
		if (outputChannel == MILES_RHYTHM_CHANNEL) {
			// Correct possible wrong GS drumkit number
			patchId = _gsDrumkitFallbackMap[patchId];
		} else if (!_nativeMT32) {
			// Correct possible wrong bank / instrument variation
			byte correctedBank = correctInstrumentBank(outputChannel, patchId);
			if (correctedBank != 0xFF) {
				// Send out a bank select for the corrected bank number
				controlChange(outputChannel, MILES_CONTROLLER_BANK_SELECT_MSB, correctedBank, source, controlData, sendMessage);
				controlChange(outputChannel, MILES_CONTROLLER_BANK_SELECT_LSB, 0, source, controlData, sendMessage);
			}
		} else {
			// GM on an MT-32: map the patch to the MT-32 equivalent
			patchId = _gmToMt32[patchId];
		}
	}

	// Finally send program change to MIDI device
	if (sendMessage) {
		_driver->send(0xC0 | outputChannel | (patchId << 8));
	}
}

int16 MidiDriver_Miles_Midi::searchCustomTimbre(byte patchBank, byte patchId) {
	byte customTimbreId = 0;

	for (customTimbreId = 0; customTimbreId < MILES_MT32_CUSTOMTIMBRE_COUNT; customTimbreId++) {
		if (_customTimbres[customTimbreId].used) {
			if ((_customTimbres[customTimbreId].currentPatchBank == patchBank) && (_customTimbres[customTimbreId].currentPatchId == patchId)) {
				return customTimbreId;
			}
		}
	}
	return -1;
}

const MilesMT32InstrumentEntry *MidiDriver_Miles_Midi::searchCustomInstrument(byte patchBank, byte patchId) {
	const MilesMT32InstrumentEntry *instrumentPtr = _instrumentTablePtr;

	for (uint16 instrumentNr = 0; instrumentNr < _instrumentTableCount; instrumentNr++) {
		if ((instrumentPtr->bankId == patchBank) && (instrumentPtr->patchId == patchId))
			return instrumentPtr;
		instrumentPtr++;
	}
	return NULL;
}

void MidiDriver_Miles_Midi::setupPatch(byte patchBank, byte patchId, bool useSysExQueue) {
	_patchesBank[patchId] = patchBank;

	if (patchBank) {
		// non-built-in bank
		int16 customTimbreId = searchCustomTimbre(patchBank, patchId);
		if (customTimbreId >= 0) {
			// now available? -> use this timbre
			writePatchTimbre(patchId, 2, customTimbreId, useSysExQueue); // Group MEMORY
			return;
		}
	}

	// for built-in bank (or timbres, that are not available) use default MT32 timbres
	byte timbreId = patchId & 0x3F;
	if (!(patchId & 0x40)) {
		writePatchTimbre(patchId, 0, timbreId, useSysExQueue); // Group A
	} else {
		writePatchTimbre(patchId, 1, timbreId, useSysExQueue); // Group B
	}
}

void MidiDriver_Miles_Midi::processXMIDITimbreChunk(const byte *timbreListPtr, uint32 timbreListSize) {
	if (_midiType != MT_MT32)
		// Some GM files contain timbre chunks, but custom patches cannot
		// be loaded on a GM device.
		return;

	uint16 timbreCount = 0;
	uint32 expectedSize = 0;
	const byte *timbreListSeeker = timbreListPtr;

	if (timbreListSize < 2) {
		warning("MILES-MIDI: XMIDI-TIMB chunk - not enough bytes in chunk");
		return;
	}

	timbreCount = READ_LE_UINT16(timbreListPtr);
	expectedSize = timbreCount * 2;
	if (expectedSize > timbreListSize) {
		warning("MILES-MIDI: XMIDI-TIMB chunk - size mismatch");
		return;
	}

	timbreListSeeker += 2;

	while (timbreCount) {
		const byte  patchId   = *timbreListSeeker++;
		const byte  patchBank = *timbreListSeeker++;
		int16       customTimbreId = 0;

		switch (patchBank) {
		case MILES_MT32_TIMBREBANK_STANDARD_ROLAND:
		case MILES_MT32_TIMBREBANK_MELODIC_MODULE:
			// ignore those 2 banks
			break;

		default:
			// Check, if this timbre was already loaded
			customTimbreId = searchCustomTimbre(patchBank, patchId);

			if (customTimbreId < 0) {
				// currently not loaded, try to install it
				installCustomTimbre(patchBank, patchId);
			}
		}
		timbreCount--;
	}
}

//
int16 MidiDriver_Miles_Midi::installCustomTimbre(byte patchBank, byte patchId) {
	switch(patchBank) {
	case MILES_MT32_TIMBREBANK_STANDARD_ROLAND: // Standard Roland MT32 bank
	case MILES_MT32_TIMBREBANK_MELODIC_MODULE:  // Reserved for melodic mode
		return -1;
	default:
		break;
	}

	// Original driver did a search for custom timbre here
	// and in case it was found, it would call setup_patch()
	// we are called from within setup_patch(), so this isn't needed

	int16 customTimbreId = -1;
	int16 leastUsedTimbreId = -1;
	uint32 leastUsedTimbreNoteCounter = _noteCounter;
	const MilesMT32InstrumentEntry *instrumentPtr = NULL;

	// Check, if requested instrument is actually available
	instrumentPtr = searchCustomInstrument(patchBank, patchId);
	if (!instrumentPtr) {
		warning("MILES-MIDI: instrument not found during installCustomTimbre()");
		return -1; // not found -> bail out
	}

	// Look for an empty timbre slot
	// or get the least used non-protected slot
	for (byte customTimbreNr = 0; customTimbreNr < MILES_MT32_CUSTOMTIMBRE_COUNT; customTimbreNr++) {
		if (!_customTimbres[customTimbreNr].used) {
			// found an empty slot -> use this one
			customTimbreId = customTimbreNr;
			break;
		} else {
			// used slot
			if (!_customTimbres[customTimbreNr].protectionEnabled) {
				// not protected
				uint32 customTimbreNoteCounter = _customTimbres[customTimbreNr].lastUsedNoteCounter;
				if (customTimbreNoteCounter < leastUsedTimbreNoteCounter) {
					leastUsedTimbreId          = customTimbreNr;
					leastUsedTimbreNoteCounter = customTimbreNoteCounter;
				}
			}
		}
	}

	if (customTimbreId < 0) {
		// no empty slot found, check if we got a least used non-protected slot
		if (leastUsedTimbreId < 0) {
			// everything is protected, bail out
			warning("MILES-MIDI: no non-protected timbre slots available during installCustomTimbre()");
			return -1;
		}
		customTimbreId = leastUsedTimbreId;
	}

	// setup timbre slot
	_customTimbres[customTimbreId].used                = true;
	_customTimbres[customTimbreId].currentPatchBank    = patchBank;
	_customTimbres[customTimbreId].currentPatchId      = patchId;
	_customTimbres[customTimbreId].lastUsedNoteCounter = _noteCounter;
	_customTimbres[customTimbreId].protectionEnabled   = false;

	uint32 targetAddress = 0x080000 | (customTimbreId << 9);
	uint32 targetAddressCommon   = targetAddress + 0x000000;
	uint32 targetAddressPartial1 = targetAddress + 0x00000E;
	uint32 targetAddressPartial2 = targetAddress + 0x000048;
	uint32 targetAddressPartial3 = targetAddress + 0x000102;
	uint32 targetAddressPartial4 = targetAddress + 0x00013C;

#if 0
	byte parameterData[MILES_MT32_PATCHDATA_TOTAL_SIZE + 1];
	uint16 parameterDataPos = 0;

	memcpy(parameterData, instrumentPtr->commonParameter, MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE);
	parameterDataPos += MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE;
	memcpy(parameterData + parameterDataPos, instrumentPtr->partialParameters[0], MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE);
	parameterDataPos += MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE;
	memcpy(parameterData + parameterDataPos, instrumentPtr->partialParameters[1], MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE);
	parameterDataPos += MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE;
	memcpy(parameterData + parameterDataPos, instrumentPtr->partialParameters[2], MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE);
	parameterDataPos += MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE;
	memcpy(parameterData + parameterDataPos, instrumentPtr->partialParameters[3], MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE);
	parameterDataPos += MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE;
	parameterData[parameterDataPos] = MILES_MT32_SYSEX_TERMINATOR;

	MT32SysEx(targetAddressCommon, parameterData);
#endif

	// upload common parameter data
	MT32SysEx(targetAddressCommon, instrumentPtr->commonParameter, true);
	// upload partial parameter data
	MT32SysEx(targetAddressPartial1, instrumentPtr->partialParameters[0], true);
	MT32SysEx(targetAddressPartial2, instrumentPtr->partialParameters[1], true);
	MT32SysEx(targetAddressPartial3, instrumentPtr->partialParameters[2], true);
	MT32SysEx(targetAddressPartial4, instrumentPtr->partialParameters[3], true);

	setupPatch(patchBank, patchId, true);

	return customTimbreId;
}

uint32 MidiDriver_Miles_Midi::calculateSysExTargetAddress(uint32 baseAddress, uint32 index) {
	uint16 targetAddressLSB = baseAddress & 0xFF;
	uint16 targetAddressKSB = (baseAddress >> 8) & 0xFF;
	uint16 targetAddressMSB = (baseAddress >> 16) & 0xFF;

	// add index to it, but use 7-bit of the index for each byte
	targetAddressLSB += (index & 0x7F);
	targetAddressKSB += ((index >> 7) & 0x7F);
	targetAddressMSB += ((index >> 14) & 0x7F);

	// adjust bytes, so that none of them is above or equal 0x80
	while (targetAddressLSB >= 0x80) {
		targetAddressLSB -= 0x80;
		targetAddressKSB++;
	}
	while (targetAddressKSB >= 0x80) {
		targetAddressKSB -= 0x80;
		targetAddressMSB++;
	}
	assert(targetAddressMSB < 0x80);

	// put everything together
	return targetAddressLSB | (targetAddressKSB << 8) | (targetAddressMSB << 16);
}

void MidiDriver_Miles_Midi::writeRhythmSetup(byte note, byte customTimbreId) {
	byte   sysExData[2];
	uint32 targetAddress = 0;

	targetAddress = calculateSysExTargetAddress(0x030110, ((note - 24) << 2));

	sysExData[0] = customTimbreId;
	sysExData[1] = MILES_MT32_SYSEX_TERMINATOR; // terminator

	MT32SysEx(targetAddress, sysExData);
}

void MidiDriver_Miles_Midi::writePatchTimbre(byte patchId, byte timbreGroup, byte timbreId, bool useSysExQueue) {
	byte   sysExData[3];
	uint32 targetAddress = 0;

	// write to patch memory (starts at 0x050000, each entry is 8 bytes)
	targetAddress = calculateSysExTargetAddress(0x050000, patchId << 3);

	sysExData[0] = timbreGroup; // 0 - group A, 1 - group B, 2 - memory, 3 - rhythm
	sysExData[1] = timbreId;    // timbre number (0-63)
	sysExData[2] = MILES_MT32_SYSEX_TERMINATOR; // terminator

	MT32SysEx(targetAddress, sysExData, useSysExQueue);
}

void MidiDriver_Miles_Midi::writePatchByte(byte patchId, byte index, byte patchValue) {
	byte   sysExData[2];
	uint32 targetAddress = 0;

	targetAddress = calculateSysExTargetAddress(0x050000, (patchId << 3) + index);

	sysExData[0] = patchValue;
	sysExData[1] = MILES_MT32_SYSEX_TERMINATOR; // terminator

	MT32SysEx(targetAddress, sysExData);
}

void MidiDriver_Miles_Midi::writeToSystemArea(byte index, byte value) {
	byte   sysExData[2];
	uint32 targetAddress = 0;

	targetAddress = calculateSysExTargetAddress(0x100000, index);

	sysExData[0] = value;
	sysExData[1] = MILES_MT32_SYSEX_TERMINATOR; // terminator

	MT32SysEx(targetAddress, sysExData);
}

MidiDriver_Miles_Midi *MidiDriver_Miles_MT32_create(const Common::String &instrumentDataFilename) { return MidiDriver_Miles_MIDI_create(MT_MT32, instrumentDataFilename); }

MidiDriver_Miles_Midi *MidiDriver_Miles_MIDI_create(MusicType midiType, const Common::String &instrumentDataFilename) {
	assert(midiType == MT_MT32 || midiType == MT_GM || midiType == MT_GS);

	MilesMT32InstrumentEntry *instrumentTablePtr = NULL;
	uint16                    instrumentTableCount = 0;

	if (midiType == MT_MT32 && !instrumentDataFilename.empty()) {
		// Load MT32 instrument data from file SAMPLE.MT
		Common::File *fileStream = new Common::File();
		uint32        fileSize = 0;
		byte         *fileDataPtr = NULL;
		uint32        fileDataOffset = 0;
		uint32        fileDataLeft = 0;

		byte curBankId;
		byte curPatchId;

		MilesMT32InstrumentEntry *instrumentPtr = NULL;
		uint32                    instrumentOffset;
		uint16                    instrumentDataSize;

		if (!fileStream->open(instrumentDataFilename))
			error("MILES-MIDI: could not open instrument file '%s'", instrumentDataFilename.c_str());

		fileSize = fileStream->size();

		fileDataPtr = new byte[fileSize];

		if (fileStream->read(fileDataPtr, fileSize) != fileSize)
			error("MILES-MIDI: error while reading instrument file");
		fileStream->close();
		delete fileStream;

		// File is like this:
		// [patch:BYTE] [bank:BYTE] [patchoffset:UINT32]
		// ...
		// until patch + bank are both 0xFF, which signals end of header

		// First we check how many entries there are
		fileDataOffset = 0;
		fileDataLeft = fileSize;
		while (1) {
			if (fileDataLeft < 6)
				error("MILES-MIDI: unexpected EOF in instrument file");

			curPatchId = fileDataPtr[fileDataOffset++];
			curBankId  = fileDataPtr[fileDataOffset++];

			if ((curBankId == 0xFF) && (curPatchId == 0xFF))
				break;

			fileDataOffset += 4; // skip over offset
			instrumentTableCount++;
		}

		if (instrumentTableCount == 0)
			error("MILES-MIDI: no instruments in instrument file");

		// Allocate space for instruments
		instrumentTablePtr = new MilesMT32InstrumentEntry[instrumentTableCount];

		// Now actually read all entries
		instrumentPtr = instrumentTablePtr;

		fileDataOffset = 0;
		while (1) {
			curPatchId = fileDataPtr[fileDataOffset++];
			curBankId  = fileDataPtr[fileDataOffset++];

			if ((curBankId == 0xFF) && (curPatchId == 0xFF))
				break;

			instrumentOffset = READ_LE_UINT32(fileDataPtr + fileDataOffset);
			fileDataOffset += 4;

			instrumentPtr->bankId = curBankId;
			instrumentPtr->patchId = curPatchId;

			instrumentDataSize = READ_LE_UINT16(fileDataPtr + instrumentOffset);
			if (instrumentDataSize != (MILES_MT32_PATCHDATA_TOTAL_SIZE + 2))
				error("MILES-MIDI: unsupported instrument size");

			instrumentOffset += 2;
			// Copy common parameter data
			memcpy(instrumentPtr->commonParameter, fileDataPtr + instrumentOffset, MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE);
			instrumentPtr->commonParameter[MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE] = MILES_MT32_SYSEX_TERMINATOR; // Terminator
			instrumentOffset += MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE;

			// Copy partial parameter data
			for (byte partialNr = 0; partialNr < MILES_MT32_PATCHDATA_PARTIALPARAMETERS_COUNT; partialNr++) {
				memcpy(&instrumentPtr->partialParameters[partialNr], fileDataPtr + instrumentOffset, MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE);
				instrumentPtr->partialParameters[partialNr][MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE] = MILES_MT32_SYSEX_TERMINATOR; // Terminator
				instrumentOffset += MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE;
			}

			// Instrument read, next instrument please
			instrumentPtr++;
		}

		// Free instrument file data
		delete[] fileDataPtr;
	}

	return new MidiDriver_Miles_Midi(midiType, instrumentTablePtr, instrumentTableCount);
}

void MidiDriver_Miles_Midi::deinitSource(uint8 source) {
	assert(source < MILES_MAXIMUM_SOURCES);

	// Unlock and unprotect channels which were locked or protected by this source.
	for (int i = 0; i < MILES_MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		if (_midiChannels[i].currentData.source == source && _midiChannels[i].locked) {
			unlockChannel(i);
		}
		if (_midiChannels[i].lockProtected && _midiChannels[i].protectedSource == source) {
			_midiChannels[i].lockProtected = false;
			_midiChannels[i].protectedSource = -1;
		}
		if (_midiChannels[i].currentData.source == source)
			_midiChannels[i].currentData.source = -1;
		if (_midiChannels[i].unlockData.source == source)
			_midiChannels[i].unlockData.source = -1;
	}
	// Reset the data to output channel mapping
	for (int i = 0; i < MILES_MIDI_CHANNEL_COUNT; ++i) {
		_sources[source].channelMap[i] = i;
	}
	// Stop any active notes.
	for (int i = 0; i < _maximumActiveNotes; ++i) {
		if (_activeNotes[i].source == source) {
			if (_activeNotes[i].sustain) {
				// Turn off sustain
				controlChange(_activeNotes[i].channel, MILES_CONTROLLER_SUSTAIN, 0x00, source, _midiChannels[_activeNotes[i].channel].currentData, true);
			} else {
				// Send note off
				send(source, 0x80 | _activeNotes[i].channel, _activeNotes[i].note, 0x00);
			}
		}
	}
}

void MidiDriver_Miles_Midi::setSourceVolume(uint8 source, uint16 volume) {
	assert(source < MILES_MAXIMUM_SOURCES);

	_sources[source].volume = volume;

	for (int i = 0; i < MILES_MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		MidiChannelEntry &channel = _midiChannels[i];
		MidiChannelControlData *channelData = 0;
		bool sendMessage = false;
		// Apply the new source volume to this channel if this source is active
		// on this channel, or if it was active on the channel before it was
		// locked.
		if (channel.currentData.source == source) {
			channelData = &channel.currentData;
			sendMessage = true;
		} else if (channel.locked && channel.unlockData.source == source) {
			channelData = &channel.unlockData;
		}

		if (channelData && channelData->volume != 0xFF)
			controlChange(i, MILES_CONTROLLER_VOLUME, channelData->volume, source, *channelData, sendMessage);
	}
}

void MidiDriver_Miles_Midi::onTimer() {
	Common::StackLock lock(_sysExQueueMutex);

	_sysExDelay -= (_sysExDelay > _timerRate) ? _timerRate : _sysExDelay;

	if (!_sysExQueue.empty() && _sysExDelay == 0) {
		// Ready to send next SysEx message to the MIDI device
		SysExData sysEx = _sysExQueue.pop();
		_sysExDelay = sysExNoDelay(sysEx.data, sysEx.length) * 1000;
	}
}

} // End of namespace Audio
