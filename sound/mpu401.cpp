/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "mpu401.h"
#include "common/engine.h"	// for g_system
#include "common/util.h"	// for ARRAYSIZE


void MidiChannel_MPU401::init(MidiDriver_MPU401 *owner, byte channel) {
	_owner = owner;
	_channel = channel;
	_allocated = false;
}

MidiDriver *MidiChannel_MPU401::device() {
	return _owner;
}

void MidiChannel_MPU401::noteOff (byte note) {
	_owner->send(note << 8 | 0x80 | _channel);
}

void MidiChannel_MPU401::noteOn(byte note, byte velocity) {
	_owner->send (velocity << 16 | note << 8 | 0x90 | _channel);
}

void MidiChannel_MPU401::programChange(byte program) {
	_owner->send(program << 8 | 0xC0 | _channel);
}

void MidiChannel_MPU401::pitchBend(int16 bend) {
	_owner->send((((bend + 0x2000) >> 7) & 0x7F) << 16 | ((bend + 0x2000) & 0x7F) << 8 | 0xE0 | _channel);
}

void MidiChannel_MPU401::controlChange(byte control, byte value) {
	_owner->send(value << 16 | control << 8 | 0xB0 | _channel);
}

void MidiChannel_MPU401::pitchBendFactor(byte value) {
	_owner->setPitchBendRange (_channel, value);
}

void MidiChannel_MPU401::sysEx_customInstrument(uint32 type, byte *instr) {
	_owner->sysEx_customInstrument (_channel, type, instr);
}

MidiDriver_MPU401::MidiDriver_MPU401() : MidiDriver() {
	uint i;
	
	_started_thread = false;	// palmos
	_timer_proc = NULL;			// palmos
	_timer_param = NULL;		// palmos
	
	for (i = 0; i < ARRAYSIZE(_midi_channels); ++i) {
		_midi_channels [i].init (this, i);
	}
}

MidiChannel *MidiDriver_MPU401::allocateChannel() {
	MidiChannel_MPU401 *chan;
	uint i;

	for (i = 0; i < ARRAYSIZE(_midi_channels); ++i) {
		if (i == 9)
			continue;
		chan = &_midi_channels[i];
		if (!chan->_allocated) {
			chan->allocate();
			return (chan);
		}
	}
	return NULL;
}

void MidiDriver_MPU401::setTimerCallback (void *timer_param, void (*timer_proc) (void *)) {
	if (!_timer_proc || !timer_proc) {
		_timer_proc = (TimerCallback *) timer_proc;
		_timer_param = timer_param;
		if (!_started_thread && timer_proc)
			g_system->create_thread(midi_driver_thread, this);
		_started_thread = true;
	}
}

#if !defined(__MORPHOS__) && !defined(__PALM_OS__)
int MidiDriver_MPU401::midi_driver_thread(void *param) {
	MidiDriver_MPU401 *mid = (MidiDriver_MPU401 *)param;
	int old_time, cur_time;

	old_time = g_system->get_msecs();

	for (;;) {
		g_system->delay_msecs(10);

		cur_time = g_system->get_msecs();
		while (old_time < cur_time) {
			old_time += 10;
			// Don't use mid->_se_on_timer()
			// We must come in through IMuseMonitor to protect
			// against conflicts with script access to IMuse.
			if (mid->_timer_proc)
				(*(mid->_timer_proc)) (mid->_timer_param);
		}
	}

	return 0;
}
#endif

