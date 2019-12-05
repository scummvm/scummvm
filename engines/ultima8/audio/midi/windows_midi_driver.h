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

#ifndef ULTIMA8_AUDIO_MIDI_WINDOWSMIDIDRIVER_H
#define ULTIMA8_AUDIO_MIDI_WINDOWSMIDIDRIVER_H

//if defined(WIN32) && !defined(UNDER_CE)
#ifdef USE_WINDOWS_MIDI
#define USE_WINDOWS_MIDI

#include "ultima8/audio/midi/low_level_midi_driver.h"

// Slight hack here. Uncomment it to enable the ability to use
// both A and B devices on an SB Live to distribute the notes
//#define WIN32_USE_DUAL_MIDIDRIVERS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

namespace Ultima8 {

class WindowsMidiDriver : public LowLevelMidiDriver {
	int          dev_num;
	HMIDIOUT            midi_port;
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	HMIDIOUT            midi_port2;
#endif

	// SysEx stuff. Borrowed from ScummVM
	MIDIHDR _streamHeader;
	uint8 *_streamBuffer;
	int     _streamBufferSize;
	HANDLE _streamEvent;

	const static MidiDriverDesc desc;
	static MidiDriver *createInstance() {
		return new WindowsMidiDriver();
	}

	static bool         doMCIError(MMRESULT res);

public:
	const static MidiDriverDesc *getDesc() {
		return &desc;
	}
	WindowsMidiDriver();

protected:
	virtual int         open();
	virtual void        close();
	virtual void        send(uint32 message);
	virtual void        send_sysex(uint8 status, const uint8 *msg, uint16 length);
	virtual void        increaseThreadPriority();
	virtual void        yield();
};

} // End of namespace Ultima8

#endif //WIN32

#endif
