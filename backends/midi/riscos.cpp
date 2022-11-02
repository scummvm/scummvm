/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef RISCOS

#include "common/error.h"
#include "common/textconsole.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <kernel.h>
#include <swis.h>

#ifndef MIDI_TxByte
#define MIDI_TxByte 0x404C9
#endif
#ifndef MIDI_TxCommand
#define MIDI_TxCommand 0x404CA
#endif


class MidiDriver_RISCOS final : public MidiDriver_MPU401 {
public:
	MidiDriver_RISCOS(int port = 0) : _isOpen(false), _port(port) {}
	~MidiDriver_RISCOS() {}

	int open() override;
	bool isOpen() const override { return _isOpen; }
	void close() override;
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;

private:
	_kernel_oserror *txByte(int r0);
	_kernel_oserror *txCommand(int r0, int r1);

	bool _isOpen;
	int _port;
};

int MidiDriver_RISCOS::open() {
	if (isOpen())
		return MERR_ALREADY_OPEN;

	_isOpen = true;
	return 0;
}

void MidiDriver_RISCOS::close() {
	MidiDriver_MPU401::close();
	_isOpen = false;
}

void MidiDriver_RISCOS::send(uint32 b) {
	assert(isOpen());

	midiDriverCommonSend(b);

	// Extract the MIDI data
	byte status_byte = (b & 0x000000FF);
	// byte first_byte = (b & 0x0000FF00) >> 8;
	// byte second_byte = (b & 0x00FF0000) >> 16;

	// Compute the correct length of the MIDI command. This is important,
	// else things may screw up badly...
	byte length;
	switch (status_byte & 0xF0) {
	case 0x80:	// Note Off
	case 0x90:	// Note On
	case 0xA0:	// Polyphonic Aftertouch
	case 0xB0:	// Controller Change
	case 0xE0:	// Pitch Bending
		length = 3;
		break;
	case 0xC0:	// Programm Change
	case 0xD0:	// Monophonic Aftertouch
		length = 2;
		break;
	default:
		warning("RISC OS driver encountered unsupported status byte: 0x%02x", status_byte);
		length = 3;
		break;
	}

	// Finally send it out to the synthesizer.
	txCommand(b | (length << 24) | (_port << 28), 0);
}

void MidiDriver_RISCOS::sysEx(const byte *msg, uint16 length) {
	assert(isOpen());

	int port = (_port << 28);

	txByte(0xF0 | port);
	for (; length; --length, ++msg) {
		txByte(*msg | port);
	}
	txByte(0xF7 | port);
}

_kernel_oserror *MidiDriver_RISCOS::txByte(int r0) {
	_kernel_swi_regs regs;
	regs.r[0] = r0;
	return _kernel_swi(MIDI_TxByte, &regs, &regs);
}

_kernel_oserror *MidiDriver_RISCOS::txCommand(int r0, int r1) {
	_kernel_swi_regs regs;
	regs.r[0] = r0;
	regs.r[1] = r1;
	return _kernel_swi(MIDI_TxCommand, &regs, &regs);
}


// Plugin interface

class RISCOSMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "RISC OS MIDI";
	}

	const char *getId() const {
		return "riscos";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices RISCOSMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error RISCOSMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle device) const {
	*mididriver = new MidiDriver_RISCOS();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(RISCOS)
	//REGISTER_PLUGIN_DYNAMIC(RISCOS, PLUGIN_TYPE_MUSIC, RISCOSMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(RISCOS, PLUGIN_TYPE_MUSIC, RISCOSMusicPlugin);
//#endif

#endif // RISCOS
