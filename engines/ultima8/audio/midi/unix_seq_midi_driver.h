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

#ifndef ULTIMA8_AUDIO_MIDI_UNIXSEQMIDIDRIVER_H
#define ULTIMA8_AUDIO_MIDI_UNIXSEQMIDIDRIVER_H

#ifdef TODO
#if (defined(UNIX) || defined(__unix__))
#define USE_UNIX_SEQ_MIDI

#include "ultima8/audio/midi/low_level_midi_driver.h"
#include "ultima8/std/string.h"

namespace Ultima8 {

class UnixSeqMidiDriver : public LowLevelMidiDriver {
	const static MidiDriverDesc desc;
	static MidiDriver *createInstance() {
		return new UnixSeqMidiDriver();
	}

public:
	static const MidiDriverDesc *getDesc() {
		return &desc;
	}
	UnixSeqMidiDriver();

protected:
	virtual int         open();
	virtual void        close();
	virtual void        send(uint32 message);
//	virtual void     yield();
	virtual void        send_sysex(uint8 status, const uint8 *msg,
	                               uint16 length);

	std::string devname;
	bool isOpen;
	int device, deviceNum;
};

} // End of namespace Ultima8

#endif
#endif

#endif
