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

#ifndef ZVISION_MIDI_H
#define ZVISION_MIDI_H

class MidiDriver;

namespace ZVision {

class MidiManager {
public:
	MidiManager();
	~MidiManager();

	void stop();
	void noteOn(uint8 channel, uint8 noteNumber, uint8 velocity);
	void noteOff(uint8 channel);
	void setVolume(uint8 channel, uint8 volume);
	void setBalance(uint8 channel, int8 balance);
	void setPan(uint8 channel, int8 pan);
	void setProgram(uint8 channel, uint8 prog);

	int8 getFreeChannel();  // Negative if none available
	bool isAvailable() const {
		return _available;
	}

protected:
	bool _available = false;
	bool _mt32 = false;
	struct chan {
		bool playing;
		uint8 note;

		chan() : playing(false), note(0) {}
	};
	void send(uint8 status, uint8 data1 = 0x00, uint8 data2 = 0x00);
	uint8 _startChannel = 0;
	uint8 _maxChannels = 16;
	MidiDriver *_driver;
	chan _activeChannels[16];
};

}

#endif
