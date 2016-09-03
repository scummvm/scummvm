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

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "audio/mididrv.h"

#include "zvision/sound/midi.h"

namespace ZVision {

MidiManager::MidiManager() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	_driver = MidiDriver::createMidi(dev);
	if (_driver->open())
		warning("Can't open MIDI, no MIDI output!");
}

MidiManager::~MidiManager() {
	stop();
	_driver->close();
	delete _driver;
}

void MidiManager::stop() {
	for (int8 i = 0; i < 16; i++)
		if (_playChannels[i].playing)
			noteOff(i);
}

void MidiManager::noteOn(int8 channel, int8 note, int8 velocity) {
	assert(channel <= 15);

	_playChannels[channel].playing = true;
	_playChannels[channel].note = note;
	_driver->send(channel | (velocity << 16) | (note << 8) | 0x90);
}

void MidiManager::noteOff(int8 channel) {
	assert(channel <= 15);

	if (_playChannels[channel].playing) {
		_playChannels[channel].playing = false;
		_driver->send(channel | (_playChannels[channel].note << 8) | 0x80);
	}
}

int8 MidiManager::getFreeChannel() {
	for (int8 i = 0; i < 16; i++)
		if (!_playChannels[i].playing)
			return i;
	return -1;
}

void MidiManager::setPan(int8 channel, int8 pan) {
	assert(channel <= 15);

	_driver->send(channel | (pan << 16) | 0xAB0);
}

void MidiManager::setVolume(int8 channel, int8 volume) {
	assert(channel <= 15);

	_driver->send(channel | (volume << 16) | 0x7B0);
}

void MidiManager::setProgram(int8 channel, int8 prog) {
	assert(channel <= 15);

	_driver->send(channel | (prog << 8) | 0xC0);
}

} // End of namespace ZVision
