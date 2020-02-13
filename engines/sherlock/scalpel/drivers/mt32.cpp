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
#include "sherlock/scalpel/drivers/mididriver.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

//#include "audio/mididrv.h"

namespace Sherlock {

#define SHERLOCK_MT32_CHANNEL_COUNT 16

const byte mt32ReverbDataSysEx[] = {
	0x10, 0x00, 0x01, 0x01, 0x05, 0x05, 0xFF
};

class MidiDriver_MT32 : public MidiDriver {
public:
	MidiDriver_MT32() {
		_driver = NULL;
		_isOpen = false;
		_nativeMT32 = false;
		_baseFreq = 250;

		memset(_MIDIchannelActive, 1, sizeof(_MIDIchannelActive));
	}
	~MidiDriver_MT32() override;

	// MidiDriver
	int open() override;
	void close() override;
	bool isOpen() const override { return _isOpen; }

	void send(uint32 b) override;

	void newMusicData(byte *musicData, int32 musicDataSize);

	MidiChannel *allocateChannel() override {
		if (_driver)
			return _driver->allocateChannel();
		return NULL;
	}
	MidiChannel *getPercussionChannel() override {
		if (_driver)
			return _driver->getPercussionChannel();
		return NULL;
	}

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override {
		if (_driver)
			_driver->setTimerCallback(timer_param, timer_proc);
	}

	uint32 getBaseTempo() override {
		if (_driver) {
			return _driver->getBaseTempo();
		}
		return 1000000 / _baseFreq;
	}

protected:
	Common::Mutex _mutex;
	MidiDriver *_driver;
	bool _nativeMT32;

	bool _isOpen;
	int _baseFreq;

private:
	// points to a MIDI channel for each of the new voice channels
	byte _MIDIchannelActive[SHERLOCK_MT32_CHANNEL_COUNT];

public:
	void uploadMT32Patches(byte *driverData, int32 driverSize);

	void mt32SysEx(const byte *&dataPtr, int32 &bytesLeft);
};

MidiDriver_MT32::~MidiDriver_MT32() {
	Common::StackLock lock(_mutex);
	if (_driver) {
		_driver->setTimerCallback(0, 0);
		_driver->close();
		delete _driver;
	}
	_driver = NULL;
}

int MidiDriver_MT32::open() {
	assert(!_driver);

	debugC(kDebugLevelMT32Driver, "MT32: starting driver");

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

	_driver = MidiDriver::createMidi(dev);
	if (!_driver)
		return 255;

	if (_nativeMT32)
		_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	int ret = _driver->open();
	if (ret)
		return ret;

	if (_nativeMT32)
		_driver->sendMT32Reset();
	else
		_driver->sendGMReset();

	return 0;
}

void MidiDriver_MT32::close() {
	if (_driver) {
		_driver->close();
	}
}

// Called when a music track got loaded into memory
void MidiDriver_MT32::newMusicData(byte *musicData, int32 musicDataSize) {
	assert(musicDataSize >= 0x7F); // Security check

	// MIDI Channel Enable/Disable bytes at offset 0x2 of music data
	memcpy(&_MIDIchannelActive, musicData + 0x2, SHERLOCK_MT32_CHANNEL_COUNT);

	// Send 16 bytes from offset 0x12 to MT32
	// All the music tracks of Sherlock seem to contain dummy data
	// probably a feature, that was used in the game "Ski or Die"
	// that's why we don't implement this

	// Also send these bytes to MT32 (SysEx) - seems to be reverb configuration
	if (_nativeMT32) {
		const byte *reverbData = mt32ReverbDataSysEx;
		int32       reverbDataSize = sizeof(mt32ReverbDataSysEx);
		mt32SysEx(reverbData, reverbDataSize);
	}
}

void MidiDriver_MT32::uploadMT32Patches(byte *driverData, int32 driverSize) {
	if (!_driver)
		return;

	if (!_nativeMT32)
		return;

	// patch data starts at offset 0x863
	assert(driverSize == 0x13B9); // Security check
	assert(driverData[0x863] == 0x7F); // another security check

	const byte *patchPtr  = driverData + 0x863;
	int32       bytesLeft = driverSize - 0x863;

	while(1) {
		mt32SysEx(patchPtr, bytesLeft);

		assert(bytesLeft);
		if (*patchPtr == 0x80) // List terminator
			break;
	}
}

void MidiDriver_MT32::mt32SysEx(const byte *&dataPtr, int32 &bytesLeft) {
	byte   sysExMessage[270];
	uint16 sysExPos      = 0;
	byte   sysExByte     = 0;
	uint16 sysExChecksum = 0;

	memset(&sysExMessage, 0, sizeof(sysExMessage));

	sysExMessage[0] = 0x41; // Roland
	sysExMessage[1] = 0x10;
	sysExMessage[2] = 0x16; // Model MT32
	sysExMessage[3] = 0x12; // Command DT1

	sysExPos      = 4;
	sysExChecksum = 0;
	while (1) {
		assert(bytesLeft);

		sysExByte = *dataPtr++;
		bytesLeft--;
		if (sysExByte == 0xff)
			break; // Message done

		assert(sysExPos < sizeof(sysExMessage));
		sysExMessage[sysExPos++] = sysExByte;
		sysExChecksum -= sysExByte;
	}

	// Calculate checksum
	assert(sysExPos < sizeof(sysExMessage));
	sysExMessage[sysExPos++] = sysExChecksum & 0x7f;

	debugC(kDebugLevelMT32Driver, "MT32: uploading patch data, size %d", sysExPos);

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
void MidiDriver_MT32::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;

	if (command == 0xF0) {
		if (_driver) {
			_driver->send(b);
		}
		return;
	}

	if (_MIDIchannelActive[channel]) {
		// Only forward MIDI-data in case the channel is currently enabled via music-data
		if (_driver) {
			_driver->send(b);
		}
	}
}

MidiDriver *MidiDriver_MT32_create() {
	return new MidiDriver_MT32();
}

void MidiDriver_MT32_newMusicData(MidiDriver *driver, byte *musicData, int32 musicDataSize) {
	static_cast<MidiDriver_MT32 *>(driver)->newMusicData(musicData, musicDataSize);
}

void MidiDriver_MT32_uploadPatches(MidiDriver *driver, byte *driverData, int32 driverSize) {
	static_cast<MidiDriver_MT32 *>(driver)->uploadMT32Patches(driverData, driverSize);
}

} // End of namespace Sherlock
