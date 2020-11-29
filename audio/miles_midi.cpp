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

const byte milesMT32SysExChansSetup[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
};

const byte milesMT32SysExPartialReserveTable[] = {
	0x03, 0x04, 0x03, 0x04, 0x03, 0x04, 0x03, 0x04, 0x04
};

const byte milesMT32SysExInitReverb[] = {
	0x00, 0x03, 0x02 // Reverb mode 0, reverb time 3, reverb level 2
};

MidiDriver_Miles_Midi::MidiDriver_Miles_Midi(MusicType midiType, MilesMT32InstrumentEntry *instrumentTablePtr, uint16 instrumentTableCount) :
		MidiDriver_MT32GM(midiType), _noteCounter(0) {
	memset(_patchesBank, 0, sizeof(_patchesBank));

	_instrumentTablePtr = instrumentTablePtr;
	_instrumentTableCount = instrumentTableCount;

	// Disable user volume scaling by default. Most (all?)
	// engines using Miles implement this themselves. Can
	// be turned on using the property function.
	_userVolumeScaling = false;

	setSourceNeutralVolume(MILES_DEFAULT_SOURCE_NEUTRAL_VOLUME);
}

MidiDriver_Miles_Midi::~MidiDriver_Miles_Midi() {
	if (_instrumentTablePtr)
		delete[] _instrumentTablePtr;

	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (_midiChannels[i].unlockData)
			delete _midiChannels[i].unlockData;
	}
}

void MidiDriver_Miles_Midi::initControlData() {
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		_controlData[i] = _midiChannels[i].currentData = new MilesMidiChannelControlData();
		_midiChannels[i].unlockData = new MilesMidiChannelControlData();
		_controlData[i]->volume = _controlData[i]->scaledVolume =
			(_nativeMT32 ? MT32_DEFAULT_CHANNEL_VOLUME : GM_DEFAULT_CHANNEL_VOLUME);
		if (_nativeMT32 && i >= 1 && i <= 8) {
			_midiChannels[i].currentData->program = MT32_DEFAULT_INSTRUMENTS[i - 1];
			_midiChannels[i].currentData->panPosition = MT32_DEFAULT_PANNING[i - 1];
		}
	}
}

void MidiDriver_Miles_Midi::initMidiDevice() {
	MidiDriver_MT32GM::initMidiDevice();

	// Additional Miles AIL specific initialization

	if (_midiType == MT_MT32 && _nativeMT32) {
		// init part/channel assignments
		sysExMT32(milesMT32SysExChansSetup, 9, (0x10 << 14) | (0x00 << 7) | 0x0D, false, true);

		// partial reserve table
		sysExMT32(milesMT32SysExPartialReserveTable, 9, (0x10 << 14) | (0x00 << 7) | 0x04, false, true);

		// init reverb
		sysExMT32(milesMT32SysExInitReverb, 3, (0x10 << 14) | (0x00 << 7) | 0x01, false, true);
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
		send(-1, MIDI_COMMAND_CONTROL_CHANGE | i, MIDI_CONTROLLER_VOLUME, 0x7F);
		if (_midiType == MT_MT32) {
			// Panning center - not the MT-32 default for all channels
			send(-1, MIDI_COMMAND_CONTROL_CHANGE | i, MIDI_CONTROLLER_PANNING, 0x40);
		}
		// Patch
		if (i != MIDI_RHYTHM_CHANNEL) {
			if (_midiType == MT_GM) {
				// Send the MT-32 default instrument numbers out to GM devices.
				send(-1, MIDI_COMMAND_PROGRAM_CHANGE | i, MT32_DEFAULT_INSTRUMENTS[i - 1], 0);
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

// MIDI messages can be found at http://www.midi.org/techspecs/midimessages.php
void MidiDriver_Miles_Midi::send(int8 source, uint32 b) {
	assert(source < MAXIMUM_SOURCES);

	byte command = b & 0xf0;
	byte dataChannel = b & 0xf;
	byte outputChannel = source < 0 ? dataChannel : _sources[source].channelMap[dataChannel];

	MidiChannelEntry &outputChannelEntry = _midiChannels[outputChannel];
	// Only send the message to the MIDI device if the channel is not locked or
	// if the source that locked the channel is sending the message
	bool channelLockedByOtherSource = source >= 0 && outputChannelEntry.locked &&
		outputChannelEntry.currentData->source != source;
	// Track controller changes on the current data if the MIDI message is sent out,
	// or on the unlock data otherwise.
	MilesMidiChannelControlData &controlData = channelLockedByOtherSource ?
		*outputChannelEntry.unlockData : *outputChannelEntry.currentData;

	processEvent(source, b, outputChannel, controlData, channelLockedByOtherSource);

	if (command == MIDI_COMMAND_NOTE_OFF || command == MIDI_COMMAND_NOTE_ON || command == MIDI_COMMAND_PITCH_BEND ||
		command == MIDI_COMMAND_POLYPHONIC_AFTERTOUCH || command == MIDI_COMMAND_CHANNEL_AFTERTOUCH) {
		_noteCounter++;
		if (controlData.usingCustomTimbre) {
			// Remember that this timbre got used now
			_customTimbres[controlData.currentCustomTimbreId].lastUsedNoteCounter = _noteCounter;
		}
	}
}

void MidiDriver_Miles_Midi::controlChange(byte outputChannel, byte controllerNumber, byte controllerValue,
	int8 source, MidiChannelControlData &controlData, bool channelLockedByOtherSource) {
	assert(source < MAXIMUM_SOURCES);

	MilesMidiChannelControlData &milesControlData = channelLockedByOtherSource ?
		*_midiChannels[outputChannel].unlockData : *_midiChannels[outputChannel].currentData;

	// XMIDI controllers
	switch (controllerNumber) {
	case MILES_CONTROLLER_SELECT_PATCH_BANK:
		milesControlData.currentPatchBank = controllerValue;
		return;

	case MILES_CONTROLLER_PROTECT_TIMBRE:
		if (milesControlData.usingCustomTimbre) {
			// custom timbre set on current channel
			_customTimbres[milesControlData.currentCustomTimbreId].protectionEnabled = controllerValue >= 64;
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
			writePatchByte(milesControlData.program, 6, controllerValue);
			if (!channelLockedByOtherSource)
				_driver->send(MIDI_COMMAND_PROGRAM_CHANGE | outputChannel | (milesControlData.program << 8));
			return;

		case MILES_CONTROLLER_PATCH_BENDER:
			writePatchByte(milesControlData.program, 4, controllerValue);
			if (!channelLockedByOtherSource)
				_driver->send(MIDI_COMMAND_PROGRAM_CHANGE | outputChannel | (milesControlData.program << 8));
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
			if (milesControlData.usingCustomTimbre) {
				// custom timbre is set on current channel
				writeRhythmSetup(controllerValue, milesControlData.currentCustomTimbreId);
			}
			return;
		default:
			break;
		}
	}

	// XMIDI MT-32 SysEx controllers
	if (_midiType == MT_MT32 && (controllerNumber >= MILES_CONTROLLER_SYSEX_RANGE_BEGIN) &&
			(controllerNumber <= MILES_CONTROLLER_SYSEX_RANGE_END)) {
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
			_milesSysExQueues[sysExQueueNr].targetAddress &= 0x003FFF;
			_milesSysExQueues[sysExQueueNr].targetAddress |= (controllerValue << 14);
			break;
		case MILES_CONTROLLER_SYSEX_COMMAND_ADDRESS2:
			_milesSysExQueues[sysExQueueNr].targetAddress &= 0x1FC07F;
			_milesSysExQueues[sysExQueueNr].targetAddress |= (controllerValue << 7);
			break;
		case MILES_CONTROLLER_SYSEX_COMMAND_ADDRESS3:
			_milesSysExQueues[sysExQueueNr].targetAddress &= 0x1FFF80;
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
				sysExMT32(_milesSysExQueues[sysExQueueNr].data, sysExPos, _milesSysExQueues[sysExQueueNr].targetAddress, false, true);

				// Adjust target address to point at the final data byte, or at the
				// end of the current data in case of an overflow
				_milesSysExQueues[sysExQueueNr].targetAddress += _milesSysExQueues[sysExQueueNr].dataPos;

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

	// Handle other controllers and send message (if necessary)
	MidiDriver_MT32GM::controlChange(outputChannel, controllerNumber, controllerValue, source, milesControlData, channelLockedByOtherSource);
}

bool MidiDriver_Miles_Midi::addActiveNote(uint8 outputChannel, uint8 note, int8 source) {
	bool added = MidiDriver_MT32GM::addActiveNote(outputChannel, note, source);

	if (added)
		_midiChannels[outputChannel].activeNotes++;

	return added;
}

bool MidiDriver_Miles_Midi::removeActiveNote(uint8 outputChannel, uint8 note, int8 source) {
	bool removed = MidiDriver_MT32GM::removeActiveNote(outputChannel, note, source);

	if (removed) {
		if (_midiChannels[outputChannel].activeNotes == 0) {
			warning("MILES-MIDI: active notes 0 on channel %d when turning off note %x", outputChannel, note);
		} else {
			_midiChannels[outputChannel].activeNotes--;
		}
	}

	return removed;
}

void MidiDriver_Miles_Midi::removeActiveNotes(uint8 outputChannel, bool sustainedNotes) {
	Common::StackLock lock(_activeNotesMutex);

	// Remove sustained or non-sustained notes from the active notes registration
	for (int i = 0; i < _maximumActiveNotes; ++i) {
		if (_activeNotes[i].channel == outputChannel && _activeNotes[i].sustain == sustainedNotes) {
			_activeNotes[i].clear();
			if (_midiChannels[outputChannel].activeNotes == 0) {
				warning("MILES-MIDI: active notes 0 on channel %d (sustained %i) when removing active notes", outputChannel, sustainedNotes);
				continue;
			}
			--_midiChannels[outputChannel].activeNotes;
		}
	}
}

void MidiDriver_Miles_Midi::lockChannel(uint8 source, uint8 dataChannel) {
	assert(source < MAXIMUM_SOURCES);

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
	*_midiChannels[lockChannel].unlockData = *_midiChannels[lockChannel].currentData;
	_midiChannels[lockChannel].currentData->source = source;

	// Send volume change to apply the new source volume
	controlChange(lockChannel, MIDI_CONTROLLER_VOLUME, 0x7F, source, *_midiChannels[lockChannel].currentData);

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
	for (int i = MIDI_CHANNEL_COUNT - 1; i >= 0; --i) {
		if (!isOutputChannelUsed(i) || i == MIDI_RHYTHM_CHANNEL || _midiChannels[i].locked ||
				(!useProtectedChannels && _midiChannels[i].lockProtected))
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
	_sources[channel.currentData->source].channelMap[channel.lockDataChannel] = channel.lockDataChannel;
	channel.lockDataChannel = -1;
	channel.currentData->source = channel.unlockData->source;

	// Send the unlock channel data to the MIDI device to reset the channel parameters
	if (channel.unlockData->volume != 0xFF) {
		controlChange(outputChannel, MIDI_CONTROLLER_VOLUME, channel.unlockData->volume, channel.currentData->source, *channel.currentData);
	} else {
		channel.currentData->volume = 0xFF;
	}
	if (channel.currentData->modulation != channel.unlockData->modulation)
		controlChange(outputChannel, MIDI_CONTROLLER_MODULATION, channel.unlockData->modulation, channel.currentData->source, *channel.currentData);
	if (channel.currentData->panPosition != channel.unlockData->panPosition)
		controlChange(outputChannel, MIDI_CONTROLLER_PANNING, channel.unlockData->panPosition, channel.currentData->source, *channel.currentData);
	if (channel.currentData->expression != channel.unlockData->expression)
		controlChange(outputChannel, MIDI_CONTROLLER_EXPRESSION, channel.unlockData->expression, channel.currentData->source, *channel.currentData);
	if (channel.currentData->sustain != channel.unlockData->sustain)
		controlChange(outputChannel, MIDI_CONTROLLER_SUSTAIN, channel.unlockData->sustain ? 0x7F : 0x00, channel.currentData->source, *channel.currentData);
	if (channel.currentData->currentPatchBank != channel.unlockData->currentPatchBank)
		controlChange(outputChannel, MILES_CONTROLLER_SELECT_PATCH_BANK, channel.unlockData->currentPatchBank,
			channel.currentData->source, *channel.currentData);
	if (channel.unlockData->program != 0xFF && (channel.currentData->program != channel.unlockData->program ||
			channel.currentData->currentPatchBank != channel.unlockData->currentPatchBank))
		programChange(outputChannel, channel.unlockData->program, channel.currentData->source, *channel.currentData);
	if (channel.currentData->pitchWheel != channel.unlockData->pitchWheel)
		send(channel.currentData->source, MIDI_COMMAND_PITCH_BEND | outputChannel,
			channel.unlockData->pitchWheel & 0x7F, (channel.unlockData->pitchWheel >> 7) & 0x7F);
}

void MidiDriver_Miles_Midi::stopNotesOnChannel(uint8 outputChannelNumber) {
	MidiChannelEntry &channel = _midiChannels[outputChannelNumber];
	if (channel.currentData->sustain) {
		controlChange(outputChannelNumber, MIDI_CONTROLLER_SUSTAIN, 0, channel.currentData->source, *channel.currentData);
	}
	if (channel.activeNotes > 0) {
		controlChange(outputChannelNumber, MIDI_CONTROLLER_ALL_NOTES_OFF, 0, channel.currentData->source, *channel.currentData);
	}
}

void MidiDriver_Miles_Midi::stopAllNotes(bool stopSustainedNotes) {
	MidiDriver_MT32GM::stopAllNotes(stopSustainedNotes);

	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (isOutputChannelUsed(i))
			_midiChannels[i].activeNotes = 0;
	}
}

void MidiDriver_Miles_Midi::programChange(byte outputChannel, byte patchId, int8 source,
		MidiChannelControlData &controlData, bool channelLockedByOtherSource) {
	MilesMidiChannelControlData &milesControlData = channelLockedByOtherSource ?
		*_midiChannels[outputChannel].unlockData : *_midiChannels[outputChannel].currentData;

	if (_midiType == MT_MT32) {
		byte channelPatchBank = milesControlData.currentPatchBank;
		byte activePatchBank = _patchesBank[patchId];

		//warning("patch channel %d, patch %x, bank %x", midiChannel, patchId, channelPatchBank);

		if (channelPatchBank != activePatchBank) {
			// associate patch with timbre
			setupPatch(channelPatchBank, patchId);
		}

		// If this is a custom patch, remember customTimbreId
		int16 customTimbre = searchCustomTimbre(channelPatchBank, patchId);
		if (customTimbre >= 0) {
			milesControlData.usingCustomTimbre = true;
			milesControlData.currentCustomTimbreId = customTimbre;
		} else {
			milesControlData.usingCustomTimbre = false;
		}
	}

	MidiDriver_MT32GM::programChange(outputChannel, patchId, source, milesControlData, channelLockedByOtherSource);
}

int16 MidiDriver_Miles_Midi::searchCustomTimbre(byte patchBank, byte patchId) {
	byte customTimbreId = 0;

	for (customTimbreId = 0; customTimbreId < MILES_MT32_CUSTOMTIMBRE_COUNT; customTimbreId++) {
		if (_customTimbres[customTimbreId].used) {
			if ((_customTimbres[customTimbreId].currentPatchBank == patchBank) &&
					(_customTimbres[customTimbreId].currentPatchId == patchId)) {
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

	uint32 targetAddress = ((0x08 << 14) | (0x00 << 7) | 0x00) + (customTimbreId * 0x100);
	uint32 targetAddressCommon   = targetAddress;
	uint32 targetAddressPartial1 = targetAddress + 0x0E;
	uint32 targetAddressPartial2 = targetAddress + 0x48;
	uint32 targetAddressPartial3 = targetAddress + 0x82;
	uint32 targetAddressPartial4 = targetAddress + 0xBC;

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
	sysExMT32(instrumentPtr->commonParameter, MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE, targetAddressCommon, true);
	// upload partial parameter data
	sysExMT32(instrumentPtr->partialParameters[0], MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE, targetAddressPartial1, true);
	sysExMT32(instrumentPtr->partialParameters[1], MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE, targetAddressPartial2, true);
	sysExMT32(instrumentPtr->partialParameters[2], MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE, targetAddressPartial3, true);
	sysExMT32(instrumentPtr->partialParameters[3], MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE, targetAddressPartial4, true);

	setupPatch(patchBank, patchId, true);

	return customTimbreId;
}

void MidiDriver_Miles_Midi::writeRhythmSetup(byte note, byte customTimbreId) {
	byte   sysExData[1];
	uint32 targetAddress = (0x03 << 14) | (0x01 << 7) | 0x10;

	targetAddress += ((note - 24) << 2);

	sysExData[0] = customTimbreId;

	sysExMT32(sysExData, 1, targetAddress);
}

void MidiDriver_Miles_Midi::writePatchTimbre(byte patchId, byte timbreGroup, byte timbreId, bool useSysExQueue) {
	byte   sysExData[2];
	uint32 targetAddress = (0x05 << 14) | (0x00 << 7) | 0x00;

	// write to patch memory (starts at 0x050000, each entry is 8 bytes)
	targetAddress += (patchId << 3);

	sysExData[0] = timbreGroup; // 0 - group A, 1 - group B, 2 - memory, 3 - rhythm
	sysExData[1] = timbreId;    // timbre number (0-63)

	sysExMT32(sysExData, 2, targetAddress, useSysExQueue);
}

void MidiDriver_Miles_Midi::writePatchByte(byte patchId, byte index, byte patchValue) {
	byte   sysExData[1];
	uint32 targetAddress = (0x05 << 14) | (0x00 << 7) | 0x00;

	targetAddress += (patchId << 3) + index;

	sysExData[0] = patchValue;

	sysExMT32(sysExData, 1, targetAddress);
}

void MidiDriver_Miles_Midi::writeToSystemArea(byte index, byte value) {
	byte   sysExData[1];
	uint32 targetAddress = (0x10 << 14) | (0x00 << 7) | 0x00;

	targetAddress += index;

	sysExData[0] = value;

	sysExMT32(sysExData, 1, targetAddress);
}

MidiDriver_Miles_Midi *MidiDriver_Miles_MT32_create(const Common::String &instrumentDataFilename) {
	return MidiDriver_Miles_MIDI_create(MT_MT32, instrumentDataFilename);
}

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
			instrumentOffset += MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE;

			// Copy partial parameter data
			for (byte partialNr = 0; partialNr < MILES_MT32_PATCHDATA_PARTIALPARAMETERS_COUNT; partialNr++) {
				memcpy(&instrumentPtr->partialParameters[partialNr], fileDataPtr + instrumentOffset, MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE);
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
	// Unlock and unprotect channels which were locked or protected by this source.
	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		if (_midiChannels[i].currentData->source == source && _midiChannels[i].locked) {
			unlockChannel(i);
		}
		if (_midiChannels[i].lockProtected && _midiChannels[i].protectedSource == source) {
			_midiChannels[i].lockProtected = false;
			_midiChannels[i].protectedSource = -1;
		}
		if (_midiChannels[i].currentData->source == source)
			_midiChannels[i].currentData->source = -1;
		if (_midiChannels[i].unlockData->source == source)
			_midiChannels[i].unlockData->source = -1;
	}

	MidiDriver_MT32GM::deinitSource(source);
}

void MidiDriver_Miles_Midi::setSourceVolume(uint8 source, uint16 volume) {
	assert(source < MAXIMUM_SOURCES);

	_sources[source].volume = volume;

	for (int i = 0; i < MIDI_CHANNEL_COUNT; ++i) {
		if (!isOutputChannelUsed(i))
			continue;

		MidiChannelEntry &channel = _midiChannels[i];
		MilesMidiChannelControlData *channelData = 0;
		bool channelLockedByOtherSource = false;
		// Apply the new source volume to this channel if this source is active
		// on this channel, or if it was active on the channel before it was
		// locked.
		if (channel.currentData->source == source) {
			channelData = channel.currentData;
		} else if (channel.locked && channel.unlockData->source == source) {
			channelData = channel.unlockData;
			channelLockedByOtherSource = true;
		}

		if (channelData && channelData->volume != 0xFF)
			controlChange(i, MIDI_CONTROLLER_VOLUME, channelData->volume, source, *channelData, channelLockedByOtherSource);
	}
}

} // End of namespace Audio
