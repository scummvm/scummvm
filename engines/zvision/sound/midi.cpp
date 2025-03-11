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

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "audio/mididrv.h"

#include "zvision/sound/midi.h"
#include "common/debug.h"

namespace ZVision {

MidiManager::MidiManager() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);  //NB Bug current in MT-32 emulation: MIDI channel 0 does not play anything!
	_driver = MidiDriver::createMidi(dev);
  if (_driver->open()) {
	  warning("Can't open MIDI, no MIDI output!");
	  available = false;
	  }
	else {
	  Common::String driverName = MidiDriver::getDeviceString(dev,MidiDriver::DeviceStringType::kDriverName);
	  Common::String deviceName = MidiDriver::getDeviceString(dev,MidiDriver::DeviceStringType::kDeviceName);
	  mt32workaround = driverName.contains("MT-32");
    debug(1,"MIDI opened, driver type: %s, device name: %s", driverName.c_str(), deviceName.c_str());
    if(mt32workaround)
      debug(1,"Blocking use of MIDI channel 0 to work around bug in MT-32 emulation.");
	  available = true;
	  maxChannels = _driver->MIDI_CHANNEL_COUNT;
  }
}

MidiManager::~MidiManager() {
	stop();
	_driver->close();
	delete _driver;
}

void MidiManager::stop() {
	for (int8 i = 0; i < 16; i++)
		if (activeChannels[i].playing)
			noteOff(i);
}

void MidiManager::noteOn(int8 channel, int8 note, int8 velocity) {
	assert(channel <= 15);
	activeChannels[channel].playing = true;
	activeChannels[channel].note = note;
	_driver->send(channel | (velocity << 16) | (note << 8) | 0x90);
	debug(1,"MIDI note on, channel %d, note %d, velocity %d", channel, note, velocity);
}

void MidiManager::noteOff(int8 channel) {
	assert(channel <= 15);

	if (activeChannels[channel].playing) {
		activeChannels[channel].playing = false;
		_driver->send(channel | (activeChannels[channel].note << 8) | 0x80);
	}
}

int8 MidiManager::getFreeChannel() {
  uint8 start = mt32workaround ? 1 : 0; //MT-32 emulator driver currently fails to play anything when sent notes on channel 0.  Quick & dirty fix, since we only need a few notes for Nemesis music puzzles.  Inform maintainer of MT-32 emulator code of bug for proper fix in the future & then remove this nasty hack once that's sorted out.
	for (int8 i = start; i < 16; i++)
		if (!activeChannels[i].playing)
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
