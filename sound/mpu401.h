/* ScummVM - Scumm Interpreter
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

#ifndef SOUND_MPU401_H
#define SOUND_MPU401_H

#include "common/system.h"
#include "mididrv.h"

////////////////////////////////////////
//
// Common MPU401 implementation methods
//
////////////////////////////////////////

class MidiDriver_MPU401;

class MidiChannel_MPU401 : public MidiChannel {
	friend class MidiDriver_MPU401;

private:
	MidiDriver_MPU401 *_owner;
	bool _allocated;
	byte _channel;

	void init (MidiDriver_MPU401 *owner, byte channel);
	void allocate() { _allocated = true; }

public:
	MidiDriver *device();
	byte getNumber() { return _channel; }
	void release() { _allocated = false; }

	void send (uint32 b);

	// Regular messages
	void noteOff (byte note);
	void noteOn (byte note, byte velocity);
	void programChange (byte program);
	void pitchBend (int16 bend);

	// Control Change messages
	void controlChange (byte control, byte value);
	void modulationWheel (byte value) { controlChange (1, value); }
	void volume (byte value) { controlChange (7, value); }
	void panPosition (byte value) { controlChange (10, value); }
	void pitchBendFactor (byte value);
	void detune (byte value) { controlChange (17, value); }
	void priority (byte value) { controlChange (18, value); }
	void sustain (bool value) { controlChange (64, value ? 1 : 0); }
	void effectLevel (byte value) { controlChange (91, value); }
	void chorusLevel (byte value) { controlChange (93, value); }
	void allNotesOff() { controlChange (123, 0); }

	// SysEx messages
	void sysEx_customInstrument (uint32 type, byte *instr);
};



class MidiDriver_MPU401 : public MidiDriver {
private:
	typedef void (*TimerCallback) (void*);

	MidiChannel_MPU401 _midi_channels [16];
	volatile bool _started_thread;
	OSystem::MutexRef _mutex; // Concurrent shutdown barrier
	volatile TimerCallback _timer_proc;
	void *_timer_param;
	uint16 _channel_mask;

	static int midi_driver_thread (void *param);

public:
	MidiDriver_MPU401();

	virtual void close();
	void setTimerCallback(void *timer_param, TimerCallback timer_proc);
	uint32 getBaseTempo(void) { return 10000; }
	uint32 property(int prop, uint32 param);

	MidiChannel *allocateChannel();
	MidiChannel *getPercussionChannel() { return &_midi_channels [9]; }
};


#endif
