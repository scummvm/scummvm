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

#include "sherlock/sherlock.h"
#include "sherlock/tattoo/drivers/tattoo_mididriver.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Sherlock {

#define MILES_MT32_PATCH_COUNT 128

const byte milesMT32SysExResetParameters[] = {
	0x01, 0xFF
};

const byte milesMT32SysExChansSetup[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF
};

const byte milesMT32SysExPartialReserveTable[] = {
	0x03, 0x04, 0x03, 0x04, 0x03, 0x04, 0x03, 0x04, 0x04, 0xFF
};

const byte milesMT32SysExInitReverb[] = {
	0x00, 0x03, 0x02, 0xFF // Reverb mode 0, reverb time 3, reverb level 2
};

class MidiDriver_Miles_MT32 : public MidiDriver {
public:
	MidiDriver_Miles_MT32();
	virtual ~MidiDriver_Miles_MT32();

	// MidiDriver
	int open();
	void close();
	bool isOpen() const { return _isOpen; }

	void send(uint32 b);

	MidiChannel *allocateChannel() {
		if (_driver)
			return _driver->allocateChannel();
		return NULL;
	}
	MidiChannel *getPercussionChannel() {
		if (_driver)
			return _driver->getPercussionChannel();
		return NULL;
	}

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
		if (_driver)
			_driver->setTimerCallback(timer_param, timer_proc);
	}

	uint32 getBaseTempo() {
		if (_driver) {
			return _driver->getBaseTempo();
		}
		return 1000000 / _baseFreq;
	}

protected:
	Common::Mutex _mutex;
	MidiDriver *_driver;
	bool _MT32;
	bool _nativeMT32;

	bool _isOpen;
	int _baseFreq;

public:

private:
	void resetMT32();

	void MT32SysEx(const uint32 targetAddress, const byte *dataPtr);

	void writePatchTimbre(byte patchId, byte timbreGroup, byte timbreId);
	void writePatchByte(byte patchId, byte index, byte patchValue);
	void writeToSystemArea(byte index, byte value);

	void controlChange(byte midiChannel, byte controllerNumber, byte controllerValue);
	void programChange(byte midiChannel, byte patchId);

	void setupPatch(byte patchId, byte patchBank);

private:
	struct MidiChannelEntry {
		byte   currentPatchBank;
		byte   currentPatchId;
		bool   patchIdSet;

		MidiChannelEntry() : currentPatchBank(0),
							currentPatchId(0),
							patchIdSet(false) { }
	};

	// stores information about all MIDI channels
	MidiChannelEntry _midiChannels[MILES_MIDI_CHANNEL_COUNT];

	byte _patchesBank[MILES_MT32_PATCH_COUNT];
};

MidiDriver_Miles_MT32::MidiDriver_Miles_MT32() {
	_driver = NULL;
	_isOpen = false;
	_MT32 = false;
	_nativeMT32 = false;
	_baseFreq = 250;

	memset(_patchesBank, 0, sizeof(_patchesBank));
}

MidiDriver_Miles_MT32::~MidiDriver_Miles_MT32() {
	Common::StackLock lock(_mutex);
	if (_driver) {
		_driver->setTimerCallback(0, 0);
		_driver->close();
		delete _driver;
	}
	_driver = NULL;
}

int MidiDriver_Miles_MT32::open() {
	assert(!_driver);

	// Setup midi driver
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_PREFER_MT32);
	MusicType musicType = MidiDriver::getMusicType(dev);

	switch (musicType) {
	case MT_MT32:
		_nativeMT32 = true;
		break;
	case MT_GM:
		if (ConfMan.getBool("native_mt32")) {
			_nativeMT32 = true;
		}
		break;
	default:
		break;
	}

	if (!_nativeMT32) {
		error("MILES-MT32: non-mt32 currently not supported!");
	}

	_driver = MidiDriver::createMidi(dev);
	if (!_driver)
		return 255;

	if (_nativeMT32)
		_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	int ret = _driver->open();
	if (ret)
		return ret;

	if (_nativeMT32) {
		_driver->sendMT32Reset();

		resetMT32();
	}

	return 0;
}

void MidiDriver_Miles_MT32::close() {
	if (_driver) {
		_driver->close();
	}
}

void MidiDriver_Miles_MT32::resetMT32() {
	// reset all internal parameters / patches
	MT32SysEx(0x7F0000, milesMT32SysExResetParameters);

	// init part/channel assignments
	MT32SysEx(0x10000D, milesMT32SysExChansSetup);

	// partial reserve table
	MT32SysEx(0x100004, milesMT32SysExPartialReserveTable);

	// init reverb
	MT32SysEx(0x100001, milesMT32SysExInitReverb);
}

void MidiDriver_Miles_MT32::MT32SysEx(const uint32 targetAddress, const byte *dataPtr) {
	byte   sysExMessage[270];
	uint16 sysExPos      = 0;
	byte   sysExByte     = 0;
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

	sysExChecksum -= sysExMessage[4];
	sysExChecksum -= sysExMessage[5];
	sysExChecksum -= sysExMessage[6];

	sysExPos      = 7;
	while (1) {
		sysExByte = *dataPtr++;
		if (sysExByte == 0xff)
			break; // Message done

		assert(sysExPos < sizeof(sysExMessage));
		sysExMessage[sysExPos++] = sysExByte;
		sysExChecksum -= sysExByte;
	}

	// Calculate checksum
	assert(sysExPos < sizeof(sysExMessage));
	sysExMessage[sysExPos++] = sysExChecksum & 0x7f;

	// Send SysEx
	_driver->sysEx(sysExMessage, sysExPos);

	// Wait the time it takes to send the SysEx data
	uint32 delay = (sysExPos + 2) * 1000 / 3125;

	// Plus an additional delay for the MT-32 rev00
	if (_nativeMT32)
		delay += 40;

	g_system->delayMillis(delay);
}

// MIDI messages can be found at http://www.midi.org/techspecs/midimessages.php
void MidiDriver_Miles_MT32::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	switch (command) {
	case 0x80: // note off
	case 0x90: // note on
	case 0xe0: // pitch bend change
		_driver->send(b);
		break;
	case 0xb0: // Control change
		controlChange(channel, op1, op2);
		break;
	case 0xc0: // Program Change
		programChange(channel, op1);
		break;
	case 0xa0: // Polyphonic key pressure (aftertouch)
	case 0xd0: // Channel pressure (aftertouch)
		// Aftertouch doesn't seem to be implemented in the Sherlock Holmes adlib driver
		break;
	case 0xf0: // SysEx
		warning("MILES-MT32: SysEx: %x", b);
		break;
	default:
		warning("MILES-MT32: Unknown event %02x", command);
	}
}

void MidiDriver_Miles_MT32::controlChange(byte midiChannel, byte controllerNumber, byte controllerValue) {
	byte channelPatchId = 0;

	switch (controllerNumber) {
	case MILES_CONTROLLER_SELECT_PATCH_BANK:
		_midiChannels[midiChannel].currentPatchBank = controllerValue;
		return;

	case MILES_CONTROLLER_PATCH_REVERB:
		channelPatchId = _midiChannels[midiChannel].currentPatchId;

		writePatchByte(channelPatchId, 6, controllerValue);
		_driver->send(0xC0 | midiChannel | (channelPatchId << 8)); // execute program change
		return;

	case MILES_CONTROLLER_PATCH_BENDER:
		channelPatchId = _midiChannels[midiChannel].currentPatchId;

		writePatchByte(channelPatchId, 4, controllerValue);
		_driver->send(0xC0 | midiChannel | (channelPatchId << 8)); // execute program change
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
		// uses .MT data, cannot implement atm
		return;

	case MILES_CONTROLLER_PROTECT_TIMBRE:
		// timbre .MT data, cannot implement atm
		return;

	default:
		break;
	}

	if ((controllerNumber >= MILES_CONTROLLER_SYSEX_RANGE_BEGIN) && (controllerNumber <= MILES_CONTROLLER_SYSEX_RANGE_END)) {
		// send SysEx
		warning("MILES-MT32: embedded SysEx controller %2x, value %2x", controllerNumber, controllerValue);
		return;
	}

	if ((controllerNumber >= MILES_CONTROLLER_XMIDI_RANGE_BEGIN) && (controllerNumber <= MILES_CONTROLLER_XMIDI_RANGE_END)) {
		// XMIDI controllers? ignore those
		return;
	}

	_driver->send(0xB0 | midiChannel | (controllerNumber << 8) | (controllerValue << 16));
}

void MidiDriver_Miles_MT32::programChange(byte midiChannel, byte patchId) {
	byte channelPatchBank = _midiChannels[midiChannel].currentPatchBank;
	byte activePatchBank = _patchesBank[patchId];

	// remember patch id for the current MIDI-channel
	_midiChannels[midiChannel].currentPatchId = patchId;

	if (channelPatchBank != activePatchBank) {
		// associate patch with timbre
		setupPatch(patchId, channelPatchBank);
		warning("setup patch");
	}

	// Search timbre and remember it (only used when timbre file is available)
	// TODO

	// Finally send to MT32
	_driver->send(0xC0 | midiChannel | (patchId << 8));
}

void MidiDriver_Miles_MT32::setupPatch(byte patchId, byte patchBank) {
	byte timbreId = 0;

	_patchesBank[patchId] = patchBank;

	if (patchBank) {
		// non-built-in bank
		// TODO: search timbre
	}

	// for built-in bank (or timbres, that are not available) use default MT32 timbres
	timbreId = patchId & 0x3F;
	if (!(patchId & 0x40)) {
		writePatchTimbre(patchId, 0, timbreId); // Group A
	} else {
		writePatchTimbre(patchId, 1, timbreId); // Group B
	}
}

void MidiDriver_Miles_MT32::writePatchTimbre(byte patchId, byte timbreGroup, byte timbreId) {
	byte   sysExData[3];
	uint32 targetAddress = 0;

	targetAddress = ((patchId << 3) << 16) | 0x000500;

	sysExData[0] = timbreGroup;
	sysExData[1] = timbreId;
	sysExData[2] = 0xFF; // terminator

	MT32SysEx(targetAddress, sysExData);
}

void MidiDriver_Miles_MT32::writePatchByte(byte patchId, byte index, byte patchValue) {
	byte   sysExData[2];
	uint32 targetAddress = 0;

	targetAddress = (((patchId << 3) + index ) << 16) | 0x000500;

	sysExData[0] = patchValue;
	sysExData[1] = 0xFF; // terminator

	MT32SysEx(targetAddress, sysExData);
}

void MidiDriver_Miles_MT32::writeToSystemArea(byte index, byte value) {
	byte   sysExData[2];
	uint32 targetAddress = 0;

	targetAddress = 0x100000 | index;

	sysExData[0] = value;
	sysExData[1] = 0xFF; // terminator

	MT32SysEx(targetAddress, sysExData);
}

MidiDriver *MidiDriver_Miles_MT32_create(const Common::String instrumentDataFilename) {
	// For some games there are timbre files called [something].MT
	// Sherlock Holmes 2 doesn't have one of those
	// so I can't implement them
	return new MidiDriver_Miles_MT32();
}

} // End of namespace Sherlock
