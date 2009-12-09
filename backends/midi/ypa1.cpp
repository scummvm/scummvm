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

#include "common/util.h"
#include "sound/musicplugin.h"
#include "sound/mpu401.h"

#include "Pa1Lib.h"

class MidiDriver_YamahaPa1:public MidiDriver_MPU401 {
public:
	MidiDriver_YamahaPa1();
	int open();
	void close();
	void send(uint32 b);

private:
	UInt8 _midiHandle;
	Boolean _isOpen;
 };

MidiDriver_YamahaPa1::MidiDriver_YamahaPa1() {
	_isOpen = false;
	_midiHandle = 0;
}

int MidiDriver_YamahaPa1::open() {
	if (!(_isOpen = Pa1Lib_midiOpen(NULL, &_midiHandle)))
		return MERR_DEVICE_NOT_AVAILABLE;

	return 0;
}

void MidiDriver_YamahaPa1::close() {
	if (_isOpen) {
		_isOpen = false;
		MidiDriver_MPU401::close();
		for (UInt8 channel = 0; channel < 16; channel++) {
			Pa1Lib_midiControlChange(_midiHandle, channel, 120,0); // all sound off
			Pa1Lib_midiControlChange(_midiHandle, channel, 121,0); // reset all controller
			Pa1Lib_midiControlChange(_midiHandle, channel, 123, 0); // all notes off
		}
		Pa1Lib_midiClose(_midiHandle);
	}
}

void MidiDriver_YamahaPa1::send(uint32 b) {
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
			Pa1Lib_midiNoteOff(_midiHandle, chanID, midiCmd[1], 0);
			break;

		case 0x90:	// note on
			Pa1Lib_midiNoteOn(_midiHandle, chanID, midiCmd[1], midiCmd[2]);
			break;

		case 0xB0:	// control change
			Pa1Lib_midiControlChange(_midiHandle, chanID, midiCmd[1], midiCmd[2]);
			break;

		case 0xC0:	// progam change
			Pa1Lib_midiProgramChange(_midiHandle, chanID, midiCmd[1]);
			break;

		case 0xE0:	// pitchBend
			Pa1Lib_midiPitchBend(_midiHandle, chanID, (short)(midiCmd[1] | (midiCmd[2] << 8)));
			break;
	}
}


// Plugin interface

class YamahaPa1MusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "Yamaha Pa1";
	}

	const char *getId() const {
		return "ypa1";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver) const;
};

MusicDevices YamahaPa1MusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error YamahaPa1MusicPlugin::createInstance(MidiDriver **mididriver) const {
	*mididriver = new MidiDriver_YamahaPa1();

	return Common::kNoError;
}

MidiDriver *MidiDriver_YamahaPa1_create() {
	MidiDriver *mididriver;

	YamahaPa1MusicPlugin p;
	p.createInstance(&mididriver);

	return mididriver;
}

//#if PLUGIN_ENABLED_DYNAMIC(YPA1)
	//REGISTER_PLUGIN_DYNAMIC(YPA1, PLUGIN_TYPE_MUSIC, YamahaPa1MusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(YPA1, PLUGIN_TYPE_MUSIC, YamahaPa1MusicPlugin);
//#endif
