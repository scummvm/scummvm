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

#include "agos/drivers/accolade/mididriver.h"
#include "agos/drivers/accolade/mt32.h"

#include "audio/mididrv.h"

#include "common/config-manager.h"

namespace AGOS {

MidiDriver_Accolade_MT32::MidiDriver_Accolade_MT32() {
	_driver = NULL;
	_isOpen = false;
	_nativeMT32 = false;
	_baseFreq = 250;

	memset(_channelMapping, 0, sizeof(_channelMapping));
	memset(_instrumentMapping, 0, sizeof(_instrumentMapping));
}

MidiDriver_Accolade_MT32::~MidiDriver_Accolade_MT32() {
	Common::StackLock lock(_mutex);
	if (_driver) {
		_driver->setTimerCallback(0, 0);
		_driver->close();
		delete _driver;
	}
	_driver = NULL;
}

int MidiDriver_Accolade_MT32::open() {
	assert(!_driver);

//	debugC(kDebugLevelMT32Driver, "MT32: starting driver");

	// Setup midi driver
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_PREFER_MT32);
	MusicType musicType = MidiDriver::getMusicType(dev);

	// check, if we got a real MT32 (or MUNT, or MUNT over MIDI)
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

	int ret = _driver->open();
	if (ret)
		return ret;

	if (_nativeMT32)
		_driver->sendMT32Reset();
	else
		_driver->sendGMReset();

	return 0;
}

void MidiDriver_Accolade_MT32::close() {
	if (_driver) {
		_driver->close();
	}
}

// MIDI messages can be found at http://www.midi.org/techspecs/midimessages.php
void MidiDriver_Accolade_MT32::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;

	if (command == 0xF0) {
		if (_driver) {
			_driver->send(b);
		}
		return;
	}

	byte mappedChannel = _channelMapping[channel];

	if (mappedChannel < AGOS_MIDI_CHANNEL_COUNT) {
		// channel mapped to an actual MIDI channel, so use that one
		b = (b & 0xFFFFFFF0) | mappedChannel;
		if (command == 0xC0) {
			// Program change
			// Figure out the requested instrument
			byte midiInstrument = (b >> 8) & 0xFF;
			byte mappedInstrument = _instrumentMapping[midiInstrument];

			// If there is no actual MT32 (or MUNT), we make a second mapping to General MIDI instruments
			if (!_nativeMT32) {
				mappedInstrument = (MidiDriver::_mt32ToGm[mappedInstrument]);
			}
			// And replace it
			b = (b & 0xFFFF00FF) | (mappedInstrument << 8);
		}

		if (_driver) {
			_driver->send(b);
		}
	}
}

// Called right at the start, we get an INSTR.DAT entry
bool MidiDriver_Accolade_MT32::setupInstruments(byte *driverData, uint16 driverDataSize, bool useMusicDrvFile) {
	uint16 channelMappingOffset = 0;
	uint16 channelMappingSize = 0;
	uint16 instrumentMappingOffset = 0;
	uint16 instrumentMappingSize = 0;

	if (!useMusicDrvFile) {
		// INSTR.DAT: we expect at least 354 bytes
		if (driverDataSize < 354)
			return false;

		// Data is like this:
		// 128 bytes  instrument mapping
		// 128 bytes  instrument volume adjust (signed!) (not used for MT32)
		//  16 bytes  unknown
		//  16 bytes  channel mapping
		//  64 bytes  key note mapping (not really used for MT32)
		//   1 byte   instrument count
		//   1 byte   bytes per instrument
		//   x bytes  no instruments used for MT32

		channelMappingOffset    = 256 + 16;
		channelMappingSize      = 16;
		instrumentMappingOffset = 0;
		instrumentMappingSize   = 128;

	} else {
		// MUSIC.DRV: we expect at least 468 bytes
		if (driverDataSize < 468)
			return false;

		channelMappingOffset    = 396;
		channelMappingSize      = 16;
		instrumentMappingOffset = 140;
		instrumentMappingSize   = 128;
	}

	// Channel mapping
	if (channelMappingSize) {
		// Get these 16 bytes for MIDI channel mapping
		if (channelMappingSize != sizeof(_channelMapping))
			return false;

		memcpy(_channelMapping, driverData + channelMappingOffset, sizeof(_channelMapping));
	} else {
		// Set up straight mapping
		for (uint16 channelNr = 0; channelNr < sizeof(_channelMapping); channelNr++) {
			_channelMapping[channelNr] = channelNr;
		}
	}

	if (instrumentMappingSize) {
		// And these for instrument mapping
		if (instrumentMappingSize > sizeof(_instrumentMapping))
			return false;

		memcpy(_instrumentMapping, driverData + instrumentMappingOffset, instrumentMappingSize);
	}
	// Set up straight mapping for the remaining data
	for (uint16 instrumentNr = instrumentMappingSize; instrumentNr < sizeof(_instrumentMapping); instrumentNr++) {
		_instrumentMapping[instrumentNr] = instrumentNr;
	}
	return true;
}

MidiDriver *MidiDriver_Accolade_MT32_create(Common::String driverFilename) {
	byte  *driverData = NULL;
	uint16 driverDataSize = 0;
	bool   isMusicDrvFile = false;

	MidiDriver_Accolade_readDriver(driverFilename, MT_MT32, driverData, driverDataSize, isMusicDrvFile);
	if (!driverData)
		error("ACCOLADE-ADLIB: error during readDriver()");

	MidiDriver_Accolade_MT32 *driver = new MidiDriver_Accolade_MT32();
	if (driver) {
		if (!driver->setupInstruments(driverData, driverDataSize, isMusicDrvFile)) {
			delete driver;
			driver = nullptr;
		}
	}

	delete[] driverData;
	return driver;
}

} // End of namespace AGOS
