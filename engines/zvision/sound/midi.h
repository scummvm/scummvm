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

#ifndef ZVISION_MIDI_H
#define ZVISION_MIDI_H

class MidiDriver;

namespace ZVision {

class MidiManager {
public:
	MidiManager();
	~MidiManager();

	void stop();
	void noteOn(int8 channel, int8 noteNumber, int8 velocity);
	void noteOff(int8 channel);
	void setPan(int8 channel, int8 pan);
	void setVolume(int8 channel, int8 volume);
	void setProgram(int8 channel, int8 prog);

	int8 getFreeChannel();

protected:

	struct chan {
		bool playing;
		int8 note;

		chan() : playing(false), note(0) {};
	};

	MidiDriver *_driver;
	chan _playChannels[16];
};

}

#endif
