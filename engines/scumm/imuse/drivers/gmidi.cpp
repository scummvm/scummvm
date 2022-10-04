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

#include "common/debug.h"
#include "common/system.h"
#include "scumm/imuse/drivers/gmidi.h"

namespace Scumm {

IMuseDriver_GMidi::IMuseDriver_GMidi(MidiDriver::DeviceHandle dev, bool rolandGSMode, bool newSystem) : MidiDriver(), _drv(nullptr), _gsMode(rolandGSMode), _newSystem(newSystem) {
	_drv = MidiDriver::createMidi(dev);
	assert(_drv);
}

IMuseDriver_GMidi::~IMuseDriver_GMidi() {
	delete _drv;
}

int IMuseDriver_GMidi::open() {
	if (!_drv)
		return MERR_CANNOT_CONNECT;

	int res = _drv->open();
	if (res)
		return res;

	if (_gsMode)
		initDeviceAsRolandGS();
	else
		initDevice();

	return res;
}

void IMuseDriver_GMidi::close() {
	if (isOpen() && _drv)
		_drv->close();

	//releaseChannels();
}

MidiChannel *IMuseDriver_GMidi::allocateChannel() {
	if (!isOpen())
		return nullptr;

	// Pass through everything for now.
	//if (!_newSystem)
		return _drv->allocateChannel();

	return nullptr;
}

MidiChannel *IMuseDriver_GMidi::getPercussionChannel() {
	if (!isOpen())
		return nullptr;

	// Pass through everything for now.
	//if (!_newSystem)
		return _drv->getPercussionChannel();

	return nullptr;
}

void IMuseDriver_GMidi::initDevice() {
	// These are the init messages from the DOTT General Midi
	// driver. This is the major part of the bug fix for bug
	// no. 13460 ("DOTT: Incorrect MIDI pitch bending").
	// SAMNMAX has some less of the default settings (since
	// the driver works a bit different), but it uses the same
	// values for the pitch bend range.
	for (int i = 0; i < 16; ++i) {
		send(0x0064B0 | i);
		send(0x0065B0 | i);
		send(0x1006B0 | i);
		send(0x7F07B0 | i);
		send(0x3F0AB0 | i);
		send(0x0000C0 | i);
		send(0x4000E0 | i);
		send(0x0001B0 | i);
		send(0x0040B0 | i);
		send(0x405BB0 | i);
		send(0x005DB0 | i);
		send(0x0000B0 | i);
		send(0x007BB0 | i);
	}
}

void IMuseDriver_GMidi::initDeviceAsRolandGS() {
	byte buffer[12];
	int i;

	// General MIDI System On message
	// Resets all GM devices to default settings
	memcpy(&buffer[0], "\x7E\x7F\x09\x01", 4);
	sysEx(buffer, 4);
	debug(2, "GM SysEx: GM System On");
	g_system->delayMillis(200);

	// All GS devices recognize the GS Reset command,
	// even using Roland's ID. It is impractical to
	// support other manufacturers' devices for
	// further GS settings, as there are limitless
	// numbers of them out there that would each
	// require individual SysEx commands with unique IDs.

	// Roland GS SysEx ID
	memcpy(&buffer[0], "\x41\x10\x42\x12", 4);

	// GS Reset
	memcpy(&buffer[4], "\x40\x00\x7F\x00\x41", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: GS Reset");
	g_system->delayMillis(200);

	// Set global Master Tune to 442.0kHz, as on the MT-32
	memcpy(&buffer[4], "\x40\x00\x00\x00\x04\x04\x0F\x29", 8);
	sysEx(buffer, 12);
	debug(2, "GS SysEx: Master Tune set to 442.0kHz");

	// Note: All Roland GS devices support CM-64/32L maps

	// Set Channels 1-16 to SC-55 Map, then CM-64/32L Variation
	for (i = 0; i < 16; ++i) {
		_drv->send((127 << 16) | (0  << 8) | (0xB0 | i));
		_drv->send((1   << 16) | (32 << 8) | (0xB0 | i));
		_drv->send((0   << 16) | (0  << 8) | (0xC0 | i));
	}
	debug(2, "GS Program Change: CM-64/32L Map Selected");

	// Set Percussion Channel to SC-55 Map (CC#32, 01H), then
	// Switch Drum Map to CM-64/32L (MT-32 Compatible Drums)
	getPercussionChannel()->controlChange(0, 0);
	getPercussionChannel()->controlChange(32, 1);
	send(127 << 8 | 0xC0 | 9);
	debug(2, "GS Program Change: Drum Map is CM-64/32L");

	// Set Master Chorus to 0. The MT-32 has no chorus capability.
	memcpy(&buffer[4], "\x40\x01\x3A\x00\x05", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Master Chorus Level is 0");

	// Set Channels 1-16 Reverb to 64, which is the
	// equivalent of MT-32 default Reverb Level 5
	for (i = 0; i < 16; ++i)
		send((64 << 16) | (91 << 8) | (0xB0 | i));
	debug(2, "GM Controller 91 Change: Channels 1-16 Reverb Level is 64");

	// Set Channels 1-16 Pitch Bend Sensitivity to
	// 12 semitones; then lock the RPN by setting null.
	for (i = 0; i < 16; ++i)
		setPitchBendRange(i, 12);
	debug(2, "GM Controller 6 Change: Channels 1-16 Pitch Bend Sensitivity is 12 semitones");

	// Set channels 1-16 Mod. LFO1 Pitch Depth to 4
	memcpy(&buffer[4], "\x40\x20\x04\x04\x18", 5);
	for (i = 0; i < 16; ++i) {
		buffer[5] = 0x20 + i;
		buffer[8] = 0x18 - i;
		sysEx(buffer, 9);
	}

	debug(2, "GS SysEx: Channels 1-16 Mod. LFO1 Pitch Depth Level is 4");

	// Set Percussion Channel Expression to 80
	getPercussionChannel()->controlChange(11, 80);
	debug(2, "GM Controller 11 Change: Percussion Channel Expression Level is 80");

	// Turn off Percussion Channel Rx. Expression so that
	// Expression cannot be modified. I don't know why, but
	// Roland does it this way.
	memcpy(&buffer[4], "\x40\x10\x0E\x00\x22", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Percussion Channel Rx. Expression is OFF");

	// Change Reverb Character to 0. I don't think this
	// sounds most like MT-32, but apparently Roland does.
	memcpy(&buffer[4], "\x40\x01\x31\x00\x0E", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Reverb Character is 0");

	// Change Reverb Pre-LF to 4, which is similar to
	// what MT-32 reverb does.
	memcpy(&buffer[4], "\x40\x01\x32\x04\x09", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Reverb Pre-LF is 4");

	// Change Reverb Time to 106; the decay on Hall 2
	// Reverb is too fast compared to the MT-32's
	memcpy(&buffer[4], "\x40\x01\x34\x6A\x21", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Reverb Time is 106");
}

} // End of namespace Scumm
