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
 * $Id: dmedia.cpp$
 */

/*
 * IRIX dmedia support by Rainer Canavan <scumm@canavan.de>
 *    some code liberated from seq.cpp and coremidi.cpp
 */

#if defined(IRIX)

#include "common/scummsys.h"
#include "common/util.h"
#include "sound/musicplugin.h"
#include "sound/mpu401.h"

#include <dmedia/midi.h>
#include <sys/types.h>
#include <bstring.h>
#include <unistd.h>

////////////////////////////////////////
//
// IRIX dmedia midi driver
//
////////////////////////////////////////

#define SEQ_MIDIPUTC 5

class MidiDriver_DMEDIA : public MidiDriver_MPU401 {
public:
	MidiDriver_DMEDIA();
	int open();
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	bool _isOpen;
	int _deviceNum;
	char *_midiportName;
	MDport _midiPort;
	int _fd;
};

MidiDriver_DMEDIA::MidiDriver_DMEDIA() {
	_isOpen = false;
	_deviceNum = 0;
	_midiportName = NULL;
}

int MidiDriver_DMEDIA::open() {
	int numinterfaces;

	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_isOpen = true;

	warning("dmedia init");
	numinterfaces = mdInit();
	if (numinterfaces <= 0) {
		fprintf(stderr,"No MIDI interfaces configured.\n");
		perror("Cannot initialize libmd for sound output");
		return -1;
	}

	if (getenv("SCUMMVM_MIDIPORT")) {
		_deviceNum = atoi(getenv("SCUMMVM_MIDIPORT"));
		_midiportName = mdGetName(_deviceNum);
	}
		else
	{
		_midiportName = mdGetName(0);
		warning("SCUMMVM_MIDIPORT environment variable not set, using Port %s", _midiportName);
		_deviceNum = 0;
	}

	_midiPort = mdOpenOutPort(_midiportName);
	if (!_midiPort) {
		warning("Failed to open MIDI interface %s", _midiportName);
		return -1;
	}

	_fd = mdGetFd(_midiPort);
	if (!_fd) {
		warning("Failed to aquire filehandle for MIDI port %s", _midiportName);
		mdClosePort(_midiPort);
		return -1;
	}

	mdSetStampMode(_midiPort, MD_NOSTAMP);  /* don't use Timestamps */

	return 0;
}

void MidiDriver_DMEDIA::close() {
	mdClosePort(_midiPort);
	_isOpen = false;
	_deviceNum = 0;
	_midiportName = NULL;
}

void MidiDriver_DMEDIA::send(uint32 b) {
	MDevent event;
	byte status_byte = (b & 0x000000FF);
	byte first_byte = (b & 0x0000FF00) >> 8;
	byte second_byte = (b & 0x00FF0000) >> 16;


	event.sysexmsg = NULL;
	event.msg[0] = status_byte;
	event.msg[1] = first_byte;
	event.msg[2] = second_byte;

	switch (status_byte & 0xF0) {
	case 0x80:      // Note Off
	case 0x90:      // Note On
	case 0xA0:      // Polyphonic Aftertouch
	case 0xB0:      // Controller Change
	case 0xE0:      // Pitch Bending
		event.msglen = 3;
		break;
	case 0xC0:      // Programm Change
	case 0xD0:      // Monophonic Aftertouch
		event.msglen = 2;
		break;
	default:
		warning("DMediaMIDI driver encountered unsupported status byte: 0x%02x", status_byte);
		event.msglen = 3;
		break;
	}
	if (mdSend(_midiPort, &event, 1) != 1) {
		warning("failed sending MIDI event (dump follows...)");
		warning("MIDI Event (len=%u):", event.msglen);
		for (int i=0; i<event.msglen; i++) warning("%02x ",(int)event.msg[i]);
	}
}

void MidiDriver_DMEDIA::sysEx (const byte *msg, uint16 length) {
	MDevent event;
	char buf [1024];

	assert(length + 2 <= 256);

	memcpy(buf, msg, length);
	buf[length] = MD_EOX;
	event.sysexmsg = buf;
        event.msglen = length;
	event.msg[0] = MD_SYSEX;
	event.msg[1] = 0;
	event.msg[2] = 0;

	if (mdSend(_midiPort, &event, 1) != 1) {
		fprintf(stderr,"failed sending MIDI SYSEX event (dump follows...)\n");
	}
}


// Plugin interface

class DMediaMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "DMedia";
	}

	const char *getId() const {
		return "dmedia";
	}

	MusicDevices getDevices() const;
	PluginError createInstance(Audio::Mixer *mixer, MidiDriver **mididriver) const;
};

MusicDevices DMediaMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

PluginError DMediaMusicPlugin::createInstance(Audio::Mixer *mixer, MidiDriver **mididriver) const {
	*mididriver = new MidiDriver_DMEDIA();

	return kNoError;
}

MidiDriver *MidiDriver_DMEDIA_create(Audio::Mixer *mixer) {
	MidiDriver *mididriver;

	DMediaMusicPlugin p;
	p.createInstance(mixer, &mididriver);

	return mididriver;
}

//#if PLUGIN_ENABLED_DYNAMIC(DMEDIA)
	//REGISTER_PLUGIN_DYNAMIC(DMEDIA, PLUGIN_TYPE_MUSIC, DMediaMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(DMEDIA, PLUGIN_TYPE_MUSIC, DMediaMusicPlugin);
//#endif

#endif
