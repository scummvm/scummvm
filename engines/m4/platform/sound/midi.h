
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

#ifndef M4_SOUND_PLATFORM_MIDI_H
#define M4_SOUND_PLATFORM_MIDI_H

#include "audio/midiplayer.h"
#include "m4/m4_types.h"

namespace M4 {
namespace Sound {

class Midi : public Audio::MidiPlayer {
private:
	static int _midiEndTrigger;
	Audio::Mixer *_mixer;
public:
	Midi(Audio::Mixer *mixer);

	void midi_play(const char *name, int volume, int loop, int trigger, int roomNum);
	void task();
	void loop();
	void set_overall_volume(int vol);
	int get_overall_volume() const;
};

} // namespace Sound

void midi_play(const char *name, int volume, int loop, int trigger, int roomNum);
void midi_loop();
void midi_stop();
void midi_set_overall_volume(int vol);
int midi_get_overall_volume();
void midi_fade_volume(int val1, int val2);

} // namespace M4

#endif
