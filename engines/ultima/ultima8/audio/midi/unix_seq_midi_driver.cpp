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
 */

/*
  Unix MIDI sequencer
  Adapted from ScummVM's backends/midi/seq.cpp
*/

#ifdef USE_UNIX_SEQ_MIDI

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/audio/midi/unix_seq_midi_driver.h"

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

namespace Ultima8 {

const MidiDriver::MidiDriverDesc UnixSeqMidiDriver::desc =
    MidiDriver::MidiDriverDesc("UnixSeqDevice", createInstance);

#define SEQ_MIDIPUTC 5
#define SEQ_DEVICE "/dev/sequencer"

UnixSeqMidiDriver::UnixSeqMidiDriver()
	: isOpen(false), device(0), deviceNum(0) {
	// see if the config file specifies an alternate midi device
	devname = getConfigSetting("unixseqdevice", SEQ_DEVICE);
}

int UnixSeqMidiDriver::open() {
	if (isOpen) return 1;
	isOpen = true;
	device = 0;

	pout << "UnixSeqDevice: opening device: " << devname << std::endl;

	device = ::open(devname.c_str(), O_RDWR, 0);
	if (device < 0) {
		perr << "UnixSeqDevice: failed: " << strerror(errno);
		perr << std::endl;
		return device;
	}

	return 0;
}

void UnixSeqMidiDriver::close() {
	::close(device);
	isOpen = false;
}

void UnixSeqMidiDriver::send(uint32 b) {
	unsigned char buf[256];
	int position = 0;

	switch (b & 0xF0) {
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0xE0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)b;
		buf[position++] = deviceNum;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 8) & 0x7F);
		buf[position++] = deviceNum;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 16) & 0x7F);
		buf[position++] = deviceNum;
		buf[position++] = 0;
		break;
	case 0xC0:
	case 0xD0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)b;
		buf[position++] = deviceNum;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 8) & 0x7F);
		buf[position++] = deviceNum;
		buf[position++] = 0;
		break;
	default:
		perr << "UnixSeqMidiDriver: Unknown Command: "
		     << std::hex << (int)b << std::dec << std::endl;
		break;
	}

	::write(device, buf, position);
}

void UnixSeqMidiDriver::send_sysex(uint8 status, const uint8 *msg, uint16 length) {
	if (length > 511) {
		perr << "UnixSeqMidiDriver: "
		     << "Cannot send SysEx block - data too large" << std::endl;
		return;
	}

	unsigned char buf [2048];
	int position = 0;
	const uint8 *chr = msg;

	buf[position++] = SEQ_MIDIPUTC;
	buf[position++] = status;
	buf[position++] = deviceNum;
	buf[position++] = 0;

	for (; length; --length) {
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = *chr++;
		buf[position++] = deviceNum;
		buf[position++] = 0;
	}

	::write(device, buf, position);
}

} // End of namespace Ultima8

#endif
