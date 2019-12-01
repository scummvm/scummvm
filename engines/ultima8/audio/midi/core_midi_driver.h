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

#ifndef ULTIMA8_AUDIO_MIDI_COREMIDIDRIVER_H
#define ULTIMA8_AUDIO_MIDI_COREMIDIDRIVER_H

#ifdef MACOSX
#define USE_CORE_MIDI

#include "LowLevelMidiDriver.h"

#include <CoreMIDI/CoreMIDI.h>

namespace Ultima8 {

class CoreMidiDriver : public LowLevelMidiDriver {

	MIDIClientRef   mClient;
	MIDIPortRef     mOutPort;
	MIDIEndpointRef mDest;

	static const MidiDriverDesc desc;
	static MidiDriver *createInstance() {
		return new CoreMidiDriver();
	}

public:
	static const MidiDriverDesc *getDesc() {
		return &desc;
	}

	CoreMidiDriver();
	~CoreMidiDriver();

protected:
	virtual int         open();
	virtual void        close();
	virtual void        send(uint32 message);
	virtual void        send_sysex(uint8 status, const uint8 *msg, uint16 length);
	virtual void        increaseThreadPriority();
	virtual void        yield();
};

} // End of namespace Ultima8

#endif //MACOSX

#endif
