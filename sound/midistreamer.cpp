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

// #include "stdafx.h"
#include "midistreamer.h"
#include "common/util.h"

MidiStreamer::MidiStreamer (MidiDriver *target) :
_target (target),
_stream_proc (0),
_stream_param (0),
_isOpen (false),
_paused (false),
_event_count (0),
_event_index (0),
_tempo (500000), // 120 BPM = 500,000 microseconds between each beat
_ticks_per_beat (96),
_delay (0)
{ }

void MidiStreamer::set_stream_callback (void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;

	if (_isOpen) {
		_event_count = _stream_proc (_stream_param, _events, ARRAYSIZE (_events));
		_event_index = 0;
	}
}

void MidiStreamer::timer_thread (void *param) {
	((MidiStreamer *) param)->on_timer();
}

void MidiStreamer::on_timer()
{
	if (_paused || !_stream_proc)
		return;

	_delay += _driver_tempo;
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

int MidiStreamer::open()
{
	if (_isOpen)
		close();

	int res = _target->open();
	if (res && res != MidiDriver::MERR_ALREADY_OPEN)
		return res;

	_event_index = _event_count = _delay = 0;
	_isOpen = true;
	_paused = false;

	_driver_tempo = _target->getBaseTempo() / 500;

	_target->setTimerCallback (this, &timer_thread);
	return 0;
}

void MidiStreamer::close()
{
	if (!_isOpen)
		return;

	_target->setTimerCallback (NULL, NULL);

	// Turn off all notes on all channels,
	// just to catch anything still playing.
	int i;
	for (i = 0; i < 16; ++i)
		_target->send ((0x7B << 8) | 0xB0 | i);

	_isOpen = false;
	_paused = true;
}

uint32 MidiStreamer::property (int prop, uint32 param)
{
	switch (prop) {

	// 16-bit time division according to standard midi specification
	case MidiDriver::PROP_TIMEDIV:
		_ticks_per_beat = (uint16)param;
		return 1;
	}

	return 0;
}
