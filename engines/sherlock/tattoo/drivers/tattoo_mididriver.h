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

#ifndef SHERLOCK_TATTOO_DRIVERS_MIDIDRIVER_H
#define SHERLOCK_TATTOO_DRIVERS_MIDIDRIVER_H

#include "sherlock/sherlock.h"
#include "audio/mididrv.h"
#include "common/error.h"

namespace Sherlock {

#define MILES_MIDI_CHANNEL_COUNT 16

// Miles Audio supported controllers for control change messages
#define MILES_CONTROLLER_SELECT_PATCH_BANK 114
#define MILES_CONTROLLER_PROTECT_VOICE 112
#define MILES_CONTROLLER_PROTECT_TIMBRE 113
#define MILES_CONTROLLER_MODULATION 1
#define MILES_CONTROLLER_VOLUME 7
#define MILES_CONTROLLER_EXPRESSION 11
#define MILES_CONTROLLER_PANNING 10
#define MILES_CONTROLLER_SUSTAIN 64
#define MILES_CONTROLLER_PITCH_RANGE 6
#define MILES_CONTROLLER_RESET_ALL 121
#define MILES_CONTROLLER_ALL_NOTES_OFF 123
#define MILES_CONTROLLER_PATCH_REVERB 59
#define MILES_CONTROLLER_PATCH_BENDER 60
#define MILES_CONTROLLER_REVERB_MODE 61
#define MILES_CONTROLLER_REVERB_TIME 62
#define MILES_CONTROLLER_REVERB_LEVEL 63
#define MILES_CONTROLLER_RHYTHM_KEY_TIMBRE 58

// 3 SysEx controllers, each range 14
#define MILES_CONTROLLER_SYSEX_RANGE_BEGIN 32
#define MILES_CONTROLLER_SYSEX_RANGE_END 64

#define MILES_CONTROLLER_XMIDI_RANGE_BEGIN 110
#define MILES_CONTROLLER_XMIDI_RANGE_END 120

// Miles Audio actually used 0x4000, because they didn't shift the 2 bytes properly
#define MILES_PITCHBENDER_DEFAULT 0x2000

extern MidiDriver *MidiDriver_Miles_AdLib_create(const Common::String instrumentDataFilename, const Common::String instrumentDataFilenameOPL3);

extern MidiDriver *MidiDriver_Miles_MT32_create(const Common::String instrumentDataFilename);

} // End of namespace Sherlock

#endif // SHERLOCK_TATTOO_DRIVERS_MIDIDRIVER_H
