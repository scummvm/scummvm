/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "sound/mpu401.h"
#include "common/util.h"

#ifndef DISABLE_TAPWAVE

#include "TwTraps.h"
#include "TwMidi.h"


class MidiDriver_Zodiac:public MidiDriver_MPU401 {
public:
	MidiDriver_Zodiac();
	int open();
	void close();
	void send(uint32 b);
	void sysEx(byte *msg, uint16 length);

private:
	TwMidiHandle _midiHandle;
	Boolean _isOpen;
	Int32 _oldVol;
 };

MidiDriver_Zodiac::MidiDriver_Zodiac() {
	_isOpen = false;
	_midiHandle = 0;
}

int MidiDriver_Zodiac::open() {
	Err e;

	if (e = TwMidiOpen(&_midiHandle))
 		return MERR_DEVICE_NOT_AVAILABLE;

	TwMidiGetMasterVolume(&_oldVol);
	TwMidiSetMasterVolume(twMidiMaxVolume);

	_isOpen = true;
	return 0;
}

void MidiDriver_Zodiac::close() {
	if (_isOpen) {
		_isOpen = false;
		MidiDriver_MPU401::close();

		TwMidiSetMasterVolume(_oldVol);
		TwMidiClose(_midiHandle);
	}
}

void MidiDriver_Zodiac::send(uint32 b) {
	if (!_isOpen)
		return;

	UInt8 midiCmd[4];
	UInt8 chanID,mdCmd;

	midiCmd[3] = (b & 0xFF000000) >> 24;
	midiCmd[2] = (b & 0x00FF0000) >> 16;
	midiCmd[1] = (b & 0x0000FF00) >> 8;
	midiCmd[0] = (b & 0x000000FF);

	chanID = (midiCmd[0] & 0x0F) ;
	mdCmd = midiCmd[0] & 0xF0;

	switch (mdCmd) {
		case 0x80:	// note off
			TwMidiNoteOff(_midiHandle, chanID, midiCmd[1], 0);
			break;

		case 0x90:	// note on
			TwMidiNoteOn(_midiHandle, chanID, midiCmd[1], midiCmd[2]);
			break;

		case 0xB0:	// control change
			TwMidiControlChange(_midiHandle, chanID, midiCmd[1], midiCmd[2]);
			break;

		case 0xC0:	// progam change
			TwMidiProgramChange(_midiHandle, chanID, midiCmd[1]);
			break;

		case 0xE0:	// pitchBend
			TwMidiPitchBend(_midiHandle, chanID, (short)(midiCmd[1] | (midiCmd[2] << 8)));
			break;
	}
}

void MidiDriver_Zodiac::sysEx(byte *msg, uint16 length) {
	TwMidiSysEx(_midiHandle, 0, msg, length);
}

MidiDriver *MidiDriver_Zodiac_create() {
	return new MidiDriver_Zodiac();
}

#endif
