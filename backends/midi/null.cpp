/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include "sound/mpu401.h"
#include "common/engine.h"	// for warning/error/debug

/* NULL driver */
class MidiDriver_NULL : public MidiDriver_MPU401 {
public:
	int open(int mode);
	void close() { }
	void send(uint32 b) { }
	void pause(bool p) { }
	void set_stream_callback(void *param, StreamCallback *sc) { }
};

int MidiDriver_NULL::open(int mode)
{
	warning("Music not enabled - MIDI support selected with no MIDI driver available. Try Adlib");
	return 0;
}

MidiDriver *MidiDriver_NULL_create()
{
	return new MidiDriver_NULL();
}
