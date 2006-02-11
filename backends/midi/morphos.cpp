/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#if defined(__MORPHOS__)

#include <exec/memory.h>
#include <exec/types.h>
#include <devices/etude.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/etude.h>

#include "common/stdafx.h"
#include "sound/mpu401.h"
#include "common/util.h"
#include "morphos.h"
#include "morphos_sound.h"

/*
 * MorphOS MIDI driver for Etude device
 */

class MidiDriver_ETUDE : public MidiDriver_MPU401 {
public:
	MidiDriver_ETUDE();
	int open();
	void close();
	void send(uint32 b);

private:
	bool _isOpen;
};

MidiDriver_ETUDE::MidiDriver_ETUDE()
{
	_isOpen = false;
}

int MidiDriver_ETUDE::open()
{
	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_isOpen = true;
	if (!init_morphos_music(ScummMidiUnit, ETUDEF_DIRECT))
		return MERR_DEVICE_NOT_AVAILABLE;

	return 0;
}

void MidiDriver_ETUDE::close()
{
	MidiDriver_MPU401::close();
	exit_morphos_music();
	_isOpen = false;
}

void MidiDriver_ETUDE::send(uint32 b)
{
	if (!_isOpen)
		error("MidiDriver_ETUDE::send called but driver was no opened");

	if (ScummMidiRequest) {
		ULONG midi_data = READ_LE_UINT32(&b);
		SendShortMidiMsg(ScummMidiRequest, midi_data);
	}
}

extern MidiDriver* EtudeMidiDriver;

MidiDriver *MidiDriver_ETUDE_create()
{
	if (!EtudeMidiDriver)
		EtudeMidiDriver = new MidiDriver_ETUDE();
	return EtudeMidiDriver;
}

#endif

