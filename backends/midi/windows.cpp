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

#if defined(WIN32) && !defined(_WIN32_WCE)

#include "stdafx.h"
#include "sound/mpu401.h"
#include "common/engine.h" // For warning/error/debug

////////////////////////////////////////
//
// Windows MIDI driver
//
////////////////////////////////////////

class MidiDriver_WIN : public MidiDriver_MPU401 {
public:
	MidiDriver_WIN();
	int open(int mode);
	void close();
	void send(uint32 b);

private:
	HMIDIOUT _mo;

	void check_error(MMRESULT result);
	uint32 property(int prop, uint32 param);
};

MidiDriver_WIN::MidiDriver_WIN()
{
	_isOpen = false;
}

int MidiDriver_WIN::open()
{
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_isOpen = true;

	MMRESULT res = midiOutOpen((HMIDIOUT *) &_mo, MIDI_MAPPER, 0, 0, 0);
	if (res != MMSYSERR_NOERROR)
		check_error(res);

	return 0;
}

void MidiDriver_WIN::close()
{
	_isOpen = false;
	check_error(midiOutClose(_mo));
}

void MidiDriver_WIN::send(uint32 b)
{
	union {
		DWORD dwData;
		BYTE bData[4];
	} u;

	u.bData[3] = (byte)((b & 0xFF000000) >> 24);
	u.bData[2] = (byte)((b & 0x00FF0000) >> 16);
	u.bData[1] = (byte)((b & 0x0000FF00) >> 8);
	u.bData[0] = (byte)(b & 0x000000FF);

	// printMidi(u.bData[0], u.bData[1], u.bData[2], u.bData[3]);
	check_error(midiOutShortMsg(_mo, u.dwData));
}

void MidiDriver_WIN::check_error(MMRESULT result)
{
	char buf[200];
	if (result != MMSYSERR_NOERROR) {
		midiOutGetErrorText(result, buf, 200);
		warning("MM System Error '%s'", buf);
	}
}

MidiDriver *MidiDriver_WIN_create()
{
	return new MidiDriver_WIN();
}

#endif
