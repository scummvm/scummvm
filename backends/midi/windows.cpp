/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 */

#if defined(WIN32) && !defined(_WIN32_WCE)

#include "common/stdafx.h"
#include "sound/mpu401.h"
#include "common/util.h"

////////////////////////////////////////
//
// Windows MIDI driver
//
////////////////////////////////////////

class MidiDriver_WIN : public MidiDriver_MPU401 {
private:
	MIDIHDR _streamHeader;
	byte _streamBuffer [258];	// SysEx blocks should be no larger than 256 bytes
	HANDLE _streamEvent;
	HMIDIOUT _mo;
	bool _isOpen;

	void check_error(MMRESULT result);

public:
	MidiDriver_WIN() : _isOpen (false) { }
	int open();
	void close();
	void send(uint32 b);
	void sysEx (byte *msg, uint16 length);
};

int MidiDriver_WIN::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_streamEvent = CreateEvent (NULL, true, true, NULL);
	MMRESULT res = midiOutOpen((HMIDIOUT *)&_mo, MIDI_MAPPER, (unsigned long) _streamEvent, 0, CALLBACK_EVENT);
	if (res != MMSYSERR_NOERROR) {
		check_error(res);
		CloseHandle(_streamEvent);
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	_isOpen = true;
	return 0;
}

void MidiDriver_WIN::close() {
	if (!_isOpen)
		return;
	_isOpen = false;
	MidiDriver_MPU401::close();
	midiOutUnprepareHeader (_mo, &_streamHeader, sizeof (_streamHeader));
	check_error(midiOutClose(_mo));
	CloseHandle (_streamEvent);
}

void MidiDriver_WIN::send(uint32 b) {
	union {
		DWORD dwData;
		BYTE bData[4];
	} u;

	u.bData[3] = (byte)((b & 0xFF000000) >> 24);
	u.bData[2] = (byte)((b & 0x00FF0000) >> 16);
	u.bData[1] = (byte)((b & 0x0000FF00) >> 8);
	u.bData[0] = (byte)(b & 0x000000FF);

	check_error(midiOutShortMsg(_mo, u.dwData));
}

void MidiDriver_WIN::sysEx(byte *msg, uint16 length) {
	if (!_isOpen)
		return;

	if (WaitForSingleObject (_streamEvent, 2000) == WAIT_TIMEOUT) {
		warning ("Could not send SysEx - MMSYSTEM is still trying to send data.");
		return;
	}

	midiOutUnprepareHeader (_mo, &_streamHeader, sizeof (_streamHeader));
	_streamBuffer [0] = 0xF0;
	memcpy(&_streamBuffer[1], msg, length);
	_streamBuffer [length+1] = 0xF7;

	_streamHeader.lpData = (char *) _streamBuffer;
	_streamHeader.dwBufferLength = length + 2;
	_streamHeader.dwBytesRecorded = length + 2;
	_streamHeader.dwUser = 0;
	_streamHeader.dwFlags = 0;

	MMRESULT result = midiOutPrepareHeader (_mo, &_streamHeader, sizeof (_streamHeader));
	if (result != MMSYSERR_NOERROR) {
		check_error (result);
		return;
	}

	ResetEvent(_streamEvent);
	result = midiOutLongMsg (_mo, &_streamHeader, sizeof (_streamHeader));
	if (result != MMSYSERR_NOERROR) {
		check_error(result);
		SetEvent(_streamEvent);
		return;
	}
}

void MidiDriver_WIN::check_error(MMRESULT result) {
	char buf[200];
	if (result != MMSYSERR_NOERROR) {
		midiOutGetErrorText(result, buf, 200);
		warning("MM System Error '%s'", buf);
	}
}

MidiDriver *MidiDriver_WIN_create() {
	return new MidiDriver_WIN();
}

#endif
