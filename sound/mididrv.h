/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SOUND_MIDIDRV_H
#define SOUND_MIDIDRV_H

#include "common/scummsys.h"

class MidiChannel;
class SoundMixer;

// Abstract MIDI Driver Class
class MidiDriver {
public:
	virtual ~MidiDriver() { }

	// Error codes returned by open.
	// Can be converted to a string with getErrorName()
	enum {
		MERR_CANNOT_CONNECT = 1,
//		MERR_STREAMING_NOT_AVAILABLE = 2,
		MERR_DEVICE_NOT_AVAILABLE = 3,
		MERR_ALREADY_OPEN = 4
	};

	enum {
//		PROP_TIMEDIV = 1,
		PROP_OLD_ADLIB = 2
	};

	// Open the midi driver.
	// Returns 0 if successful, otherwise an error code.
	virtual int open() = 0;

	// Close the midi driver
	virtual void close() = 0;

	// Output a packed midi command to the midi stream
	virtual void send(uint32 b) = 0;

	// Get or set a property
	virtual uint32 property(int prop, uint32 param) { return 0; }

	// Retrieve a string representation of an error code
	static const char *getErrorName(int error_code);

	// HIGH-LEVEL SEMANTIC METHODS
	virtual void setPitchBendRange (byte channel, uint range)
	{
		send((  0   << 16) | (101 << 8) | (0xB0 | channel));
		send((  0   << 16) | (100 << 8) | (0xB0 | channel));
		send((range << 16) | (  6 << 8) | (0xB0 | channel));
		send((  0   << 16) | ( 38 << 8) | (0xB0 | channel));
	}

	virtual void sysEx (byte *msg, uint16 length) { }
	virtual void sysEx_customInstrument (byte channel, uint32 type, byte *instr) { }
	virtual void metaEvent (byte type, byte*data, uint16 length) { }

	// Timing functions - MidiDriver now operates timers
	virtual void setTimerCallback (void *timer_param, void (*timer_proc) (void *)) = 0;
	virtual uint32 getBaseTempo (void) = 0;

	// Channel allocation functions
	virtual MidiChannel *allocateChannel() = 0;
	virtual MidiChannel *getPercussionChannel() = 0;
};

class MidiChannel {
public:
	virtual MidiDriver *device() = 0;
	virtual byte getNumber() = 0;
	virtual void release() = 0;

	// Regular messages
	virtual void noteOff (byte note) = 0;
	virtual void noteOn (byte note, byte velocity) = 0;
	virtual void programChange (byte program) = 0;
	virtual void pitchBend (int16 bend) = 0; // -0x2000 to +0x1FFF

	// Control Change messages
	virtual void controlChange (byte control, byte value) = 0;
	virtual void modulationWheel (byte value) = 0;
	virtual void volume (byte value) = 0;
	virtual void panPosition (byte value) = 0;
	virtual void pitchBendFactor (byte value) = 0;
	virtual void detune (byte value) = 0;
	virtual void priority (byte value) = 0;
	virtual void sustain (bool value) = 0;
	virtual void effectLevel (byte value) = 0;
	virtual void chorusLevel (byte value) = 0;
	virtual void allNotesOff() = 0;

	// SysEx messages
	virtual void sysEx_customInstrument (uint32 type, byte *instr) = 0;
};

// MIDI Driver Types
enum {
	MD_AUTO = 0,
	MD_NULL = 1,
	MD_WINDOWS = 2,
	MD_TIMIDITY = 3,
	MD_SEQ = 4,
	MD_QTMUSIC = 5,
	MD_ETUDE = 6,
	MD_COREAUDIO = 7,
	MD_MIDIEMU = 8,
	MD_ALSA = 9,
	MD_ADLIB = 10,
	
	MD_YPA1 = 100	// palmos
};

// Factory functions, for faster compile
extern MidiDriver *MidiDriver_NULL_create();
extern MidiDriver *MidiDriver_ADLIB_create(SoundMixer *mixer);
extern MidiDriver *MidiDriver_WIN_create();
extern MidiDriver *MidiDriver_SEQ_create();
extern MidiDriver *MidiDriver_QT_create();
extern MidiDriver *MidiDriver_CORE_create();
extern MidiDriver *MidiDriver_ETUDE_create();
extern MidiDriver *MidiDriver_ALSA_create();
extern MidiDriver *MidiDriver_YamahaPa1_create();

#endif
