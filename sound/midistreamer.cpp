/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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


#include "stdafx.h"
#include "mididrv.h"
#include "engine.h"
#include "common/util.h"

class MidiStreamer : public MidiDriver {
private:
	MidiDriver *_target;
	StreamCallback *_stream_proc;
	void *_stream_param;
	volatile int _mode;
	volatile bool _paused;

	MidiEvent _events [64];
	int _event_count;
	int _event_index;

	long _driver_tempo;
	long _tempo;
	uint16 _ticks_per_beat;
	long _delay;

	volatile bool _active;

	uint32 property(int prop, uint32 param);
	static void timer_thread (void *param);
	void on_timer();

public:
	MidiStreamer (MidiDriver *target);

	int open(int mode);
	void close();
	void send(uint32 b) { if (_mode) _target->send (b); }
	void pause(bool p) { _paused = p; }
	void set_stream_callback(void *param, StreamCallback *sc);
	void setPitchBendRange (byte channel, uint range) { _target->setPitchBendRange (channel, range); }

	void setTimerCallback (void *timer_param, void (*timer_proc) (void *)) { }
	uint32 getBaseTempo (void) { return _target->getBaseTempo(); }
};

MidiStreamer::MidiStreamer (MidiDriver *target) :
_target (target),
_stream_proc (0),
_stream_param (0),
_mode (0),
_paused (false),
_event_count (0),
_event_index (0),
_tempo (500000), // 120 BPM = 500,000 microseconds between each beat
_ticks_per_beat (96),
_delay (0),
_active (false)
{ }

void MidiStreamer::set_stream_callback (void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;

	if (_mode) {
		_event_count = _stream_proc (_stream_param, _events, ARRAYSIZE (_events));
		_event_index = 0;
	}
}
/*
int MidiStreamer::timer_thread (void *param) {
	MidiStreamer *mid = (MidiStreamer *) param;
	int old_time, cur_time;
	while (mid->_mode) {
		g_system->delay_msecs (100);
		while (!mid->_stream_proc);
		old_time = g_system->get_msecs();
		while (!mid->_paused) {
			g_system->delay_msecs(10);

			cur_time = g_system->get_msecs();
			while (old_time < cur_time) {
				old_time += 10;
				mid->on_timer();
			}
		}
	}

	// Turn off all notes on all channels,
	// just to catch anything still playing.
	int i;
	for (i = 0; i < 16; ++i)
		mid->_target->send ((0x7B << 8) | 0xB0 | i);
	mid->_active = false;
	return 0;
}
*/
void MidiStreamer::timer_thread (void *param) {
	((MidiStreamer *) param)->on_timer();
}

void MidiStreamer::on_timer()
{
	_delay += _driver_tempo; // 10000;
	while (true) {
		if (_event_index >= _event_count) {
			_event_count = _stream_proc (_stream_param, _events, ARRAYSIZE (_events));
			_event_index = 0;
		}

		if (!_event_count)
			return;

		MidiEvent *ev = &_events [_event_index];
		if (_delay < _tempo * (long) ev->delta / (long) _ticks_per_beat)
			return;
		_delay -= _tempo * ev->delta / _ticks_per_beat;
		if ((ev->event >> 24) != ME_TEMPO) {
			_target->send (ev->event);
		} else {
			_tempo = ev->event & 0xFFFFFF;
		}

		++_event_index;
	} // end while
}

int MidiStreamer::open (int mode)
{
	if (_mode != 0)
		close();

	int res = _target->open (MidiDriver::MO_SIMPLE);
	if (res && res != MERR_ALREADY_OPEN)
		return res;

	_event_index = _event_count = _delay = 0;
	_mode = mode;
	_paused = false;

	if (mode == MO_SIMPLE)
		return 0;

//	g_system->create_thread (timer_thread, this);
	_driver_tempo = _target->getBaseTempo() / 500;

	_target->setTimerCallback (this, &timer_thread);
	return 0;
}

void MidiStreamer::close()
{
	if (!_mode)
		return;

	_target->setTimerCallback (NULL, NULL);

	// Turn off all notes on all channels,
	// just to catch anything still playing.
	int i;
	for (i = 0; i < 16; ++i)
		_target->send ((0x7B << 8) | 0xB0 | i);

	_mode = 0;
	_paused = true;
}

uint32 MidiStreamer::property (int prop, uint32 param)
{
	switch (prop) {

	// 16-bit time division according to standard midi specification
	case PROP_TIMEDIV:
		_ticks_per_beat = (uint16)param;
		return 1;
	}

	return 0;
}
