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

#include "agos/drivers/accolade/mt32.h"

#include "agos/drivers/accolade/mididriver.h"
#include "agos/sfxparser_accolade.h"

namespace AGOS {

const uint8 MidiDriver_Accolade_MT32::SYSEX_INSTRUMENT_ASSIGNMENT[7] = { 0x02, 0x00, 0x18, 0x32, 0x01, 0x00, 0x01 };

MidiDriver_Accolade_MT32::MidiDriver_Accolade_MT32() : MidiDriver_MT32GM(MT_MT32) {
	Common::fill(_channelRemapping, _channelRemapping + ARRAYSIZE(_channelRemapping), 0);
	Common::fill(_instrumentRemapping, _instrumentRemapping + ARRAYSIZE(_instrumentRemapping), 0);
	Common::fill(_channelLocks, _channelLocks + ARRAYSIZE(_channelLocks), false);
}

int MidiDriver_Accolade_MT32::open(MidiDriver *driver, bool nativeMT32) {
	int result = MidiDriver_MT32GM::open(driver, nativeMT32);

	setInstrumentRemapping(_instrumentRemapping);

	return result;
}

void MidiDriver_Accolade_MT32::send(int8 source, uint32 b) {
	byte dataChannel = b & 0xf;
	int8 outputChannel = mapSourceChannel(source, dataChannel);

	MidiChannelControlData &controlData = *_controlData[outputChannel];

	// Check if this event is sent by a music source and the channel is locked
	// by an SFX source.
	bool channelLockedByOtherSource = _sources[source].type != SOURCE_TYPE_SFX && _channelLocks[outputChannel];

	processEvent(source, b, outputChannel, controlData, channelLockedByOtherSource);
}

int8 MidiDriver_Accolade_MT32::mapSourceChannel(uint8 source, uint8 dataChannel) {
	if (!_isOpen)
		// Use 1 on 1 mapping during device initialization.
		return dataChannel;

	if (_sources[source].type == SOURCE_TYPE_SFX) {
		// Use channels 7 and 8 for SFX (sources 1 and 2).
		uint8 sfxChannel =  9 - source;

		_allocationMutex.lock();

		if (!_channelLocks[sfxChannel]) {
			// Lock channel
			stopAllNotes(0xFF, sfxChannel);
			_channelLocks[sfxChannel] = true;
		}

		_allocationMutex.unlock();

		return sfxChannel;
	} else {
		return _channelRemapping[dataChannel];
	}
}

void MidiDriver_Accolade_MT32::deinitSource(uint8 source) {
	_allocationMutex.lock();

	if (_sources[source].type == SOURCE_TYPE_SFX) {
		for (int i = 0; i < MIDI_CHANNEL_COUNT; i++) {
			if (_controlData[i]->source == source) {
				// Restore the music instrument.
				programChange(i, _controlData[i]->program, 0, *_controlData[i], false);
				// Unlock the channel.
				_channelLocks[i] = false;
			}
		}
	}

	_allocationMutex.unlock();

	MidiDriver_MT32GM::deinitSource(source);
}

void MidiDriver_Accolade_MT32::loadSfxInstrument(uint8 source, byte *instrumentData) {
	if (!(source == 1 || source == 2)) {
		warning("MidiDriver_Accolade_MT32::loadSfxInstrument - unexpected source %d", source);
		return;
	}

	// Send the instrument data to the timbre memory (patch 1 or 2).
	uint32 address = (0x08 << 14) | (((source - 1) * 2) << 7);
	sysExMT32(instrumentData + 3, SfxParser_Accolade::INSTRUMENT_SIZE_MT32 - 3, address, true, true, source);

	// Allocate the new patch to instrument number 0x75 or 0x76.
	byte instrNum = SFX_PROGRAM_BASE + source - 1;
	address = (0x05 << 14) | instrNum << 3;
	byte instrAssignData[7];
	Common::copy(SYSEX_INSTRUMENT_ASSIGNMENT, SYSEX_INSTRUMENT_ASSIGNMENT + ARRAYSIZE(instrAssignData), instrAssignData);
	instrAssignData[1] = source - 1;
	sysExMT32(instrAssignData, 7, address, true, true, source);
}

void MidiDriver_Accolade_MT32::changeSfxInstrument(uint8 source) {
	// Change to the newly loaded instrument.
	byte channel = mapSourceChannel(source, 0);
	MidiChannelControlData &controlData = *_controlData[channel];
	byte originalInstrument = controlData.program;
	programChange(channel, SFX_PROGRAM_BASE + source - 1, source, controlData);
	// Store the original instrument so it can be used when deinitializing
	// the source.
	controlData.program = originalInstrument;
}

void MidiDriver_Accolade_MT32::readDriverData(byte *driverData, uint16 driverDataSize, bool newVersion) {
	uint16 minDataSize = newVersion ? 468 : 354;
	if (driverDataSize < minDataSize)
		error("ACCOLADE-ADLIB: Expected minimum driver data size of %d - got %d", minDataSize, driverDataSize);

	// INSTR.DAT Data is like this:
	// 128 bytes  instrument mapping
	// 128 bytes  instrument volume adjust (signed!) (not used for MT32)
	//  16 bytes  unknown
	//  16 bytes  channel mapping
	//  64 bytes  key note mapping (not really used for MT32)
	//   1 byte   instrument count
	//   1 byte   bytes per instrument
	//   x bytes  no instruments used for MT32

	// music.drv is basically a driver, but with a few fixed locations for certain data

	uint16 channelMappingOffset = newVersion ? 396 : 256 + 16;
	Common::copy(driverData + channelMappingOffset, driverData + channelMappingOffset + ARRAYSIZE(_channelRemapping), _channelRemapping);

	uint16 instrumentMappingOffset = newVersion ? 140 : 0;
	Common::copy(driverData + instrumentMappingOffset, driverData + instrumentMappingOffset + ARRAYSIZE(_instrumentRemapping), _instrumentRemapping);
}

MidiDriver_Multisource *MidiDriver_Accolade_MT32_create(Common::String driverFilename) {
	byte *driverData = nullptr;
	uint16 driverDataSize = 0;
	bool newVersion = false;

	MidiDriver_Accolade_readDriver(driverFilename, MT_MT32, driverData, driverDataSize, newVersion);
	if (!driverData)
		error("ACCOLADE-MT32: error during readDriver()");

	MidiDriver_Accolade_MT32 *driver = new MidiDriver_Accolade_MT32();
	if (!driver)
		error("ACCOLADE-MT32: could not create driver");

	driver->readDriverData(driverData, driverDataSize, newVersion);

	delete[] driverData;
	return driver;
}

} // End of namespace AGOS
