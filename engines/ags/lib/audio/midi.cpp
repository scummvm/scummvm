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

#include "ags/lib/audio/midi.h"
#include "ags/lib/allegro/file.h"
#include "common/textconsole.h"

namespace AGS3 {

BEGIN_MIDI_DRIVER_LIST
END_MIDI_DRIVER_LIST

MIDI_DRIVER *midi_driver;

MIDI_DRIVER *midi_input_driver;

int midi_card;

int midi_input_card;

volatile long midi_pos;       /* current position in the midi file, in beats */
volatile long midi_time;      /* current position in the midi file, in seconds */

long midi_loop_start;         /* where to loop back to at EOF */
long midi_loop_end;           /* loop when we hit this position */


int detect_midi_driver(int driver_id) {
	return 0;
}


void stop_midi() {
	warning("TODO: stop_midi");
}

void destroy_midi(MIDI *midi) {
	delete midi;
}

int play_midi(MIDI *tune, bool repeat) {
	warning("TODO: play_midi");
	return 0;
}

size_t get_midi_length(MIDI *tune) {
	warning("TODO: get_midi_length");
	return 0;
}

void midi_seek(int target) {
	warning("TODO: midi_seek");
}

void midi_pause() {
	warning("TODO: midi_pause");
}

void midi_resume() {
	warning("TODO: midi_resume");
}

int load_midi_patches() {
	warning("TODO: load_midi_patches");
	return 0;
}

MIDI *load_midi_pf(PACKFILE *fp) {
	error("TODO: load_midi_pf");
}

} // namespace AGS3
