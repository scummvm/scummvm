/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef SOUND_MIDIDRV_H
#define SOUND_MIDIDRV_H

#include "common/scummsys.h"
#include "common/timer.h"

class MidiChannel;
namespace Audio {
	class Mixer;
}
namespace Common { class String; }

/** MIDI Driver Types */
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
	MD_PCSPK = 11,
	MD_PCJR = 12,
	MD_TOWNS = 13,
	MD_YPA1 = 14, // PalmOS
	MD_ZODIAC = 15, // PalmOS
	MD_MT32 = 16,
	MD_FLUIDSYNTH = 17
};

enum MidiDriverType {
	MDT_NONE   = 0,
	MDT_PCSPK  = 1, // MD_PCSPK and MD_PCJR
	MDT_ADLIB  = 2, // MD_ADLIB
	MDT_TOWNS  = 4, // MD_TOWNS
	MDT_NATIVE = 8, // Everything else
	MDT_PREFER_NATIVE = 16
};

/**
 * Abstract description of a MIDI driver. Used by the config file and command
 * line parsing code, and also to be able to give the user a list of available
 * drivers.
 */
struct MidiDriverDescription {
	const char *name;
	const char *description;
	int id;
};

/** Abstract MIDI Driver Class */
class MidiDriver {
public:
	/** Convert a string containing a music driver name into MIDI Driver type. */
	static int parseMusicDriver(const Common::String &str);

	/**
	 * Get a list of all available MidiDriver types.
	 * @return list of all available midi drivers, terminated by  a zero entry
	 */
	static const MidiDriverDescription *getAvailableMidiDrivers();

	static MidiDriver *createMidi(int midiDriver);

	static int detectMusicDriver(int midiFlags);


public:
	virtual ~MidiDriver() { }

	static const byte _mt32ToGm[128];
	static const byte _gmToMt32[128];

	/**
	 * Error codes returned by open.
	 * Can be converted to a string with getErrorName().
	 */
	enum {
		MERR_CANNOT_CONNECT = 1,
//		MERR_STREAMING_NOT_AVAILABLE = 2,
		MERR_DEVICE_NOT_AVAILABLE = 3,
		MERR_ALREADY_OPEN = 4
	};

	enum {
//		PROP_TIMEDIV = 1,
		PROP_OLD_ADLIB = 2,
		PROP_CHANNEL_MASK = 3
	};

	/**
	 * Open the midi driver.
	 * @return 0 if successful, otherwise an error code.
	 */
	virtual int open() = 0;

	/** Close the midi driver. */
	virtual void close() = 0;

	/** Output a packed midi command to the midi stream. */
	virtual void send(uint32 b) = 0;

	/** Get or set a property. */
	virtual uint32 property(int prop, uint32 param) { return 0; }

	/** Retrieve a string representation of an error code. */
	static const char *getErrorName(int error_code);

	// HIGH-LEVEL SEMANTIC METHODS
	virtual void setPitchBendRange(byte channel, uint range)
	{
		send((  0   << 16) | (101 << 8) | (0xB0 | channel));
		send((  0   << 16) | (100 << 8) | (0xB0 | channel));
		send((range << 16) | (  6 << 8) | (0xB0 | channel));
		send((  0   << 16) | ( 38 << 8) | (0xB0 | channel));
		send(( 127  << 16) | (101 << 8) | (0xB0 | channel));
		send(( 127  << 16) | (100 << 8) | (0xB0 | channel));
	}

	virtual void sysEx(byte *msg, uint16 length) { }
	virtual void sysEx_customInstrument(byte channel, uint32 type, byte *instr) { }
	virtual void metaEvent(byte type, byte*data, uint16 length) { }

	// Timing functions - MidiDriver now operates timers
	virtual void setTimerCallback(void *timer_param, Common::Timer::TimerProc timer_proc) = 0;

	/** The time in microseconds between invocations of the timer callback. */
	virtual uint32 getBaseTempo(void) = 0;

	// Channel allocation functions
	virtual MidiChannel *allocateChannel() = 0;
	virtual MidiChannel *getPercussionChannel() = 0;
};

class MidiChannel {
public:
	virtual ~MidiChannel() {}

	virtual MidiDriver *device() = 0;
	virtual byte getNumber() = 0;
	virtual void release() = 0;

	virtual void send(uint32 b) = 0; // 4-bit channel portion is ignored

	// Regular messages
	virtual void noteOff(byte note) = 0;
	virtual void noteOn(byte note, byte velocity) = 0;
	virtual void programChange(byte program) = 0;
	virtual void pitchBend(int16 bend) = 0; // -0x2000 to +0x1FFF

	// Control Change messages
	virtual void controlChange(byte control, byte value) = 0;
	virtual void modulationWheel(byte value) { controlChange (1, value); }
	virtual void volume(byte value) { controlChange (7, value); }
	virtual void panPosition(byte value) { controlChange (10, value); }
	virtual void pitchBendFactor(byte value) = 0;
	virtual void detune(byte value) { controlChange (17, value); }
	virtual void priority(byte value) { }
	virtual void sustain(bool value) { controlChange (64, value ? 1 : 0); }
	virtual void effectLevel(byte value) { controlChange (91, value); }
	virtual void chorusLevel(byte value) { controlChange (93, value); }
	virtual void allNotesOff() { controlChange (123, 0); }

	// SysEx messages
	virtual void sysEx_customInstrument(uint32 type, byte *instr) = 0;
};


// Factory functions, for faster compile
extern MidiDriver *MidiDriver_NULL_create();
extern MidiDriver *MidiDriver_ADLIB_create(Audio::Mixer *mixer);
extern MidiDriver *MidiDriver_WIN_create();
extern MidiDriver *MidiDriver_SEQ_create();
extern MidiDriver *MidiDriver_QT_create();
extern MidiDriver *MidiDriver_CORE_create();
extern MidiDriver *MidiDriver_ETUDE_create();
extern MidiDriver *MidiDriver_ALSA_create();
extern MidiDriver *MidiDriver_YM2612_create(Audio::Mixer *mixer);
#ifdef USE_FLUIDSYNTH
extern MidiDriver *MidiDriver_FluidSynth_create(Audio::Mixer *mixer);
#endif
#ifdef USE_MT32EMU
extern MidiDriver *MidiDriver_MT32_create(Audio::Mixer *mixer);
#endif
extern MidiDriver *MidiDriver_YamahaPa1_create();
extern MidiDriver *MidiDriver_Zodiac_create();

#endif
