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

#include "midiparser.h"
#include "mididrv.h"
#include "common/util.h"

#include <stdio.h>
#include <memory.h>

//////////////////////////////////////////////////
//
// MidiParser implementation
//
//////////////////////////////////////////////////

MidiParser::MidiParser() :
_driver (0),
_timer_rate (0x4A0000),
_ppqn (96),
_tempo (500000),
_psec_per_tick (5208), // 500000 / 96
_autoLoop (false),
_num_tracks (0),
_active_track (255),
_play_pos (0),
_play_time (0),
_last_event_time (0),
_last_event_tick (0),
_running_status (0)
{ }

void MidiParser::property (int prop, int value) {
	switch (prop) {
	case mpAutoLoop:
		_autoLoop = (value != 0);
	}
}

// This is the conventional (i.e. SMF) variable length quantity
uint32 MidiParser::readVLQ (byte * &data) {
	byte str;
	uint32 value = 0;
	int i;

	for (i = 0; i < 4; ++i) {
		str = data[0];
		++data;
		value = (value << 7) | (str & 0x7F);
		if (!(str & 0x80))
			break;
	}
	return value;
}

void MidiParser::onTimer() {
	uint32 end_time;
	uint32 event_time;

	if (!_play_pos || !_driver)
		return;

	end_time = _play_time + _timer_rate;

	while (true) {
		EventInfo &info = _next_event;

		event_time = _last_event_time + info.delta * _psec_per_tick;
		if (event_time > end_time)
			break;

		// Process the next info.
		_last_event_tick += info.delta;
		if (info.event < 0x80) {
			printf ("ERROR! Bad command or running status %02X", info.event);
			_play_pos = 0;
			return;
		}
		_running_status = info.event;

		if (info.event == 0xF0) {
			// SysEx event
			_driver->sysEx (info.ext.data, (uint16) info.ext.length);
		} else if (info.event == 0xFF) {
			// META event
			if (info.ext.type == 0x2F) {
				// End of Track must be processed by us,
				// as well as sending it to the output device.
				allNotesOff();
				if (_autoLoop) {
					_play_pos = _tracks[_active_track];
					parseNextEvent (_next_event);
				} else {
					_play_pos = 0;
					_driver->metaEvent (info.ext.type, info.ext.data, (uint16) info.ext.length);
				}
				return;
			} else if (info.ext.type == 0x51) {
				if (info.ext.length >= 3) {
					_tempo = info.ext.data[0] << 16 | info.ext.data[1] << 8 | info.ext.data[2];
					_psec_per_tick = (_tempo + (_ppqn >> 2)) / _ppqn;
				}
			}
			_driver->metaEvent (info.ext.type, info.ext.data, (uint16) info.ext.length);
		} else {
			_driver->send (info.event | info.basic.param1 << 8 | info.basic.param2 << 16);
		}


		_last_event_time = event_time;
		parseNextEvent (_next_event);
	}

	_play_time = end_time;
}

void MidiParser::allNotesOff() {
	if (!_driver)
		return;

	int i;
	for (i = 0; i < 15; ++i) {
		_driver->send (0x007BB0 | i);
	}
}

void MidiParser::resetTracking() {
	_play_pos = 0;
	_tempo = 500000;
	_psec_per_tick = 500000 / _ppqn;
	_play_time = 0;
	_last_event_time = 0;
	_last_event_tick = 0;
	_running_status = 0;
}

void MidiParser::setTrack (byte track) {
	if (track >= _num_tracks || track == _active_track)
		return;
	resetTracking();
	allNotesOff();
	_active_track = track;
	_play_pos = _tracks[track];
	parseNextEvent (_next_event);
}

void MidiParser::jumpToTick (uint32 tick) {
	if (_active_track >= _num_tracks)
		return;
	resetTracking();
	allNotesOff();

	_play_pos = _tracks[_active_track];
	parseNextEvent (_next_event);
	if (tick == 0)
		return;

	while (true) {
		EventInfo &info = _next_event;
		if (_last_event_tick + info.delta >= tick) {
			_play_time += (tick - _last_event_tick) * _psec_per_tick;
			break;
		}

		_last_event_tick += info.delta;
		_play_time += info.delta * _psec_per_tick;
		_last_event_time = _play_time;

		if (info.event == 0xFF) {
			if (info.ext.type == 0x2F) { // End of track
				if (_autoLoop) {
					_play_pos = _tracks[_active_track];
					parseNextEvent (_next_event);
				} else {
					_play_pos = 0;
					_driver->metaEvent (0x2F, info.ext.data, (uint16) info.ext.length);
				}
				break;
			} else if (info.ext.type == 0x51) { // Tempo
				if (info.ext.length >= 3) {
					_tempo = info.ext.data[0] << 16 | info.ext.data[1] << 8 | info.ext.data[2];
					_psec_per_tick = (_tempo + (_ppqn >> 2)) / _ppqn;
				}
			}
		}

		parseNextEvent (_next_event);
	}
}
