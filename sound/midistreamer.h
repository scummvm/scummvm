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

#ifndef INCLUDED_MIDISTREAMER
#define INCLUDED_MIDISTREAMER

class MidiStreamer;

#include "mididrv.h"

struct MidiEvent {
	uint32 delta;
	uint32 event;
};

class MidiStreamer {
public:
	// Called whenever more MIDI commands need to be generated.
	// Return 0 to tell MidiStreamer that end of stream was reached.
	typedef int StreamCallback (void *param, MidiEvent *ev, int num);

	// Special events that can be inserted in a MidiEvent.
	// event = (ME_xxx<<24) | <24-bit data associated with event>
	enum {
		ME_NONE = 0,
		ME_TEMPO = 1
	};

private:
	MidiDriver *_target;
	StreamCallback *_stream_proc;
	void *_stream_param;
	bool _isOpen;
	bool _paused;

	MidiEvent _events[64];
	int _event_count;
	int _event_index;

	long _driver_tempo;
	long _tempo;
	uint16 _ticks_per_beat;
	long _delay;

	static void timer_thread (void *param);
	void on_timer();

public:
	MidiStreamer (MidiDriver *target);

	int open();
	void close();
	void send(uint32 b) { if (_isOpen) _target->send (b); }
	uint32 property(int prop, uint32 param);
	void pause(bool p) { _paused = p; }
	void set_stream_callback(void *param, StreamCallback *sc);
};

#endif
