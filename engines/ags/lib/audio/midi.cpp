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
#include "ags/music.h"
#include "common/textconsole.h"

namespace AGS3 {

static byte dummy_driver_data[1] = { 0 };

BEGIN_MIDI_DRIVER_LIST
{ SCUMMVM_ID, &dummy_driver_data, true },
END_MIDI_DRIVER_LIST

int midi_card;

int midi_input_card;

volatile long midi_pos;       /* current position in the midi file, in beats */
volatile long midi_time;      /* current position in the midi file, in seconds */

long midi_loop_start;         /* where to loop back to at EOF */
long midi_loop_end;           /* loop when we hit this position */


int detect_midi_driver(int driver_id) {
	return 16;
}


void stop_midi() {
	::AGS::g_music->stop();
}

void destroy_midi(MIDI *midi) {
	delete midi;
}

int play_midi(MIDI *tune, bool repeat) {
	::AGS::g_music->playMusic(tune, repeat);
	return 0;
}

size_t get_midi_length(MIDI *tune) {
	warning("TODO: get_midi_length");
	return 0;
}

void midi_seek(int target) {
	::AGS::g_music->seek(target);
}

void midi_pause() {
	::AGS::g_music->pause();
}

void midi_resume() {
	::AGS::g_music->resume();
}

int load_midi_patches() {
	warning("TODO: load_midi_patches");
	return 0;
}

#define MIDI_BLOCK_SIZE 32768

MIDI *load_midi_pf(PACKFILE *fp) {
	MIDI *midi = new MIDI();
	int bytesRead;

	// Iterate through loading blocks of MIDI data
	for (;;) {
		size_t priorSize = midi->size();
		midi->resize(priorSize + MIDI_BLOCK_SIZE);
		bytesRead = pack_fread(&(*midi)[priorSize], MIDI_BLOCK_SIZE, fp);

		if (bytesRead < MIDI_BLOCK_SIZE) {
			midi->resize(priorSize + bytesRead);
			break;
		}
	}

	return midi;
}

} // namespace AGS3
