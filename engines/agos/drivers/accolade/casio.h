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

#ifndef AGOS_DRIVERS_ACCOLADE_CASIO_H
#define AGOS_DRIVERS_ACCOLADE_CASIO_H

#include "audio/casio.h"

namespace AGOS {

class MidiDriver_Accolade_Casio : public MidiDriver_Casio {
public:
	MidiDriver_Accolade_Casio();

	int open() override;
	int8 mapSourceChannel(uint8 source, uint8 dataChannel) override;

	void readDriverData(byte *driverData, uint16 driverDataSize);

protected:
	// Mapping between MT-32 data MIDI channels and Casio channels.
	byte _channelRemapping[16];
	// Mapping between MT-32 data instruments and Casio instruments.
	byte _instrumentRemappingData[128];
	// Mapping between MT-32 data rhythm notes and Casio rhythm notes.
	byte _rhythmNoteRemappingData[128];
};

} // End of namespace AGOS

#endif
