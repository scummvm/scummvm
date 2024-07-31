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

#include "m4/platform/sound/midi.h"
#include "m4/vars.h"

namespace M4 {
namespace Sound {

void Midi::midi_play(const char *name, int volume, int loop, int trigger, int roomNum) {
	warning("TODO: midi_play");
}

void Midi::task() {
	// No implementation
}

void Midi::loop() {
	// No implementation
}

void Midi::stop() {
	// No implementation
}

void Midi::set_overall_volume(int vol) {
	// No implementation
}

} // namespace Sound

void midi_play(const char *name, int volume, int loop, int trigger, int roomNum) {
	_G(midi).midi_play(name, volume, loop, trigger, roomNum);
}

void midi_loop() {
	_G(midi).loop();
}

void midi_stop() {
	_G(midi).stop();
}

void midi_set_overall_volume(int vol) {
	_G(midi).set_overall_volume(vol);
}

void midi_fade_volume(int val1, int val2) {
	warning("TODO: midi_fade_volume");
}

} // namespace M4
