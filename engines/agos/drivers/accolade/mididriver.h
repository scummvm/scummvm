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

#ifndef AGOS_DRIVERS_ACCOLADE_MIDIDRIVER_H
#define AGOS_DRIVERS_ACCOLADE_MIDIDRIVER_H

#include "agos/agos.h"

#include "audio/fmopl.h"
#include "audio/mididrv.h"
#include "audio/mididrv_ms.h"

#include "common/error.h"

namespace AGOS {

#define AGOS_MIDI_CHANNEL_COUNT 16
#define AGOS_MIDI_INSTRUMENT_COUNT 128

#define AGOS_MIDI_KEYNOTE_COUNT 64

extern void MidiDriver_Accolade_readDriver(Common::String filename, MusicType requestedDriverType, byte *&driverData, uint16 &driverDataSize, bool &isMusicDrvFile);

extern MidiDriver_Multisource *MidiDriver_Accolade_AdLib_create(Common::String driverFilename, OPL::Config::OplType oplType, int callbackFrequency = OPL::OPL::kDefaultCallbackFrequency);
extern MidiDriver_Multisource *MidiDriver_Accolade_MT32_create(Common::String driverFilename);
extern MidiDriver_Multisource *MidiDriver_Accolade_Casio_create(Common::String driverFilename);
extern MidiDriver *MidiDriverPC98_create(MidiDriver::DeviceHandle dev);

} // End of namespace AGOS

#endif // AGOS_DRIVERS_ACCOLADE_MIDIDRIVER_H
