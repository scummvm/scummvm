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

#ifndef SHERLOCK_SCALPEL_DRIVERS_MIDIDRIVER_H
#define SHERLOCK_SCALPEL_DRIVERS_MIDIDRIVER_H

#include "sherlock/sherlock.h"
#include "audio/mididrv.h"
#include "common/error.h"

namespace Sherlock {

extern MidiDriver *MidiDriver_SH_AdLib_create();
extern void MidiDriver_SH_AdLib_newMusicData(MidiDriver *driver, byte *musicData, int32 musicDataSize);

extern MidiDriver *MidiDriver_MT32_create();
extern void MidiDriver_MT32_uploadPatches(MidiDriver *driver, byte *driverData, int32 driverSize);
extern void MidiDriver_MT32_newMusicData(MidiDriver *driver, byte *musicData, int32 musicDataSize);

} // End of namespace Sherlock

#endif // SHERLOCK_SCALPEL_DRIVERS_MIDIDRIVER_H
