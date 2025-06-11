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

#include "audio/mididrv.h"
#include "common/debug.h"
#include "common/scummsys.h"
#include "common/textconsole.h"
#include "zvision/detection.h"
#include "zvision/sound/midi.h"

namespace ZVision {

MidiManager::MidiManager() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	_driver = MidiDriver::createMidi(dev);
	if (_driver->open()) {
		warning("Can't open MIDI, no MIDI output!");
		_available = false;
	} else {
		Common::String driverName = MidiDriver::getDeviceString(dev, MidiDriver::DeviceStringType::kDriverName);
		Common::String deviceName = MidiDriver::getDeviceString(dev, MidiDriver::DeviceStringType::kDeviceName);
		_mt32 = MidiDriver::getMusicType(dev) == MT_MT32;
		debugC(1, kDebugSound, "MIDI opened, driver type: %s, device name: %s", driverName.c_str(), deviceName.c_str());
		_available = true;
		_maxChannels = _driver->MIDI_CHANNEL_COUNT;
	}
}

MidiManager::~MidiManager() {
	stop();
	_driver->close();
	delete _driver;
}

void MidiManager::send(uint8 status, uint8 data1, uint8 data2) {
	assert(status & 0x80 && "Malformed MIDI status byte");
	assert(!(data1 & 0x80) && "Malformed MIDI data byte 1");
	assert(!(data2 & 0x80) && "Malformed MIDI data byte 2");
	_driver->send(status | (data1 << 8) | (data2 << 16));
}

void MidiManager::stop() {
	for (uint8 i = 0; i < 16; i++)
		noteOff(i);
}

void MidiManager::noteOn(uint8 channel, uint8 note, uint8 velocity) {
	assert(channel <= 15);

	_activeChannels[channel].playing = true;
	_activeChannels[channel].note = note;
	send(0x90 | channel, note, velocity);
	debugC(1, kDebugSound, "MIDI note on, channel %d, note %d, velocity %d", channel, note, velocity);
}

void MidiManager::noteOff(uint8 channel) {
	assert(channel <= 15);

	if (_activeChannels[channel].playing) {
		_activeChannels[channel].playing = false;
		send(0x80 | channel, _activeChannels[channel].note);
	}
}

int8 MidiManager::getFreeChannel() {
	uint8 start = _mt32 ? 1 : 0; // MT-32 can be used for MIDI, but does not play anything on MIDI channel 0
	for (uint8 i = start; i < 16; i++)
		if (!_activeChannels[i].playing)
			return i;
	return -1;
}

void MidiManager::setVolume(uint8 channel, uint8 volume) {
	assert(channel <= 15);
	debugC(1, kDebugSound, "MIDI volume out %d", volume >> 1);
	send(0xB0 | channel, 0x07, volume >> 1);
}

void MidiManager::setBalance(uint8 channel, int8 balance) {
	assert(channel <= 15);
	uint8 _balance = (uint8)(balance + 128);
	debugC(1, kDebugSound, "MIDI balance out %d", _balance >> 1);
	send(0xB0 | channel, 0x08, _balance >> 1);
}

void MidiManager::setPan(uint8 channel, int8 pan) {
	assert(channel <= 15);
	uint8 _pan = (uint8)(pan + 128);
	debugC(1, kDebugSound, "MIDI pan in %d, out %d", pan, _pan >> 1);
	send(0xB0 | channel, 0x0A, _pan >> 1);
}

void MidiManager::setProgram(uint8 channel, uint8 prog) {
	assert(channel <= 15);
	send(0xC0 | channel, prog);
}

} // End of namespace ZVision
