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

#include "agos/drivers/accolade/casio.h"

#include "agos/drivers/accolade/mididriver.h"

namespace AGOS {

MidiDriver_Accolade_Casio::MidiDriver_Accolade_Casio() : MidiDriver_Casio(MT_CT460) {
	Common::fill(_channelRemapping, _channelRemapping + ARRAYSIZE(_channelRemapping), 0);
	Common::fill(_instrumentRemappingData, _instrumentRemappingData + ARRAYSIZE(_instrumentRemappingData), 0);
	Common::fill(_rhythmNoteRemappingData, _rhythmNoteRemappingData + ARRAYSIZE(_rhythmNoteRemappingData), 0);
}

int MidiDriver_Accolade_Casio::open() {
	int result = MidiDriver_Casio::open();

	// Apply instrument and rhythm note remapping after device initialization.
	_instrumentRemapping = _instrumentRemappingData;
	// WORKAROUND The MT-32 and Casio devices use a different mapping of notes
	// to instruments on the rhythm channel. The INSTR.DAT file of Elvira 1
	// contains a remapping of the rhythm notes in the game's MT-32 MIDI data
	// to the Casio's rhythm notes, but this does not seem to be used - the
	// game outputs the MT-32 rhythm notes when CT-460 is selected. As a result
	// the wrong rhythm instruments are played. This is fixed here by using the
	// game's remapping data to properly remap the rhythm notes to the notes
	// that the Casio devices use.
	_rhythmNoteRemapping = _rhythmNoteRemappingData;

	return result;
}

int8 MidiDriver_Accolade_Casio::mapSourceChannel(uint8 source, uint8 dataChannel) {
	if (!_isOpen)
		// Use 1 on 1 mapping during device initialization.
		return dataChannel;

	return _channelRemapping[dataChannel];
}

void MidiDriver_Accolade_Casio::readDriverData(byte *driverData, uint16 driverDataSize) {
	uint16 minDataSize = 354;
	if (driverDataSize < minDataSize)
		error("MidiDriver_Accolade_Casio::readDriverData - Expected minimum driver data size of %d - got %d", minDataSize, driverDataSize);

	// INSTR.DAT Data is like this:
	// 128 bytes  instrument mapping
	// 128 bytes  instrument volume adjust (signed!) (not used for Casio)
	//  16 bytes  unknown
	//  16 bytes  channel mapping
	//  64 bytes  key note mapping
	//   1 byte   instrument count
	//   1 byte   bytes per instrument
	//   x bytes  no instruments used for Casio

	uint16 channelMappingOffset = 256 + 16;
	Common::copy(driverData + channelMappingOffset, driverData + channelMappingOffset + ARRAYSIZE(_channelRemapping), _channelRemapping);

	uint16 instrumentMappingOffset = 0;
	Common::copy(driverData + instrumentMappingOffset, driverData + instrumentMappingOffset + ARRAYSIZE(_instrumentRemappingData), _instrumentRemappingData);

	uint16 rhythmNoteMappingOffset = 256 + 32;
	// 64 bytes are reserved for the rhythm note mapping, but only 40 seem to
	// be used. The first mapping is for note 0x24.
	Common::copy(driverData + rhythmNoteMappingOffset, driverData + rhythmNoteMappingOffset + 40, _rhythmNoteRemappingData + 0x24);
}

MidiDriver_Multisource *MidiDriver_Accolade_Casio_create(Common::String driverFilename) {
	byte *driverData = nullptr;
	uint16 driverDataSize = 0;
	bool newVersion = false;

	MidiDriver_Accolade_readDriver(driverFilename, MT_CT460, driverData, driverDataSize, newVersion);
	if (!driverData)
		error("MidiDriver_Accolade_Casio_create - Error during readDriver()");

	if (newVersion)
		// Only Elvira 1 has support for Casio and this uses the old drivers.
		error("MidiDriver_Accolade_Casio_create - Driver not supported for Elvira 2 / Waxworks / Simon 1 demo");

	MidiDriver_Accolade_Casio *driver = new MidiDriver_Accolade_Casio();
	if (!driver)
		error("MidiDriver_Accolade_Casio_create - Could not create driver");

	driver->readDriverData(driverData, driverDataSize);

	delete[] driverData;
	return driver;
}

} // End of namespace AGOS
