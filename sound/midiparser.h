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

#ifndef INCLUDED_MIDIPARSER
#define INCLUDED_MIDIPARSER

class MidiParser;

#include "common/scummsys.h"

class MidiDriver;

struct EventInfo {
	byte * start; // Points to delta
	uint32 delta;
	byte   event;
	union {
		struct {
			byte param1;
			byte param2;
		} basic;
		struct {
			byte   type; // Used for METAs
			byte * data; // Used for SysEx and METAs
		} ext;
	};
	uint32 length; // Used for SysEx and METAs, and note lengths

	byte channel() { return event & 0x0F; }
	byte command() { return event >> 4; }
};

struct NoteTimer {
	byte channel;
	byte note;
	uint32 time_left;
	NoteTimer() : channel(0), note(0), time_left(0) {}
};

class MidiParser {
private:
	uint16    _active_notes[128]; // Each uint16 is a bit mask for channels that have that note on
	NoteTimer _hanging_notes[32]; // Supports "smart" jump with notes still playing
	byte      _hanging_notes_count;

protected:
	MidiDriver *_driver;
	uint32 _timer_rate;
	uint32 _ppqn;           // Pulses (ticks) Per Quarter Note
	uint32 _tempo;          // Microseconds per quarter note
	uint32 _psec_per_tick;  // Microseconds per tick (_tempo / _ppqn)
	bool   _autoLoop;       // For lightweight clients that don't monitor events
	bool   _smartJump;      // Support smart expiration of hanging notes when jumping

	byte * _tracks[16];
	byte   _num_tracks;
	byte   _active_track;

	byte * _play_pos;
	uint32 _play_time;
	uint32 _play_tick;
	uint32 _last_event_time;
	uint32 _last_event_tick;
	byte   _running_status;
	EventInfo _next_event;

protected:
	static uint32 readVLQ (byte * &data);
	virtual void resetTracking();
	virtual void allNotesOff();
	virtual void parseNextEvent (EventInfo &info) = 0;

	void activeNote (byte channel, byte note, bool active);
	void hangingNote (byte channel, byte note, uint32 ticks_left);

	// Multi-byte read helpers
	uint32 read4high (byte * &data) {
		uint32 val = 0;
		int i;
		for (i = 0; i < 4; ++i) val = (val << 8) | *data++;
		return val;
	}
	uint16 read2low  (byte * &data) {
		uint16 val = 0;
		int i;
		for (i = 0; i < 2; ++i) val |= (*data++) << (i * 8);
		return val;
	}

public:
	enum {
		mpMalformedPitchBends = 1,
		mpAutoLoop = 2,
		mpSmartJump = 3
	};

public:
	MidiParser();
	virtual ~MidiParser() { }

	virtual bool loadMusic (byte *data, uint32 size) = 0;
	virtual void unloadMusic() = 0;
	virtual void property (int prop, int value);

	void setMidiDriver (MidiDriver *driver) { _driver = driver; }
	void setTimerRate (uint32 rate) { _timer_rate = rate / 500; }
	void onTimer();

	bool setTrack (int track);
	void jumpToTick (uint32 tick);

	static MidiParser *createParser_SMF();
	static MidiParser *createParser_XMIDI();
	static void timerCallback (void *data) { ((MidiParser *) data)->onTimer(); }
};

#endif
