/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#if defined(WIN32) && !defined(_WIN32_WCE)

#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE

#include "sound/musicplugin.h"
#include "sound/mpu401.h"

#include <mmsystem.h>

////////////////////////////////////////
//
// Windows MIDI driver
//
////////////////////////////////////////

class MidiDriver_WIN : public MidiDriver_MPU401 {
private:
	MIDIHDR _streamHeader;
	byte _streamBuffer[266];	// SysEx blocks should be no larger than 266 bytes
	HANDLE _streamEvent;
	HMIDIOUT _mo;
	bool _isOpen;

	void check_error(MMRESULT result);

public:
	MidiDriver_WIN() : _isOpen(false) { }
	int open();
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);
};

int MidiDriver_WIN::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_streamEvent = CreateEvent(NULL, true, true, NULL);
	MMRESULT res = midiOutOpen((HMIDIOUT *)&_mo, MIDI_MAPPER, (DWORD_PTR)_streamEvent, 0, CALLBACK_EVENT);
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
	midiOutUnprepareHeader(_mo, &_streamHeader, sizeof(_streamHeader));
	check_error(midiOutClose(_mo));
	CloseHandle(_streamEvent);
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

void MidiDriver_WIN::sysEx(const byte *msg, uint16 length) {
	if (!_isOpen)
		return;

	if (WaitForSingleObject (_streamEvent, 2000) == WAIT_TIMEOUT) {
		warning ("Could not send SysEx - MMSYSTEM is still trying to send data.");
		return;
	}

	assert(length+2 <= 266);

	midiOutUnprepareHeader(_mo, &_streamHeader, sizeof(_streamHeader));

	// Add SysEx frame
	_streamBuffer[0] = 0xF0;
	memcpy(&_streamBuffer[1], msg, length);
	_streamBuffer[length+1] = 0xF7;

	_streamHeader.lpData = (char *)_streamBuffer;
	_streamHeader.dwBufferLength = length + 2;
	_streamHeader.dwBytesRecorded = length + 2;
	_streamHeader.dwUser = 0;
	_streamHeader.dwFlags = 0;

	MMRESULT result = midiOutPrepareHeader(_mo, &_streamHeader, sizeof(_streamHeader));
	if (result != MMSYSERR_NOERROR) {
		check_error (result);
		return;
	}

	ResetEvent(_streamEvent);
	result = midiOutLongMsg(_mo, &_streamHeader, sizeof(_streamHeader));
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


// Plugin interface

class WindowsMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "Windows MIDI";
	}

	const char *getId() const {
		return "windows";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver) const;
};

MusicDevices WindowsMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error WindowsMusicPlugin::createInstance(MidiDriver **mididriver) const {
	*mididriver = new MidiDriver_WIN();

	return Common::kNoError;
}

MidiDriver *MidiDriver_WIN_create() {
	MidiDriver *mididriver;

	WindowsMusicPlugin p;
	p.createInstance(&mididriver);

	return mididriver;
}

//#if PLUGIN_ENABLED_DYNAMIC(WINDOWS)
	//REGISTER_PLUGIN_DYNAMIC(WINDOWS, PLUGIN_TYPE_MUSIC, WindowsMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(WINDOWS, PLUGIN_TYPE_MUSIC, WindowsMusicPlugin);
//#endif

#endif
