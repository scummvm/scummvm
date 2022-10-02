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

#include "common/str.h"
#include "common/system.h"
#include "base/version.h"
#include "scumm/imuse/drivers/mt32.h"

namespace Scumm {

IMuseDriver_MT32::IMuseDriver_MT32(MidiDriver::DeviceHandle dev, bool newSystem) : MidiDriver(), _newSystem(newSystem), _drv(nullptr) {
	_drv = MidiDriver::createMidi(dev);
	assert(_drv);

	if (!_newSystem)
		_drv->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
}

IMuseDriver_MT32::~IMuseDriver_MT32() {
	delete _drv;
}

int IMuseDriver_MT32::open() {
	if (!_drv)
		return MERR_CANNOT_CONNECT;

	int res = _drv->open();

	initDevice();

	return res;
}

MidiChannel *IMuseDriver_MT32::allocateChannel() {
	if (!_newSystem)
		return _drv->allocateChannel();

	return nullptr;
}

MidiChannel *IMuseDriver_MT32::getPercussionChannel() {
	if (!_newSystem)
		return _drv->getPercussionChannel();

	return nullptr;
}

void IMuseDriver_MT32::initDevice() {
	byte buffer[52];

	// Reset the MT-32
	_drv->sysEx((const byte *) "\x41\x10\x16\x12\x7f\x00\x00\x01\x00", 9);
	g_system->delayMillis(250);

	// Setup master tune, reverb mode, reverb time, reverb level,
	// channel mapping, partial reserve and master volume
	_drv->sysEx((const byte *) "\x41\x10\x16\x12\x10\x00\x00\x40\x00\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x64\x77", 31);
	g_system->delayMillis(250);

	// Map percussion to notes 24 - 34 without reverb
	if (!_newSystem) {
		_drv->sysEx((const byte *) "\x41\x10\x16\x12\x03\x01\x10\x40\x64\x07\x00\x4a\x64\x06\x00\x41\x64\x07\x00\x4b\x64\x08\x00\x45\x64\x06\x00\x44\x64\x0b\x00\x51\x64\x05\x00\x43\x64\x08\x00\x50\x64\x07\x00\x42\x64\x03\x00\x4c\x64\x07\x00\x44", 52);
		g_system->delayMillis(250);
	}

	// Compute version string (truncated to 20 chars max.)
	Common::String infoStr = "ScummVM ";
	infoStr += gScummVMVersion;
	int len = infoStr.size();
	if (len > 20)
		len = 20;

	// Display a welcome message on MT-32 displays.
	memcpy(&buffer[0], "\x41\x10\x16\x12\x20\x00\x00", 7);
	memcpy(&buffer[7], "                    ", 20);
	memcpy(buffer + 7 + (20 - len) / 2, infoStr.c_str(), len);
	byte checksum = 0;
	for (int i = 4; i < 27; ++i)
		checksum -= buffer[i];
	buffer[27] = checksum & 0x7F;
	_drv->sysEx(buffer, 28);
	g_system->delayMillis(1000);
}

} // End of namespace Scumm
